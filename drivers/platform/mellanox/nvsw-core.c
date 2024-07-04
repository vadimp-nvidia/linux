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
#include <linux/platform_data/mlxreg.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include "nvsw.h"

static bool nvsw_writeable_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case NVSW_REG_PG_EVENT_OFFSET:
	case NVSW_REG_PG_MASK_OFFSET:
	case NVSW_REG_RESET_GP1_OFFSET:
	case NVSW_REG_FIELD_UPGRADE:
	case NVSW_REG_GP0_OFFSET:
	case NVSW_REG_GP1_OFFSET:
	case NVSW_REG_GP7_OFFSET:
	case NVSW_REG_PWM_CONTROL_OFFSET:
	case NVSW_REG_RESET_GP2_OFFSET:
	case NVSW_REG_GP4_OFFSET:
	case NVSW_REG_GP6_OFFSET:
	case NVSW_REG_LED1_OFFSET:
	case NVSW_REG_LED5_OFFSET:
	case NVSW_REG_LED6_OFFSET:
	case NVSW_REG_LED7_OFFSET:
	case NVSW_REG_AGGRCO_MASK_OFFSET:
	case NVSW_REG_HEALTH_EVENT_OFFSET:
	case NVSW_REG_HEALTH_MASK_OFFSET:
	case NVSW_REG_AGGR_MASK_OFFSET:
	case NVSW_REG_FU_CAP_OFFSET:
	case NVSW_REG_BRD4_EVENT_OFFSET:
	case NVSW_REG_BRD4_MASK_OFFSET:
	case NVSW_REG_AGGRLO_MASK_OFFSET:
	case NVSW_REG_BRD1_EVENT_OFFSET:
	case NVSW_REG_BRD1_MASK_OFFSET:
	case NVSW_REG_ASIC1_EVENT_OFFSET:
	case NVSW_REG_ASIC1_MASK_OFFSET:
	case NVSW_REG_ASIC2_EVENT_OFFSET:
	case NVSW_REG_ASIC2_MASK_OFFSET:
	case NVSW_REG_ASIC3_EVENT_OFFSET:
	case NVSW_REG_ASIC3_MASK_OFFSET:
	case NVSW_REG_FAN_EVENT_OFFSET:
	case NVSW_REG_FAN_MASK_OFFSET:
	case NVSW_REG_PWRB_EVENT_OFFSET:
	case NVSW_REG_PWRB_MASK_OFFSET:
	case NVSW_REG_EROT_EVENT_OFFSET:
	case NVSW_REG_EROT_MASK_OFFSET:
	case NVSW_REG_EROT_ERR_EVENT_OFFSET:
	case NVSW_REG_EROT_ERR_MASK_OFFSET:
	case NVSW_REG_FRU1_EVENT_OFFSET:
	case NVSW_REG_FRU1_MASK_OFFSET:
	case NVSW_REG_LEAK_EVENT_OFFSET:
	case NVSW_REG_LEAK_MASK_OFFSET:
	case NVSW_REG_SPI_CHNL_SELECT:
	case NVSW_REG_WD2_TMR_OFFSET:
	case NVSW_REG_WD2_TLEFT_OFFSET:
	case NVSW_REG_WD2_ACT_OFFSET:
	case NVSW_REG_WD3_TMR_OFFSET:
	case NVSW_REG_WD3_TLEFT_OFFSET:
	case NVSW_REG_WD3_ACT_OFFSET:
	case NVSW_REG_PWM1_OFFSET:
	case NVSW_REG_MUX0_OFFSET:
	case NVSW_REG_MUX1_OFFSET:
		return true;
	}
	return false;
}

