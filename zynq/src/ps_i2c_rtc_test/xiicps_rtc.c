#define TESTAPP_GEN

/* $Id: xiicps_intr_master_example.c,v 1.1.2.1 2011/01/20 03:45:04 sadanan Exp $ */
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
 * @file xiicps_selftest_example.c
 *
 * This file contains a example for using the IIC hardware device and
 * XIicPs driver.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	 Who Date    	Changes
 * ----- --- -------- 	-----------------------------------------------
 * 1.00a sdm 05/30/11 	First release
 *
 * </pre>
 *
 ****************************************************************************/
 
#define CTRL_STATUS_1   0
#define CTRL_STATUS_2   1
#define VL_SECONDS      2
#define MINUTES         3
#define HOURS           4
#define DAYS            5
#define WEEKDAYS        6
#define CENTURY_MONTHS  7
#define YEARS           8

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
#define IIC_SLAVE_ADDR		0x51        /* PCF8563 address */

/*****************************************************************************/
/**
*
* This function does a minimal test on the Iic device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XIicPs component.
*
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*
*******************************************************************************/

int zynq_ps_i2c_rtc_test(void)
{
	int Status;
    u8 ctrl_stat_1, ctrl_stat_2;
    u8 data;
    Status = IicPsMasterPolled_Init(XPAR_XIICPS_0_DEVICE_ID);
    if (Status != XST_SUCCESS)
        printf("I2c init rts error\r\n");
    
    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, CTRL_STATUS_1, &ctrl_stat_1);
    printf("ctrl status 1:0x%x\r\n", ctrl_stat_1);
    
    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, CTRL_STATUS_2, &ctrl_stat_2);
    printf("ctrl status 2:0x%x\r\n", ctrl_stat_2);
    
#if 0
    data = 0x00;
    Status = IicPsRtcPolled_Write(IIC_SLAVE_ADDR, VL_SECONDS, data);
    data = 0x14;
    Status = IicPsRtcPolled_Write(IIC_SLAVE_ADDR, MINUTES, data);
    data = 0x08;
    Status = IicPsRtcPolled_Write(IIC_SLAVE_ADDR, HOURS, data);
#endif
    
#if 0
    data = 0x02;
    Status = IicPsRtcPolled_Write(IIC_SLAVE_ADDR, DAYS, data);
    data = 0x02;
    Status = IicPsRtcPolled_Write(IIC_SLAVE_ADDR, WEEKDAYS, data);
    data = 0x09;
    Status = IicPsRtcPolled_Write(IIC_SLAVE_ADDR, CENTURY_MONTHS, data);
    data = 0x14;
    Status = IicPsRtcPolled_Write(IIC_SLAVE_ADDR, YEARS, data);
#endif
    
    
    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, VL_SECONDS, &data);
    printf("vl seconds:0x%x\r\n", data);

    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, MINUTES, &data);
    printf("minutes:0x%x\r\n", data);

    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, HOURS, &data);
    printf("hours:0x%x\r\n", data);

    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, DAYS, &data);
    printf("days:0x%x\r\n", data);

    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, WEEKDAYS, &data);
    printf("weekdays:0x%x\r\n", data);

    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, CENTURY_MONTHS, &data);
    printf("months:0x%x\r\n", data);
    
    Status = IicPsRtcPolled_Read(IIC_SLAVE_ADDR, YEARS, &data);
    printf("years:0x%x\r\n", data);

    return 0;
}
