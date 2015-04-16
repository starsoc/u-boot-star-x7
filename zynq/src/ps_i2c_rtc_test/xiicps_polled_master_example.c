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
* @file xiicps_polled_master_example.c
*
* A design example of using the device in polled mode as master.
*
* The example uses buffer size 132. Please set the send buffer of the
* Aardvark device to be continuous 64 bytes from 0x00 to 0x3F.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00a jz  01/30/10 First release
 *
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID

/*
 * The slave address to send to and receive from.
 */

/*
 * The following constant controls the length of the buffers to be sent
 * and received with the IIC.
 */
#define TEST_BUFFER_SIZE	132

/**************************** Type Definitions ********************************/


/************************** Function Prototypes *******************************/

int IicPsMasterPolledInit(u16 DeviceId);

/************************** Variable Definitions ******************************/

XIicPs IICInstance;		/**< Instance of the IIC Device */

/*
 * The following buffers are used in this example to send and receive data
 * with the IIC.
 */


/******************************************************************************/
/**
*
* Main function to call the polled master example.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*******************************************************************************/


int IicPsMasterPolled_Init(u16 DeviceId)
{
	int Status;
	XIicPs_Config *Config;
	int Index;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}
    
	Status = XIicPs_CfgInitialize(&IICInstance, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    
	/*
	 * Perform a self-test to ensure that the hardware was built correctly.
	 */
	Status = XIicPs_SelfTest(&IICInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    
	/*
	 * Set the IIC serial clock rate.
	 */
	XIicPs_SetSClk(&IICInstance, IIC_SCLK_RATE);
    
	return XST_SUCCESS;
}


int IicPsRtcPolled_Write(u8 dev_id, u8 waddr, u8 wdata)
{

	int Status;
    u8 WriteBuffer[2];              /* Write buffer for writing register. */
    
	WriteBuffer[0]=waddr;
	WriteBuffer[1]=wdata;
    
	/*
	 * Send the buffer using the IIC and ignore the number of bytes sent
	 * as the return value since we are using it in interrupt mode.
	 */
	Status = XIicPs_MasterSendPolled(&IICInstance, WriteBuffer,
			 2, dev_id);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IICInstance)) {
		/* NOP */
	}
    
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function sends data and expects to receive data from slave as modular
* of 64.
*
* This function uses interrupt-driven mode of the device.
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*******************************************************************************/

int IicPsRtcPolled_Read(u8 dev_id, u8 raddr, u8 ReadBuffer[])
{
	int Status;
    u8 WriteBuffer[2];              /* Write buffer for writing register. */
    WriteBuffer[0] = raddr;
    /** Send the Data. */
    
	Status = XIicPs_MasterSendPolled(&IICInstance, WriteBuffer,
			 1, dev_id);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
    
    /*
     * Wait until bus is idle to start another transfer.
     */
    while (XIicPs_BusIsBusy(&IICInstance)) {
        /* NOP */
    }

    /*
     * Wait for a bit of time to allow the programming to complete
     */
    udelay(250000);
    
    
	Status = XIicPs_MasterRecvPolled(&IICInstance, ReadBuffer,
			  1, dev_id);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IICInstance));

return XST_SUCCESS;
}
