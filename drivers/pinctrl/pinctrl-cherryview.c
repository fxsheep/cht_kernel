/* gpio-cherryview.c Cherrytrail platform GPIO driver
 *
 * Copyright (c) 2013,  Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>
#include <linux/gpio.h>
#include <linux/acpi.h>
#include <linux/seq_file.h>
#include <linux/io.h>
#include <linux/pnp.h>
#include "pinctrl-cherryview.h"

#define GPIO_PATH_MAX	64

#define FAMILY0_PAD_REGS_OFF	0x4400
#define FAMILY_PAD_REGS_SIZE	0x400
#define MAX_FAMILY_PAD_GPIO_NO	15
#define GPIO_REGS_SIZE		8

#define CV_PADCTRL0_REG		0x000
#define CV_PADCTRL1_REG		0x004
#define CV_INT_STAT_REG		0x300
#define CV_INT_MASK_REG		0x380

#define CV_GPIO_RX_STAT		BIT(0)
#define CV_GPIO_TX_STAT		BIT(1)
#define CV_GPIO_EN		BIT(15)
#define CV_GPIO_PULL		BIT(23)

#define CV_CFG_LOCK_MASK	BIT(31)
#define CV_INT_CFG_MASK		(BIT(0) | BIT(1) | BIT(2))
#define CV_PAD_MODE_MASK	(0xF << 16)

#define CV_GPIO_CFG_MASK	(BIT(8) | BIT(9) | BIT(10))
#define CV_GPIO_TX_EN		(1 << 8)
#define CV_GPIO_RX_EN		(2 << 8)

#define CV_INV_RX_DATA		BIT(6)

#define CV_INT_SEL_MASK		(0xF << 28)
#define CV_PAD_MODE_MASK	(0xF << 16)
#define CV_GPIO_PULL_MODE	(0xF << 20)
#define CV_GPIO_PULL_STRENGTH_MASK	(0x7 << 20)

#define MAX_INTR_LINE_NUM	16

/* When Pad Cfg is locked, driver can only change GPIOTXState or GPIORXState */
#define PAD_CFG_LOCKED(offset)	(chv_readl(chv_gpio_reg(&cg->chip,	\
				offset, CV_PADCTRL1_REG)) & CV_CFG_LOCK_MASK)

enum INTR_CFG {
	CV_INTR_DISABLE,
	CV_TRIG_EDGE_FALLING,
	CV_TRIG_EDGE_RISING,
	CV_TRIG_EDGE_BOTH,
	CV_TRIG_LEVEL,
};

struct gpio_pad_info {
	int family;		/* Family ID */
	int pad;		/* Pad ID in this family */

	/* Interrupt line selected (0~15), -1 if not interruptible. */
	int interrupt_line;

};

struct gpio_bank_pnp {
	char			*name;
	int			gpio_base;
	int			irq_base;
	int			ngpio;
	struct gpio_pad_info	*pads_info;
	struct chv_gpio		 *cg;
};

/* For invalid GPIO number(not found in GPIO list),
 * initialize all fields as -1.
 */
static struct gpio_pad_info north_pads_info[CV_NGPIO_NORTH] = {
	/*0~14*/
	[GPIO_DFX_0]		= { 0,		0,	-1 },
	[GPIO_DFX_3]		= { 0,		1,	-1 },
	[GPIO_DFX_7]		= { 0,		2,	-1 },
	[GPIO_DFX_1]		= { 0,		3,	-1 },
	[GPIO_DFX_5]		= { 0,		4,	-1 },
	[GPIO_DFX_4]		= { 0,		5,	-1 },
	[GPIO_DFX_8]		= { 0,		6,	-1 },
	[GPIO_DFX_2]		= { 0,		7,	-1 },
	[GPIO_DFX_6]		= { 0,		8,	-1 },
	[9]					= { -1,		-1,	-1 },
	[10]				= { -1,		-1,	-1 },
	[11]				= { -1,		-1,	-1 },
	[12]				= { -1,		-1,	-1 },
	[13]				= { -1,		-1,	-1 },
	[14]				= { -1,		-1,	-1 },

	/*15~29*/
	[GPIO_SUS0]			= { 1,		0,	-1 },
	[SEC_GPIO_SUS10]	= { 1,		1,	-1 },
	[GPIO_SUS3]			= { 1,		2,	-1 },
	[GPIO_SUS7]			= { 1,		3,	-1 },
	[GPIO_SUS1]			= { 1,		4,	-1 },
	[GPIO_SUS5]			= { 1,		5,	-1 },
	[SEC_GPIO_SUS11]	= { 1,		6,	-1 },
	[GPIO_SUS4]			= { 1,		7,	-1 },
	[SEC_GPIO_SUS8]		= { 1,		8,	-1 },
	[GPIO_SUS2]			= { 1,		9,	-1 },
	[GPIO_SUS6]			= { 1,		10,	-1 },
	[CX_PREQ_B]			= { 1,		11,	-1 },
	[SEC_GPIO_SUS9]		= { 1,		12,	-1 },
	[28]				= { -1,		-1,	-1 },
	[29]				= { -1,		-1,	-1 },

