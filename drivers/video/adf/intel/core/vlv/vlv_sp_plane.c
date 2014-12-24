/*
 * Copyright (C) 2014, Intel Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <drm/i915_drm.h>
#include <video/intel_adf.h>
#include <drm/drm_rect.h>

#include "intel_adf_device.h"
#include "core/common/intel_dc_regs.h"
#include "core/common/dsi/dsi_pipe.h"
#include "core/vlv/vlv_dc_regs.h"
#include "core/vlv/vlv_dc_config.h"
#include "core/intel_dc_config.h"
#include "core/vlv/vlv_sp_plane.h"
#include "core/vlv/vlv_pri_plane.h"
#include "core/vlv/vlv_pm.h"

struct format_info {
	u32 drm_format;
	u32 hw_config;
	u8 bpp;
};

static const struct format_info format_mappings[] = {
	{
		.drm_format = DRM_FORMAT_YUYV,
		.hw_config = SP_FORMAT_YUV422 | SP_YUV_ORDER_YUYV,
		.bpp = 2,
	},

	{
		.drm_format = DRM_FORMAT_YVYU,
		.hw_config = SP_FORMAT_YUV422 | SP_YUV_ORDER_YVYU,
		.bpp = 2,
	},

	{
		.drm_format = DRM_FORMAT_UYVY,
		.hw_config = SP_FORMAT_YUV422 | SP_YUV_ORDER_UYVY,
		.bpp = 2,
	},

	{
		.drm_format = DRM_FORMAT_VYUY,
		.hw_config = SP_FORMAT_YUV422 | SP_YUV_ORDER_VYUY,
		.bpp = 2,
	},

	{
		.drm_format = DRM_FORMAT_C8,
		.hw_config = DISPPLANE_8BPP,
		.bpp = 1,
	},

	{
		.drm_format = DRM_FORMAT_RGB565,
		.hw_config = DISPPLANE_BGRX565,
		.bpp = 2,
	},

	{
		.drm_format = DRM_FORMAT_XRGB8888,
		.hw_config = DISPPLANE_BGRX888,
		.bpp = 4,
	},

	{
		.drm_format = DRM_FORMAT_ARGB8888,
		.hw_config = DISPPLANE_BGRA888,
		.bpp = 4,
	},

	{
		.drm_format = DRM_FORMAT_XBGR2101010,
		.hw_config = DISPPLANE_RGBX101010,
		.bpp = 4,
	},

	{
		.drm_format = DRM_FORMAT_ABGR2101010,
		.hw_config = DISPPLANE_RGBA101010,
		.bpp = 4,
	},

	{
		.drm_format = DRM_FORMAT_XBGR8888,
		.hw_config = DISPPLANE_RGBX888,
		.bpp = 4,
	},
	{
		.drm_format = DRM_FORMAT_ABGR8888,
		.hw_config = DISPPLANE_RGBA888,
		.bpp = 4,
	},
};

static void vlv_adf_flush_sp_plane(u32 pipe, u32 plane)
{
	REG_WRITE(SPSURF(pipe, plane), REG_READ(SPSURF(pipe, plane)));
}

static int context_init(struct vlv_sp_plane_context *ctx, u8 idx)
{
	switch (idx) {
	case SPRITE_A:
		ctx->plane = 0;
		ctx->pipe = 0;
		break;
	case SPRITE_B:
		ctx->plane = 1;
		ctx->pipe = 0;
		break;
	case SPRITE_C:
		ctx->plane = 0;
		ctx->pipe = 1;
		break;
	case SPRITE_D:
		ctx->plane = 1;
		ctx->pipe = 1;
		break;
	case SPRITE_E:
		ctx->plane = 0;
		ctx->pipe = 2;
		break;
	case SPRITE_F:
		ctx->plane = 1;
		ctx->pipe = 2;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static void context_destroy(struct vlv_sp_plane_context *ctx)
{
	return;
}

static int get_format_config(u32 drm_format, u32 *format, u32 *bpp,
		u8 alpha)
{
	int i;
	int ret = -EINVAL;

	for (i = 0; i < ARRAY_SIZE(format_mappings); i++) {
		if (format_mappings[i].drm_format == drm_format) {
			*format = format_mappings[i].hw_config;
			*bpp = format_mappings[i].bpp;
			ret = 0;
			break;
		}
	}
	if (alpha)
		return ret;
	switch (*format) {
	case DISPPLANE_BGRA888:
		*format = DISPPLANE_BGRX888;
		break;
	case DISPPLANE_RGBA101010:
		*format = DISPPLANE_RGBX101010;
		break;
	case DISPPLANE_RGBA888:
		*format = DISPPLANE_RGBX888;
		break;
	}

	return ret;
}

static void vlv_sp_suspend(struct intel_dc_component *component)
{
	return;
}

static void vlv_sp_resume(struct intel_dc_component *component)
{
	return;
}
static bool format_is_yuv(uint32_t format)
{
	switch (format) {
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_VYUY:
	case DRM_FORMAT_YVYU:
		return true;
	default:
		return false;
	}
}

static inline struct vlv_pipeline *to_vlv_pipeline_sp1_plane(
	struct vlv_sp_plane *splane)
{
	int plane = splane->ctx.plane;
	struct vlv_sp_plane *tmp_plane = splane;

	if (plane == 1)
		tmp_plane -= 1;

	return container_of(tmp_plane, struct vlv_pipeline, splane[0]);
}

static void vlv_sp_pane_save_ddl(struct vlv_sp_plane *splane, u32 ddl)
{
	int plane = splane->ctx.plane;
	struct vlv_pipeline *disp = NULL;
	struct vlv_pm *pm;

	disp = to_vlv_pipeline_sp1_plane(splane);

	pm = &disp->pm;
	vlv_pm_save_values(pm, true, (plane ? false : true),
		(plane ? true : false), ddl);
}

void vlv_get_pfit_mode(struct intel_plane_config *config,
		struct intel_plane *plane)
{
	struct intel_pipe *intel_pipe = plane->pipe;
	struct dsi_pipe *dsi_pipe = NULL;
	struct hdmi_pipe *hdmi_pipe = NULL;
	struct dp_pipe *dp_pipe = NULL;
	struct drm_mode_modeinfo mode;
	u32 scaled_width = 0;
	u32 scaled_height = 0;
	u32 pfit_control = intel_pipe->pipe_reg.pfit_control;

	if (intel_pipe->type == INTEL_PIPE_DSI) {
		dsi_pipe = to_dsi_pipe(intel_pipe);
		dsi_pipe->panel->ops->get_config_mode(&dsi_pipe->config, &mode);
	} else if (intel_pipe->type == INTEL_PIPE_HDMI) {
		hdmi_pipe = hdmi_pipe_from_intel_pipe(intel_pipe);
		intel_pipe->ops->get_current_mode(intel_pipe, &mode);
	} else if ((intel_pipe->type == INTEL_PIPE_DP) ||
		   (intel_pipe->type == INTEL_PIPE_EDP)) {
		dp_pipe = to_dp_pipe(intel_pipe);
		intel_pipe->ops->get_current_mode(intel_pipe, &mode);
	} else  {
		pr_err("ADF: sp:get_pfit_mode: unknown pipe type-%d",
				intel_pipe->type);
		return;
	}
	scaled_width = mode.hdisplay * config->src_h;
	scaled_height = config->src_w * mode.vdisplay;
	if (scaled_width > scaled_height) {
		pfit_control &= MASK_PFIT_SCALING_MODE;
		pfit_control |= PFIT_SCALING_PILLAR;
	} else if (scaled_width < scaled_height) {
		pfit_control &=  MASK_PFIT_SCALING_MODE;
		pfit_control |= PFIT_SCALING_LETTER;
	} else if (!(mode.hdisplay <= (config->src_w+25) &&
			mode.hdisplay >= (config->src_w-25))) {
		/*
		 * TODO: If native width doest not lies b/n src layer
		 * width-25 and width+25, we put pfit in auto scale,
		 * not expecting variation more than 25
		 */
		pfit_control &=  MASK_PFIT_SCALING_MODE;
		pfit_control |= PFIT_SCALING_AUTO;
	}
	intel_pipe->pipe_reg.pfit_control = pfit_control;
}

