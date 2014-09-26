/*
 * Copyright © 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Author: Shobhit Kumar <shobhit.kumar@intel.com>
 *
 */

#include <linux/slab.h>
#include <video/mipi_display.h>
#include <asm/intel-mid.h>
#include <drm/i915_drm.h>
#include <drm/i915_adf_wrapper.h>
#include <core/common/dsi/dsi_config.h>
#include <core/common/dsi/dsi_pipe.h>
#include <core/common/dsi/dsi_panel.h>
#include <core/vlv/vlv_dc_regs.h>
#include <linux/i2c.h>
#include "intel_dsi.h"
#include "intel_dsi_cmd.h"
#include "dsi_vbt.h"

#define MIPI_TRANSFER_MODE_SHIFT	0
#define MIPI_VIRTUAL_CHANNEL_SHIFT	1
#define MIPI_PORT_SHIFT			3

#define PREPARE_CNT_MAX		0x3F
#define EXIT_ZERO_CNT_MAX	0x3F
#define CLK_ZERO_CNT_MAX	0xFF
#define TRAIL_CNT_MAX		0x1F

#define NS_KHZ_RATIO 1000000

#define GPI0_NC_0_HV_DDI0_HPD           0x4130
#define GPIO_NC_0_HV_DDI0_PAD           0x4138
#define GPIO_NC_1_HV_DDI0_DDC_SDA       0x4120
#define GPIO_NC_1_HV_DDI0_DDC_SDA_PAD   0x4128
#define GPIO_NC_2_HV_DDI0_DDC_SCL       0x4110
#define GPIO_NC_2_HV_DDI0_DDC_SCL_PAD   0x4118
#define GPIO_NC_3_PANEL0_VDDEN          0x4140
#define GPIO_NC_3_PANEL0_VDDEN_PAD      0x4148
#define GPIO_NC_4_PANEL0_BLKEN          0x4150
#define GPIO_NC_4_PANEL0_BLKEN_PAD      0x4158
#define GPIO_NC_5_PANEL0_BLKCTL         0x4160
#define GPIO_NC_5_PANEL0_BLKCTL_PAD     0x4168
#define GPIO_NC_6_PCONF0                0x4180
#define GPIO_NC_6_PAD                   0x4188
#define GPIO_NC_7_PCONF0                0x4190
#define GPIO_NC_7_PAD                   0x4198
#define GPIO_NC_8_PCONF0                0x4170
#define GPIO_NC_8_PAD                   0x4178
#define GPIO_NC_9_PCONF0                0x4100
#define GPIO_NC_9_PAD                   0x4108
#define GPIO_NC_10_PCONF0               0x40E0
#define GPIO_NC_10_PAD                  0x40E8
#define GPIO_NC_11_PCONF0               0x40F0
#define GPIO_NC_11_PAD                  0x40F8

static void vlv_gpio_nc_write(u32 reg, u32 val)
{
	intel_dpio_sideband_rw(INTEL_SIDEBAND_REG_WRITE, IOSF_PORT_GPIO_NC,
			       reg, &val);
}

struct gpio_table {
	u16 function_reg;
	u16 pad_reg;
	u8 init;
};

