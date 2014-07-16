#include "StdAfx.h"
#include <tmmintrin.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#ifndef CLIP3
	#define CLIP3(a,b,c)	((c) < (a) ? (a) : ((c) > (b) ? (b) : (c))) // MIN, MAX, val
#endif

void InsertWatermark_reference(unsigned char *Src_IYUV, int Src_width, int Src_height, int Src_stride, int Src_start_x, int Src_start_y, int Src_copy_width, int Src_copy_height, unsigned char *Dst_IYUV, int Dst_width, int Dst_height, int Dst_stride, int Dst_start_x, int Dst_start_y )
{
	//sanity checks
	if( Src_width < Src_start_x + Src_copy_width )
		return;
	if( Src_height < Src_start_y + Src_copy_height )
		return;
	if( Dst_width < Dst_start_x + Src_copy_width )
		return;
	if( Dst_height < Dst_start_y + Src_copy_height )
		return;
	int Src_stride_half = Src_stride / 2;
	int Dst_stride_half = Dst_stride / 2;
	unsigned char *Src_IYUV_Y = Src_IYUV + Src_stride * Src_start_y + Src_start_x;
	unsigned char *Src_IYUV_U = Src_IYUV + ( Src_stride_half ) * ( Src_start_y / 2 ) + ( Src_start_x / 2 ) + Src_height * Src_stride;
	unsigned char *Src_IYUV_V = Src_IYUV + ( Src_stride_half ) * ( Src_start_y / 2 ) + ( Src_start_x / 2 ) + Src_height * Src_stride + Src_height / 2 * Src_stride / 2;
	unsigned char *Dst_IYUV_Y = Dst_IYUV + Dst_stride * Dst_start_y + Dst_start_x;
	unsigned char *Dst_IYUV_U = Dst_IYUV + ( Dst_stride_half ) * ( Dst_start_y / 2 ) + ( Dst_start_x / 2 ) + Dst_height * Dst_stride;
	unsigned char *Dst_IYUV_V = Dst_IYUV + ( Dst_stride_half ) * ( Dst_start_y / 2 ) + ( Dst_start_x / 2 ) + Dst_height * Dst_stride + Dst_height / 2 * Dst_stride / 2;
	for( int i=Src_copy_height;i>0;i-- )
	{
		memcpy( Dst_IYUV_Y, Src_IYUV_Y, Src_copy_width );
		Dst_IYUV_Y += Dst_stride;
		Src_IYUV_Y += Src_stride;
	}
	int Src_copy_height_half = Src_copy_height / 2;
	int Src_copy_width_half = Src_copy_width / 2;
	for( int i=Src_copy_height_half;i>0;i-- )
	{
		memcpy( Dst_IYUV_U, Src_IYUV_U, Src_copy_width_half );
		memcpy( Dst_IYUV_V, Src_IYUV_V, Src_copy_width_half );
		Dst_IYUV_U += Dst_stride_half;
		Src_IYUV_U += Src_stride_half;
		Dst_IYUV_V += Dst_stride_half;
		Src_IYUV_V += Src_stride_half;
	}
}

void RGB24_to_IYUV_reference(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_IYUV, int dst_stride)
{
	int i,j;
	unsigned char * out_Y = Dst_IYUV ;
	unsigned char * out_U = out_Y + dst_stride*height ;
	unsigned char * out_V = out_U + dst_stride/2*height/2 ;

	for( j = 0; j < height; j++)
	{
		for( i = 0; i < width; i++)
		{
			int R, G, B;
			int iX3 = i+i+i;
			B = Src_RGB[iX3+0]; 
			G = Src_RGB[iX3+1]; 
			R = Src_RGB[iX3+2];
			out_Y[i] = CLIP3(0, 255, ( ( 66*R + 129*G + 25*B + 128 ) / 256 ) + 16);
			if( (i&1) == 0 && (j&1) == 0)
			{
				out_U[i>>1] =  CLIP3(0, 255, ( (-38*R - 74*G + 112*B + 128) / 256 ) + 128 );
				out_V[i>>1] =  CLIP3(0, 255, ( (112*R - 94*G -  18*B + 128) / 256 ) + 128 );
			}
		}
		out_Y += dst_stride;
		if((j&1)==0)
		{
			out_U += (dst_stride>>1);
			out_V += (dst_stride>>1);
		}
		Src_RGB += src_stride;
	}
}

