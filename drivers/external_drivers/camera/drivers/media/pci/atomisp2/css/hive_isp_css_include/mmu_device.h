/*
Support for Intel Camera Imaging ISP subsystem.
Copyright (c) 2010 - 2015, Intel Corporation.

This program is free software; you can redistribute it and/or modify it
under the terms and conditions of the GNU General Public License,
version 2, as published by the Free Software Foundation.

This program is distributed in the hope it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.
*/

#ifndef __MMU_DEVICE_H_INCLUDED__
#define __MMU_DEVICE_H_INCLUDED__

/* The file mmu.h already exists */

/*
 * This file is included on every cell {SP,ISP,host} and on every system
 * that uses the MMU device. It defines the API to DLI bridge
 *
 * System and cell specific interfaces and inline code are included
 * conditionally through Makefile path settings.
 *
 *  - .        system and cell agnostic interfaces, constants and identifiers
 *	- public:  system agnostic, cell specific interfaces
 *	- private: system dependent, cell specific interfaces & inline implementations
 *	- global:  system specific constants and identifiers
 *	- local:   system and cell specific constants and identifiers
 */

#include "storage_class.h"

#include "system_local.h"
#include "mmu_local.h"

#ifndef __INLINE_MMU__
#define STORAGE_CLASS_MMU_H STORAGE_CLASS_EXTERN
#define STORAGE_CLASS_MMU_C 
#include "mmu_public.h"
#else  /* __INLINE_MMU__ */
#define STORAGE_CLASS_MMU_H STORAGE_CLASS_INLINE
#define STORAGE_CLASS_MMU_C STORAGE_CLASS_INLINE
#include "mmu_private.h"
#endif /* __INLINE_MMU__ */

#endif /* __MMU_DEVICE_H_INCLUDED__ */
