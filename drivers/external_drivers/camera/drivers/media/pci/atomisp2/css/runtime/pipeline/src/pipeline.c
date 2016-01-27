/**
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

#include "ia_css_debug.h"
#include "sw_event_global.h"		/* encode_sw_event */
#include "sp.h"			/* cnd_sp_irq_enable() */
#include "assert_support.h"
#include "memory_access.h"
#include "sh_css_sp.h"
#include "ia_css_pipeline.h"
#include "ia_css_isp_param.h"
#include "ia_css_bufq.h"
#include "ia_css_frame.h"

#define PIPELINE_NUM_UNMAPPED                   (~0U)
#define PIPELINE_SP_THREAD_EMPTY_TOKEN          (0x0)
#define PIPELINE_SP_THREAD_RESERVED_TOKEN       (0x1)


/*******************************************************
*** Static variables
********************************************************/
static unsigned int pipeline_num_to_sp_thread_map[IA_CSS_PIPELINE_NUM_MAX];
static unsigned int pipeline_sp_thread_list[SH_CSS_MAX_SP_THREADS];

/*******************************************************
*** Static functions
********************************************************/
static void pipeline_init_sp_thread_map(void);
static void pipeline_map_num_to_sp_thread(unsigned int pipe_num);
static void pipeline_unmap_num_to_sp_thread(unsigned int pipe_num);
static void pipeline_init_defaults(
	struct ia_css_pipeline *pipeline,
	enum ia_css_pipe_id pipe_id,
	unsigned int pipe_num,
	unsigned int dvs_frame_delay);

static void pipeline_stage_destroy(struct ia_css_pipeline_stage *stage);
static enum ia_css_err pipeline_stage_create(
	struct ia_css_pipeline *pipeline,
	struct ia_css_pipeline_stage_desc *stage_desc,
	struct ia_css_pipeline_stage **new_stage);
static void ia_css_pipeline_set_zoom_stage(struct ia_css_pipeline *pipeline);
static void ia_css_pipeline_configure_inout_port(struct ia_css_pipeline *me,
	bool continuous);
static unsigned int  get_shared_stage_buffer_size(
	unsigned int width, unsigned int height, unsigned buf_idx);

/*******************************************************
*** Public functions
********************************************************/
void ia_css_pipeline_init(void)
{
	pipeline_init_sp_thread_map();
}

enum ia_css_err ia_css_pipeline_create(
	struct ia_css_pipeline *pipeline,
	enum ia_css_pipe_id pipe_id,
	unsigned int pipe_num,
	unsigned int dvs_frame_delay)
{
	assert(pipeline != NULL);
	IA_CSS_ENTER_PRIVATE("pipeline = %p, pipe_id = %d, pipe_num = %d, dvs_frame_delay = %d",
		     pipeline, pipe_id, pipe_num, dvs_frame_delay);
	if (pipeline == NULL) {
		IA_CSS_LEAVE_ERR_PRIVATE(IA_CSS_ERR_INVALID_ARGUMENTS);
		return IA_CSS_ERR_INVALID_ARGUMENTS;
	}

	pipeline_init_defaults(pipeline, pipe_id, pipe_num, dvs_frame_delay);

	IA_CSS_LEAVE_ERR_PRIVATE(IA_CSS_SUCCESS);
	return IA_CSS_SUCCESS;
}

void ia_css_pipeline_map(unsigned int pipe_num, bool map)
{
	assert(pipe_num < IA_CSS_PIPELINE_NUM_MAX);
	IA_CSS_ENTER_PRIVATE("pipe_num = %d, map = %d", pipe_num, map);

	if (pipe_num >= IA_CSS_PIPELINE_NUM_MAX) {
		IA_CSS_ERROR("Invalid pipe number");
		IA_CSS_LEAVE_PRIVATE("void");
		return;
	}
	if (map)
		pipeline_map_num_to_sp_thread(pipe_num);
	else
		pipeline_unmap_num_to_sp_thread(pipe_num);
	IA_CSS_LEAVE_PRIVATE("void");
}

/** @brief destroy a pipeline
 *
 * @param[in] pipeline
 * @return    None
 *
 */
