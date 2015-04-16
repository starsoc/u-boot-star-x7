/*
 * (C) Copyright 2012 Xilinx
 *
 * The XEmacPss Wrapper driver.
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
#include <malloc.h>
#include <miiphy.h>
#include <net.h>
#include "zynq_gem.h"

/* add by starsoc  */
#include <linux/mii.h>
/* delete by starsoc for Realtek RTL8211E */
/* #define MARVELL_88E1116R */
#define MII_MARVELL_PHY_PAGE			22
#define MII_M1116R_CONTROL_REG_MAC		21
#define MII_M1011_PHY_SCR				0x10
#define MII_M1011_PHY_SCR_AUTO_CROSS	0x0060

#define PHY_CRTL_REG                  							0x00
#define PHY_EXTNAL_CRTL_REG1          						0x1c
#define PHY_AUX_CTRL_REG									0X18
#define	PHY_EXTNAL_CRTL_REG16								0X10

#define PHY_MII_SHADOW_WEN_MASK  				0x8000    		//shaow????????D??：o?┐?：1
#define PHY_MII_SHADOW_ADDR		  					0x2c00   		//：：：a?t?????：????│：?????????|━???∴
#define PHY_MII_RESET_MASK               				 	0x8000         //?????bit15?┷?3?D?2.0us
#define PHY_MII_CLRRST_MASK               				0x0000         //???????：♂?
#define PHY_EXTNAL_CRTL_WREN_MASK         		0xA500         //D??reg：o1?：1
#define PHY_MII_RSTEN_MASK                				0x2540         //?????：o1?：1?┷oshadow value: bit[14:10] 01011; bit6 1
#define PHY_MII_MODE_MASK                 				0x2510         //RGMII 1.8v: bit[4:3] 10

#define PHY_SHADOW_MASK									0x83FF			//：????│：??????????????：2??
#define PHY_SPEED_MASK										0xdfbF			//：????│：??????????????：2??

#define PHY_AUTO_NIGOTIATION_MASK         		0x1000         //bit12,0   1??┐??│?：o：o：?|
#define PHY_DUPLEX_MODE_MASK              			0x0100         //bit8, 1   ?a??：：???1?┬
#define PHY_SPEED_100M               	 						0x2000         //bit[6,13], 01
#define PHY_SPEED_1000M             	 						0x0040         //bit[6,13], 10

#define PHY_Status_REG											0x01
#define PHY_AutoNegDone_MASK							0x0020
#define PHY_LinkUp_MASK										0x0004
#define PHY_100M_FULL_MASK								0x4000			//reg
#define PHY_10M_FULL_MASK									0x1000			//reg

#define PHY_Status_EXREG										0x0f
#define PHY_1000M_FULL_MASK								0x2000			//exreg

#define XEMACPS_GMII2RGMII_SPEED1000_FD		0x140
#define XEMACPS_GMII2RGMII_SPEED100_FD		0x2100
#define XEMACPS_GMII2RGMII_SPEED10_FD		0x100
#define XEMACPS_GMII2RGMII_REG_NUM			0x10


u32 g_phyaddr = 0;



/************************ Forward function declaration **********************/

static int Xgmac_process_rx(XEmacPss *EmacPssInstancePtr);
static int Xgmac_init_rxq(XEmacPss *EmacPssInstancePtr,
			void *bd_start, int num_elem);
static int Xgmac_make_rxbuff_mem(XEmacPss *EmacPssInstancePtr,
			void *rx_buf_start, u32 rx_buffsize);
static int Xgmac_next_rx_buf(XEmacPss *EmacPssInstancePtr);
static int Xgmac_phy_mgmt_idle(XEmacPss *EmacPssInstancePtr);
static void Xgmac_set_eth_advertise(XEmacPss *EmacPssInstancePtr,
			int link_speed);

/*************************** Constant Definitions ***************************/

#define EMACPSS_DEVICE_ID   0
#define RXBD_CNT       8	/* Number of RxBDs to use */
#define TXBD_CNT       8	/* Number of TxBDs to use */

#define phy_spinwait(e) do { while (!Xgmac_phy_mgmt_idle(e)); } while (0)

#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")

/*************************** Variable Definitions ***************************/

/*
 * Aligned memory segments to be used for buffer descriptors
 */
//#define BRAM_BUFFERS
#ifdef BRAM_BUFFERS
static XEmacPss_Bd RxBdSpace[RXBD_CNT] __attribute__ ((section (".bram_buffers")));
static XEmacPss_Bd TxBdSpace[TXBD_CNT] __attribute__ ((section (".bram_buffers")));
static char RxBuffers[RXBD_CNT * XEMACPSS_RX_BUF_SIZE] __attribute__ ((section (".bram_buffers")));
static uchar data_buffer[XEMACPSS_RX_BUF_SIZE] __attribute__ ((section (".bram_buffers")));
#else
static XEmacPss_Bd RxBdSpace[RXBD_CNT];
static XEmacPss_Bd TxBdSpace[TXBD_CNT];
static char RxBuffers[RXBD_CNT * XEMACPSS_RX_BUF_SIZE];
static uchar data_buffer[XEMACPSS_RX_BUF_SIZE];
#endif