static bool nvsw_readable_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case NVSW_REG_CPLD1_VER_OFFSET:
	case NVSW_REG_CPLD1_PN_OFFSET:
	case NVSW_REG_CPLD1_PN1_OFFSET:
	case NVSW_REG_CPLD2_VER_OFFSET:
	case NVSW_REG_CPLD2_PN_OFFSET:
	case NVSW_REG_CPLD2_PN1_OFFSET:
	case NVSW_REG_CPLD3_VER_OFFSET:
	case NVSW_REG_CPLD3_PN_OFFSET:
	case NVSW_REG_CPLD3_PN1_OFFSET:
	case NVSW_REG_CPLD4_VER_OFFSET:
	case NVSW_REG_CPLD4_PN_OFFSET:
	case NVSW_REG_CPLD4_PN1_OFFSET:
	case NVSW_REG_PG_OFFSET:
	case NVSW_REG_PG_EVENT_OFFSET:
	case NVSW_REG_PG_MASK_OFFSET:
	case NVSW_REG_RESET_GP1_OFFSET:
	case NVSW_REG_FIELD_UPGRADE:
	case NVSW_REG_SAFE_BIOS_OFFSET:
	case NVSW_REG_RESET_CAUSE_OFFSET:
	case NVSW_REG_RESET_CAUSE1_OFFSET:
	case NVSW_REG_RESET_CAUSE2_OFFSET:
	case NVSW_REG_LED1_OFFSET:
	case NVSW_REG_LED5_OFFSET:
	case NVSW_REG_LED6_OFFSET:
	case NVSW_REG_LED7_OFFSET:
	case NVSW_REG_RESET_GP2_OFFSET:
	case NVSW_REG_GP0_RO_OFFSET:
	case NVSW_REG_GP1_RO_OFFSET:
	case NVSW_REG_GPCOM0_OFFSET:
	case NVSW_REG_GP0_OFFSET:
	case NVSW_REG_GP1_OFFSET:
	case NVSW_REG_GP7_OFFSET:
	case NVSW_REG_PWM_CONTROL_OFFSET:
	case NVSW_REG_GP4_OFFSET:
	case NVSW_REG_GP6_OFFSET:
	case NVSW_REG_AGGRCO_OFFSET:
	case NVSW_REG_AGGRCO_MASK_OFFSET:
	case NVSW_REG_HEALTH_OFFSET:
	case NVSW_REG_HEALTH_EVENT_OFFSET:
	case NVSW_REG_HEALTH_MASK_OFFSET:
	case NVSW_REG_AGGR_OFFSET:
	case NVSW_REG_AGGR_MASK_OFFSET:
	case NVSW_REG_FU_CAP_OFFSET:
	case NVSW_REG_BRD4_OFFSET:
	case NVSW_REG_BRD4_EVENT_OFFSET:
	case NVSW_REG_BRD4_MASK_OFFSET:
	case NVSW_REG_AGGRLO_OFFSET:
	case NVSW_REG_AGGRLO_MASK_OFFSET:
	case NVSW_REG_BRD1_OFFSET:
	case NVSW_REG_BRD1_EVENT_OFFSET:
	case NVSW_REG_BRD1_MASK_OFFSET:
	case NVSW_REG_ASIC1_HEALTH_OFFSET:
	case NVSW_REG_ASIC1_EVENT_OFFSET:
	case NVSW_REG_ASIC1_MASK_OFFSET:
	case NVSW_REG_ASIC2_HEALTH_OFFSET:
	case NVSW_REG_ASIC2_EVENT_OFFSET:
	case NVSW_REG_ASIC2_MASK_OFFSET:
	case NVSW_REG_ASIC3_HEALTH_OFFSET:
	case NVSW_REG_ASIC3_EVENT_OFFSET:
	case NVSW_REG_ASIC3_MASK_OFFSET:
	case NVSW_REG_FAN_OFFSET:
	case NVSW_REG_FAN_EVENT_OFFSET:
	case NVSW_REG_FAN_MASK_OFFSET:
	case NVSW_REG_EROT_OFFSET:
	case NVSW_REG_EROT_EVENT_OFFSET:
	case NVSW_REG_EROT_MASK_OFFSET:
	case NVSW_REG_EROT_ERR_OFFSET:
	case NVSW_REG_EROT_ERR_EVENT_OFFSET:
	case NVSW_REG_EROT_ERR_MASK_OFFSET:
	case NVSW_REG_PWRB_OFFSET:
	case NVSW_REG_PWRB_EVENT_OFFSET:
	case NVSW_REG_PWRB_MASK_OFFSET:
	case NVSW_REG_FRU1_OFFSET:
	case NVSW_REG_FRU1_EVENT_OFFSET:
	case NVSW_REG_FRU1_MASK_OFFSET:
	case NVSW_REG_LEAK_OFFSET:
	case NVSW_REG_LEAK_EVENT_OFFSET:
	case NVSW_REG_LEAK_MASK_OFFSET:
	case NVSW_REG_GP4_RO_OFFSET:
	case NVSW_REG_CPLD1_MVER_OFFSET:
	case NVSW_REG_CPLD2_MVER_OFFSET:
	case NVSW_REG_CPLD3_MVER_OFFSET:
	case NVSW_REG_CPLD4_MVER_OFFSET:
	case NVSW_REG_PWM1_OFFSET:
	case NVSW_REG_TACHO1_OFFSET:
	case NVSW_REG_TACHO2_OFFSET:
	case NVSW_REG_TACHO3_OFFSET:
	case NVSW_REG_TACHO4_OFFSET:
	case NVSW_REG_TACHO5_OFFSET:
	case NVSW_REG_TACHO6_OFFSET:
	case NVSW_REG_TACHO7_OFFSET:
	case NVSW_REG_TACHO8_OFFSET:
	case NVSW_REG_TACHO9_OFFSET:
	case NVSW_REG_TACHO10_OFFSET:
	case NVSW_REG_TACHO11_OFFSET:
	case NVSW_REG_TACHO12_OFFSET:
	case NVSW_REG_FAN_CAP1_OFFSET:
	case NVSW_REG_FAN_DRW_CAP_OFFSET:
	case NVSW_REG_TACHO_SPEED_OFFSET:
	case NVSW_REG_CONFIG1_OFFSET:
	case NVSW_REG_CONFIG2_OFFSET:
	case NVSW_REG_CONFIG3_OFFSET:
	case NVSW_REG_SPI_CHNL_SELECT:
	case NVSW_REG_WD2_TMR_OFFSET:
	case NVSW_REG_WD2_TLEFT_OFFSET:
	case NVSW_REG_WD2_ACT_OFFSET:
	case NVSW_REG_WD3_TMR_OFFSET:
	case NVSW_REG_WD3_TLEFT_OFFSET:
	case NVSW_REG_WD3_ACT_OFFSET:
	case NVSW_REG_MUX0_OFFSET:
	case NVSW_REG_MUX1_OFFSET:
	case NVSW_REG_UFM_VERSION_OFFSET:
		return true;
	}
	return false;
}