void ia_css_pipeline_destroy(struct ia_css_pipeline *pipeline)
{
	assert(pipeline != NULL);
	IA_CSS_ENTER_PRIVATE("pipeline = %p", pipeline);

	if (pipeline == NULL) {
		IA_CSS_ERROR("NULL input parameter");
		IA_CSS_LEAVE_PRIVATE("void");
		return;
	}

	IA_CSS_LOG("pipe_num = %d", pipeline->pipe_num);

	/* Free the pipeline number */
	ia_css_pipeline_clean(pipeline);

	IA_CSS_LEAVE_PRIVATE("void");
}

/* Run a pipeline and wait till it completes. */
void ia_css_pipeline_start(enum ia_css_pipe_id pipe_id,
			   struct ia_css_pipeline *pipeline)
{
	uint8_t pipe_num = 0;
	unsigned int thread_id;

	assert(pipeline != NULL);
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
	      "ia_css_pipeline_start() enter: pipe_id=%d, pipeline=%p\n",
	      pipe_id, pipeline);
	pipeline->pipe_id = pipe_id;
	sh_css_sp_init_pipeline(pipeline, pipe_id, pipe_num,
				false, false, false, true, SH_CSS_BDS_FACTOR_1_00,
				SH_CSS_PIPE_CONFIG_OVRD_NO_OVRD,
				IA_CSS_INPUT_MODE_MEMORY, NULL, NULL,
#if !defined(HAS_NO_INPUT_SYSTEM)
				(mipi_port_ID_t) 0,
#endif
				NULL, NULL);
	ia_css_pipeline_get_sp_thread_id(pipe_num, &thread_id);
	if (!sh_css_sp_is_running()) {
		ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_pipeline_start() error,leaving\n");
		/* queues are invalid*/
		return;
	}
	ia_css_bufq_enqueue_psys_event(IA_CSS_PSYS_SW_EVENT_START_STREAM,
				       (uint8_t)thread_id,
				       0,
				       0);

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
	      "ia_css_pipeline_start() leave: return_void\n");
}

/**
 * @brief Query the SP thread ID.
 * Refer to "sh_css_internal.h" for details.
 */
bool ia_css_pipeline_get_sp_thread_id(unsigned int key, unsigned int *val)
{

	IA_CSS_ENTER("key=%d, val=%p", key, val);

	if ((val == NULL) || (key >= IA_CSS_PIPELINE_NUM_MAX) || (key >= IA_CSS_PIPE_ID_NUM)) {
		IA_CSS_LEAVE("return value = false");
		return false;
	}

	*val = pipeline_num_to_sp_thread_map[key];

	if (*val == (unsigned)PIPELINE_NUM_UNMAPPED) {
		IA_CSS_LOG("unmapped pipeline number");
		IA_CSS_LEAVE("return value = false");
		return false;
	}
	IA_CSS_LEAVE("return value = true");
	return true;
}

void ia_css_pipeline_dump_thread_map_info(void)
{
	unsigned int i;
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"pipeline_num_to_sp_thread_map:\n");
	for (i = 0; i < IA_CSS_PIPELINE_NUM_MAX; i++) {
		ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
			"pipe_num: %u, tid: 0x%x\n", i, pipeline_num_to_sp_thread_map[i]);
	}
}

enum ia_css_err ia_css_pipeline_request_stop(struct ia_css_pipeline *pipeline)
{
	enum ia_css_err err = IA_CSS_SUCCESS;
	unsigned int thread_id;

	assert(pipeline != NULL);

	if (pipeline == NULL)
		return IA_CSS_ERR_INVALID_ARGUMENTS;

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_pipeline_request_stop() enter: pipeline=%p\n",
		pipeline);
	pipeline->stop_requested = true;

	/* Send stop event to the sp*/
	/* This needs improvement, stop on all the pipes available
	 * in the stream*/
	ia_css_pipeline_get_sp_thread_id(pipeline->pipe_num, &thread_id);
	if (!sh_css_sp_is_running())
	{
		ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		"ia_css_pipeline_request_stop() leaving\n");
		/* queues are invalid */
		return IA_CSS_ERR_RESOURCE_NOT_AVAILABLE;
	}
	ia_css_bufq_enqueue_psys_event(IA_CSS_PSYS_SW_EVENT_STOP_STREAM,
				       (uint8_t)thread_id,
				       0,
				       0);
	sh_css_sp_uninit_pipeline(pipeline->pipe_num);

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		      "ia_css_pipeline_request_stop() leave: return_err=%d\n",
		      err);
	return err;
}

