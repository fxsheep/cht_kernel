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

#ifndef _sp_map_h_
#define _sp_map_h_


#ifndef _hrt_dummy_use_blob_sp
#define _hrt_dummy_use_blob_sp()
#endif

#define _hrt_cell_load_program_sp(proc) _hrt_cell_load_program_embedded(proc, sp)

/* function input_system_acquisition_stop: AE9 */

/* function longjmp: 6A5C */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_HIVE_IF_SRST_MASK
#define HIVE_MEM_HIVE_IF_SRST_MASK scalar_processor_2400_dmem
#define HIVE_ADDR_HIVE_IF_SRST_MASK 0x1C8
#define HIVE_SIZE_HIVE_IF_SRST_MASK 16
#else
#endif
#endif
#define HIVE_MEM_sp_HIVE_IF_SRST_MASK scalar_processor_2400_dmem
#define HIVE_ADDR_sp_HIVE_IF_SRST_MASK 0x1C8
#define HIVE_SIZE_sp_HIVE_IF_SRST_MASK 16

/* function tmpmem_init_dmem: 6756 */

/* function ia_css_isys_sp_token_map_receive_ack: 609A */

/* function ia_css_dmaproxy_sp_set_addr_B: 350A */

/* function ia_css_pipe_data_init_tagger_resources: A60 */

/* function debug_buffer_set_ddr_addr: DD */

/* function receiver_port_reg_load: ACD */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_vbuf_mipi
#define HIVE_MEM_vbuf_mipi scalar_processor_2400_dmem
#define HIVE_ADDR_vbuf_mipi 0x6348
#define HIVE_SIZE_vbuf_mipi 12
#else
#endif
#endif
#define HIVE_MEM_sp_vbuf_mipi scalar_processor_2400_dmem
#define HIVE_ADDR_sp_vbuf_mipi 0x6348
#define HIVE_SIZE_sp_vbuf_mipi 12

/* function ia_css_event_sp_decode: 36FB */

/* function ia_css_queue_get_size: 4AD4 */

/* function ia_css_queue_load: 50EB */

/* function setjmp: 6A65 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_sp2host_isys_event_queue
#define HIVE_MEM_sem_for_sp2host_isys_event_queue scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_sp2host_isys_event_queue 0x46B0
#define HIVE_SIZE_sem_for_sp2host_isys_event_queue 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_sp2host_isys_event_queue scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_sp2host_isys_event_queue 0x46B0
#define HIVE_SIZE_sp_sem_for_sp2host_isys_event_queue 20

/* function ia_css_dmaproxy_sp_wait_for_ack: 6FE6 */

/* function ia_css_sp_rawcopy_func: 5310 */

/* function ia_css_tagger_buf_sp_pop_marked: 2B83 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_isp_stage
#define HIVE_MEM_isp_stage scalar_processor_2400_dmem
#define HIVE_ADDR_isp_stage 0x5C30
#define HIVE_SIZE_isp_stage 832
#else
#endif
#endif
#define HIVE_MEM_sp_isp_stage scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isp_stage 0x5C30
#define HIVE_SIZE_sp_isp_stage 832

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_vbuf_raw
#define HIVE_MEM_vbuf_raw scalar_processor_2400_dmem
#define HIVE_ADDR_vbuf_raw 0x2F4
#define HIVE_SIZE_vbuf_raw 4
#else
#endif
#endif
#define HIVE_MEM_sp_vbuf_raw scalar_processor_2400_dmem
#define HIVE_ADDR_sp_vbuf_raw 0x2F4
#define HIVE_SIZE_sp_vbuf_raw 4

/* function ia_css_sp_bin_copy_func: 5237 */

/* function ia_css_queue_item_store: 4E39 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_pipe_private_metadata_bufs
#define HIVE_MEM_ia_css_bufq_sp_pipe_private_metadata_bufs scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_pipe_private_metadata_bufs 0x4ACC
#define HIVE_SIZE_ia_css_bufq_sp_pipe_private_metadata_bufs 20
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_pipe_private_metadata_bufs scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_pipe_private_metadata_bufs 0x4ACC
#define HIVE_SIZE_sp_ia_css_bufq_sp_pipe_private_metadata_bufs 20

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_pipe_private_buffer_bufs
#define HIVE_MEM_ia_css_bufq_sp_pipe_private_buffer_bufs scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_pipe_private_buffer_bufs 0x4AE0
#define HIVE_SIZE_ia_css_bufq_sp_pipe_private_buffer_bufs 160
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_pipe_private_buffer_bufs scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_pipe_private_buffer_bufs 0x4AE0
#define HIVE_SIZE_sp_ia_css_bufq_sp_pipe_private_buffer_bufs 160

/* function sp_start_isp: 45D */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_binary_group
#define HIVE_MEM_sp_binary_group scalar_processor_2400_dmem
#define HIVE_ADDR_sp_binary_group 0x6020
#define HIVE_SIZE_sp_binary_group 32
#else
#endif
#endif
#define HIVE_MEM_sp_sp_binary_group scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_binary_group 0x6020
#define HIVE_SIZE_sp_sp_binary_group 32

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_sw_state
#define HIVE_MEM_sp_sw_state scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sw_state 0x62D8
#define HIVE_SIZE_sp_sw_state 4
#else
#endif
#endif
#define HIVE_MEM_sp_sp_sw_state scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_sw_state 0x62D8
#define HIVE_SIZE_sp_sp_sw_state 4

/* function ia_css_thread_sp_main: D66 */

/* function ia_css_ispctrl_sp_init_internal_buffers: 390F */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp2host_psys_event_queue_handle
#define HIVE_MEM_sp2host_psys_event_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp2host_psys_event_queue_handle 0x4B80
#define HIVE_SIZE_sp2host_psys_event_queue_handle 12
#else
#endif
#endif
#define HIVE_MEM_sp_sp2host_psys_event_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp2host_psys_event_queue_handle 0x4B80
#define HIVE_SIZE_sp_sp2host_psys_event_queue_handle 12

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_sp2host_psys_event_queue
#define HIVE_MEM_sem_for_sp2host_psys_event_queue scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_sp2host_psys_event_queue 0x46C4
#define HIVE_SIZE_sem_for_sp2host_psys_event_queue 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_sp2host_psys_event_queue scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_sp2host_psys_event_queue 0x46C4
#define HIVE_SIZE_sp_sem_for_sp2host_psys_event_queue 20

/* function ia_css_tagger_sp_propagate_frame: 244A */

/* function input_system_reg_load: B22 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_vbuf_handles
#define HIVE_MEM_vbuf_handles scalar_processor_2400_dmem
#define HIVE_ADDR_vbuf_handles 0x6354
#define HIVE_SIZE_vbuf_handles 960
#else
#endif
#endif
#define HIVE_MEM_sp_vbuf_handles scalar_processor_2400_dmem
#define HIVE_ADDR_sp_vbuf_handles 0x6354
#define HIVE_SIZE_sp_vbuf_handles 960

/* function ia_css_queue_store: 4F9F */

/* function ia_css_sp_flash_register: 2DB8 */

/* function ia_css_sp_rawcopy_dummy_function: 5828 */

/* function ia_css_isys_sp_backend_create: 5D0D */

/* function ia_css_pipeline_sp_init: 1857 */

/* function ia_css_tagger_sp_configure: 233A */

