/* i915_drv.c -- i830,i845,i855,i865,i915 driver -*- linux-c -*-
 */
/*
 *
 * Copyright 2003 Tungsten Graphics, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <linux/device.h>
#include <drm/drmP.h>
#include <drm/i915_drm.h>
#include "i915_drv.h"
#include "i915_trace.h"
#include "intel_drv.h"
#include "intel_lrc_tdr.h"
#include "i915_scheduler.h"

#include <linux/console.h>
#include <linux/module.h>
#include <linux/pm_runtime.h>
#include <drm/drm_crtc_helper.h>

#define CHV_PCI_MINOR_STEP_MASK		0x0C
#define CHV_PCI_MINOR_STEP_SHIFT	0x02
#define CHV_PCI_MAJOR_STEP_MASK		0x30
#define CHV_PCI_MAJOR_STEP_SHIFT	0x04
#define CHV_PCI_STEP_SEL_MASK		0x40
#define CHV_PCI_STEP_SEL_SHIFT		0x06
#define CHV_PCI_OVERFLOW_MASK		0x80
#define CHV_PCI_OVERFLOW_SHIFT		0x07

#define CHV_MAX_STEP_SEL	1
#define CHV_MAX_MAJ_STEP	1
#define CHV_MAX_MIN_STEP	3

static struct drm_driver driver;

#define GEN_DEFAULT_PIPEOFFSETS \
	.pipe_offsets = { PIPE_A_OFFSET, PIPE_B_OFFSET, \
			  PIPE_C_OFFSET, PIPE_EDP_OFFSET }, \
	.trans_offsets = { TRANSCODER_A_OFFSET, TRANSCODER_B_OFFSET, \
			   TRANSCODER_C_OFFSET, TRANSCODER_EDP_OFFSET }, \
	.palette_offsets = { PALETTE_A_OFFSET, PALETTE_B_OFFSET }

#define GEN_CHV_PIPEOFFSETS \
	.pipe_offsets = { PIPE_A_OFFSET, PIPE_B_OFFSET, \
			  CHV_PIPE_C_OFFSET }, \
	.trans_offsets = { TRANSCODER_A_OFFSET, TRANSCODER_B_OFFSET, \
			   CHV_TRANSCODER_C_OFFSET, }, \
	.palette_offsets = { PALETTE_A_OFFSET, PALETTE_B_OFFSET, \
			     CHV_PALETTE_C_OFFSET }

#define CURSOR_OFFSETS \
	.cursor_offsets = { CURSOR_A_OFFSET, CURSOR_B_OFFSET, CHV_CURSOR_C_OFFSET }

#define IVB_CURSOR_OFFSETS \
	.cursor_offsets = { CURSOR_A_OFFSET, IVB_CURSOR_B_OFFSET, IVB_CURSOR_C_OFFSET }

static const struct intel_device_info intel_i830_info = {
	.gen = 2, .is_mobile = 1, .cursor_needs_physical = 1, .num_pipes = 2,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_845g_info = {
	.gen = 2, .num_pipes = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_i85x_info = {
	.gen = 2, .is_i85x = 1, .is_mobile = 1, .num_pipes = 2,
	.cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.has_fbc = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_i865g_info = {
	.gen = 2, .num_pipes = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_i915g_info = {
	.gen = 3, .is_i915g = 1, .cursor_needs_physical = 1, .num_pipes = 2,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};
static const struct intel_device_info intel_i915gm_info = {
	.gen = 3, .is_mobile = 1, .num_pipes = 2,
	.cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.supports_tv = 1,
	.has_fbc = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};
static const struct intel_device_info intel_i945g_info = {
	.gen = 3, .has_hotplug = 1, .cursor_needs_physical = 1, .num_pipes = 2,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};
static const struct intel_device_info intel_i945gm_info = {
	.gen = 3, .is_i945gm = 1, .is_mobile = 1, .num_pipes = 2,
	.has_hotplug = 1, .cursor_needs_physical = 1,
	.has_overlay = 1, .overlay_needs_physical = 1,
	.supports_tv = 1,
	.has_fbc = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_i965g_info = {
	.gen = 4, .is_broadwater = 1, .num_pipes = 2,
	.has_hotplug = 1,
	.has_overlay = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_i965gm_info = {
	.gen = 4, .is_crestline = 1, .num_pipes = 2,
	.is_mobile = 1, .has_fbc = 1, .has_hotplug = 1,
	.has_overlay = 1,
	.supports_tv = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_g33_info = {
	.gen = 3, .is_g33 = 1, .num_pipes = 2,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_overlay = 1,
	.ring_mask = RENDER_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_g45_info = {
	.gen = 4, .is_g4x = 1, .need_gfx_hws = 1, .num_pipes = 2,
	.has_pipe_cxsr = 1, .has_hotplug = 1,
	.ring_mask = RENDER_RING | BSD_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_gm45_info = {
	.gen = 4, .is_g4x = 1, .num_pipes = 2,
	.is_mobile = 1, .need_gfx_hws = 1, .has_fbc = 1,
	.has_pipe_cxsr = 1, .has_hotplug = 1,
	.supports_tv = 1,
	.ring_mask = RENDER_RING | BSD_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_pineview_info = {
	.gen = 3, .is_g33 = 1, .is_pineview = 1, .is_mobile = 1, .num_pipes = 2,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_overlay = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_ironlake_d_info = {
	.gen = 5, .num_pipes = 2,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.ring_mask = RENDER_RING | BSD_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_ironlake_m_info = {
	.gen = 5, .is_mobile = 1, .num_pipes = 2,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 1,
	.ring_mask = RENDER_RING | BSD_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_sandybridge_d_info = {
	.gen = 6, .num_pipes = 2,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING,
	.has_llc = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_sandybridge_m_info = {
	.gen = 6, .is_mobile = 1, .num_pipes = 2,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.has_fbc = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING,
	.has_llc = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

#define GEN7_FEATURES  \
	.gen = 7, .num_pipes = 3, \
	.need_gfx_hws = 1, .has_hotplug = 1, \
	.has_fbc = 1, \
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING, \
	.has_llc = 1

static const struct intel_device_info intel_ivybridge_d_info = {
	GEN7_FEATURES,
	.is_ivybridge = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_ivybridge_m_info = {
	GEN7_FEATURES,
	.is_ivybridge = 1,
	.is_mobile = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_ivybridge_q_info = {
	GEN7_FEATURES,
	.is_ivybridge = 1,
	.num_pipes = 0, /* legal, last one wins */
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_valleyview_m_info = {
	GEN7_FEATURES,
	.is_mobile = 1,
	.num_pipes = 2,
	.is_valleyview = 1,
	.display_mmio_offset = VLV_DISPLAY_BASE,
	.has_fbc = 0, /* legal, last one wins */
	.has_llc = 0, /* legal, last one wins */
	.has_dpst = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_valleyview_d_info = {
	GEN7_FEATURES,
	.num_pipes = 2,
	.is_valleyview = 1,
	.display_mmio_offset = VLV_DISPLAY_BASE,
	.has_fbc = 0, /* legal, last one wins */
	.has_llc = 0, /* legal, last one wins */
	GEN_DEFAULT_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

static const struct intel_device_info intel_haswell_d_info = {
	GEN7_FEATURES,
	.is_haswell = 1,
	.has_ddi = 1,
	.has_fpga_dbg = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING | VEBOX_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_haswell_m_info = {
	GEN7_FEATURES,
	.is_haswell = 1,
	.is_mobile = 1,
	.has_ddi = 1,
	.has_fpga_dbg = 1,
	.has_dpst = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING | VEBOX_RING,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_broadwell_d_info = {
	.gen = 8, .num_pipes = 3,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING | VEBOX_RING,
	.has_llc = 1,
	.has_ddi = 1,
	.has_fbc = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_broadwell_m_info = {
	.gen = 8, .is_mobile = 1, .num_pipes = 3,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING | VEBOX_RING,
	.has_llc = 1,
	.has_ddi = 1,
	.has_fbc = 1,
	.has_dpst = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_broadwell_gt3d_info = {
	.gen = 8, .num_pipes = 3,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING | VEBOX_RING | BSD2_RING,
	.has_llc = 1,
	.has_ddi = 1,
	.has_fbc = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_broadwell_gt3m_info = {
	.gen = 8, .is_mobile = 1, .num_pipes = 3,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING | VEBOX_RING | BSD2_RING,
	.has_llc = 1,
	.has_ddi = 1,
	.has_fbc = 1,
	GEN_DEFAULT_PIPEOFFSETS,
	IVB_CURSOR_OFFSETS,
};

static const struct intel_device_info intel_cherryview_info = {
	.is_preliminary = 1,
	.gen = 8, .num_pipes = 3,
	.need_gfx_hws = 1, .has_hotplug = 1,
	.ring_mask = RENDER_RING | BSD_RING | BLT_RING | VEBOX_RING,
	.is_valleyview = 1,
	.has_dpst = 1,
	.display_mmio_offset = VLV_DISPLAY_BASE,
	GEN_CHV_PIPEOFFSETS,
	CURSOR_OFFSETS,
};

/*
 * Make sure any device matches here are from most specific to most
 * general.  For example, since the Quanta match is based on the subsystem
 * and subvendor IDs, we need it to come before the more general IVB
 * PCI ID matches, otherwise we'll use the wrong info struct above.
 */
#define INTEL_PCI_IDS \
	INTEL_I830_IDS(&intel_i830_info),	\
	INTEL_I845G_IDS(&intel_845g_info),	\
	INTEL_I85X_IDS(&intel_i85x_info),	\
	INTEL_I865G_IDS(&intel_i865g_info),	\
	INTEL_I915G_IDS(&intel_i915g_info),	\
	INTEL_I915GM_IDS(&intel_i915gm_info),	\
	INTEL_I945G_IDS(&intel_i945g_info),	\
	INTEL_I945GM_IDS(&intel_i945gm_info),	\
	INTEL_I965G_IDS(&intel_i965g_info),	\
	INTEL_G33_IDS(&intel_g33_info),		\
	INTEL_I965GM_IDS(&intel_i965gm_info),	\
	INTEL_GM45_IDS(&intel_gm45_info), 	\
	INTEL_G45_IDS(&intel_g45_info), 	\
	INTEL_PINEVIEW_IDS(&intel_pineview_info),	\
	INTEL_IRONLAKE_D_IDS(&intel_ironlake_d_info),	\
	INTEL_IRONLAKE_M_IDS(&intel_ironlake_m_info),	\
	INTEL_SNB_D_IDS(&intel_sandybridge_d_info),	\
	INTEL_SNB_M_IDS(&intel_sandybridge_m_info),	\
	INTEL_IVB_Q_IDS(&intel_ivybridge_q_info), /* must be first IVB */ \
	INTEL_IVB_M_IDS(&intel_ivybridge_m_info),	\
	INTEL_IVB_D_IDS(&intel_ivybridge_d_info),	\
	INTEL_HSW_D_IDS(&intel_haswell_d_info), \
	INTEL_HSW_M_IDS(&intel_haswell_m_info), \
	INTEL_VLV_M_IDS(&intel_valleyview_m_info),	\
	INTEL_VLV_D_IDS(&intel_valleyview_d_info),	\
	INTEL_BDW_GT12M_IDS(&intel_broadwell_m_info),	\
	INTEL_BDW_GT12D_IDS(&intel_broadwell_d_info),	\
	INTEL_BDW_GT3M_IDS(&intel_broadwell_gt3m_info),	\
	INTEL_BDW_GT3D_IDS(&intel_broadwell_gt3d_info), \
	INTEL_CHV_IDS(&intel_cherryview_info)

static const struct pci_device_id pciidlist[] = {		/* aka */
	INTEL_PCI_IDS,
	{0, 0, 0}
};

#if defined(CONFIG_DRM_I915_KMS)
MODULE_DEVICE_TABLE(pci, pciidlist);
#endif

void intel_detect_pch(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct pci_dev *pch = NULL;

	/* In all current cases, num_pipes is equivalent to the PCH_NOP setting
	 * (which really amounts to a PCH but no South Display).
	 */
	if (INTEL_INFO(dev)->num_pipes == 0) {
		dev_priv->pch_type = PCH_NOP;
		return;
	}

	/*
	 * The reason to probe ISA bridge instead of Dev31:Fun0 is to
	 * make graphics device passthrough work easy for VMM, that only
	 * need to expose ISA bridge to let driver know the real hardware
	 * underneath. This is a requirement from virtualization team.
	 *
	 * In some virtualized environments (e.g. XEN), there is irrelevant
	 * ISA bridge in the system. To work reliably, we should scan trhough
	 * all the ISA bridge devices and check for the first match, instead
	 * of only checking the first one.
	 */
	while ((pch = pci_get_class(PCI_CLASS_BRIDGE_ISA << 8, pch))) {
		if (pch->vendor == PCI_VENDOR_ID_INTEL) {
			unsigned short id = pch->device & INTEL_PCH_DEVICE_ID_MASK;
			dev_priv->pch_id = id;

			if (id == INTEL_PCH_IBX_DEVICE_ID_TYPE) {
				dev_priv->pch_type = PCH_IBX;
				DRM_DEBUG_KMS("Found Ibex Peak PCH\n");
				WARN_ON(!IS_GEN5(dev));
			} else if (id == INTEL_PCH_CPT_DEVICE_ID_TYPE) {
				dev_priv->pch_type = PCH_CPT;
				DRM_DEBUG_KMS("Found CougarPoint PCH\n");
				WARN_ON(!(IS_GEN6(dev) || IS_IVYBRIDGE(dev)));
			} else if (id == INTEL_PCH_PPT_DEVICE_ID_TYPE) {
				/* PantherPoint is CPT compatible */
				dev_priv->pch_type = PCH_CPT;
				DRM_DEBUG_KMS("Found PantherPoint PCH\n");
				WARN_ON(!(IS_GEN6(dev) || IS_IVYBRIDGE(dev)));
			} else if (id == INTEL_PCH_LPT_DEVICE_ID_TYPE) {
				dev_priv->pch_type = PCH_LPT;
				DRM_DEBUG_KMS("Found LynxPoint PCH\n");
				WARN_ON(!IS_HASWELL(dev));
				WARN_ON(IS_ULT(dev));
			} else if (IS_BROADWELL(dev)) {
				dev_priv->pch_type = PCH_LPT;
				dev_priv->pch_id =
					INTEL_PCH_LPT_LP_DEVICE_ID_TYPE;
				DRM_DEBUG_KMS("This is Broadwell, assuming "
					      "LynxPoint LP PCH\n");
			} else if (id == INTEL_PCH_LPT_LP_DEVICE_ID_TYPE) {
				dev_priv->pch_type = PCH_LPT;
				DRM_DEBUG_KMS("Found LynxPoint LP PCH\n");
				WARN_ON(!IS_HASWELL(dev));
				WARN_ON(!IS_ULT(dev));
			} else
				continue;

			break;
		}
	}
	if (!pch)
		DRM_DEBUG_KMS("No PCH found.\n");

	pci_dev_put(pch);
}

void intel_detect_stepping(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	u16 stepping_id;
	u16 rev_id;

	pci_read_config_word(dev->pdev, PCI_REVISION_ID, &rev_id);

	if (IS_CHERRYVIEW(dev)) {
		stepping_id = ((rev_id & CHV_PCI_MINOR_STEP_MASK)
				>> CHV_PCI_MINOR_STEP_SHIFT) + ASCII_0;

		if ((rev_id & CHV_PCI_STEP_SEL_MASK) >> CHV_PCI_STEP_SEL_SHIFT)
			stepping_id = stepping_id + (ASCII_K << 8);
		else
			stepping_id = stepping_id + (ASCII_A << 8);

		stepping_id = stepping_id + (((rev_id & CHV_PCI_MAJOR_STEP_MASK)
				>> CHV_PCI_MAJOR_STEP_SHIFT) << 8);

		dev_priv->stepping_id = stepping_id;

		DRM_DEBUG_KMS("stepping id = 0x%x\n", dev_priv->stepping_id);
	}
}

bool i915_semaphore_is_enabled(struct drm_device *dev)
{
	/* Hardware semaphores are not compatible with the scheduler due to the
	 * seqno values being potentially out of order. However, semaphores are
	 * also not required as the scheduler will handle interring dependencies
	 * and try do so in a way that does not cause dead time on the hardware.
	 */
	if (i915_scheduler_is_enabled(dev))
		return 0;

	if (INTEL_INFO(dev)->gen < 6)
		return false;

	if (i915.semaphores >= 0)
		return i915.semaphores;

	/* TODO: make semaphores and Execlists play nicely together */
	if (i915.enable_execlists)
		return false;

	/* Until we get further testing... */
	if (IS_GEN8(dev))
		return false;

#ifdef CONFIG_INTEL_IOMMU
	/* Enable semaphores on SNB when IO remapping is off */
	if (INTEL_INFO(dev)->gen == 6 && intel_iommu_gfx_mapped)
		return false;
#endif

	return true;
}


static int intel_suspend_complete(struct drm_i915_private *dev_priv);
static int intel_resume_prepare(struct drm_i915_private *dev_priv,
				bool rpm_resume);

static int i915_drm_freeze(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_crtc *crtc;
	int ret;
	u32 i;

	/* ignore lid events during suspend */
	mutex_lock(&dev_priv->modeset_restore_lock);
	dev_priv->modeset_restore = MODESET_SUSPENDED;
	mutex_unlock(&dev_priv->modeset_restore_lock);

	/* We do a lot of poking in a lot of registers, make sure they work
	 * properly. */
	if (IS_VALLEYVIEW(dev))
		WARN_ON(!dev_priv->power_domains.init_power_on);
	else {
		/* We do a lot of poking in a lot of registers, make sure they
		 * work properly. */
		intel_display_set_init_power(dev_priv, true);
	}

	drm_kms_helper_poll_disable(dev);

	pci_save_state(dev->pdev);

	/* If KMS is active, we do the leavevt stuff here */
	if (drm_core_check_feature(dev, DRIVER_MODESET)) {
		int error;

		error = i915_gem_suspend(dev);
		if (error) {
			dev_err(&dev->pdev->dev,
				"GEM idle failed, resume might fail\n");
			return error;
		}

		if (IS_VALLEYVIEW(dev)) {
			spin_lock_irq(&dev_priv->irq_lock);
			valleyview_disable_display_irqs(dev_priv);
			spin_unlock_irq(&dev_priv->irq_lock);
		}

		/* Clear any pending reset requests. They should be picked up
		* after resume when new work is submitted */
		for (i = 0; i < I915_NUM_RINGS; i++)
			atomic_set(&dev_priv->ring[i].hangcheck.flags, 0);

		atomic_clear_mask(I915_RESET_IN_PROGRESS_FLAG,
			&dev_priv->gpu_error.reset_counter);

		drm_irq_uninstall(dev);

		intel_suspend_gt_powersave(dev);

		/*
		 * Disable CRTCs directly since we want to preserve sw state
		 * for _thaw. Also, power gate the CRTC power wells.
		 */
		drm_modeset_lock_all(dev);
		for_each_crtc(dev, crtc) {
			intel_crtc_control(crtc, false);
		}
		drm_modeset_unlock_all(dev);

		intel_modeset_suspend_hw(dev);
	}

	i915_gem_suspend_gtt_mappings(dev);

	i915_save_state(dev);

	intel_uncore_forcewake_reset(dev, false);
	intel_opregion_fini(dev);

	console_lock();
	intel_fbdev_set_suspend(dev, FBINFO_STATE_SUSPENDED);
	console_unlock();

	dev_priv->suspend_count++;

	ret = intel_suspend_complete(dev_priv);

	if (ret)
		WARN(1, "Suspend complete failed: %d\n", ret);

	if (!IS_VALLEYVIEW(dev))
		intel_display_set_init_power(dev_priv, false);

	return 0;
}

int i915_suspend(struct drm_device *dev, pm_message_t state)
{
	int error;

	if (!dev || !dev->dev_private) {
		DRM_ERROR("dev: %p\n", dev);
		DRM_ERROR("DRM not initialized, aborting suspend.\n");
		return -ENODEV;
	}

	if (state.event == PM_EVENT_PRETHAW)
		return 0;


	if (dev->switch_power_state == DRM_SWITCH_POWER_OFF)
		return 0;

	error = i915_drm_freeze(dev);
	if (error)
		return error;

	if (state.event == PM_EVENT_SUSPEND) {
		/* Shut down the device */
		pci_disable_device(dev->pdev);
		pci_set_power_state(dev->pdev, PCI_D3hot);
	}

	return 0;
}

void intel_console_resume(struct work_struct *work)
{
	struct drm_i915_private *dev_priv =
		container_of(work, struct drm_i915_private,
			     console_resume_work);
	struct drm_device *dev = dev_priv->dev;

	console_lock();
	intel_fbdev_set_suspend(dev, FBINFO_STATE_RUNNING);
	console_unlock();
}

static bool display_is_on(struct drm_device *dev)
{
	struct drm_connector *connector;
	bool display_is_on = false;

	drm_modeset_lock_all(dev);
	list_for_each_entry(connector, &dev->mode_config.connector_list, head) {
		if (!connector->encoder || !connector->encoder->crtc)
			continue;
		/*
		 * If Display wasn't turned off, before going to suspend then
		 * it should be re-enabled now, as we don't expect the DPMS on
		 * call to come in that cases
		 */
		if (connector->dpms != DRM_MODE_DPMS_OFF) {
			DRM_DEBUG_KMS("Display was on before suspend\n");
			display_is_on = true;
			break;
		}
	}
	drm_modeset_unlock_all(dev);

	return display_is_on;
}

static int i915_drm_thaw_early(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	if (!IS_VALLEYVIEW(dev) || IS_CHERRYVIEW(dev)) {
		intel_uncore_early_sanitize(dev);
		intel_uncore_sanitize(dev);
		intel_power_domains_init_hw(dev_priv);
	}

	dev_priv->thaw_early_done = true;

	return 0;
}

static int __i915_drm_thaw(struct drm_device *dev, bool restore_gtt_mappings)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	int ret;

	if (IS_VALLEYVIEW(dev) && !IS_CHERRYVIEW(dev)) {
		intel_uncore_early_sanitize(dev);
		intel_uncore_sanitize(dev);
		intel_power_domains_init_hw(dev_priv);
	}

	if (!dev_priv->thaw_early_done)
		i915_drm_thaw_early(dev);

	dev_priv->thaw_early_done = false;

	ret = intel_resume_prepare(dev_priv, false);
	if (ret)
		WARN(1, "Resume prepare failed: %d,Continuing resume\n", ret);

	if (drm_core_check_feature(dev, DRIVER_MODESET) &&
	    restore_gtt_mappings) {
		mutex_lock(&dev->struct_mutex);
		i915_gem_restore_gtt_mappings(dev);
		mutex_unlock(&dev->struct_mutex);
	}

	i915_restore_state(dev);
	intel_opregion_setup(dev);

	/* KMS EnterVT equivalent */
	if (drm_core_check_feature(dev, DRIVER_MODESET)) {
		intel_init_pch_refclk(dev);
		drm_mode_config_reset(dev);

		mutex_lock(&dev->struct_mutex);
		ret = i915_gem_init_hw(dev);
		if (ret) {
			DRM_ERROR("failed to re-initialize GPU, declaring wedged!\n");
			atomic_set_mask(I915_WEDGED, &dev_priv->gpu_error.reset_counter);
		}
		mutex_unlock(&dev->struct_mutex);

		/* We need working interrupts for modeset enabling ... */
		drm_irq_install(dev, dev->pdev->irq);

		intel_modeset_init_hw(dev);

		/* We need to load HuC after enabling irq */
		if (ret == 0)
			intel_chv_huc_load(dev);

		if (display_is_on(dev)) {
			drm_modeset_lock_all(dev);
			intel_modeset_setup_hw_state(dev, true);
			drm_modeset_unlock_all(dev);
		}

		/*
		 * ... but also need to make sure that hotplug processing
		 * doesn't cause havoc. Like in the driver load code we don't
		 * bother with the tiny race here where we might loose hotplug
		 * notifications.
		 * */
		intel_hpd_init(dev);
		/* Config may have changed between suspend and resume */
		drm_helper_hpd_irq_event(dev);
	}

	intel_opregion_init(dev);

	/*
	 * The console lock can be pretty contented on resume due
	 * to all the printk activity.  Try to keep it out of the hot
	 * path of resume if possible.
	 */
	if (console_trylock()) {
		intel_fbdev_set_suspend(dev, FBINFO_STATE_RUNNING);
		console_unlock();
	} else {
		schedule_work(&dev_priv->console_resume_work);
	}

	mutex_lock(&dev_priv->modeset_restore_lock);
	dev_priv->modeset_restore = MODESET_DONE;
	mutex_unlock(&dev_priv->modeset_restore_lock);

	/*
	 * VLV has a special case and we need to avoid the display going to D0
	 * until we get suspend.
	 * */
	if ((!IS_VALLEYVIEW(dev) || IS_CHERRYVIEW(dev))
			&& display_is_on(dev))
		intel_display_set_init_power(dev_priv, false);

	sysfs_notify(&dev->primary->kdev->kobj, NULL, "thaw");

	return 0;
}

static int i915_drm_thaw(struct drm_device *dev)
{
	if (drm_core_check_feature(dev, DRIVER_MODESET))
		i915_check_and_clear_faults(dev);

	return __i915_drm_thaw(dev, true);
}

static int i915_resume_early(struct drm_device *dev)
{
	if (dev->switch_power_state == DRM_SWITCH_POWER_OFF)
		return 0;

	/*
	 * We have a resume ordering issue with the snd-hda driver also
	 * requiring our device to be power up. Due to the lack of a
	 * parent/child relationship we currently solve this with an early
	 * resume hook.
	 *
	 * FIXME: This should be solved with a special hdmi sink device or
	 * similar so that power domains can be employed.
	 */
	if (pci_enable_device(dev->pdev))
		return -EIO;

	pci_set_master(dev->pdev);

	return i915_drm_thaw_early(dev);
}

int i915_resume(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	int ret;

	/*
	 * Platforms with opregion should have sane BIOS, older ones (gen3 and
	 * earlier) need to restore the GTT mappings since the BIOS might clear
	 * all our scratch PTEs.
	 */
	ret = __i915_drm_thaw(dev, !dev_priv->opregion.header);
	if (ret)
		return ret;

	drm_kms_helper_poll_enable(dev);
	return 0;
}

static int i915_resume_legacy(struct drm_device *dev)
{
	i915_resume_early(dev);
	i915_resume(dev);

	return 0;
}

/**
 * i915_handle_hung_ring - reset ring after a hang
 * @ringid: ring id to be reset
 *
 * TDR - Reset Individual ring: Useful if a hang is detected on a ring.
 * Returns zero on successful reset or otherwise an error code.
 *
 * Procedure is fairly simple:
 *   - ring is first disabled
 *   - ring specific registers are saved
 *   - reset the ring using chipset reg
 *   - restore saved ring specific registers
 *   - enable the ring after reset
 *
 * If page flips are submitted using rings and if the ring is hung
 * on a page flip it will be released after reset.
 *
 * WARNING: Hold dev->struct_mutex before entering this function
 */
int i915_handle_hung_ring(struct drm_device *dev, uint32_t ringid)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_engine_cs *ring = &dev_priv->ring[ringid];
	int ret = 0;
	int pipe = 0;
	uint32_t head;
	uint32_t acthd;
	uint32_t ring_flags = 0;
	uint32_t completed_seqno;
	struct drm_crtc *crtc;
	struct intel_crtc *intel_crtc;
	struct drm_i915_gem_request *request;
	struct intel_unpin_work *unpin_work;
	struct intel_context *current_context = NULL;
	uint32_t hw_context_id1 = ~0u;
	uint32_t hw_context_id2 = ~0u;

	acthd = intel_ring_get_active_head(ring);
	completed_seqno = ring->get_seqno(ring, false);

	WARN_ON(!mutex_is_locked(&dev->struct_mutex));

	/* Take wake lock to prevent power saving mode */
	gen6_gt_force_wake_get(dev_priv, FORCEWAKE_ALL);

	/* Search the request list to see which batch buffer caused
	* the hang. Only checks requests that haven't yet completed.*/
	list_for_each_entry(request, &ring->request_list, list) {
		if (request && (request->seqno > completed_seqno))
			i915_set_reset_status(dev_priv, request->ctx, false);
	}

	if (i915.enable_execlists) {
		enum context_submission_status status =
			i915_gem_context_get_current_context(ring,
					&current_context);

		/*
		 * If the hardware and driver states do not coincide
		 * or if there for some reason is no current context
		 * in the process of being submitted then bail out and
		 * try again. Do not proceed unless we have reliable
		 * current context state information.
		 */
		if (status != CONTEXT_SUBMISSION_STATUS_OK) {
			ret = -EAGAIN;
			goto handle_hung_ring_error;
		}
	}

	/*
	 * Check if the ring has hung on a MI_DISPLAY_FLIP command.
	 * The pipe value will be stored in the HWS page if it has.
	 * At the moment this should only happen for the blitter but
	 * each ring has its own status page so this should work for
	 * all rings
	 */
	pipe = intel_read_status_page(ring, I915_GEM_PGFLIP_INDEX);
	if (pipe) {
		/* Clear it to avoid responding to it twice */
		intel_write_status_page(ring, I915_GEM_PGFLIP_INDEX, 0);
	}

	/* Clear any simulated hang flags */
	if (dev_priv->gpu_error.stop_rings) {
		DRM_DEBUG_TDR("Simulated gpu hang, rst stop_rings bits %08x\n",
			(0x1 << ringid));
		dev_priv->gpu_error.stop_rings &= ~(0x1 << ringid);
	}

	ret = intel_ring_disable(ring, current_context);
	if (ret != 0) {
		DRM_ERROR("Failed to disable ring %d\n", ringid);
			goto handle_hung_ring_error;
	}

	if (!i915.enable_execlists) {
		/* Sample the current ring head position */
		head = I915_READ_HEAD(ring) & HEAD_ADDR;

	} else {
		hw_context_id1 = I915_READ(RING_EXECLIST_STATUS_CTX_ID(ring));

		/* Sample the current ring head position */
		head = I915_READ_HEAD(ring) & HEAD_ADDR;

		/*
		 * Make sure that the current context state is stable. If the
		 * context is changing then the MMIO head value might not be
		 * reliable. This is not a likely scenario but we have seen
		 * issues like this in the past.
		 */
		hw_context_id2 = I915_READ(RING_EXECLIST_STATUS_CTX_ID(ring));

		if (hw_context_id1 != hw_context_id2) {
			WARN(1, "Somehow the currently running context has " \
				"changed (%x != %x)! Bailing and retrying!\n",
				hw_context_id1, hw_context_id2);

			ret = intel_ring_enable(ring, current_context);
			if (ret != 0)
				DRM_ERROR("Failed to re-enable %s\n",
						ring->name);

			ret = -EAGAIN;
			goto handle_hung_ring_error;
		}
	}

	DRM_DEBUG_TDR("head 0x%08X, last_head 0x%08X\n",
		head, dev_priv->ring[ringid].hangcheck.last_head);
	if (head == dev_priv->ring[ringid].hangcheck.last_head) {
		/*
		 * The ring has not advanced since the last
		 * time it hung so force it to advance to the
		 * next QWORD. In most cases the ring head
		 * pointer will automatically advance to the
		 * next instruction as soon as it has read the
		 * current instruction, without waiting for it
		 * to complete. This seems to be the default
		 * behaviour, however an MBOX wait inserted
		 * directly to the VCS/BCS rings does not behave
		 * in the same way, instead the head pointer
		 * will still be pointing at the MBOX instruction
		 * until it completes.
		 */
		ring_flags = FORCE_ADVANCE;
		DRM_DEBUG_TDR("Force ring head to advance\n");
	}
	dev_priv->ring[ringid].hangcheck.last_head = head;

	ret = intel_ring_save(ring, current_context, ring_flags);
	if (ret == -EAGAIN) {
		if (intel_ring_enable(ring, current_context))
			DRM_ERROR("Failed to re-enable %s after " \
				  "deciding to retry\n", ring->name);

		goto handle_hung_ring_error;
	} else if (ret != 0) {
		DRM_ERROR("Failed to save ring state\n");
		goto handle_hung_ring_error;
	}

	ret = intel_gpu_engine_reset(dev, ringid);
	if (ret != 0) {
		DRM_ERROR("Failed to reset %s\n", ring->name);
		goto handle_hung_ring_error;
	}
	DRM_DEBUG_TDR("%s reset (GPU Hang)\n", ring->name);

	if (!i915.enable_execlists) {
		ret = intel_ring_invalidate_tlb(ring);
		if (ret != 0) {
			DRM_ERROR("Failed to invalidate tlb for %s\n",
					ring->name);
			goto handle_hung_ring_error;
		}
	}

	/* Clear last_acthd for the next hang check on this ring */
	dev_priv->ring[ringid].hangcheck.last_acthd = 0;

	/* Clear reset flags to allow future hangchecks */
	atomic_set(&dev_priv->ring[ringid].hangcheck.flags, 0);

	ret = intel_ring_restore(ring, current_context);
	if (ret != 0) {
		DRM_ERROR("Failed to restore ring state\n");
		goto handle_hung_ring_error;
	}

	/* Correct driver state */
	intel_gpu_engine_reset_resample(ring, current_context);

	ret = intel_ring_enable(ring, current_context);
	if (ret != 0) {
		DRM_ERROR("Failed to enable ring\n");
		goto handle_hung_ring_error;
	}

	/* Wake up anything waiting on this rings queue */
	wake_up_all(&ring->irq_queue);

	/*
	 * Note: This will not happen when MMIO based page flipping is used.
	 * Page flipping should continue unhindered as it will
	 * not be relying on a ring.
	 */
	if (pipe &&
		((pipe - 1) < ARRAY_SIZE(dev_priv->pipe_to_crtc_mapping))) {
		/* The pipe value in the status page is offset by 1 */
		pipe -= 1;

		/* The ring hung on a page flip command so we
		* must manually release the pending flip queue */
		crtc = dev_priv->pipe_to_crtc_mapping[pipe];
		intel_crtc = to_intel_crtc(crtc);
		unpin_work = intel_crtc->unpin_work;

		if (unpin_work && unpin_work->pending_flip_obj) {
			intel_prepare_page_flip(dev, intel_crtc->pipe);
			intel_finish_page_flip(dev, intel_crtc->pipe);
			DRM_DEBUG_TDR("Released stuck page flip for pipe %d\n",
				pipe);
		}
	}

handle_hung_ring_error:
	if (i915.enable_execlists)
		i915_gem_context_unreference(current_context);

	/* Release power lock */
	gen6_gt_force_wake_put(dev_priv, FORCEWAKE_ALL);

	return ret;
}

static int i915_reset_resubmit_contexts(struct drm_i915_private *dev_priv,
		struct intel_context **current_contexts)
{
	struct intel_engine_cs *ring;
	u32 i;

	for_each_ring(ring, dev_priv, i) {
		int ret = 0;
		u32 tail = 0;

		if (!current_contexts[i])
			continue;

		ret = I915_READ_TAIL_CTX(ring, current_contexts[i], tail);
		if (ret)
			return ret;

		intel_execlists_TDR_context_queue(ring, current_contexts[i],
					tail);
	}

	return 0;
}

static inline void i915_reset_unreference_contexts(struct drm_device *dev,
			struct intel_context **current_contexts)
{
	struct intel_engine_cs *ring;
	struct drm_i915_private *dev_priv = dev->dev_private;
	u32 i;

	for_each_ring(ring, dev_priv, i) {
		if (!current_contexts[i])
			continue;

		i915_gem_context_unreference(current_contexts[i]);
	}
}

/**
 * i915_reset - reset chip after a hang
 * @dev: drm device to reset
 *
 * Reset the chip.  Useful if a hang is detected. Returns zero on successful
 * reset or otherwise an error code.
 *
 * Procedure is fairly simple:
 *   - reset the chip using the reset reg
 *   - re-init context state
 *   - re-init hardware status page
 *   - re-init ring buffer
 *   - re-init interrupt state
 *   - re-init display
 */
int i915_reset(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;
	bool simulated;
	int ret = 0;
	struct intel_engine_cs *ring;
	u32 i;
	struct intel_context *current_contexts[I915_NUM_RINGS];

	if (!i915.reset)
		return 0;

	mutex_lock(&dev->struct_mutex);
	memset(current_contexts, 0, sizeof(current_contexts));

	DRM_ERROR("Reset GPU (GPU Hang)\n");

	if (i915.enable_execlists) {
		/*
		 * Store local reference to the current ring contexts before
		 * reset so that we can restore them after the reset breaks them
		 * (EXECLIST_STATUS register is clobbered by GPU reset and we
		 * use that register to fetch the current context, which is
		 * needed for final TDR context resubmission to kick off the
		 * hardware again post-reset)
		 */
		for_each_ring(ring, dev_priv, i) {
			enum context_submission_status status =
				i915_gem_context_get_current_context(ring,
					&current_contexts[i]);

			if (status == CONTEXT_SUBMISSION_STATUS_NONE_SUBMITTED) {
				i915_gem_context_unreference(current_contexts[i]);
				current_contexts[i] = NULL;
			}

		}
	}
	i915_gem_reset(dev);

	simulated = dev_priv->gpu_error.stop_rings != 0;

	if (!simulated && (get_seconds() - dev_priv->gpu_error.last_reset)
		< i915.gpu_reset_min_alive_period) {
		DRM_ERROR("GPU hanging too fast!\n");
		ret = -ENODEV;
	} else {
		ret = intel_gpu_reset(dev);

		/* Also reset the gpu hangman. */
		if (simulated) {
			DRM_INFO("Simulated gpu hang, resetting stop_rings\n");
			dev_priv->gpu_error.stop_rings = 0;
			if (ret == -ENODEV) {
				DRM_INFO("Reset not implemented, but ignoring "
					  "error for simulated gpu hangs\n");
				ret = 0;
			}
		} else
			dev_priv->gpu_error.last_reset = get_seconds();
	}

	if (ret) {
		DRM_ERROR("Failed to reset chip: %i\n", ret);
		goto exit_locked;
	}

	/* Ok, now get things going again... */

	/*
	 * Everything depends on having the GTT running, so we need to start
	 * there.  Fortunately we don't need to do this unless we reset the
	 * chip at a PCI level.
	 *
	 * Ring buffer needs to be re-initialized in the KMS case, or if X
	 * was running at the time of the reset (i.e. we weren't VT
	 * switched away).
	 */
	if (drm_core_check_feature(dev, DRIVER_MODESET) ||
			!dev_priv->ums.mm_suspended) {
		dev_priv->ums.mm_suspended = 0;

		ret = i915_gem_init_hw(dev);
		if (ret) {
			DRM_ERROR("Failed hw init on reset %d\n", ret);
			goto exit_locked;
		}

		if (ret == 0)
			intel_chv_huc_load(dev);

		if (i915.enable_execlists)
			for_each_ring(ring, dev_priv, i) {
				if (current_contexts[i]) {
					/*
					 * Init context state based
					 * on engine state
					 */
					intel_gpu_reset_resample(ring,
						current_contexts[i]);
				}
			}

		mutex_unlock(&dev->struct_mutex);

		/*
		 * FIXME: This races pretty badly against concurrent holders of
		 * ring interrupts. This is possible since we've started to drop
		 * dev->struct_mutex in select places when waiting for the gpu.
		 */

		/*
		 * rps/rc6 re-init is necessary to restore state lost after the
		 * reset and the re-install of gt irqs. Skip for ironlake per
		 * previous concerns that it doesn't respond well to some forms
		 * of re-init after reset.
		 */
		if (INTEL_INFO(dev)->gen > 5)
			intel_reset_gt_powersave(dev);

		intel_hpd_init(dev);
	} else {
		mutex_unlock(&dev->struct_mutex);
	}

	if (i915.enable_execlists) {
		i915_reset_resubmit_contexts(dev_priv, current_contexts);

		mutex_lock(&dev->struct_mutex);
		i915_reset_unreference_contexts(dev, current_contexts);
		mutex_unlock(&dev->struct_mutex);
	}

	return ret;

exit_locked:
	WARN_ON(!mutex_is_locked(&dev->struct_mutex));

	if (i915.enable_execlists)
		i915_reset_unreference_contexts(dev, current_contexts);

	mutex_unlock(&dev->struct_mutex);

	return ret;
}

void i915_init_watchdog(struct drm_device *dev)
{
	struct drm_i915_private *dev_priv = dev->dev_private;

	/* Based on pre-defined time out value (60ms or 30ms) calculate
	* timer count thresholds needed based on core frequency.
	*
	* For RCS.
	* The timestamp resolution changed in Gen7 and beyond to 80ns
	* for all pipes. Before that it was 640ns.*/

	int freq;

	if (INTEL_INFO(dev)->gen >= 7)
		freq = KM_TIMESTAMP_CNTS_PER_SEC_80NS;
	else
		freq = KM_TIMESTAMP_CNTS_PER_SEC_640NS;

	dev_priv->ring[RCS].watchdog_threshold =
		((KM_MEDIA_ENGINE_TIMEOUT_VALUE_IN_MS) *
		(freq / KM_TIMER_MILLISECOND));

	dev_priv->ring[VCS].watchdog_threshold =
		((KM_BSD_ENGINE_TIMEOUT_VALUE_IN_MS) *
		(freq / KM_TIMER_MILLISECOND));

	dev_priv->ring[VCS2].watchdog_threshold =
		((KM_BSD_ENGINE_TIMEOUT_VALUE_IN_MS) *
		(freq / KM_TIMER_MILLISECOND));

	DRM_DEBUG_TDR("Watchdog Timeout Threshold, " \
			"RCS: 0x%08X, VCS: 0x%08X, VCS2: 0x%08X\n", \
			dev_priv->ring[RCS].watchdog_threshold,
			dev_priv->ring[VCS].watchdog_threshold,
			dev_priv->ring[VCS2].watchdog_threshold);
}

static int i915_pci_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct intel_device_info *intel_info =
		(struct intel_device_info *) ent->driver_data;

	if (IS_PRELIMINARY_HW(intel_info) && !i915.preliminary_hw_support) {
		DRM_INFO("This hardware requires preliminary hardware support.\n"
			 "See CONFIG_DRM_I915_PRELIMINARY_HW_SUPPORT, and/or modparam preliminary_hw_support\n");
		return -ENODEV;
	}

	/* Only bind to function 0 of the device. Early generations
	 * used function 1 as a placeholder for multi-head. This causes
	 * us confusion instead, especially on the systems where both
	 * functions have the same PCI-ID!
	 */
	if (PCI_FUNC(pdev->devfn))
		return -ENODEV;

	driver.driver_features &= ~(DRIVER_USE_AGP);

	return drm_get_pci_dev(pdev, ent, &driver);
}

static void
i915_pci_remove(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	drm_put_dev(dev);
}

static int i915_pm_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);

	if (!drm_dev || !drm_dev->dev_private) {
		dev_err(dev, "DRM not initialized, aborting suspend.\n");
		return -ENODEV;
	}

	if (drm_dev->switch_power_state == DRM_SWITCH_POWER_OFF)
		return 0;

	return i915_drm_freeze(drm_dev);
}

static int i915_pm_suspend_late(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);

	/*
	 * We have a suspedn ordering issue with the snd-hda driver also
	 * requiring our device to be power up. Due to the lack of a
	 * parent/child relationship we currently solve this with an late
	 * suspend hook.
	 *
	 * FIXME: This should be solved with a special hdmi sink device or
	 * similar so that power domains can be employed.
	 */
	if (drm_dev->switch_power_state == DRM_SWITCH_POWER_OFF)
		return 0;

	pci_disable_device(pdev);
	pci_set_power_state(pdev, PCI_D3hot);

	return 0;
}

static int i915_pm_resume_early(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);

	return i915_resume_early(drm_dev);
}