void ia_css_pipeline_clean(struct ia_css_pipeline *pipeline)
{
	struct ia_css_pipeline_stage *s;
	unsigned i, j;

	assert(pipeline != NULL);
	IA_CSS_ENTER_PRIVATE("pipeline = %p", pipeline);

	if (pipeline == NULL) {
		IA_CSS_ERROR("NULL input parameter");
		IA_CSS_LEAVE_PRIVATE("void");
		return;
	}

	for ( i = 0; i < IA_CSS_PIPE_MAX_OUTPUT_STAGE; i++) {
		for ( j = 0; j < NUM_SHARED_STAGE_BUFFERS; j++) {
			if (pipeline->shared_frame[i][j]) {
				ia_css_frame_free(pipeline->shared_frame[i][j]);
				pipeline->shared_frame[i][j] = NULL;
			}
		}
	}

	s = pipeline->stages;

	while (s) {
		struct ia_css_pipeline_stage *next = s->next;
		pipeline_stage_destroy(s);
		s = next;
	}
	pipeline_init_defaults(pipeline, pipeline->pipe_id, pipeline->pipe_num, pipeline->dvs_frame_delay);

	IA_CSS_LEAVE_PRIVATE("void");
}

/** @brief Add a stage to pipeline.
 *
 * @param       pipeline      Pointer to the pipeline to be added to.
 * @param[in]   stage_desc    The description of the stage
 * @param[out]	stage         The successor of the stage.
 * @return      IA_CSS_SUCCESS or error code upon error.
 *
 * Add a new stage to a non-NULL pipeline.
 * The stage consists of an ISP binary or firmware and input and
 * output arguments.
*/
enum ia_css_err ia_css_pipeline_create_and_add_stage(
		struct ia_css_pipeline *pipeline,
		struct ia_css_pipeline_stage_desc *stage_desc,
		struct ia_css_pipeline_stage **stage)
{
	struct ia_css_pipeline_stage *last, *new_stage = NULL;
	enum ia_css_err err;

	/* other arguments can be NULL */
	assert(pipeline != NULL);
	assert(stage_desc != NULL);
	last = pipeline->stages;

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		      "ia_css_pipeline_create_and_add_stage() enter:\n");
	if (!stage_desc->binary && !stage_desc->firmware
	    && (stage_desc->sp_func == IA_CSS_PIPELINE_NO_FUNC)) {
		ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
			      "ia_css_pipeline_create_and_add_stage() done:"
			      " Invalid args\n");

		return IA_CSS_ERR_INTERNAL_ERROR;
	}

	/* Find the last stage */
	while (last && last->next)
		last = last->next;

	/* if in_frame is not set, we use the out_frame from the previous
	 * stage, if no previous stage, it's an error.
	 */
	if ((stage_desc->sp_func == IA_CSS_PIPELINE_NO_FUNC)
		&& (!stage_desc->in_frame)
		&& (!stage_desc->firmware)
		&& (!stage_desc->binary->online)) {

		/* Do this only for ISP stages*/
		if (last && last->args.out_frame[0])
			stage_desc->in_frame = last->args.out_frame[0];

		if (!stage_desc->in_frame)
			return IA_CSS_ERR_INTERNAL_ERROR;
	}

	/* Create the new stage */
	err = pipeline_stage_create(pipeline, stage_desc, &new_stage);
	if (err != IA_CSS_SUCCESS) {
		ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
			      "ia_css_pipeline_create_and_add_stage() done:"
			      " stage_create_failed\n");
		return err;
	}

	if (last)
		last->next = new_stage;
	else
		pipeline->stages = new_stage;

	/* Output the new stage */
	if (stage)
		*stage = new_stage;

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		      "ia_css_pipeline_create_and_add_stage() done:\n");
	return IA_CSS_SUCCESS;
}

void ia_css_pipeline_finalize_stages(struct ia_css_pipeline *pipeline,
			bool continuous)
{
	unsigned i = 0;
	struct ia_css_pipeline_stage *stage;

	assert(pipeline != NULL);
	for (stage = pipeline->stages; stage; stage = stage->next) {
		stage->stage_num = i;
		i++;
	}
	pipeline->num_stages = i;

	ia_css_pipeline_set_zoom_stage(pipeline);
	ia_css_pipeline_configure_inout_port(pipeline, continuous);
}