/* function ia_css_ispctrl_sp_end_binary: 3744 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_h_pipe_private_per_frame_ddr_ptrs
#define HIVE_MEM_ia_css_bufq_sp_h_pipe_private_per_frame_ddr_ptrs scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_h_pipe_private_per_frame_ddr_ptrs 0x4B8C
#define HIVE_SIZE_ia_css_bufq_sp_h_pipe_private_per_frame_ddr_ptrs 20
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_h_pipe_private_per_frame_ddr_ptrs scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_h_pipe_private_per_frame_ddr_ptrs 0x4B8C
#define HIVE_SIZE_sp_ia_css_bufq_sp_h_pipe_private_per_frame_ddr_ptrs 20

/* function receiver_port_reg_store: AD4 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_event_is_pending_mask
#define HIVE_MEM_event_is_pending_mask scalar_processor_2400_dmem
#define HIVE_ADDR_event_is_pending_mask 0x5C
#define HIVE_SIZE_event_is_pending_mask 44
#else
#endif
#endif
#define HIVE_MEM_sp_event_is_pending_mask scalar_processor_2400_dmem
#define HIVE_ADDR_sp_event_is_pending_mask 0x5C
#define HIVE_SIZE_sp_event_is_pending_mask 44

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_all_cb_elems_frame
#define HIVE_MEM_sp_all_cb_elems_frame scalar_processor_2400_dmem
#define HIVE_ADDR_sp_all_cb_elems_frame 0x46D8
#define HIVE_SIZE_sp_all_cb_elems_frame 16
#else
#endif
#endif
#define HIVE_MEM_sp_sp_all_cb_elems_frame scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_all_cb_elems_frame 0x46D8
#define HIVE_SIZE_sp_sp_all_cb_elems_frame 16

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp2host_isys_event_queue_handle
#define HIVE_MEM_sp2host_isys_event_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp2host_isys_event_queue_handle 0x4BA0
#define HIVE_SIZE_sp2host_isys_event_queue_handle 12
#else
#endif
#endif
#define HIVE_MEM_sp_sp2host_isys_event_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp2host_isys_event_queue_handle 0x4BA0
#define HIVE_SIZE_sp_sp2host_isys_event_queue_handle 12

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_host_sp_com
#define HIVE_MEM_host_sp_com scalar_processor_2400_dmem
#define HIVE_ADDR_host_sp_com 0x4118
#define HIVE_SIZE_host_sp_com 220
#else
#endif
#endif
#define HIVE_MEM_sp_host_sp_com scalar_processor_2400_dmem
#define HIVE_ADDR_sp_host_sp_com 0x4118
#define HIVE_SIZE_sp_host_sp_com 220

/* function ia_css_queue_get_free_space: 4BFE */

/* function exec_image_pipe: 669 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_init_dmem_data
#define HIVE_MEM_sp_init_dmem_data scalar_processor_2400_dmem
#define HIVE_ADDR_sp_init_dmem_data 0x62DC
#define HIVE_SIZE_sp_init_dmem_data 24
#else
#endif
#endif
#define HIVE_MEM_sp_sp_init_dmem_data scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_init_dmem_data 0x62DC
#define HIVE_SIZE_sp_sp_init_dmem_data 24

/* function ia_css_sp_metadata_start: 5AEA */

/* function ia_css_bufq_sp_init_buffer_queues: 2E27 */

/* function ia_css_pipeline_sp_stop: 183A */

/* function ia_css_tagger_sp_connect_pipes: 2824 */

/* function sp_isys_copy_wait: 6B2 */

/* function is_isp_debug_buffer_full: 337 */

/* function ia_css_dmaproxy_sp_configure_channel_from_info: 347A */

/* function encode_and_post_timer_event: 9D5 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_per_frame_data
#define HIVE_MEM_sp_per_frame_data scalar_processor_2400_dmem
#define HIVE_ADDR_sp_per_frame_data 0x41F4
#define HIVE_SIZE_sp_per_frame_data 4
#else
#endif
#endif
#define HIVE_MEM_sp_sp_per_frame_data scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_per_frame_data 0x41F4
#define HIVE_SIZE_sp_sp_per_frame_data 4

/* function ia_css_rmgr_sp_vbuf_dequeue: 64AA */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_host2sp_psys_event_queue_handle
#define HIVE_MEM_host2sp_psys_event_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_host2sp_psys_event_queue_handle 0x4BAC
#define HIVE_SIZE_host2sp_psys_event_queue_handle 12
#else
#endif
#endif
#define HIVE_MEM_sp_host2sp_psys_event_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp_host2sp_psys_event_queue_handle 0x4BAC
#define HIVE_SIZE_sp_host2sp_psys_event_queue_handle 12

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_xmem_bin_addr
#define HIVE_MEM_xmem_bin_addr scalar_processor_2400_dmem
#define HIVE_ADDR_xmem_bin_addr 0x41F8
#define HIVE_SIZE_xmem_bin_addr 4
#else
#endif
#endif
#define HIVE_MEM_sp_xmem_bin_addr scalar_processor_2400_dmem
#define HIVE_ADDR_sp_xmem_bin_addr 0x41F8
#define HIVE_SIZE_sp_xmem_bin_addr 4

/* function tmr_clock_init: 6776 */

/* function ia_css_pipeline_sp_run: 140E */

/* function memcpy: 6B05 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_GP_DEVICE_BASE
#define HIVE_MEM_GP_DEVICE_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_GP_DEVICE_BASE 0x2FC
#define HIVE_SIZE_GP_DEVICE_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_GP_DEVICE_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_GP_DEVICE_BASE 0x2FC
#define HIVE_SIZE_sp_GP_DEVICE_BASE 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_thread_sp_ready_queue
#define HIVE_MEM_ia_css_thread_sp_ready_queue scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_thread_sp_ready_queue 0x1E0
#define HIVE_SIZE_ia_css_thread_sp_ready_queue 12
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_thread_sp_ready_queue scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_thread_sp_ready_queue 0x1E0
#define HIVE_SIZE_sp_ia_css_thread_sp_ready_queue 12

/* function input_system_reg_store: B29 */

/* function ia_css_isys_sp_frontend_start: 5F23 */

/* function ia_css_uds_sp_scale_params: 680E */

/* function ia_css_circbuf_increase_size: E4B */

/* function __divu: 6A83 */

/* function ia_css_thread_sp_get_state: C8E */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_cont_capt_stop
#define HIVE_MEM_sem_for_cont_capt_stop scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_cont_capt_stop 0x46E8
#define HIVE_SIZE_sem_for_cont_capt_stop 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_cont_capt_stop scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_cont_capt_stop 0x46E8
#define HIVE_SIZE_sp_sem_for_cont_capt_stop 20

/* function thread_fiber_sp_main: E44 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_isp_pipe_thread
#define HIVE_MEM_sp_isp_pipe_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isp_pipe_thread 0x482C
#define HIVE_SIZE_sp_isp_pipe_thread 340
#else
#endif
#endif
#define HIVE_MEM_sp_sp_isp_pipe_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_isp_pipe_thread 0x482C
#define HIVE_SIZE_sp_sp_isp_pipe_thread 340

/* function ia_css_parambuf_sp_handle_parameter_sets: 1295 */

/* function ia_css_spctrl_sp_set_state: 5B19 */

/* function ia_css_thread_sem_sp_signal: 6D05 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_IRQ_BASE
#define HIVE_MEM_IRQ_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_IRQ_BASE 0x2C
#define HIVE_SIZE_IRQ_BASE 16
#else
#endif
#endif
#define HIVE_MEM_sp_IRQ_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_IRQ_BASE 0x2C
#define HIVE_SIZE_sp_IRQ_BASE 16

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_TIMED_CTRL_BASE
#define HIVE_MEM_TIMED_CTRL_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_TIMED_CTRL_BASE 0x40
#define HIVE_SIZE_TIMED_CTRL_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_TIMED_CTRL_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_TIMED_CTRL_BASE 0x40
#define HIVE_SIZE_sp_TIMED_CTRL_BASE 4

/* function ia_css_isys_sp_isr: 71D4 */

/* function ia_css_isys_sp_generate_exp_id: 62BB */

/* function ia_css_rmgr_sp_init: 63A5 */

/* function ia_css_thread_sem_sp_init: 6DD6 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_reading_cb_frame
#define HIVE_MEM_sem_for_reading_cb_frame scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_reading_cb_frame 0x46FC
#define HIVE_SIZE_sem_for_reading_cb_frame 40
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_reading_cb_frame scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_reading_cb_frame 0x46FC
#define HIVE_SIZE_sp_sem_for_reading_cb_frame 40

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_is_isp_requested
#define HIVE_MEM_is_isp_requested scalar_processor_2400_dmem
#define HIVE_ADDR_is_isp_requested 0x308
#define HIVE_SIZE_is_isp_requested 4
#else
#endif
#endif
#define HIVE_MEM_sp_is_isp_requested scalar_processor_2400_dmem
#define HIVE_ADDR_sp_is_isp_requested 0x308
#define HIVE_SIZE_sp_is_isp_requested 4

/* function ia_css_dmaproxy_sp_execute: 33E0 */

/* function ia_css_queue_is_empty: 7133 */

/* function ia_css_pipeline_sp_has_stopped: 1830 */

/* function ia_css_circbuf_extract: F4F */

