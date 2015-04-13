/*
 * Copyright © 2011 Intel Corporation
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *   Jesse Barnes <jbarnes@virtuousgeek.org>
 *
 * New plane/sprite handling.
 *
 * The older chips had a separate interface for programming plane related
 * registers; newer ones are much simpler and we can use the new DRM plane
 * support.
 */
#include <drm/drmP.h>
#include <drm/drm_crtc.h>
#include <drm/drm_fourcc.h>
#include <drm/drm_rect.h>
#include "intel_drv.h"
#include <drm/i915_drm.h>
#include "i915_drv.h"

static int usecs_to_scanlines(const struct drm_display_mode *mode, int usecs)
{
	/* paranoia */
	if (!mode->crtc_htotal)
		return 1;

	return DIV_ROUND_UP(usecs * mode->crtc_clock, 1000 * mode->crtc_htotal);
}

static bool intel_pipe_update_start(struct intel_crtc *crtc, uint32_t *start_vbl_count)
{
	struct drm_device *dev = crtc->base.dev;
	const struct drm_display_mode *mode = &crtc->config.adjusted_mode;
	enum pipe pipe = crtc->pipe;
	long timeout = msecs_to_jiffies_timeout(1);
	int scanline, min, max, vblank_start;
	DEFINE_WAIT(wait);

	WARN_ON(!drm_modeset_is_locked(&crtc->base.mutex));

	vblank_start = mode->crtc_vblank_start;
	if (mode->flags & DRM_MODE_FLAG_INTERLACE)
		vblank_start = DIV_ROUND_UP(vblank_start, 2);

	/* FIXME needs to be calibrated sensibly */
	min = vblank_start - usecs_to_scanlines(mode, 100);
	max = vblank_start - 1;

	if (min <= 0 || max <= 0)
		return false;

	if (WARN_ON(drm_vblank_get(dev, pipe)))
		return false;

	if (intel_dsi_is_enc_on_crtc_cmd_mode(&crtc->base)) {
		/*
		 * In case of cmd mode the flips are triggered by software
		 * when mem write command is sent and hence the flips
		 * are already atomic.
		 *
		 * TBD: if more than one flip is requested by user space
		 * for a frame, then need to figure it and make mem_write is
		 * sent for the last flip.
		 */
		*start_vbl_count = dev->driver->get_vblank_counter(dev, pipe);
		return false;
	}

	local_irq_disable();

	trace_i915_pipe_update_start(crtc, min, max);

	for (;;) {
		/*
		 * prepare_to_wait() has a memory barrier, which guarantees
		 * other CPUs can see the task state update by the time we
		 * read the scanline.
		 */
		prepare_to_wait(&crtc->vbl_wait, &wait, TASK_UNINTERRUPTIBLE);

		scanline = intel_get_crtc_scanline(crtc);
		if (scanline < min || scanline > max)
			break;

		if (timeout <= 0) {
			DRM_ERROR("Potential atomic update failure on pipe %c\n",
				  pipe_name(crtc->pipe));
			break;
		}

		local_irq_enable();

		timeout = schedule_timeout(timeout);

		local_irq_disable();
	}

	finish_wait(&crtc->vbl_wait, &wait);

	drm_vblank_put(dev, pipe);

	*start_vbl_count = dev->driver->get_vblank_counter(dev, pipe);

	trace_i915_pipe_update_vblank_evaded(crtc, min, max, *start_vbl_count);

	return true;
}

static void intel_pipe_update_end(struct intel_crtc *crtc, u32 start_vbl_count)
{
	struct drm_device *dev = crtc->base.dev;
	enum pipe pipe = crtc->pipe;
	u32 end_vbl_count = dev->driver->get_vblank_counter(dev, pipe);

	trace_i915_pipe_update_end(crtc, end_vbl_count);

	local_irq_enable();

	if (start_vbl_count != end_vbl_count)
		DRM_ERROR("Atomic update failure on pipe %c (start=%u end=%u)\n",
			  pipe_name(pipe), start_vbl_count, end_vbl_count);
}

static void intel_update_primary_plane(struct drm_plane *dplane,
	struct intel_crtc *intel_crtc)
{
	struct drm_i915_private *dev_priv = intel_crtc->base.dev->dev_private;
	int dspreg = DSPCNTR(intel_crtc->plane);
	int plane = intel_crtc->plane;
	int pipe = intel_crtc->pipe;
	struct intel_plane *intel_plane = to_intel_plane(dplane);
	int mask = 0x000000ff;

	if (intel_crtc->primary_enabled) {
		intel_crtc->reg.cntr = I915_READ(dspreg) | DISPLAY_PLANE_ENABLE;
		intel_plane->reg.dspcntr =
			(I915_READ(dspreg) | DISPLAY_PLANE_ENABLE);
		intel_crtc->pri_update = true;
		intel_plane->pri_update = true;
		if (!intel_crtc->atomic_update)
			I915_WRITE(dspreg,
				I915_READ(dspreg) | DISPLAY_PLANE_ENABLE);
		dev_priv->pipe_plane_stat |=
				VLV_UPDATEPLANE_STAT_PRIM_PER_PIPE(pipe);
	}
	else {
		intel_crtc->reg.cntr =
			I915_READ(dspreg) & ~DISPLAY_PLANE_ENABLE;
		intel_plane->reg.dspcntr =
			I915_READ(dspreg) & ~DISPLAY_PLANE_ENABLE;
		intel_crtc->pri_update = true;
		intel_plane->pri_update = true;
		if (!intel_crtc->atomic_update) {
			I915_WRITE(dspreg,
				I915_READ(dspreg) & ~DISPLAY_PLANE_ENABLE);
			I915_WRITE(DSPSURF(plane),
				I915_READ(DSPSURF(plane)));

			intel_dsi_send_fb_on_crtc(&intel_crtc->base);
		}
		dev_priv->pipe_plane_stat &=
				~VLV_UPDATEPLANE_STAT_PRIM_PER_PIPE(pipe);
		I915_WRITE_BITS(VLV_DDL(pipe), 0x00, mask);
	}
}

void
__alpha_set_plane(u32 pixformat, int plane, u32 *dspcntr, int alpha)
{
	switch (pixformat) {
	case DISPPLANE_RGBX888:
		*dspcntr |= DISPPLANE_RGBX888;
		break;
	case DISPPLANE_RGBA888:
		if (alpha)
			*dspcntr |= DISPPLANE_RGBA888;
		else
			*dspcntr |= DISPPLANE_RGBX888;
		break;
	case DISPPLANE_BGRX888:
		*dspcntr |= DISPPLANE_BGRX888;
		break;
	case DISPPLANE_BGRA888:
		if (alpha)
			*dspcntr |= DISPPLANE_BGRA888;
		else
			*dspcntr |= DISPPLANE_BGRX888;
		break;
	case DISPPLANE_RGBX101010:
		*dspcntr |= DISPPLANE_RGBX101010;
		break;
	case DISPPLANE_RGBA101010:
		if (alpha)
			*dspcntr |= DISPPLANE_RGBA101010;
		else
			*dspcntr |= DISPPLANE_RGBX101010;
		break;
	case DISPPLANE_BGRX101010:
		*dspcntr |= DISPPLANE_BGRX101010;
		break;
	case DISPPLANE_BGRA101010:
		if (alpha)
			*dspcntr |= DISPPLANE_BGRA101010;
		else
			*dspcntr |= DISPPLANE_BGRX101010;
		break;
	case DISPPLANE_RGBX161616:
		*dspcntr |= DISPPLANE_RGBX161616;
		break;
	case DISPPLANE_RGBA161616:
		if (alpha)
			*dspcntr |= DISPPLANE_RGBA161616;
		else
			*dspcntr |= DISPPLANE_RGBX161616;
		break;
	default:
		DRM_ERROR("Unknown pixel format %x\n", pixformat);
		break;
	}
}

/*
 * enable/disable alpha for planes
 */
int
i915_set_plane_alpha(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_i915_set_plane_alpha *alphadata = data;
	int plane = alphadata->plane;
	bool alpha = alphadata->alpha;
	u32 cntrval, reg, pixformat;
	struct intel_plane *intel_plane;
	struct drm_mode_object *drmmode_obj;
	struct intel_crtc *intel_crtc;

	drmmode_obj = drm_mode_object_find(dev, plane,
			DRM_MODE_OBJECT_PLANE);

	if (drmmode_obj) {
		intel_plane = to_intel_plane(obj_to_plane(drmmode_obj));
		reg = SPCNTR(intel_plane->pipe, intel_plane->plane);
	} else {
		drmmode_obj = drm_mode_object_find(dev, plane,
			DRM_MODE_OBJECT_CRTC);
		if (drmmode_obj) {
			intel_crtc = to_intel_crtc(obj_to_crtc(drmmode_obj));
			reg = DSPCNTR(intel_crtc->plane);
		} else {
			DRM_ERROR("No such CRTC id for Plane or Sprite\n");
			return -EINVAL;
		}
	}

	cntrval = I915_READ(reg);
	pixformat = cntrval & DISPPLANE_PIXFORMAT_MASK;
	cntrval &= ~DISPPLANE_PIXFORMAT_MASK;

	if (pixformat) {
		__alpha_set_plane(pixformat, plane,
						&cntrval, alpha);
		if (cntrval & DISPPLANE_PIXFORMAT_MASK)
			if (cntrval != I915_READ(reg))
				I915_WRITE(reg, cntrval);
	} else
		DRM_ERROR("Plane might not be enabled/configured!\n");

	return 0;
}

