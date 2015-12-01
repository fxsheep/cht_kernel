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
/* Generated code: do not edit or commmit. */

#ifndef _IA_CSS_ISP_PARAM_H
#define _IA_CSS_ISP_PARAM_H

/* Code generated by genparam/gencode.c:gen_param_enum() */

enum ia_css_parameter_ids {
	IA_CSS_AA_ID,
	IA_CSS_ANR_ID,
	IA_CSS_ANR2_ID,
	IA_CSS_BH_ID,
	IA_CSS_CNR_ID,
	IA_CSS_CROP_ID,
	IA_CSS_CSC_ID,
	IA_CSS_DP_ID,
	IA_CSS_BNR_ID,
	IA_CSS_DPC2_ID,
	IA_CSS_EED1_8_ID,
	IA_CSS_DE_ID,
	IA_CSS_ECD_ID,
	IA_CSS_FORMATS_ID,
	IA_CSS_FPN_ID,
	IA_CSS_GC_ID,
	IA_CSS_CE_ID,
	IA_CSS_YUV2RGB_ID,
	IA_CSS_RGB2YUV_ID,
	IA_CSS_R_GAMMA_ID,
	IA_CSS_G_GAMMA_ID,
	IA_CSS_B_GAMMA_ID,
	IA_CSS_UDS_ID,
	IA_CSS_RAA_ID,
	IA_CSS_S3A_ID,
	IA_CSS_OB_ID,
	IA_CSS_OB2_ID,
	IA_CSS_OUTPUT_ID,
	IA_CSS_SC_ID,
	IA_CSS_BDS_ID,
	IA_CSS_TNR_ID,
	IA_CSS_MACC_ID,
	IA_CSS_MACC1_5_ID,
	IA_CSS_SDIS_HORICOEF_ID,
	IA_CSS_SDIS_VERTCOEF_ID,
	IA_CSS_SDIS_HORIPROJ_ID,
	IA_CSS_SDIS_VERTPROJ_ID,
	IA_CSS_SDIS2_HORICOEF_ID,
	IA_CSS_SDIS2_VERTCOEF_ID,
	IA_CSS_SDIS2_HORIPROJ_ID,
	IA_CSS_SDIS2_VERTPROJ_ID,
	IA_CSS_WB_ID,
	IA_CSS_NR_ID,
	IA_CSS_YEE_ID,
	IA_CSS_YNR_ID,
	IA_CSS_FC_ID,
	IA_CSS_CTC_ID,
	IA_CSS_CTC2_ID,
	IA_CSS_XNR_TABLE_ID,
	IA_CSS_XNR_ID,
	IA_CSS_XNR3_ID,
	IA_CSS_IEFD2_6_ID,
	IA_CSS_XNR3_0_11_ID,
	IA_CSS_NUM_PARAMETER_IDS
};

/* Code generated by genparam/gencode.c:gen_param_offsets() */

struct ia_css_memory_offsets {
	struct {
		struct ia_css_isp_parameter aa;
		struct ia_css_isp_parameter anr;
		struct ia_css_isp_parameter bh;
		struct ia_css_isp_parameter cnr;
		struct ia_css_isp_parameter crop;
		struct ia_css_isp_parameter csc;
		struct ia_css_isp_parameter dp;
		struct ia_css_isp_parameter bnr;
		struct ia_css_isp_parameter dpc2;
		struct ia_css_isp_parameter eed1_8;
		struct ia_css_isp_parameter de;
		struct ia_css_isp_parameter ecd;
		struct ia_css_isp_parameter formats;
		struct ia_css_isp_parameter fpn;
		struct ia_css_isp_parameter gc;
		struct ia_css_isp_parameter ce;
		struct ia_css_isp_parameter yuv2rgb;
		struct ia_css_isp_parameter rgb2yuv;
		struct ia_css_isp_parameter uds;
		struct ia_css_isp_parameter raa;
		struct ia_css_isp_parameter s3a;
		struct ia_css_isp_parameter ob;
		struct ia_css_isp_parameter ob2;
		struct ia_css_isp_parameter output;
		struct ia_css_isp_parameter sc;
		struct ia_css_isp_parameter bds;
		struct ia_css_isp_parameter tnr;
		struct ia_css_isp_parameter macc;
		struct ia_css_isp_parameter macc1_5;
		struct ia_css_isp_parameter sdis_horiproj;
		struct ia_css_isp_parameter sdis_vertproj;
		struct ia_css_isp_parameter sdis2_horiproj;
		struct ia_css_isp_parameter sdis2_vertproj;
		struct ia_css_isp_parameter wb;
		struct ia_css_isp_parameter nr;
		struct ia_css_isp_parameter yee;
		struct ia_css_isp_parameter ynr;
		struct ia_css_isp_parameter fc;
		struct ia_css_isp_parameter ctc;
		struct ia_css_isp_parameter ctc2;
		struct ia_css_isp_parameter xnr;
		struct ia_css_isp_parameter xnr3;
		struct ia_css_isp_parameter iefd2_6;
		struct ia_css_isp_parameter xnr3_0_11;
		struct ia_css_isp_parameter get;
		struct ia_css_isp_parameter put;
		struct ia_css_isp_parameter plane_io_config;
	} dmem;
	struct {
		struct ia_css_isp_parameter anr2;
		struct ia_css_isp_parameter eed1_8;
		struct ia_css_isp_parameter ob;
		struct ia_css_isp_parameter macc1_5;
		struct ia_css_isp_parameter sdis_horicoef;
		struct ia_css_isp_parameter sdis_vertcoef;
		struct ia_css_isp_parameter sdis2_horicoef;
		struct ia_css_isp_parameter sdis2_vertcoef;
		struct ia_css_isp_parameter ctc2;
		struct ia_css_isp_parameter xnr3;
		struct ia_css_isp_parameter iefd2_6;
		struct ia_css_isp_parameter xnr3_0_11;
	} vmem;
	struct {
		struct ia_css_isp_parameter bh;
	} hmem0;
	struct {
		struct ia_css_isp_parameter gc;
		struct ia_css_isp_parameter g_gamma;
		struct ia_css_isp_parameter xnr_table;
	} vamem1;
	struct {
		struct ia_css_isp_parameter r_gamma;
		struct ia_css_isp_parameter ctc;
	} vamem0;
	struct {
		struct ia_css_isp_parameter b_gamma;
	} vamem2;
};

