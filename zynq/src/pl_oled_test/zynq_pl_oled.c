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

#include "oled_pic.h"
#include "xgpio.h"

#define CS_GPIO 0x00001000              //bit12
#define WE_GPIO  0x00000200              //bit9
#define RD_GPIO  0x00000100              //bit8
#define RES_GPIO 0x00000800             //bit11
#define DC_GPIO  0x00000400             //bit10

XGpio Gpio_oled;

void oled_init();

int write_i(u8 data)
{
   u32 reg_data;
    
   XGpio_SetDataDirection(&Gpio_oled, 1, 0x0);                 //set data0~data7 is output,
   reg_data=0x7f00;                                               //set data to 0
   reg_data&=~DC_GPIO;
   XGpio_DiscreteWrite(&Gpio_oled, 1, reg_data);                 //set D/C pin low for command write
   reg_data&=~CS_GPIO;
   XGpio_DiscreteWrite(&Gpio_oled, 1, reg_data);                 //set CS pin low for command write
   reg_data&=~WE_GPIO;
   reg_data|=data;
   XGpio_DiscreteWrite(&Gpio_oled, 1, reg_data);                 //set WE pin low for write the data
   udelay(50);
   reg_data|=WE_GPIO;
   XGpio_DiscreteWrite(&Gpio_oled, 1, reg_data);                 //set WE pin high
   reg_data|=CS_GPIO;
   XGpio_DiscreteWrite(&Gpio_oled, 1, reg_data);                 //set CS pin high
   udelay(10);
        
   XGpio_SetDataDirection(&Gpio_oled, 1, 0xff);                  //set data0~data7 is input,

   return 0;
}

int write_d(u8 data)
{
	u32 disp_data;

	XGpio_SetDataDirection(&Gpio_oled, 1, 0x0);                 //set data0~data7 is output,
	disp_data=0x7f00;                                               //set data to 0
	disp_data|=DC_GPIO;
	XGpio_DiscreteWrite(&Gpio_oled, 1, disp_data);                 //set D/C pin high for data write
	disp_data&=~CS_GPIO;
	XGpio_DiscreteWrite(&Gpio_oled, 1, disp_data);                 //set CS pin low for data write
	disp_data&=~WE_GPIO;
	disp_data|=data;
	XGpio_DiscreteWrite(&Gpio_oled, 1, disp_data);                 //set WE pin low for write the data
	udelay(50);
	disp_data|=WE_GPIO;
	XGpio_DiscreteWrite(&Gpio_oled, 1, disp_data);                 //set WE pin high
	disp_data|=CS_GPIO;
	XGpio_DiscreteWrite(&Gpio_oled, 1, disp_data);                 //set CS pin high
	udelay(10);
	XGpio_SetDataDirection(&Gpio_oled, 1, 0xff);               //set data0~data7 is input,

	return 0;

}

void  block_write(unsigned char  dat1,unsigned char dat2)
{
    unsigned  char  x,y;
    u16 pixel;
    write_i(0x15);  /*  set column  address  */
    write_i(0x00);  /*  set  start  address  */
    write_i(0x5f);  /*  set end   address  */
    write_i(0x75);  /*  set row  address  */
    write_i(0x00);  /*  set  start  address  */
    write_i(0x3f);  /*  set end   address  */
    pixel=0;
    for(y=0;y<64;y++)
    {
        for(x=0;x<96;x++)
        {
            write_d(gImage_pic[pixel]);
            pixel++;
            write_d(gImage_pic[pixel]);
            pixel++;

            // 	   write_d(dat1);
            //      write_d(dat2);
        }
    }
}


void  ssd1331_init()
{
	write_i(0xae); /* Display off*/
    
	write_i(0x81); /*set contrast for colorA*/
	write_i(0x91); /*145*/

	write_i(0x82); /*set contrast for colorB*/
	write_i(0x5f); /*50*/

	write_i(0x83); /*set contrast for colorC*/
	write_i(0x7d); /*125*/

	write_i(0x87); /*master current control*/
	write_i(0x06); /*6*/

	write_i(0x8a); /*Set Second Pre-change Speed For ColorA*/
	write_i(0x64); /*100*/

	write_i(0x8b); /*Set Second Pre-change Speed For ColorB*/
	write_i(0x78); /*120*/

	write_i(0x8c); /*Set Second Pre-change Speed For ColorC*/
	write_i(0x64); /*100*/

	write_i(0xa0); /*set re=map & data format 1*/
	write_i(0x60);  /*0x72 180*/

	write_i(0xa1); /*set display start line*/
	write_i(0x00);

	write_i(0xa2); /*set display offset*/
	write_i(0x00);

	write_i(0xa4); /*set display mode*/

	write_i(0xa8); /*set multiplex ratio*/
	write_i(0x3f);

	write_i(0xad); /*set master configuration*/
	write_i(0x8e);

	write_i(0xb0); /*set power save*/
	write_i(0x00);

	write_i(0xb1); /*phase 1 and 2 period adjustment*/
	write_i(0x31);

	write_i(0xb3); /*display clock divider / oscillator frequency*/
	write_i(0xf0);

	write_i(0xbb); /*Set Pre-Change Level*/
	write_i(0x3a); /*58*/

	write_i(0xbe); /*set vcomh*/
	write_i(0x3e); /*62*/
    
	write_i(0x25); /*clear*/
	write_i(0x00);
	write_i(0x5f);
	write_i(0x00);
	write_i(0x30);
    
	write_i(0xaf); /*set display on  */
    mdelay(3000);
    
	write_i(0xae); /* Display off*/
    
}



void oled_init()
{
    
    XGpio_Initialize(&Gpio_oled, XPAR_OLED_IO_DEVICE_ID);
    XGpio_SetDataDirection(&Gpio_oled, 1,  0xff);               //data0~data7 is input, other is output
    XGpio_DiscreteWrite(&Gpio_oled, 1, 0x7f00);                 //set all high for 8080 interface
    
    ssd1331_init();
    block_write(0xf8,0x00);
    return;
}