static struct gpio_table gtable[] = {
	{ GPI0_NC_0_HV_DDI0_HPD, GPIO_NC_0_HV_DDI0_PAD, 0 },
	{ GPIO_NC_1_HV_DDI0_DDC_SDA, GPIO_NC_1_HV_DDI0_DDC_SDA_PAD, 0 },
	{ GPIO_NC_2_HV_DDI0_DDC_SCL, GPIO_NC_2_HV_DDI0_DDC_SCL_PAD, 0 },
	{ GPIO_NC_3_PANEL0_VDDEN, GPIO_NC_3_PANEL0_VDDEN_PAD, 0 },
	{ GPIO_NC_4_PANEL0_BLKEN, GPIO_NC_4_PANEL0_BLKEN_PAD, 0 },
	{ GPIO_NC_5_PANEL0_BLKCTL, GPIO_NC_5_PANEL0_BLKCTL_PAD, 0 },
	{ GPIO_NC_6_PCONF0, GPIO_NC_6_PAD, 0 },
	{ GPIO_NC_7_PCONF0, GPIO_NC_7_PAD, 0 },
	{ GPIO_NC_8_PCONF0, GPIO_NC_8_PAD, 0 },
	{ GPIO_NC_9_PCONF0, GPIO_NC_9_PAD, 0 },
	{ GPIO_NC_10_PCONF0, GPIO_NC_10_PAD, 0},
	{ GPIO_NC_11_PCONF0, GPIO_NC_11_PAD, 0}
};

static u8 *mipi_exec_send_packet(struct dsi_pipe *dsi_pipe, u8 *data)
{
	struct dsi_context *intel_dsi = &dsi_pipe->config.ctx;
	u8 type, byte, mode, vc, port;
	u16 len;

	byte = *data++;
	mode = (byte >> MIPI_TRANSFER_MODE_SHIFT) & 0x1;
	vc = (byte >> MIPI_VIRTUAL_CHANNEL_SHIFT) & 0x3;
	port = (byte >> MIPI_PORT_SHIFT) & 0x3;

	/* LP or HS mode */
	intel_dsi->hs = mode;

	/* get packet type and increment the pointer */
	type = *data++;

	len = *((u16 *) data);
	data += 2;

	switch (type) {
	case MIPI_DSI_GENERIC_SHORT_WRITE_0_PARAM:
		dsi_vc_generic_write_0(dsi_pipe, vc);
		break;
	case MIPI_DSI_GENERIC_SHORT_WRITE_1_PARAM:
		dsi_vc_generic_write_1(dsi_pipe, vc, *data);
		break;
	case MIPI_DSI_GENERIC_SHORT_WRITE_2_PARAM:
		dsi_vc_generic_write_2(dsi_pipe, vc, *data, *(data + 1));
		break;
	case MIPI_DSI_GENERIC_READ_REQUEST_0_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_1_PARAM:
	case MIPI_DSI_GENERIC_READ_REQUEST_2_PARAM:
		pr_debug("Generic Read not yet implemented or used\n");
		break;
	case MIPI_DSI_GENERIC_LONG_WRITE:
		dsi_vc_generic_write(dsi_pipe, vc, data, len);
		break;
	case MIPI_DSI_DCS_SHORT_WRITE:
		dsi_vc_dcs_write_0(dsi_pipe, vc, *data);
		break;
	case MIPI_DSI_DCS_SHORT_WRITE_PARAM:
		dsi_vc_dcs_write_1(dsi_pipe, vc, *data, *(data + 1));
		break;
	case MIPI_DSI_DCS_READ:
		pr_debug("DCS Read not yet implemented or used\n");
		break;
	case MIPI_DSI_DCS_LONG_WRITE:
		dsi_vc_dcs_write(dsi_pipe, vc, data, len);
		break;
	}

	data += len;

	return data;
}

static u8 *mipi_exec_delay(struct dsi_pipe *dsi_pipe, u8 *data)
{
	u32 delay = *((u32 *) data);

	usleep_range(delay, delay + 10);
	data += 4;

	return data;
}

static u8 *mipi_exec_gpio(struct dsi_pipe *dsi_pipe, u8 *data)
{
	u8 gpio, action;
	u16 function, pad;
	u32 val;

	gpio = *data++;

	/* pull up/down */
	action = *data++;

	function = gtable[gpio].function_reg;
	pad = gtable[gpio].pad_reg;

	if (!gtable[gpio].init) {
		/* program the function */
		/* FIXME: remove constant below */
		vlv_gpio_nc_write(function, 0x2000CC00);
		gtable[gpio].init = 1;
	}

	val = 0x4 | action;

	/* pull up/down */
	vlv_gpio_nc_write(pad, val);

	return data;
}