	/*30~44*/
	[TRST_B]			= { 2,		0,	-1 },
	[TCK]				= { 2,		1,	-1 },
	[PROCHOT_B]			= { 2,		2,	-1 },
	[SVIDO_DATA]		= { 2,		3,	-1 },
	[TMS]				= { 2,		4,	-1 },
	[CX_PRDY_B_2]		= { 2,		5,	-1 },
	[TDO_2]				= { 2,		6,	-1 },
	[CX_PRDY_B]			= { 2,		7,	-1 },
	[SVIDO_ALERT_B]		= { 2,		8,	-1 },
	[TDO]				= { 2,		9,	-1 },
	[SVIDO_CLK]			= { 2,		10,	-1 },
	[TDI]				= { 2,		11,	-1 },
	[42]				= { -1,		-1,	-1 },
	[43]				= { -1,		-1,	-1 },
	[44]				= { -1,		-1,	-1 },

	/*45~59*/
	[GP_CAMERASB_05]	= { 3,		0,	-1 },
	[GP_CAMERASB_02]	= { 3,		1,	-1 },
	[GP_CAMERASB_08]	= { 3,		2,	-1 },
	[GP_CAMERASB_00]	= { 3,		3,	-1 },
	[GP_CAMERASB_06]	= { 3,		4,	-1 },
	[GP_CAMERASB_10]	= { 3,		5,	-1 },
	[GP_CAMERASB_03]	= { 3,		6,	-1 },
	[GP_CAMERASB_09]	= { 3,		7,	-1 },
	[GP_CAMERASB_01]	= { 3,		8,	-1 },
	[GP_CAMERASB_07]	= { 3,		9,	-1 },
	[GP_CAMERASB_11]	= { 3,		10,	-1 },
	[GP_CAMERASB_04]	= { 3,		11,	-1 },
	[57]				= { -1,		-1,	-1 },
	[58]				= { -1,		-1,	-1 },
	[59]				= { -1,		-1,	-1 },

	/*60~72*/
	[PANEL0_BKLTEN]		= { 4,		0,	-1 },
	[HV_DDI0_HPD]		= { 4,		1,	-1 },
	[HV_DDI2_DDC_SDA]	= { 4,		2,	-1 },
	[PANEL1_BKLTCTL]	= { 4,		3,	-1 },
	[HV_DDI1_HPD]		= { 4,		4,	-1 },
	[PANEL0_BKLTCTL]	= { 4,		5,	-1 },
	[HV_DDI0_DDC_SDA]	= { 4,		6,	-1 },
	[HV_DDI2_DDC_SCL]	= { 4,		7,	-1 },
	[HV_DDI2_HPD]		= { 4,		8,	-1 },
	[PANEL1_VDDEN]		= { 4,		9,	-1 },
	[PANEL1_BKLTEN]		= { 4,		10,	-1 },
	[HV_DDI0_DDC_SCL]	= { 4,		11,	-1 },
	[PANEL0_VDDEN]		= { 4,		12,	-1 },
};

static struct gpio_pad_info southeast_pads_info[CV_NGPIO_SOUTHEAST] = {
	/*0~14*/
	[MF_PLT_CLK0]		= { 0,		0,	-1 },
	[PWM1]				= { 0,		1,	-1 },
	[MF_PLT_CLK1]		= { 0,		2,	-1 },
	[MF_PLT_CLK4]		= { 0,		3,	-1 },
	[MF_PLT_CLK3]		= { 0,		4,	-1 },
	[PWM0]				= { 0,		5,	-1 },
	[MF_PLT_CLK5]		= { 0,		6,	-1 },
	[MF_PLT_CLK2]		= { 0,		7,	-1 },
	[9]					= { -1,		-1,	-1 },
	[10]				= { -1,		-1,	-1 },
	[11]				= { -1,		-1,	-1 },
	[12]				= { -1,		-1,	-1 },
	[13]				= { -1,		-1,	-1 },
	[14]				= { -1,		-1,	-1 },

	/*15~29*/
	[SDMMC2_D3_CD_B]	= { 1,		0,	-1 },
	[SDMMC1_CLK]		= { 1,		1,	-1 },
	[SDMMC1_D0]			= { 1,		2,	-1 },
	[SDMMC2_D1]			= { 1,		3,	-1 },
	[SDMMC2_CLK]		= { 1,		4,	-1 },
	[SDMMC1_D2]			= { 1,		5,	-1 },
	[SDMMC2_D2]			= { 1,		6,	-1 },
	[SDMMC2_CMD]		= { 1,		7,	-1 },
	[SDMMC1_CMD]		= { 1,		8,	-1 },
	[SDMMC1_D1]			= { 1,		9,	-1 },
	[SDMMC2_D0]			= { 1,		10,	-1 },
	[SDMMC1_D3_CD_B]	= { 1,		11,	-1 },
	[27]				= { -1,		-1,	-1 },
	[28]				= { -1,		-1,	-1 },
	[29]				= { -1,		-1,	-1 },

