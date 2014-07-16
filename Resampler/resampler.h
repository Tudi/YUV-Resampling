#ifndef __RESAMPLER_H__
#define __RESAMPLER_H__

#include <windows.h>
#include "2dlinearresampler.h"
#include "bilinearresampler.h"
#include "bicubicresampler.h"

typedef enum {RT_2DLINEAR, RT_BILINEAR, RT_BICUBIC} TResamplerType;
typedef enum {CM_UNCLIPPED, CM_LUMA, CM_CHROMA} TClipMode;

typedef struct {
	int sw, sh, dw, dh;
	TResamplerType rType;
	void *pResampler;
} TResampler;

#ifdef __cplusplus
extern "C" void ReleaseResampler(TResampler *resampler);
extern "C" void Resample(TResampler *resampler, BYTE *dst, BYTE *src, TClipMode clipmode = CM_UNCLIPPED);
extern "C" void ResampleRGB24(TResampler *resampler, BYTE *dst, BYTE *src, BYTE is_BMP=1);
extern "C" TResampler *CreateResampler(TResamplerType type, int dstWidth, int dstHeight, int srcWidth, int srcHeight);


//Hoi Ming YUV resizer
extern "C" void ResampleYUV420(TResampler *resampler, BYTE *dst_Y, BYTE *dst_U, BYTE *dst_V, 
							   BYTE *src_Y, BYTE *src_U, BYTE *src_V, BYTE is_BMP=1);
//end

//[jerry:2011-02-16]
//{
extern "C" void ResampleRGB24_2(TResampler *resampler, BYTE *dst, int dst_stride, BYTE *src, int src_stride);
extern "C" void ResampleYUV420_2(TResampler *resampler, BYTE *dst_Y, BYTE *dst_U, BYTE *dst_V, int dst_stride, BYTE *src_Y, BYTE *src_U, BYTE *src_V, int src_stride );
//}

#else
void void ReleaseResampler(TResampler *resampler);
void Resample(TResampler *resampler, BYTE *dst, BYTE *src, TClipMode clipmode = CM_UNCLIPPED);
void ResampleRGB24(TResampler *resampler, BYTE *dst, BYTE *src);
TResampler *CreateResampler(TResamplerType type, int dstWidth, int dstHeight, int srcWidth, int srcHeight);
#endif


#endif