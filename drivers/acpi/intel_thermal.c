/*
 * ACPI support for intel thermal drivers
 *
 * Copyright (C) 2014, Intel Corporation
 * Authors: Zhang Rui <rui.zhang@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/acpi.h>
#include <linux/module.h>

#include "internal.h"

#define DO_ENUMERATION 0x01
static const struct acpi_device_id intel_thermal_device_ids[]= {
	{"INT3400", DO_ENUMERATION },
	{"INT3401"},
	{"INT3402"},
	{"INT3403"},
	{"INT3404"},
	{"INT3406"},
	{"INT3407"},
	{"INT3408"},
	{"INT3409"},
	{"INT340A"},
	{"INT340B"},
	{""},
};

static int intel_thermal_handler_attach(struct acpi_device *adev,
                                   const struct acpi_device_id *id)
{
#ifdef CONFIG_INTEL_THERMAL
	if (id->driver_data == DO_ENUMERATION)
		acpi_create_platform_device(adev, NULL);
#endif
	return 1;
}

static struct acpi_scan_handler intel_thermal_handler = {
	.ids = intel_thermal_device_ids,
	.attach = intel_thermal_handler_attach,
};

void __init acpi_intel_thermal_init(void)
{
	acpi_scan_add_handler(&intel_thermal_handler);
}
