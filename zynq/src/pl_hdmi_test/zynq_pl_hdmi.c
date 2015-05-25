/*
 * Driver for the Zynq-7000 PSS I2C controller
 * IP from Cadence (ID T-CS-PE-0007-100, Version R1p10f2)
 *
 * Author: Joe Hershberger <joe.hershberger@ni.com>
 * Copyright (c) 2012 Joe Hershberger.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <asm/errno.h>

#include "xusbps_ch9.h"
#include "xusbps.h"


#include "xdevcfg_hw.h"
#include "xdevcfg.h"
#include "fsbl.h"
#include "xil_io.h"

#include "hdmi_header.h"
#include <malloc.h>
#include "xiicps.h"
#include "i2c.h"
#include "ff.h"

#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID
#define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define IIC_INTR_ID	    XPAR_XIICPS_0_INTR

/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 */
#define IIC_SCLK_RATE		100000

#define CF_CLKGEN_BASEADDR  XPAR_AXI_CLKGEN_0_BASEADDR
#define CFV_BASEADDR        XPAR_AXI_HDMI_TX_24B_0_BASEADDR
#define CFA_BASEADDR        0x75c00000
#define DDR_BASEADDR        0x00000000
#define UART_BASEADDR       0xe0001000
#define VDMA_BASEADDR       0x43000000
#define ADMA_BASEADDR       0x40400000
#define H_STRIDE            1920
#define H_COUNT             2200
#define H_ACTIVE            1920
#define H_WIDTH             44
#define H_FP                88
#define H_BP                148
#define V_COUNT             1125
#define V_ACTIVE            1080
#define V_WIDTH             5
#define V_FP                4
#define V_BP                36
#define A_SAMPLE_FREQ       48000
#define A_FREQ              1400

#define H_DE_MIN (H_WIDTH+H_BP)
#define H_DE_MAX (H_WIDTH+H_BP+H_ACTIVE)
#define V_DE_MIN (V_WIDTH+V_BP)
#define V_DE_MAX (V_WIDTH+V_BP+V_ACTIVE)
#define VIDEO_LENGTH  (H_ACTIVE*V_ACTIVE)
#define AUDIO_LENGTH  (A_SAMPLE_FREQ/A_FREQ)
#define VIDEO_BASEADDR DDR_BASEADDR + 0x02000000
#define AUDIO_BASEADDR DDR_BASEADDR + 0x01000000
#define DATA_READ_ADDR DDR_BASEADDR + 0x03000000;

//#define SII9022_I2C_ADDR        0x3b			 zturn board
#define SII9022_I2C_ADDR        0x39		     // star-x7 board


XIicPs Iic1Instance;		/* The instance of the IIC device. */
int g_image_len = 0;

int sii9022_i2c_init(int i2c_id)
{
	int Status;
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */
    
	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIicPs_LookupConfig(i2c_id);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
    printf("######sii9022_i2c_init(), i2c baseaddr:0x%x\r\n", ConfigPtr->BaseAddress);
	
	Status = XIicPs_CfgInitialize(&Iic1Instance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
    
	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIicPs_SelfTest(&Iic1Instance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    

	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&Iic1Instance, IIC_SCLK_RATE);
    
#if 0   
    u8 data[8];
	int i=0;
    i2c_read(ZC702_HDMI_ADDR1,0,0,data,8); 
    #if 1
	for(i = 0;i < 8;i++)
	{
        printf("%x ",data[i]);
	}
    printf("\n");
	printf("som initialize hdmi starting...\n\r");
    #endif
	// write Sii9134 configuration
	iic_writex( ZC702_HDMI_ADDR1, zc702_hdmi_out_config1, ZC702_HDMI_OUT_LEN1 );
    
    
	iic_writex( ZC702_HDMI_ADDR2, zc702_hdmi_out_config2, ZC702_HDMI_OUT_LEN2 );

	memset(data,0,8);
    i2c_read(ZC702_HDMI_ADDR1,0,0,data,8);  
        
    #if 1
	printf("i2c configure si9134 done!\n\r");
            
	for(i = 0;i < 8;i++)
	{
        printf("%x ",data[i]);
	}
    printf("\n");
    #endif
#endif 
    
    return 0;
}


/*****************************************************************************/
/*This function writes a data to the SiI9134 register.
******************************************************************************/
int SiI9022_write(u8 dev_id, u8 waddr, u8 wdata)
{
	int Status;

    
    u8 WriteBuffer[2];              /* Write buffer for writing register. */
    
	WriteBuffer[0]=waddr;
	WriteBuffer[1]=wdata;
    
	Status = XIicPs_MasterSendPolled(&Iic1Instance, WriteBuffer,
			 2, dev_id);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&Iic1Instance));

	/*
	 * Wait for a bit of time to allow the programming to complete
	 */
	mdelay(250);

	return XST_SUCCESS;
}