/* function ia_css_tagger_buf_sp_is_locked_from_start: 2C99 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_current_sp_thread
#define HIVE_MEM_current_sp_thread scalar_processor_2400_dmem
#define HIVE_ADDR_current_sp_thread 0x1DC
#define HIVE_SIZE_current_sp_thread 4
#else
#endif
#endif
#define HIVE_MEM_sp_current_sp_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_current_sp_thread 0x1DC
#define HIVE_SIZE_sp_current_sp_thread 4

/* function ia_css_spctrl_sp_get_spid: 5B20 */

/* function ia_css_bufq_sp_reset_buffers: 2EAE */

/* function ia_css_dmaproxy_sp_read_byte_addr: 7014 */

/* function ia_css_rmgr_sp_uninit: 639E */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_threads_stack
#define HIVE_MEM_sp_threads_stack scalar_processor_2400_dmem
#define HIVE_ADDR_sp_threads_stack 0x164
#define HIVE_SIZE_sp_threads_stack 28
#else
#endif
#endif
#define HIVE_MEM_sp_sp_threads_stack scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_threads_stack 0x164
#define HIVE_SIZE_sp_sp_threads_stack 28

/* function ia_css_circbuf_peek: F31 */

/* function ia_css_parambuf_sp_wait_for_in_param: 105E */

/* function ia_css_isys_sp_token_map_get_exp_id: 6183 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_all_cb_elems_param
#define HIVE_MEM_sp_all_cb_elems_param scalar_processor_2400_dmem
#define HIVE_ADDR_sp_all_cb_elems_param 0x4724
#define HIVE_SIZE_sp_all_cb_elems_param 16
#else
#endif
#endif
#define HIVE_MEM_sp_sp_all_cb_elems_param scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_all_cb_elems_param 0x4724
#define HIVE_SIZE_sp_sp_all_cb_elems_param 16

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_pipeline_sp_curr_binary_id
#define HIVE_MEM_pipeline_sp_curr_binary_id scalar_processor_2400_dmem
#define HIVE_ADDR_pipeline_sp_curr_binary_id 0x1EC
#define HIVE_SIZE_pipeline_sp_curr_binary_id 4
#else
#endif
#endif
#define HIVE_MEM_sp_pipeline_sp_curr_binary_id scalar_processor_2400_dmem
#define HIVE_ADDR_sp_pipeline_sp_curr_binary_id 0x1EC
#define HIVE_SIZE_sp_pipeline_sp_curr_binary_id 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_all_cbs_frame_desc
#define HIVE_MEM_sp_all_cbs_frame_desc scalar_processor_2400_dmem
#define HIVE_ADDR_sp_all_cbs_frame_desc 0x4734
#define HIVE_SIZE_sp_all_cbs_frame_desc 8
#else
#endif
#endif
#define HIVE_MEM_sp_sp_all_cbs_frame_desc scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_all_cbs_frame_desc 0x4734
#define HIVE_SIZE_sp_sp_all_cbs_frame_desc 8

/* function sp_isys_copy_func_v2: 6AB */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_reading_cb_param
#define HIVE_MEM_sem_for_reading_cb_param scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_reading_cb_param 0x473C
#define HIVE_SIZE_sem_for_reading_cb_param 40
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_reading_cb_param scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_reading_cb_param 0x473C
#define HIVE_SIZE_sp_sem_for_reading_cb_param 40

/* function ia_css_queue_get_used_space: 4BB2 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_cont_capt_start
#define HIVE_MEM_sem_for_cont_capt_start scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_cont_capt_start 0x4764
#define HIVE_SIZE_sem_for_cont_capt_start 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_cont_capt_start scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_cont_capt_start 0x4764
#define HIVE_SIZE_sp_sem_for_cont_capt_start 20

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_tmp_heap
#define HIVE_MEM_tmp_heap scalar_processor_2400_dmem
#define HIVE_ADDR_tmp_heap 0x6040
#define HIVE_SIZE_tmp_heap 640
#else
#endif
#endif
#define HIVE_MEM_sp_tmp_heap scalar_processor_2400_dmem
#define HIVE_ADDR_sp_tmp_heap 0x6040
#define HIVE_SIZE_sp_tmp_heap 640

/* function ia_css_rmgr_sp_get_num_vbuf: 66AE */

/* function ia_css_ispctrl_sp_output_compute_dma_info: 4133 */

/* function ia_css_tagger_sp_lock_exp_id: 2107 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_pipe_private_s3a_bufs
#define HIVE_MEM_ia_css_bufq_sp_pipe_private_s3a_bufs scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_pipe_private_s3a_bufs 0x4BB8
#define HIVE_SIZE_ia_css_bufq_sp_pipe_private_s3a_bufs 60
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_pipe_private_s3a_bufs scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_pipe_private_s3a_bufs 0x4BB8
#define HIVE_SIZE_sp_ia_css_bufq_sp_pipe_private_s3a_bufs 60

/* function ia_css_queue_is_full: 4C49 */

/* function debug_buffer_init_isp: E4 */

/* function ia_css_isys_sp_frontend_uninit: 5EDD */

/* function ia_css_tagger_sp_exp_id_is_locked: 203D */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_rmgr_sp_mipi_frame_sem
#define HIVE_MEM_ia_css_rmgr_sp_mipi_frame_sem scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_rmgr_sp_mipi_frame_sem 0x6714
#define HIVE_SIZE_ia_css_rmgr_sp_mipi_frame_sem 60
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_rmgr_sp_mipi_frame_sem scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_rmgr_sp_mipi_frame_sem 0x6714
#define HIVE_SIZE_sp_ia_css_rmgr_sp_mipi_frame_sem 60

/* function ia_css_rmgr_sp_refcount_dump: 6485 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_pipe_private_isp_parameters_id
#define HIVE_MEM_ia_css_bufq_sp_pipe_private_isp_parameters_id scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_pipe_private_isp_parameters_id 0x4BF4
#define HIVE_SIZE_ia_css_bufq_sp_pipe_private_isp_parameters_id 20
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_pipe_private_isp_parameters_id scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_pipe_private_isp_parameters_id 0x4BF4
#define HIVE_SIZE_sp_ia_css_bufq_sp_pipe_private_isp_parameters_id 20

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_pipe_threads
#define HIVE_MEM_sp_pipe_threads scalar_processor_2400_dmem
#define HIVE_ADDR_sp_pipe_threads 0x150
#define HIVE_SIZE_sp_pipe_threads 20
#else
#endif
#endif
#define HIVE_MEM_sp_sp_pipe_threads scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_pipe_threads 0x150
#define HIVE_SIZE_sp_sp_pipe_threads 20

/* function sp_event_proxy_func: 6C0 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_host2sp_isys_event_queue_handle
#define HIVE_MEM_host2sp_isys_event_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_host2sp_isys_event_queue_handle 0x4C08
#define HIVE_SIZE_host2sp_isys_event_queue_handle 12
#else
#endif
#endif
#define HIVE_MEM_sp_host2sp_isys_event_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp_host2sp_isys_event_queue_handle 0x4C08
#define HIVE_SIZE_sp_host2sp_isys_event_queue_handle 12

/* function ia_css_thread_sp_yield: 6C7E */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_all_cbs_param_desc
#define HIVE_MEM_sp_all_cbs_param_desc scalar_processor_2400_dmem
#define HIVE_ADDR_sp_all_cbs_param_desc 0x4778
#define HIVE_SIZE_sp_all_cbs_param_desc 8
#else
#endif
#endif
#define HIVE_MEM_sp_sp_all_cbs_param_desc scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_all_cbs_param_desc 0x4778
#define HIVE_SIZE_sp_sp_all_cbs_param_desc 8

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_dmaproxy_sp_invalidate_tlb
#define HIVE_MEM_ia_css_dmaproxy_sp_invalidate_tlb scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_dmaproxy_sp_invalidate_tlb 0x5C20
#define HIVE_SIZE_ia_css_dmaproxy_sp_invalidate_tlb 4
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_dmaproxy_sp_invalidate_tlb scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_dmaproxy_sp_invalidate_tlb 0x5C20
#define HIVE_SIZE_sp_ia_css_dmaproxy_sp_invalidate_tlb 4

/* function ia_css_thread_sp_fork: D1B */

/* function ia_css_tagger_sp_destroy: 282E */