static u8 *mipi_exec_i2c(struct dsi_pipe *dsi_pipe, u8 *data)
{
	struct i2c_adapter *adapter;
	int ret;
	u8 reg_offset, payload_size, retries = 5;
	struct i2c_msg msg;
	u8 *transmit_buffer = NULL;

	u8 flag = *data++;
	u8 index = *data++;
	u8 bus_number = *data++;
	u16 slave_add = *(u16 *)(data);
	data = data + 2;
	reg_offset = *data++;
	payload_size = *data++;

	adapter = i2c_get_adapter(bus_number);

	if (!adapter) {
		DRM_ERROR("i2c_get_adapter(%u) failed, index:%u flag: %u\n",
				(bus_number + 1), index, flag);
		goto out;
	}

	transmit_buffer = kmalloc(1 + payload_size, GFP_TEMPORARY);

	if (!transmit_buffer)
		goto out;

	transmit_buffer[0] = reg_offset;
	memcpy(&transmit_buffer[1], data, (size_t)payload_size);

	msg.addr   = slave_add;
	msg.flags  = 0;
	msg.len    = 2;
	msg.buf    = &transmit_buffer[0];

	do {
		ret =  i2c_transfer(adapter, &msg, 1);
		if (ret == -EAGAIN)
			usleep_range(1000, 2500);
		else if (ret != 1) {
			DRM_ERROR("i2c transfer failed %d\n", ret);
			break;
		}
	} while (retries--);

	if (retries == 0)
		DRM_ERROR("i2c transfer failed");

out:
	kfree(transmit_buffer);

	data = data + payload_size;
	return data;
}

typedef u8 * (*fn_mipi_elem_exec)(struct dsi_pipe *dsi_pipe, u8 *data);
static const fn_mipi_elem_exec exec_elem[] = {
	NULL, /* reserved */
	mipi_exec_send_packet,
	mipi_exec_delay,
	mipi_exec_gpio,
	mipi_exec_i2c,
	NULL, /* status read; later */
};

/*
 * MIPI Sequence from VBT #53 parsing logic
 * We have already separated each seqence during bios parsing
 * Following is generic execution function for any sequence
 */

static const char * const seq_name[] = {
	"UNDEFINED",
	"MIPI_SEQ_ASSERT_RESET",
	"MIPI_SEQ_INIT_OTP",
	"MIPI_SEQ_DISPLAY_ON",
	"MIPI_SEQ_DISPLAY_OFF",
	"MIPI_SEQ_DEASSERT_RESET",
	"MIPI_BACKLIGHT_ON",
	"MIPI_BACKLIGHT_OFF",
	"MIPI_TEAR_ON",
};

static void generic_exec_sequence(struct dsi_pipe *dsi_pipe, char *sequence)
{
	u8 *data = sequence;
	fn_mipi_elem_exec mipi_elem_exec;
	int index;

	if (!sequence)
		return;

	pr_debug("Starting MIPI sequence - %s\n", seq_name[*data]);

	/* go to the first element of the sequence */
	data++;

	/* parse each byte till we reach end of sequence byte - 0x00 */
	while (1) {
		index = *data;
		mipi_elem_exec = exec_elem[index];
		if (!mipi_elem_exec) {
			pr_err("Unsupported MIPI element, skipping sequence execution\n");
			return;
		}

		/* goto element payload */
		data++;

		/* execute the element specific rotines */
		data = mipi_elem_exec(dsi_pipe, data);

		/*
		 * After processing the element, data should point to
		 * next element or end of sequence
		 * check if have we reached end of sequence
		 */
		if (*data == 0x00)
			break;
	}
}

