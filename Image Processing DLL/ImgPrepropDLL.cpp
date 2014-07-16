// ImgPrepropDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ImgPrepropDLL.h"
#include <tmmintrin.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <Windows.h>

#ifndef CLIP3
	#define CLIP3(a,b,c)	((c) < (a) ? (a) : ((c) > (b) ? (b) : (c))) // MIN, MAX, val
#endif

#define DIV_PRECISSION_BITS	12
#define DIV_PRECISION_VALUE ( 1 << DIV_PRECISSION_BITS )

//!!!warning this might is slow and create a huuuge log file. It is a temp implementation for confirming a crash
//#define ENABLE_CALL_LOGGING
#ifdef ENABLE_CALL_LOGGING
FILE *f=fopen("CallLog.txt","at");
	#define LOG_FUNCTION_ENTER( F_NAME ) { \
		if( f ) \
		{ \
			fprintf( f, "%s : Enter", F_NAME ); \
			fflush( f ); \
		} \
	}
	#define LOG_FUNCTION_EXIT( F_NAME ) { \
		if( f ) \
		{ \
			fprintf( f, " - Exit\n", F_NAME ); \
			fflush( f ); \
		} \
	}
#else
	#define LOG_FUNCTION_ENTER( F_NAME )
	#define LOG_FUNCTION_EXIT( F_NAME )
#endif

// This is an example of an exported function.
IMGPREPROPDLL_API int APIENTRY ConvertYV12toRGB(int width, int height, int src_stride, unsigned char *Src_IYUV, int Src_IYUV_Size, unsigned char *Dst_RGB, int Dst_RGB_Size, int targetStride)
{
	if( Src_IYUV == NULL || Dst_RGB == NULL )
		return ERR_BUFFER_NULL;
	//SSE friendly parameters ?
	if( width <= 1 || height <= 1)
		return ERR_INVALID_SIZE;
	//width needs to be at least the size of the stride
	if( width > src_stride  )
		return ERR_STRIDE_MISSMATCH;
	//we expect the output to fit in 24 bit / pixel
	if( width*3 > targetStride  )
		return ERR_STRIDE_MISSMATCH;

	LOG_FUNCTION_ENTER( "ConvertYV12toRGB" )

	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) char c_shufflemask[16] = { 0,1,2,4,5,6,8,9,10,12,13,14 };
	int SSEHeight = height & (~0x01);
	int SSEWidth = (width-8) & (~0x07); //write 32 bytes instead 24(12+12) leads to bad output
	for( j = 0; j < SSEHeight; j+=2)
	{
		for( i = 0; i < SSEWidth; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sY2, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi, sY300Lo2, sY300Hi2;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi, sUVLo, sUVHi;
			__declspec(align(16)) __m128i R[4],G[4],B[4];
			__declspec(align(16)) __m128i Const16;
			__declspec(align(16)) __m128i Const128;

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sY2 = _mm_loadl_epi64((__m128i*)(&py[i+src_stride]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));

			Const16 = _mm_set1_epi16(16);
			Const128 = _mm_set1_epi16(128);
			temp0 = _mm_set1_epi16(300);

			sY = _mm_unpacklo_epi8(sY, zero);
			sY2 = _mm_unpacklo_epi8(sY2, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_sub_epi16(sY, Const16);
			sY2 = _mm_sub_epi16(sY2, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp1 = _mm_mullo_epi16(sY2, temp0);
			temp2 = _mm_mulhi_epi16(sY2, temp0);
			sY300Lo2 = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi2 = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi32(128);
			sY300Lo = _mm_add_epi32(sY300Lo, temp0);
			sY300Hi = _mm_add_epi32(sY300Hi, temp0);
			sY300Lo2 = _mm_add_epi32(sY300Lo2, temp0);
			sY300Hi2 = _mm_add_epi32(sY300Hi2, temp0);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			sUVLo = _mm_add_epi32(sV208Lo, sU100Lo);
			sUVHi = _mm_add_epi32(sV208Hi, sU100Hi);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[1] = _mm_packus_epi16(AccLo, zero);

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_sub_epi32(sY300Lo2, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi2, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[1] = _mm_packus_epi16(AccLo, zero);

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[1] = _mm_packus_epi16(AccLo, zero);

			R[2] = _mm_unpacklo_epi8(R[0], G[0]);
			R[3] = _mm_unpacklo_epi8(R[1], G[1]);
			B[2] = _mm_unpacklo_epi8(B[0], zero);
			B[3] = _mm_unpacklo_epi8(B[1], zero);
			__declspec(align(16)) __m128i RGB32Lo[2],RGB32Hi[2];
			RGB32Lo[0] = _mm_unpacklo_epi16(R[2], B[2]);
			RGB32Hi[0] = _mm_unpackhi_epi16(R[2], B[2]);
			RGB32Lo[1] = _mm_unpacklo_epi16(R[3], B[3]);
			RGB32Hi[1] = _mm_unpackhi_epi16(R[3], B[3]);
			__declspec(align(16)) __m128i shufflemask;
			shufflemask = _mm_load_si128((__m128i*)c_shufflemask);
			__declspec(align(16)) __m128i pix1234,pix5678,pix1234_,pix5678_;
			pix1234 = _mm_shuffle_epi8(RGB32Lo[0], shufflemask);
			pix5678 = _mm_shuffle_epi8(RGB32Hi[0], shufflemask);
			pix1234_ = _mm_shuffle_epi8(RGB32Lo[1], shufflemask);
			pix5678_ = _mm_shuffle_epi8(RGB32Hi[1], shufflemask);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+0], pix1234);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+12], pix5678);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+targetStride+0], pix1234_);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+targetStride+12], pix5678_);
		}
		for( ;i < width; i++)
		{
			int Y, Y2, U, V, R, G, B, R2, G2, B2;
			int iX3 = i+i+i;
			Y = py[i];
			Y2 = py[i+src_stride];
			U = pu[i/2];
			V = pv[i/2];
			int T1 = 517*(U-128) + 128;
			int T2 = -208*(V-128) - 100*(U-128) + 128;
			int T3 = 409*(V-128) + 128;
			R = (300*(Y-16) + T1 ) / 256;
			G = (300*(Y-16) + T2 ) / 256;
			B = (300*(Y-16) + T3 ) / 256;
			R2 = (300*(Y2-16) + T1 ) / 256;
			G2 = (300*(Y2-16) + T2 ) / 256;
			B2 = (300*(Y2-16) + T3 ) / 256;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, R);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, B);
			Dst_RGB[targetStride + iX3 + 0] = CLIP3(0, 255, R2);
			Dst_RGB[targetStride + iX3 + 1] = CLIP3(0, 255, G2);
			Dst_RGB[targetStride + iX3 + 2] = CLIP3(0, 255, B2);
		}/**/
		py += (src_stride*2);
		pu += (src_stride/2);
		pv += (src_stride/2);
		Dst_RGB += (targetStride*2);
	}
	if( height & 0x01 )
	{
		for( i=0; i < width; i++)
		{
			int Y, U, V, R, G, B;
			int iX3 = i+i+i;
			Y = py[i];
			U = pu[i/2];
			V = pv[i/2];
			R = (300*(Y-16) + 517*(U-128) + 128 ) / 256;
			G = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) / 256;
			B = (300*(Y-16) + 409*(V-128) + 128 ) / 256;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, R);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, B);
		}
	}/**/

	LOG_FUNCTION_EXIT( "ConvertYV12toRGB" )

	return 0;
}


