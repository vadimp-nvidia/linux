// SPDX-License-Identifier: GPL-2.0+
/*
 * Nvidia BMC platform driver
 *
 * Copyright (C) 2025 Nvidia Technologies Ltd.
 */

#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/platform_data/i2c-mux-regmap.h>
#include <linux/platform_data/mlxreg.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>

#include "nvsw.h"

#define NVSW_HID162_TACHO_SAMPLES	20
#define NVSW_HID162_TACHO_DIV	1981

/* Configuration for the register map of a device with 2 bytes address space. */
static const struct reg_default nvsw_bmc_hid162_reg_def[] = {
	{ NVSW_REG_PWM_CONTROL_OFFSET, 0x00 },
};

/* Channels vectors.
 * They contain only the channels, which physically connected to the devices,
 * empty channels are skipped.
 */
static int nvsw_bmc_hid162_chan1[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x11, 0x14, 0x18, 0x20, 0x21, 0x30, 0x31,
	0x32, 0x33, 0x34, 0x35, 0x24, 0x0a,
};

static int nvsw_bmc_hid176_chan1[] = {
	0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x08, 0x09, 0x11, 0x14, 0x18, 0x20, 0x21, 0x30, 0x31,
	0x32, 0x33, 0x34, 0x35, 0x24, 0x0a, 0x22,
};

/* Mux configuration. */
static struct i2c_mux_regmap_platform_data nvsw_bmc_hid162_mux_data[] = {
	{
		.parent = 14,
		.chan_ids = nvsw_bmc_hid162_chan1,
		.num_adaps = ARRAY_SIZE(nvsw_bmc_hid162_chan1),
		.sel_reg_addr = NVSW_REG_MUX1_OFFSET,
		.reg_size = 1,
	},
};

static struct i2c_mux_regmap_platform_data nvsw_bmc_hid176_mux_data[] = {
	{
		.parent = 14,
		.chan_ids = nvsw_bmc_hid176_chan1,
		.num_adaps = ARRAY_SIZE(nvsw_bmc_hid176_chan1),
		.sel_reg_addr = NVSW_REG_MUX1_OFFSET,
		.reg_size = 1,
	},
};

static struct i2c_mux_regmap_platform_data *mux_data[NVSW_MUX_MAX];
static struct i2c_board_info *mux_brdinfo[NVSW_MUX_MAX];

/* Mux board info. */
static struct i2c_board_info nvsw_bmc_hid162_mux_brdinfo = {
	I2C_BOARD_INFO("i2c-mux-mlxcpld", 0x32),
};