static struct {
	u8 initialized;
} ethstate = {0};

XEmacPss EmacPssInstance;

/*****************************************************************************/
/*
*	Following are the supporting functions to read and write GEM PHY registers.
*/
static int Xgmac_phy_mgmt_idle(XEmacPss * EmacPssInstancePtr)
{
	return ((XEmacPss_ReadReg
		 (EmacPssInstancePtr->Config.BaseAddress, XEMACPSS_NWSR_OFFSET)
		 & XEMACPSS_NWSR_MDIOIDLE_MASK) == XEMACPSS_NWSR_MDIOIDLE_MASK);
}

#if defined(CONFIG_CMD_MII) && !defined(CONFIG_BITBANGMII)
static int Xgmac_mii_read(const char *devname, unsigned char addr,
		unsigned char reg, unsigned short *value)
{
	phy_spinwait(&EmacPssInstance);
	XEmacPss_PhyRead(&EmacPssInstance, addr, reg, value);
	phy_spinwait(&EmacPssInstance);
	return 0;
}

static int Xgmac_mii_write(const char *devname, unsigned char addr,
		unsigned char reg, unsigned short value)
{
	phy_spinwait(&EmacPssInstance);
	XEmacPss_PhyWrite(&EmacPssInstance, addr, reg, value);
	phy_spinwait(&EmacPssInstance);
	return 0;
}
#endif

static u32 phy_rd_with_addr(XEmacPss * e, u32 a, u32 phy_addr)
{
	u16 PhyData;

	phy_spinwait(e);
	XEmacPss_PhyRead(e, phy_addr, a, &PhyData);
	phy_spinwait(e);
	return PhyData;
}

static void phy_wr_with_addr(XEmacPss * e, u32 a, u32 v, u32 phy_addr)
{
	phy_spinwait(e);
	XEmacPss_PhyWrite(e, phy_addr, a, v);
	phy_spinwait(e);
}

static u32 phy_rd(XEmacPss * e, u32 a)
{
	u16 PhyData;
	
	phy_spinwait(e);
	XEmacPss_PhyRead(e, g_phyaddr, a, &PhyData);
	phy_spinwait(e);
	return PhyData;
}

static void phy_wr(XEmacPss * e, u32 a, u32 v)
{
	phy_spinwait(e);
	XEmacPss_PhyWrite(e, g_phyaddr, a, v);
	phy_spinwait(e);
}

static void phy_rst(XEmacPss * e)
{
	int tmp;
	int count=0;

	puts("Resetting PHY...\n");
	tmp = phy_rd(e, 0);
	tmp |= 0x8000;
	phy_wr(e, 0, tmp);

	while (phy_rd(e, 0) & 0x8000) {
		udelay(10000);
		//tmp++;
		count++;
		if (count > 1000) { /* stalled if reset unfinished after 10 seconds */
			puts("***Error: Reset stalled...\n");
			return;
		}
	}
	puts("\nPHY reset complete.\n");
}

static void Out32(u32 OutAddress, u32 Value)
{
	*(volatile u32 *) OutAddress = Value;
	dmb();
}

/*****************************************************************************/

int Xgmac_one_time_init(void)
{
	int tmp;
	int Status;
	XEmacPss_Config *Config;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;
	XEmacPss_Bd BdTemplate;

	Config = XEmacPss_LookupConfig(EMACPSS_DEVICE_ID);

	Status =
	    XEmacPss_CfgInitialize(EmacPssInstancePtr, Config,
				   Config->BaseAddress);
	if (Status != 0) {
		puts("Error in initialize");
		return 0;
	}

	/*
	 * Setup RxBD space.
	 */

	if (Xgmac_init_rxq(EmacPssInstancePtr, &RxBdSpace, RXBD_CNT)) {
		puts("Xgmac_init_rxq failed!\n");
		return -1;
	}

	/*
	 * Create the RxBD ring
	 */
	tmp =
	    Xgmac_make_rxbuff_mem(EmacPssInstancePtr, &RxBuffers,
				  sizeof(RxBuffers));
	if (tmp == 0 || tmp == -1) {
		printf("Xgmac_make_rxbuff_mem failed! (%i)\n", tmp);
		return -1;
	}

	/*
	 * Setup TxBD space.
	 */

	XEmacPss_BdClear(&BdTemplate);
	XEmacPss_BdSetStatus(&BdTemplate, XEMACPSS_TXBUF_USED_MASK);

	/*
	 * Create the TxBD ring
	 */
	Status =
	    XEmacPss_BdRingCreate(&(XEmacPss_GetTxRing(EmacPssInstancePtr)),
				  (u32) & TxBdSpace, (u32) & TxBdSpace,
				  XEMACPSS_BD_ALIGNMENT, TXBD_CNT);
	if (Status != 0) {
		puts("Error setting up TxBD space, BdRingCreate");
		return -1;
	}

	Status = XEmacPss_BdRingClone(&(XEmacPss_GetTxRing(EmacPssInstancePtr)),
				      &BdTemplate, XEMACPSS_SEND);
	if (Status != 0) {
		puts("Error setting up TxBD space, BdRingClone");
		return -1;
	}
	
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_TXQBASE_OFFSET,
			  EmacPssInstancePtr->TxBdRing.BaseBdAddr);

	/*************************** MAC Setup ***************************/
	tmp = (3 << 18);	/* MDC clock division (48 for up to 120MHz) */
	tmp |= (1 << 17);	/* set for FCS removal */
	tmp |= (1 << 10);	/* enable gigabit */
	tmp |= (1 << 4);	/* copy all frames */
	tmp |= (1 << 1);	/* enable full duplex */

	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET, tmp);
    
	/* MDIO enable */
	tmp =
	    XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			     XEMACPSS_NWCTRL_OFFSET);
	tmp |= XEMACPSS_NWCTRL_MDEN_MASK;
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCTRL_OFFSET, tmp);

	return 0;
}


