#include "colorconv.h"

ColorConverter::ColorConverter()
{
  int i;
  // Fill the conversion tables. 
  for (i = 0; i <= MAXJSAMPLE; i++)
  {
    rgb_ycc_tab[i+R_Y_OFF] = ((ROUND(0.29900) * i)>>SCALEBITS) ;
    rgb_ycc_tab[i+G_Y_OFF] = ((ROUND(0.58700) * i)>>SCALEBITS) ;
    rgb_ycc_tab[i+B_Y_OFF] = ((ROUND(0.11400) * i + ONE_HALF)>>SCALEBITS) ;
    rgb_ycc_tab[i+R_U_OFF] = (((-ROUND(0.16874)) * i)>>SCALEBITS) ;
    rgb_ycc_tab[i+G_U_OFF] = (((-ROUND(0.33126)) * i)>>SCALEBITS) ;
    // We use a rounding factor of 0.5-epsilon for Cb and Cr.
    // This ensures that the maximum output will round to MAXJSAMPLE
    // not MAXJSAMPLE+1, so we don't have to saturate.
    rgb_ycc_tab[i+B_U_OFF] =(( ROUND(0.50000) * i + CBCR_OFFSET + ONE_HALF-1)>>SCALEBITS) ;
    //B=>Cb and R=>Cr tables are the same
    //rgb_ycc_tab[i+R_V_OFF] = FIX(0.50000) * i + CBCR_OFFSET + ONE_HALF-1;
    rgb_ycc_tab[i+G_V_OFF] =((((-ROUND(0.41869)) * i))>>SCALEBITS) ;
    rgb_ycc_tab[i+B_V_OFF] =((((-ROUND(0.08131)) * i))>>SCALEBITS) ;
  }
#ifdef USE_BOOSTED_PRECISE_YUV_TO_RGB
//memset(YUV_to_RGB,0,BYTES_FOR_LOOKUP*sizeof(BYTE));
  for( int Y = 0; Y < 255; Y += COLOR_STEP )
	  for( int U = 0; U < 255; U += COLOR_STEP )
		for( int V = 0; V < 255; V += COLOR_STEP )
		{
			int CY,CU,CV;
			int R,G,B;
			int C,D,E;

			CY = Y;
			CU = U;
			CV = V;

			//microsoft formula -> this sucks !
/*			C = CY - 16;
			D = CU - 128;
			E = CV - 128;
			C = 298 * C;
			R = ( C           + 409 * E + 128) >> 8;
			G = ( C - 100 * D - 208 * E + 128) >> 8;
			B = ( C + 516 * D           + 128) >> 8;/**/

			//Avery Lee's JFIF adjustments to CCIR 601
			D = CU - 128 ;
			E = CV - 128 ;
			C = CY << 16 ;

			R = ( C + 91881 * E ) >> 16;
			G = ( C - 22553 * D - 46801 * E ) >> 16;
			B = ( C + 116129 * D ) >> 16;/**/

			if( R < 0 ) 
				R = 0;
			if( R > 255 ) 
				R = 255;
			if( G < 0 ) 
				G = 0;
			if( G > 255 ) 
				G = 255;
			if( B < 0 ) 
				B = 0;
			if( B > 255 ) 
				B = 255; /**/

			CY = Y >> PRECISSION_DOWNSCALE;
			CU = U >> PRECISSION_DOWNSCALE;
			CV = V >> PRECISSION_DOWNSCALE;
			int index = ( CY << ( BITS_PER_PLANE * 2 ) ) | ( CU << ( BITS_PER_PLANE ) ) | CV;
			index *= 3;
			if( index + 2 >= BYTES_FOR_LOOKUP || index < 0 )
			{
				//panic !!!!
				index = 0;
			}

//			if( YUV_to_RGB[ index + 0 ] != 0 || YUV_to_RGB[ index + 1 ] != 0 || YUV_to_RGB[ index + 2 ] != 0 )
//				index = 0; //collision
			YUV_to_RGB[ index + 0 ] = R;
			YUV_to_RGB[ index + 1 ] = G;
			YUV_to_RGB[ index + 2 ] = B; /**/
		}
#endif
		//Hoi Ming lookup table
	for (i = 0; i < 256; i++) RGB2YUV_YR[i] = (int)(0.299f * (i<<8));
	for (i = 0; i < 256; i++) RGB2YUV_YG[i] = (int)(0.587f * (i<<8));
	for (i = 0; i < 256; i++) RGB2YUV_YB[i] = (int)(0.114f * (i<<8));
	for (i = 0; i < 256; i++) RGB2YUV_UR[i] = (int)((-0.14713f) * (i<<8));
	for (i = 0; i < 256; i++) RGB2YUV_UG[i] = (int)((-0.28886f) * (i<<8));
	for (i = 0; i < 256; i++) RGB2YUV_UB[i] = (int)(0.436f * (i<<8));
	for (i = 0; i < 256; i++) RGB2YUV_VR[i] = (int)(0.615f * (i<<8));
	for (i = 0; i < 256; i++) RGB2YUV_VG[i] = (int)((-0.51499f) * (i<<8));
	for (i = 0; i < 256; i++) RGB2YUV_VB[i] = (int)((-0.10001f) * (i<<8));
	inited_h = inited_w = 0;
	uu = vv = NULL;
}