/* function ia_css_dmaproxy_sp_vmem_read: 3380 */

/* function ia_css_ifmtr_sp_init: 630C */

/* function initialize_sp_group: 679 */

/* function ia_css_tagger_buf_sp_peek: 2AA5 */

/* function ia_css_thread_sp_init: D47 */

/* function ia_css_isys_sp_reset_exp_id: 62B3 */

/* function qos_scheduler_update_fps: 67FE */

/* function ia_css_ispctrl_sp_set_stream_base_addr: 4820 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ISP_DMEM_BASE
#define HIVE_MEM_ISP_DMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_ISP_DMEM_BASE 0x10
#define HIVE_SIZE_ISP_DMEM_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_ISP_DMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ISP_DMEM_BASE 0x10
#define HIVE_SIZE_sp_ISP_DMEM_BASE 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_SP_DMEM_BASE
#define HIVE_MEM_SP_DMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_SP_DMEM_BASE 0x4
#define HIVE_SIZE_SP_DMEM_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_SP_DMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_SP_DMEM_BASE 0x4
#define HIVE_SIZE_sp_SP_DMEM_BASE 4

/* function __ia_css_queue_is_empty_text: 4B0F */

/* function ia_css_dmaproxy_sp_read: 33F6 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_raw_copy_line_count
#define HIVE_MEM_raw_copy_line_count scalar_processor_2400_dmem
#define HIVE_ADDR_raw_copy_line_count 0x2C8
#define HIVE_SIZE_raw_copy_line_count 4
#else
#endif
#endif
#define HIVE_MEM_sp_raw_copy_line_count scalar_processor_2400_dmem
#define HIVE_ADDR_sp_raw_copy_line_count 0x2C8
#define HIVE_SIZE_sp_raw_copy_line_count 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_host2sp_tag_cmd_queue_handle
#define HIVE_MEM_host2sp_tag_cmd_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_host2sp_tag_cmd_queue_handle 0x4C14
#define HIVE_SIZE_host2sp_tag_cmd_queue_handle 12
#else
#endif
#endif
#define HIVE_MEM_sp_host2sp_tag_cmd_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp_host2sp_tag_cmd_queue_handle 0x4C14
#define HIVE_SIZE_sp_host2sp_tag_cmd_queue_handle 12

/* function ia_css_queue_peek: 4B28 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_flash_sp_frame_cnt
#define HIVE_MEM_ia_css_flash_sp_frame_cnt scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_flash_sp_frame_cnt 0x4AC0
#define HIVE_SIZE_ia_css_flash_sp_frame_cnt 4
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_flash_sp_frame_cnt scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_flash_sp_frame_cnt 0x4AC0
#define HIVE_SIZE_sp_ia_css_flash_sp_frame_cnt 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_event_can_send_token_mask
#define HIVE_MEM_event_can_send_token_mask scalar_processor_2400_dmem
#define HIVE_ADDR_event_can_send_token_mask 0x88
#define HIVE_SIZE_event_can_send_token_mask 44
#else
#endif
#endif
#define HIVE_MEM_sp_event_can_send_token_mask scalar_processor_2400_dmem
#define HIVE_ADDR_sp_event_can_send_token_mask 0x88
#define HIVE_SIZE_sp_event_can_send_token_mask 44

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_isp_thread
#define HIVE_MEM_isp_thread scalar_processor_2400_dmem
#define HIVE_ADDR_isp_thread 0x5F70
#define HIVE_SIZE_isp_thread 4
#else
#endif
#endif
#define HIVE_MEM_sp_isp_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isp_thread 0x5F70
#define HIVE_SIZE_sp_isp_thread 4

/* function encode_and_post_sp_event_non_blocking: A1D */

/* function ia_css_isys_sp_frontend_destroy: 5FB5 */

/* function is_ddr_debug_buffer_full: 2CC */

/* function ia_css_isys_sp_frontend_stop: 5EF5 */

/* function ia_css_isys_sp_token_map_init: 6251 */

/* function ia_css_tagger_buf_sp_get_oldest_marked_offset: 2AF5 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_threads_fiber
#define HIVE_MEM_sp_threads_fiber scalar_processor_2400_dmem
#define HIVE_ADDR_sp_threads_fiber 0x19C
#define HIVE_SIZE_sp_threads_fiber 28
#else
#endif
#endif
#define HIVE_MEM_sp_sp_threads_fiber scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_threads_fiber 0x19C
#define HIVE_SIZE_sp_sp_threads_fiber 28

/* function encode_and_post_sp_event: 9A6 */

/* function debug_enqueue_ddr: EE */

/* function ia_css_rmgr_sp_refcount_init_vbuf: 6440 */

/* function dmaproxy_sp_read_write: 70B2 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_dmaproxy_isp_dma_cmd_buffer
#define HIVE_MEM_ia_css_dmaproxy_isp_dma_cmd_buffer scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_dmaproxy_isp_dma_cmd_buffer 0x5C24
#define HIVE_SIZE_ia_css_dmaproxy_isp_dma_cmd_buffer 4
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_dmaproxy_isp_dma_cmd_buffer scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_dmaproxy_isp_dma_cmd_buffer 0x5C24
#define HIVE_SIZE_sp_ia_css_dmaproxy_isp_dma_cmd_buffer 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_host2sp_buffer_queue_handle
#define HIVE_MEM_host2sp_buffer_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_host2sp_buffer_queue_handle 0x4C20
#define HIVE_SIZE_host2sp_buffer_queue_handle 480
#else
#endif
#endif
#define HIVE_MEM_sp_host2sp_buffer_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp_host2sp_buffer_queue_handle 0x4C20
#define HIVE_SIZE_sp_host2sp_buffer_queue_handle 480

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_flash_sp_in_service
#define HIVE_MEM_ia_css_flash_sp_in_service scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_flash_sp_in_service 0x317C
#define HIVE_SIZE_ia_css_flash_sp_in_service 4
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_flash_sp_in_service scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_flash_sp_in_service 0x317C
#define HIVE_SIZE_sp_ia_css_flash_sp_in_service 4

/* function ia_css_dmaproxy_sp_process: 6DFE */

/* function ia_css_tagger_buf_sp_mark_from_end: 2D7D */

/* function ia_css_isys_sp_backend_rcv_acquire_ack: 5BC2 */

/* function ia_css_isys_sp_backend_pre_acquire_request: 5BD8 */

/* function ia_css_ispctrl_sp_init_cs: 383F */

/* function ia_css_spctrl_sp_init: 5B2E */

/* function sp_event_proxy_init: 6D5 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_pipe_private_previous_clock_tick
#define HIVE_MEM_ia_css_bufq_sp_pipe_private_previous_clock_tick scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_pipe_private_previous_clock_tick 0x4E00
#define HIVE_SIZE_ia_css_bufq_sp_pipe_private_previous_clock_tick 40
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_pipe_private_previous_clock_tick scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_pipe_private_previous_clock_tick 0x4E00
#define HIVE_SIZE_sp_ia_css_bufq_sp_pipe_private_previous_clock_tick 40

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_output
#define HIVE_MEM_sp_output scalar_processor_2400_dmem
#define HIVE_ADDR_sp_output 0x41FC
#define HIVE_SIZE_sp_output 16
#else
#endif
#endif
#define HIVE_MEM_sp_sp_output scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_output 0x41FC
#define HIVE_SIZE_sp_sp_output 16

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_sems_for_host2sp_buf_queues
#define HIVE_MEM_ia_css_bufq_sp_sems_for_host2sp_buf_queues scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_sems_for_host2sp_buf_queues 0x4E28
#define HIVE_SIZE_ia_css_bufq_sp_sems_for_host2sp_buf_queues 800
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_sems_for_host2sp_buf_queues scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_sems_for_host2sp_buf_queues 0x4E28
#define HIVE_SIZE_sp_ia_css_bufq_sp_sems_for_host2sp_buf_queues 800

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ISP_CTRL_BASE
#define HIVE_MEM_ISP_CTRL_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_ISP_CTRL_BASE 0x8
#define HIVE_SIZE_ISP_CTRL_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_ISP_CTRL_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ISP_CTRL_BASE 0x8
#define HIVE_SIZE_sp_ISP_CTRL_BASE 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_INPUT_FORMATTER_BASE
#define HIVE_MEM_INPUT_FORMATTER_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_INPUT_FORMATTER_BASE 0x4C
#define HIVE_SIZE_INPUT_FORMATTER_BASE 16
#else
#endif
#endif
#define HIVE_MEM_sp_INPUT_FORMATTER_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_INPUT_FORMATTER_BASE 0x4C
#define HIVE_SIZE_sp_INPUT_FORMATTER_BASE 16

/* function sp_dma_proxy_reset_channels: 3665 */