void
__alpha_setting_cursor(u32 pixformat, int plane, u32 *dspcntr, int alpha)
{
	/* For readability, can split to individual cases */
	switch (pixformat) {
	case CURSOR_MODE_128_32B_AX:
	case CURSOR_MODE_128_ARGB_AX:
		if (alpha)
			*dspcntr |= CURSOR_MODE_128_ARGB_AX;
		else
			*dspcntr |= CURSOR_MODE_128_32B_AX;
		break;

	case CURSOR_MODE_256_ARGB_AX:
	case CURSOR_MODE_256_32B_AX:
		if (alpha)
			*dspcntr |= CURSOR_MODE_256_ARGB_AX;
		else
			*dspcntr |= CURSOR_MODE_256_32B_AX;
		break;

	case CURSOR_MODE_64_ARGB_AX:
	case CURSOR_MODE_64_32B_AX:
		if (alpha)
			*dspcntr |= CURSOR_MODE_64_ARGB_AX;
		else
			*dspcntr |= CURSOR_MODE_64_32B_AX;
		break;
	default:
		DRM_ERROR("Unknown pixel format:Cursor 0x%08x\n", pixformat);
		break;
	}
}

int i915_set_plane_zorder(struct drm_device *dev, void *data,
			  struct drm_file *file)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	u32 val = 0;
	struct drm_i915_set_plane_zorder *zorder = data;
	struct drm_mode_object *obj;
	struct intel_crtc *intel_crtc;
	int pipe;
	u32 order = zorder->order;
	int s1_zorder, s1_bottom, s2_zorder, s2_bottom;

	obj = drm_mode_object_find(dev, zorder->obj_id, DRM_MODE_OBJECT_CRTC);
	if (!obj) {
		DRM_ERROR("Unknown CRTC ID: %lu\n",
				(unsigned long)zorder->obj_id);
		return -EINVAL;
	}

	intel_crtc = to_intel_crtc(obj_to_crtc(obj));
	pipe = intel_crtc->pipe;

	s1_zorder = (order >> 3) & 0x1;
	s1_bottom = (order >> 2) & 0x1;
	s2_zorder = (order >> 1) & 0x1;
	s2_bottom = (order >> 0) & 0x1;

	if (intel_crtc->atomic_update)
		goto calc_zorder;

	/* Clear the older Z-order */
	val = I915_READ(SPCNTR(pipe, 0));
	if (dev_priv->maxfifo_enabled && !(val & SPRITE_ZORDER_ENABLE)) {
		intel_update_maxfifo(dev_priv, obj_to_crtc(obj), false);
		intel_wait_for_vblank(dev, pipe);
	}
	val &= ~(SPRITE_FORCE_BOTTOM | SPRITE_ZORDER_ENABLE);
	I915_WRITE(SPCNTR(pipe, 0), val);

	val = I915_READ(SPCNTR(pipe, 1));
	if (dev_priv->maxfifo_enabled && !(val & SPRITE_ZORDER_ENABLE)) {
		intel_update_maxfifo(dev_priv, obj_to_crtc(obj), false);
		intel_wait_for_vblank(dev, pipe);
	}
	val &= ~(SPRITE_FORCE_BOTTOM | SPRITE_ZORDER_ENABLE);
	I915_WRITE(SPCNTR(pipe, 1), val);

calc_zorder:

	/* Program new Z-order */
	if (!intel_crtc->atomic_update)
		val = I915_READ(SPCNTR(pipe, 0));
	if (s1_zorder)
		val |= SPRITE_ZORDER_ENABLE;
	if (s1_bottom)
		val |= SPRITE_FORCE_BOTTOM;
	if (intel_crtc->atomic_update)
		intel_crtc->reg.spacntr = val;
	else
		I915_WRITE(SPCNTR(pipe, 0), val);

	if (intel_crtc->atomic_update)
		val = 0;
	else
		val = I915_READ(SPCNTR(pipe, 1));
	if (s2_zorder)
		val |= SPRITE_ZORDER_ENABLE;
	if (s2_bottom)
		val |= SPRITE_FORCE_BOTTOM;
	if (intel_crtc->atomic_update)
		intel_crtc->reg.spbcntr = val;
	else
		I915_WRITE(SPCNTR(pipe, 1), val);

	return 0;
}

static void
vlv_update_plane(struct drm_plane *dplane, struct drm_crtc *crtc,
		 struct drm_framebuffer *fb,
		 struct drm_i915_gem_object *obj, int crtc_x, int crtc_y,
		 unsigned int crtc_w, unsigned int crtc_h,
		 uint32_t x, uint32_t y,
		 uint32_t src_w, uint32_t src_h,
		 struct drm_pending_vblank_event *event)
{
	struct drm_device *dev = dplane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(dplane);
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	int pipe = intel_plane->pipe;
	int plane = intel_plane->plane;
	int pipe_stat = VLV_PIPE_STATS(dev_priv->pipe_plane_stat);
	u32 sprctl;
	bool rotate = false;
	bool alpha_changed = false;
	bool yuv_format = false;
	unsigned long sprsurf_offset, linear_offset;
	int pixel_size = drm_format_plane_cpp(fb->pixel_format, 0);
	struct drm_display_mode *mode = &intel_crtc->config.requested_mode;
	u32 start_vbl_count;
	bool atomic_update = false;
	int sprite_ddl, sp_prec_multi;
	int ch, index;
	u32 mask, shift;

	sprctl = I915_READ(SPCNTR(pipe, plane));

	/* Mask out pixel format bits in case we change it */
	sprctl &= ~SP_PIXFORMAT_MASK;
	sprctl &= ~SP_YUV_BYTE_ORDER_MASK;
	sprctl &= ~SP_TILED;

	/* Update plane alpha */
	if (intel_plane->flags & DRM_MODE_SET_DISPLAY_PLANE_UPDATE_ALPHA) {
		alpha_changed = true;
		intel_plane->flags &= ~DRM_MODE_SET_DISPLAY_PLANE_UPDATE_ALPHA;
	}

	switch (fb->pixel_format) {
	case DRM_FORMAT_YUYV:
		sprctl |= SP_FORMAT_YUV422 | SP_YUV_ORDER_YUYV;
		yuv_format = true;
		break;
	case DRM_FORMAT_YVYU:
		sprctl |= SP_FORMAT_YUV422 | SP_YUV_ORDER_YVYU;
		yuv_format = true;
		break;
	case DRM_FORMAT_UYVY:
		sprctl |= SP_FORMAT_YUV422 | SP_YUV_ORDER_UYVY;
		yuv_format = true;
		break;
	case DRM_FORMAT_VYUY:
		sprctl |= SP_FORMAT_YUV422 | SP_YUV_ORDER_VYUY;
		yuv_format = true;
		break;
	case DRM_FORMAT_RGB565:
		sprctl |= SP_FORMAT_BGR565;
		break;
	case DRM_FORMAT_XRGB8888:
		sprctl |= SP_FORMAT_BGRX8888;
		break;
	case DRM_FORMAT_ARGB8888:
		if (alpha_changed && !intel_plane->alpha)
			sprctl |= SP_FORMAT_BGRX8888;
		else
			sprctl |= SP_FORMAT_BGRA8888;
		break;
	case DRM_FORMAT_XBGR2101010:
		sprctl |= SP_FORMAT_RGBX1010102;
		break;
	case DRM_FORMAT_ABGR2101010:
		if (alpha_changed && !intel_plane->alpha)
			sprctl |= SP_FORMAT_RGBX1010102;
		else
			sprctl |= SP_FORMAT_RGBA1010102;
		break;
	case DRM_FORMAT_XBGR8888:
		sprctl |= SP_FORMAT_RGBX8888;
		break;
	case DRM_FORMAT_ABGR8888:
		if (alpha_changed && !intel_plane->alpha)
			sprctl |= SP_FORMAT_RGBX8888;
		else
			sprctl |= SP_FORMAT_RGBA8888;
		break;
	default:
		/*
		 * If we get here one of the upper layers failed to filter
		 * out the unsupported plane formats
		 */
		BUG();
		break;
	}

	/*
	 * Enable gamma to match primary/cursor plane behaviour.
	 * FIXME should be user controllable via propertiesa.
	 */
	sprctl |= SP_GAMMA_ENABLE;

	if (obj->tiling_mode != I915_TILING_NONE)
		sprctl |= SP_TILED;
	else
		sprctl &= ~SP_TILED;

	sprctl |= SP_ENABLE;

	/* disable current DRRS work scheduled and restart
	 * to push work by another x seconds
	 */
	intel_restart_idleness_drrs(intel_crtc);

	if (!intel_crtc->atomic_update) {
		intel_update_sprite_watermarks(dplane, crtc, src_w, pixel_size,
				true, src_w != crtc_w || src_h != crtc_h);
	}

	if (intel_plane->rotate180)
		rotate = true;

	/* Sizes are 0 based */
	src_w--;
	src_h--;
	crtc_w--;
	crtc_h--;

	intel_plane->reg.pos = (crtc_y << 16) | crtc_x;
	if (!intel_crtc->atomic_update)
		I915_WRITE(SPPOS(pipe, plane), intel_plane->reg.pos);

	linear_offset = y * fb->pitches[0] + x * pixel_size;
	sprsurf_offset = intel_gen4_compute_page_offset(&x, &y,
							obj->tiling_mode,
							pixel_size,
							fb->pitches[0]);
	linear_offset -= sprsurf_offset;

	if (!intel_crtc->atomic_update) {
		atomic_update = intel_pipe_update_start(intel_crtc,
			&start_vbl_count);
		intel_update_primary_plane(dplane, intel_crtc);
	}

