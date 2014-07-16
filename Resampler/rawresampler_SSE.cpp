// **********************************************************************************************
//                              Copyright (C) 2009  - SPLASH Software 
//
//                    Use or duplication without written consent prohibitted
// **********************************************************************************************
#include <windows.h>
#include <stdlib.h>

#include "rawResampler_sse.h"

#define RGB_BYTE_COUNT	3

RawResamplerSSE::RawResamplerSSE(int _width1, int _height1, int _width2, int _height2,int _lumalow , int _lumahigh , int _chromalow ,int  _chromahigh )
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
}

void RawResamplerSSE::ResampleRGB24_downscale(PBYTE src, int   src_stride, PBYTE dest, int   dest_stride )
{
	//the error in consealed in every max 4 pixel copy and it does not stack up to the end of the row
	//see what is the skip ratio : generate a skip mask
	// 1.20 = 11110
	// 1.30 = 11101
	// 1.40 = 11011
	// 1.60 = 10110
	// 1.80 = 10101
	// 2.50 = 01010
	// 2.89 = 01001
	// 3.89 = 00100
	// 4.89 = 00010
	// 5.00 = 00001
	float add2;
	//moving only 1 pixel with SSE is slower then with normal move
	if( conv_x >= 3.89f )
	{
		//move values by 4 instead of 3 times
		add2 = conv_x*3;
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride);
			for (unsigned int x = 0; x < out_w; x++)
			{
				*(int*)&dest[ v1 ] = *(int*)&src[ (unsigned int)( v2 ) ];
				v1 += 3;
				v2 += add2;
			}
		}
	}
	//01001
	else if( conv_x >= 2.89f )
	{
		add2 = conv_x*3*2; //copy 2 pixels from source
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride + 3);
			for ( unsigned int x = 0; x < out_w; x += 2 )
			{
				BYTE *psrc = &src[ (unsigned int)( v2 ) ];
				BYTE *pdst = &dest[ v1 ];
				_asm 
				{ 
					mov		eax, psrc;	// src addr into eax	
					MOVUPS	xmm1,[eax];	// xmm1 has value from src addr ( 16 bytes ) 
					mov		eax, pdst;	// dest addr into eax	
					MOVD	[eax],xmm1;	// write 4 bytes intead of 3	
					PSRLDQ	xmm1,3*3;	// loose the middle 2 bytes and the already written one from the register 
					add		eax, 3;		// next store addr	
					MOVD	[eax],xmm1;	// write 4 bytes instead of 3	
				} 
				v1 += 2 * 3 ; //copy 2 pixels to destination
				v2 += add2;
			}
		}
	}
