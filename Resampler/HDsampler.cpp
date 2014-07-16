// **********************************************************************************************
//                              Copyright (C) 2009  - SPLASH Software 
//
//                    Use or duplication without written consent prohibitted
// **********************************************************************************************
#include <windows.h>
#include <stdlib.h>

#ifdef _DEBUG
	#include <stdio.h>
#endif

#include "HDResampler.h"

//#define PUSH_SPEED_LIMITS_FOR_RGB_RESIZE

#define RGB_BYTE_COUNT	3
#define FLOAT_PRECISSION_BITS 15		// values are on 16 bits, using more for float precission does not count. Make sure to not have this number too big !
#define FLOAT_PRECISSION ( 1 << 15 )
#define YUV_SCALE_LOOKUP_PRECISSION 1	// influences the amount of memory required to store table. Don;t mess with this unless you know what you do

HDResampler::HDResampler(int _width1, int _height1, int _width2, int _height2,int _lumalow , int _lumahigh , int _chromalow ,int  _chromahigh )
{	
	in_h = _height1;
	in_w = _width1;
	out_h = _height2;
	out_w = _width2;
	if( _width2 != 0 )
		conv_x = (float)in_w/out_w;
	else conv_x = 1 ;//panic ?
	if( _height2 != 0 )
		conv_y = (float)in_h/out_h;
	else conv_y = 1 ;//panic ?

	conv_table = NULL;
	src_stride = dst_stride = 0;
}

//
// This is just to explain the simplest version of raw resizing. As you can see there is no quality 
//
/*
void HDResampler::ResampleRGB24_anyscale(PBYTE src, int   src_stride, PBYTE dest, int   dest_stride )
{
	for (unsigned int y = 0; y < out_h; y++)
		for (unsigned int x = 0; x < out_w; x++)
		{
			dest[y * dest_stride + x * 3 + 0 ] = src[ (unsigned int)( y * conv_y ) * src_stride + (unsigned int)( x * conv_x ) * 3 + 0 ];
			dest[y * dest_stride + x * 3 + 1 ] = src[ (unsigned int)( y * conv_y ) * src_stride + (unsigned int)( x * conv_x ) * 3 + 1 ];
			dest[y * dest_stride + x * 3 + 2 ] = src[ (unsigned int)( y * conv_y ) * src_stride + (unsigned int)( x * conv_x ) * 3 + 2 ];
		}
}/**/

#define SLOW_COPY_MOST_IMAGE_RGB { \
	/*move values by 4 instead of 3 times*/ \
	for (unsigned int y = 0; y < out_h - 1; y++) \
	{ \
		unsigned int dest_ind = y * dest_stride; \
		unsigned int src_row_selector = (unsigned int)(y * conv_y); \
		unsigned int src_ind_base = src_row_selector * src_stride; \
		unsigned int stacking_src_precision = 0; \
		unsigned int src_precision_inc = (unsigned int)(conv_x * FLOAT_PRECISSION); \
		unsigned int src_ind = src_ind_base ; \
		for (unsigned int x = 0; x < out_w; x++ ) \
			{ \
				*(int*)&dest[ dest_ind ] = *(int*)&src[ (unsigned int)( src_ind ) ]; \
				dest_ind += 3; \
				stacking_src_precision += src_precision_inc; \
				src_ind = src_ind_base + ( stacking_src_precision / FLOAT_PRECISSION ) * 3; \
			} \
	} \
}

#define SLOW_COPY_LAST_ROW_RGB { \
	unsigned int dest_ind = ( out_h - 1 ) * dest_stride; \
	unsigned int src_row_selector = (unsigned int)(( out_h - 1 ) * conv_y); \
	if ( src_row_selector >= in_h ) \
		src_row_selector = in_h - 1; \
	unsigned int float_precision = 1 << FLOAT_PRECISSION_BITS; \
	unsigned int src_ind_base = src_row_selector * src_stride; \
	unsigned int stacking_src_precision = 0; \
	unsigned int src_precision_inc = (unsigned int)(conv_y * float_precision); \
	if( src_precision_inc * out_w / float_precision > in_h ) \
		src_precision_inc = in_h * float_precision / out_w; \
	unsigned int src_ind = src_ind_base ; \
	for (unsigned int x = 0; x < out_w; x++ ) \
		{ \
			dest[ dest_ind + 0 ] = src[ src_ind + 0 ]; \
			dest[ dest_ind + 1 ] = src[ src_ind + 1 ]; \
			dest[ dest_ind + 2 ] = src[ src_ind + 2 ]; \
			dest_ind += 3; \
			stacking_src_precision += src_precision_inc; \
			src_ind = src_ind_base + ( stacking_src_precision / float_precision ) * 3; \
		} \
}

//optimization 2
void HDResampler::ResampleRGB24_anyscale(PBYTE src, int   src_stride, PBYTE dest, int   dest_stride )
{
	SLOW_COPY_MOST_IMAGE_RGB
	//rest needs to be moved normally
//	SLOW_COPY_LAST_ROW_RGB
}/**/