static int i915_pm_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);

	return i915_resume(drm_dev);
}

static int i915_pm_freeze(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);

	if (!drm_dev || !drm_dev->dev_private) {
		dev_err(dev, "DRM not initialized, aborting suspend.\n");
		return -ENODEV;
	}

	return i915_drm_freeze(drm_dev);
}

static int i915_pm_thaw_early(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);

	return i915_drm_thaw_early(drm_dev);
}

static int i915_pm_thaw(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);

	return i915_drm_thaw(drm_dev);
}

static int i915_pm_poweroff(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct drm_device *drm_dev = pci_get_drvdata(pdev);

	return i915_drm_freeze(drm_dev);
}

static int hsw_suspend_complete(struct drm_i915_private *dev_priv)
{
	hsw_enable_pc8(dev_priv);

	return 0;
}

static int snb_resume_prepare(struct drm_i915_private *dev_priv,
				bool rpm_resume)
{
	struct drm_device *dev = dev_priv->dev;

	if (rpm_resume)
		intel_init_pch_refclk(dev);

	return 0;
}

static int hsw_resume_prepare(struct drm_i915_private *dev_priv,
				bool rpm_resume)
{
	hsw_disable_pc8(dev_priv);

	return 0;
}

/*
 * Save all Gunit registers that may be lost after a D3 and a subsequent
 * S0i[R123] transition. The list of registers needing a save/restore is
 * defined in the VLV2_S0IXRegs document. This documents marks all Gunit
 * registers in the following way:
 * - Driver: saved/restored by the driver
 * - Punit : saved/restored by the Punit firmware
 * - No, w/o marking: no need to save/restore, since the register is R/O or
 *                    used internally by the HW in a way that doesn't depend
 *                    keeping the content across a suspend/resume.
 * - Debug : used for debugging
 *
 * We save/restore all registers marked with 'Driver', with the following
 * exceptions:
 * - Registers out of use, including also registers marked with 'Debug'.
 *   These have no effect on the driver's operation, so we don't save/restore
 *   them to reduce the overhead.
 * - Registers that are fully setup by an initialization function called from
 *   the resume path. For example many clock gating and RPS/RC6 registers.
 * - Registers that provide the right functionality with their reset defaults.
 *
 * TODO: Except for registers that based on the above 3 criteria can be safely
 * ignored, we save/restore all others, practically treating the HW context as
 * a black-box for the driver. Further investigation is needed to reduce the
 * saved/restored registers even further, by following the same 3 criteria.
 */
