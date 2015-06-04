#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libzynq.o

# add by starsoc
COBJS-y += common/xdevcfg.o
COBJS-y += common/xdevcfg_g.o
COBJS-y += common/xdevcfg_intr.o
COBJS-y += common/xdevcfg_sinit.o
COBJS-y += common/pcap.o
COBJS-y += common/xil_io.o
COBJS-y += common/xil_assert.o
COBJS-y += common/outbyte.o
COBJS-y += common/xuartps_hw.o
COBJS-y += common/xil_cache.o
COBJS-y += common/xil_exception.o
COBJS-y += common/xgpio.o
COBJS-y += common/xgpio_extra.o
COBJS-y += common/xgpio_g.o
COBJS-y += common/xgpio_intr.o
COBJS-y += common/xgpio_selftest.o
COBJS-y += common/xgpio_sinit.o
COBJS-y += common/sleep.o
COBJS-y += common/usleep.o
COBJS-y += common/xtime_l.o

COBJS-y += common/xiicps.o
COBJS-y += common/xiicps_g.o
COBJS-y += common/xiicps_hw.o
COBJS-y += common/xiicps_intr.o
COBJS-y += common/xiicps_sinit.o
COBJS-y += common/xiicps_options.o
COBJS-y += common/xiicps_master.o
COBJS-y += common/xiic_l.o
COBJS-y += common/zynq_i2c.o


COBJS-y += ps_memory_test/xil_testmem.o

COBJS-y += ps_i2c_eeprom_test/xiicps_eeprom_polled_example.o
COBJS-y += ps_i2c_test/xiicps_polled_master_example.o
COBJS-y += ps_i2c_test/xiicps_rtc.o
COBJS-y += ps_i2c_test/xiicps_selftest.o

COBJS-y += ps_usb_test/xusbps_sinit.o
COBJS-y += ps_usb_test/xusbps_intr.o
COBJS-y += ps_usb_test/xusbps_hw.o
COBJS-y += ps_usb_test/xusbps_g.o
COBJS-y += ps_usb_test/xusbps_endpoint.o
COBJS-y += ps_usb_test/xusbps.o
COBJS-y += ps_usb_test/zynq_usb.o

#COBJS-y += ps_qspi_test/xqspips_selftest_example.o
#COBJS-y += ps_qspi_test/xqspips_sinit.o

COBJS-y += ps_qspi_test/xqspips.o
#COBJS-y += ps_qspi_test/xqspips_g.o
COBJS-y += ps_qspi_test/xqspips_hw.o
COBJS-y += ps_qspi_test/xqspips_options.o
COBJS-y += ps_qspi_test/xqspips_selftest.o



COBJS-y += ps_gpio_test/xgpiops.o
COBJS-y += ps_gpio_test/xgpiops_g.o
COBJS-y += ps_gpio_test/xgpiops_hw.o
COBJS-y += ps_gpio_test/xgpiops_intr.o
COBJS-y += ps_gpio_test/xgpiops_sinit.o
COBJS-y += ps_uart_test/zynq_ps_uart.o
COBJS-y += pl_hdmi_test/zynq_pl_hdmi.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