void RGB24_to_IYUV_reference1(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_IYUV, int dst_stride)
{
	int i,j;
	unsigned char * out_Y = Dst_IYUV ;
	unsigned char * out_U = out_Y + dst_stride*height ;
	unsigned char * out_V = out_U + dst_stride/2*height/2 ;

	for( j = 0; j < height; j++)
	{
		for( i = 0; i < width; i++)
		{
			int R, G, B;
			int iX3 = i+i+i;
			B = Src_RGB[iX3+0]; 
			G = Src_RGB[iX3+1]; 
			R = Src_RGB[iX3+2];
			out_Y[i] = CLIP3(0, 255, ( ( 66*R + 129*G + 25*B + 128 ) / 256 ) + 16);
			if( (i&1) == 0 && (j&1) == 0)
			{
				int Bs = B+(int)Src_RGB[iX3+3]+(int)Src_RGB[iX3+src_stride+0]+(int)Src_RGB[iX3+src_stride+3]; 
				int Gs = G+(int)Src_RGB[iX3+4]+(int)Src_RGB[iX3+src_stride+1]+(int)Src_RGB[iX3+src_stride+4]; 
				int Rs = R+(int)Src_RGB[iX3+5]+(int)Src_RGB[iX3+src_stride+2]+(int)Src_RGB[iX3+src_stride+5]; 
				out_U[i>>1] =  CLIP3(0, 255, ( (-38*Rs - 74*Gs + 112*Bs + 4*128) / (256*4) ) + 128 );
				out_V[i>>1] =  CLIP3(0, 255, ( (112*Rs - 94*Gs -  18*Bs + 4*128) / (256*4) ) + 128 );
			}
		}
		out_Y += dst_stride;
		if((j&1)==0)
		{
			out_U += (dst_stride>>1);
			out_V += (dst_stride>>1);
		}
		Src_RGB += src_stride;
	}
}

void RGB24_to_IYUV_1(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_IYUV, int dst_stride)
{
	int i, j;

	unsigned char * out_Y = Dst_IYUV;
	unsigned char * out_U = out_Y + dst_stride*height;
	unsigned char * out_V = out_U + dst_stride/2*height/2;

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
			temp0 = _mm_loadu_si128((__m128i*)(&Src_RGB[iX3]));		// xx xx xx xx 32 31 30 22 21 20 12 11 10 02 01 00
			temp1 = _mm_loadu_si128((__m128i*)(&Src_RGB[iX3+12]));	// xx xx xx xx 72 71 70 62 61 60 52 51 50 42 41 40
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
				temp0 = _mm_loadu_si128((__m128i*)(&Src_RGB[iX3+widthX3]));		// xx xx xx xx 32 31 30 22 21 20 12 11 10 02 01 00
				temp1 = _mm_loadu_si128((__m128i*)(&Src_RGB[iX3+widthX3+12]));	// xx xx xx xx 72 71 70 62 61 60 52 51 50 42 41 40
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
			B = Src_RGB[iX3]; 
			G = Src_RGB[iX3+1]; 
			R = Src_RGB[iX3+2];
			out_Y[i] = CLIP3(0, 255, ((66*R + 129*G + 25*B + 128) >> 8 ) + 16);

			if( (i&1) == 0 && (j&1) == 0)
			{
				unsigned char R_01, G_01, B_01, R_11, G_11, B_11, R_10, G_10, B_10;
				B_01 = Src_RGB[3+iX3]; 
				G_01 = Src_RGB[3+iX3+1]; 
				R_01 = Src_RGB[3+iX3+2];
				B_10 = Src_RGB[widthX3+iX3]; 
				G_10 = Src_RGB[widthX3+iX3+1]; 
				R_10 = Src_RGB[widthX3+iX3+2];
				B_11 = Src_RGB[3+widthX3+iX3]; 
				G_11 = Src_RGB[3+widthX3+iX3+1]; 
				R_11 = Src_RGB[3+widthX3+iX3+2];

				out_U[i>>1] =  CLIP3(0, 255, (((-38*R - 74*G + 112*B) + (-38*R_01  - 74*G_01  + 112*B_01 ) 
					+ (-38*R_10 - 74*G_10 + 112*B_10) + (-38*R_11 - 74*G_11 + 112*B_11) + 512 ) >> 10 ) + 128);
				out_V[i>>1] =  CLIP3(0, 255, (((112*R - 94*G - 18*B) + (112*R_01  - 94*G_01 - 18*B_01 ) 
					+ (112*R_10 - 94*G_10 - 18*B_10) + (112*R_11 - 94*G_11 - 18*B_11) + 512 ) >>10 ) + 128);
			}
		}
		out_Y += dst_stride;
		if((j&1)==0)
		{
			out_U += (dst_stride>>1);
			out_V += (dst_stride>>1);
		}
		Src_RGB += src_stride;
	}
}

