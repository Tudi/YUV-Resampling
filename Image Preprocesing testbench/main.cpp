#include "StdAfx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <assert.h>
#include <conio.h>
#include <emmintrin.h>

#ifdef _DEBUG
	#ifdef _WIN64
		#pragma comment (lib, "../../../Common/ImageProcessing/Build/x64/Debug/Plugins/ImageProcessing.lib")
	#else
		#pragma comment (lib, "../../../Common/ImageProcessing/Build/Debug/Plugins/ImageProcessing.lib")
	#endif
#else
	#ifdef _WIN64
		#pragma comment (lib, "../../../Common/ImageProcessing/Build/x64/Release/Plugins/ImageProcessing.lib")
	#else
		#pragma comment (lib, "../../../Common/ImageProcessing/Build/Release/Plugins/ImageProcessing.lib")
	#endif
#endif


//#define TEST_STRICT_BUFFER_SIZE			//buffer size is exactly as needed to test external bounce checkers for buffer violations
#define TEST_BOUNCE_CHECKER_INTERNAL	//Internal bounce checker
//#define TEST_PLENTY_BUFFER_SIZE

#define SRC_Width				1920
#define SRC_Height				1080
#define RBG_BIT_COUNT			24
#define	YUV_BIT_COUNT			12
#define BOUNCE_CHECKER_CODE		0xBADBEEF
#ifdef TEST_STRICT_BUFFER_SIZE
	#define BOUNCE_CHECKER_SIZE		0
	#define YUV_STRIDE				SRC_Width
	#define RGB_STRIDE				SRC_Width * 3
	#define YUV_BUFF_SIZE			( YUV_STRIDE * SRC_Height * 3 / 2 )
	#define RGB_BUFF_SIZE			( RGB_STRIDE * SRC_Height * 3 )
#elif defined( TEST_BOUNCE_CHECKER_INTERNAL )
	#define BOUNCE_CHECKER_SIZE		32		//multiple of 4
	#define YUV_STRIDE				( SRC_Width + 2 * BOUNCE_CHECKER_SIZE )
	#define RGB_STRIDE				( SRC_Width * 3 + 2 * BOUNCE_CHECKER_SIZE )
	#define SSE_FRIENDLY_PADDING	64
	#define YUV_BUFF_SIZE			( YUV_STRIDE * ( SRC_Height + 2 * BOUNCE_CHECKER_SIZE ) * 3 / 2 )
	#define RGB_BUFF_SIZE			( RGB_STRIDE * ( SRC_Height + 2 * BOUNCE_CHECKER_SIZE ) * 3 )
#else
	#define BOUNCE_CHECKER_SIZE		32		//multiple of 4
	#define YUV_STRIDE				( SRC_Width + 2 * BOUNCE_CHECKER_SIZE )
	#define RGB_STRIDE				( SRC_Width * 3 + 2 * BOUNCE_CHECKER_SIZE )
	#define SSE_FRIENDLY_PADDING	64
	#define YUV_BUFF_SIZE			( YUV_STRIDE * ( SRC_Height + 2 * BOUNCE_CHECKER_SIZE ) * RBG_BIT_COUNT / 8 + SSE_FRIENDLY_PADDING )
	#define RGB_BUFF_SIZE			( RGB_STRIDE * ( SRC_Height + 2 * BOUNCE_CHECKER_SIZE ) * RBG_BIT_COUNT / 8 + SSE_FRIENDLY_PADDING )
#endif


#ifdef _DEBUG
	#define LOOP_TEST_COUNT			1	//to have a larger runtime
#else
	#define LOOP_TEST_COUNT			1000	//to have a larger runtime
#endif

BYTE *taddress1;
BYTE *taddress2;
BYTE *taddress3;
BYTE *taddress4;
BYTE *taddress5;
BYTE *taddress6;
BYTE* addressYUV1;
BYTE* addressYUV2;
BYTE* addressYUV3;
BYTE* addressRGB1;
BYTE* addressRGB2;
BYTE* addressRGB3;


void DoSpeedTestYUV2RGB(unsigned char *src,int src_width, int src_height, int src_stride, unsigned char *dst, int dst_stride)
{
	UINT so,eo;
	UINT diffo;
	diffo = 0;

	so = GetTickCount();
	for( int loop=0;loop<LOOP_TEST_COUNT;loop++)
	{
//		IYUV_to_RGB24( src, src_width, src_height, src_stride, dst, dst_stride );
//		IYUV_to_RGB24_1( src, src_width, src_height, src_stride, dst, dst_stride );
//		IYUV_to_RGB24_2( src, src_width, src_height, src_stride, dst, dst_stride );
//		IYUV_to_RGB24_3( src, src_width, src_height, src_stride, dst, dst_stride );
//		IYUV_to_RGB24_4( src, src_width, src_height, src_stride, dst, dst_stride );
//		IYUV_to_RGB24_5( src, src_width, src_height, src_stride, dst, dst_stride );
//		IYUV_to_RGB24_6( src, src_width, src_height, src_stride, dst, dst_stride );
//		IYUV_to_RGB24_7( src, src_width, src_height, src_stride, dst, dst_stride );
		IYUV_to_RGB24_8( src, src_width, src_height, src_stride, dst, dst_stride ); //2.54
//		IYUV_to_RGB24_9( src, src_width, src_height, src_stride, dst, dst_stride ); //2.465
//		ConvertYV12toRGB( src_width, src_height, src_stride, src, dst, dst_stride ); //2.54
	}
	eo = GetTickCount();
	diffo += eo - so;
	printf("Input resolution : %dx%d \n", src_width, src_height );
	printf("Number of test runs : %d \n", LOOP_TEST_COUNT );
	printf("Total duration : %d MS\n", diffo );
	printf("Duration / frame : %f MS\n", (float)diffo/LOOP_TEST_COUNT );
}