static bool nvsw_volatile_reg(struct device *dev, unsigned int reg)
{
	switch (reg) {
	case NVSW_REG_CPLD1_VER_OFFSET:
	case NVSW_REG_CPLD1_PN_OFFSET:
	case NVSW_REG_CPLD1_PN1_OFFSET:
	case NVSW_REG_CPLD2_VER_OFFSET:
	case NVSW_REG_CPLD2_PN_OFFSET:
	case NVSW_REG_CPLD2_PN1_OFFSET:
	case NVSW_REG_CPLD3_VER_OFFSET:
	case NVSW_REG_CPLD3_PN_OFFSET:
	case NVSW_REG_CPLD3_PN1_OFFSET:
	case NVSW_REG_CPLD4_VER_OFFSET:
	case NVSW_REG_CPLD4_PN_OFFSET:
	case NVSW_REG_CPLD4_PN1_OFFSET:
	case NVSW_REG_PG_OFFSET:
	case NVSW_REG_PG_EVENT_OFFSET:
	case NVSW_REG_PG_MASK_OFFSET:
	case NVSW_REG_RESET_GP1_OFFSET:
	case NVSW_REG_FIELD_UPGRADE:
	case NVSW_REG_SAFE_BIOS_OFFSET:
	case NVSW_REG_RESET_CAUSE_OFFSET:
	case NVSW_REG_RESET_CAUSE1_OFFSET:
	case NVSW_REG_RESET_CAUSE2_OFFSET:
	case NVSW_REG_LED1_OFFSET:
	case NVSW_REG_LED5_OFFSET:
	case NVSW_REG_LED6_OFFSET:
	case NVSW_REG_LED7_OFFSET:
	case NVSW_REG_RESET_GP2_OFFSET:
	case NVSW_REG_GP0_RO_OFFSET:
	case NVSW_REG_GP1_RO_OFFSET:
	case NVSW_REG_GPCOM0_OFFSET:
	case NVSW_REG_GP0_OFFSET:
	case NVSW_REG_GP1_OFFSET:
	case NVSW_REG_GP7_OFFSET:
	case NVSW_REG_PWM_CONTROL_OFFSET:
	case NVSW_REG_GP4_OFFSET:
	case NVSW_REG_GP6_OFFSET:
	case NVSW_REG_AGGRCO_OFFSET:
	case NVSW_REG_AGGRCO_MASK_OFFSET:
	case NVSW_REG_HEALTH_OFFSET:
	case NVSW_REG_HEALTH_EVENT_OFFSET:
	case NVSW_REG_HEALTH_MASK_OFFSET:
	case NVSW_REG_AGGR_OFFSET:
	case NVSW_REG_AGGR_MASK_OFFSET:
	case NVSW_REG_FU_CAP_OFFSET:
	case NVSW_REG_BRD4_OFFSET:
	case NVSW_REG_BRD4_EVENT_OFFSET:
	case NVSW_REG_BRD4_MASK_OFFSET:
	case NVSW_REG_AGGRLO_OFFSET:
	case NVSW_REG_AGGRLO_MASK_OFFSET:
	case NVSW_REG_BRD1_OFFSET:
	case NVSW_REG_BRD1_EVENT_OFFSET:
	case NVSW_REG_BRD1_MASK_OFFSET:
	case NVSW_REG_ASIC1_HEALTH_OFFSET:
	case NVSW_REG_ASIC1_EVENT_OFFSET:
	case NVSW_REG_ASIC1_MASK_OFFSET:
	case NVSW_REG_ASIC2_HEALTH_OFFSET:
	case NVSW_REG_ASIC2_EVENT_OFFSET:
	case NVSW_REG_ASIC2_MASK_OFFSET:
	case NVSW_REG_ASIC3_HEALTH_OFFSET:
	case NVSW_REG_ASIC3_EVENT_OFFSET:
	case NVSW_REG_ASIC3_MASK_OFFSET:
	case NVSW_REG_FAN_OFFSET:
	case NVSW_REG_FAN_EVENT_OFFSET:
	case NVSW_REG_FAN_MASK_OFFSET:
	case NVSW_REG_EROT_OFFSET:
	case NVSW_REG_EROT_EVENT_OFFSET:
	case NVSW_REG_EROT_MASK_OFFSET:
	case NVSW_REG_EROT_ERR_OFFSET:
	case NVSW_REG_EROT_ERR_EVENT_OFFSET:
	case NVSW_REG_EROT_ERR_MASK_OFFSET:
	case NVSW_REG_PWRB_OFFSET:
	case NVSW_REG_PWRB_EVENT_OFFSET:
	case NVSW_REG_PWRB_MASK_OFFSET:
	case NVSW_REG_FRU1_OFFSET:
	case NVSW_REG_FRU1_EVENT_OFFSET:
	case NVSW_REG_FRU1_MASK_OFFSET:
	case NVSW_REG_LEAK_OFFSET:
	case NVSW_REG_LEAK_EVENT_OFFSET:
	case NVSW_REG_LEAK_MASK_OFFSET:
	case NVSW_REG_GP4_RO_OFFSET:
	case NVSW_REG_CPLD1_MVER_OFFSET:
	case NVSW_REG_CPLD2_MVER_OFFSET:
	case NVSW_REG_CPLD3_MVER_OFFSET:
	case NVSW_REG_CPLD4_MVER_OFFSET:
	case NVSW_REG_PWM1_OFFSET:
	case NVSW_REG_TACHO1_OFFSET:
	case NVSW_REG_TACHO2_OFFSET:
	case NVSW_REG_TACHO3_OFFSET:
	case NVSW_REG_TACHO4_OFFSET:
	case NVSW_REG_TACHO5_OFFSET:
	case NVSW_REG_TACHO6_OFFSET:
	case NVSW_REG_TACHO7_OFFSET:
	case NVSW_REG_TACHO8_OFFSET:
	case NVSW_REG_TACHO9_OFFSET:
	case NVSW_REG_TACHO10_OFFSET:
	case NVSW_REG_TACHO11_OFFSET:
	case NVSW_REG_TACHO12_OFFSET:
	case NVSW_REG_FAN_CAP1_OFFSET:
	case NVSW_REG_FAN_DRW_CAP_OFFSET:
	case NVSW_REG_TACHO_SPEED_OFFSET:
	case NVSW_REG_CONFIG1_OFFSET:
	case NVSW_REG_CONFIG2_OFFSET:
	case NVSW_REG_CONFIG3_OFFSET:
	case NVSW_REG_SPI_CHNL_SELECT:
	case NVSW_REG_WD2_TMR_OFFSET:
	case NVSW_REG_WD2_TLEFT_OFFSET:
	case NVSW_REG_WD2_ACT_OFFSET:
	case NVSW_REG_WD3_TMR_OFFSET:
	case NVSW_REG_WD3_TLEFT_OFFSET:
	case NVSW_REG_WD3_ACT_OFFSET:
	case NVSW_REG_MUX0_OFFSET:
	case NVSW_REG_MUX1_OFFSET:
	case NVSW_REG_UFM_VERSION_OFFSET:
		return true;
	}
	return false;
}

