#ifndef _MAIN_H_
#define _MAIN_H_

#include "../Resampler/LinearReSampler.h"
#include "../Resampler/LinearReSampler_SSE3.h"
#include "../Resampler/resampler.h"
#include "../Resampler/RatioResampler2.h"
#include "YUVLib.h"

#ifdef _DEBUG
	#define TEST_REPEAT_COUNT 1
#else
	#define TEST_REPEAT_COUNT 2000
#endif

#define MAX_WIDTH		4920
#define MAX_HEIGHT		4920

int DoBenchmark1( int InWidth, int InHeight, int OutWidth, int OutHeight, char *Src, char *Dst, char *Conformance );
int DoBenchmark2( int InWidth, int InHeight, int OutWidth, int OutHeight, char *Src, char *Dst, char *Conformance );

#endif