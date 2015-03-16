/*
 * (C) Copyright 2012 Xilinx
 * Copyright (c) 2012 Digilent. All right reserved.
 * 	Author: Tinghui WANG <steven.wang@digilentinc.com>
 *
 * Configuration for Zynq Evaluation and Development Board - ZedBoard
 * See zynq_common.h for Zynq common configs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_ZYNQ_ZED_H
#define __CONFIG_ZYNQ_ZED_H

#include <configs/zynq_common.h>

/*
 * High Level Configuration Options
 */
 
/* Community Board */
/* #define CONFIG_ZED  */

/* Default environment */
/* #define CONFIG_IPADDR   192.168.1.253
#define CONFIG_SERVERIP 192.168.1.6
*/

/* add by starsoc */

#define CONFIG_SYS_LONGHELP


#define CONFIG_IPADDR   192.168.1.253
#define CONFIG_SERVERIP 192.168.1.8

#define CONFIG_SYS_DCACHE_OFF   /*verify hdmi function*/
#define CONFIG_SYS_ICACHE_OFF




/* #define CONFIG_BOOTARGS "console=ttyPS0,115200 root=/dev/mmcblk0p2 rw earlyprintk rootfstype=ext4 rootwait devtmpfs.mount=0" */
#define DEVICE_TREE_ADDR 0x1000000
#undef CONFIG_ZYNQ_XIL_LQSPI

/* No NOR Flash available on ZedBoard */
#define CONFIG_SYS_NO_FLASH
#define CONFIG_ENV_IS_NOWHERE


/* #define CONFIG_BOOTARGS "console=ttyPS0,115200 root=/dev/ram rw initrd=0x800000,8M init=/init earlyprintk rootwait devtmpfs.mount=1" */
/* "sdboot=setenv bootargs " CONFIG_BOOTARGS ";" */

/*    
"star_test=go 0x05000000\0;"   \  
"star_test=mmcinfo\0"    \

*/

#undef CONFIG_EXTRA_ENV_SETTINGS
#define CONFIG_EXTRA_ENV_SETTINGS 	\
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_size=0x140000\0" 	\
	"ramdisk_size=0x200000\0" 	\
	"star_test=go 0x05000000\0" \
	"burn-qspi=sf probe 0 0 0\0" \
	"qspiboot=sf probe 0 0 0;" \
		"sf read 0x8000 0x00500000 0x300000;" \
		"sf read 0x10000000 0x00900000 0x00400000;" \
		"sf read 0x01000000 0x00e00000 0x00010000;" \
		"go 0x8000\0" \
	"sdboot=echo Copying Linux from SD to RAM...; " \
		"mmcinfo;" \
		"fatload mmc 0 0x8000 zImage;" \
		"fatload mmc 0 0x1000000 devicetree.dtb;" \
		"fatload mmc 0 0x10000000 ramdisk8M.image.gz;" \
		"go 0x8000\0" \
	"jtagboot=echo TFTPing Linux to RAM...;" \
		"tftp 0x8000 zImage;" \
		"tftp 0x1000000 devicetree.dtb;" \
		"tftp 0x800000 ramdisk8M.image.gz;" \
		"go 0x8000\0"           \
		"fpga=0\0"  \
		"fpgadata=0x00100000"

#include <config_cmd_default.h>
#define CONFIG_CMD_DATE		/* RTC? */
#define CONFIG_CMD_PING		/* Might be useful for debugging */
#define CONFIG_CMD_SAVEENV	/* Command to save ENV to Flash */
#define CONFIG_REGINFO		/* Again, debugging */
#undef CONFIG_CMD_SETGETDCR	/* README says 4xx only */

#define CONFIG_TIMESTAMP	/* print image timestamp on bootm, etc */

#define CONFIG_PANIC_HANG /* For development/debugging */

#define CONFIG_AUTO_COMPLETE
#define CONFIG_CMDLINE_EDITING

#undef CONFIG_SYS_PROMPT
#define CONFIG_SYS_PROMPT	"star-uboot> "


#define CONFIG_BOOTDELAY		3

/* this is to initialize GEM at uboot start */
/* #define CONFIG_ZYNQ_INIT_GEM	*/
/* this is to set ipaddr, ethaddr and serverip env variables. */
#define CONFIG_ZYNQ_IP_ENV

/* HW to use */
/* modify by starsoc */
#define CONFIG_UART1
/* #define CONFIG_UART0 */
#define CONFIG_CMD_I2C
#define CONFIG_HARD_I2C
#define CONFIG_SYS_I2C_SPEED 100000  //didn't use speed and slave_addr MACRO
#define CONFIG_SYS_I2C_SLAVE 0x50

#define CONFIG_ZYNQ_I2C
#define CONFIG_ZYNQ_I2C_CTLR_1


#define CONFIG_TTC0
#define CONFIG_GEM0
#define CONFIG_ZYNQ_GEM
#define CONFIG_XGMAC_PHY_ADDR 0

/*
 * Physical Memory map
 */
#define PHYS_SDRAM_1_SIZE (512 * 1024 * 1024)

/*
 * SPI Settings
 */
#define CONFIG_ZYNQ_SPI
#define CONFIG_CMD_SPI
#define CONFIG_SF_DEFAULT_SPEED 30000000
#define CONFIG_SPI_FLASH
#define CONFIG_CMD_SF

/* modify by starsoc, using winbond QSPI instead of spansion */
/* #define CONFIG_SPI_FLASH_SPANSION */
#define CONFIG_SPI_FLASH_WINBOND

/* Place a Xilinx Boot ROM header in u-boot image? */
#undef CONFIG_ZYNQ_XILINX_FLASH_HEADER

#ifdef CONFIG_ZYNQ_XILINX_FLASH_HEADER
/* Address Xilinx boot rom should use to launch u-boot */
#ifdef CONFIG_ZYNQ_XIL_LQSPI
#define CONFIG_ZYNQ_XIP_START XPSS_QSPI_LIN_BASEADDR
#endif
#endif

/* Secure Digital */
#define CONFIG_MMC

#ifdef CONFIG_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_ZYNQ_MMC
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_EXT2
#define CONFIG_DOS_PARTITION
#endif

#endif /* __CONFIG_ZYNQ_ZED_H */