/* function ia_css_isys_sp_backend_acquire: 5CE3 */

/* function ia_css_tagger_sp_update_size: 2A74 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_host_sp_queue
#define HIVE_MEM_ia_css_bufq_host_sp_queue scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_host_sp_queue 0x5148
#define HIVE_SIZE_ia_css_bufq_host_sp_queue 2008
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_host_sp_queue scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_host_sp_queue 0x5148
#define HIVE_SIZE_sp_ia_css_bufq_host_sp_queue 2008

/* function thread_fiber_sp_create: DB3 */

/* function ia_css_dmaproxy_sp_set_increments: 34F7 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_writing_cb_frame
#define HIVE_MEM_sem_for_writing_cb_frame scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_writing_cb_frame 0x4780
#define HIVE_SIZE_sem_for_writing_cb_frame 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_writing_cb_frame scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_writing_cb_frame 0x4780
#define HIVE_SIZE_sp_sem_for_writing_cb_frame 20

/* function receiver_reg_store: AE2 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_writing_cb_param
#define HIVE_MEM_sem_for_writing_cb_param scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_writing_cb_param 0x4794
#define HIVE_SIZE_sem_for_writing_cb_param 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_writing_cb_param scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_writing_cb_param 0x4794
#define HIVE_SIZE_sp_sem_for_writing_cb_param 20

/* function sp_start_isp_entry: 453 */
#ifndef HIVE_MULTIPLE_PROGRAMS
#ifdef HIVE_ADDR_sp_start_isp_entry
#endif
#define HIVE_ADDR_sp_start_isp_entry 0x453
#endif
#define HIVE_ADDR_sp_sp_start_isp_entry 0x453

/* function ia_css_tagger_buf_sp_unmark_all: 2D01 */

/* function ia_css_tagger_buf_sp_unmark_from_start: 2D42 */

/* function ia_css_dmaproxy_sp_channel_acquire: 3691 */

/* function ia_css_rmgr_sp_add_num_vbuf: 668A */

/* function ia_css_isys_sp_token_map_create: 629A */

/* function __ia_css_dmaproxy_sp_wait_for_ack_text: 334C */

/* function ia_css_tagger_sp_acquire_buf_elem: 2015 */

/* function ia_css_bufq_sp_is_dynamic_buffer: 31F8 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_group
#define HIVE_MEM_sp_group scalar_processor_2400_dmem
#define HIVE_ADDR_sp_group 0x420C
#define HIVE_SIZE_sp_group 1184
#else
#endif
#endif
#define HIVE_MEM_sp_sp_group scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_group 0x420C
#define HIVE_SIZE_sp_sp_group 1184

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_event_proxy_thread
#define HIVE_MEM_sp_event_proxy_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_event_proxy_thread 0x4980
#define HIVE_SIZE_sp_event_proxy_thread 68
#else
#endif
#endif
#define HIVE_MEM_sp_sp_event_proxy_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_event_proxy_thread 0x4980
#define HIVE_SIZE_sp_sp_event_proxy_thread 68

/* function ia_css_thread_sp_kill: CE1 */

/* function ia_css_tagger_sp_create: 2A22 */

/* function tmpmem_acquire_dmem: 6737 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_MMU_BASE
#define HIVE_MEM_MMU_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_MMU_BASE 0x24
#define HIVE_SIZE_MMU_BASE 8
#else
#endif
#endif
#define HIVE_MEM_sp_MMU_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_MMU_BASE 0x24
#define HIVE_SIZE_sp_MMU_BASE 8

/* function ia_css_dmaproxy_sp_channel_release: 367D */

/* function ia_css_dmaproxy_sp_is_idle: 365D */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_qos_start
#define HIVE_MEM_sem_for_qos_start scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_qos_start 0x47A8
#define HIVE_SIZE_sem_for_qos_start 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_qos_start scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_qos_start 0x47A8
#define HIVE_SIZE_sp_sem_for_qos_start 20

/* function isp_hmem_load: B60 */

/* function ia_css_tagger_sp_release_buf_elem: 1FF1 */

/* function ia_css_eventq_sp_send: 36D3 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_isys_sp_error_cnt
#define HIVE_MEM_ia_css_isys_sp_error_cnt scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_isys_sp_error_cnt 0x6300
#define HIVE_SIZE_ia_css_isys_sp_error_cnt 16
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_isys_sp_error_cnt scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_isys_sp_error_cnt 0x6300
#define HIVE_SIZE_sp_ia_css_isys_sp_error_cnt 16

/* function ia_css_tagger_buf_sp_unlock_from_start: 2C31 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_debug_buffer_ddr_address
#define HIVE_MEM_debug_buffer_ddr_address scalar_processor_2400_dmem
#define HIVE_ADDR_debug_buffer_ddr_address 0xBC
#define HIVE_SIZE_debug_buffer_ddr_address 4
#else
#endif
#endif
#define HIVE_MEM_sp_debug_buffer_ddr_address scalar_processor_2400_dmem
#define HIVE_ADDR_sp_debug_buffer_ddr_address 0xBC
#define HIVE_SIZE_sp_debug_buffer_ddr_address 4

/* function sp_isys_copy_request: 6B9 */

/* function ia_css_rmgr_sp_refcount_retain_vbuf: 651A */

/* function ia_css_thread_sp_set_priority: CD9 */

/* function sizeof_hmem: C07 */

/* function tmpmem_release_dmem: 6726 */

/* function cnd_input_system_cfg: 392 */

/* function __ia_css_sp_rawcopy_func_critical: 715D */

/* function __ia_css_dmaproxy_sp_process_text: 32F0 */

/* function ia_css_dmaproxy_sp_set_width_exception: 34E2 */

/* function sp_event_assert: 856 */

/* function ia_css_flash_sp_init_internal_params: 2E1C */

/* function ia_css_tagger_buf_sp_pop_unmarked_and_unlocked: 2B37 */

/* function __modu: 6AC9 */

/* function ia_css_dmaproxy_sp_init_isp_vector: 3352 */

/* function isp_vamem_store: 0 */

/* function ia_css_tagger_sp_set_copy_pipe: 2A19 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_GDC_BASE
#define HIVE_MEM_GDC_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_GDC_BASE 0x44
#define HIVE_SIZE_GDC_BASE 8
#else
#endif
#endif
#define HIVE_MEM_sp_GDC_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_GDC_BASE 0x44
#define HIVE_SIZE_sp_GDC_BASE 8

/* function ia_css_queue_local_init: 4E13 */

/* function sp_event_proxy_callout_func: 6B96 */

/* function qos_scheduler_schedule_stage: 67AA */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_thread_sp_num_ready_threads
#define HIVE_MEM_ia_css_thread_sp_num_ready_threads scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_thread_sp_num_ready_threads 0x4A0C
#define HIVE_SIZE_ia_css_thread_sp_num_ready_threads 4
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_thread_sp_num_ready_threads scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_thread_sp_num_ready_threads 0x4A0C
#define HIVE_SIZE_sp_ia_css_thread_sp_num_ready_threads 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_threads_stack_size
#define HIVE_MEM_sp_threads_stack_size scalar_processor_2400_dmem
#define HIVE_ADDR_sp_threads_stack_size 0x180
#define HIVE_SIZE_sp_threads_stack_size 28
#else
#endif
#endif
#define HIVE_MEM_sp_sp_threads_stack_size scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_threads_stack_size 0x180
#define HIVE_SIZE_sp_sp_threads_stack_size 28

/* function ia_css_ispctrl_sp_isp_done_row_striping: 4119 */

/* function __ia_css_isys_sp_isr_text: 5FDF */

/* function ia_css_queue_dequeue: 4C91 */

/* function is_qos_standalone_mode: 6785 */

