/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

/***************************** Include Files *********************************/
#include "xparameters.h"
#include "xgpio.h"
#include "sleep.h"

/************************** Variable Defintions ******************************/
XGpio Gpio_leds;
XGpio Gpio_keys;
/* Instance For GPIO */
XGpio GpioOutput;

int pl_gpio_key_init ()
{

    int Status;
    u32 DataRead = 0;
    int i = 0;
    int test_count = 0;
    
    Status = XGpio_Initialize(&Gpio_keys, XPAR_KEY_IO_DEVICE_ID);
    if (Status != XST_SUCCESS) 
    {
        return XST_FAILURE;
    }
	/* set gpio key as input */
    XGpio_SetDataDirection(&Gpio_keys, 1, 0xFFFFFFFF);
	
    Status = XGpio_Initialize(&Gpio_leds, XPAR_LED_IO_DEVICE_ID);
    if (Status != XST_SUCCESS) 

    {
        return XST_FAILURE;
    }
    /* set gpio led as output */
    XGpio_SetDataDirection(&Gpio_leds, 1, 0x0);
    printf("****** Totally 4 times testing, you have %d times left\r\n", (5 - test_count));
    while(test_count < 4)
    {
        DataRead = XGpio_DiscreteRead(&Gpio_keys, 1);
        /* if press one of five key, DataRead would not equal to 0x1F */
        if ((DataRead & 0x1F) != 0x1F)
        {
            XGpio_DiscreteWrite(&Gpio_leds, 1, DataRead);
            usleep(300000);
            DataRead = XGpio_DiscreteRead(&Gpio_keys, 1);
            XGpio_DiscreteWrite(&Gpio_leds, 1, DataRead);
            usleep(300000);
            test_count++;
            printf("****** read again, you still have %d times left\r\n", (5 - test_count));
        }                
    }
    usleep(500000);
    DataRead = XGpio_DiscreteRead(&Gpio_keys, 1);
    /* finally, wirte all led as 1, turn off the led */
    XGpio_DiscreteWrite(&Gpio_leds, 1, 0x1F);
    
    return 0;
}



int pl_gpio_led_init()
{

	u32 Delay;
	u32 Ledwidth;
    int i = 0;
    XGpio_Initialize(&GpioOutput, XPAR_LED_IO_DEVICE_ID);
    XGpio_SetDataDirection(&GpioOutput, 1, 0x0);
    XGpio_DiscreteWrite(&GpioOutput, 1, 0x0);
    
    for (i = 0; i < 2; i++)
    {
        printf("--The %d time led flashing --\r\n", i+1);
        for (Ledwidth = 0x0; Ledwidth < 4; Ledwidth++)
        {
             XGpio_DiscreteWrite(&GpioOutput, 1, 1 << Ledwidth);
             usleep(500000);
             XGpio_DiscreteClear(&GpioOutput, 1, 1 << Ledwidth);
        }
    }
    /* Turn OFF all the LED */
    XGpio_DiscreteWrite(&GpioOutput, 1, 0x1F);
    
    return 0;
}

