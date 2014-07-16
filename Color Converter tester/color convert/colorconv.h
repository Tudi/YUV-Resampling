#ifndef _COLOR_CONV2_H_
#define _COLOR_CONV2_H_
#include <Windows.h>

//this was an experimetn to firther reduce image details to increase texture quality
//sasdly it darkens the image but it does not improve encode qualityin visible way
//#define CONVERSION_FORCED_PRECISSION_FACTOR_Y	*(int)90/100
//#define CONVERSION_FORCED_PRECISSION_FACTOR_UV	&( ~0x03 )
#define CONVERSION_FORCED_PRECISSION_FACTOR_Y	
#define CONVERSION_FORCED_PRECISSION_FACTOR_UV	

#define MAXJSAMPLE      255
#define CENTERJSAMPLE   128
#define TABLE_SIZE      (8*(MAXJSAMPLE+1))
#define SCALEBITS       16      
#define CBCR_OFFSET     ((INT32) CENTERJSAMPLE << SCALEBITS)
#define ONE_HALF        ((INT32) 1 << (SCALEBITS-1))
#define ROUND(x)        ((INT32) ((x) * (1L<<SCALEBITS) + 0.5))
#define R_Y_OFF         0                       // offset to R => Y section 
#define G_Y_OFF         (1*(MAXJSAMPLE+1))      // offset to G => Y section 
#define B_Y_OFF         (2*(MAXJSAMPLE+1))      // etc.
#define R_U_OFF         (3*(MAXJSAMPLE+1))
#define G_U_OFF         (4*(MAXJSAMPLE+1))
#define B_U_OFF         (5*(MAXJSAMPLE+1))
#define R_V_OFF         B_U_OFF                // B=>Cb, R=>Cr are the same 
#define G_V_OFF         (6*(MAXJSAMPLE+1))
#define B_V_OFF         (7*(MAXJSAMPLE+1))

//used to convert YUV to RGB
#define BITS_PER_PLANE			5
#define MAX_COLOR_SAMPLES		( 1 << ( BITS_PER_PLANE * 3 ) )	//pretty big precission lost from 24 bits
#define BYTES_FOR_LOOKUP		( MAX_COLOR_SAMPLES * 3 )
#define PRECISSION_DOWNSCALE	( 8 - BITS_PER_PLANE )
#define COLOR_STEP				( 1 << PRECISSION_DOWNSCALE )

//consumes a lot of memory and has this gradient effect. Not as usefull as i planned to be
//#define USE_BOOSTED_PRECISE_YUV_TO_RGB //gives around 30% speed boos

class ColorConverter
{
public:
		ColorConverter();
		~ColorConverter();
		// Y size : X*Y 
		// U size : x/2*y/2
		// V size : x/2*y/2
// 		void RGB24_to_YUV(BYTE *src, BYTE *dst, ULONG w, ULONG h);
		//this barelly has any affect on the quality
 		void BGR24_to_YUV_interpolate(BYTE *src, BYTE *dst, ULONG w, ULONG h);
 		void BGR24_to_YUV_interpolate_NX(BYTE *src, BYTE *dstY, BYTE *dstU, BYTE *dstV, ULONG w, ULONG h,ULONG dest_stride);	//special buffer setup to be cache aligned
 		void BGR24_to_YUV(BYTE *src, BYTE *dst, ULONG w, ULONG h);
 		void BGR24_to_YUV_NX(BYTE *src, BYTE *dstY, BYTE *dstU, BYTE *dstV, ULONG w, ULONG h,ULONG dest_stride);	//special buffer setup to be cache aligned
		void BGR24_to_YUV_2x2_smooth(BYTE *src, BYTE *dst, ULONG w, ULONG h);
 		void YUV_to_BGR24(BYTE *src, BYTE *dst, ULONG w, ULONG h);
 		void YUV_to_BGR24_precize(BYTE *src, BYTE *dst, ULONG w, ULONG h);
//private:
		BYTE rgb_to_y(UINT r,UINT g,UINT b);
		BYTE rgb_to_u(UINT r,UINT g,UINT b);
		BYTE rgb_to_v(UINT r,UINT g,UINT b);
		INT32 rgb_ycc_tab[ TABLE_SIZE ];               //colorspace conversion
#ifdef USE_BOOSTED_PRECISE_YUV_TO_RGB
		BYTE YUV_to_RGB[ BYTES_FOR_LOOKUP ];
#endif
		// implementation by Hoi Ming
		int RGB2YUV_YR[256];
		int RGB2YUV_YG[256];
		int RGB2YUV_YB[256];
		int RGB2YUV_UR[256];
		int RGB2YUV_UG[256];
		int RGB2YUV_UB[256];
		int RGB2YUV_VR[256];
		int RGB2YUV_VG[256];
		int RGB2YUV_VB[256];
		unsigned char *uu,*vv;
		ULONG inited_h,inited_w;
};

#endif