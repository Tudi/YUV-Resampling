#include "LinearReSampler_SSE3.h"
#include <string.h>
#include <assert.h>
#include <tmmintrin.h>

#ifndef MAX
	#define  MAX(a,b)		(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
	#define  MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

//scale 16 input bytes into N(more then 16) output bytes
void UpSample1PlaneLiniarSSE3( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int SrcStride, int DestW, int DestH, int DestStride, unsigned char *RowMask16 )
{
	int int_conv_y = ( ( SrcH << FLOAT_PRECISSION_BITS ) / DestH );
	unsigned int stacking_precission_y = 0;
	int PrevSrcRow = -1;

	unsigned int int_conv_x = (unsigned int)( ( SrcW << FLOAT_PRECISSION_BITS ) / DestW );
	unsigned int int_conv_x_16 = 16 * int_conv_x;
	for( int y = 0; y < DestH; y++ )
	{
		int SrcRowNow = stacking_precission_y >> FLOAT_PRECISSION_BITS;
		stacking_precission_y += int_conv_y;
		if( SrcRowNow == PrevSrcRow )
			memcpy( dst+y*DestStride, dst+(y-1)*DestStride, DestW );
		else
		{
			unsigned char	*tdst = dst + y * DestStride;
			unsigned char	*tsrc = src + SrcRowNow * SrcStride;
			unsigned int	stacking_precission_x = 0;
			for( int x = 0; x < DestW; x += 16 ) 
			{ 
				__m128i BuffIn = _mm_loadu_si128((__m128i*)(tsrc+(stacking_precission_x>>FLOAT_PRECISSION_BITS))); 
				stacking_precission_x += int_conv_x_16;
				__m128i CopyMask = _mm_load_si128((__m128i*)(RowMask16 + x)); 
				__m128i BuffOut = _mm_shuffle_epi8( BuffIn, CopyMask ); 
				_mm_storeu_si128((__m128i*)(tdst+x),BuffOut); 
			} 
		}
		PrevSrcRow = SrcRowNow;
	}
}

//scale 16 input bytes into N(less then 16) output bytes
void DownSample1PlaneLiniarSSE3( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int SrcStride, int DestW, int DestH, int DestStride, unsigned char *RowMask16,unsigned char *Segments )
{
	int int_conv_y = ( ( SrcH << FLOAT_PRECISSION_BITS ) / DestH );
	unsigned int stacking_precission_y = 0;
	int PrevSrcRow = -1;

	unsigned int int_conv_x = (unsigned int)( ( DestW << FLOAT_PRECISSION_BITS ) / SrcW );
	unsigned int int_conv_x_16 = 16 * int_conv_x;
	for( int y = 0; y < DestH; y++ )
	{
		int SrcRowNow = stacking_precission_y >> FLOAT_PRECISSION_BITS;
		stacking_precission_y += int_conv_y;
		if( SrcRowNow == PrevSrcRow )
			memcpy( dst+y*DestStride, dst+(y-1)*DestStride, DestW );
		else
		{
			unsigned char	*tdst = dst + y * DestStride;
			unsigned char	*tsrc = src + SrcRowNow * SrcStride;
			unsigned char	*tseg = Segments; //really need to remake this when i have the time
			//load 16 bytes from src
			for( int x = 0; x < DestW; ) 
			{ 
				if( *tseg )
				{
					__m128i BuffIn = _mm_loadu_si128((__m128i*)(tsrc+0)); 
					tsrc += 16;
					__m128i CopyMask = _mm_loadu_si128((__m128i*)(RowMask16 + x)); 
					__m128i BuffOut = _mm_shuffle_epi8( BuffIn, CopyMask ); 
					_mm_storeu_si128((__m128i*)(tdst+x),BuffOut); 
					x += *tseg;
				}
				tseg++;
			} 
		}
		PrevSrcRow = SrcRowNow;
	}
}/**/

//width is the same. Height can be different
void DirectCopy1Plane( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int SrcStride, int DestW, int DestH, int DestStride )
{
	int ConversionRatioWidth1Pel = ( ( DestW << FLOAT_PRECISSION_BITS ) / SrcW );
	int ConversionRatioHeight1Pel = ( ( SrcH << FLOAT_PRECISSION_BITS ) / DestH );
	for( int y = 0; y < DestH; y++ )
	{
		int SrcRowNow = ( y * ConversionRatioHeight1Pel ) >> FLOAT_PRECISSION_BITS;
		memcpy( dst+y*DestStride, src+SrcStride, DestW );
	}
}