static int generic_init(struct dsi_pipe *pipe)
{
	struct dsi_config *dsi_config = &pipe->config;
	struct dsi_context *intel_dsi = &dsi_config->ctx;
	struct dsi_vbt *dsi_vbt = NULL;
	struct mipi_config *mipi_config;
	struct drm_display_mode *mode = NULL;
	struct mipi_pps_data *pps;

	u32 bits_per_pixel = 24;
	u32 tlpx_ns, extra_byte_count, bitrate, tlpx_ui;
	u32 ui_num, ui_den;
	u32 prepare_cnt, exit_zero_cnt, clk_zero_cnt, trail_cnt;
	u32 ths_prepare_ns, tclk_trail_ns;
	u32 tclk_prepare_clkzero, ths_prepare_hszero;
	u32 lp_to_hs_switch, hs_to_lp_switch;
	u32 pclk, computed_ddr;
	u16 burst_mode_ratio;

	pr_debug("ADF: %s\n", __func__);

	/* get the VBT parsed MIPI data and support mode from i915 wrapper */
	intel_get_dsi_vbt_data((void **)&dsi_vbt, &mode);
	if (!dsi_vbt || !mode) {
		pr_err("ADF: %s: No VBT data from i915\n", __func__);
		return -1;
	}

	dsi_config->dsi = dsi_vbt;
	memcpy(&dsi_config->vbt_mode, mode, sizeof(struct drm_display_mode));
	mipi_config = dsi_config->dsi->config;
	pps = dsi_config->dsi->pps;

	intel_dsi->eotp_pkt = mipi_config->eot_pkt_disabled ? 0 : 1;
	intel_dsi->clock_stop = mipi_config->enable_clk_stop ? 1 : 0;
	intel_dsi->lane_count = mipi_config->lane_cnt + 1;
	intel_dsi->pixel_format = mipi_config->videomode_color_format << 7;

	if (intel_dsi->pixel_format == VID_MODE_FORMAT_RGB666)
		bits_per_pixel = 18;
	else if (intel_dsi->pixel_format == VID_MODE_FORMAT_RGB565)
		bits_per_pixel = 16;

	intel_dsi->operation_mode = mipi_config->is_cmd_mode;
	intel_dsi->video_mode_format = mipi_config->video_transfer_mode;
	intel_dsi->escape_clk_div = mipi_config->byte_clk_sel;
	intel_dsi->lp_rx_timeout = mipi_config->lp_rx_timeout;
	intel_dsi->turn_arnd_val = mipi_config->turn_around_timeout;
	intel_dsi->rst_timer_val = mipi_config->device_reset_timer;
	intel_dsi->init_count = mipi_config->master_init_timer;
	intel_dsi->bw_timer = mipi_config->dbi_bw_timer;
	intel_dsi->video_frmt_cfg_bits =
		mipi_config->bta_enabled ? DISABLE_VIDEO_BTA : 0;
	intel_dsi->dual_link = mipi_config->dual_link;

	pclk = mode->clock;

	/* Burst Mode Ratio
	 * Target ddr frequency from VBT / non burst ddr freq
	 * multiply by 100 to preserve remainder
	 */
	if (intel_dsi->video_mode_format == VIDEO_MODE_BURST) {
		if (mipi_config->target_burst_mode_freq) {
			computed_ddr =
				(pclk * bits_per_pixel) / intel_dsi->lane_count;

			if (mipi_config->target_burst_mode_freq <
								computed_ddr) {
				pr_err("Burst mode freq is less than computed\n");
				return -1;
			}

			burst_mode_ratio = DIV_ROUND_UP(
				mipi_config->target_burst_mode_freq * 100,
				computed_ddr);

			pclk = DIV_ROUND_UP(pclk * burst_mode_ratio, 100);
		} else {
			pr_err("Burst mode target is not set\n");
			return -1;
		}
	} else
		burst_mode_ratio = 100;

	intel_dsi->burst_mode_ratio = burst_mode_ratio;
	intel_dsi->pclk = pclk;

	bitrate = (pclk * bits_per_pixel) / intel_dsi->lane_count;

	switch (intel_dsi->escape_clk_div) {
	case 0:
		tlpx_ns = 50;
		break;
	case 1:
		tlpx_ns = 100;
		break;

	case 2:
		tlpx_ns = 200;
		break;
	default:
		tlpx_ns = 50;
		break;
	}

	switch (intel_dsi->lane_count) {
	case 1:
	case 2:
		extra_byte_count = 2;
		break;
	case 3:
		extra_byte_count = 4;
		break;
	case 4:
	default:
		extra_byte_count = 3;
		break;
	}

	/*
	 * ui(s) = 1/f [f in hz]
	 * ui(ns) = 10^9 / (f*10^6) [f in Mhz] -> 10^3/f(Mhz)
	 */

	/* in Kbps */
	ui_num = NS_KHZ_RATIO;
	ui_den = bitrate;

	tclk_prepare_clkzero = mipi_config->tclk_prepare_clkzero;
	ths_prepare_hszero = mipi_config->ths_prepare_hszero;

	/*
	 * B060
	 * LP byte clock = TLPX/ (8UI)
	 */
	intel_dsi->lp_byte_clk = DIV_ROUND_UP(tlpx_ns * ui_den, 8 * ui_num);

	/* count values in UI = (ns value) * (bitrate / (2 * 10^6))
	 *
	 * Since txddrclkhs_i is 2xUI, all the count values programmed in
	 * DPHY param register are divided by 2
	 *
	 * prepare count
	 */
	ths_prepare_ns = max(mipi_config->ths_prepare,
			     mipi_config->tclk_prepare);
	prepare_cnt = DIV_ROUND_UP(ths_prepare_ns * ui_den, ui_num * 2);

	/* exit zero count */
	exit_zero_cnt = DIV_ROUND_UP(
				(ths_prepare_hszero - ths_prepare_ns) * ui_den,
				ui_num * 2
				);

	/*
	 * Exit zero  is unified val ths_zero and ths_exit
	 * minimum value for ths_exit = 110ns
	 * min (exit_zero_cnt * 2) = 110/UI
	 * exit_zero_cnt = 55/UI
	 */
	 if (exit_zero_cnt < (55 * ui_den / ui_num))
		if ((55 * ui_den) % ui_num)
			exit_zero_cnt += 1;

	/* clk zero count */
	clk_zero_cnt = DIV_ROUND_UP(
			(tclk_prepare_clkzero -	ths_prepare_ns)
			* ui_den, 2 * ui_num);

	/* trail count */
	tclk_trail_ns = max(mipi_config->tclk_trail, mipi_config->ths_trail);
	trail_cnt = DIV_ROUND_UP(tclk_trail_ns * ui_den, 2 * ui_num);

	if (prepare_cnt > PREPARE_CNT_MAX ||
		exit_zero_cnt > EXIT_ZERO_CNT_MAX ||
		clk_zero_cnt > CLK_ZERO_CNT_MAX ||
		trail_cnt > TRAIL_CNT_MAX)
		pr_debug("Values crossing maximum limits, restricting to max values\n");

	if (prepare_cnt > PREPARE_CNT_MAX)
		prepare_cnt = PREPARE_CNT_MAX;

	if (exit_zero_cnt > EXIT_ZERO_CNT_MAX)
		exit_zero_cnt = EXIT_ZERO_CNT_MAX;

	if (clk_zero_cnt > CLK_ZERO_CNT_MAX)
		clk_zero_cnt = CLK_ZERO_CNT_MAX;

	if (trail_cnt > TRAIL_CNT_MAX)
		trail_cnt = TRAIL_CNT_MAX;

	/* B080 */
	intel_dsi->dphy_reg = exit_zero_cnt << 24 | trail_cnt << 16 |
						clk_zero_cnt << 8 | prepare_cnt;

	/*
	 * LP to HS switch count = 4TLPX + PREP_COUNT * 2 + EXIT_ZERO_COUNT * 2
	 *					+ 10UI + Extra Byte Count
	 *
	 * HS to LP switch count = THS-TRAIL + 2TLPX + Extra Byte Count
	 * Extra Byte Count is calculated according to number of lanes.
	 * High Low Switch Count is the Max of LP to HS and
	 * HS to LP switch count
	 *
	 */
	tlpx_ui = DIV_ROUND_UP(tlpx_ns * ui_den, ui_num);

	/* B044 */
	/* FIXME:
	 * The comment above does not match with the code */
	lp_to_hs_switch = DIV_ROUND_UP(4 * tlpx_ui + prepare_cnt * 2 +
						exit_zero_cnt * 2 + 10, 8);

	hs_to_lp_switch = DIV_ROUND_UP(mipi_config->ths_trail + 2 * tlpx_ui, 8);

	intel_dsi->hs_to_lp_count = max(lp_to_hs_switch, hs_to_lp_switch);
	intel_dsi->hs_to_lp_count += extra_byte_count;

	/* B088 */
	/* LP -> HS for clock lanes
	 * LP clk sync + LP11 + LP01 + tclk_prepare + tclk_zero +
	 *						extra byte count
	 * 2TPLX + 1TLPX + 1 TPLX(in ns) + prepare_cnt * 2 + clk_zero_cnt *
	 *					2(in UI) + extra byte count
	 * In byteclks = (4TLPX + prepare_cnt * 2 + clk_zero_cnt *2 (in UI)) /
	 *					8 + extra byte count
	 */
	intel_dsi->clk_lp_to_hs_count =
		DIV_ROUND_UP(
			4 * tlpx_ui + prepare_cnt * 2 +
			clk_zero_cnt * 2,
			8);

	intel_dsi->clk_lp_to_hs_count += extra_byte_count;

	/* HS->LP for Clock Lanes
	 * Low Power clock synchronisations + 1Tx byteclk + tclk_trail +
	 *						Extra byte count
	 * 2TLPX + 8UI + (trail_count*2)(in UI) + Extra byte count
	 * In byteclks = (2*TLpx(in UI) + trail_count*2 +8)(in UI)/8 +
	 *						Extra byte count
	 */
	intel_dsi->clk_hs_to_lp_count =
		DIV_ROUND_UP(2 * tlpx_ui + trail_cnt * 2 + 8,
			8);
	intel_dsi->clk_hs_to_lp_count += extra_byte_count;

	pr_info("ADF: %s: Eot %s\n", __func__,
		intel_dsi->eotp_pkt ? "enabled" : "disabled");
	pr_info("ADF: %s: Clockstop %s\n", __func__, intel_dsi->clock_stop ?
						"disabled" : "enabled");
	pr_info("ADF: %s: Mode %s\n", __func__,
		intel_dsi->operation_mode ? "command" : "video");
	pr_info("ADF: %s: Pixel Format %d\n", __func__,
		intel_dsi->pixel_format);
	pr_info("ADF: %s: TLPX %d\n", __func__, intel_dsi->escape_clk_div);
	pr_info("ADF: %s: LP RX Timeout 0x%x\n",
		__func__, intel_dsi->lp_rx_timeout);
	pr_info("ADF: %s: Turnaround Timeout 0x%x\n",
		__func__, intel_dsi->turn_arnd_val);
	pr_info("ADF: %s: Init Count 0x%x\n", __func__, intel_dsi->init_count);
	pr_info("ADF: %s: HS to LP Count 0x%x\n",
		__func__, intel_dsi->hs_to_lp_count);
	pr_info("ADF: %s: LP Byte Clock %d\n",
		__func__, intel_dsi->lp_byte_clk);
	pr_info("ADF: %s: DBI BW Timer 0x%x\n", __func__, intel_dsi->bw_timer);
	pr_info("ADF: %s: LP to HS Clock Count 0x%x\n",
		__func__, intel_dsi->clk_lp_to_hs_count);
	pr_info("ADF: %s: HS to LP Clock Count 0x%x\n",
		__func__, intel_dsi->clk_hs_to_lp_count);
	pr_info("ADF: %s: BTA %s\n", __func__,
			intel_dsi->video_frmt_cfg_bits & DISABLE_VIDEO_BTA ?
			"disabled" : "enabled");

	/* delays in VBT are in unit of 100us, so need to convert
	 * here in ms
	 * Delay (100us) * 100 /1000 = Delay / 10 (ms) */
	intel_dsi->backlight_off_delay = pps->bl_disable_delay / 10;
	intel_dsi->backlight_on_delay = pps->bl_enable_delay / 10;
	intel_dsi->panel_on_delay = pps->panel_on_delay / 10;
	intel_dsi->panel_off_delay = pps->panel_off_delay / 10;
	intel_dsi->panel_pwr_cycle_delay = pps->panel_power_cycle_delay / 10;

	return 0;
}

