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

#include <type_support.h>
#include <string_support.h> /* memcpy */
#include "system_global.h"
#include "vamem.h"
#include "ia_css_types.h"
#include "ia_css_ctc_table.host.h"

struct ia_css_ctc_table       default_ctc_table;

#if defined(HAS_VAMEM_VERSION_2)

static const uint16_t
default_ctc_table_data[IA_CSS_VAMEM_2_CTC_TABLE_SIZE] = {
   0,  384,  837,  957, 1011, 1062, 1083, 1080,
1078, 1077, 1053, 1039, 1012,  992,  969,  951,
 929,  906,  886,  866,  845,  823,  809,  790,
 772,  758,  741,  726,  711,  701,  688,  675,
 666,  656,  648,  639,  633,  626,  618,  612,
 603,  594,  582,  572,  557,  545,  529,  516,
 504,  491,  480,  467,  459,  447,  438,  429,
 419,  412,  404,  397,  389,  382,  376,  368,
 363,  357,  351,  345,  340,  336,  330,  326,
 321,  318,  312,  308,  304,  300,  297,  294,
 291,  286,  284,  281,  278,  275,  271,  268,
 261,  257,  251,  245,  240,  235,  232,  225,
 223,  218,  213,  209,  206,  204,  199,  197,
 193,  189,  186,  185,  183,  179,  177,  175,
 172,  170,  169,  167,  164,  164,  162,  160,
 158,  157,  156,  154,  154,  152,  151,  150,
 149,  148,  146,  147,  146,  144,  143,  143,
 142,  141,  140,  141,  139,  138,  138,  138,
 137,  136,  136,  135,  134,  134,  134,  133,
 132,  132,  131,  130,  131,  130,  129,  128,
 129,  127,  127,  127,  127,  125,  125,  125,
 123,  123,  122,  120,  118,  115,  114,  111,
 110,  108,  106,  105,  103,  102,  100,   99,
  97,   97,   96,   95,   94,   93,   93,   91,
  91,   91,   90,   90,   89,   89,   88,   88,
  89,   88,   88,   87,   87,   87,   87,   86,
  87,   87,   86,   87,   86,   86,   84,   84,
  82,   80,   78,   76,   74,   72,   70,   68,
  67,   65,   62,   60,   58,   56,   55,   54,
  53,   51,   49,   49,   47,   45,   45,   45,
  41,   40,   39,   39,   34,   33,   34,   32,
  25,   23,   24,   20,   13,    9,   12,    0,
   0
};

#elif defined(HAS_VAMEM_VERSION_1)

