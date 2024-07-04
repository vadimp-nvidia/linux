/* SPDX-License-Identifier: GPL-2.0+
 *
 * Nvidia BMC platform driver
 *
 * Copyright (C) 2025 Nvidia Technologies Ltd.
 */

#ifndef __NVSW_H__
#define __NVSW_H__

/* I2C bus IO offsets */
#define NVSW_REG_CPLD1_VER_OFFSET	0x2500
#define NVSW_REG_CPLD2_VER_OFFSET	0x2501
#define NVSW_REG_CPLD3_VER_OFFSET	0x2502
#define NVSW_REG_CPLD4_VER_OFFSET	0x2503
#define NVSW_REG_CPLD1_PN_OFFSET	0x2504
#define NVSW_REG_CPLD1_PN1_OFFSET	0x2505
#define NVSW_REG_CPLD2_PN_OFFSET	0x2506
#define NVSW_REG_CPLD2_PN1_OFFSET	0x2507
#define NVSW_REG_CPLD3_PN_OFFSET	0x2508
#define NVSW_REG_CPLD3_PN1_OFFSET	0x2509
#define NVSW_REG_CPLD4_PN_OFFSET	0x250a
#define NVSW_REG_CPLD4_PN1_OFFSET	0x250b
#define NVSW_REG_PG_OFFSET		0x2514
#define NVSW_REG_PG_EVENT_OFFSET	0x2515
#define NVSW_REG_PG_MASK_OFFSET		0x2516
#define NVSW_REG_RESET_GP1_OFFSET	0x2517
#define NVSW_REG_RESET_GP2_OFFSET	0x2519
#define NVSW_REG_RESET_CAUSE_OFFSET	0x251d
#define NVSW_REG_RESET_CAUSE1_OFFSET	0x251e
#define NVSW_REG_RESET_CAUSE2_OFFSET	0x251f
#define NVSW_REG_LED1_OFFSET		0x2520
#define NVSW_REG_LED5_OFFSET		0x2524
#define NVSW_REG_LED6_OFFSET		0x2525
#define NVSW_REG_LED7_OFFSET		0x2526
#define NVSW_REG_GP0_RO_OFFSET		0x252a
#define NVSW_REG_GP1_RO_OFFSET		0x252c
#define NVSW_REG_GPCOM0_OFFSET		0x252d
#define NVSW_REG_GP0_OFFSET		0x252e
#define NVSW_REG_GP7_OFFSET		0x252f
#define NVSW_REG_GP1_OFFSET		0x2530
#define NVSW_REG_FIELD_UPGRADE		0x2534
#define NVSW_REG_SAFE_BIOS_OFFSET	0x2535
#define NVSW_REG_PWM_CONTROL_OFFSET	0x2537
#define NVSW_REG_GP4_OFFSET		0x2538
#define NVSW_REG_GP6_OFFSET		0x2539
#define NVSW_REG_AGGR_OFFSET		0x253a
#define NVSW_REG_AGGR_MASK_OFFSET	0x253b
#define NVSW_REG_FU_CAP_OFFSET		0x253c
#define NVSW_REG_BRD4_OFFSET		0x253d
#define NVSW_REG_BRD4_EVENT_OFFSET	0x253e
#define NVSW_REG_BRD4_MASK_OFFSET	0x253f
#define NVSW_REG_AGGRLO_OFFSET		0x2540
#define NVSW_REG_AGGRLO_MASK_OFFSET	0x2541
#define NVSW_REG_AGGRCO_OFFSET		0x2542
#define NVSW_REG_AGGRCO_MASK_OFFSET	0x2543
#define NVSW_REG_BRD1_OFFSET		0x2547
#define NVSW_REG_BRD1_EVENT_OFFSET	0x2548
#define NVSW_REG_BRD1_MASK_OFFSET	0x2549
#define NVSW_REG_HEALTH_OFFSET		0x254d
#define NVSW_REG_HEALTH_EVENT_OFFSET	0x254e
#define NVSW_REG_HEALTH_MASK_OFFSET	0x254f
#define NVSW_REG_ASIC1_HEALTH_OFFSET	0x2550
#define NVSW_REG_ASIC1_EVENT_OFFSET	0x2551
#define NVSW_REG_ASIC1_MASK_OFFSET	0x2552
#define NVSW_REG_ASIC2_HEALTH_OFFSET	0x2553
#define NVSW_REG_ASIC2_EVENT_OFFSET	0x2554
#define NVSW_REG_ASIC2_MASK_OFFSET	0x2555
#define NVSW_REG_ASIC3_HEALTH_OFFSET	0x2582
#define NVSW_REG_ASIC3_EVENT_OFFSET	0x2583
#define NVSW_REG_ASIC3_MASK_OFFSET	0x2584
#define NVSW_REG_FAN_OFFSET		0x2588
#define NVSW_REG_FAN_EVENT_OFFSET	0x2589
#define NVSW_REG_FAN_MASK_OFFSET	0x258a
#define NVSW_REG_EROT_OFFSET		0x2591
#define NVSW_REG_EROT_EVENT_OFFSET	0x2592
#define NVSW_REG_EROT_MASK_OFFSET	0x2593
#define NVSW_REG_EROT_ERR_OFFSET	0x2594
#define NVSW_REG_EROT_ERR_EVENT_OFFSET	0x2595
#define NVSW_REG_EROT_ERR_MASK_OFFSET	0x2596
#define NVSW_REG_PWRB_OFFSET		0x2597
#define NVSW_REG_PWRB_EVENT_OFFSET	0x2598
#define NVSW_REG_PWRB_MASK_OFFSET	0x2599
#define NVSW_REG_FRU1_OFFSET		0x25ac
#define NVSW_REG_FRU1_EVENT_OFFSET	0x25ad
#define NVSW_REG_FRU1_MASK_OFFSET	0x25ae
#define NVSW_REG_LEAK_OFFSET		0x25af
#define NVSW_REG_LEAK_EVENT_OFFSET	0x25b0
#define NVSW_REG_LEAK_MASK_OFFSET	0x25b1
#define NVSW_REG_GP4_RO_OFFSET		0x25c2
#define NVSW_REG_SPI_CHNL_SELECT	0x25c3
#define NVSW_REG_WD2_TMR_OFFSET		0x25cd
#define NVSW_REG_WD2_TLEFT_OFFSET	0x25ce
#define NVSW_REG_WD2_ACT_OFFSET		0x25cf
#define NVSW_REG_WD3_TMR_OFFSET		0x25d1
#define NVSW_REG_WD3_TLEFT_OFFSET	0x25d2
#define NVSW_REG_WD3_ACT_OFFSET		0x25d3
#define NVSW_REG_MUX0_OFFSET		0x25da
#define NVSW_REG_MUX1_OFFSET		0x25db
#define NVSW_REG_CPLD1_MVER_OFFSET	0x25de
#define NVSW_REG_CPLD2_MVER_OFFSET	0x25df
#define NVSW_REG_CPLD3_MVER_OFFSET	0x25e0
#define NVSW_REG_CPLD4_MVER_OFFSET	0x25e1
#define NVSW_REG_UFM_VERSION_OFFSET	0x25e2
#define NVSW_REG_PWM1_OFFSET		0x25e3
#define NVSW_REG_TACHO1_OFFSET		0x25e4
#define NVSW_REG_TACHO2_OFFSET		0x25e5
#define NVSW_REG_TACHO3_OFFSET		0x25e6
#define NVSW_REG_TACHO4_OFFSET		0x25e7
#define NVSW_REG_TACHO5_OFFSET		0x25e8
#define NVSW_REG_TACHO6_OFFSET		0x25e9
#define NVSW_REG_TACHO7_OFFSET		0x25eb
#define NVSW_REG_TACHO8_OFFSET		0x25ec
#define NVSW_REG_TACHO9_OFFSET		0x25ed
#define NVSW_REG_TACHO10_OFFSET		0x25ee
#define NVSW_REG_TACHO11_OFFSET		0x25ef
#define NVSW_REG_TACHO12_OFFSET		0x25f0
#define NVSW_REG_FAN_CAP1_OFFSET	0x25f5
#define NVSW_REG_FAN_DRW_CAP_OFFSET	0x25f7
#define NVSW_REG_TACHO_SPEED_OFFSET	0x25f8
#define NVSW_REG_CONFIG1_OFFSET		0x25fb
#define NVSW_REG_CONFIG2_OFFSET		0x25fc
#define NVSW_REG_CONFIG3_OFFSET		0x25fd
#define NVSW_REG_MIN			0x2500
#define NVSW_REG_MAX			0x26ff