enum ia_css_err ia_css_pipeline_get_stage(struct ia_css_pipeline *pipeline,
					  int mode,
					  struct ia_css_pipeline_stage **stage)
{
	struct ia_css_pipeline_stage *s;
	assert(pipeline != NULL);
	assert(stage != NULL);
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		      "ia_css_pipeline_get_stage() enter:\n");
	for (s = pipeline->stages; s; s = s->next) {
		if (s->mode == mode) {
			*stage = s;
			return IA_CSS_SUCCESS;
		}
	}
	return IA_CSS_ERR_INTERNAL_ERROR;
}

enum ia_css_err ia_css_pipeline_get_stage_from_fw(struct ia_css_pipeline *pipeline,
			  uint32_t fw_handle,
			  struct ia_css_pipeline_stage **stage)
{
	struct ia_css_pipeline_stage *s;
	assert(pipeline != NULL);
	assert(stage != NULL);
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,"%s() \n",__func__);
	for (s = pipeline->stages; s; s = s->next) {
		if ((s->firmware) && (s->firmware->handle == fw_handle)) {
			*stage = s;
			return IA_CSS_SUCCESS;
		}
	}
	return IA_CSS_ERR_INTERNAL_ERROR;
}

enum ia_css_err ia_css_pipeline_get_fw_from_stage(struct ia_css_pipeline *pipeline,
			  uint32_t stage_num,
			  uint32_t *fw_handle)
{
	struct ia_css_pipeline_stage *s;

	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,"%s() \n",__func__);
	if ((pipeline == NULL) || (fw_handle == NULL))
		return IA_CSS_ERR_INVALID_ARGUMENTS;

	for (s = pipeline->stages; s; s = s->next) {
		if((s->stage_num == stage_num) && (s->firmware)) {
			*fw_handle = s->firmware->handle;
			return IA_CSS_SUCCESS;
		}
	}
	return IA_CSS_ERR_INTERNAL_ERROR;
}

enum ia_css_err ia_css_pipeline_get_output_stage(
		struct ia_css_pipeline *pipeline,
		int mode,
		struct ia_css_pipeline_stage **stage)
{
	struct ia_css_pipeline_stage *s;
	assert(pipeline != NULL);
	assert(stage != NULL);
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
		      "ia_css_pipeline_get_output_stage() enter:\n");

	*stage = NULL;
	/* First find acceleration firmware at end of pipe */
	for (s = pipeline->stages; s; s = s->next) {
		if (s->firmware && s->mode == mode &&
		    s->firmware->info.isp.sp.enable.output)
			*stage = s;
	}
	if (*stage)
		return IA_CSS_SUCCESS;
	/* If no firmware, find binary in pipe */
	return ia_css_pipeline_get_stage(pipeline, mode, stage);
}

bool ia_css_pipeline_has_stopped(struct ia_css_pipeline *pipeline)
{
	/* Android compilation files if made an local variable
	stack size on android is limited to 2k and this structure
	is around 2.5K, in place of static malloc can be done but
	if this call is made too often it will lead to fragment memory
	versus a fixed allocation */
	static struct sh_css_sp_group sp_group;
	unsigned int thread_id;
	const struct ia_css_fw_info *fw;
	unsigned int HIVE_ADDR_sp_group;

	fw = &sh_css_sp_fw;
	HIVE_ADDR_sp_group = fw->info.sp.group;

	ia_css_pipeline_get_sp_thread_id(pipeline->pipe_num, &thread_id);
	sp_dmem_load(SP0_ID,
		     (unsigned int)sp_address_of(sp_group),
		     &sp_group, sizeof(struct sh_css_sp_group));
	return sp_group.pipe[thread_id].num_stages == 0;
}

#if defined(USE_INPUT_SYSTEM_VERSION_2401)
struct sh_css_sp_pipeline_io_status *ia_css_pipeline_get_pipe_io_status(void)
{
	return(&sh_css_sp_group.pipe_io_status);
}
#endif

bool ia_css_pipeline_is_mapped(unsigned int key)
{
	bool ret = false;

	IA_CSS_ENTER_PRIVATE("key = %d", key);

	if ((key >= IA_CSS_PIPELINE_NUM_MAX) || (key >= IA_CSS_PIPE_ID_NUM)) {
		IA_CSS_ERROR("Invalid key!!");
		IA_CSS_LEAVE_PRIVATE("return = %d", false);
		return false;
	}

	ret = (bool)(pipeline_num_to_sp_thread_map[key] != (unsigned)PIPELINE_NUM_UNMAPPED);

	IA_CSS_LEAVE_PRIVATE("return = %d", ret);
	return ret;
}

