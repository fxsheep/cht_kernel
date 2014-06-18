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

#include "core/intel_dc_config.h"

int intel_pipe_init(struct intel_pipe *pipe, struct device *dev,
	u8 idx, bool primary, enum intel_pipe_type type,
	const struct intel_plane *primary_plane,
	const struct intel_pipe_ops *ops, const char *name)
{
	if (!pipe || !primary_plane || !ops)
		return -EINVAL;

	pipe->primary = primary;
	pipe->type = type;
	pipe->primary_plane = primary_plane;
	pipe->ops = ops;

	return intel_dc_component_init(&pipe->base, dev, idx, name);
}

void intel_pipe_destroy(struct intel_pipe *pipe)
{
	if (pipe) {
		intel_dc_component_destroy(&pipe->base);
		memset(pipe, 0, sizeof(*pipe));
	}
}

int intel_pipe_hw_init(const struct intel_pipe *pipe)
{
	return 0;
}