/*****************************************************************************/
/*This function read a data from the SiI9134 register.
******************************************************************************/
int SiI9022_read(u8 dev_id, u8 raddr, u8 ReadBuffer[])
{

	int Status;
    u8 WriteBuffer[2];              /* Write buffer for writing register. */
    

	WriteBuffer[0]=raddr;
	/** Send the Data. */
	Status = XIicPs_MasterSendPolled(&Iic1Instance, WriteBuffer,
			 1, dev_id);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&Iic1Instance));

	/*
	 * Wait for a bit of time to allow the programming to complete
	 */
	mdelay(250);

    
	Status = XIicPs_MasterRecvPolled(&Iic1Instance, ReadBuffer,
			  1, dev_id);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&Iic1Instance));

	return XST_SUCCESS;
}



void open_hdmi_pic()
{
	
    FRESULT rc;
    static char buffer[80] = "star.bin";
    char *boot_file = buffer;   
    static FIL fil;     /* File object */
    static FATFS fatfs;
    UINT br;
    u32 SourceAddress = 0;
	u32 LengthBytes = 8192;
    u32 Data_read_addr = DATA_READ_ADDR;
    u32 Data_read_Begin_addr = Data_read_addr;
    /* Register volume work area, initialize device */
    rc = f_mount(0, &fatfs);
    
    printf("read hdmi pic, SD: rc= %.8x\n\r", rc);
   	    
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
	    rc = f_lseek(&fil, SourceAddress);
	    if (rc) 
	    {
	        printf("SD: Unable to seek to %x\n", SourceAddress);
	        return XST_FAILURE;
	    }
	    
	    rc = f_read(&fil, (void*)Data_read_addr, LengthBytes, &br);               
	    if (rc) 
	    {
	        printf("*** ERROR: f_read returned %d\r\n", rc);
	    }
        
        printf("***Data_read_addr:0x%x, br:0x%x\r\n", Data_read_addr, br);
        printf("***read data:0x%x, 0x%x, 0x%x, 0x%x\r\n", 
            *((u32*)(Data_read_addr)), *((u32*)(Data_read_addr+1)), *((u32*)(Data_read_addr+2)), *((u32*)(Data_read_addr+3)));
        
	    SourceAddress += br;
	    Data_read_addr += (br/4);
	   	g_image_len += br;
        
	}while(br == LengthBytes);
    
	printf("image len:%d\r\n", g_image_len);
    
}



void ddr_video_wr() 
{
	
    u32 n;
    u32 d;
    u32 dcnt;
    u32 read_image_data = DATA_READ_ADDR;
    dcnt = 0;
	#if 0
    open_hdmi_pic();
	printf("%x, %x, %x, %x\r\n", 
        *((u32*)(read_image_data)), *((u32*)(read_image_data+1)), *((u32*)(read_image_data+2)), *((u32*)(read_image_data+3)));
	#endif
    #if 1
    printf("DDR write: started (length %d)\n\r", IMG_LENGTH);
    for (n = 0; n < IMG_LENGTH; n++) 
    {
        for (d = 0; d < ((IMG_DATA[n]>>24) & 0xff); d++) 
        {
            Xil_Out32((VIDEO_BASEADDR + (dcnt*4)), (IMG_DATA[n] & 0xffffff));
            dcnt = dcnt + 1;
        }
    }
    #endif
    printf("DDR write: completed (total %d)\n\r", dcnt);
}