//this does not require row to be padded. Required for last ROW in case of float precision error
void ReSample1Row( unsigned char *src, unsigned char *dst, int SrcW, int DestW )
{
	unsigned int int_conv_x = (unsigned int)( ( SrcW << FLOAT_PRECISSION_BITS ) / DestW );
	unsigned int stacking_precission_x = 0;
	for( int x=0;x<DestW;x++)
	{
		int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
		dst[ x ] = src[ converted_col_index ];
		stacking_precission_x += int_conv_x;
	}
}

void ResampleYUV420LiniarSSE3( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int SrcStride, int DestW, int DestH, int DestStride, int DestBuffHeight, int DestStartY, int DestStartX )
{
	//disabled until something is actually working. Only tested with upsample rate 2 for col
	unsigned char *SrcY = src;
	unsigned char *DstY = dst + DestStartX + DestStartY * DestStride;
	unsigned char *SrcU = src + SrcStride * SrcH;
	unsigned char *DstU = dst + DestStride * DestBuffHeight + DestStartX / 2 + DestStartY / 2 * DestStride / 2;
	unsigned char *SrvV = SrcU + SrcStride / 2 * SrcH / 2;
	unsigned char *DstV = DstU + DestStride / 2 * DestBuffHeight / 2 + DestStartX / 2 + DestStartY / 2 * DestStride / 2;
	if( DestW > SrcW )
	{
		int ConversionRatioWidth1Pel = ( ( SrcW << FLOAT_PRECISSION_BITS ) / DestW );
		int ConversionRatioWidth16Pel = 16 * ConversionRatioWidth1Pel;
		//index vector : bytes positions : dest from src
		__declspec(align(16)) unsigned char RowMask16[8000];
		unsigned int MaxMaskUsed = DestW+( 16*ConversionRatioWidth1Pel >> FLOAT_PRECISSION_BITS );
#ifdef _DEBUG
		assert( MaxMaskUsed < 8000 );
#endif
		if( MaxMaskUsed > 8000 )
			return;
		int BaseShift = 0;

		for( int i=0;i<DestW+15;i+=16)
		{
			int SrcStartBase = ( i * ConversionRatioWidth1Pel ) >> FLOAT_PRECISSION_BITS;
			for( int j=0;j<16;j++ )
			{
				int SrcStart = ( ( i + j ) * ConversionRatioWidth1Pel ) >> FLOAT_PRECISSION_BITS;
				int SrcStartSub5 = SrcStart - SrcStartBase;
				RowMask16[i+j] = SrcStartSub5;
			}
			BaseShift += ConversionRatioWidth16Pel;
		}/**/
		UpSample1PlaneLiniarSSE3( SrcY, DstY, SrcW, SrcH, SrcStride, DestW, DestH, DestStride, RowMask16 );
		UpSample1PlaneLiniarSSE3( SrcU, DstU, SrcW / 2, SrcH / 2, SrcStride / 2, DestW / 2 , DestH / 2, DestStride / 2, RowMask16 );
		UpSample1PlaneLiniarSSE3( SrvV, DstV, SrcW / 2, SrcH / 2, SrcStride / 2, DestW / 2 , DestH / 2, DestStride / 2, RowMask16 );
	}
	else if( DestW < SrcW )
	{
		int ConversionRatioWidth1Pel = ( ( SrcW << FLOAT_PRECISSION_BITS ) / DestW );
		int ConversionRatioWidth16Pel = 16 * ConversionRatioWidth1Pel;
		//index vector : bytes positions : dest from src
		__declspec(align(16)) unsigned char RowMask16[8000];
		__declspec(align(16)) unsigned char Segments[8000]; //this is very lame, but i received a new task and have to close this
		unsigned int MaxMaskUsed = DestW+( 16*ConversionRatioWidth1Pel >> FLOAT_PRECISSION_BITS );
#ifdef _DEBUG
		assert( MaxMaskUsed < 8000 );
#endif
		if( MaxMaskUsed > 8000 )
			return;
		//take 16 bytes from src and generate X output
		int BaseShift = 0;
		int SegmentInd = 0;
		int prevSegmentInd = 0;
		for( signed int i=0;i<DestW;i++)
		{
			int converted_col_index = ( i * ConversionRatioWidth1Pel ) >> FLOAT_PRECISSION_BITS;
			if( converted_col_index - BaseShift >= 16 )
			{
				BaseShift = converted_col_index & ~0x0F;
				Segments[SegmentInd++] = i - prevSegmentInd;
				prevSegmentInd = i;
			}
			int SegmentedIndex = converted_col_index - BaseShift;
			RowMask16[i] = SegmentedIndex;
		}
		Segments[SegmentInd++] = 255;
		DownSample1PlaneLiniarSSE3( SrcY, DstY, SrcW, SrcH, SrcStride, DestW, DestH, DestStride, RowMask16, Segments );
		DownSample1PlaneLiniarSSE3( SrcU, DstU, SrcW / 2, SrcH / 2, SrcStride / 2, DestW / 2 , DestH / 2, DestStride / 2, RowMask16, Segments );
		DownSample1PlaneLiniarSSE3( SrvV, DstV, SrcW / 2, SrcH / 2, SrcStride / 2, DestW / 2 , DestH / 2, DestStride / 2, RowMask16, Segments );
	}
	else
	{
		if( SrcH == DestH )
		{
			memcpy( dst, src, SrcH * SrcW * 3 / 2 );
		}
		else
		{
			DirectCopy1Plane( SrcY, DstY, SrcW, SrcH, SrcStride, DestW, DestH, DestStride );
			DirectCopy1Plane( SrcU, DstU, SrcW / 2, SrcH / 2, SrcStride / 2, DestW / 2 , DestH / 2, DestStride / 2 );
			DirectCopy1Plane( SrvV, DstV, SrcW / 2, SrcH / 2, SrcStride / 2, DestW / 2 , DestH / 2, DestStride / 2 );
		}
	}
}

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// You need to use SSE friendly input and output buffer fro this. That means the size of the input + output buffer needs to be dividable by 16 !!
// SSE will read and write 16 byte blocks and if your buffer is too small it might lead to memory corruption
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void ResampleYUV420LiniarSSE3( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int DestW, int DestH, bool KeepAspectRatio  )
{
	if( KeepAspectRatio == true )
	{
		// using int for the sake of rounding
		int ScaleBoth;
		int ScaleWidth = ( DestW * FLOAT_PRECISSION ) / SrcW;
		int ScaleHeigth = ( DestH * FLOAT_PRECISSION ) / SrcH;
		if( ScaleWidth < ScaleHeigth )
			ScaleBoth = ScaleWidth;
		else
			ScaleBoth = ScaleHeigth;
		// calc up / down , left / right box size
		int NewHeight = MIN( ( SrcH * ScaleBoth + FLOAT_PRECISSION / 2 ) / FLOAT_PRECISSION, DestH );
		int NewWidth = MIN( ( SrcW * ScaleBoth + FLOAT_PRECISSION / 2 ) / FLOAT_PRECISSION, DestW );
		int RemainingGapSizeHeight = DestH - NewHeight;
		int RemainingGapSizeWidth = DestW - NewWidth;
		//apply boxes if needed. Worst case we could init the whole output image. That would be bugfree and slower in some cases
		if( RemainingGapSizeHeight > 0 )
		{
			AddBlackBoxYUV420( dst, DestW, DestH, 0, 0, DestW, RemainingGapSizeHeight / 2 );
			AddBlackBoxYUV420( dst, DestW, DestH, 0, RemainingGapSizeHeight / 2 + NewHeight, DestW, DestH - NewHeight - RemainingGapSizeHeight / 2 );
		}
		else if( RemainingGapSizeWidth > 0 )
		{
			AddBlackBoxYUV420( dst, DestW, DestH, 0, 0, RemainingGapSizeWidth / 2, DestH );
			AddBlackBoxYUV420( dst, DestW, DestH, RemainingGapSizeWidth / 2 + NewWidth, 0, DestW - NewWidth - RemainingGapSizeWidth / 2, DestH );
		} 
		// scale the picture in the remaining box
		ResampleYUV420LiniarSSE3( src, dst, SrcW, SrcH, SrcW, NewWidth, NewHeight, DestW, DestH, RemainingGapSizeWidth / 2, RemainingGapSizeHeight / 2 );
	}
	else 
		ResampleYUV420LiniarSSE3( src, dst, SrcW, SrcH, SrcW, DestW, DestH, DestW, DestH, 0, 0 );
}