static int vlv_sp_calculate(struct intel_plane *planeptr,
			struct intel_buffer *buf,
			struct intel_plane_config *config)
{
	struct vlv_sp_plane *splane = to_vlv_sp_plane(planeptr);
	struct sp_plane_regs_value *regs = &splane->ctx.regs;
	struct intel_pipe *intel_pipe = config->pipe;
	struct vlv_pipeline *pipeline = to_vlv_pipeline_sp1_plane(splane);
	struct vlv_dc_config *vlv_config = pipeline->config;
	struct drm_mode_modeinfo mode;
	unsigned long sprsurf_offset, linear_offset;
	int sprite_ddl, prec_multi, sp_prec_multi;
	int plane = splane->ctx.plane;
	int pipe = splane->ctx.pipe;
	int s1_zorder, s1_bottom, s2_zorder, s2_bottom;
	int order = config->zorder & 0x000F;
	u32 hw_format = 0;
	u32 bpp = 0, prev_bpp = 0;
	u32 sprctl, prev_sprctl;
	u32 mask, shift;
	u32 src_x = config->src_x & VLV_SP_12BIT_MASK;
	u32 src_y = config->src_y & VLV_SP_12BIT_MASK;
	u32 dst_x = config->dst_x & VLV_SP_12BIT_MASK;
	u32 dst_y = config->dst_y & VLV_SP_12BIT_MASK;
	u32 dst_w = (config->dst_w & VLV_SP_12BIT_MASK) - 1;
	u32 dst_h = (config->dst_h & VLV_SP_12BIT_MASK) - 1;
	u32 src_w = (config->src_w & VLV_SP_12BIT_MASK) - 1;
	u8 i = 0;