#define NVSW_AGGR_MASK_COMEX		BIT(0)
#define NVSW_AGGR_MASK			BIT(2)
#define NVSW_LOW_AGGR_MASK_LOW		0xe0
#define NVSW_REG_FRU1_MASK		GENMASK(3, 0)
#define NVSW_LEAK_MASK			GENMASK(5, 0)
#define NVSW_SSD_I2C_ALERT_MASK		BIT(0)
#define NVSW_WD_EXP_MASK		BIT(1)
#define NVSW_5V_USB_MASK		BIT(2)
#define NVSW_PCB_TEMP1_MASK		BIT(3)
#define NVSW_PCB_TEMP2_MASK		BIT(4)
#define NVSW_SGMII_MASK			BIT(5)
#define NVSW_SDD_PG_MASK		BIT(6)
#define NVSW_LEAK_AGGR_MASK		BIT(7)
#define NVSW_EROT_MASK			GENMASK(2, 0)
#define NVSW_FAN_NG_MASK		GENMASK(6, 0)
#define NVSW_ASIC_MASK			GENMASK(1, 0)
#define NVSW_LOW_AGGR_MASK_ASIC1	BIT(0)
#define NVSW_LOW_AGGR_MASK_ASIC2	BIT(1)
#define NVSW_LOW_AGGR_MASK_ASIC3	BIT(2)
#define NVSW_PWR_BUTTON_MASK		BIT(0)
#define NVSW_AMB_TEMP_SENSE_MASK	BIT(1)
#define NVSW_GRACEFUL_POWER_OFF_MASK	BIT(2)
#define NVSW_CPU_POWER_OFF_READY_MASK	BIT(3)
#define NVSW_CPU_RESET_MASK		BIT(4)
#define NVSW_APML_SMB_ALERT_MASK	BIT(5)
#define NVSW_CPU_UNEXP_POWER_OFF_MASK	BIT(6)
#define NVSW_UID_PUSH_BUTTON_MASK	BIT(7)
#define NVSW_RTC_MASK			BIT(0)
#define NVSW_HOT_SWAP_ALERT_MASK	BIT(1)
#define NVSW_LED_LO_NIBBLE_MASK		GENMASK(7, 4)
#define NVSW_LED_HI_NIBBLE_MASK		GENMASK(3, 0)
#define NVSW_UART_SEL_MASK		GENMASK(7, 6)
#define NVSW_BIOS_STATUS_MASK		GENMASK(3, 1)
#define NVSW_REG_RESET_MASK		BIT(1)
#define NVSW_FU_CAP_MASK		GENMASK(1, 0)
#define NVSW_WD_RESET_ACT_MASK		GENMASK(7, 1)
#define NVSW_WD_FAN_ACT_MASK		(GENMASK(7, 0) & ~BIT(4))
#define NVSW_WD_TYPE2_TO_MASK		0
#define NVSW_WD3_DFLT_TIMEOUT		600

