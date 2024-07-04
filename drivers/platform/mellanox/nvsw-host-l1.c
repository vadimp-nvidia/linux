// SPDX-License-Identifier: GPL-2.0+
/*
 * Nvidia BMC platform driver
 *
 * Copyright (C) 2025 Nvidia Technologies Ltd.
 */

#include <linux/device.h>
#include <linux/dmi.h>
#include <linux/i2c.h>
#include <linux/i2c-mux.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/platform_data/i2c-mux-reg.h>
#include <linux/platform_data/mlxreg.h>
#include <linux/regmap.h>

#include "nvsw.h"

#define NVSW_HOST_DEVICE_NAME	"mlxplat"

/* LPC bus IO offsets */
#define NVSW_I2C_BASE_ADRR	0x2000
#define NVSW_REG_BASE_ADRR	0x2500
#define NVSW_LPC_IO_RANGE	0x100
#define NVSW_LPC_PIO_OFFSET	0x10000UL
#define NVSW_REG_MUX1		(NVSW_REG_MUX1_OFFSET | NVSW_LPC_PIO_OFFSET)
#define NVSW_REG_MUX2		(NVSW_REG_MUX0_OFFSET | NVSW_LPC_PIO_OFFSET)

/* Start channel numbers */
#define NVSW_PARENT_CH_L1	1
#define NVSW_CH1_L1		9
#define NVSW_CH2_L1		63

/* Regions for LPC I2C controller and LPC base register space */
static const struct resource nvsw_host_io_resources[] = {
	[0] = DEFINE_RES_NAMED(NVSW_I2C_BASE_ADRR, NVSW_LPC_IO_RANGE,
			       "nvsw_cpld_lpc_i2c_ctrl", IORESOURCE_IO),
	[1] = DEFINE_RES_NAMED(NVSW_REG_BASE_ADRR, NVSW_LPC_IO_RANGE,
			       "nvsw_cpld_lpc_regs", IORESOURCE_IO),
};

/* Platform channels for L1 scale out system family */
static const int nvsw_host_l1_mgmt_channels[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 48, 49,
	50, 51, 52, 53, 54,
};