#if 0
static int generic_mode_valid(struct intel_dsi_device *dsi,
		   struct drm_display_mode *mode)
{
	return MODE_OK;
}

static bool generic_mode_fixup(struct intel_dsi_device *dsi,
		    const struct drm_display_mode *mode,
		    struct drm_display_mode *adjusted_mode) {
	return true;
}
#endif

static int generic_panel_reset(struct dsi_pipe *interface)
{
	struct dsi_vbt *dsi = interface->config.dsi;
	char *sequence = dsi->sequence[MIPI_SEQ_ASSERT_RESET];
	pr_debug("ADF: %s\n", __func__);

	generic_exec_sequence(interface, sequence);

	return 0;
}

static int generic_disable_panel_power(struct dsi_pipe *interface)
{
	struct dsi_vbt *dsi = interface->config.dsi;
	char *sequence = dsi->sequence[MIPI_SEQ_DEASSERT_RESET];
	pr_debug("ADF: %s\n", __func__);

	generic_exec_sequence(interface, sequence);
	return 0;
}

static int generic_send_otp_cmds(struct dsi_pipe *interface)
{
	struct dsi_vbt *dsi = interface->config.dsi;
	char *sequence = dsi->sequence[MIPI_SEQ_INIT_OTP];
	pr_debug("ADF: %s\n", __func__);

	generic_exec_sequence(interface, sequence);

	return 0;
}