	/*
	 * While disabling the panel fitter in decremental sequence, the scalar
	 * mode is decided on the present resolution.
	 */
	if (vlv_config->status.pfit_changed &&
			!(intel_pipe->pipe_reg.pfit_control & PFIT_ENABLE) &&
			planeptr->pipe)
		vlv_get_pfit_mode(config, planeptr);

	/* Z-order */
	s1_zorder = (order >> 3) & 0x1;
	s1_bottom = (order >> 2) & 0x1;
	s2_zorder = (order >> 1) & 0x1;
	s2_bottom = (order >> 0) & 0x1;

	get_format_config(buf->format, &hw_format, &bpp,
			config->alpha);
	sprctl = REG_READ(SPCNTR(pipe, plane));
	prev_sprctl = sprctl;

	if (plane == 0) {
		if (s1_zorder)
			sprctl |= SPRITE_ZORDER_ENABLE;
		else
			sprctl &= ~SPRITE_ZORDER_ENABLE;

		if (s1_bottom)
			sprctl |= SPRITE_FORCE_BOTTOM;
		else
			sprctl &= ~SPRITE_FORCE_BOTTOM;
	} else {
		if (s2_zorder)
			sprctl |= SPRITE_ZORDER_ENABLE;
		else
			sprctl &= ~SPRITE_ZORDER_ENABLE;
		if (s2_bottom)
			sprctl |= SPRITE_FORCE_BOTTOM;
		else
			sprctl &= ~SPRITE_FORCE_BOTTOM;
	}

	/* Mask out pixel format bits in case we change it */
	sprctl &= ~SP_PIXFORMAT_MASK;
	sprctl &= ~SP_YUV_BYTE_ORDER_MASK;
	sprctl &= ~SP_TILED;

	sprctl |= hw_format;
	sprctl |= SP_GAMMA_ENABLE;
	/* Calculate the ddl if there is a change in bpp */
	for (i = 0; i < ARRAY_SIZE(format_mappings); i++) {
		if (format_mappings[i].hw_config ==
				(prev_sprctl & SP_PIXFORMAT_MASK)) {
			prev_bpp = format_mappings[i].bpp;
			break;
		}
	}
	if (plane == 0) {
		mask = DDL_SPRITEA_MASK;
		shift = DDL_SPRITEA_SHIFT;
	} else {
		mask = DDL_SPRITEB_MASK;
		shift = DDL_SPRITEB_SHIFT;
	}
	if (bpp != prev_bpp || !(REG_READ(VLV_DDL(pipe)) & mask)) {
		intel_pipe->ops->get_current_mode(intel_pipe, &mode);
		vlv_calc_ddl(mode.clock, bpp, &prec_multi,
				&sprite_ddl);
		sp_prec_multi = (prec_multi ==
					DDL_PRECISION_L) ?
					DDL_PLANE_PRECISION_L :
					DDL_PLANE_PRECISION_H;
		sprite_ddl = (sp_prec_multi | sprite_ddl) << shift;
		vlv_sp_pane_save_ddl(splane, sprite_ddl);
		REG_WRITE_BITS(VLV_DDL(pipe), 0x00, mask);

	}

