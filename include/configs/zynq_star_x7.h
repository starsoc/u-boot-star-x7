/*
 * (C) Copyright 2013 Xilinx, Inc.
 *
 * Configuration for Zynq Evaluation and Development Board - ZedBoard
 * See zynq-common.h for Zynq common configs
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_ZYNQ_ZTURN_H
#define __CONFIG_ZYNQ_ZTURN_H

//#define CONFIG_SYS_SDRAM_SIZE		(1024 * 1024 * 1024)
#define	PHYS_SDRAM_1_SIZE	(1024 * 1024 * 1024)

// UART
#define CONFIG_ZYNQ_SERIAL_UART1
#define CONFIG_ZYNQ_GEM0
#define CONFIG_ZYNQ_GEM_PHY_ADDR0	0

#define CONFIG_SYS_NO_FLASH

#define CONFIG_ZYNQ_USB
#define CONFIG_ZYNQ_SDHCI0
#define CONFIG_ZYNQ_QSPI

#define CONFIG_ZYNQ_BOOT_FREEBSD
#define CONFIG_DEFAULT_DEVICE_TREE	zynq-zed

#define CONFIG_IPADDR	192.168.1.55
#define CONFIG_SERVERIP	192.168.1.13

// add by starsoc
// #define CONFIG_ZYNQ_I2C
#define CONFIG_ZYNQ_I2C_CTLR_1


#define CONFIG_EXTRA_ENV_SETTINGS	\
	"ethaddr=00:0a:35:00:01:22\0"	\
	"kernel_image=uImage\0"	\
	"kernel_load_address=0x2080000\0" \
	"ramdisk_image=uramdisk.image.gz\0"	\
	"ramdisk_load_address=0x4000000\0"	\
	"devicetree_image=devicetree.dtb\0"	\
	"devicetree_load_address=0x2000000\0"	\
	"bitstream_image=system.bit.bin\0"	\
	"boot_image=BOOT.bin\0"	\
	"loadbit_addr=0x100000\0"	\
	"loadbootenv_addr=0x2000000\0" \
	"kernel_size=0x500000\0"	\
	"devicetree_size=0x20000\0"	\
	"ramdisk_size=0x5E0000\0"	\
	"boot_size=0xF00000\0"	\
	"fdt_high=0x20000000\0"	\
	"initrd_high=0x20000000\0"	\
	"bootenv=uEnv.txt\0" \
	"loadbootenv=fatload mmc 0 ${loadbootenv_addr} ${bootenv}\0" \
	"importbootenv=echo Importing environment from SD ...; " \
		"env import -t ${loadbootenv_addr} $filesize\0" \
	"mmc_loadbit_fat=echo Loading bitstream from SD/MMC/eMMC to RAM.. && " \
		"mmcinfo && " \
		"fatload mmc 0 ${loadbit_addr} ${bitstream_image} && " \
		"fpga load 0 ${loadbit_addr} ${filesize}\0" \
	"norboot=echo Copying Linux from NOR flash to RAM... && " \
		"cp.b 0xE2100000 ${kernel_load_address} ${kernel_size} && " \
		"cp.b 0xE2600000 ${devicetree_load_address} ${devicetree_size} && " \
		"echo Copying ramdisk... && " \
		"cp.b 0xE2620000 ${ramdisk_load_address} ${ramdisk_size} && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"uenvboot=" \
		"if run loadbootenv; then " \
			"echo Loaded environment from ${bootenv}; " \
			"run importbootenv; " \
		"fi; " \
		"if test -n $uenvcmd; then " \
			"echo Running uenvcmd ...; " \
			"run uenvcmd; " \
		"fi\0" \
	"sdboot=if mmcinfo; then " \
			"run uenvboot; " \
			"echo Copying Linux from SD to RAM... && " \
			"fatload mmc 0 ${kernel_load_address} ${kernel_image} && " \
			"fatload mmc 0 ${devicetree_load_address} ${devicetree_image} && " \
			"fatload mmc 0 ${ramdisk_load_address} ${ramdisk_image} && " \
			"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}; " \
		"fi\0" \
	"usbboot=if usb start; then " \
			"run uenvboot; " \
			"echo Copying Linux from USB to RAM... && " \
			"fatload usb 0 ${kernel_load_address} ${kernel_image} && " \
			"fatload usb 0 ${devicetree_load_address} ${devicetree_image} && " \
			"fatload usb 0 ${ramdisk_load_address} ${ramdisk_image} && " \
			"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}; " \
		"fi\0" \
	"nandboot=echo Copying Linux from NAND flash to RAM... && " \
		"nand read ${kernel_load_address} 0x100000 ${kernel_size} && " \
		"nand read ${devicetree_load_address} 0x600000 ${devicetree_size} && " \
		"echo Copying ramdisk... && " \
		"nand read ${ramdisk_load_address} 0x620000 ${ramdisk_size} && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"jtagboot=echo TFTPing Linux to RAM... && " \
		"tftpboot ${kernel_load_address} ${kernel_image} && " \
		"tftpboot ${devicetree_load_address} ${devicetree_image} && " \
		"tftpboot ${ramdisk_load_address} ${ramdisk_image} && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_norboot=echo Copying Image from NOR flash to RAM... && " \
		"cp.b 0xE2100000 0x100000 ${boot_size} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_nandboot=echo Copying Image from NAND flash to RAM... && " \
		"nand read 0x100000 0x0 ${boot_size} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_qspiboot=echo Copying Image from QSPI flash to RAM... && " \
		"sf probe 0 0 0 && " \
		"sf read 0x100000 0x0 ${boot_size} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_sdboot=echo Copying Image from SD to RAM... && " \
		"fatload mmc 0 0x100000 ${boot_image} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"rsa_jtagboot=echo TFTPing Image to RAM... && " \
		"tftpboot 0x100000 ${boot_image} && " \
		"zynqrsa 0x100000 && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"qspiboot=echo Copying Linux from QSPI flash to RAM... && " \
		"sf probe 0 0 0 && " \
		"sf read ${kernel_load_address} 0x300000 ${kernel_size} && " \
		"sf read ${devicetree_load_address} 0x800000 ${devicetree_size} && " \
		"echo Copying ramdisk... && " \
		"sf read ${ramdisk_load_address} 0x820000 ${ramdisk_size} && " \
		"bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0" \
	"qspiupdate=echo Update qspi images from sd card... && " \
		"echo - Init mmc... && mmc rescan && " \
		"echo - Init qspi flash... && sf probe 0 0 0 && " \
		"echo - Write boot.bin... && " \
		"mw.b 0x200000 0x600000 && " \
		"fatload mmc 0 0x200000 boot.bin && " \
		"sf erase 0 0x300000 && " \
		"sf write 0x200000 0 0x300000 && " \
		"echo - Write uImage... && " \
		"mw.b 0x200000 0x600000 && " \
		"fatload mmc 0 0x200000 uImage && " \
		"sf erase 0x300000 ${kernel_size} && " \
		"sf write 0x200000 0x300000 ${kernel_size} && " \
		"echo - Write device tree... && " \
		"mw.b 0x200000 0x600000 && " \
		"fatload mmc 0 0x200000 devicetree.dtb && " \
		"sf erase 0x800000 ${devicetree_size} && " \
		"sf write 0x200000 0x800000 ${devicetree_size} && " \
		"echo - Write Ramdisk... && " \
		"mw.b 0x200000 0x600000 && " \
		"fatload mmc 0 0x200000 uramdisk.image.gz && " \
		"sf erase 0x820000 ${ramdisk_size} && " \
		"sf write 0x200000 0x820000 ${ramdisk_size} && " \
		"echo - Done.\0"

#include <configs/zynq_common.h>

#endif /* __CONFIG_ZYNQ_ZED_H */