/* function ia_css_dmaproxy_sp_configure_channel: 702B */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_current_thread_fiber_sp
#define HIVE_MEM_current_thread_fiber_sp scalar_processor_2400_dmem
#define HIVE_ADDR_current_thread_fiber_sp 0x4A14
#define HIVE_SIZE_current_thread_fiber_sp 4
#else
#endif
#endif
#define HIVE_MEM_sp_current_thread_fiber_sp scalar_processor_2400_dmem
#define HIVE_ADDR_sp_current_thread_fiber_sp 0x4A14
#define HIVE_SIZE_sp_current_thread_fiber_sp 4

/* function ia_css_circbuf_pop: FE3 */

/* function memset: 6B48 */

/* function irq_raise_set_token: B6 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_GPIO_BASE
#define HIVE_MEM_GPIO_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_GPIO_BASE 0x3C
#define HIVE_SIZE_GPIO_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_GPIO_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_GPIO_BASE 0x3C
#define HIVE_SIZE_sp_GPIO_BASE 4

/* function ia_css_pipeline_acc_stage_enable: 17E9 */

/* function ia_css_tagger_sp_unlock_exp_id: 2062 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_isp_ph
#define HIVE_MEM_isp_ph scalar_processor_2400_dmem
#define HIVE_ADDR_isp_ph 0x6310
#define HIVE_SIZE_isp_ph 28
#else
#endif
#endif
#define HIVE_MEM_sp_isp_ph scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isp_ph 0x6310
#define HIVE_SIZE_sp_isp_ph 28

/* function ia_css_isys_sp_token_map_flush: 61DF */

/* function ia_css_ispctrl_sp_init_ds: 399E */

/* function get_xmem_base_addr_raw: 3D49 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_all_cbs_param
#define HIVE_MEM_sp_all_cbs_param scalar_processor_2400_dmem
#define HIVE_ADDR_sp_all_cbs_param 0x47BC
#define HIVE_SIZE_sp_all_cbs_param 16
#else
#endif
#endif
#define HIVE_MEM_sp_sp_all_cbs_param scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_all_cbs_param 0x47BC
#define HIVE_SIZE_sp_sp_all_cbs_param 16

/* function ia_css_circbuf_create: 1031 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_sp_group
#define HIVE_MEM_sem_for_sp_group scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_sp_group 0x47CC
#define HIVE_SIZE_sem_for_sp_group 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_sp_group scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_sp_group 0x47CC
#define HIVE_SIZE_sp_sem_for_sp_group 20

/* function __ia_css_dmaproxy_sp_configure_channel_text: 34C1 */

/* function ia_css_framebuf_sp_wait_for_in_frame: 66B5 */

/* function ia_css_sp_rawcopy_tag_frame: 5757 */

/* function isp_hmem_clear: B30 */

/* function ia_css_framebuf_sp_release_in_frame: 66F8 */

/* function ia_css_isys_sp_backend_snd_acquire_request: 5C35 */

/* function ia_css_isys_sp_token_map_is_full: 6066 */

/* function input_system_acquisition_run: B04 */

/* function ia_css_ispctrl_sp_start_binary: 381D */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_h_pipe_private_ddr_ptrs
#define HIVE_MEM_ia_css_bufq_sp_h_pipe_private_ddr_ptrs scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_h_pipe_private_ddr_ptrs 0x5920
#define HIVE_SIZE_ia_css_bufq_sp_h_pipe_private_ddr_ptrs 20
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_h_pipe_private_ddr_ptrs scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_h_pipe_private_ddr_ptrs 0x5920
#define HIVE_SIZE_sp_ia_css_bufq_sp_h_pipe_private_ddr_ptrs 20

/* function ia_css_eventq_sp_recv: 36A5 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_isp_pool
#define HIVE_MEM_isp_pool scalar_processor_2400_dmem
#define HIVE_ADDR_isp_pool 0x2E8
#define HIVE_SIZE_isp_pool 4
#else
#endif
#endif
#define HIVE_MEM_sp_isp_pool scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isp_pool 0x2E8
#define HIVE_SIZE_sp_isp_pool 4

/* function ia_css_rmgr_sp_rel_gen: 63E7 */

/* function ia_css_tagger_sp_unblock_clients: 28EA */

/* function css_get_frame_processing_time_end: 1FE1 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_event_any_pending_mask
#define HIVE_MEM_event_any_pending_mask scalar_processor_2400_dmem
#define HIVE_ADDR_event_any_pending_mask 0x300
#define HIVE_SIZE_event_any_pending_mask 8
#else
#endif
#endif
#define HIVE_MEM_sp_event_any_pending_mask scalar_processor_2400_dmem
#define HIVE_ADDR_sp_event_any_pending_mask 0x300
#define HIVE_SIZE_sp_event_any_pending_mask 8

/* function ia_css_isys_sp_backend_push: 5BEC */

/* function sh_css_decode_tag_descr: 352 */

/* function debug_enqueue_isp: 27B */

/* function qos_scheduler_update_stage_budget: 678D */

/* function ia_css_spctrl_sp_uninit: 5B27 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_HIVE_IF_SWITCH_CODE
#define HIVE_MEM_HIVE_IF_SWITCH_CODE scalar_processor_2400_dmem
#define HIVE_ADDR_HIVE_IF_SWITCH_CODE 0x1D8
#define HIVE_SIZE_HIVE_IF_SWITCH_CODE 4
#else
#endif
#endif
#define HIVE_MEM_sp_HIVE_IF_SWITCH_CODE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_HIVE_IF_SWITCH_CODE 0x1D8
#define HIVE_SIZE_sp_HIVE_IF_SWITCH_CODE 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_pipe_private_dis_bufs
#define HIVE_MEM_ia_css_bufq_sp_pipe_private_dis_bufs scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_pipe_private_dis_bufs 0x5934
#define HIVE_SIZE_ia_css_bufq_sp_pipe_private_dis_bufs 140
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_pipe_private_dis_bufs scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_pipe_private_dis_bufs 0x5934
#define HIVE_SIZE_sp_ia_css_bufq_sp_pipe_private_dis_bufs 140

/* function ia_css_tagger_buf_sp_lock_from_start: 2C65 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_isp_idle
#define HIVE_MEM_sem_for_isp_idle scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_isp_idle 0x47E0
#define HIVE_SIZE_sem_for_isp_idle 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_isp_idle scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_isp_idle 0x47E0
#define HIVE_SIZE_sp_sem_for_isp_idle 20

/* function ia_css_dmaproxy_sp_write_byte_addr: 33AF */

/* function ia_css_dmaproxy_sp_init: 3326 */

/* function ia_css_bufq_sp_release_dynamic_buf_clock_tick: 2EEE */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ISP_VAMEM_BASE
#define HIVE_MEM_ISP_VAMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_ISP_VAMEM_BASE 0x14
#define HIVE_SIZE_ISP_VAMEM_BASE 12
#else
#endif
#endif
#define HIVE_MEM_sp_ISP_VAMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ISP_VAMEM_BASE 0x14
#define HIVE_SIZE_sp_ISP_VAMEM_BASE 12

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_rawcopy_sp_tagger
#define HIVE_MEM_ia_css_rawcopy_sp_tagger scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_rawcopy_sp_tagger 0x62C0
#define HIVE_SIZE_ia_css_rawcopy_sp_tagger 24
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_rawcopy_sp_tagger scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_rawcopy_sp_tagger 0x62C0
#define HIVE_SIZE_sp_ia_css_rawcopy_sp_tagger 24

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_pipe_private_exp_ids
#define HIVE_MEM_ia_css_bufq_sp_pipe_private_exp_ids scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_pipe_private_exp_ids 0x59C0
#define HIVE_SIZE_ia_css_bufq_sp_pipe_private_exp_ids 70
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_pipe_private_exp_ids scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_pipe_private_exp_ids 0x59C0
#define HIVE_SIZE_sp_ia_css_bufq_sp_pipe_private_exp_ids 70

/* function ia_css_queue_item_load: 4F05 */

/* function ia_css_spctrl_sp_get_state: 5B12 */

/* function ia_css_isys_sp_token_map_uninit: 61FC */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_callout_sp_thread
#define HIVE_MEM_callout_sp_thread scalar_processor_2400_dmem
#define HIVE_ADDR_callout_sp_thread 0x4A08
#define HIVE_SIZE_callout_sp_thread 4
#else
#endif
#endif
#define HIVE_MEM_sp_callout_sp_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_callout_sp_thread 0x4A08
#define HIVE_SIZE_sp_callout_sp_thread 4