/*	
	//commenting these patterns out : since it is a repeating patter there is no need to make a difference where the missing line is
	//01010
	else if( conv_x >= 2.50f )
	{
		add2 = conv_x*3*2; //copy 2 pixels from source
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride + 3);
			for ( unsigned int x = 0; x < out_w; x += 2 )
			{
				BYTE *psrc = &src[ (unsigned int)( v2 ) ];
				BYTE *pdst = &dest[ v1 ];
				_asm 
				{ 
					mov		eax, psrc;	// src addr into eax	
					MOVUPS	xmm1,[eax];	// xmm1 has value from src addr ( 16 bytes ) 
					mov		eax, pdst;	// dest addr into eax	
					MOVD	[eax],xmm1;	// write 4 bytes intead of 3	
					PSRLDQ	xmm1,2*3;	// loose the middle 2 bytes and the already written one from the register 
					add		eax, 3;		// next store addr	
					MOVD	[eax],xmm1;	// write 4 bytes instead of 3	
				} 
				v1 += 2 * 3 ; //copy 2 pixels to destination
				v2 += add2;
			}
		}
	}
	//10101
	else if( conv_x >= 1.80f )
	{
		add2 = conv_x*3*3; //copy 3 pixels from source
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;								//jump to the beggining of a row in dest
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride ); //jump to the beggining of a row in src
			for ( unsigned int x = 0; x < out_w; x += 3 )
			{
				BYTE *psrc = &src[ (unsigned int)( v2 ) ];
				BYTE *pdst = &dest[ v1 ];
				_asm 
				{ 
					mov		eax, psrc;	// src addr into eax	
					MOVUPS	xmm1,[eax];	// xmm1 has value from src addr ( 16 bytes ) 
					mov		eax, pdst;	// dest addr into eax	
					MOVD	[eax],xmm1;	// write 4 bytes intead of 3	
					PSRLDQ	xmm1,2*3;	// loose the middle 2 bytes and the already written one from the register 
					add		eax, 3;		// next store addr	
					MOVD	[eax],xmm1;	// write 4 bytes instead of 3	
					PSRLDQ	xmm1,2*3;	// loose the middle 2 bytes and the already written one from the register 
					add		eax, 3;		// next store addr	
					MOVD	[eax],xmm1;	// write 4 bytes instead of 3	
				} 
				v1 += 3 * 3 ; //copy 3 pixels to destination
				v2 += add2;
			}
		}
	}*/
	//10110
	else if( conv_x >= 1.60f )
	{
		add2 = conv_x*3*3; //copy 3 pixels from source
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride );
			for ( unsigned int x = 0; x < out_w; x += 3 )
			{
				BYTE *psrc = &src[ (unsigned int)( v2 ) ];
				BYTE *pdst = &dest[ v1 ];
				_asm 
				{ 
					mov		eax, psrc;	// src addr into eax	
					MOVUPS	xmm1,[eax];	// xmm1 has value from src addr ( 16 bytes ) 
					mov		eax, pdst;	// dest addr into eax	
					MOVD	[eax],xmm1;	// write 4 bytes intead of 3	
					PSRLDQ	xmm1,2*3;	// loose the middle 2 bytes and the already written one from the register 
					add		eax, 3;		// next store addr	
					MOVSD	[eax],xmm1;	// write 8 bytes instead of 6	
				} 
				v1 += 3 * 3 ; //copy 3 pixels to destination
				v2 += add2;
			}
		}
	}
/*	
	//commenting these patterns out : since it is a repeating patter there is no need to make a difference where the missing line is
	// 11011
	else if( conv_x >= 1.40f )
	{
		add2 = conv_x*3*4; //copy 4 pixels from source
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride );
			for ( unsigned int x = 0; x < out_w; x += 4 )
			{
				BYTE *psrc = &src[ (unsigned int)( v2 ) ];
				BYTE *pdst = &dest[ v1 ];
				_asm 
				{ 
					mov		eax, psrc;	// src addr into eax	
					MOVUPS	xmm1,[eax];	// xmm1 has value from src addr ( 16 bytes ) 
					mov		eax, pdst;	// dest addr into eax	
					MOVSD	[eax],xmm1;	// write 8 bytes intead of 6	
					PSRLDQ	xmm1,3*3;	// loose the middle 2 bytes and the already written one from the register 
					add		eax, 3;		// next store addr	
					MOVSD	[eax],xmm1;	// write 8 bytes instead of 6	
				} 
				v1 += 4 * 3 ; //copy 4 pixels to destination
				v2 += add2;
			}
		}
	}
	//11101
	else if( conv_x >= 1.30f )
	{
		add2 = conv_x*3*4; //copy 4 pixels from source
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride );
			for ( unsigned int x = 0; x < out_w; x += 4 )
			{
				BYTE *psrc = &src[ (unsigned int)( v2 ) ];
				BYTE *pdst = &dest[ v1 ];
				_asm 
				{ 
					mov		eax, psrc;	// src addr into eax	
					MOVUPS	xmm1,[eax];	// xmm1 has value from src addr ( 16 bytes ) 
					mov		eax, pdst;	// dest addr into eax	
					MOVUPS	[eax],xmm1;	// write 16 bytes intead of 9	
					PSRLDQ	xmm1,4*3;	// loose the middle 2 bytes and the already written one from the register 
					add		eax, 3;		// next store addr	
					MOVD	[eax],xmm1;	// write 4 bytes instead of 3	
				} 
				v1 += 4 * 3 ; //copy 4 pixels to destination
				v2 += add2;
			}
		}
	}*/
	// 11110
	else if( conv_x > 1.20f )
	{
		add2 = conv_x*3*4; //copy 4 pixels from source
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride );
			for ( unsigned int x = 0; x < out_w; x += 4 )
			{
				BYTE *psrc = &src[ (unsigned int)( v2 ) ];
				BYTE *pdst = &dest[ v1 ];
				_asm 
				{ 
					mov		eax, psrc;	// src addr into eax	
					MOVUPS	xmm1,[eax];	// xmm1 has value from src addr ( 16 bytes ) 
					mov		eax, pdst;	// dest addr into eax	
					MOVUPS	[eax],xmm1;	// write 16 bytes intead of 12	
				} 
				v1 += 4 * 3 ; //copy 4 pixels to destination
				v2 += add2;
			}
		}
	}
	// 11111 -> conversion ration is between 1 and 1.2
	else
	{
		add2 = conv_x*3*5; //copy 5 pixels from source
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride);
			for ( unsigned int x = 0; x < out_w; x += 4 )
			{
				BYTE *psrc = &src[ (unsigned int)( v2 ) ];
				BYTE *pdst = &dest[ v1 ];
				_asm 
				{ 
					mov		eax, psrc;	// src addr into eax	
					MOVUPS	xmm1,[eax];	// xmm1 has value from src addr ( 16 bytes ) 
					mov		eax, pdst;	// dest addr into eax	
					MOVUPS	[eax],xmm1;	// write 16 bytes intead of 15	
				} 
				v1 += 5 * 3 ; //copy 4 pixels to destination
				v2 += add2;
			}
		}
	}
	//rest needs to be moved normally
	unsigned int v1 = ( out_h - 1 ) * dest_stride;
	float v2 = (float)((unsigned int)( ( out_h - 1 ) * src_stride * conv_y ));
	add2 = conv_x*3;
	for (unsigned int x = 0; x < out_w; x++)
		{
			dest[ v1 + 0 ] = src[ (unsigned int)( v2 ) + 0 ];
			dest[ v1 + 1 ] = src[ (unsigned int)( v2 ) + 1 ];
			dest[ v1 + 2 ] = src[ (unsigned int)( v2 ) + 2 ];
			v1 += 3;
			v2 += add2;
		}
}/**/