ColorConverter::~ColorConverter()
{
	if( uu )
	{
		delete uu;
		uu = NULL;
	}
	if( vv )
	{
		delete vv;
		vv = NULL;
	}
}

__inline BYTE ColorConverter::rgb_to_y(UINT r,UINT g,UINT b)
{
   return (BYTE) ( (rgb_ycc_tab[r+R_Y_OFF] + rgb_ycc_tab[g+G_Y_OFF] + rgb_ycc_tab[b+B_Y_OFF]));
}

__inline BYTE ColorConverter::rgb_to_u(UINT r,UINT g,UINT b)
{
   return ((rgb_ycc_tab[r+R_U_OFF] + rgb_ycc_tab[g+G_U_OFF] + rgb_ycc_tab[b+B_U_OFF]));
}
__inline BYTE ColorConverter::rgb_to_v(UINT r,UINT g,UINT b)
{
   return ((rgb_ycc_tab[r+R_V_OFF] + rgb_ycc_tab[g+G_V_OFF] + rgb_ycc_tab[b+B_V_OFF]) );
}

//destination buffer has special setup
void ColorConverter::BGR24_to_YUV_NX(BYTE *src, BYTE *dstY, BYTE *dstU, BYTE *dstV, ULONG w, ULONG h,ULONG dest_stride)
{ 
	ULONG i,j,k;
	ULONG src_stride=w*3;
	ULONG dst_stride=dest_stride;
	int max_input_image_size = src_stride * h;
	src = src + max_input_image_size - src_stride;  

	for (i=0; i < h; i+=2)
	{
	  for (j=0,k=0; j < w; j+=2)
	  {
		 dstY[j]=rgb_to_y(src[k+2],src[k+1],src[k]);

		 dstU[j>>1]=rgb_to_u(src[k+2],src[k+1],src[k]);
		 dstV[j>>1]=rgb_to_v(src[k+2],src[k+1],src[k]);
		 k+=3;
		 dstY[j+1]=rgb_to_y(src[k+2],src[k+1],src[k]);
		 k+=3;
	  }         

	  dstY+=dst_stride;
	  dstU+=(dst_stride>>1);
	  dstV+=(dst_stride>>1);
	  src-=src_stride;

	  for (j=0,k=0; j < w; j++,k+=3)
		 dstY[j]=rgb_to_y(src[k+2],src[k+1],src[k]);
	 
	  dstY+=dst_stride;
	  src-=src_stride;
	}
}/**/

void ColorConverter::BGR24_to_YUV(BYTE *src, BYTE *dst, ULONG w, ULONG h)
{ 
	ULONG dst_stride=w;
	BYTE *Y = dst;
	BYTE *U = dst + w * h;
	BYTE *V = U + ( w * h ) / 4;
	BGR24_to_YUV_NX(src,Y,U,V,w,h,dst_stride);
}/**/

void ColorConverter::BGR24_to_YUV_interpolate(BYTE *src, BYTE *dst, ULONG w, ULONG h)
{ 
	ULONG dst_stride=w;
	BYTE *Y = dst;
	BYTE *U = dst + w * h;
	BYTE *V = U + ( w * h ) / 4;
	BGR24_to_YUV_interpolate_NX(src,Y,U,V,w,h,dst_stride);
}/**/

