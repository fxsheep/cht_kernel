#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/dmi.h>
#include <linux/efi.h>
#include <linux/acpi.h>
#include <linux/atomisp_platform.h>
#include <asm/spid.h>

#define MAX_SUBDEVS 8

/* This needs to be initialized at runtime so the various
 * platform-checking macros in spid.h return the correct results.
 * Either that, or we need to fix up the usage of those macros so that
 * it's checking more appropriate runtime-detectable data. */
struct soft_platform_id spid;
EXPORT_SYMBOL(spid);

/* Submodules use type==0 for the end-of-list marker */
static struct intel_v4l2_subdev_table pdata_subdevs[MAX_SUBDEVS+1];

static const struct atomisp_platform_data pdata = {
	.subdevs = pdata_subdevs,
	.spid = &spid,
};

/*
 *   struct atomisp_sensor_caps {
 *       int stream_num;
 *   };
 *   struct atomisp_camera_caps {
 *       int sensor_num;
 *       struct atomisp_sensor_caps sensor[MAX_SENSORS_PER_PORT];
 *   };
 */
const struct atomisp_camera_caps *atomisp_get_default_camera_caps(void)
{
	/* This is near-legacy.  The camera_caps field is ultimately used
	 * only in two spots in atomisp_cmd, one checks if it's ==1 and
	 * the other if it's <2 (is 0 legal?). */
	return NULL;
}
EXPORT_SYMBOL_GPL(atomisp_get_default_camera_caps);

/*
 *   struct intel_v4l2_subdev_i2c_board_info {
 *       struct i2c_board_info board_info;
 *       int i2c_adapter_id;
 *   };
 *   struct intel_v4l2_subdev_table {
 *       struct intel_v4l2_subdev_i2c_board_info v4l2_subdev;
 *       enum intel_v4l2_subdev_qtype type;
 *       enum atomisp_camera_port port;
 *   };
 *   struct atomisp_platform_data {
 *       struct intel_v4l2_subdev_table *subdevs;
 *       const struct soft_platform_id *spid;
 *   };
 */
const struct atomisp_platform_data *atomisp_get_platform_data(void)
{
	return &pdata;
}
EXPORT_SYMBOL_GPL(atomisp_get_platform_data);

/*
 * Used in a handful of modules.  Focus motor control, I think.  Note
 * that there is no configurability in the API, so this needs to be
 * fixed where it is used.
 *
 * struct camera_af_platform_data {
 *     int (*power_ctrl)(struct v4l2_subdev *subdev, int flag);
 * };
 */
const struct camera_af_platform_data *camera_get_af_platform_data(void)
{
	return NULL;
}
EXPORT_SYMBOL_GPL(camera_get_af_platform_data);

int atomisp_register_i2c_module(struct i2c_client *client,
                                enum intel_v4l2_subdev_type type,
                                enum atomisp_camera_port port)
{
	int i;
	struct i2c_board_info *bi;

	dev_info(&client->dev, "register atomisp i2c module type %d on port %d\n", type, port);

	for (i=0; i < MAX_SUBDEVS; i++)
		if (!pdata.subdevs[i].type)
			break;

	if (pdata.subdevs[i].type)
		return -ENOMEM;

	pdata.subdevs[i].type = type;
	pdata.subdevs[i].port = port;
	pdata.subdevs[i].v4l2_subdev.i2c_adapter_id = client->adapter->nr;

	/* Convert i2c_client to i2c_board_info */
	bi = &pdata.subdevs[i].v4l2_subdev.board_info;
	memcpy(bi->type, client->name, I2C_NAME_SIZE);
	bi->flags = client->flags;
	bi->addr = client->addr;
	bi->irq = client->irq;
	bi->comp_addr_count = client->comp_addr_count;
	bi->comp_addrs = client->comp_addrs;
	bi->irq_flags = client->irq_flags;

	return 0;
}
EXPORT_SYMBOL_GPL(atomisp_register_i2c_module);

struct gmin_cfg_var {
	const char *name, *val;
};

static const struct gmin_cfg_var ffrd8_vars[] = {
	{ "INTCF1B:00_ImxId",    "0x134" },
	{ "INTCF1B:00_CamType",  "1" },
	{ "INTCF1B:00_CsiPort",  "1" },
	{ "INTCF1B:00_CsiLanes", "4" },
	{ "INTCF1B:00_CsiFmt",   "13" },
	{ "INTCF1B:00_CsiBayer", "1" },
	{},
};

static const struct {
	const char *dmi_board_name;
	const struct gmin_cfg_var *vars;
} hard_vars[] = {
	{ "BYT-T FFD8", ffrd8_vars },
};


#define GMIN_CFG_VAR_EFI_GUID EFI_GUID(0xecb54cd9, 0xe5ae, 0x4fdc, \
				       0xa9, 0x71, 0xe8, 0x77,	   \
				       0x75, 0x60, 0x68, 0xf7)

#define CFG_VAR_NAME_MAX 64

/* Retrieves a device-specific configuration variable.  The dev
 * argument should be a device with an ACPI companion, as all
 * configuration is based on firmware ID. */
int gmin_get_config_var(struct device *dev, const char *var, char *out, size_t *out_len)
{
	struct device *adev;
	char var8[CFG_VAR_NAME_MAX];
	unsigned short var16[CFG_VAR_NAME_MAX];
	u32 efiattr_dummy;
	int i, j, ret;
	unsigned long efilen;

	if (!ACPI_COMPANION(dev))
		return -ENODEV;

	adev = &ACPI_COMPANION(dev)->dev;

	ret = snprintf(var8, sizeof(var8), "%s_%s", dev_name(adev), var);
	if (ret < 0 || ret >= sizeof(var8)-1)
		return -EINVAL;

	/* First check a hard-coded list of board-specific variables.
	 * Some device firmwares lack the ability to set EFI variables at
	 * runtime. */
	for (i = 0; i < ARRAY_SIZE(hard_vars); i++) {
		if (dmi_match(DMI_BOARD_NAME, hard_vars[i].dmi_board_name)) {
			for (j = 0; hard_vars[i].vars[j].name; j++) {
				size_t vl;
				const struct gmin_cfg_var *gv;

				gv = &hard_vars[i].vars[j];
				vl = strlen(gv->val);

				if (strcmp(var8, gv->name))
					continue;
				if (vl > *out_len-1)
					return -ENOSPC;

				memcpy(out, gv->val, min(*out_len, vl+1));
				out[*out_len-1] = 0;
				*out_len = vl;

				return 0;
			}
		}
	}

	/* Our variable names are ASCII by construction, but EFI names
	 * are wide chars.  Convert and zero-pad. */
	memset(var16, 0, sizeof(var16));
	for (i=0; var8[i] && i < sizeof(var8); i++)
		var16[i] = var8[i];

	if (!efi.get_variable)
		return -EINVAL;

	ret = efi.get_variable(var16, &GMIN_CFG_VAR_EFI_GUID, &efiattr_dummy,
			       &efilen, out);
	*out_len = efilen;

	return ret == EFI_SUCCESS ? 0 : -EINVAL;
}
EXPORT_SYMBOL_GPL(gmin_get_config_var);