static int generic_enable(struct dsi_pipe *interface)
{
	struct dsi_vbt *dsi = interface->config.dsi;
	char *sequence = dsi->sequence[MIPI_SEQ_DISPLAY_ON];
	pr_debug("ADF: %s\n", __func__);

	generic_exec_sequence(interface, sequence);

	return 0;
}

static int generic_disable(struct dsi_pipe *interface)
{
	struct dsi_vbt *dsi = interface->config.dsi;
	char *sequence = dsi->sequence[MIPI_SEQ_DISPLAY_OFF];
	pr_debug("ADF: %s\n", __func__);

	generic_exec_sequence(interface, sequence);

	return 0;
}

int generic_enable_bklt(struct dsi_pipe *interface)
{
	struct dsi_vbt *dsi = interface->config.dsi;
	char *sequence = dsi->sequence[MIPI_SEQ_BACKLIGHT_ON];
	pr_err("ADF: %s\n", __func__);

	generic_exec_sequence(interface, sequence);
	return 0;
}

int generic_disable_bklt(struct dsi_pipe *interface)
{
	struct dsi_vbt *dsi = interface->config.dsi;
	char *sequence = dsi->sequence[MIPI_SEQ_BACKLIGHT_OFF];
	pr_err("ADF: %s\n", __func__);

	generic_exec_sequence(interface, sequence);
	return 0;
}

