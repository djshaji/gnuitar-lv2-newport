/*
 * tubeamp.c
 * 
 * Copyright 2024 Shaji Khan <djshaji@wanderer>
 * Port of the legendary
 * GNUitar
 * Distortion effect 3 -- Tube amplifier
 * Copyright (C) 2006 Antti S. Lankila  <alankila@bel.fi>
 * to lv2
 * GPL license.
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "log.h"

typedef float DSP_SAMPLE ;

#include "biquad.h"
#include "tubeamp.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

// ha ha
#ifdef __ANDROID__
    #include <android/log.h>
    //~ #define MODULE_NAME "GNUitar Tube Amp"
    //~ #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, MODULE_NAME, __VA_ARGS__)
#else
    #define LOGD(...) printf(__VA_ARGS__)
#endif

#define URI "http://shaji.in/plugins/gnuitar-tubeamp-np"

#define UPSAMPLE_RATIO 6
#define IMPULSE_SIZE   512
#define MAX_SAMPLE (32767 << 8)

typedef enum {
	INPUT,
	OUTPUT,
	STAGES,
	IMPULSE_QUALITY,
	IMPULSE_MODEL,
	BIAS_FACTOR,
	ASYMMETRY,
	GAIN,
	TONE_BASS,
	TONE_MIDDLE,
	TONE_TREBLE
} PortIndex ;

/* Marshall G15R */
static const DSP_SAMPLE __attribute__((aligned(16))) impulse_g15r[IMPULSE_SIZE] = {
  4405,  17364,  30475,  33517,  28810,  20846,   9309,  -4045, -13421, -13737,  -6939,
  -644,   2381,   4726,   4890,   -577,  -8708, -13224, -11835,  -6840,   -805,   5847,
 11158,  10895,   3963,  -5524, -11923, -13616, -12717,  -9577,  -4247,   -180,    568,
  -647,  -3434,  -7154,  -9508,  -9045,  -5524,   -436,   3582,   4809,   2857,   -600,
 -3441,  -5228,  -5474,  -3608,   -936,    568,    785,    961,   1268,    763,   -657,
 -1941,  -2769,  -3712,  -4174,  -3405,  -2362,  -2122,  -2521,  -2793,  -2499,  -1808,
 -1174,  -1051,  -1339,  -1579,  -1982,  -2796,  -3538,  -3834,  -3492,  -2315,   -813,
   461,   1832,   3235,   3535,   1981,   -688,  -2823,  -3401,  -2683,  -1290,    290,
  1276,    924,   -613,  -2299,  -3054,  -2582,  -1428,   -275,    411,    317,   -452,
 -1440,  -2337,  -2950,  -3028,  -2519,  -1646,   -584,    580,   1663,   2431,   2623,
  2135,   1272,    563,    325,    470,    861,   1568,   2341,   2490,   1838,    954,
   227,   -215,   -167,    182,    305,    140,    -70,   -334,   -565,   -404,    123,
   592,    852,   1030,   1158,   1222,   1264,   1225,    932,    389,   -103,   -322,
  -347,   -283,    -81,    280,    683,   1030,   1335,   1549,   1539,   1323,   1070,
   928,   1095,   1687,   2403,   2755,   2615,   2172,   1513,    736,    167,    -33,
   -77,    -72,     70,    261,    311,    206,     41,   -127,   -177,     79,    635,
  1166,   1339,   1139,    812,    641,    707,    913,   1170,   1414,   1597,   1697,
  1667,   1492,   1250,    993,    687,    328,      7,   -170,   -125,    118,    346,
   425,    483,    579,    577,    465,    405,    358,    138,   -141,   -242,   -189,
  -124,    -41,    136,    373,    533,    603,    652,    682,    710,    728,    673,
   560,    468,    378,    222,     39,    -20,    117,    391,    740,   1171,   1582,
  1737,   1543,   1113,    601,    108,   -273,   -479,   -516,   -482,   -554,   -802,
 -1038,  -1068,   -905,   -618,   -236,    137,    338,    388,    463,    582,    567,
   325,     52,    -29,     22,    141,    347,    481,    382,    206,    121,     38,
  -141,   -394,   -684,   -914,   -923,   -729,   -536,   -457,   -459,   -499,   -510,
  -412,   -254,   -135,   -115,   -243,   -509,   -723,   -695,   -588,   -668,   -874,
 -1000,   -994,   -927,   -852,   -743,   -587,   -456,   -436,   -472,   -439,   -281,
   -41,    140,    152,     17,   -204,   -454,   -649,   -740,   -707,   -573,   -403,
  -268,   -229,   -307,   -460,   -685,  -1015,  -1384,  -1657,  -1755,  -1664,  -1434,
 -1150,   -859,   -555,   -255,    -60,    -17,    -32,    -22,     -2,    -20,    -83,
  -169,   -288,   -388,   -351,   -173,     47,    273,    483,    573,    444,     86,
  -348,   -621,   -701,   -706,   -631,   -489,   -438,   -519,   -577,   -562,   -513,
  -373,   -123,    113,    269,    380,    438,    421,    343,    202,     -1,   -229,
  -401,   -450,   -374,   -220,    -65,     23,     47,     17,    -60,   -135,   -151,
   -83,     72,    264,    398,    415,    295,     73,   -134,   -237,   -268,   -307,
  -370,   -414,   -411,   -308,    -64,    233,    424,    422,    267,     72,    -23,
    63,    282,    545,    824,   1069,   1165,   1098,   1012,    931,    752,    577,
   612,    765,    831,    816,    783,    684,    537,    484,    564,    671,    759,
   843,    860,    742,    569,    459,    432,    459,    541,    680,    798,    812,
   741,    611,    453,    352,    382,    527,    728,    923,   1016,    928,    707,
   491,    393,    467,    670,    900,   1073,   1115,   1000,    778,    577,    520,
   562,    574,    536,    494,    407,    222,     15,    -98,   -111,    -97,    -68,
     6,     73,    109,    224,    442,    592,    535,    306,     37,   -127,   -114,
    23,    183,    268,    193,     30,    -61,    -74,    -46,     45,    156,    201,
   217,    275,    317,    268,    146,    -16,   -186,   -257,   -148,     74,    275,
   372,    338,    207,     53,    -63,   -131,   -187,   -265,   -372,   -482,   -551,
  -587,   -642,   -689,   -662,   -577,   -485
};

