/******************************************************************************
*
* (c) Copyright 2010-13 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xiicps_eeprom_polled_example.c
*
* This file consists of a polled mode design example which uses the Xilinx PS
* IIC device and XIicPs driver to exercise the EEPROM.
*
* The XIicPs_MasterSendPolled() API is used to transmit the data and
* XIicPs_MasterRecvPolled() API is used to receive the data.
*
* The example is tested with a 2Kb/8Kb serial IIC EEPROM (ST M24C02/M24C08).
* The WP pin of this EEPROM is hardwired to ground in the HW in which this
* was tested.
*
* The AddressType should be u8 as the address pointer in the on-board
* EEPROM is 1 bytes.
*
* This code assumes that no Operating System is being used.
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a sdm  03/15/10 First release
* 1.01a sg   04/13/12 Added MuxInit function for initializing the IIC Mux
*		      on the ZC702 board and to configure it for accessing
*		      the IIC EEPROM.
*                     Updated to use usleep instead of delay loop
* 1.04a hk   09/03/13 Removed GPIO code to pull MUX out of reset - CR#722425.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "sleep.h"
#include "xiicps.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID	XPAR_XIICPS_0_DEVICE_ID

/*
 * The following constant defines the address of the IIC Slave device on the
 * IIC bus. Note that since the address is only 7 bits, this constant is the
 * address divided by 2.
 */
 
/* since AT24C02A is used, A2 A1 A0: 0 0 0 */
#define IIC_SLAVE_ADDR		0x50   


#define IIC_MUX_ADDRESS 	0x74
/* 2K size */
#define PAGE_SIZE			(0x800/0x8) 
/*
 * The Starting address in the IIC EEPROM on which this test is performed.
 */
#define EEPROM_START_ADDRESS	0

/*
 * The write data size determines how much data should be written at a time.
 * The write function should be called with this as a maximum byte count.
 */
#define WRITE_DATA_SIZE  8
#define READ_DATA_SIZE   8
#define ERASE_DATA_SIZE  32
#define WRITE_EEPROM_START_ADDRESS 128
/**************************** Type Definitions *******************************/

/*
 * The AddressType should be u8 as the address pointer in the on-board
 * EEPROM is 1 bytes.
 */
typedef u8 AddressType;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int IicPsEepromWrite(u8 *pwriteBuf, u32 size);
int IicPsEepromErase(void);
int IicPsEepromPolledExample(void);
int IicPsEepromPolledInit(void);
int EepromWriteData(u8* pWriteBuffer, u16 ByteCount);
int MuxInit(void);
int EepromReadData(u8 *pReadBufferPtr, u8*pWriteBufPtr, u16 ByteCount);


/************************** Variable Definitions *****************************/

XIicPs IicInstance;		/* The instance of the IIC device. */

/*
 * Write buffer for writing a page.
 */


/************************** Function Definitions *****************************/



int IicPsEepromPolledInit(void)
{
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */

	int Status;

	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIicPs_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr == NULL) 
    {
		return XST_FAILURE;
	}
    
	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
	
	/*
    	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&IicInstance, IIC_SCLK_RATE);

	return 0;
}


int IicPsEepromWrite(u8 *pwriteBuf, u32 size)
{
	u32 i,j;
    u8 ReadBuffer[WRITE_DATA_SIZE]; /* Read buffer for reading a page. */
    int Status;
    int left_write_data = PAGE_SIZE;
	/*
	 * Write to the EEPROM.
	 */
	Status = EepromWriteData(pwriteBuf, size);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
	return Status;
}


int IicPsEepromRead(u8 *pReadBuf, u8 *pWriteBuf, u32 size)
{
    int Status;
    int i = 0;
	/*
	 * Write to the EEPROM.
	 */
	printf("EEPROM read data from Address:0x%x\r\n", *pWriteBuf);
	Status = EepromReadData(pReadBuf, pWriteBuf, size);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    for (i = 0; i < size; i++)
    {
        printf("0x%x, ", *(pReadBuf+i));
    } 
    printf("\r\n");
	return Status;
}