//destination buffer has special setup
void ColorConverter::BGR24_to_YUV_interpolate_NX(BYTE *src, BYTE *dstY, BYTE *dstU, BYTE *dstV, ULONG w, ULONG h,ULONG dest_stride)
{ 
	ULONG i,j,k;
	ULONG src_stride=w*3;
	ULONG dst_stride=dest_stride;
	int max_input_image_size = src_stride * h;
	src = src + max_input_image_size - src_stride;  

	for (i=0; i < h; i+=2)
	{
	  for (j=0,k=0; j < w; j+=2)
	  {
		 dstY[j]=rgb_to_y(src[k+2] ,src[k+1],src[k])CONVERSION_FORCED_PRECISSION_FACTOR_Y;

		 //we only smooth color planes to keep sharp edges
		 int Ri = src[k+2] + src[k+2+3] + src[k+2-src_stride] + src[k+2-src_stride+3];		 Ri = Ri >> 2;
		 int Gi = src[k+1] + src[k+1+3] + src[k+1-src_stride] + src[k+1-src_stride+3];		 Gi = Gi >> 2;
	     int Bi = src[k+0] + src[k+0+3] + src[k+0-src_stride] + src[k+0-src_stride+3];		 Bi = Bi >> 2;
		 dstU[j>>1]=rgb_to_u(Ri,Gi,Bi)CONVERSION_FORCED_PRECISSION_FACTOR_UV;
		 dstV[j>>1]=rgb_to_v(Ri,Gi,Bi)CONVERSION_FORCED_PRECISSION_FACTOR_UV;
		 k+=3;
		 dstY[j+1]=rgb_to_y(src[k+2],src[k+1],src[k])CONVERSION_FORCED_PRECISSION_FACTOR_Y;
		 k+=3;
	  }         

	  dstY+=dst_stride;
	  dstU+=(dst_stride>>1);
	  dstV+=(dst_stride>>1);
	  src-=src_stride;

	  for (j=0,k=0; j < w; j++,k+=3)
		 dstY[j]=rgb_to_y(src[k+2],src[k+1],src[k])CONVERSION_FORCED_PRECISSION_FACTOR_Y;
	 
	  dstY+=dst_stride;
	  src-=src_stride;
	}
}/**/

