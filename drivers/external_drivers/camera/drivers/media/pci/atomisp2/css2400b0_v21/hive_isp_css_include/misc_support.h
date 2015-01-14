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

#ifndef __MISC_SUPPORT_H_INCLUDED__
#define __MISC_SUPPORT_H_INCLUDED__

/* suppress compiler warnings on unused variables */
#ifndef NOT_USED
#define NOT_USED(a) ((void)(a))
#endif

/* Calculate the  total bytes for pow(2) byte alignment */
#define tot_bytes_for_pow2_align(pow2, cur_bytes)	((cur_bytes + (pow2 - 1)) & ~(pow2 - 1))

#endif /* __MISC_SUPPORT_H_INCLUDED__ */
