CONFIG_SPI_FLASH_WINBOND=y
CONFIG_CMD_FAT=y
CONFIG_ARMV7=y
CONFIG_CMD_ITEST=y
CONFIG_CMD_EDITENV=y
CONFIG_SYS_ENET=y
CONFIG_ZYNQ_BOOT_FREEBSD=y
CONFIG_ZYNQ=y
CONFIG_CMD_CRC32=y
CONFIG_SYS_LONGHELP=y
CONFIG_SYS_LOAD_ADDR=0
CONFIG_CMD_XIMG=y
CONFIG_CMD_CACHE=y
CONFIG_BOOTDELAY=3
CONFIG_SPI_FLASH=y
CONFIG_SYS_HELP_CMD_WIDTH=8
CONFIG_NR_DRAM_BANKS=y
CONFIG_FS_FAT=y
CONFIG_BOOTM_RTEMS=y
CONFIG_SYS_CBSIZE=2048
CONFIG_SDHCI=y
CONFIG_BOOTM_LINUX=y
CONFIG_BOARD_LATE_INIT=y
CONFIG_CMD_CONSOLE=y
CONFIG_SYS_CPU="armv7"
CONFIG_MII=y
CONFIG_SYS_CACHELINE_SIZE=32
CONFIG_MMC=y
CONFIG_CMD_MISC=y
CONFIG_FIT=y
CONFIG_ENV_OFFSET=0xE0000
CONFIG_ENV_OVERWRITE=y
CONFIG_CMD_NET=y
CONFIG_ZYNQ_GEM0=y
CONFIG_ZYNQ_SERIAL_UART1=y
CONFIG_ENV_SIZE="(128 << 10)"
CONFIG_CMD_CLK=y
CONFIG_SPL_BUILD=y
CONFIG_SYS_MALLOC_LEN=0x400000
CONFIG_SPI_FLASH_SPANSION=y
CONFIG_SYS_TEXT_BASE=0x04000000
CONFIG_SYS_DEF_EEPROM_ADDR=0
CONFIG_CMD_SAVEENV=y
CONFIG_ENV_SECT_SIZE=$(CONFIG_ENV_SIZE)
CONFIG_BOOTM_PLAN9=y
CONFIG_SPI_FLASH_BAR=y
CONFIG_FPGA_ZYNQPL=y
CONFIG_CMD_MEMORY=y
CONFIG_SYS_MAXARGS=32
CONFIG_CMD_RUN=y
CONFIG_IPADDR="192.168.1.253"
CONFIG_SYS_PBSIZE="(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)"
CONFIG_BOARDDIR="board/xilinx/zynq"
CONFIG_SPI_FLASH_STMICRO=y
CONFIG_OF_LIBFDT=y
CONFIG_SUPPORT_VFAT=y
CONFIG_PHY_MARVELL=y
CONFIG_PHYLIB=y
CONFIG_FPGA_XILINX=y
CONFIG_CMDLINE_EDITING=y
CONFIG_FPGA=y
CONFIG_CMD_EXT2=y
CONFIG_BOOTCOMMAND="run $modeboot"
CONFIG_CMD_SETGETDCR=y
CONFIG_SYS_L2CACHE_OFF=y
CONFIG_ZLIB=y
CONFIG_CMD_GO=y
CONFIG_CMD_BOOTD=y
CONFIG_CMD_BOOTM=y
CONFIG_CMD_BOOTZ=y
CONFIG_NET_MULTI=y
CONFIG_AUTO_COMPLETE=y
CONFIG_SYS_SOC="zynq"
CONFIG_ZYNQ_GEM_PHY_ADDR0=0
CONFIG_SYS_HZ=1000
CONFIG_DOS_PARTITION=y
CONFIG_GZIP=y
CONFIG_SYS_VENDOR="xilinx"
CONFIG_CMD_SF=y
CONFIG_CMD_FPGA=y
CONFIG_SYS_INIT_RAM_SIZE=0x1000
CONFIG_ZYNQ_USB=y
CONFIG_SYS_BAUDRATE_TABLE="{ 9600, 38400, 115200 }"
CONFIG_SYS_HUSH_PARSER=y
CONFIG_SYS_SDRAM_BASE=0
CONFIG_SYS_BOOT_RAMDISK_HIGH=y
CONFIG_CMD_SPI=y
CONFIG_ZYNQ_QSPI=y
CONFIG_SYS_PROMPT_HUSH_PS2="> "
CONFIG_CMD_ECHO=y
CONFIG_GENERIC_MMC=y
CONFIG_SYS_INIT_RAM_ADDR=0xFFFF0000
CONFIG_ZYNQ_GEM=y
CONFIG_ZYNQ_SDHCI0=y
CONFIG_EXTRA_ENV_SETTINGS="ethaddr=00:0a:35:00:01:22\0kernel_image=uImage\0kernel_load_address=0x2080000\0ramdisk_image=uramdisk.image.gz\0ramdisk_load_address=0x4000000\0devicetree_image=devicetree.dtb\0devicetree_load_address=0x2000000\0bitstream_image=system.bit.bin\0boot_image=BOOT.bin\0loadbit_addr=0x100000\0loadbootenv_addr=0x2000000\0kernel_size=0x500000\0devicetree_size=0x20000\0ramdisk_size=0x5E0000\0boot_size=0xF00000\0fdt_high=0x20000000\0initrd_high=0x20000000\0bootenv=uEnv.txt\0loadbootenv=fatload mmc 0 ${loadbootenv_addr} ${bootenv}\0importbootenv=echo Importing environment from SD ...; env import -t ${loadbootenv_addr} $filesize\0mmc_loadbit_fat=echo Loading bitstream from SD/MMC/eMMC to RAM.. && mmcinfo && fatload mmc 0 ${loadbit_addr} ${bitstream_image} && fpga load 0 ${loadbit_addr} ${filesize}\0norboot=echo Copying Linux from NOR flash to RAM... && cp.b 0xE2100000 ${kernel_load_address} ${kernel_size} && cp.b 0xE2600000 ${devicetree_load_address} ${devicetree_size} && echo Copying ramdisk... && cp.b 0xE2620000 ${ramdisk_load_address} ${ramdisk_size} && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0uenvboot=if run loadbootenv; then echo Loaded environment from ${bootenv}; run importbootenv; fi; if test -n $uenvcmd; then echo Running uenvcmd ...; run uenvcmd; fi\0sdboot=if mmcinfo; then run uenvboot; echo Copying Linux from SD to RAM... && fatload mmc 0 ${kernel_load_address} ${kernel_image} && fatload mmc 0 ${devicetree_load_address} ${devicetree_image} && fatload mmc 0 ${ramdisk_load_address} ${ramdisk_image} && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}; fi\0usbboot=if usb start; then run uenvboot; echo Copying Linux from USB to RAM... && fatload usb 0 ${kernel_load_address} ${kernel_image} && fatload usb 0 ${devicetree_load_address} ${devicetree_image} && fatload usb 0 ${ramdisk_load_address} ${ramdisk_image} && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}; fi\0nandboot=echo Copying Linux from NAND flash to RAM... && nand read ${kernel_load_address} 0x100000 ${kernel_size} && nand read ${devicetree_load_address} 0x600000 ${devicetree_size} && echo Copying ramdisk... && nand read ${ramdisk_load_address} 0x620000 ${ramdisk_size} && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0jtagboot=echo TFTPing Linux to RAM... && tftpboot ${kernel_load_address} ${kernel_image} && tftpboot ${devicetree_load_address} ${devicetree_image} && tftpboot ${ramdisk_load_address} ${ramdisk_image} && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0rsa_norboot=echo Copying Image from NOR flash to RAM... && cp.b 0xE2100000 0x100000 ${boot_size} && zynqrsa 0x100000 && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0rsa_nandboot=echo Copying Image from NAND flash to RAM... && nand read 0x100000 0x0 ${boot_size} && zynqrsa 0x100000 && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0rsa_qspiboot=echo Copying Image from QSPI flash to RAM... && sf probe 0 0 0 && sf read 0x100000 0x0 ${boot_size} && zynqrsa 0x100000 && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0rsa_sdboot=echo Copying Image from SD to RAM... && fatload mmc 0 0x100000 ${boot_image} && zynqrsa 0x100000 && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0rsa_jtagboot=echo TFTPing Image to RAM... && tftpboot 0x100000 ${boot_image} && zynqrsa 0x100000 && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0qspiboot=echo Copying Linux from QSPI flash to RAM... && sf probe 0 0 0 && sf read ${kernel_load_address} 0x300000 ${kernel_size} && sf read ${devicetree_load_address} 0x800000 ${devicetree_size} && echo Copying ramdisk... && sf read ${ramdisk_load_address} 0x820000 ${ramdisk_size} && bootm ${kernel_load_address} ${ramdisk_load_address} ${devicetree_load_address}\0qspiupdate=echo Update qspi images from sd card... && echo - Init mmc... && mmc rescan && echo - Init qspi flash... && sf probe 0 0 0 && echo - Write boot.bin... && mw.b 0x200000 0x600000 && fatload mmc 0 0x200000 boot.bin && sf erase 0 0x300000 && sf write 0x200000 0 0x300000 && echo - Write uImage... && mw.b 0x200000 0x600000 && fatload mmc 0 0x200000 uImage && sf erase 0x300000 ${kernel_size} && sf write 0x200000 0x300000 ${kernel_size} && echo - Write device tree... && mw.b 0x200000 0x600000 && fatload mmc 0 0x200000 devicetree.dtb && sf erase 0x800000 ${devicetree_size} && sf write 0x200000 0x800000 ${devicetree_size} && echo - Write Ramdisk... && mw.b 0x200000 0x600000 && fatload mmc 0 0x200000 uramdisk.image.gz && sf erase 0x820000 ${ramdisk_size} && sf write 0x200000 0x820000 ${ramdisk_size} && echo - Done.\0"
CONFIG_SYS_INIT_SP_ADDR="(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)"
CONFIG_SYS_ARCH="arm"
CONFIG_BAUDRATE=115200
CONFIG_SYS_BOARD="zynq"
CONFIG_ENV_IS_IN_SPI_FLASH=y
CONFIG_SYS_MMC_MAX_DEVICE=y
CONFIG_CMD_IMPORTENV=y
CONFIG_CMD_EXPORTENV=y
CONFIG_PARTITIONS=y
CONFIG_SYS_MEMTEST_END="(CONFIG_SYS_MEMTEST_START + PHYS_SDRAM_1_SIZE - (16 * 1024 * 1024))"
CONFIG_FIT_VERBOSE=y
CONFIG_CMD_ELF=y
CONFIG_ZYNQ_SERIAL=y
CONFIG_SYS_NO_FLASH=y
CONFIG_FS_EXT4=y
CONFIG_CMD_SOURCE=y
CONFIG_SYS_PROMPT="zynq-uboot> "
CONFIG_CLOCKS=y
CONFIG_SYS_FAULT_ECHO_LINK_DOWN=y
CONFIG_SYS_MEMTEST_START="PHYS_SDRAM_1"
CONFIG_CMD_LOADB=y
CONFIG_CMD_LOADS=y
CONFIG_CMD_IMI=y
CONFIG_SF_DEFAULT_SPEED=30000000
CONFIG_API=y
CONFIG_LMB=y
CONFIG_ARM=y
CONFIG_SYS_SDRAM_SIZE="PHYS_SDRAM_1_SIZE"
CONFIG_CMD_BDI=y
CONFIG_SERVERIP="192.168.1.7"
CONFIG_BOOTSTAGE_USER_COUNT=20
CONFIG_DEFAULT_DEVICE_TREE="zynq-zed"
CONFIG_ZYNQ_SDHCI=y
CONFIG_CMD_MII=y
CONFIG_CMD_MMC=y
CONFIG_TPL_BUILD=y