static int find_phy(XEmacPss *EmacPssInstancePtr)
{
	int phy_addr = 1;//0;
	u16 ctrl, oldctrl;
		
	do {
		
		printf("trying phy addr %d\n", phy_addr);
		ctrl = phy_rd_with_addr(EmacPssInstancePtr, MII_BMCR, phy_addr);
		oldctrl = ctrl & BMCR_ANENABLE;
		
		ctrl ^= BMCR_ANENABLE;
				
		phy_wr_with_addr(EmacPssInstancePtr, MII_BMCR, ctrl, phy_addr);
		
		ctrl = phy_rd_with_addr(EmacPssInstancePtr, MII_BMCR, phy_addr);
		ctrl &= BMCR_ANENABLE;
	
		if (ctrl == oldctrl) {
			phy_addr++;
		} else {
			ctrl ^= BMCR_ANENABLE;
			phy_wr_with_addr(EmacPssInstancePtr, MII_BMCR, ctrl, phy_addr);		
			printf("found phy addr:%d\n", phy_addr);
			return phy_addr;
		}
	} while (phy_addr < 32);
	
	return -1;
}


#define EMAC0_BASE_ADDRESS				0xE000B000
#define EMAC1_BASE_ADDRESS				0xE000C000

/*
 * SLCR setting
 */

#define XPS_SYS_CTRL_BASEADDR		0xF8000000	/* AKA SLCR */
#define SLCR_LOCK_ADDR			(XPS_SYS_CTRL_BASEADDR + 0x4)
#define SLCR_UNLOCK_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x8)
#define SLCR_GEM0_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x140)
#define SLCR_GEM1_CLK_CTRL_ADDR		(XPS_SYS_CTRL_BASEADDR + 0x144)
#define SLCR_LOCK_KEY_VALUE 			0x767B
#define SLCR_UNLOCK_KEY_VALUE			0xDF0D
#define EMACPS_SLCR_DIV_MASK			0xFC0FC0FF


static void SetUpSLCRDivisors(int mac_baseaddr, int speed)
{
	volatile u32 slcrBaseAddress;
#ifndef PEEP
	u32 SlcrDiv0;
	u32 SlcrDiv1;
	u32 SlcrTxClkCntrl;
#endif

	*(volatile unsigned int *)(SLCR_UNLOCK_ADDR) = SLCR_UNLOCK_KEY_VALUE;
        
	if (mac_baseaddr == EMAC0_BASE_ADDRESS) {
		slcrBaseAddress = SLCR_GEM0_CLK_CTRL_ADDR;
	} else {
		slcrBaseAddress = SLCR_GEM1_CLK_CTRL_ADDR;
	}
#ifdef PEEP
	if (speed == 1000) {
		*(volatile unsigned int *)(slcrBaseAddress) =
											SLCR_GEM_1G_CLK_CTRL_VALUE;
	} else if (speed == 100) {
		*(volatile unsigned int *)(slcrBaseAddress) =
											SLCR_GEM_100M_CLK_CTRL_VALUE;
	} else {
		*(volatile unsigned int *)(slcrBaseAddress) =
											SLCR_GEM_10M_CLK_CTRL_VALUE;
	}
#else
	if (speed == 1000) {
		if (mac_baseaddr == EMAC0_BASE_ADDRESS) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0
			SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV0;
			SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_1000MBPS_DIV1;
#endif
		} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0
			SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV0;
			SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_1000MBPS_DIV1;
#endif
		}
	} else if (speed == 100) {
		if (mac_baseaddr == EMAC0_BASE_ADDRESS) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0
			SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV0;
			SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_100MBPS_DIV1;
#endif
		} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0
			SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV0;
			SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_100MBPS_DIV1;