void DoSpeedTestBGR2YUV(unsigned char *src,int src_width, int src_height, int src_stride, unsigned char *dst, int dst_stride)
{
	UINT so,eo;
	UINT diffo;
	diffo = 0;

	so = GetTickCount();
	for( int loop=0;loop<LOOP_TEST_COUNT;loop++)
	{
		IYUV_to_BGR24_1( src, src_width, src_height, src_stride, dst, dst_stride ); //2.54
	}
	eo = GetTickCount();
	diffo += eo - so;
	printf("Input resolution : %dx%d \n", src_width, src_height );
	printf("Number of test runs : %d \n", LOOP_TEST_COUNT );
	printf("Total duration : %d MS\n", diffo );
	printf("Duration / frame : %f MS\n", (float)diffo/LOOP_TEST_COUNT );
}

void DoSpeedTestRGB2GrayScale(unsigned char *src,int src_width, int src_height, int src_stride, unsigned char *dst, int dst_stride)
{
	UINT so,eo;
	UINT diffo;
	diffo = 0;

	so = GetTickCount();
	for( int loop=0;loop<LOOP_TEST_COUNT;loop++)
	{
//		RGB24_to_GrayScale_reference( src, src_width, src_height, src_stride, dst, dst_stride, 16 );
//		RGB24_to_GrayScale_1( src, src_width, src_height, src_stride, dst, dst_stride, 16 );
//		RGB24_to_GrayScale_2( src, src_width, src_height, src_stride, dst, dst_stride, 16 );
//		RGB24_to_GrayScale_3( src, src_width, src_height, src_stride, dst, dst_stride, 16 );
//		RGB24_to_GrayScale_4( src, src_width, src_height, src_stride, dst, dst_stride, 16 );
//		RGB24_to_GrayScale_5( src, src_width, src_height, src_stride, dst, dst_stride, 16 );
		RGB24_to_GrayScale_6( src, src_width, src_height, src_stride, dst, dst_stride, 16 );
	}
	eo = GetTickCount();
	diffo += eo - so;
	printf("Input resolution : %dx%d \n", src_width, src_height );
	printf("Number of test runs : %d \n", LOOP_TEST_COUNT );
	printf("Total duration : %d MS\n", diffo );
	printf("Duration / frame : %f MS\n", (float)diffo/LOOP_TEST_COUNT );
}

float DoSpeedTestRGBCompare(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels)
{
	UINT so,eo;
	UINT diffo;
	diffo = 0;
	float res;

	so = GetTickCount();
	for( int loop=0;loop<LOOP_TEST_COUNT;loop++)
	{
		for(int i=0;i<height*src_stride;i++)
			Src_RGB[i] = 145+i;
//		res = CompareRGBRegions_reference( Src_RGB, width, height, src_stride, Startx, Starty, BoxWidth, BoxHeight, Ref_RGB, HighlightPixels );
//		res = CompareRGBRegions_1( Src_RGB, width, height, src_stride, Startx, Starty, BoxWidth, BoxHeight, Ref_RGB, HighlightPixels );
		res = CompareRGBRegions_2( Src_RGB, width, height, src_stride, Startx, Starty, BoxWidth, BoxHeight, Ref_RGB, HighlightPixels );
//		res = CompareRGBRegions_3( Src_RGB, width, height, src_stride, Startx, Starty, BoxWidth, BoxHeight, Ref_RGB, HighlightPixels );
//		res = CompareRGBRegions_4( Src_RGB, width, height, src_stride, Startx, Starty, BoxWidth, BoxHeight, Ref_RGB, HighlightPixels );
	}
	eo = GetTickCount();
	diffo += eo - so;
	printf("Input resolution : %dx%d \n", width, height );
	printf("Test area resolution : %dx%d \n", BoxWidth, BoxHeight );
	printf("Number of test runs : %d \n", LOOP_TEST_COUNT );
	printf("Total duration : %d MS\n", diffo );
	printf("Duration / frame : %f MS\n", (float)diffo/LOOP_TEST_COUNT );
	return res;
}


float DoSpeedTestBGRCompare(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels)
{
	UINT so,eo;
	UINT diffo;
	diffo = 0;
	float res;

	so = GetTickCount();
	for( int loop=0;loop<LOOP_TEST_COUNT;loop++)
	{
		for(int i=0;i<height*src_stride;i++)
			Src_RGB[i] = 145+i;
		res = CompareBGRRegions_1( Src_RGB, width, height, src_stride, Startx, Starty, BoxWidth, BoxHeight, Ref_RGB, HighlightPixels );
	}
	eo = GetTickCount();
	diffo += eo - so;
	printf("Input resolution : %dx%d \n", width, height );
	printf("Test area resolution : %dx%d \n", BoxWidth, BoxHeight );
	printf("Number of test runs : %d \n", LOOP_TEST_COUNT );
	printf("Total duration : %d MS\n", diffo );
	printf("Duration / frame : %f MS\n", (float)diffo/LOOP_TEST_COUNT );
	return res;
}

void DoSpeedTestWatermark(unsigned char *src,int src_width, int src_height, int src_stride, unsigned char *dst,int dst_width, int dst_height, int dst_stride)
{
	UINT so,eo;
	UINT diffo;
	diffo = 0;

	so = GetTickCount();
	for( int loop=0;loop<LOOP_TEST_COUNT;loop++)
	{
//		InsertWatermark_reference( src, src_width, src_height, src_stride, src_width/3, src_height/3, src_width/3, src_height/3, dst, dst_width, dst_height, dst_stride, dst_width/3, dst_height/3 );
		InsertWatermark_reference( src, src_width, src_height, src_stride, 1, 1, src_width-2, src_height-2, dst, dst_width, dst_height, dst_stride, 1, 1 );
	}
	eo = GetTickCount();
	diffo += eo - so;
	printf("Input resolution : %dx%d \n", src_width, src_height );
	printf("Number of test runs : %d \n", LOOP_TEST_COUNT );
	printf("Total duration : %d MS\n", diffo );
	printf("Duration / frame : %f MS\n", (float)diffo/LOOP_TEST_COUNT );
}

void DoSpeedTestRGB2YUV(unsigned char *src,int src_width, int src_height, int src_stride, unsigned char *dst, int dst_stride)
{
	UINT so,eo;
	UINT diffo;
	diffo = 0;

	so = GetTickCount();
	for( int loop=0;loop<LOOP_TEST_COUNT;loop++)
	{
//		RGB24_to_IYUV_reference1( src, src_width, src_height, src_stride, dst, dst_stride ); //11.34
		RGB24_to_IYUV_1( src, src_width, src_height, src_stride, dst, dst_stride );			//6.28
	}
	eo = GetTickCount();
	diffo += eo - so;
	printf("Input resolution : %dx%d \n", src_width, src_height );
	printf("Number of test runs : %d \n", LOOP_TEST_COUNT );
	printf("Total duration : %d MS\n", diffo );
	printf("Duration / frame : %f MS\n", (float)diffo/LOOP_TEST_COUNT );
}

