/*
 * Support for Intel Camera Imaging ISP subsystem.
 * Copyright (c) 2010 - 2015, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */


#ifndef __SP_LOCAL_H_INCLUDED__
#define __SP_LOCAL_H_INCLUDED__

#include <type_support.h>
#include "sp_global.h"

struct sp_state_s {
	int		pc;
	int		status_register;
	bool	is_broken;
	bool	is_idle;
	bool	is_sleeping;
	bool	is_stalling;
};

struct sp_stall_s {
	bool	fifo0;
	bool	fifo1;
	bool	fifo2;
	bool	fifo3;
	bool	fifo4;
	bool	fifo5;
	bool	fifo6;
	bool	fifo7;
	bool	fifo8;
	bool	fifo9;
	bool	fifoa;
	bool	dmem;
	bool	control_master;
	bool	icache_master;
};

#ifdef C_RUN
#include "hive_isp_css_sp_hrt.h"
#define sp_address_of(var)	_hrt_cell_get_crun_indexed_symbol(SP, var)
#ifdef C_RUN_DYNAMIC_LINK_PROGRAMS
#ifndef DUMMY_HRT_SYSMEM_FUNCTIONS
#define DUMMY_HRT_SYSMEM_FUNCTIONS
/* These two inline functions prevent gcc from generating compiler warnings
 * about unused static functions. */
static inline void *
dummy__hrt_sysmem_ident_address(hive_device_id mem, const char *sym)
{
	return __hrt_sysmem_ident_address(mem, sym);
}

static inline void
dummy_hrt_sysmem_map_var(hive_mem_id mem, const char *ident, volatile void *native_address, unsigned int size)
{
	_hrt_sysmem_map_var(mem, ident, native_address, size);
}
#endif /* DUMMY_HRT_SYSMEM_FUNCTIONS */
#endif /* C_RUN */
#else
#define sp_address_of(var)	(HIVE_ADDR_ ## var)
#endif

/*
 * deprecated
 */
#define store_sp_int(var, value) \
	sp_dmem_store_uint32(SP0_ID, (unsigned)sp_address_of(var), \
		(uint32_t)(value))

#define store_sp_ptr(var, value) \
	sp_dmem_store_uint32(SP0_ID, (unsigned)sp_address_of(var), \
		(uint32_t)(value))

#define load_sp_uint(var) \
	sp_dmem_load_uint32(SP0_ID, (unsigned)sp_address_of(var))

#define load_sp_array_uint8(array_name, index) \
	sp_dmem_load_uint8(SP0_ID, (unsigned)sp_address_of(array_name) + \
		(index)*sizeof(uint8_t))

#define load_sp_array_uint16(array_name, index) \
	sp_dmem_load_uint16(SP0_ID, (unsigned)sp_address_of(array_name) + \
		(index)*sizeof(uint16_t))

#define load_sp_array_uint(array_name, index) \
	sp_dmem_load_uint32(SP0_ID, (unsigned)sp_address_of(array_name) + \
		(index)*sizeof(uint32_t))

#define store_sp_var(var, data, bytes) \
	sp_dmem_store(SP0_ID, (unsigned)sp_address_of(var), data, bytes)

#define store_sp_array_uint8(array_name, index, value) \
	sp_dmem_store_uint8(SP0_ID, (unsigned)sp_address_of(array_name) + \
		(index)*sizeof(uint8_t), value)

#define store_sp_array_uint16(array_name, index, value) \
	sp_dmem_store_uint16(SP0_ID, (unsigned)sp_address_of(array_name) + \
		(index)*sizeof(uint16_t), value)

#define store_sp_array_uint(array_name, index, value) \
	sp_dmem_store_uint32(SP0_ID, (unsigned)sp_address_of(array_name) + \
		(index)*sizeof(uint32_t), value)

#define store_sp_var_with_offset(var, offset, data, bytes) \
	sp_dmem_store(SP0_ID, (unsigned)sp_address_of(var) + \
		offset, data, bytes)

#define load_sp_var(var, data, bytes) \
	sp_dmem_load(SP0_ID, (unsigned)sp_address_of(var), data, bytes)

#define load_sp_var_with_offset(var, offset, data, bytes) \
	sp_dmem_load(SP0_ID, (unsigned)sp_address_of(var) + offset, \
		data, bytes)

#endif /* __SP_LOCAL_H_INCLUDED__ */