#endif
		}
	} else {
		if (mac_baseaddr == EMAC0_BASE_ADDRESS) {
#ifdef XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0
			SlcrDiv0 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV0;
			SlcrDiv1 = XPAR_PS7_ETHERNET_0_ENET_SLCR_10MBPS_DIV1;
#endif
		} else {
#ifdef XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0
			SlcrDiv0 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV0;
			SlcrDiv1 = XPAR_PS7_ETHERNET_1_ENET_SLCR_10MBPS_DIV1;
#endif
		}
	}
	SlcrTxClkCntrl = *(volatile unsigned int *)(slcrBaseAddress);
	SlcrTxClkCntrl &= EMACPS_SLCR_DIV_MASK;
	SlcrTxClkCntrl |= (SlcrDiv1 << 20);
	SlcrTxClkCntrl |= (SlcrDiv0 << 8);
	*(volatile unsigned int *)(slcrBaseAddress) = SlcrTxClkCntrl;
#endif
	*(volatile unsigned int *)(SLCR_LOCK_ADDR) = SLCR_LOCK_KEY_VALUE;
	return;
}


int Xgmac_init(struct eth_device *dev, bd_t * bis)
{
	int tmp;
	int link_speed;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;
	
	if (ethstate.initialized)
		return 1;
    
	/*
	 * Setup the ethernet.
	 */
	printf("Trying to set up GEM link...\n");
    
	/* Configure DMA */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_DMACR_OFFSET, 0x00180704);
    
	/* Disable all the MAC Interrupts */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_IDR_OFFSET, 0xFFFFFFFF);
    
	/* Rx and Tx enable */
	tmp =
	    XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			     XEMACPSS_NWCTRL_OFFSET);
	tmp |= XEMACPSS_NWCTRL_RXEN_MASK | XEMACPSS_NWCTRL_TXEN_MASK;
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCTRL_OFFSET, tmp);

    
	{
		/*************************** PHY Setup ***************************/
		/* ---------------------------------------------------------------------*/
		/* add by starsoc  */
		/* --------------------------------------------- */
		int phy_addr;
        u16 phyCtrlRegVal        = 0;
        u16 phyExtnalCtrlRegVal  = 0;
		phy_addr = find_phy(EmacPssInstancePtr);
		g_phyaddr = phy_addr;
        u32 tmp_status = 0;
	    unsigned long convspeeddupsetting = 0;
                
		printf("Reset PHY \r\n");
		tmp = phy_rd(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1);
		tmp =	tmp & PHY_SHADOW_MASK;
		tmp =	tmp | PHY_MII_SHADOW_WEN_MASK;
		tmp =	tmp | PHY_MII_SHADOW_ADDR;
		phy_wr(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1, tmp);
            
            
        
        phyCtrlRegVal =  phyCtrlRegVal | PHY_MII_RSTEN_MASK ;
        phy_wr(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1, phyCtrlRegVal);
        phyCtrlRegVal =  phyCtrlRegVal & (~PHY_MII_SHADOW_WEN_MASK) ;
        phy_wr(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1, phyCtrlRegVal);
		tmp = phy_rd(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1);


		//PHY
		phyCtrlRegVal = phy_rd(EmacPssInstancePtr, PHY_CRTL_REG);
		phyCtrlRegVal = phyCtrlRegVal | PHY_MII_RESET_MASK;
        phy_wr(EmacPssInstancePtr, PHY_CRTL_REG, phyCtrlRegVal);
		phyCtrlRegVal = phy_rd(EmacPssInstancePtr, PHY_CRTL_REG);

        
        
		sleep(1);

		phyCtrlRegVal =  phyCtrlRegVal & (~PHY_MII_RESET_MASK);        
        phy_wr(EmacPssInstancePtr, PHY_CRTL_REG, phyCtrlRegVal);
		printf("Reset PHY Successful \r\n");
        
		/*
		 * Config MII MODE
		 */
		printf("Config RGMII 1.8V HSTL MODE \r\n");
        
		phyExtnalCtrlRegVal = phy_rd(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1);
		phyExtnalCtrlRegVal =	phyExtnalCtrlRegVal & PHY_SHADOW_MASK;
		phyExtnalCtrlRegVal =	phyExtnalCtrlRegVal | PHY_MII_SHADOW_WEN_MASK;
		phyExtnalCtrlRegVal =	phyExtnalCtrlRegVal | PHY_MII_SHADOW_ADDR;
        phy_wr(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1, phyExtnalCtrlRegVal);
        
        
		//	RGMII 1.8V HSTL??┷：o?：o1?：1
		phyCtrlRegVal =  phyCtrlRegVal | PHY_MII_MODE_MASK ;
        phy_wr(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1, phyCtrlRegVal);        
		phyCtrlRegVal =  phyCtrlRegVal & (~PHY_MII_SHADOW_WEN_MASK) ;
        phy_wr(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1, phyCtrlRegVal);                
		phyExtnalCtrlRegVal = phy_rd(EmacPssInstancePtr, PHY_EXTNAL_CRTL_REG1);
		printf("Config RGMII 1.8V HSTL MODE Successful \r\n");

		printf("Start PHY auto negotiation \r\n");
		do{

            tmp_status = phy_rd(EmacPssInstancePtr, PHY_Status_REG);
		}while(tmp_status&PHY_AutoNegDone_MASK);
        
	    printf("PHY auto negotiation Done \r\n");

        
		printf("Waiting for LinkUp \r\n");
		do{            
            tmp_status = phy_rd(EmacPssInstancePtr, PHY_Status_REG);
		}while(tmp_status&PHY_LinkUp_MASK);
		printf("LinkUp Successful \r\n");

        tmp_status = phy_rd(EmacPssInstancePtr, PHY_Status_EXREG);
		if(tmp_status&PHY_1000M_FULL_MASK)
		{
			printf("XEmacPsDetectPHY():  Speed = 1000Mbps\r\n");
            SetUpSLCRDivisors(EmacPssInstancePtr->Config.BaseAddress, 1000);
            convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED1000_FD;
            link_speed = 1000;
		}
        else
		{
            tmp_status = phy_rd(EmacPssInstancePtr, PHY_Status_REG);
			if(tmp_status&PHY_100M_FULL_MASK)
			{
				printf("XEmacPsDetectPHY():  Speed = 100Mbps \r\n");                
                SetUpSLCRDivisors(EmacPssInstancePtr->Config.BaseAddress, 100);
                convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED100_FD;
                link_speed = 100;
			}
			else if(tmp_status&PHY_10M_FULL_MASK)
			{
                SetUpSLCRDivisors(EmacPssInstancePtr->Config.BaseAddress, 10);
				printf("XEmacPsDetectPHY():  Speed = 10Mbps \r\n");
                convspeeddupsetting = XEMACPS_GMII2RGMII_SPEED10_FD;
                link_speed = 10;
			}			
		}	
        
        phy_wr(EmacPssInstancePtr, XEMACPS_GMII2RGMII_REG_NUM, convspeeddupsetting);  
        
        printf("link speed: %d\r\n", link_speed);
        
        #if 0
		/* --------------------------------------------- */
		
		phy_wr(EmacPssInstancePtr, MII_MARVELL_PHY_PAGE, 0);	/* page 0 */
		
		tmp = phy_rd(EmacPssInstancePtr, MII_PHYSID1);
		printf("Phy ID: %04X", tmp);
		tmp = phy_rd(EmacPssInstancePtr, MII_PHYSID2);
		printf("%04X\n", tmp);
		
		
		/* Auto-negotiation advertisement register */
		tmp = phy_rd(EmacPssInstancePtr, MII_ADVERTISE);
		tmp |= (1 << 11);	/* asymmetric pause */
		tmp |= (1 << 10);	/* MAC pause implemented */
		phy_wr(EmacPssInstancePtr, MII_ADVERTISE, tmp);
		
		/* Copper specific control register 1 */
		tmp = phy_rd(EmacPssInstancePtr, MII_M1011_PHY_SCR);
		tmp |= (7 << 12);	/* max number of gigabit attempts */
		tmp |= (1 << 11);	/* enable downshift */
		phy_wr(EmacPssInstancePtr, MII_M1011_PHY_SCR, tmp);

		
		/* Control register - MAC */
		phy_wr(EmacPssInstancePtr, MII_MARVELL_PHY_PAGE, 2);	/* page 2 */
		tmp = phy_rd(EmacPssInstancePtr, MII_M1116R_CONTROL_REG_MAC);
		tmp |= (1 << 5);	/* RGMII receive timing transition when data stable */
		tmp |= (1 << 4);	/* RGMII transmit clock internally delayed */
		phy_wr(EmacPssInstancePtr, MII_M1116R_CONTROL_REG_MAC, tmp);
		phy_wr(EmacPssInstancePtr, MII_MARVELL_PHY_PAGE, 0);	/* page 0 */
		
		/* Control register */
		tmp = phy_rd(EmacPssInstancePtr, MII_BMCR);
		tmp |= (1 << 12);	/* auto-negotiation enable */
		tmp |= (1 << 8);	/* enable full duplex */
		phy_wr(EmacPssInstancePtr, MII_BMCR, tmp);
        #endif
	}
	/* ---------------------------------------------------------------------*/
	
	