/* Princeton II */
static const DSP_SAMPLE __attribute__((aligned(16))) impulse_princeton2[IMPULSE_SIZE] = {
  2799,  11631,  23881,  32811,  34786,  30693,  22401,  12097,   3608,    333,   1986,
  5050,   5906,   3149,  -2263,  -7957, -11151,  -9808,  -4421,   1179,   2345,  -1974,
 -8064, -11426, -10826,  -7845,  -4476,  -2085,  -1307,  -1743,  -2306,  -2291,  -1539,
  -317,    118,  -1496,  -4272,  -5880,  -5257,  -2844,    410,   2743,   1949,  -2161,
 -6821,  -9053,  -8010,  -4596,   -871,    603,  -1322,  -5235,  -8392,  -8852,  -6539,
 -2851,    514,   2273,   1827,   -491,  -3243,  -4689,  -4452,  -3242,  -1617,   -302,
  -339,  -1949,  -3846,  -4448,  -3687,  -3003,  -3510,  -4938,  -6335,  -7114,  -7239,
 -6769,  -5656,  -4182,  -2937,  -2249,  -1904,  -1544,  -1072,   -366,    566,   1098,
   532,  -1043,  -2570,  -2751,  -1216,   1249,   3294,   3731,   2278,   -206,  -2371,
 -3195,  -2402,   -639,    959,   1533,    852,   -597,  -1887,  -2102,   -820,   1549,
  3846,   4837,   4143,   2398,    624,   -298,    -81,    793,   1552,   1605,    889,
   -26,   -445,   -138,    631,   1455,   1937,   1740,    745,   -735,  -2002,  -2435,
 -1966,  -1092,   -495,   -539,  -1109,  -1708,  -1762,  -1079,     21,    935,   1103,
   489,   -400,   -960,   -718,    427,   2035,   3310,   3636,   3115,   2310,   1690,
  1475,   1623,   1770,   1452,    548,   -577,  -1348,  -1424,   -922,   -242,    234,
   357,    244,     66,      9,    208,    450,    376,    -69,   -688,  -1261,  -1605,
 -1569,  -1160,   -563,    -59,    110,    -19,   -277,   -570,   -774,   -798,   -733,
  -693,   -715,   -705,   -525,   -217,     69,    263,    338,    298,    199,     98,
    28,     83,    344,    774,   1293,   1772,   2010,   1960,   1771,   1555,   1321,
  1015,    609,    246,     59,    -23,    -17,    164,    447,    666,    749,    681,
   472,    145,   -206,   -444,   -476,   -389,   -386,   -535,   -683,   -692,   -618,
  -550,   -459,   -335,   -222,   -121,    -20,     42,     19,   -109,   -284,   -432,
  -588,   -803,   -925,   -743,   -296,    158,    419,    486,    393,    140,   -150,
  -305,   -249,    -36,    209,    377,    353,    143,    -27,     72,    414,    789,
  1050,   1150,   1066,    861,    697,    716,    850,    914,    803,    592,    463,
   502,    652,    831,    933,    826,    398,   -277,   -895,  -1136,   -937,   -460,
    54,    374,    374,    147,    -70,   -119,    -51,     13,     48,     69,     99,
   171,    305,    515,    759,    919,    904,    753,    606,    589,    731,    945,
  1125,   1182,   1069,    824,    600,    546,    682,    890,   1042,   1070,    917,
   588,    228,     58,    181,    439,    561,    488,    369,    277,    186,    150,
   218,    306,    309,    228,    161,    210,    355,    474,    526,    538,    479,
   352,    292,    356,    429,    432,    369,    220,     33,     -9,    149,    329,
   374,    347,    353,    356,    312,    369,    588,    735,    650,    497,    459,
   465,    419,    401,    447,    408,    218,     75,    122,    298,    550,    802,
   914,    793,    468,    134,     48,    265,    538,    603,    401,    -11,   -499,
  -805,   -752,   -451,   -167,   -106,   -314,   -666,   -922,   -896,   -621,   -313,
  -169,   -249,   -522,   -850,   -981,   -774,   -374,    -45,     49,    -87,   -320,
  -443,   -281,    207,    831,   1269,   1309,    999,    586,    307,    276,    451,
   608,    527,    265,     86,    145,    403,    710,    869,    762,    456,    139,
   -46,    -62,     53,    189,    214,     73,   -149,   -301,   -285,   -111,    104,
   202,    101,    -96,   -195,   -102,    106,    263,    252,     62,   -223,   -442,
  -446,   -214,    166,    549,    811,    893,    791,    564,    360,    344,    506,
   676,    744,    719,    635,    511,    382,    289,    245,    207,    124,     35,
     6,     -1,    -50,    -83,    -24,    115,    247,    331,    411,    479,    436,
   235,    -15,   -148,   -130,    -84,    -98,   -135,   -147,   -160,   -220,   -304,
  -396,   -515,   -664,   -772,   -733,   -528,   -267,    -96,   -105,   -307,   -610,
  -870,   -939,   -708,   -263,    109,    199
};