IMGPREPROPDLL_API int APIENTRY ConvertYV12toBGR(int width, int height, int src_stride, unsigned char *Src_IYUV, int Src_IYUV_Size, unsigned char *Dst_RGB, int Dst_RGB_Size, int targetStride)
{
	if( Src_IYUV == NULL || Dst_RGB == NULL )
		return ERROR_INVALID_ADDRESS;
	//SSE friendly parameters ?
	if( width <= 1 || height <= 1 || src_stride <= 1 )
		return ERROR_INVALID_PARAMETER;
	//width needs to be at least the size of the stride
	if( width > src_stride  )
		return ERROR_INVALID_PARAMETER;
	//we expect the output to fit in 24 bit / pixel
	if( width*3 > targetStride  )
		return ERROR_INVALID_PARAMETER;
	//check YUV Buffer size
	if(Src_IYUV_Size < (width*height * 1.5))
		return ERROR_INVALID_PARAMETER;
	//check Dest Buffer
	if (Dst_RGB_Size< (targetStride*height))
		return ERROR_INVALID_PARAMETER;

	LOG_FUNCTION_ENTER( "ConvertYV12toBGR" )

	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) char c_shufflemask[16] = { 0,1,2,4,5,6,8,9,10,12,13,14 };
	int SSEHeight = height & (~0x01);
	int SSEWidth = (width-8) & (~0x07); //write 32 bytes instead 24(12+12) leads to bad output
	for( j = 0; j < SSEHeight; j+=2)
	{
		for( i = 0; i < SSEWidth; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sY2, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi, sY300Lo2, sY300Hi2;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi, sUVLo, sUVHi;
			__declspec(align(16)) __m128i R[4],G[4],B[4];
			__declspec(align(16)) __m128i Const16;
			__declspec(align(16)) __m128i Const128;

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sY2 = _mm_loadl_epi64((__m128i*)(&py[i+src_stride]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));

			Const16 = _mm_set1_epi16(16);
			Const128 = _mm_set1_epi16(128);
			temp0 = _mm_set1_epi16(300);

			sY = _mm_unpacklo_epi8(sY, zero);
			sY2 = _mm_unpacklo_epi8(sY2, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_sub_epi16(sY, Const16);
			sY2 = _mm_sub_epi16(sY2, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp1 = _mm_mullo_epi16(sY2, temp0);
			temp2 = _mm_mulhi_epi16(sY2, temp0);
			sY300Lo2 = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi2 = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi32(128);
			sY300Lo = _mm_add_epi32(sY300Lo, temp0);
			sY300Hi = _mm_add_epi32(sY300Hi, temp0);
			sY300Lo2 = _mm_add_epi32(sY300Lo2, temp0);
			sY300Hi2 = _mm_add_epi32(sY300Hi2, temp0);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			sUVLo = _mm_add_epi32(sV208Lo, sU100Lo);
			sUVHi = _mm_add_epi32(sV208Hi, sU100Hi);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[1] = _mm_packus_epi16(AccLo, zero);

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_sub_epi32(sY300Lo2, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi2, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[1] = _mm_packus_epi16(AccLo, zero);

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[1] = _mm_packus_epi16(AccLo, zero);

			B[2] = _mm_unpacklo_epi8(B[0], G[0]); //R is swapped with B !
			B[3] = _mm_unpacklo_epi8(B[1], G[1]); //R is swapped with B !
			R[2] = _mm_unpacklo_epi8(R[0], zero);
			R[3] = _mm_unpacklo_epi8(R[1], zero); //R is swapped with B !
			__declspec(align(16)) __m128i RGB32Lo[2],RGB32Hi[2];
			RGB32Lo[0] = _mm_unpacklo_epi16(B[2], R[2]);
			RGB32Hi[0] = _mm_unpackhi_epi16(B[2], R[2]);
			RGB32Lo[1] = _mm_unpacklo_epi16(B[3], R[3]);
			RGB32Hi[1] = _mm_unpackhi_epi16(B[3], R[3]);
			__declspec(align(16)) __m128i shufflemask;
			shufflemask = _mm_load_si128((__m128i*)c_shufflemask);
			__declspec(align(16)) __m128i pix1234,pix5678,pix1234_,pix5678_;
			pix1234 = _mm_shuffle_epi8(RGB32Lo[0], shufflemask);
			pix5678 = _mm_shuffle_epi8(RGB32Hi[0], shufflemask);
			pix1234_ = _mm_shuffle_epi8(RGB32Lo[1], shufflemask);
			pix5678_ = _mm_shuffle_epi8(RGB32Hi[1], shufflemask);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+0], pix1234);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+12], pix5678);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+targetStride+0], pix1234_);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+targetStride+12], pix5678_);
		}
		for( ;i < width; i++)
		{
			int Y, Y2, U, V, R, G, B, R2, G2, B2;
			int iX3 = i+i+i;
			Y = py[i];
			Y2 = py[i+src_stride];
			U = pu[i/2];
			V = pv[i/2];
			int T1 = 517*(U-128) + 128;
			int T2 = -208*(V-128) - 100*(U-128) + 128;
			int T3 = 409*(V-128) + 128;
			R = (300*(Y-16) + T1 ) / 256;
			G = (300*(Y-16) + T2 ) / 256;
			B = (300*(Y-16) + T3 ) / 256;
			R2 = (300*(Y2-16) + T1 ) / 256;
			G2 = (300*(Y2-16) + T2 ) / 256;
			B2 = (300*(Y2-16) + T3 ) / 256;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, B);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, R);
			Dst_RGB[targetStride + iX3 + 0] = CLIP3(0, 255, B2);
			Dst_RGB[targetStride + iX3 + 1] = CLIP3(0, 255, G2);
			Dst_RGB[targetStride + iX3 + 2] = CLIP3(0, 255, R2);
		}/**/
		py += (src_stride*2);
		pu += (src_stride/2);
		pv += (src_stride/2);
		Dst_RGB += (targetStride*2);
	}
	if( height & 0x01 )
	{
		for( i=0; i < width; i++)
		{
			int Y, U, V, R, G, B;
			int iX3 = i+i+i;
			Y = py[i];
			U = pu[i/2];
			V = pv[i/2];
			R = (300*(Y-16) + 517*(U-128) + 128 ) / 256;
			G = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) / 256;
			B = (300*(Y-16) + 409*(V-128) + 128 ) / 256;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, B);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, R);
		}
	}/**/

	LOG_FUNCTION_EXIT( "ConvertYV12toBGR" )

	return 0;
}