#if 1

	/***** Try to establish a link at the highest speed possible  *****/
#ifdef CONFIG_EP107
	/* CR-659040:
	 * Advertise link speed as 100Mbps for ep107 targets
	 */
	Xgmac_set_eth_advertise(EmacPssInstancePtr, 100);
#else
	/* CR-659040 */
	/* Could be 1000 if an unknown bug is fixed */
	Xgmac_set_eth_advertise(EmacPssInstancePtr, 1000);
#endif
    
	phy_rst(EmacPssInstancePtr);
	
	/* Attempt auto-negotiation */
	puts("Waiting for PHY to complete auto-negotiation...\n");
	tmp = 0; /* delay counter */
	while (!(phy_rd(EmacPssInstancePtr, 1) & (1 << 5))) {
		udelay(10000);
		tmp++;
		if (tmp > 1000) { /* stalled if no link after 10 seconds */
			puts("***Error: Auto-negotiation stalled...\n");
			return -1;
		}
	}
	
	/* Check if the link is up */
	tmp = phy_rd(EmacPssInstancePtr, 17);
	if (  ((tmp >> 10) & 1) ) {
		/* Check for an auto-negotiation error */
		tmp = phy_rd(EmacPssInstancePtr, 19);
		if ( (tmp >> 15) & 1 ) {
			puts("***Error: Auto-negotiation error is present.\n");
			return -1;
		}
	} else {
		puts("***Error: Link is not up.\n");
		return -1;
	}
	
	/********************** Determine link speed **********************/
	tmp = phy_rd(EmacPssInstancePtr, 17);
	if ( ((tmp >> 14) & 3) == 2)		/* 1000Mbps */
		link_speed = 1000;
	else if ( ((tmp >> 14) & 3) == 1)	/* 100Mbps */
		link_speed = 100;
	else					/* 10Mbps */
		link_speed = 10;
 #endif
    
 
	/*************************** MAC Setup ***************************/
	tmp = XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET);
	if (link_speed == 10)
		tmp &= ~(0x1);		/* enable 10Mbps */
	else
		tmp |= 0x1;		/* enable 100Mbps */
	if (link_speed == 1000)
		tmp |= 0x400;		/* enable 1000Mbps */
	else
		tmp &= ~(0x400);	/* disable gigabit */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET, tmp);

	/************************* GEM0_CLK Setup *************************/
	/* SLCR unlock */
	Out32(0xF8000008, 0xDF0D);

	/* Configure GEM0_RCLK_CTRL */
	Out32(0xF8000138, ((0 << 4) | (1 << 0)));

	/* Set divisors for appropriate frequency in GEM0_CLK_CTRL */
