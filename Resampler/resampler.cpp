#include "stdafx.h"
#include "resampler.h"

TResampler *CreateResampler(TResamplerType type, int dstWidth, int dstHeight, int srcWidth, int srcHeight)
{
	//TResampler *resampler = (TResampler*)MemAlloc(sizeof(TResampler));
	TResampler *resampler = (TResampler*)malloc(sizeof(TResampler));

	resampler->sw = srcWidth;
	resampler->sh = srcHeight;
	resampler->dw = dstWidth;
	resampler->dh = dstHeight;

	resampler->rType = RT_BILINEAR;

	//resampler->rType = type;
	resampler->pResampler = (void*)new CBilinearResampler(srcWidth, srcHeight, dstWidth, dstHeight);
	/*switch(type)
	{
		case RT_2DLINEAR:
			resampler->pResampler = (void*)new C2DLinearResampler(srcWidth, srcHeight, dstWidth, dstHeight);														
			break;
		case RT_BILINEAR:
			resampler->pResampler = (void*)new CBilinearResampler(srcWidth, srcHeight, dstWidth, dstHeight);
			break;
		case RT_BICUBIC:
			resampler->pResampler = (void*)new CBicubicResampler(srcWidth, srcHeight, dstWidth, dstHeight);
			break;
	}*/

	return resampler;
}

void ReleaseResampler(TResampler *resampler)
{
	switch(resampler->rType)
	{
		case RT_2DLINEAR:
			delete (C2DLinearResampler*)resampler->pResampler;
			break;
		case RT_BILINEAR:
			delete (CBilinearResampler*)resampler->pResampler;
			break;
		case RT_BICUBIC:
			delete (CBicubicResampler*)resampler->pResampler;
			break;
	}
	
	//delete resampler;
	//MemFree( resampler );
	//MemFree( resampler );
	free(resampler );
}

void Resample(TResampler *resampler, BYTE *dst, BYTE *src, TClipMode clipmode)
{
	switch(resampler->rType)
	{
	case RT_2DLINEAR:
		((C2DLinearResampler*)resampler->pResampler)->Resample(src, resampler->sw, dst, resampler->dw, clipmode);
		break;
	case RT_BILINEAR:
		((CBilinearResampler*)resampler->pResampler)->Resample(src, resampler->sw, dst, resampler->dw, clipmode);
		break;
	case RT_BICUBIC:
		((CBicubicResampler*)resampler->pResampler)->Resample(src, resampler->sw, dst, resampler->dw, clipmode);
		break;
	}
}

void ResampleRGB24(TResampler *resampler, BYTE *dst, BYTE *src, BYTE is_BMP)
{
	// ewlee 20090703 - add Jozsa's fix (XLE can not resize correctly when input width is not devided by 16.
	//this is BMP row srtide : row must be dividable by 4 always. Padding is added to obtain the dividable number
	int src_stride = resampler->sw*3;
	//if( is_BMP && src_stride % 4 )
	//	src_stride += 4 - src_stride % 4;

	switch(resampler->rType)
	{
	case RT_2DLINEAR:
		((C2DLinearResampler*)resampler->pResampler)->ResampleRGB24(src, src_stride, dst, 3*resampler->dw, CM_UNCLIPPED);
		break;
	case RT_BILINEAR:
		((CBilinearResampler*)resampler->pResampler)->ResampleRGB24(src, src_stride, dst, 3*resampler->dw, CM_UNCLIPPED);
		break;
	/*case RT_BICUBIC:
		((CBicubicResampler*)resampler->pResampler)->Resample(src, resampler->sw, dst, resampler->dw, clipmode);
		break;*/
	}
}
void ResampleRGB24_2(TResampler *resampler, BYTE *dst, int dst_stride, BYTE *src, int src_stride)
{
	switch(resampler->rType)
	{
	case RT_2DLINEAR:
		((C2DLinearResampler*)resampler->pResampler)->ResampleRGB24(src, src_stride, dst, dst_stride, CM_UNCLIPPED);
		break;
	case RT_BILINEAR:
		((CBilinearResampler*)resampler->pResampler)->ResampleRGB24(src, src_stride, dst, dst_stride, CM_UNCLIPPED);
		break;	
	}
}


//Hoi Ming YUV resizer
void ResampleYUV420(TResampler *resampler, BYTE *dst_Y, BYTE *dst_U, BYTE *dst_V, 
							   BYTE *src_Y, BYTE *src_U, BYTE *src_V, BYTE is_BMP)
{
	// ewlee 20090703 - add Jozsa's fix (XLE can not resize correctly when input width is not devided by 16.
	//this is BMP row srtide : row must be dividable by 4 always. Padding is added to obtain the dividable number
	int src_stride = resampler->sw;
	//if( is_BMP && src_stride % 4 )
	//	src_stride += 4 - src_stride % 4;

	((CBilinearResampler*)resampler->pResampler)->ResampleYUV420(src_Y, src_U, src_V, src_stride, 
		dst_Y, dst_U, dst_V, resampler->dw, CM_UNCLIPPED);
}

void ResampleYUV420_2(TResampler *resampler, BYTE *dst_Y, BYTE *dst_U, BYTE *dst_V, int dst_stride,
					BYTE *src_Y, BYTE *src_U, BYTE *src_V, int src_stride )
{
	((CBilinearResampler*)resampler->pResampler)->ResampleYUV420(src_Y, src_U, src_V, src_stride, dst_Y, dst_U, dst_V, dst_stride, CM_UNCLIPPED);
}