void RawResamplerSSE::ResampleRGB24_upscale(PBYTE src, int   src_stride, PBYTE dest, int   dest_stride )
{
	// source is max 2x smaller
	if( conv_x >= 0.5f )
	{
#define SCALE_UPS 2
		unsigned int	float_precission = 1 << 16; // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * SCALE_UPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / SCALE_UPS;
		unsigned int	out_h_local = out_h;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows
		sub		EDX, 1

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
		add		EDI, SCALE_UPS*RGB_BYTE_COUNT// advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// for division we have value in DX:AX
		xor		EDX, EDX			// not using this for the division
		idiv	float_precission	// we use int with increased precission
		IMUL	EAX, RGB_BYTE_COUNT	// we jump in pixels
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop2			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		xor		EDX, EDX			// 32 bit is enough for us (high part)
		idiv	float_precission	// scale value to usable integer
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
#undef SCALE_UPS
	}
	// source is max 3x smaller
	else if( conv_x >= 0.33f )
	{
#define SCALE_UPS 3
		unsigned int	float_precission = 1 << 16; // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * SCALE_UPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / SCALE_UPS;
		unsigned int	out_h_local = out_h;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows
		sub		EDX, 1

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
		add		EDI, SCALE_UPS*RGB_BYTE_COUNT// advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// for division we have value in DX:AX
		xor		EDX, EDX			// not using this for the division
		idiv	float_precission	// we use int with increased precission
		IMUL	EAX, RGB_BYTE_COUNT	// we jump in pixels
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop3			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		xor		EDX, EDX			// 32 bit is enough for us (high part)
		idiv	float_precission	// scale value to usable integer
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
#undef SCALE_UPS
	}
	// source is max 4x smaller
	else if( conv_x >= 0.25f )
	{
#define SCALE_UPS 4
		unsigned int	float_precission = 1 << 16; // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * SCALE_UPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / SCALE_UPS;
		unsigned int	out_h_local = out_h;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows
		sub		EDX, 1

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
		add		EDI, SCALE_UPS*RGB_BYTE_COUNT// advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// for division we have value in DX:AX
		xor		EDX, EDX			// not using this for the division
		idiv	float_precission	// we use int with increased precission
		IMUL	EAX, RGB_BYTE_COUNT	// we jump in pixels
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop4			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		xor		EDX, EDX			// 32 bit is enough for us (high part)
		idiv	float_precission	// scale value to usable integer
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
#undef SCALE_UPS
	}
	// source is max 5x smaller
	else if( conv_x >= 0.20f )
	{
#define SCALE_UPS 5
		unsigned int	float_precission = 1 << 16; // real value should not exceed 16 bits. Make sure real value bitcount + precision bitcount to not exceed 32 bits
		unsigned int	inc_src_col_int = (unsigned int)( conv_x * float_precission * SCALE_UPS );
		unsigned int	inc_src_row_int = (unsigned int)( conv_y * float_precission );
		unsigned int	col_copy_count = out_w / SCALE_UPS;
		unsigned int	out_h_local = out_h;
		unsigned int	remaining_dst_stride = dest_stride - out_w * RGB_BYTE_COUNT;
		PBYTE			tsrc = src;
		_asm 
		{
		mov		EDX, out_h_local	// times to loop rows
		sub		EDX, 1

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
		add		EDI, SCALE_UPS*RGB_BYTE_COUNT// advance our destination address

		ADD		ECX, inc_src_col_int// increase source address, increaser

		mov		eax, ecx			// for division we have value in DX:AX
		xor		EDX, EDX			// not using this for the division
		idiv	float_precission	// we use int with increased precission
		IMUL	EAX, RGB_BYTE_COUNT	// we jump in pixels
		
		mov		ESI, tsrc			// restore original source address
		add		ESI, EAX			// add the converted precission to our source addr

		sub		EBX, 1				// decrease col loop counter by 2 pixels
		jnz		col_loop5			// need more operations ?
//end col loop
		pop		EBX					// the stacking row precision
		add		ebx,inc_src_row_int // add row as simulated float

		mov		eax, EBX			// lower part of divided number
		xor		EDX, EDX			// 32 bit is enough for us (high part)
		idiv	float_precission	// scale value to usable integer
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
#undef SCALE_UPS
	}
	else
	{
		float add2 = conv_x*3;
		for (unsigned int y = 0; y < out_h - 1; y++)
		{
			unsigned int v1 = y * dest_stride;
			float v2 = (float)((unsigned int)( y * conv_y ) * src_stride);
			for (unsigned int x = 0; x < out_w; x++)
			{
				*(int*)&dest[ v1 ] = *(int*)&src[ (unsigned int)( v2 ) ];
				v1 += 3;
				v2 += add2;
			}
		}
	}
	//rest needs to be moved normally
	unsigned int v1 = ( out_h - 1 ) * dest_stride;
	float v2 = (float)((unsigned int)( ( out_h - 1 ) * src_stride * conv_y ));
	float add2 = conv_x * 3;
	for (unsigned int x = 0; x < out_w; x++)
		{
			dest[ v1 + 0 ] = src[ (unsigned int)( v2 ) + 0 ];
			dest[ v1 + 1 ] = src[ (unsigned int)( v2 ) + 1 ];
			dest[ v1 + 2 ] = src[ (unsigned int)( v2 ) + 2 ];
			v1 += 3;
			v2 += add2;
		}
}/**/


//!!!!!!!!!!!!!!!!!!!!!! this mode is not finished yet !
void RawResamplerSSE::ResampleRGB24(PBYTE src, int   src_stride, PBYTE dest, int   dest_stride )
{
	return; // disabling any functionality until finished

	//very rare case when input = output
/*	if ( conv_x == conv_y == 1 && src_stride == dest_stride )
		memcpy( dest, src, src_stride * out_h ); //copy whole img
	//maybe there is no width resampling
	else if ( conv_x == 1 )
		for (unsigned int y = 0; y < out_h; y++)
			memcpy( &dest[ y * dest_stride ], &src[ (unsigned int)( y * src_stride * conv_y ) ], dest_stride ); //copy 1 line
	//copy X pixels then skip 1. Ex : upscale from QCIF to CIF
	else if( conv_x < 1.0f )
		ResampleRGB24_upscale(src,src_stride,dest,dest_stride);
	//src is fragmented : many to one
	else
		ResampleRGB24_downscale(src,src_stride,dest,dest_stride);*/
}/**/
