#include <stdio.h>
#include <stdlib.h>
#include "resampler.h"
#include <Windows.h>
#include <conio.h>

#if 0
unsigned int		m_nSrcWidth,m_nSrcHeight,m_nDesWidth,m_nDesHeight;
TResampler			*m_pResampler;		//!< Resampler filter instance
CRatioResampler		*m_pRatioResampler;	// ewlee 20081111 - use two resampler because of supporting to keep source aspect ratio
BYTE				*inb,*outb;
unsigned int		number_of_samples = 1000;

char alg_names[ RT_ENUM_COUNT ][500]={"2dLiniar","bilinear","bicubic","HDraw","raw_sse"};
float timings[ RT_ENUM_COUNT ], tests_made = 0;

//#define TB
#define TN

float runtest(int inw,int inh,int ow,int oh, TResamplerType type, int is_YUV=0)
{
   printf("Testing resampling algorithm %s with input resolution : %u x %u and output resolution %u x %u \n",alg_names[type],inw,inh,ow,oh);
   if( m_pResampler )
	   delete m_pResampler;
   m_pResampler = CreateResampler(type, ow, oh, inw, inh);
   unsigned int ts_start = GetTickCount();
   for(unsigned int i=0;i<number_of_samples;i++)
	   if( is_YUV == 1 )
	   {
//			ResampleYUV_I420(m_pResampler, outb, inb, ow, inw *2 );
			ResampleYUV_I420(m_pResampler, outb, inb, ow, inw );
	   }
	   else
		   ResampleRGB24(m_pResampler, outb, inb);
   unsigned int ts_end = GetTickCount();
   unsigned int test_duration = ts_end - ts_start;

   printf("Resize time %f Milliseconds / frame \n",(float)test_duration/number_of_samples);
   return ((float)test_duration/number_of_samples);
}

void Test_RGB()
{
   ////////////////////////////
   // UPSCALING
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176,144, 176*2, 144*2,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176,144, 176*2, 144*2,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176,144, 176*2, 144*2,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176,144, 176*2, 144*2,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176*2,144*2, 176*4, 144*4,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*2,144*2, 176*4, 144*4,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*2,144*2, 176*4, 144*4,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*2,144*2, 176*4, 144*4,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176*4,144*4, 176*8, 144*8,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*4,144*4, 176*8, 144*8,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*4,144*4, 176*8, 144*8,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*4,144*4, 176*8, 144*8,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176*2,144*2, 176*8, 144*8,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*2,144*2, 176*8, 144*8,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*2,144*2, 176*8, 144*8,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*2,144*2, 176*8, 144*8,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");/**/
   ////////////////////////////
   // Downscaling
   ////////////////////////////
/*	timings[ RT_BILINEAR ] += runtest( 176*2,144*2, 176, 144,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*2,144*2, 176, 144,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*4,144*4, 176, 144,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*4,144*4, 176, 144,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
	timings[ RT_BILINEAR ] += runtest( 176*2,144*2, 176, 144,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*2,144*2, 176, 144,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*2,144*2, 176, 144,RT_HDRAW);
	timings[ RT_RAW_SSE ] += runtest( 176*2,144*2, 176, 144,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176*4,144*4, 176*2, 144*2,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*4,144*4, 176*2, 144*2,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*4,144*4, 176*2, 144*2,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*4,144*4, 176*2, 144*2,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176*8,144*8, 176*4, 144*4,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*8,144*8, 176*4, 144*4,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*8,144*8, 176*4, 144*4,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*8,144*8, 176*4, 144*4,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");/**/
   ////////////////////////////
   // Noscale
   ////////////////////////////
/*	timings[ RT_BILINEAR ] += runtest( 176,144, 176, 144,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176,144, 176, 144,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176,144, 176, 144,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176,144, 176, 144,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176*2,144*2, 176*2, 144*2,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*2,144*2, 176*2, 144*2,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*2,144*2, 176*2, 144*2,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*2,144*2, 176*2, 144*2,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176*4,144*4, 176*4, 144*4,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*4,144*4, 176*4, 144*4,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*4,144*4, 176*4, 144*4,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*4,144*4, 176*4, 144*4,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
	timings[ RT_BILINEAR ] += runtest( 176*8,144*8, 176*8, 144*8,RT_BILINEAR);
	timings[ RT_2DLINEAR ] += runtest( 176*8,144*8, 176*8, 144*8,RT_2DLINEAR);
	timings[ RT_HDRAW ] += runtest( 176*8,144*8, 176*8, 144*8,RT_HDRAW);
//	timings[ RT_RAW_SSE ] += runtest( 176*8,144*8, 176*8, 144*8,RT_RAW_SSE);
	tests_made++;
	printf("======================================\n");/**/
   ////////////////////////////
}

