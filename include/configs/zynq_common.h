/*
 * (C) Copyright 2012 Xilinx
 *
 * Xilinx Zynq common configuration settings
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

#ifndef __CONFIG_ZYNQ_COMMON_H
#define __CONFIG_ZYNQ_COMMON_H

/*
 * High Level Configuration Options
 */
#define CONFIG_ARMV7 /* CPU */
#define CONFIG_ZYNQ /* SoC */

#include <asm/arch/xparameters.h>

#define CONFIG_SYS_TEXT_BASE 0x04000000

/*
 * Open Firmware flat tree
 */
#define CONFIG_OF_LIBFDT

/* Default environment */
#define CONFIG_EXTRA_ENV_SETTINGS	\
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_image=uImage\0"	\
	"ramdisk_image=uramdisk.image.gz\0"	\
	"devicetree_image=devicetree.dtb\0"	\
	"kernel_size=0x140000\0"	\
	"ramdisk_size=0x200000\0"	\
	"nand_kernel_size=0x400000\0"	\
	"nand_ramdisk_size=0x400000\0"	\
	"fdt_high=0x20000000\0"	\
	"initrd_high=0x20000000\0"	\
	"norboot=echo Copying Linux from NOR flash to RAM...;" \
		"cp 0xE2100000 0x3000000 ${kernel_size};" \
		"cp 0xE2600000 0x2A00000 0x20000;" \
		"echo Copying ramdisk...;" \
		"cp 0xE3000000 0x2000000 ${ramdisk_size};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"qspiboot=echo Copying Linux from QSPI flash to RAM...;" \
		"cp 0xFC100000 0x3000000 ${kernel_size};" \
		"cp 0xFC600000 0x2A00000 0x20000;" \
		"echo Copying ramdisk...;" \
		"cp 0xFC800000 0x2000000 ${ramdisk_size};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"sdboot=echo Copying Linux from SD to RAM...;" \
		"mmcinfo;" \
		"fatload mmc 0 0x3000000 ${kernel_image};" \
		"fatload mmc 0 0x2A00000 ${devicetree_image};" \
		"fatload mmc 0 0x2000000 ${ramdisk_image};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"nandboot=echo Copying Linux from NAND flash to RAM...;" \
		"nand read 0x3000000 0x200000 ${nand_kernel_size};" \
		"nand read 0x2A00000 0x700000 0x20000;" \
		"echo Copying ramdisk...;" \
		"nand read 0x2000000 0x900000 ${nand_ramdisk_size};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0" \
	"jtagboot=echo TFTPing Linux to RAM...;" \
		"tftp 0x3000000 ${kernel_image};" \
		"tftp 0x2A00000 ${devicetree_image};" \
		"tftp 0x2000000 ${ramdisk_image};" \
		"bootm 0x3000000 0x2000000 0x2A00000\0"

/* default boot is according to the bootmode switch settings */
#define CONFIG_BOOTCOMMAND "run modeboot"
#define CONFIG_BAUDRATE		115200

#define CONFIG_SYS_BAUDRATE_TABLE { 9600, 38400, 115200 }
#define CONFIG_BOOTDELAY	3 /* -1 to Disable autoboot */

#define	CONFIG_PSS_SERIAL
#define	CONFIG_RTC_XPSSRTC

#define CONFIG_SYS_PROMPT	"zynq-uboot> "

/* CONFIG_SYS_MONITOR_BASE? */
/* CONFIG_SYS_MONITOR_LEN? */

/* Keep L2 Cache Disabled */
#define CONFIG_SYS_L2CACHE_OFF
#define CONFIG_SYS_CACHELINE_SIZE	32

/*
 * Physical Memory map
 */
#define CONFIG_NR_DRAM_BANKS    	1
#define PHYS_SDRAM_1            	0

#define CONFIG_SYS_MEMTEST_START PHYS_SDRAM_1
#define CONFIG_SYS_MEMTEST_END (CONFIG_SYS_MEMTEST_START + \
				    PHYS_SDRAM_1_SIZE - (16 * 1024 * 1024))

#define CONFIG_SYS_SDRAM_BASE		0
#define CONFIG_SYS_INIT_RAM_ADDR	0xFFFF0000
#define CONFIG_SYS_INIT_RAM_SIZE	0x1000
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_INIT_RAM_ADDR + \
					CONFIG_SYS_INIT_RAM_SIZE - \
					GENERATED_GBL_DATA_SIZE)

#define CONFIG_ENV_SIZE 0x10000
#define CONFIG_SYS_MALLOC_LEN 0x400000
#define CONFIG_SYS_MAXARGS 16
#define CONFIG_SYS_CBSIZE 2048
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)

/* HW to use */
#define TIMER_INPUT_CLOCK	(XPAR_CPU_CORTEXA9_CORE_CLOCK_FREQ_HZ / 2)
#define CONFIG_TIMER_PRESCALE	255
#define TIMER_TICK_HZ		(TIMER_INPUT_CLOCK / CONFIG_TIMER_PRESCALE)
#define CONFIG_SYS_HZ		1000

#define CONFIG_SYS_LOAD_ADDR	0 /* default? */

/* Enable the PL to be downloaded */

#define CONFIG_FPGA
#define CONFIG_FPGA_XILINX
#define CONFIG_FPGA_ZYNQPL
#define CONFIG_CMD_FPGA

/* FIT support */
#define CONFIG_FIT		1
#define CONFIG_FIT_VERBOSE	1 /* enable fit_format_{error,warning}() */

/* For now, use only single block reads for the MMC */

#define CONFIG_SYS_MMC_MAX_BLK_COUNT 1

#define CONFIG_BOARD_LATE_INIT

#endif /* __CONFIG_ZYNQ_COMMON_H */