int IicPsCheckEepromData(void)
{
	AddressType Address = EEPROM_START_ADDRESS;    
    u8 ReadBuffer[READ_DATA_SIZE]; /* Read buffer for reading a page. */
    int Status;
    u8 WriteBuffer = Address;
    int i = 0, j;
	/*
	 * Read from the EEPROM.
	 */
	printf("check the first 16 bytes value, address from:0x%x\r\n", Address);
    for (i = 0; i < 2; i++)
	{
		Status = EepromReadData(ReadBuffer, &WriteBuffer, READ_DATA_SIZE);
		if (Status != XST_SUCCESS) 
		{
			return XST_FAILURE;
		}
        for (j = 0; j < READ_DATA_SIZE; j++)
        	printf("0x%x , ", ReadBuffer[j]);
        WriteBuffer += READ_DATA_SIZE;
	}
    printf("\r\n");
    
    WriteBuffer = PAGE_SIZE - 2 * READ_DATA_SIZE;
    printf("check the last 16 bytes value, address from:0x%x\r\n", WriteBuffer);
	for (i = 0; i < 2; i++)
	{
	    Status = EepromReadData(ReadBuffer, &WriteBuffer, READ_DATA_SIZE);
	    if (Status != XST_SUCCESS) 
	    {
	        return XST_FAILURE;
	    }
	    for (j = 0; j < READ_DATA_SIZE; j++)
	        printf("0x%x , ", ReadBuffer[j]);
	    WriteBuffer += READ_DATA_SIZE;
	}
	printf("\r\n");    
    return Status;
}

int IicPsEepromErase(void)
{
	
	u32 i,j;
	AddressType Address = EEPROM_START_ADDRESS;
    u8 WriteBuffer[sizeof(AddressType) + ERASE_DATA_SIZE];
    u8 ReadBuffer[ERASE_DATA_SIZE]; /* Read buffer for reading a page. */
    int Status;
    int left_write_data = PAGE_SIZE;
	/*
	 * Initialize the data to write and the read buffer.
	 */
	do
    {    
        printf("######IicPsEepromErase(), Address:0x%x, left_write_data:%d\r\n", 
                Address, left_write_data);
		WriteBuffer[0] = (u8) (Address);
        
		for (i = 0; i < ERASE_DATA_SIZE; i++) 
	    {
			WriteBuffer[sizeof(Address) + i] = 0xFF;
		}
		
		/*
		 * Write to the EEPROM.
		 */
		Status = EepromWriteData(WriteBuffer, sizeof(Address) + ERASE_DATA_SIZE);
		if (Status != XST_SUCCESS) 
	    {
			return XST_FAILURE;
		}
		Address += ERASE_DATA_SIZE;
        left_write_data -= ERASE_DATA_SIZE;
    }while(left_write_data > 0);
	return 0;
}


/*****************************************************************************/
/**
* This function writes, reads, and verifies the data to the IIC EEPROM. It
* does the write as a single page write, performs a buffered read.
*
* @param	None.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int IicPsEepromPolledExample(void)
{
	u32 Index;
	AddressType Address = WRITE_EEPROM_START_ADDRESS;
    int Status;
    
    u8 WriteBuffer[sizeof(AddressType) + WRITE_DATA_SIZE];
    u8 ReadBuffer[WRITE_DATA_SIZE]; /* Read buffer for reading a page. */
    