void DoSpeedTestColorSampling(unsigned char *src, int src_stride, int width, int height)
{
	UINT so,eo;
	UINT diffo;
	diffo = 0;
	RGB24Color Ref,Tol,Ret;
	float NrRet;
	Ref.R = Ref.G = Ref.B = 128;
	Tol.R = Tol.G = Tol.B = 64;

	so = GetTickCount();
	for( int loop=0;loop<LOOP_TEST_COUNT;loop++)
	{
//		NrRet = GetColorValues_reference( src, src_stride, 1, 1, width, height, &Ref, &Tol, &Ret ); //7.14
//		NrRet = GetColorValues_1( src, src_stride, 1, 1, width, height, &Ref, &Tol, &Ret ); //6.31
//		NrRet = GetColorValues_2( src, src_stride, 1, 1, width, height, &Ref, &Tol, &Ret ); //6.31
//		NrRet = GetColorValues_3( src, src_stride, 1, 1, width, height, &Ref, &Tol, &Ret ); //6.61 + bad
//		NrRet = GetColorValues_4( src, src_stride, 1, 1, width, height, &Ref, &Tol, &Ret ); //4.83
//		NrRet = GetColorValues_5( src, src_stride, 1, 1, width, height, &Ref, &Tol, &Ret ); //4.75
		NrRet = GetColorValues_6( src, src_stride, 1, 1, width, height, &Ref, &Tol, &Ret ); //4.10
	}
	eo = GetTickCount();
	diffo += eo - so;
	printf("Input resolution : %dx%d \n", width, height );
	printf("Number of test runs : %d \n", LOOP_TEST_COUNT );
	printf("Total duration : %d MS\n", diffo );
	printf("Duration / frame : %f MS\n", (float)diffo/LOOP_TEST_COUNT );
	printf("Anti optimization junk ( ignore this line ): %fd %d %d %d %d\n",NrRet,Ret.R,Ret.G,Ret.B);
}