typedef struct {
    const char       *name;
    const DSP_SAMPLE *impulse;
} ampmodels_t;

static const ampmodels_t ampmodels[] = {
    { "Marshall G15R", impulse_g15r,      },
    { "Princeton II",  impulse_princeton2 },
    { NULL,            NULL               }
};

typedef struct {
    const char       *name;
    const int        quality;
} ampqualities_t;

static const ampqualities_t ampqualities[] = {
    { "Lowest",  64  },
    { "Normal",  128 },
    { "High",    256 },
    { "Extreme", 512 },
    { NULL,      0   }
};

#define NONLINEARITY_SIZE 16384      /* the bigger the table, the louder before hardclip */
#define NONLINEARITY_PRECISION (1/16.0)   /* the bigger the value, the lower the noise */

#define NONLINEARITY_SCALE 1024     /* this variable works like gain */
static float nonlinearity[NONLINEARITY_SIZE];

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
            double                    sample_rate,
            const char*               bundle_path,
            const LV2_Feature* const* features)
{
	IN
	TubeAmp * params = (TubeAmp *) malloc (sizeof (TubeAmp));
	params->sample_rate = (int) sample_rate ;

    int i;
    float tmp;

    params = gnuitar_memalign(1, sizeof(struct tubeamp_params));

    params->stages = 4;
    params->gain = 35.0; /* dB */
    params->biasfactor = -7;
    params->asymmetry = -3500;
    params->impulse_model = 0;
    params->impulse_quality = 1;

    params->tone_bass = +3; /* dB */
    params->tone_middle = -10; /* dB */
    params->tone_treble = 0 ;

    /* configure the various stages */
    params->r_i[0] = 68e3 / 3000;
    params->r_k_p[0] = 2700 / 100000.0;
    set_chebyshev1_biquad(sample_rate * UPSAMPLE_RATIO, 22570, 0.0, TRUE, &params->lowpass[0]);
    set_rc_lowpass_biquad(sample_rate * UPSAMPLE_RATIO, 86, &params->biaslowpass[0]);
    set_rc_highpass_biquad(sample_rate * UPSAMPLE_RATIO, 37, &params->highpass[0]);
    
    params->r_i[1] = 250e3 / 3000;
    params->r_k_p[1] = 1500 / 100000.0;
    set_chebyshev1_biquad(sample_rate * UPSAMPLE_RATIO, 6531, 0.0, TRUE, &params->lowpass[1]);
    set_rc_lowpass_biquad(sample_rate * UPSAMPLE_RATIO, 132, &params->biaslowpass[1]);
    set_rc_highpass_biquad(sample_rate * UPSAMPLE_RATIO, 37, &params->highpass[1]);
    
    params->r_i[2] = 250e3 / 3000;
    params->r_k_p[2] = 820 / 1000000.0;
    set_chebyshev1_biquad(sample_rate * UPSAMPLE_RATIO, 6531, 0.0, TRUE, &params->lowpass[2]);
    set_rc_lowpass_biquad(sample_rate * UPSAMPLE_RATIO, 194, &params->biaslowpass[2]);
    set_rc_highpass_biquad(sample_rate * UPSAMPLE_RATIO, 37, &params->highpass[2]);
    
    params->r_i[3] = 250e3 / 3000;
    params->r_k_p[3] = 820 / 100000.0;
    set_chebyshev1_biquad(sample_rate * UPSAMPLE_RATIO, 6531, 0.0, TRUE, &params->lowpass[3]);
    set_rc_lowpass_biquad(sample_rate * UPSAMPLE_RATIO, 250, &params->biaslowpass[3]);
    set_rc_highpass_biquad(sample_rate * UPSAMPLE_RATIO, 37, &params->highpass[3]);

    set_chebyshev1_biquad(sample_rate * UPSAMPLE_RATIO, 12000, 10.0, TRUE, &params->decimation_filter);

#define STEEPNESS   3e-3
#define SCALE       1e2
#define STEEPNESS2  1e-2
#define SCALE2      5e-1
#define NONLINEARITY_MAX 1024           /* normalize table between -1 .. 1 */
    double y = 0.0;
    for (i = 0; i < NONLINEARITY_SIZE; i += 1) {
        int iter = 10000;
        /* Solve implicit equation
         * x - y = e^(-y / 10) / 10
         * for x values from -250 to 250. */
        double x = (i - NONLINEARITY_SIZE / 2) / (float) NONLINEARITY_PRECISION;
        while (--iter) {
            double value = x - y - SCALE * exp(STEEPNESS * y) + SCALE2 * exp(STEEPNESS2 * -y);
            double dvalue_y = -1 - (SCALE * STEEPNESS) * exp(STEEPNESS * y) - (SCALE2 * STEEPNESS2) * exp(STEEPNESS2 * -y);
            double dy = value / dvalue_y;
            y = (y + (y - dy)) / 2; /* average damp */

            if (fabs(value) < 1e-4)
                break;
        }
        if (iter == 0) {
            LOGD( "Failed to solve the nonlinearity equation for %f!\n", x);
        }
        nonlinearity[i] = y / NONLINEARITY_MAX;
        // printf("%d %f\n", i, nonlinearity[i]);
    }
    /* balance median to 0 */
    tmp = nonlinearity[NONLINEARITY_SIZE / 2];
    for (i = 0; i < NONLINEARITY_SIZE; i += 1)
        nonlinearity[i] -= tmp;

    for (i = 0; i < MAX_CHANNELS; i += 1)
        params->buf[i] = gnuitar_memalign(IMPULSE_SIZE * 2, sizeof(DSP_SAMPLE));

	OUT
	return (LV2_Handle) params;
}

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	if (port > 1)
		LOGD ("connect port: %d: %f\n", port, (float *) data);
	if (data == null)
		return ;

	TubeAmp * tubeamp = (TubeAmp *) instance ;
	float * d = (float *) data ;
	float f = * d ;
	int i = (int ) f ;
	switch ((PortIndex)port) {
		case STAGES:
			tubeamp->stages = i;
			break ;
		case INPUT:
			tubeamp -> input = (float *) data ;
			break ;
		case OUTPUT:
			tubeamp -> output = (float *) data ;
			break ;
		case IMPULSE_QUALITY:
			tubeamp->impulse_quality = i ;
			break ;
		case IMPULSE_MODEL:
			tubeamp->impulse_model = i ;
			break;
		case TONE_BASS:
			tubeamp->tone_bass = * (float *) data ;
			break;
		case TONE_MIDDLE:
			tubeamp->tone_middle = * (float *) data ;
			break;
		case TONE_TREBLE:
			tubeamp->tone_treble = * (float *) data ;
			break;
		case GAIN:
			tubeamp->gain = * (float *) data ;
			LOGD ("gain: %f\n", tubeamp->gain);
			break ;
		case ASYMMETRY:
			tubeamp->asymmetry = * (float *) data ;
			break ;
		case BIAS_FACTOR:
			tubeamp->biasfactor = * (float *) data ;
			break ;
	}
}