static int generic_detect(struct dsi_pipe *interface)
{
	pr_debug("ADF: %s\n", __func__);
	return 1;
}

#if 0
static bool generic_get_hw_state(struct intel_dsi_device *dev)
{
	return true;
}
#endif

static int generic_get_modes(struct dsi_config *config,
			     struct drm_mode_modeinfo *modeinfo)
{
	struct drm_display_mode *mode = &config->vbt_mode;

	pr_err("ADF: %s\n", __func__);

	modeinfo->clock = mode->clock;
	modeinfo->hdisplay = (u16) mode->hdisplay;
	modeinfo->hsync_start = (u16) mode->hsync_start;
	modeinfo->hsync_end = (u16) mode->hsync_end;
	modeinfo->htotal = (u16) mode->htotal;
	modeinfo->vdisplay = (u16) mode->vdisplay;
	modeinfo->vsync_start = (u16) mode->vsync_start;
	modeinfo->vsync_end = (u16) mode->vsync_end;
	modeinfo->vtotal = (u16) mode->vtotal;
	modeinfo->hskew = (u16) mode->hskew;
	modeinfo->vscan = (u16) mode->vscan;
	modeinfo->vrefresh = (u32) mode->vrefresh;
	modeinfo->flags = mode->flags;
	modeinfo->type |= mode->type | DRM_MODE_TYPE_PREFERRED;
	strncpy(modeinfo->name, mode->name, DRM_DISPLAY_MODE_LEN);