	/*30~44*/
	[SDMMC3_D1]			= { 2,		0,	-1 },
	[SDMMC3_CLK]		= { 2,		1,	-1 },
	[SDMMC3_D3]			= { 2,		2,	-1 },
	[SDMMC3_D2]			= { 2,		3,	-1 },
	[SDMMC3_CMD]		= { 2,		4,	-1 },
	[SDMMC3_D0]			= { 2,		5,	-1 },
	[36]				= { -1,		-1,	-1 },
	[37]				= { -1,		-1,	-1 },
	[38]				= { -1,		-1,	-1 },
	[39]				= { -1,		-1,	-1 },
	[40]				= { -1,		-1,	-1 },
	[41]				= { -1,		-1,	-1 },
	[42]				= { -1,		-1,	-1 },
	[43]				= { -1,		-1,	-1 },
	[44]				= { -1,		-1,	-1 },

	/*45~59*/
	[MF_LPC_AD2]		= { 3,		0,	-1 },
	[LPC_CLKRUNB]		= { 3,		1,	-1 },
	[MF_LPC_AD0]		= { 3,		2,	-1 },
	[LPC_FRAMEB]		= { 3,		3,	-1 },
	[MF_LPC_CLKOUT1]	= { 3,		4,	-1 },
	[MF_LPC_AD3]		= { 3,		5,	-1 },
	[MF_LPC_CLKOUT0]	= { 3,		6,	-1 },
	[MF_LPC_AD1]		= { 3,		7,	-1 },
	[53]				= { -1,		-1,	-1 },
	[54]				= { -1,		-1,	-1 },
	[55]				= { -1,		-1,	-1 },
	[56]				= { -1,		-1,	-1 },
	[57]				= { -1,		-1,	-1 },
	[58]				= { -1,		-1,	-1 },
	[59]				= { -1,		-1,	-1 },

	/*60~74*/
	[SPI1_MISO]			= { 4,		0,	-1 },
	[SPI1_CSO_B]		= { 4,		1,	-1 },
	[SPI1_CLK]			= { 4,		2,	-1 },
	[MMC1_D6]			= { 4,		3,	-1 },
	[SPI1_MOSI]			= { 4,		4,	-1 },
	[MMC1_D5]			= { 4,		5,	-1 },
	[SPI1_CS1_B]		= { 4,		6,	-1 },
	[MMC1_D4_SD_WE]		= { 4,		7,	-1 },
	[MMC1_D7]			= { 4,		8,	-1 },
	[MMC1_RCLK]			= { 4,		9,	-1 },
	[70]				= { -1,		-1,	-1 },
	[71]				= { -1,		-1,	-1 },
	[72]				= { -1,		-1,	-1 },
	[73]				= { -1,		-1,	-1 },
	[74]				= { -1,		-1,	-1 },

	/*75~85*/
	[USB_OC1_B]			= { 5,		0,	-1 },
	[PMU_RESETBUTTON_B]	= { 5,		1,	-1 },
	[GPIO_ALERT]		= { 5,		2,	-1 },
	[SDMMC3_PWR_EN_B]	= { 5,		3,	-1 },
	[ILB_SERIRQ]		= { 5,		4,	-1 },
	[USB_OC0_B]			= { 5,		5,	-1 },
	[SDMMC3_CD_B]		= { 5,		6,	-1 },
	[SPKR]				= { 5,		7,	-1 },
	[SUSPWRDNACK]		= { 5,		8,	-1 },
	[SPARE_PIN]			= { 5,		9,	-1 },
	[SDMMC3_1P8_EN]		= { 5,		10,	-1 },
};

static struct gpio_pad_info east_pads_info[CV_NGPIO_EAST] = {
	/*0~14*/
	[PMU_SLP_S3_B]		= { 0,		0,	-1 },
	[PMU_BATLOW_B]		= { 0,		1,	-1 },
	[SUS_STAT_B]		= { 0,		2,	-1 },
	[PMU_SLP_S0IX_B]	= { 0,		3,	-1 },
	[PMU_AC_PRESENT]	= { 0,		4,	-1 },
	[PMU_PLTRST_B]		= { 0,		5,	-1 },
	[PMU_SUSCLK]		= { 0,		6,	-1 },
	[PMU_SLP_LAN_B]		= { 0,		7,	-1 },
	[PMU_PWRBTN_B]		= { 0,		8,	-1 },
	[PMU_SLP_S4_B]		= { 0,		9,	-1 },
	[PMU_WAKE_B]		= { 0,		10,	-1 },
	[PMU_WAKE_LAN_B]	= { 0,		11,	-1 },
	[12]				= { -1,		-1,	-1 },
	[13]				= { -1,		-1,	-1 },
	[14]				= { -1,		-1,	-1 },

	/*15~26*/
	[MF_ISH_GPIO_3]		= { 1,		0,	-1 },
	[MF_ISH_GPIO_7]		= { 1,		1,	-1 },
	[MF_ISH_I2C1_SCL]	= { 1,		2,	-1 },
	[MF_ISH_GPIO_1]		= { 1,		3,	-1 },
	[MF_ISH_GPIO_5]		= { 1,		4,	-1 },
	[MF_ISH_GPIO_9]		= { 1,		5,	-1 },
	[MF_ISH_GPIO_0]		= { 1,		6,	-1 },
	[MF_ISH_GPIO_4]		= { 1,		7,	-1 },
	[MF_ISH_GPIO_8]		= { 1,		8,	-1 },
	[MF_ISH_GPIO_2]		= { 1,		9,	-1 },
	[MF_ISH_GPIO_6]		= { 1,		10,	-1 },
	[MF_ISH_I2C1_SDA]	= { 1,		11,	-1 },
};