static void vlv_save_gunit_s0ix_state(struct drm_i915_private *dev_priv)
{
	struct vlv_s0ix_state *s = &dev_priv->vlv_s0ix_state;
	int i;

	/* GAM 0x4000-0x4770 */
	s->wr_watermark		= I915_READ(GEN7_WR_WATERMARK);
	s->gfx_prio_ctrl	= I915_READ(GEN7_GFX_PRIO_CTRL);
	s->arb_mode		= I915_READ(ARB_MODE);
	s->gfx_pend_tlb0	= I915_READ(GEN7_GFX_PEND_TLB0);
	s->gfx_pend_tlb1	= I915_READ(GEN7_GFX_PEND_TLB1);

	for (i = 0; i < ARRAY_SIZE(s->lra_limits); i++)
		s->lra_limits[i] = I915_READ(GEN7_LRA_LIMITS_BASE + i * 4);

	s->media_max_req_count	= I915_READ(GEN7_MEDIA_MAX_REQ_COUNT);
	s->gfx_max_req_count	= I915_READ(GEN7_MEDIA_MAX_REQ_COUNT);

	s->render_hwsp		= I915_READ(RENDER_HWS_PGA_GEN7);
	s->ecochk		= I915_READ(GAM_ECOCHK);
	s->bsd_hwsp		= I915_READ(BSD_HWS_PGA_GEN7);
	s->blt_hwsp		= I915_READ(BLT_HWS_PGA_GEN7);

	s->tlb_rd_addr		= I915_READ(GEN7_TLB_RD_ADDR);

	/* MBC 0x9024-0x91D0, 0x8500 */
	s->g3dctl		= I915_READ(VLV_G3DCTL);
	s->gsckgctl		= I915_READ(VLV_GSCKGCTL);
	s->mbctl		= I915_READ(GEN6_MBCTL);

	/* GCP 0x9400-0x9424, 0x8100-0x810C */
	s->ucgctl1		= I915_READ(GEN6_UCGCTL1);
	s->ucgctl3		= I915_READ(GEN6_UCGCTL3);
	s->rcgctl1		= I915_READ(GEN6_RCGCTL1);
	s->rcgctl2		= I915_READ(GEN6_RCGCTL2);
	s->rstctl		= I915_READ(GEN6_RSTCTL);
	s->misccpctl		= I915_READ(GEN7_MISCCPCTL);

	/* GPM 0xA000-0xAA84, 0x8000-0x80FC */
	s->gfxpause		= I915_READ(GEN6_GFXPAUSE);
	s->rpdeuhwtc		= I915_READ(GEN6_RPDEUHWTC);
	s->rpdeuc		= I915_READ(GEN6_RPDEUC);
	s->ecobus		= I915_READ(ECOBUS);
	s->pwrdwnupctl		= I915_READ(VLV_PWRDWNUPCTL);
	s->rp_down_timeout	= I915_READ(GEN6_RP_DOWN_TIMEOUT);
	s->rp_deucsw		= I915_READ(GEN6_RPDEUCSW);
	s->rcubmabdtmr		= I915_READ(GEN6_RCUBMABDTMR);
	s->rcedata		= I915_READ(VLV_RCEDATA);
	s->spare2gh		= I915_READ(VLV_SPAREG2H);

	/* Display CZ domain, 0x4400C-0x4402C, 0x4F000-0x4F11F */
	s->gt_imr		= I915_READ(GTIMR);
	s->gt_ier		= I915_READ(GTIER);
	s->pm_imr		= I915_READ(GEN6_PMIMR);
	s->pm_ier		= I915_READ(GEN6_PMIER);

	for (i = 0; i < ARRAY_SIZE(s->gt_scratch); i++)
		s->gt_scratch[i] = I915_READ(GEN7_GT_SCRATCH_BASE + i * 4);

	/* GT SA CZ domain, 0x100000-0x138124 */
	s->tilectl		= I915_READ(TILECTL);
	s->gt_fifoctl		= I915_READ(GTFIFOCTL);
	s->gtlc_wake_ctrl	= I915_READ(VLV_GTLC_WAKE_CTRL);
	s->gtlc_survive		= I915_READ(VLV_GTLC_SURVIVABILITY_REG);
	s->pmwgicz		= I915_READ(VLV_PMWGICZ);

	/* Gunit-Display CZ domain, 0x182028-0x1821CF */
	s->gu_ctl0		= I915_READ(VLV_GU_CTL0);
	s->gu_ctl1		= I915_READ(VLV_GU_CTL1);
	s->clock_gate_dis2	= I915_READ(VLV_GUNIT_CLOCK_GATE2);

	/*
	 * Not saving any of:
	 * DFT,		0x9800-0x9EC0
	 * SARB,	0xB000-0xB1FC
	 * GAC,		0x5208-0x524C, 0x14000-0x14C000
	 * PCI CFG
	 */
}