/* Configuration for the register map of a device with 2 bytes address space. */
static const struct reg_default nvsw_core_reg_def[] = {
	{ NVSW_REG_PWM_CONTROL_OFFSET, 0x00 },
};

static const struct regmap_config nvsw_regmap_i2c_conf = {
	.reg_bits = 16,
	.val_bits = 8,
	.max_register = NVSW_REG_MAX,
	.cache_type = REGCACHE_FLAT,
	.reg_defaults = nvsw_core_reg_def,
	.num_reg_defaults = ARRAY_SIZE(nvsw_core_reg_def),
	.writeable_reg = nvsw_writeable_reg,
	.readable_reg = nvsw_readable_reg,
	.volatile_reg = nvsw_volatile_reg,
};

static const struct reg_default nvsw_reg_def_l1[] = {
	{ NVSW_REG_PWM_CONTROL_OFFSET, 0x00 },
	{ NVSW_REG_WD2_ACT_OFFSET, 0x00 },
	{ NVSW_REG_WD3_ACT_OFFSET, 0x00 },
	{ NVSW_REG_LEAK_MASK_OFFSET, 0x3f },
};

struct nvsw_io_regmap_context {
	void __iomem *base;
};

static struct nvsw_io_regmap_context nvsw_io_regmap_ctx;