	/* if panel fitter is enabled program the input src size */
	if (intel_crtc->scaling_src_size &&
		(intel_crtc->config.gmch_pfit.control &	PFIT_ENABLE)) {
		intel_plane->reg.pfit_control =
				intel_crtc->config.gmch_pfit.control;
		intel_plane->reg.pipesrc = intel_crtc->scaling_src_size;
		if (!intel_crtc->atomic_update) {
			I915_WRITE(PFIT_CONTROL, intel_plane->reg.pfit_control);
			I915_WRITE(PIPESRC(pipe), intel_plane->reg.pipesrc);
			intel_crtc->pfit_en_status = true;
		}
	} else if (intel_crtc->pfit_en_status) {
		i9xx_get_pfit_mode(crtc, src_w, src_h);
		intel_plane->reg.pfit_control =
			intel_crtc->config.gmch_pfit.control;
		intel_plane->reg.pipesrc =
			((mode->hdisplay - 1) << SCALING_SRCSIZE_SHIFT) |
			(mode->vdisplay - 1);
		if (!intel_crtc->atomic_update) {
			I915_WRITE(PIPESRC(pipe), intel_plane->reg.pipesrc);
			I915_WRITE(PFIT_CONTROL, intel_plane->reg.pfit_control);
			intel_crtc->pfit_en_status = false;
		}
	}

	intel_plane->reg.stride = fb->pitches[0];
	if (!intel_crtc->atomic_update)
		I915_WRITE(SPSTRIDE(pipe, plane), intel_plane->reg.stride);

	if (obj->tiling_mode != I915_TILING_NONE) {
		if (rotate)
			intel_plane->reg.tileoff =
				((y + crtc_h) << 16) | (x + crtc_w);
		else
			intel_plane->reg.tileoff = (y << 16) | x;
		if (!intel_crtc->atomic_update)
			I915_WRITE(SPTILEOFF(pipe, plane),
				intel_plane->reg.tileoff);
	} else {
		if (rotate)
			intel_plane->reg.linoff = linear_offset +
					 crtc_h * fb->pitches[0] +
					 (crtc_w + 1) * pixel_size;
		else
			intel_plane->reg.linoff = linear_offset;
		if (!intel_crtc->atomic_update)
			I915_WRITE(SPLINOFF(pipe, plane),
				intel_plane->reg.linoff);
	}

	intel_plane->reg.size = (crtc_h << 16) | crtc_w;
	if (!intel_crtc->atomic_update)
		I915_WRITE(SPSIZE(pipe, plane), intel_plane->reg.size);

	if (rotate)
		sprctl |= DISPPLANE_180_ROTATION_ENABLE;
	else
		sprctl &= ~DISPPLANE_180_ROTATION_ENABLE;

	/* program csc registers */
	if (IS_CHERRYVIEW(dev) && STEP_FROM(STEP_B0) &&
		intel_plane->pipe == PIPE_B && yuv_format) {
		struct chv_sprite_csc *sp_csc =
			chv_sprite_cscs[intel_plane->csc_profile - 1];

		for (ch = SPCSC_YG; ch <= SPCSC_CR; ch++) {
			I915_WRITE(CHV_SPCSC_OFFSET(plane, ch),
				sp_csc->csc_val[ch][SPCSC_OUT].offset << 16 |
				sp_csc->csc_val[ch][SPCSC_IN].offset);

			I915_WRITE(CHV_SPCSC_CLAMP(plane, ch, SPCSC_IN),
				sp_csc->csc_val[ch][SPCSC_IN].max_clamp << 16 |
				sp_csc->csc_val[ch][SPCSC_IN].min_clamp);

			I915_WRITE(CHV_SPCSC_CLAMP(plane, ch, SPCSC_OUT),
				sp_csc->csc_val[ch][SPCSC_OUT].max_clamp << 16 |
				sp_csc->csc_val[ch][SPCSC_OUT].min_clamp);
		}

		for (index = 0; index < (CHV_NUM_SPCSC_COEFFS-1); index += 2) {
			I915_WRITE(CHV_SPCSC_COEFFS(plane, index),
					sp_csc->coeff[index+1] << 16 |
					sp_csc->coeff[index]);
		}
		I915_WRITE(CHV_SPCSC_C8(plane),
				sp_csc->coeff[CHV_NUM_SPCSC_COEFFS-1]);
	}

	/* When in maxfifo dspcntr cannot be changed */
	if (sprctl != I915_READ(SPCNTR(pipe, plane)) &&
				dev_priv->maxfifo_enabled &&
				intel_crtc->atomic_update) {
		intel_update_maxfifo(dev_priv, crtc, false);
		dev_priv->wait_vbl = true;
		dev_priv->vblcount =
			atomic_read(&dev->vblank[intel_crtc->pipe].count);
	}

	/*
	 * calculate the DDL and set to 0 is there is a change. Else cache
	 * the value and wrrite on next vblank.
	 */
	if (intel_plane->plane == 0) {
		mask = 0x0000ff00;
		shift = DDL_SPRITEA_SHIFT;
	} else {
		mask = 0x00ff0000;
		shift = DDL_SPRITEB_SHIFT;
	}

	vlv_calculate_ddl(crtc, pixel_size, &sp_prec_multi, &sprite_ddl);
	sprite_ddl = (sp_prec_multi | sprite_ddl) << shift;

	/*
	 * The current Dl formula doesnt consider multipipe
	 * cases, Use this value suggested by sv till the
	 * actual formula gets used, same applies for all
	 * hdmi cases. Since secondary display comes on PIPEC
	 * we are checking for pipe C, pipe_stat variable
	 * tells us the number of pipes enabled.
	 */
	if (IS_CHERRYVIEW(dev))
		if (!single_pipe_enabled(pipe_stat) ||
				(pipe_stat & PIPE_ENABLE(PIPE_C)))
			sprite_ddl = DDL_MULTI_PIPE_CHV << shift;

	if (intel_plane->plane) {
		intel_crtc->reg_ddl.spriteb_ddl = sprite_ddl;
		intel_crtc->reg_ddl.spriteb_ddl_mask = mask;
	} else {
		intel_crtc->reg_ddl.spritea_ddl = sprite_ddl;
		intel_crtc->reg_ddl.spritea_ddl_mask = mask;
	}
	if ((sprite_ddl & mask) != (I915_READ(VLV_DDL(pipe)) & mask))
		I915_WRITE_BITS(VLV_DDL(pipe), 0x00, mask);

	/* calculate  watermark */
	if (intel_plane->plane == 0)
		intel_crtc->vlv_wm.sa = vlv_calculate_wm(intel_crtc,
							pixel_size);
	else
		intel_crtc->vlv_wm.sb = vlv_calculate_wm(intel_crtc,
							pixel_size);

	intel_crtc->vlv_wm.sr = vlv_calculate_wm(intel_crtc,
							pixel_size);
	intel_plane->reg.surf = I915_READ(SPSURF(pipe, plane));

	if (intel_plane->rrb2_enable)
		intel_plane->reg.surf |= PLANE_RESERVED_REG_BIT_2_ENABLE;
	else
		intel_plane->reg.surf &= ~PLANE_RESERVED_REG_BIT_2_ENABLE;

	intel_plane->reg.cntr = sprctl;
	intel_plane->reg.surf &= ~DISP_BASEADDR_MASK;
	intel_plane->reg.surf |= i915_gem_obj_ggtt_offset(obj) + sprsurf_offset;
	if (!intel_crtc->atomic_update) {
		I915_WRITE(SPCNTR(pipe, plane), sprctl);
		I915_MODIFY_DISPBASE(SPSURF(pipe, plane),
			i915_gem_obj_ggtt_offset(obj) + sprsurf_offset);

		intel_dsi_send_fb_on_crtc(crtc);
	}

	dev_priv->pipe_plane_stat |=
			VLV_UPDATEPLANE_STAT_SP_PER_PIPE(pipe, plane);

	if (!intel_crtc->atomic_update)
		intel_flush_primary_plane(dev_priv, intel_crtc->plane);

	if (event == NULL)
		POSTING_READ(SPSURF(pipe, plane));

	if (!intel_crtc->atomic_update)
		intel_update_sprite_watermarks(dplane, crtc, src_w, pixel_size,
				true, src_w != crtc_w || src_h != crtc_h);

	if (!intel_crtc->atomic_update) {
		if (atomic_update)
			intel_pipe_update_end(intel_crtc, start_vbl_count);
	}
}

static void
vlv_disable_plane(struct drm_plane *dplane, struct drm_crtc *crtc)
{
	struct drm_device *dev = dplane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(dplane);
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	int pipe = intel_plane->pipe;
	int plane = intel_plane->plane;
	u32 start_vbl_count;
	bool atomic_update = false;
	u32 mask, shift;

	if (!intel_crtc->atomic_update) {
		atomic_update = intel_pipe_update_start(intel_crtc,
			&start_vbl_count);
		intel_update_primary_plane(dplane, intel_crtc);
	}

	dev_priv->pipe_plane_stat &=
			~VLV_UPDATEPLANE_STAT_SP_PER_PIPE(pipe, plane);

	intel_plane->reg.cntr = I915_READ(SPCNTR(pipe, plane)) & ~SP_ENABLE;
	if (!intel_crtc->atomic_update)
		I915_WRITE(SPCNTR(pipe, plane), I915_READ(SPCNTR(pipe, plane)) &
			~SP_ENABLE);

	/* Activate double buffered register update */
	intel_plane->reg.surf = 0;
	if (!intel_crtc->atomic_update) {
		I915_MODIFY_DISPBASE(SPSURF(pipe, plane), 0);
		POSTING_READ(SPSURF(pipe, plane));
	}

	if (!intel_crtc->atomic_update) {
		intel_flush_primary_plane(dev_priv, intel_crtc->plane);
		vlv_update_dsparb(intel_crtc);
		if (atomic_update)
			intel_pipe_update_end(intel_crtc, start_vbl_count);
	}

	if (!intel_crtc->atomic_update)
		intel_update_sprite_watermarks(dplane,
			crtc, 0, 0, false, false);
	intel_plane->last_plane_state = INTEL_PLANE_STATE_DISABLED;

	/* set to 0 as the plane is disabled */
	if (intel_plane->plane == 0) {
		mask = 0x0000ff00;
		shift = DDL_SPRITEA_SHIFT;
	} else {
		mask = 0x00ff0000;
		shift = DDL_SPRITEB_SHIFT;
	}
	I915_WRITE_BITS(VLV_DDL(pipe), 0x00, mask);
}