void ColorConverter::BGR24_to_YUV_2x2_smooth(BYTE *src, BYTE *dst, ULONG w, ULONG h)
{

	unsigned char *u,*v,*y;
	unsigned char *pu1,*pu2,*pu3,*pu4;
	unsigned char *pv1,*pv2,*pv3,*pv4;
	unsigned char *r,*g,*b;
	ULONG i,j;

	if( inited_h < h || inited_w < w )
	{
		uu=new unsigned char[w*h];
		vv=new unsigned char[w*h];
		if(uu==NULL || vv==NULL)
			return;
		inited_h = h;
		inited_w = w;
	}

	y=dst+w*(h-1);
	u=uu;
	v=vv;


	b=src;
	g=src+1;
	r=src+2;
	/*r=in;
	g=in+1;
	b=in+2;*/

	for(i=0;i<h;i++)
	{
		for(j=0;j<w;j++)
		{
			 *y++ = rgb_to_y(*r,*g,*b);
			 *u++ = rgb_to_u(*r,*g,*b);
			 *v++ = rgb_to_v(*r,*g,*b);

/*			*y++ = ( RGB2YUV_YR[*r] + RGB2YUV_YG[*g] + RGB2YUV_YB[*b] + 128 )>>8;
			*u++ = (( RGB2YUV_UR[*r] + RGB2YUV_UG[*g] + RGB2YUV_UB[*b] + 32768 + 128 )>>8) ;
			*v++ = (( RGB2YUV_VR[*r] + RGB2YUV_VG[*g] + RGB2YUV_VB[*b] + 32768 + 128 )>>8) ;*/

		/*	*y++ = (int)(*r)*0.299 + (int)(*g)*0.587 + (int)(*b)*0.114;
			*u++ = max(0, min(255, (int)(*r)*(-0.14713) + (int)(*g)*(-0.28886) + (int)(*b)*0.436 +128 ));
			*v++ = max(0, min(255, (int)(*r)*0.615 + (int)(*g)*(-0.51499) + (int)(*b)*(-0.10001) +128));*/

		/*	*y++ = (*r)*0.299 + (*g)*0.587 + (*b)*0.114;
			*u++ = (*r)*(-0.14713) + (*g)*(-0.28886) + (*b)*0.436;
			*v++ = (*r)*0.615 + (*g)*(-0.51499) + (*b)*(-0.10001);
*/
			/**y++=( RGB2YUV_YR[*r]  +RGB2YUV_YG[*g]+RGB2YUV_YB[*b]+1048576)>>16;
			*u++=(-RGB2YUV_UR[*r]  -RGB2YUV_UG[*g]+RGB2YUV_UBVR[*b]+8388608)>>16;
			*v++=( RGB2YUV_UBVR[*r]-RGB2YUV_VG[*g]-RGB2YUV_VB[*b]+8388608)>>16;*/

			r+=3;
			g+=3;
			b+=3;
		}
		y -= (w<<1);

	}

	u=dst+w*h+w*h/4-(w/2);
	v=u+(w*h)/4;
	/*u=out+w*h;
	v=u+(w*h)/4;*/

	// For U
	pu1=uu;
	pu2=pu1+1;
	pu3=pu1+w;
	pu4=pu3+1;

	// For V
	pv1=vv;
	pv2=pv1+1;
	pv3=pv1+w;
	pv4=pv3+1;

	// Do sampling....
	for(i=0;i<h;i+=2)
	{

		for(j=0;j<w;j+=2)
		{
			*u++=(*pu1+*pu2+*pu3+*pu4 + 2)>>2;
			*v++=(*pv1+*pv2+*pv3+*pv4 + 2)>>2;

			pu1+=2;
			pu2+=2;
			pu3+=2;
			pu4+=2;

			pv1+=2;
			pv2+=2;
			pv3+=2;
			pv4+=2;
		}

		u -= w;
		v -= w;

		pu1+=w;
		pu2+=w;
		pu3+=w;
		pu4+=w;

		pv1+=w;
		pv2+=w;
		pv3+=w;
		pv4+=w;

	}
}