void Test_YUV()
{
   ////////////////////////////
   // UPSCALING
   ////////////////////////////
#ifdef	TB 
	timings[ RT_BILINEAR ] += runtest( 176,144, 176*2, 144*2,RT_BILINEAR,1);	
#endif
#ifdef	TB 
	timings[ RT_2DLINEAR ] += runtest( 176,144, 176*2, 144*2,RT_BILINEAR,0);	
#endif
#ifdef	TN 
	timings[ RT_HDRAW ] += runtest( 176,144, 176*2, 144*2,RT_HDRAW,1);	
#endif
#ifdef	TN 
	timings[ RT_RAW_SSE ] += runtest( 176,144, 176*2, 144*2,RT_HDRAW,0);
#endif
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
#ifdef	TB 	
	timings[ RT_BILINEAR ] += runtest( 176*2,144*2, 176*4, 144*4,RT_BILINEAR,1);
#endif
#ifdef	TB 	
	timings[ RT_2DLINEAR ] += runtest( 176*2,144*2, 176*4, 144*4,RT_BILINEAR,0);
#endif
#ifdef	TN 	
	timings[ RT_HDRAW ] += runtest( 176*2,144*2, 176*4, 144*4,RT_HDRAW,1);
#endif
#ifdef	TN 	
	timings[ RT_RAW_SSE ] += runtest( 176*2,144*2, 176*4, 144*4,RT_HDRAW,0);
#endif
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
#ifdef	TB 	
	timings[ RT_BILINEAR ] += runtest( 176*4,144*4, 176*8, 144*8,RT_BILINEAR,1);
#endif
#ifdef	TB 	
		timings[ RT_2DLINEAR ] += runtest( 176*4,144*4, 176*8, 144*8,RT_BILINEAR,0);
#endif
#ifdef	TN 	
		timings[ RT_HDRAW ] += runtest( 176*4,144*4, 176*8, 144*8,RT_HDRAW,1);
#endif
#ifdef	TN 	
		timings[ RT_RAW_SSE ] += runtest( 176*4,144*4, 176*8, 144*8,RT_HDRAW,0);
#endif
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
#ifdef	TB 	
	timings[ RT_BILINEAR ] += runtest( 176*2,144*2, 176*8, 144*8,RT_BILINEAR,1);
#endif
#ifdef	TB 	
		timings[ RT_2DLINEAR ] += runtest( 176*2,144*2, 176*8, 144*8,RT_BILINEAR,0);
#endif
#ifdef	TN 	
		timings[ RT_HDRAW ] += runtest( 176*2,144*2, 176*8, 144*8,RT_HDRAW,1);
#endif
#ifdef	TN 	
		timings[ RT_RAW_SSE ] += runtest( 176*2,144*2, 176*8, 144*8,RT_HDRAW,0);
#endif
	tests_made++;
	printf("======================================\n");/**/
   ////////////////////////////
   // Downscaling
   ////////////////////////////
#ifdef	TB 	
	timings[ RT_BILINEAR ] += runtest( 176*2,144*2, 176, 144,RT_BILINEAR,1);
#endif
#ifdef	TB 	
		timings[ RT_2DLINEAR ] += runtest( 176*2,144*2, 176, 144,RT_BILINEAR,0);
#endif
#ifdef	TN 	
		timings[ RT_HDRAW ] += runtest( 176*2,144*2, 176, 144,RT_HDRAW,1);
#endif
#ifdef	TN 	
		timings[ RT_RAW_SSE ] += runtest( 176*2,144*2, 176, 144,RT_HDRAW,0);
#endif
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
#ifdef	TB 	
	timings[ RT_BILINEAR ] += runtest( 176*4,144*4, 176*2, 144*2,RT_BILINEAR,1);
#endif
#ifdef	TB 	
		timings[ RT_2DLINEAR ] += runtest( 176*4,144*4, 176*2, 144*2,RT_BILINEAR,0);
#endif
#ifdef	TN 	
		timings[ RT_HDRAW ] += runtest( 176*4,144*4, 176*2, 144*2,RT_HDRAW,1);
#endif
#ifdef	TN 	
		timings[ RT_RAW_SSE ] += runtest( 176*4,144*4, 176*2, 144*2,RT_HDRAW,0);
#endif
	tests_made++;
	printf("======================================\n");
   ////////////////////////////
#ifdef	TB 	
	timings[ RT_BILINEAR ] += runtest( 176*8,144*8, 176*4, 144*4,RT_BILINEAR,1);
#endif
#ifdef	TB 	
		timings[ RT_2DLINEAR ] += runtest( 176*8,144*8, 176*4, 144*4,RT_BILINEAR,0);
#endif
#ifdef	TN 	
		timings[ RT_HDRAW ] += runtest( 176*8,144*8, 176*4, 144*4,RT_HDRAW,1);
#endif
#ifdef	TN 	
		timings[ RT_RAW_SSE ] += runtest( 176*8,144*8, 176*4, 144*4,RT_HDRAW,0);
#endif
	tests_made++;
	printf("======================================\n");/**/
}