static void vlv_restore_gunit_s0ix_state(struct drm_i915_private *dev_priv)
{
	struct vlv_s0ix_state *s = &dev_priv->vlv_s0ix_state;
	u32 val;
	int i;

	/* GAM 0x4000-0x4770 */
	I915_WRITE(GEN7_WR_WATERMARK,	s->wr_watermark);
	I915_WRITE(GEN7_GFX_PRIO_CTRL,	s->gfx_prio_ctrl);
	I915_WRITE(ARB_MODE,		s->arb_mode | (0xffff << 16));
	I915_WRITE(GEN7_GFX_PEND_TLB0,	s->gfx_pend_tlb0);
	I915_WRITE(GEN7_GFX_PEND_TLB1,	s->gfx_pend_tlb1);

	for (i = 0; i < ARRAY_SIZE(s->lra_limits); i++)
		I915_WRITE(GEN7_LRA_LIMITS_BASE + i * 4, s->lra_limits[i]);

	I915_WRITE(GEN7_MEDIA_MAX_REQ_COUNT, s->media_max_req_count);
	I915_WRITE(GEN7_MEDIA_MAX_REQ_COUNT, s->gfx_max_req_count);

	I915_WRITE(RENDER_HWS_PGA_GEN7,	s->render_hwsp);
	I915_WRITE(GAM_ECOCHK,		s->ecochk);
	I915_WRITE(BSD_HWS_PGA_GEN7,	s->bsd_hwsp);
	I915_WRITE(BLT_HWS_PGA_GEN7,	s->blt_hwsp);

	I915_WRITE(GEN7_TLB_RD_ADDR,	s->tlb_rd_addr);

	/* MBC 0x9024-0x91D0, 0x8500 */
	I915_WRITE(VLV_G3DCTL,		s->g3dctl);
	I915_WRITE(VLV_GSCKGCTL,	s->gsckgctl);
	I915_WRITE(GEN6_MBCTL,		s->mbctl);

	/* GCP 0x9400-0x9424, 0x8100-0x810C */
	I915_WRITE(GEN6_UCGCTL1,	s->ucgctl1);
	I915_WRITE(GEN6_UCGCTL3,	s->ucgctl3);
	I915_WRITE(GEN6_RCGCTL1,	s->rcgctl1);
	I915_WRITE(GEN6_RCGCTL2,	s->rcgctl2);
	I915_WRITE(GEN6_RSTCTL,		s->rstctl);
	I915_WRITE(GEN7_MISCCPCTL,	s->misccpctl);

	/* GPM 0xA000-0xAA84, 0x8000-0x80FC */
	I915_WRITE(GEN6_GFXPAUSE,	s->gfxpause);
	I915_WRITE(GEN6_RPDEUHWTC,	s->rpdeuhwtc);
	I915_WRITE(GEN6_RPDEUC,		s->rpdeuc);
	I915_WRITE(ECOBUS,		s->ecobus);
	I915_WRITE(VLV_PWRDWNUPCTL,	s->pwrdwnupctl);
	I915_WRITE(GEN6_RP_DOWN_TIMEOUT,s->rp_down_timeout);
	I915_WRITE(GEN6_RPDEUCSW,	s->rp_deucsw);
	I915_WRITE(GEN6_RCUBMABDTMR,	s->rcubmabdtmr);
	I915_WRITE(VLV_RCEDATA,		s->rcedata);
	I915_WRITE(VLV_SPAREG2H,	s->spare2gh);

	/* Display CZ domain, 0x4400C-0x4402C, 0x4F000-0x4F11F */
	I915_WRITE(GTIMR,		s->gt_imr);
	I915_WRITE(GTIER,		s->gt_ier);
	I915_WRITE(GEN6_PMIMR,		s->pm_imr);
	I915_WRITE(GEN6_PMIER,		s->pm_ier);

	for (i = 0; i < ARRAY_SIZE(s->gt_scratch); i++)
		I915_WRITE(GEN7_GT_SCRATCH_BASE + i * 4, s->gt_scratch[i]);

	/* GT SA CZ domain, 0x100000-0x138124 */
	I915_WRITE(TILECTL,			s->tilectl);
	I915_WRITE(GTFIFOCTL,			s->gt_fifoctl);
	/*
	 * Preserve the GT allow wake and GFX force clock bit, they are not
	 * be restored, as they are used to control the s0ix suspend/resume
	 * sequence by the caller.
	 */
	val = I915_READ(VLV_GTLC_WAKE_CTRL);
	val &= VLV_GTLC_ALLOWWAKEREQ;
	val |= s->gtlc_wake_ctrl & ~VLV_GTLC_ALLOWWAKEREQ;
	I915_WRITE(VLV_GTLC_WAKE_CTRL, val);

	val = I915_READ(VLV_GTLC_SURVIVABILITY_REG);
	val &= VLV_GFX_CLK_FORCE_ON_BIT;
	val |= s->gtlc_survive & ~VLV_GFX_CLK_FORCE_ON_BIT;
	I915_WRITE(VLV_GTLC_SURVIVABILITY_REG, val);

	I915_WRITE(VLV_PMWGICZ,			s->pmwgicz);

	/* Gunit-Display CZ domain, 0x182028-0x1821CF */
	I915_WRITE(VLV_GU_CTL0,			s->gu_ctl0);
	I915_WRITE(VLV_GU_CTL1,			s->gu_ctl1);
	I915_WRITE(VLV_GUNIT_CLOCK_GATE2,	s->clock_gate_dis2);
}