void intel_prepare_sprite_page_flip(struct drm_device *dev, int plane)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_crtc *intel_crtc =
		to_intel_crtc(dev_priv->plane_to_crtc_mapping[plane]);
	unsigned long flags;

	spin_lock_irqsave(&dev->event_lock, flags);

	if (intel_crtc->sprite_unpin_work) {
		atomic_inc(&intel_crtc->sprite_unpin_work->pending);
		if (atomic_read(&intel_crtc->sprite_unpin_work->pending) > 1)
			DRM_ERROR("Prepared flip multiple times\n");
	}

	spin_unlock_irqrestore(&dev->event_lock, flags);
}

void intel_finish_sprite_page_flip(struct drm_device *dev, int pipe)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_crtc *crtc = dev_priv->pipe_to_crtc_mapping[pipe];
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	struct intel_unpin_work *work;
	struct drm_i915_gem_object *obj;
	unsigned long flags;

	/* Ignore early vblank irqs */
	if (intel_crtc == NULL)
		return;

	/* Program the precalculated DDL value */
	if (intel_crtc->reg_ddl.spritea_ddl) {
		I915_WRITE_BITS(VLV_DDL(pipe), intel_crtc->reg_ddl.spritea_ddl,
			intel_crtc->reg_ddl.spritea_ddl_mask);
		intel_crtc->reg_ddl.spritea_ddl = 0;
	}
	if (intel_crtc->reg_ddl.spriteb_ddl) {
		I915_WRITE_BITS(VLV_DDL(pipe), intel_crtc->reg_ddl.spriteb_ddl,
			intel_crtc->reg_ddl.spriteb_ddl_mask);
		intel_crtc->reg_ddl.spriteb_ddl = 0;
	}

	spin_lock_irqsave(&dev->event_lock, flags);
	work = intel_crtc->sprite_unpin_work;

	if (work == NULL || !atomic_read(&work->pending)) {
		spin_unlock_irqrestore(&dev->event_lock, flags);
		return;
	}

	intel_crtc->sprite_unpin_work = NULL;
	if (work->event)
		drm_send_vblank_event(dev, intel_crtc->pipe, work->event);

	drm_vblank_put(dev, intel_crtc->pipe);
	spin_unlock_irqrestore(&dev->event_lock, flags);

	if (work->old_fb_obj != NULL) {
		obj = work->old_fb_obj;

		atomic_clear_mask(1 << intel_crtc->plane,
			&obj->pending_flip.counter);

		if (atomic_read(&obj->pending_flip) == 0)
			wake_up_all(&dev_priv->pending_flip_queue);
	} else
		wake_up_all(&dev_priv->pending_flip_queue);

	queue_work(dev_priv->wq, &work->work);
	trace_i915_flip_complete(intel_crtc->plane, work->pending_flip_obj);
}

void intel_unpin_sprite_work_fn(struct work_struct *__work)
{
	struct intel_unpin_work *work =
			container_of(__work, struct intel_unpin_work, work);
	struct drm_device *dev = work->crtc->dev;
	mutex_lock(&dev->struct_mutex);
	if (work->old_fb_obj != NULL)
		intel_unpin_fb_obj(work->old_fb_obj);
	mutex_unlock(&dev->struct_mutex);

	kfree(work);
}

static int
vlv_update_colorkey(struct drm_plane *dplane,
		    struct drm_intel_sprite_colorkey *key)
{
	struct drm_device *dev = dplane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(dplane);
	int pipe = intel_plane->pipe;
	int plane = intel_plane->plane;
	u32 sprctl;

	if (key->flags & I915_SET_COLORKEY_DESTINATION)
		return -EINVAL;

	I915_WRITE(SPKEYMINVAL(pipe, plane), key->min_value);
	I915_WRITE(SPKEYMAXVAL(pipe, plane), key->max_value);
	I915_WRITE(SPKEYMSK(pipe, plane), key->channel_mask);

	sprctl = I915_READ(SPCNTR(pipe, plane));
	sprctl &= ~SP_SOURCE_KEY;
	if (key->flags & I915_SET_COLORKEY_SOURCE)
		sprctl |= SP_SOURCE_KEY;
	I915_WRITE(SPCNTR(pipe, plane), sprctl);

	POSTING_READ(SPKEYMSK(pipe, plane));

	return 0;
}

static void
vlv_get_colorkey(struct drm_plane *dplane,
		 struct drm_intel_sprite_colorkey *key)
{
	struct drm_device *dev = dplane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(dplane);
	int pipe = intel_plane->pipe;
	int plane = intel_plane->plane;
	u32 sprctl;

	key->min_value = I915_READ(SPKEYMINVAL(pipe, plane));
	key->max_value = I915_READ(SPKEYMAXVAL(pipe, plane));
	key->channel_mask = I915_READ(SPKEYMSK(pipe, plane));

	sprctl = I915_READ(SPCNTR(pipe, plane));
	if (sprctl & SP_SOURCE_KEY)
		key->flags = I915_SET_COLORKEY_SOURCE;
	else
		key->flags = I915_SET_COLORKEY_NONE;
}

static void
ivb_update_plane(struct drm_plane *plane, struct drm_crtc *crtc,
		 struct drm_framebuffer *fb,
		 struct drm_i915_gem_object *obj, int crtc_x, int crtc_y,
		 unsigned int crtc_w, unsigned int crtc_h,
		 uint32_t x, uint32_t y,
		 uint32_t src_w, uint32_t src_h,
		 struct drm_pending_vblank_event *event)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(plane);
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	int pipe = intel_plane->pipe;
	u32 sprctl, sprscale = 0;
	unsigned long sprsurf_offset, linear_offset;
	int pixel_size = drm_format_plane_cpp(fb->pixel_format, 0);
	u32 start_vbl_count;
	bool atomic_update;

	sprctl = I915_READ(SPRCTL(pipe));

	/* Mask out pixel format bits in case we change it */
	sprctl &= ~SPRITE_PIXFORMAT_MASK;
	sprctl &= ~SPRITE_RGB_ORDER_RGBX;
	sprctl &= ~SPRITE_YUV_BYTE_ORDER_MASK;
	sprctl &= ~SPRITE_TILED;

	switch (fb->pixel_format) {
	case DRM_FORMAT_XBGR8888:
		sprctl |= SPRITE_FORMAT_RGBX888 | SPRITE_RGB_ORDER_RGBX;
		break;
	case DRM_FORMAT_XRGB8888:
		sprctl |= SPRITE_FORMAT_RGBX888;
		break;
	case DRM_FORMAT_YUYV:
		sprctl |= SPRITE_FORMAT_YUV422 | SPRITE_YUV_ORDER_YUYV;
		break;
	case DRM_FORMAT_YVYU:
		sprctl |= SPRITE_FORMAT_YUV422 | SPRITE_YUV_ORDER_YVYU;
		break;
	case DRM_FORMAT_UYVY:
		sprctl |= SPRITE_FORMAT_YUV422 | SPRITE_YUV_ORDER_UYVY;
		break;
	case DRM_FORMAT_VYUY:
		sprctl |= SPRITE_FORMAT_YUV422 | SPRITE_YUV_ORDER_VYUY;
		break;
	default:
		BUG();
	}

	/*
	 * Enable gamma to match primary/cursor plane behaviour.
	 * FIXME should be user controllable via propertiesa.
	 */
	sprctl |= SPRITE_GAMMA_ENABLE;

	if (obj->tiling_mode != I915_TILING_NONE)
		sprctl |= SPRITE_TILED;

	if (IS_HASWELL(dev) || IS_BROADWELL(dev))
		sprctl &= ~SPRITE_TRICKLE_FEED_DISABLE;
	else
		sprctl |= SPRITE_TRICKLE_FEED_DISABLE;

	sprctl |= SPRITE_ENABLE;

	if (IS_HASWELL(dev) || IS_BROADWELL(dev))
		sprctl |= SPRITE_PIPE_CSC_ENABLE;

	intel_update_sprite_watermarks(plane, crtc, src_w, pixel_size, true,
				       src_w != crtc_w || src_h != crtc_h);

	/* Sizes are 0 based */
	src_w--;
	src_h--;
	crtc_w--;
	crtc_h--;

	if (crtc_w != src_w || crtc_h != src_h)
		sprscale = SPRITE_SCALE_ENABLE | (src_w << 16) | src_h;

	linear_offset = y * fb->pitches[0] + x * pixel_size;
	sprsurf_offset =
		intel_gen4_compute_page_offset(&x, &y, obj->tiling_mode,
					       pixel_size, fb->pitches[0]);
	linear_offset -= sprsurf_offset;

	atomic_update = intel_pipe_update_start(intel_crtc, &start_vbl_count);

	intel_update_primary_plane(plane, intel_crtc);

	I915_WRITE(SPRSTRIDE(pipe), fb->pitches[0]);
	I915_WRITE(SPRPOS(pipe), (crtc_y << 16) | crtc_x);

	/* HSW consolidates SPRTILEOFF and SPRLINOFF into a single SPROFFSET
	 * register */
	if (IS_HASWELL(dev) || IS_BROADWELL(dev))
		I915_WRITE(SPROFFSET(pipe), (y << 16) | x);
	else if (obj->tiling_mode != I915_TILING_NONE)
		I915_WRITE(SPRTILEOFF(pipe), (y << 16) | x);
	else
		I915_WRITE(SPRLINOFF(pipe), linear_offset);

	I915_WRITE(SPRSIZE(pipe), (crtc_h << 16) | crtc_w);
	if (intel_plane->can_scale)
		I915_WRITE(SPRSCALE(pipe), sprscale);
	I915_WRITE(SPRCTL(pipe), sprctl);
	I915_MODIFY_DISPBASE(SPRSURF(pipe),
		   i915_gem_obj_ggtt_offset(obj) + sprsurf_offset);

	intel_flush_primary_plane(dev_priv, intel_crtc->plane);

	if (atomic_update)
		intel_pipe_update_end(intel_crtc, start_vbl_count);
}