static struct gpio_pad_info southwest_pads_info[CV_NGPIO_SOUTHWEST] = {
	/*0~14*/
	[FST_SPI_D2]		= { 0,		0,	-1 },
	[FST_SPI_D0]		= { 0,		1,	-1 },
	[FST_SPI_CLK]		= { 0,		2,	-1 },
	[FST_SPI_D3]		= { 0,		3,	-1 },
	[FST_SPI_CS1_B]		= { 0,		4,	-1 },
	[FST_SPI_D1]		= { 0,		5,	-1 },
	[FST_SPI_CS0_B]		= { 0,		6,	-1 },
	[FST_SPI_CS2_B]		= { 0,		7,	-1 },
	[8]					= { -1,		-1,	-1 },
	[9]					= { -1,		-1,	-1 },
	[10]				= { -1,		-1,	-1 },
	[11]				= { -1,		-1,	-1 },
	[12]				= { -1,		-1,	-1 },
	[13]				= { -1,		-1,	-1 },
	[14]				= { -1,		-1,	-1 },

	/*15~29*/
	[UART1_RTS_B]		= { 1,		0,	-1 },
	[UART1_RXD]			= { 1,		1,	-1 },
	[UART2_RXD]			= { 1,		2,	-1 },
	[UART1_CTS_B]		= { 1,		3,	-1 },
	[UART2_RTS_B]		= { 1,		4,	-1 },
	[UART1_TXD]			= { 1,		5,	-1 },
	[UART2_TXD]			= { 1,		6,	-1 },
	[UART2_CTS_B]		= { 1,		7,	-1 },
	[23]				= { -1,		-1,	-1 },
	[24]				= { -1,		-1,	-1 },
	[25]				= { -1,		-1,	-1 },
	[26]				= { -1,		-1,	-1 },
	[27]				= { -1,		-1,	-1 },
	[28]				= { -1,		-1,	-1 },
	[29]				= { -1,		-1,	-1 },

	/*30~44*/
	[MF_HDA_CLK]		= { 2,		0,	-1 },
	[MF_HDA_RSTB]		= { 2,		1,	-1 },
	[MF_HDA_SDIO]		= { 2,		2,	-1 },
	[MF_HDA_SDO]		= { 2,		3,	-1 },
	[MF_HDA_DOCKRSTB]	= { 2,		4,	-1 },
	[MF_HDA_SYNC]		= { 2,		5,	-1 },
	[MF_HDA_SDI1]		= { 2,		6,	-1 },
	[MF_HDA_DOCKENB]	= { 2,		7,	-1 },
	[38]				= { -1,		-1,	-1 },
	[39]				= { -1,		-1,	-1 },
	[40]				= { -1,		-1,	-1 },
	[41]				= { -1,		-1,	-1 },
	[42]				= { -1,		-1,	-1 },
	[43]				= { -1,		-1,	-1 },
	[44]				= { -1,		-1,	-1 },

	/*45~59*/
	[I2C5_SDA]			= { 3,		0,	-1 },
	[I2C4_SDA]			= { 3,		1,	-1 },
	[47]				= { -1,		-1,	-1 },
	[48]				= { -1,		-1,	-1 },
	[I2C6_SDA]			= { 3,		2,	-1 },
	[I2C5_SCL]			= { 3,		3,	-1 },
	[I2C_NFC_SDA]		= { 3,		4,	-1 },
	[I2C4_SCL]			= { 3,		5,	-1 },
	[I2C6_SCL]			= { 3,		6,	-1 },
	[I2C_NFC_SCL]		= { 3,		7,	-1 },
	[55]				= { -1,		-1,	-1 },
	[56]				= { -1,		-1,	-1 },
	[57]				= { -1,		-1,	-1 },
	[58]				= { -1,		-1,	-1 },
	[59]				= { -1,		-1,	-1 },

	/*60~74*/
	[I2C1_SDA]			= { 4,		0,	-1 },
	[I2C0_SDA]			= { 4,		1,	-1 },
	[I2C2_SDA]			= { 4,		2,	-1 },
	[I2C1_SCL]			= { 4,		3,	-1 },
	[I2C3_SDA]			= { 4,		4,	-1 },
	[I2C0_SCL]			= { 4,		5,	-1 },
	[I2C2_SCL]			= { 4,		6,	-1 },
	[I2C3_SCL]			= { 4,		7,	-1 },
	[68]				= { -1,		-1,	-1 },
	[69]				= { -1,		-1,	-1 },
	[70]				= { -1,		-1,	-1 },
	[71]				= { -1,		-1,	-1 },
	[72]				= { -1,		-1,	-1 },
	[73]				= { -1,		-1,	-1 },
	[74]				= { -1,		-1,	-1 },

	/*75~89*/
	[SATA_GP0]			= { 5,		0,	-1 },
	[SATA_GP1]			= { 5,		1,	-1 },
	[SATA_LEDN]			= { 5,		2,	-1 },
	[SATA_GP2]			= { 5,		3,	-1 },
	[MF_SMB_ALERTB]		= { 5,		4,	-1 },
	[SATA_GP3]			= { 5,		5,	-1 },
	[MF_SMB_CLK]		= { 5,		6,	-1 },
	[MF_SMB_DATA]		= { 5,		7,	-1 },
	[83]				= { -1,		-1,	-1 },
	[84]				= { -1,		-1,	-1 },
	[85]				= { -1,		-1,	-1 },
	[86]				= { -1,		-1,	-1 },
	[87]				= { -1,		-1,	-1 },
	[88]				= { -1,		-1,	-1 },
	[89]				= { -1,		-1,	-1 },