/* function thread_fiber_sp_init: E3A */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_SP_PMEM_BASE
#define HIVE_MEM_SP_PMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_SP_PMEM_BASE 0x0
#define HIVE_SIZE_SP_PMEM_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_SP_PMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_SP_PMEM_BASE 0x0
#define HIVE_SIZE_sp_SP_PMEM_BASE 4

/* function ia_css_isys_sp_token_map_snd_acquire_req: 616C */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_isp_input_stream_format
#define HIVE_MEM_sp_isp_input_stream_format scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isp_input_stream_format 0x40FC
#define HIVE_SIZE_sp_isp_input_stream_format 20
#else
#endif
#endif
#define HIVE_MEM_sp_sp_isp_input_stream_format scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_isp_input_stream_format 0x40FC
#define HIVE_SIZE_sp_sp_isp_input_stream_format 20

/* function __mod: 6AB5 */

/* function ia_css_dmaproxy_sp_init_dmem_channel: 3410 */

/* function ia_css_thread_sp_join: D0A */

/* function ia_css_dmaproxy_sp_add_command: 711D */

/* function ia_css_sp_metadata_thread_func: 59C6 */

/* function __sp_event_proxy_func_critical: 6B83 */

/* function ia_css_sp_metadata_wait: 5AD9 */

/* function ia_css_circbuf_peek_from_start: F13 */

/* function ia_css_event_sp_encode: 3730 */

/* function ia_css_thread_sp_run: D7D */

/* function sp_isys_copy_func: 69B */

/* function ia_css_isys_sp_backend_flush: 5C55 */

/* function ia_css_isys_sp_backend_frame_exists: 5B71 */

/* function ia_css_sp_isp_param_init_isp_memories: 49B8 */

/* function register_isr: 84E */

/* function irq_raise: C8 */

/* function ia_css_dmaproxy_sp_mmu_invalidate: 32B6 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_HIVE_IF_SRST_ADDRESS
#define HIVE_MEM_HIVE_IF_SRST_ADDRESS scalar_processor_2400_dmem
#define HIVE_ADDR_HIVE_IF_SRST_ADDRESS 0x1B8
#define HIVE_SIZE_HIVE_IF_SRST_ADDRESS 16
#else
#endif
#endif
#define HIVE_MEM_sp_HIVE_IF_SRST_ADDRESS scalar_processor_2400_dmem
#define HIVE_ADDR_sp_HIVE_IF_SRST_ADDRESS 0x1B8
#define HIVE_SIZE_sp_HIVE_IF_SRST_ADDRESS 16

/* function pipeline_sp_initialize_stage: 192F */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_isys_sp_frontend_states
#define HIVE_MEM_ia_css_isys_sp_frontend_states scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_isys_sp_frontend_states 0x62F4
#define HIVE_SIZE_ia_css_isys_sp_frontend_states 12
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_isys_sp_frontend_states scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_isys_sp_frontend_states 0x62F4
#define HIVE_SIZE_sp_ia_css_isys_sp_frontend_states 12

/* function ia_css_dmaproxy_sp_read_byte_addr_mmio: 6FFD */

/* function ia_css_ispctrl_sp_done_ds: 3985 */

/* function ia_css_sp_isp_param_get_mem_inits: 4993 */

/* function ia_css_parambuf_sp_init_buffer_queues: 13DB */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_vbuf_pfp_spref
#define HIVE_MEM_vbuf_pfp_spref scalar_processor_2400_dmem
#define HIVE_ADDR_vbuf_pfp_spref 0x2F0
#define HIVE_SIZE_vbuf_pfp_spref 4
#else
#endif
#endif
#define HIVE_MEM_sp_vbuf_pfp_spref scalar_processor_2400_dmem
#define HIVE_ADDR_sp_vbuf_pfp_spref 0x2F0
#define HIVE_SIZE_sp_vbuf_pfp_spref 4

/* function input_system_cfg: AC6 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ISP_HMEM_BASE
#define HIVE_MEM_ISP_HMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_ISP_HMEM_BASE 0x20
#define HIVE_SIZE_ISP_HMEM_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_ISP_HMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ISP_HMEM_BASE 0x20
#define HIVE_SIZE_sp_ISP_HMEM_BASE 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_pipe_private_frames
#define HIVE_MEM_ia_css_bufq_sp_pipe_private_frames scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_pipe_private_frames 0x5A08
#define HIVE_SIZE_ia_css_bufq_sp_pipe_private_frames 280
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_pipe_private_frames scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_pipe_private_frames 0x5A08
#define HIVE_SIZE_sp_ia_css_bufq_sp_pipe_private_frames 280

/* function qos_scheduler_init_stage_budget: 67EB */

/* function ia_css_isys_sp_backend_release: 5CCA */

/* function ia_css_isys_sp_backend_destroy: 5CF4 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp2host_buffer_queue_handle
#define HIVE_MEM_sp2host_buffer_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp2host_buffer_queue_handle 0x5B20
#define HIVE_SIZE_sp2host_buffer_queue_handle 96
#else
#endif
#endif
#define HIVE_MEM_sp_sp2host_buffer_queue_handle scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp2host_buffer_queue_handle 0x5B20
#define HIVE_SIZE_sp_sp2host_buffer_queue_handle 96

/* function ia_css_isys_sp_token_map_check_mipi_frame_size: 6130 */

/* function ia_css_ispctrl_sp_init_isp_vars: 4685 */

/* function ia_css_isys_sp_frontend_has_empty_mipi_buffer_cb: 5D46 */

/* function sp_warning: 881 */

/* function ia_css_rmgr_sp_vbuf_enqueue: 64DA */

/* function ia_css_tagger_sp_tag_exp_id: 217C */

/* function ia_css_dmaproxy_sp_write: 33C6 */

/* function ia_css_parambuf_sp_release_in_param: 125B */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_irq_sw_interrupt_token
#define HIVE_MEM_irq_sw_interrupt_token scalar_processor_2400_dmem
#define HIVE_ADDR_irq_sw_interrupt_token 0x40F8
#define HIVE_SIZE_irq_sw_interrupt_token 4
#else
#endif
#endif
#define HIVE_MEM_sp_irq_sw_interrupt_token scalar_processor_2400_dmem
#define HIVE_ADDR_sp_irq_sw_interrupt_token 0x40F8
#define HIVE_SIZE_sp_irq_sw_interrupt_token 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_isp_addresses
#define HIVE_MEM_sp_isp_addresses scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isp_addresses 0x5F74
#define HIVE_SIZE_sp_isp_addresses 172
#else
#endif
#endif
#define HIVE_MEM_sp_sp_isp_addresses scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_isp_addresses 0x5F74
#define HIVE_SIZE_sp_sp_isp_addresses 172

/* function ia_css_rmgr_sp_acq_gen: 63FF */

/* function receiver_reg_load: ADB */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_isps
#define HIVE_MEM_isps scalar_processor_2400_dmem
#define HIVE_ADDR_isps 0x632C
#define HIVE_SIZE_isps 28
#else
#endif
#endif
#define HIVE_MEM_sp_isps scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isps 0x632C
#define HIVE_SIZE_sp_isps 28

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_host_sp_queues_initialized
#define HIVE_MEM_host_sp_queues_initialized scalar_processor_2400_dmem
#define HIVE_ADDR_host_sp_queues_initialized 0x4110
#define HIVE_SIZE_host_sp_queues_initialized 4
#else
#endif
#endif
#define HIVE_MEM_sp_host_sp_queues_initialized scalar_processor_2400_dmem
#define HIVE_ADDR_sp_host_sp_queues_initialized 0x4110
#define HIVE_SIZE_sp_host_sp_queues_initialized 4

/* function ia_css_queue_uninit: 4DD1 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_ispctrl_sp_isp_started
#define HIVE_MEM_ia_css_ispctrl_sp_isp_started scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_ispctrl_sp_isp_started 0x5C28
#define HIVE_SIZE_ia_css_ispctrl_sp_isp_started 4
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_ispctrl_sp_isp_started scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_ispctrl_sp_isp_started 0x5C28
#define HIVE_SIZE_sp_ia_css_ispctrl_sp_isp_started 4

/* function ia_css_bufq_sp_release_dynamic_buf: 2F5A */

/* function ia_css_dmaproxy_sp_set_height_exception: 34D3 */