static void
ivb_disable_plane(struct drm_plane *plane, struct drm_crtc *crtc)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(plane);
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	int pipe = intel_plane->pipe;
	u32 start_vbl_count;
	bool atomic_update;

	atomic_update = intel_pipe_update_start(intel_crtc, &start_vbl_count);

	intel_update_primary_plane(plane, intel_crtc);

	I915_WRITE(SPRCTL(pipe), I915_READ(SPRCTL(pipe)) & ~SPRITE_ENABLE);
	/* Can't leave the scaler enabled... */
	if (intel_plane->can_scale)
		I915_WRITE(SPRSCALE(pipe), 0);

	/* Scheduling the sprite disable to corresponding flip */
	to_intel_crtc(crtc)->disable_sprite = true;

	intel_flush_primary_plane(dev_priv, intel_crtc->plane);

	if (atomic_update)
		intel_pipe_update_end(intel_crtc, start_vbl_count);

	/*
	 * Avoid underruns when disabling the sprite.
	 * FIXME remove once watermark updates are done properly.
	 */
	intel_wait_for_vblank(dev, pipe);

	intel_update_sprite_watermarks(plane, crtc, 0, 0, false, false);
}

static int
ivb_update_colorkey(struct drm_plane *plane,
		    struct drm_intel_sprite_colorkey *key)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane;
	u32 sprctl;
	int ret = 0;

	intel_plane = to_intel_plane(plane);

	I915_WRITE(SPRKEYVAL(intel_plane->pipe), key->min_value);
	I915_WRITE(SPRKEYMAX(intel_plane->pipe), key->max_value);
	I915_WRITE(SPRKEYMSK(intel_plane->pipe), key->channel_mask);

	sprctl = I915_READ(SPRCTL(intel_plane->pipe));
	sprctl &= ~(SPRITE_SOURCE_KEY | SPRITE_DEST_KEY);
	if (key->flags & I915_SET_COLORKEY_DESTINATION)
		sprctl |= SPRITE_DEST_KEY;
	else if (key->flags & I915_SET_COLORKEY_SOURCE)
		sprctl |= SPRITE_SOURCE_KEY;
	I915_WRITE(SPRCTL(intel_plane->pipe), sprctl);

	POSTING_READ(SPRKEYMSK(intel_plane->pipe));

	return ret;
}

static void
ivb_get_colorkey(struct drm_plane *plane, struct drm_intel_sprite_colorkey *key)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane;
	u32 sprctl;

	intel_plane = to_intel_plane(plane);

	key->min_value = I915_READ(SPRKEYVAL(intel_plane->pipe));
	key->max_value = I915_READ(SPRKEYMAX(intel_plane->pipe));
	key->channel_mask = I915_READ(SPRKEYMSK(intel_plane->pipe));
	key->flags = 0;

	sprctl = I915_READ(SPRCTL(intel_plane->pipe));

	if (sprctl & SPRITE_DEST_KEY)
		key->flags = I915_SET_COLORKEY_DESTINATION;
	else if (sprctl & SPRITE_SOURCE_KEY)
		key->flags = I915_SET_COLORKEY_SOURCE;
	else
		key->flags = I915_SET_COLORKEY_NONE;
}

static u32
ivb_current_surface(struct drm_plane *plane)
{
	struct intel_plane *intel_plane;

	intel_plane = to_intel_plane(plane);

	return SPRSURFLIVE(intel_plane->pipe);
}

static void
ilk_update_plane(struct drm_plane *plane, struct drm_crtc *crtc,
		 struct drm_framebuffer *fb,
		 struct drm_i915_gem_object *obj, int crtc_x, int crtc_y,
		 unsigned int crtc_w, unsigned int crtc_h,
		 uint32_t x, uint32_t y,
		 uint32_t src_w, uint32_t src_h,
		 struct drm_pending_vblank_event *event)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(plane);
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	int pipe = intel_plane->pipe;
	unsigned long dvssurf_offset, linear_offset;
	u32 dvscntr, dvsscale;
	int pixel_size = drm_format_plane_cpp(fb->pixel_format, 0);
	u32 start_vbl_count;
	bool atomic_update;

	dvscntr = I915_READ(DVSCNTR(pipe));

	/* Mask out pixel format bits in case we change it */
	dvscntr &= ~DVS_PIXFORMAT_MASK;
	dvscntr &= ~DVS_RGB_ORDER_XBGR;
	dvscntr &= ~DVS_YUV_BYTE_ORDER_MASK;
	dvscntr &= ~DVS_TILED;

	switch (fb->pixel_format) {
	case DRM_FORMAT_XBGR8888:
		dvscntr |= DVS_FORMAT_RGBX888 | DVS_RGB_ORDER_XBGR;
		break;
	case DRM_FORMAT_XRGB8888:
		dvscntr |= DVS_FORMAT_RGBX888;
		break;
	case DRM_FORMAT_YUYV:
		dvscntr |= DVS_FORMAT_YUV422 | DVS_YUV_ORDER_YUYV;
		break;
	case DRM_FORMAT_YVYU:
		dvscntr |= DVS_FORMAT_YUV422 | DVS_YUV_ORDER_YVYU;
		break;
	case DRM_FORMAT_UYVY:
		dvscntr |= DVS_FORMAT_YUV422 | DVS_YUV_ORDER_UYVY;
		break;
	case DRM_FORMAT_VYUY:
		dvscntr |= DVS_FORMAT_YUV422 | DVS_YUV_ORDER_VYUY;
		break;
	default:
		BUG();
	}

	/*
	 * Enable gamma to match primary/cursor plane behaviour.
	 * FIXME should be user controllable via propertiesa.
	 */
	dvscntr |= DVS_GAMMA_ENABLE;

	if (obj->tiling_mode != I915_TILING_NONE)
		dvscntr |= DVS_TILED;

	if (IS_GEN6(dev))
		dvscntr |= DVS_TRICKLE_FEED_DISABLE; /* must disable */
	dvscntr |= DVS_ENABLE;

	intel_update_sprite_watermarks(plane, crtc, src_w, pixel_size, true,
				       src_w != crtc_w || src_h != crtc_h);

	/* Sizes are 0 based */
	src_w--;
	src_h--;
	crtc_w--;
	crtc_h--;

	dvsscale = 0;
	if (crtc_w != src_w || crtc_h != src_h)
		dvsscale = DVS_SCALE_ENABLE | (src_w << 16) | src_h;

	linear_offset = y * fb->pitches[0] + x * pixel_size;
	dvssurf_offset =
		intel_gen4_compute_page_offset(&x, &y, obj->tiling_mode,
					       pixel_size, fb->pitches[0]);
	linear_offset -= dvssurf_offset;

	atomic_update = intel_pipe_update_start(intel_crtc, &start_vbl_count);

	intel_update_primary_plane(plane, intel_crtc);

	I915_WRITE(DVSSTRIDE(pipe), fb->pitches[0]);
	I915_WRITE(DVSPOS(pipe), (crtc_y << 16) | crtc_x);

	if (obj->tiling_mode != I915_TILING_NONE)
		I915_WRITE(DVSTILEOFF(pipe), (y << 16) | x);
	else
		I915_WRITE(DVSLINOFF(pipe), linear_offset);

	I915_WRITE(DVSSIZE(pipe), (crtc_h << 16) | crtc_w);
	I915_WRITE(DVSSCALE(pipe), dvsscale);
	I915_WRITE(DVSCNTR(pipe), dvscntr);
	I915_MODIFY_DISPBASE(DVSSURF(pipe),
		   i915_gem_obj_ggtt_offset(obj) + dvssurf_offset);

	intel_flush_primary_plane(dev_priv, intel_crtc->plane);

	if (atomic_update)
		intel_pipe_update_end(intel_crtc, start_vbl_count);
}