IMGPREPROPDLL_API int APIENTRY ConvertRGBtoGrayscale(int width, int height, int src_stride, unsigned char *Src_RGB, int GrayscaleLevels)
{
	unsigned char *Dst_RGB = Src_RGB;
	int targetStride = src_stride;

	if( Src_RGB == NULL )
		return ERROR_INVALID_ADDRESS;
	//SSE friendly parameters ?
	if( width <= 1 || height <= 1 || src_stride <= 1 )
		return ERROR_INVALID_PARAMETER;
	//width needs to be at least the size of the stride
	if( width * 3 > src_stride  )
		return ERROR_INVALID_PARAMETER;
	//width needs to be at least the size of the stride
	if( GrayscaleLevels > 255 || GrayscaleLevels < 1  )
		return ERROR_INVALID_PARAMETER;

	LOG_FUNCTION_ENTER( "ConvertRGBtoGrayscale" )

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();

	int multiplier = ( DIV_PRECISION_VALUE * GrayscaleLevels ) / ( 256 * 3 );
	__declspec(align(16)) __m128i InvMull;
	InvMull = _mm_set1_epi16( multiplier );

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,3,6,9,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 };
	__declspec(align(16)) unsigned char c_shufflemask2[16] = { 1,4,7,10,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 };
	__declspec(align(16)) unsigned char c_shufflemask3[16] = { 2,5,8,11,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 };
	__declspec(align(16)) unsigned char c_shufflemask4[16] = { 0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5 };
	__declspec(align(16)) unsigned char c_shufflemask5[16] = { 5,5,6,6,6,7,7,7,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80 };
	__declspec(align(16)) __m128i shufflemask1,shufflemask2,shufflemask3;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);
	shufflemask2 = _mm_load_si128((__m128i*)c_shufflemask2);
	shufflemask3 = _mm_load_si128((__m128i*)c_shufflemask3);

	int SSEWidth = ( width * 3 ) & ~0x07;
	for( int j = 0; j < height; j++)
	{
		int i;
		for( i = 0; i < SSEWidth; i += 3*8 )
		{
			__declspec(align(16)) __m128i RGB1,RGB2,R1,G1,B1,RGBL,RGBH,temp1,temp2,R2,G2,B2,RGBL2,RGBH2;
			//load up 8 values
			RGB1 = _mm_loadu_si128((__m128i*)(&Src_RGB[i]));
			RGB2 = _mm_loadu_si128((__m128i*)(&Src_RGB[i+12]));
			//filter used values
			R1 = _mm_shuffle_epi8(RGB1, shufflemask1);
			G1 = _mm_shuffle_epi8(RGB1, shufflemask2);
			B1 = _mm_shuffle_epi8(RGB1, shufflemask3);
			R2 = _mm_shuffle_epi8(RGB2, shufflemask1);
			G2 = _mm_shuffle_epi8(RGB2, shufflemask2);
			B2 = _mm_shuffle_epi8(RGB2, shufflemask3);
			//unpack
			R1 = _mm_unpacklo_epi8(R1, zero);
			G1 = _mm_unpacklo_epi8(G1, zero);
			B1 = _mm_unpacklo_epi8(B1, zero);
			R2 = _mm_unpacklo_epi8(R2, zero);
			G2 = _mm_unpacklo_epi8(G2, zero);
			B2 = _mm_unpacklo_epi8(B2, zero);
			//add them up
			RGBL = _mm_add_epi16(R1, G1);
			RGBL2 = _mm_add_epi16(R2, G2);
			RGBL = _mm_add_epi16(RGBL, B1);
			RGBL2 = _mm_add_epi16(RGBL2, B2);
			//divide to get only N levels
			temp1 = _mm_mullo_epi16(RGBL, InvMull);
			temp2 = _mm_mulhi_epi16(RGBL, InvMull);
			RGBL = _mm_unpacklo_epi16(temp1, temp2);
			RGBH = _mm_unpackhi_epi16(temp1, temp2);
			RGBL = _mm_srai_epi32(RGBL, DIV_PRECISSION_BITS);
			RGBH = _mm_srai_epi32(RGBH, DIV_PRECISSION_BITS);

			temp1 = _mm_mullo_epi16(RGBL2, InvMull);
			temp2 = _mm_mulhi_epi16(RGBL2, InvMull);
			RGBL2 = _mm_unpacklo_epi16(temp1, temp2);
			RGBH2 = _mm_unpackhi_epi16(temp1, temp2);
			RGBL2 = _mm_srai_epi32(RGBL2, DIV_PRECISSION_BITS);
			RGBH2 = _mm_srai_epi32(RGBH2, DIV_PRECISSION_BITS);
			//pack to smaller type
			RGBL = _mm_packs_epi32(RGBL, RGBL2);
			//scale up again
			__declspec(align(16)) __m128i Mull;
			Mull = _mm_set1_epi16( GrayscaleLevels );
			temp1 = _mm_mullo_epi16(RGBL, Mull);
			RGBL = _mm_packus_epi16(temp1, zero);
			__declspec(align(16)) __m128i bloatmask1,bloatmask2;
			bloatmask1 = _mm_load_si128((__m128i*)c_shufflemask4);
			bloatmask2 = _mm_load_si128((__m128i*)c_shufflemask5);
			R1 = _mm_shuffle_epi8(RGBL, bloatmask1);
			G1 = _mm_shuffle_epi8(RGBL, bloatmask2);
			//store
			_mm_storeu_si128((__m128i*)&Dst_RGB[i+0], R1);
			_mm_storel_epi64((__m128i*)&Dst_RGB[i+16], G1);
		}
		for( ; i < width*3; i += 3 )
		{
			int R,G,B;
			R = Src_RGB[i+0];
			G = Src_RGB[i+1];
			B = Src_RGB[i+2];
			int res = (R+G+B)*multiplier;
			res = res >> DIV_PRECISSION_BITS;
			res = res * GrayscaleLevels;
			Dst_RGB[i+0] = res;
			Dst_RGB[i+1] = res;
			Dst_RGB[i+2] = res;
		}
		Src_RGB += src_stride;
		Dst_RGB += targetStride;
	}

	LOG_FUNCTION_EXIT( "ConvertRGBtoGrayscale" )

	return 0;
}

IMGPREPROPDLL_API float APIENTRY CompareBitmapRegions(int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, unsigned char *Src_RGB, char HighlightPixels )
{

	if( Src_RGB == NULL || Ref_RGB == NULL )
		return ERROR_INVALID_ADDRESS;
	//SSE friendly parameters ?
	if( width <= 1 || height <= 1 || src_stride <= 1 )
		return ERROR_INVALID_PARAMETER;
	//width needs to be at least the size of the stride
	if( width * 3 > src_stride  )
		return ERROR_INVALID_PARAMETER;
	//width needs to be at least the size of the stride	
	if( Startx < 0 || Starty < 0 || BoxWidth <= 0 || BoxHeight <= 0 || Startx + BoxWidth > width || Starty + BoxHeight > height )
		return ERROR_INVALID_PARAMETER;

	LOG_FUNCTION_ENTER( "CompareBitmapRegions" )

	int PixelMatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,0x80,3,0x80,6,0x80,9,0x80,12,0x80,0x80,0x80,0x80,0x80,0x80,0x80 };
	__declspec(align(16)) __m128i shufflemask1;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);

	int SSEWidth = ( ( BoxWidth / 5 ) * 5 ) * 3;
	for( int j = 0; j < BoxHeight; j++)
	{
		int i;
		__declspec(align(16)) __m128i SumDiff;
		SumDiff = _mm_setzero_si128();
		for( i = 0; i < SSEWidth; i += 3*5 )
		{
			__declspec(align(16)) __m128i SR,SG,SB,RR,RG,RB,T,T1;
			//load up 8 values from sec
			SR = _mm_loadu_si128((__m128i*)(&Src_RGB[i+0]));
			SG = _mm_loadu_si128((__m128i*)(&Src_RGB[i+1]));
			SB = _mm_loadu_si128((__m128i*)(&Src_RGB[i+2]));
			//load up 8 values from ref
			RR = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+0]));
			RG = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+1]));
			RB = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+2]));

			//0xFF - match, 0x00 - no match
			T = _mm_cmpeq_epi8(SR,RR);
			SG = _mm_cmpeq_epi8(SG,RG);
			SB = _mm_cmpeq_epi8(SB,RB);

			//see if all 3 match : 0xFF - match, 0x00 - no match
			T = _mm_and_si128(T,SG);
			T = _mm_and_si128(T,SB);

			//we are interested in every 3rd value only
			T1 = _mm_shuffle_epi8(T, shufflemask1);
			if( HighlightPixels )
			{
				if( T.m128i_i8[0] )
					Src_RGB[i+0] = 255;
				if( T.m128i_i8[3] )
					Src_RGB[i+3] = 255;
				if( T.m128i_i8[6] )
					Src_RGB[i+6] = 255;
				if( T.m128i_i8[9] )
					Src_RGB[i+9] = 255;
				if( T.m128i_i8[12] )
					Src_RGB[i+12] = 255;
			}

			//1 bit if match
			T = _mm_set1_epi8( 1 );
			T = _mm_and_si128(T, T1);
			SumDiff = _mm_add_epi16(T, SumDiff);
		}
		for( ; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ i + 0 ] == Ref_RGB[ i + 0 ]
				&& Src_RGB[ i + 1 ] == Ref_RGB[ i + 1 ]
				&& Src_RGB[ i + 2 ] == Ref_RGB[ i + 2 ] )
				{
					PixelMatches++;
					if( HighlightPixels )
						Src_RGB[ i + 0 ] = 255;
				}
		Src_RGB += src_stride;
		Ref_RGB += src_stride;
		__declspec(align(16)) unsigned short tstore[16];
		_mm_storeu_si128((__m128i*)(tstore), SumDiff);
		PixelMatches += tstore[0]+tstore[1]+tstore[2]+tstore[3]+tstore[4];
	}
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	
	float ret = ((float)(PixelsToInvestigate-PixelMatches)/(float)PixelsToInvestigate);
	LOG_FUNCTION_EXIT( "CompareBitmapRegions" )
	
		return ret;
}