int vlv_force_gfx_clock(struct drm_i915_private *dev_priv, bool force_on)
{
	u32 val;
	int err;

	val = I915_READ(VLV_GTLC_SURVIVABILITY_REG);

#define COND (I915_READ(VLV_GTLC_SURVIVABILITY_REG) & VLV_GFX_CLK_STATUS_BIT)
	/* Wait for a previous force-off to settle */
	if (force_on && !IS_CHERRYVIEW(dev_priv->dev)) {
		/* WARN_ON only for the Valleyview */
		WARN_ON(!!(val & VLV_GFX_CLK_FORCE_ON_BIT) == force_on);

		err = wait_for(!COND, 20);
		if (err) {
			DRM_ERROR("timeout waiting for GFX clock force-off (%08x)\n",
				  I915_READ(VLV_GTLC_SURVIVABILITY_REG));
			return err;
		}
	}

	val = I915_READ(VLV_GTLC_SURVIVABILITY_REG);
	val &= ~VLV_GFX_CLK_FORCE_ON_BIT;
	if (force_on)
		val |= VLV_GFX_CLK_FORCE_ON_BIT;
	I915_WRITE(VLV_GTLC_SURVIVABILITY_REG, val);

	if (!force_on)
		return 0;

	err = wait_for(COND, 20);
	if (err)
		DRM_ERROR("timeout waiting for GFX clock force-on (%08x)\n",
			  I915_READ(VLV_GTLC_SURVIVABILITY_REG));

	return err;
#undef COND
}