#ifdef CONFIG_EP107
	if (link_speed == 1000)		/* 125MHz */
		Out32(0xF8000140, ((1 << 20) | (48 << 8) | (1 << 4) | (1 << 0)));
	else if (link_speed == 100)	/* 25 MHz */
		Out32(0xF8000140, ((1 << 20) | (48 << 8) | (0 << 4) | (1 << 0)));
	else				/* 2.5 MHz */
		Out32(0xF8000140, ((1 << 20) | (48 << 8) | (3 << 4) | (1 << 0)));
#else
	if (link_speed == 1000)		/* 125MHz */
		Out32(0xF8000140, ((1 << 20) | (8 << 8) | (0 << 4) | (1 << 0)));
	else if (link_speed == 100)	/* 25 MHz */
		Out32(0xF8000140, ((1 << 20) | (40 << 8) | (0 << 4) | (1 << 0)));
	else				/* 2.5 MHz */
		Out32(0xF8000140, ((10 << 20) | (40 << 8) | (0 << 4) | (1 << 0)));
#endif

	/* SLCR lock */
	Out32(0xF8000004, 0x767B);

	printf("Link is now at %dMbps!\n", link_speed);

	ethstate.initialized = 1;
	return 0;
}

void Xgmac_halt(struct eth_device *dev)
{
	return;
}

int Xgmac_send(struct eth_device *dev, volatile void *packet, int length)
{
	volatile int Status;
	XEmacPss_Bd *BdPtr;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;

	if (!ethstate.initialized) 
    {
		puts("Error GMAC not initialized");
		return 0;
	}
	Status =
	    XEmacPss_BdRingAlloc(&(XEmacPss_GetTxRing(&EmacPssInstance)), 1,
				 &BdPtr);
	if (Status != 0) {
		puts("Error allocating TxBD");
		return 0;
	}
    
	/*
	 * Setup TxBD
	 */
	XEmacPss_BdSetAddressTx(BdPtr, (u32)packet);
	XEmacPss_BdSetLength(BdPtr, length);
	XEmacPss_BdClearTxUsed(BdPtr);
	XEmacPss_BdSetLast(BdPtr);

	/*
	 * Enqueue to HW
	 */
	Status =
	    XEmacPss_BdRingToHw(&(XEmacPss_GetTxRing(&EmacPssInstance)), 1,
				BdPtr);
	if (Status != 0) {
		puts("Error committing TxBD to HW");
		return 0;
	}
    
	/* Start transmit */
	XEmacPss_Transmit(EmacPssInstancePtr);

	/* Read the status register to know if the packet has been Transmitted. */
	Status =
	    XEmacPss_ReadReg(EmacPssInstance.Config.BaseAddress,
			     XEMACPSS_TXSR_OFFSET);
	if (Status &
	    (XEMACPSS_TXSR_HRESPNOK_MASK | XEMACPSS_TXSR_URUN_MASK |
	     XEMACPSS_TXSR_BUFEXH_MASK)) {
		printf("Something has gone wrong here!? Status is 0x%x.\n",
		       Status);
	}
        
	if (Status & XEMACPSS_TXSR_TXCOMPL_MASK) {
        /* add by starsoc */
        
		/*
		 * Now that the frame has been sent, post process our TxBDs.
		 */
		if (XEmacPss_BdRingFromHwTx
		    (&(XEmacPss_GetTxRing(&EmacPssInstance)), 1, &BdPtr) == 0) {
			puts("TxBDs were not ready for post processing");
			return 0;
		}
        
		/*
		 * Free the TxBD.
		 */
		Status =
		    XEmacPss_BdRingFree(&(XEmacPss_GetTxRing(&EmacPssInstance)),
					1, BdPtr);
		if (Status != 0) {
			puts("Error freeing up TxBDs");
			return 0;
		}
	}
	/* Clear Tx status register before leaving . */
	XEmacPss_WriteReg(EmacPssInstance.Config.BaseAddress,
			  XEMACPSS_TXSR_OFFSET, Status);
	return 1;

}

