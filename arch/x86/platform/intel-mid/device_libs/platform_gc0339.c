/*
 * platform_gc0339.c: gc0339 platform data initilization file
 *
 * (C) Copyright 2013 Intel Corporation
 * Author:
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2
 * of the License.
 */

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/acpi.h>
#include <linux/atomisp_platform.h>
#include <asm/intel-mid.h>
#include <media/v4l2-subdev.h>
#include <linux/mfd/intel_mid_pmic.h>
#include <linux/vlv2_plat_clock.h>

/* workround - pin defined for byt */
#define CAMERA_0_RESET 126
#define CAMERA_0_PWDN 123
#ifdef CONFIG_VLV2_PLAT_CLK
#define OSC_CAM0_CLK 0x0
#define CLK_19P2MHz 0x1
#endif
#ifdef CONFIG_CRYSTAL_COVE
#define VPROG_2P8V 0x66
#define VPROG_1P8V 0x5D
#define VPROG_ENABLE 0x3
#define VPROG_DISABLE 0x2
#endif

enum camera_pmic_pin {
        CAMERA_1P8V,
        CAMERA_2P8V,
        CAMERA_POWER_NUM,
};


static int camera_reset;
static int camera_power_down;
static int camera_vprog1_on;

#ifdef CONFIG_CRYSTAL_COVE
/*
 * WA for BTY as simple VRF management
 */
int camera_set_pmic_power(enum camera_pmic_pin pin, bool flag)
{
        u8 reg_addr[CAMERA_POWER_NUM] = {VPROG_1P8V, VPROG_2P8V};
        u8 reg_value[2] = {VPROG_DISABLE, VPROG_ENABLE};
        int val;
        static DEFINE_MUTEX(mutex_power);
        int ret = 0;

        if (pin >= CAMERA_POWER_NUM)
                return -EINVAL;

        mutex_lock(&mutex_power);
        val = intel_mid_pmic_readb(reg_addr[pin]) & 0x3;

        if ((flag && (val == VPROG_DISABLE)) ||
                (!flag && (val == VPROG_ENABLE)))
                ret = intel_mid_pmic_writeb(reg_addr[pin], reg_value[flag]);

        mutex_unlock(&mutex_power);
        return ret;
}
EXPORT_SYMBOL_GPL(camera_set_pmic_power);
#endif


/*
 * GC0339 platform data
 */
/*
 * Cloned from MCG platform_camera.c because it's small and
 * self-contained.  All it does is maintain the V4L2 subdev hostdate
 * pointer
 */
static int camera_sensor_csi(struct v4l2_subdev *sd, u32 port,
			     u32 lanes, u32 format, u32 bayer_order, int flag)
{
        struct i2c_client *client = v4l2_get_subdevdata(sd);
        struct camera_mipi_info *csi = NULL;

        if (flag) {
                csi = kzalloc(sizeof(*csi), GFP_KERNEL);
                if (!csi) {
                        dev_err(&client->dev, "out of memory\n");
                        return -ENOMEM;
                }
                csi->port = port;
                csi->num_lanes = lanes;
                csi->input_format = format;
                csi->raw_bayer_order = bayer_order;
                v4l2_set_subdev_hostdata(sd, (void *)csi);
                csi->metadata_format = ATOMISP_INPUT_FORMAT_EMBEDDED;
                csi->metadata_effective_width = NULL;
                dev_info(&client->dev,
                         "camera pdata: port: %d lanes: %d order: %8.8x\n",
                         port, lanes, bayer_order);
        } else {
                csi = v4l2_get_subdev_hostdata(sd);
                kfree(csi);
        }

	return 0;
}

