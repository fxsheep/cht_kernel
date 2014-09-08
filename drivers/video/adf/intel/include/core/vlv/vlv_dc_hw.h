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

#ifndef VLV_DC_HW_H
#define VLV_DC_HW_H

struct hw_context_entry {
	u32 reg;
	u32 value;
};

enum {
	PLANE_A = 0,
	PLANE_B,
	N_PRI_PLANE,
};

#endif