static int vlv_allow_gt_wake(struct drm_i915_private *dev_priv, bool allow)
{
	u32 val;
	int err = 0;

	val = I915_READ(VLV_GTLC_WAKE_CTRL);
	val &= ~VLV_GTLC_ALLOWWAKEREQ;
	if (allow)
		val |= VLV_GTLC_ALLOWWAKEREQ;
	I915_WRITE(VLV_GTLC_WAKE_CTRL, val);
	POSTING_READ(VLV_GTLC_WAKE_CTRL);

#define COND (!!(I915_READ(VLV_GTLC_PW_STATUS) & VLV_GTLC_ALLOWWAKEACK) == \
	      allow)
	err = wait_for(COND, 1);
	if (err)
		DRM_ERROR("timeout disabling GT waking\n");
	return err;
#undef COND
}

static int vlv_wait_for_gt_wells(struct drm_i915_private *dev_priv,
				 bool wait_for_on)
{
	u32 mask;
	u32 val;
	int err;

	mask = VLV_GTLC_PW_MEDIA_STATUS_MASK | VLV_GTLC_PW_RENDER_STATUS_MASK;
	val = wait_for_on ? mask : 0;
#define COND ((I915_READ(VLV_GTLC_PW_STATUS) & mask) == val)
	if (COND)
		return 0;

	DRM_DEBUG_KMS("waiting for GT wells to go %s (%08x)\n",
			wait_for_on ? "on" : "off",
			I915_READ(VLV_GTLC_PW_STATUS));

	/*
	 * RC6 transitioning can be delayed up to 2 msec (see
	 * valleyview_enable_rps), use 3 msec for safety.
	 */
	err = wait_for(COND, 3);
	if (err)
		DRM_ERROR("timeout waiting for GT wells to go %s\n",
			  wait_for_on ? "on" : "off");

	return err;
#undef COND
}