	sprctl |= SP_ENABLE;
	regs->dspcntr = sprctl;

	if (buf->tiling_mode != I915_TILING_NONE)
		regs->dspcntr |= SP_TILED;
	else
		regs->dspcntr &= ~SP_TILED;

	linear_offset = src_y * buf->stride + src_x * bpp;
	sprsurf_offset = vlv_compute_page_offset(&src_x, &src_y,
			buf->tiling_mode, bpp, buf->stride);
	linear_offset -= sprsurf_offset;

	regs->linearoff = linear_offset;
	regs->stride = buf->stride;
	regs->pos = ((dst_y << 16) | dst_x);
	regs->size = (dst_h << 16) | dst_w;
	regs->tileoff = (src_y << 16) | src_x;

	/*
	 * H mirroring available on PIPE B Pri and sp plane only
	 * For CHV, FLIPH and 180 are mutually exclusive
	 */
	if ((intel_adf_get_platform_id() == gen_cherryview) &&
	    STEP_FROM(pipeline->dc_stepping, STEP_B0))
		regs->dspcntr &= ~(DISPPLANE_H_MIRROR_ENABLE |
				   DISPPLANE_180_ROTATION_ENABLE);
	else
		regs->dspcntr &= ~DISPPLANE_180_ROTATION_ENABLE;

	switch (config->transform) {
	case INTEL_ADF_TRANSFORM_FLIPH:
		if ((intel_adf_get_platform_id() == gen_cherryview) &&
		    STEP_FROM(pipeline->dc_stepping, STEP_B0) &&
		    (pipe == PIPE_B)) {
			regs->dspcntr |= DISPPLANE_H_MIRROR_ENABLE;
			regs->tileoff = (src_y << 16) | (src_x + src_w - 1);
			regs->linearoff += ((src_w - 1) * bpp);
		}
		break;
	case INTEL_ADF_TRANSFORM_ROT180:
		regs->dspcntr |= DISPPLANE_180_ROTATION_ENABLE;
		regs->linearoff =  regs->linearoff + (dst_h - 1) *
			regs->stride + dst_w * bpp;
		regs->tileoff = (((src_y + dst_h - 1) << 16) |
			(src_x + dst_w - 1));
		break;
	}

	regs->surfaddr = (buf->gtt_offset_in_pages + sprsurf_offset);

	/* when in maxfifo display control register cannot be modified */
	if (vlv_config->status.maxfifo_enabled &&
					regs->dspcntr != prev_sprctl) {
		REG_WRITE(FW_BLC_SELF_VLV, ~FW_CSPWRDWNEN);
		vlv_config->status.maxfifo_enabled = false;
		pipeline->status.wait_vblank = true;
		pipeline->status.vsync_counter =
			intel_pipe->ops->get_vsync_counter(intel_pipe, 0);
	}

	return 0;
}

static int vlv_sp_attach(struct intel_plane *plane, struct intel_pipe *pipe)
{
	/* attach the requested plane to pipe */
	plane->pipe = pipe;

	return 0;
}

static int vlv_sp_validate(struct intel_plane *planeptr,
		struct intel_buffer *buf, struct intel_plane_config *config)
{
	struct intel_pipe *intel_pipe = config->pipe;
	struct drm_mode_modeinfo mode;
	u32 format_config, bpp;
	bool visible = false;
	u32 width, height;
	int max_downscale = 1;
	bool can_scale = false;
	int hscale, vscale;
	int max_scale;
	int min_scale;
	struct drm_rect clip;

	struct drm_rect src = {
		/* sample coordinates in 16.16 fixed point */
		.x1 = config->src_x,
		.x2 = config->src_x + config->src_w,
		.y1 = config->src_y,
		.y2 = config->src_y + config->src_h,
	};

	struct drm_rect dst = {
		/* integer pixels */
		.x1 = config->dst_x,
		.x2 = config->dst_x + config->dst_w,
		.y1 = config->dst_y,
		.y2 = config->dst_y + config->dst_h,
	};

	/* make sure the src rectangle in 16.16 fixed point format */
	if (!(config->src_w / (1 << 16)) ||
	    !(config->src_h / (1 << 16))) {
		pr_err("ADF:src rec are not in 16.16 fixed fmt%s\n", __func__);
		return -ERANGE;
	}