IMGPREPROPDLL_API float APIENTRY CompareBGRBitmapRegions(int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, unsigned char *Src_RGB, char HighlightPixels )
{
	if( Src_RGB == NULL || Ref_RGB == NULL )
		return ERROR_INVALID_ADDRESS;
	//SSE friendly parameters ?
	if( width <= 1 || height <= 1 || src_stride <= 1 )
		return ERROR_INVALID_PARAMETER;
	//width needs to be at least the size of the stride
	if( width * 3 > src_stride  )
		return ERROR_INVALID_PARAMETER;
	//width needs to be at least the size of the stride	
	if( Startx < 0 || Starty < 0 || BoxWidth <= 0 || BoxHeight <= 0 || Startx + BoxWidth > width || Starty + BoxHeight > height )
		return ERROR_INVALID_PARAMETER;

	LOG_FUNCTION_ENTER( "CompareBGRBitmapRegions" )

	int PixelMatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,0xF0,3,0xF0,6,0xF0,9,0xF0,12,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask1;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);

	int SSEWidth = ( ( BoxWidth / 5) * 5 ) * 3;
	for( int j = 0; j < BoxHeight; j++)
	{
		int i;
		__declspec(align(16)) __m128i SumDiff;
		SumDiff = _mm_setzero_si128();
		for( i = 0; i < SSEWidth; i += 3*5 )
		{
			__declspec(align(16)) __m128i SR,SG,SB,RR,RG,RB,T,T1;
			//load up 8 values from sec
			SB = _mm_loadu_si128((__m128i*)(&Src_RGB[i+0]));
			SG = _mm_loadu_si128((__m128i*)(&Src_RGB[i+1]));
			SR = _mm_loadu_si128((__m128i*)(&Src_RGB[i+2]));
			//load up 8 values from ref
			RB = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+0]));
			RG = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+1]));
			RR = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+2]));

			//0xFF - match, 0x00 - no match
			T = _mm_cmpeq_epi8(SR,RR);
			SG = _mm_cmpeq_epi8(SG,RG);
			SB = _mm_cmpeq_epi8(SB,RB);

			//see if all 3 match : 0xFF - match, 0x00 - no match
			T = _mm_and_si128(T,SG);
			T = _mm_and_si128(T,SB);

			//we are interested in every 3rd value only
			T1 = _mm_shuffle_epi8(T, shufflemask1);
			if( HighlightPixels )
			{
				if( T.m128i_i8[2] )
					Src_RGB[i+2] = 255;
				if( T.m128i_i8[5] )
					Src_RGB[i+5] = 255;
				if( T.m128i_i8[8] )
					Src_RGB[i+8] = 255;
				if( T.m128i_i8[11] )
					Src_RGB[i+11] = 255;
				if( T.m128i_i8[14] )
					Src_RGB[i+14] = 255;
			}

			//1 bit if match
			T = _mm_set1_epi8( 1 );
			T = _mm_and_si128(T, T1);
			SumDiff = _mm_add_epi16(T, SumDiff);
		}
		for( ; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ i + 0 ] == Ref_RGB[ i + 0 ]
				&& Src_RGB[ i + 1 ] == Ref_RGB[ i + 1 ]
				&& Src_RGB[ i + 2 ] == Ref_RGB[ i + 2 ] )
				{
					PixelMatches++;
					if( HighlightPixels )
						Src_RGB[ i + 2 ] = 255;
				}
		Src_RGB += src_stride;
		Ref_RGB += src_stride;
		__declspec(align(16)) unsigned short tstore[16];
		_mm_storeu_si128((__m128i*)(tstore), SumDiff);
		PixelMatches += tstore[0]+tstore[1]+tstore[2]+tstore[3]+tstore[4];
	}
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	
	float ret = ((float)(PixelsToInvestigate-PixelMatches)/(float)PixelsToInvestigate); 

	LOG_FUNCTION_EXIT( "CompareBGRBitmapRegions" )

	return ret;
}

void APIENTRY InsertBuffer(unsigned char *targetBufferYUV, int targetBufferSize, int targetWidth, int targetHeight, int targetStride, unsigned char *sourceBufferYUV, int sourceBufferSize, int sourceWidth, int sourceHeight, int Src_stride, int Dst_start_x, int Dst_start_y )
{
	int Src_start_x = 0;
	int Src_start_y = 0;
	int Src_copy_width = sourceWidth;
	int Src_copy_height = sourceHeight;
	//sanity checks
	if( sourceWidth < Src_start_x + Src_copy_width )
		return;
	if( sourceHeight < Src_start_y + Src_copy_height )
		return;
	if( targetWidth < Dst_start_x + Src_copy_width )
		return;
	if( targetHeight < Dst_start_y + Src_copy_height )
		return;

	LOG_FUNCTION_ENTER( "InsertBuffer" )

	int Src_stride_half = Src_stride / 2;
	int targetStride_half = targetStride / 2;
	unsigned char *sourceBufferYUV_Y = sourceBufferYUV + Src_stride * Src_start_y + Src_start_x;
	unsigned char *sourceBufferYUV_U = sourceBufferYUV + ( Src_stride_half ) * ( Src_start_y / 2 ) + ( Src_start_x / 2 ) + sourceHeight * Src_stride;
	unsigned char *sourceBufferYUV_V = sourceBufferYUV + ( Src_stride_half ) * ( Src_start_y / 2 ) + ( Src_start_x / 2 ) + sourceHeight * Src_stride + sourceHeight / 2 * Src_stride / 2;
	unsigned char *targetBufferYUV_Y = targetBufferYUV + targetStride * Dst_start_y + Dst_start_x;
	unsigned char *targetBufferYUV_U = targetBufferYUV + ( targetStride_half ) * ( Dst_start_y / 2 ) + ( Dst_start_x / 2 ) + targetHeight * targetStride;
	unsigned char *targetBufferYUV_V = targetBufferYUV + ( targetStride_half ) * ( Dst_start_y / 2 ) + ( Dst_start_x / 2 ) + targetHeight * targetStride + targetHeight / 2 * targetStride / 2;
	for( int i=Src_copy_height;i>0;i-- )
	{
		memcpy( targetBufferYUV_Y, sourceBufferYUV_Y, Src_copy_width );
		targetBufferYUV_Y += targetStride;
		sourceBufferYUV_Y += Src_stride;
	}
	int Src_copy_height_half = Src_copy_height / 2;
	int Src_copy_width_half = Src_copy_width / 2;
	for( int i=Src_copy_height_half;i>0;i-- )
	{
		memcpy( targetBufferYUV_U, sourceBufferYUV_U, Src_copy_width_half );
		memcpy( targetBufferYUV_V, sourceBufferYUV_V, Src_copy_width_half );
		targetBufferYUV_U += targetStride_half;
		sourceBufferYUV_U += Src_stride_half;
		targetBufferYUV_V += targetStride_half;
		sourceBufferYUV_V += Src_stride_half;
	}
	LOG_FUNCTION_EXIT( "InsertBuffer" )
}