/* waveshaper based on generic lookup table */
static float
F_tube(float in, float r_i)
{
    float pos;
    int_fast32_t idx;
    
    pos = (in / r_i) * (float) (NONLINEARITY_SCALE * NONLINEARITY_PRECISION) + (float) (NONLINEARITY_SIZE / 2);
    
    /* This safety catch should be made unnecessary.
     * But it may require us to extend the nonlinearity table to ridiculously far.
     * Besides, hard blocking distortion is fairly ok as effect when you go too loud. */
    if (pos < 0.f) {
        //printf("pos < 0!");
        pos = 0.f;
    }
    if (pos > (float) (NONLINEARITY_SIZE - 2)) {
        //printf("pos > size!");
        pos = (float) (NONLINEARITY_SIZE - 2);
    }

    idx = pos;
    pos -= idx;
    return (nonlinearity[idx] * (1.0f-pos) + nonlinearity[idx+1] * pos) * r_i;
}


static void
activate(LV2_Handle instance) {
	
}


static void
run(LV2_Handle instance, uint32_t n_samples)
{
	TubeAmp * params = (TubeAmp *) instance;
	int_fast16_t i, j, k, curr_channel = 0;
    DSP_SAMPLE *ptr1;

	float gain;

	    printf ("asym %f bias %f\n", params->asymmetry, params->biasfactor);
    /* update bq states from tone controls */
    set_lsh_biquad(params->sample_rate * UPSAMPLE_RATIO, 500, params->tone_bass, &params->bq_bass);
    set_peq_biquad(params->sample_rate * UPSAMPLE_RATIO, 650, 500.0, params->tone_middle, &params->bq_middle);
    set_hsh_biquad(params->sample_rate * UPSAMPLE_RATIO, 800, params->tone_treble, &params->bq_treble);

	/* Note to Shaji
	 * this is how we calculate gain in dB
	 * I *think*
	 */
	
    gain = pow(10.f, params->gain / 20.f);
    //~ LOGD ("%f: %f\n", params->gain, gain);
    params->in[curr_channel] = params->input [0];
    
    /* highpass -> low shelf eq -> lowpass -> waveshaper */
    for (i = 0; i < n_samples; i += 1) {
        float result;
        params->output [i] = params->input [i];
        
        for (k = 0; k < UPSAMPLE_RATIO; k += 1) {
            /* IIR interpolation */
            params->in[curr_channel] = (params->output[i] + params->in[curr_channel] * (float) (UPSAMPLE_RATIO-1)) / (float) UPSAMPLE_RATIO;
            result = params->in[curr_channel] ;// (float) MAX_SAMPLE;
            for (j = 0; j < params->stages; j += 1) {
                /* gain of the block */
                result *= gain;
                /* low-pass filter that mimicks input capacitance */
                result = do_biquad(result, &params->lowpass[j], curr_channel);
                /* add feedback bias current for "punch" simulation for waveshaper */
                result = F_tube(params->bias[j] - result, params->r_i[j]);
                /* feedback bias */
                params->bias[j] = do_biquad((-3500 - -7 * result) * params->r_k_p[j], &params->biaslowpass[j], curr_channel);
                /* high pass filter to remove bias from the current stage */
                result = do_biquad(result, &params->highpass[j], curr_channel);
                
                /* run tone controls after second stage */
                if (j == 1) {
                    //~ result = do_biquad(result, &params->bq_bass, curr_channel);
                    //~ result = do_biquad(result, &params->bq_middle, curr_channel);
                    //~ result = do_biquad(result, &params->bq_treble, curr_channel);
                }
            }
            result = do_biquad(result, &params->decimation_filter, curr_channel);
        }

        ptr1 = params->buf[curr_channel] + params->bufidx[curr_channel];
        
        /* convolve the output. We put two buffers side-by-side to avoid & in loop. */
        ptr1[IMPULSE_SIZE] = ptr1[0] = result / 500.f * (float) (MAX_SAMPLE >> 13);
        params->output[i] = convolve(ampmodels[params->impulse_model].impulse, ptr1, ampqualities[params->impulse_quality].quality) / 32.f;
        
        params->bufidx[curr_channel] -= 1;
        if (params->bufidx[curr_channel] < 0)
            params->bufidx[curr_channel] += IMPULSE_SIZE;
        
        curr_channel = (curr_channel + 1) % 1;
    }

}


static void
deactivate(LV2_Handle instance) {

}

static void
cleanup(LV2_Handle instance) {
	free(instance);
}

static const void*
extension_data(const char* uri)
{
	return NULL;
}

static const LV2_Descriptor descriptor = {
	URI,
	instantiate,
	connect_port,
	activate,
	run,
	deactivate,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor*
lv2_descriptor(uint32_t index) {
	switch (index) {
        case 0:  
            return &descriptor;
        default: 
            return NULL;
	}
}
