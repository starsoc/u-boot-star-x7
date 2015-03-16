/*
 * (C) Copyright 2012 Xilinx
 *
 * Xilinx Serial PS driver
 * Based on existing U-boot serial drivers.
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

#include <common.h>

#include "serial_xpsuart.h"

DECLARE_GLOBAL_DATA_PTR;

/* add by star */
extern int test_bootdelay;
/* Set up the baud rate in gd struct */
void serial_setbrg(void)
{
	/*              master clock
	 * Baud rate = ---------------
	 *              bgen*(bdiv+1)
	 */
	long baud = gd->baudrate;

	/* Variables to vary. */
	unsigned int bdiv, bgen;

	/* Calculation results. */
	long calc_baud = 0;
	unsigned int calc_bauderror;

	/* Find acceptable values for baud generation. */
	for (bdiv = 4; bdiv < 255; bdiv++) {

		bgen = XZYNQUART_MASTER / (baud * (bdiv + 1));
		if (bgen < 2 || bgen > 65535)
			continue;

		calc_baud = XZYNQUART_MASTER / (bgen * (bdiv + 1));

		/* Use first calculated baudrate with an acceptable
		 * (<3%) error.
		 */
		if (baud > calc_baud)
			calc_bauderror = baud - calc_baud;
		else
			calc_bauderror = calc_baud - baud;
		if ( ((calc_bauderror * 100) / baud) < 3 )
			break;

	}
    
	xdfuart_writel(BAUDDIV,bdiv);
	xdfuart_writel(BAUDGEN,bgen);
}

/* Initialize the UART, with...some settings. */
int serial_init(void)
{
	xdfuart_writel(CR,0x17); /* RX/TX enabled & reset */
	xdfuart_writel(MR,0x20); /* 8 bit, no parity */
	serial_setbrg();
	return 0;
}

/* Write a char to the Tx buffer */
void serial_putc(char c)
{
	while ((xdfuart_readl(SR) & XZYNQUART_SR_TXFULL) != 0)
		;
	if (c == '\n') {
		xdfuart_writel(FIFO,'\r');
		while ((xdfuart_readl(SR) & XZYNQUART_SR_TXFULL) != 0)
			;
	}
	xdfuart_writel(FIFO,c);
}

/* Write a null-terminated string to the UART */
void serial_puts(const char *s)
{
	while (*s)
		serial_putc(*s++);
}

/* Get a char from Rx buffer */
int serial_getc(void)
{
	while (!serial_tstc());
	return xdfuart_readl(FIFO);
}

/* Test character presence in Rx buffer */
int serial_tstc(void)
{
	/* add by starsoc */
	if (test_bootdelay)
		printf("***actual serial_tstc function \n");
	/* UART_BASE + XZYNQUART_SR_OFFSET */
	return (xdfuart_readl(SR) & XZYNQUART_SR_RXEMPTY) == 0;
}