int Xgmac_rx(struct eth_device *dev)
{
	u32 status, retval;
	XEmacPss *EmacPssInstancePtr = &EmacPssInstance;

	status =
	    XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			     XEMACPSS_RXSR_OFFSET);
	if (status & XEMACPSS_RXSR_FRAMERX_MASK) {

//		printf("rx packet received\n");
	
		do {
			retval = Xgmac_process_rx(EmacPssInstancePtr);
		} while (retval == 0) ;
	}

	/* Clear interrupt status.
	 */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
	                  XEMACPSS_RXSR_OFFSET, status);
	
	return 1;
}

static int Xgmac_write_hwaddr(struct eth_device *dev)
{
	/* Initialize the first MAC filter with our address */
	XEmacPss_SetMacAddress((XEmacPss *)dev->priv, dev->enetaddr, 1);

	return 0;
}

int zynq_gem_initialize(bd_t *bis)
{
	struct eth_device *dev;
	dev = malloc(sizeof(*dev));
	if (dev == NULL)
		return 1;
	
	memset(dev, 0, sizeof(*dev));
	sprintf(dev->name, "zynq_gem");

	if (Xgmac_one_time_init() < 0) {
		printf("zynq_gem init failed!");
		return -1;
	}
	dev->iobase = EmacPssInstance.Config.BaseAddress;
	dev->priv = &EmacPssInstance;
	dev->init = Xgmac_init;
	dev->halt = Xgmac_halt;
	dev->send = Xgmac_send;
	dev->recv = Xgmac_rx;
	dev->write_hwaddr = Xgmac_write_hwaddr;
    
	eth_register(dev);

#if defined(CONFIG_CMD_MII) && !defined(CONFIG_BITBANGMII)
	miiphy_register(dev->name, Xgmac_mii_read, Xgmac_mii_write);
#endif
	return 0;
}

/*=============================================================================
 *
 * Xgmac_process_rx- process the next incoming packet
 *
 * return's 0 if OK, -1 on error
 */
int Xgmac_process_rx(XEmacPss * EmacPssInstancePtr)
{
	uchar *buffer = data_buffer;
	u32 rx_status, hwbuf;
	int frame_len;
	u32 *bd_addr;

    bd_addr = (u32 *) & EmacPssInstancePtr->RxBdRing.
	    RxBD_start[EmacPssInstancePtr->RxBdRing.RxBD_current];

	rx_status = XEmacPss_BdRead((bd_addr), XEMACPSS_BD_ADDR_OFFSET);
	if (! (rx_status & XEMACPSS_RXBUF_NEW_MASK)) {
		return (-1);
	}
    
	rx_status = XEmacPss_BdIsRxSOF(bd_addr);
	if (!rx_status) {
		printf("GEM: SOF not set for last buffer received!\n");
		return (-1);
	}
	rx_status = XEmacPss_BdIsRxEOF(bd_addr);
	if (!rx_status) {
		printf("GEM: EOF not set for last buffer received!\n");
		return (-1);
	}

	frame_len = XEmacPss_BdGetLength(bd_addr);
	if (frame_len == 0) {
		printf("GEM: Hardware reported 0 length frame!\n");
		return (-1);
	}

	hwbuf = (u32) (*bd_addr & XEMACPSS_RXBUF_ADD_MASK);
	if (hwbuf == (u32) NULL) {
		printf("GEM: Error swapping out buffer!\n");
		return (-1);
	}
	memcpy(buffer, (void *)hwbuf, frame_len);
	Xgmac_next_rx_buf(EmacPssInstancePtr);
	NetReceive(buffer, frame_len);

	return (0);
}

int Xgmac_init_rxq(XEmacPss * EmacPssInstancePtr, void *bd_start, int num_elem)
{
	XEmacPss_BdRing *r;
	int loop = 0;

	if ((num_elem <= 0) || (num_elem > RXBD_CNT)) {
		return (-1);
	}

	for (; loop < 2 * (num_elem);) {
		*(((u32 *) bd_start) + loop) = 0x00000000;
		*(((u32 *) bd_start) + loop + 1) = 0xF0000000;
		loop += 2;
	}

	r = & EmacPssInstancePtr->RxBdRing;
	r->RxBD_start = (XEmacPss_Bd *) bd_start;
	r->Length = num_elem;
	r->RxBD_current = 0;
	r->RxBD_end = 0;
	r->Rx_first_buf = 0;

	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_RXQBASE_OFFSET, (u32) bd_start);

	return 0;
}