	if (config->src_x && !(config->src_x / (1 << 16))) {
		pr_err("ADF:src rec are not in 16.16 fixed fmt%s\n", __func__);
		return -ERANGE;
	}

	if (config->src_y && !(config->src_y / (1 << 16))) {
		pr_err("ADF:src rec are not in 16.16 fixed fmt%s\n", __func__);
		return -ERANGE;
	}

	intel_pipe->ops->get_current_mode(intel_pipe, &mode);

	clip.x1 = 0;
	clip.y1 = 0;
	if (intel_pipe->pipe_reg.pfit_control & PFIT_ENABLE) {
		clip.x2 = (((intel_pipe->pipe_reg.scaling_src_size >> 16) &
					0x0000FFFF) + 1);
		clip.y2 = (((intel_pipe->pipe_reg.scaling_src_size) &
					0x0000FFFF) + 1);
	} else {
		clip.x2 = mode.hdisplay;
		clip.y2 = mode.vdisplay;
	}

	if (get_format_config(buf->format, &format_config, &bpp,
				config->alpha)) {
		pr_err("ADF: pixel format not supported %s\n", __func__);
		return -EINVAL;
	}

	width = buf->w << 16;
	height = buf->h << 16;

	/* make sure src co-ordinates are inside the input buffer size */
	if (config->src_w > width ||
	    config->src_x > width - config->src_w ||
	    config->src_h > height ||
	    config->src_y > height - config->src_h) {
		pr_err("ADF: Invalid source co-ordinates %s\n", __func__);
		return -ENOSPC;
	}

	/* check against integer overflows */
	if (config->dst_w > INT_MAX ||
	    config->dst_x > INT_MAX - (int32_t) config->dst_w ||
	    config->dst_h > INT_MAX ||
	    config->dst_y > INT_MAX - (int32_t) config->dst_h) {
		pr_err("ADF: Invalid dst co-ordinates %s\n", __func__);
		return -ERANGE;
	}

	/* check buf limits */
	if (buf->w < 3 || buf->h < 3 || buf->stride > 16384) {
		pr_err("ADF: Unsutable fb for the plane %s\n", __func__);
		return -EINVAL;
	}

	/* sprite planes can be linear or x-tiled surfaces */
	if (buf->tiling_mode != I915_TILING_NONE &&
		buf->tiling_mode != I915_TILING_X) {
		pr_err("ADF: unsupported tiling mode %s\n", __func__);
		return -EINVAL;
	}
	max_scale = max_downscale << 16;
	min_scale = can_scale ? 1 : (1 << 16);

	hscale = drm_rect_calc_hscale_relaxed(&src, &dst, min_scale, max_scale);
	BUG_ON(hscale < 0);

	vscale = drm_rect_calc_vscale_relaxed(&src, &dst, min_scale, max_scale);
	BUG_ON(vscale < 0);

	visible = drm_rect_clip_scaled(&src, &dst, &clip, hscale, vscale);

	config->dst_x = dst.x1;
	config->dst_y = dst.y1;
	config->dst_w = (dst.x2 - dst.x1);
	config->dst_h = (dst.y2 - dst.y1);

	if (visible) {
		/* sanity check to make sure the src viewport wasn't enlarged */
		WARN_ON(src.x1 < (int) config->src_x ||
			src.y1 < (int) config->src_y ||
			src.x2 > (int) (config->src_x + config->src_w) ||
			src.y2 > (int) (config->src_y + config->src_h));

		/*
		 * Hardware doesn't handle subpixel coordinates.
		 * Adjust to (macro)pixel boundary
		 */
		config->src_x = src.x1 >> 16;
		config->src_w = (src.x2 - src.x1) >> 16;
		config->src_y = src.y1 >> 16;
		config->src_h = (src.y2 - src.y1) >> 16;

		if (format_is_yuv(buf->format)) {
			config->src_x &= ~1;
			config->src_w &= ~1;
			config->dst_w &= ~1;
		}
	} else {
		pr_err("ADF: plane is not visible %s\n", __func__);
		return -EINVAL;
	}

	return vlv_sp_calculate(planeptr, buf, config);
}

