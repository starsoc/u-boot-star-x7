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


#include <malloc.h>


#define ULPI_VIEW_WAKEUP	(1 << 31)
#define ULPI_VIEW_RUN		(1 << 30)
#define ULPI_VIEW_WRITE		(1 << 29)
#define ULPI_VIEW_READ		(0 << 29)
#define ULPI_VIEW_ADDR(x)	(((x) & 0xff) << 16)
#define ULPI_VIEW_DATA_READ(x)	(((x) >> 8) & 0xff)
#define ULPI_VIEW_DATA_WRITE(x)	((x) & 0xff)

#define ULPI_VIEWPORT		0x170

#define	ETIMEDOUT	110

#define ULPI_PRODUCT_ID_HIGH			0x03

#define ULPI_USB3320_VENDOR_ID          0x424
#define ULPI_USB3320_PRODUCT_ID         0x7


#define writel_ulpi(b,addr) (void)((*(volatile unsigned int *)(addr)) = (b))

static XUsbPs UsbInstance;	/* The instance of the USB Controller */
u8 Buffer[65535];


void usb_phy_init()
{
    
	int	Status;
	XUsbPs_Config		*UsbConfigPtr;
    /* printf("usb_phy_init begin***\n"); */
	u8	*MemPtr = NULL;
    
	u32	MemSize;
    XUsbPs_DeviceConfig  DeviceConfig;
    u16 UsbDeviceId = XPAR_XUSBPS_0_DEVICE_ID;
    XUsbPs *UsbInstancePtr = &UsbInstance;
    
	const u8 NumEndpoints = 2;
    
	UsbConfigPtr = XUsbPs_LookupConfig(UsbDeviceId);
	if (NULL == UsbConfigPtr) {  
        printf("usb_phy_init step1***\n");
		return;
	}
    
	Status = XUsbPs_CfgInitialize(UsbInstancePtr,
				       UsbConfigPtr,
				       UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
        printf("usb_phy_init step2***\n");
		return;
	}

    
	DeviceConfig.EpCfg[0].Out.Type		= XUSBPS_EP_TYPE_CONTROL;
	DeviceConfig.EpCfg[0].Out.NumBufs	= 2;
	DeviceConfig.EpCfg[0].Out.BufSize	= 64;
	DeviceConfig.EpCfg[0].Out.MaxPacketSize	= 64;
	DeviceConfig.EpCfg[0].In.Type		= XUSBPS_EP_TYPE_CONTROL;
	DeviceConfig.EpCfg[0].In.NumBufs	= 2;
	DeviceConfig.EpCfg[0].In.MaxPacketSize	= 64;

	DeviceConfig.EpCfg[1].Out.Type		= XUSBPS_EP_TYPE_BULK;
	DeviceConfig.EpCfg[1].Out.NumBufs	= 16;
	DeviceConfig.EpCfg[1].Out.BufSize	= 512;
	DeviceConfig.EpCfg[1].Out.MaxPacketSize	= 512;
	DeviceConfig.EpCfg[1].In.Type		= XUSBPS_EP_TYPE_BULK;
	DeviceConfig.EpCfg[1].In.NumBufs	= 16;
	DeviceConfig.EpCfg[1].In.MaxPacketSize	= 512;

	DeviceConfig.NumEndpoints = NumEndpoints;
	MemSize = XUsbPs_DeviceMemRequired(&DeviceConfig);

	MemPtr = &Buffer[0];
	memset(&Buffer[0],0,65535);
	Xil_DCacheFlushRange((unsigned int)&Buffer,65535);

    
	DeviceConfig.DMAMemVirt = (u32) MemPtr;
	DeviceConfig.DMAMemPhys = (u32) MemPtr;

	Status = XUsbPs_ConfigureDevice(UsbInstancePtr, &DeviceConfig);
	if (XST_SUCCESS != Status) {
        printf("usb_phy_init step4***\n");
		return;
	}
    
	/* Enable the interrupts. */
	XUsbPs_IntrEnable(UsbInstancePtr, XUSBPS_IXR_UR_MASK |
					   XUSBPS_IXR_UI_MASK);

	/* Start the USB engine */
	XUsbPs_Start(UsbInstancePtr);
    
    /* printf("usb_phy_init end***\n"); */
    
    return;
}



static int readl_ulpi(const volatile int addr)
{
	return *(const volatile int *) addr;
}


static int ulpi_viewport_wait(u32 view, u32 mask)
{
	unsigned long usec = 200;

	while (usec--) {
		if (!(readl_ulpi(view) & mask))
			return 0;

		udelay(10000);
	};
    
	return -ETIMEDOUT;
}


static int ulpi_viewport_read(u32 reg)
{
	int ret;
	u32 view_port = 0xe0002000 + ULPI_VIEWPORT;
	printf("view_port:0x%x\n\r", view_port);
	writel_ulpi(ULPI_VIEW_WAKEUP | ULPI_VIEW_WRITE, view_port);
    
	printf("view_port=0x%x\n",readl_ulpi(view_port));
	ret = ulpi_viewport_wait(view_port, ULPI_VIEW_WAKEUP);
	if (ret)
		return ret;

	writel_ulpi(ULPI_VIEW_RUN | ULPI_VIEW_READ | ULPI_VIEW_ADDR(reg), view_port);
	printf("view_port=0x%x\n",readl_ulpi(view_port));
	
	ret = ulpi_viewport_wait(view_port, ULPI_VIEW_RUN);
	if (ret)
		return ret;
    
	return ULPI_VIEW_DATA_READ(readl_ulpi(view_port));

}


int ulpi_phy_init()
{
	u32 i = 0;
	u32 ulpi_id = 0;
	int vid, pid, ret;
    
    printf("ulpi_phy_init...\n\r");
    
	for (i = 0; i < 4; i++)
	{
		ret = ulpi_viewport_read(ULPI_PRODUCT_ID_HIGH - i);
        printf("ret:0x%x\n", ret);
		if (ret < 0)
			return ret;
		ulpi_id = (ulpi_id << 8) | ret;
	}
	vid = ulpi_id & 0xffff;
	pid = ulpi_id >> 16;
    
	printf("ULPI transceiver vendor/product ID 0x%04x/0x%04x\n", vid, pid);
    
    if ((vid == ULPI_USB3320_VENDOR_ID) && (pid == ULPI_USB3320_PRODUCT_ID))
    {
        printf("ulpi phy read successfully\r\n");
    }
    
    return 0;
}

/*****************************/