static int gc0339_gpio_ctrl(struct v4l2_subdev *sd, int flag)
{
	int ret;
	int pin;

	/*
	 * FIXME: WA using hardcoded GPIO value here.
	 * The GPIO value would be provided by ACPI table, which is
	 * not implemented currently.
	 */
	pin = CAMERA_0_RESET;
	if (camera_reset < 0) {
		ret = gpio_request(pin, "camera_0_reset");
		if (ret) {
			pr_err("%s: failed to request gpio(pin %d)\n",
			       __func__, pin);
			return ret;
		}
		camera_reset = pin;
	}

	/*
	 * FIXME: WA using hardcoded GPIO value here.
	 * The GPIO value would be provided by ACPI table, which is
	 * not implemented currently.
	 */
	pin = CAMERA_0_PWDN;
	if (camera_power_down < 0) {
		ret = gpio_request(pin, "camera_0_power");
		if (ret) {
			pr_err("%s: failed to request gpio(pin %d)\n",
			       __func__, pin);
			return ret;
		}
		camera_power_down = pin;
	}

	if (flag) {
		pr_info("pull low reset\n");
		gpio_set_value(camera_reset, 0);
		msleep(5);
		pr_info("pull high reset\n");
		gpio_set_value(camera_reset, 1);
		msleep(10);
		pr_info("pull low pwn\n");
		gpio_set_value(camera_power_down, 0);
		msleep(10);
	} else {
		pr_info("pull high pwn\n");
		gpio_set_value(camera_power_down, 1);
		pr_info("pull low reset\n");
		gpio_set_value(camera_reset, 0);
	}

	return 0;
}

static int gc0339_flisclk_ctrl(struct v4l2_subdev *sd, int flag)
{
	int ret = 0;

#ifdef CONFIG_VLV2_PLAT_CLK
	if (flag) {
		pr_info("mclk enable\n");
		ret = vlv2_plat_set_clock_freq(OSC_CAM0_CLK, CLK_19P2MHz);
		if (ret)
			return ret;
	}
	ret = vlv2_plat_configure_clock(OSC_CAM0_CLK, flag);
#endif
	return ret;
}

/*
 * The power_down gpio pin is to control GC0339's
 * internal power state.
 */
static int gc0339_power_ctrl(struct v4l2_subdev *sd, int flag)
{
	int ret = 0;

	if (flag) {
		if (camera_vprog1_on != 1) {
#ifdef CONFIG_CRYSTAL_COVE
			pr_info("1 disable 1V8\n");
			ret = camera_set_pmic_power(CAMERA_1P8V, false);
			if (ret)
				return ret;
			pr_info("1 disable 2V8\n");
			ret = camera_set_pmic_power(CAMERA_2P8V, false);
			mdelay(50);

			/*
			 * This should call VRF APIs.
			 *
			 * VRF not implemented for BTY, so call this
			 * as WAs
			 */
			pr_info("enable 1V8\n");
			ret = camera_set_pmic_power(CAMERA_1P8V, true);
			if (ret)
				return ret;
			//msleep(10);
			pr_info("enable 2V8\n");
			ret = camera_set_pmic_power(CAMERA_2P8V, true);
			msleep(10);
#endif
			if (!ret)
				camera_vprog1_on = 1;
			return ret;
		}
	} else {
		if (camera_vprog1_on != 0) {
#ifdef CONFIG_CRYSTAL_COVE
			pr_info("disable 1V8\n");
			ret = camera_set_pmic_power(CAMERA_1P8V, false);
			if (ret)
				return ret;
			pr_info("disable 2V8\n");
			ret = camera_set_pmic_power(CAMERA_2P8V, false);
#endif
			if (!ret)
				camera_vprog1_on = 0;
			return ret;
		}
	}

	return 0;
}

static int gc0339_csi_configure(struct v4l2_subdev *sd, int flag)
{
	return camera_sensor_csi(sd, ATOMISP_CAMERA_PORT_PRIMARY, 1,
		ATOMISP_INPUT_FORMAT_RAW_10, atomisp_bayer_order_grbg, flag);
}

static struct camera_sensor_platform_data gc0339_sensor_platform_data = {
	.gpio_ctrl	= gc0339_gpio_ctrl,
	.flisclk_ctrl	= gc0339_flisclk_ctrl,
	.power_ctrl	= gc0339_power_ctrl,
	.csi_cfg	= gc0339_csi_configure,
};

void *gc0339_platform_data(void *info)
{
	camera_reset = -1;
	camera_power_down = -1;
	camera_vprog1_on = -1;

	return &gc0339_sensor_platform_data;
}