/* function ia_css_dmaproxy_sp_init_vmem_channel: 3444 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_num_ready_threads
#define HIVE_MEM_num_ready_threads scalar_processor_2400_dmem
#define HIVE_ADDR_num_ready_threads 0x4A10
#define HIVE_SIZE_num_ready_threads 4
#else
#endif
#endif
#define HIVE_MEM_sp_num_ready_threads scalar_processor_2400_dmem
#define HIVE_ADDR_sp_num_ready_threads 0x4A10
#define HIVE_SIZE_sp_num_ready_threads 4

/* function ia_css_dmaproxy_sp_write_byte_addr_mmio: 3398 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_vbuf_spref
#define HIVE_MEM_vbuf_spref scalar_processor_2400_dmem
#define HIVE_ADDR_vbuf_spref 0x2EC
#define HIVE_SIZE_vbuf_spref 4
#else
#endif
#endif
#define HIVE_MEM_sp_vbuf_spref scalar_processor_2400_dmem
#define HIVE_ADDR_sp_vbuf_spref 0x2EC
#define HIVE_SIZE_sp_vbuf_spref 4

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_metadata_thread
#define HIVE_MEM_sp_metadata_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_metadata_thread 0x49C4
#define HIVE_SIZE_sp_metadata_thread 68
#else
#endif
#endif
#define HIVE_MEM_sp_sp_metadata_thread scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_metadata_thread 0x49C4
#define HIVE_SIZE_sp_sp_metadata_thread 68

/* function ia_css_queue_enqueue: 4D1B */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_flash_sp_request
#define HIVE_MEM_ia_css_flash_sp_request scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_flash_sp_request 0x4AC4
#define HIVE_SIZE_ia_css_flash_sp_request 4
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_flash_sp_request scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_flash_sp_request 0x4AC4
#define HIVE_SIZE_sp_ia_css_flash_sp_request 4

/* function ia_css_dmaproxy_sp_vmem_write: 3369 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_tagger_frames
#define HIVE_MEM_tagger_frames scalar_processor_2400_dmem
#define HIVE_ADDR_tagger_frames 0x4A18
#define HIVE_SIZE_tagger_frames 168
#else
#endif
#endif
#define HIVE_MEM_sp_tagger_frames scalar_processor_2400_dmem
#define HIVE_ADDR_sp_tagger_frames 0x4A18
#define HIVE_SIZE_sp_tagger_frames 168

/* function ia_css_isys_sp_token_map_snd_capture_req: 618E */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_reading_if
#define HIVE_MEM_sem_for_reading_if scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_reading_if 0x47F4
#define HIVE_SIZE_sem_for_reading_if 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_reading_if scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_reading_if 0x47F4
#define HIVE_SIZE_sp_sem_for_reading_if 20

/* function sp_generate_interrupts: 900 */

/* function ia_css_pipeline_sp_start: 1842 */

/* function ia_css_sp_rawcopy_init: 52F8 */

/* function tmr_clock_read: 676C */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ISP_BAMEM_BASE
#define HIVE_MEM_ISP_BAMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_ISP_BAMEM_BASE 0x2F8
#define HIVE_SIZE_ISP_BAMEM_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_ISP_BAMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ISP_BAMEM_BASE 0x2F8
#define HIVE_SIZE_sp_ISP_BAMEM_BASE 4

/* function ia_css_isys_sp_frontend_rcv_capture_ack: 5DF5 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_bufq_sp_sems_for_sp2host_buf_queues
#define HIVE_MEM_ia_css_bufq_sp_sems_for_sp2host_buf_queues scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_bufq_sp_sems_for_sp2host_buf_queues 0x5B80
#define HIVE_SIZE_ia_css_bufq_sp_sems_for_sp2host_buf_queues 160
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_bufq_sp_sems_for_sp2host_buf_queues scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_bufq_sp_sems_for_sp2host_buf_queues 0x5B80
#define HIVE_SIZE_sp_ia_css_bufq_sp_sems_for_sp2host_buf_queues 160

/* function css_get_frame_processing_time_start: 1FE9 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_all_cbs_frame
#define HIVE_MEM_sp_all_cbs_frame scalar_processor_2400_dmem
#define HIVE_ADDR_sp_all_cbs_frame 0x4808
#define HIVE_SIZE_sp_all_cbs_frame 16
#else
#endif
#endif
#define HIVE_MEM_sp_sp_all_cbs_frame scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_all_cbs_frame 0x4808
#define HIVE_SIZE_sp_sp_all_cbs_frame 16

/* function thread_sp_queue_print: D9A */

/* function sp_notify_eof: 8AC */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sem_for_str2mem
#define HIVE_MEM_sem_for_str2mem scalar_processor_2400_dmem
#define HIVE_ADDR_sem_for_str2mem 0x4818
#define HIVE_SIZE_sem_for_str2mem 20
#else
#endif
#endif
#define HIVE_MEM_sp_sem_for_str2mem scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sem_for_str2mem 0x4818
#define HIVE_SIZE_sp_sem_for_str2mem 20

/* function ia_css_tagger_buf_sp_is_marked_from_start: 2CCD */

/* function ia_css_bufq_sp_acquire_dynamic_buf: 3112 */

/* function ia_css_circbuf_destroy: 1028 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ISP_PMEM_BASE
#define HIVE_MEM_ISP_PMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_ISP_PMEM_BASE 0xC
#define HIVE_SIZE_ISP_PMEM_BASE 4
#else
#endif
#endif
#define HIVE_MEM_sp_ISP_PMEM_BASE scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ISP_PMEM_BASE 0xC
#define HIVE_SIZE_sp_ISP_PMEM_BASE 4

/* function ia_css_sp_isp_param_mem_load: 4926 */

/* function ia_css_tagger_buf_sp_pop_from_start: 2AB9 */

/* function __div: 6A6D */

/* function ia_css_isys_sp_frontend_create: 5FC6 */

/* function ia_css_rmgr_sp_refcount_release_vbuf: 64F9 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_ia_css_flash_sp_in_use
#define HIVE_MEM_ia_css_flash_sp_in_use scalar_processor_2400_dmem
#define HIVE_ADDR_ia_css_flash_sp_in_use 0x4AC8
#define HIVE_SIZE_ia_css_flash_sp_in_use 4
#else
#endif
#endif
#define HIVE_MEM_sp_ia_css_flash_sp_in_use scalar_processor_2400_dmem
#define HIVE_ADDR_sp_ia_css_flash_sp_in_use 0x4AC8
#define HIVE_SIZE_sp_ia_css_flash_sp_in_use 4

/* function ia_css_thread_sem_sp_wait: 6D50 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_sleep_mode
#define HIVE_MEM_sp_sleep_mode scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sleep_mode 0x4114
#define HIVE_SIZE_sp_sleep_mode 4
#else
#endif
#endif
#define HIVE_MEM_sp_sp_sleep_mode scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_sleep_mode 0x4114
#define HIVE_SIZE_sp_sp_sleep_mode 4

/* function ia_css_tagger_buf_sp_push: 2BC8 */

/* function mmu_invalidate_cache: D3 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_sp_max_cb_elems
#define HIVE_MEM_sp_max_cb_elems scalar_processor_2400_dmem
#define HIVE_ADDR_sp_max_cb_elems 0x148
#define HIVE_SIZE_sp_max_cb_elems 8
#else
#endif
#endif
#define HIVE_MEM_sp_sp_max_cb_elems scalar_processor_2400_dmem
#define HIVE_ADDR_sp_sp_max_cb_elems 0x148
#define HIVE_SIZE_sp_sp_max_cb_elems 8

/* function ia_css_queue_remote_init: 4DF3 */

#ifndef HIVE_MULTIPLE_PROGRAMS
#ifndef HIVE_MEM_isp_stop_req
#define HIVE_MEM_isp_stop_req scalar_processor_2400_dmem
#define HIVE_ADDR_isp_stop_req 0x46AC
#define HIVE_SIZE_isp_stop_req 4
#else
#endif
#endif
#define HIVE_MEM_sp_isp_stop_req scalar_processor_2400_dmem
#define HIVE_ADDR_sp_isp_stop_req 0x46AC
#define HIVE_SIZE_sp_isp_stop_req 4


#endif /* _sp_map_h_ */
extern void sh_css_dump_sp_dmem(void);
void sh_css_dump_sp_dmem(void)
{
}