static int nvsw_reg_read(void *context, unsigned int reg, unsigned int *val)
{
	struct nvsw_io_regmap_context *ctx = context;

	*val = ioread8(ctx->base - NVSW_REG_MIN + reg);
	return 0;
}

static int nvsw_reg_write(void *context, unsigned int reg, unsigned int val)
{
	struct nvsw_io_regmap_context *ctx = context;

	iowrite8(val, ctx->base - NVSW_REG_MIN + reg);
	return 0;
}

static const struct regmap_config nvsw_regmap_conf_l1 = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = NVSW_REG_MAX,
	.cache_type = REGCACHE_FLAT,
	.writeable_reg = nvsw_writeable_reg,
	.readable_reg = nvsw_readable_reg,
	.volatile_reg = nvsw_volatile_reg,
	.reg_defaults = nvsw_reg_def_l1,
	.num_reg_defaults = ARRAY_SIZE(nvsw_reg_def_l1),
	.reg_read = nvsw_reg_read,
	.reg_write = nvsw_reg_write,
};

static int nvsw_core_platform_init(struct nvsw_core *nvsw_core)
{
	int i, err;

	/* Add registers io access driver. */
	if (nvsw_core->regio_data) {
		nvsw_core->regio_data->regmap = nvsw_core->regmap;
		nvsw_core->regio =
			platform_device_register_resndata(nvsw_core->dev, "mlxreg-io",
							  PLATFORM_DEVID_NONE, NULL, 0,
							  nvsw_core->regio_data,
							  sizeof(*nvsw_core->regio_data));
		if (IS_ERR(nvsw_core->regio)) {
			err = PTR_ERR(nvsw_core->regio);
			goto fail_platform_io_register;
		}
	}

	/* Add FAN driver. */
	if (nvsw_core->fan_data) {
		nvsw_core->fan_data->regmap = nvsw_core->regmap;
		nvsw_core->fan = platform_device_register_resndata(nvsw_core->dev, "mlxreg-fan",
								   PLATFORM_DEVID_NONE, NULL, 0,
								   nvsw_core->fan_data,
								   sizeof(*nvsw_core->fan_data));
		if (IS_ERR(nvsw_core->fan_data)) {
			err = PTR_ERR(nvsw_core->fan_data);
			goto fail_platform_fan_register;
		}
	}

	/* Add LED driver. */
	if (nvsw_core->led_data) {
		nvsw_core->led_data->regmap = nvsw_core->regmap;
		nvsw_core->led = platform_device_register_resndata(nvsw_core->dev, "leds-mlxreg",
								   PLATFORM_DEVID_NONE, NULL, 0,
								   nvsw_core->led_data,
								   sizeof(*nvsw_core->led_data));
		if (IS_ERR(nvsw_core->led)) {
			err = PTR_ERR(nvsw_core->led);
			goto fail_platform_leds_register;
		}
	}

	/* Add hotplug driver. */
	if (nvsw_core->hotplug_data && nvsw_core->np) {
		nvsw_core->hotplug_data->irq = nvsw_core->client->irq;
		dev_info(nvsw_core->dev, "irq %d\n", nvsw_core->hotplug_data->irq);
		nvsw_core->hotplug_data->regmap = nvsw_core->regmap;
		nvsw_core->hotplug =
			platform_device_register_resndata(nvsw_core->dev, "mlxreg-hotplug",
							  PLATFORM_DEVID_NONE, NULL, 0,
							  nvsw_core->hotplug_data,
							  sizeof(*nvsw_core->hotplug_data));
		if (IS_ERR(nvsw_core->hotplug)) {
			err = PTR_ERR(nvsw_core->hotplug);
			goto fail_platform_hotplug_register;
		}
	}

	for (i = 0; i < ARRAY_SIZE(nvsw_core->wd_data); i++) {
		if (nvsw_core->wd_data[i]) {
			nvsw_core->wd_data[i]->regmap = nvsw_core->regmap;
			nvsw_core->wd[i] =
				platform_device_register_resndata(nvsw_core->dev, "mlx-wdt", i,
								  NULL, 0, nvsw_core->wd_data[i],
								  sizeof(*nvsw_core->wd_data[i]));
			if (IS_ERR(nvsw_core->wd[i])) {
				err = PTR_ERR(nvsw_core->wd[i]);
				goto fail_platform_wd_register;
			}
		}
	}

	return 0;

fail_platform_wd_register:
	while (i--)
		platform_device_unregister(nvsw_core->wd[i]);
fail_platform_hotplug_register:
	if (nvsw_core->led_data)
		platform_device_unregister(nvsw_core->led);
fail_platform_leds_register:
	if (nvsw_core->fan_data)
		platform_device_unregister(nvsw_core->fan);
fail_platform_fan_register:
	if (nvsw_core->regio_data)
		platform_device_unregister(nvsw_core->regio);
fail_platform_io_register:
	return err;
}

