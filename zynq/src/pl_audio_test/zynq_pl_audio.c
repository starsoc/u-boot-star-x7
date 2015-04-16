
/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiic.h"
#include "xil_exception.h"
#include "xgpio.h"
#include "ff.h"

/* Definitions for peripheral AXI_CLKGEN_0 */
#define XPAR_AUDIO_I2S_0_BASEADDR 0x79000000


/************************** Constant Definitions *****************************/
#define IIC_DEVICE_ID	XPAR_IIC_0_DEVICE_ID
#define IIC_BASE_ADDRESS	XPAR_IIC_0_BASEADDR
#define WM8731_IIC_ADDRESS   0x1A              /* WM8731 7bit Audio i2c address */

#define WORD_SWAP(X) (((X)&0xff)<<24)+      \
		    (((X)&0xff00)<<8)+      \
		    (((X)&0xff0000)>>8)+    \
		    (((X)&0xff000000)>>24)


/**************************** Type Definitions *******************************/
typedef u8 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int wm8731_iic_intial();
void wm8731_iic_write(u8 regaddr, u8 regdata);




/************************** Variable Definitions *****************************/



/*****************************************************************************/
/**
* Main function.
*
******************************************************************************/
int pl_audio_init(void)
{
	int Status;
	u32 receive_data,send_data, send_data_swap;
	u32 key_data;
	u32 *audio_low_address = (u32 *)0x20000000;
	u32 *audio_high_address = (u32 *)0x20200000;
    
	u32 *audio_read_address;
	u32 *audio_write_address;
    u32 *audio_read_last_address;
	u32 audio_reg0,audio_reg1;
	u32 i = 0;
    u32 i_read_index = 0;
    XGpio Gpio_keys;
    XGpio GpioOutput;
    
	/* initial wm8731 register. */
	Status = wm8731_iic_intial();
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    
    /* initial Key button. */
    XGpio_Initialize(&Gpio_keys, XPAR_KEY_IO_DEVICE_ID);
    XGpio_SetDataDirection(&Gpio_keys, 1, 0xFFFFFFFF);
    XGpio_Initialize(&GpioOutput, XPAR_LED_IO_DEVICE_ID);
    XGpio_SetDataDirection(&GpioOutput, 1, 0x0);
    
    /* clear buffer of audio area */
    for (audio_write_address = audio_low_address; audio_write_address < audio_high_address;audio_write_address++ )
    {
        *(audio_write_address) = 0;
        i++;
    }
    
    audio_write_address = audio_low_address;
    audio_read_address = audio_low_address;
    
    printf("audio write addr:0x%x, audio read addr:0x%x\r\n", audio_write_address, audio_read_address);
    
    while(1)
    {
        key_data = XGpio_DiscreteRead(&Gpio_keys, 1);		           // read key status
        usleep(300000);
        XGpio_DiscreteWrite(&GpioOutput, 1, key_data);
        audio_reg1 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000004);        // read register 1
        
        /****************************以下为录音的代码***************************/
    
        /*if Key1 is touched, record the voice to DDR from wm8731*/
        if ((key_data & 0x01) == 0x00) 
        {
            if((audio_reg1 & 0x02)== 0x00)               //if receive FIFO is not empty
            {
                audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000); //read the value of register0
                while ((audio_reg0 & 0x2) != 0x0) 
                {                          //wait until bit1 is 0
                    audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);
                }

                Xil_Out32((XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000),(audio_reg0 | 0x2));    //set bit1 of register0
                audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);   //read the value of register0
                while ((audio_reg0 & 0x2) != 0x0)
                {                              //wait until bit1 is 0
                    audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);
                }

                receive_data = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x0000000C);   //read receive data from register3
                
                *audio_write_address  = receive_data;                             //save the voice data to DDR
                audio_write_address++;

                if (audio_write_address >= audio_high_address) 
                {
                    audio_write_address = audio_low_address;
                }
            }
        }

        /*if key2 is touched, send the record voice to wm8731 from DDR*/
        else if ((key_data & 0x02) == 0x00) 
        {
            if((audio_reg1 & 0x04)== 0x00)               //if send FIFO is not full
            {
                audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000); //read the value of register0
                while ((audio_reg0 & 0x1) != 0x0) 
                {                            //wait until bit0 is 0
                    audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);
                }
                
                send_data = *audio_read_address;
                audio_read_address++;
                Xil_Out32((XPAR_AUDIO_I2S_0_BASEADDR + 0x00000008),send_data);             //write the data to tx fifo
                Xil_Out32((XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000),(audio_reg0 | 0x1));    //set bit0 of register0
                audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);             //read the value of register0
                while ((audio_reg0 & 0x1) != 0x0) 
                {                                        //wait until bit0 is 0
                    audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);
                }

                if (audio_read_address >= audio_high_address) 
                {
                    audio_read_address = audio_low_address;
                }
            }
        }
        else if ((key_data & 0x04) == 0x00) 
        {

            FRESULT rc;
            u32 SourceAddress = 0;
            u32 dest_data;
            u32 LengthBytes = 0x200000;            
            UINT br;
            
            static char buffer[80] = "shanghai.bin";
            // static char buffer[80] = "BOOT.BIN";
            char *boot_file = buffer;   
            static FIL fil;     /* File object */
            static FATFS fatfs;

            /* Register volume work area, initialize device */
            rc = f_mount(0, &fatfs);
            
            printf("SD: rc= %.8x\n\r", rc);
            
            printf("######begin broadcast music, LengthBytes:0x%x, high_addr:0x%x, low_addr:0x%x\r\n", 
                LengthBytes, audio_high_address, audio_low_address);
            
            if (rc != FR_OK) {
                return XST_FAILURE;
            }

            rc = f_open(&fil, buffer, FA_READ);
            if (rc) 
            {
                printf("SD: Unable to open file %s: %d\n", boot_file, rc);
                return XST_FAILURE;
            }
            do
            {
                printf("source addr:0x%x, read_addr:0x%x\r\n", SourceAddress, audio_read_address);
                rc = f_lseek(&fil, SourceAddress);
                if (rc) 
                {
                    printf("SD: Unable to seek to %x\n", SourceAddress);
                    return XST_FAILURE;
                }
                
                rc = f_read(&fil, (void*)audio_read_address, LengthBytes, &br);               
                if (rc) 
                {
                    printf("*** ERROR: f_read returned %d\r\n", rc);
                }
                SourceAddress += br;
                audio_read_address += (br/4);
                
            }while(br == LengthBytes);
            
            audio_read_last_address = audio_read_address;
            
            audio_read_address = audio_low_address;
            
            send_data = *audio_read_address;
            printf("######before boardcast, data:0x%x, last addr:0x%x\r\n", send_data, audio_read_last_address);
            
            while (audio_read_address < audio_read_last_address)
            {
                if((audio_reg1 & 0x04)== 0x00)               //if send FIFO is not full
                {
                    audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);  //read the value of register0
                    while ((audio_reg0 & 0x1) != 0x0) 
                    {                            //wait until bit0 is 0
                        audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);
                    }
                    send_data = *audio_read_address;
                    audio_read_address++;
                    
                    Xil_Out32((XPAR_AUDIO_I2S_0_BASEADDR + 0x00000008), send_data);             //write the data to tx fifo
                    Xil_Out32((XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000), (audio_reg0 | 0x1));    //set bit0 of register0
                    audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);             //read the value of register0
                    while ((audio_reg0 & 0x1) != 0x0) 
                    {                                        //wait until bit0 is 0
                        audio_reg0 = Xil_In32(XPAR_AUDIO_I2S_0_BASEADDR + 0x00000000);
                    }
                }
                usleep(15);
            }
            
            printf("######after boardcast, audio_read_address:0x%x, source_addr:0x%x\r\n", 
                    audio_read_address, SourceAddress);
            
            printf("\r\n");
            break;   
        }
    }
    

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* wm8731 initial
*
******************************************************************************/
int wm8731_iic_intial()
{
	int Status;
	u32 StatusReg;

    
	/*
	 * Initialize the IIC Core.
	 */
	Status = XIic_DynInit(IIC_BASE_ADDRESS);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}

	/*
	 * Make sure all the Fifo's are cleared and Bus is Not busy.
	 */
	while (((StatusReg = XIic_ReadReg(IIC_BASE_ADDRESS,
		XIIC_SR_REG_OFFSET)) &
		(XIIC_SR_RX_FIFO_EMPTY_MASK |
		XIIC_SR_TX_FIFO_EMPTY_MASK |
 		XIIC_SR_BUS_BUSY_MASK)) !=
 		(XIIC_SR_RX_FIFO_EMPTY_MASK |
		XIIC_SR_TX_FIFO_EMPTY_MASK)) 
	{
        
	}


	 /**********************WM8731 register initial*******************/

	wm8731_iic_write(0x0C,0x10);    /* Power down register(0x0C):0x10	     */
	wm8731_iic_write(0x00,0x97);    /* Left line in register(0x00):0x97      */
	wm8731_iic_write(0x02,0x97);    /* right line in register(0x02):0x97     */
	wm8731_iic_write(0x08,0x15);    /* MIC path control register(0x08):0x15  */
	wm8731_iic_write(0x0A,0x00);    /* Audio path control register(0x0A):0x00*/
	/***Loopback test******/
	//wm8731_iic_write(0x08,0x25);
	//wm8731_iic_write(0x0A,0x08);
	wm8731_iic_write(0x0E,0x42);    /* Digital audio format control register(0x0E):0x41; master mode, 16bit, i2s left justified*/
	wm8731_iic_write(0x10,0x00);	/* Sampling control register(0x10):0x00  */
	wm8731_iic_write(0x12,0x01);   	/* Active control(0x12):0x01             */
	wm8731_iic_write(0x0C,0x00); 	/* Power down register(0x0C):0x00        */

	return 0;

}

/*****************************************************************************/
/* This function writes a byte to address of the IIC serial Device.
******************************************************************************/
void wm8731_iic_write(u8 regaddr, u8 regdata)
{
	u8 WriteBuffer[2];
    u8 Wm8731IicAddr;       /* Variable for wm8731 IIC address */

	WriteBuffer[0] = regaddr;
	WriteBuffer[1] = regdata;
    
	Wm8731IicAddr = WM8731_IIC_ADDRESS;
    
	XIic_DynSend(IIC_BASE_ADDRESS, Wm8731IicAddr, WriteBuffer,2,XIIC_STOP);
}

        