void HDResampler::ResampleRGB24_downscale(PBYTE src, int   src_stride, PBYTE dest, int   dest_stride )
{
	//the error is consealed in every max 8 pixel copy and it does not stack up to the end of the row
	//see what is the skip ratio : generate a skip mask
	// 1.20 = 1111

	// 1.30 = 1110
	// 1.40 = 1101

	// 1.60 = 1011
	// 1.80 = 1010
	// 2.50 = 0101

	// 2.89 = 0100
	// 3.89 = 0010
	// 4.89 = 0001

	// source is larger x times then output
	if( conv_x >= 4.89f )
		SLOW_COPY_MOST_IMAGE_RGB
	// source is larger x times then output
	else if( conv_x >= 2.89f )
	{
		// 1 pixel out of 4 : 0100
#define STORE_STEPS 4	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 4	// since looping represents at least 10% of processing we unroll it 
		unsigned int	float_precission = (1 << FLOAT_PRECISSION_BITS); // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * STORE_STEPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT + ( out_w % STORE_STEPS ) * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//columns precision is not vital since we skip processing last row
		if( inc_src_col_int * col_copy_count / float_precission > (unsigned int)src_stride )
			inc_src_col_int = src_stride * float_precission / col_copy_count; //result should be less then the stride now due to integer divisions
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//very sensible to read access violations 
		if( inc_src_row_int * out_h_local / float_precission > in_h )
			inc_src_row_int = in_h * float_precission / out_h_local; //result should be less then the stride now due to integer divisions
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows

		mov		EBX, 0				// store stacking float precisions for increasing row
		mov		EDI, dest			// the destination addr in EDI
row_loop1:
		push	EDX					// save row loop
		push	EBX

		mov		ESI, tsrc			// base src addr
		mov		ECX, 0				// ECX stores float increase as int

		mov		EBX, col_copy_count	// times to loop for column
col_loop1:
		mov		EAX, [ ESI + 3 * 1 ]// get 4 bytes instead of 3 from src
		mov		[ EDI ], EAX		// store the pixel at the destination 1

		mov		EAX, [ ESI + 3 * 5 ]// get 4 bytes instead of 3 from src : 01000100
		mov		[ EDI + 3], EAX		// store the pixel at the destination 1

		mov		EAX, [ ESI + 3 * 9 ]// get 4 bytes instead of 3 from src : 0100010001000100
		mov		[ EDI + 6 ], EAX	// store the pixel at the destination 1-> loop unroll 1

		mov		EAX, [ ESI + 3 * 13 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 9 ], EAX	// store the pixel at the destination 1

		add		EDI, STORE_STEPS * RGB_BYTE_COUNT // advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// backup the sum of floats = conv_x
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		imul	eax,RGB_BYTE_COUNT		// position to a start of a pixel
	
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop1			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		IMUL	EAX, src_stride		// jump to the next src row we should read
		mov		ecx, src			// make sure tsrc points to the next beggining of the row
		add		ecx, eax			// new source addrr points to Xth row from original source
		mov		tsrc, ecx			// store in some addr

		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading


		pop		EDX
		sub		EDX, 1				// decrease row loop counter
		jnz		row_loop1			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	// source is larger x times then output
	else if( conv_x >= 1.60f )
	{
		// 2 pixel out of 4 : 1010
#define STORE_STEPS 4	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 2	// since looping represents at least 10% of processing we unroll it 
		unsigned int	float_precission = (1 << FLOAT_PRECISSION_BITS); // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * STORE_STEPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / STORE_STEPS ;
		unsigned int	col_copy_count_fetch = 128 / STORE_STEPS;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT + ( out_w % STORE_STEPS ) * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//columns precision is not vital since we skip processing last row
		if( inc_src_col_int * col_copy_count / float_precission > (unsigned int)src_stride )
			inc_src_col_int = src_stride * float_precission / col_copy_count; //result should be less then the stride now due to integer divisions
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//very sensible to read access violations 
		if( inc_src_row_int * out_h_local / float_precission > in_h )
			inc_src_row_int = in_h * float_precission / out_h_local; //result should be less then the stride now due to integer divisions
		unsigned char *last_dst_addr = dest + out_h*out_w*3;
		unsigned char *last_src_addr = dest + in_h*in_w*3;
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows

		mov		EBX, 0				// store stacking float precisions for increasing row
		mov		EDI, dest			// the destination addr in EDI
row_loop2:
		push	EDX					// save row loop
		push	EBX

		mov		ESI, tsrc			// base src addr
		mov		ECX, 0				// ECX stores float increase as int

		mov		EBX, col_copy_count	// times to loop for column
col_loop2:
		mov		EAX, [ ESI ]		// get 4 bytes instead of 3 from src
		mov		[ EDI ], EAX		// store the pixel at the destination 1
		mov		EAX, [ ESI + 2 * 3 ]// get 4 bytes instead of 3 from src 1010
		mov		[ EDI + 3 ], EAX	// store the pixel at the destination 1

		mov		EAX, [ ESI + 4 * 3 ]// get 4 bytes instead of 3 from src : 10101010
		mov		[ EDI + 6 ], EAX	// store the pixel at the destination 1 -> loop unroll 1
		mov		EAX, [ ESI + 6 * 3 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 9 ], EAX	// store the pixel at the destination 1

		add		EDI, STORE_STEPS*RGB_BYTE_COUNT // advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// backup the sum of floats = conv_x
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		imul	eax,RGB_BYTE_COUNT	// position to a start of a pixel
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop2			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		IMUL	EAX, src_stride		// jump to the next src row we should read
		mov		ecx, src			// make sure tsrc points to the next beggining of the row
		add		ecx, eax			// new source addrr points to Xth row from original source
		mov		tsrc, ecx			// store in some addr

		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading


		pop		EDX
		sub		EDX, 1				// decrease row loop counter
		jnz		row_loop2			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	// source is larger x times then output
	else if( conv_x >= 1.30f )
	{
		// 3 pixel out of 4 : 1101
#define STORE_STEPS 6	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 2	// since looping represents at least 10% of processing we unroll it 
		unsigned int	float_precission = (1 << FLOAT_PRECISSION_BITS); // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * STORE_STEPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT + ( out_w % STORE_STEPS ) * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//columns precision is not vital since we skip processing last row
		if( inc_src_col_int * col_copy_count / float_precission > (unsigned int)src_stride )
			inc_src_col_int = src_stride * float_precission / col_copy_count; //result should be less then the stride now due to integer divisions
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//very sensible to read access violations 
		if( inc_src_row_int * out_h_local / float_precission > in_h )
			inc_src_row_int = in_h * float_precission / out_h_local; //result should be less then the stride now due to integer divisions
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows

		mov		EBX, 0				// store stacking float precisions for increasing row
		mov		EDI, dest			// the destination addr in EDI
row_loop3:
		push	EDX					// save row loop
		push	EBX

		mov		ESI, tsrc			// base src addr
		mov		ECX, 0				// ECX stores float increase as int

		mov		EBX, col_copy_count	// times to loop for column
col_loop3:
		mov		EAX, [ ESI + 0 * 3 ]// get 4 bytes instead of 3 from src
		mov		[ EDI ], EAX		// store the pixel at the destination 1
		mov		EAX, [ ESI + 1 * 3 ]// get 4 bytes instead of 3 from src 1101
		mov		[ EDI + 3 ], EAX	// store the pixel at the destination 1
		mov		EAX, [ ESI + 3 * 3 ]// get 4 bytes instead of 3 from src 1101
		mov		[ EDI + 6 ], EAX	// store the pixel at the destination 1

		mov		EAX, [ ESI + 4 * 3 ]// get 4 bytes instead of 3 from src : 11011101
		mov		[ EDI + 9 ], EAX	// store the pixel at the destination 1 -> loop unroll 1
		mov		EAX, [ ESI + 5 * 3 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 12 ], EAX	// store the pixel at the destination 1
		mov		EAX, [ ESI + 7 * 3 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 15 ], EAX	// store the pixel at the destination 1

		add		EDI, STORE_STEPS*RGB_BYTE_COUNT // advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// backup the sum of floats = conv_x
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		imul	eax,RGB_BYTE_COUNT		// position to a start of a pixel
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop3			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		IMUL	EAX, src_stride		// jump to the next src row we should read
		mov		ecx, src			// make sure tsrc points to the next beggining of the row
		add		ecx, eax			// new source addrr points to Xth row from original source
		mov		tsrc, ecx			// store in some addr

		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading


		pop		EDX
		sub		EDX, 1				// decrease row loop counter
		jnz		row_loop3			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	// source is larger x times then output
	else
	{
		// 4 pixel out of 4 : 1111
#define STORE_STEPS 4	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 1	// since looping represents at least 10% of processing we unroll it 
		unsigned int	float_precission = (1 << FLOAT_PRECISSION_BITS); // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * STORE_STEPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT + ( out_w % STORE_STEPS ) * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//columns precision is not vital since we skip processing last row
		if( inc_src_col_int * col_copy_count / float_precission > (unsigned int)src_stride )
			inc_src_col_int = src_stride * float_precission / col_copy_count; //result should be less then the stride now due to integer divisions
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//very sensible to read access violations 
		if( inc_src_row_int * out_h_local / float_precission > in_h )
			inc_src_row_int = in_h * float_precission / out_h_local; //result should be less then the stride now due to integer divisions
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows

		mov		EBX, 0				// store stacking float precisions for increasing row
		mov		EDI, dest			// the destination addr in EDI
row_loop4:
		push	EDX					// save row loop
		push	EBX

		mov		ESI, tsrc			// base src addr
		mov		ECX, 0				// ECX stores float increase as int

		mov		EBX, col_copy_count	// times to loop for column
col_loop4:
		mov		EAX, [ ESI + 0 * 3 ]// get 4 bytes instead of 3 from src
		mov		[ EDI ], EAX		// store the pixel at the destination 1
		mov		EAX, [ ESI + 1 * 3 ]// get 4 bytes instead of 3 from src 1111
		mov		[ EDI + 3 ], EAX	// store the pixel at the destination 1
		mov		EAX, [ ESI + 2 * 3 ]// get 4 bytes instead of 3 from src 1111
		mov		[ EDI + 6 ], EAX	// store the pixel at the destination 1
		mov		EAX, [ ESI + 3 * 3 ]// get 4 bytes instead of 3 from src 1111
		mov		[ EDI + 9 ], EAX	// store the pixel at the destination 1

/*		mov		EAX, [ ESI + 5 * 3 ]// get 4 bytes instead of 3 from src : 11111111
		mov		[ EDI + 12 ], EAX	// store the pixel at the destination 1 -> loop unroll 1
		mov		EAX, [ ESI + 6 * 3 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 15 ], EAX	// store the pixel at the destination 1
		mov		EAX, [ ESI + 7 * 3 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 18 ], EAX	// store the pixel at the destination 1*/

		add		EDI, STORE_STEPS*RGB_BYTE_COUNT // advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// backup the sum of floats = conv_x
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		imul	eax,RGB_BYTE_COUNT	// position to a start of a pixel
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop4			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		IMUL	EAX, src_stride		// jump to the next src row we should read
		mov		ecx, src			// make sure tsrc points to the next beggining of the row
		add		ecx, eax			// new source addrr points to Xth row from original source
		mov		tsrc, ecx			// store in some addr

		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading


		pop		EDX
		sub		EDX, 1				// decrease row loop counter
		jnz		row_loop4			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	//rest needs to be moved normally
	SLOW_COPY_LAST_ROW_RGB
}/**/

void HDResampler::ResampleRGB24_upscale(PBYTE src, int   src_stride, PBYTE dest, int   dest_stride )
{
	// source is max 2x smaller
	if( conv_x >= 0.5f )
	{
#define STORE_STEPS 2	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 3	// since looping represents at least 10% of processing we unroll it 
		unsigned int	float_precission = (1 << FLOAT_PRECISSION_BITS); // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * STORE_STEPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT + ( out_w % STORE_STEPS ) * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//columns precision is not vital since we skip processing last row
		if( inc_src_col_int * col_copy_count / float_precission > (unsigned int)src_stride )
			inc_src_col_int = src_stride * float_precission / col_copy_count; //result should be less then the stride now due to integer divisions
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//very sensible to read access violations 
		if( inc_src_row_int * out_h_local / float_precission > in_h )
			inc_src_row_int = in_h * float_precission / out_h_local; //result should be less then the stride now due to integer divisions
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows

		mov		EBX, 0				// store stacking float precisions for increasing row
		mov		EDI, dest			// the destination addr in EDI
row_loop2:
		push	EDX					// save row loop
		push	EBX

		mov		ESI, tsrc			// base src addr
		mov		ECX, 0				// ECX stores float increase as int

		mov		EBX, col_copy_count	// times to loop for column
col_loop2:
		mov		EAX, [ ESI ]		// get 4 bytes instead of 3 from src
		mov		[ EDI ], EAX		// store the pixel at the destination 1
		mov		[ EDI + 3], EAX		// store the pixel at the destination 2

		mov		EAX, [ ESI + 3 ]	// get 4 bytes instead of 3 from src -> loop unroll 1
		mov		[ EDI + 6], EAX		// store the pixel at the destination 1
		mov		[ EDI + 9], EAX		// store the pixel at the destination 2

		mov		EAX, [ ESI + 6 ]	// get 4 bytes instead of 3 from src -> loop unroll 1
		mov		[ EDI + 12], EAX	// store the pixel at the destination 1
		mov		[ EDI + 15], EAX	// store the pixel at the destination 2

		add		EDI, STORE_STEPS*RGB_BYTE_COUNT // advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// backup the sum of floats = conv_x
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		imul	eax,RGB_BYTE_COUNT	// position to a start of a pixel
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop2			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		IMUL	EAX, src_stride		// jump to the next src row we should read
		mov		ecx, src			// make sure tsrc points to the next beggining of the row
		add		ecx, eax			// new source addrr points to Xth row from original source
		mov		tsrc, ecx			// store in some addr

		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading


		pop		EDX
		sub		EDX, 1				// decrease row loop counter
		jnz		row_loop2			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	// source is max 3x smaller
	else if( conv_x >= 0.33f )
	{
#define STORE_STEPS 3
//#define LOOP_UNROLLS 2	// since looping represents at least 10% of processing we unroll it 
		unsigned int	float_precission = (1 << FLOAT_PRECISSION_BITS); // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * STORE_STEPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT + ( out_w % STORE_STEPS ) * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//columns precision is not vital since we skip processing last row
		if( inc_src_col_int * col_copy_count / float_precission > (unsigned int)src_stride )
			inc_src_col_int = src_stride * float_precission / col_copy_count; //result should be less then the stride now due to integer divisions
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//very sensible to read access violations 
		if( inc_src_row_int * out_h_local / float_precission > in_h )
			inc_src_row_int = in_h * float_precission / out_h_local; //result should be less then the stride now due to integer divisions
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows

		mov		EBX, 0				// store stacking float precisions for increasing row
		mov		EDI, dest			// the destination addr in EDI
row_loop3:
		push	EDX					// save row loop
		push	EBX

		mov		ESI, tsrc			// base src addr
		mov		ECX, 0				// ECX stores float increase as int

		mov		EBX, col_copy_count	// times to loop for column
col_loop3:
		mov		EAX, [ ESI ]		// get 4 bytes instead of 3 from src
		mov		[ EDI ], EAX		// store the pixel at the destination 1
		mov		[ EDI + 3], EAX		// store the pixel at the destination 2
		mov		[ EDI + 6], EAX		// store the pixel at the destination 2

		mov		EAX, [ ESI + 3]		// get 4 bytes instead of 3 from src
		mov		[ EDI + 9], EAX		// store the pixel at the destination 1
		mov		[ EDI + 12], EAX	// store the pixel at the destination 2
		mov		[ EDI + 15], EAX	// store the pixel at the destination 2

		add		EDI, STORE_STEPS*RGB_BYTE_COUNT // advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// backup the sum of floats = conv_x
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		imul	eax, RGB_BYTE_COUNT	// position to a start of a pixel
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop3			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		IMUL	EAX, src_stride		// jump to the next src row we should read
		mov		ecx, src			// make sure tsrc points to the next beggining of the row
		add		ecx, eax			// new source addrr points to Xth row from original source
		mov		tsrc, ecx			// store in some addr

		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading


		pop		EDX
		sub		EDX, 1				// decrease row loop counter
		jnz		row_loop3			// need more operations ?
//end row loop
		}
//#undef LOOP_UNROLLS
#undef STORE_STEPS
	}
	// source is max 4x smaller
	else if( conv_x >= 0.25f )
	{
#define STORE_STEPS 4
		unsigned int	float_precission = (1 << FLOAT_PRECISSION_BITS); // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * STORE_STEPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / STORE_STEPS;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT + ( out_w % STORE_STEPS ) * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//columns precision is not vital since we skip processing last row
		if( inc_src_col_int * col_copy_count / float_precission > (unsigned int)src_stride )
			inc_src_col_int = src_stride * float_precission / col_copy_count; //result should be less then the stride now due to integer divisions
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//very sensible to read access violations 
		if( inc_src_row_int * out_h_local / float_precission > in_h )
			inc_src_row_int = in_h * float_precission / out_h_local; //result should be less then the stride now due to integer divisions
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows

		mov		EBX, 0				// store stacking float precisions for increasing row
		mov		EDI, dest			// the destination addr in EDI
row_loop4:
		push	EDX					// save row loop
		push	EBX

		mov		ESI, tsrc			// base src addr
		mov		ECX, 0				// ECX stores float increase as int

		mov		EBX, col_copy_count	// times to loop for column
col_loop4:
		mov		EAX, [ ESI ]		// get 4 bytes instead of 3 from src
		mov		[ EDI ], EAX		// store the pixel at the destination 1
		mov		[ EDI + 3], EAX		// store the pixel at the destination 2
		mov		[ EDI + 6], EAX		// store the pixel at the destination 2
		mov		[ EDI + 9], EAX		// store the pixel at the destination 2
		add		EDI, STORE_STEPS*RGB_BYTE_COUNT// advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// backup the sum of floats = conv_x
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		imul	eax, RGB_BYTE_COUNT	// position to a start of a pixel
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop4			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		IMUL	EAX, src_stride		// jump to the next src row we should read
		mov		ecx, src			// make sure tsrc points to the next beggining of the row
		add		ecx, eax			// new source addrr points to Xth row from original source
		mov		tsrc, ecx			// store in some addr

		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading


		pop		EDX
		sub		EDX, 1				// decrease row loop counter
		jnz		row_loop4			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
	}
	// source is max 5x smaller
	else if( conv_x >= 0.20f )
	{
#define STORE_STEPS 5
		unsigned int	float_precission = (1 << FLOAT_PRECISSION_BITS); // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * STORE_STEPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / STORE_STEPS;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT + ( out_w % STORE_STEPS ) * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//columns precision is not vital since we skip processing last row
		if( inc_src_col_int * col_copy_count / float_precission > (unsigned int)src_stride )
			inc_src_col_int = src_stride * float_precission / col_copy_count; //result should be less then the stride now due to integer divisions
		//just make sure they will not point outside the source memory boundary due to stacking precission bug
		//very sensible to read access violations 
		if( inc_src_row_int * out_h_local / float_precission > in_h )
			inc_src_row_int = in_h * float_precission / out_h_local; //result should be less then the stride now due to integer divisions
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows

		mov		EBX, 0				// store stacking float precisions for increasing row
		mov		EDI, dest			// the destination addr in EDI
row_loop5:
		push	EDX					// save row loop
		push	EBX

		mov		ESI, tsrc			// base src addr
		mov		ECX, 0				// ECX stores float increase as int

		mov		EBX, col_copy_count	// times to loop for column
col_loop5:
		mov		EAX, [ ESI ]		// get 4 bytes instead of 3 from src
		mov		[ EDI ], EAX		// store the pixel at the destination 1
		mov		[ EDI + 3], EAX		// store the pixel at the destination 2
		mov		[ EDI + 6], EAX		// store the pixel at the destination 2
		mov		[ EDI + 9], EAX		// store the pixel at the destination 2
		mov		[ EDI + 12], EAX	// store the pixel at the destination 2
		add		EDI, STORE_STEPS*RGB_BYTE_COUNT// advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// backup the sum of floats = conv_x
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		imul	eax, RGB_BYTE_COUNT	// position to a start of a pixel
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop5			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		shr		eax, FLOAT_PRECISSION_BITS //restore float to int
		IMUL	EAX, src_stride		// jump to the next src row we should read
		mov		ecx, src			// make sure tsrc points to the next beggining of the row
		add		ecx, eax			// new source addrr points to Xth row from original source
		mov		tsrc, ecx			// store in some addr

		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading


		pop		EDX
		sub		EDX, 1				// decrease row loop counter
		jnz		row_loop5			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
	}
	else
		SLOW_COPY_MOST_IMAGE_RGB
	//rest needs to be moved normally
	SLOW_COPY_LAST_ROW_RGB
}/**/

void HDResampler::ResampleRGB24(PBYTE src, int   src_stride, PBYTE dest, int   dest_stride )
{
	//very rare case when input = output
	if ( conv_x == conv_y == 1 && src_stride == dest_stride )
		memcpy( dest, src, src_stride * out_h ); //copy whole img
	//maybe there is no width resampling
	else if ( conv_x == 1 )
		for (unsigned int y = 0; y < out_h; y++)
			memcpy( &dest[ y * dest_stride ], &src[ (unsigned int)( y * src_stride * conv_y ) ], dest_stride ); //copy 1 line
	//copy 1 pixel x times. Ex : upscale from QCIF to CIF
#ifdef PUSH_SPEED_LIMITS_FOR_RGB_RESIZE
	else if( conv_x < 1.0f )
		ResampleRGB24_upscale(src,src_stride,dest,dest_stride);
	//src is fragmented : many to one
	else
		ResampleRGB24_downscale(src,src_stride,dest,dest_stride);
#else
	ResampleRGB24_anyscale(src,src_stride,dest,dest_stride);
#endif
}/**/


///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////

//! Not sure if it is correct when using stride != width
void HDResampler::InitConvTable(unsigned int psrc_stride, unsigned int pdst_stride)
{
	if( conv_table )
	{
		free( conv_table );
		conv_table = NULL;
	}

	src_stride = psrc_stride;
	dst_stride = pdst_stride;

//	if( src_stride != in_w || dst_stride != out_w )
//		return;

	conv_table = (unsigned int*)malloc(out_h*2*dst_stride*sizeof(unsigned int));
	unsigned int max_value = psrc_stride * in_h + psrc_stride * in_h / 4 * 2;

	unsigned int *Y;
	for( unsigned int y=0;y<out_h;y++)
	{
		Y = conv_table + y*dst_stride;
		unsigned int src_y = (unsigned int)( y * conv_y ) * src_stride;
		for( unsigned int x=0;x<out_w;x++)
		{
			Y[x] = src_y + (unsigned int)( x * conv_x ) ;
#ifdef _DEBUG
			if( Y[x] > max_value )
				printf("!!!!warning, conversion table points out of input memory bounds\n");
#endif
		}
		for( unsigned int x=out_w;x<dst_stride;x++)
			Y[x] = 0xF0000000 | x;
	}
	unsigned int *baseU;
	unsigned int *U;
	baseU = conv_table + out_h * dst_stride;
	for( unsigned int y=0;y<out_h /2 ;y++)
	{
		U = baseU + y * dst_stride / 2;
		unsigned int src_y = src_stride * in_h + (unsigned int)( y * conv_y ) * src_stride / 2;
		for( unsigned int x=0;x<out_w/2;x++)
		{
			U[x] = src_y + (unsigned int)( x * conv_x ) ;
#ifdef _DEBUG
			if( U[x] > max_value )
				printf("!!!!warning, conversion table points out of input memory bounds\n");
#endif
		}
		for( unsigned int x=out_w/2;x<dst_stride/2;x++)
			U[x] = 0xF0000000 | x;
	}
	unsigned int *baseV;
	unsigned int *V;
	baseV = baseU + out_h * dst_stride / 4;
	for( unsigned int y=0;y<out_h/2;y++)
	{
		V = baseV + y * dst_stride / 2;
		unsigned int src_y = src_stride * in_h + src_stride * in_h / 4 + (unsigned int)( y * conv_y ) * src_stride / 2;
		for( unsigned int x=0;x<out_w/2;x++)
		{
			V[x] = src_y + (unsigned int)( x * conv_x ) ;
#ifdef _DEBUG
			if( U[x] > max_value )
				printf("!!!!warning, conversion table points out of input memory bounds\n");
#endif
		}
		for( unsigned int x=out_w/2;x<dst_stride/2;x++)
			U[x] = 0xF0000000 | x;
	}

	/**/
}

void copy_no_stride_4x(PBYTE dst,PBYTE src, unsigned int *lookuptable, unsigned int copy_count)
{
	_asm 
	{
		mov		EDX, copy_count	// copy count

		mov		EDI, dst			// the destination addr in EDI
		mov		ECX, src			// src addr
		mov		ESI, lookuptable	// lookup table base adress
loop_jump:
		//CPU has 5 instruction pipes. Groupped intructions should get half execution time (in theory )

		//fetch the indexes from where we get values from src
		mov         EAX, dword ptr [ ESI + 0 ]
		mov         EBX, dword ptr [ ESI + 4 ]

		//fetch values from src
		mov         AL,byte ptr [ ECX + EAX ]
		mov         BL,byte ptr [ ECX + EBX ]

		//store values at dst
		mov         byte ptr [ EDI + 0 ],AL
		mov         byte ptr [ EDI + 1 ],BL

		//fetch the indexes from where we get values from src
		mov         EAX, dword ptr [ ESI + 8 ]
		mov         EBX, dword ptr [ ESI + 12 ]

		//fetch values from src
		mov         AL,byte ptr [ ECX + EAX ]
		mov         BL,byte ptr [ ECX + EBX ]

		//store values at dst
		mov         byte ptr [ EDI + 2 ],AL
		mov         byte ptr [ EDI + 3 ],BL

		//advance lookup table and dst adress
		add			EDI, 4
		add			ESI, 4 * 4		// sizeof( unsigned int )

		sub			EDX, 4			// decrease col loop counter by 2 pixels
		jnz			loop_jump		// need more operations ?
	};
}

void copy_mem_4x(PBYTE dst, const unsigned int dstw, const unsigned int dststride, PBYTE src, const unsigned int copy_rows, unsigned int *lookuptable)
{
	signed int extra_stride_add = dststride - dstw;			//stride should be at least width
	signed int extra_stride_add2 = extra_stride_add * 4;	//stride should be at least width
	_asm 
	{
		mov			EDI, dst			// the destination addr in EDI
		mov			ECX, src			// src addr
		mov			ESI, lookuptable	// lookup table base adress
		mov			EDX, copy_rows
row_loop_jump:
		push		EDX
		mov			EDX, dstw			// col copy count
col_loop_jump:
		//CPU has 5 instruction pipes. Grouped intructions should get half execution time

		//fetch the indexes from where we get values from src
		mov         EAX, dword ptr [ ESI + 0 ]
		mov         EBX, dword ptr [ ESI + 4 ]

		//fetch values from src
		mov         AL,byte ptr [ ECX + EAX ]
		mov         BL,byte ptr [ ECX + EBX ]

		//store values at dst
		mov         byte ptr [ EDI + 0 ],AL
		mov         byte ptr [ EDI + 1 ],BL

		//fetch the indexes from where we get values from src
		mov         EAX, dword ptr [ ESI + 8 ]
		mov         EBX, dword ptr [ ESI + 12 ]

		//fetch values from src
		mov         AL,byte ptr [ ECX + EAX ]
		mov         BL,byte ptr [ ECX + EBX ]

		//store values at dst
		mov         byte ptr [ EDI + 2 ],AL
		mov         byte ptr [ EDI + 3 ],BL

		//advance lookup table and dst adress
		add			EDI, 4				// advance dest addr
		add			ESI, 4 * 4			// sizeof( unsigned int )

		sub			EDX, 4				// decrease col loop counter by 2 pixels
		jnz			col_loop_jump		// need more operations ?
//end of copy col loop
		add			EDI, extra_stride_add	// stride to the end of the row with dest
		add			ESI, extra_stride_add2	// stride to the end of the row with dest
		pop			EDX					// get our row copy counter
		sub			EDX, 1				// decrease count
		jnz			row_loop_jump		// need more operations ?
//end of copy rows loop
	};
}

//this downscales only 1 "color"(YUV) component

void HDResampler::ResampleYUV_I420_downscale(PBYTE src, unsigned int *lookup_table, PBYTE dest, int destw, int   dest_stride )
{
	//the error is consealed in every max 8 pixel copy and it does not stack up to the end of the row
	//see what is the skip ratio : generate a skip mask
	// 1.20 = 1111

	// 1.30 = 1110
	// 1.40 = 1101

	// 1.60 = 1011
	// 1.80 = 1010
	// 2.50 = 0101

	// 2.89 = 0100
	// 3.89 = 0010
	// 4.89 = 0001

	// source is larger x times then output
	if( conv_x >= 4.89f )
		copy_mem_4x( dest, destw, dest_stride, src, out_h, lookup_table );
	// source is larger x times then output
	else if( conv_x >= 2.89f )
	{
		// 1 pixel out of 4 : 0100
#define STORE_STEPS 4	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 4	// since looping represents at least 10% of processing we unroll it 
		unsigned int	col_copy_count = destw / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - destw;
		unsigned int	inc_lookup_table_index = STORE_STEPS*sizeof(unsigned int);
		PBYTE			tsrc = src;
		_asm 
		{
		mov		EDI, dest			// the destination addr in EDI

		mov		EBX, out_h_local	// times to loop rows
row_loop1:
		push	EBX					// save row loop

		mov		ESI, tsrc			// base src addr
		mov		ECX, lookup_table	// lookup table

		mov		EBX, col_copy_count	// times to loop for column
col_loop1:
		// conseal error by fetching the next correct index for SRC
		mov		EDX, [ ECX ]		// EAX has the next valid index for src addr
		add		EDX, ESI

		mov		AL, [ EDX + 0 ]  // get 1 byte from src
		mov		[ EDI ], AL		// store the "color" byte at the destination 1

		mov		AL, [ EDX + 5 ]// get 4 bytes instead of 3 from src : 01000100
		mov		[ EDI + 1], AL		// store the pixel at the destination 1

		mov		AL, [ EDX + 9 ]// get 4 bytes instead of 3 from src : 0100010001000100
		mov		[ EDI + 2 ], AL	// store the pixel at the destination 1-> loop unroll 1

		mov		AL, [ EDX + 13 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 3 ], AL	// store the pixel at the destination 1

		add		EDI, STORE_STEPS	// advance our destination address

		ADD		ECX, inc_lookup_table_index	// increase source address, increaser

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop1			// need more operations ?
//end col loop
		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading
		add		ECX,remaining_dst_stride	// lookup table also has same stride as dest

		pop		EBX
		sub		EBX, 1				// decrease row loop counter
		jnz		row_loop1			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	// source is larger x times then output
	else if( conv_x >= 1.60f )
	{
		// 2 pixel out of 4 : 1010
#define STORE_STEPS 4	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 2	// since looping represents at least 10% of processing we unroll it 
		unsigned int	col_copy_count = destw / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - destw;
		unsigned int	inc_lookup_table_index = STORE_STEPS*sizeof(unsigned int);
		PBYTE			tsrc = src;
		_asm 
		{
		mov		EDI, dest			// the destination addr in EDI

		mov		EBX, out_h_local	// times to loop rows
row_loop2:
		push	EBX					// save row loop

		mov		ESI, tsrc			// base src addr
		mov		ECX, lookup_table	// lookup table

		mov		EBX, col_copy_count	// times to loop for column
col_loop2:
		// conseal error by fetching the next correct index for SRC
		mov		EDX, [ ECX ]		// EAX has the next valid index for src addr
		add		EDX, ESI
		
		mov		AL, [ EDX + 0 ]  // get 1 byte from src
		mov		[ EDI ], AL		// store the "color" byte at the destination 1

		mov		AL, [ EDX + 2 ]// get 4 bytes instead of 3 from src : 01000100
		mov		[ EDI + 1 ], AL		// store the pixel at the destination 1

		mov		AL, [ EDX + 4 ]// get 4 bytes instead of 3 from src : 0100010001000100
		mov		[ EDI + 2 ], AL	// store the pixel at the destination 1-> loop unroll 1

		mov		AL, [ EDX + 6 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 3 ], AL	// store the pixel at the destination 1

		add		EDI, STORE_STEPS	// advance our destination address

		ADD		ECX, inc_lookup_table_index	// increase source address, increaser

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop2			// need more operations ?
//end col loop
		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading
		add		ECX,remaining_dst_stride	// lookup table also has same stride as dest

		pop		EBX
		sub		EBX, 1				// decrease row loop counter
		jnz		row_loop2			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	// source is larger x times then output
	else if( conv_x >= 1.30f )
	{
		// 3 pixel out of 4 : 1101
#define STORE_STEPS 6	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 2	// since looping represents at least 10% of processing we unroll it 
		unsigned int	col_copy_count = destw / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - destw;
		unsigned int	inc_lookup_table_index = STORE_STEPS*sizeof(unsigned int);
		PBYTE			tsrc = src;
		_asm 
		{
		mov		EDI, dest			// the destination addr in EDI

		mov		EBX, out_h_local	// times to loop rows
row_loop3:
		push	EBX					// save row loop

		mov		ESI, tsrc			// base src addr
		mov		ECX, lookup_table	// lookup table

		mov		EBX, col_copy_count	// times to loop for column
col_loop3:
		// conseal error by fetching the next correct index for SRC
		mov		EDX, [ ECX ]		// EAX has the next valid index for src addr
		add		EDX, ESI

		mov		AL, [ EDX + 0 ]  // get 1 byte from src
		mov		[ EDI ], AL		// store the "color" byte at the destination 1

		mov		AL, [ EDX + 1 ]// get 4 bytes instead of 3 from src : 01000100
		mov		[ EDI + 1 ], AL		// store the pixel at the destination 1

		mov		AL, [ EDX + 3 ]// get 4 bytes instead of 3 from src : 0100010001000100
		mov		[ EDI + 2 ], AL	// store the pixel at the destination 1-> loop unroll 1

		mov		AL, [ EDX + 4 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 3 ], AL	// store the pixel at the destination 1

		mov		AL, [ EDX + 5 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 4 ], AL	// store the pixel at the destination 1

		mov		AL, [ EDX + 7 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 5 ], AL	// store the pixel at the destination 1

		add		EDI, STORE_STEPS	// advance our destination address

		ADD		ECX, inc_lookup_table_index	// increase source address, increaser

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop3			// need more operations ?
//end col loop
		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading
		add		ECX,remaining_dst_stride	// lookup table also has same stride as dest

		pop		EBX
		sub		EBX, 1				// decrease row loop counter
		jnz		row_loop3			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	// source is larger x times then output
	else
	{
		// 4 pixel out of 4 : 1111
#define STORE_STEPS 4	// source fetches are synced to destination writes. We write x values at once
//#define LOOP_UNROLLS 1	// since looping represents at least 10% of processing we unroll it 
		unsigned int	col_copy_count = destw / STORE_STEPS ;
		unsigned int	out_h_local = out_h - 1;
		unsigned int	remaining_dst_stride = dest_stride - destw;
		unsigned int	inc_lookup_table_index = STORE_STEPS*sizeof(unsigned int);
		PBYTE			tsrc = src;
		_asm 
		{
		mov		EDI, dest			// the destination addr in EDI

		mov		EBX, out_h_local	// times to loop rows
row_loop4:
		push	EBX					// save row loop

		mov		ESI, tsrc			// base src addr
		mov		ECX, lookup_table	// lookup table

		mov		EBX, col_copy_count	// times to loop for column
col_loop4:
		// conseal error by fetching the next correct index for SRC
		mov		EDX, [ ECX ]		// EAX has the next valid index for src addr
		add		EDX, ESI

		mov		AL, [ EDX + 0 ]  // get 1 byte from src
		mov		[ EDI ], AL		// store the "color" byte at the destination 1

		mov		AL, [ EDX + 1 ]// get 4 bytes instead of 3 from src : 01000100
		mov		[ EDI + 1], AL		// store the pixel at the destination 1

		mov		AL, [ EDX + 2 ]// get 4 bytes instead of 3 from src : 0100010001000100
		mov		[ EDI + 2 ], AL	// store the pixel at the destination 1-> loop unroll 1

		mov		AL, [ EDX + 3 ]// get 4 bytes instead of 3 from src 
		mov		[ EDI + 3 ], AL	// store the pixel at the destination 1

		add		EDI, STORE_STEPS	// advance our destination address

		ADD		ECX, inc_lookup_table_index	// increase source address, increaser

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop4			// need more operations ?
//end col loop
		add		EDI, remaining_dst_stride	// probably this is 0 but we should make sure to see if there is extra col pading
		add		ECX,remaining_dst_stride	// lookup table also has same stride as dest

		pop		EBX
		sub		EBX, 1				// decrease row loop counter
		jnz		row_loop4			// need more operations ?
//end row loop
		}
#undef STORE_STEPS
//#undef LOOP_UNROLLS
	}
	//rest needs to be moved normally
	PBYTE lastrow_dest = dest + out_h * dest_stride;
	PBYTE lastrow_src = src + out_h * src_stride;
	copy_mem_4x( lastrow_dest, destw, dest_stride, src, out_h, lookup_table );
}/**/


void HDResampler::ResampleYUV_I420_anyscale(PBYTE src, int   psrc_stride, PBYTE dest, int   pdest_stride )
{
	// 38% slower then with lookup table asm version
/*	for( unsigned int x=0;x<out_h*dst_stride;x+=4)
	{
		dest[ x ] = src[ conv_table[ x ] ];
		dest[ x + 1 ] = src[ conv_table[ x + 1 ] ];
		dest[ x + 2 ] = src[ conv_table[ x + 2 ] ];
		dest[ x + 3 ] = src[ conv_table[ x + 3 ] ];
	}
	//uV
	for( unsigned int y=0;y<out_h;y++)
	{
		unsigned int start = (y + out_h)* dst_stride;
		unsigned int end = start + dst_stride / 2;
		for( unsigned int x=start;x<end;x+=2)
		{
			dest[ x ] = src[ conv_table[ x ] ];
			dest[ x + 1 ] = src[ conv_table[ x + 1 ] ];
		}
	}/**/
	//using simulated float values is 3% slower then asm with lookup table
	//y
	unsigned int int_conv_y = (unsigned int)( conv_y * FLOAT_PRECISSION );
	unsigned int int_conv_x = (unsigned int)( conv_x * FLOAT_PRECISSION );
	unsigned int stacking_precission_y = 0;
	for( unsigned int y=0;y<out_h;y++)
	{
		unsigned int stacking_precission_x = 0;

		unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
		PBYTE			tsrc = src + converted_row_index * src_stride;
		stacking_precission_y += int_conv_y;

		unsigned int start = y * dst_stride;
		unsigned int end = start + out_w;

		for( unsigned int x=start;x<end;x+=1)
		{
			int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
			dest[ x ] = tsrc[ converted_col_index ];
			stacking_precission_x += int_conv_x;
		}
	}
	//U
	stacking_precission_y = 0;
	PBYTE destU = dest + out_h * dst_stride;
	PBYTE srcU = src + in_h * src_stride;
	for( unsigned int y=0;y<out_h/2;y++)
	{
		unsigned int stacking_precission_x = 0;

		unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
		PBYTE			tsrc = srcU + converted_row_index * src_stride / 2;
		stacking_precission_y += int_conv_y;

		unsigned int start = y * dst_stride / 2;
		unsigned int end = start + out_w / 2;

		for( unsigned int x=start;x<end;x+=1)
		{
			int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
			destU[ x ] = tsrc[ converted_col_index ];
			stacking_precission_x += int_conv_x;
		}
	}
	//V
	stacking_precission_y = 0;
	PBYTE destV = destU + out_h * dst_stride / 4;
	PBYTE srcV = srcU + in_h * src_stride / 4;
	for( unsigned int y=0;y<out_h/2;y++)
	{
		unsigned int stacking_precission_x = 0;

		unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
		PBYTE			tsrc = srcV + converted_row_index * src_stride / 2;
		stacking_precission_y += int_conv_y;

		unsigned int start = y * dst_stride / 2;
		unsigned int end = start + out_w / 2;

		for( unsigned int x=start;x<end;x+=1)
		{
			int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
			destV[ x ] = tsrc[ converted_col_index ];
			stacking_precission_x += int_conv_x;
		}
	}
	/**/
	// in this form is 1414% slower then lookup table with asm
/*	for (unsigned int y = 0; y < out_h; y++)
		for (unsigned int x = 0; x < out_w; x+=1)
			dest[y * pdest_stride + x * 3 + 0 ] = src[ (unsigned int)( y * src_stride * conv_y ) + (unsigned int)( x * conv_x ) * 3 + 0 ];
	for (unsigned int y = 0; y < out_h; y++)
		for (unsigned int x = 0; x < out_w / 2; x+=1)
			dest[( y + out_h ) * pdest_stride + x * 3 + 0 ] = src[ (unsigned int)( ( y + out_h ) * src_stride * conv_y ) + (unsigned int)( x * conv_x ) * 3 + 0 ];
	/**/

	// When implementing this tendency in Hardware progress is making Memory faster and multi core. Multi core does not influence us but Memory does
//	if( out_w % 4 == 0 )
/*	{
		if( pdest_stride == out_w )
			copy_no_stride_4x( dest, src, conv_table, out_h*out_w + out_h*out_w / 4 * 2 );
		else
		{
			unsigned int Y_plane_size = pdest_stride * out_h;
			copy_mem_4x( dest, out_w, pdest_stride, src, out_h, conv_table );
			copy_mem_4x( dest + Y_plane_size, out_w / 2, pdest_stride / 2, src, out_h / 2, conv_table + Y_plane_size );
			copy_mem_4x( dest + Y_plane_size + Y_plane_size / 4, out_w / 2, pdest_stride / 2, src, out_h / 2, conv_table + Y_plane_size + Y_plane_size / 4 );
		}
	}
	/**/
}

void HDResampler::ResampleYUV_I420(PBYTE src, int   psrc_stride, PBYTE dest, int   pdest_stride )
{
	if( psrc_stride != src_stride || pdest_stride != dst_stride )
		InitConvTable( psrc_stride, pdest_stride );

	//very rare case when input = output
	if ( conv_x == 1 && conv_y == 1 && psrc_stride == pdest_stride )
		memcpy( dest, src, src_stride * out_h * 3 / 2 ); //copy whole img
	//maybe there is no width resampling
	else if ( conv_x == 1 )
	{
		//scale All the same : Y
		for (unsigned int y = 0; y < out_h; y++)
			memcpy( &dest[ y * pdest_stride ], &src[ (unsigned int)( y * conv_y ) * psrc_stride ], pdest_stride ); //copy 1 line
		//u
		dest = dest + out_h * pdest_stride;
		src = src + in_h * psrc_stride;
		for (unsigned int y = 0; y < out_h / 2; y++)
			memcpy( &dest[ y * pdest_stride / 2 ], &src[ (unsigned int)( y * conv_y ) * psrc_stride / 2 ], pdest_stride ); //copy 1 line
		//v
		dest = dest + out_h * pdest_stride / 4;
		src = src + in_h * psrc_stride / 4;
		for (unsigned int y = 0; y < out_h / 2; y++)
			memcpy( &dest[ y * pdest_stride / 2 ], &src[ (unsigned int)( y * conv_y ) * psrc_stride / 2 ], pdest_stride ); //copy 1 line
	}
	//copy 1 pixel x times. Ex : upscale from QCIF to CIF
//	else if( conv_x < 1.0f )
//		ResampleRGB24_upscale(src,src_stride,dest,dest_stride);
	//src is fragmented : many to one
/*	else if( conv_x > 1.0f )
	{
		//Y
		ResampleYUV_I420_downscale(src,conv_table,dest,out_w,pdest_stride);
		//UV
		ResampleYUV_I420_downscale(src + in_h * psrc_stride ,conv_table,dest + out_h * pdest_stride, out_w / 2, pdest_stride );
	}/**/
	else ResampleYUV_I420_anyscale(src,src_stride,dest,pdest_stride);
}/**/