static void vlv_sp_flip(struct intel_plane *planeptr, struct intel_buffer *buf,
			struct intel_plane_config *config)
{
	struct vlv_sp_plane *splane = to_vlv_sp_plane(planeptr);
	struct sp_plane_regs_value *regs = &splane->ctx.regs;
	struct vlv_pipeline *pipeline = to_vlv_pipeline_sp1_plane(splane);
	struct vlv_dc_config *vlv_config = pipeline->config;
	int plane = splane->ctx.plane;
	int pipe = splane->ctx.pipe;
	u32 val = 0;

	REG_WRITE(SPSTRIDE(pipe, plane), regs->stride);
	REG_WRITE(SPPOS(pipe, plane), regs->pos);
	REG_WRITE(SPTILEOFF(pipe, plane), regs->tileoff);
	REG_WRITE(SPLINOFF(pipe, plane), regs->linearoff);
	REG_WRITE(SPSIZE(pipe, plane), regs->size);
	REG_WRITE(SPCNTR(pipe, plane), regs->dspcntr);

	I915_MODIFY_DISPBASE(SPSURF(pipe, plane), regs->surfaddr);
	REG_POSTING_READ(SPSURF(pipe, plane));
	splane->enabled = true;
	/* Check for reserved register bit 2 */
	val = REG_READ(SPSURF(pipe, plane));
	if (config->flags & INTEL_ADF_PLANE_HW_PRIVATE_1) {
		if (!(val & PLANE_RESERVED_REG_BIT_2_ENABLE)) {
			val |= PLANE_RESERVED_REG_BIT_2_ENABLE;
			REG_WRITE(SPSURF(pipe, plane), val);
		}
	} else if (val & PLANE_RESERVED_REG_BIT_2_ENABLE) {
		val &= ~PLANE_RESERVED_REG_BIT_2_ENABLE;
		REG_WRITE(SPSURF(pipe, plane), val);
	}
	vlv_update_plane_status(&vlv_config->base, planeptr->base.idx, true);

	return;
}

bool vlv_sp_plane_is_enabled(struct vlv_sp_plane *splane)
{
	return splane->enabled;
}

static int vlv_sp_enable(struct intel_plane *planeptr)
{
	struct vlv_sp_plane *splane = to_vlv_sp_plane(planeptr);
	struct vlv_pipeline *pipeline = to_vlv_pipeline_sp1_plane(splane);
	struct vlv_dc_config *vlv_config = pipeline->config;
	u32 reg, value;
	int plane = splane->ctx.plane;
	int pipe = splane->ctx.pipe;

	reg = SPCNTR(pipe, plane);
	value = REG_READ(reg);
	if (value & DISPLAY_PLANE_ENABLE) {
		return 0;
		dev_dbg(splane->base.base.dev, "%splane already enabled\n",
				__func__);
	}

	splane->enabled = true;
	REG_WRITE(reg, value | DISPLAY_PLANE_ENABLE);
	vlv_adf_flush_sp_plane(pipe, plane);
	vlv_update_plane_status(&vlv_config->base, planeptr->base.idx, true);
	/*
	 * TODO:No need to wait in case of mipi.
	 * Since data will flow only when port is enabled.
	 * wait for vblank will time out for mipi
	 */
	return 0;
}


static int vlv_sp_disable(struct intel_plane *planeptr)
{
	struct vlv_sp_plane *splane = to_vlv_sp_plane(planeptr);
	struct vlv_pipeline *pipeline = to_vlv_pipeline_sp1_plane(splane);
	struct vlv_dc_config *vlv_config = pipeline->config;
	u32 value, mask;
	int plane = splane->ctx.plane;
	int pipe = splane->ctx.pipe;

	value = REG_READ(splane->offset);
	if ((value & DISPLAY_PLANE_ENABLE) == 0) {
		dev_dbg(splane->base.base.dev, "%splane already disabled\n",
				__func__);
		return 0;
	}

	splane->enabled = false;
	REG_WRITE(splane->offset, value & ~DISPLAY_PLANE_ENABLE);
	vlv_adf_flush_sp_plane(pipe, plane);
	vlv_update_plane_status(&vlv_config->base, planeptr->base.idx, false);
	/* While disabling plane reset the plane DDL value */
	if (plane == 0)
		mask = DDL_SPRITEA_MASK;
	else
		mask = DDL_SPRITEB_MASK;

	REG_WRITE_BITS(VLV_DDL(pipe), 0x00, mask);

	return 0;
}