/*******************************************************
*** Static functions
********************************************************/

/* Pipeline:
 * To organize the several different binaries for each type of mode,
 * we use a pipeline. A pipeline contains a number of stages, each with
 * their own binary and frame pointers.
 * When stages are added to a pipeline, output frames that are not passed
 * from outside are automatically allocated.
 * When input frames are not passed from outside, each stage will use the
 * output frame of the previous stage as input (the full resolution output,
 * not the viewfinder output).
 * Pipelines must be cleaned and re-created when settings of the binaries
 * change.
 */
static void pipeline_stage_destroy(struct ia_css_pipeline_stage *stage)
{
	unsigned int i;
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		if (stage->out_frame_allocated[i]) {
			ia_css_frame_free(stage->args.out_frame[i]);
			stage->args.out_frame[i] = NULL;
		} else {
			if (stage->out_frame_shared_buffer[i] && stage->args.out_frame[i]) {
				sh_css_free(stage->args.out_frame[i]);
				stage->args.out_frame[i] = NULL;
			}
		}
	}
	if (stage->vf_frame_allocated) {
		ia_css_frame_free(stage->args.out_vf_frame);
		stage->args.out_vf_frame = NULL;
	} else {
		if (stage->vf_frame_shared_buffer && stage->args.out_vf_frame) {
			sh_css_free(stage->args.out_vf_frame);
			stage->args.out_vf_frame = NULL;
		}
	}
	sh_css_free(stage);
}

static void pipeline_init_sp_thread_map(void)
{
	unsigned int i;

	for (i = 1; i < SH_CSS_MAX_SP_THREADS; i++)
		pipeline_sp_thread_list[i] = PIPELINE_SP_THREAD_EMPTY_TOKEN;

	for (i = 0; i < IA_CSS_PIPELINE_NUM_MAX; i++)
		pipeline_num_to_sp_thread_map[i] = PIPELINE_NUM_UNMAPPED;
}

static void pipeline_map_num_to_sp_thread(unsigned int pipe_num)
{
	unsigned int i;
	bool found_sp_thread = false;

	/* pipe is not mapped to any thread */
	assert(pipeline_num_to_sp_thread_map[pipe_num]
		== (unsigned)PIPELINE_NUM_UNMAPPED);

	for (i = 0; i < SH_CSS_MAX_SP_THREADS; i++) {
		if (pipeline_sp_thread_list[i] ==
		    PIPELINE_SP_THREAD_EMPTY_TOKEN) {
			pipeline_sp_thread_list[i] =
			    PIPELINE_SP_THREAD_RESERVED_TOKEN;
			pipeline_num_to_sp_thread_map[pipe_num] = i;
			found_sp_thread = true;
			break;
		}
	}

	/* Make sure a mapping is found */
	/* I could do:
		assert(i < SH_CSS_MAX_SP_THREADS);

		But the below is more descriptive.
	*/
	assert(found_sp_thread != false);
}

static void pipeline_unmap_num_to_sp_thread(unsigned int pipe_num)
{
	unsigned int thread_id;
	assert(pipeline_num_to_sp_thread_map[pipe_num]
		!= (unsigned)PIPELINE_NUM_UNMAPPED);

	thread_id = pipeline_num_to_sp_thread_map[pipe_num];
	pipeline_num_to_sp_thread_map[pipe_num] = PIPELINE_NUM_UNMAPPED;
	pipeline_sp_thread_list[thread_id] = PIPELINE_SP_THREAD_EMPTY_TOKEN;
}

static enum ia_css_err pipeline_stage_create(
	struct ia_css_pipeline *pipeline,
	struct ia_css_pipeline_stage_desc *stage_desc,
	struct ia_css_pipeline_stage **new_stage)
{
	enum ia_css_err err = IA_CSS_SUCCESS;
	struct ia_css_pipeline_stage *stage = NULL;
	struct ia_css_binary *binary;
	struct ia_css_frame *vf_frame;
	struct ia_css_frame *out_frame[IA_CSS_BINARY_MAX_OUTPUT_PORTS];
	const struct ia_css_fw_info *firmware;
	unsigned int i;
	bool share_stage_buffers = false;

