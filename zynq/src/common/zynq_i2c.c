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
#include <i2c.h>
#include <asm/errno.h>

#include <malloc.h>
#include "xiicps.h"

#define DEBUG


/* Data Memory Barrier */
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#define SYNCHRONIZE_IO dmb()
static inline void Xout_le32(u32 *addr, u32 value)
{
	*(volatile u32 *)addr = value;
	SYNCHRONIZE_IO;
}

static inline u32 Xin_le32(u32 *addr)
{
	volatile u32 temp = *(volatile u32 *)addr;
	SYNCHRONIZE_IO;
	return temp;
}

static inline void Xclrbits_le32(u32 *addr, u32 clear)
{
	Xout_le32((addr), Xin_le32(addr) & ~(clear));
}

static inline void Xsetbits_le32(u32 *addr, u32 set)
{
	Xout_le32((addr), Xin_le32(addr) | (set));
}

struct zynq_i2c_registers {
	u32 control;
	u32 status;
	u32 address;
	u32 data;
	u32 interrupt_status;
	u32 transfer_size;
	u32 slave_mon_pause;
	u32 time_out;
	u32 interrupt_mask;
	u32 interrupt_enable;
	u32 interrupt_disable;
};

/*
 * Control register fields
 */

#define	ZYNQ_I2C_CONTROL_RW		0x00000001
#define	ZYNQ_I2C_CONTROL_MS		0x00000002
#define	ZYNQ_I2C_CONTROL_NEA	0x00000004
#define	ZYNQ_I2C_CONTROL_ACKEN	0x00000008
#define	ZYNQ_I2C_CONTROL_HOLD	0x00000010
#define	ZYNQ_I2C_CONTROL_SLVMON	0x00000020
#define	ZYNQ_I2C_CONTROL_CLR_FIFO		0x00000040
#define	ZYNQ_I2C_CONTROL_DIV_B_SHIFT	8
#define	ZYNQ_I2C_CONTROL_DIV_B_MASK		0x00003F00
#define	ZYNQ_I2C_CONTROL_DIV_A_SHIFT	14
#define	ZYNQ_I2C_CONTROL_DIV_A_MASK		0x0000C000

/*
 * Status register values
 */

#define	ZYNQ_I2C_STATUS_RXDV	0x00000020
#define	ZYNQ_I2C_STATUS_TXDV	0x00000040
#define	ZYNQ_I2C_STATUS_RXOVF	0x00000080
#define	ZYNQ_I2C_STATUS_BA		0x00000100

/*
 * Interrupt register fields
 */

#define	ZYNQ_I2C_INTERRUPT_COMP		0x00000001
#define	ZYNQ_I2C_INTERRUPT_DATA		0x00000002
#define	ZYNQ_I2C_INTERRUPT_NACK		0x00000004
#define	ZYNQ_I2C_INTERRUPT_TO		0x00000008
#define	ZYNQ_I2C_INTERRUPT_SLVRDY	0x00000010
#define	ZYNQ_I2C_INTERRUPT_RXOVF	0x00000020
#define	ZYNQ_I2C_INTERRUPT_TXOVF	0x00000040
#define	ZYNQ_I2C_INTERRUPT_RXUNF	0x00000080
#define	ZYNQ_I2C_INTERRUPT_ARBLOST	0x00000200

#if defined(CONFIG_ZYNQ_I2C_CTLR_0)
#define ZYNQ_I2C_BASE 0xE0004000
#if defined(CONFIG_ZYNQ_I2C_CTLR_1)
#warning Only CONFIG_ZYNQ_I2C_CTLR_0 will be accessible
#endif
#elif defined(CONFIG_ZYNQ_I2C_CTLR_1)
#define ZYNQ_I2C_BASE 0xE0005000
#else
#error You must select CONFIG_ZYNQ_I2C_CTLR_0 or CONFIG_ZYNQ_I2C_CTLR_1
#endif

#define ZYNQ_I2C_FIFO_DEPTH 16

static struct zynq_i2c_registers *zynq_i2c =
	(struct zynq_i2c_registers *) ZYNQ_I2C_BASE;






// I2C Config Struct
typedef struct {
	u8 Reg;
	u8 Data;
	u8 Init;
} ZC702_I2C_CONFIG;

#define ZC702_HDMI_ADDR1     0x39 // (Sil9134)
#define ZC702_HDMI_ADDR2	 0x3d // (Sil9134)
// HDMI 9134 1080P Separate Sync 422 422
#define ZC702_HDMI_OUT_LEN1  3
ZC702_I2C_CONFIG zc702_hdmi_out_config1[ZC702_HDMI_OUT_LEN1] =
{

		{0x05, 0x00, 0x01}, //
		{0x05, 0x00, 0x00}, //
		{0x08, 0x00, 0xfd} //

};