	/*90~97*/
	[PCIE_CLKREQ0B]		= { 6,		0,	-1 },
	[PCIE_CLKREQ1B]		= { 6,		1,	-1 },
	[GP_SSP_2_CLK]		= { 6,		2,	-1 },
	[PCIE_CLKREQ2B]		= { 6,		3,	-1 },
	[GP_SSP_2_RXD]		= { 6,		4,	-1 },
	[PCIE_CLKREQ3B]		= { 6,		5,	-1 },
	[GP_SSP_2_FS]		= { 6,		6,	-1 },
	[GP_SSP_2_TXD]		= { 6,		7,	-1 },
};

static struct gpio_pad_info virtual_pads_info[CV_NGPIO_VIRTUAL] = {
	[VIRTUAL0]		= { 0,		0,	-1 },
	[VIRTUAL1]		= { 0,		1,	-1 },
	[VIRTUAL2]		= { 0,		2,	-1 },
	[VIRTUAL3]		= { 0,		3,	-1 },
	[VIRTUAL4]		= { 0,		4,	-1 },
	[VIRTUAL5]		= { 0,		5,	-1 },
	[VIRTUAL6]		= { 0,		6,	-1 },
	[VIRTUAL7]		= { 0,		7,	-1 },
};

static struct gpio_bank_pnp chv_banks_pnp[] = {
	{
		.name = "GPO0",
		.gpio_base = CV_GPIO_SOUTHWEST_BASE,
		.irq_base = CV_GPIO_SOUTHWEST_IRQBASE,
		.ngpio = CV_NGPIO_SOUTHWEST,
		.pads_info = southwest_pads_info,
	},
	{
		.name = "GPO1",
		.gpio_base = CV_GPIO_NORTH_BASE,
		.irq_base = CV_GPIO_NORTH_IRQBASE,
		.ngpio = CV_NGPIO_NORTH,
		.pads_info = north_pads_info,
	},
	{
		.name = "GPO2",
		.gpio_base = CV_GPIO_EAST_BASE,
		.irq_base = CV_GPIO_EAST_IRQBASE,
		.ngpio = CV_NGPIO_EAST,
		.pads_info = east_pads_info,
	},
	{
		.name = "GPO3",
		.gpio_base = CV_GPIO_SOUTHEAST_BASE,
		.irq_base = CV_GPIO_SOUTHEAST_IRQBASE,
		.ngpio = CV_NGPIO_SOUTHEAST,
		.pads_info = southeast_pads_info,
	},
	{
		.name = "GPO4",
		.gpio_base = CV_GPIO_VIRTUAL_BASE,
		.irq_base = CV_GPIO_VIRTUAL_IRQBASE,
		.ngpio = CV_NGPIO_VIRTUAL,
		.pads_info = virtual_pads_info,
	},
};

struct chv_gpio {
	struct gpio_chip	chip;
	struct pnp_dev		*pdev;
	spinlock_t		lock;
	void __iomem		*reg_base;
	struct gpio_pad_info	*pad_info;
	struct irq_domain	*domain;
	int			irq_base;
	int			intr_lines[MAX_INTR_LINE_NUM];
};

static DEFINE_SPINLOCK(chv_reg_access_lock);

#define to_chv_priv(chip)	container_of(chip, struct chv_gpio, chip)

static void __iomem *chv_gpio_reg(struct gpio_chip *chip, unsigned offset,
				 int reg)
{
	struct chv_gpio *cg = to_chv_priv(chip);
	u32 reg_offset;
	void __iomem *ptr;

	if (reg == CV_INT_STAT_REG || reg == CV_INT_MASK_REG)
		reg_offset = 0;
	else
		reg_offset = FAMILY0_PAD_REGS_OFF +
		      FAMILY_PAD_REGS_SIZE * (offset / MAX_FAMILY_PAD_GPIO_NO) +
		      GPIO_REGS_SIZE * (offset % MAX_FAMILY_PAD_GPIO_NO);

	ptr = (void __iomem *) (cg->reg_base + reg_offset + reg);
	return ptr;
}

static u32 chv_readl(void __iomem *reg)
{
	u32 value;
	unsigned long flags;

	spin_lock_irqsave(&chv_reg_access_lock, flags);
	value = readl(reg);
	spin_unlock_irqrestore(&chv_reg_access_lock, flags);

	return value;
}

static void chv_writel(u32 value, void __iomem *reg)
{
	unsigned long flags;

	spin_lock_irqsave(&chv_reg_access_lock, flags);
	writel(value, reg);
	/* simple readback to confirm the bus transferring done */
	readl(reg);
	spin_unlock_irqrestore(&chv_reg_access_lock, flags);
}

static int chv_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	struct chv_gpio *cg = to_chv_priv(chip);

	if (cg->pad_info[offset].family < 0)
		return -EINVAL;

	return 0;
}

static void chv_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	return;
}