	/* Verify input parameters*/
	if (!(stage_desc->in_frame) && !(stage_desc->firmware)
	    && (stage_desc->binary) && !(stage_desc->binary->online)) {
	    err = IA_CSS_ERR_INTERNAL_ERROR;
		goto ERR;
	}

	binary = stage_desc->binary;
	firmware = stage_desc->firmware;
	vf_frame = stage_desc->vf_frame;
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		out_frame[i] = stage_desc->out_frame[i];
	}

	stage = sh_css_malloc(sizeof(*stage));
	if (stage == NULL) {
		err = IA_CSS_ERR_CANNOT_ALLOCATE_MEMORY;
		goto ERR;
	}
	memset(stage, 0, sizeof(*stage));

	if (firmware) {
		stage->binary = NULL;
		stage->binary_info =
		    (struct ia_css_binary_info *)&firmware->info.isp;
	} else {
		stage->binary = binary;
		if (binary)
			stage->binary_info =
			    (struct ia_css_binary_info *)binary->info;
		else
			stage->binary_info = NULL;
	}

	stage->firmware = firmware;
	stage->sp_func = stage_desc->sp_func;
	stage->max_input_width = stage_desc->max_input_width;
	stage->mode = stage_desc->mode;
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		stage->out_frame_allocated[i] = false;
		stage->out_frame_shared_buffer[i] = false;
	}
	stage->vf_frame_allocated = false;
	stage->vf_frame_shared_buffer = false;
	stage->next = NULL;
	sh_css_binary_args_reset(&stage->args);

	/* Turn on shared_stage_buffers on ISP2.7 to save memory allocation */
	share_stage_buffers = binary && (binary->info->sp.pipeline.isp_pipe_version == SH_CSS_ISP_PIPE_VERSION_2_7);
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		if (!(out_frame[i]) && (binary)
			&& (binary->out_frame_info[i].res.width)) {

			if (share_stage_buffers) {
				unsigned shared_buf_idx = pipeline->shared_buf_idx[i];

				if (pipeline->shared_frame[i][shared_buf_idx] == NULL) {
					struct ia_css_frame_info myInfo = binary->out_frame_info[i];

					unsigned stage_buf_size = get_shared_stage_buffer_size(
									myInfo.padded_width, myInfo.res.height, shared_buf_idx);
					if (!stage_buf_size) {
						err = IA_CSS_ERR_INTERNAL_ERROR;
						goto ERR;
					}

					err = ia_css_frame_allocate_with_buffer_size(
							&pipeline->shared_frame[i][shared_buf_idx], stage_buf_size, false);

					if (err != IA_CSS_SUCCESS)
						goto ERR;
				}

				/* Create frame w/o allocating memory */
				err = ia_css_frame_create_from_info(&out_frame[i], &binary->out_frame_info[i]);
				if (err != IA_CSS_SUCCESS)
					goto ERR;

				/* Assign data from shared resource */
				err = ia_css_frame_set_data(out_frame[i],
						pipeline->shared_frame[i][shared_buf_idx]->data,
						pipeline->shared_frame[i][shared_buf_idx]->data_bytes);
				if (err != IA_CSS_SUCCESS) {
					ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
						"pipeline_stage_create() shared buffer too small! sz(%d) req(%d)\n",
						pipeline->shared_frame[i][shared_buf_idx]->data_bytes,
						out_frame[i]->data_bytes);
					goto ERR;
				}

				stage->out_frame_shared_buffer[i] = true;
				pipeline->shared_buf_idx[i] ^= 0x1;
			} else {
				err = ia_css_frame_allocate_from_info(&out_frame[i],
							&binary->out_frame_info[i]);
				if (err != IA_CSS_SUCCESS)
					goto ERR;
				stage->out_frame_allocated[i] = true;
			}
		}
	}
	/* VF frame is not needed in case of need_pp
	   However, the capture binary needs a vf frame to write to.
	 */
	if (!vf_frame) {
		if ((binary && binary->vf_frame_info.res.width) ||
		    (firmware && firmware->info.isp.sp.enable.vf_veceven)
		    ) {
			if (share_stage_buffers) {
				unsigned port = 0;
				/* Create vf_frame w/o allocating memory */
				err = ia_css_frame_create_from_info(&vf_frame,
						&binary->vf_frame_info);
				if (err != IA_CSS_SUCCESS)
					goto ERR;

				/* Assign data from shared resource */
				err = ia_css_frame_set_data(vf_frame,
						pipeline->shared_frame[port][pipeline->shared_buf_idx[port]]->data,
						pipeline->shared_frame[port][pipeline->shared_buf_idx[port]]->data_bytes);
				if (err != IA_CSS_SUCCESS) {
					ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE,
						"pipeline_stage_create() vf shared buffer too small! sz(%d) req(%d)\n",
						pipeline->shared_frame[port][pipeline->shared_buf_idx[port]]->data_bytes,
						vf_frame->data_bytes);
					goto ERR;
				}

				stage->vf_frame_shared_buffer = true;
				pipeline->shared_buf_idx[port] ^= 0x1;
			} else {
				err = ia_css_frame_allocate_from_info(&vf_frame,
							&binary->vf_frame_info);
				if (err != IA_CSS_SUCCESS)
					goto ERR;
				stage->vf_frame_allocated = true;
			}
		}
	} else if (vf_frame && binary && binary->vf_frame_info.res.width
		&& !firmware) {
		/* only mark as allocated if buffer pointer available */
		if (vf_frame->data != mmgr_NULL)
			stage->vf_frame_allocated = true;
	}

	stage->args.in_frame = stage_desc->in_frame;
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++)
		stage->args.out_frame[i] = out_frame[i];
	stage->args.out_vf_frame = vf_frame;
	*new_stage = stage;
	return err;
