CONFIG_SPI_FLASH_WINBOND=y
CONFIG_CMD_FAT=y
CONFIG_ZYNQ_SPI=y
CONFIG_BOOTM_NETBSD=y
CONFIG_HARD_I2C=y
CONFIG_ARMV7=y
CONFIG_CMD_ITEST=y
CONFIG_CMD_EDITENV=y
CONFIG_ZYNQ=y
CONFIG_CMD_CRC32=y
CONFIG_TIMER_PRESCALE=255
CONFIG_SYS_LOAD_ADDR=0
CONFIG_CMD_XIMG=y
CONFIG_BOOTDELAY=3
CONFIG_SPI_FLASH=y
CONFIG_SYS_HELP_CMD_WIDTH=8
CONFIG_NR_DRAM_BANKS=y
CONFIG_RTC_XPSSRTC=y
CONFIG_BOOTM_RTEMS=y
CONFIG_SYS_CBSIZE=2048
CONFIG_MD5=y
CONFIG_BOOTM_LINUX=y
CONFIG_BOARD_LATE_INIT=y
CONFIG_CMD_CONSOLE=y
CONFIG_ZYNQ_I2C=y
CONFIG_SYS_CACHELINE_SIZE=32
CONFIG_MMC=y
CONFIG_CMD_MISC=y
CONFIG_FIT=y
CONFIG_CMD_NET=y
CONFIG_CMD_NFS=y
CONFIG_ENV_SIZE=0x10000
CONFIG_CMD_PING=y
CONFIG_SYS_MALLOC_LEN=0x400000
CONFIG_SYS_I2C_SPEED=100000
CONFIG_SYS_TEXT_BASE=0x04000000
CONFIG_XGMAC_PHY_ADDR=0
CONFIG_CMD_SAVEENV=y
CONFIG_ZYNQ_MMC=y
CONFIG_FPGA_ZYNQPL=y
CONFIG_CMD_MEMORY=y
CONFIG_SYS_MAXARGS=16
CONFIG_CMD_RUN=y
CONFIG_IPADDR="192.168.1.253"
CONFIG_SYS_PBSIZE="(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)"
CONFIG_BOARDDIR="board/xilinx/zynq_common"
CONFIG_SYS_LONGHELP=y
CONFIG_OF_LIBFDT=y
CONFIG_FPGA_XILINX=y
CONFIG_CMDLINE_EDITING=y
CONFIG_FPGA=y
CONFIG_CMD_EXT2=y
CONFIG_BOOTCOMMAND="run modeboot"
CONFIG_TIMESTAMP=y
CONFIG_SYS_L2CACHE_OFF=y
CONFIG_SYS_MMC_MAX_BLK_COUNT=y
CONFIG_ZLIB=y
CONFIG_CMD_GO=y
CONFIG_CMD_BOOTD=y
CONFIG_CMD_BOOTM=y
CONFIG_TTC0=y
CONFIG_AUTO_COMPLETE=y
CONFIG_PANIC_HANG=y
CONFIG_SYS_HZ=1000
CONFIG_DOS_PARTITION=y
CONFIG_GZIP=y
CONFIG_CMD_SF=y
CONFIG_BOOTSTAGE_USER_COUNT=20
CONFIG_CMD_FPGA=y
CONFIG_SYS_INIT_RAM_SIZE=0x1000
CONFIG_SYS_BAUDRATE_TABLE="{ 9600, 38400, 115200 }"
CONFIG_SYS_DCACHE_OFF=y
CONFIG_SYS_SDRAM_BASE=0
CONFIG_SYS_BOOT_RAMDISK_HIGH=y
CONFIG_ZYNQ_IP_ENV=y
CONFIG_CMD_SPI=y
CONFIG_CMD_ECHO=y
CONFIG_GENERIC_MMC=y
CONFIG_SYS_INIT_RAM_ADDR=0xFFFF0000
CONFIG_ZYNQ_GEM=y
CONFIG_EXTRA_ENV_SETTINGS="ethaddr=00:0a:35:00:01:22\0kernel_size=0x140000\0ramdisk_size=0x200000\0star_test=go 0x05000000\0burn-qspi=sf probe 0 0 0\0qspiboot=sf probe 0 0 0;sf read 0x8000 0x00500000 0x300000;sf read 0x10000000 0x00900000 0x00400000;sf read 0x01000000 0x00e00000 0x00010000;go 0x8000\0sdboot=echo Copying Linux from SD to RAM...; mmcinfo;fatload mmc 0 0x8000 zImage;fatload mmc 0 0x1000000 devicetree.dtb;fatload mmc 0 0x10000000 ramdisk8M.image.gz;go 0x8000\0jtagboot=echo TFTPing Linux to RAM...;tftp 0x8000 zImage;tftp 0x1000000 devicetree.dtb;tftp 0x800000 ramdisk8M.image.gz;go 0x8000\0fpga=0\0fpgadata=0x00100000"
CONFIG_SYS_INIT_SP_ADDR="(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)"
CONFIG_BAUDRATE=115200
CONFIG_REGINFO=y
CONFIG_UART0=y
CONFIG_ZYNQ_I2C_CTLR_1=y
CONFIG_ENV_IS_NOWHERE=y
CONFIG_CMD_IMPORTENV=y
CONFIG_CMD_EXPORTENV=y
CONFIG_PARTITIONS=y
CONFIG_SYS_MEMTEST_END="(CONFIG_SYS_MEMTEST_START + PHYS_SDRAM_1_SIZE - (16 * 1024 * 1024))"
CONFIG_CMD_I2C=y
CONFIG_FIT_VERBOSE=y
CONFIG_CMD_DATE=y
CONFIG_SYS_NO_FLASH=y
CONFIG_SYS_DEF_EEPROM_ADDR=0
CONFIG_GEM0=y
CONFIG_SYS_I2C_SLAVE=0x50
CONFIG_CMD_SOURCE=y
CONFIG_SYS_PROMPT="star-uboot> "
CONFIG_SHA1=y
CONFIG_SYS_MEMTEST_START="PHYS_SDRAM_1"
CONFIG_CMD_LOADB=y
CONFIG_CMD_LOADS=y
CONFIG_CMD_IMI=y
CONFIG_SF_DEFAULT_SPEED=30000000
CONFIG_LMB=y
CONFIG_SYS_ICACHE_OFF=y
CONFIG_ARM=y
CONFIG_CMD_BDI=y
CONFIG_SERVERIP="192.168.1.8"
CONFIG_PSS_SERIAL=y
CONFIG_CMD_MMC=y