#define ZC702_HDMI_OUT_LEN2  11
ZC702_I2C_CONFIG zc702_hdmi_out_config2[ZC702_HDMI_OUT_LEN2] =
{
		{0x2f, 0x00, 0x21},
		{0x3e, 0x00, 0x03}, //
		{0x40, 0x00, 0x82}, //
		{0x41, 0x00, 0x02},
		{0x42, 0x00, 0x0d},
		{0x43, 0x00, 0xf7},//rgb in rgb out checksum
		{0x44, 0x00, 0x10},//rgb
		//	{0x44, 0x00, 0x20},	// yuv422
		{0x45, 0x00, 0x68},
		{0x46, 0x00, 0x00},
		{0x47, 0x00, 0x00},
		{0x3d, 0x00, 0x07}
};


XIicPs IIC1Instance;  


static void iic_writex( u8 Address, ZC702_I2C_CONFIG Config[], u32 Length )
{
	int i;

	for ( i = 0; i < Length; i++ )
	{
		//int i2c_write(u8 dev, uint addr, int alen, u8 *data, int length)
		i2c_write(Address, Config[i].Reg, 0, &Config[i].Init, 1);
	}
}


/*
 * I2C init called by cmd_i2c when doing 'i2c reset'.
 */
void i2c_init(int requested_speed, int slaveadd)
{
	/* Assume APB speed is 100MHz */
	/* For now, set it to ~400k always */
	/* 100MHz / (12 + 1) / 22 = ~350kHz */
	Xout_le32(&zynq_i2c->control, 12 << ZYNQ_I2C_CONTROL_DIV_B_SHIFT);
	/* Enable master mode, ack, and 7-bit addressing */
	Xsetbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_MS |
		ZYNQ_I2C_CONTROL_ACKEN | ZYNQ_I2C_CONTROL_NEA);
}

#ifdef DEBUG
static void zynq_i2c_debug_status(void)
{
	int int_status;
	int status;
	int_status = Xin_le32(&zynq_i2c->interrupt_status);
	status = Xin_le32(&zynq_i2c->status);
	if (int_status || status) {
		debug("Status: ");
		if (int_status & ZYNQ_I2C_INTERRUPT_COMP) debug("COMP ");
		if (int_status & ZYNQ_I2C_INTERRUPT_DATA) debug("DATA ");
		if (int_status & ZYNQ_I2C_INTERRUPT_NACK) debug("NACK ");
		if (int_status & ZYNQ_I2C_INTERRUPT_TO) debug("TO ");
		if (int_status & ZYNQ_I2C_INTERRUPT_SLVRDY) debug("SLVRDY ");
		if (int_status & ZYNQ_I2C_INTERRUPT_RXOVF) debug("RXOVF ");
		if (int_status & ZYNQ_I2C_INTERRUPT_TXOVF) debug("TXOVF ");
		if (int_status & ZYNQ_I2C_INTERRUPT_RXUNF) debug("RXUNF ");
		if (int_status & ZYNQ_I2C_INTERRUPT_ARBLOST) debug("ARBLOST ");
		if (status & ZYNQ_I2C_STATUS_RXDV) debug("RXDV ");
		if (status & ZYNQ_I2C_STATUS_TXDV) debug("TXDV ");
		if (status & ZYNQ_I2C_STATUS_RXOVF) debug("RXOVF ");
		if (status & ZYNQ_I2C_STATUS_BA) debug("BA ");
		debug("TS%d ", Xin_le32(&zynq_i2c->transfer_size));
		debug("\n");
	}
}
#endif

/*
 * Wait for an interrupt
 */
static u32 zynq_i2c_wait(u32 mask)
{
	int timeout, int_status;
	for (timeout = 0; timeout < 100; timeout++) {
		udelay(100);
		int_status = Xin_le32(&zynq_i2c->interrupt_status);
		if (int_status & mask)
			break;
	}
#ifdef DEBUG
	zynq_i2c_debug_status();
#endif
	/* Clear interrupt status flags */
	Xout_le32(&zynq_i2c->interrupt_status, int_status & mask);
	return int_status & mask;
}

/*
 * I2C probe called by cmd_i2c when doing 'i2c probe'.
 * Begin read, nak data byte, end.
 */