int SiI9134_i2c_config(void)
{
    int Status;
    u8 ReadBuffer[2];    
    
    printf("######SiI9134_i2c_config()\r\n"); 
    
    sii9022_i2c_init(IIC_DEVICE_ID);
    
    
    SiI9022_write(0x39, 0x05, 0x01);        //soft reset
    SiI9022_write(0x39, 0x05, 0x00);        //soft reset
    SiI9022_write(0x39, 0x08, 0xfd);        //PD#=1,power on mode

    SiI9022_write(0x3d, 0x2f, 0x21);        //HDMI mode enable, 24bit per pixel(8 bits per channel; no packing)
    SiI9022_write(0x3d, 0x3e, 0x03);        //Enable AVI infoFrame transmission, Enable(send in every VBLANK period)
    SiI9022_write(0x3d, 0x40, 0x82);
    SiI9022_write(0x3d, 0x41, 0x02);
    SiI9022_write(0x3d, 0x42, 0x0d);
    SiI9022_write(0x3d, 0x43, 0xf7);
    SiI9022_write(0x3d, 0x44, 0x10);
    SiI9022_write(0x3d, 0x45, 0x68);
    SiI9022_write(0x3d, 0x46, 0x00);
    SiI9022_write(0x3d, 0x47, 0x00);
    SiI9022_write(0x3d, 0x3d, 0x07);
      
    
    
	//display Device ID information
    Status = SiI9022_read(0x39, 0x02, ReadBuffer);

	if (Status != XST_SUCCESS)	
    {
		return XST_FAILURE;
	}
	else	
    {
	    printf("Read Device ID of (%d)\n\r", ReadBuffer[0]);
	}

    Status=SiI9022_read(0x39, 0x03, ReadBuffer);
	if (Status != XST_SUCCESS)	
    {
		return XST_FAILURE;
	}
	else	
    {
	    printf("Read Device ID of (%d)\n\r", ReadBuffer[0]);
	}
    printf("done.\n\r");

    return 0;
}



static void sii902x_poweron(void)
{
	printf("%s\n", __func__);    
	/* Turn on DVI or HDMI */    
    SiI9022_write(SII9022_I2C_ADDR, 0x1A, 0x00);      
	return;
}


static void sii902x_poweroff(void)
{
	printf("%s\n", __func__);       
	/* disable tmds before changing resolution */
    SiI9022_write(SII9022_I2C_ADDR, 0x1A, 0x10);      
	return;
}

static void sii902x_setup(void)
{
    u8 data[8] = {0,1,2,3,4,5,6,7};
    int i = 0;
	printf("%s\n", __func__);       
	/* Power up */    
    SiI9022_write(SII9022_I2C_ADDR, 0x1E, 0x10);      
    
	for (i = 0; i < 8; i++)        
        SiI9022_write(SII9022_I2C_ADDR, i, data[i]);      
    
    SiI9022_write(SII9022_I2C_ADDR, 0x08, 0x70);      
    SiI9022_write(SII9022_I2C_ADDR, 0x09, 0x00);      
    SiI9022_write(SII9022_I2C_ADDR, 0x0A, 0x00);      
    
    SiI9022_write(SII9022_I2C_ADDR, 0x25, 0x00);      
    SiI9022_write(SII9022_I2C_ADDR, 0x26, 0x40);      
    SiI9022_write(SII9022_I2C_ADDR, 0x27, 0x00);      
    return;
}