void APIENTRY ConvertBGRtoYV12(unsigned char *sourceBufferRGB, int sourceBufferSize, int width, int height, int sourceStride, unsigned char *targetBufferYUV, int targetBufferSize, int targetStride)
{
	int i, j;

	LOG_FUNCTION_ENTER( "ConvertBGRtoYV12" )

	unsigned char * out_Y = targetBufferYUV ;
	unsigned char * out_U = out_Y + targetStride*height ;
	unsigned char * out_V = out_U + targetStride/2*height/2 ;

	int widthX3 = width + width + width;

#define CALCULATE_ONE_COEFF(A0, A1) temp2 = _mm_unpacklo_epi16(temp0, temp1);temp3 = _mm_unpackhi_epi16(temp0, temp1);A0 = _mm_add_epi32(A0, temp2);A1 = _mm_add_epi32(A1, temp3);

	for( j = 0; j < height; j++)
	{
		__m128i zero;
		zero = _mm_setzero_si128();

		for( i = 0; i < (width-16); i+=8)
		{

			int iX3 = i+i+i;
			__m128i sR, sG, sB, R, G, B, temp0, temp1, temp2, temp3, temp4, Coeff, Acc0, Acc1, Acc2, Acc3;

			Acc0 = _mm_setzero_si128();
			Acc1 = _mm_setzero_si128();

			//load data
			temp0 = _mm_loadu_si128((__m128i*)(&sourceBufferRGB[iX3]));		// xx xx xx xx 32 31 30 22 21 20 12 11 10 02 01 00
			temp1 = _mm_loadu_si128((__m128i*)(&sourceBufferRGB[iX3+12]));	// xx xx xx xx 72 71 70 62 61 60 52 51 50 42 41 40
			temp2 = _mm_srli_si128(temp0, 3);					// xx xx xx xx xx xx xx 32 31 30 22 21 20 12 11 10
			temp3 = _mm_srli_si128(temp0, 6);					// xx xx xx xx xx xx xx xx xx xx 32 31 30 22 21 20
			temp4 = _mm_srli_si128(temp0, 9);					// xx xx xx xx xx xx xx xx xx xx xx xx xx 32 31 30
			temp0 = _mm_unpacklo_epi8(temp0, temp2);			// 31 21 30 20 22 12 21 11 20 10 12 02 11 01 10 00
			temp3 = _mm_unpacklo_epi8(temp3, temp4);			// xx xx xx xx xx xx xx xx xx xx 32 22 31 21 30 20
			temp0 = _mm_unpacklo_epi16(temp0, temp3);			// xx xx 20 10 32 22 12 02 31 21 11 01 30 20 10 00
			sB = temp0;											// xx xx 20 10 32 22 12 02 31 21 11 01 30 20 10 00
			sG = _mm_srli_si128(temp0, 4);						// xx xx xx xx xx xx 20 10 32 22 12 02 31 21 11 01
			sR = _mm_srli_si128(temp0, 8);						// xx xx xx xx xx xx xx xx xx xx 20 10 32 22 12 02
			temp2 = _mm_srli_si128(temp1, 3);					// xx xx xx xx xx xx xx 72 71 70 62 61 60 52 51 50
			temp3 = _mm_srli_si128(temp1, 6);					// xx xx xx xx xx xx xx xx xx xx 72 71 70 62 61 60
			temp4 = _mm_srli_si128(temp1, 9);					// xx xx xx xx xx xx xx xx xx xx xx xx xx 72 71 70
			temp1 = _mm_unpacklo_epi8(temp1, temp2);			// 71 61 70 60 62 52 61 51 60 50 52 42 51 41 50 40
			temp3 = _mm_unpacklo_epi8(temp3, temp4);			// xx xx xx xx xx xx xx xx xx xx 72 62 71 61 70 60
			temp1 = _mm_unpacklo_epi16(temp1, temp3);			// xx xx xx xx 72 62 52 42 71 61 51 41 70 60 50 40
			sB = _mm_unpacklo_epi32(sB, temp1);					// 71 61 51 41 31 21 11 01 70 60 50 40 30 20 10 00
			temp1 = _mm_srli_si128(temp1, 4);					// xx xx xx xx xx xx xx xx 72 62 52 42 71 61 51 41
			sG = _mm_unpacklo_epi32(sG, temp1);					// 72 62 52 42 32 22 12 02 71 61 51 41 31 21 11 01
			temp1 = _mm_srli_si128(temp1, 4);					// xx xx xx xx xx xx xx xx xx xx xx xx 72 62 52 42
			sR = _mm_unpacklo_epi32(sR, temp1);					// xx xx xx xx xx xx 20 10 72 62 52 42 32 22 12 02
			sB = _mm_unpacklo_epi8(sB, zero);					// 70 60 50 40 30 20 10 00
			sG = _mm_unpacklo_epi8(sG, zero);					// 71 61 51 41 31 21 11 01
			sR = _mm_unpacklo_epi8(sR, zero);					// 72 62 52 42 32 22 12 02
			//end load data

			//Y
			B = sB;
			G = sG;
			R = sR;

			Coeff = _mm_set1_epi16(66);
			temp0 = _mm_mullo_epi16(R, Coeff);
			temp1 = _mm_mulhi_epi16(R, Coeff);
			CALCULATE_ONE_COEFF(Acc0, Acc1)

			Coeff = _mm_set1_epi16(129);
			temp0 = _mm_mullo_epi16(G, Coeff);
			temp1 = _mm_mulhi_epi16(G, Coeff);
			CALCULATE_ONE_COEFF(Acc0, Acc1)

			Coeff = _mm_set1_epi16(25);
			temp0 = _mm_mullo_epi16(B, Coeff);
			temp1 = _mm_mulhi_epi16(B, Coeff);
			CALCULATE_ONE_COEFF(Acc0, Acc1)

			//add offset
			temp0 = _mm_set1_epi32(128);
			Acc0 = _mm_add_epi32(Acc0, temp0);
			Acc1 = _mm_add_epi32(Acc1, temp0);
			Acc0 = _mm_srai_epi32(Acc0, 8);
			Acc1 = _mm_srai_epi32(Acc1, 8);
			temp0 = _mm_set1_epi32(16);
			Acc0 = _mm_add_epi32(Acc0, temp0);
			Acc1 = _mm_add_epi32(Acc1, temp0);
			
			//store result
			Acc0 = _mm_packs_epi32(Acc0, Acc1);
			Acc0 = _mm_packus_epi16(Acc0, zero);
			_mm_storel_epi64((__m128i*)(&out_Y[i]), Acc0);
			//end Y

			if((j&1) == 0)
			{
				Acc0 = _mm_setzero_si128();
				Acc1 = _mm_setzero_si128();
				Acc2 = _mm_setzero_si128();
				Acc3 = _mm_setzero_si128();

				// sB: 70 60 50 40 30 20 10 00
				// sG: 71 61 51 41 31 21 11 01
				// sR: 72 62 52 42 32 22 12 02

				//00 and 01 for both U and V
				temp0 = _mm_shufflelo_epi16(sR, 0xD8);			// xx xx xx xx 32 12 22 02
				temp1 = _mm_shufflehi_epi16(sR, 0xD8);			// 72 52 62 42 xx xx xx xx
				temp1 = _mm_srli_si128(temp1, 8);				// xx xx xx xx 72 52 62 42
				R = _mm_unpacklo_epi32(temp0, temp1);			// 72 52 32 12 62 42 22 02

				temp0 = _mm_shufflelo_epi16(sG, 0xD8);			// xx xx xx xx 32 12 22 02
				temp1 = _mm_shufflehi_epi16(sG, 0xD8);			// 72 52 62 42 xx xx xx xx
				temp1 = _mm_srli_si128(temp1, 8);				// xx xx xx xx 72 52 62 42
				G = _mm_unpacklo_epi32(temp0, temp1);			// 72 52 32 12 62 42 22 02

				temp0 = _mm_shufflelo_epi16(sB, 0xD8);			// xx xx xx xx 32 12 22 02
				temp1 = _mm_shufflehi_epi16(sB, 0xD8);			// 72 52 62 42 xx xx xx xx
				temp1 = _mm_srli_si128(temp1, 8);				// xx xx xx xx 72 52 62 42
				B = _mm_unpacklo_epi32(temp0, temp1);			// 72 52 32 12 62 42 22 02

				//for U
				Coeff = _mm_set1_epi16(-38);
				temp0 = _mm_mullo_epi16(R, Coeff);
				temp1 = _mm_mulhi_epi16(R, Coeff);
				CALCULATE_ONE_COEFF(Acc0, Acc1)

				Coeff = _mm_set1_epi16(-74);
				temp0 = _mm_mullo_epi16(G, Coeff);
				temp1 = _mm_mulhi_epi16(G, Coeff);
				CALCULATE_ONE_COEFF(Acc0, Acc1)

				Coeff = _mm_set1_epi16(112);
				temp0 = _mm_mullo_epi16(B, Coeff);
				temp1 = _mm_mulhi_epi16(B, Coeff);
				CALCULATE_ONE_COEFF(Acc0, Acc1)

				//for V
				Coeff = _mm_set1_epi16(112);
				temp0 = _mm_mullo_epi16(R, Coeff);
				temp1 = _mm_mulhi_epi16(R, Coeff);
				CALCULATE_ONE_COEFF(Acc2, Acc3)

				Coeff = _mm_set1_epi16(-94);
				temp0 = _mm_mullo_epi16(G, Coeff);
				temp1 = _mm_mulhi_epi16(G, Coeff);
				CALCULATE_ONE_COEFF(Acc2, Acc3)

				Coeff = _mm_set1_epi16(-18);
				temp0 = _mm_mullo_epi16(B, Coeff);
				temp1 = _mm_mulhi_epi16(B, Coeff);
				CALCULATE_ONE_COEFF(Acc2, Acc3)

				//load data
				temp0 = _mm_loadu_si128((__m128i*)(&sourceBufferRGB[iX3+widthX3]));		// xx xx xx xx 32 31 30 22 21 20 12 11 10 02 01 00
				temp1 = _mm_loadu_si128((__m128i*)(&sourceBufferRGB[iX3+widthX3+12]));	// xx xx xx xx 72 71 70 62 61 60 52 51 50 42 41 40
				temp2 = _mm_srli_si128(temp0, 3);					// xx xx xx xx xx xx xx 32 31 30 22 21 20 12 11 10
				temp3 = _mm_srli_si128(temp0, 6);					// xx xx xx xx xx xx xx xx xx xx 32 31 30 22 21 20
				temp4 = _mm_srli_si128(temp0, 9);					// xx xx xx xx xx xx xx xx xx xx xx xx xx 32 31 30
				temp0 = _mm_unpacklo_epi8(temp0, temp2);			// 31 21 30 20 22 12 21 11 20 10 12 02 11 01 10 00
				temp3 = _mm_unpacklo_epi8(temp3, temp4);			// xx xx xx xx xx xx xx xx xx xx 32 22 31 21 30 20
				temp0 = _mm_unpacklo_epi16(temp0, temp3);			// xx xx 20 10 32 22 12 02 31 21 11 01 30 20 10 00
				sB = temp0;											// xx xx 20 10 32 22 12 02 31 21 11 01 30 20 10 00
				sG = _mm_srli_si128(temp0, 4);						// xx xx xx xx xx xx 20 10 32 22 12 02 31 21 11 01
				sR = _mm_srli_si128(temp0, 8);						// xx xx xx xx xx xx xx xx xx xx 20 10 32 22 12 02
				temp2 = _mm_srli_si128(temp1, 3);					// xx xx xx xx xx xx xx 72 71 70 62 61 60 52 51 50
				temp3 = _mm_srli_si128(temp1, 6);					// xx xx xx xx xx xx xx xx xx xx 72 71 70 62 61 60
				temp4 = _mm_srli_si128(temp1, 9);					// xx xx xx xx xx xx xx xx xx xx xx xx xx 72 71 70
				temp1 = _mm_unpacklo_epi8(temp1, temp2);			// 71 61 70 60 62 52 61 51 60 50 52 42 51 41 50 40
				temp3 = _mm_unpacklo_epi8(temp3, temp4);			// xx xx xx xx xx xx xx xx xx xx 72 62 71 61 70 60
				temp1 = _mm_unpacklo_epi16(temp1, temp3);			// xx xx xx xx 72 62 52 42 71 61 51 41 70 60 50 40
				sB = _mm_unpacklo_epi32(sB, temp1);					// 71 61 51 41 31 21 11 01 70 60 50 40 30 20 10 00
				temp1 = _mm_srli_si128(temp1, 4);					// xx xx xx xx xx xx xx xx 72 62 52 42 71 61 51 41
				sG = _mm_unpacklo_epi32(sG, temp1);					// 72 62 52 42 32 22 12 02 71 61 51 41 31 21 11 01
				temp1 = _mm_srli_si128(temp1, 4);					// xx xx xx xx xx xx xx xx xx xx xx xx 72 62 52 42
				sR = _mm_unpacklo_epi32(sR, temp1);					// xx xx xx xx xx xx 20 10 72 62 52 42 32 22 12 02
				sB = _mm_unpacklo_epi8(sB, zero);					// 70 60 50 40 30 20 10 00
				sG = _mm_unpacklo_epi8(sG, zero);					// 71 61 51 41 31 21 11 01
				sR = _mm_unpacklo_epi8(sR, zero);					// 72 62 52 42 32 22 12 02
				//end load data

				temp0 = _mm_shufflelo_epi16(sR, 0xD8);			// xx xx xx xx 32 12 22 02
				temp1 = _mm_shufflehi_epi16(sR, 0xD8);			// 72 52 62 42 xx xx xx xx
				temp1 = _mm_srli_si128(temp1, 8);				// xx xx xx xx 72 52 62 42
				R = _mm_unpacklo_epi32(temp0, temp1);			// 72 52 32 12 62 42 22 02

				temp0 = _mm_shufflelo_epi16(sG, 0xD8);			// xx xx xx xx 32 12 22 02
				temp1 = _mm_shufflehi_epi16(sG, 0xD8);			// 72 52 62 42 xx xx xx xx
				temp1 = _mm_srli_si128(temp1, 8);				// xx xx xx xx 72 52 62 42
				G = _mm_unpacklo_epi32(temp0, temp1);			// 72 52 32 12 62 42 22 02

				temp0 = _mm_shufflelo_epi16(sB, 0xD8);			// xx xx xx xx 32 12 22 02
				temp1 = _mm_shufflehi_epi16(sB, 0xD8);			// 72 52 62 42 xx xx xx xx
				temp1 = _mm_srli_si128(temp1, 8);				// xx xx xx xx 72 52 62 42
				B = _mm_unpacklo_epi32(temp0, temp1);			// 72 52 32 12 62 42 22 02

				//for U
				Coeff = _mm_set1_epi16(-38);
				temp0 = _mm_mullo_epi16(R, Coeff);
				temp1 = _mm_mulhi_epi16(R, Coeff);
				CALCULATE_ONE_COEFF(Acc0, Acc1)

				Coeff = _mm_set1_epi16(-74);
				temp0 = _mm_mullo_epi16(G, Coeff);
				temp1 = _mm_mulhi_epi16(G, Coeff);
				CALCULATE_ONE_COEFF(Acc0, Acc1)

				Coeff = _mm_set1_epi16(112);
				temp0 = _mm_mullo_epi16(B, Coeff);
				temp1 = _mm_mulhi_epi16(B, Coeff);
				CALCULATE_ONE_COEFF(Acc0, Acc1)

				//for V
				Coeff = _mm_set1_epi16(112);
				temp0 = _mm_mullo_epi16(R, Coeff);
				temp1 = _mm_mulhi_epi16(R, Coeff);
				CALCULATE_ONE_COEFF(Acc2, Acc3)

				Coeff = _mm_set1_epi16(-94);
				temp0 = _mm_mullo_epi16(G, Coeff);
				temp1 = _mm_mulhi_epi16(G, Coeff);
				CALCULATE_ONE_COEFF(Acc2, Acc3)

				Coeff = _mm_set1_epi16(-18);
				temp0 = _mm_mullo_epi16(B, Coeff);
				temp1 = _mm_mulhi_epi16(B, Coeff);
				CALCULATE_ONE_COEFF(Acc2, Acc3)

				Acc0 = _mm_add_epi32(Acc0, Acc1);	//U
				Acc2 = _mm_add_epi32(Acc2, Acc3);	//V

				//add offset
				temp0 = _mm_set1_epi32(512);
				Acc0 = _mm_add_epi32(Acc0, temp0);
				Acc2 = _mm_add_epi32(Acc2, temp0);
				Acc0 = _mm_srai_epi32(Acc0, 10);
				Acc2 = _mm_srai_epi32(Acc2, 10);
				temp0 = _mm_set1_epi32(128);
				Acc0 = _mm_add_epi32(Acc0, temp0);
				Acc2 = _mm_add_epi32(Acc2, temp0);

				//store result
				Acc0 = _mm_packs_epi32(Acc0, zero);
				Acc0 = _mm_packus_epi16(Acc0, zero);
				*(__int32*)(&out_U[i>>1]) = _mm_cvtsi128_si32(Acc0);
				Acc2 = _mm_packs_epi32(Acc2, zero);
				Acc2 = _mm_packus_epi16(Acc2, zero);
				*(__int32*)(&out_V[i>>1]) = _mm_cvtsi128_si32(Acc2);
			}
		}
		for( ; i < width; i++)
		{
			int iX3 = i+i+i;
			unsigned char R, G, B;
			B = sourceBufferRGB[iX3]; 
			G = sourceBufferRGB[iX3+1]; 
			R = sourceBufferRGB[iX3+2];
			out_Y[i] = CLIP3(0, 255, ((66*R + 129*G + 25*B + 128) >> 8 ) + 16);

			if( (i&1) == 0 && (j&1) == 0)
			{
				unsigned char R_01, G_01, B_01, R_11, G_11, B_11, R_10, G_10, B_10;
				B_01 = sourceBufferRGB[3+iX3]; 
				G_01 = sourceBufferRGB[3+iX3+1]; 
				R_01 = sourceBufferRGB[3+iX3+2];
				B_10 = sourceBufferRGB[widthX3+iX3]; 
				G_10 = sourceBufferRGB[widthX3+iX3+1]; 
				R_10 = sourceBufferRGB[widthX3+iX3+2];
				B_11 = sourceBufferRGB[3+widthX3+iX3]; 
				G_11 = sourceBufferRGB[3+widthX3+iX3+1]; 
				R_11 = sourceBufferRGB[3+widthX3+iX3+2];

				out_U[i>>1] =  CLIP3(0, 255, (((-38*R - 74*G + 112*B) + (-38*R_01  - 74*G_01  + 112*B_01 ) 
					+ (-38*R_10 - 74*G_10 + 112*B_10) + (-38*R_11 - 74*G_11 + 112*B_11) + 512 ) >> 10 ) + 128);
				out_V[i>>1] =  CLIP3(0, 255, (((112*R - 94*G - 18*B) + (112*R_01  - 94*G_01 - 18*B_01 ) 
					+ (112*R_10 - 94*G_10 - 18*B_10) + (112*R_11 - 94*G_11 - 18*B_11) + 512 ) >>10 ) + 128);
			}
		}
		out_Y += targetStride;
		if((j&1)==0)
		{
			out_U += (targetStride>>1);
			out_V += (targetStride>>1);
		}
		sourceBufferRGB += widthX3;
	}
	LOG_FUNCTION_EXIT( "ConvertBGRtoYV12" )
}

