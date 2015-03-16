/*
 * (C) Copyright 2000, 2001
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

/*
 *  FPGA support
 */
#include <common.h>
#include <command.h>
#if defined(CONFIG_CMD_NET)
#include <net.h>
#endif
#include <fpga.h>
#include <malloc.h>

#include "xdevcfg_hw.h"
#include "xdevcfg.h"
#include "fsbl.h"
#include "pcap.h"

/* Local functions */
static int fpga_get_op (char *opstr);

/* Local defines */
#define FPGA_NONE   -1
#define FPGA_INFO   0
#define FPGA_LOAD   1
#define FPGA_LOADB  2
#define FPGA_DUMP   3
#define FPGA_LOADMK 4


#define WORD_SWAP(X) (((X)&0xff)<<24)+      \
		    (((X)&0xff00)<<8)+      \
		    (((X)&0xff0000)>>8)+    \
		    (((X)&0xff000000)>>24)


extern u32 PcapDataTransfer(u32 *SourceDataPtr, u32 *DestinationDataPtr,
				u32 SourceLength, u32 DestinationLength, u32 SecureTransfer);

/* Convert bitstream data and load into the fpga */
int fpga_loadbitstream(unsigned long dev, char* fpgadata, size_t size)
{
#if defined(CONFIG_FPGA_XILINX)
	unsigned int length;
	unsigned int swapsize;
	char buffer[80];
	unsigned char *dataptr;
	unsigned int i;
	int rc;

	dataptr = (unsigned char *)fpgadata;

	/* skip the first bytes of the bitsteam, their meaning is unknown */
	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	dataptr+=length;

	/* get design name (identifier, length, string) */
	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	if (*dataptr++ != 0x61) {
		debug("%s: Design name identifier not recognized "
			"in bitstream\n",
			__func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	for(i=0;i<length;i++)
		buffer[i] = *dataptr++;

	printf("  design filename = \"%s\"\n", buffer);

	/* get part number (identifier, length, string) */
	if (*dataptr++ != 0x62) {
		printf("%s: Part number identifier not recognized "
			"in bitstream\n",
			__func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	for(i=0;i<length;i++)
		buffer[i] = *dataptr++;
	printf("  part number = \"%s\"\n", buffer);

	/* get date (identifier, length, string) */
	if (*dataptr++ != 0x63) {
		printf("%s: Date identifier not recognized in bitstream\n",
		       __func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	for(i=0;i<length;i++)
		buffer[i] = *dataptr++;
	printf("  date = \"%s\"\n", buffer);

	/* get time (identifier, length, string) */
	if (*dataptr++ != 0x64) {
		printf("%s: Time identifier not recognized in bitstream\n",
			__func__);
		return FPGA_FAIL;
	}

	length = (*dataptr << 8) + *(dataptr+1);
	dataptr+=2;
	for(i=0;i<length;i++)
		buffer[i] = *dataptr++;
	printf("  time = \"%s\"\n", buffer);

	/* get fpga data length (identifier, length) */
	if (*dataptr++ != 0x65) {
		printf("%s: Data length identifier not recognized in bitstream\n",
			__func__);
		return FPGA_FAIL;
	}
	swapsize = ((unsigned int) *dataptr     <<24) +
	           ((unsigned int) *(dataptr+1) <<16) +
	           ((unsigned int) *(dataptr+2) <<8 ) +
	           ((unsigned int) *(dataptr+3)     ) ;
	dataptr+=4;
	printf("  bytes in bitstream = %d\n", swapsize);

	rc = fpga_load(dev, dataptr, swapsize);
	return rc;
#else
	printf("Bitstream support only for Xilinx devices\n");
	return FPGA_FAIL;
#endif
}



u32 big_to_little(u32 *SourceData, u32 SourceLength)
{
    int i = 0;
    u32 tmp_big;
    u32 tmp_little;
    for (i = 0; i < SourceLength; i++)
    {
        tmp_big = *SourceData;
        tmp_little = WORD_SWAP(tmp_big);
        *SourceData++ = tmp_little;
    }
    return 0;
}


/* ------------------------------------------------------------------------- */
/* command form:
 *   fpga <op> <device number> <data addr> <datasize>
 * where op is 'load', 'dump', or 'info'
 * If there is no device number field, the fpga environment variable is used.
 * If there is no data addr field, the fpgadata environment variable is used.
 * The info command requires no data address field.
 */
int do_fpga (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int op, dev = FPGA_INVALID_DEVICE;
	size_t data_size = 0;
    int image_word_len = 0;
    int data_addr;
	void *fpga_data = NULL;
	char *devstr = getenv ("fpga");
	int rc = FPGA_FAIL;
	int wrong_parms = 0;
    int Status;
    
    /* loadb\t[dev] [address] [size] */
    /* fpga loadb 0x00100000 0x000F6EBF*/
    printf("Bit stream Unencrypted \r\n");

    
	if (devstr)
		dev = (int) simple_strtoul (devstr, NULL, 16);
        
	switch (argc) 
    {
	case 4:		/* fpga <op><data addr> <datasize> */
		data_size = simple_strtoul (argv[3], NULL, 16);
		data_addr = (void *) simple_strtoul (argv[2], NULL, 16);
		printf("%s: data_size: 0x%x, data_addr: 0x%x\n", __func__, data_size, data_addr);        
		op = (int) fpga_get_op (argv[1]);		 
		break;
            
	default:
		debug("%s: Too many or too few args (%d)\n",
			__func__, argc);
		op = FPGA_NONE;	/* force usage display */
		break;
	}
    
	if (dev == FPGA_INVALID_DEVICE) 
    {
		puts("FPGA device not specified\n");
		op = FPGA_NONE;
	}
    
	switch (op) 
    {
	case FPGA_NONE:
	case FPGA_INFO:
		break;
	case FPGA_LOAD:
	case FPGA_LOADB:
	case FPGA_DUMP:
		if (!data_size)
			wrong_parms = 1;
		break;
	case FPGA_LOADMK:
		if (!fpga_data)
			wrong_parms = 1;
		break;
	}
    
	if (wrong_parms) 
    {
		puts("Wrong parameters for FPGA request\n");
		op = FPGA_NONE;
	}

	switch (op) {
	case FPGA_NONE:
		return CMD_RET_USAGE;

	case FPGA_INFO:
		rc = fpga_info (dev);
		break;

	case FPGA_LOAD:
		rc = fpga_load (dev, fpga_data, data_size);
		break;
    
	case FPGA_LOADB:
        /* add by starsoc */
        {
            u32 Status = 0;
        
            //printf("before unlock, status: %d...\n", LockStatus());
            
            SlcrUnlock();
            
            //printf("after unlock, status: %d...\n", LockStatus());
            Status = InitPcap();
            if (Status == 1) 
            {
                printf("PCAP_INIT_FAIL \n\r");
            }
        }
        printf("before swap...\n");
        big_to_little((u32 *)(data_addr), data_size);
        printf("after swap...\n");
        if (data_size%4)
            image_word_len = (data_size/4+1)*4;
        else
            image_word_len = data_size;
        
        printf("do_fpga(), start_addr:0x%x, imagelen:0x%x, datalength:0x%x\r\n", 
            data_addr, image_word_len, data_size); 
        
		rc = PcapLoadPartition((u32*)data_addr,
					0,
					image_word_len,
					data_size,
					XDCFG_NON_SECURE_PCAP_WRITE);  
                    
        if(rc != FPGA_SUCCESS) 
        {
            printf("PCAP Bitstream Download Failed\r\n");
            return FPGA_FAIL;
        }
        
        SlcrLock();        
		break;
        
	case FPGA_LOADMK:
		switch (genimg_get_format (fpga_data)) {
		case IMAGE_FORMAT_LEGACY:
			{
				image_header_t *hdr = (image_header_t *)fpga_data;
				ulong	data;

				data = (ulong)image_get_data (hdr);
				data_size = image_get_data_size (hdr);
				rc = fpga_load (dev, (void *)data, data_size);
			}
			break;
		default:
			puts ("** Unknown image type\n");
			rc = FPGA_FAIL;
			break;
		}
		break;

	case FPGA_DUMP:
		rc = fpga_dump (dev, fpga_data, data_size);
		break;

	default:
		printf ("Unknown operation\n");
		return CMD_RET_USAGE;
	}
	return (rc);
}

/*
 * Map op to supported operations.  We don't use a table since we
 * would just have to relocate it from flash anyway.
 */
static int fpga_get_op (char *opstr)
{
	int op = FPGA_NONE;

	if (!strcmp ("info", opstr)) {
		op = FPGA_INFO;
	} else if (!strcmp ("loadb", opstr)) {
		op = FPGA_LOADB;
	} else if (!strcmp ("load", opstr)) {
		op = FPGA_LOAD;
	} else if (!strcmp ("loadmk", opstr)) {
		op = FPGA_LOADMK;
	} else if (!strcmp ("dump", opstr)) {
		op = FPGA_DUMP;
	}

	if (op == FPGA_NONE) {
		printf ("Unknown fpga operation \"%s\"\n", opstr);
	}
	return op;
}

U_BOOT_CMD (fpga, 4, 0, do_fpga,
	"loadable FPGA image support",
	"fpga operations:\n"
	"  loadb\t [address] [size]\t"
);