int SiI9022_i2c_config(void)
{
    int Status;
    u8 dat;    
    int i  = 0;
    
    printf("######SiI9134_i2c_config()\r\n"); 
    
    sii9022_i2c_init(IIC_DEVICE_ID);
    printf("######SiI9134_i2c_config() over\r\n"); 
	int i2c_addr = SII9022_I2C_ADDR;
    printf("######begin config sii9022, i2caddr:0x%x\r\n", i2c_addr); 
    /* Set 902x in hardware TPI mode on and jump out of D3 state */    
    SiI9022_write(SII9022_I2C_ADDR, 0xc7, 0x00);        
   	
	/* read device ID */
	for (i = 10; i > 0; i--) 
    {
        Status = SiI9022_read(SII9022_I2C_ADDR, 0x1B, &dat);
        
		printf("Sii902x: read id = 0x%02X", dat);
        
		if (dat == 0xb0) 
        {
            Status = SiI9022_read(SII9022_I2C_ADDR, 0x1C, &dat);
			printf("-0x%02X", dat);
            
            Status = SiI9022_read(SII9022_I2C_ADDR, 0x1D, &dat);
			printf("-0x%02X", dat);
            
            Status = SiI9022_read(SII9022_I2C_ADDR, 0x30, &dat);
			printf("-0x%02X", dat);
			break;
		}
	}
    printf("\r\n");
    
    /*enable cable hot plug irq*/
    SiI9022_write(SII9022_I2C_ADDR, 0x3C, 0x01);      

	Status = SiI9022_read(SII9022_I2C_ADDR, 0x3D, &dat);
    printf("-0x%02X\r\n", dat);
    if (dat > 0)
    {
		if (dat & 0x4) 
        {
            /* Power on sii902x */
            sii902x_poweron();
        }
        else
        {
			/* Power off sii902x */
			sii902x_poweroff();
        }
    }
    // TBD:
    sii902x_setup();
    return 0;
}


int HDMI_init( void )
{
    u32 data;
    
    //SiI9022_i2c_config();
	// return 0;
	
    /*for logicvc,*/
    ddr_video_wr();
        
    data = Xil_In32(CF_CLKGEN_BASEADDR + (0x1f*4));

    if ((data & 0x1) == 0x0) 
    {
        printf("CLKGEN (148.5MHz) out of lock (0x%04x)\n\r", data);
        return(0);
    }
    
    //**********configure VDMA**************//
    Xil_Out32((VDMA_BASEADDR + 0x000), 0x00000003); // enable circular mode
    Xil_Out32((VDMA_BASEADDR + 0x05c), VIDEO_BASEADDR); // start address
    Xil_Out32((VDMA_BASEADDR + 0x060), VIDEO_BASEADDR); // start address
    Xil_Out32((VDMA_BASEADDR + 0x064), VIDEO_BASEADDR); // start address
    Xil_Out32((VDMA_BASEADDR + 0x058), (H_STRIDE*4)); // h offset (2048 * 4) bytes
    Xil_Out32((VDMA_BASEADDR + 0x054), (H_ACTIVE*4)); // h size (1920 * 4) bytes
    Xil_Out32((VDMA_BASEADDR + 0x050), V_ACTIVE); // v size (1080)
	
    //**********configure axi_hdmi_tx_24b**************//
    Xil_Out32((CFV_BASEADDR + 0x08), ((H_WIDTH << 16) | H_COUNT));      //HSYNC Width
    Xil_Out32((CFV_BASEADDR + 0x0c), ((H_DE_MIN << 16) | H_DE_MAX));    //HSYNC DE MAX
    Xil_Out32((CFV_BASEADDR + 0x10), ((V_WIDTH << 16) | V_COUNT));      //VSYNC width
    Xil_Out32((CFV_BASEADDR + 0x14), ((V_DE_MIN << 16) | V_DE_MAX));    //VSYC DE MAX
    Xil_Out32((CFV_BASEADDR + 0x04), 0x00000002);                       //bit2=0: disable test pattern,Bypass the CSC, Video output disable
    Xil_Out32((CFV_BASEADDR + 0x04), 0x00000003);                       //Video output enable

    // SiI9134_i2c_config();
    
    SiI9022_i2c_config();
	
    return 0;
}
