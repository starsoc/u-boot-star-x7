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

#define PL_GPIO_LED_NUM		4
#define PL_GPIO_KEY_TEST_TIME 	4
#define PL_GPIO_LED_OFF_DATA	0xF

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
    bool b_pl_key_pressed = false;
	
    Status = XGpio_Initialize(&Gpio_keys, XPAR_KEY_IO_DEVICE_ID);
    if (Status != XST_SUCCESS) 
    {
        return XST_FAILURE;
    }
	/* set pl gpio key as input */
    XGpio_SetDataDirection(&Gpio_keys, 1, 0xFFFFFFFF);
	
    Status = XGpio_Initialize(&Gpio_leds, XPAR_LED_IO_DEVICE_ID);
    if (Status != XST_SUCCESS) 

    {
        return XST_FAILURE;
    }
    /* set pl gpio led as output */
    XGpio_SetDataDirection(&Gpio_leds, 1, 0x0);
	
    printf("****** Totally 4 times testing, you have %d times left\r\n", (PL_GPIO_KEY_TEST_TIME - test_count));
    while(test_count < PL_GPIO_KEY_TEST_TIME)
    {
        DataRead = XGpio_DiscreteRead(&Gpio_keys, 1);
        /* if press one of 4 key, DataRead would not equal to PL_GPIO_LED_OFF_DATA */
        if ((DataRead & PL_GPIO_LED_OFF_DATA) != PL_GPIO_LED_OFF_DATA)
        {
			printf("******zynq_pl_gpio_key_test, key is pressed\r\n");
            XGpio_DiscreteWrite(&Gpio_leds, 1, DataRead);
            usleep(50);
            DataRead = XGpio_DiscreteRead(&Gpio_keys, 1);
			printf("---After setting PL GPIO LED ON, value:0x%x\r\n", DataRead);
        	b_pl_key_pressed = true;
			while(b_pl_key_pressed)
			{
            	DataRead = XGpio_DiscreteRead(&Gpio_keys, 1);
				if (DataRead == 0xF)
				{
					mdelay(200);
					printf("******zynq_pl_gpio_key_test, key is released\r\n");
					/* GPIO key is released*/
					b_pl_key_pressed = false;
					/* Set the GPIO output to be high, led off*/
					XGpio_DiscreteWrite(&Gpio_leds, 1, 0xF);
					mdelay(50);
					DataRead = XGpio_DiscreteRead(&Gpio_keys, 1);
					printf("---After setting GPIO LED OFF, value:0x%x\r\n", DataRead);	
					test_count++;
				}
			}			
            printf("****** read again, you still have %d times left\r\n", (PL_GPIO_KEY_TEST_TIME - test_count));
        }                
    }
	
    DataRead = XGpio_DiscreteRead(&Gpio_keys, 1);
    /* finally, wirte all led as 1, turn off the led */
    XGpio_DiscreteWrite(&Gpio_leds, 1, 0xF);
	
    return 0;
}


int pl_gpio_led_init()
{

	u32 Delay;
	u32 Ledwidth;
	u32 DataRead;
    int i = 0;

	printf("XGpio_Initialize\r\n");
    XGpio_Initialize(&GpioOutput, XPAR_LED_IO_DEVICE_ID);
	
	printf("XGpio_SetDataDirection\r\n");
    XGpio_SetDataDirection(&GpioOutput, 1, 0x0);
	printf("XGpio_DiscreteWrite\r\n");
    XGpio_DiscreteWrite(&GpioOutput, 1, 0x0);

	
	
    for (i = 0; i < 2; i++)
    {
        printf("--The %d time led flashing --\r\n", i+1);
        for (Ledwidth = 0x0; Ledwidth < PL_GPIO_LED_NUM; Ledwidth++)
        {
             XGpio_DiscreteWrite(&GpioOutput, 1, 1 << Ledwidth);
			 DataRead = XGpio_DiscreteRead(&GpioOutput, 1);
			 printf("******pl gpio led value:0x%x\r\n", DataRead);
             usleep(500000);
             XGpio_DiscreteClear(&GpioOutput, 1, 1 << Ledwidth);
        }
    }
    /* Turn OFF all the LED */
    XGpio_DiscreteWrite(&GpioOutput, 1, PL_GPIO_LED_OFF_DATA);
    
    return 0;
}