static void nvsw_core_platform_exit(struct nvsw_core *nvsw_core)
{
	int i;

	for (i = NVSW_WD_MAX - 1; i >= 0; i--)
		platform_device_unregister(nvsw_core->wd[i]);
	if (nvsw_core->hotplug_data)
		platform_device_unregister(nvsw_core->hotplug);
	if (nvsw_core->led_data)
		platform_device_unregister(nvsw_core->led);
	if (nvsw_core->fan_data)
		platform_device_unregister(nvsw_core->fan);
	if (nvsw_core->regio_data)
		platform_device_unregister(nvsw_core->regio);
}

static int nvsw_core_mux_topology_init(struct nvsw_core *nvsw_core)
{
	return nvsw_core->mux_init(nvsw_core);
}

static void nvsw_core_mux_topology_exit(struct nvsw_core *nvsw_core)
{
	nvsw_core->mux_exit(nvsw_core);
}

static int nvsw_core_regmap_init(struct nvsw_core *nvsw_core)
{
	int err;

	switch (nvsw_core->regmap_type) {
	case REGMAP_I2C:
		nvsw_core->regmap = devm_regmap_init_i2c(nvsw_core->client, &nvsw_regmap_i2c_conf);
		break;
	case REGMAP_IO:
		if (nvsw_core->port_map) {
			nvsw_io_regmap_ctx.base = nvsw_core->port_map(nvsw_core);
			if (!nvsw_io_regmap_ctx.base)
				return -ENOMEM;
		}
		nvsw_core->regmap = devm_regmap_init(nvsw_core->dev, NULL, &nvsw_io_regmap_ctx,
						     &nvsw_regmap_conf_l1);
		break;
	default:
		return -EINVAL;
	}

	if (IS_ERR(nvsw_core->regmap)) {
		dev_err(nvsw_core->dev, "Failed to create regmap");
		return PTR_ERR(nvsw_core->regmap);
	}

	/* Sync registers with hardware. */
	regcache_mark_dirty(nvsw_core->regmap);
	err = regcache_sync(nvsw_core->regmap);
	if (err) {
		dev_err(nvsw_core->dev, "Failed to sync regmap");
		return err;
	}

	/* Set registers default values. */
	if (nvsw_core->set_reg_default) {
		err = nvsw_core->set_reg_default(nvsw_core->regmap);
		if (err) {
			dev_err(nvsw_core->dev, "Failed to set default regmap");
			return err;
		}
	}

	return 0;
}

int nvsw_core_init(struct nvsw_core *nvsw_core)
{
	int err;

	err = nvsw_core_regmap_init(nvsw_core);
	if (err)
		return err;

	err = nvsw_core_mux_topology_init(nvsw_core);
	if (err)
		goto nvsw_core_mux_topology_init_fail;

	err = nvsw_core_platform_init(nvsw_core);
	if (err)
		goto nvsw_core_platform_init_fail;

	return 0;
nvsw_core_platform_init_fail:
	nvsw_core_mux_topology_exit(nvsw_core);
nvsw_core_mux_topology_init_fail:
	return err;
}
EXPORT_SYMBOL(nvsw_core_init);

void nvsw_core_exit(struct nvsw_core *nvsw_core)
{
	nvsw_core_platform_exit(nvsw_core);
	nvsw_core_mux_topology_exit(nvsw_core);
	if (nvsw_core->set_reg_default)
		nvsw_core->set_reg_default(nvsw_core->regmap);
}
EXPORT_SYMBOL(nvsw_core_exit);

MODULE_AUTHOR("Vadim Pasternak <vadimp@mellanox.com>");
MODULE_DESCRIPTION("Nvidia platform driver");
MODULE_LICENSE("Dual BSD/GPL");