int Xgmac_make_rxbuff_mem(XEmacPss * EmacPssInstancePtr, void *rx_buf_start,
			  u32 rx_buffsize)
{
	XEmacPss_BdRing *r;
	int num_bufs;
	int assigned_bufs;
	int i;
	u32 *bd_addr;

	if ((EmacPssInstancePtr == NULL) || (rx_buf_start == NULL)) {
		return (-1);
	}

	r = & EmacPssInstancePtr->RxBdRing;

	assigned_bufs = 0;

	if ((num_bufs = rx_buffsize / XEMACPSS_RX_BUF_SIZE) == 0) {
		return 0;
	}
	for (i = 0; i < num_bufs; i++) {
		if (r->RxBD_end < r->Length) {
			memset((char *)(rx_buf_start +
					(i * XEMACPSS_RX_BUF_SIZE)), 0, XEMACPSS_RX_BUF_SIZE);

			bd_addr = (u32 *) & r->RxBD_start[r->RxBD_end];

			XEmacPss_BdSetAddressRx(bd_addr,
						(u32) (((char *)
							rx_buf_start) + (i * XEMACPSS_RX_BUF_SIZE)));

			r->RxBD_end++;
			assigned_bufs++;
		} else {
			return assigned_bufs;
		}
	}
	bd_addr = (u32 *) & r->RxBD_start[r->RxBD_end - 1];
	XEmacPss_BdSetRxWrap(bd_addr);

	return assigned_bufs;
}

int Xgmac_next_rx_buf(XEmacPss * EmacPssInstancePtr)
{
	XEmacPss_BdRing *r;
	u32 prev_stat = 0;
	u32 *bd_addr = NULL;

	if (EmacPssInstancePtr == NULL) {
		printf
		    ("\ngem_clr_rx_buf with EmacPssInstancePtr as !!NULL!! \n");
		return -1;
	}

	r = & EmacPssInstancePtr->RxBdRing;

	bd_addr = (u32 *) & r->RxBD_start[r->RxBD_current];
	prev_stat = XEmacPss_BdIsRxSOF(bd_addr);
	if (prev_stat) {
		r->Rx_first_buf = r->RxBD_current;
	} else {
		XEmacPss_BdClearRxNew(bd_addr);
		XIo_Out32((u32) (bd_addr + 1), 0xF0000000);
	}

	if (XEmacPss_BdIsRxEOF(bd_addr)) {
		bd_addr = (u32 *) & r->RxBD_start[r->Rx_first_buf];
		XEmacPss_BdClearRxNew(bd_addr);
		XIo_Out32((u32) (bd_addr + 1), 0xF0000000);
	}

	if ((++r->RxBD_current) > r->Length - 1) {
		r->RxBD_current = 0;
	}

	return 0;
}

void Xgmac_set_eth_advertise(XEmacPss *EmacPssInstancePtr, int link_speed)
{
	int tmp;

	/* MAC setup */
	tmp = XEmacPss_ReadReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET);
	if (link_speed == 10)
		tmp &= ~(1 << 0);	/* enable 10Mbps */
	else if (link_speed == 100)
		tmp |= (1 << 0);	/* enable 100Mbps */
	XEmacPss_WriteReg(EmacPssInstancePtr->Config.BaseAddress,
			  XEMACPSS_NWCFG_OFFSET, tmp);

	phy_wr(EmacPssInstancePtr, 22, 0);	/* page 0 */

	/* Auto-negotiation advertisement register */
	tmp = phy_rd(EmacPssInstancePtr, 4);
	if (link_speed >= 100) {
		tmp |= (1 << 8);	/* advertise 100Mbps F */
		tmp |= (1 << 7);	/* advertise 100Mbps H */
	} else {
		tmp &= ~(1 << 8);	/* advertise 100Mbps F */
		tmp &= ~(1 << 7);	/* advertise 100Mbps H */
	}
	if (link_speed >= 10) {
		tmp |= (1 << 6);	/* advertise 10Mbps F */
		tmp |= (1 << 5);	/* advertise 10Mbps H */
	} else {
		tmp &= ~(1 << 6);	/* advertise 10Mbps F */
		tmp &= ~(1 << 5);	/* advertise 10Mbps H */
	}
	phy_wr(EmacPssInstancePtr, 4, tmp);

	/* 1000BASE-T control register */
	tmp = phy_rd(EmacPssInstancePtr, 9);
	if (link_speed == 1000) {
		tmp |= (1 << 9);	/* advertise 1000Mbps F */
		tmp |= (1 << 8);	/* advertise 1000Mbps H */
	} else {
		tmp &= ~(1 << 9);	/* advertise 1000Mbps F */
		tmp &= ~(1 << 8);	/* advertise 1000Mbps H */
	}
	phy_wr(EmacPssInstancePtr, 9, tmp);

}