float APIENTRY GetColorValues(unsigned char *pbuff, int bufferSize, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *_Ref, RGB24Color *_Tolerance, RGB24Color *_CurrentColor)
{
	LOG_FUNCTION_ENTER( "GetColorValues" )

	unsigned char *buff = pbuff + StartY * Stride + StartX * 3;
	int Counter = 0;
	int SumR = 0;
	int SumG = 0;
	int SumB = 0;
	__m128i RefXMM,AllowedDiff;
	RefXMM.m128i_u8[0] = _Ref->B;
	RefXMM.m128i_u8[1] = _Ref->G;
	RefXMM.m128i_u8[2] = _Ref->R;
	RefXMM.m128i_u8[3] = _Ref->B;
	RefXMM.m128i_u8[4] = _Ref->G;
	RefXMM.m128i_u8[5] = _Ref->R;
	RefXMM.m128i_u8[6] = _Ref->B;
	RefXMM.m128i_u8[7] = _Ref->G;
	RefXMM.m128i_u8[8] = _Ref->R;
	RefXMM.m128i_u8[9] = _Ref->B;
	RefXMM.m128i_u8[10] = _Ref->G;
	RefXMM.m128i_u8[11] = _Ref->R;
	RefXMM.m128i_u8[12] = _Ref->B;
	RefXMM.m128i_u8[13] = _Ref->G;
	RefXMM.m128i_u8[14] = _Ref->R;
	AllowedDiff.m128i_u8[0] = _Tolerance->B + 1;
	AllowedDiff.m128i_u8[1] = _Tolerance->G + 1;
	AllowedDiff.m128i_u8[2] = _Tolerance->R + 1;
	AllowedDiff.m128i_u8[3] = _Tolerance->B + 1;
	AllowedDiff.m128i_u8[4] = _Tolerance->G + 1;
	AllowedDiff.m128i_u8[5] = _Tolerance->R + 1;
	AllowedDiff.m128i_u8[6] = _Tolerance->B + 1;
	AllowedDiff.m128i_u8[7] = _Tolerance->G + 1;
	AllowedDiff.m128i_u8[8] = _Tolerance->R + 1;
	AllowedDiff.m128i_u8[9] = _Tolerance->B + 1;
	AllowedDiff.m128i_u8[10] = _Tolerance->G + 1;
	AllowedDiff.m128i_u8[11] = _Tolerance->R + 1;
	AllowedDiff.m128i_u8[12] = _Tolerance->B + 1;
	AllowedDiff.m128i_u8[13] = _Tolerance->G + 1;
	AllowedDiff.m128i_u8[14] = _Tolerance->R + 1; 
	__m128i Sum1,Sum2,Sum3,Sum4;
	Sum1 =  _mm_setzero_si128();
	Sum2 =  _mm_setzero_si128();
	Sum3 =  _mm_setzero_si128();
	Sum4 =  _mm_setzero_si128();
	__m128i Mask1;
	Mask1 =  _mm_setzero_si128();
	Mask1.m128i_u8[0] = 128;
	Mask1.m128i_u8[1] = 128;
	Mask1.m128i_u8[2] = 128;

	Mask1.m128i_u8[9] = 128;
	Mask1.m128i_u8[10] = 128;
	Mask1.m128i_u8[11] = 128;

	Mask1.m128i_u8[12] = 128;
	Mask1.m128i_u8[13] = 128;
	Mask1.m128i_u8[14] = 128;
	for( int y=0;y<RegionHeight;y++)
	{
		int x;
		int Counter1,Counter2,Counter3,Counter4;
		Counter1 = Counter2 = Counter3 = Counter4 = 0;
		for( x=0;x<(RegionWidth-5);x+=5 )
		{
			__m128i RGB,ColoDiff;
			int pos = 3*x;
			//load 5 pixels
			RGB = _mm_loadu_si128((__m128i*)(&buff[pos]));
			__m128i Zero,RGBL1,RGBH1,RGBL11,RGBL12,RGBH11,RGBH12;
			Zero =  _mm_setzero_si128();
			RGBL1 = _mm_unpacklo_epi8(RGB, Zero);
			RGBH1 = _mm_unpackhi_epi8(RGB, Zero);
			RGBL11 = _mm_unpacklo_epi16(RGBL1, Zero);
			RGBH11 = _mm_unpackhi_epi16(RGBL1, Zero);
			RGBL12 = _mm_unpacklo_epi16(RGBH1, Zero);
			RGBH12 = _mm_unpackhi_epi16(RGBH1, Zero);

			Sum1 = _mm_add_epi32( Sum1, RGBL11 );
			Sum2 = _mm_add_epi32( Sum2, RGBH11 );
			Sum3 = _mm_add_epi32( Sum3, RGBL12 );
			Sum4 = _mm_add_epi32( Sum4, RGBH12 );

			//do the abs to reference
			ColoDiff = _mm_sub_epi8( RGB, RefXMM );
			ColoDiff = _mm_abs_epi8( ColoDiff );
			ColoDiff = _mm_sub_epi8( ColoDiff, AllowedDiff );
			//check if we are smaller then reference
			//mark result if apropriate
			__m128i T1,T2;
			T1 = _mm_and_si128( ColoDiff, Mask1 );
			T2 = _mm_slli_si128(ColoDiff, 6);
			T2 = _mm_and_si128( T2, Mask1 );
			if( T1.m128i_u32[0] == 0x00808080 )
				Counter1++;
			if( T2.m128i_u32[2] == 0x80808000 )
				Counter2++;
			if( T2.m128i_u32[3] == 0x00808080 )
				Counter3++;
			if( T1.m128i_u32[2] == 0x80808000 )
				Counter4++;
			if( T1.m128i_u32[3] == 0x00808080 )
				Counter++;			
		}/**/
		Counter += Counter1;
		Counter += Counter2;
		Counter += Counter3;
		Counter += Counter4;
		for( ;x<RegionWidth;x++)
		{
			int pos = x * 3;
			SumR += buff[pos + 0];
			SumG += buff[pos + 1];
			SumB += buff[pos + 2];
			if( (abs((int)buff[pos + 0] - (int)_Ref->B) <= (int)_Tolerance->B) &&
				(abs((int)buff[pos + 1] - (int)_Ref->G) <= (int)_Tolerance->G) &&
				(abs((int)buff[pos + 2] - (int)_Ref->R) <= (int)_Tolerance->R) )
				Counter++;
		}
		buff += Stride;
	}
	SumR += Sum1.m128i_i32[0];
	SumG += Sum1.m128i_i32[1];
	SumB += Sum1.m128i_i32[2];

	SumR += Sum1.m128i_i32[3];
	SumG += Sum2.m128i_i32[0];
	SumB += Sum2.m128i_i32[1];

	SumR += Sum2.m128i_i32[2];
	SumG += Sum2.m128i_i32[3];
	SumB += Sum3.m128i_i32[0];

	SumR += Sum3.m128i_i32[1];
	SumG += Sum3.m128i_i32[2];
	SumB += Sum3.m128i_i32[3];

	SumR += Sum4.m128i_i32[0];
	SumG += Sum4.m128i_i32[1];
	SumB += Sum4.m128i_i32[2];

	int _RegionSize = RegionWidth * RegionHeight;
	_CurrentColor->R = SumR /_RegionSize;
	_CurrentColor->G = SumG /_RegionSize;
	_CurrentColor->B = SumB /_RegionSize;

	float ret = (float )( 100 * Counter ) / (float)(_RegionSize);

	LOG_FUNCTION_EXIT( "GetColorValues" )

	return ret; 
}