static void
ilk_disable_plane(struct drm_plane *plane, struct drm_crtc *crtc)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(plane);
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	int pipe = intel_plane->pipe;
	u32 start_vbl_count;
	bool atomic_update;

	atomic_update = intel_pipe_update_start(intel_crtc, &start_vbl_count);

	intel_update_primary_plane(plane, intel_crtc);

	I915_WRITE(DVSCNTR(pipe), I915_READ(DVSCNTR(pipe)) & ~DVS_ENABLE);
	/* Disable the scaler */
	I915_WRITE(DVSSCALE(pipe), 0);
	/* Flush double buffered register updates */
	I915_MODIFY_DISPBASE(DVSSURF(pipe), 0);

	intel_flush_primary_plane(dev_priv, intel_crtc->plane);

	if (atomic_update)
		intel_pipe_update_end(intel_crtc, start_vbl_count);

	/*
	 * Avoid underruns when disabling the sprite.
	 * FIXME remove once watermark updates are done properly.
	 */
	intel_wait_for_vblank(dev, pipe);

	intel_update_sprite_watermarks(plane, crtc, 0, 0, false, false);
}

static void
intel_post_enable_primary(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);

	/*
	 * BDW signals flip done immediately if the plane
	 * is disabled, even if the plane enable is already
	 * armed to occur at the next vblank :(
	 */
	if (IS_BROADWELL(dev))
		intel_wait_for_vblank(dev, intel_crtc->pipe);

	/*
	 * FIXME IPS should be fine as long as one plane is
	 * enabled, but in practice it seems to have problems
	 * when going from primary only to sprite only and vice
	 * versa.
	 */
	hsw_enable_ips(intel_crtc);

	mutex_lock(&dev->struct_mutex);
	intel_update_fbc(dev);
	intel_restart_idleness_drrs(intel_crtc);
	mutex_unlock(&dev->struct_mutex);
}

static void
intel_pre_disable_primary(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);

	mutex_lock(&dev->struct_mutex);
	if (dev_priv->fbc.plane == intel_crtc->plane)
		intel_disable_fbc(dev);
	mutex_unlock(&dev->struct_mutex);

	/*
	 * FIXME IPS should be fine as long as one plane is
	 * enabled, but in practice it seems to have problems
	 * when going from primary only to sprite only and vice
	 * versa.
	 */
	hsw_disable_ips(intel_crtc);
}

static int
ilk_update_colorkey(struct drm_plane *plane,
		    struct drm_intel_sprite_colorkey *key)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane;
	u32 dvscntr;
	int ret = 0;

	intel_plane = to_intel_plane(plane);

	I915_WRITE(DVSKEYVAL(intel_plane->pipe), key->min_value);
	I915_WRITE(DVSKEYMAX(intel_plane->pipe), key->max_value);
	I915_WRITE(DVSKEYMSK(intel_plane->pipe), key->channel_mask);

	dvscntr = I915_READ(DVSCNTR(intel_plane->pipe));
	dvscntr &= ~(DVS_SOURCE_KEY | DVS_DEST_KEY);
	if (key->flags & I915_SET_COLORKEY_DESTINATION)
		dvscntr |= DVS_DEST_KEY;
	else if (key->flags & I915_SET_COLORKEY_SOURCE)
		dvscntr |= DVS_SOURCE_KEY;
	I915_WRITE(DVSCNTR(intel_plane->pipe), dvscntr);

	POSTING_READ(DVSKEYMSK(intel_plane->pipe));

	return ret;
}

static void
ilk_get_colorkey(struct drm_plane *plane, struct drm_intel_sprite_colorkey *key)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane;
	u32 dvscntr;

	intel_plane = to_intel_plane(plane);

	key->min_value = I915_READ(DVSKEYVAL(intel_plane->pipe));
	key->max_value = I915_READ(DVSKEYMAX(intel_plane->pipe));
	key->channel_mask = I915_READ(DVSKEYMSK(intel_plane->pipe));
	key->flags = 0;

	dvscntr = I915_READ(DVSCNTR(intel_plane->pipe));

	if (dvscntr & DVS_DEST_KEY)
		key->flags = I915_SET_COLORKEY_DESTINATION;
	else if (dvscntr & DVS_SOURCE_KEY)
		key->flags = I915_SET_COLORKEY_SOURCE;
	else
		key->flags = I915_SET_COLORKEY_NONE;
}

static bool
format_is_yuv(uint32_t format)
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

static bool colorkey_enabled(struct intel_plane *intel_plane)
{
	struct drm_intel_sprite_colorkey key;

	intel_plane->get_colorkey(&intel_plane->base, &key);

	return key.flags != I915_SET_COLORKEY_NONE;
}

static u32
ilk_current_surface(struct drm_plane *plane)
{
	struct intel_plane *intel_plane;

	intel_plane = to_intel_plane(plane);

	return DVSSURFLIVE(intel_plane->pipe);
}

static void
intel_plane_queue_unpin(struct intel_plane *plane,
			struct drm_i915_gem_object *obj)
{
	/*
	 * If the surface is currently being scanned out, we need to
	 * wait until the next vblank event latches in the new base address
	 * before we unpin it, or we may end up displaying the wrong data.
	 * However, if the old object isn't currently 'live', we can just
	 * unpin right away.
	 */
	if (plane->current_surface)
		if (plane->current_surface(&plane->base) !=
					i915_gem_obj_ggtt_offset(obj)) {
			intel_unpin_fb_obj(obj);
			return;
		}

	intel_crtc_queue_unpin(to_intel_crtc(plane->base.crtc), obj);
}

static int
intel_update_plane(struct drm_plane *plane, struct drm_crtc *crtc,
		   struct drm_framebuffer *fb, int crtc_x, int crtc_y,
		   unsigned int crtc_w, unsigned int crtc_h,
		   uint32_t src_x, uint32_t src_y,
		   uint32_t src_w, uint32_t src_h,
		   struct drm_pending_vblank_event *event)
{
	struct drm_device *dev = plane->dev;
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	struct intel_plane *intel_plane = to_intel_plane(plane);
	struct intel_framebuffer *intel_fb = to_intel_framebuffer(fb);
	struct drm_i915_gem_object *obj = intel_fb->obj;
	struct drm_i915_gem_object *old_obj = intel_plane->old_obj;
	int ret;
	bool primary_enabled = false;
	unsigned long flags;
	bool visible;
	int hscale, vscale;
	int max_scale, min_scale;
	int pixel_size = drm_format_plane_cpp(fb->pixel_format, 0);
	struct drm_rect src = {
		/* sample coordinates in 16.16 fixed point */
		.x1 = src_x,
		.x2 = src_x + src_w,
		.y1 = src_y,
		.y2 = src_y + src_h,
	};
	struct drm_rect dst = {
		/* integer pixels */
		.x1 = crtc_x,
		.x2 = crtc_x + crtc_w,
		.y1 = crtc_y,
		.y2 = crtc_y + crtc_h,
	};
	struct drm_rect clip = {
		.x2 = intel_crtc->active ? intel_crtc->config.pipe_src_w : 0,
		.y2 = intel_crtc->active ? intel_crtc->config.pipe_src_h : 0,
	};
	const struct {
		int crtc_x, crtc_y;
		unsigned int crtc_w, crtc_h;
		uint32_t src_x, src_y, src_w, src_h;
	} orig = {
		.crtc_x = crtc_x,
		.crtc_y = crtc_y,
		.crtc_w = crtc_w,
		.crtc_h = crtc_h,
		.src_x = src_x,
		.src_y = src_y,
		.src_w = src_w,
		.src_h = src_h,
	};
	struct intel_unpin_work *work = NULL;

	/* Don't modify another pipe's plane */
	if (intel_plane->pipe != intel_crtc->pipe) {
		DRM_DEBUG_KMS("Wrong plane <-> crtc mapping\n");
		return -EINVAL;
	}

	/* FIXME check all gen limits */
	if (fb->width < 3 || fb->height < 3 || fb->pitches[0] > 16384) {
		/*
		 * User layer can send width/height < 3 in few instances
		 * Relaxing these limits for all platforms are being
		 * considered. But for now, do it only for VLV
		 * based devices.
		 */
		if (IS_VALLEYVIEW(dev) && fb->pitches[0] <= 16384)
			DRM_DEBUG_KMS("Allow lesser fb width/height\n");
		else {
			DRM_DEBUG_KMS("Unsuitable framebuffer for plane\n");
			return -EINVAL;
		}
	}

	/* Sprite planes can be linear or x-tiled surfaces */
	switch (obj->tiling_mode) {
		case I915_TILING_NONE:
		case I915_TILING_X:
			break;
		default:
			DRM_DEBUG_KMS("Unsupported tiling mode\n");
			return -EINVAL;
	}

	/*
	 * FIXME the following code does a bunch of fuzzy adjustments to the
	 * coordinates and sizes. We probably need some way to decide whether
	 * more strict checking should be done instead.
	 */
	max_scale = intel_plane->max_downscale << 16;
	min_scale = intel_plane->can_scale ? 1 : (1 << 16);

	if (IS_VALLEYVIEW(dev) && intel_crtc->scaling_src_size &&
		(intel_crtc->pfit_control & PFIT_ENABLE)) {
		clip.x2 = ((intel_crtc->scaling_src_size >>
				SCALING_SRCSIZE_SHIFT) &
				SCALING_SRCSIZE_MASK) + 1;
		clip.y2 = (intel_crtc->scaling_src_size &
				SCALING_SRCSIZE_MASK) + 1;
	}

	hscale = drm_rect_calc_hscale_relaxed(&src, &dst, min_scale, max_scale);
	BUG_ON(hscale < 0);

	vscale = drm_rect_calc_vscale_relaxed(&src, &dst, min_scale, max_scale);
	BUG_ON(vscale < 0);

	visible = drm_rect_clip_scaled(&src, &dst, &clip, hscale, vscale);

	crtc_x = dst.x1;
	crtc_y = dst.y1;
	crtc_w = drm_rect_width(&dst);
	crtc_h = drm_rect_height(&dst);

