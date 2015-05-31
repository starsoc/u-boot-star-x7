/*
 * SPI flash interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <spi.h>
#include <spi_flash.h>

static int spi_flash_read_write(struct spi_slave *spi,
				const u8 *cmd, size_t cmd_len,
				const u8 *data_out, u8 *data_in,
				size_t data_len)
{
	unsigned long flags = SPI_XFER_BEGIN;
	int ret;

	if ((spi->is_dual == MODE_DUAL_STACKED) && (spi->u_page == 1))
		flags |= SPI_FLASH_U_PAGE;
	if (data_len == 0)
		flags |= SPI_XFER_END;
    
    printf("######spi_flash_read_write(), data_len:%d cmd_len:%d\r\n", data_len, cmd_len);
    
    ret = spi_xfer(spi, cmd_len * 8, cmd, NULL, flags);
	if (ret) 
    {
		debug("SF: Failed to send command (%zu bytes): %d\n",
		      cmd_len, ret);
	}
    else if (data_len != 0) 
    {
		ret = spi_xfer(spi, data_len * 8, data_out, data_in,
					SPI_XFER_END);
		if (ret)
			debug("SF: Failed to transfer %zu bytes of data: %d\n",
			      data_len, ret);
	}
    
	return ret;
}

int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	return spi_flash_read_write(spi, cmd, cmd_len, NULL, data, data_len);
}

int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len)
{
	return spi_flash_cmd_read(spi, &cmd, 1, response, len);
}

int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len)
{
	return spi_flash_read_write(spi, cmd, cmd_len, data, NULL, data_len);
}

int spi_flash_cmd_erase(struct spi_flash *flash, u8 erase_cmd,
			u32 offset, size_t len)
{
	u32 start, end, erase_size;
	int ret;
	unsigned long page_addr;
	u8 cmd[flash->addr_width+1];
	
	erase_size = flash->sector_size;
	if (offset % erase_size || len % erase_size) 
	{
		debug("SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}
	printf("spi_flash_cmd_erase(), offset:0x%x,len:0x%x, erase_size:0x%x\r\n", 
		offset, len, erase_size);
	qspi_erase(offset, len);
		
	
    return 0;
}


int spi_flash_cmd_read_fast(struct spi_flash *flash, u32 offset, size_t len, void *data)
{
	unsigned long page_addr;
	unsigned long page_size;
	unsigned long byte_addr;
	u8 cmd[flash->addr_width+2];
	
	page_size = flash->page_size;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;
	
	printf("spi_flash_cmd_read_fast(), offset:0x%x, len:0x%x, data:0x%x, page_size:0x%x, page_addr:0x%x, byte_addr:0x%x\r\n",
			offset, len, (u32)(data), page_size, page_addr, byte_addr);
	qspi_read(offset, len, data, page_size);
#if 0
	cmd[0] = CMD_READ_ARRAY_FAST;
	spi_flash_addr(flash, page_addr, byte_addr, cmd);
	cmd[sizeof(cmd)-1] = 0x00;

	return spi_flash_read_common(flash, cmd, sizeof(cmd), data, len);
#endif
	return 0;

}

int spi_flash_cmd_write_multi(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	unsigned long page_addr, byte_addr, page_size;
	int page_count, page;
	size_t chunk_len, actual;
	int ret;
	u8 cmd[flash->addr_width+1];
	
	page_size = flash->page_size;
	page_addr = offset / page_size;
	byte_addr = offset % page_size;
	page_count = len / page_size;
	printf("spi_flash_cmd_write_multi(), offset:0x%x, len:0x%x, buf:0x%x, page_size:0x%x, page_addr:0x%x, byte_addr:0x%x\r\n",
			offset, len, (u32)(buf), page_size, page_addr, byte_addr);
	
	qspi_write(offset, len, buf, page_size);
	
	
	return 0;
}

