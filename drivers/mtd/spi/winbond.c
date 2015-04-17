/*
 * Copyright 2008, Network Appliance Inc.
 * Author: Jason McMullan <mcmullan <at> netapp.com>
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <malloc.h>
#include <spi_flash.h>

#include "spi_flash_internal.h"

/* M25Pxx-specific commands */
#define CMD_W25_WREN		0x06	/* Write Enable */
#define CMD_W25_WRDI		0x04	/* Write Disable */
#define CMD_W25_RDSR		0x05	/* Read Status Register */
#define CMD_W25_WRSR		0x01	/* Write Status Register */
#define CMD_W25_READ		0x03	/* Read Data Bytes */
#define CMD_W25_FAST_READ	0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_W25_PP		0x02	/* Page Program */
#define CMD_W25_SE		0x20	/* Sector (4K) Erase */
#define CMD_W25_BE		0xd8	/* Block (64K) Erase */
#define CMD_W25_CE		0xc7	/* Chip Erase */
#define CMD_W25_DP		0xb9	/* Deep Power-down */
#define CMD_W25_RES		0xab	/* Release from DP, and Read Signature */

struct winbond_spi_flash_params {
	uint16_t	id;
	/* Log2 of page size in power-of-two mode */
	uint8_t		l2_page_size;
	uint16_t	pages_per_sector;
	uint16_t	sectors_per_block;
	uint16_t	nr_blocks;
	const char	*name;
};

static const struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	{
		.id			= 0x3013,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 8,
		.name			= "W25X40",
	},
	{
		.id			= 0x3015,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 32,
		.name			= "W25X16",
	},
	{
		.id			= 0x3016,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 64,
		.name			= "W25X32",
	},
	{
		.id			= 0x3017,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 128,
		.name			= "W25X64",
	},
	{
		.id			= 0x4015,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 32,
		.name			= "W25Q16",
	},
	{
		.id			= 0x4016,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 64,
		.name			= "W25Q32",
	},
	{
		.id			= 0x4017,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 128,
		.name			= "W25Q64",
	},
	{
		.id			= 0x4018,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 256,
		.name			= "W25Q128",
	},
	/* add by star */
	{
		.id			= 0x4019,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 256,
		.name			= "W25Q256",
	},
	{
		.id			= 0x6017,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 128,
		.name			= "W25Q64DW",
	},
};



static int winbond_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	return spi_flash_cmd_erase(flash, CMD_W25_SE, offset, len);
}

struct spi_flash *spi_flash_probe_winbond(struct spi_slave *spi, u8 *idcode)
{
	const struct winbond_spi_flash_params *params;
	struct spi_flash *flash;
	unsigned int i;
	unsigned page_size;
	/* idcode[1]: 0x40  idcode[2]:0x19 */
	for (i = 0; i < ARRAY_SIZE(winbond_spi_flash_table); i++) {
		params = &winbond_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}
	
	
	if (i == ARRAY_SIZE(winbond_spi_flash_table)) {
		debug("SF: Unsupported Winbond ID %02x%02x\n",
				idcode[1], idcode[2]);
		/* add by star */
		printf("SF: Unsupported Winbond ID %02x%02x\n", idcode[1], idcode[2]);
		return NULL;
	}
	/* add by star */
	printf("Winbond Flash found, id: 0x%x, name: %s\n", params->id, params->name);
	
	flash = malloc(sizeof(*flash));
	if (!flash) {
		debug("SF: Failed to allocate memory\n");
		return NULL;
	}
	
	flash->spi = spi;
	flash->name = params->name;
        
	/* Assuming power-of-two page size initially. */
	page_size = 1 << params->l2_page_size;
    
	flash->write = spi_flash_cmd_write_multi;
	flash->erase = winbond_erase;
	flash->read = spi_flash_cmd_read_fast;
	flash->page_size = page_size;
	flash->sector_size = page_size * params->pages_per_sector;
    
	/* add by star */
    /*
            printf("Winbond Flash found, is_dual:%d, page_size:%d, pages_per_sector:%d, sectors_per_block:%d, nr_blocks:%d\n", 
                flash->spi->is_dual, page_size, params->pages_per_sector, params->sectors_per_block, params->nr_blocks);
            {
                .id			= 0x4019,
                .l2_page_size		= 8,
                .pages_per_sector	= 16,
                .sectors_per_block	= 16,
                .nr_blocks		= 256,
                .name			= "W25Q256",
            }	
    */
	/* address width is 4 for dual and 3 for single qspi */

#if 0
	if (flash->spi->is_dual == 1) 
    {
		flash->addr_width = 4;
		flash->size = page_size * params->pages_per_sector
				* params->sectors_per_block
				* (2 * params->nr_blocks);
	}    
#endif
/*    else if (flash->spi->is_dual == 0)  */
    {
		flash->addr_width = 3;
        /* 256*16*16*256 */
		flash->size = page_size * params->pages_per_sector
				* params->sectors_per_block
				* 2* params->nr_blocks;
        /* printf("Winbond flash size:%x\r\n", flash->size); */
	}
	return flash;
}