	return 0;
}

#if 0
static void generic_destroy(struct intel_dsi_device *dsi) { }
#endif

int generic_get_panel_info(struct dsi_config *config, struct panel_info *info)
{
	struct drm_display_mode *mode = &config->vbt_mode;
	struct dsi_context *ctx = &config->ctx;
	int bpp = 24;

	pr_err("ADF: %s\n", __func__);

	info->width_mm = mode->width_mm;
	info->height_mm = mode->height_mm;
	info->dsi_type = ctx->operation_mode;
	info->lane_num = ctx->lane_count;
	info->dual_link = ctx->dual_link;

	if (ctx->pixel_format == VID_MODE_FORMAT_RGB666)
		bpp = 18;
	else if (ctx->pixel_format == VID_MODE_FORMAT_RGB565)
		bpp = 16;

	info->bpp = bpp;

	return 0;
}

static int generic_exit_standby(struct dsi_pipe *interface)
{
	pr_debug("ADF: %s\n", __func__);
	return 0;
}

static int generic_set_brightness(struct dsi_pipe *interface, int level)
{
	pr_debug("ADF: %s\n", __func__);
	return 0;
}

static int generic_set_mode(struct dsi_pipe *interface)
{
	pr_debug("ADF: %s\n", __func__);
	return 0;
}

struct panel_ops generic_ops = {
		.get_config_mode = generic_get_modes,
		.dsi_controller_init = generic_init,
		.get_panel_info = generic_get_panel_info,
		.reset = generic_panel_reset,
		.exit_deep_standby = generic_exit_standby,
		.detect = generic_detect,
		.power_on = generic_enable,
		.power_off = generic_disable,
		.enable_backlight = generic_enable_bklt,
		.disable_backlight = generic_disable_bklt,
		.set_brightness = generic_set_brightness,
		.drv_ic_init = generic_send_otp_cmds,
		.drv_set_panel_mode = generic_set_mode,
		.disable_panel_power = generic_disable_panel_power,
/*
 * Might need to add these hooks in panel_ops
		.mode_valid = generic_mode_valid,
		.mode_fixup = generic_mode_fixup,
		.get_hw_state = generic_get_hw_state,
		.destroy = generic_destroy,
*/
};

struct dsi_panel generic_panel = {
	.panel_id = MIPI_DSI_GENERIC_PANEL_ID,
	.ops = &generic_ops,
};

const struct dsi_panel *get_generic_panel(void)
{
	pr_debug("ADF: %s\n", __func__);
	return &generic_panel;
}