#if defined(IA_CSS_INCLUDE_PARAMETERS)

#include "ia_css_stream.h"   /* struct ia_css_stream */
#include "ia_css_binary.h"   /* struct ia_css_binary */
/* Code generated by genparam/gencode.c:gen_param_process_table() */

struct ia_css_pipeline_stage; /* forward declaration */

extern void (* ia_css_kernel_process_param[IA_CSS_NUM_PARAMETER_IDS])(
			unsigned pipe_id,
			const struct ia_css_pipeline_stage *stage,
			struct ia_css_isp_parameters *params);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_dp_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dp_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_dpc2_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dpc2_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_eed1_8_config(struct ia_css_isp_parameters *params,
			const struct ia_css_eed1_8_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_wb_config(struct ia_css_isp_parameters *params,
			const struct ia_css_wb_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_tnr_config(struct ia_css_isp_parameters *params,
			const struct ia_css_tnr_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_ob_config(struct ia_css_isp_parameters *params,
			const struct ia_css_ob_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_ob2_config(struct ia_css_isp_parameters *params,
			const struct ia_css_ob2_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_de_config(struct ia_css_isp_parameters *params,
			const struct ia_css_de_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_anr_config(struct ia_css_isp_parameters *params,
			const struct ia_css_anr_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_anr2_config(struct ia_css_isp_parameters *params,
			const struct ia_css_anr_thres *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_ce_config(struct ia_css_isp_parameters *params,
			const struct ia_css_ce_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_ecd_config(struct ia_css_isp_parameters *params,
			const struct ia_css_ecd_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_ynr_config(struct ia_css_isp_parameters *params,
			const struct ia_css_ynr_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_fc_config(struct ia_css_isp_parameters *params,
			const struct ia_css_fc_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_cnr_config(struct ia_css_isp_parameters *params,
			const struct ia_css_cnr_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_macc_config(struct ia_css_isp_parameters *params,
			const struct ia_css_macc_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_macc1_5_config(struct ia_css_isp_parameters *params,
			const struct ia_css_macc1_5_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_ctc_config(struct ia_css_isp_parameters *params,
			const struct ia_css_ctc_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_ctc2_config(struct ia_css_isp_parameters *params,
			const struct ia_css_ctc2_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_iefd2_6_config(struct ia_css_isp_parameters *params,
			const struct ia_css_iefd2_6_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_aa_config(struct ia_css_isp_parameters *params,
			const struct ia_css_aa_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_yuv2rgb_config(struct ia_css_isp_parameters *params,
			const struct ia_css_cc_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_rgb2yuv_config(struct ia_css_isp_parameters *params,
			const struct ia_css_cc_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_csc_config(struct ia_css_isp_parameters *params,
			const struct ia_css_cc_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_nr_config(struct ia_css_isp_parameters *params,
			const struct ia_css_nr_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_gc_config(struct ia_css_isp_parameters *params,
			const struct ia_css_gc_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_sdis_horicoef_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dvs_coefficients *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_sdis_vertcoef_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dvs_coefficients *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_sdis_horiproj_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dvs_coefficients *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_sdis_vertproj_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dvs_coefficients *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_sdis2_horicoef_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dvs2_coefficients *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_sdis2_vertcoef_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dvs2_coefficients *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_sdis2_horiproj_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dvs2_coefficients *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_sdis2_vertproj_config(struct ia_css_isp_parameters *params,
			const struct ia_css_dvs2_coefficients *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_r_gamma_config(struct ia_css_isp_parameters *params,
			const struct ia_css_rgb_gamma_table *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_g_gamma_config(struct ia_css_isp_parameters *params,
			const struct ia_css_rgb_gamma_table *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_b_gamma_config(struct ia_css_isp_parameters *params,
			const struct ia_css_rgb_gamma_table *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_xnr_table_config(struct ia_css_isp_parameters *params,
			const struct ia_css_xnr_table *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_formats_config(struct ia_css_isp_parameters *params,
			const struct ia_css_formats_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_xnr_config(struct ia_css_isp_parameters *params,
			const struct ia_css_xnr_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_xnr3_config(struct ia_css_isp_parameters *params,
			const struct ia_css_xnr3_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_xnr3_0_11_config(struct ia_css_isp_parameters *params,
			const struct ia_css_xnr3_0_11_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_s3a_config(struct ia_css_isp_parameters *params,
			const struct ia_css_3a_config *config);

/* Code generated by genparam/gencode.c:gen_set_function() */

void
ia_css_set_output_config(struct ia_css_isp_parameters *params,
			const struct ia_css_output_config *config);

/* Code generated by genparam/gencode.c:gen_global_access_function() */

void
ia_css_get_configs(struct ia_css_isp_parameters *params,
		const struct ia_css_isp_config *config)
;

/* Code generated by genparam/gencode.c:gen_global_access_function() */

void
ia_css_set_configs(struct ia_css_isp_parameters *params,
		const struct ia_css_isp_config *config)
;

#endif /* IA_CSS_INCLUDE_PARAMETER */

#endif /* _IA_CSS_ISP_PARAM_H */
