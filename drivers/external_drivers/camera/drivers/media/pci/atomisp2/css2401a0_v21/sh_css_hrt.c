/*
 * Support for Intel Camera Imaging ISP subsystem.
 *
 * Copyright (c) 2010 - 2015 Intel Corporation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version
 * 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#include "platform_support.h"

#include "sh_css_hrt.h"
#include "ia_css_debug.h"

#include "device_access.h"

#define __INLINE_EVENT__
#include "event_fifo.h"
#define __INLINE_SP__
#include "sp.h"
#define __INLINE_ISP__
#include "isp.h"
#define __INLINE_IRQ__
#include "irq.h"
#define __INLINE_FIFO_MONITOR__
#include "fifo_monitor.h"

#if !defined(HAS_NO_INPUT_SYSTEM)
#include "input_system.h"	/* MIPI_PREDICTOR_NONE,... */
#endif

/* System independent */
#include "sh_css_internal.h"
#if !defined(HAS_NO_INPUT_SYSTEM)
#include "ia_css_isys.h"
#endif


bool sh_css_hrt_system_is_idle(void)
{
	bool not_idle = false, idle;
	fifo_channel_t ch;

	idle = sp_ctrl_getbit(SP0_ID, SP_SC_REG, SP_IDLE_BIT);
	not_idle |= !idle;
	if (!idle)
		IA_CSS_WARNING("SP not idle");

	idle = isp_ctrl_getbit(ISP0_ID, ISP_SC_REG, ISP_IDLE_BIT);
	not_idle |= !idle;
	if (!idle)
		IA_CSS_WARNING("ISP not idle");

	for (ch=0; ch<N_FIFO_CHANNEL; ch++) {
		fifo_channel_state_t state;
		fifo_channel_get_state(FIFO_MONITOR0_ID, ch, &state);
		if (state.fifo_valid) {
			IA_CSS_WARNING("FIFO channel %d is not empty", ch);
			not_idle = true;
		}
	}

	return !not_idle;
}

enum ia_css_err sh_css_hrt_sp_wait(void)
{
#if defined(HAS_IRQ_MAP_VERSION_2)
	irq_sw_channel_id_t	irq_id = IRQ_SW_CHANNEL0_ID;
#else
	irq_sw_channel_id_t	irq_id = IRQ_SW_CHANNEL2_ID;
#endif
	/*
	 * Wait till SP is idle or till there is a SW2 interrupt
	 * The SW2 interrupt will be used when frameloop runs on SP
	 * and signals an event with similar meaning as SP idle
	 * (e.g. frame_done)
	 */
	while (!sp_ctrl_getbit(SP0_ID, SP_SC_REG, SP_IDLE_BIT) &&
		((irq_reg_load(IRQ0_ID,
			_HRT_IRQ_CONTROLLER_STATUS_REG_IDX) &
			(1U<<(irq_id + IRQ_SW_CHANNEL_OFFSET))) == 0)) {
		hrt_sleep();
	}

return IA_CSS_SUCCESS;
}