#define REQUIRED_SSE_BYTE_ALLIGNEMENT			16
#define ALLOCATOR_HEADER_SIGNITURE				0xCDECCDEC
#define ALLOCATOR_BOUNDS_CHECKER_CODE			0x0BADBEEF

// we need byte precision when we define this struct
#pragma pack(push,1)

//size of this struct need to be multiple of 16
struct MemoryHeader
{
	CRITICAL_SECTION	TransactionLock;			// in case you need this buffer to be threadsafe then you can lock deallocator until jobs finish using the buffer
	unsigned int		HeaderSigniture;			// In case we are not using transaction lock we can use as a very very low security measure
	unsigned int		SizeOfAllocedBuffer;		// Size of the actual requested buffer. Could be used by external tool for bounds checking
	unsigned char		Alligment;					// We can create specialized functions when memory is alligned that can be up to 2 times faster then non alligned. Not implemented for now
	unsigned int		SizeOfBoundsChecker;		// add a row / col to the buffer so we can check if we overwrote something
	unsigned char		HeaderStuffing[11];			// stuff header to be multiple of 16. This is barbaric hardcoding
};

//get rid of breaking compiler alligment. Let compiler generate alligned reading
#pragma pack(pop)

int		AllocateMemory( unsigned char **OutputBuffer, int BufferSize, int BoundsCheckerSize )
{
	assert( ( sizeof( MemoryHeader ) & ( REQUIRED_SSE_BYTE_ALLIGNEMENT - 1 ) ) == 0 );
	//we return 0 by default unless it is a success
	*OutputBuffer = NULL;
	//needs to be multiple of 4
	BoundsCheckerSize = ( ( BoundsCheckerSize / 4 + 1 ) * 4 ); 
	int AllocSize = sizeof( MemoryHeader ) + BufferSize + BoundsCheckerSize;
	void *Buff =  _aligned_malloc( AllocSize, REQUIRED_SSE_BYTE_ALLIGNEMENT );
	if( Buff == NULL )
		return ERR_BUFFER_NULL;
	MemoryHeader	*Header = (MemoryHeader	*)Buff;
	memset( Header, 0, sizeof( MemoryHeader ) );
	Header->HeaderSigniture = ALLOCATOR_HEADER_SIGNITURE;
	Header->SizeOfAllocedBuffer = BufferSize;
	Header->Alligment = REQUIRED_SSE_BYTE_ALLIGNEMENT;
	Header->SizeOfBoundsChecker = BoundsCheckerSize;
	//init Bounds Checker
	char *PBoundsChecker = (char *)Buff + sizeof( MemoryHeader );
	PBoundsChecker += Header->SizeOfAllocedBuffer;
	for( unsigned int i=0;i<Header->SizeOfBoundsChecker;i+=4)
		*(int*)&PBoundsChecker[i] = ALLOCATOR_BOUNDS_CHECKER_CODE;
	//intialize critical section
	InitializeCriticalSection( &Header->TransactionLock );
	//set output 
	*OutputBuffer = (unsigned char *)Buff + sizeof( MemoryHeader );
	return 0;
}

