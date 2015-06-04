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
#include "xuartps_hw.h"

extern int ps_uart_init (void);


/************************** Variable Defintions ******************************/
void uart_send(char c)
{
	 XUartPs_SendByte(STDOUT_BASEADDRESS, c);
}


char uart_rcv(void)
{
	 return XUartPs_RecvByte(STDIN_BASEADDRESS);
}

int ps_uart_init (void)
{

	char rcv_char;
    
    uart_send('H');
    uart_send('E');
    uart_send('L');
    uart_send('L');
    uart_send('O');
    uart_send(',');
    uart_send('S');
    uart_send('T');
    uart_send('A');
    uart_send('R');
    uart_send('S');
    uart_send('O');
    uart_send('C');
    uart_send('\r');
    uart_send('\n');
#if 0
    /* print("Hello World\n\r"); */
    /* testing star-zynq7000 send char to PC terminal */
    while(1)
    {
        rcv_char = uart_rcv();
        printf("recieved char is %c\r\n", rcv_char);
    }
#endif    
    return 0;
}