static void chv_update_irq_type(struct chv_gpio *cg, unsigned type,
				void __iomem *reg)
{
	u32 value;

	value = chv_readl(reg);
	value &= ~CV_INT_CFG_MASK;
	value &= ~CV_INV_RX_DATA;

	if (type & IRQ_TYPE_EDGE_BOTH) {
		if ((type & IRQ_TYPE_EDGE_BOTH) == IRQ_TYPE_EDGE_BOTH)
			value |= CV_TRIG_EDGE_BOTH;
		else if (type & IRQ_TYPE_EDGE_RISING)
			value |= CV_TRIG_EDGE_RISING;
		else if (type & IRQ_TYPE_EDGE_FALLING)
			value |= CV_TRIG_EDGE_FALLING;
	} else if (type & IRQ_TYPE_LEVEL_MASK) {
			value |= CV_TRIG_LEVEL;
		if (type & IRQ_TYPE_LEVEL_LOW)
			value |= CV_INV_RX_DATA;
	}

	chv_writel(value, reg);
}

/* BIOS programs IntSel bits for shared interrupt.
 * GPIO driver follows it.
 */
static void pad_intr_line_save(struct chv_gpio *cg, unsigned offset)
{
	u32 value;
	u32 intr_line;
	void __iomem *reg = chv_gpio_reg(&cg->chip, offset, CV_PADCTRL0_REG);
	struct gpio_pad_info *pad_info = cg->pad_info + offset;

	value = chv_readl(reg);
	intr_line = (value & CV_INT_SEL_MASK) >> 28;
	pad_info->interrupt_line = intr_line;
	cg->intr_lines[intr_line] = offset;
}

static int chv_irq_type(struct irq_data *d, unsigned type)
{
	struct chv_gpio *cg = irq_data_get_irq_chip_data(d);
	u32 offset = irqd_to_hwirq(d);
	void __iomem *reg;
	unsigned long flags;
	int ret = 0;

	if (offset >= cg->chip.ngpio)
		return -EINVAL;

	if (cg->pad_info[offset].family < 0)
		return -EINVAL;

	spin_lock_irqsave(&cg->lock, flags);

	/* Pins which can be used as shared interrupt are configured in BIOS.
	 * Driver trusts BIOS configurations and assigns different handler
	 * according to the irq type.
	 *
	 * Driver needs to save the mapping between each pin and
	 * its interrupt line.
	 * 1. If the pin cfg is locked in BIOS:
	 *	Trust BIOS has programmed IntWakeCfg bits correctly,
	 *	driver just needs to save the mapping.
	 * 2. If the pin cfg is not locked in BIOS:
	 *	Driver programs the IntWakeCfg bits and save the mapping.
	 *
	 */
	if (!PAD_CFG_LOCKED(offset)) {
		reg = chv_gpio_reg(&cg->chip, offset, CV_PADCTRL1_REG);

		chv_update_irq_type(cg, type, reg);
	}

	pad_intr_line_save(cg, offset);

	if (type & IRQ_TYPE_EDGE_BOTH)
		__irq_set_handler_locked(d->irq, handle_edge_irq);
	else if (type & IRQ_TYPE_LEVEL_MASK)
		__irq_set_handler_locked(d->irq, handle_level_irq);

	spin_unlock_irqrestore(&cg->lock, flags);

	return ret;
}

static int chv_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct chv_gpio *cg = to_chv_priv(chip);
	void __iomem *reg;
	u32 value;

	if (cg->pad_info[offset].family < 0)
		return -EINVAL;

	reg = chv_gpio_reg(chip, offset, CV_PADCTRL0_REG);
	value = chv_readl(reg);

	return value & CV_GPIO_RX_STAT;
}

static void chv_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct chv_gpio *cg = to_chv_priv(chip);
	void __iomem *reg;
	unsigned long flags;
	u32 old_val;

	if (cg->pad_info[offset].family < 0)
		return;

	reg = chv_gpio_reg(chip, offset, CV_PADCTRL0_REG);

	spin_lock_irqsave(&cg->lock, flags);

	old_val = chv_readl(reg);

	if (value)
		chv_writel(old_val | CV_GPIO_TX_STAT, reg);
	else
		chv_writel(old_val & ~CV_GPIO_TX_STAT, reg);

	spin_unlock_irqrestore(&cg->lock, flags);
}

static int chv_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct chv_gpio *cg = to_chv_priv(chip);
	void __iomem *reg;
	unsigned long flags;
	u32 value;

	if (cg->pad_info[offset].family < 0)
		return -EINVAL;

	if (PAD_CFG_LOCKED(offset))
		return 0;

	reg = chv_gpio_reg(chip, offset, CV_PADCTRL0_REG);

	spin_lock_irqsave(&cg->lock, flags);

	value = chv_readl(reg) & (~CV_GPIO_CFG_MASK);
	/* Disable TX and Enable RX */
	value |= CV_GPIO_RX_EN;
	chv_writel(value, reg);

	spin_unlock_irqrestore(&cg->lock, flags);

	return 0;
}