	if (visible) {
		/* check again in case clipping clamped the results */
		hscale = drm_rect_calc_hscale(&src, &dst, min_scale, max_scale);
		if (hscale < 0) {
			DRM_DEBUG_KMS("Horizontal scaling factor out of limits\n");
			drm_rect_debug_print(&src, true);
			drm_rect_debug_print(&dst, false);

			return hscale;
		}

		vscale = drm_rect_calc_vscale(&src, &dst, min_scale, max_scale);
		if (vscale < 0) {
			DRM_DEBUG_KMS("Vertical scaling factor out of limits\n");
			drm_rect_debug_print(&src, true);
			drm_rect_debug_print(&dst, false);

			return vscale;
		}

		/* Make the source viewport size an exact multiple of the scaling factors. */
		drm_rect_adjust_size(&src,
				     drm_rect_width(&dst) * hscale - drm_rect_width(&src),
				     drm_rect_height(&dst) * vscale - drm_rect_height(&src));

		/* sanity check to make sure the src viewport wasn't enlarged */
		WARN_ON(src.x1 < (int) src_x ||
			src.y1 < (int) src_y ||
			src.x2 > (int) (src_x + src_w) ||
			src.y2 > (int) (src_y + src_h));

		/*
		 * Hardware doesn't handle subpixel coordinates.
		 * Adjust to (macro)pixel boundary, but be careful not to
		 * increase the source viewport size, because that could
		 * push the downscaling factor out of bounds.
		 */
		src_x = src.x1 >> 16;
		src_w = drm_rect_width(&src) >> 16;
		src_y = src.y1 >> 16;
		src_h = drm_rect_height(&src) >> 16;

		if (format_is_yuv(fb->pixel_format)) {
			src_x &= ~1;
			src_w &= ~1;

			/*
			 * Must keep src and dst the
			 * same if we can't scale.
			 */
			if (!intel_plane->can_scale)
				crtc_w &= ~1;

			if (crtc_w == 0)
				visible = false;
		}
	}

	/* Check size restrictions when scaling */
	if (visible && (src_w != crtc_w || src_h != crtc_h)) {
		unsigned int width_bytes;

		WARN_ON(!intel_plane->can_scale);

		/* FIXME interlacing min height is 6 */

		if (crtc_w < 3 || crtc_h < 3)
			visible = false;

		if (src_w < 3 || src_h < 3)
			visible = false;

		width_bytes = ((src_x * pixel_size) & 63) + src_w * pixel_size;

		if (src_w > 2048 || src_h > 2048 ||
		    width_bytes > 4096 || fb->pitches[0] > 4096) {
			DRM_DEBUG_KMS("Source dimensions exceed hardware limits\n");
			return -EINVAL;
		}
	}

	dst.x1 = crtc_x;
	dst.x2 = crtc_x + crtc_w;
	dst.y1 = crtc_y;
	dst.y2 = crtc_y + crtc_h;

	/*
	 * If the sprite is completely covering the primary plane,
	 * we can disable the primary and save power.
	 */
	if (!IS_VALLEYVIEW(dev)) {
		primary_enabled = !drm_rect_equals(&dst, &clip) ||
			colorkey_enabled(intel_plane);
		WARN_ON(!primary_enabled && !visible && intel_crtc->active);
	}

	/*
	 * Ideally when one unpin work is in progress another request will not
	 * come from the user layer. But if in worst case faulty situations
	 * we get then the system will enter into an unrecoverable state, which
	 * needs hard shutdown. So as a precaution if the sprite_unpin_work is
	 * not null, wait for the pending flip to be completed and then proceed.
	 */
	if (intel_crtc->sprite_unpin_work)
		intel_crtc_wait_for_pending_flips(crtc);

	if (event) {
		work = kzalloc(sizeof(*work), GFP_KERNEL);
		if (work == NULL)
			return -ENOMEM;
		work->event = event;
		work->crtc = crtc;
		work->old_fb_obj = old_obj;
		INIT_WORK(&work->work, intel_unpin_sprite_work_fn);

		ret = drm_vblank_get(dev, intel_crtc->pipe);
		if (ret)
			goto free_work;

		/* We borrow the event spin lock for protecting unpin_work */
		spin_lock_irqsave(&dev->event_lock, flags);
		if (intel_crtc->sprite_unpin_work) {
			spin_unlock_irqrestore(&dev->event_lock, flags);
			kfree(work);
			drm_vblank_put(dev, intel_crtc->pipe);
			DRM_ERROR("flip queue: crtc already busy\n");
			return -EBUSY;
		}

		intel_crtc->sprite_unpin_work = work;
		spin_unlock_irqrestore(&dev->event_lock, flags);

		ret = i915_mutex_lock_interruptible(dev);
		if (ret)
			goto cleanup;

		if (IS_VALLEYVIEW(dev))
			intel_vlv_edp_psr_disable(dev);

		work->pending_flip_obj = obj;
		/* Block clients from rendering to the new back buffer until
		* the flip occurs and the object is no longer visible.
		*/
		if (work->old_fb_obj != NULL)
			atomic_add(1 << intel_crtc->plane,
				&work->old_fb_obj->pending_flip);
	} else
		mutex_lock(&dev->struct_mutex);

	/* Disable PSR */
	if (IS_VALLEYVIEW(dev))
		intel_vlv_edp_psr_disable(dev);

	/* Note that this will apply the VT-d workaround for scanouts,
	 * which is more restrictive than required for sprites. (The
	 * primary plane requires 256KiB alignment with 64 PTE padding,
	 * the sprite planes only require 128KiB alignment and 32 PTE padding.
	 */
	ret = intel_pin_and_fence_fb_obj(dev, obj, NULL);
	mutex_unlock(&dev->struct_mutex);
	if (ret) {
		DRM_ERROR("pin and fence of fb failed with %d\n", ret);
		spin_lock_irqsave(&dev->event_lock, flags);
		intel_crtc->sprite_unpin_work = NULL;
		spin_unlock_irqrestore(&dev->event_lock, flags);
		if (event)
			drm_vblank_put(dev, intel_crtc->pipe);
		goto out_unlock;
	}

	intel_plane->crtc_x = orig.crtc_x;
	intel_plane->crtc_y = orig.crtc_y;
	intel_plane->crtc_w = orig.crtc_w;
	intel_plane->crtc_h = orig.crtc_h;
	intel_plane->src_x = orig.src_x;
	intel_plane->src_y = orig.src_y;
	intel_plane->src_w = orig.src_w;
	intel_plane->src_h = orig.src_h;
	intel_plane->old_obj = intel_plane->obj;
	intel_plane->obj = obj;

	if (intel_crtc->active) {
		bool primary_was_enabled = intel_crtc->primary_enabled;

		intel_crtc->primary_enabled = primary_enabled;

		if (!IS_VALLEYVIEW(dev)) {
			if (primary_was_enabled != primary_enabled)
				intel_crtc_wait_for_pending_flips(crtc);
		}

		if (!IS_VALLEYVIEW(dev)) {
			if (primary_was_enabled && !primary_enabled)
				intel_pre_disable_primary(crtc);
		}

		if (event == NULL) {

			/* Enable for non-VLV if required */
			if (IS_VALLEYVIEW(dev)) {
				intel_crtc->primary_enabled = true;
				if (intel_crtc->atomic_update)
					intel_update_primary_plane(plane,
						intel_crtc);
				intel_post_enable_primary(crtc);
			}
		}

		if (visible)
			intel_plane->update_plane(plane, crtc, fb, obj,
						  crtc_x, crtc_y, crtc_w, crtc_h,
				src_x, src_y, src_w, src_h, event);
		else
			intel_plane->disable_plane(plane, crtc);

		if (!IS_VALLEYVIEW(dev)) {
			if (!primary_was_enabled && primary_enabled)
				intel_post_enable_primary(crtc);
		}

		if (event != NULL) {
			/* Enable for non-VLV if required */
			if (IS_VALLEYVIEW(dev)) {
				intel_crtc->primary_enabled = false;
				intel_pre_disable_primary(crtc);
				if (intel_crtc->atomic_update)
					intel_update_primary_plane(plane,
						intel_crtc);
			}
		}
	}

	/* Unpin old obj after new one is active to avoid ugliness */
	if (old_obj && (event == NULL)) {
		mutex_lock(&dev->struct_mutex);
		if (IS_VALLEYVIEW(dev))
			intel_unpin_fb_obj(old_obj);
		else
			intel_plane_queue_unpin(intel_plane, old_obj);
		mutex_unlock(&dev->struct_mutex);
	}

out_unlock:
	if (event)
		trace_i915_flip_request(intel_crtc->plane, obj);
	return ret;
cleanup:
	spin_lock_irqsave(&dev->event_lock, flags);
	intel_crtc->sprite_unpin_work = NULL;
	spin_unlock_irqrestore(&dev->event_lock, flags);
	drm_vblank_put(dev, intel_crtc->pipe);
free_work:
	kfree(work);

	return ret;

}

static void intel_disable_plane_unpin_work_fn(struct work_struct *__work)
{
	struct intel_plane *intel_plane =
			container_of(__work, struct intel_plane, work);
	struct drm_device *dev = intel_plane->base.dev;

	intel_wait_for_vblank(dev, intel_plane->pipe);
	if (intel_plane->obj || intel_plane->old_obj) {
		mutex_lock(&dev->struct_mutex);

		if (intel_plane->obj)
			intel_unpin_fb_obj(intel_plane->obj);

		if (intel_plane->old_obj)
			intel_unpin_fb_obj(intel_plane->old_obj);

		mutex_unlock(&dev->struct_mutex);
	}

	kfree(intel_plane);
}