#define NVSW_NR_NONE			-1
#define NVSW_MUX_MAX			2
#define NVSW_WD_MAX			2

 /* The system type. */
enum nvsw_core_hid_type {
	HID162,
	HID176,
	HID177,
	HID180,
};

 /* The system register map type. */
enum nvsw_core_regmap_type {
	REGMAP_IO,
	REGMAP_I2C,
};

/* nvsw_core - device private data
 * hid: hardware Id;
 * regmap_type: register map type;
 * @device: device;
 * @client: I2C client device;
 * @np: device node;
 * @regmap: device register map;
 * @regio_data: register access platform data;
 * @regio: register access device;
 * @hotplug_data: hotplug platform data;
 * @hotplug: hotplug device;
 * @led: led devices;
 * @led_data: led platform data;
 * @fan - fan device;
 * @fan_data - fan platform data;
 * @wd - watchdog device;
 * @wd_data - watchdog platform data;
 * @port_map - io port mapping;
 * @set_reg_default - set default registers callback;
 * @mux_init: mux initialization callback;
 * @mux_exit: mux de-initialization callback;
 * @mux: mux devices;
 * @mux_num: number of mux device;
 */
struct nvsw_core {
	enum nvsw_core_hid_type hid;
	enum nvsw_core_regmap_type regmap_type;
	struct device *dev;
	struct i2c_client *client;
	struct device_node *np;
	struct regmap *regmap;
	struct mlxreg_core_platform_data *regio_data;
	struct platform_device *regio;
	struct mlxreg_core_hotplug_platform_data *hotplug_data;
	struct platform_device *hotplug;
	struct platform_device *led;
	struct mlxreg_core_platform_data *led_data;
	struct platform_device *fan;
	struct mlxreg_core_platform_data *fan_data;
	struct platform_device *wd[NVSW_WD_MAX];
	struct mlxreg_core_platform_data *wd_data[NVSW_WD_MAX];
	void __iomem *(*port_map)(struct nvsw_core *nvsw_core);
	int (*set_reg_default)(struct regmap *regmap);
	int (*mux_init)(struct nvsw_core *nvsw_core);
	void (*mux_exit)(struct nvsw_core *nvsw_core);
	struct platform_device *mux[NVSW_MUX_MAX];
	u32 mux_num;
};

/* Platform core init/exit functions. */
int nvsw_core_init(struct nvsw_core *nvsw_core);
void nvsw_core_exit(struct nvsw_core *nvsw_core);

#endif /* !defined(__NVSW_H__) */