int i2c_probe(u8 dev)
{
	/* Attempt to read a byte */
	Xsetbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_CLR_FIFO |
		ZYNQ_I2C_CONTROL_RW);
	Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);
	Xout_le32(&zynq_i2c->interrupt_status, 0xFF);
	Xout_le32(&zynq_i2c->address, dev);
	Xout_le32(&zynq_i2c->transfer_size, 1);

	return (zynq_i2c_wait(ZYNQ_I2C_INTERRUPT_COMP | ZYNQ_I2C_INTERRUPT_NACK) &
		ZYNQ_I2C_INTERRUPT_COMP) ? 0 : ETIMEDOUT;
}

/*
 * I2C read called by cmd_i2c when doing 'i2c read' and by cmd_eeprom.c
 * Begin write, send address byte(s), begin read, receive data bytes, end.
 */
int i2c_read(u8 dev, uint addr, int alen, u8 *data, int length)
{
	u32 status;
	u32 i=0;
	u8 *cur_data = data;

	debug("dev:%x addr:%x alen:%x data:%x length:%x\n",dev,addr,alen,data,length);
	/* Write the register address */
	Xsetbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_CLR_FIFO |
		ZYNQ_I2C_CONTROL_HOLD);
	/* Temporarily disable restart (by clearing hold)... */
	/* It doesn't seem to work. */
	Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_RW |
		ZYNQ_I2C_CONTROL_HOLD);
	Xout_le32(&zynq_i2c->interrupt_status, 0xFF);
	while (alen--)
		Xout_le32(&zynq_i2c->data, addr >> (8*alen));
	Xout_le32(&zynq_i2c->address, dev);

	/* wait for the address to be sent */
	if (!zynq_i2c_wait(ZYNQ_I2C_INTERRUPT_COMP)) {
		/* Release the bus */
		Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);
		debug("timeout\n");
		return ETIMEDOUT;
	}
	debug("Device acked address\n");

	Xsetbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_CLR_FIFO |
		ZYNQ_I2C_CONTROL_RW);
	/* Start reading data */
	Xout_le32(&zynq_i2c->address, dev);
	Xout_le32(&zynq_i2c->transfer_size, length);

	/* wait for data */
	do {
		status = zynq_i2c_wait(ZYNQ_I2C_INTERRUPT_COMP |
			ZYNQ_I2C_INTERRUPT_DATA);
		if (!status) {
			/* Release the bus */
			Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);
			return ETIMEDOUT;
		}
		debug("Read %d bytes\n", length - Xin_le32(&zynq_i2c->transfer_size));
		for (; i < length - Xin_le32(&zynq_i2c->transfer_size); i++)
			*(cur_data++) = Xin_le32(&zynq_i2c->data);
	} while (Xin_le32(&zynq_i2c->transfer_size) != 0);
	/* All done... release the bus */
	Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);

#ifdef DEBUG
	zynq_i2c_debug_status();
#endif
	return 0;
}

/*
 * I2C write called by cmd_i2c when doing 'i2c write' and by cmd_eeprom.c
 * Begin write, send address byte(s), send data bytes, end.
 */
int i2c_write(u8 dev, uint addr, int alen, u8 *data, int length)
{
	u8 *cur_data = data;

	/* Write the register address */
	Xsetbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_CLR_FIFO |
		ZYNQ_I2C_CONTROL_HOLD);
	Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_RW);
	Xout_le32(&zynq_i2c->interrupt_status, 0xFF);
	while (alen--)
		Xout_le32(&zynq_i2c->data, addr >> (8*alen));
	/* Start the tranfer */
	Xout_le32(&zynq_i2c->address, dev);
	if (!zynq_i2c_wait(ZYNQ_I2C_INTERRUPT_COMP)) {
		/* Release the bus */
		Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);
		return ETIMEDOUT;
	}

	debug("Device acked address\n");
	while (length--) {
		Xout_le32(&zynq_i2c->data, *(cur_data++));
		if (Xin_le32(&zynq_i2c->transfer_size) == ZYNQ_I2C_FIFO_DEPTH) {
			if (!zynq_i2c_wait(ZYNQ_I2C_INTERRUPT_COMP)) {
				/* Release the bus */
				Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);
				return ETIMEDOUT;
			}
		}
	}

	/* All done... release the bus */
	Xclrbits_le32(&zynq_i2c->control, ZYNQ_I2C_CONTROL_HOLD);
	/* wait for the address and data to be sent */
	if (!zynq_i2c_wait(ZYNQ_I2C_INTERRUPT_COMP)) return ETIMEDOUT;
	return 0;
}

int i2c_set_bus_num(unsigned int bus)
{
	/* Only support bus 0 */
	if (bus > 0) {
		return -1;
	}
	return 0;
}

unsigned int i2c_get_bus_num(void)
{
	/* Only support bus 0 */
	return 0;
}