static void vlv_check_no_gt_access(struct drm_i915_private *dev_priv)
{
	if (!(I915_READ(VLV_GTLC_PW_STATUS) & VLV_GTLC_ALLOWWAKEERR))
		return;

	DRM_ERROR("GT register access while GT waking disabled\n");
	I915_WRITE(VLV_GTLC_PW_STATUS, VLV_GTLC_ALLOWWAKEERR);
}

static int vlv_suspend_complete(struct drm_i915_private *dev_priv)
{
	u32 mask;
	int err;

	WARN_ON(!dev_priv->power_domains.init_power_on);

	/*
	 * Bspec defines the following GT well on flags as debug only, so
	 * don't treat them as hard failures.
	 */
	(void)vlv_wait_for_gt_wells(dev_priv, false);

	mask = VLV_GTLC_RENDER_CTX_EXISTS | VLV_GTLC_MEDIA_CTX_EXISTS;
	WARN_ON((I915_READ(VLV_GTLC_WAKE_CTRL) & mask) != mask);

	vlv_check_no_gt_access(dev_priv);

	err = vlv_force_gfx_clock(dev_priv, true);
	if (err)
		goto err1;

	err = vlv_allow_gt_wake(dev_priv, false);
	if (err)
		goto err2;

	if (!IS_CHERRYVIEW(dev_priv->dev))
		vlv_save_gunit_s0ix_state(dev_priv);

	err = vlv_force_gfx_clock(dev_priv, false);
	if (err)
		goto err2;

	intel_display_set_init_power(dev_priv, false);

	return 0;

err2:
	/* For safety always re-enable waking and disable gfx clock forcing */
	vlv_allow_gt_wake(dev_priv, true);
err1:
	vlv_force_gfx_clock(dev_priv, false);

	return err;
}

static int vlv_resume_prepare(struct drm_i915_private *dev_priv,
				bool rpm_resume)
{
	struct drm_device *dev = dev_priv->dev;
	int err;
	int ret = 0;

	/*
	 * If any of the steps fail just try to continue, that's the best we
	 * can do at this point. Return the first error code (which will also
	 * leave RPM permanently disabled).
	 */
	if (!rpm_resume)
		WARN_ON(I915_READ(GEN6_RC_CONTROL));

	if (rpm_resume)
		ret = vlv_force_gfx_clock(dev_priv, true);

	if (!IS_CHERRYVIEW(dev_priv->dev))
		vlv_restore_gunit_s0ix_state(dev_priv);

	err = vlv_allow_gt_wake(dev_priv, true);
	if (!ret)
		ret = err;

	if (rpm_resume) {
		err = vlv_force_gfx_clock(dev_priv, false);
		if (!ret)
			ret = err;
	}

	vlv_check_no_gt_access(dev_priv);

	if (rpm_resume) {
		intel_init_clock_gating(dev);
		i915_gem_restore_fences(dev);
	}

	intel_display_set_init_power(dev_priv, true);

	return ret;
}