ERR:
	if (stage != NULL)
		pipeline_stage_destroy(stage);
	return err;
}

static void pipeline_init_defaults(
	struct ia_css_pipeline *pipeline,
	enum ia_css_pipe_id pipe_id,
	unsigned int pipe_num,
	unsigned int dvs_frame_delay)
{
	struct ia_css_frame init_frame = DEFAULT_FRAME;
	unsigned int i, j;

	pipeline->pipe_id = pipe_id;
	pipeline->stages = NULL;
	pipeline->stop_requested = false;
	pipeline->current_stage = NULL;
	pipeline->in_frame = init_frame;
	for (i = 0; i < IA_CSS_PIPE_MAX_OUTPUT_STAGE; i++) {
		pipeline->out_frame[i] = init_frame;
		pipeline->vf_frame[i] = init_frame;
	}

	for (i = 0 ; i < IA_CSS_PIPE_MAX_OUTPUT_STAGE; i++) {
		pipeline->shared_buf_idx[i] = 0;
		for (j = 0; j < NUM_SHARED_STAGE_BUFFERS; j++) {
			pipeline->shared_frame[i][j] = NULL;
		}
	}

	pipeline->num_execs = -1;
	pipeline->acquire_isp_each_stage = true;
	pipeline->pipe_num = (uint8_t)pipe_num;
	pipeline->dvs_frame_delay = dvs_frame_delay;
}

static void ia_css_pipeline_set_zoom_stage(struct ia_css_pipeline *pipeline)
{
	struct ia_css_pipeline_stage *stage = NULL;
	enum ia_css_err err = IA_CSS_SUCCESS;

	assert(pipeline != NULL);
	if (pipeline->pipe_id == IA_CSS_PIPE_ID_PREVIEW) {
		/* in preview pipeline, vf_pp stage should do zoom */
		err = ia_css_pipeline_get_stage(pipeline, IA_CSS_BINARY_MODE_VF_PP, &stage);
		if (err == IA_CSS_SUCCESS)
			stage->enable_zoom = true;
	} else if (pipeline->pipe_id == IA_CSS_PIPE_ID_CAPTURE) {
		/* in capture pipeline, capture_pp stage should do zoom */
		err = ia_css_pipeline_get_stage(pipeline, IA_CSS_BINARY_MODE_CAPTURE_PP, &stage);
		if (err == IA_CSS_SUCCESS)
			stage->enable_zoom = true;
	} else if (pipeline->pipe_id == IA_CSS_PIPE_ID_VIDEO) {
		/* in video pipeline, video stage should do zoom */
		err = ia_css_pipeline_get_stage(pipeline, IA_CSS_BINARY_MODE_VIDEO, &stage);
		if (err == IA_CSS_SUCCESS)
			stage->enable_zoom = true;
	} else if (pipeline->pipe_id == IA_CSS_PIPE_ID_YUVPP) {
		/* in yuvpp pipeline, first yuv_scaler stage should do zoom */
		err = ia_css_pipeline_get_stage(pipeline, IA_CSS_BINARY_MODE_CAPTURE_PP, &stage);
		if (err == IA_CSS_SUCCESS)
			stage->enable_zoom = true;
	}
}