static int
intel_disable_plane(struct drm_plane *plane)
{
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane = to_intel_plane(plane);
	struct intel_crtc *intel_crtc;
	struct intel_plane *intel_plane_wq;

	if (!plane->fb)
		return 0;

	if (WARN_ON(!plane->crtc))
		return -EINVAL;

	intel_crtc = to_intel_crtc(plane->crtc);
	intel_plane_wq = kzalloc(sizeof(*intel_plane_wq), GFP_KERNEL);
	if (!intel_plane_wq)
		return -ENOMEM;

	/* To support deffered plane disable */
	INIT_WORK(&intel_plane_wq->work, intel_disable_plane_unpin_work_fn);

	if (dev_priv->maxfifo_enabled) {
		intel_update_maxfifo(dev_priv, plane->crtc, false);
	}

	if (intel_crtc->active) {
		bool primary_was_enabled = intel_crtc->primary_enabled;
		intel_crtc->primary_enabled = true;
		intel_plane->disable_plane(plane, plane->crtc);
		if (!primary_was_enabled && intel_crtc->primary_enabled) {
			if (intel_crtc->atomic_update)
				intel_update_primary_plane(plane, intel_crtc);
			intel_post_enable_primary(plane->crtc);
		}
	}

	mutex_lock(&dev->struct_mutex);

	intel_plane_wq->base.dev = plane->dev;
	intel_plane_wq->old_obj = intel_plane->old_obj;
	intel_plane_wq->obj = intel_plane->obj;
	intel_plane_wq->pipe = intel_plane->pipe;

	intel_plane->obj = NULL;
	intel_plane->old_obj = NULL;

	schedule_work(&intel_plane_wq->work);
	mutex_unlock(&dev->struct_mutex);

	return 0;
}

static void intel_destroy_plane(struct drm_plane *plane)
{
	struct intel_plane *intel_plane = to_intel_plane(plane);
	intel_disable_plane(plane);

	if (intel_plane->csc_profile_property) {
		drm_property_destroy(plane->dev,
				intel_plane->csc_profile_property);
		intel_plane->csc_profile_property = NULL;
	}

	drm_plane_cleanup(plane);
	kfree(intel_plane);
}

int intel_sprite_set_colorkey(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	struct drm_intel_sprite_colorkey *set = data;
	struct drm_mode_object *obj;
	struct drm_plane *plane;
	struct intel_plane *intel_plane;
	int ret = 0;

	if (!drm_core_check_feature(dev, DRIVER_MODESET))
		return -ENODEV;

	/* Make sure we don't try to enable both src & dest simultaneously */
	if ((set->flags & (I915_SET_COLORKEY_DESTINATION | I915_SET_COLORKEY_SOURCE)) == (I915_SET_COLORKEY_DESTINATION | I915_SET_COLORKEY_SOURCE))
		return -EINVAL;

	drm_modeset_lock_all(dev);

	obj = drm_mode_object_find(dev, set->plane_id, DRM_MODE_OBJECT_PLANE);
	if (!obj) {
		ret = -ENOENT;
		goto out_unlock;
	}

	plane = obj_to_plane(obj);
	intel_plane = to_intel_plane(plane);
	ret = intel_plane->update_colorkey(plane, set);

out_unlock:
	drm_modeset_unlock_all(dev);
	return ret;
}

int intel_sprite_get_colorkey(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	struct drm_intel_sprite_colorkey *get = data;
	struct drm_mode_object *obj;
	struct drm_plane *plane;
	struct intel_plane *intel_plane;
	int ret = 0;

	if (!drm_core_check_feature(dev, DRIVER_MODESET))
		return -ENODEV;

	drm_modeset_lock_all(dev);

	obj = drm_mode_object_find(dev, get->plane_id, DRM_MODE_OBJECT_PLANE);
	if (!obj) {
		ret = -ENOENT;
		goto out_unlock;
	}

	plane = obj_to_plane(obj);
	intel_plane = to_intel_plane(plane);
	intel_plane->get_colorkey(plane, get);

out_unlock:
	drm_modeset_unlock_all(dev);
	return ret;
}

void intel_plane_restore(struct drm_plane *plane)
{
	struct intel_plane *intel_plane = to_intel_plane(plane);

	if (!plane->crtc || !plane->fb)
		return;

	intel_update_plane(plane, plane->crtc, plane->fb,
			   intel_plane->crtc_x, intel_plane->crtc_y,
			   intel_plane->crtc_w, intel_plane->crtc_h,
			   intel_plane->src_x, intel_plane->src_y,
			   intel_plane->src_w, intel_plane->src_h, NULL);
}

void intel_plane_disable(struct drm_plane *plane)
{
	if (!plane->crtc || !plane->fb)
		return;

	intel_disable_plane(plane);
}

static int intel_plane_set_property(struct drm_plane *plane,
	struct drm_property *property, uint64_t val)
{
	struct intel_plane *intel_plane = to_intel_plane(plane);
	struct drm_device *dev = plane->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;

	if (IS_CHERRYVIEW(dev) && STEP_FROM(STEP_B0) &&
		intel_plane->pipe == PIPE_B) {
		if (property == intel_plane->csc_profile_property)
			intel_plane->csc_profile = (uint32_t) val;
		return 0;
	}
	return -EINVAL;
}

static const struct drm_plane_funcs intel_plane_funcs = {
	.update_plane = intel_update_plane,
	.disable_plane = intel_disable_plane,
	.destroy = intel_destroy_plane,
	.set_property = intel_plane_set_property,
};

static uint32_t ilk_plane_formats[] = {
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_YUYV,
	DRM_FORMAT_YVYU,
	DRM_FORMAT_UYVY,
	DRM_FORMAT_VYUY,
};

static uint32_t snb_plane_formats[] = {
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_YUYV,
	DRM_FORMAT_YVYU,
	DRM_FORMAT_UYVY,
	DRM_FORMAT_VYUY,
};

static uint32_t vlv_plane_formats[] = {
	DRM_FORMAT_RGB565,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR2101010,
	DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_YUYV,
	DRM_FORMAT_YVYU,
	DRM_FORMAT_UYVY,
	DRM_FORMAT_VYUY,
};

int
intel_plane_init(struct drm_device *dev, enum pipe pipe, int plane)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_plane *intel_plane;
	unsigned long possible_crtcs;
	const uint32_t *plane_formats;
	int num_plane_formats;
	int ret;

	if (INTEL_INFO(dev)->gen < 5)
		return -ENODEV;

	intel_plane = kzalloc(sizeof(*intel_plane), GFP_KERNEL);
	if (!intel_plane)
		return -ENOMEM;

	switch (INTEL_INFO(dev)->gen) {
	case 5:
	case 6:
		intel_plane->can_scale = true;
		intel_plane->max_downscale = 16;
		intel_plane->update_plane = ilk_update_plane;
		intel_plane->disable_plane = ilk_disable_plane;
		intel_plane->update_colorkey = ilk_update_colorkey;
		intel_plane->get_colorkey = ilk_get_colorkey;
		intel_plane->current_surface = ilk_current_surface;

		if (IS_GEN6(dev)) {
			plane_formats = snb_plane_formats;
			num_plane_formats = ARRAY_SIZE(snb_plane_formats);
		} else {
			plane_formats = ilk_plane_formats;
			num_plane_formats = ARRAY_SIZE(ilk_plane_formats);
		}
		break;

	case 7:
	case 8:
		if (IS_IVYBRIDGE(dev)) {
			intel_plane->can_scale = true;
			intel_plane->max_downscale = 2;
		} else {
			intel_plane->can_scale = false;
			intel_plane->max_downscale = 1;
		}

		if (IS_VALLEYVIEW(dev)) {
			intel_plane->update_plane = vlv_update_plane;
			intel_plane->disable_plane = vlv_disable_plane;
			intel_plane->update_colorkey = vlv_update_colorkey;
			intel_plane->get_colorkey = vlv_get_colorkey;

			plane_formats = vlv_plane_formats;
			num_plane_formats = ARRAY_SIZE(vlv_plane_formats);
		} else {
			intel_plane->update_plane = ivb_update_plane;
			intel_plane->disable_plane = ivb_disable_plane;
			intel_plane->update_colorkey = ivb_update_colorkey;
			intel_plane->get_colorkey = ivb_get_colorkey;
			intel_plane->current_surface = ivb_current_surface;

			plane_formats = snb_plane_formats;
			num_plane_formats = ARRAY_SIZE(snb_plane_formats);
		}
		break;

	default:
		kfree(intel_plane);
		return -ENODEV;
	}

	intel_plane->pipe = pipe;
	intel_plane->plane = plane;
	intel_plane->rotate180 = false;
	intel_plane->rrb2_enable = 0;
	intel_plane->last_plane_state = INTEL_PLANE_STATE_DISABLED;
	possible_crtcs = (1 << pipe);
	ret = drm_plane_init(dev, &intel_plane->base, possible_crtcs,
			     &intel_plane_funcs,
			     plane_formats, num_plane_formats,
			     false);

	if (ret) {
		kfree(intel_plane);
		DRM_DEBUG_KMS("Returning from plane init...\n");
		return ret;
	}

	if (IS_CHERRYVIEW(dev) && STEP_FROM(STEP_B0) && pipe == PIPE_B) {
		intel_plane->csc_profile = 4;
		intel_plane->csc_profile_property =
			drm_property_create_range(dev, 0, "csc profile", 1,
				chv_sprite_csc_num_entries);
		drm_object_attach_property(&intel_plane->base.base,
			intel_plane->csc_profile_property, 4);
	}

	return ret;
}