/* Default Parameters */
static const uint16_t
default_ctc_table_data[IA_CSS_VAMEM_1_CTC_TABLE_SIZE] = {
		0, 0, 256, 384, 384, 497, 765, 806,
		837, 851, 888, 901, 957, 981, 993, 1001,
		1011, 1029, 1028, 1039, 1062, 1059, 1073, 1080,
		1083, 1085, 1085, 1098, 1080, 1084, 1085, 1093,
		1078, 1073, 1070, 1069, 1077, 1066, 1072, 1063,
		1053, 1044, 1046, 1053, 1039, 1028, 1025, 1024,
		1012, 1013, 1016, 996, 992, 990, 990, 980,
		969, 968, 961, 955, 951, 949, 933, 930,
		929, 925, 921, 916, 906, 901, 895, 893,
		886, 877, 872, 869, 866, 861, 857, 849,
		845, 838, 836, 832, 823, 821, 815, 813,
		809, 805, 796, 793, 790, 785, 784, 778,
		772, 768, 766, 763, 758, 752, 749, 745,
		741, 740, 736, 730, 726, 724, 723, 718,
		711, 709, 706, 704, 701, 698, 691, 689,
		688, 683, 683, 678, 675, 673, 671, 669,
		666, 663, 661, 660, 656, 656, 653, 650,
		648, 647, 646, 643, 639, 638, 637, 635,
		633, 632, 629, 627, 626, 625, 622, 621,
		618, 618, 614, 614, 612, 609, 606, 606,
		603, 600, 600, 597, 594, 591, 590, 586,
		582, 581, 578, 575, 572, 569, 563, 560,
		557, 554, 551, 548, 545, 539, 536, 533,
		529, 527, 524, 519, 516, 513, 510, 507,
		504, 501, 498, 493, 491, 488, 485, 484,
		480, 476, 474, 471, 467, 466, 464, 460,
		459, 455, 453, 449, 447, 446, 443, 441,
		438, 435, 432, 432, 429, 427, 426, 422,
		419, 418, 416, 414, 412, 410, 408, 406,
		404, 402, 401, 398, 397, 395, 393, 390,
		389, 388, 387, 384, 382, 380, 378, 377,
		376, 375, 372, 370, 368, 368, 366, 364,
		363, 361, 360, 358, 357, 355, 354, 352,
		351, 350, 349, 346, 345, 344, 344, 342,
		340, 339, 337, 337, 336, 335, 333, 331,
		330, 329, 328, 326, 326, 324, 324, 322,
		321, 320, 318, 318, 318, 317, 315, 313,
		312, 311, 311, 310, 308, 307, 306, 306,
		304, 304, 302, 301, 300, 300, 299, 297,
		297, 296, 296, 294, 294, 292, 291, 291,
		291, 290, 288, 287, 286, 286, 287, 285,
		284, 283, 282, 282, 281, 281, 279, 278,
		278, 278, 276, 276, 275, 274, 274, 273,
		271, 270, 269, 268, 268, 267, 265, 262,
		261, 260, 260, 259, 257, 254, 252, 252,
		251, 251, 249, 246, 245, 244, 243, 242,
		240, 239, 239, 237, 235, 235, 233, 231,
		232, 230, 229, 226, 225, 224, 225, 224,
		223, 220, 219, 219, 218, 217, 217, 214,
		213, 213, 212, 211, 209, 209, 209, 208,
		206, 205, 204, 203, 204, 203, 201, 200,
		199, 197, 198, 198, 197, 195, 194, 194,
		193, 192, 192, 191, 189, 190, 189, 188,
		186, 187, 186, 185, 185, 184, 183, 181,
		183, 182, 181, 180, 179, 178, 178, 178,
		177, 176, 175, 176, 175, 174, 174, 173,
		172, 173, 172, 171, 170, 170, 169, 169,
		169, 168, 167, 166, 167, 167, 166, 165,
		164, 164, 164, 163, 164, 163, 162, 163,
		162, 161, 160, 161, 160, 160, 160, 159,
		158, 157, 158, 158, 157, 157, 156, 156,
		156, 156, 155, 155, 154, 154, 154, 154,
		154, 153, 152, 153, 152, 152, 151, 152,
		151, 152, 151, 150, 150, 149, 149, 150,
		149, 149, 148, 148, 148, 149, 148, 147,
		146, 146, 147, 146, 147, 146, 145, 146,
		146, 145, 144, 145, 144, 145, 144, 144,
		143, 143, 143, 144, 143, 142, 142, 142,
		142, 142, 142, 141, 141, 141, 141, 140,
		140, 141, 140, 140, 141, 140, 139, 139,
		139, 140, 139, 139, 138, 138, 137, 139,
		138, 138, 138, 137, 138, 137, 137, 137,
		137, 136, 137, 136, 136, 136, 136, 135,
		136, 135, 135, 135, 135, 136, 135, 135,
		134, 134, 133, 135, 134, 134, 134, 133,
		134, 133, 134, 133, 133, 132, 133, 133,
		132, 133, 132, 132, 132, 132, 131, 131,
		131, 132, 131, 131, 130, 131, 130, 132,
		131, 130, 130, 129, 130, 129, 130, 129,
		129, 129, 130, 129, 128, 128, 128, 128,
		129, 128, 128, 127, 127, 128, 128, 127,
		127, 126, 126, 127, 127, 126, 126, 126,
		127, 126, 126, 126, 125, 125, 126, 125,
		125, 124, 124, 124, 125, 125, 124, 124,
		123, 124, 124, 123, 123, 122, 122, 122,
		122, 122, 121, 120, 120, 119, 118, 118,
		118, 117, 117, 116, 115, 115, 115, 114,
		114, 113, 113, 112, 111, 111, 111, 110,
		110, 109, 109, 108, 108, 108, 107, 107,
		106, 106, 105, 105, 105, 104, 104, 103,
		103, 102, 102, 102, 102, 101, 101, 100,
		100, 99, 99, 99, 99, 99, 99, 98,
		97, 98, 97, 97, 97, 96, 96, 95,
		96, 95, 96, 95, 95, 94, 94, 95,
		94, 94, 94, 93, 93, 92, 93, 93,
		93, 93, 92, 92, 91, 92, 92, 92,
		91, 91, 90, 90, 91, 91, 91, 90,
		90, 90, 90, 91, 90, 90, 90, 89,
		89, 89, 90, 89, 89, 89, 89, 89,
		88, 89, 89, 88, 88, 88, 88, 87,
		89, 88, 88, 88, 88, 88, 87, 88,
		88, 88, 87, 87, 87, 87, 87, 88,
		87, 87, 87, 87, 87, 87, 88, 87,
		87, 87, 87, 86, 86, 87, 87, 87,
		87, 86, 86, 86, 87, 87, 86, 87,
		86, 86, 86, 87, 87, 86, 86, 86,
		86, 86, 87, 87, 86, 85, 85, 85,
		84, 85, 85, 84, 84, 83, 83, 82,
		82, 82, 81, 81, 80, 79, 79, 79,
		78, 77, 77, 76, 76, 76, 75, 74,
		74, 74, 73, 73, 72, 71, 71, 71,
		70, 70, 69, 69, 68, 68, 67, 67,
		67, 66, 66, 65, 65, 64, 64, 63,
		62, 62, 62, 61, 60, 60, 59, 59,
		58, 58, 57, 57, 56, 56, 56, 55,
		55, 54, 55, 55, 54, 53, 53, 52,
		53, 53, 52, 51, 51, 50, 51, 50,
		49, 49, 50, 49, 49, 48, 48, 47,
		47, 48, 46, 45, 45, 45, 46, 45,
		45, 44, 45, 45, 45, 43, 42, 42,
		41, 43, 41, 40, 40, 39, 40, 41,
		39, 39, 39, 39, 39, 38, 35, 35,
		34, 37, 36, 34, 33, 33, 33, 35,
		34, 32, 32, 31, 32, 30, 29, 26,
		25, 25, 27, 26, 23, 23, 23, 25,
		24, 24, 22, 21, 20, 19, 16, 14,
		13, 13, 13, 10, 9, 7, 7, 7,
		12, 12, 12, 7, 0, 0, 0, 0
};

#else
#error "VAMEM version must be one of {VAMEM_VERSION_1, VAMEM_VERSION_2}"
#endif

void
ia_css_config_ctc_table(void)
{
#if defined(HAS_VAMEM_VERSION_2)
	memcpy(default_ctc_table.data.vamem_2, default_ctc_table_data,
	       sizeof(default_ctc_table_data));
	default_ctc_table.vamem_type     = IA_CSS_VAMEM_TYPE_2;
#else
	memcpy(default_ctc_table.data.vamem_1, default_ctc_table_data,
	       sizeof(default_ctc_table_data));
	default_ctc_table.vamem_type     = 1IA_CSS_VAMEM_TYPE_1;
#endif
}