static int chv_gpio_direction_output(struct gpio_chip *chip,
				     unsigned offset, int value)
{
	struct chv_gpio *cg = to_chv_priv(chip);
	void __iomem *ctrl0, *ctrl1;
	unsigned long flags;
	u32 reg_val;

	if (cg->pad_info[offset].family < 0)
		return -EINVAL;

	if (PAD_CFG_LOCKED(offset))
		return 0;

	ctrl0 = chv_gpio_reg(chip, offset, CV_PADCTRL0_REG);
	ctrl1 = chv_gpio_reg(chip, offset, CV_PADCTRL1_REG);

	spin_lock_irqsave(&cg->lock, flags);

	/* Make sure interrupt of this pad is disabled */
	chv_update_irq_type(cg, IRQ_TYPE_NONE, ctrl1);

	reg_val = chv_readl(ctrl0) & (~CV_GPIO_CFG_MASK);

	/* Enable both RX and TX, control TX State */
	if (value)
		reg_val |= CV_GPIO_TX_STAT;
	else
		reg_val &= ~CV_GPIO_TX_STAT;

	chv_writel(reg_val, ctrl0);

	spin_unlock_irqrestore(&cg->lock, flags);

	return 0;
}

static void chv_irq_unmask(struct irq_data *d)
{
	struct chv_gpio *cg = irq_data_get_irq_chip_data(d);
	u32 offset = irqd_to_hwirq(d);
	int interrupt_line;
	u32 value;
	void __iomem *reg = chv_gpio_reg(&cg->chip, 0, CV_INT_MASK_REG);
	unsigned long flags;
	struct gpio_pad_info *pad_info = cg->pad_info + offset;

	if (cg->pad_info[offset].family < 0)
		return;

	if (offset >= cg->chip.ngpio)
		return;

	spin_lock_irqsave(&cg->lock, flags);

	interrupt_line = pad_info->interrupt_line;
	/* Unmask if this GPIO has valid interrupt line */
	if (interrupt_line >= 0) {
		value = chv_readl(reg);
		value |= (1 << interrupt_line);
		chv_writel(value, reg);
	} else {
		dev_warn(&cg->pdev->dev,
			"Trying to unmask GPIO intr which is not allocated\n");
	}

	spin_unlock_irqrestore(&cg->lock, flags);
}

static void chv_irq_mask(struct irq_data *d)
{
	struct chv_gpio *cg = irq_data_get_irq_chip_data(d);
	u32 offset = irqd_to_hwirq(d);
	int interrupt_line;
	u32 value;
	unsigned long flags;
	void __iomem *reg = chv_gpio_reg(&cg->chip, 0, CV_INT_MASK_REG);
	struct gpio_pad_info *pad_info = cg->pad_info + offset;

	if (cg->pad_info[offset].family < 0)
		return;

	if (offset >= cg->chip.ngpio)
		return;

	spin_lock_irqsave(&cg->lock, flags);

	interrupt_line = pad_info->interrupt_line;
	/* Mask if this GPIO has valid interrupt line */
	if (interrupt_line >= 0) {
		value = chv_readl(reg);
		value &= (~(1 << interrupt_line));
		chv_writel(value, reg);
	} else {
		dev_warn(&cg->pdev->dev,
			"Trying to mask GPIO intr which is not allocated\n");
	}

	spin_unlock_irqrestore(&cg->lock, flags);
}

static int chv_irq_wake(struct irq_data *d, unsigned on)
{
	return 0;
}

static void chv_irq_ack(struct irq_data *d)
{
}

static void chv_irq_shutdown(struct irq_data *d)
{
	struct chv_gpio *cg = irq_data_get_irq_chip_data(d);
	u32 offset = irqd_to_hwirq(d);
	void __iomem *reg;
	unsigned long flags;

	if (cg->pad_info[offset].family < 0)
		return;

	reg = chv_gpio_reg(&cg->chip, offset, CV_PADCTRL1_REG);

	chv_irq_mask(d);

	if (!PAD_CFG_LOCKED(offset)) {
		spin_lock_irqsave(&cg->lock, flags);
		chv_update_irq_type(cg, IRQ_TYPE_NONE, reg);
		spin_unlock_irqrestore(&cg->lock, flags);
	}
}

static struct irq_chip chv_irqchip = {
	.name		= "CHV-GPIO",
	.irq_mask	= chv_irq_mask,
	.irq_unmask	= chv_irq_unmask,
	.irq_set_type	= chv_irq_type,
	.irq_set_wake	= chv_irq_wake,
	.irq_ack	= chv_irq_ack,
	.irq_shutdown	= chv_irq_shutdown,
};

static void chv_gpio_irq_dispatch(struct chv_gpio *cg)
{
	u32 intr_line, mask, offset;
	void __iomem *reg, *mask_reg;
	u32 pending;

	/* each GPIO controller has one INT_STAT reg */
	reg = chv_gpio_reg(&cg->chip, 0, CV_INT_STAT_REG);
	mask_reg = chv_gpio_reg(&cg->chip, 0, CV_INT_MASK_REG);
	while ((pending = (chv_readl(reg) & chv_readl(mask_reg) & 0xFFFF))) {
		intr_line = __ffs(pending);
		mask = BIT(intr_line);
		chv_writel(mask, reg);
		offset = cg->intr_lines[intr_line];
		if (unlikely(offset < 0)) {
			dev_warn(&cg->pdev->dev, "unregistered shared irq\n");
			continue;
		}

		generic_handle_irq(irq_find_mapping(cg->domain, offset));
	}
}