int CheckBoundsContent( int IsInit = 0 )
{
	int YUV_STRIDE_HALF = YUV_STRIDE/2;
	int Y_SIZE = YUV_STRIDE * SRC_Height;
	int YU_SIZE = Y_SIZE + YUV_STRIDE_HALF*SRC_Height/2;
	if( IsInit )
	{
		//rows before and after image content buffer
		for( int y=0;y<BOUNCE_CHECKER_SIZE;y++)
			for( int x=0;x<YUV_STRIDE;x+=4)
			{
				*(int*)&taddress1[y*YUV_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV1[y*YUV_STRIDE+x+Y_SIZE*3/2] = BOUNCE_CHECKER_CODE;
				*(int*)&taddress2[y*YUV_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV2[y*YUV_STRIDE+x+Y_SIZE*3/2] = BOUNCE_CHECKER_CODE;
				*(int*)&taddress3[y*YUV_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV3[y*YUV_STRIDE+x+Y_SIZE*3/2] = BOUNCE_CHECKER_CODE;
			}
		//column before and after image
		for( int y=0;y<SRC_Height;y++)
			for( int x=0;x<BOUNCE_CHECKER_SIZE;x+=4)
			{
				//Y
				*(int*)&addressYUV1[y*YUV_STRIDE+x-BOUNCE_CHECKER_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV1[y*YUV_STRIDE+x+SRC_Width] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV2[y*YUV_STRIDE+x-BOUNCE_CHECKER_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV2[y*YUV_STRIDE+x+SRC_Width] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV3[y*YUV_STRIDE+x-BOUNCE_CHECKER_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV3[y*YUV_STRIDE+x+SRC_Width] = BOUNCE_CHECKER_CODE;
			}
		for( int y=0;y<SRC_Height/2;y++)
			for( int x=0;x<BOUNCE_CHECKER_SIZE/2;x+=4)
			{
				//U
				*(int*)&addressYUV1[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+Y_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV1[y*YUV_STRIDE_HALF+x+SRC_Width/2+Y_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV2[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+Y_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV2[y*YUV_STRIDE_HALF+x+SRC_Width/2+Y_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV3[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+Y_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV3[y*YUV_STRIDE_HALF+x+SRC_Width/2+Y_SIZE] = BOUNCE_CHECKER_CODE;
				//V
				*(int*)&addressYUV1[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+YU_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV1[y*YUV_STRIDE_HALF+x+SRC_Width/2+YU_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV2[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+YU_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV2[y*YUV_STRIDE_HALF+x+SRC_Width/2+YU_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV3[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+YU_SIZE] = BOUNCE_CHECKER_CODE;
				*(int*)&addressYUV3[y*YUV_STRIDE_HALF+x+SRC_Width/2+YU_SIZE] = BOUNCE_CHECKER_CODE; /**/
			}
		//for RGB buffers
		for( int y=0;y<BOUNCE_CHECKER_SIZE;y++)
			for( int x=0;x<RGB_STRIDE;x+=4)
			{
				*(int*)&taddress4[y*RGB_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressRGB1[y*RGB_STRIDE+x+RGB_STRIDE*SRC_Height] = BOUNCE_CHECKER_CODE;
				*(int*)&taddress5[y*RGB_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressRGB2[y*RGB_STRIDE+x+RGB_STRIDE*SRC_Height] = BOUNCE_CHECKER_CODE;
				*(int*)&taddress6[y*RGB_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressRGB3[y*RGB_STRIDE+x+RGB_STRIDE*SRC_Height] = BOUNCE_CHECKER_CODE;
			}
		//column before and after image
		for( int y=0;y<SRC_Height;y++)
			for( int x=0;x<BOUNCE_CHECKER_SIZE;x+=4)
			{
				*(int*)&taddress4[y*RGB_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressRGB1[y*RGB_STRIDE+x+SRC_Width*3] = BOUNCE_CHECKER_CODE;
				*(int*)&taddress5[y*RGB_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressRGB2[y*RGB_STRIDE+x+SRC_Width*3] = BOUNCE_CHECKER_CODE;
				*(int*)&taddress6[y*RGB_STRIDE+x] = BOUNCE_CHECKER_CODE;
				*(int*)&addressRGB3[y*RGB_STRIDE+x+SRC_Width*3] = BOUNCE_CHECKER_CODE;
			}
	}
	else
	{
		//rows before and after image content buffer
		for( int y=0;y<BOUNCE_CHECKER_SIZE;y++)
			for( int x=0;x<YUV_STRIDE;x+=4)
			{
				if( *(int*)&taddress1[y*YUV_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV1[y*YUV_STRIDE+x+Y_SIZE*3/2] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&taddress2[y*YUV_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV2[y*YUV_STRIDE+x+Y_SIZE*3/2] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&taddress3[y*YUV_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV3[y*YUV_STRIDE+x+Y_SIZE*3/2] != BOUNCE_CHECKER_CODE )
					return 1;
			}
		//column before and after image
		for( int y=0;y<SRC_Height;y++)
			for( int x=0;x<BOUNCE_CHECKER_SIZE;x+=4)
			{
				//Y
				if( *(int*)&addressYUV1[y*YUV_STRIDE+x-BOUNCE_CHECKER_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV1[y*YUV_STRIDE+x+SRC_Width] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV2[y*YUV_STRIDE+x-BOUNCE_CHECKER_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV2[y*YUV_STRIDE+x+SRC_Width] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV3[y*YUV_STRIDE+x-BOUNCE_CHECKER_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV3[y*YUV_STRIDE+x+SRC_Width] != BOUNCE_CHECKER_CODE )
					return 1;
			}
		for( int y=0;y<SRC_Height/2;y++)
			for( int x=0;x<BOUNCE_CHECKER_SIZE/2;x+=4)
			{
				//U
				if( *(int*)&addressYUV1[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+Y_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV1[y*YUV_STRIDE_HALF+x+SRC_Width/2+Y_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV2[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+Y_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV2[y*YUV_STRIDE_HALF+x+SRC_Width/2+Y_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV3[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+Y_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV3[y*YUV_STRIDE_HALF+x+SRC_Width/2+Y_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				//V
				if( *(int*)&addressYUV1[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+YU_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV1[y*YUV_STRIDE_HALF+x+SRC_Width/2+YU_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV2[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+YU_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV2[y*YUV_STRIDE_HALF+x+SRC_Width/2+YU_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV3[y*YUV_STRIDE_HALF+x-BOUNCE_CHECKER_SIZE/2+YU_SIZE] != BOUNCE_CHECKER_CODE )
					return 1;
				if( *(int*)&addressYUV3[y*YUV_STRIDE_HALF+x+SRC_Width/2+YU_SIZE] != BOUNCE_CHECKER_CODE )
					return 1; 
			}
		//for RGB buffers
		for( int y=0;y<BOUNCE_CHECKER_SIZE;y++)
			for( int x=0;x<RGB_STRIDE;x+=4)
			{
				if( *(int*)&taddress4[y*RGB_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&addressRGB1[y*RGB_STRIDE+x+RGB_STRIDE*SRC_Height] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&taddress5[y*RGB_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&addressRGB2[y*RGB_STRIDE+x+RGB_STRIDE*SRC_Height] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&taddress6[y*RGB_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&addressRGB3[y*RGB_STRIDE+x+RGB_STRIDE*SRC_Height] != BOUNCE_CHECKER_CODE )
					return 2;
			}
		//column before and after image
		for( int y=0;y<SRC_Height;y++)
			for( int x=0;x<BOUNCE_CHECKER_SIZE;x+=4)
			{
				if( *(int*)&taddress4[y*RGB_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&addressRGB1[y*RGB_STRIDE+x+SRC_Width*3] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&taddress5[y*RGB_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&addressRGB2[y*RGB_STRIDE+x+SRC_Width*3] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&taddress6[y*RGB_STRIDE+x] != BOUNCE_CHECKER_CODE )
					return 2;
				if( *(int*)&addressRGB3[y*RGB_STRIDE+x+SRC_Width*3] != BOUNCE_CHECKER_CODE )
					return 2;
			} /**/
	}
	return 0;
}

enum TestTypes {
	NO_TEST,
	TEST_IYUV_RGB,
	TEST_IYUV_BGR,
	TEST_RGB_GRAYSCALE,
	TEST_RGB_PICTURE_SIMILARITY,
	TEST_BGR_PICTURE_SIMILARITY,
	TEST_IYUV_WATERMARK,
	TEST_RGB_IYUV,
	TEST_RGB_COLOURSAMPLING,
	TEST_MEMORY_ALLOCATOR,
	TEST_MEMORY_ALLOCATOR_DLL,
};

void PrintMenu()
{
	printf( "Available speed / functionality tests : \n");
	printf( "%d) IYUV420 -> RGB24 \n", TEST_IYUV_RGB );
	printf( "%d) IYUV420 -> BGR24 \n", TEST_IYUV_BGR );
	printf( "%d) RGB24 -> RGB24 grayscale conversion \n", TEST_RGB_GRAYSCALE );
	printf( "%d) RGB24 picture similarity \n", TEST_RGB_PICTURE_SIMILARITY );
	printf( "%d) BGR24 picture similarity \n", TEST_BGR_PICTURE_SIMILARITY );
	printf( "%d) IYUV watermark \n", TEST_IYUV_WATERMARK );
	printf( "%d) RGB24 -> IYUV420 \n", TEST_RGB_IYUV );
	printf( "%d) RGB24 Coloursampling \n", TEST_RGB_COLOURSAMPLING );
	printf( "%d) Memory Allocator \n", TEST_MEMORY_ALLOCATOR );
	printf( "%d) Memory Allocator from DLL \n", TEST_MEMORY_ALLOCATOR_DLL );
}

int main()
{
	taddress1 = (BYTE*)malloc( YUV_BUFF_SIZE );
	taddress2 = (BYTE*)malloc( YUV_BUFF_SIZE );
	taddress3 = (BYTE*)malloc( YUV_BUFF_SIZE );
	taddress4 = (BYTE*)malloc( RGB_BUFF_SIZE );
	taddress5 = (BYTE*)malloc( RGB_BUFF_SIZE );
	taddress6 = (BYTE*)malloc( RGB_BUFF_SIZE );
	//break alignment to get worst case scenario
#ifdef TEST_PLENTY_BUFFER_SIZE
	addressYUV1 = UnAllignAddress( taddress1 );
	addressYUV2 = UnAllignAddress( taddress2 );
	addressYUV3 = UnAllignAddress( taddress3 );
	addressRGB1 = UnAllignAddress( taddress4 );
	addressRGB2 = UnAllignAddress( taddress5 );
	addressRGB3 = UnAllignAddress( taddress6 );
#else
	addressYUV1 = taddress1 + BOUNCE_CHECKER_SIZE + BOUNCE_CHECKER_SIZE * YUV_STRIDE;
	addressYUV2 = taddress2 + BOUNCE_CHECKER_SIZE + BOUNCE_CHECKER_SIZE * YUV_STRIDE;
	addressYUV3 = taddress3 + BOUNCE_CHECKER_SIZE + BOUNCE_CHECKER_SIZE * YUV_STRIDE;
	addressRGB1 = taddress4 + BOUNCE_CHECKER_SIZE + BOUNCE_CHECKER_SIZE * RGB_STRIDE;
	addressRGB2 = taddress5 + BOUNCE_CHECKER_SIZE + BOUNCE_CHECKER_SIZE * RGB_STRIDE;
	addressRGB3 = taddress6 + BOUNCE_CHECKER_SIZE + BOUNCE_CHECKER_SIZE * RGB_STRIDE;
#endif

	int AntiInliningProtection;

#ifdef _DEBUG
	AntiInliningProtection = 1;
#else
	printf("Please press 1 and hit enter : ");
	scanf_s( "%d", &AntiInliningProtection );
	if( AntiInliningProtection != 1 )
		return 1;
#endif

	int SelectedTestType;
	PrintMenu();
	printf( "Select test type : " );
	scanf_s( "%d", &SelectedTestType );

	//load some data into input buffer so we may check output visually
	int LoadRet = LoadYUVImageFromFile( addressYUV1, SRC_Width, SRC_Height, AntiInliningProtection*YUV_STRIDE, 100, "d:/film/yuv/park_joy_1080p.yuv" );
	if( LoadRet != 0 )
	{
		for(int i=0;i<YUV_BUFF_SIZE;i++)
		{
			addressYUV1[i] = 55+i;
			addressYUV2[i] = 145+i;
			addressYUV3[i] = 175+i;
		}
		for(int i=0;i<RGB_BUFF_SIZE;i++)
		{
			addressRGB1[i] = 10+i; 
			addressRGB2[i] = 35+i; 
			addressRGB3[i] = 65+i; 
		}
	}
	//init BounceCheckerBorder
	CheckBoundsContent( 1 );

	if( SelectedTestType == TEST_IYUV_RGB )
	{
		printf("\nStarting IYUV420 -> RGB24 conversion speed test\n");
		int SrcYuvSize = AntiInliningProtection * YUV_STRIDE * AntiInliningProtection * SRC_Height * 3 / 2;
		int DstRGBSize = AntiInliningProtection * RGB_STRIDE * AntiInliningProtection * SRC_Height * 3 / 2;
		DoSpeedTestYUV2RGB( addressYUV1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressRGB1, AntiInliningProtection*RGB_STRIDE );
		ConvertYV12toRGB( AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressYUV1, SrcYuvSize, addressRGB1, DstRGBSize, AntiInliningProtection*RGB_STRIDE );
		IYUV_to_RGB24_reference( addressYUV1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressRGB2, AntiInliningProtection*RGB_STRIDE );
		//check if we messed up the bounds
		if( CheckBoundsContent( ) )
			printf("Bounds checker reported an issue\n");
		else
			printf("Bounds found no overflow issue\n");
		if( memcmp( addressRGB1, addressRGB2, AntiInliningProtection*SRC_Height*AntiInliningProtection*RGB_STRIDE ) != 0 )
		{
			//printf("Mismatch detected in output");
			for( int y = 0; y < AntiInliningProtection*SRC_Height; y++ )
				for( int x = 0; x < AntiInliningProtection*SRC_Width*3; x++ )
				{
					int ind = y*AntiInliningProtection*RGB_STRIDE+x;
					if( addressRGB1[ind] != addressRGB2[ind] )
					{
						printf("Pixel mismatch at %d x %d ( Y x X )\n",y,x );
						y=0x0FFFFFFF;
						break;
					}
				}
		}
		else
			printf("Found no mismatch in output compared to reference function \n");

		//chek the output visually
		if( LoadRet == 0 )
		{
			char OutFileName[500];
			sprintf_s( OutFileName, 500, "out_colorconvert_%d_%d.y4m", AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height );
			RGB24_to_IYUV_reference( addressRGB1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, addressYUV2, AntiInliningProtection*YUV_STRIDE );	
			SaveYUVImageToFile( addressYUV2, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, OutFileName, 1 );
		}/**/
	}

	if( SelectedTestType == TEST_IYUV_BGR )
	{
		printf("\nStarting IYUV420 -> BGR24 conversion speed test\n");
		int SrcYuvSize = AntiInliningProtection * YUV_STRIDE * AntiInliningProtection * SRC_Height * 3 / 2;
		int DstRGBSize = AntiInliningProtection * RGB_STRIDE * AntiInliningProtection * SRC_Height * 3 / 2;
		DoSpeedTestBGR2YUV( addressYUV1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressRGB1, AntiInliningProtection*RGB_STRIDE );
		ConvertYV12toBGR( AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressYUV1, SrcYuvSize, addressRGB1, DstRGBSize, AntiInliningProtection*RGB_STRIDE );
		IYUV_to_BGR24_reference( addressYUV1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressRGB2, AntiInliningProtection*RGB_STRIDE );
		//check if we messed up the bounds
		if( CheckBoundsContent( ) )
			printf("Bounds checker reported an issue\n");
		else
			printf("Bounds found no overflow issue\n");
		if( memcmp( addressRGB1, addressRGB2, AntiInliningProtection*SRC_Height*AntiInliningProtection*RGB_STRIDE ) != 0 )
		{
			//printf("Mismatch detected in output");
			for( int y = 0; y < AntiInliningProtection*SRC_Height; y++ )
				for( int x = 0; x < AntiInliningProtection*SRC_Width*3; x++ )
				{
					int ind = y*AntiInliningProtection*RGB_STRIDE+x;
					if( addressRGB1[ind] != addressRGB2[ind] )
					{
						printf("Pixel mismatch at %d x %d ( Y x X )\n",y,x );
						y=0x0FFFFFFF;
						break;
					}
				}
		}
		else
			printf("Found no mismatch in output compared to reference function \n");
	}/**/

	if( SelectedTestType == TEST_RGB_GRAYSCALE )
	{
		printf("\nStarting RGB24 -> RGB24 grayscale conversion speed test\n");
		DoSpeedTestRGB2GrayScale( addressYUV1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressRGB1, AntiInliningProtection*RGB_STRIDE );
		//do content test
		RGB24_to_GrayScale_reference( addressRGB1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, addressRGB2, AntiInliningProtection*RGB_STRIDE, 16 );
		ConvertRGBtoGrayscale( AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, addressRGB1, 16 );
		//check if we messed up the bounds
		if( CheckBoundsContent( ) )
			printf("Bounds checker reported an issue\n");
		else
			printf("Bounds found no overflow issue\n");
		if( memcmp( addressRGB1, addressRGB2, AntiInliningProtection*SRC_Height*AntiInliningProtection*RGB_STRIDE ) != 0 )
		{
			//printf("Mismatch detected in output");
			for( int y = 0; y < AntiInliningProtection*SRC_Height; y++ )
				for( int x = 0; x < AntiInliningProtection*SRC_Width; x++ )
				{
					int ind = y*AntiInliningProtection*RGB_STRIDE+x;
					if( addressRGB1[ind] != addressRGB2[ind] )
					{
						printf("Pixel mismatch at %d x %d ( Y x X )\n",x,y );
						y=0x0FFFFFFF;
						break;
					}
				}
		}
		else
			printf("Found no mismatch in output compared to reference function \n");
	}/**/

	if( SelectedTestType == TEST_RGB_PICTURE_SIMILARITY )
	{
		printf("\nStarting RGB24 picture similarity speed test\n");
		memcpy( addressRGB3, addressRGB2, SRC_Height * RGB_STRIDE );
		float res1 = CompareBitmapRegions( AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, 1, 1, AntiInliningProtection*SRC_Width-2, AntiInliningProtection*SRC_Height-2, addressRGB1, addressRGB2, 1 );
//		float res1 = CompareRGBRegions_2(address2, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*SRC_Width*3, 1, 1, AntiInliningProtection*SRC_Width-2, AntiInliningProtection*SRC_Height-2, address1, 1 );
		float res2 = CompareRGBRegions_reference(addressRGB3, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, 1, 1, AntiInliningProtection*SRC_Width-2, AntiInliningProtection*SRC_Height-2, addressRGB1, 1 );
		if( res1 != res2 )
			printf("picture similarity result mismatch : %f - %f\n", res1, res2 );
		//check if we messed up the bounds
		if( CheckBoundsContent( ) )
			printf("Bounds checker reported an issue\n");
		else
			printf("Bounds found no overflow issue\n");
		if( memcmp( addressRGB2, addressRGB3, AntiInliningProtection*SRC_Height*AntiInliningProtection*RGB_STRIDE ) != 0 )
		{
			//printf("Mismatch detected in output");
			for( int y = 0; y < AntiInliningProtection*SRC_Height; y++ )
				for( int x = 0; x < AntiInliningProtection*SRC_Width; x++ )
				{
					int ind = y*AntiInliningProtection*RGB_STRIDE+x;
					if( addressRGB2[ind] != addressRGB3[ind] )
					{
						printf("Pixel mismatch at %d x %d ( Y x X )\n",x,y );
						y=0x0FFFFFFF;
						break;
					}
				}
		}
		else
			printf("Found no mismatch in output compared to reference function \n");
		DoSpeedTestRGBCompare( addressRGB1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, 1, 1, AntiInliningProtection*SRC_Width-2, AntiInliningProtection*SRC_Height-2, addressRGB2, 1 );
	}/**/

	if( SelectedTestType == TEST_BGR_PICTURE_SIMILARITY )
	{
		printf("\nStarting BGR24 picture similarity speed test\n");
		memcpy( addressRGB3, addressRGB2, SRC_Height * RGB_STRIDE );
//		float res1 = CompareBitmapRegions( AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, 1, 1, AntiInliningProtection*SRC_Width-2, AntiInliningProtection*SRC_Height-2, address1, address2, 1 );
		float res1 = CompareBGRRegions_1(addressRGB2, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, 1, 1, AntiInliningProtection*SRC_Width-2, AntiInliningProtection*SRC_Height-2, addressRGB1, 1 );
		float res2 = CompareBGRRegions_reference(addressRGB3, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, 1, 1, AntiInliningProtection*SRC_Width-2, AntiInliningProtection*SRC_Height-2, addressRGB1, 1 );
		if( res1 != res2 )
			printf("picture similarity result mismatch : %f - %f\n", res1, res2 );
		//check if we messed up the bounds
		if( CheckBoundsContent( ) )
			printf("Bounds checker reported an issue\n");
		else
			printf("Bounds found no overflow issue\n");
		if( memcmp( addressRGB2, addressRGB3, AntiInliningProtection*SRC_Height*AntiInliningProtection*RGB_STRIDE ) != 0 )
		{
			//printf("Mismatch detected in output");
			for( int y = 0; y < AntiInliningProtection*SRC_Height; y++ )
				for( int x = 0; x < AntiInliningProtection*SRC_Width; x++ )
				{
					int ind = y*AntiInliningProtection*RGB_STRIDE+x;
					if( addressRGB2[ind] != addressRGB3[ind] )
					{
						printf("Pixel mismatch at %d x %d ( Y x X )\n",x,y );
						y=0x0FFFFFFF;
						break;
					}
				}
		}
		else
			printf("Found no mismatch in output compared to reference function \n");
		DoSpeedTestRGBCompare( addressRGB1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, 1, 1, AntiInliningProtection*SRC_Width-2, AntiInliningProtection*SRC_Height-2, addressRGB2, 1 );
	}/**/

	if( SelectedTestType == TEST_IYUV_WATERMARK )
	{
		printf("\nStarting IYUV watermark speed test\n");
		//resize the input
		ResampleYUV420LiniarSSE3( addressYUV1, addressYUV2, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, AntiInliningProtection*SRC_Width / 3, AntiInliningProtection*SRC_Height / 3, AntiInliningProtection*YUV_STRIDE, AntiInliningProtection*SRC_Height / 3, 0, 0 );
		//do speed test
		DoSpeedTestWatermark( addressYUV2, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressYUV3, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE );
		//copy src to dest buffer to check for output quality
		memcpy( addressYUV3, addressYUV1, AntiInliningProtection*SRC_Height * AntiInliningProtection*YUV_STRIDE * 3 / 2 );
		//insert water mark again
		InsertWatermark_reference( addressYUV2, AntiInliningProtection*SRC_Width / 3, AntiInliningProtection*SRC_Height / 3, AntiInliningProtection*YUV_STRIDE, 0, 0, AntiInliningProtection*SRC_Width / 3, AntiInliningProtection*SRC_Height / 3, addressYUV3,  AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, AntiInliningProtection*SRC_Width / 3, AntiInliningProtection*SRC_Height / 3 );
		//chek the output visually
		{
			char OutFileName[500];
			sprintf_s( OutFileName, 500, "out_watermark_resized_%d_%d.y4m", AntiInliningProtection*SRC_Width / 3, AntiInliningProtection*SRC_Height / 3 );
			SaveYUVImageToFile( addressYUV2, AntiInliningProtection*SRC_Width / 3, AntiInliningProtection*SRC_Height / 3, AntiInliningProtection*YUV_STRIDE, OutFileName, 1 );
			sprintf_s( OutFileName, 500, "out_watermark_%d_%d.y4m", AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height );
			SaveYUVImageToFile( addressYUV3, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, OutFileName, 1 );
		}/**/
	}

	if( SelectedTestType == TEST_RGB_IYUV )
	{
		printf("\nStarting RGB24 -> IYUV420  conversion speed test\n");
		if( LoadRet == 0 )
		{
			//duplicate input buffer
			int SrcYuvSize = AntiInliningProtection * YUV_STRIDE * AntiInliningProtection * SRC_Height * 3 / 2;
			int DstRGBSize = AntiInliningProtection * RGB_STRIDE * AntiInliningProtection * SRC_Height * 3 / 2;
			memcpy( addressYUV2, addressYUV1, AntiInliningProtection*SRC_Height * AntiInliningProtection*YUV_STRIDE * 3 / 2 );
			ConvertYV12toRGB( AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressYUV1, SrcYuvSize, addressRGB1, DstRGBSize, AntiInliningProtection*RGB_STRIDE );
		}
		DoSpeedTestRGB2YUV( addressRGB1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, addressYUV3, AntiInliningProtection*YUV_STRIDE );
		RGB24_to_IYUV_1( addressRGB1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, addressYUV2, AntiInliningProtection*YUV_STRIDE );
		RGB24_to_IYUV_reference1( addressRGB1, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, addressYUV3, AntiInliningProtection*YUV_STRIDE );	
		//check if we messed up the bounds
		if( CheckBoundsContent( ) )
			printf("Bounds checker reported an issue\n");
		else
			printf("Bounds found no overflow issue\n");
		if( memcmp( addressYUV2, addressYUV3, AntiInliningProtection*SRC_Height * AntiInliningProtection*YUV_STRIDE * 3 / 2 ) != 0 )
		{
			printf("Mismatch detected in output\n");
			for( int y = 0; y < AntiInliningProtection*SRC_Height; y++ )
				for( int x = 0; x < AntiInliningProtection*SRC_Width; x++ )
				{
					int ind = y*AntiInliningProtection*YUV_STRIDE+x;
					if( addressYUV2[ind] != addressYUV3[ind] )
					{
						printf("Pixel mismatch at Y %d x %d ( Y x X ) : %d != %d\n",y,x,addressYUV2[ind],addressYUV3[ind] );
						y=0x0FFFFFFF;
						break;
					}
				}
			for( int y = 0; y < AntiInliningProtection*SRC_Height/2; y++ )
				for( int x = 0; x < AntiInliningProtection*SRC_Width/2; x++ )
				{
					int ind = AntiInliningProtection*SRC_Height * AntiInliningProtection*YUV_STRIDE + y*AntiInliningProtection*YUV_STRIDE/2+x;
					if( addressYUV2[ind] != addressYUV3[ind] )
					{
						printf("Pixel mismatch at U %d x %d ( Y x X ) : %d != %d\n",y,x,addressYUV2[ind],addressYUV3[ind] );
						y=0x0FFFFFFF;
						break;
					}
				}
			for( int y = 0; y < AntiInliningProtection*SRC_Height/2; y++ )
				for( int x = 0; x < AntiInliningProtection*SRC_Width/2; x++ )
				{
					int ind = AntiInliningProtection*SRC_Height * AntiInliningProtection*YUV_STRIDE + AntiInliningProtection*SRC_Height/2 * AntiInliningProtection*YUV_STRIDE/2 + y*AntiInliningProtection*YUV_STRIDE/2+x;
					if( addressYUV2[ind] != addressYUV3[ind] )
					{
						printf("Pixel mismatch at V %d x %d ( Y x X ) : %d != %d\n",y,x,addressYUV2[ind],addressYUV3[ind] );
						y=0x0FFFFFFF;
						break;
					}
				}
		}
		else
			printf("Found no mismatch in output compared to reference function \n");
		//chek the output visually
		{
			char OutFileName[500];
			sprintf_s( OutFileName, 500, "out_colorconvert_%d_%d.y4m", AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height );
			SaveYUVImageToFile( addressYUV3, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, OutFileName, 1 );
		}/**/
	}

	if( SelectedTestType == TEST_RGB_COLOURSAMPLING )
	{
		printf("\nStarting Color samplings speed test\n");
		if( LoadRet == 0 )
		{
			//duplicate input buffer
			int SrcYuvSize = AntiInliningProtection * YUV_STRIDE * AntiInliningProtection * SRC_Height * 3 / 2;
			int DstRGBSize = AntiInliningProtection * RGB_STRIDE * AntiInliningProtection * SRC_Height * 3 / 2;
			memcpy( addressYUV2, addressYUV1, AntiInliningProtection*SRC_Height * AntiInliningProtection*YUV_STRIDE * 3 / 2 );
			ConvertYV12toRGB( AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, addressYUV1, SrcYuvSize, addressRGB1, DstRGBSize, AntiInliningProtection*RGB_STRIDE );
		}
		RGB24Color Ref,Tol,Ret1,Ret2;
		float NrRet1,NrRet2;
		Ref.R = Ref.G = Ref.B = 128;
		Tol.R = Tol.G = Tol.B = 64;
		NrRet1 = GetColorValues_reference( addressRGB1, AntiInliningProtection*RGB_STRIDE, 1, 1, SRC_Width-1, SRC_Height-1, &Ref, &Tol, &Ret1 );
		NrRet2 = GetColorValues_6( addressRGB1, AntiInliningProtection*RGB_STRIDE, 1, 1, SRC_Width-1, SRC_Height-1, &Ref, &Tol, &Ret2 );
		if( NrRet1 != NrRet2 || Ret1.R != Ret2.R || Ret1.G != Ret2.G || Ret1.B != Ret2.B )
			printf( "Output mismatch\n");
		DoSpeedTestColorSampling( addressRGB1, AntiInliningProtection*RGB_STRIDE, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height );
	}/**/

	if( SelectedTestType == TEST_MEMORY_ALLOCATOR )
	{
		printf("\nStarting memory allocator test from local\n");
		unsigned char *TempYUVStore;
		unsigned char *TempRGBStore;
		if( _AllocateRGBMemory( &TempRGBStore, AntiInliningProtection*RGB_STRIDE * AntiInliningProtection*SRC_Height, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE ) )
		{
			printf( "We failed to allocate the RGB memory\n");
			return 1;
		}
		if( _AllocateYUVMemory( &TempYUVStore, AntiInliningProtection*YUV_STRIDE * AntiInliningProtection*SRC_Height * 3 / 2, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE ) )
		{
			printf( "We failed to allocate the YUV memory\n");
			return 1;
		}
		if( LoadRet == 0 )
		{
			//duplicate input buffer
			memcpy( TempYUVStore, addressYUV1, AntiInliningProtection*SRC_Height * AntiInliningProtection*YUV_STRIDE * 3 / 2 );
			//lock memory 
			_MemorySpinLockEnterTransaction( TempYUVStore );
			_MemorySpinLockEnterTransaction( TempRGBStore );
			//do some operations
			IYUV_to_RGB24_8( TempYUVStore, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, TempRGBStore, AntiInliningProtection*RGB_STRIDE );
			RGB24_to_IYUV_1( TempRGBStore, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, TempYUVStore, AntiInliningProtection*YUV_STRIDE );
			//release memory lock
			_MemorySpinLockExitTransaction( TempYUVStore );
			_MemorySpinLockExitTransaction( TempRGBStore );
			//dealloc memory
			int ErrorCodeFreeYUV = _FreeMemoryWaitForTransactions( TempYUVStore );
			int ErrorCodeFreeRGB = _FreeMemoryWaitForTransactions( TempRGBStore );
			if( ErrorCodeFreeYUV )
				printf(" Error code while trying to free buffer : %d \n ", ErrorCodeFreeYUV );
			if( ErrorCodeFreeRGB )
				printf(" Error code while trying to free buffer : %d \n ", ErrorCodeFreeRGB );
		}
		printf("Done testing : Allocator, Transaction Lock, Deallocator\n");
	}/**/

	if( SelectedTestType == TEST_MEMORY_ALLOCATOR_DLL )
	{
		printf("\nStarting memory allocator test from DLL\n");
		unsigned char *TempYUVStore;
		unsigned char *TempRGBStore;
		if( AllocateRGBMemory( &TempRGBStore, AntiInliningProtection*RGB_STRIDE * AntiInliningProtection*SRC_Height, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE ) )
		{
			printf( "We failed to allocate the RGB memory\n");
			return 1;
		}
		if( AllocateYUVMemory( &TempYUVStore, AntiInliningProtection*YUV_STRIDE * AntiInliningProtection*SRC_Height * 3 / 2, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE ) )
		{
			printf( "We failed to allocate the YUV memory\n");
			return 1;
		}
		if( LoadRet == 0 )
		{
			//duplicate input buffer
			memcpy( TempYUVStore, addressYUV1, AntiInliningProtection*SRC_Height * AntiInliningProtection*YUV_STRIDE * 3 / 2 );
			//lock memory 
			MemorySpinLockEnterTransaction( TempYUVStore );
			MemorySpinLockEnterTransaction( TempRGBStore );
			//do some operations
			IYUV_to_RGB24_8( TempYUVStore, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*YUV_STRIDE, TempRGBStore, AntiInliningProtection*RGB_STRIDE );
			RGB24_to_IYUV_1( TempRGBStore, AntiInliningProtection*SRC_Width, AntiInliningProtection*SRC_Height, AntiInliningProtection*RGB_STRIDE, TempYUVStore, AntiInliningProtection*YUV_STRIDE );
			//release memory lock
			MemorySpinLockExitTransaction( TempYUVStore );
			MemorySpinLockExitTransaction( TempRGBStore );
			//dealloc memory
			int ErrorCodeFreeYUV = FreeMemoryWaitForTransactions( TempYUVStore );
			int ErrorCodeFreeRGB = FreeMemoryWaitForTransactions( TempRGBStore );
			if( ErrorCodeFreeYUV )
				printf(" Error code while trying to free buffer : %d \n ", ErrorCodeFreeYUV );
			if( ErrorCodeFreeRGB )
				printf(" Error code while trying to free buffer : %d \n ", ErrorCodeFreeRGB );
		}
		printf("Done testing : Allocator, Transaction Lock, Deallocator\n");
	}/**/

	printf("Press any key");
	_getch();

	free( taddress1 );
	free( taddress2 );
	free( taddress3 );

	return 0;

}