void main()
{
	m_nSrcWidth = 10*176;
	m_nSrcHeight = 10*144;
	m_nDesWidth = 10*176;
	m_nDesHeight = 10*144;

	inb = (BYTE *)malloc( m_nSrcWidth * m_nSrcHeight * 3 );
	outb = (BYTE *)malloc( m_nDesWidth * m_nDesHeight * 3 );

	//fill in buffer with random crap
	for(unsigned int i=0;i<m_nSrcWidth * m_nSrcHeight;i++)
		inb[i]= i % 10 + 1;

/*   if ((m_nDesWidth != m_nSrcWidth) || (m_nDesHeight != m_nSrcHeight))
   {
      // ewlee 20081111 - check aspect ratio value
      if (m_bKeepAspectRatio)   // keeping source aspect ratio
         m_pRatioResampler = new CRatioResampler(m_nSrcWidth, m_nSrcHeight, m_nDesWidth, m_nDesHeight, FALSE);
      else   // don't keep source aspect ratio
		 m_pResampler = CreateResampler(RT_HDRAW, m_nDesWidth, m_nDesHeight, m_nSrcWidth, m_nSrcHeight);
//		 m_pResampler = CreateResampler(RT_BILINEAR, m_nDesWidth, m_nDesHeight, m_nSrcWidth, m_nSrcHeight);
//		 m_pResampler = CreateResampler(RT_2DLINEAR, m_nDesWidth, m_nDesHeight, m_nSrcWidth, m_nSrcHeight);
   }*/

	memset( timings, 0, RT_ENUM_COUNT*sizeof(float) );
//	Test_RGB();
//	Test_YUV();

/*	float prev[6]={0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
	float cur[6]={0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
	for(float i=1;i<5;i+=0.1f)
	{
		for(int j=1;j<=5;j++)
			cur[j]= (int)(i*j);
		int is_same=1;
		for(int j=1;j<=5;j++)
		{
			if( cur[j] != prev[j] )
				is_same = 0;
			prev[j] = cur[j];
		}
		if( is_same == 0 )
		{
			printf("%f => ",i);
			for(int j=1;j<=5;j++)
				printf("%f,",prev[j]);
			printf("\n");
		}
	}/**/
   ////////////////////////////
/*	timings[ RT_BILINEAR ] += runtest( 176*4,144*4, 176, 144,RT_BILINEAR,1);
	timings[ RT_2DLINEAR ] += runtest( 176*4,144*4, 176, 144,RT_BILINEAR,0);
	timings[ RT_HDRAW ] += runtest( 176*4,144*4, 176, 144,RT_HDRAW,1);
	timings[ RT_RAW_SSE ] += runtest( 176*4,144*4, 176, 144,RT_HDRAW,0);
	tests_made++;*/

	for( int i=0;i<RT_ENUM_COUNT;i++)
		printf(" Avg time for mode %s : %f \n",alg_names[i],timings[ i ] / tests_made );

	//resample a file for testing
//	m_nSrcWidth = 176;	m_nSrcHeight = 144;	m_nDesWidth = 2*176;	m_nDesHeight = 2*144;
	m_nSrcWidth = 2*176;	m_nSrcHeight = 2*144;	m_nDesWidth = 176;	m_nDesHeight = 144;
//	m_pResampler = CreateResampler(RT_HDRAW, m_nDesWidth, m_nDesHeight, m_nSrcWidth, m_nSrcHeight);
	m_pResampler = CreateResampler(RT_BILINEAR, m_nDesWidth, m_nDesHeight, m_nSrcWidth, m_nSrcHeight);
	FILE *inf=fopen("d:/film/yuv/foreman_cif.yuv","rb");
//	FILE *inf=fopen("d:/film/yuv/suzie_cif.yuv","rb");
	FILE *outf=fopen("out.yuv","wb");
	for(int i=0;i<400;i++)
	{
		if( !fread(inb,1,m_nSrcWidth*m_nSrcHeight*3/2, inf) )
			break;
		ResampleYUV_I420( m_pResampler, outb, inb, m_nDesWidth, m_nSrcWidth );
		fwrite( outb, 1, m_nDesWidth*m_nDesHeight*3/2, outf );
	}
	fclose( inf );
	fclose( outf );/**/

   char c=getch();

   free( inb );
   free( outb );
}

#endif