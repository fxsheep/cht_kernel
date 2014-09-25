/*
 * linux/drivers/modem_control/mcd_cpu.c
 *
 * Version 1.0
 * This code permits to access the cpu specifics
 * of each supported platform.
 * among other things, it permits to configure and access gpios
 *
 * Copyright (C) 2013 Intel Corporation. All rights reserved.
 *
 * Contact: Ranquet Guillaume <guillaumex.ranquet@intel.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/mdm_ctrl_board.h>

#include "mdm_util.h"

/**
 * mdm_ctrl_configure_gpio - Configure GPIOs
 * @gpio: GPIO to configure
 * @direction: GPIO direction - 0: IN | 1: OUT
 *
 */
static inline int mdm_ctrl_configure_gpio(struct gpio_desc *gpio,
					  int direction,
					  int value)
{
	int ret = 0;

	if (direction)
		ret += gpiod_direction_output(gpio, value);
	else
		ret += gpiod_direction_input(gpio);

	if (ret) {
		pr_err(DRVNAME ": Unable to configure GPIO%d\n",
						desc_to_gpio(gpio));
		ret = -ENODEV;
	}

	return ret;
}

int cpu_init_gpio(void *data)
{
	struct mdm_ctrl_cpu_data *cpu_data = data;
	int ret;

	pr_debug("cpu_init");

	/* Configure the RESET_BB gpio */
	ret = mdm_ctrl_configure_gpio(cpu_data->gpio_rst_bbn, 1, 0);
	if (ret)
		goto out;

	/* Configure the ON gpio */
	ret = mdm_ctrl_configure_gpio(cpu_data->gpio_pwr_on, 1, 0);

	/* Configure the RESET_OUT gpio & irq */
	ret = mdm_ctrl_configure_gpio(cpu_data->gpio_rst_out, 0, 0);

	cpu_data->irq_reset = gpiod_to_irq(cpu_data->gpio_rst_out);

	/* Configure the CORE_DUMP gpio & irq */
	ret = mdm_ctrl_configure_gpio(cpu_data->gpio_cdump, 0, 0);
	if (ret)
		goto free_ctx2;

	cpu_data->irq_cdump = gpiod_to_irq(cpu_data->gpio_cdump);
	if (cpu_data->irq_cdump < 0)
		goto free_ctx2;

	pr_info(DRVNAME
		": GPIO (rst_bbn: %d, pwr_on: %d, rst_out: %d, fcdp_rb: %d)\n",
		desc_to_gpio(cpu_data->gpio_rst_bbn),
		desc_to_gpio(cpu_data->gpio_pwr_on),
		desc_to_gpio(cpu_data->gpio_rst_out),
		desc_to_gpio(cpu_data->gpio_cdump));

	return 0;

 free_ctx2:
	if (cpu_data->irq_reset > 0)
		free_irq(cpu_data->irq_reset, cpu_data);
	cpu_data->irq_reset = 0;
 out:
	return -ENODEV;
}

int cpu_cleanup_gpio(void *data)
{
	struct mdm_ctrl_cpu_data *cpu_data = data;

	cpu_data->irq_cdump = 0;
	cpu_data->irq_reset = 0;

	return 0;
}

int get_gpio_irq_cdump(void *data)
{
	struct mdm_ctrl_cpu_data *cpu_data = data;
	return cpu_data->irq_cdump;
}

int get_gpio_irq_rst(void *data)
{
	struct mdm_ctrl_cpu_data *cpu_data = data;
	return cpu_data->irq_reset;
}

int get_gpio_mdm_state(void *data)
{
	struct mdm_ctrl_cpu_data *cpu_data = data;
	return gpiod_get_value(cpu_data->gpio_rst_out);
}

int get_gpio_rst(void *data)
{
	struct mdm_ctrl_cpu_data *cpu_data = data;
	return desc_to_gpio(cpu_data->gpio_rst_bbn);
}

int get_gpio_pwr(void *data)
{
	struct mdm_ctrl_cpu_data *cpu_data = data;
	return desc_to_gpio(cpu_data->gpio_pwr_on);
}

int cpu_init_gpio_ngff(void *data)
{
	struct mdm_ctrl_cpu_data *cpu_data = data;
	int ret;

	pr_debug("cpu_init");

	/* Configure the RESET_BB gpio */
	ret = mdm_ctrl_configure_gpio(cpu_data->gpio_rst_bbn, 1, 0);
	if (ret)
		goto out;

	ret = mdm_ctrl_configure_gpio(cpu_data->gpio_rst_usbhub, 1, 1);

	pr_info(DRVNAME ": GPIO (rst_bbn: %d, rst_usb_hub: %d)\n",
				desc_to_gpio(cpu_data->gpio_rst_bbn),
				desc_to_gpio(cpu_data->gpio_rst_usbhub));

	return 0;

 out:
	return -ENODEV;
}

int cpu_cleanup_gpio_ngff(void *data)
{
	return 0;
}

int get_gpio_mdm_state_ngff(void *data)
{
	return 0;
}

int get_gpio_irq_cdump_ngff(void *data)
{
	return 0;
}

int get_gpio_irq_rst_ngff(void *data)
{
	return 0;
}