float GetColorValues_reference(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *_Ref, RGB24Color *_Tolerance, RGB24Color *_CurrentColor)
{
	int SumR = 0;
	int SumG = 0;
	int SumB = 0;
	int flat = 0;

	for (int y = StartY; y < StartY + RegionHeight; y++)
	{
		unsigned char *row = pbuff + y*Stride;
		for (int x = StartX; x < StartX + RegionWidth; x++)
		{
			int pos = x * 3;
			SumR += row[pos + 0];
			SumG += row[pos + 1];
			SumB += row[pos + 2];
			if( (abs((int)row[pos + 0] - (int)_Ref->B) <= (int)_Tolerance->B) &&
				(abs((int)row[pos + 1] - (int)_Ref->G) <= (int)_Tolerance->G) &&
				(abs((int)row[pos + 2] - (int)_Ref->R) <= (int)_Tolerance->R) )
				flat++;
		}
	}

	int _RegionSize = RegionWidth * RegionHeight;
	_CurrentColor->R = SumR /_RegionSize;
	_CurrentColor->G = SumG /_RegionSize;
	_CurrentColor->B = SumB /_RegionSize;

	return (float )( 100 * flat )/ (float)(_RegionSize);
}

float GetColorValues_1(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *_Ref, RGB24Color *_Tolerance, RGB24Color *_CurrentColor)
{
	unsigned char *buff = pbuff + StartY * Stride + StartX * 3;
	int Counter = 0;
	int SumR = 0;
	int SumG = 0;
	int SumB = 0;
	__m128i RefXMM;
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
	for( int y=0;y<RegionHeight;y++)
	{
		int x;
		for( x=0;x<(RegionWidth-5);x+=5 )
		{
			__m128i RGB,ColoDiff;
			int x3 = 3*x;
			//load 5 pixels
			RGB = _mm_loadu_si128((__m128i*)(&buff[x3]));
			for( int i=0;i<15;i+=3)
			{
				SumR += buff[x3 + i + 0];
				SumG += buff[x3 + i + 1];
				SumB += buff[x3 + i + 2];
			}
			//do the abs to reference
			ColoDiff = _mm_sub_epi8( RGB, RefXMM );
			ColoDiff = _mm_abs_epi8( ColoDiff );
			//check if we are smaller then reference
			//mark result if apropriate
			if( ColoDiff.m128i_u8[0] <= _Tolerance->B && ColoDiff.m128i_u8[1] <= _Tolerance->G && ColoDiff.m128i_u8[2] <= _Tolerance->R )
				Counter++;
			if( ColoDiff.m128i_u8[3] <= _Tolerance->B && ColoDiff.m128i_u8[4] <= _Tolerance->G && ColoDiff.m128i_u8[5] <= _Tolerance->R )
				Counter++;
			if( ColoDiff.m128i_u8[6] <= _Tolerance->B && ColoDiff.m128i_u8[7] <= _Tolerance->G && ColoDiff.m128i_u8[8] <= _Tolerance->R )
				Counter++;
			if( ColoDiff.m128i_u8[9] <= _Tolerance->B && ColoDiff.m128i_u8[10] <= _Tolerance->G && ColoDiff.m128i_u8[11] <= _Tolerance->R )
				Counter++;
			if( ColoDiff.m128i_u8[12] <= _Tolerance->B && ColoDiff.m128i_u8[13] <= _Tolerance->G && ColoDiff.m128i_u8[14] <= _Tolerance->R )
				Counter++;
		}/**/
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
	int _RegionSize = RegionWidth * RegionHeight;
	_CurrentColor->R = SumR /_RegionSize;
	_CurrentColor->G = SumG /_RegionSize;
	_CurrentColor->B = SumB /_RegionSize;

	return (float )( 100 * Counter ) / (float)(_RegionSize);
}

float GetColorValues_2(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *_Ref, RGB24Color *_Tolerance, RGB24Color *_CurrentColor)
{
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
	AllowedDiff.m128i_u8[0] = _Tolerance->B;
	AllowedDiff.m128i_u8[1] = _Tolerance->G;
	AllowedDiff.m128i_u8[2] = _Tolerance->R;
	AllowedDiff.m128i_u8[3] = _Tolerance->B;
	AllowedDiff.m128i_u8[4] = _Tolerance->G;
	AllowedDiff.m128i_u8[5] = _Tolerance->R;
	AllowedDiff.m128i_u8[6] = _Tolerance->B;
	AllowedDiff.m128i_u8[7] = _Tolerance->G;
	AllowedDiff.m128i_u8[8] = _Tolerance->R;
	AllowedDiff.m128i_u8[9] = _Tolerance->B;
	AllowedDiff.m128i_u8[10] = _Tolerance->G;
	AllowedDiff.m128i_u8[11] = _Tolerance->R;
	AllowedDiff.m128i_u8[12] = _Tolerance->B;
	AllowedDiff.m128i_u8[13] = _Tolerance->G;
	AllowedDiff.m128i_u8[14] = _Tolerance->R; 
	for( int y=0;y<RegionHeight;y++)
	{
		int x;
		x=0;
		for( x=0;x<(RegionWidth-5);x+=5 )
		{
			__m128i RGB,ColoDiff;
			int x3 = 3*x;
			//load 5 pixels
			RGB = _mm_loadu_si128((__m128i*)(&buff[x3]));
			int i=0;
			for( int i=0;i<15;i+=3)
			{
				SumR += buff[x3 + i + 0];
				SumG += buff[x3 + i + 1];
				SumB += buff[x3 + i + 2];
			}
			//do the abs to reference
			ColoDiff = _mm_sub_epi8( RGB, RefXMM );
			ColoDiff = _mm_abs_epi8( ColoDiff );
			//check if we are smaller then reference
//			CMPres = _mm_cmpgt_epi8( ColorSad, AllowedDiff );
			//mark result if apropriate
			if( ColoDiff.m128i_u8[0] <= AllowedDiff.m128i_u8[0] && ColoDiff.m128i_u8[1] <= AllowedDiff.m128i_u8[1] && ColoDiff.m128i_u8[2] <= AllowedDiff.m128i_u8[2] )
				Counter++;
			if( ColoDiff.m128i_u8[3] <= AllowedDiff.m128i_u8[3] && ColoDiff.m128i_u8[4] <= AllowedDiff.m128i_u8[4] && ColoDiff.m128i_u8[5] <= AllowedDiff.m128i_u8[5] )
				Counter++;
			if( ColoDiff.m128i_u8[6] <= AllowedDiff.m128i_u8[6] && ColoDiff.m128i_u8[7] <= AllowedDiff.m128i_u8[7] && ColoDiff.m128i_u8[8] <= AllowedDiff.m128i_u8[8] )
				Counter++;
			if( ColoDiff.m128i_u8[9] <= AllowedDiff.m128i_u8[9] && ColoDiff.m128i_u8[10] <= AllowedDiff.m128i_u8[10] && ColoDiff.m128i_u8[11] <= AllowedDiff.m128i_u8[11] )
				Counter++;
			if( ColoDiff.m128i_u8[12] <= AllowedDiff.m128i_u8[12] && ColoDiff.m128i_u8[13] <= AllowedDiff.m128i_u8[13] && ColoDiff.m128i_u8[14] <= AllowedDiff.m128i_u8[14] )
				Counter++;
		}/**/
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
	int _RegionSize = RegionWidth * RegionHeight;
	_CurrentColor->R = SumR /_RegionSize;
	_CurrentColor->G = SumG /_RegionSize;
	_CurrentColor->B = SumB /_RegionSize;

	return (float )( 100 * Counter ) / (float)(_RegionSize);
}

float GetColorValues_3(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *_Ref, RGB24Color *_Tolerance, RGB24Color *_CurrentColor)
{
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
	AllowedDiff.m128i_u8[0] = _Tolerance->B;
	AllowedDiff.m128i_u8[1] = _Tolerance->G;
	AllowedDiff.m128i_u8[2] = _Tolerance->R;
	AllowedDiff.m128i_u8[3] = _Tolerance->B;
	AllowedDiff.m128i_u8[4] = _Tolerance->G;
	AllowedDiff.m128i_u8[5] = _Tolerance->R;
	AllowedDiff.m128i_u8[6] = _Tolerance->B;
	AllowedDiff.m128i_u8[7] = _Tolerance->G;
	AllowedDiff.m128i_u8[8] = _Tolerance->R;
	AllowedDiff.m128i_u8[9] = _Tolerance->B;
	AllowedDiff.m128i_u8[10] = _Tolerance->G;
	AllowedDiff.m128i_u8[11] = _Tolerance->R;
	AllowedDiff.m128i_u8[12] = _Tolerance->B;
	AllowedDiff.m128i_u8[13] = _Tolerance->G;
	AllowedDiff.m128i_u8[14] = _Tolerance->R; 
	for( int y=0;y<RegionHeight;y++)
	{
		int x;
		x=0;
		for( x=0;x<(RegionWidth-5);x+=5 )
		{
			__m128i RGB,ColoDiff,CMPres;
			int x3 = 3*x;
			//load 5 pixels
			RGB = _mm_loadu_si128((__m128i*)(&buff[x3]));
			int i=0;
			for( int i=0;i<15;i+=3)
			{
				SumR += buff[x3 + i + 0];
				SumG += buff[x3 + i + 1];
				SumB += buff[x3 + i + 2];
			}
			//do the abs to reference
			ColoDiff = _mm_sub_epi8( RGB, RefXMM );
			ColoDiff = _mm_abs_epi8( ColoDiff );
			//check if we are smaller then reference
			CMPres = _mm_cmpgt_epi8( ColoDiff, AllowedDiff );
			//mark result if apropriate
			if( CMPres.m128i_u8[0] == 0 && CMPres.m128i_u8[1] == 0 && CMPres.m128i_u8[2] == 0 )
				Counter++;
			if( CMPres.m128i_u8[3] == 0 && CMPres.m128i_u8[4] == 0 && CMPres.m128i_u8[5] == 0 )
				Counter++;
			if( CMPres.m128i_u8[6] == 0 && CMPres.m128i_u8[7] == 0 && CMPres.m128i_u8[8] == 0 )
				Counter++;
			if( CMPres.m128i_u8[9] == 0 && CMPres.m128i_u8[10] == 0 && CMPres.m128i_u8[11] == 0 )
				Counter++;
			if( CMPres.m128i_u8[12] == 0 && CMPres.m128i_u8[13] == 0 && CMPres.m128i_u8[14] == 0 )
				Counter++;
		}/**/
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
	int _RegionSize = RegionWidth * RegionHeight;
	_CurrentColor->R = SumR /_RegionSize;
	_CurrentColor->G = SumG /_RegionSize;
	_CurrentColor->B = SumB /_RegionSize;

	return (float )( 100 * Counter ) / (float)(_RegionSize);
}

float GetColorValues_4(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *_Ref, RGB24Color *_Tolerance, RGB24Color *_CurrentColor)
{
	unsigned char *buff = pbuff + StartY * Stride + StartX * 3;
	int Counter = 0;
	int SumR = 0;
	int SumG = 0;
	int SumB = 0;
	__m128i RefXMM;
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
	__m128i Sum1,Sum2,Sum3,Sum4;
	Sum1 =  _mm_setzero_si128();
	Sum2 =  _mm_setzero_si128();
	Sum3 =  _mm_setzero_si128();
	Sum4 =  _mm_setzero_si128();
	for( int y=0;y<RegionHeight;y++)
	{
		int x;
		int Counter1,Counter2,Counter3,Counter4;
		Counter1 = Counter2 = Counter3 = Counter4 = 0;
		for( x=0;x<(RegionWidth-5);x+=5 )
		{
			__m128i RGB,ColoDiff;
			int x3 = 3*x;
			//load 5 pixels
			RGB = _mm_loadu_si128((__m128i*)(&buff[x3]));
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
			//check if we are smaller then reference
			//mark result if apropriate
			if( ColoDiff.m128i_u8[0] <= _Tolerance->B && ColoDiff.m128i_u8[1] <= _Tolerance->G && ColoDiff.m128i_u8[2] <= _Tolerance->R )
				Counter1++;
			if( ColoDiff.m128i_u8[3] <= _Tolerance->B && ColoDiff.m128i_u8[4] <= _Tolerance->G && ColoDiff.m128i_u8[5] <= _Tolerance->R )
				Counter2++;
			if( ColoDiff.m128i_u8[6] <= _Tolerance->B && ColoDiff.m128i_u8[7] <= _Tolerance->G && ColoDiff.m128i_u8[8] <= _Tolerance->R )
				Counter3++;
			if( ColoDiff.m128i_u8[9] <= _Tolerance->B && ColoDiff.m128i_u8[10] <= _Tolerance->G && ColoDiff.m128i_u8[11] <= _Tolerance->R )
				Counter4++;
			if( ColoDiff.m128i_u8[12] <= _Tolerance->B && ColoDiff.m128i_u8[13] <= _Tolerance->G && ColoDiff.m128i_u8[14] <= _Tolerance->R )
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

	return (float )( 100 * Counter ) / (float)(_RegionSize);
}

float GetColorValues_5(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *_Ref, RGB24Color *_Tolerance, RGB24Color *_CurrentColor)
{
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
	AllowedDiff.m128i_u8[0] = _Tolerance->B;
	AllowedDiff.m128i_u8[1] = _Tolerance->G;
	AllowedDiff.m128i_u8[2] = _Tolerance->R;
	AllowedDiff.m128i_u8[3] = _Tolerance->B;
	AllowedDiff.m128i_u8[4] = _Tolerance->G;
	AllowedDiff.m128i_u8[5] = _Tolerance->R;
	AllowedDiff.m128i_u8[6] = _Tolerance->B;
	AllowedDiff.m128i_u8[7] = _Tolerance->G;
	AllowedDiff.m128i_u8[8] = _Tolerance->R;
	AllowedDiff.m128i_u8[9] = _Tolerance->B;
	AllowedDiff.m128i_u8[10] = _Tolerance->G;
	AllowedDiff.m128i_u8[11] = _Tolerance->R;
	AllowedDiff.m128i_u8[12] = _Tolerance->B;
	AllowedDiff.m128i_u8[13] = _Tolerance->G;
	AllowedDiff.m128i_u8[14] = _Tolerance->R; 
	__m128i Sum1,Sum2,Sum3,Sum4;
	Sum1 =  _mm_setzero_si128();
	Sum2 =  _mm_setzero_si128();
	Sum3 =  _mm_setzero_si128();
	Sum4 =  _mm_setzero_si128();
	for( int y=0;y<RegionHeight;y++)
	{
		int x;
		int Counter1,Counter2,Counter3,Counter4;
		Counter1 = Counter2 = Counter3 = Counter4 = 0;
		for( x=0;x<(RegionWidth-5);x+=5 )
		{
			__m128i RGB,ColoDiff;
			int x3 = 3*x;
			//load 5 pixels
			RGB = _mm_loadu_si128((__m128i*)(&buff[x3]));
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
			if( ColoDiff.m128i_i8[0] <= 0 && ColoDiff.m128i_i8[1] <= 0 && ColoDiff.m128i_i8[2] <= 0 )
				Counter1++;
			if( ColoDiff.m128i_i8[3] <= 0 && ColoDiff.m128i_i8[4] <= 0 && ColoDiff.m128i_i8[5] <= 0 )
				Counter2++;
			if( ColoDiff.m128i_i8[6] <= 0 && ColoDiff.m128i_i8[7] <= 0 && ColoDiff.m128i_i8[8] <= 0 )
				Counter3++;
			if( ColoDiff.m128i_i8[9] <= 0 && ColoDiff.m128i_i8[10] <= 0 && ColoDiff.m128i_i8[11] <= 0 )
				Counter4++;
			if( ColoDiff.m128i_i8[12] <= 0 && ColoDiff.m128i_i8[13] <= 0 && ColoDiff.m128i_i8[14] <= 0 )
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

	return (float )( 100 * Counter ) / (float)(_RegionSize);
}

float GetColorValues_6(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *_Ref, RGB24Color *_Tolerance, RGB24Color *_CurrentColor)
{
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

	return (float )( 100 * Counter ) / (float)(_RegionSize);
}