static void
ia_css_pipeline_configure_inout_port(struct ia_css_pipeline *me, bool continuous)
{
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE_PRIVATE,
		"ia_css_pipeline_configure_inout_port() enter: pipe_id(%d) continuous(%d)\n",
			me->pipe_id, continuous);
	switch (me->pipe_id) {
		case IA_CSS_PIPE_ID_PREVIEW:
		case IA_CSS_PIPE_ID_VIDEO:
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_INPUT,
						   (uint8_t)(continuous ? SH_CSS_COPYSINK_TYPE : SH_CSS_HOST_TYPE), 1);
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_OUTPUT,
						   (uint8_t)SH_CSS_HOST_TYPE, 1);
			break;
		case IA_CSS_PIPE_ID_COPY: /*Copy pipe ports configured to "offline" mode*/
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_INPUT,
						   (uint8_t)SH_CSS_HOST_TYPE, 1);
			if (continuous) {
				SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_OUTPUT,
						   (uint8_t)SH_CSS_COPYSINK_TYPE, 1);
				SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_OUTPUT,
						   (uint8_t)SH_CSS_TAGGERSINK_TYPE, 1);
			} else {
				SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_OUTPUT,
						   (uint8_t)SH_CSS_HOST_TYPE, 1);
			}
			break;
		case IA_CSS_PIPE_ID_CAPTURE:
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_INPUT,
						   (uint8_t)(continuous ? SH_CSS_TAGGERSINK_TYPE : SH_CSS_HOST_TYPE),
						   1);
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_OUTPUT,
						   (uint8_t)SH_CSS_HOST_TYPE, 1);
			break;
		case IA_CSS_PIPE_ID_YUVPP:
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_INPUT,
						   (uint8_t)(SH_CSS_HOST_TYPE), 1);
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_OUTPUT,
						   (uint8_t)SH_CSS_HOST_TYPE, 1);
			break;
		case IA_CSS_PIPE_ID_ACC:
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_INPUT,
						   (uint8_t)SH_CSS_HOST_TYPE, 1);
			SH_CSS_PIPE_PORT_CONFIG_SET(me->inout_port_config,
						   (uint8_t)SH_CSS_PORT_OUTPUT,
						   (uint8_t)SH_CSS_HOST_TYPE, 1);
			break;
		default:
			break;
	}
	ia_css_debug_dtrace(IA_CSS_DEBUG_TRACE_PRIVATE,
		"ia_css_pipeline_configure_inout_port() leave: inout_port_config(%x)\n",
		me->inout_port_config);
}

static unsigned int
get_shared_stage_buffer_size(unsigned int width, unsigned int height, unsigned buf_idx)
{
	unsigned int frame = width * height;

	/* On ISP 2.7 capture Pipe stages are constructed with required memory size as following
	 * PRE_DE #0 (RAW format):        2 frames (of padded input-res)
	 *         frame * 2 raw_bit_depth
	 * PRIMARY #0 (EED, YCgCo444_16): 6 frames (of padded inupt-res)
	 *         frame * 3 YUV * 2 bytes/elem
	 * PRIMARY #1 (IEFD, YUV420_16) : 3 frames (of padded input-res)
	 *         frame * 1 Y * 2 bytes/elem + frame * 2 UV * 1/4 decimation * 2 bytes/elem
	 * PRIMARY #2 (XNR3) :            1.5 frames (of effective-res)
	 *         frame * 1 Y * 1 bytes/elem + frame * 2 UV * 1/4 decimation * 1 bytes/elem
	 * PRIMARY #3 (CROP) :            1.5 frames (of output-res)
	 *         frame * 1 Y * 1 bytes/elem + frame * 2 UV * 1/4 decimation * 1 bytes/elem
	 * ...
	 * To ensure shared_frames are sufficiently allocated for 2 largest of stage buffers,
	 * we'll allocate
	 *   buf_idx0 : 3 frames (PRE_DE, PRIM #1, PRIM #3)
	 *   buf_idx1 : 6 frames (PRIM #0, PRIM #2 ..)
	 */
	switch (buf_idx) {
		case 0:  return 3*frame;
		case 1:  return 6*frame;
		default: return 0;
	}

}