static void chv_gpio_irq_handler(unsigned irq, struct irq_desc *desc)
{
	struct irq_data *data = irq_desc_get_irq_data(desc);
	struct chv_gpio *cg = irq_data_get_irq_handler_data(data);
	struct irq_chip *chip = irq_data_get_irq_chip(data);

	chv_gpio_irq_dispatch(cg);
	chip->irq_eoi(data);
}

static void chv_irq_init_hw(struct chv_gpio *cg)
{
	void __iomem *reg;

	reg = chv_gpio_reg(&cg->chip, 0, CV_INT_STAT_REG);
	chv_writel(0xffff, reg);
}


static int chv_gpio_to_irq(struct gpio_chip *chip, unsigned offset)
{
	struct chv_gpio *cg = to_chv_priv(chip);

	return irq_create_mapping(cg->domain, offset);
}

static int chv_gpio_irq_map(struct irq_domain *d, unsigned int virq,
			    irq_hw_number_t hw)
{
	struct chv_gpio *cg = d->host_data;

	irq_set_chip_and_handler_name(virq, &chv_irqchip, handle_simple_irq,
				      "demux");
	irq_set_chip_data(virq, cg);

	return 0;
}

static const struct irq_domain_ops chv_gpio_irq_ops = {
	.map = chv_gpio_irq_map,
	.xlate = irq_domain_xlate_twocell,
};

static int
chv_gpio_pnp_probe(struct pnp_dev *pdev, const struct pnp_device_id *id)
{
	int i;
	struct chv_gpio *cg;
	struct gpio_chip *gc;
	struct resource *mem_rc, *irq_rc;
	struct device *dev = &pdev->dev;
	struct gpio_bank_pnp *bank;
	int ret = 0;
	int nbanks = sizeof(chv_banks_pnp) / sizeof(struct gpio_bank_pnp);

	cg = devm_kzalloc(dev, sizeof(struct chv_gpio), GFP_KERNEL);
	if (!cg) {
		dev_err(dev, "can't allocate chv_gpio chip data\n");
		return -ENOMEM;
	}
	cg->pdev = pdev;

	for (i = 0; i < nbanks; i++) {
		bank = chv_banks_pnp + i;
		if (!strcmp(pdev->name, bank->name)) {
			cg->chip.ngpio = bank->ngpio;
			cg->pad_info = bank->pads_info;
			cg->irq_base = bank->irq_base;
			bank->cg = cg;
			break;
		}
	}
	if (!bank || cg->chip.ngpio == 0) {
		dev_err(dev, "can't find %s\n", pdev->name);
		ret = -ENODEV;
		goto err;
	}

	mem_rc = pnp_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem_rc) {
		dev_err(dev, "missing MEM resource\n");
		ret = -EINVAL;
		goto err;
	}

	irq_rc = pnp_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!irq_rc) {
		dev_err(dev, "missing IRQ resource\n");
		ret = -EINVAL;
		goto err;
	}

	cg->reg_base = devm_request_and_ioremap(dev, mem_rc);
	if (cg->reg_base == NULL) {
		dev_err(dev, "error mapping resource\n");
		ret = -EINVAL;
		goto err;
	}

	spin_lock_init(&cg->lock);
	gc = &cg->chip;
	gc->label = dev_name(&pdev->dev);
	gc->owner = THIS_MODULE;
	gc->request = chv_gpio_request;
	gc->free = chv_gpio_free;
	gc->direction_input = chv_gpio_direction_input;
	gc->direction_output = chv_gpio_direction_output;
	gc->get = chv_gpio_get;
	gc->set = chv_gpio_set;
	gc->to_irq = chv_gpio_to_irq;
	gc->base = bank->gpio_base;
	gc->can_sleep = 0;
	gc->dev = dev;

	/* Initialize interrupt lines array with negative value */
	for (i = 0; i < MAX_INTR_LINE_NUM; i++)
		cg->intr_lines[i] = -1;

	cg->domain = irq_domain_add_simple(pdev->dev.of_node,
			cg->chip.ngpio, cg->irq_base,
			&chv_gpio_irq_ops, cg);
	if (!cg->domain) {
		ret = -ENOMEM;
		goto err;
	}

	ret = gpiochip_add(gc);
	if (ret) {
		dev_err(dev, "failed adding chv-gpio chip\n");
		goto err;
	}

	chv_irq_init_hw(cg);

	if (irq_rc && irq_rc->start) {
		irq_set_handler_data(irq_rc->start, cg);
		irq_set_chained_handler(irq_rc->start, chv_gpio_irq_handler);
	}

	dev_info(dev, "Cherryview GPIO %s probed\n", pdev->name);

	return 0;
err:
	devm_kfree(dev, cg);
	return ret;
}


static const struct pnp_device_id chv_gpio_pnp_match[] = {
	{ "INT33FF", 0 },
	{ }
};
MODULE_DEVICE_TABLE(pnp, chv_gpio_pnp_match);

static struct pnp_driver chv_gpio_pnp_driver = {
	.name		= "chv_gpio",
	.id_table	= chv_gpio_pnp_match,
	.probe          = chv_gpio_pnp_probe,
};

static int __init chv_gpio_init(void)
{
	return pnp_register_driver(&chv_gpio_pnp_driver);
}

fs_initcall(chv_gpio_init);