static int intel_runtime_suspend(struct device *device)
{
	struct pci_dev *pdev = to_pci_dev(device);
	struct drm_device *dev = pci_get_drvdata(pdev);
	struct drm_i915_private *dev_priv = dev->dev_private;
	int ret, i;

	if (WARN_ON_ONCE(!(dev_priv->rps.enabled && intel_enable_rc6(dev))))
		return -ENODEV;

	if (WARN_ON_ONCE(!HAS_RUNTIME_PM(dev)))
		return -ENODEV;

	assert_force_wake_inactive(dev_priv);

	DRM_DEBUG_KMS("Suspending device\n");

	/*
	 * We could deadlock here in case another thread holding struct_mutex
	 * calls RPM suspend concurrently, since the RPM suspend will wait
	 * first for this RPM suspend to finish. In this case the concurrent
	 * RPM resume will be followed by its RPM suspend counterpart. Still
	 * for consistency return -EAGAIN, which will reschedule this suspend.
	 */
	if (!mutex_trylock(&dev->struct_mutex)) {
		DRM_DEBUG_KMS("device lock contention, deffering suspend\n");
		/*
		 * Bump the expiration timestamp, otherwise the suspend won't
		 * be rescheduled.
		 */
		pm_runtime_mark_last_busy(device);

		return -EAGAIN;
	}
	/*
	 * We are safe here against re-faults, since the fault handler takes
	 * an RPM reference.
	 */
	i915_gem_release_all_mmaps(dev_priv);
	mutex_unlock(&dev->struct_mutex);

	if (IS_VALLEYVIEW(dev)) {
		spin_lock_irq(&dev_priv->irq_lock);
		valleyview_disable_display_irqs(dev_priv);
		spin_unlock_irq(&dev_priv->irq_lock);
	}
	/*
	 * rps.work can't be rearmed here, since we get here only after making
	 * sure the GPU is idle and the RPS freq is set to the minimum. See
	 * intel_mark_idle().
	 */
	cancel_work_sync(&dev_priv->rps.work);
	intel_runtime_pm_disable_interrupts(dev);

	ret = intel_suspend_complete(dev_priv);
	if (ret) {
		DRM_ERROR("Runtime suspend failed, disabling it (%d)\n", ret);
		intel_runtime_pm_restore_interrupts(dev);

		return ret;
	}

	for (i = 0; i < I915_NUM_RINGS; i++)
		cancel_delayed_work_sync(&dev_priv->ring[i].hangcheck.work);

	dev_priv->pm.suspended = true;

	/*
	 * current versions of firmware which depend on this opregion
	 * notification have repurposed the D1 definition to mean
	 * "runtime suspended" vs. what you would normally expect (D3)
	 * to distinguish it from notifications that might be sent
	 * via the suspend path.
	 */
	intel_opregion_notify_adapter(dev, PCI_D1);

	DRM_DEBUG_KMS("Device suspended\n");
	return 0;
}

#define __raw_i915_read32(dev_priv__, reg__) \
		readl((dev_priv__)->regs + (reg__))
#define __raw_i915_write32(dev_priv__, reg__, val__) \
		writel(val__, (dev_priv__)->regs + (reg__))

static int intel_runtime_resume(struct device *device)
{
	struct pci_dev *pdev = to_pci_dev(device);
	struct drm_device *dev = pci_get_drvdata(pdev);
	struct drm_i915_private *dev_priv = dev->dev_private;
	int ret;
	u32 gtfifodbg;

	/*
	 * FIXME: GTFIFODBG registers gets set to 0x10 post resume from S0iX.
	 * This leads to warning to be hit in gen6_gt_check_fifodbg from
	 * __vlv_force_wake_put called from register read first time post
	 * resume. Clearing it here.
	*/
	if (IS_VALLEYVIEW(dev)) {
		gtfifodbg = __raw_i915_read32(dev_priv, GTFIFODBG);
		__raw_i915_write32(dev_priv, GTFIFODBG, gtfifodbg);
	}

	if (WARN_ON_ONCE(!HAS_RUNTIME_PM(dev)))
		return -ENODEV;

	DRM_DEBUG_KMS("Resuming device\n");

	intel_opregion_notify_adapter(dev, PCI_D0);
	dev_priv->pm.suspended = false;

	ret = intel_resume_prepare(dev_priv, true);
	/*
	 * No point of rolling back things in case of an error, as the best
	 * we can do is to hope that things will still work (and disable RPM).
	 */
	i915_gem_init_swizzling(dev);
	gen6_update_ring_freq(dev);

	intel_runtime_pm_restore_interrupts(dev);
	intel_reset_gt_powersave(dev);

	if (ret)
		DRM_ERROR("Runtime resume failed, disabling it (%d)\n", ret);
	else
		DRM_DEBUG_KMS("Device resumed\n");

	return ret;
}

/*
 * This function implements common functionality of runtime and system
 * suspend sequence.
 */
static int intel_suspend_complete(struct drm_i915_private *dev_priv)
{
	struct drm_device *dev = dev_priv->dev;
	int ret;

	if (IS_HASWELL(dev) || IS_BROADWELL(dev))
		ret = hsw_suspend_complete(dev_priv);
	else if (IS_VALLEYVIEW(dev))
		ret = vlv_suspend_complete(dev_priv);
	else
		ret = 0;

	return ret;
}

/*
 * This function implements common functionality of runtime and system
 * resume sequence. Variable rpm_resume used for implementing different
 * code paths.
 */
static int intel_resume_prepare(struct drm_i915_private *dev_priv,
				bool rpm_resume)
{
	struct drm_device *dev = dev_priv->dev;
	int ret;

	if (IS_GEN6(dev))
		ret = snb_resume_prepare(dev_priv, rpm_resume);
	else if (IS_HASWELL(dev) || IS_BROADWELL(dev))
		ret = hsw_resume_prepare(dev_priv, rpm_resume);
	else if (IS_VALLEYVIEW(dev))
		ret = vlv_resume_prepare(dev_priv, rpm_resume);
	else
		ret = 0;

	return ret;
}

static const struct dev_pm_ops i915_pm_ops = {
	.suspend = i915_pm_suspend,
	.suspend_late = i915_pm_suspend_late,
	.resume_early = i915_pm_resume_early,
	.resume = i915_pm_resume,
	.freeze = i915_pm_freeze,
	.thaw_early = i915_pm_thaw_early,
	.thaw = i915_pm_thaw,
	.poweroff = i915_pm_poweroff,
	.restore_early = i915_pm_resume_early,
	.restore = i915_pm_resume,
	.runtime_suspend = intel_runtime_suspend,
	.runtime_resume = intel_runtime_resume,
};

static const struct vm_operations_struct i915_gem_vm_ops = {
	.fault = i915_gem_fault,
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};

static const struct file_operations i915_driver_fops = {
	.owner = THIS_MODULE,
	.open = drm_open,
	.release = drm_release,
	.unlocked_ioctl = drm_ioctl,
	.mmap = drm_gem_mmap,
	.poll = drm_poll,
	.read = drm_read,
#ifdef CONFIG_COMPAT
	.compat_ioctl = i915_compat_ioctl,
#endif
	.llseek = noop_llseek,
};

static struct drm_driver driver = {
	/* Don't use MTRRs here; the Xserver or userspace app should
	 * deal with them for Intel hardware.
	 */
	.driver_features =
	    DRIVER_USE_AGP |
	    DRIVER_HAVE_IRQ | DRIVER_IRQ_SHARED | DRIVER_GEM | DRIVER_PRIME |
	    DRIVER_RENDER,
	.load = i915_driver_load,
	.unload = i915_driver_unload,
	.open = i915_driver_open,
	.lastclose = i915_driver_lastclose,
	.preclose = i915_driver_preclose,
	.postclose = i915_driver_postclose,

	/* Used in place of i915_pm_ops for non-DRIVER_MODESET */
	.suspend = i915_suspend,
	.resume = i915_resume_legacy,

	.device_is_agp = i915_driver_device_is_agp,
	.master_create = i915_master_create,
	.master_destroy = i915_master_destroy,
#if defined(CONFIG_DEBUG_FS)
	.debugfs_init = i915_debugfs_init,
	.debugfs_cleanup = i915_debugfs_cleanup,
#endif
	.gem_open_object = i915_gem_open_object,
	.gem_close_object = i915_gem_close_object,
	.gem_free_object = i915_gem_free_object,
	.gem_vm_ops = &i915_gem_vm_ops,

	.prime_handle_to_fd = drm_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_export = i915_gem_prime_export,
	.gem_prime_import = i915_gem_prime_import,

	.dumb_create = i915_gem_dumb_create,
	.dumb_map_offset = i915_gem_mmap_gtt,
	.dumb_destroy = drm_gem_dumb_destroy,
	.ioctls = i915_ioctls,
	.fops = &i915_driver_fops,
	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
	.patchlevel = DRIVER_PATCHLEVEL,
};

static struct pci_driver i915_pci_driver = {
	.name = DRIVER_NAME,
	.id_table = pciidlist,
	.probe = i915_pci_probe,
	.remove = i915_pci_remove,
	.driver.pm = &i915_pm_ops,
};

static int __init i915_init(void)
{
	driver.num_ioctls = i915_max_ioctl;

	/*
	 * If CONFIG_DRM_I915_KMS is set, default to KMS unless
	 * explicitly disabled with the module pararmeter.
	 *
	 * Otherwise, just follow the parameter (defaulting to off).
	 *
	 * Allow optional vga_text_mode_force boot option to override
	 * the default behavior.
	 */
#if defined(CONFIG_DRM_I915_KMS)
	if (i915.modeset != 0)
		driver.driver_features |= DRIVER_MODESET;
#endif
	if (i915.modeset == 1)
		driver.driver_features |= DRIVER_MODESET;

#ifdef CONFIG_VGA_CONSOLE
	if (vgacon_text_force() && i915.modeset == -1)
		driver.driver_features &= ~DRIVER_MODESET;
#endif

	if (!(driver.driver_features & DRIVER_MODESET)) {
		driver.get_vblank_timestamp = NULL;
#ifndef CONFIG_DRM_I915_UMS
		/* Silently fail loading to not upset userspace. */
		DRM_DEBUG_DRIVER("KMS and UMS disabled.\n");
		return 0;
#endif
	}

	return drm_pci_init(&driver, &i915_pci_driver);
}

static void __exit i915_exit(void)
{
#ifndef CONFIG_DRM_I915_UMS
	if (!(driver.driver_features & DRIVER_MODESET))
		return; /* Never loaded a driver. */
#endif

	drm_pci_exit(&driver, &i915_pci_driver);
}

module_init(i915_init);
module_exit(i915_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL and additional rights");