static const int nvsw_host_l1_channels[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

/* Platform L1 scale out mux data */
static struct i2c_mux_reg_platform_data nvsw_host_l1_mux_data[] = {
	{
		.parent = NVSW_PARENT_CH_L1,
		.base_nr = NVSW_CH1_L1 + 1,
		.write_only = 1,
		.reg = (void __iomem *)NVSW_REG_MUX1,
		.reg_size = 1,
		.idle_in_use = 1,
		.values = nvsw_host_l1_mgmt_channels,
		.n_values = ARRAY_SIZE(nvsw_host_l1_mgmt_channels),
	},
	{
		.parent = NVSW_PARENT_CH_L1,
		.base_nr = NVSW_CH2_L1,
		.write_only = 1,
		.reg = (void __iomem *)NVSW_REG_MUX2,
		.reg_size = 1,
		.idle_in_use = 1,
		.values = nvsw_host_l1_channels,
		.n_values = ARRAY_SIZE(nvsw_host_l1_channels),
	},

};

static struct platform_device *nvsw_host_dev;
static struct i2c_mux_reg_platform_data *nvsw_host_mux_data[NVSW_MUX_MAX];
static struct mlxreg_core_platform_data *nvsw_led_data;
static struct mlxreg_core_platform_data *nvsw_regs_io_data;
static struct mlxreg_core_platform_data *nvsw_wd_data[NVSW_WD_MAX];
static int mux_num;
static enum nvsw_core_hid_type nvsw_host_hid;

/* Platform register access for l1 systems families data */
static struct mlxreg_core_data nvsw_host_l1_regs_io_data[] = {
	{
		.label = "cpld1_version",
		.reg = NVSW_REG_CPLD1_VER_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "cpld2_version",
		.reg = NVSW_REG_CPLD2_VER_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "cpld1_pn",
		.reg = NVSW_REG_CPLD1_PN_OFFSET,
		.bit = GENMASK(15, 0),
		.mode = 0444,
		.regnum = 2,
	},
	{
		.label = "cpld2_pn",
		.reg = NVSW_REG_CPLD2_PN_OFFSET,
		.bit = GENMASK(15, 0),
		.mode = 0444,
		.regnum = 2,
	},
	{
		.label = "cpld1_version_min",
		.reg = NVSW_REG_CPLD1_MVER_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "cpld2_version_min",
		.reg = NVSW_REG_CPLD2_MVER_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "bios_status",
		.reg = NVSW_REG_GPCOM0_OFFSET,
		.mask = GENMASK(3, 1),
		.bit = 3,
		.mode = 0444,
	},
	{
		.label = "bios_start_retry",
		.reg = NVSW_REG_GPCOM0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0444,
	},
	{
		.label = "bios_active_image",
		.reg = NVSW_REG_GPCOM0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0444,
	},
	{
		.label = "pwr_converter_prog_en",
		.reg = NVSW_REG_GP0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(0),
		.mode = 0644,
	},
	{
		.label = "cpu_mctp_ready",
		.reg = NVSW_REG_GP0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0644,
	},
	{
		 .label = "cpu_shutdown_req",
		 .reg = NVSW_REG_GP0_OFFSET,
		 .mask = GENMASK(7, 0) & ~BIT(2),
		 .mode = 0444,
	},
	{
		.label = "vpd_wp",
		.reg = NVSW_REG_GP0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0644,
		.secured = 1,
	},
	{
		.label = "pcie_asic_reset_dis",
		.reg = NVSW_REG_GP0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0644,
	},
	{
		.label = "shutdown_unlock",
		.reg = NVSW_REG_GP0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0644,
	},
	{
		.label = "cpu_power_off_ready",
		.reg = NVSW_REG_GP0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0644,
	},
	{
		.label = "pwr_cycle",
		.reg = NVSW_REG_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(2),
		.mode = 0244,
	},
	{
		.label = "pwr_down",
		.reg = NVSW_REG_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0200,
	},
	{
		.label = "aux_pwr_cycle",
		.reg = NVSW_REG_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0200,
	},
	{
		.label = "bmc_to_cpu_ctrl",
		.reg = NVSW_REG_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0644,
	},
	{
		.label = "uart_sel",
		.reg = NVSW_REG_GP1_OFFSET,
		.mask = NVSW_UART_SEL_MASK,
		.bit = 7,
		.mode = 0644,
	},
	{
		.label = "clk_brd_prog_en",
		.reg = NVSW_REG_PWM_CONTROL_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0644,
	},
	{
		.label = "hotswap_alert",
		.reg = NVSW_REG_BRD4_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(0),
		.mode = 0444,
	},
	{
		.label = "cartridge1",
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = BIT(0),
		.mode = 0444,
	},
	{
		.label = "cartridge2",
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = BIT(1),
		.mode = 0444,
	},
	{
		.label = "cartridge3",
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = BIT(2),
		.mode = 0444,
	},
	{
		.label = "cartridge4",
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = BIT(3),
		.mode = 0444,
	},
	{
		.label = "cartridge_status_clear",
		.reg = NVSW_REG_FRU1_EVENT_OFFSET,
		.bit = GENMASK(3, 0),
		.mode = 0644,
	},
	{
		.label = "leakage1",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(0),
		.mode = 0444,
	},
	{
		.label = "leakage2",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0444,
	},
	{
		.label = "leakage3",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(2),
		.mode = 0444,
	},
	{
		.label = "leakage4",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0444,
	},
	{
		.label = "leakage_status_clear",
		.reg = NVSW_REG_LEAK_EVENT_OFFSET,
		.bit = GENMASK(5, 0),
		.mode = 0644,
	},
	{
		.label = "asic_reset",
		.reg = NVSW_REG_RESET_GP2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0644,
	},
	{
		.label = "sgmii_phy_reset",
		.reg = NVSW_REG_RESET_GP2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0644,
	},
	{
		.label = "reset_long_pb",
		.reg = NVSW_REG_RESET_CAUSE_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(0),
		.mode = 0444,
	},
	{
		.label = "reset_short_pb",
		.reg = NVSW_REG_RESET_CAUSE_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0444,
	},
	{
		.label = "reset_aux_pwr_or_fu",
		.reg = NVSW_REG_RESET_CAUSE_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(2),
		.mode = 0444,
	},
	{
		.label = "reset_swb_dc_dc_pwr_fail",
		.reg = NVSW_REG_RESET_CAUSE_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0444,
	},
	{
		.label = "reset_pwr_button_or_leak_con",
		.reg = NVSW_REG_RESET_CAUSE_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0444,
	},
	{
		.label = "reset_swb_wd",
		.reg = NVSW_REG_RESET_CAUSE_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0444,
	},
	{
		.label = "reset_asic_thermal",
		.reg = NVSW_REG_RESET_CAUSE_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(7),
		.mode = 0444,
	},
	{
		.label = "reset_comex_pwr_fail",
		.reg = NVSW_REG_RESET_CAUSE1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0444,
	},
	{
		.label = "reset_platform",
		.reg = NVSW_REG_RESET_CAUSE1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0444,
	},
	{
		.label = "reset_soc",
		.reg = NVSW_REG_RESET_CAUSE1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0444,
	},
	{
		.label = "reset_system",
		.reg = NVSW_REG_RESET_CAUSE2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0444,
	},
	{
		.label = "reset_sw_pwr_off",
		.reg = NVSW_REG_RESET_CAUSE2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(2),
		.mode = 0444,
	},
	{
		.label = "reset_comex_thermal",
		.reg = NVSW_REG_RESET_CAUSE2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0444,
	},
	{
		.label = "reset_comex_power",
		.reg = NVSW_REG_RESET_CAUSE2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0444,
	},
	{
		.label = "reset_pwr_converter_fail",
		.reg = NVSW_REG_RESET_CAUSE2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0444,
	},
	{
		.label = "reset_main_5v",
		.reg = NVSW_REG_RESET_CAUSE2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0444,
	},
	{
		.label = "reset_mgmt_pwr",
		.reg = NVSW_REG_RESET_CAUSE2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(7),
		.mode = 0444,
	},
	{
		.label = "port80",
		.reg = NVSW_REG_GP1_RO_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "jtag_cap",
		.reg = NVSW_REG_FU_CAP_OFFSET,
		.mask = NVSW_FU_CAP_MASK,
		.bit = 1,
		.mode = 0444,
	},
	{
		.label = "jtag_enable",
		.reg = NVSW_REG_FIELD_UPGRADE,
		.mask = GENMASK(1, 0),
		.bit = 1,
		.mode = 0644,
	},
	{
		.label = "asic_health",
		.reg = NVSW_REG_ASIC1_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.bit = 1,
		.mode = 0444,
	},
	{
		.label = "asic2_health",
		.reg = NVSW_REG_ASIC2_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.bit = 1,
		.mode = 0444,
	},
	{
		.label = "clk_brd1_boot_fail",
		.reg = NVSW_REG_GP4_RO_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0444,
	},
	{
		.label = "clk_brd2_boot_fail",
		.reg = NVSW_REG_GP4_RO_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0444,
	},
	{
		.label = "clk_brd_fail",
		.reg = NVSW_REG_GP4_RO_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0444,
	},
	{
		.label = "asic_pg_fail",
		.reg = NVSW_REG_GP4_RO_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(7),
		.mode = 0444,
	},
	{
		.label = "spi_chnl_select",
		.reg = NVSW_REG_SPI_CHNL_SELECT,
		.mask = GENMASK(7, 0),
		.bit = 1,
		.mode = 0644,
	},
	{
		.label = "config1",
		.reg = NVSW_REG_CONFIG1_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "config2",
		.reg = NVSW_REG_CONFIG2_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "config3",
		.reg = NVSW_REG_CONFIG3_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "sgmii_phy",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0444,
	},
	{
		.label = "graseful_pwr_off",
		.reg = NVSW_REG_GP7_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0644,
	},
	{
		.label = "power_button_evt",
		.reg = NVSW_REG_PWRB_EVENT_OFFSET,
		.mask = GENMASK(7, 0) & ~NVSW_PWR_BUTTON_MASK,
		.mode = 0644,
	},
	{
		.label = "power_button_mask",
		.reg = NVSW_REG_PWRB_MASK_OFFSET,
		.mask = GENMASK(7, 0) & ~NVSW_PWR_BUTTON_MASK,
		.mode = 0644,
	},
	{
		.label = "amb_sens",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0444,
	},
};

static struct mlxreg_core_platform_data nvsw_host_l1_regs_io = {
	.data = nvsw_host_l1_regs_io_data,
	.counter = ARRAY_SIZE(nvsw_host_l1_regs_io_data),
};

/* Platform led data for L1 switch systems with liquid cooling (without FANs) */
static struct mlxreg_core_data nvsw_host_l1_liquid_coling_led_data[] = {
	{
		.label = "status:green",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK
	},
	{
		.label = "status:amber",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK
	},
	{
		.label = "power:green",
		.mode = 0444,
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_HI_NIBBLE_MASK,
	},
	{
		.label = "power:amber",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_HI_NIBBLE_MASK,
	},
	{
		.label = "uid:blue",
		.reg = NVSW_REG_LED5_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
};

static struct mlxreg_core_platform_data nvsw_host_l1_liquid_coling_led = {
		.data = nvsw_host_l1_liquid_coling_led_data,
		.counter = ARRAY_SIZE(nvsw_host_l1_liquid_coling_led_data),
};

/* Watchdog type3 platform data */
static struct mlxreg_core_data nvsw_host_wd_main_regs_type3[] = {
	{
		.label = "action",
		.reg = NVSW_REG_WD2_ACT_OFFSET,
		.mask = NVSW_WD_RESET_ACT_MASK,
		.bit = 0,
	},
	{
		.label = "timeout",
		.reg = NVSW_REG_WD2_TMR_OFFSET,
		.mask = NVSW_WD_TYPE2_TO_MASK,
		.health_cntr = NVSW_WD3_DFLT_TIMEOUT,
	},
	{
		.label = "timeleft",
		.reg = NVSW_REG_WD2_TMR_OFFSET,
		.mask = NVSW_WD_TYPE2_TO_MASK,
	},
	{
		.label = "ping",
		.reg = NVSW_REG_WD2_ACT_OFFSET,
		.mask = NVSW_WD_RESET_ACT_MASK,
		.bit = 0,
	},
	{
		.label = "reset",
		.reg = NVSW_REG_RESET_CAUSE_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.bit = 6,
	},
};

static struct mlxreg_core_data nvsw_host_wd_aux_regs_type3[] = {
	{
		.label = "action",
		.reg = NVSW_REG_WD3_ACT_OFFSET,
		.mask = NVSW_WD_FAN_ACT_MASK,
		.bit = 4,
	},
	{
		.label = "timeout",
		.reg = NVSW_REG_WD3_TMR_OFFSET,
		.mask = NVSW_WD_TYPE2_TO_MASK,
		.health_cntr = NVSW_WD3_DFLT_TIMEOUT,
	},
	{
		.label = "timeleft",
		.reg = NVSW_REG_WD3_TMR_OFFSET,
		.mask = NVSW_WD_TYPE2_TO_MASK,
	},
	{
		.label = "ping",
		.reg = NVSW_REG_WD3_ACT_OFFSET,
		.mask = NVSW_WD_FAN_ACT_MASK,
		.bit = 4,
	},
};

static struct mlxreg_core_platform_data nvsw_host_wd_set_type3[] = {
	{
		.data = nvsw_host_wd_main_regs_type3,
		.counter = ARRAY_SIZE(nvsw_host_wd_main_regs_type3),
		.version = MLX_WDT_TYPE3,
		.identity = "mlx-wdt-main",
	},
	{
		.data = nvsw_host_wd_aux_regs_type3,
		.counter = ARRAY_SIZE(nvsw_host_wd_aux_regs_type3),
		.version = MLX_WDT_TYPE3,
		.identity = "mlx-wdt-aux",
	},
};

/* IO port mapping callback. */
static void __iomem *nvsw_host_l1_port_map(struct nvsw_core *nvsw_core)
{
	return devm_ioport_map(nvsw_core->dev, nvsw_host_io_resources[1].start, 1);
}

/* Mux init/exit callbacks. */
static int nvsw_host_l1_mux_topology_init(struct nvsw_core *nvsw_core)
{
	int i, err;

	/* Create mux infrastructure. */
	for (i = 0; i < nvsw_core->mux_num; i++) {
		nvsw_core->mux[i] =
			platform_device_register_resndata(nvsw_core->dev, "i2c-mux-reg", i, NULL,
							  0, nvsw_host_mux_data[i],
							  sizeof(*nvsw_host_mux_data[i]));
		if (IS_ERR(nvsw_core->mux[i])) {
			dev_err(nvsw_core->dev, "Failed to create mux infra\n");
			err = PTR_ERR(nvsw_core->mux[i]);
			goto fail_platform_mux_register;
		}
	}

	return 0;
fail_platform_mux_register:
	while (--i >= 0)
		platform_device_unregister(nvsw_core->mux[i]);
	return err;
}

static void nvsw_host_l1_mux_topology_exit(struct nvsw_core *nvsw_core)
{
	int i;

	for (i = 0; i < nvsw_core->mux_num; i++) {
		if (nvsw_core->mux[i])
			platform_device_unregister(nvsw_core->mux[i]);
	}
}

static int __init nvsw_host_register_platform_device(void)
{
	nvsw_host_dev = platform_device_register_simple(NVSW_HOST_DEVICE_NAME, -1,
							nvsw_host_io_resources,
							ARRAY_SIZE(nvsw_host_io_resources));
	if (IS_ERR(nvsw_host_dev))
		return PTR_ERR(nvsw_host_dev);
	return 1;
}

static int __init nvsw_host_dmi_l1_switch_matched(const struct dmi_system_id *dmi)
{
	int i;

	/* Set system configuration. */
	nvsw_host_hid = HID180;
	mux_num = ARRAY_SIZE(nvsw_host_l1_mux_data);
	for (i = 0; i < mux_num; i++)
		nvsw_host_mux_data[i] = &nvsw_host_l1_mux_data[i];
	nvsw_led_data = &nvsw_host_l1_liquid_coling_led;
	nvsw_regs_io_data = &nvsw_host_l1_regs_io;
	for (i = 0; i < ARRAY_SIZE(nvsw_host_wd_set_type3); i++)
		nvsw_wd_data[i] = &nvsw_host_wd_set_type3[i];

	return nvsw_host_register_platform_device();
}

static const struct dmi_system_id nvsw_host_dmi_table[] __initconst = {
	{
		.callback = nvsw_host_dmi_l1_switch_matched,
		.matches = {
			DMI_MATCH(DMI_BOARD_NAME, "VMOD0023"),
			DMI_EXACT_MATCH(DMI_PRODUCT_SKU, "HI180"),
		},
	},
	{ }
};

MODULE_DEVICE_TABLE(dmi, nvsw_host_dmi_table);

static int nvsw_host_probe(struct platform_device *pdev)
{
	struct nvsw_core *nvsw_core;
	int i;

	nvsw_core = devm_kzalloc(&nvsw_host_dev->dev, sizeof(*nvsw_core), GFP_KERNEL);
	if (!nvsw_core)
		return -ENOMEM;

	/* Set system configuration. */
	nvsw_core->dev = &nvsw_host_dev->dev;
	nvsw_core->hid = nvsw_host_hid;
	nvsw_core->regmap_type = REGMAP_IO;
	nvsw_core->mux_num = mux_num;
	for (i = 0; i < ARRAY_SIZE(nvsw_wd_data); i++)
		nvsw_core->wd_data[i] = nvsw_wd_data[i];
	nvsw_core->regio_data = nvsw_regs_io_data;
	nvsw_core->led_data = nvsw_led_data;
	nvsw_core->port_map = nvsw_host_l1_port_map;
	nvsw_core->mux_init = nvsw_host_l1_mux_topology_init;
	nvsw_core->mux_exit = nvsw_host_l1_mux_topology_exit;
	platform_set_drvdata(nvsw_host_dev, nvsw_core);

	return nvsw_core_init(nvsw_core);
}

static int nvsw_host_remove(struct platform_device *pdev)
{
	struct nvsw_core *nvsw_core = platform_get_drvdata(nvsw_host_dev);

	nvsw_core_exit(nvsw_core);
	return 0;
}

static struct platform_driver nvsw_host_driver = {
	.driver		= {
		.name	= NVSW_HOST_DEVICE_NAME,
		.probe_type = PROBE_FORCE_SYNCHRONOUS,
	},
	.probe		= nvsw_host_probe,
	.remove		= nvsw_host_remove,
};

static int __init nvsw_host_init(void)
{
	if (!dmi_check_system(nvsw_host_dmi_table))
		return -ENODEV;

	return platform_driver_register(&nvsw_host_driver);
}
module_init(nvsw_host_init);

static void __exit nvsw_host_exit(void)
{
	if (nvsw_host_dev)
		platform_device_unregister(nvsw_host_dev);

	platform_driver_unregister(&nvsw_host_driver);
}
module_exit(nvsw_host_exit);

MODULE_AUTHOR("Vadim Pasternak <vadimp@mellanox.com>");
MODULE_DESCRIPTION("Nvidia platform driver");
MODULE_LICENSE("Dual BSD/GPL");