int		AllocateRGBMemory( unsigned char **OutputBuffer, int BufferSize, int Width, int Height, int Stride )
{
	//safety checks for parameters
	if( Width <= 0 || Height <= 0 || Stride <= 0 )
		return ERR_INVALID_SIZE;
	if( Width * 3 > Stride )
		return ERR_INVALID_SIZE;
	if( BufferSize < Height * Stride )
		return ERR_INVALID_SIZE;
	return AllocateMemory( OutputBuffer, BufferSize, Stride );
}

int		AllocateYUVMemory( unsigned char **OutputBuffer, int BufferSize, int Width, int Height, int Stride )
{
	//safety checks for parameters
	if( Width <= 0 || Height <= 0 || Stride <= 0 )
		return ERR_INVALID_SIZE;
	if( Width > Stride )
		return ERR_INVALID_SIZE;
	int RGB24Size = Width * Height * 3;		//we will use this. In case people will accidentally swap a YUV buffer with a RGB24 buffer we will not cause a memory corruption
	int YUVSize = Height * Stride + ( Height / 2 ) * ( Stride / 2 ) * 2; //we only need so much
	if( BufferSize < YUVSize )
		return ERR_INVALID_SIZE;
	int AllocBufferSize = max( RGB24Size, BufferSize );
	return AllocateMemory( OutputBuffer, AllocBufferSize, AllocBufferSize - YUVSize + Stride );
}

int HeaderIntegrityCheck( MemoryHeader *Header )
{
	//check for header integrity to avoid double dealloc
	if( Header->Alligment != REQUIRED_SSE_BYTE_ALLIGNEMENT )
		return ERR_INTEGRITY_CHECK_FAILED;
	if( Header->HeaderSigniture != ALLOCATOR_HEADER_SIGNITURE )
		return ERR_INTEGRITY_CHECK_FAILED;
	return 0;
}

int		FreeMemory( void *Buff )
{
	MemoryHeader	*Header = (MemoryHeader	*)((char *)Buff - sizeof( MemoryHeader ) ) ;

	//make sure we are not double deallocating
	int HeaderIntegrity = HeaderIntegrityCheck( Header );
	if( HeaderIntegrity != 0 )
		return HeaderIntegrity;

	//check bounds 
	bool BoundsCheckerResult = true;
	char *PBoundsChecker = (char *)Buff;
	PBoundsChecker += Header->SizeOfAllocedBuffer;
	for( unsigned int i=0;i<Header->SizeOfBoundsChecker;i+=4)
		if( *(int*)&PBoundsChecker[i] != ALLOCATOR_BOUNDS_CHECKER_CODE )
		{
			BoundsCheckerResult = false;
			break;
		}

	//destroy header 
	Header->Alligment = 0;
	Header->HeaderSigniture = 0;
	DeleteCriticalSection( &Header->TransactionLock );
	Header->SizeOfAllocedBuffer = 0;
	Header->SizeOfBoundsChecker = 0;

	//destroy buffer
	_aligned_free( Header );

	//signal if we corrupted memory or not
	if( BoundsCheckerResult == false )
		return ERR_BOUNDSCHECK_FAILED;

	return 0;
}

int		FreeMemoryWaitForTransactions( void *Buff )
{
	MemoryHeader	*Header = (MemoryHeader	*)((char *)Buff - sizeof( MemoryHeader ) ) ;

	//make sure we are not double deallocating
	int HeaderIntegrity = HeaderIntegrityCheck( Header );
	if( HeaderIntegrity != 0 )
		return HeaderIntegrity;

	//request lock on the buffer 
	EnterCriticalSection( &Header->TransactionLock );
	//when we get here other threads finished using the buffer
	LeaveCriticalSection( &Header->TransactionLock );

	//if other threads released the buffer then we assume it is safe to release it
	//!! note that we can't lock the buffer we are releasing, because we deallocate the lock also !
	return FreeMemory( Buff );
}

int		MemorySpinLockEnterTransaction( void *Buff )
{
	MemoryHeader	*Header = (MemoryHeader	*)((char *)Buff - sizeof( MemoryHeader ) ) ;

	//make sure we are not double deallocating
	int HeaderIntegrity = HeaderIntegrityCheck( Header );
	if( HeaderIntegrity != 0 )
		return HeaderIntegrity;

	EnterCriticalSection( &Header->TransactionLock );
	return 0;
}

int		MemorySpinLockExitTransaction( void *Buff )
{
	MemoryHeader	*Header = (MemoryHeader	*)((char *)Buff - sizeof( MemoryHeader ) ) ;

	//make sure we are not double deallocating
	int HeaderIntegrity = HeaderIntegrityCheck( Header );
	if( HeaderIntegrity != 0 )
		return HeaderIntegrity;

	LeaveCriticalSection( &Header->TransactionLock );
	return 0;
}