/* Platform hotplug data  */
static struct mlxreg_core_data nvsw_bmc_hid162_events_items_data[] = {
	{
		.label = "power_button",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_PWR_BUTTON_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "amb_temp_sense",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_AMB_TEMP_SENSE_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "graceful_power_off_req",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_GRACEFUL_POWER_OFF_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "cpu_power_off_ready",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_CPU_POWER_OFF_READY_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "cpu_reset",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_CPU_RESET_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "apml_smb_alert",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_APML_SMB_ALERT_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "cpu_unexp_power_off",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_CPU_UNEXP_POWER_OFF_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "uid_push_button",
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_UID_PUSH_BUTTON_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid162_asic1_items_data[] = {
	{
		.label = "asic1_health",
		.reg = NVSW_REG_ASIC1_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid162_asic2_items_data[] = {
	{
		.label = "asic2_health",
		.reg = NVSW_REG_ASIC2_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid177_asic3_items_data[] = {
	{
		.label = "asic3_health",
		.reg = NVSW_REG_ASIC3_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid162_cartridge_items_data[] = {
	{
		.label = "cartridge1",
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = BIT(0),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "cartridge2",
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = BIT(1),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "cartridge3",
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = BIT(2),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "cartridge4",
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = BIT(3),
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid162_leakage_items_data[] = {
	{
		.label = "leakage1",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = BIT(0),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "leakage2",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = BIT(1),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "leakage3",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = BIT(2),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "leakage4",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = BIT(3),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "leakage5",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = BIT(4),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "leakage6",
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = BIT(5),
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid162_pwr_good_items_data[] = {
	{
		.label = "rtc",
		.reg = NVSW_REG_BRD4_OFFSET,
		.mask = NVSW_RTC_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "hot_swap_alert",
		.reg = NVSW_REG_BRD4_OFFSET,
		.mask = NVSW_HOT_SWAP_ALERT_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid162_alarms_items_data[] = {
	{
		.label = "ssd_i2c_alert",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_SSD_I2C_ALERT_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "wd_exp",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_WD_EXP_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "5v_usb",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_5V_USB_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "pcb_temp_sense_1",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_PCB_TEMP1_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "pcb_temp_sense_2",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_PCB_TEMP2_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "sgmii_phy",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_SGMII_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "ssd_pw_good",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_SDD_PG_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "leakage_aggr",
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_LEAK_AGGR_MASK,
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid162_erot_ap_items_data[] = {
	{
		.label = "erot_asic1_ap",
		.reg = NVSW_REG_EROT_OFFSET,
		.mask = BIT(0),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "erot_asic2_ap",
		.reg = NVSW_REG_EROT_OFFSET,
		.mask = BIT(1),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "erot_cpu_ap",
		.reg = NVSW_REG_EROT_OFFSET,
		.mask = BIT(2),
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid162_erot_error_items_data[] = {
	{
		.label = "erot_asic1_error",
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = BIT(0),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "erot_asic2_error",
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = BIT(1),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "erot_cpu_error",
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = BIT(2),
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_data nvsw_bmc_hid176_erot_error_items_data[] = {
	{
		.label = "erot_cpu_error",
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = BIT(0),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "erot_asic1_error",
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = BIT(1),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "erot_asic2_error",
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = BIT(2),
		.hpdev.nr = NVSW_NR_NONE,
	},
	{
		.label = "erot_asic3_error",
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = BIT(3),
		.hpdev.nr = NVSW_NR_NONE,
	},
};

static struct mlxreg_core_item nvsw_bmc_hid162_hotplug_items_data[] = {
	{
		.data = nvsw_bmc_hid162_events_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_PWR_BUTTON_MASK | NVSW_AMB_TEMP_SENSE_MASK |
			NVSW_GRACEFUL_POWER_OFF_MASK | NVSW_CPU_POWER_OFF_READY_MASK |
			NVSW_CPU_RESET_MASK | NVSW_APML_SMB_ALERT_MASK |
			NVSW_CPU_UNEXP_POWER_OFF_MASK | NVSW_UID_PUSH_BUTTON_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_events_items_data),
		.inversed = 1,
		.health = false,
		.non_sticky = true,
	},
	{
		.data = nvsw_bmc_hid162_asic1_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_ASIC1_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_asic1_items_data),
		.inversed = 0,
		.health = true,
	},
	{
		.data = nvsw_bmc_hid162_asic2_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_ASIC2_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_asic2_items_data),
		.inversed = 0,
		.health = true,
	},
	{
		.data = nvsw_bmc_hid162_cartridge_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_FRU1_OFFSET,
		.mask = NVSW_REG_FRU1_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_cartridge_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_leakage_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = NVSW_LEAK_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_leakage_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_pwr_good_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_BRD4_OFFSET,
		.mask = NVSW_RTC_MASK | NVSW_HOT_SWAP_ALERT_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_pwr_good_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_alarms_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_SSD_I2C_ALERT_MASK | NVSW_WD_EXP_MASK |
			NVSW_5V_USB_MASK | NVSW_PCB_TEMP1_MASK |
			NVSW_PCB_TEMP2_MASK | NVSW_SGMII_MASK |
			NVSW_SDD_PG_MASK | NVSW_LEAK_AGGR_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_alarms_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_erot_ap_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_EROT_OFFSET,
		.mask = NVSW_EROT_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_erot_ap_items_data),
		.inversed = 1,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_erot_error_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = NVSW_EROT_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_erot_error_items_data),
		.inversed = 1,
		.health = false,
	},
};

static
struct mlxreg_core_hotplug_platform_data nvsw_bmc_hid162_hotplug = {
	.items = nvsw_bmc_hid162_hotplug_items_data,
	.count = ARRAY_SIZE(nvsw_bmc_hid162_hotplug_items_data),
	.cell = NVSW_REG_AGGR_OFFSET,
	.mask = NVSW_AGGR_MASK | NVSW_AGGR_MASK_COMEX,
	.cell_low = NVSW_REG_AGGRLO_OFFSET,
	.mask_low = NVSW_LOW_AGGR_MASK_LOW | NVSW_LOW_AGGR_MASK_ASIC2 |
		    NVSW_LOW_AGGR_MASK_ASIC1,
};

static struct mlxreg_core_item nvsw_bmc_hid176_hotplug_items_data[] = {
	{
		.data = nvsw_bmc_hid162_events_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_PWR_BUTTON_MASK | NVSW_AMB_TEMP_SENSE_MASK |
			NVSW_GRACEFUL_POWER_OFF_MASK | NVSW_CPU_POWER_OFF_READY_MASK |
			NVSW_CPU_RESET_MASK | NVSW_APML_SMB_ALERT_MASK |
			NVSW_CPU_UNEXP_POWER_OFF_MASK | NVSW_UID_PUSH_BUTTON_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_events_items_data),
		.inversed = 1,
		.health = false,
		.non_sticky = true,
	},
	{
		.data = nvsw_bmc_hid162_asic1_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_ASIC1_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_asic1_items_data),
		.inversed = 0,
		.health = true,
	},
	{
		.data = nvsw_bmc_hid162_asic2_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_ASIC2_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_asic2_items_data),
		.inversed = 0,
		.health = true,
	},
	{
		.data = nvsw_bmc_hid162_leakage_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = NVSW_LEAK_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_leakage_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_pwr_good_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_BRD4_OFFSET,
		.mask = NVSW_RTC_MASK | NVSW_HOT_SWAP_ALERT_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_pwr_good_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_alarms_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_SSD_I2C_ALERT_MASK | NVSW_WD_EXP_MASK |
			NVSW_5V_USB_MASK | NVSW_PCB_TEMP1_MASK |
			NVSW_PCB_TEMP2_MASK | NVSW_SGMII_MASK |
			NVSW_SDD_PG_MASK | NVSW_LEAK_AGGR_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_alarms_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid176_erot_error_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = NVSW_EROT_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid176_erot_error_items_data),
		.inversed = 1,
		.health = false,
	},
};

static
struct mlxreg_core_hotplug_platform_data nvsw_bmc_hid176_hotplug = {
	.items = nvsw_bmc_hid176_hotplug_items_data,
	.count = ARRAY_SIZE(nvsw_bmc_hid176_hotplug_items_data),
	.cell = NVSW_REG_AGGR_OFFSET,
	.mask = NVSW_AGGR_MASK | NVSW_AGGR_MASK_COMEX,
	.cell_low = NVSW_REG_AGGRLO_OFFSET,
	.mask_low = NVSW_LOW_AGGR_MASK_LOW | NVSW_LOW_AGGR_MASK_ASIC2 |
		    NVSW_LOW_AGGR_MASK_ASIC1,
};

static struct mlxreg_core_item nvsw_bmc_hid177_hotplug_items_data[] = {
	{
		.data = nvsw_bmc_hid162_events_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_PWRB_OFFSET,
		.mask = NVSW_PWR_BUTTON_MASK | NVSW_AMB_TEMP_SENSE_MASK |
			NVSW_GRACEFUL_POWER_OFF_MASK | NVSW_CPU_POWER_OFF_READY_MASK |
			NVSW_CPU_RESET_MASK | NVSW_APML_SMB_ALERT_MASK |
			NVSW_CPU_UNEXP_POWER_OFF_MASK | NVSW_UID_PUSH_BUTTON_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_events_items_data),
		.inversed = 1,
		.health = false,
		.non_sticky = true,
	},
	{
		.data = nvsw_bmc_hid162_asic1_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_ASIC1_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_asic1_items_data),
		.inversed = 0,
		.health = true,
	},
	{
		.data = nvsw_bmc_hid162_asic2_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_ASIC2_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_asic2_items_data),
		.inversed = 0,
		.health = true,
	},
	{
		.data = nvsw_bmc_hid177_asic3_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_ASIC3_HEALTH_OFFSET,
		.mask = NVSW_ASIC_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid177_asic3_items_data),
		.inversed = 0,
		.health = true,
	},
	{
		.data = nvsw_bmc_hid162_leakage_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_LEAK_OFFSET,
		.mask = NVSW_LEAK_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_leakage_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_pwr_good_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_BRD4_OFFSET,
		.mask = NVSW_RTC_MASK | NVSW_HOT_SWAP_ALERT_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_pwr_good_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid162_alarms_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_BRD1_OFFSET,
		.mask = NVSW_SSD_I2C_ALERT_MASK | NVSW_WD_EXP_MASK |
			NVSW_5V_USB_MASK | NVSW_PCB_TEMP1_MASK |
			NVSW_PCB_TEMP2_MASK | NVSW_SGMII_MASK |
			NVSW_SDD_PG_MASK | NVSW_LEAK_AGGR_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid162_alarms_items_data),
		.inversed = 0,
		.health = false,
	},
	{
		.data = nvsw_bmc_hid176_erot_error_items_data,
		.aggr_mask = NVSW_AGGR_MASK,
		.reg = NVSW_REG_EROT_ERR_OFFSET,
		.mask = NVSW_EROT_MASK,
		.count = ARRAY_SIZE(nvsw_bmc_hid176_erot_error_items_data),
		.inversed = 1,
		.health = false,
	},
};

static
struct mlxreg_core_hotplug_platform_data nvsw_bmc_hid177_hotplug = {
	.items = nvsw_bmc_hid177_hotplug_items_data,
	.count = ARRAY_SIZE(nvsw_bmc_hid177_hotplug_items_data),
	.cell = NVSW_REG_AGGR_OFFSET,
	.mask = NVSW_AGGR_MASK | NVSW_AGGR_MASK_COMEX,
	.cell_low = NVSW_REG_AGGRLO_OFFSET,
	.mask_low = NVSW_LOW_AGGR_MASK_LOW | NVSW_LOW_AGGR_MASK_ASIC3 |
		    NVSW_LOW_AGGR_MASK_ASIC2 | NVSW_LOW_AGGR_MASK_ASIC1,
};

/* Platform register access data. */
static struct mlxreg_core_data nvsw_bmc_hid162_regio_data[] = {
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
		.label = "cpld3_version",
		.reg = NVSW_REG_CPLD3_VER_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "cpld4_version",
		.reg = NVSW_REG_CPLD4_VER_OFFSET,
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
		.label = "cpld3_pn",
		.reg = NVSW_REG_CPLD3_PN_OFFSET,
		.bit = GENMASK(15, 0),
		.mode = 0444,
		.regnum = 2,
	},
	{
		.label = "cpld4_pn",
		.reg = NVSW_REG_CPLD4_PN_OFFSET,
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
		.label = "cpld3_version_min",
		.reg = NVSW_REG_CPLD3_MVER_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "cpld4_version_min",
		.reg = NVSW_REG_CPLD4_MVER_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "fan_dir",
		.reg = NVSW_REG_GP0_RO_OFFSET,
		.mask = GENMASK(7, 0),
		.mode = 0444,
	},
	{
		.label = "fan_present",
		.reg = NVSW_REG_FAN_OFFSET,
		.mask = GENMASK(7, 0),
		.mode = 0444,
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
		.mode = 0644,
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
		.label = "ignore_next_reset",
		.reg = NVSW_REG_GP0_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(7),
		.mode = 0644,
	},
	{
		.label = "leakage_conn_en",
		.reg = NVSW_REG_GP6_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(0),
		.mode = 0644,
	},
	{
		.label = "bmc_reset_reg",
		.reg = NVSW_REG_GP6_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0644,
	},
	{
		.label = "spi_chnl_select",
		.reg = NVSW_REG_SPI_CHNL_SELECT,
		.mask = GENMASK(7, 0),
		.bit = 1,
		.mode = 0644,
	},
	{
		.label = "pwr_converter_prog_en",
		.reg = NVSW_REG_GP7_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(0),
		.mode = 0644,
		.secured = 1,
	},
	{
		.label = "graceful_power_off",
		.reg = NVSW_REG_GP7_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0644,
	},
	{
		.label = "bmc_perst_en",
		.reg = NVSW_REG_GP7_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0200,
	},
	{
		.label = "bmc_shutdown_unlock",
		.reg = NVSW_REG_GP7_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0200,
	},
	{
		.label = "platform_reset",
		.reg = NVSW_REG_RESET_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(0),
		.mode = 0200,
	},
	{
		.label = "main_brd_reset",
		.reg = NVSW_REG_RESET_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(2),
		.mode = 0200,
	},
	{
		.label = "nic_reset",
		.reg = NVSW_REG_RESET_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0200,
	},
	{
		.label = "tpm_reset",
		.reg = NVSW_REG_RESET_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0200,
	},
	{
		.label = "erot_asic3_reset",
		.reg = NVSW_REG_RESET_GP2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(2),
		.mode = 0200,
	},
	{
		.label = "asics_reset",
		.reg = NVSW_REG_RESET_GP2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0200,
	},
	{
		.label = "sgmii_phy_reset",
		.reg = NVSW_REG_RESET_GP2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0200,
	},
	{
		.label = "erot_cpu_reset",
		.reg = NVSW_REG_RESET_GP2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0200,
	},
	{
		.label = "erot_asic1_reset",
		.reg = NVSW_REG_RESET_GP2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0200,
	},
	{
		.label = "erot_asic2_reset",
		.reg = NVSW_REG_RESET_GP2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(7),
		.mode = 0200,
	},
	{
		.label = "jtag_enable",
		.reg = NVSW_REG_FIELD_UPGRADE,
		.mask = GENMASK(1, 0),
		.bit = 1,
		.mode = 0644,
	},
	{
		.label = "non_active_bios_select",
		.reg = NVSW_REG_SAFE_BIOS_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0644,
	},
	{
		.label = "bios_upgrade_fail",
		.reg = NVSW_REG_SAFE_BIOS_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0444,
	},
	{
		.label = "bios_image_invert",
		.reg = NVSW_REG_SAFE_BIOS_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0644,
	},
	{
		.label = "erot_asic3_recovery",
		.reg = NVSW_REG_PWM_CONTROL_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(4),
		.mode = 0644,
	},
	{
		.label = "erot_cpu_recovery",
		.reg = NVSW_REG_PWM_CONTROL_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(5),
		.mode = 0644,
	},
	{
		.label = "erot_asic1_recovery",
		.reg = NVSW_REG_PWM_CONTROL_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0644,
	},
	{
		.label = "erot_asic2_recovery",
		.reg = NVSW_REG_PWM_CONTROL_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(7),
		.mode = 0644,
	},
	{
		.label = "pwr_button_halt",
		.reg = NVSW_REG_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0644,
	},
	{
		.label = "pwr_cycle",
		.reg = NVSW_REG_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(2),
		.mode = 0200,
	},
	{
		.label = "pwr_down",
		.reg = NVSW_REG_GP1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(3),
		.mode = 0644,
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
		.label = "reset_swb_pwr_fail",
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
		.label = "reset_from_carrier",
		.reg = NVSW_REG_RESET_CAUSE1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(1),
		.mode = 0444,
	},
	{
		.label = "reset_aux_pwr_or_reload",
		.reg = NVSW_REG_RESET_CAUSE1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(2),
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
		.label = "reset_from_erot",
		.reg = NVSW_REG_RESET_CAUSE1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(6),
		.mode = 0444,
	},
	{
		.label = "reset_pwr",
		.reg = NVSW_REG_RESET_CAUSE1_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(7),
		.mode = 0444,
	},
	{
		.label = "reset_erot",
		.reg = NVSW_REG_RESET_CAUSE2_OFFSET,
		.mask = GENMASK(7, 0) & ~BIT(0),
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
		.label = "bios_status",
		.reg = NVSW_REG_GPCOM0_OFFSET,
		.mask = NVSW_BIOS_STATUS_MASK,
		.bit = 2,
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
		.label = "ufm_version",
		.reg = NVSW_REG_UFM_VERSION_OFFSET,
		.bit = GENMASK(7, 0),
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
		.label = "geo_addr",
		.reg = NVSW_REG_CONFIG2_OFFSET,
		.bit = GENMASK(7, 0),
		.mode = 0444,
	},
};

static struct mlxreg_core_platform_data nvsw_bmc_hid162_regio = {
		.data = nvsw_bmc_hid162_regio_data,
		.counter = ARRAY_SIZE(nvsw_bmc_hid162_regio_data),
};

/* Platform fan data. */
static struct mlxreg_core_data nvsw_bmc_hid162_fan_data[] = {
	{
		.label = "pwm1",
		.reg = NVSW_REG_PWM1_OFFSET,
	},
	{
		.label = "tacho1",
		.reg = NVSW_REG_TACHO1_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 1,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho2",
		.reg = NVSW_REG_TACHO2_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 2,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho3",
		.reg = NVSW_REG_TACHO3_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 3,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho4",
		.reg = NVSW_REG_TACHO4_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 4,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho5",
		.reg = NVSW_REG_TACHO5_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 5,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho6",
		.reg = NVSW_REG_TACHO6_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 6,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho7",
		.reg = NVSW_REG_TACHO7_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 7,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho8",
		.reg = NVSW_REG_TACHO8_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 8,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho9",
		.reg = NVSW_REG_TACHO9_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 9,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho10",
		.reg = NVSW_REG_TACHO10_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 10,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho11",
		.reg = NVSW_REG_TACHO11_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 11,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "tacho12",
		.reg = NVSW_REG_TACHO12_OFFSET,
		.mask = GENMASK(7, 0),
		.capability = NVSW_REG_FAN_CAP1_OFFSET,
		.slot = 12,
		.reg_prsnt = NVSW_REG_FAN_OFFSET,
	},
	{
		.label = "conf",
		.mask = NVSW_HID162_TACHO_SAMPLES,
		.bit = NVSW_HID162_TACHO_DIV,
		.capability = NVSW_REG_TACHO_SPEED_OFFSET,
	},
};

static struct mlxreg_core_platform_data nvsw_bmc_hid162_fan = {
		.data = nvsw_bmc_hid162_fan_data,
		.counter = ARRAY_SIZE(nvsw_bmc_hid162_fan_data),
		.capability = NVSW_REG_FAN_DRW_CAP_OFFSET,
		.version = 1,
};

/* Platform led data for HI162 system type. */
static struct mlxreg_core_data nvsw_bmc_hid162_led_data[] = {
	{
		.label = "status:green",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
	{
		.label = "status:amber",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
	{
		.label = "power:green",
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
	{
		.label = "fan:green",
		.reg = NVSW_REG_LED6_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
	{
		.label = "fan:amber",
		.reg = NVSW_REG_LED6_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
};

static struct mlxreg_core_platform_data nvsw_bmc_hid162_led = {
		.data = nvsw_bmc_hid162_led_data,
		.counter = ARRAY_SIZE(nvsw_bmc_hid162_led_data),
};

/* Platform led data for HI1676 system type. */
static struct mlxreg_core_data nvsw_bmc_hid176_led_data[] = {
	{
		.label = "status:green",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
	{
		.label = "status:amber",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
	{
		.label = "power:green",
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
	{
		.label = "leakage:green",
		.reg = NVSW_REG_LED7_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
	{
		.label = "leakage:amber",
		.reg = NVSW_REG_LED7_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
};

static struct mlxreg_core_platform_data nvsw_bmc_hid176_led = {
		.data = nvsw_bmc_hid176_led_data,
		.counter = ARRAY_SIZE(nvsw_bmc_hid176_led_data),
};

/* Platform led data for HI1676 system type. */
static struct mlxreg_core_data nvsw_bmc_hid177_led_data[] = {
	{
		.label = "status:green",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
	{
		.label = "status:amber",
		.reg = NVSW_REG_LED1_OFFSET,
		.mask = NVSW_LED_LO_NIBBLE_MASK,
	},
	{
		.label = "power:green",
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

static struct mlxreg_core_platform_data nvsw_bmc_hid177_led = {
		.data = nvsw_bmc_hid177_led_data,
		.counter = ARRAY_SIZE(nvsw_bmc_hid177_led_data),
};

/* Mux init/exit callbacks. */
static int nvsw_bmc_hid162_mux_topology_init(struct nvsw_core *nvsw_core)
{
	int i, err;

	/* Create mux infrastructure. */
	for (i = 0; i < nvsw_core->mux_num; i++) {
		mux_data[i]->regmap = nvsw_core->regmap;
		nvsw_core->mux[i] = platform_device_register_resndata(nvsw_core->dev,
								      "i2c-mux-regmap", i, NULL, 0,
								      mux_data[i],
								      sizeof(*mux_data[i]));
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

static void nvsw_bmc_hid162_mux_topology_exit(struct nvsw_core *nvsw_core)
{
	int i;

	for (i = 0; i < nvsw_core->mux_num; i++) {
		if (nvsw_core->mux[i])
			platform_device_unregister(nvsw_core->mux[i]);
	}
}

/* Callbact to set initial values for specific registers. */
static int nvsw_bmc_hid162_set_reg_default(struct regmap *regmap)
{
	u32 regval;
	int err;

	err = regmap_read(regmap, NVSW_REG_GP6_OFFSET, &regval);
	if (err)
		return err;

	return regmap_write(regmap, NVSW_REG_GP6_OFFSET, regval | NVSW_REG_RESET_MASK);
}

/* Callback is used to indicate that all adapter devices has been created. */
static int
nvsw_bmc_hid162_completion_notify(void *handle, struct i2c_adapter *parent,
				  struct i2c_adapter *adapters[])
{
	/* struct nvsw_core *nvsw_core = handle; */

	return 0;
}

static int nvsw_bmc_hid162_platform_data_init(struct nvsw_core *nvsw_core)
{
	int i;

	/* Set system configuration. */
	nvsw_core->hid = HID162;
	nvsw_core->mux_num = ARRAY_SIZE(nvsw_bmc_hid162_mux_data);
	for (i = 0; i < nvsw_core->mux_num; i++) {
		mux_data[i] = &nvsw_bmc_hid162_mux_data[i];
		mux_data[i]->handle = nvsw_core;
		mux_data[i]->completion_notify = nvsw_bmc_hid162_completion_notify;
		mux_brdinfo[i] = &nvsw_bmc_hid162_mux_brdinfo;
		mux_brdinfo[i]->platform_data = mux_data[i];
	}

	nvsw_core->regio_data = &nvsw_bmc_hid162_regio;
	nvsw_core->led_data = &nvsw_bmc_hid162_led;
	nvsw_core->fan_data = &nvsw_bmc_hid162_fan;
	nvsw_core->hotplug_data = &nvsw_bmc_hid162_hotplug;
	nvsw_core->mux_init = nvsw_bmc_hid162_mux_topology_init;
	nvsw_core->mux_exit = nvsw_bmc_hid162_mux_topology_exit;
	nvsw_core->set_reg_default = nvsw_bmc_hid162_set_reg_default;

	return 0;
}

static int nvsw_bmc_hid176_platform_data_init(struct nvsw_core *nvsw_core)
{
	int i;

	/* Set system configuration. */
	nvsw_core->hid = HID176;
	nvsw_core->mux_num = ARRAY_SIZE(nvsw_bmc_hid176_mux_data);
	for (i = 0; i < nvsw_core->mux_num; i++) {
		mux_data[i] = &nvsw_bmc_hid176_mux_data[i];
		mux_data[i]->handle = nvsw_core;
		mux_data[i]->completion_notify = nvsw_bmc_hid162_completion_notify;
		mux_brdinfo[i] = &nvsw_bmc_hid162_mux_brdinfo;
		mux_brdinfo[i]->platform_data = mux_data[i];
	}

	nvsw_core->regio_data = &nvsw_bmc_hid162_regio;
	nvsw_core->led_data = &nvsw_bmc_hid176_led;
	nvsw_core->hotplug_data = &nvsw_bmc_hid176_hotplug;
	nvsw_core->mux_init = nvsw_bmc_hid162_mux_topology_init;
	nvsw_core->mux_exit = nvsw_bmc_hid162_mux_topology_exit;
	nvsw_core->set_reg_default = nvsw_bmc_hid162_set_reg_default;

	return 0;
}

static int nvsw_bmc_hid177_platform_data_init(struct nvsw_core *nvsw_core)
{
	int i;

	/* Set system configuration. */
	nvsw_core->hid = HID177;
	nvsw_core->mux_num = ARRAY_SIZE(nvsw_bmc_hid176_mux_data);
	for (i = 0; i < nvsw_core->mux_num; i++) {
		mux_data[i] = &nvsw_bmc_hid176_mux_data[i];
		mux_data[i]->handle = nvsw_core;
		mux_data[i]->completion_notify = nvsw_bmc_hid162_completion_notify;
		mux_brdinfo[i] = &nvsw_bmc_hid162_mux_brdinfo;
		mux_brdinfo[i]->platform_data = mux_data[i];
	}

	nvsw_core->regio_data = &nvsw_bmc_hid162_regio;
	nvsw_core->led_data = &nvsw_bmc_hid177_led;
	nvsw_core->hotplug_data = &nvsw_bmc_hid177_hotplug;
	nvsw_core->mux_init = nvsw_bmc_hid162_mux_topology_init;
	nvsw_core->mux_exit = nvsw_bmc_hid162_mux_topology_exit;
	nvsw_core->set_reg_default = nvsw_bmc_hid162_set_reg_default;

	return 0;
}

static int nvsw_bmc_platform_data_init(struct nvsw_core *nvsw_core, enum nvsw_core_hid_type type)
{
	switch (type) {
	case HID162:
		return nvsw_bmc_hid162_platform_data_init(nvsw_core);
	case HID176:
		return nvsw_bmc_hid176_platform_data_init(nvsw_core);
	case HID177:
		return nvsw_bmc_hid177_platform_data_init(nvsw_core);
	default:
		return -ENODEV;
	}
}

static int nvsw_bmc_hid162_probe(struct i2c_client *client)
{
	struct device_node *np = client->dev.of_node;
	enum nvsw_core_hid_type type;
	struct nvsw_core *nvsw_core;
	int err;

	if (!np)
		return -ENODEV;

	if (of_device_is_compatible(np, "nvidia,hid162"))
		type = HID162;
	else if (of_device_is_compatible(np, "nvidia,hid176"))
		type = HID176;
	else if (of_device_is_compatible(np, "nvidia,hid177"))
		type = HID177;
	else
		return -ENODEV;

	nvsw_core = devm_kzalloc(&client->dev, sizeof(*nvsw_core), GFP_KERNEL);
	if (!nvsw_core)
		return -ENOMEM;

	nvsw_core->dev = &client->dev;
	nvsw_core->client = client;
	nvsw_core->np = np;
	nvsw_core->regmap_type = REGMAP_I2C;
	i2c_set_clientdata(client, nvsw_core);
	err = nvsw_bmc_platform_data_init(nvsw_core, type);
	if (!err)
		return err;

	return nvsw_core_init(nvsw_core);
}

static void nvsw_bmc_hid162_remove(struct i2c_client *client)
{
	struct nvsw_core *nvsw_core = i2c_get_clientdata(client);

	nvsw_core_exit(nvsw_core);
}

static const struct i2c_device_id nvsw_bmc_hid162_id[] = {
	{ "hid162", HID162 },
	{ "hid176", HID176 },
	{ "hid177", HID177 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, nvsw_bmc_hid162_id);

static const struct of_device_id nvsw_bmc_hid162_dt_match[] = {
	{ .compatible = "nvidia,hid162" },
	{ .compatible = "nvidia,hid176" },
	{ .compatible = "nvidia,hid177" },
	{ },
};
MODULE_DEVICE_TABLE(of, nvsw_bmc_hid162_dt_match);

static struct i2c_driver nvsw_bmc_hid162_driver = {
	.driver = {
	    .name = "nvsw-bmc-hid162",
	    .of_match_table = of_match_ptr(nvsw_bmc_hid162_dt_match),
	},
	.probe = nvsw_bmc_hid162_probe,
	.remove = nvsw_bmc_hid162_remove,
	.id_table = nvsw_bmc_hid162_id,
};

module_i2c_driver(nvsw_bmc_hid162_driver);

MODULE_AUTHOR("Vadim Pasternak <vadimp@mellanox.com>");
MODULE_DESCRIPTION("Nvidia platform driver");
MODULE_LICENSE("Dual BSD/GPL");