static const u32 sprite_supported_formats[] = {
	DRM_FORMAT_C8,
	DRM_FORMAT_YUYV,
	DRM_FORMAT_YVYU,
	DRM_FORMAT_UYVY,
	DRM_FORMAT_VYUY,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_XBGR2101010,
	DRM_FORMAT_ABGR2101010,
};

#if defined(CONFIG_ADF_INTEL_VLV) && !defined(CONFIG_ADF_INTEL_CHV)
static const u32 sprite_supported_transforms[] = {
	INTEL_ADF_TRANSFORM_ROT180,
};
#elif defined(CONFIG_ADF_INTEL_CHV)
static const u32 sprite_supported_transforms[] = {
	INTEL_ADF_TRANSFORM_FLIPH,
	INTEL_ADF_TRANSFORM_ROT180,
};
#endif

static const u32 sprite_supported_blendings[] = {
	INTEL_PLANE_BLENDING_NONE,
	INTEL_PLANE_BLENDING_PREMULT,
};

static const u32 sprite_supported_tiling[] = {
	INTEL_PLANE_TILE_NONE,
	INTEL_PLANE_TILE_X,
	INTEL_PLANE_TILE_Y,
};

static const u32 sprite_supported_zorder[] = {
	INTEL_PLANE_P1S1S2C1,
	INTEL_PLANE_P1S2S1C1,
	INTEL_PLANE_S2P1S1C1,
	INTEL_PLANE_S2S1P1C1,
	INTEL_PLANE_S1P1S2C1,
	INTEL_PLANE_S1S2P1C1,
};

static const u32 sprite_supported_reservedbit[] = {
	INTEL_PLANE_RESERVED_BIT_ZERO,
	INTEL_PLANE_RESERVED_BIT_SET,
};

static const struct intel_plane_ops vlv_sp_ops = {
	.base = {
		.suspend = vlv_sp_suspend,
		.resume = vlv_sp_resume,
	},
	.adf_ops = {
		.base = {
			.ioctl = intel_overlay_engine_obj_ioctl,
		},
		.supported_formats = sprite_supported_formats,
		.n_supported_formats = ARRAY_SIZE(sprite_supported_formats),
	},
	.attach = vlv_sp_attach,
	.validate = vlv_sp_validate,
	.flip = vlv_sp_flip,
	.enable = vlv_sp_enable,
	.disable = vlv_sp_disable,
};

static const struct intel_plane_capabilities vlv_sp_caps = {
	.supported_formats = sprite_supported_formats,
	.n_supported_formats = ARRAY_SIZE(sprite_supported_formats),
	.supported_blendings = sprite_supported_blendings,
	.n_supported_blendings = ARRAY_SIZE(sprite_supported_blendings),
	.supported_transforms = sprite_supported_transforms,
	.n_supported_transforms = ARRAY_SIZE(sprite_supported_transforms),
	.supported_scalings = NULL,
	.n_supported_scalings = 0,
	.supported_decompressions = NULL,
	.n_supported_decompressions = 0,
	.supported_tiling = sprite_supported_tiling,
	.n_supported_tiling = ARRAY_SIZE(sprite_supported_tiling),
	.supported_zorder = sprite_supported_zorder,
	.n_supported_zorder = ARRAY_SIZE(sprite_supported_zorder),
	.supported_reservedbit = sprite_supported_reservedbit,
	.n_supported_reservedbit = ARRAY_SIZE(sprite_supported_reservedbit),
};

int vlv_sp_plane_init(struct vlv_sp_plane *splane,
		struct intel_pipeline *pipeline, struct device *dev, u8 idx)
{
	int err;
	pr_debug("ADF: %s\n", __func__);

	if (!splane) {
		dev_err(dev, "data provided is NULL\n");
		return -EINVAL;
	}
	err = context_init(&splane->ctx, idx);

	splane->offset = SPCNTR(splane->ctx.pipe, splane->ctx.plane);

	if (err) {
		dev_err(dev, "failed to init sprite context\n");
		return err;
	}
	return intel_adf_plane_init(&splane->base, dev, idx, &vlv_sp_caps,
			&vlv_sp_ops, "sp_plane");
}

void vlv_sp_plane_destroy(struct vlv_sp_plane *splane)
{
	if (splane) {
		intel_plane_destroy(&splane->base);
		context_destroy(&splane->ctx);
	}
}