void ColorConverter::YUV_to_BGR24(BYTE *src, BYTE *dst, ULONG w, ULONG h)
{ 
#ifdef USE_BOOSTED_PRECISE_YUV_TO_RGB
	BYTE *Y;
	BYTE *U;
	BYTE *V;
	ULONG i,j,k;
	ULONG src_stride=w;
	ULONG dst_stride=w*3;
	Y = src;
	U = Y + src_stride * h;
	V = U + ( src_stride * h ) / 4;
	int max_input_image_size = dst_stride * h;
	dst = dst + max_input_image_size
		- dst_stride
		;

	for( i=1;i<=h;i++ )
	{
		for( j=0,k=0;j<w;j++,k+=3)
		{
			int CY,CU,CV;
			CY = Y[ j ];
			CU = U[ j >> 1 ];
			CV = V[ j >> 1 ];

			CY = CY >> PRECISSION_DOWNSCALE;
			CU = CU >> PRECISSION_DOWNSCALE;
			CV = CV >> PRECISSION_DOWNSCALE;
			int index = ( CY << ( BITS_PER_PLANE * 2 ) ) | ( CU << ( BITS_PER_PLANE ) ) | CV;
			index *= 3;

			if( index + 2 >= BYTES_FOR_LOOKUP )
				index = 0;

			dst[ k + 0 ] = YUV_to_RGB[ index + 2];	//b
			dst[ k + 1 ] = YUV_to_RGB[ index + 1 ];	//g
			dst[ k + 2 ] = YUV_to_RGB[ index + 0 ];	//r
		}
		dst -= dst_stride;
		Y += src_stride;
		if( i % 2 == 0 )
		{
			U += src_stride >> 1;
			V += src_stride >> 1;
		}
	}
#else
	YUV_to_BGR24_precize(src,dst,w,h);
#endif
}/**/
void ColorConverter::YUV_to_BGR24_precize(BYTE *src, BYTE *dst, ULONG w, ULONG h)
{ 
	BYTE *Y;
	BYTE *U;
	BYTE *V;
	ULONG i,j,k;
	ULONG src_stride=w;
	ULONG dst_stride=w*3;
	Y = src;
	U = Y + src_stride * h;
	V = U + ( src_stride * h ) / 4;
	int max_input_image_size = dst_stride * h;
	dst = dst + max_input_image_size
		- dst_stride
		;

	for( i=1;i<=h;i++ )
	{
		for( j=0,k=0;j<w;j++,k+=3)
		{
			int CY,CU,CV;
			CY = Y[ j ];
			CU = U[ j >> 1 ];
			CV = V[ j >> 1 ];

			int R,G,B;
			int C,D,E;

			//Avery Lee's JFIF adjustments to CCIR 601
			D = CU - 128 ;
			E = CV - 128 ;
			C = CY << 16 ;

			R = ( C + 91881 * E ) >> 16;
			G = ( C - 22553 * D - 46801 * E ) >> 16;
			B = ( C + 116129 * D ) >> 16;/**/

			if( R < 0 ) 
				R = 0;
			if( R > 255 ) 
				R = 255;
			if( G < 0 ) 
				G = 0;
			if( G > 255 ) 
				G = 255;
			if( B < 0 ) 
				B = 0;
			if( B > 255 ) 
				B = 255; /**/

			dst[ k + 0 ] = B;	//b
			dst[ k + 1 ] = G;	//g
			dst[ k + 2 ] = R;	//r
		}
		dst -= dst_stride;
		Y += src_stride;
		if( i % 2 == 0 )
		{
			U += src_stride >> 1;
			V += src_stride >> 1;
		}
	}
}/**/
/*
void ColorConverter::RGB24_to_YUV(BYTE *src, BYTE *dst, ULONG w, ULONG h)
{ 

	BYTE *Y;
	BYTE *U;
	BYTE *V;
	ULONG i,j,k;
	ULONG src_stride=w*3;
	ULONG dst_stride=w;
	int max_input_image_size = src_stride * h;
	src = src + max_input_image_size - src_stride*3;  
	Y = dst;
	U = dst + w * h;
	V = U + ( w * h ) / 4 ;

	for (i=0; i < h; i+=2)
	{
	  for (j=0,k=0; j < w; j+=2)
	  {
		 Y[j]=rgb_to_y(src[k],src[k+1],src[k+2]);

		 U[j>>1]=rgb_to_u(src[k],src[k+1],src[k+2]);
		 V[j>>1]=rgb_to_v(src[k],src[k+1],src[k+2]);
		 k+=3;
		 Y[j+1]=rgb_to_y(src[k],src[k+1],src[k+2]);
		 k+=3;
	  }         

	  Y+=dst_stride;
	  U+=(dst_stride>>1);
	  V+=(dst_stride>>1);
	  src-=src_stride;

	  for (j=0,k=0; j < w; j++,k+=3)
		 Y[j]=rgb_to_y(src[k],src[k+1],src[k+2]);
	 
	  Y+=dst_stride;
	  src-=src_stride;
	}
}/**/
/*
void ColorConverter::RGB24_to_YUV(BYTE *src, BYTE *dst, ULONG w, ULONG h)
{ 
	BYTE *Y;
	BYTE *U;
	BYTE *V;
	ULONG src_stride=w*3;
	ULONG dst_stride=w;
	int max_input_image_size = src_stride * h;
	src = src + max_input_image_size - src_stride*3;  
	Y = dst;
	U = dst + w * h;
	V = U + ( w * h ) / 4 ;
	src_stride=w*3*2;
	for( ULONG y=0;y<h;y+=2)
	{
		int R,G,B;
		for(ULONG x=0;x<w;x+=2)
		{
			R = src[0];
			G = src[1];
			B = src[2];
			src += 3;
			*Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
			Y++;
			*U = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
			U++;
			*V = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
			V++;
			R = src[0];
			G = src[1];
			B = src[2];
			src += 3;
			*Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
			Y++;
		}
		src-=src_stride;
		for(ULONG x=0;x<w;x++)
		{
			R = src[0];
			G = src[1];
			B = src[2];
			src += 3;
			*Y = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16;
			Y++;
		}
		src-=src_stride;
	}
}/**/