#if 0
	XIicPs_Config *ConfigPtr;	/* Pointer to configuration data */
	int Status;
	/*
	 * Initialize the IIC driver so that it is ready to use.
	 */
	ConfigPtr = XIicPs_LookupConfig(IIC_DEVICE_ID);
	if (ConfigPtr == NULL) 
    {
		return XST_FAILURE;
	}
    
	Status = XIicPs_CfgInitialize(&IicInstance, ConfigPtr,
					ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
	
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&IicInstance, IIC_SCLK_RATE);
#endif    
	/*
	 * Initialize the data to write and the read buffer.
	 */
	if (sizeof(Address) == 1) 
    {
		WriteBuffer[0] = (u8) (Address);
	} 
    else 
    {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
	}
    
	for (Index = 0; Index < WRITE_DATA_SIZE; Index++) 
    {
		WriteBuffer[sizeof(Address) + Index] = 0xFF;
		ReadBuffer[Index] = 0;
	}
    
	/*
	 * Write to the EEPROM.
	 */
	Status = EepromWriteData(WriteBuffer, sizeof(Address) + WRITE_DATA_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    
	/*
	 * Read from the EEPROM.
	 */
	WriteBuffer[0] = WRITE_EEPROM_START_ADDRESS;
	Status = EepromReadData(ReadBuffer, WriteBuffer, WRITE_DATA_SIZE);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    
	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < WRITE_DATA_SIZE; Index++) 
    {
		if (ReadBuffer[Index] != WriteBuffer[Index + sizeof(Address)]) 
        {
			return XST_FAILURE;
        }
        
		ReadBuffer[Index] = 0;
	}
		
	/*
	 * Initialize the data to write and the read buffer.
	 */
	if (sizeof(Address) == 1) 
    {
		WriteBuffer[0] = (u8) (Address);
	} 
    else 
    {
		WriteBuffer[0] = (u8) (Address >> 8);
		WriteBuffer[1] = (u8) (Address);
		ReadBuffer[Index] = 0;
	}
    
	for (Index = 0; Index < WRITE_DATA_SIZE; Index++) 
    {
		WriteBuffer[sizeof(Address) + Index] = Index + 10;
		ReadBuffer[Index] = 0;
	}
    
	/*
	 * Write to the EEPROM.
	 */
	Status = EepromWriteData(WriteBuffer, sizeof(Address) + WRITE_DATA_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    printf("******I2C EEPROM write data:\r\n");
	for (Index = 0; Index < WRITE_DATA_SIZE; Index++) 
    {
        printf("%d  ", WriteBuffer[Index + sizeof(Address)]);
    }   
    printf("\r\n");
	/*
	 * Read from the EEPROM.
	 */
	 
	WriteBuffer[0] = WRITE_EEPROM_START_ADDRESS;
	Status = EepromReadData(ReadBuffer, WriteBuffer, WRITE_DATA_SIZE);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    
    printf("******I2C EEPROM read data:\r\n");
	for (Index = 0; Index < WRITE_DATA_SIZE; Index++) 
    {
        printf("%d  ", ReadBuffer[Index]);
    }   
    printf("\r\n");
    
	/*
	 * Verify the data read against the data written.
	 */
	for (Index = 0; Index < WRITE_DATA_SIZE; Index++) 
    {
		if (ReadBuffer[Index] != WriteBuffer[Index + sizeof(Address)]) 
        {
			return XST_FAILURE;
		}
        
		ReadBuffer[Index] = 0;
	}
    
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function writes a buffer of data to the IIC serial EEPROM.
*
* @param	ByteCount contains the number of bytes in the buffer to be
*		written.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		The Byte count should not exceed the page size of the EEPROM as
*		noted by the constant WRITE_DATA_SIZE.
*
******************************************************************************/
int EepromWriteData(u8* pWriteBuffer, u16 ByteCount)
{
	int Status;

	/*
	 * Send the Data.
	 */
	Status = XIicPs_MasterSendPolled(&IicInstance, pWriteBuffer,
					  ByteCount, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance));
    
	/*
	 * Wait for a bit of time to allow the programming to complete
	 */
	udelay(250000);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function reads data from the IIC serial EEPROM into a specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int EepromReadData(u8 *pReadBufferPtr, u8*pWriteBufPtr, u16 ByteCount)
{
	int Status;
    
	Status = EepromWriteData(pWriteBufPtr, 1);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    
	/*
	 * Receive the Data.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, pReadBufferPtr,
					  ByteCount, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) 
    {
		return XST_FAILURE;
	}
    
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance));

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* This function initializes the IIC MUX to select EEPROM.
*
* @param	None.
*
* @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
int MuxInit(void)
{
	u8 WriteBuffer;
	u8 MuxIicAddr = IIC_MUX_ADDRESS;
	u8 Buffer = 0;
	int Status = 0;

	/*
	 * Channel select value for EEPROM.
	 */
	WriteBuffer = 0x04;

	/*
	 * Send the Data.
	 */
	Status = XIicPs_MasterSendPolled(&IicInstance, &WriteBuffer,1,
					MuxIicAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance));

	/*
	 * Receive the Data.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance, &Buffer,1, MuxIicAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance));
    
	return XST_SUCCESS;
}
