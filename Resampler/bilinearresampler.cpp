//#include "SObjects.h"
#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <math.h>
#include "BilinearResampler.h"

#include <stdio.h>

#define BILINEAR_PREC  15
#define BILINEAR_SCALE (1<<15)
#define BILINEAR_MASK  ((1<<15)-1)

//Hoi Ming TBB
//#define _TBB_SUPPORT
//end

//Hoi Ming
#define EDGE_PRESERVING_FILTER

#ifdef EDGE_PRESERVING_FILTER
#include <malloc.h>
#endif

//Hoi Ming TBB
#ifdef _TBB_SUPPORT
typedef BOOL (WINAPI *LPFN_GLPI)(
								 PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, 
								 PDWORD);

// Helper function to count set bits in the processor mask.
DWORD CountSetBits(ULONG_PTR bitMask)
{
	DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
	DWORD i;

	for (i = 0; i <= LSHIFT; ++i)
	{
		bitSetCount += ((bitMask & bitTest)?1:0);
		bitTest/=2;
	}

	return bitSetCount;
}
#endif
//end

void CBilinearResampler::CopyRGB( BYTE * pDst, BYTE * pSrc, int nSourceStride  )
{
	if( nSourceStride == m_sw*3 )
	{
		memcpy( pDst, pSrc, m_sw*m_sh*3 );
	}
	else
	{
		int Width = m_sw*3;
		for( int No = 0; No < m_sh; ++No )
		{
			memcpy( pDst, pSrc, Width );
			pDst += Width;
			pSrc += nSourceStride;
		}
	}
}
void CBilinearResampler::CopyYUV420( BYTE * pDst, BYTE * pSrcY, BYTE * pSrcU, BYTE * pSrcV, int nSourceStride )
{
	if( nSourceStride == m_sw )
	{
		memcpy( pDst, pSrcY, m_sw*m_sh );
		pDst += (m_sw*m_sh);

		memcpy( pDst, pSrcU, m_sw*m_sh/4 );
		pDst += (m_sw*m_sh/4);

		memcpy( pDst, pSrcV, m_sw*m_sh/4 );
	}
	else
	{
		int No = 0;
		int CrH = m_sh/2;
		int CrW = m_sw/2;
		int CrStride = nSourceStride/2;
		BYTE * pDstY = pDst;
		BYTE * pDstU = pDstY+(m_sw*m_sh);
		BYTE * pDstV = pDstU+(m_sw*m_sh)/4;

		for( No = 0; No < m_sh; ++No )
		{
			memcpy( pDstY, pSrcY, m_sw );
			pDstY += m_sw;
			pSrcY += nSourceStride;
		}
		for( No = 0; No < CrH; ++No)
		{
			memcpy( pDstU, pSrcU, CrW );
			pDstU += CrW;
			pSrcU += CrStride;

			memcpy( pDstV, pSrcV, CrW );
			pDstV += CrW;
			pSrcV += CrStride;
		}
	}

}

CBilinearResampler::CBilinearResampler(int sw, int sh, int dw, int dh,int precise,int lumalow , int lumahigh ,int chromalow ,int chromahigh )
{
	
	double inc;
	int i,j;
	
	
	m_sw=sw;
	m_sh=sh;
	m_dw=dw;
	m_dh=dh;
	
	//Hoi Ming test 4May2009
	for(i = -255; i <= 255; i++){
		absArray[i+255] = abs(i);
	}
	//end
	m_x_ratio=(double)(sw)/(double)dw;
	m_y_ratio=(double)(sh)/(double)dh;
	
	inc=m_x_ratio*BILINEAR_SCALE;
	m_w_inc=(int)inc;
	
	inc=m_y_ratio*BILINEAR_SCALE;
	m_h_inc=(int)inc;
	
	m_precise=precise;

	SetupLowpassKernels();
	
	m_action = BILINEAR;

	if(m_precise)
	{
		int ratio_sum;
		ratio_sum = (int)(m_x_ratio + m_y_ratio);
		//check if downsampling
		if( ratio_sum > 2 )
		{
			if(ratio_sum < 6)
				m_action = BILINEAR_3;
			else
				if( ratio_sum < 10 )
					m_action = BILINEAR_5;
				else
					m_action = BILINEAR_7;
		}
	}

//Hoi Ming
#ifdef EDGE_PRESERVING_FILTER

	m_action = BILINEAR;

	//Hoi Ming Sinc Filter
	//[0]: 1/4; [1]: 1/2; [2]: 3/4
	SincFilter[0][0] = -70;
	SincFilter[0][1] = 540;
	SincFilter[0][2] = 70;
	SincFilter[0][3] = -28;

	SincFilter[1][0] = -110;
	SincFilter[1][1] = 524;
	SincFilter[1][2] = 162;
	SincFilter[1][3] = -64;

	SincFilter[2][0] = -128;
	SincFilter[2][1] = 464;
	SincFilter[2][2] = 272;
	SincFilter[2][3] = -96;

	SincFilter[3][0] = -120;
	SincFilter[3][1] = 376;
	SincFilter[3][2] = 376;
	SincFilter[3][3] = -120;

	SincFilter[4][0] = -96;
	SincFilter[4][1] = 272;
	SincFilter[4][2] = 464;
	SincFilter[4][3] = -128;

	SincFilter[5][0] = -64;
	SincFilter[5][1] = 162;
	SincFilter[5][2] = 524;
	SincFilter[5][3] = -110;

	SincFilter[6][0] = -28;
	SincFilter[6][1] = 70;
	SincFilter[6][2] = 540;
	SincFilter[6][3] = -70;
	//end

	int filter_coeff[5][5] = {
		{0,1,1,1,0},
		{1,2,2,2,1},
		{1,2,4,2,1},
		{1,2,2,2,1},
		{0,1,1,1,0}
	};

	for(int i = 0;i < 7;i++)
		for(int j = 0;j<7;j++)
			for(int h = 0;h<4;h++)
				for(int k = 0;k<4;k++)
					SincFilter_mix[i][j][h][k] = SincFilter[i][h]*SincFilter[j][k]/512;

	for(int i = 0;i < 7;i++){
		for(int j = 0;j<7;j++){
			//init SincFilter_mix_conv_1_1
			memset(&SincFilter_mix_conv_1_1[i][j][0][0],0,sizeof(short)*8*8);
			
			for(int h = 0; h<4; h++){
				for(int k = 0; k<4; k++){
					SincFilter_mix_conv_1_1[i][j][h+1][k+1] = SincFilter_mix[i][j][h][k]*4;
					if(k>0)
						SincFilter_mix_conv_1_1[i][j][h+1][k+1] += SincFilter_mix[i][j][h][k-1];
					if(h>0)
						SincFilter_mix_conv_1_1[i][j][h+1][k+1] += SincFilter_mix[i][j][h-1][k];
					if(k<3)
						SincFilter_mix_conv_1_1[i][j][h+1][k+1] += SincFilter_mix[i][j][h][k+1];
					if(h<3)
						SincFilter_mix_conv_1_1[i][j][h+1][k+1] += SincFilter_mix[i][j][h+1][k];
				}
			}
			for(int h = 0; h < 4; h++){
				SincFilter_mix_conv_1_1[i][j][h+1][0] = SincFilter_mix[i][j][h][0];
				SincFilter_mix_conv_1_1[i][j][h+1][5] = SincFilter_mix[i][j][h][3];
			}
			for(int k = 0; k < 4; k++){
				SincFilter_mix_conv_1_1[i][j][0][k+1] = SincFilter_mix[i][j][0][k];
				SincFilter_mix_conv_1_1[i][j][5][k+1] = SincFilter_mix[i][j][3][k];
			}
			//end

			//init SincFilter_mix_conv_2_2
			memset(&SincFilter_mix_conv_2_2[i][j][0][0],0,sizeof(short)*8*8);

			for(int h = 0; h<4; h++){
				for(int k = 0; k<4; k++){
					for(int m = -2; m < 3;m++){
						for(int n = -2;n < 3;n++){
							SincFilter_mix_conv_2_2[i][j][h+2+m][k+2+n] += SincFilter_mix[i][j][h][k] * filter_coeff[m+2][n+2];
						}
					}

				}
			}
			//end

			//printf("!!");
			
		}
	}

	for(int i = 0; i < 7; i++){
		memset(&SincFilter_mix_conv_1_1_h[i][0][0],0,sizeof(short)*3*8);
		for(int k = 0; k < 4; k++){
			SincFilter_mix_conv_1_1_h[i][0][k+1] = SincFilter[i][k];
			SincFilter_mix_conv_1_1_h[i][2][k+1] = SincFilter[i][k];
		}
		SincFilter_mix_conv_1_1_h[i][1][0] = SincFilter[i][0];
		SincFilter_mix_conv_1_1_h[i][1][5] = SincFilter[i][3];

		SincFilter_mix_conv_1_1_h[i][1][1] = 4*SincFilter[i][0] + SincFilter[i][1];
		SincFilter_mix_conv_1_1_h[i][1][2] = 4*SincFilter[i][1] + SincFilter[i][0] + SincFilter[i][2];
		SincFilter_mix_conv_1_1_h[i][1][3] = 4*SincFilter[i][2] + SincFilter[i][1] + SincFilter[i][3];
		SincFilter_mix_conv_1_1_h[i][1][4] = 4*SincFilter[i][3] + SincFilter[i][2];

		memset(&SincFilter_mix_conv_2_2_h[i][0][0],0,sizeof(short)*5*8);
		for(int k = 0; k<4; k++){
			for(int m = -2; m < 3;m++){
				for(int n = -2;n < 3;n++){
					SincFilter_mix_conv_2_2_h[i][2+m][k+2+n] += SincFilter[i][k] * filter_coeff[m+2][n+2];
				}
			}
		}

		memset(&SincFilter_mix_conv_1_1_v[i][0][0],0,sizeof(short)*6*8);

		int filter_coeff_2[3][3] = {
			{0,1,0},
			{1,4,1},
			{0,1,0}
		};

		for(int h = 0; h<4; h++){
			for(int m = -1; m < 2;m++){
				for(int n = -1;n < 2;n++){
					SincFilter_mix_conv_1_1_v[i][h+1+m][1+n] += SincFilter[i][h] * filter_coeff_2[m+1][n+1];
				}
			}
		}
		
	}

	for(int i = 0; i < 7; i++){
		
		memset(&SincFilter_mix_conv_2_2_v[i][0][0],0,sizeof(short)*8*8);
		for(int h = 0; h<4; h++){
			for(int m = -2; m < 3;m++){
				for(int n = -2;n < 3;n++){
					SincFilter_mix_conv_2_2_v[i][h+2+m][2+n] += SincFilter[i][h] * filter_coeff[m+2][n+2];
				}
			}
		}
		
	}
	
	/*for(int i = 0;i < 7;i++){
		for(int j = 0;j<7;j++){
			for(int h = 0; h<6; h++){
				for(int k = 0; k<6; k++){
					printf("%d ",SincFilter_mix_conv_1_1[i][j][k][h]);
				}
				printf("\n");
			}
			printf("\n");
		}
	}*/

	for(int i = 0;i < 16;i++){
		for(int j = 0;j < 16;j++){
			//int p = i<<11;
			//int q = j<<11;
			Tab_P_Q[i][j][0] = (16-i)*(16-j);
			Tab_P_Q[i][j][1] = i*(16-j);
			Tab_P_Q[i][j][2] = (16-i)*j;
			Tab_P_Q[i][j][3] = i*j;

			//memset(&Tab_P_Q_mix_conv_1_1[i][j][0][0],0,16*sizeof(short));
			Tab_P_Q_mix_conv_1_1[i][j][0][1] = Tab_P_Q[i][j][0];
			Tab_P_Q_mix_conv_1_1[i][j][0][2] = Tab_P_Q[i][j][1];
			Tab_P_Q_mix_conv_1_1[i][j][3][1] = Tab_P_Q[i][j][2];
			Tab_P_Q_mix_conv_1_1[i][j][3][2] = Tab_P_Q[i][j][3];

			Tab_P_Q_mix_conv_1_1[i][j][1][0] = Tab_P_Q[i][j][0];
			Tab_P_Q_mix_conv_1_1[i][j][2][0] = Tab_P_Q[i][j][2];
			Tab_P_Q_mix_conv_1_1[i][j][1][3] = Tab_P_Q[i][j][1];
			Tab_P_Q_mix_conv_1_1[i][j][2][3] = Tab_P_Q[i][j][3];

			Tab_P_Q_mix_conv_1_1[i][j][1][1] = (Tab_P_Q[i][j][0]<<2) + Tab_P_Q[i][j][1] + Tab_P_Q[i][j][2];
			Tab_P_Q_mix_conv_1_1[i][j][1][2] = (Tab_P_Q[i][j][1]<<2) + Tab_P_Q[i][j][0] + Tab_P_Q[i][j][3];
			Tab_P_Q_mix_conv_1_1[i][j][2][1] = (Tab_P_Q[i][j][2]<<2) + Tab_P_Q[i][j][0] + Tab_P_Q[i][j][3];
			Tab_P_Q_mix_conv_1_1[i][j][2][2] = (Tab_P_Q[i][j][3]<<2) + Tab_P_Q[i][j][1] + Tab_P_Q[i][j][2];

			Tab_P_Q_mix_conv_1_1[i][j][0][0] = 
			Tab_P_Q_mix_conv_1_1[i][j][0][3] = 
			Tab_P_Q_mix_conv_1_1[i][j][3][0] = 
			Tab_P_Q_mix_conv_1_1[i][j][3][3] = 0;

			


		}
		Tab_P_Q_mix_conv_1_1_h[i][0][0] = 
		Tab_P_Q_mix_conv_1_1_h[i][0][3] = 
		Tab_P_Q_mix_conv_1_1_h[i][2][0] = 
		Tab_P_Q_mix_conv_1_1_h[i][2][3] = 0;

		Tab_P_Q_mix_conv_1_1_h[i][0][1] = Tab_P_Q[i][0][0];
		Tab_P_Q_mix_conv_1_1_h[i][0][2] = Tab_P_Q[i][0][1];

		Tab_P_Q_mix_conv_1_1_h[i][2][1] = Tab_P_Q[i][0][0];
		Tab_P_Q_mix_conv_1_1_h[i][2][2] = Tab_P_Q[i][0][1];

		Tab_P_Q_mix_conv_1_1_h[i][1][0] = Tab_P_Q[i][0][0];
		Tab_P_Q_mix_conv_1_1_h[i][1][3] = Tab_P_Q[i][0][1];

		Tab_P_Q_mix_conv_1_1_h[i][1][1] = (Tab_P_Q[i][0][0] << 2) + Tab_P_Q[i][0][1];
		Tab_P_Q_mix_conv_1_1_h[i][1][2] = (Tab_P_Q[i][0][1] << 2) + Tab_P_Q[i][0][0];
	}


#endif

//Hoi Ming TBB
#ifdef _TBB_SUPPORT

	/*SYSTEM_INFO lpSystemInfo;

	GetSystemInfo(&lpSystemInfo);

	m_num_cores = max(1, lpSystemInfo.dwNumberOfProcessors);
	if(m_sh < 480)
		m_step = -1;
	else
		m_step = m_sh/m_num_cores;*/

	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	DWORD logicalProcessorCount = 0;
	DWORD processorCoreCount = 0;
	DWORD byteOffset = 0;

	DWORD rlen = 0;
	int rc = GetLogicalProcessorInformation(NULL, &rlen);
	buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(rlen);
	rc = GetLogicalProcessorInformation(buffer, &rlen);

	ptr = buffer;

	if( rc == FALSE){
		m_num_cores = -1;
		m_step = -1;
	}
	else{
		while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= rlen) 
		{
			switch (ptr->Relationship) 
			{

			case RelationProcessorCore:
				processorCoreCount++;

				logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
				break;

			default:
				break;
			}
			byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			ptr++;
		}

		m_num_cores = logicalProcessorCount;

		if(m_sh < 480 || m_num_cores == 1)
			m_step = -1;
		else{
			m_step = m_sh/m_num_cores;
			m_step = max(m_step, 150);
		}

	}

	if(buffer)
		free(buffer);

	/*{
		FILE * temp = fopen("C:\\temp\\hm_xoe.txt", "a");
		fprintf(temp, "m_num_cores: %d\n", (int)m_num_cores);
		fclose(temp);
	}*/

#else
	m_step = -1;
	m_num_cores = -1;
#endif
//end

	//Hoi Ming, For low-pass filter
	SourceBackup = (PBYTE)malloc(m_sw*m_sh*3);
	//end


	//////////////////
	// vos 4249 , [jerry:2010-11-09]
	m_bIsNeedCopy = TRUE;
	m_pSourceBuffer = NULL;

	/*if(m_x_ratio > 3.5 || m_y_ratio > 3.5){
		m_bIsNeedCopy = TRUE;
	}
	else if(m_x_ratio > 1.5 || m_y_ratio > 1.5){
		m_bIsNeedCopy = TRUE;
	}*/
	if( m_bIsNeedCopy ) //Alwasy copy the source. 
	{
		m_pSourceBuffer = (PBYTE)malloc( m_sw*m_sh*3 );
		if( !m_pSourceBuffer )
		{
			m_bIsNeedCopy = FALSE; //
		}
	}
	//}
	///////////////

	for(int i = 0; i < 2048; i++ )
	{
		j = i - 1024;

		if(j < lumalow)
		{	clip_tab[1][i] = (BYTE)lumalow;
		}

		else if(j< lumahigh )
		{	clip_tab[1][i] = (BYTE)i;
		}
		else
		{
			clip_tab[1][i] = (BYTE)lumahigh;
		}

		if( j < 0 )
		{
            
			clip_tab[0][i] = 0;
		}
	    else
	    if( j < 256 )
		{
            
			clip_tab[0][i] = (BYTE)i;
		}
		else
		{
			clip_tab[0][i] = 255;
		}




		 
		if(j <  chromalow)
		{
			
			clip_tab[2][i] = (BYTE)chromalow;
			
		}
		else if(j< ( chromahigh+1))
		{
			clip_tab[2][i] = (BYTE)i;
		}
		else
		{
			clip_tab[2][i] = (BYTE)chromahigh;
		}
	}
};

CBilinearResampler::~CBilinearResampler()
{

	//Hoi Ming
	if(SourceBackup)
		free(SourceBackup);
	//end

	//////////////
	//{  [jerry:2010-11-09]
	if( m_pSourceBuffer )
	{
		free(m_pSourceBuffer );
		m_pSourceBuffer = NULL;
	}
	//}
	/////////

};


#define BILINEAR_LOWPASS_PREC ( (double)(1<<15) )
#define BILINEAR_LOWPASS_SHIFT 15
#define PI 3.1415926535897932384626433832795

static void compute_lowpass_coeffs(int diam, double sigma, double tmp[8][8])
{
	int i,j,x,y,d_2;
	double sum;
	double ct0;
	double ct1;

	ct0=2*sigma*sigma;
	ct1=1.0/(PI*ct0);
	d_2=diam/2;
	sum=0.0;

	for(i=0;i<diam;i++)
	{	
		y=i-d_2;
		x=-d_2;
		
		for(j=0;j<diam;j++)
		{
			tmp[i][j]=ct1*exp(-((x*x+y*y)/ct0));
			
			
			sum+=tmp[i][j];
			x++;
		}
	}
	
	for(i=0;i<diam;i++)
		for(j=0;j<diam;j++)
			tmp[i][j]/=sum;
};


void CBilinearResampler::SetupLowpassKernels()
{
	double tmp[8][8];
	int i,j;
	double sigma;
	
	//3x3
	sigma=1.0;
	compute_lowpass_coeffs(3,sigma,tmp);
	memset(m_LowpassKernel_3,0,sizeof(m_LowpassKernel_3));
	for(i=0;i<2;i++)
		for(j=0;j<3;j++)
		{
			double v=tmp[i][j];
			m_LowpassKernel_3[i][j]=(unsigned short)(v*BILINEAR_LOWPASS_PREC);
		}

	
	memset(m_LowpassKernel_5,0,sizeof(m_LowpassKernel_5));
	
	//5x5
	sigma=1.45;
	compute_lowpass_coeffs(5,sigma,tmp);
	for(i=0;i<3;i++)
		for(j=0;j<4;j++)
		{
			double v=tmp[i][j];
			m_LowpassKernel_5[i][j]=(unsigned short)(v*BILINEAR_LOWPASS_PREC);
		}

	memset(m_LowpassKernel_7,0,sizeof(m_LowpassKernel_7));
	//7x7
	sigma=3.0;
	compute_lowpass_coeffs(7,sigma,tmp);
	for(i=0;i<4;i++)
	{
		for(j=0;j<7;j++)
		{
			double v=tmp[i][j];
			m_LowpassKernel_7[i][j]=(unsigned short)(v*BILINEAR_LOWPASS_PREC);
		}
		m_LowpassKernel_7[i][7]=0;
	}
};


//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------
void CBilinearResampler::Resample(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	switch(m_action	)
	{
		case BILINEAR:
				resample_bilinear(pSource, nSourceStride, pDest, nDestStride, clipindex);
				break;
		case BILINEAR_3:
				resample_lowpass_3x3(pSource, nSourceStride, pDest, nDestStride, clipindex);
				break;
		case BILINEAR_5:
				resample_lowpass_5x5(pSource, nSourceStride, pDest, nDestStride, clipindex);
				break;
		case BILINEAR_7:
				resample_lowpass_7x7(pSource, nSourceStride, pDest, nDestStride, clipindex);
				break;
		default:
				break;
	}
}

void CBilinearResampler::ResampleRGB24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{

	if( m_bIsNeedCopy )
	{
		CopyRGB( m_pSourceBuffer, pSource, nSourceStride );
		pSource = m_pSourceBuffer;
		nSourceStride = m_sw*3;
	}

	switch(m_action	)
	{
	case BILINEAR:
		resample_bilinear_24(pSource, nSourceStride, pDest, nDestStride, clipindex);
		break;
	case BILINEAR_3:
		resample_lowpass_3x3_24(pSource, nSourceStride, pDest, nDestStride, clipindex);
		break;
	case BILINEAR_5:
		resample_lowpass_5x5_24(pSource, nSourceStride, pDest, nDestStride, clipindex);
		break;
	case BILINEAR_7:
		resample_lowpass_7x7_24(pSource, nSourceStride, pDest, nDestStride, clipindex);
		break;
	default:
		break;
	}
}

//Hoi Ming YUV resizer
void CBilinearResampler::ResampleYUV420(PBYTE pSourceY, PBYTE pSourceU, PBYTE pSourceV, int nSourceStride, 
										PBYTE pDestY, PBYTE pDestU, PBYTE pDestV, int nDestStride, int clipindex)
{

	if(m_sw > m_dw*1.5 || m_sh > m_dh*1.5)
		resample_bilinear_yuv420_downsample(pSourceY, pSourceU, pSourceV, nSourceStride, pDestY, pDestU, pDestV, nDestStride,clipindex);
	else{
	if( m_bIsNeedCopy )
	{
		CopyYUV420( m_pSourceBuffer, pSourceY, pSourceU, pSourceV, nSourceStride  );		
		pSourceY = m_pSourceBuffer;
		pSourceU = m_pSourceBuffer + m_sw*m_sh;
		pSourceV = pSourceU + m_sw*m_sh/4;
		nSourceStride = m_sw;
	}

	resample_bilinear_yuv420(pSourceY, pSourceU, pSourceV, nSourceStride, pDestY, pDestU, pDestV, nDestStride, clipindex);
	}



}
//end

//#define __MMX_RESAMPLER__

#ifndef __MMX_RESAMPLER__
//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------
//Hoi Ming
#ifdef EDGE_PRESERVING_FILTER

#define EPF_V2

#ifdef EPF_V2

//Hoi Ming X64
#define _X64_FILTER
//end

//Hoi Ming TBB
#ifdef _TBB_SUPPORT
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/task_scheduler_init.h"
using namespace tbb;
using namespace std;
#endif
//end

#define SSE2_PROCESS_2_2(reg, acc, shift)	reg = _mm_unpacklo_epi8(reg, zero); reg = _mm_slli_epi16(reg, shift); acc = _mm_add_epi16(acc, reg);

#define SSE2_PROCESS_2_2_no_shift(reg, acc)	reg = _mm_unpacklo_epi8(reg, zero); acc = _mm_add_epi16(acc, reg);

//Hoi Ming YUV resizer
void CBilinearResampler::LowPassFilter_SingleColour_2_2(PBYTE pSource, int nSourceStride, int sw, int sh){

	int i, j;
	PBYTE pSourcePtr = pSource + 2*nSourceStride + 2;
	PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 2;
	int nSourceStrideX2 = nSourceStride*2;

	/*
	LPF : 
	0 1 1 1 0
	1 2 2 2 1
	1 2 4 2 1
	1 2 2 2 1
	0 1 1 1 0
	*/
	for( j = 2; j < (sh-4); j+=2){

		const unsigned char* pSourcePtr_cur = pSourcePtr-2;
		unsigned char* FilterPixelPtr_cur = FilterPixelPtr;
		
		for( i = 2; i < (sw-2-8); i+=8){
			__declspec(align(16)) __m128i r0, r1, r2, r3, r4, r5, zero, Acc0, temp0;
			r0 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStrideX2));				//13 12 .... 3 2 1 0 -1 -2
			r1 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStride));					//13 12 .... 3 2 1 0 -1 -2
			r2 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur));								//13 12 .... 3 2 1 0 -1 -2
			r3 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStride));					//13 12 .... 3 2 1 0 -1 -2
			r4 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2));				//13 12 .... 3 2 1 0 -1 -2
			r5 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStride));	//13 12 .... 3 2 1 0 -1 -2

			//First line
			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
			temp0 = _mm_srli_si128(r2, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

			temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)
				temp0 = _mm_srli_si128(r1, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3-nSourceStride]<<1)

			temp0 = _mm_srli_si128(r1, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3-nSourceStride]<<1)

			temp0 = _mm_srli_si128(r3, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3+nSourceStride]<<1)

			temp0 = _mm_srli_si128(r3, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3+nSourceStride]<<1)

			temp0 = _mm_srli_si128(r0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3-nSourceStrideX2])
			temp0 = _mm_srli_si128(r0, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-nSourceStrideX2])

			temp0 = _mm_srli_si128(r0, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3-nSourceStrideX2])

			temp0 = _mm_srli_si128(r4, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3+nSourceStrideX2])
			temp0 = _mm_srli_si128(r4, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+nSourceStrideX2])

			temp0 = _mm_srli_si128(r4, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3+nSourceStrideX2])

			temp0 = r1;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6-nSourceStride])
			temp0 = r2;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6])
			temp0 = r3;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6+nSourceStride])

			temp0 = _mm_srli_si128(r1, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6-nSourceStride])

			temp0 = _mm_srli_si128(r2, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6])

			temp0 = _mm_srli_si128(r3, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6+nSourceStride])

			zero = _mm_set1_epi16(16);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 5);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)FilterPixelPtr_cur, Acc0);
			//end

			//second line
			r0 = r1;
			r1 = r2;
			r2 = r3;
			r3 = r4;
			r4 = r5;

			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
			temp0 = _mm_srli_si128(r2, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

			temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)
				temp0 = _mm_srli_si128(r1, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3-nSourceStride]<<1)

			temp0 = _mm_srli_si128(r1, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3-nSourceStride]<<1)

			temp0 = _mm_srli_si128(r3, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3+nSourceStride]<<1)

			temp0 = _mm_srli_si128(r3, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3+nSourceStride]<<1)

			temp0 = _mm_srli_si128(r0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3-nSourceStrideX2])
			temp0 = _mm_srli_si128(r0, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-nSourceStrideX2])

			temp0 = _mm_srli_si128(r0, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3-nSourceStrideX2])

			temp0 = _mm_srli_si128(r4, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3+nSourceStrideX2])
			temp0 = _mm_srli_si128(r4, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+nSourceStrideX2])

			temp0 = _mm_srli_si128(r4, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3+nSourceStrideX2])

			temp0 = r1;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6-nSourceStride])
			temp0 = r2;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6])
			temp0 = r3;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6+nSourceStride])

			temp0 = _mm_srli_si128(r1, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6-nSourceStride])

			temp0 = _mm_srli_si128(r2, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6])

			temp0 = _mm_srli_si128(r3, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6+nSourceStride])

			zero = _mm_set1_epi16(16);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 5);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStride), Acc0);
			//end

			pSourcePtr_cur+=8;
			FilterPixelPtr_cur+=8;

		}
		pSourcePtr_cur += 2;
		for(; i < (sw-2); i++){
			FilterPixelPtr_cur[0] = 
				( (pSourcePtr_cur[0]<<2)
				+ (pSourcePtr_cur[0-1]<<1) + (pSourcePtr_cur[0+1]<<1)
				+ (pSourcePtr_cur[0-nSourceStride]<<1) + (pSourcePtr_cur[0+nSourceStride]<<1)
				+ (pSourcePtr_cur[0-1-nSourceStride]<<1) + (pSourcePtr_cur[0+1-nSourceStride]<<1)
				+ (pSourcePtr_cur[0-1+nSourceStride]<<1) + (pSourcePtr_cur[0+1+nSourceStride]<<1)
				+ pSourcePtr_cur[0-1-nSourceStrideX2] + pSourcePtr_cur[0-nSourceStrideX2] + pSourcePtr_cur[0+1-nSourceStrideX2]
			+ pSourcePtr_cur[0-1+nSourceStrideX2] + pSourcePtr_cur[0+nSourceStrideX2] + pSourcePtr_cur[0+1+nSourceStrideX2]
			+ pSourcePtr_cur[0-2-nSourceStride] + pSourcePtr_cur[0-2] + pSourcePtr_cur[0-2+nSourceStride]
			+ pSourcePtr_cur[0+2-nSourceStride] + pSourcePtr_cur[0+2] + pSourcePtr_cur[0+2+nSourceStride]
			+ 16 ) >> 5;
			FilterPixelPtr_cur[0+nSourceStride] = 
				( (pSourcePtr_cur[0+nSourceStride]<<2)
				+ (pSourcePtr_cur[0+nSourceStride-1]<<1) + (pSourcePtr_cur[0+nSourceStride+1]<<1)
				+ (pSourcePtr_cur[0+nSourceStride-nSourceStride]<<1) + (pSourcePtr_cur[0+nSourceStride+nSourceStride]<<1)
				+ (pSourcePtr_cur[0+nSourceStride-1-nSourceStride]<<1) + (pSourcePtr_cur[0+nSourceStride+1-nSourceStride]<<1)
				+ (pSourcePtr_cur[0+nSourceStride-1+nSourceStride]<<1) + (pSourcePtr_cur[0+nSourceStride+1+nSourceStride]<<1)
				+ pSourcePtr_cur[0+nSourceStride-1-nSourceStrideX2] + pSourcePtr_cur[0+nSourceStride-nSourceStrideX2] + pSourcePtr_cur[0+nSourceStride+1-nSourceStrideX2]
			+ pSourcePtr_cur[0+nSourceStride-1+nSourceStrideX2] + pSourcePtr_cur[0+nSourceStride+nSourceStrideX2] + pSourcePtr_cur[0+nSourceStride+1+nSourceStrideX2]
			+ pSourcePtr_cur[0+nSourceStride-2-nSourceStride] + pSourcePtr_cur[0+nSourceStride-2] + pSourcePtr_cur[0+nSourceStride-2+nSourceStride]
			+ pSourcePtr_cur[0+nSourceStride+2-nSourceStride] + pSourcePtr_cur[0+nSourceStride+2] + pSourcePtr_cur[0+nSourceStride+2+nSourceStride]
			+ 16 ) >> 5;
			pSourcePtr_cur++;
			FilterPixelPtr_cur++;
		}
		pSourcePtr+=nSourceStrideX2;
		FilterPixelPtr+=nSourceStrideX2;

	}
}


void CBilinearResampler::LowPassFilter_SingleColour_1_1(PBYTE pSource, int nSourceStride, int sw, int sh){
	/*
	LPF : 
	0 1 0
	1 4 1
	0 1 0
	*/
	int i, j;
	PBYTE pSourcePtr = pSource + 2*nSourceStride + 2;
	PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 2;
	int nSourceStrideX2 = 2*nSourceStride;
	__declspec(align(16)) __m128i zero, const_4;
	zero = _mm_setzero_si128();
	const_4 = _mm_set1_epi16(4);

	for( j = 2; j < (sh-4); j+=2){

		const unsigned char* pSourcePtr_cur = pSourcePtr-2;
		unsigned char* FilterPixelPtr_cur = FilterPixelPtr;

		for( i = 2; i < (sw-2-8); i+=8){

			__declspec(align(16)) __m128i r1, r2, r3, r4, temp0, Acc0;

			r1 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStride));			//13 12 .... 2 1 0 -1 -2
			r2 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur));						//13 12 .... 2 1 0 -1 -2
			r3 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStride));			//13 12 .... 2 1 0 -1 -2
			r4 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2));		//13 12 .... 2 1 0 -1 -2
			//r5 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStride));	//13 12 .... 2 1 0 -1 -2
			//r6 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStrideX2));	//13 12 .... 2 1 0 -1 -2


			//first line
			
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
				temp0 = _mm_srli_si128(r2, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

			temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

				
			Acc0 = _mm_add_epi16(Acc0, const_4);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, const_4);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur), Acc0);
			//end

			//second line
			r1 = r2;
			r2 = r3;
			r3 = r4;

			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
				temp0 = _mm_srli_si128(r2, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

			temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

			Acc0 = _mm_add_epi16(Acc0, const_4);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, const_4);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStride), Acc0);
			//end


			pSourcePtr_cur += 8;
			FilterPixelPtr_cur += 8;
		}
		pSourcePtr_cur += 2;
		for( ; i < (sw-2); i++){
			
			FilterPixelPtr_cur[0] = 
				( (pSourcePtr_cur[0]<<2)
				+ (pSourcePtr_cur[0-1]) + (pSourcePtr_cur[0+1])
				+ (pSourcePtr_cur[0-nSourceStride]) + (pSourcePtr_cur[0+nSourceStride])
				+ 4 ) >> 3;
			FilterPixelPtr_cur[0+nSourceStride] = 
				( (pSourcePtr_cur[0+nSourceStride]<<2)
				+ (pSourcePtr_cur[0+nSourceStride-1]) + (pSourcePtr_cur[0+nSourceStride+1])
				+ (pSourcePtr_cur[0+nSourceStride-nSourceStride]) + (pSourcePtr_cur[0+nSourceStride+nSourceStride])
				+ 4 ) >> 3;

			pSourcePtr_cur++;
			FilterPixelPtr_cur++;

		}
		pSourcePtr+=nSourceStrideX2;
		FilterPixelPtr+=nSourceStrideX2;
	}

	/*int i, j;
	PBYTE pSourcePtr = pSource + 2*nSourceStride + 2;
	PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 2;
	int nSourceStrideX2 = 2*nSourceStride;

	for( j = 2; j < (sh-2); j+=4){

		const unsigned char* pSourcePtr_cur = pSourcePtr-2;
		unsigned char* FilterPixelPtr_cur = FilterPixelPtr-2;

		for( i = 2; i < (sw-2); i++){
			
			FilterPixelPtr_cur[2] = 
				( (pSourcePtr_cur[2]<<2)
				+ (pSourcePtr_cur[2-1]) + (pSourcePtr_cur[2+1])
				+ (pSourcePtr_cur[2-nSourceStride]) + (pSourcePtr_cur[2+nSourceStride])
				+ 4 ) >> 3;
			FilterPixelPtr_cur[2+nSourceStride] = 
				( (pSourcePtr_cur[2+nSourceStride]<<2)
				+ (pSourcePtr_cur[2+nSourceStride-1]) + (pSourcePtr_cur[2+nSourceStride+1])
				+ (pSourcePtr_cur[2+nSourceStride-nSourceStride]) + (pSourcePtr_cur[2+nSourceStride+nSourceStride])
				+ 4 ) >> 3;

			const unsigned char* pSourcePtr_cur_temp = pSourcePtr_cur + nSourceStrideX2;
			unsigned char* FilterPixelPtr_cur_temp = FilterPixelPtr_cur + nSourceStrideX2;

			FilterPixelPtr_cur_temp[2] = 
				( (pSourcePtr_cur_temp[2]<<2)
				+ (pSourcePtr_cur_temp[2-1]) + (pSourcePtr_cur_temp[2+1])
				+ (pSourcePtr_cur_temp[2-nSourceStride]) + (pSourcePtr_cur_temp[2+nSourceStride])
				+ 4 ) >> 3;
			FilterPixelPtr_cur_temp[2+nSourceStride] = 
				( (pSourcePtr_cur_temp[2+nSourceStride]<<2)
				+ (pSourcePtr_cur_temp[2+nSourceStride-1]) + (pSourcePtr_cur_temp[2+nSourceStride+1])
				+ (pSourcePtr_cur_temp[2+nSourceStride-nSourceStride]) + (pSourcePtr_cur_temp[2+nSourceStride+nSourceStride])
				+ 4 ) >> 3;

			pSourcePtr_cur++;
			FilterPixelPtr_cur++;

		}
		pSourcePtr+=nSourceStrideX2+nSourceStrideX2;
		FilterPixelPtr+=nSourceStrideX2+nSourceStrideX2;
	}*/
}

#ifdef _TBB_SUPPORT

class ImageFilterYUV22{

	PBYTE pSource;
	int nSourceStride;
	PBYTE SourceBackup;
	int nDestStride;
	int m_sw;
	int StartImage;
	int EndImage;

public:
	void operator() (const blocked_range<size_t>& r) const {

		int i, j;
		PBYTE pSourcePtr = pSource + 2*nSourceStride + 2;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 2;
		int nSourceStrideX2 = nSourceStride*2;

		int begin, end;

		begin =  r.begin();
		end = r.end() + (2-((r.end()-begin)%2));

	/*	if(end > EndImage){
			end -= 2;
			if(end != EndImage){
				memset((FilterPixelPtr+(end-StartImage)*nSourceStride), 0, nSourceStride*(EndImage-end));
			}
		}*/

		pSourcePtr += (begin-StartImage)*nSourceStride;
		FilterPixelPtr += (begin-StartImage)*nSourceStride;

		for ( size_t j = begin; j != end; j+=2 ) {

			//		for( j = 2; j < (m_sh-2); j+=2){

			const unsigned char* pSourcePtr_cur = pSourcePtr-2;
			unsigned char* FilterPixelPtr_cur = FilterPixelPtr;

			for( i = 2; i < (m_sw-2-8); i+=8){

				__declspec(align(16)) __m128i r0, r1, r2, r3, r4, r5, zero, Acc0, temp0;
			r0 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStrideX2));				//13 12 .... 3 2 1 0 -1 -2
			r1 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStride));					//13 12 .... 3 2 1 0 -1 -2
			r2 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur));								//13 12 .... 3 2 1 0 -1 -2
			r3 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStride));					//13 12 .... 3 2 1 0 -1 -2
			r4 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2));				//13 12 .... 3 2 1 0 -1 -2
			r5 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStride));	//13 12 .... 3 2 1 0 -1 -2

			//First line
			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
			temp0 = _mm_srli_si128(r2, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

			temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)
				temp0 = _mm_srli_si128(r1, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3-nSourceStride]<<1)

			temp0 = _mm_srli_si128(r1, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3-nSourceStride]<<1)

			temp0 = _mm_srli_si128(r3, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3+nSourceStride]<<1)

			temp0 = _mm_srli_si128(r3, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3+nSourceStride]<<1)

			temp0 = _mm_srli_si128(r0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3-nSourceStrideX2])
			temp0 = _mm_srli_si128(r0, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-nSourceStrideX2])

			temp0 = _mm_srli_si128(r0, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3-nSourceStrideX2])

			temp0 = _mm_srli_si128(r4, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3+nSourceStrideX2])
			temp0 = _mm_srli_si128(r4, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+nSourceStrideX2])

			temp0 = _mm_srli_si128(r4, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3+nSourceStrideX2])

			temp0 = r1;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6-nSourceStride])
			temp0 = r2;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6])
			temp0 = r3;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6+nSourceStride])

			temp0 = _mm_srli_si128(r1, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6-nSourceStride])

			temp0 = _mm_srli_si128(r2, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6])

			temp0 = _mm_srli_si128(r3, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6+nSourceStride])

			zero = _mm_set1_epi16(16);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 5);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)FilterPixelPtr_cur, Acc0);
			//end

			//second line
			r1 = r2;
			r2 = r3;
			r3 = r4;
			r4 = r5;

			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
			temp0 = _mm_srli_si128(r2, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

			temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 2);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)
				temp0 = _mm_srli_si128(r1, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3-nSourceStride]<<1)

			temp0 = _mm_srli_si128(r1, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3-nSourceStride]<<1)

			temp0 = _mm_srli_si128(r3, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3+nSourceStride]<<1)

			temp0 = _mm_srli_si128(r3, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3+nSourceStride]<<1)

			temp0 = _mm_srli_si128(r0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3-nSourceStrideX2])
			temp0 = _mm_srli_si128(r0, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-nSourceStrideX2])

			temp0 = _mm_srli_si128(r0, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3-nSourceStrideX2])

			temp0 = _mm_srli_si128(r4, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3+nSourceStrideX2])
			temp0 = _mm_srli_si128(r4, 2);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+nSourceStrideX2])

			temp0 = _mm_srli_si128(r4, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3+nSourceStrideX2])

			temp0 = r1;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6-nSourceStride])
			temp0 = r2;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6])
			temp0 = r3;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6+nSourceStride])

			temp0 = _mm_srli_si128(r1, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6-nSourceStride])

			temp0 = _mm_srli_si128(r2, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6])

			temp0 = _mm_srli_si128(r3, 4);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6+nSourceStride])

			zero = _mm_set1_epi16(16);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 5);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStride), Acc0);
			//end

			pSourcePtr_cur+=8;
			FilterPixelPtr_cur+=8;

			}
			
			for(; i < (m_sw-2); i++){
				FilterPixelPtr_cur[0] = 
					( (pSourcePtr_cur[0]<<2)
					+ (pSourcePtr_cur[0-1]<<1) + (pSourcePtr_cur[0+1]<<1)
					+ (pSourcePtr_cur[0-nSourceStride]<<1) + (pSourcePtr_cur[0+nSourceStride]<<1)
					+ (pSourcePtr_cur[0-1-nSourceStride]<<1) + (pSourcePtr_cur[0+1-nSourceStride]<<1)
					+ (pSourcePtr_cur[0-1+nSourceStride]<<1) + (pSourcePtr_cur[0+1+nSourceStride]<<1)
					+ pSourcePtr_cur[0-1-nSourceStrideX2] + pSourcePtr_cur[0-nSourceStrideX2] + pSourcePtr_cur[0+1-nSourceStrideX2]
				+ pSourcePtr_cur[0-1+nSourceStrideX2] + pSourcePtr_cur[0+nSourceStrideX2] + pSourcePtr_cur[0+1+nSourceStrideX2]
				+ pSourcePtr_cur[0-2-nSourceStride] + pSourcePtr_cur[0-2] + pSourcePtr_cur[0-2+nSourceStride]
				+ pSourcePtr_cur[0+2-nSourceStride] + pSourcePtr_cur[0+2] + pSourcePtr_cur[0+2+nSourceStride]
				+ 16 ) >> 5;
				FilterPixelPtr_cur[0+nSourceStride] = 
					( (pSourcePtr_cur[0+nSourceStride]<<2)
					+ (pSourcePtr_cur[0+nSourceStride-1]<<1) + (pSourcePtr_cur[0+nSourceStride+1]<<1)
					+ (pSourcePtr_cur[0+nSourceStride-nSourceStride]<<1) + (pSourcePtr_cur[0+nSourceStride+nSourceStride]<<1)
					+ (pSourcePtr_cur[0+nSourceStride-1-nSourceStride]<<1) + (pSourcePtr_cur[0+nSourceStride+1-nSourceStride]<<1)
					+ (pSourcePtr_cur[0+nSourceStride-1+nSourceStride]<<1) + (pSourcePtr_cur[0+nSourceStride+1+nSourceStride]<<1)
					+ pSourcePtr_cur[0+nSourceStride-1-nSourceStrideX2] + pSourcePtr_cur[0+nSourceStride-nSourceStrideX2] + pSourcePtr_cur[0+nSourceStride+1-nSourceStrideX2]
				+ pSourcePtr_cur[0+nSourceStride-1+nSourceStrideX2] + pSourcePtr_cur[0+nSourceStride+nSourceStrideX2] + pSourcePtr_cur[0+nSourceStride+1+nSourceStrideX2]
				+ pSourcePtr_cur[0+nSourceStride-2-nSourceStride] + pSourcePtr_cur[0+nSourceStride-2] + pSourcePtr_cur[0+nSourceStride-2+nSourceStride]
				+ pSourcePtr_cur[0+nSourceStride+2-nSourceStride] + pSourcePtr_cur[0+nSourceStride+2] + pSourcePtr_cur[0+nSourceStride+2+nSourceStride]
				+ 16 ) >> 5;
				pSourcePtr_cur++;
				FilterPixelPtr_cur++;
			}

			pSourcePtr+=nSourceStrideX2;
			FilterPixelPtr+=nSourceStrideX2;

		}


	}
	ImageFilterYUV22(PBYTE inSource, int SourceStride, PBYTE inSourceBackup, int DestStride, 
		int in_m_sw, int inStart, int inEnd) :
	pSource(inSource), nSourceStride(SourceStride), SourceBackup(inSourceBackup), nDestStride(DestStride), 
	m_sw(in_m_sw), StartImage(inStart), EndImage(inEnd) {}
};

class ImageFilterYUV11{

	PBYTE pSource;
	int nSourceStride;
	PBYTE SourceBackup;
	int nDestStride;
	int m_sw;
	int StartImage;
	int EndImage;

public:
	void operator() (const blocked_range<size_t>& r) const {

		int i, j;
		PBYTE pSourcePtr = pSource + 2*nSourceStride + 2;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 2;
		int nSourceStrideX2 = nSourceStride*2;

		//int begin = r.begin();
		/*int begin = (r.begin()>>2)<<2;
		int end = ((r.end()+2)>>2)<<2;

		begin =  r.begin();

		end = r.end() + (2-((r.end()-begin)%2));
		
		if(end > EndImage){
			end -= 4;
			if(end != EndImage){
				memset((FilterPixelPtr+(end-StartImage)*nSourceStride), 0, nSourceStride*(EndImage-end));
			}
		}
		*/

		int begin =  r.begin();
		int end = r.end() + (2-((r.end()-begin)%2));
		
		/*if(end > EndImage){
			end -= 2;
			if(end != EndImage){
				memset((FilterPixelPtr+(end-StartImage)*nSourceStride), 0, nSourceStride*(EndImage-end));
			}
		}*/

		pSourcePtr += (begin-StartImage)*nSourceStride;
		FilterPixelPtr += (begin-StartImage)*nSourceStride;

		for ( size_t j = begin; j != end; j+=2 ) {

			//		for( j = 2; j < (m_sh-2); j+=2){

			const unsigned char* pSourcePtr_cur = pSourcePtr-2;
			unsigned char* FilterPixelPtr_cur = FilterPixelPtr;


			for( i = 2; i < (m_sw-2-8); i+=8){

				__declspec(align(16)) __m128i r1, r2, r3, r4, temp0, Acc0, zero;

				r1 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStride));			//13 12 .... 2 1 0 -1 -2
				r2 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur));						//13 12 .... 2 1 0 -1 -2
				r3 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStride));			//13 12 .... 2 1 0 -1 -2
				r4 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2));		//13 12 .... 2 1 0 -1 -2
				//r5 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStride));	//13 12 .... 2 1 0 -1 -2
				//r6 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStrideX2));	//13 12 .... 2 1 0 -1 -2


				//first line
				zero = _mm_setzero_si128();
				Acc0 = _mm_setzero_si128();

				temp0 = _mm_srli_si128(r2, 2);
				SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
					temp0 = _mm_srli_si128(r2, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

					temp0 = _mm_srli_si128(r2, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

					temp0 = _mm_srli_si128(r1, 2);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
					temp0 = _mm_srli_si128(r3, 2);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

					zero = _mm_set1_epi16(4);
				Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
				Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
				Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

				_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur), Acc0);
				//end

				//second line
				r1 = r2;
				r2 = r3;
				r3 = r4;

				zero = _mm_setzero_si128();
				Acc0 = _mm_setzero_si128();

				temp0 = _mm_srli_si128(r2, 2);
				SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
					temp0 = _mm_srli_si128(r2, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

					temp0 = _mm_srli_si128(r2, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

					temp0 = _mm_srli_si128(r1, 2);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
					temp0 = _mm_srli_si128(r3, 2);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

					zero = _mm_set1_epi16(4);
				Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
				Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
				Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

				_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStride), Acc0);
				//end

				pSourcePtr_cur += 8;
				FilterPixelPtr_cur += 8;
			}
			for( ; i < (m_sw-2); i++){

				FilterPixelPtr_cur[0] = 
					( (pSourcePtr_cur[0]<<2)
					+ (pSourcePtr_cur[0-3]) + (pSourcePtr_cur[0+1])
					+ (pSourcePtr_cur[0-nSourceStride]) + (pSourcePtr_cur[0+nSourceStride])
					+ 4 ) >> 3;
				FilterPixelPtr_cur[0+nSourceStride] = 
					( (pSourcePtr_cur[0+nSourceStride]<<2)
					+ (pSourcePtr_cur[0+nSourceStride-1]) + (pSourcePtr_cur[0+nSourceStride+3])
					+ (pSourcePtr_cur[0+nSourceStride-nSourceStride]) + (pSourcePtr_cur[0+nSourceStride+nSourceStride])
					+ 4 ) >> 3;

				pSourcePtr_cur++;
				FilterPixelPtr_cur++;

			}
			pSourcePtr+=nSourceStrideX2;
			FilterPixelPtr+=nSourceStrideX2;
		}

	}
	ImageFilterYUV11(PBYTE inSource, int SourceStride, PBYTE inSourceBackup, int DestStride, 
		int in_m_sw, int inStart, int inEnd) :
	pSource(inSource), nSourceStride(SourceStride), SourceBackup(inSourceBackup), nDestStride(DestStride), 
		m_sw(in_m_sw), StartImage(inStart), EndImage(inEnd) {}
};



class ImageFilterYUVInterpolation{

	PBYTE pSource;
	int nSourceStride;
	PBYTE pDest;
	int nDestStride;
	int m_sw;
	int m_dw;
	int StartImage;
	int EndImage;
	int m_w_inc;
	int m_h_inc;
	PBYTE pClipTab;
	short SincFilter[7][4];
	int image;

public:
	void operator() (const blocked_range<size_t>& r) const {

		int i, j;
		PBYTE pSourcePtr = pSource;
		PBYTE pDest_ptr = pDest;

		int begin =  r.begin();
		//int end = r.end() + (2-((r.end()-begin)%2));

		int end = min(r.end(), EndImage);

		pSourcePtr += (begin-StartImage)*nSourceStride;
		pDest_ptr += (begin-StartImage)*nDestStride;

		/*{
			FILE* temp = fopen("c:\\temp\\resizer_test.txt", "a");
			fprintf(temp, "ImageFilterLumaInterpolation:	%d	%d	%d	%d\n", begin, end, StartImage, EndImage);
			fclose(temp);
		}*/
		//for ( size_t j = begin; j != end; j+=2 ) {

		int InterData[3][6];

		unsigned int y_accum = m_h_inc*(begin-StartImage);
		for (size_t vy = begin; vy != end; vy++)  
		{
			BYTE *s1, *s2, *sTemp;
			int q;
			int ry;

			ry = (y_accum>>BILINEAR_PREC);

			//upper line 
			s1 = pSource + ry * nSourceStride;
			if(ry + 1 < EndImage)
			{
				//lower line
				s2 = s1 + nSourceStride;
			}
			else
				s2 = s1;
			unsigned int x_accum = 0;
			q = y_accum & BILINEAR_MASK;

			int y_sinc_index = (q>>11);

			if(y_sinc_index == 0){
				y_sinc_index = -1;
			}
			else if(y_sinc_index == 15){
				y_sinc_index = -2;
			}
			else{
				y_sinc_index = (y_sinc_index-1)>>1;
			}

			for (int vx = 0; vx < m_dw; vx++)
			{
				BYTE res[3];
				int rx;
				int a[3], b[3], c[3], d[3];
				int p;

				rx = x_accum>>BILINEAR_PREC;
				p = x_accum & BILINEAR_MASK;

				int x_sinc_index = (p>>11);

				if(x_sinc_index <= 0){
					x_sinc_index = -1;
				}
				else if(x_sinc_index >= 15){
					x_sinc_index = -2;
				}
				else{
					x_sinc_index = (x_sinc_index-1)>>1;
				}

				__declspec(align(16)) __int32 InterDataRevDim[6][4];

				//__declspec(align(16)) __int32 InterDataRevDimTemp[6][4];

				if( image == 0 && (rx+2) < m_sw && (rx-1) >= 0 && (ry+2) < EndImage && (ry-1) >= 0)
				{
					//Horizontal Filtering
					if( x_sinc_index >= 0 ){
						sTemp = s1 - nSourceStride;
						for(int filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
							InterData[0][filterIdx1] = 0;
							int src_pos = -1;
							for( int filterIdx2 = 0; filterIdx2 < 4; filterIdx2++){
								InterData[0][filterIdx1] 
								+= sTemp[rx+src_pos]*SincFilter[x_sinc_index][filterIdx2];
								src_pos += 1;
							}
							InterData[0][filterIdx1] = (InterData[0][filterIdx1] + 256) >> 9;

							sTemp += nSourceStride;
						}
					}
					else{
						if(x_sinc_index == -1){
							sTemp = s1 - nSourceStride;
							for(int filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
								InterData[0][filterIdx1] = sTemp[rx];
								sTemp += nSourceStride;
							}
						}
						else{
							sTemp = s1 - nSourceStride;
							for(int filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
								InterData[0][filterIdx1] = sTemp[rx+1];
								sTemp += nSourceStride;
							}
						}
					}
					//end
					//Vertical Filtering
					if( y_sinc_index >= 0){

						int acc[3];

						acc[0] = acc[1] = acc[2] = 0;

						for(int filterIdx2 = 0; filterIdx2 < 4; filterIdx2++){
							acc[0] += InterData[0][filterIdx2]*SincFilter[y_sinc_index][filterIdx2];
						}
						res[0] =  pClipTab[(acc[0] + 256) >> 9];
					}
					else{
						if(y_sinc_index == -1){
							res[0] = pClipTab[InterData[0][1]];
						}
						else{
							res[0] = pClipTab[InterData[0][2]];
						}	
					}
					//end

				}
				else
				{
					if((rx+1) < m_sw){
						a[0] = s1[rx]; b[0] = s1[rx+1];
						c[0] = s2[rx]; d[0] = s2[rx+1];
					}
					else{
						a[0] = b[0] = s1[rx];
						c[0] = d[0] = s2[rx];
					}
					int t0[3], t1[3];

					t0[0] = a[0] - ((p * (a[0]-b[0]) + (1<<14) ) >>BILINEAR_PREC);
					t1[0] = c[0] - ((p * (c[0]-d[0]) + (1<<14) ) >>BILINEAR_PREC);

					res[0] = (BYTE)(t0[0] - ((q*(t0[0]-t1[0]) + (1<<14))>>BILINEAR_PREC));
		

				}


				pDest_ptr[vx] = pClipTab[res[0]];

				x_accum += m_w_inc;

			}

			pDest_ptr += nDestStride;

			y_accum += m_h_inc;
		}


	}
	ImageFilterYUVInterpolation(PBYTE inSource, int SourceStride, PBYTE inpDest, int DestStride, 
		int in_m_sw, int in_m_dw, int in_m_w_inc, int in_m_h_inc, int inStart, int inEnd, 
		PBYTE in_pClipTab, short in_SincFilter[7][4], int in_image) :
	pSource(inSource), nSourceStride(SourceStride), pDest(inpDest), nDestStride(DestStride), 
		m_sw(in_m_sw), m_dw(in_m_dw), m_w_inc(in_m_w_inc), m_h_inc(in_m_h_inc), StartImage(inStart), EndImage(inEnd),
		pClipTab(in_pClipTab), image(in_image) {

			for( int j = 0; j < 7; j++){
				for( int i = 0; i < 4; i++){
					SincFilter[j][i] = in_SincFilter[j][i];
				}
			}
	}
};


#endif


//end

void CBilinearResampler::LowPassFilter_2_2(PBYTE pSource, int nSourceStride){

	int i, j;
	PBYTE pSourcePtr = pSource + 2*nSourceStride + 6;
	PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 6;
	int nSourceStrideX2 = nSourceStride*2;

	for( j = 2; j < (m_sh-4); j+=2){

		const unsigned char* pSourcePtr_cur = pSourcePtr-6;
		unsigned char* FilterPixelPtr_cur = FilterPixelPtr;

		for( i = 6; i < (m_sw*3-6-8); i+=8){

			__declspec(align(16)) __m128i r0, r1, r2, r3, r4, r5, temp0, Acc0, zero, temp1;
			__int32 r0_hi, r1_hi, r2_hi, r3_hi, r4_hi, r5_hi;

			r0 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStrideX2));		//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
			r1 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStride));			//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
			r2 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur));						//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
			r3 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStride));		//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
			r4 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
			r5 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStride));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
			r0_hi = *(__int32*)(pSourcePtr_cur+16-nSourceStrideX2);
			r1_hi = *(__int32*)(pSourcePtr_cur+16-nSourceStride);
			r2_hi = *(__int32*)(pSourcePtr_cur+16);
			r3_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStride);
			r4_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStrideX2);
			r5_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStrideX2+nSourceStride);

			//First line
			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
				temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

				temp1 = _mm_cvtsi32_si128(r2_hi);
			temp0 = _mm_srli_si128(r2, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)
				temp0 = _mm_srli_si128(r1, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3-nSourceStride]<<1)

				temp1 = _mm_cvtsi32_si128(r1_hi);
			temp0 = _mm_srli_si128(r1, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3-nSourceStride]<<1)

				temp0 = _mm_srli_si128(r3, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3+nSourceStride]<<1)

				temp1 = _mm_cvtsi32_si128(r3_hi);
			temp0 = _mm_srli_si128(r3, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3+nSourceStride]<<1)

				temp0 = _mm_srli_si128(r0, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3-nSourceStrideX2])
				temp0 = _mm_srli_si128(r0, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-nSourceStrideX2])

				temp1 = _mm_cvtsi32_si128(r0_hi);
			temp0 = _mm_srli_si128(r0, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3-nSourceStrideX2])

				temp0 = _mm_srli_si128(r4, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3+nSourceStrideX2])
				temp0 = _mm_srli_si128(r4, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+nSourceStrideX2])

				temp1 = _mm_cvtsi32_si128(r4_hi);
			temp0 = _mm_srli_si128(r4, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3+nSourceStrideX2])

				temp0 = r1;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6-nSourceStride])
				temp0 = r2;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6])
				temp0 = r3;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6+nSourceStride])

				temp1 = _mm_cvtsi32_si128(r1_hi);
			temp0 = _mm_srli_si128(r1, 12);
			temp0 = _mm_unpacklo_epi32(temp0, temp1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6-nSourceStride])

				temp1 = _mm_cvtsi32_si128(r2_hi);
			temp0 = _mm_srli_si128(r2, 12);
			temp0 = _mm_unpacklo_epi32(temp0, temp1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6])

				temp1 = _mm_cvtsi32_si128(r3_hi);
			temp0 = _mm_srli_si128(r3, 12);
			temp0 = _mm_unpacklo_epi32(temp0, temp1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6+nSourceStride])

				zero = _mm_set1_epi16(16);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 5);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)FilterPixelPtr_cur, Acc0);
			//end

			//second line
			r1 = r2;
			r2 = r3;
			r3 = r4;
			r4 = r5;
			r1_hi = r2_hi;
			r2_hi = r3_hi;
			r3_hi = r4_hi;
			r4_hi = r5_hi;

			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
				temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

				temp1 = _mm_cvtsi32_si128(r2_hi);
			temp0 = _mm_srli_si128(r2, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3]<<1)


				temp0 = _mm_srli_si128(r1, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)
				temp0 = _mm_srli_si128(r1, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3-nSourceStride]<<1)

				temp1 = _mm_cvtsi32_si128(r1_hi);
			temp0 = _mm_srli_si128(r1, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3-nSourceStride]<<1)

				temp0 = _mm_srli_si128(r3, 3);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3+nSourceStride]<<1)

				temp1 = _mm_cvtsi32_si128(r3_hi);
			temp0 = _mm_srli_si128(r3, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3+nSourceStride]<<1)

				temp0 = _mm_srli_si128(r0, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3-nSourceStrideX2])
				temp0 = _mm_srli_si128(r0, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-nSourceStrideX2])

				temp1 = _mm_cvtsi32_si128(r0_hi);
			temp0 = _mm_srli_si128(r0, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3-nSourceStrideX2])

				temp0 = _mm_srli_si128(r4, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3+nSourceStrideX2])
				temp0 = _mm_srli_si128(r4, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+nSourceStrideX2])

				temp1 = _mm_cvtsi32_si128(r4_hi);
			temp0 = _mm_srli_si128(r4, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3+nSourceStrideX2])

				temp0 = r1;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6-nSourceStride])
				temp0 = r2;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6])
				temp0 = r3;
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6+nSourceStride])

				temp1 = _mm_cvtsi32_si128(r1_hi);
			temp0 = _mm_srli_si128(r1, 12);
			temp0 = _mm_unpacklo_epi32(temp0, temp1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6-nSourceStride])

				temp1 = _mm_cvtsi32_si128(r2_hi);
			temp0 = _mm_srli_si128(r2, 12);
			temp0 = _mm_unpacklo_epi32(temp0, temp1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6])

				temp1 = _mm_cvtsi32_si128(r3_hi);
			temp0 = _mm_srli_si128(r3, 12);
			temp0 = _mm_unpacklo_epi32(temp0, temp1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6+nSourceStride])

				zero = _mm_set1_epi16(16);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 5);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStride), Acc0);
			//end

			pSourcePtr_cur += 8;
			FilterPixelPtr_cur += 8;

		}
		for( ; i < (m_sw*3-6); i++){
			FilterPixelPtr[0] = 
				( (pSourcePtr[0]<<2)
				+ (pSourcePtr[0-3]<<1) + (pSourcePtr[0+3]<<1)
				+ (pSourcePtr[0-nSourceStride]<<1) + (pSourcePtr[0+nSourceStride]<<1)
				+ (pSourcePtr[0-3-nSourceStride]<<1) + (pSourcePtr[0+3-nSourceStride]<<1)
				+ (pSourcePtr[0-3+nSourceStride]<<1) + (pSourcePtr[0+3+nSourceStride]<<1)
				+ pSourcePtr[0-3-nSourceStrideX2] + pSourcePtr[0-nSourceStrideX2] + pSourcePtr[0+3-nSourceStrideX2]
			+ pSourcePtr[0-3+nSourceStrideX2] + pSourcePtr[0+nSourceStrideX2] + pSourcePtr[0+3+nSourceStrideX2]
			+ pSourcePtr[0-6-nSourceStride] + pSourcePtr[0-6] + pSourcePtr[6-6+nSourceStride]
			+ pSourcePtr[0+6-nSourceStride] + pSourcePtr[0+6] + pSourcePtr[6+6+nSourceStride]
			+ 16 ) >> 5;
			FilterPixelPtr[0+nSourceStride] = 
				( (pSourcePtr[0+nSourceStride]<<2)
				+ (pSourcePtr[0+nSourceStride-3]<<1) + (pSourcePtr[0+nSourceStride+3]<<1)
				+ (pSourcePtr[0+nSourceStride-nSourceStride]<<1) + (pSourcePtr[0+nSourceStride+nSourceStride]<<1)
				+ (pSourcePtr[0+nSourceStride-3-nSourceStride]<<1) + (pSourcePtr[0+nSourceStride+3-nSourceStride]<<1)
				+ (pSourcePtr[0+nSourceStride-3+nSourceStride]<<1) + (pSourcePtr[0+nSourceStride+3+nSourceStride]<<1)
				+ pSourcePtr[0+nSourceStride-3-nSourceStrideX2] + pSourcePtr[0+nSourceStride-nSourceStrideX2] + pSourcePtr[0+nSourceStride+3-nSourceStrideX2]
			+ pSourcePtr[0+nSourceStride-3+nSourceStrideX2] + pSourcePtr[0+nSourceStride+nSourceStrideX2] + pSourcePtr[0+nSourceStride+3+nSourceStrideX2]
			+ pSourcePtr[0+nSourceStride-6-nSourceStride] + pSourcePtr[0+nSourceStride-6] + pSourcePtr[0+nSourceStride-6+nSourceStride]
			+ pSourcePtr[0+nSourceStride+6-nSourceStride] + pSourcePtr[0+nSourceStride+6] + pSourcePtr[0+nSourceStride+6+nSourceStride]
			+ 16 ) >> 5;
		}

		pSourcePtr+=nSourceStrideX2;
		FilterPixelPtr+=nSourceStrideX2;
	}
}


void CBilinearResampler::LowPassFilter_1_1(PBYTE pSource, int nSourceStride){
	int i, j;
	PBYTE pSourcePtr = pSource + 2*nSourceStride + 6;
	PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 6;
	int nSourceStrideX2 = 2*nSourceStride;

	for( j = 2; j < (m_sh-4); j+=4){

		const unsigned char* pSourcePtr_cur = pSourcePtr-6;
		unsigned char* FilterPixelPtr_cur = FilterPixelPtr;

		for( i = 6; i < (m_sw*3-6-8); i+=8){

			__declspec(align(16)) __m128i r1, r2, r3, r4, r5, r6, temp0, Acc0, zero, temp1;
			__int32 r2_hi, r3_hi, r4_hi, r5_hi;

			r1 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStride));			//9 ... 3 2 1 0 -1 -2 -3 -4 -5
			r2 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur));						//9 ... 3 2 1 0 -1 -2 -3 -4 -5
			r3 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStride));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5
			r4 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5
			r5 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStride));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5
			r6 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStrideX2));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5
			r2_hi = *(__int32*)(pSourcePtr_cur+16);
			r3_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStride);
			r4_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStrideX2);
			r5_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStrideX2+nSourceStride);

			//first line
			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
				temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

				temp1 = _mm_cvtsi32_si128(r2_hi);
			temp0 = _mm_srli_si128(r2, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

				zero = _mm_set1_epi16(4);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur), Acc0);
			//end

			//second line
			r1 = r2;
			r2 = r3;
			r3 = r4;
			r2_hi = r3_hi;

			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
				temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

				temp1 = _mm_cvtsi32_si128(r2_hi);
			temp0 = _mm_srli_si128(r2, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

				zero = _mm_set1_epi16(4);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStride), Acc0);
			//end

			//third line
			r1 = r3;
			r2 = r4;
			r3 = r5;
			r2_hi = r4_hi;

			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
				temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

				temp1 = _mm_cvtsi32_si128(r2_hi);
			temp0 = _mm_srli_si128(r2, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

				zero = _mm_set1_epi16(4);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStrideX2), Acc0);
			//end

			//forth line
			r1 = r4;
			r2 = r5;
			r3 = r6;
			r2_hi = r5_hi;

			zero = _mm_setzero_si128();
			Acc0 = _mm_setzero_si128();

			temp0 = _mm_srli_si128(r2, 6);
			SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
				temp0 = _mm_srli_si128(r2, 3);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

				temp1 = _mm_cvtsi32_si128(r2_hi);
			temp0 = _mm_srli_si128(r2, 8);
			temp0 = _mm_unpacklo_epi64(temp0, temp1);
			temp0 = _mm_srli_si128(temp0, 1);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

				temp0 = _mm_srli_si128(r1, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
				temp0 = _mm_srli_si128(r3, 6);
			SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

				zero = _mm_set1_epi16(4);
			Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
			Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
			Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

			_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStrideX2+nSourceStride), Acc0);
			//end

			pSourcePtr_cur += 8;
			FilterPixelPtr_cur += 8;
		}
		for( ; i < (m_sw*3-6); i++){
			
			FilterPixelPtr_cur[0] = 
				( (pSourcePtr_cur[0]<<2)
				+ (pSourcePtr_cur[0-3]) + (pSourcePtr_cur[0+3])
				+ (pSourcePtr_cur[0-nSourceStride]) + (pSourcePtr_cur[0+nSourceStride])
				+ 4 ) >> 3;
			FilterPixelPtr_cur[0+nSourceStride] = 
				( (pSourcePtr_cur[0+nSourceStride]<<2)
				+ (pSourcePtr_cur[0+nSourceStride-3]) + (pSourcePtr_cur[0+nSourceStride+3])
				+ (pSourcePtr_cur[0+nSourceStride-nSourceStride]) + (pSourcePtr_cur[0+nSourceStride+nSourceStride])
				+ 4 ) >> 3;

			const unsigned char* pSourcePtr_cur_temp = pSourcePtr_cur + nSourceStrideX2;
			unsigned char* FilterPixelPtr_cur_temp = FilterPixelPtr_cur + nSourceStrideX2;

			FilterPixelPtr_cur_temp[0] = 
				( (pSourcePtr_cur_temp[0]<<2)
				+ (pSourcePtr_cur_temp[0-3]) + (pSourcePtr_cur_temp[0+3])
				+ (pSourcePtr_cur_temp[0-nSourceStride]) + (pSourcePtr_cur_temp[0+nSourceStride])
				+ 4 ) >> 3;
			FilterPixelPtr_cur_temp[0+nSourceStride] = 
				( (pSourcePtr_cur_temp[0+nSourceStride]<<2)
				+ (pSourcePtr_cur_temp[0+nSourceStride-3]) + (pSourcePtr_cur_temp[0+nSourceStride+3])
				+ (pSourcePtr_cur_temp[0+nSourceStride-nSourceStride]) + (pSourcePtr_cur_temp[0+nSourceStride+nSourceStride])
				+ 4 ) >> 3;

			pSourcePtr_cur++;
			FilterPixelPtr_cur++;

		}
		pSourcePtr+=nSourceStrideX2+nSourceStrideX2;
		FilterPixelPtr+=nSourceStrideX2+nSourceStrideX2;
	}
}



//Hoi Ming TBB
#ifdef _TBB_SUPPORT

class ImageFilter22{

	PBYTE pSource;
	int nSourceStride;
	PBYTE SourceBackup;
	int nDestStride;
	int m_sw;
	int StartImage;
	int EndImage;

public:
	void operator() (const blocked_range<size_t>& r) const {

		int i, j;
		PBYTE pSourcePtr = pSource + 2*nSourceStride + 6;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 6;
		int nSourceStrideX2 = nSourceStride*2;

		int begin, end;
		//int begin = r.begin();
		/*int begin = (r.begin()>>1)<<1;
		int end = ((r.end()+1)>>1)<<1;*/

		begin =  r.begin();
		end = r.end() + (2-((r.end()-begin)%2));

		/*if(end > EndImage){
			end -= 2;
			if(end != EndImage){
				memset((FilterPixelPtr+(end-StartImage)*nSourceStride), 0, nSourceStride*(EndImage-end));
			}
		}*/

		pSourcePtr += (begin-StartImage)*nSourceStride;
		FilterPixelPtr += (begin-StartImage)*nSourceStride;

		/*{
			FILE * temp = fopen("C:\\temp\\xle_hm_test.txt", "a");
			fprintf(temp, "In Loop: %d	%d	%d	%d	%d\n", begin, end, StartImage, EndImage, m_sw);
			fclose(temp);
		}*/

		for ( size_t j = begin; j != end; j+=2 ) {

			//		for( j = 2; j < (m_sh-2); j+=2){

			const unsigned char* pSourcePtr_cur = pSourcePtr-6;
			unsigned char* FilterPixelPtr_cur = FilterPixelPtr;

			for( i = 6; i < (m_sw*3-6-8); i+=8){

				__declspec(align(16)) __m128i r0, r1, r2, r3, r4, r5, temp0, Acc0, zero, temp1;
				__int32 r0_hi, r1_hi, r2_hi, r3_hi, r4_hi, r5_hi;

				r0 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStrideX2));		//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
				r1 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStride));			//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
				r2 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur));						//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
				r3 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStride));		//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
				r4 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
				r5 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStride));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5 -6
				r0_hi = *(__int32*)(pSourcePtr_cur+16-nSourceStrideX2);
				r1_hi = *(__int32*)(pSourcePtr_cur+16-nSourceStride);
				r2_hi = *(__int32*)(pSourcePtr_cur+16);
				r3_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStride);
				r4_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStrideX2);
				r5_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStrideX2+nSourceStride);

				//First line
				zero = _mm_setzero_si128();
				Acc0 = _mm_setzero_si128();

				temp0 = _mm_srli_si128(r2, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
					temp0 = _mm_srli_si128(r2, 3);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

					temp1 = _mm_cvtsi32_si128(r2_hi);
				temp0 = _mm_srli_si128(r2, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

					temp0 = _mm_srli_si128(r1, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
					temp0 = _mm_srli_si128(r3, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)
					temp0 = _mm_srli_si128(r1, 3);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3-nSourceStride]<<1)

					temp1 = _mm_cvtsi32_si128(r1_hi);
				temp0 = _mm_srli_si128(r1, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3-nSourceStride]<<1)

					temp0 = _mm_srli_si128(r3, 3);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3+nSourceStride]<<1)

					temp1 = _mm_cvtsi32_si128(r3_hi);
				temp0 = _mm_srli_si128(r3, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3+nSourceStride]<<1)

					temp0 = _mm_srli_si128(r0, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3-nSourceStrideX2])
					temp0 = _mm_srli_si128(r0, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-nSourceStrideX2])

					temp1 = _mm_cvtsi32_si128(r0_hi);
				temp0 = _mm_srli_si128(r0, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3-nSourceStrideX2])

					temp0 = _mm_srli_si128(r4, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3+nSourceStrideX2])
					temp0 = _mm_srli_si128(r4, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+nSourceStrideX2])

					temp1 = _mm_cvtsi32_si128(r4_hi);
				temp0 = _mm_srli_si128(r4, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3+nSourceStrideX2])

					temp0 = r1;
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6-nSourceStride])
					temp0 = r2;
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6])
					temp0 = r3;
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6+nSourceStride])

					temp1 = _mm_cvtsi32_si128(r1_hi);
				temp0 = _mm_srli_si128(r1, 12);
				temp0 = _mm_unpacklo_epi32(temp0, temp1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6-nSourceStride])

					temp1 = _mm_cvtsi32_si128(r2_hi);
				temp0 = _mm_srli_si128(r2, 12);
				temp0 = _mm_unpacklo_epi32(temp0, temp1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6])

					temp1 = _mm_cvtsi32_si128(r3_hi);
				temp0 = _mm_srli_si128(r3, 12);
				temp0 = _mm_unpacklo_epi32(temp0, temp1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6+nSourceStride])

					zero = _mm_set1_epi16(16);
				Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
				Acc0 = _mm_srai_epi16(Acc0, 5);						//filter_counter >>= 5;
				Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

				_mm_storel_epi64((__m128i*)FilterPixelPtr_cur, Acc0);
				//end

				//second line
				r1 = r2;
				r2 = r3;
				r3 = r4;
				r4 = r5;
				r1_hi = r2_hi;
				r2_hi = r3_hi;
				r3_hi = r4_hi;
				r4_hi = r5_hi;

				zero = _mm_setzero_si128();
				Acc0 = _mm_setzero_si128();

				temp0 = _mm_srli_si128(r2, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
					temp0 = _mm_srli_si128(r2, 3);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

					temp1 = _mm_cvtsi32_si128(r2_hi);
				temp0 = _mm_srli_si128(r2, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3]<<1)


					temp0 = _mm_srli_si128(r1, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
					temp0 = _mm_srli_si128(r3, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)
					temp0 = _mm_srli_si128(r1, 3);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3-nSourceStride]<<1)

					temp1 = _mm_cvtsi32_si128(r1_hi);
				temp0 = _mm_srli_si128(r1, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3-nSourceStride]<<1)

					temp0 = _mm_srli_si128(r3, 3);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0-3+nSourceStride]<<1)

					temp1 = _mm_cvtsi32_si128(r3_hi);
				temp0 = _mm_srli_si128(r3, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2(temp0, Acc0, 1)						//filter_counter += (pSourcePtr_cur[0+3+nSourceStride]<<1)

					temp0 = _mm_srli_si128(r0, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3-nSourceStrideX2])
					temp0 = _mm_srli_si128(r0, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-nSourceStrideX2])

					temp1 = _mm_cvtsi32_si128(r0_hi);
				temp0 = _mm_srli_si128(r0, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3-nSourceStrideX2])

					temp0 = _mm_srli_si128(r4, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-3+nSourceStrideX2])
					temp0 = _mm_srli_si128(r4, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+nSourceStrideX2])

					temp1 = _mm_cvtsi32_si128(r4_hi);
				temp0 = _mm_srli_si128(r4, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+3+nSourceStrideX2])

					temp0 = r1;
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6-nSourceStride])
					temp0 = r2;
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6])
					temp0 = r3;
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0-6+nSourceStride])

					temp1 = _mm_cvtsi32_si128(r1_hi);
				temp0 = _mm_srli_si128(r1, 12);
				temp0 = _mm_unpacklo_epi32(temp0, temp1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6-nSourceStride])

					temp1 = _mm_cvtsi32_si128(r2_hi);
				temp0 = _mm_srli_si128(r2, 12);
				temp0 = _mm_unpacklo_epi32(temp0, temp1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6])

					temp1 = _mm_cvtsi32_si128(r3_hi);
				temp0 = _mm_srli_si128(r3, 12);
				temp0 = _mm_unpacklo_epi32(temp0, temp1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)				//filter_counter += (pSourcePtr_cur[0+6+nSourceStride])

					zero = _mm_set1_epi16(16);
				Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
				Acc0 = _mm_srai_epi16(Acc0, 5);						//filter_counter >>= 5;
				Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

				_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStride), Acc0);
				//end

				pSourcePtr_cur += 8;
				FilterPixelPtr_cur += 8;

			}
			
			for( ; i < (m_sw*3-6); i++){
				FilterPixelPtr[0] = 
					( (pSourcePtr[0]<<2)
					+ (pSourcePtr[0-3]<<1) + (pSourcePtr[0+3]<<1)
					+ (pSourcePtr[0-nSourceStride]<<1) + (pSourcePtr[0+nSourceStride]<<1)
					+ (pSourcePtr[0-3-nSourceStride]<<1) + (pSourcePtr[0+3-nSourceStride]<<1)
					+ (pSourcePtr[0-3+nSourceStride]<<1) + (pSourcePtr[0+3+nSourceStride]<<1)
					+ pSourcePtr[0-3-nSourceStrideX2] + pSourcePtr[0-nSourceStrideX2] + pSourcePtr[0+3-nSourceStrideX2]
				+ pSourcePtr[0-3+nSourceStrideX2] + pSourcePtr[0+nSourceStrideX2] + pSourcePtr[0+3+nSourceStrideX2]
				+ pSourcePtr[0-6-nSourceStride] + pSourcePtr[0-6] + pSourcePtr[0-6+nSourceStride]
				+ pSourcePtr[0+6-nSourceStride] + pSourcePtr[0+6] + pSourcePtr[0+6+nSourceStride]
				+ 16 ) >> 5;
				FilterPixelPtr[0+nSourceStride] = 
					( (pSourcePtr[0+nSourceStride]<<2)
					+ (pSourcePtr[0+nSourceStride-3]<<1) + (pSourcePtr[0+nSourceStride+3]<<1)
					+ (pSourcePtr[0+nSourceStride-nSourceStride]<<1) + (pSourcePtr[0+nSourceStride+nSourceStride]<<1)
					+ (pSourcePtr[0+nSourceStride-3-nSourceStride]<<1) + (pSourcePtr[0+nSourceStride+3-nSourceStride]<<1)
					+ (pSourcePtr[0+nSourceStride-3+nSourceStride]<<1) + (pSourcePtr[0+nSourceStride+3+nSourceStride]<<1)
					+ pSourcePtr[0+nSourceStride-3-nSourceStrideX2] + pSourcePtr[0+nSourceStride-nSourceStrideX2] + pSourcePtr[0+nSourceStride+3-nSourceStrideX2]
				+ pSourcePtr[0+nSourceStride-3+nSourceStrideX2] + pSourcePtr[0+nSourceStride+nSourceStrideX2] + pSourcePtr[0+nSourceStride+3+nSourceStrideX2]
				+ pSourcePtr[0+nSourceStride-6-nSourceStride] + pSourcePtr[0+nSourceStride-6] + pSourcePtr[0+nSourceStride-6+nSourceStride]
				+ pSourcePtr[0+nSourceStride+6-nSourceStride] + pSourcePtr[0+nSourceStride+6] + pSourcePtr[0+nSourceStride+6+nSourceStride]
				+ 16 ) >> 5;
			}


			pSourcePtr+=nSourceStrideX2;
			FilterPixelPtr+=nSourceStrideX2;

		}


	/*	{
			FILE * temp = fopen("C:\\temp\\xle_hm_test.txt", "a");
			fprintf(temp, "END\n");
			fclose(temp);
		}*/

	}
	ImageFilter22(PBYTE inSource, int SourceStride, PBYTE inSourceBackup, int DestStride, 
		int in_m_sw, int inStart, int inEnd) :
	pSource(inSource), nSourceStride(SourceStride), SourceBackup(inSourceBackup), nDestStride(DestStride), 
	m_sw(in_m_sw), StartImage(inStart), EndImage(inEnd) {}
};

class ImageFilter11{

	PBYTE pSource;
	int nSourceStride;
	PBYTE SourceBackup;
	int nDestStride;
	int m_sw;
	int StartImage;
	int EndImage;

public:
	void operator() (const blocked_range<size_t>& r) const {

		int i, j;
		PBYTE pSourcePtr = pSource + 2*nSourceStride + 6;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 6;
		int nSourceStrideX2 = nSourceStride*2;

		//int begin = r.begin();
		int begin = (r.begin()>>2)<<2;
		int end = ((r.end()+2)>>2)<<2;

		/*if(r.begin() != StartImage)
			begin = (r.begin()>>2)<<2;
		else*/
			begin =  r.begin();

		end = r.end() + (4-((r.end()-begin)%4));

		/*if(end > EndImage){
			end -= 4;
			if(end != EndImage){
				memset((FilterPixelPtr+(end-StartImage)*nSourceStride), 0, nSourceStride*(EndImage-end));
			}
		}*/

		/*{
			FILE * temp = fopen("C:\\temp\\xle_hm_test.txt", "a");
			fprintf(temp, "11:	%d	%d	%d	%d\n", begin, end, StartImage,  EndImage);
			fclose(temp);
		}*/

		pSourcePtr += (begin-StartImage)*nSourceStride;
		FilterPixelPtr += (begin-StartImage)*nSourceStride;

		for ( size_t j = begin; j != end; j+=4 ) {

			//		for( j = 2; j < (m_sh-2); j+=2){

			const unsigned char* pSourcePtr_cur = pSourcePtr-6;
			unsigned char* FilterPixelPtr_cur = FilterPixelPtr;


			for( i = 6; i < (m_sw*3-6-8); i+=8){

				__declspec(align(16)) __m128i r1, r2, r3, r4, r5, r6, temp0, Acc0, zero, temp1;
				__int32 r2_hi, r3_hi, r4_hi, r5_hi;

				r1 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur-nSourceStride));			//9 ... 3 2 1 0 -1 -2 -3 -4 -5
				r2 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur));						//9 ... 3 2 1 0 -1 -2 -3 -4 -5
				r3 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStride));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5
				r4 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5
				r5 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStride));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5
				r6 = _mm_loadu_si128((__m128i*)(pSourcePtr_cur+nSourceStrideX2+nSourceStrideX2));	//9 ... 3 2 1 0 -1 -2 -3 -4 -5
				r2_hi = *(__int32*)(pSourcePtr_cur+16);
				r3_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStride);
				r4_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStrideX2);
				r5_hi = *(__int32*)(pSourcePtr_cur+16+nSourceStrideX2+nSourceStride);

				//first line
				zero = _mm_setzero_si128();
				Acc0 = _mm_setzero_si128();

				temp0 = _mm_srli_si128(r2, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
					temp0 = _mm_srli_si128(r2, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

					temp1 = _mm_cvtsi32_si128(r2_hi);
				temp0 = _mm_srli_si128(r2, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

					temp0 = _mm_srli_si128(r1, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
					temp0 = _mm_srli_si128(r3, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

					zero = _mm_set1_epi16(4);
				Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
				Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
				Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

				_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur), Acc0);
				//end

				/*{
					FILE * temp = fopen("C:\\temp\\xle_hm_test.txt", "a");
					fprintf(temp, "FilterPixelPtr_cur:	%d		%d\n", FilterPixelPtr_cur[0], FilterPixelPtr_cur[1]);
					fclose(temp);
				}*/

				//second line
				r1 = r2;
				r2 = r3;
				r3 = r4;
				r2_hi = r3_hi;

				zero = _mm_setzero_si128();
				Acc0 = _mm_setzero_si128();

				temp0 = _mm_srli_si128(r2, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
					temp0 = _mm_srli_si128(r2, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

					temp1 = _mm_cvtsi32_si128(r2_hi);
				temp0 = _mm_srli_si128(r2, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

					temp0 = _mm_srli_si128(r1, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
					temp0 = _mm_srli_si128(r3, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

					zero = _mm_set1_epi16(4);
				Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
				Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
				Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

				_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStride), Acc0);
				//end

				//third line
				r1 = r3;
				r2 = r4;
				r3 = r5;
				r2_hi = r4_hi;

				zero = _mm_setzero_si128();
				Acc0 = _mm_setzero_si128();

				temp0 = _mm_srli_si128(r2, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
					temp0 = _mm_srli_si128(r2, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

					temp1 = _mm_cvtsi32_si128(r2_hi);
				temp0 = _mm_srli_si128(r2, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

					temp0 = _mm_srli_si128(r1, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
					temp0 = _mm_srli_si128(r3, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

					zero = _mm_set1_epi16(4);
				Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
				Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
				Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

				_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStrideX2), Acc0);
				//end

				//forth line
				r1 = r4;
				r2 = r5;
				r3 = r6;
				r2_hi = r5_hi;

				zero = _mm_setzero_si128();
				Acc0 = _mm_setzero_si128();

				temp0 = _mm_srli_si128(r2, 6);
				SSE2_PROCESS_2_2(temp0, Acc0, 2)						//filter_counter += (pSourcePtr_cur[0]<<2)
					temp0 = _mm_srli_si128(r2, 3);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-3]<<1)

					temp1 = _mm_cvtsi32_si128(r2_hi);
				temp0 = _mm_srli_si128(r2, 8);
				temp0 = _mm_unpacklo_epi64(temp0, temp1);
				temp0 = _mm_srli_si128(temp0, 1);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+3]<<1)

					temp0 = _mm_srli_si128(r1, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0-nSourceStride]<<1)
					temp0 = _mm_srli_si128(r3, 6);
				SSE2_PROCESS_2_2_no_shift(temp0, Acc0)						//filter_counter += (pSourcePtr_cur[0+nSourceStride]<<1)

					zero = _mm_set1_epi16(4);
				Acc0 = _mm_add_epi16(Acc0, zero);					//filter_counter += 16;
				Acc0 = _mm_srai_epi16(Acc0, 3);						//filter_counter >>= 5;
				Acc0 = _mm_packus_epi16(Acc0, zero);				//pack filter_counter to bytes

				_mm_storel_epi64((__m128i*)(FilterPixelPtr_cur+nSourceStrideX2+nSourceStride), Acc0);
				//end

				pSourcePtr_cur += 8;
				FilterPixelPtr_cur += 8;
			}
			for( ; i < (m_sw*3-6); i++){

				FilterPixelPtr_cur[0] = 
					( (pSourcePtr_cur[0]<<2)
					+ (pSourcePtr_cur[0-3]) + (pSourcePtr_cur[0+3])
					+ (pSourcePtr_cur[0-nSourceStride]) + (pSourcePtr_cur[0+nSourceStride])
					+ 4 ) >> 3;
				FilterPixelPtr_cur[0+nSourceStride] = 
					( (pSourcePtr_cur[0+nSourceStride]<<2)
					+ (pSourcePtr_cur[0+nSourceStride-3]) + (pSourcePtr_cur[0+nSourceStride+3])
					+ (pSourcePtr_cur[0+nSourceStride-nSourceStride]) + (pSourcePtr_cur[0+nSourceStride+nSourceStride])
					+ 4 ) >> 3;

				const unsigned char* pSourcePtr_cur_temp = pSourcePtr_cur + nSourceStrideX2;
				unsigned char* FilterPixelPtr_cur_temp = FilterPixelPtr_cur + nSourceStrideX2;

				FilterPixelPtr_cur_temp[0] = 
					( (pSourcePtr_cur_temp[0]<<2)
					+ (pSourcePtr_cur_temp[0-3]) + (pSourcePtr_cur_temp[0+3])
					+ (pSourcePtr_cur_temp[0-nSourceStride]) + (pSourcePtr_cur_temp[0+nSourceStride])
					+ 4 ) >> 3;
				FilterPixelPtr_cur_temp[0+nSourceStride] = 
					( (pSourcePtr_cur_temp[0+nSourceStride]<<2)
					+ (pSourcePtr_cur_temp[0+nSourceStride-3]) + (pSourcePtr_cur_temp[0+nSourceStride+3])
					+ (pSourcePtr_cur_temp[0+nSourceStride-nSourceStride]) + (pSourcePtr_cur_temp[0+nSourceStride+nSourceStride])
					+ 4 ) >> 3;

				pSourcePtr_cur++;
				FilterPixelPtr_cur++;

			}
			pSourcePtr+=nSourceStrideX2+nSourceStrideX2;
			FilterPixelPtr+=nSourceStrideX2+nSourceStrideX2;
		}

	}
	ImageFilter11(PBYTE inSource, int SourceStride, PBYTE inSourceBackup, int DestStride, 
		int in_m_sw, int inStart, int inEnd) :
	pSource(inSource), nSourceStride(SourceStride), SourceBackup(inSourceBackup), nDestStride(DestStride), 
		m_sw(in_m_sw), StartImage(inStart), EndImage(inEnd) {}
};

#endif
//end

void CBilinearResampler::resample_bilinear_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw = m_dw;
	int dh = m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//Hoi Ming 4May2009
#define MAX(a, b) ((a>=b)?a:b)
#define MIN(a, b) ((a<=b)?a:b)
#define EDGETHRESHOLD 256
//#define FASTCHECKTHRESHOLD 64
#define FASTCHECKTHRESHOLD 128

	int rx3, rx3mins1, rx3mins2, rx3plus1, rx3plus2, rx3plus3;
//	int bilinear_flag = 1;
	int j;

	int pixShitArray[2][3];
	
	pixShitArray[0][0] = 4*3;
	pixShitArray[0][1] = nSourceStride-4*3;
	pixShitArray[0][2] = nSourceStride+4*3;
	pixShitArray[1][0] = (nSourceStride<<2);
	pixShitArray[1][1] = (nSourceStride<<2)-1*3;
	pixShitArray[1][2] = (nSourceStride<<2)+1*3;
	//end

	int x_sinc_index, y_sinc_index;
	int filterIdx1, filterIdx2;
	int acc[3];
	int src_pos;
	
	int LPFlagX, LPFlagY;

	LPFlagX = LPFlagY = 0;

#ifdef _X64_FILTER

	if(m_x_ratio > 3.5 || m_y_ratio > 3.5){
		LPFlagX = LPFlagY = 2;
	}
	else if(m_x_ratio > 1.5 || m_y_ratio > 1.5){
		LPFlagX = LPFlagY = 1;
	}

#else

	int InterData[3][6];

	if(m_x_ratio > 3.5 || m_y_ratio > 3.5){
		LPFlagX = LPFlagY = 2;
	}
	else{
		if(m_x_ratio > 3.5)
			LPFlagX = 2;
		else if(m_x_ratio > 1.5)
			LPFlagX = 1;
		else
			LPFlagX = 0;
		if(m_y_ratio > 3.5)
			LPFlagY = 2;
		else if(m_y_ratio > 1.5)
			LPFlagY = 1;
		else
			LPFlagY = 0;
	}


#endif


#ifdef _X64_FILTER

	if(LPFlagX == 2 && LPFlagY == 2){
		//Hoi Ming TBB
#ifdef _TBB_SUPPORT

		if(m_step > 0){
			int nthread = (((m_sh-2)-4)+m_step-1)/m_step;

			task_scheduler_init init(task_scheduler_init::deferred);
			init.initialize(nthread);

			parallel_for(blocked_range<size_t>(2, (m_sh-4), m_step),
				ImageFilter22( pSource, nSourceStride, SourceBackup, nSourceStride, m_sw, 2, (m_sh-4) ) );

			init.terminate();
		}
		else{
			LowPassFilter_2_2(pSource, nSourceStride);
		}


#else
		LowPassFilter_2_2(pSource, nSourceStride);
#endif
	}
	else if(LPFlagX == 1 && LPFlagY == 1){
				//Hoi Ming TBB
#ifdef _TBB_SUPPORT

		if(m_step > 0){
			int nthread = (((m_sh-2)-4)+m_step-1)/m_step;

			task_scheduler_init init(task_scheduler_init::deferred);
			init.initialize(nthread);

			parallel_for(blocked_range<size_t>(2, (m_sh-4), m_step),
				ImageFilter11( pSource, nSourceStride, SourceBackup, nSourceStride, m_sw, 2, (m_sh-4) ) );

			init.terminate();
		}
		else{
			LowPassFilter_1_1(pSource, nSourceStride);
		}


#else
		LowPassFilter_1_1(pSource, nSourceStride);
#endif
	}

	if(LPFlagX || LPFlagY){
		PBYTE pSourcePtr = pSource + 4*nSourceStride + 12;
		PBYTE FilterPixelPtr = SourceBackup + 4*nSourceStride + 12;
	
		for( j = 4; j < (m_sh-4); j++){
			memcpy(pSourcePtr, FilterPixelPtr, (m_sw*3)-24);
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}

	/*{
		FILE * temp = fopen("C:\\temp\\xle_hm_test.txt", "a");
		PBYTE pTempPtr = pSource+(m_sh-1)*nSourceStride;
		for( int k = 0; k < 20; k++){
			for( int j = 0; j < 100; j++){
				fprintf(temp, "%d	", pTempPtr[j]);
			}
			fprintf(temp, "\n");
			pTempPtr -= nSourceStride;
		}
		fprintf(temp, "\n");
		fclose(temp);
	}*/

#else
	if(LPFlagX == 2 && LPFlagY == 2){
		PBYTE pSourcePtr = pSource + 2*nSourceStride/* + 6*/;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride/* + 6*/;
		int nSourceStrideX2 = nSourceStride*2;
		for( j = 2; j < (m_sh-2); j++){
			for( i = 6; i < (m_sw*3-6); i++){
				FilterPixelPtr[i] = 
					( (pSourcePtr[i]<<2)
					+ (pSourcePtr[i-3]<<1) + (pSourcePtr[i+3]<<1)
					+ (pSourcePtr[i-nSourceStride]<<1) + (pSourcePtr[i+nSourceStride]<<1)
					+ (pSourcePtr[i-3-nSourceStride]<<1) + (pSourcePtr[i+3-nSourceStride]<<1)
					+ (pSourcePtr[i-3+nSourceStride]<<1) + (pSourcePtr[i+3+nSourceStride]<<1)
					+ pSourcePtr[i-3-nSourceStrideX2] + pSourcePtr[i-nSourceStrideX2] + pSourcePtr[i+3-nSourceStrideX2]
				+ pSourcePtr[i-3+nSourceStrideX2] + pSourcePtr[i+nSourceStrideX2] + pSourcePtr[i+3+nSourceStrideX2]
				+ pSourcePtr[i-6-nSourceStride] + pSourcePtr[i-6] + pSourcePtr[i-6+nSourceStride]
				+ pSourcePtr[i+6-nSourceStride] + pSourcePtr[i+6] + pSourcePtr[i+6+nSourceStride]
				+ 16 ) >> 5;
			}
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}
	else if(LPFlagX == 2 && LPFlagY == 1){
		PBYTE pSourcePtr = pSource + 2*nSourceStride /*+ 6*/;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride /*+ 6*/;
//		int nSourceStrideX2 = nSourceStride*2;
		for( j = 2; j < (m_sh-2); j++){
			for( i = 6; i < (m_sw*3-6); i++){
				FilterPixelPtr[i] = 
					( (pSourcePtr[i]<<2)
					+ (pSourcePtr[i-nSourceStride]) + (pSourcePtr[i+nSourceStride])
					+ (pSourcePtr[i-3]<<1) + (pSourcePtr[i+3]<<1)
					+ (pSourcePtr[i-3-nSourceStride]) + (pSourcePtr[i+3-nSourceStride])
					+ (pSourcePtr[i-3+nSourceStride]) + (pSourcePtr[i+3+nSourceStride])
					+ (pSourcePtr[i-6]) + (pSourcePtr[i+6])
					+ 8 ) >> 4;
			}
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}
	else if(LPFlagX == 2 && LPFlagY == 0){
		PBYTE pSourcePtr = pSource + 2*nSourceStride /*+ 6*/;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride /*+ 6*/;
//		int nSourceStrideX2 = nSourceStride*2;
		for( j = 2; j < (m_sh-2); j++){
			for( i = 6; i < (m_sw*3-6); i++){
				FilterPixelPtr[i] = 
					( (pSourcePtr[i]<<3)
					+ (pSourcePtr[i-3]<<1) + (pSourcePtr[i-3]) + (pSourcePtr[i+3]<<1) + (pSourcePtr[i+3])
					+ (pSourcePtr[i-6]) + (pSourcePtr[i+6])
					+ 8 ) >> 4;
			}
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}
	else if(LPFlagX == 1 && LPFlagY == 2){
		PBYTE pSourcePtr = pSource + 2*nSourceStride /*+ 6*/;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride /*+ 6*/;
		int nSourceStrideX2 = nSourceStride*2;
		for( j = 2; j < (m_sh-2); j++){
			for( i = 6; i < (m_sw*3-6); i++){
				FilterPixelPtr[i] = 
					( (pSourcePtr[i]<<2)
					+ (pSourcePtr[i-nSourceStride]<<1) + (pSourcePtr[i+nSourceStride]<<1)
					+ (pSourcePtr[i-3]) + (pSourcePtr[i+3])
					+ (pSourcePtr[i-3-nSourceStride]) + (pSourcePtr[i+3-nSourceStride])
					+ (pSourcePtr[i-3+nSourceStride]) + (pSourcePtr[i+3+nSourceStride])
					+ (pSourcePtr[i-nSourceStrideX2]) + (pSourcePtr[i+nSourceStrideX2])
					+ 8 ) >> 4;
			}
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}
	else if(LPFlagX == 1 && LPFlagY == 1){
		PBYTE pSourcePtr = pSource + 2*nSourceStride /*+ 6*/;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride/* + 6*/;
		for( j = 2; j < (m_sh-2); j++){
			for( i = 6; i < (m_sw*3-6); i++){
				FilterPixelPtr[i] = 
					( (pSourcePtr[i]<<2)
					+ (pSourcePtr[i-3]) + (pSourcePtr[i+3])
					+ (pSourcePtr[i-nSourceStride]) + (pSourcePtr[i+nSourceStride])
					+ 4 ) >> 3;
			}
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}
	else if(LPFlagX == 1 && LPFlagY == 0){
		PBYTE pSourcePtr = pSource + 2*nSourceStride/* + 6*/;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride/* + 6*/;
		for( j = 2; j < (m_sh-2); j++){
			for( i = 6; i < (m_sw*3-6); i++){
				FilterPixelPtr[i] = 
					( (pSourcePtr[i]<<1)
					+ (pSourcePtr[i-3]) + (pSourcePtr[i+3])
					+ 2 ) >> 2;
			}
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}
	else if(LPFlagX == 0 && LPFlagY == 2){
		PBYTE pSourcePtr = pSource + 2*nSourceStride/* + 6*/;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride /*+ 6*/;
		int nSourceStrideX2 = nSourceStride*2;
		for( j = 2; j < (m_sh-2); j++){
			for( i = 6; i < (m_sw*3-6); i++){
				FilterPixelPtr[i] = 
					( (pSourcePtr[i]<<3)
					+ (pSourcePtr[i-nSourceStride]<<1) + (pSourcePtr[i-nSourceStride]) + (pSourcePtr[i+nSourceStride]<<1) + (pSourcePtr[i+nSourceStride])
					+ (pSourcePtr[i-nSourceStrideX2]) + (pSourcePtr[i+nSourceStrideX2])
					+ 8 ) >> 4;
			}
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}
	else if(LPFlagX == 0 && LPFlagY == 1){
		PBYTE pSourcePtr = pSource + 2*nSourceStride /*+ 6*/;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride /*+ 6*/;
		for( j = 2; j < (m_sh-2); j++){
			for( i = 6; i < (m_sw*3-6); i++){
				FilterPixelPtr[i] = 
					( (pSourcePtr[i]<<1)
					+ (pSourcePtr[i-nSourceStride]) + (pSourcePtr[i+nSourceStride])
					+ 2 ) >> 2;
			}
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}


	if(LPFlagX || LPFlagY){
		PBYTE pSourcePtr = pSource + 2*nSourceStride + 6;
		PBYTE FilterPixelPtr = SourceBackup + 2*nSourceStride + 6;
		for( j = 2; j < (m_sh-2); j++){
			memcpy(pSourcePtr, FilterPixelPtr, (m_sw*3)-12);
			pSourcePtr += nSourceStride;
			FilterPixelPtr += nSourceStride;
		}
	}
#endif



	y_accum = 0;
	for (vy = 0; vy < dh; vy++)  
	{
		BYTE *s1, *s2, *sTemp;
		int q;
		int ry;
		ry = (y_accum>>BILINEAR_PREC);

		//upper line 
		s1 = pSource + ry * nSourceStride;
		if(ry + 1 < m_sh)
		{
			//lower line
			s2 = s1 + nSourceStride;
		}
		else
			s2 = s1;
		x_accum = 0;
		q = y_accum & BILINEAR_MASK;

		y_sinc_index = (q>>11);

		if(y_sinc_index == 0){
			y_sinc_index = -1;
		}
		else if(y_sinc_index == 15){
			y_sinc_index = -2;
		}
		else{
			y_sinc_index = (y_sinc_index-1)>>1;
		}


		for (vx = 0; vx < 3 * dw; vx += 3)
		{
			BYTE res[3];
			int rx;
			int a[3], b[3], c[3], d[3];
			int p;

			rx = x_accum>>BILINEAR_PREC;
			p = x_accum & BILINEAR_MASK;

			x_sinc_index = (p>>11);

			if(x_sinc_index <= 0){
				x_sinc_index = -1;
			}
			else if(x_sinc_index >= 15){
				x_sinc_index = -2;
			}
			else{
				x_sinc_index = (x_sinc_index-1)>>1;
			}

			rx3 = rx+rx+rx;
			rx3mins2 = rx3-6;
			rx3mins1 = rx3-3;
			rx3plus1 = rx3+3;
			rx3plus2 = rx3+6;
			rx3plus3 = rx3+9;

#ifdef _X64_FILTER

			__declspec(align(16)) __int32 InterDataRevDim[6][4];

			//__declspec(align(16)) __int32 InterDataRevDimTemp[6][4];

			if( (rx+3) < m_sw && (rx-3) >= 0 && (ry+3) < m_sh && (ry-3) >= 0)
			{
				//Horizontal Filtering
				if( x_sinc_index >= 0 ){
					sTemp = s1 - nSourceStride;

					__declspec(align(16)) __m128i filter0, filter1, temp;
					filter0 = _mm_set1_epi16(SincFilter[x_sinc_index][0]);
					temp = _mm_set1_epi16(SincFilter[x_sinc_index][1]);
					filter0 = _mm_unpacklo_epi64(filter0, temp);
					filter1 = _mm_set1_epi16(SincFilter[x_sinc_index][2]);
					temp = _mm_set1_epi16(SincFilter[x_sinc_index][3]);
					filter1 = _mm_unpacklo_epi64(filter1, temp);

					for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){

						InterDataRevDim[filterIdx1][0] = InterDataRevDim[filterIdx1][1] = InterDataRevDim[filterIdx1][2] = 0;
						src_pos = -3;

						__declspec(align(16)) __m128i Acc, zero, r0, r1, result_hi, result_lo;
						Acc = _mm_setzero_si128();
						zero = _mm_setzero_si128();

						//first 2 taps
						r0 = _mm_cvtsi32_si128(*(__int32*)(sTemp+rx3+src_pos));		//x ... x 2 1 0
						r1 = _mm_cvtsi32_si128(*(__int32*)(sTemp+rx3+src_pos+3));		//x ... x 2 1 0
						r0 = _mm_unpacklo_epi32(r0, r1);							//x ... x 2 1 0 x 2 1 0
						r0 = _mm_unpacklo_epi8(r0, zero);							//x 2 1 0 x 2 1 0
						r1 = _mm_mulhi_epi16(r0, filter0);
						r0 = _mm_mullo_epi16(r0, filter0);
						result_lo = _mm_unpacklo_epi16(r0, r1);
						Acc = _mm_add_epi32(Acc, result_lo);
						result_hi = _mm_unpackhi_epi16(r0, r1);
						Acc = _mm_add_epi32(Acc, result_hi);

						//last 2 taps
						r0 = _mm_cvtsi32_si128(*(__int32*)(sTemp+rx3+src_pos+6));		//x ... x 2 1 0
						r1 = _mm_cvtsi32_si128(*(__int32*)(sTemp+rx3+src_pos+9));		//x ... x 2 1 0
						r0 = _mm_unpacklo_epi32(r0, r1);							//x ... x 2 1 0 x 2 1 0
						r0 = _mm_unpacklo_epi8(r0, zero);							//x 2 1 0 x 2 1 0
						r1 = _mm_mulhi_epi16(r0, filter1);
						r0 = _mm_mullo_epi16(r0, filter1);
						result_lo = _mm_unpacklo_epi16(r0, r1);
						Acc = _mm_add_epi32(Acc, result_lo);
						result_hi = _mm_unpackhi_epi16(r0, r1);
						Acc = _mm_add_epi32(Acc, result_hi);

						temp = _mm_set1_epi32(256);
						Acc = _mm_add_epi32(Acc, temp);
						Acc = _mm_srai_epi32(Acc, 9);

						_mm_store_si128((__m128i*)(&InterDataRevDim[filterIdx1][0]), Acc);

						sTemp += nSourceStride;
					}
				}
				else{
					if(x_sinc_index == -1){
						sTemp = s1 - nSourceStride;
						for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
							InterDataRevDim[filterIdx1][0] = sTemp[rx3];
							InterDataRevDim[filterIdx1][1] = sTemp[rx3+1];
							InterDataRevDim[filterIdx1][2] = sTemp[rx3+2];
							sTemp += nSourceStride;
						}
					}
					else{
						sTemp = s1 - nSourceStride;
						for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
							InterDataRevDim[filterIdx1][0] = sTemp[rx3plus1];
							InterDataRevDim[filterIdx1][1] = sTemp[rx3plus1+1];
							InterDataRevDim[filterIdx1][2] = sTemp[rx3plus1+2];
							sTemp += nSourceStride;
						}
					}
				}
				//end
				//Vertical Filtering
				if( y_sinc_index >= 0){

					acc[0] = acc[1] = acc[2] = 0;

					for(filterIdx2 = 0; filterIdx2 < 4; filterIdx2++){
						acc[0] += InterDataRevDim[filterIdx2][0]*SincFilter[y_sinc_index][filterIdx2];
						acc[1] += InterDataRevDim[filterIdx2][1]*SincFilter[y_sinc_index][filterIdx2];
						acc[2] += InterDataRevDim[filterIdx2][2]*SincFilter[y_sinc_index][filterIdx2];
					}
					res[0] =  pClipTab[(acc[0] + 256) >> 9];
					res[1] =  pClipTab[(acc[1] + 256) >> 9];
					res[2] =  pClipTab[(acc[2] + 256) >> 9];
				}
				else{
					if(y_sinc_index == -1){
						res[0] = pClipTab[InterDataRevDim[1][0]];
						res[1] = pClipTab[InterDataRevDim[1][1]];
						res[2] = pClipTab[InterDataRevDim[1][2]];
					}
					else{
						res[0] = pClipTab[InterDataRevDim[2][0]];
						res[1] = pClipTab[InterDataRevDim[2][1]];
						res[2] = pClipTab[InterDataRevDim[2][2]];
					}	
				}
				//end

			}
			else
			{
				a[0] = b[0] = s1[rx3];
				a[1] = b[1] = s1[rx3+1];
				a[2] = b[2] = s1[rx3+2];
				c[0] = d[0] = s2[rx3];
				c[1] = d[1] = s2[rx3+1];
				c[2] = d[2] = s2[rx3+2];

				int t0[3], t1[3];

				t0[0] = a[0] - ((p * (a[0]-b[0]) + (1<<14) ) >>BILINEAR_PREC);
				t0[1] = a[1] - ((p * (a[1]-b[1]) + (1<<14) ) >>BILINEAR_PREC);
				t0[2] = a[2] - ((p * (a[2]-b[2]) + (1<<14) ) >>BILINEAR_PREC);
				t1[0] = c[0] - ((p * (c[0]-d[0]) + (1<<14) ) >>BILINEAR_PREC);
				t1[1] = c[1] - ((p * (c[1]-d[1]) + (1<<14) ) >>BILINEAR_PREC);
				t1[2] = c[2] - ((p * (c[2]-d[2]) + (1<<14) ) >>BILINEAR_PREC);

				res[0] = (BYTE)(t0[0] - ((q*(t0[0]-t1[0]) + (1<<14))>>BILINEAR_PREC));
				res[1] = (BYTE)(t0[1] - ((q*(t0[1]-t1[1]) + (1<<14))>>BILINEAR_PREC));
				res[2] = (BYTE)(t0[2] - ((q*(t0[2]-t1[2]) + (1<<14))>>BILINEAR_PREC));
			}

#else
			if( (rx+3) < m_sw && (rx-3) >= 0 && (ry+3) < m_sh && (ry-3) >= 0)
			{
				//Horizontal Filtering
				if( x_sinc_index >= 0 ){
					sTemp = s1 - nSourceStride;
					for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
						InterData[0][filterIdx1] = InterData[1][filterIdx1] = InterData[2][filterIdx1] = 0;
						src_pos = -3;
						for( filterIdx2 = 0; filterIdx2 < 4; filterIdx2++){
							InterData[0][filterIdx1] 
							+= sTemp[rx3+src_pos]*SincFilter[x_sinc_index][filterIdx2];
							InterData[1][filterIdx1] 
							+= sTemp[rx3+src_pos+1]*SincFilter[x_sinc_index][filterIdx2];
							InterData[2][filterIdx1] 
							+= sTemp[rx3+src_pos+2]*SincFilter[x_sinc_index][filterIdx2];
							src_pos += 3;
						}
						InterData[0][filterIdx1] = (InterData[0][filterIdx1] + 256) >> 9;
						InterData[1][filterIdx1] = (InterData[1][filterIdx1] + 256) >> 9;
						InterData[2][filterIdx1] = (InterData[2][filterIdx1] + 256) >> 9;
						sTemp += nSourceStride;
					}
				}
				else{
					if(x_sinc_index == -1){
						sTemp = s1 - nSourceStride;
						for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
							InterData[0][filterIdx1] = sTemp[rx3];
							InterData[1][filterIdx1] = sTemp[rx3+1];
							InterData[2][filterIdx1] = sTemp[rx3+2];
							sTemp += nSourceStride;
						}
					}
					else{
						sTemp = s1 - nSourceStride;
						for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
							InterData[0][filterIdx1] = sTemp[rx3plus1];
							InterData[1][filterIdx1] = sTemp[rx3plus1+1];
							InterData[2][filterIdx1] = sTemp[rx3plus1+2];
							sTemp += nSourceStride;
						}
					}
				}
				//end
				//Vertical Filtering
				if( y_sinc_index >= 0){

					acc[0] = acc[1] = acc[2] = 0;

					for(filterIdx2 = 0; filterIdx2 < 4; filterIdx2++){
						acc[0] += InterData[0][filterIdx2]*SincFilter[y_sinc_index][filterIdx2];
						acc[1] += InterData[1][filterIdx2]*SincFilter[y_sinc_index][filterIdx2];
						acc[2] += InterData[2][filterIdx2]*SincFilter[y_sinc_index][filterIdx2];
					}
					res[0] =  pClipTab[(acc[0] + 256) >> 9];
					res[1] =  pClipTab[(acc[1] + 256) >> 9];
					res[2] =  pClipTab[(acc[2] + 256) >> 9];
				}
				else{
					if(y_sinc_index == -1){
						res[0] = pClipTab[InterData[0][1]];
						res[1] = pClipTab[InterData[1][1]];
						res[2] = pClipTab[InterData[2][1]];
					}
					else{
						res[0] = pClipTab[InterData[0][2]];
						res[1] = pClipTab[InterData[1][2]];
						res[2] = pClipTab[InterData[2][2]];
					}	
				}
				//end

			}
			else
			{
				a[0] = b[0] = s1[rx3];
				a[1] = b[1] = s1[rx3+1];
				a[2] = b[2] = s1[rx3+2];
				c[0] = d[0] = s2[rx3];
				c[1] = d[1] = s2[rx3+1];
				c[2] = d[2] = s2[rx3+2];

				int t0[3], t1[3];

				t0[0] = a[0] - ((p * (a[0]-b[0]) + (1<<14) ) >>BILINEAR_PREC);
				t0[1] = a[1] - ((p * (a[1]-b[1]) + (1<<14) ) >>BILINEAR_PREC);
				t0[2] = a[2] - ((p * (a[2]-b[2]) + (1<<14) ) >>BILINEAR_PREC);
				t1[0] = c[0] - ((p * (c[0]-d[0]) + (1<<14) ) >>BILINEAR_PREC);
				t1[1] = c[1] - ((p * (c[1]-d[1]) + (1<<14) ) >>BILINEAR_PREC);
				t1[2] = c[2] - ((p * (c[2]-d[2]) + (1<<14) ) >>BILINEAR_PREC);

				res[0] = (BYTE)(t0[0] - ((q*(t0[0]-t1[0]) + (1<<14))>>BILINEAR_PREC));
				res[1] = (BYTE)(t0[1] - ((q*(t0[1]-t1[1]) + (1<<14))>>BILINEAR_PREC));
				res[2] = (BYTE)(t0[2] - ((q*(t0[2]-t1[2]) + (1<<14))>>BILINEAR_PREC));
			}

#endif

			pDest[vx] = pClipTab[res[0]];	
			pDest[vx+1] = pClipTab[res[1]];	
			pDest[vx+2] = pClipTab[res[2]];	

			x_accum += m_w_inc;

		}


		pDest += nDestStride;
		
		y_accum += m_h_inc;
	}
	
}

//Hoi Ming YUV resizer
void CBilinearResampler::resample_bilinear_yuv420(PBYTE pSourceY, PBYTE pSourceU, PBYTE pSourceV, int nSourceStride, 
												  PBYTE pDestY, PBYTE pDestU, PBYTE pDestV, int nDestStride, int clipindex)
{
	

	int vx,vy;
	int dw = m_dw;
	int dh = m_dh;

	unsigned int x_accum;
	unsigned int y_accum;

	BYTE *pClipTab = &(clip_tab[clipindex][1024]);


#define MAX(a, b) ((a>=b)?a:b)
#define MIN(a, b) ((a<=b)?a:b)

	//int rx3, rx3mins1, rx3mins2, rx3plus1, rx3plus2, rx3plus3;
	int j;

#define	IMAGE_Y 0
#define	IMAGE_U 1
#define	IMAGE_V 2


	PBYTE pSource_plane[3] = {pSourceY, pSourceU, pSourceV};
	PBYTE pDest_plane[3] = {pDestY, pDestU, pDestV};
	int nSourceStride_plane[3] = {nSourceStride, (nSourceStride>>1), (nSourceStride>>1)};
	int nDestStride_plane[3] = {nDestStride, (nDestStride>>1), (nDestStride>>1)};
	int m_dw_plane[3] = {m_dw, (m_dw>>1), (m_dw>>1)};
	int m_dh_plane[3] = {m_dh, (m_dh>>1), (m_dh>>1)};
	int m_sw_plane[3] = {m_sw, (m_sw>>1), (m_sw>>1)};
	int m_sh_plane[3] = {m_sh, (m_sh>>1), (m_sh>>1)};
	

	int x_sinc_index, y_sinc_index;
	int InterData[3][6];
	int filterIdx1, filterIdx2;
	int acc[3];
	int src_pos;
	
	int LPFlagX, LPFlagY;

	int IsFiltered = 0;


	LPFlagX = LPFlagY = 0;

	if(m_x_ratio > 3.5 || m_y_ratio > 3.5){
		LPFlagX = LPFlagY = 2;
	}
	else if(m_x_ratio > 1.5 || m_y_ratio > 1.5){
		LPFlagX = LPFlagY = 1;
	}

	for(int image = IMAGE_Y; image <= IMAGE_V; image++){

		IsFiltered = 1;

		if(LPFlagX == 2 && LPFlagY == 2){
			//Hoi Ming TBB
#ifdef _TBB_SUPPORT

			if(m_step > 0){
				int nthread = (((m_sh_plane[image]-2)-4)+m_step-1)/m_step;

				task_scheduler_init init(task_scheduler_init::deferred);
				init.initialize(nthread);

				if(image == IMAGE_Y){
					parallel_for(blocked_range<size_t>(2, (m_sh_plane[image]-4), m_step),
						ImageFilterYUV22( pSource_plane[image], nSourceStride_plane[image], SourceBackup, nSourceStride_plane[image], m_sw_plane[image], 2, (m_sh_plane[image]-4) ) );
				}
				else{
					parallel_for(blocked_range<size_t>(2, (m_sh_plane[image]-4), m_step),
						ImageFilterYUV11( pSource_plane[image], nSourceStride_plane[image], SourceBackup, nSourceStride_plane[image], m_sw_plane[image], 2, (m_sh_plane[image]-4) ) );
				}

				init.terminate();
			}
			else{
				if(image == IMAGE_Y){
					LowPassFilter_SingleColour_2_2(pSource_plane[image], nSourceStride_plane[image], m_sw_plane[image], m_sh_plane[image]);
				}
				else{
					LowPassFilter_SingleColour_1_1(pSource_plane[image], nSourceStride_plane[image], m_sw_plane[image], m_sh_plane[image]);
				}
			}


#else
			if(image == IMAGE_Y){
				LowPassFilter_SingleColour_2_2(pSource_plane[image], nSourceStride_plane[image], m_sw_plane[image], m_sh_plane[image]);
			}
			else{
				LowPassFilter_SingleColour_1_1(pSource_plane[image], nSourceStride_plane[image], m_sw_plane[image], m_sh_plane[image]);
			}
#endif
		}
		else if(LPFlagX == 1 && LPFlagY == 1){
			//Hoi Ming TBB
#ifdef _TBB_SUPPORT

			if(image == IMAGE_Y){
				if(m_step > 0){
					int nthread = (((m_sh_plane[image]-2)-4)+m_step-1)/m_step;

					task_scheduler_init init(task_scheduler_init::deferred);
					init.initialize(nthread);

					parallel_for(blocked_range<size_t>(2, (m_sh_plane[image]-4), m_step),
						ImageFilterYUV11( pSource_plane[image], nSourceStride_plane[image], SourceBackup, nSourceStride_plane[image], m_sw_plane[image], 2, (m_sh_plane[image]-4) ) );

					init.terminate();
				}
				else{
					LowPassFilter_SingleColour_1_1(pSource_plane[image], nSourceStride_plane[image], m_sw_plane[image], m_sh_plane[image]);
				}
			}
			else{
				IsFiltered = 0;
			}

#else
			if(image == IMAGE_Y){
				LowPassFilter_SingleColour_1_1(pSource_plane[image], nSourceStride_plane[image], m_sw_plane[image], m_sh_plane[image]);
			}
			else{
				IsFiltered = 0;
			}
#endif
		}

		if((LPFlagX || LPFlagY) && IsFiltered){
			PBYTE pSourcePtr = pSource_plane[image] + 4*nSourceStride_plane[image] + 4;
			PBYTE FilterPixelPtr = SourceBackup + 4*nSourceStride_plane[image] + 4;
			for( j = 4; j < (m_sh_plane[image]-4); j++){
				memcpy(pSourcePtr, FilterPixelPtr, (m_sw_plane[image])-8);
				pSourcePtr += nSourceStride_plane[image];
				FilterPixelPtr += nSourceStride_plane[image];
			}
		}

	}

	for(int image = IMAGE_Y; image <= IMAGE_V; image++){

		/*{
			FILE* temp = fopen("c:\\temp\\resizer_test.txt", "a");
			fprintf(temp, "outside1:	%d\n", m_step);
			fclose(temp);
		}*/

		if(m_step > 0){

#ifdef _TBB_SUPPORT // fix complie error, 2010-07-16, jerry

			int nthread = (m_dh_plane[image]+m_step-1)/m_step;

			task_scheduler_init init(task_scheduler_init::deferred);
			init.initialize(nthread);

			parallel_for(blocked_range<size_t>(0, m_dh_plane[image], m_step),
				ImageFilterYUVInterpolation( pSource_plane[image], nSourceStride_plane[image], pDest_plane[image], nDestStride_plane[image], 
				m_sw_plane[image], m_dw_plane[image], m_w_inc, m_h_inc, 0, m_dh_plane[image], pClipTab, SincFilter, image ) );


			init.terminate();
#endif 

		}
		else{

			y_accum = 0;
			for (vy = 0; vy < m_dh_plane[image]; vy++)  
			{
				BYTE *s1, *s2, *sTemp;
				int q;
				int ry;
				ry = (y_accum>>BILINEAR_PREC);

				//upper line 
				s1 = pSource_plane[image] + ry * nSourceStride_plane[image];
				if(ry + 1 < m_sh_plane[image])
				{
					//lower line
					s2 = s1 + nSourceStride_plane[image];
				}
				else
					s2 = s1;
				x_accum = 0;
				q = y_accum & BILINEAR_MASK;

				y_sinc_index = (q>>11);

				if(y_sinc_index == 0){
					y_sinc_index = -1;
				}
				else if(y_sinc_index == 15){
					y_sinc_index = -2;
				}
				else{
					y_sinc_index = (y_sinc_index-1)>>1;
				}

				for (vx = 0; vx < m_dw_plane[image]; vx++)
				{
					BYTE res[3];
					int rx;
					int a[3], b[3], c[3], d[3];
					int p;

					rx = x_accum>>BILINEAR_PREC;
					p = x_accum & BILINEAR_MASK;

					x_sinc_index = (p>>11);

					if(x_sinc_index <= 0){
						x_sinc_index = -1;
					}
					else if(x_sinc_index >= 15){
						x_sinc_index = -2;
					}
					else{
						x_sinc_index = (x_sinc_index-1)>>1;
					}

					//__declspec(align(16)) __int32 InterDataRevDimTemp[6][4];

					if( image == IMAGE_Y && (rx+2) < m_sw_plane[image] && (rx-1) >= 0 && (ry+2) < m_sh_plane[image] && (ry-1) >= 0)
					{
						//Horizontal Filtering
						if( x_sinc_index >= 0 ){
							sTemp = s1 - nSourceStride_plane[image];
							for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
								InterData[0][filterIdx1] = 0;
								src_pos = -1;
								for( filterIdx2 = 0; filterIdx2 < 4; filterIdx2++){
									InterData[0][filterIdx1] 
									+= sTemp[rx+src_pos]*SincFilter[x_sinc_index][filterIdx2];
									src_pos += 1;
								}
								InterData[0][filterIdx1] = (InterData[0][filterIdx1] + 256) >> 9;

								sTemp += nSourceStride_plane[image];
							}
						}
						else{
							if(x_sinc_index == -1){
								sTemp = s1 - nSourceStride_plane[image];
								for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
									InterData[0][filterIdx1] = sTemp[rx];
									sTemp += nSourceStride_plane[image];
								}
							}
							else{
								sTemp = s1 - nSourceStride_plane[image];
								for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
									InterData[0][filterIdx1] = sTemp[rx+1];
									sTemp += nSourceStride_plane[image];
								}
							}
						}
						//end
						//Vertical Filtering
						if( y_sinc_index >= 0){

							acc[0] = acc[1] = acc[2] = 0;

							for(filterIdx2 = 0; filterIdx2 < 4; filterIdx2++){
								acc[0] += InterData[0][filterIdx2]*SincFilter[y_sinc_index][filterIdx2];
							}
							res[0] =  pClipTab[(acc[0] + 256) >> 9];
						}
						else{
							if(y_sinc_index == -1){
								res[0] = pClipTab[InterData[0][1]];
							}
							else{
								res[0] = pClipTab[InterData[0][2]];
							}	
						}
						//end

					}
					else
					{
						if((rx+1) < m_sw_plane[image]){
							a[0] = s1[rx]; b[0] = s1[rx+1];
							c[0] = s2[rx]; d[0] = s2[rx+1];
						}
						else{
							a[0] = b[0] = s1[rx];
							c[0] = d[0] = s2[rx];
						}
						int t0[3], t1[3];

						t0[0] = a[0] - ((p * (a[0]-b[0]) + (1<<14) ) >>BILINEAR_PREC);
						t1[0] = c[0] - ((p * (c[0]-d[0]) + (1<<14) ) >>BILINEAR_PREC);

						res[0] = (BYTE)(t0[0] - ((q*(t0[0]-t1[0]) + (1<<14))>>BILINEAR_PREC));


					}


					pDest_plane[image][vx] = pClipTab[res[0]];

					x_accum += m_w_inc;

				}

				pDest_plane[image] += nDestStride_plane[image];

				y_accum += m_h_inc;
			}

		}
	}

	
	
}

//Only for down-sample

void CBilinearResampler::resample_bilinear_yuv420_downsample(PBYTE pSourceY, PBYTE pSourceU, PBYTE pSourceV, int nSourceStride, 
												  PBYTE pDestY, PBYTE pDestU, PBYTE pDestV, int nDestStride , int clipindex)
{
	

	int vx,vy;
	int dw = m_dw;
	int dh = m_dh;

	unsigned int x_accum;
	unsigned int y_accum;

	BYTE *pClipTab = &(clip_tab[clipindex][1024]);

	__int64 CounterStart = 0;
	__int64 CounterLPF = 0;
	__int64 CounterSampling = 0;
	LARGE_INTEGER li;
	int Freq;
	QueryPerformanceFrequency(&li);
	Freq = 1;//li.QuadPart /1000;
	
	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();


#define MAX(a, b) ((a>=b)?a:b)
#define MIN(a, b) ((a<=b)?a:b)

	//int rx3, rx3mins1, rx3mins2, rx3plus1, rx3plus2, rx3plus3;
	int j;

#define	IMAGE_Y 0
#define	IMAGE_U 1
#define	IMAGE_V 2


	PBYTE pSource_plane[3] = {pSourceY, pSourceU, pSourceV};
	PBYTE pDest_plane[3] = {pDestY, pDestU, pDestV};
	int nSourceStride_plane[3] = {nSourceStride, (nSourceStride>>1), (nSourceStride>>1)};
	int nDestStride_plane[3] = {nDestStride, (nDestStride>>1), (nDestStride>>1)};
	int m_dw_plane[3] = {m_dw, (m_dw>>1), (m_dw>>1)};
	int m_dh_plane[3] = {m_dh, (m_dh>>1), (m_dh>>1)};
	int m_sw_plane[3] = {m_sw, (m_sw>>1), (m_sw>>1)};
	int m_sh_plane[3] = {m_sh, (m_sh>>1), (m_sh>>1)};

		

	int x_sinc_index, y_sinc_index;
	int InterData[3][6];
	int filterIdx1, filterIdx2;
	int acc;
	int src_pos;
	
	int LPFlagX, LPFlagY;

	int IsFiltered = 0;


	LPFlagX = LPFlagY = 0;
	

	if(m_x_ratio > 3.5 || m_y_ratio > 3.5){
		LPFlagX = LPFlagY = 2;
		//printf("LPFlagX : 2\n");
	}
	else if(m_x_ratio > 1.5 || m_y_ratio > 1.5){
		LPFlagX = LPFlagY = 1;
		//printf("LPFlagX : 1\n");
	}
	
	CounterLPF = 0;

	QueryPerformanceCounter(&li);
	CounterStart = li.QuadPart;
	int image = IMAGE_Y;
	{

		int m_dh = m_dh_plane[image];
		int m_dw = m_dw_plane[image];
		int m_sw = m_sw_plane[image];
		int m_sh = m_sh_plane[image];
		PBYTE pSource = pSource_plane[image];
		int nSourceStride = nSourceStride_plane[image];
		int nDestStride = nDestStride_plane[image];
		{

			y_accum = 0;
			for (vy = 0; vy < m_dh; vy++)  
			{
				BYTE *s1, *s2, *sTemp;
				int q;
				int ry;
				ry = (y_accum>>BILINEAR_PREC);

				//upper line 
				s1 = pSource + ry * nSourceStride;
				if(ry + 1 < m_sh)
				{
					//lower line
					s2 = s1 + nSourceStride;
				}
				else{
					s2 = s1;
				}
				x_accum = 0;
				q = y_accum & BILINEAR_MASK;

				y_sinc_index = (q>>11);

				if(y_sinc_index == 0){
					y_sinc_index = -2;
				}
				else if(y_sinc_index == 15){
					y_sinc_index = -1;
				}
				else{
					y_sinc_index = (y_sinc_index-1)>>1;
				}

				for (vx = 0; vx < m_dw; vx++)
				{
					BYTE res;
					int rx;
					__int64 a, b, c, d;
					int p;

					rx = x_accum>>BILINEAR_PREC;
					p = x_accum & BILINEAR_MASK;

					x_sinc_index = (p>>11);

					if(x_sinc_index <= 0){
						x_sinc_index = -2;
					}
					else if(x_sinc_index >= 15){
						x_sinc_index = -1;
					}
					else{
						x_sinc_index = (x_sinc_index-1)>>1;
					}


					if((rx+3) < m_sw && (rx-3) >= 0 && (ry+3) < m_sh && (ry-3) >= 0)
					{
						if(LPFlagX == 2 && LPFlagY == 2)
						{
							if(x_sinc_index < 0 && y_sinc_index < 0){
								short filter_coeff[5][8] = { 
									{0,1,1,1,0,0,0,0},
									{1,2,2,2,1,0,0,0},
									{1,2,4,2,1,0,0,0},
									{1,2,2,2,1,0,0,0},
									{0,1,1,1,0,0,0,0}
								};

								sTemp = s1 - 3*nSourceStride + rx + 2 + x_sinc_index + (3 + y_sinc_index) * nSourceStride - 2;

								__declspec(align(16)) __m128i sum, src, filter;
								int result[16];
								sum = _mm_setzero_si128();
								for(int i = 0;i<5;i++){
									src = _mm_loadl_epi64((__m128i*)sTemp);
									src = _mm_unpacklo_epi8(src, zero);
									filter = _mm_loadu_si128((__m128i*)&filter_coeff[i][0]);
									src = _mm_madd_epi16(src,filter);
									sum = _mm_add_epi32(sum,src);
									sTemp += nSourceStride;
								}
								_mm_store_si128((__m128i*)result, sum);
							
								acc = result[0] + result[1] + result[2];
								res = pClipTab[(acc + 16) >> 5];

							
							}
							else if(x_sinc_index < 0){
								
								sTemp = s1 - 3*nSourceStride + rx + 2 + x_sinc_index - 2;

								__declspec(align(16)) __m128i sum, src, filter;
								int result[16];
								sum = _mm_setzero_si128();
								//sTemp = s1_b - 3*nSourceStride + rx - 3;
							
								for(int i = 0;i<8;i++){
									src = _mm_loadl_epi64((__m128i*)sTemp);
									src = _mm_unpacklo_epi8(src, zero);
									filter = _mm_loadu_si128((__m128i*)&SincFilter_mix_conv_2_2_v[y_sinc_index][i][0]);
									src = _mm_madd_epi16(src,filter);
									sum = _mm_add_epi32(sum,src);
									sTemp += nSourceStride;
								}
								_mm_store_si128((__m128i*)result, sum);
							
								acc = result[0] + result[1] + result[2];
								res =  pClipTab[(acc + (1<<13)) >> 14];


							}
							else if(y_sinc_index < 0){

								sTemp = s1 - 3*nSourceStride + (3 + y_sinc_index) *  nSourceStride - 3 + rx;

								__declspec(align(16)) __m128i sum, src, filter;
								int result[16];
								sum = _mm_setzero_si128();
							
								for(int i = 0;i<5;i++){
									src = _mm_loadl_epi64((__m128i*)sTemp);
									src = _mm_unpacklo_epi8(src, zero);
									filter = _mm_loadu_si128((__m128i*)&SincFilter_mix_conv_2_2_h[x_sinc_index][i][0]);
									src = _mm_madd_epi16(src,filter);
									sum = _mm_add_epi32(sum,src);
									sTemp += nSourceStride;
								}
								_mm_store_si128((__m128i*)result, sum);
							
								acc = result[0] + result[1] + result[2] + result[3];
								res =  pClipTab[(acc + (1<<13)) >> 14];

								

							}
							else
							{
								__declspec(align(16)) __m128i sum, src, filter;
								int result[16];
								sum = _mm_setzero_si128();
								sTemp = s1 - 3*nSourceStride + rx - 3;
							
								for(int i = 0;i<8;i++){
									src = _mm_loadl_epi64((__m128i*)sTemp);
									src = _mm_unpacklo_epi8(src, zero);
									filter = _mm_loadu_si128((__m128i*)&SincFilter_mix_conv_2_2[y_sinc_index][x_sinc_index][i][0]);
									src = _mm_madd_epi16(src,filter);
									sum = _mm_add_epi32(sum,src);
									sTemp += nSourceStride;
								}
								_mm_store_si128((__m128i*)result, sum);
							
								acc = result[0] + result[1] + result[2] + result[3];
								res =  pClipTab[(acc + (1<<13)) >> 14];

								
							}
						}
						else{

						if(x_sinc_index < 0 && y_sinc_index < 0){
							
							//printf("!!");
							
							int pos = rx + 2 + x_sinc_index + (nSourceStride<<1) + y_sinc_index*nSourceStride;

							acc = s1[pos]<<2;
							acc += s1[pos+1];
							acc += s1[pos-1];
							acc += s1[pos+nSourceStride];
							acc += s1[pos-nSourceStride];
							res =  pClipTab[(acc + 4) >> 3];
						
						}
						else if(x_sinc_index < 0){
							/*acc = 0;
							sTemp = s1 - nSourceStride;
							for(filterIdx1 = 0; filterIdx1 < 4; filterIdx1++){
								
								int result = 0;
								result += sTemp[rx+2+x_sinc_index]<<2;
								result += sTemp[rx+2+x_sinc_index+1];
								result += sTemp[rx+2+x_sinc_index-1];
								result += sTemp[rx+2+x_sinc_index+nSourceStride];
								result += sTemp[rx+2+x_sinc_index-nSourceStride];
								result =  pClipTab[(result + 4) >> 3];

								acc += result*SincFilter[y_sinc_index][filterIdx1];
								sTemp += nSourceStride;
							}

							res =  pClipTab[(acc + 256) >> 9];*/

							__declspec(align(16)) __m128i sum, src, filter;
							int result[16];
							sum = _mm_setzero_si128();
							sTemp = s1 - 2*nSourceStride + rx + 2 + x_sinc_index - 1;
							
							for(int i = 0;i<6;i++){
								src = _mm_loadl_epi64((__m128i*)sTemp);
								src = _mm_unpacklo_epi8(src, zero);
								filter = _mm_loadl_epi64((__m128i*)&SincFilter_mix_conv_1_1_v[y_sinc_index][i][0]);
								src = _mm_madd_epi16(src,filter);
								sum = _mm_add_epi32(sum,src);
								sTemp += nSourceStride;
							}
							_mm_storel_epi64((__m128i*)result, sum);
							
							acc = result[0] + result[1];
							res =  pClipTab[(acc + 2048) >> 12];
						}
						else if(y_sinc_index < 0){

							__declspec(align(16)) __m128i sum, src, filter;
							int result[16];
							sum = _mm_setzero_si128();
							filterIdx1 = 3 + y_sinc_index;
							sTemp = s1 + (filterIdx1-2) * nSourceStride -2 + rx;
							
							for(int i = 0;i<3;i++){
								src = _mm_loadl_epi64((__m128i*)sTemp);
								src = _mm_unpacklo_epi8(src, zero);
								filter = _mm_loadu_si128((__m128i*)&SincFilter_mix_conv_1_1_h[x_sinc_index][i][0]);
								src = _mm_madd_epi16(src,filter);
								sum = _mm_add_epi32(sum,src);
								sTemp += nSourceStride;
							}
							_mm_store_si128((__m128i*)result, sum);
							
							acc = result[0] + result[1] + result[2];
							res =  pClipTab[(acc + 2048) >> 12];

						}
						else{					
											

							__declspec(align(16)) __m128i sum,  src, filter;
							int result[16];
							sum = _mm_setzero_si128();
							sTemp = s1 - nSourceStride*2 - 2 + rx;
							
							for(filterIdx1 = 0; filterIdx1 < 6; filterIdx1++){
								src = _mm_loadl_epi64((__m128i*)sTemp);
								src = _mm_unpacklo_epi8(src, zero);
								filter = _mm_loadu_si128((__m128i*)&SincFilter_mix_conv_1_1[y_sinc_index][x_sinc_index][filterIdx1][0]);
								src = _mm_madd_epi16(src,filter);
								sum = _mm_add_epi32(sum,src);
								sTemp += nSourceStride;
							}
							_mm_store_si128((__m128i*)result, sum);
							
							acc = result[0] + result[1] + result[2];
							res =  pClipTab[(acc + 2048) >> 12];

						
						}
						//end

						}
					}
					else
					{
						//printf("!!");
						if((rx+1) < m_sw){
							
							a = s1[rx]; b = s1[rx+1];
							c = s2[rx]; d = s2[rx+1];


							res = pClipTab[((a*((1<<BILINEAR_PREC)-p)*((1<<BILINEAR_PREC)-q) + b*p*((1<<BILINEAR_PREC)-q) + c*((1<<BILINEAR_PREC)-p)*q + d*p*q + (1<<29)) >> 30)];



						}
						else		
							res = pClipTab[(s1[rx] - ((q*(s1[rx]-s2[rx]) + (1<<14))>>BILINEAR_PREC))];
					}


					pDest_plane[image][vx] = res;

					x_accum += m_w_inc;

				}

				pDest_plane[image] += nDestStride;

				y_accum += m_h_inc;
			}

		}
	}

	
	for(int image = IMAGE_U; image <= IMAGE_V; image++)
	{

		int m_dh = m_dh_plane[image];
		int m_dw = m_dw_plane[image];
		int m_sw = m_sw_plane[image];
		int m_sh = m_sh_plane[image];
		PBYTE pSource = pSource_plane[image];
		int nSourceStride = nSourceStride_plane[image];
		int nDestStride = nDestStride_plane[image];
		{

			y_accum = 0;
			for (vy = 0; vy < m_dh; vy++)  
			{
				BYTE *s1, *s2, *sTemp;
				int q;
				int ry;
				ry = (y_accum>>BILINEAR_PREC);

				//upper line 
				s1 = pSource + ry * nSourceStride;
				if(ry + 1 < m_sh)
				{
					//lower line
					s2 = s1 + nSourceStride;
				}
				else{
					s2 = s1;
				}
				x_accum = 0;
				q = y_accum & BILINEAR_MASK;


				for (vx = 0; vx < m_dw; vx++)
				{
					BYTE res;
					int rx;
					__int64 a, b, c, d;
					int p;

					rx = x_accum>>BILINEAR_PREC;
					p = x_accum & BILINEAR_MASK;


					{
						if((rx+1) < m_sw && ry + 1 < m_sh){
							
							if(LPFlagX == 2 && LPFlagY == 2){
								
								
								a = s1[rx]; b = s1[rx+1];
								c = s2[rx]; d = s2[rx+1];
											
								
								int p_ = ((p+0)>>11)&15;
								int q_ = ((q+0)>>11)&15;

								if(p_ == 0 && q_ == 0){
									if(rx > 0 && ry > 0){
										acc = (s1[rx] << 2) + s1[rx + 1] + s1[rx - 1] + s1[rx + nSourceStride] + s1[rx - nSourceStride];
										res = pClipTab[(acc + 4)>>3];
									}
									else
										res = a;
								}
								else if(p_ == 0){
									if(rx > 0 && ry > 0){
										a = (s1[rx] << 2) + s1[rx + 1] + s1[rx - 1] + s1[rx + nSourceStride] + s1[rx - nSourceStride];
										c = (s2[rx] << 2) + s2[rx + 1] + s2[rx - 1] + s2[rx + nSourceStride] + s2[rx - nSourceStride];
										a = (a + 4)>>3;
										c = (c + 4)>>3;
									}

									/*int t0, t1;
									t0 = a - ((p * (a-b) + (1<<14) ) >>BILINEAR_PREC);
									t1 = c - ((p * (c-d) + (1<<14) ) >>BILINEAR_PREC);
						

									res = pClipTab[(t0 - ((q*(t0-t1) + (1<<14))>>BILINEAR_PREC))];*/

									res = pClipTab[((a*Tab_P_Q[p_][q_][0] + c*Tab_P_Q[p_][q_][2]  + (1<<7)) >> 8)];
								}
								else if(q_ == 0){
									if(rx > 0 && ry > 0){
										sTemp = s1 + rx - 1 - nSourceStride;
										__declspec(align(16)) __m128i sum, src, filter;
										int result[16];
										sum = _mm_setzero_si128();
							
										for(int i  = 0;i<3;i++){
											src = _mm_loadl_epi64((__m128i*)sTemp);
											src = _mm_unpacklo_epi8(src, zero);
											filter = _mm_loadl_epi64((__m128i*)&Tab_P_Q_mix_conv_1_1_h[p_][i][0]);
											src = _mm_madd_epi16(src,filter);
											sum = _mm_add_epi32(sum,src);
											sTemp += nSourceStride;
										}
										_mm_storel_epi64((__m128i*)result, sum);
							
										acc = result[0] + result[1];
										res = pClipTab[((acc + (1<<10)) >>11)];

									}
									else
										res = pClipTab[((a*Tab_P_Q[p_][q_][0] + b*Tab_P_Q[p_][q_][1]  + (1<<7)) >> 8)];
								}

								else{
									sTemp = s1 + rx - 1 - nSourceStride;
									__declspec(align(16)) __m128i sum, src, filter;
									int result[16];
									sum = _mm_setzero_si128();
							
									for(int i  = 0;i<4;i++){
										src = _mm_loadl_epi64((__m128i*)sTemp);
										src = _mm_unpacklo_epi8(src, zero);
										filter = _mm_loadl_epi64((__m128i*)&Tab_P_Q_mix_conv_1_1[p_][q_][i][0]);
										src = _mm_madd_epi16(src,filter);
										sum = _mm_add_epi32(sum,src);
										sTemp += nSourceStride;
									}
									_mm_storel_epi64((__m128i*)result, sum);
							
									acc = result[0] + result[1];
									res = pClipTab[((acc + (1<<10)) >>11)];
																

								}

							}
							else{
							
								a = s1[rx]; b = s1[rx+1];
								c = s2[rx]; d = s2[rx+1];
							
								int t0, t1;

								t0 = a - ((p * (a-b) + (1<<14) ) >>BILINEAR_PREC);
								t1 = c - ((p * (c-d) + (1<<14) ) >>BILINEAR_PREC);
						

								res = pClipTab[(t0 - ((q*(t0-t1) + (1<<14))>>BILINEAR_PREC))];
							
							
							}
								

						}
						else if(ry + 1 < m_sh){
							a = s1[rx]; b = s1[rx+1];
							c = s2[rx]; d = s2[rx+1];
							
							int t0, t1;

							t0 = a - ((p * (a-b) + (1<<14) ) >>BILINEAR_PREC);
							t1 = c - ((p * (c-d) + (1<<14) ) >>BILINEAR_PREC);
						

							res = pClipTab[(t0 - ((q*(t0-t1) + (1<<14))>>BILINEAR_PREC))];
							
							

						}
						else{
							res = pClipTab[(s1[rx] - ((q*(s1[rx]-s2[rx]) + (1<<14))>>BILINEAR_PREC))];
						}
						

					}


					pDest_plane[image][vx] = res;

					x_accum += m_w_inc;

				}

				pDest_plane[image] += nDestStride;

				y_accum += m_h_inc;
			}

		}
	}

	
}


#else

void CBilinearResampler::resample_bilinear_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw = m_dw;
	int dh = m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//Hoi Ming 4May2009
#define MAX(a, b) ((a>=b)?a:b)
#define MIN(a, b) ((a<=b)?a:b)
#define EDGETHRESHOLD 256
//#define FASTCHECKTHRESHOLD 64
#define FASTCHECKTHRESHOLD 128


	int rx3, rx3mins1, rx3mins2, rx3plus1, rx3plus2, rx3plus3;
	int bilinear_flag = 1;
	int msw3;
	int e[3], f[3], g[3], h[3];
	int temp1[3][4];
	int temp2[3];
	int temp_p, temp_q, color, hv, dir, pixShift, sumDiff;
	int HorizontalEdge, VerticalEdge;
	BYTE *cenPix;

	int BestSumDiff[2][3];
	int pixShitArray[2][3];
	int ppIndex;
	pixShitArray[0][0] = 4*3;
	pixShitArray[0][1] = nSourceStride-4*3;
	pixShitArray[0][2] = nSourceStride+4*3;
	pixShitArray[1][0] = (nSourceStride<<2);
	pixShitArray[1][1] = (nSourceStride<<2)-1*3;
	pixShitArray[1][2] = (nSourceStride<<2)+1*3;
	//end
	
	short sum0, sum1, sum2;
	
	double LPFlag;
	if(m_x_ratio>2.5 || m_y_ratio>2.5){
		LPFlag = 2;
	}
	else if(m_x_ratio>1.25 || m_y_ratio>1.25){
		LPFlag = 1;
	}
	else{
		LPFlag = 0;
	}

	y_accum = 0;
	for (vy = 0; vy < dh; vy++)  
	{
		BYTE *s1, *s2;
		int q;
		int ry;
		ry = (y_accum>>BILINEAR_PREC);

		//upper line 
		s1 = pSource + ry * nSourceStride;
		if(ry + 1 < m_sh)
		{
			//lower line
			s2 = s1 + nSourceStride;
		}
		else
			s2 = s1;
		x_accum = 0;
		q = y_accum & BILINEAR_MASK;

		for (vx = 0; vx < 3 * dw; vx += 3)
		{
			BYTE res[3];
			int rx;
			int a[3], b[3], c[3], d[3];
			int p;
			
			rx = x_accum>>BILINEAR_PREC;
			p = x_accum & BILINEAR_MASK;

			bilinear_flag = 1;

			if( (rx + 8) < m_sw && (ry + 8) < m_sh
				&& (rx - 8) >= 0 && (ry - 8) >= 0 && LPFlag >= 1){

					cenPix = s1 - nSourceStride + 3*(rx-1);

					int fastcheck = absArray[255+cenPix[3+nSourceStride+0]-cenPix[6+nSourceStride+0]]>FASTCHECKTHRESHOLD
						|| absArray[255+cenPix[3+nSourceStride+0]-cenPix[3+(nSourceStride<<1)+0]]>FASTCHECKTHRESHOLD
						|| absArray[255+cenPix[3+nSourceStride+0]-cenPix[6+(nSourceStride<<1)+0]]>FASTCHECKTHRESHOLD;

					if(fastcheck || LPFlag >= 2){

						if(LPFlag < 2){
							for(hv = 0; hv <= 1; hv++){
								for(color = 0; color <=2; color++){
									BestSumDiff[hv][color] = 999999;
									for(dir = 0; dir <= 2; dir++){
										pixShift = pixShitArray[hv][dir];
										sumDiff = absArray[255+cenPix[3+nSourceStride+color]-cenPix[3+nSourceStride+color-pixShift]]+absArray[255+cenPix[3+nSourceStride+color]-cenPix[3+nSourceStride+color+pixShift]]
										+ absArray[255+cenPix[6+nSourceStride+color]-cenPix[6+nSourceStride+color-pixShift]]+absArray[255+cenPix[6+nSourceStride+color]-cenPix[6+nSourceStride+color+pixShift]]
										+ absArray[255+cenPix[3+(nSourceStride<<1)+color]-cenPix[3+(nSourceStride<<1)+color-pixShift]]+absArray[255+cenPix[3+(nSourceStride<<1)+color]-cenPix[3+(nSourceStride<<1)+color+pixShift]]
										+ absArray[255+cenPix[6+(nSourceStride<<1)+color]-cenPix[6+(nSourceStride<<1)+color-pixShift]]+absArray[255+cenPix[6+(nSourceStride<<1)+color]-cenPix[6+(nSourceStride<<1)+color+pixShift]];

										if(sumDiff < BestSumDiff[hv][color]){
											BestSumDiff[hv][color] = sumDiff;
										}
									}
								}
							}
							HorizontalEdge = (BestSumDiff[0][0]<EDGETHRESHOLD
								&& BestSumDiff[0][1]<EDGETHRESHOLD
								&& BestSumDiff[0][2]<EDGETHRESHOLD)?1:0;
							VerticalEdge = (BestSumDiff[1][0]<EDGETHRESHOLD
								&& BestSumDiff[1][1]<EDGETHRESHOLD
								&& BestSumDiff[1][2]<EDGETHRESHOLD)?1:0;
						}
						if((HorizontalEdge+VerticalEdge)==1 || LPFlag >= 2){
							//turn off bilinear filter
							bilinear_flag = 0;
							//start cubic-linear filter

							//Strong Gaussian filter and then followed by a bilinear filter

							rx3 = rx+rx+rx;
							rx3mins1 = rx3-3;
							rx3plus1 = rx3+3;
							rx3mins2 = rx3-6;
							rx3plus2 = rx3+6;
							rx3plus3 = rx3+6;

							if(LPFlag >= 2){
								
								a[0] = ( ((s1[rx3]<<5)+(s1[rx3]<<3)+(s1[rx3]<<2))
									+(((s1[rx3mins1]+s1[rx3-nSourceStride]+s1[rx3plus1]+s1[rx3+nSourceStride])<<4)+((s1[rx3mins1]+s1[rx3-nSourceStride]+s1[rx3plus1]+s1[rx3+nSourceStride])<<3))
									+(((s1[rx3mins1-nSourceStride]+s1[rx3plus1-nSourceStride]+s1[rx3mins1+nSourceStride]+s1[rx3plus1+nSourceStride])<<3)+((s1[rx3mins1-nSourceStride]+s1[rx3plus1-nSourceStride]+s1[rx3mins1+nSourceStride]+s1[rx3plus1+nSourceStride])<<2))
									+((s1[rx3mins2]+s1[rx3plus2]+s1[rx3-(nSourceStride<<1)]+s1[rx3+(nSourceStride<<1)])<<3)
									+((s1[rx3mins2-nSourceStride]+s1[rx3mins2+nSourceStride]+s1[rx3plus2-nSourceStride]+s1[rx3plus2+nSourceStride]+s1[rx3mins1-(nSourceStride<<1)]+s1[rx3plus1-(nSourceStride<<1)]+s1[rx3mins1+(nSourceStride<<1)]+s1[rx3plus1+(nSourceStride<<1)])<<2)
									+(s1[rx3mins2-(nSourceStride<<1)]+s1[rx3mins2+(nSourceStride<<1)]+s1[rx3plus2-(nSourceStride<<1)]+s1[rx3plus2+(nSourceStride<<1)]) + 128)>>8;
								a[1] = ( ((s1[rx3+1]<<5)+(s1[rx3+1]<<3)+(s1[rx3+1]<<2))
									+(((s1[rx3mins1+1]+s1[rx3-nSourceStride+1]+s1[rx3plus1+1]+s1[rx3+nSourceStride+1])<<4)+((s1[rx3mins1+1]+s1[rx3-nSourceStride+1]+s1[rx3plus1+1]+s1[rx3+nSourceStride+1])<<3))
									+(((s1[rx3mins1-nSourceStride+1]+s1[rx3plus1-nSourceStride+1]+s1[rx3mins1+nSourceStride+1]+s1[rx3plus1+nSourceStride+1])<<3)+((s1[rx3mins1-nSourceStride+1]+s1[rx3plus1-nSourceStride+1]+s1[rx3mins1+nSourceStride+1]+s1[rx3plus1+nSourceStride+1])<<2))
									+((s1[rx3mins2+1]+s1[rx3plus2+1]+s1[rx3-(nSourceStride<<1)+1]+s1[rx3+(nSourceStride<<1)+1])<<3)
									+((s1[rx3mins2-nSourceStride+1]+s1[rx3mins2+nSourceStride+1]+s1[rx3plus2-nSourceStride+1]+s1[rx3plus2+nSourceStride+1]+s1[rx3mins1-(nSourceStride<<1)+1]+s1[rx3plus1-(nSourceStride<<1)+1]+s1[rx3mins1+(nSourceStride<<1)+1]+s1[rx3plus1+(nSourceStride<<1)+1])<<2)
									+(s1[rx3mins2-(nSourceStride<<1)+1]+s1[rx3mins2+(nSourceStride<<1)+1]+s1[rx3plus2-(nSourceStride<<1)+1]+s1[rx3plus2+(nSourceStride<<1)+1]) + 128)>>8;
								a[2] = ( ((s1[rx3+2]<<5)+(s1[rx3+2]<<3)+(s1[rx3+2]<<2))
									+(((s1[rx3mins1+2]+s1[rx3-nSourceStride+2]+s1[rx3plus1+2]+s1[rx3+nSourceStride+2])<<4)+((s1[rx3mins1+2]+s1[rx3-nSourceStride+2]+s1[rx3plus1+2]+s1[rx3+nSourceStride+2])<<3))
									+(((s1[rx3mins1-nSourceStride+2]+s1[rx3plus1-nSourceStride+2]+s1[rx3mins1+nSourceStride+2]+s1[rx3plus1+nSourceStride+2])<<3)+((s1[rx3mins1-nSourceStride+2]+s1[rx3plus1-nSourceStride+2]+s1[rx3mins1+nSourceStride+2]+s1[rx3plus1+nSourceStride+2])<<2))
									+((s1[rx3mins2+2]+s1[rx3plus2+2]+s1[rx3-(nSourceStride<<1)+2]+s1[rx3+(nSourceStride<<1)+2])<<3)
									+((s1[rx3mins2-nSourceStride+2]+s1[rx3mins2+nSourceStride+2]+s1[rx3plus2-nSourceStride+2]+s1[rx3plus2+nSourceStride+2]+s1[rx3mins1-(nSourceStride<<1)+2]+s1[rx3plus1-(nSourceStride<<1)+2]+s1[rx3mins1+(nSourceStride<<1)+2]+s1[rx3plus1+(nSourceStride<<1)+2])<<2)
									+(s1[rx3mins2-(nSourceStride<<1)+2]+s1[rx3mins2+(nSourceStride<<1)+2]+s1[rx3plus2-(nSourceStride<<1)+2]+s1[rx3plus2+(nSourceStride<<1)+2]) + 128)>>8;

								b[0] = ( ((s1[rx3plus1]<<5)+(s1[rx3plus1]<<3)+(s1[rx3plus1]<<2))
									+(((s1[rx3]+s1[rx3plus1-nSourceStride]+s1[rx3plus2]+s1[rx3plus1+nSourceStride])<<4)+((s1[rx3]+s1[rx3plus1-nSourceStride]+s1[rx3plus2]+s1[rx3plus1+nSourceStride])<<3))
									+(((s1[rx3-nSourceStride]+s1[rx3plus2-nSourceStride]+s1[rx3+nSourceStride]+s1[rx3plus2+nSourceStride])<<3)+((s1[rx3-nSourceStride]+s1[rx3plus2-nSourceStride]+s1[rx3+nSourceStride]+s1[rx3plus2+nSourceStride])<<2))
									+((s1[rx3mins1]+s1[rx3plus3]+s1[rx3plus1-(nSourceStride<<1)]+s1[rx3plus1+(nSourceStride<<1)])<<3)
									+((s1[rx3mins1-nSourceStride]+s1[rx3mins1+nSourceStride]+s1[rx3plus3-nSourceStride]+s1[rx3plus3+nSourceStride]+s1[rx3-(nSourceStride<<1)]+s1[rx3plus2-(nSourceStride<<1)]+s1[rx3+(nSourceStride<<1)]+s1[rx3plus2+(nSourceStride<<1)])<<2)
									+(s1[rx3mins1-(nSourceStride<<1)]+s1[rx3mins1+(nSourceStride<<1)]+s1[rx3plus3-(nSourceStride<<1)]+s1[rx3plus3+(nSourceStride<<1)]) + 128)>>8;
								b[1] = ( ((s1[rx3plus1+1]<<5)+(s1[rx3plus1+1]<<3)+(s1[rx3plus1+1]<<2))
									+(((s1[rx3+1]+s1[rx3plus1-nSourceStride+1]+s1[rx3plus2+1]+s1[rx3plus1+nSourceStride+1])<<4)+((s1[rx3+1]+s1[rx3plus1-nSourceStride+1]+s1[rx3plus2+1]+s1[rx3plus1+nSourceStride+1])<<3))
									+(((s1[rx3-nSourceStride+1]+s1[rx3plus2-nSourceStride+1]+s1[rx3+nSourceStride+1]+s1[rx3plus2+nSourceStride+1])<<3)+((s1[rx3-nSourceStride+1]+s1[rx3plus2-nSourceStride+1]+s1[rx3+nSourceStride+1]+s1[rx3plus2+nSourceStride+1])<<2))
									+((s1[rx3mins1+1]+s1[rx3plus3+1]+s1[rx3plus1-(nSourceStride<<1)+1]+s1[rx3plus1+(nSourceStride<<1)+1])<<3)
									+((s1[rx3mins1-nSourceStride+1]+s1[rx3mins1+nSourceStride+1]+s1[rx3plus3-nSourceStride+1]+s1[rx3plus3+nSourceStride+1]+s1[rx3-(nSourceStride<<1)+1]+s1[rx3plus2-(nSourceStride<<1)+1]+s1[rx3+(nSourceStride<<1)+1]+s1[rx3plus2+(nSourceStride<<1)+1])<<2)
									+(s1[rx3mins1-(nSourceStride<<1)+1]+s1[rx3mins1+(nSourceStride<<1)+1]+s1[rx3plus3-(nSourceStride<<1)+1]+s1[rx3plus3+(nSourceStride<<1)+1]) + 128)>>8;
								b[2] = ( ((s1[rx3plus1+2]<<5)+(s1[rx3plus1+2]<<3)+(s1[rx3plus1+2]<<2))
									+(((s1[rx3+2]+s1[rx3plus1-nSourceStride+2]+s1[rx3plus2+2]+s1[rx3plus1+nSourceStride+2])<<4)+((s1[rx3+2]+s1[rx3plus1-nSourceStride+2]+s1[rx3plus2+2]+s1[rx3plus1+nSourceStride+2])<<3))
									+(((s1[rx3-nSourceStride+2]+s1[rx3plus2-nSourceStride+2]+s1[rx3+nSourceStride+2]+s1[rx3plus2+nSourceStride+2])<<3)+((s1[rx3-nSourceStride+2]+s1[rx3plus2-nSourceStride+2]+s1[rx3+nSourceStride+2]+s1[rx3plus2+nSourceStride+2])<<2))
									+((s1[rx3mins1+2]+s1[rx3plus3+2]+s1[rx3plus1-(nSourceStride<<1)+2]+s1[rx3plus1+(nSourceStride<<1)+2])<<3)
									+((s1[rx3mins1-nSourceStride+2]+s1[rx3mins1+nSourceStride+2]+s1[rx3plus3-nSourceStride+2]+s1[rx3plus3+nSourceStride+2]+s1[rx3-(nSourceStride<<1)+2]+s1[rx3plus2-(nSourceStride<<1)+2]+s1[rx3+(nSourceStride<<1)+2]+s1[rx3plus2+(nSourceStride<<1)+2])<<2)
									+(s1[rx3mins1-(nSourceStride<<1)+2]+s1[rx3mins1+(nSourceStride<<1)+2]+s1[rx3plus3-(nSourceStride<<1)+2]+s1[rx3plus3+(nSourceStride<<1)+2]) + 128)>>8;
							
								c[0] = ( ((s2[rx3]<<5)+(s2[rx3]<<3)+(s2[rx3]<<2))
									+(((s2[rx3mins1]+s2[rx3-nSourceStride]+s2[rx3plus1]+s2[rx3+nSourceStride])<<4)+((s2[rx3mins1]+s2[rx3-nSourceStride]+s2[rx3plus1]+s2[rx3+nSourceStride])<<3))
									+(((s2[rx3mins1-nSourceStride]+s2[rx3plus1-nSourceStride]+s2[rx3mins1+nSourceStride]+s2[rx3plus1+nSourceStride])<<3)+((s2[rx3mins1-nSourceStride]+s2[rx3plus1-nSourceStride]+s2[rx3mins1+nSourceStride]+s2[rx3plus1+nSourceStride])<<2))
									+((s2[rx3mins2]+s2[rx3plus2]+s2[rx3-(nSourceStride<<1)]+s2[rx3+(nSourceStride<<1)])<<3)
									+((s2[rx3mins2-nSourceStride]+s2[rx3mins2+nSourceStride]+s2[rx3plus2-nSourceStride]+s2[rx3plus2+nSourceStride]+s2[rx3mins1-(nSourceStride<<1)]+s2[rx3plus1-(nSourceStride<<1)]+s2[rx3mins1+(nSourceStride<<1)]+s2[rx3plus1+(nSourceStride<<1)])<<2)
									+(s2[rx3mins2-(nSourceStride<<1)]+s2[rx3mins2+(nSourceStride<<1)]+s2[rx3plus2-(nSourceStride<<1)]+s2[rx3plus2+(nSourceStride<<1)]) + 128)>>8;
								c[1] = ( ((s2[rx3+1]<<5)+(s2[rx3+1]<<3)+(s2[rx3+1]<<2))
									+(((s2[rx3mins1+1]+s2[rx3-nSourceStride+1]+s2[rx3plus1+1]+s2[rx3+nSourceStride+1])<<4)+((s2[rx3mins1+1]+s2[rx3-nSourceStride+1]+s2[rx3plus1+1]+s2[rx3+nSourceStride+1])<<3))
									+(((s2[rx3mins1-nSourceStride+1]+s2[rx3plus1-nSourceStride+1]+s2[rx3mins1+nSourceStride+1]+s2[rx3plus1+nSourceStride+1])<<3)+((s2[rx3mins1-nSourceStride+1]+s2[rx3plus1-nSourceStride+1]+s2[rx3mins1+nSourceStride+1]+s2[rx3plus1+nSourceStride+1])<<2))
									+((s2[rx3mins2+1]+s2[rx3plus2+1]+s2[rx3-(nSourceStride<<1)+1]+s2[rx3+(nSourceStride<<1)+1])<<3)
									+((s2[rx3mins2-nSourceStride+1]+s2[rx3mins2+nSourceStride+1]+s2[rx3plus2-nSourceStride+1]+s2[rx3plus2+nSourceStride+1]+s2[rx3mins1-(nSourceStride<<1)+1]+s2[rx3plus1-(nSourceStride<<1)+1]+s2[rx3mins1+(nSourceStride<<1)+1]+s2[rx3plus1+(nSourceStride<<1)+1])<<2)
									+(s2[rx3mins2-(nSourceStride<<1)+1]+s2[rx3mins2+(nSourceStride<<1)+1]+s2[rx3plus2-(nSourceStride<<1)+1]+s2[rx3plus2+(nSourceStride<<1)+1]) + 128)>>8;
								c[2] = ( ((s2[rx3+2]<<5)+(s2[rx3+2]<<3)+(s2[rx3+2]<<2))
									+(((s2[rx3mins1+2]+s2[rx3-nSourceStride+2]+s2[rx3plus1+2]+s2[rx3+nSourceStride+2])<<4)+((s2[rx3mins1+2]+s2[rx3-nSourceStride+2]+s2[rx3plus1+2]+s2[rx3+nSourceStride+2])<<3))
									+(((s2[rx3mins1-nSourceStride+2]+s2[rx3plus1-nSourceStride+2]+s2[rx3mins1+nSourceStride+2]+s2[rx3plus1+nSourceStride+2])<<3)+((s2[rx3mins1-nSourceStride+2]+s2[rx3plus1-nSourceStride+2]+s2[rx3mins1+nSourceStride+2]+s2[rx3plus1+nSourceStride+2])<<2))
									+((s2[rx3mins2+2]+s2[rx3plus2+2]+s2[rx3-(nSourceStride<<1)+2]+s2[rx3+(nSourceStride<<1)+2])<<3)
									+((s2[rx3mins2-nSourceStride+2]+s2[rx3mins2+nSourceStride+2]+s2[rx3plus2-nSourceStride+2]+s2[rx3plus2+nSourceStride+2]+s2[rx3mins1-(nSourceStride<<1)+2]+s2[rx3plus1-(nSourceStride<<1)+2]+s2[rx3mins1+(nSourceStride<<1)+2]+s2[rx3plus1+(nSourceStride<<1)+2])<<2)
									+(s2[rx3mins2-(nSourceStride<<1)+2]+s2[rx3mins2+(nSourceStride<<1)+2]+s2[rx3plus2-(nSourceStride<<1)+2]+s2[rx3plus2+(nSourceStride<<1)+2]) + 128)>>8;

								d[0] = ( ((s2[rx3plus1]<<5)+(s2[rx3plus1]<<3)+(s2[rx3plus1]<<2))
									+(((s2[rx3]+s2[rx3plus1-nSourceStride]+s2[rx3plus2]+s2[rx3plus1+nSourceStride])<<4)+((s2[rx3]+s2[rx3plus1-nSourceStride]+s2[rx3plus2]+s2[rx3plus1+nSourceStride])<<3))
									+(((s2[rx3-nSourceStride]+s2[rx3plus2-nSourceStride]+s2[rx3+nSourceStride]+s2[rx3plus2+nSourceStride])<<3)+((s2[rx3-nSourceStride]+s2[rx3plus2-nSourceStride]+s2[rx3+nSourceStride]+s2[rx3plus2+nSourceStride])<<2))
									+((s2[rx3mins1]+s2[rx3plus3]+s2[rx3plus1-(nSourceStride<<1)]+s2[rx3plus1+(nSourceStride<<1)])<<3)
									+((s2[rx3mins1-nSourceStride]+s2[rx3mins1+nSourceStride]+s2[rx3plus3-nSourceStride]+s2[rx3plus3+nSourceStride]+s2[rx3-(nSourceStride<<1)]+s2[rx3plus2-(nSourceStride<<1)]+s2[rx3+(nSourceStride<<1)]+s2[rx3plus2+(nSourceStride<<1)])<<2)
									+(s2[rx3mins1-(nSourceStride<<1)]+s2[rx3mins1+(nSourceStride<<1)]+s2[rx3plus3-(nSourceStride<<1)]+s2[rx3plus3+(nSourceStride<<1)]) + 128)>>8;
								d[1] = ( ((s2[rx3plus1+1]<<5)+(s2[rx3plus1+1]<<3)+(s2[rx3plus1+1]<<2))
									+(((s2[rx3+1]+s2[rx3plus1-nSourceStride+1]+s2[rx3plus2+1]+s2[rx3plus1+nSourceStride+1])<<4)+((s2[rx3+1]+s2[rx3plus1-nSourceStride+1]+s2[rx3plus2+1]+s2[rx3plus1+nSourceStride+1])<<3))
									+(((s2[rx3-nSourceStride+1]+s2[rx3plus2-nSourceStride+1]+s2[rx3+nSourceStride+1]+s2[rx3plus2+nSourceStride+1])<<3)+((s2[rx3-nSourceStride+1]+s2[rx3plus2-nSourceStride+1]+s2[rx3+nSourceStride+1]+s2[rx3plus2+nSourceStride+1])<<2))
									+((s2[rx3mins1+1]+s2[rx3plus3+1]+s2[rx3plus1-(nSourceStride<<1)+1]+s2[rx3plus1+(nSourceStride<<1)+1])<<3)
									+((s2[rx3mins1-nSourceStride+1]+s2[rx3mins1+nSourceStride+1]+s2[rx3plus3-nSourceStride+1]+s2[rx3plus3+nSourceStride+1]+s2[rx3-(nSourceStride<<1)+1]+s2[rx3plus2-(nSourceStride<<1)+1]+s2[rx3+(nSourceStride<<1)+1]+s2[rx3plus2+(nSourceStride<<1)+1])<<2)
									+(s2[rx3mins1-(nSourceStride<<1)+1]+s2[rx3mins1+(nSourceStride<<1)+1]+s2[rx3plus3-(nSourceStride<<1)+1]+s2[rx3plus3+(nSourceStride<<1)+1]) + 128)>>8;
								d[2] = ( ((s2[rx3plus1+2]<<5)+(s2[rx3plus1+2]<<3)+(s2[rx3plus1+2]<<2))
									+(((s2[rx3+2]+s2[rx3plus1-nSourceStride+2]+s2[rx3plus2+2]+s2[rx3plus1+nSourceStride+2])<<4)+((s2[rx3+2]+s2[rx3plus1-nSourceStride+2]+s2[rx3plus2+2]+s2[rx3plus1+nSourceStride+2])<<3))
									+(((s2[rx3-nSourceStride+2]+s2[rx3plus2-nSourceStride+2]+s2[rx3+nSourceStride+2]+s2[rx3plus2+nSourceStride+2])<<3)+((s2[rx3-nSourceStride+2]+s2[rx3plus2-nSourceStride+2]+s2[rx3+nSourceStride+2]+s2[rx3plus2+nSourceStride+2])<<2))
									+((s2[rx3mins1+2]+s2[rx3plus3+2]+s2[rx3plus1-(nSourceStride<<1)+2]+s2[rx3plus1+(nSourceStride<<1)+2])<<3)
									+((s2[rx3mins1-nSourceStride+2]+s2[rx3mins1+nSourceStride+2]+s2[rx3plus3-nSourceStride+2]+s2[rx3plus3+nSourceStride+2]+s2[rx3-(nSourceStride<<1)+2]+s2[rx3plus2-(nSourceStride<<1)+2]+s2[rx3+(nSourceStride<<1)+2]+s2[rx3plus2+(nSourceStride<<1)+2])<<2)
									+(s2[rx3mins1-(nSourceStride<<1)+2]+s2[rx3mins1+(nSourceStride<<1)+2]+s2[rx3plus3-(nSourceStride<<1)+2]+s2[rx3plus3+(nSourceStride<<1)+2]) + 128)>>8;

							}							
							else if(LPFlag == 1){
								a[0] = ((s1[rx3]<<2)+s1[rx3mins1-nSourceStride]+(s1[rx3mins1]<<1)+s1[rx3mins1+nSourceStride]+(s1[rx3-nSourceStride]<<1)+(s1[rx3+nSourceStride]<<1)+s1[rx3plus1-nSourceStride]+(s1[rx3plus1]<<1)+s1[rx3plus1+nSourceStride]+8)>>4;
								a[1] = ((s1[rx3+1]<<2)+s1[rx3mins1-nSourceStride+1]+(s1[rx3mins1+1]<<1)+s1[rx3mins1+nSourceStride+1]+(s1[rx3-nSourceStride+1]<<1)+(s1[rx3+nSourceStride+1]<<1)+s1[rx3plus1-nSourceStride+1]+(s1[rx3plus1+1]<<1)+s1[rx3plus1+nSourceStride+1]+8)>>4;
								a[2] = ((s1[rx3+2]<<2)+s1[rx3mins1-nSourceStride+2]+(s1[rx3mins1+2]<<1)+s1[rx3mins1+nSourceStride+2]+(s1[rx3-nSourceStride+2]<<1)+(s1[rx3+nSourceStride+2]<<1)+s1[rx3plus1-nSourceStride+2]+(s1[rx3plus1+2]<<1)+s1[rx3plus1+nSourceStride+2]+8)>>4;
								b[0] = ((s1[rx3plus1]<<2)+s1[rx3-nSourceStride]+(s1[rx3]<<1)+s1[rx3+nSourceStride]+(s1[rx3plus1-nSourceStride]<<1)+(s1[rx3plus1+nSourceStride]<<1)+s1[rx3plus2-nSourceStride]+(s1[rx3plus2]<<1)+s1[rx3plus2+nSourceStride]+8)>>4;
								b[1] = ((s1[rx3plus1+1]<<2)+s1[rx3-nSourceStride+1]+(s1[rx3+1]<<1)+s1[rx3+nSourceStride+1]+(s1[rx3plus1-nSourceStride+1]<<1)+(s1[rx3plus1+nSourceStride+1]<<1)+s1[rx3plus2-nSourceStride+1]+(s1[rx3plus2+1]<<1)+s1[rx3plus2+nSourceStride+1]+8)>>4;
								b[2] = ((s1[rx3plus1+2]<<2)+s1[rx3-nSourceStride+2]+(s1[rx3+2]<<1)+s1[rx3+nSourceStride+2]+(s1[rx3plus1-nSourceStride+2]<<1)+(s1[rx3plus1+nSourceStride+2]<<1)+s1[rx3plus2-nSourceStride+2]+(s1[rx3plus2+2]<<1)+s1[rx3plus2+nSourceStride+2]+8)>>4;
								c[0] = ((s2[rx3]<<2)+s2[rx3mins1-nSourceStride]+(s2[rx3mins1]<<1)+s2[rx3mins1+nSourceStride]+(s2[rx3-nSourceStride]<<1)+(s2[rx3+nSourceStride]<<1)+s2[rx3plus1-nSourceStride]+(s2[rx3plus1]<<1)+s2[rx3plus1+nSourceStride]+8)>>4;
								c[1] = ((s2[rx3+1]<<2)+s2[rx3mins1-nSourceStride+1]+(s2[rx3mins1+1]<<1)+s2[rx3mins1+nSourceStride+1]+(s2[rx3-nSourceStride+1]<<1)+(s2[rx3+nSourceStride+1]<<1)+s2[rx3plus1-nSourceStride+1]+(s2[rx3plus1+1]<<1)+s2[rx3plus1+nSourceStride+1]+8)>>4;
								c[2] = ((s2[rx3+2]<<2)+s2[rx3mins1-nSourceStride+2]+(s2[rx3mins1+2]<<1)+s2[rx3mins1+nSourceStride+2]+(s2[rx3-nSourceStride+2]<<1)+(s2[rx3+nSourceStride+2]<<1)+s2[rx3plus1-nSourceStride+2]+(s2[rx3plus1+2]<<1)+s2[rx3plus1+nSourceStride+2]+8)>>4;
								d[0] = ((s2[rx3plus1]<<2)+s2[rx3-nSourceStride]+(s2[rx3]<<1)+s2[rx3+nSourceStride]+(s2[rx3plus1-nSourceStride]<<1)+(s2[rx3plus1+nSourceStride]<<1)+s2[rx3plus2-nSourceStride]+(s2[rx3plus2]<<1)+s2[rx3plus2+nSourceStride]+8)>>4;
								d[1] = ((s2[rx3plus1+1]<<2)+s2[rx3-nSourceStride+1]+(s2[rx3+1]<<1)+s2[rx3+nSourceStride+1]+(s2[rx3plus1-nSourceStride+1]<<1)+(s2[rx3plus1+nSourceStride+1]<<1)+s2[rx3plus2-nSourceStride+1]+(s2[rx3plus2+1]<<1)+s2[rx3plus2+nSourceStride+1]+8)>>4;
								d[2] = ((s2[rx3plus1+2]<<2)+s2[rx3-nSourceStride+2]+(s2[rx3+2]<<1)+s2[rx3+nSourceStride+2]+(s2[rx3plus1-nSourceStride+2]<<1)+(s2[rx3plus1+nSourceStride+2]<<1)+s2[rx3plus2-nSourceStride+2]+(s2[rx3plus2+2]<<1)+s2[rx3plus2+nSourceStride+2]+8)>>4;
							}
							else{
								rx3 = rx+rx+rx;
								rx3plus1 = rx3+3;
								a[0] = s1[rx3];
								a[1] = s1[rx3+1];
								a[2] = s1[rx3+2];
								b[0] = s1[rx3plus1];
								b[1] = s1[rx3plus1+1];
								b[2] = s1[rx3plus1+2];
								c[0] = s2[rx3];
								c[1] = s2[rx3+1];
								c[2] = s2[rx3+2];
								d[0] = s2[rx3plus1];
								d[1] = s2[rx3plus1+1];
								d[2] = s2[rx3plus1+2];
							}

							int t0[3], t1[3];

							t0[0] = a[0] - ((p * (a[0]-b[0]) + (1<<14) ) >>BILINEAR_PREC);
							t0[1] = a[1] - ((p * (a[1]-b[1]) + (1<<14) ) >>BILINEAR_PREC);
							t0[2] = a[2] - ((p * (a[2]-b[2]) + (1<<14) ) >>BILINEAR_PREC);
							t1[0] = c[0] - ((p * (c[0]-d[0]) + (1<<14) ) >>BILINEAR_PREC);
							t1[1] = c[1] - ((p * (c[1]-d[1]) + (1<<14) ) >>BILINEAR_PREC);
							t1[2] = c[2] - ((p * (c[2]-d[2]) + (1<<14) ) >>BILINEAR_PREC);

							res[0] = t0[0] - ((q*(t0[0]-t1[0]) + (1<<14))>>BILINEAR_PREC);
							res[1] = t0[1] - ((q*(t0[1]-t1[1]) + (1<<14))>>BILINEAR_PREC);
							res[2] = t0[2] - ((q*(t0[2]-t1[2]) + (1<<14))>>BILINEAR_PREC);
							//end

						}

					}

			} //if( (rx + 8) < m_sw && (ry + 8) < m_sh)

			if(bilinear_flag){ //Hoi Ming test 4May2009

				rx3 = rx+rx+rx;
				rx3mins1 = rx3-3;
				rx3plus1 = rx3+3;
				rx3plus2 = rx3+6;
				if( (rx+2) < m_sw && (rx-2) >= 0 && (ry+2) < m_sh && (ry-2) >= 0)
				{
					if(LPFlag >= 2){
						a[0] = ((s1[rx3]<<2)+s1[rx3mins1-nSourceStride]+(s1[rx3mins1]<<1)+s1[rx3mins1+nSourceStride]+(s1[rx3-nSourceStride]<<1)+(s1[rx3+nSourceStride]<<1)+s1[rx3plus1-nSourceStride]+(s1[rx3plus1]<<1)+s1[rx3plus1+nSourceStride]+8)>>4;
						a[1] = ((s1[rx3+1]<<2)+s1[rx3mins1-nSourceStride+1]+(s1[rx3mins1+1]<<1)+s1[rx3mins1+nSourceStride+1]+(s1[rx3-nSourceStride+1]<<1)+(s1[rx3+nSourceStride+1]<<1)+s1[rx3plus1-nSourceStride+1]+(s1[rx3plus1+1]<<1)+s1[rx3plus1+nSourceStride+1]+8)>>4;
						a[2] = ((s1[rx3+2]<<2)+s1[rx3mins1-nSourceStride+2]+(s1[rx3mins1+2]<<1)+s1[rx3mins1+nSourceStride+2]+(s1[rx3-nSourceStride+2]<<1)+(s1[rx3+nSourceStride+2]<<1)+s1[rx3plus1-nSourceStride+2]+(s1[rx3plus1+2]<<1)+s1[rx3plus1+nSourceStride+2]+8)>>4;
						b[0] = ((s1[rx3plus1]<<2)+s1[rx3-nSourceStride]+(s1[rx3]<<1)+s1[rx3+nSourceStride]+(s1[rx3plus1-nSourceStride]<<1)+(s1[rx3plus1+nSourceStride]<<1)+s1[rx3plus2-nSourceStride]+(s1[rx3plus2]<<1)+s1[rx3plus2+nSourceStride]+8)>>4;
						b[1] = ((s1[rx3plus1+1]<<2)+s1[rx3-nSourceStride+1]+(s1[rx3+1]<<1)+s1[rx3+nSourceStride+1]+(s1[rx3plus1-nSourceStride+1]<<1)+(s1[rx3plus1+nSourceStride+1]<<1)+s1[rx3plus2-nSourceStride+1]+(s1[rx3plus2+1]<<1)+s1[rx3plus2+nSourceStride+1]+8)>>4;
						b[2] = ((s1[rx3plus1+2]<<2)+s1[rx3-nSourceStride+2]+(s1[rx3+2]<<1)+s1[rx3+nSourceStride+2]+(s1[rx3plus1-nSourceStride+2]<<1)+(s1[rx3plus1+nSourceStride+2]<<1)+s1[rx3plus2-nSourceStride+2]+(s1[rx3plus2+2]<<1)+s1[rx3plus2+nSourceStride+2]+8)>>4;
						c[0] = ((s2[rx3]<<2)+s2[rx3mins1-nSourceStride]+(s2[rx3mins1]<<1)+s2[rx3mins1+nSourceStride]+(s2[rx3-nSourceStride]<<1)+(s2[rx3+nSourceStride]<<1)+s2[rx3plus1-nSourceStride]+(s2[rx3plus1]<<1)+s2[rx3plus1+nSourceStride]+8)>>4;
						c[1] = ((s2[rx3+1]<<2)+s2[rx3mins1-nSourceStride+1]+(s2[rx3mins1+1]<<1)+s2[rx3mins1+nSourceStride+1]+(s2[rx3-nSourceStride+1]<<1)+(s2[rx3+nSourceStride+1]<<1)+s2[rx3plus1-nSourceStride+1]+(s2[rx3plus1+1]<<1)+s2[rx3plus1+nSourceStride+1]+8)>>4;
						c[2] = ((s2[rx3+2]<<2)+s2[rx3mins1-nSourceStride+2]+(s2[rx3mins1+2]<<1)+s2[rx3mins1+nSourceStride+2]+(s2[rx3-nSourceStride+2]<<1)+(s2[rx3+nSourceStride+2]<<1)+s2[rx3plus1-nSourceStride+2]+(s2[rx3plus1+2]<<1)+s2[rx3plus1+nSourceStride+2]+8)>>4;
						d[0] = ((s2[rx3plus1]<<2)+s2[rx3-nSourceStride]+(s2[rx3]<<1)+s2[rx3+nSourceStride]+(s2[rx3plus1-nSourceStride]<<1)+(s2[rx3plus1+nSourceStride]<<1)+s2[rx3plus2-nSourceStride]+(s2[rx3plus2]<<1)+s2[rx3plus2+nSourceStride]+8)>>4;
						d[1] = ((s2[rx3plus1+1]<<2)+s2[rx3-nSourceStride+1]+(s2[rx3+1]<<1)+s2[rx3+nSourceStride+1]+(s2[rx3plus1-nSourceStride+1]<<1)+(s2[rx3plus1+nSourceStride+1]<<1)+s2[rx3plus2-nSourceStride+1]+(s2[rx3plus2+1]<<1)+s2[rx3plus2+nSourceStride+1]+8)>>4;
						d[2] = ((s2[rx3plus1+2]<<2)+s2[rx3-nSourceStride+2]+(s2[rx3+2]<<1)+s2[rx3+nSourceStride+2]+(s2[rx3plus1-nSourceStride+2]<<1)+(s2[rx3plus1+nSourceStride+2]<<1)+s2[rx3plus2-nSourceStride+2]+(s2[rx3plus2+2]<<1)+s2[rx3plus2+nSourceStride+2]+8)>>4;
					}
					else if(LPFlag == 1){
						a[0] = ((s1[rx3]<<2)+(s1[rx3mins1])+(s1[rx3-nSourceStride])+(s1[rx3+nSourceStride])+(s1[rx3plus1])+4)>>3;
						a[1] = ((s1[rx3+1]<<2)+(s1[rx3mins1+1])+(s1[rx3-nSourceStride+1])+(s1[rx3+nSourceStride+1])+(s1[rx3plus1+1])+4)>>3;
						a[2] = ((s1[rx3+2]<<2)+(s1[rx3mins1+2])+(s1[rx3-nSourceStride+2])+(s1[rx3+nSourceStride+2])+(s1[rx3plus1+2])+4)>>3;
						b[0] = ((s1[rx3plus1]<<2)+(s1[rx3])+(s1[rx3plus1-nSourceStride])+(s1[rx3plus1+nSourceStride])+(s1[rx3plus2])+4)>>3;
						b[1] = ((s1[rx3plus1+1]<<2)+(s1[rx3+1])+(s1[rx3plus1-nSourceStride+1])+(s1[rx3plus1+nSourceStride+1])+(s1[rx3plus2+1])+4)>>3;
						b[2] = ((s1[rx3plus1+2]<<2)+(s1[rx3+2])+(s1[rx3plus1-nSourceStride+2])+(s1[rx3plus1+nSourceStride+2])+(s1[rx3plus2+2])+4)>>3;
						c[0] = ((s2[rx3]<<2)+(s2[rx3mins1])+(s2[rx3-nSourceStride])+(s2[rx3+nSourceStride])+(s2[rx3plus1])+4)>>3;
						c[1] = ((s2[rx3+1]<<2)+(s2[rx3mins1+1])+(s2[rx3-nSourceStride+1])+(s2[rx3+nSourceStride+1])+(s2[rx3plus1+1])+4)>>3;
						c[2] = ((s2[rx3+2]<<2)+(s2[rx3mins1+2])+(s2[rx3-nSourceStride+2])+(s2[rx3+nSourceStride+2])+(s2[rx3plus1+2])+4)>>3;
						d[0] = ((s2[rx3plus1]<<2)+(s2[rx3])+(s2[rx3plus1-nSourceStride])+(s2[rx3plus1+nSourceStride])+(s2[rx3plus2])+4)>>3;
						d[1] = ((s2[rx3plus1+1]<<2)+(s2[rx3+1])+(s2[rx3plus1-nSourceStride+1])+(s2[rx3plus1+nSourceStride+1])+(s2[rx3plus2+1])+4)>>3;
						d[2] = ((s2[rx3plus1+2]<<2)+(s2[rx3+2])+(s2[rx3plus1-nSourceStride+2])+(s2[rx3plus1+nSourceStride+2])+(s2[rx3plus2+2])+4)>>3;
					}
					else
					{
						a[0] = b[0] = s1[rx3];
						a[1] = b[1] = s1[rx3+1];
						a[2] = b[2] = s1[rx3+2];
						c[0] = d[0] = s2[rx3];
						c[1] = d[1] = s2[rx3+1];
						c[2] = d[2] = s2[rx3+2];
					}
				}
				else
				{
					a[0] = b[0] = s1[rx3];
					a[1] = b[1] = s1[rx3+1];
					a[2] = b[2] = s1[rx3+2];
					c[0] = d[0] = s2[rx3];
					c[1] = d[1] = s2[rx3+1];
					c[2] = d[2] = s2[rx3+2];
				}

				int t0[3], t1[3];

				t0[0] = a[0] - ((p * (a[0]-b[0]) + (1<<14) ) >>BILINEAR_PREC);
				t0[1] = a[1] - ((p * (a[1]-b[1]) + (1<<14) ) >>BILINEAR_PREC);
				t0[2] = a[2] - ((p * (a[2]-b[2]) + (1<<14) ) >>BILINEAR_PREC);
				t1[0] = c[0] - ((p * (c[0]-d[0]) + (1<<14) ) >>BILINEAR_PREC);
				t1[1] = c[1] - ((p * (c[1]-d[1]) + (1<<14) ) >>BILINEAR_PREC);
				t1[2] = c[2] - ((p * (c[2]-d[2]) + (1<<14) ) >>BILINEAR_PREC);

				res[0] = t0[0] - ((q*(t0[0]-t1[0]) + (1<<14))>>BILINEAR_PREC);
				res[1] = t0[1] - ((q*(t0[1]-t1[1]) + (1<<14))>>BILINEAR_PREC);
				res[2] = t0[2] - ((q*(t0[2]-t1[2]) + (1<<14))>>BILINEAR_PREC);

			}
		
			pDest[vx] = pClipTab[res[0]];	
			pDest[vx+1] = pClipTab[res[1]];	
			pDest[vx+2] = pClipTab[res[2]];	
	
			x_accum += m_w_inc;
		}

		pDest += nDestStride;
		
		y_accum += m_h_inc;
	}
	
}

//end

#endif //EPF_V2

#else
void CBilinearResampler::resample_bilinear_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw = m_dw;
	int dh = m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	y_accum = 0;
	for (vy = 0; vy < dh; vy++)  
	{
		BYTE *s1, *s2;
		int q;
		int ry;
		ry = (y_accum>>BILINEAR_PREC);

		//upper line 
		s1 = pSource + ry * nSourceStride;
		if(ry + 1 < m_sh)
		{
			//lower line
			s2 = s1 + nSourceStride;
		}
		else
			s2 = s1;
		x_accum = 0;
		q = y_accum & BILINEAR_MASK;

		for (vx = 0; vx < 3 * dw; vx += 3)
		{
			BYTE res[3];
			int rx;
			int a[3], b[3], c[3], d[3];
			int p;
			
			rx = x_accum>>BILINEAR_PREC;
			p = x_accum & BILINEAR_MASK;
			
			if(rx + 1 < m_sw)
			{
				//a[0] = s1[rx];
				a[0] = s1[3*rx];
				a[1] = s1[3*rx+1];
				a[2] = s1[3*rx+2];
				//b[0] = s1[rx+1];
				b[0] = s1[3*(rx+1)];
				b[1] = s1[3*(rx+1)+1];
				b[2] = s1[3*(rx+1)+2];
				//c[0] = s2[rx];
				c[0] = s2[3*rx];
				c[1] = s2[3*rx+1];
				c[2] = s2[3*rx+2];
				//d[0] = s2[rx+1];
				d[0] = s2[3*(rx+1)];
				d[1] = s2[3*(rx+1)+1];
				d[2] = s2[3*(rx+1)+2];
			}
			else
			{
				a[0] = b[0] = s1[3*rx];
				a[1] = b[1] = s1[3*rx+1];
				a[2] = b[2] = s1[3*rx+2];
				c[0] = d[0] = s2[3*rx];
				c[1] = d[1] = s2[3*rx+1];
				c[2] = d[2] = s2[3*rx+2];
			}

			
			int t0[3], t1[3];
		
			t0[0] = a[0] - ((p * (a[0]-b[0]) ) >>BILINEAR_PREC);
			t0[1] = a[1] - ((p * (a[1]-b[1]) ) >>BILINEAR_PREC);
			t0[2] = a[2] - ((p * (a[2]-b[2]) ) >>BILINEAR_PREC);
			t1[0] = c[0] - ((p * (c[0]-d[0]) ) >>BILINEAR_PREC);
			t1[1] = c[1] - ((p * (c[1]-d[1]) ) >>BILINEAR_PREC);
			t1[2] = c[2] - ((p * (c[2]-d[2]) ) >>BILINEAR_PREC);
			
			res[0] = t0[0] - ((q*(t0[0]-t1[0]))>>BILINEAR_PREC);
			res[1] = t0[1] - ((q*(t0[1]-t1[1]))>>BILINEAR_PREC);
			res[2] = t0[2] - ((q*(t0[2]-t1[2]))>>BILINEAR_PREC);
		
			pDest[vx] = pClipTab[res[0]];	
			pDest[vx+1] = pClipTab[res[1]];	
			pDest[vx+2] = pClipTab[res[2]];	
	
			x_accum += m_w_inc;
		}

		pDest += nDestStride;
		
		y_accum += m_h_inc;
	}
	
}


#endif
void CBilinearResampler::resample_bilinear(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	y_accum=0;
	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s1,*s2;
		int q;
		int ry;
		ry=(y_accum>>BILINEAR_PREC);
		
		//upper line 
		s1=pSource+ry*nSourceStride;
		if(ry+1<m_sh)
		{
			//lower line
			s2=s1+nSourceStride;
		}
		else
			s2=s1;
		x_accum=0;
		q=y_accum & BILINEAR_MASK;
		
		for(vx = 0;vx < dw; vx++)
		{
			BYTE res;
			int rx;
			int  a,b,c,d;
			int p;
			
			rx=x_accum>>BILINEAR_PREC;
			p=x_accum & BILINEAR_MASK;
			
			if(rx+1<m_sw)
			{
				a= s1[rx];
				b= s1[rx+1];
				c= s2[rx];
				d= s2[rx+1];
			}
			else
			{
				a= b=s1[rx];
				c= d= s2[rx];
			}
			
			
			int t0,t1;
			
			t0=a- ((p*(a-b) ) >>BILINEAR_PREC);
			t1=c- ((p*(c-d) ) >>BILINEAR_PREC);
			
			res=(BYTE)(t0- ((q*(t0-t1))>>BILINEAR_PREC));
			
			
			//res = (BYTE)(((float)a*p1 + (float)b*p)*q1 + 
			//	((float)c*p1 +(float)d*p)*q);
			
			//		pDest[vx]=(BYTE)res;
			pDest[vx]=pClipTab[res];	
			
			
			x_accum+=m_w_inc;
		}
		
		pDest+=nDestStride;
		
		y_accum+=m_h_inc;
	}
	
}

//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------
void CBilinearResampler::resample_lowpass_3x3(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride=nSourceStride;
	unsigned int c_a,c_b,c_c;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//kernel coeffs
	c_a=m_LowpassKernel_3[0][0];
	c_b=m_LowpassKernel_3[0][1];
	c_c=m_LowpassKernel_3[1][1];




	y_accum=0;
	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry=(y_accum>>BILINEAR_PREC);
		
		if(ry>m_sh-2)
			ry=m_sh-2;
		else
			if(ry<1)
				ry=1;

		//upper line 
		s1=pSource+(ry-1)*nSourceStride-1;
		//lower line
		x_accum=0;
		
		for(vx = 0;vx < dw; vx++)
		{
			int rx;
			unsigned int rez;
			BYTE *s;
			
			rx=x_accum>>BILINEAR_PREC;
			
			if(rx>m_sw-2)
				rx=m_sw-2;
			else
				if(rx<1)
					rx=1;

			s=s1+rx;
			{
				unsigned int s_a,s_b,s_c;	
				s_a=s[0]+s[2]+s[2*src_stride]+s[2*src_stride+2];
				s_b=s[1]+s[2*src_stride+1]+s[src_stride]+s[src_stride+2];
				s_c=s[src_stride+1];
				rez= ( (s_a*c_a)+(s_b*c_b)+(s_c*c_c) );
				rez>>=BILINEAR_LOWPASS_SHIFT;
			}

				
	//		pDest[vx]=(BYTE)rez;
			pDest[vx]=pClipTab[rez];	
		
			x_accum+=m_w_inc;
		}

		pDest+=nDestStride;
		
		y_accum+=m_h_inc;
	}
	
}

void CBilinearResampler::resample_lowpass_3x3_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride = nSourceStride;
	unsigned int c_a, c_b, c_c;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//kernel coeffs
	c_a=m_LowpassKernel_3[0][0];
	c_b=m_LowpassKernel_3[0][1];
	c_c=m_LowpassKernel_3[1][1];
	
	y_accum=0;
	for (vy = 0;vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry = (y_accum>>BILINEAR_PREC);
		
		if (ry > m_sh - 2)
			ry = m_sh - 2;
		else
			if(ry < 1)
				ry = 1;
			
			//upper line 
			//s1 = pSource + (ry-1) * nSourceStride - 1;
			s1 = pSource + (ry-1) * nSourceStride - 3;
			//lower line
			x_accum = 0;
			
			for(vx = 0; vx < 3 * dw; vx += 3)
			{
				int rx;
				unsigned int rez[3];
				BYTE *s;
				
				rx = x_accum>>BILINEAR_PREC;
				
				if (rx > m_sw - 2)
					rx = m_sw - 2;
				else
					if(rx < 1)
						rx = 1;
					
					//s = s1 + rx;
					s = s1 + 3 * rx;
					{
						unsigned int s_a[3], s_b[3], s_c[3];	
						//s_a[0] = s[0]+s[2]+s[2*src_stride]+s[2*src_stride+2];
						s_a[0] = s[0] + s[3*2] + s[2*src_stride] + s[2*src_stride+3*2];
						s_a[1] = s[0+1] + s[3*2+1] + s[2*src_stride+1] + s[2*src_stride+3*2+1];
						s_a[2] = s[0+2] + s[3*2+2] + s[2*src_stride+2] + s[2*src_stride+3*2+2];
						//s_b[0] = s[1]+s[2*src_stride+1]+s[src_stride]+s[src_stride+2];
						s_b[0] = s[3*1]+s[2*src_stride+3*1]+s[src_stride]+s[src_stride+3*2];
						s_b[1] = s[3*1+1]+s[2*src_stride+3*1+1]+s[src_stride+1]+s[src_stride+3*2+1];
						s_b[2] = s[3*1+2]+s[2*src_stride+3*1+2]+s[src_stride+2]+s[src_stride+3*2+2];
						//s_c[0] = s[src_stride+1];
						s_c[0] = s[src_stride+3*1];
						s_c[1] = s[src_stride+3*1+1];
						s_c[2] = s[src_stride+3*1+2];

						rez[0] = ( (s_a[0] * c_a)+(s_b[0] * c_b)+(s_c[0] * c_c) );
						rez[0] >>= BILINEAR_LOWPASS_SHIFT;

						rez[1] = ( (s_a[1] * c_a)+(s_b[1] * c_b)+(s_c[1] * c_c) );
						rez[1] >>= BILINEAR_LOWPASS_SHIFT;

						rez[2] = ( (s_a[2] * c_a)+(s_b[2] * c_b)+(s_c[2] * c_c) );
						rez[2] >>= BILINEAR_LOWPASS_SHIFT;
					}
					
					pDest[vx] = pClipTab[rez[0]];	
					pDest[vx+1] = pClipTab[rez[1]];	
					pDest[vx+2] = pClipTab[rez[2]];	
					
					x_accum+=m_w_inc;
			}
			
			pDest += nDestStride;
			
			y_accum += m_h_inc;
	}
	
}

//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------
void CBilinearResampler::resample_lowpass_5x5(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride=nSourceStride;
	unsigned int c_a,c_b,c_c,c_d,c_e,c_f;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//kernel coeffs
	c_a=m_LowpassKernel_5[0][0];
	c_b=m_LowpassKernel_5[0][1];
	c_c=m_LowpassKernel_5[0][2];
	c_d=m_LowpassKernel_5[1][1];
	c_e=m_LowpassKernel_5[1][2];
	c_f=m_LowpassKernel_5[2][2];



	y_accum=0;
	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry=(y_accum>>BILINEAR_PREC);
		
		if(ry>m_sh-3)
			ry=m_sh-3;
		else
			if(ry<2)
				ry=2;
		
		//upper line 
		s1=pSource+(ry-2)*nSourceStride-2;
		//lower line
		x_accum=0;
		
		for(vx = 0;vx < dw; vx++)
		{
			int rx;
			unsigned int rez;
			BYTE *s;
			
			rx=x_accum>>BILINEAR_PREC;
			
			if(rx>m_sw-3)
				rx=m_sw-3;
			else
				if(rx<2)
					rx=2;
			s=s1+rx;
			{
				unsigned int s_a,s_b,s_c,s_d,s_e,s_f;
				s_a=s[0]+s[4]+s[4*src_stride]+s[4*src_stride+4];
				s_b=s[1]+s[3]+s[src_stride]+s[src_stride+4]+s[3*src_stride]+s[3*src_stride+4]+s[4*src_stride+1]+s[4*src_stride+3];
				s_c=s[2]+s[4*src_stride+2]+s[2*src_stride]+s[2*src_stride+4];
				s_d=s[src_stride+1]+s[src_stride+3]+s[3*src_stride+1]+s[3*src_stride+3];
				s_e=s[src_stride+2]+s[3*src_stride+2]+s[2*src_stride+1]+s[2*src_stride+3];
				s_f=s[2*src_stride+2];
				
				rez= ( (s_a*c_a)+(s_b*c_b)+(s_c*c_c)+(s_d*c_d)+(s_e*c_e)+(s_f*c_f) );
				rez>>=BILINEAR_LOWPASS_SHIFT;
			}

				
		//	pDest[vx]=(BYTE)rez;
			pDest[vx]=pClipTab[rez];	
	
		
			x_accum+=m_w_inc;
		}

		pDest+=nDestStride;
		
		y_accum+=m_h_inc;
	}
	
}

void CBilinearResampler::resample_lowpass_5x5_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride = nSourceStride;
	unsigned int c_a, c_b, c_c, c_d, c_e, c_f;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//kernel coeffs
	c_a = m_LowpassKernel_5[0][0];
	c_b = m_LowpassKernel_5[0][1];
	c_c = m_LowpassKernel_5[0][2];
	c_d = m_LowpassKernel_5[1][1];
	c_e = m_LowpassKernel_5[1][2];
	c_f = m_LowpassKernel_5[2][2];
	
	y_accum=0;
	for (vy = 0;vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry = (y_accum>>BILINEAR_PREC);
		
		if(ry>m_sh-3)
			ry=m_sh-3;
		else
			if(ry<2)
				ry=2;
			
			//upper line 
			//s1 = pSource + (ry - 2) * nSourceStride - 2;
			s1 = pSource + (ry - 2) * nSourceStride - 6;
			//lower line
			x_accum = 0;
			
			for (vx = 0;vx < 3 * dw; vx += 3)
			{
				int rx;
				unsigned int rez[3];
				BYTE *s;
				
				rx=x_accum>>BILINEAR_PREC;
				
				if(rx > m_sw - 3)
					rx = m_sw - 3;
				else
					if(rx < 2)
						rx = 2;
					//s = s1 + rx;
					s = s1 + 3 * rx;
					{
						unsigned int s_a[3], s_b[3], s_c[3], s_d[3], s_e[3], s_f[3];
						//s_a[0] = s[0]+s[4]+s[4*src_stride]+s[4*src_stride+4];
						s_a[0] = s[0]+s[3*4]+s[4*src_stride]+s[4*src_stride+3*4];
						s_a[1] = s[0+1]+s[3*4+1]+s[4*src_stride+1]+s[4*src_stride+3*4+1];
						s_a[2] = s[0+2]+s[3*4+2]+s[4*src_stride+2]+s[4*src_stride+3*4+2];
						//s_b[0] = s[1]+s[3]+s[src_stride]+s[src_stride+4]+s[3*src_stride]+s[3*src_stride+4]+s[4*src_stride+1]+s[4*src_stride+3];
						s_b[0] = s[3*1]+s[3*3]+s[src_stride]+s[src_stride+3*4]+s[3*src_stride]+s[3*src_stride+3*4]+s[4*src_stride+3*1]+s[4*src_stride+3*3];
						s_b[1] = s[3*1+1]+s[3*3+1]+s[src_stride+1]+s[src_stride+3*4+1]+s[3*src_stride+1]+s[3*src_stride+3*4+1]+s[4*src_stride+3*1+1]+s[4*src_stride+3*3+1];
						s_b[2] = s[3*1+2]+s[3*3+2]+s[src_stride+2]+s[src_stride+3*4+2]+s[3*src_stride+2]+s[3*src_stride+3*4+2]+s[4*src_stride+3*1+2]+s[4*src_stride+3*3+2];
						//s_c[0] = s[2]+s[4*src_stride+2]+s[2*src_stride]+s[2*src_stride+4];
						s_c[0] = s[3*2]+s[4*src_stride+3*2]+s[2*src_stride]+s[2*src_stride+3*4];
						s_c[1] = s[3*2+1]+s[4*src_stride+3*2+1]+s[2*src_stride+1]+s[2*src_stride+3*4+1];
						s_c[2] = s[3*2+2]+s[4*src_stride+3*2+2]+s[2*src_stride+2]+s[2*src_stride+3*4+2];
						//s_d[0] = s[src_stride+1]+s[src_stride+3]+s[3*src_stride+1]+s[3*src_stride+3];
						s_d[0] = s[src_stride+3*1]+s[src_stride+3*3]+s[3*src_stride+3*1]+s[3*src_stride+3*3];
						s_d[1] = s[src_stride+3*1+1]+s[src_stride+3*3+1]+s[3*src_stride+3*1+1]+s[3*src_stride+3*3+1];
						s_d[2] = s[src_stride+3*1+2]+s[src_stride+3*3+2]+s[3*src_stride+3*1+2]+s[3*src_stride+3*3+2];
						//s_e[0] = s[src_stride+2]+s[3*src_stride+2]+s[2*src_stride+1]+s[2*src_stride+3];
						s_e[0] = s[src_stride+3*2]+s[3*src_stride+3*2]+s[2*src_stride+3*1]+s[2*src_stride+3*3];
						s_e[1] = s[src_stride+3*2+1]+s[3*src_stride+3*2+1]+s[2*src_stride+3*1+1]+s[2*src_stride+3*3+1];
						s_e[2] = s[src_stride+3*2+2]+s[3*src_stride+3*2+2]+s[2*src_stride+3*1+2]+s[2*src_stride+3*3+2];
						//s_f[0] = s[2*src_stride+2];
						s_f[0] = s[2*src_stride+3*2];
						s_f[1] = s[2*src_stride+3*2+1];
						s_f[2] = s[2*src_stride+3*2+2];
						
						rez[0] = ( (s_a[0] * c_a)+(s_b[0] * c_b)+(s_c[0] * c_c)+(s_d[0] * c_d)+(s_e[0] * c_e)+(s_f[0] * c_f) );
						rez[0] >>= BILINEAR_LOWPASS_SHIFT;

						rez[1] = ( (s_a[1] * c_a)+(s_b[1] * c_b)+(s_c[1] * c_c)+(s_d[1] * c_d)+(s_e[1] * c_e)+(s_f[1] * c_f) );
						rez[1] >>= BILINEAR_LOWPASS_SHIFT;

						rez[2] = ( (s_a[2] * c_a)+(s_b[1] * c_b)+(s_c[2] * c_c)+(s_d[2] * c_d)+(s_e[2] * c_e)+(s_f[2] * c_f) );
						rez[2] >>= BILINEAR_LOWPASS_SHIFT;
					}
					
					pDest[vx] = pClipTab[rez[0]];	
					pDest[vx+1] = pClipTab[rez[1]];	
					pDest[vx+2] = pClipTab[rez[2]];	
					
					
					x_accum += m_w_inc;
			}
			
			pDest += nDestStride;
			
			y_accum += m_h_inc;
	}
	
}

//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------

void CBilinearResampler::resample_lowpass_7x7(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride=nSourceStride;
	unsigned int c_a,c_b,c_c,c_d,c_e,c_f,c_g,c_h,c_i,c_j;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//kernel coeffs
	c_a=m_LowpassKernel_7[0][0];
	c_b=m_LowpassKernel_7[0][1];
	c_c=m_LowpassKernel_7[0][2];
	c_d=m_LowpassKernel_7[0][3];

	c_e=m_LowpassKernel_7[1][1];
	c_f=m_LowpassKernel_7[1][2];
	c_g=m_LowpassKernel_7[1][3];
	
	c_h=m_LowpassKernel_7[2][2];
	c_i=m_LowpassKernel_7[2][3];
	c_j=m_LowpassKernel_7[3][3];
	


	y_accum=0;
	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry=(y_accum>>BILINEAR_PREC);
		if(ry>m_sh-4)
			ry=m_sh-4;
		else
			if(ry<3)
				ry=3;
		//upper line 
		s1=pSource+(ry-3)*nSourceStride-3;
		//lower line
		x_accum=0;
		
		for(vx = 0;vx < dw; vx++)
		{
			int rx;
			unsigned int rez;
			BYTE *s;
			
			rx=x_accum>>BILINEAR_PREC;
			
			if(rx>m_sw-4)
				rx=m_sw-4;
			else
				if(rx<3)
					rx=3;

			s=s1+rx;
			{
				unsigned int s_a,s_b,s_c,s_d,s_e,s_f,s_g,s_h,s_i,s_j;
				
				s_a=s[0]+s[6]+s[6*src_stride]+s[6*src_stride+6];
				s_b=s[1]+s[5]+s[src_stride]+s[src_stride+6]+s[5*src_stride]+s[5*src_stride+6]+s[6*src_stride+1]+s[6*src_stride+5];
				s_c=s[2]+s[4]+s[2*src_stride]+s[2*src_stride+6]+s[4*src_stride]+s[4*src_stride+6]+s[6*src_stride+2]+s[6*src_stride+4];
				s_d=s[3]+ s[3*src_stride]+s[3*src_stride+6]+s[6*src_stride+3];
				s_e=s[src_stride+1]+s[src_stride+5]+s[5*src_stride+1]+s[5*src_stride+5];
				s_f=s[src_stride+2]+s[src_stride+4]+s[2*src_stride+1]+s[2*src_stride+5]+s[4*src_stride+1]+s[4*src_stride+5]+s[5*src_stride+2]+s[5*src_stride+4];
				
				s_g=s[src_stride+3]+s[3*src_stride+1]+s[3*src_stride+5]+s[5*src_stride+3];
				s_h=s[2*src_stride+2]+s[2*src_stride+4]+s[4*src_stride+2]+s[4*src_stride+4];
				
				s_i=s[2*src_stride+3]+s[3*src_stride+2]+s[3*src_stride+4]+s[4*src_stride+3];
				s_j=s[3*src_stride+3];
				
				
				rez= ( (s_a*c_a)+(s_b*c_b)+(s_c*c_c)+(s_d*c_d)+(s_e*c_e)+(s_f*c_f)+(s_g*c_g)+(s_h*c_h)+(s_i*c_i) +(s_j*c_j) );
				rez>>=BILINEAR_LOWPASS_SHIFT;
			}

						
	//		pDest[vx]=(BYTE)rez;
	
			pDest[vx]=pClipTab[rez];	
		
			x_accum+=m_w_inc;
		}

		pDest+=nDestStride;
		
		y_accum+=m_h_inc;
	}
	
}

void CBilinearResampler::resample_lowpass_7x7_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw = m_dw;
	int dh = m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride = nSourceStride;
	unsigned int c_a, c_b, c_c, c_d, c_e, c_f, c_g, c_h, c_i, c_j;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//kernel coeffs
	c_a = m_LowpassKernel_7[0][0];
	c_b = m_LowpassKernel_7[0][1];
	c_c = m_LowpassKernel_7[0][2];
	c_d = m_LowpassKernel_7[0][3];
	
	c_e = m_LowpassKernel_7[1][1];
	c_f = m_LowpassKernel_7[1][2];
	c_g = m_LowpassKernel_7[1][3];
	
	c_h = m_LowpassKernel_7[2][2];
	c_i = m_LowpassKernel_7[2][3];
	c_j = m_LowpassKernel_7[3][3];
	
	y_accum = 0;
	for(vy = 0; vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry = (y_accum>>BILINEAR_PREC);
		if(ry > m_sh - 4)
			ry = m_sh - 4;
		else
			if(ry < 3)
				ry = 3;
			//upper line 
			//s1 = pSource + (ry - 3) * nSourceStride - 3;
			s1 = pSource + (ry - 3) * nSourceStride - 9;
			//lower line
			x_accum = 0;
			
			for(vx = 0; vx < 3 * dw; vx += 3)
			{
				int rx;
				unsigned int rez[3];
				BYTE *s;
				
				rx = x_accum>>BILINEAR_PREC;
				
				if(rx > m_sw-4)
					rx = m_sw-4;
				else
					if(rx < 3)
						rx = 3;
					
					//s = s1 + rx;
					s = s1 + 3 * rx;
					{
						unsigned int s_a[3], s_b[3], s_c[3], s_d[3], s_e[3], s_f[3], s_g[3], s_h[3], s_i[3], s_j[3];
						
						//s_a[0] = s[0] + s[6] + s[6*src_stride] + s[6*src_stride+6];
						s_a[0] = s[0] + s[3*6] + s[6*src_stride] + s[6*src_stride+3*6];
						s_a[1] = s[1] + s[3*6+1] + s[6*src_stride+1] + s[6*src_stride+3*6+1];
						s_a[2] = s[2] + s[3*6+2] + s[6*src_stride+2] + s[6*src_stride+3*6+2];
						//s_b[0] = s[1] + s[5] + s[src_stride] + s[src_stride+6] + s[5*src_stride]+s[5*src_stride+6]+s[6*src_stride+1]+s[6*src_stride+5];
						s_b[0] = s[3*1] + s[3*5] + s[src_stride] + s[src_stride+3*6] + s[5*src_stride]+s[5*src_stride+3*6]+s[6*src_stride+3*1]+s[6*src_stride+3*5];
						s_b[1] = s[3*1+1] + s[3*5+1] + s[src_stride+1] + s[src_stride+3*6+1] + s[5*src_stride+1]+s[5*src_stride+3*6+1]+s[6*src_stride+3*1+1]+s[6*src_stride+3*5+1];
						s_b[2] = s[3*1+2] + s[3*5+2] + s[src_stride+2] + s[src_stride+3*6+2] + s[5*src_stride+2]+s[5*src_stride+3*6+2]+s[6*src_stride+3*1+2]+s[6*src_stride+3*5+2];
						//s_c[0] = s[2] + s[4] + s[2*src_stride]+s[2*src_stride+6]+s[4*src_stride]+s[4*src_stride+6]+s[6*src_stride+2]+s[6*src_stride+4];
						s_c[0] = s[3*2] + s[3*4] + s[2*src_stride]+s[2*src_stride+3*6]+s[4*src_stride]+s[4*src_stride+3*6]+s[6*src_stride+3*2]+s[6*src_stride+3*4];
						s_c[1] = s[3*2+1] + s[3*4+1] + s[2*src_stride+1]+s[2*src_stride+3*6+1]+s[4*src_stride+1]+s[4*src_stride+3*6+1]+s[6*src_stride+3*2+1]+s[6*src_stride+3*4+1];
						s_c[2] = s[3*2+2] + s[3*4+2] + s[2*src_stride+2]+s[2*src_stride+3*6+2]+s[4*src_stride+2]+s[4*src_stride+3*6+2]+s[6*src_stride+3*2+2]+s[6*src_stride+3*4+2];
						//s_d[0] = s[3] + s[3*src_stride] + s[3*src_stride+6] + s[6*src_stride+3];
						s_d[0] = s[3*3] + s[3*src_stride] + s[3*src_stride+3*6] + s[6*src_stride+3*3];
						s_d[1] = s[3*3+1] + s[3*src_stride+1] + s[3*src_stride+3*6+1] + s[6*src_stride+3*3+1];
						s_d[2] = s[3*3+2] + s[3*src_stride+2] + s[3*src_stride+3*6+2] + s[6*src_stride+3*3+2];
						//s_e[0] = s[src_stride+1] + s[src_stride+5]+s[5*src_stride+1]+s[5*src_stride+5];
						s_e[0] = s[src_stride+3*1] + s[src_stride+3*5]+s[5*src_stride+3*1]+s[5*src_stride+3*5];
						s_e[1] = s[src_stride+3*1+1] + s[src_stride+3*5+1]+s[5*src_stride+3*1+1]+s[5*src_stride+3*5+1];
						s_e[2] = s[src_stride+3*1+2] + s[src_stride+3*5+2]+s[5*src_stride+3*1+2]+s[5*src_stride+3*5+2];
						//s_f[0] = s[src_stride+2] + s[src_stride+4]+s[2*src_stride+1]+s[2*src_stride+5]+s[4*src_stride+1]+s[4*src_stride+5]+s[5*src_stride+2]+s[5*src_stride+4];
						s_f[0] = s[src_stride+3*2] + s[src_stride+3*4]+s[2*src_stride+3*1]+s[2*src_stride+3*5]+s[4*src_stride+3*1]+s[4*src_stride+3*5]+s[5*src_stride+3*2]+s[5*src_stride+3*4];
						s_f[1] = s[src_stride+3*2+1] + s[src_stride+3*4+1]+s[2*src_stride+3*1+1]+s[2*src_stride+3*5+1]+s[4*src_stride+3*1+1]+s[4*src_stride+3*5+1]+s[5*src_stride+3*2+1]+s[5*src_stride+3*4+1];
						s_f[2] = s[src_stride+3*2+2] + s[src_stride+3*4+2]+s[2*src_stride+3*1+2]+s[2*src_stride+3*5+2]+s[4*src_stride+3*1+2]+s[4*src_stride+3*5+2]+s[5*src_stride+3*2+2]+s[5*src_stride+3*4+2];
						
						//s_g[0] = s[src_stride+3] + s[3*src_stride+1] + s[3*src_stride+5] + s[5*src_stride+3];
						s_g[0] = s[src_stride+3*3] + s[3*src_stride+3*1] + s[3*src_stride+3*5] + s[5*src_stride+3*3];
						s_g[1] = s[src_stride+3*3+1] + s[3*src_stride+3*1+1] + s[3*src_stride+3*5+1] + s[5*src_stride+3*3+1];
						s_g[2] = s[src_stride+3*3+2] + s[3*src_stride+3*1+2] + s[3*src_stride+3*5+2] + s[5*src_stride+3*3+2];
						//s_h[0] = s[2*src_stride+2] + s[2*src_stride+4] + s[4*src_stride+2] + s[4*src_stride+4];
						s_h[0] = s[2*src_stride+3*2] + s[2*src_stride+3*4] + s[4*src_stride+3*2] + s[4*src_stride+3*4];
						s_h[1] = s[2*src_stride+3*2+1] + s[2*src_stride+3*4+1] + s[4*src_stride+3*2+1] + s[4*src_stride+3*4+1];
						s_h[2] = s[2*src_stride+3*2+2] + s[2*src_stride+3*4+2] + s[4*src_stride+3*2+2] + s[4*src_stride+3*4+2];
						
						//s_i[0] = s[2*src_stride+3] + s[3*src_stride+2] + s[3*src_stride+4] + s[4*src_stride+3];
						s_i[0] = s[2*src_stride+3*3] + s[3*src_stride+3*2] + s[3*src_stride+3*4] + s[4*src_stride+3*3];
						s_i[1] = s[2*src_stride+3*3+1] + s[3*src_stride+3*2+1] + s[3*src_stride+3*4+1] + s[4*src_stride+3*3+1];
						s_i[2] = s[2*src_stride+3*3+2] + s[3*src_stride+3*2+2] + s[3*src_stride+3*4+2] + s[4*src_stride+3*3+2];
						//s_j[0] = s[3*src_stride+3];
						s_j[0] = s[3*src_stride+3*3];
						s_j[1] = s[3*src_stride+3*3+1];
						s_j[2] = s[3*src_stride+3*3+2];
						
						rez[0] = ( (s_a[0]*c_a)+(s_b[0]*c_b)+(s_c[0]*c_c)+(s_d[0]*c_d)+(s_e[0]*c_e)+(s_f[0]*c_f)+(s_g[0]*c_g)+(s_h[0]*c_h)+(s_i[0]*c_i) +(s_j[0]*c_j) );
						rez[0] >>= BILINEAR_LOWPASS_SHIFT;

						rez[1] = ( (s_a[1]*c_a)+(s_b[1]*c_b)+(s_c[1]*c_c)+(s_d[1]*c_d)+(s_e[1]*c_e)+(s_f[1]*c_f)+(s_g[1]*c_g)+(s_h[1]*c_h)+(s_i[1]*c_i) +(s_j[1]*c_j) );
						rez[1] >>= BILINEAR_LOWPASS_SHIFT;

						rez[2] = ( (s_a[2]*c_a)+(s_b[2]*c_b)+(s_c[2]*c_c)+(s_d[2]*c_d)+(s_e[2]*c_e)+(s_f[2]*c_f)+(s_g[2]*c_g)+(s_h[2]*c_h)+(s_i[2]*c_i) +(s_j[2]*c_j) );
						rez[2] >>= BILINEAR_LOWPASS_SHIFT;
					}
					
					pDest[vx] = pClipTab[rez[0]];	
					pDest[vx+1] = pClipTab[rez[1]];	
					pDest[vx+2] = pClipTab[rez[2]];	
					
					x_accum += m_w_inc;
			}
			
			pDest += nDestStride;
			
			y_accum += m_h_inc;
	}
	
}

#endif


void CBilinearResampler::ChangeColourTemprature(PBYTE address1, // base address of block 1
								  int   stride1,  // stride of block 1
								  PBYTE address2, // base address of block 2
								  int   stride2, // stride of block 2
								  int clipindex // index to be used for pClipTab
								  ) 
{
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	int i,j;
	for (j = 0; j < m_dh; j++)
	{
	
		for (i = 0; i < m_dw; i++)
		{
			address2[i] = pClipTab[address1[i]];
		}
		address2 += stride2;
		address1 += stride1;
	}


}









#ifdef __MMX_RESAMPLER__
//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------
void CBilinearResampler::resample_bilinear(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	y_accum=0;
	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s1,*s2;
		unsigned short q_tab[4];
		int q;

		int ry=(y_accum>>BILINEAR_PREC);

		//upper line 
		s1=pSource+ry*nSourceStride;
		if(ry+1<m_sh)
		{
			//lower line
			s2=s1+nSourceStride;
		}
		else
			s2=s1;
	
		x_accum=0;
		q=y_accum & BILINEAR_MASK;

		q_tab[0]=q_tab[2]=((unsigned short)BILINEAR_MASK-(unsigned short)q);
		q_tab[1]=q_tab[3]=(unsigned short)q;

		for(vx = 0;vx < dw; vx+=2)
		{
			unsigned int res,res1;
			int rx;
			int  a,b,c,d;
			int p;
			
			unsigned short pix_u[4];
			unsigned short pix_l[4];
			unsigned short p_tab[4];
			const unsigned short mask[4]={0xFFFE,0x0,0xFFFE,0x0};
			
			rx=x_accum>>BILINEAR_PREC;
			p=x_accum & BILINEAR_MASK;
			


			if(rx+1<m_sw)
			{
				a= s1[rx];
				b= s1[rx+1];
				c= s2[rx];
				d= s2[rx+1];
			}
			else
			{
				a= b=s1[rx];
				c= d= s2[rx];
			}
			
			pix_u[0]=a;
			pix_u[1]=b;
			
			pix_l[0]=c;
			pix_l[1]=d;
			
			p_tab[0]=((unsigned short)BILINEAR_MASK-(unsigned short)p);
			p_tab[1]=((unsigned short)p);
		
			x_accum+=m_w_inc;

			rx=x_accum>>BILINEAR_PREC;
			p=x_accum & BILINEAR_MASK;
			

			if(rx+1<m_sw)
			{
				a= s1[rx];
				b= s1[rx+1];
				c= s2[rx];
				d= s2[rx+1];
			}
			else
			{
				a= b=s1[rx];
				c= d= s2[rx];
			}

			pix_u[2]=a;
			pix_u[3]=b;

			pix_l[2]=c;
			pix_l[3]=d;
			
			p_tab[2]=((unsigned short)BILINEAR_MASK-(unsigned short)p);
			p_tab[3]=((unsigned short)p);
		
			
			__asm
			{
				movq mm0, pix_u
				movq mm1, pix_l
				pmaddwd mm0,p_tab
				pmaddwd mm1,p_tab
				movq mm2, q_tab

				psrld mm0,15
				psrld mm1,15
				pslld mm1,16

				por mm0,mm1
				//qtab
				pmaddwd mm0, mm2
				
				psrld mm0,15
				movd res, mm0
				psrlq mm0,32
				movd res1,mm0
			}



			
			
			pDest[vx]=pClipTab[res];	
			pDest[vx+1]=pClipTab[res1];	
	
		
			x_accum+=m_w_inc;
		}

		pDest+=nDestStride;
		
		y_accum+=m_h_inc;
	}
	__asm emms
}



//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------
void CBilinearResampler::resample_lowpass_3x3(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride=nSourceStride;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	unsigned short *kernel=(unsigned short *)m_LowpassKernel_3;




	y_accum=0;
	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry=(y_accum>>BILINEAR_PREC);
		
		if(ry>m_sh-2)
			ry=m_sh-2;
		else
			if(ry<1)
				ry=1;

		//upper line 
		s1=pSource+(ry-1)*nSourceStride-1;
		//lower line
		x_accum=0;
		
		for(vx = 0;vx < dw; vx++)
		{
			int rx;
			unsigned int rez;
			BYTE *s;
			
			rx=x_accum>>BILINEAR_PREC;
			
			if(rx>m_sw-2)
				rx=m_sw-2;
			else
				if(rx<1)
					rx=1;

			s=s1+rx;
			{/*
				unsigned int s_a,s_b,s_c;	
				s_a=s[0]+s[2]+s[2*src_stride]+s[2*src_stride+2];
				s_b=s[1]+s[2*src_stride+1]+s[src_stride]+s[src_stride+2];
				s_c=s[src_stride+1];
				rez= ( (s_a*c_a)+(s_b*c_b)+(s_c*c_c) );
				rez>>=BILINEAR_LOWPASS_SHIFT;
				*/

				__asm
				{
					pxor mm7,mm7
					mov esi, s
					mov eax, kernel

					//source
					movd mm0,[esi]
					//next row
					add esi, src_stride
					//source
					movd mm1,[esi]
					//next row
					add esi, src_stride
				
					movd mm2,[esi]
					//expand source
					punpcklbw mm0,mm7
					//expand source
					punpcklbw mm1,mm7
					//expand source
					punpcklbw mm2,mm7
					//add firast+last
					paddusw mm0,mm2

					//kernel
					movq mm3,[eax]
					//multiply and add by kernel
					pmaddwd mm0, mm3
					//kernel
					movq mm4,[eax+16]
					//multiply and add by kernel
					pmaddwd mm1, mm4
				
					//sum
					paddd mm0, mm1
					movq mm6,mm0
					psrlq mm0,32
					paddd mm0,mm6
					movd rez,mm0
					__asm emms
				}
				
				rez>>=BILINEAR_LOWPASS_SHIFT;
			}

			pDest[vx]=pClipTab[rez];	
		
			x_accum+=m_w_inc;
		}

		pDest+=nDestStride;
		
		y_accum+=m_h_inc;
	}
		__asm emms
	
}

//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------
void CBilinearResampler::resample_lowpass_5x5(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride=nSourceStride;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	unsigned short *kernel=(unsigned short *)m_LowpassKernel_5;
		

	y_accum=0;
	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry=(y_accum>>BILINEAR_PREC);
		
		if(ry>m_sh-3)
			ry=m_sh-3;
		else
			if(ry<2)
				ry=2;
		
		//upper line 
		s1=pSource+(ry-2)*nSourceStride-2;
		//lower line
		x_accum=0;
		
		for(vx = 0;vx < dw; vx++)
		{
			int rx;
			unsigned int rez;
			BYTE *s;
			
			rx=x_accum>>BILINEAR_PREC;
			
			if(rx>m_sw-3)
				rx=m_sw-3;
			else
				if(rx<2)
					rx=2;
			s=s1+rx;
			{
			/*	unsigned int s_a,s_b,s_c,s_d,s_e,s_f;
				s_a=s[0]+s[4]+s[4*src_stride]+s[4*src_stride+4];
				s_b=s[1]+s[3]+s[src_stride]+s[src_stride+4]+s[3*src_stride]+s[3*src_stride+4]+s[4*src_stride+1]+s[4*src_stride+3];
				s_c=s[2]+s[4*src_stride+2]+s[2*src_stride]+s[2*src_stride+4];
				s_d=s[src_stride+1]+s[src_stride+3]+s[3*src_stride+1]+s[3*src_stride+3];
				s_e=s[src_stride+2]+s[3*src_stride+2]+s[2*src_stride+1]+s[2*src_stride+3];
				s_f=s[2*src_stride+2];
				
				rez= ( (s_a*c_a)+(s_b*c_b)+(s_c*c_c)+(s_d*c_d)+(s_e*c_e)+(s_f*c_f) );
			*/
			
				short tmpb[4];
				__asm
				{
					pxor mm7,mm7
					pxor mm6,mm6
					mov esi, s
					mov eax, kernel
					lea ebx,tmpb
					xor ecx, ecx
					xor edx, edx
					//source 1
					movd mm0,[esi]
					//save first
					mov  dl, [esi+4]
				
				
					//next row
					add esi, src_stride
				
					//source 2
					movd mm1,[esi]
					mov  cl, [esi+4]
					mov [ebx+2],cx
				
				
					//next row
					add esi, src_stride
				
					//source 3
					movd mm2,[esi]
					mov  cl, [esi+4]
					mov [ebx+4],cx
				
					//next row
					add esi, src_stride
				
					//source 4
					movd mm3,[esi]
					mov  cl, [esi+4]
					mov  [ebx+6],cx
				
					//next row
					add esi, src_stride
				
					//source 5
					movd mm4,[esi]
					mov  cl, [esi+4]
					//sum first + last
					add ecx, edx

					mov  [ebx],cx
					movq mm5, tmpb
					//expand source
					punpcklbw mm0,mm7
					//expand source
					punpcklbw mm1,mm7
					//expand source
					punpcklbw mm2,mm7
					//expand source
					punpcklbw mm3,mm7
					//expand source
					punpcklbw mm4,mm7
				
					paddw mm0,mm4
					paddw mm0,mm5
					paddw mm1,mm3
				
					//kernel
					movq mm4,[eax]
					//multiply and add by kernel
					pmaddwd mm0, mm4
				
					//kernel
					movq mm5,[eax+2*8]
					//multiply and add by kernel
					pmaddwd mm1, mm5
				
					//kernel
					movq mm6,[eax+2*16]
					//multiply and add by kernel
					pmaddwd mm2, mm6
				
					//sum
					paddd mm0, mm1
					paddd mm0, mm2
				
					movq mm6,mm0

					psrlq mm0,32
					paddd mm0,mm6
					movd rez,mm0
			}

			rez>>=BILINEAR_LOWPASS_SHIFT;
			}

				
		//	pDest[vx]=(BYTE)rez;
			pDest[vx]=pClipTab[rez];	
	
		
			x_accum+=m_w_inc;
		}

		pDest+=nDestStride;
		
		y_accum+=m_h_inc;
	}
	__asm emms
}


//------------------------------------------------------------------------
//
//
//
//
//
//
//
//------------------------------------------------------------------------
void CBilinearResampler::resample_lowpass_7x7(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;
	unsigned int x_accum;
	unsigned int y_accum;
	int src_stride=nSourceStride;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	unsigned short *kernel=(unsigned short *)m_LowpassKernel_7;
		


	y_accum=0;
	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s1;
		int ry;
		ry=(y_accum>>BILINEAR_PREC);
		if(ry>m_sh-4)
			ry=m_sh-4;
		else
			if(ry<3)
				ry=3;
		//upper line 
		s1=pSource+(ry-3)*nSourceStride-3;
		//lower line
		x_accum=0;
		
		for(vx = 0;vx < dw; vx++)
		{
			int rx;
			unsigned int rez;
			BYTE *s;
			
			rx=x_accum>>BILINEAR_PREC;
			
			if(rx>m_sw-4)
				rx=m_sw-4;
			else
				if(rx<3)
					rx=3;

			s=s1+rx;
			{
/*				unsigned int s_a,s_b,s_c,s_d,s_e,s_f,s_g,s_h,s_i,s_j;
				
				s_a=s[0]+s[6]+s[6*src_stride]+s[6*src_stride+6];
				s_b=s[1]+s[5]+s[src_stride]+s[src_stride+6]+s[5*src_stride]+s[5*src_stride+6]+s[6*src_stride+1]+s[6*src_stride+5];
				s_c=s[2]+s[4]+s[2*src_stride]+s[2*src_stride+6]+s[4*src_stride]+s[4*src_stride+6]+s[6*src_stride+2]+s[6*src_stride+4];
				s_d=s[3]+ s[3*src_stride]+s[3*src_stride+6]+s[6*src_stride+3];
				s_e=s[src_stride+1]+s[src_stride+5]+s[5*src_stride+1]+s[5*src_stride+5];
				s_f=s[src_stride+2]+s[src_stride+4]+s[2*src_stride+1]+s[2*src_stride+5]+s[4*src_stride+1]+s[4*src_stride+5]+s[5*src_stride+2]+s[5*src_stride+4];
				
				s_g=s[src_stride+3]+s[3*src_stride+1]+s[3*src_stride+5]+s[5*src_stride+3];
				s_h=s[2*src_stride+2]+s[2*src_stride+4]+s[4*src_stride+2]+s[4*src_stride+4];
				
				s_i=s[2*src_stride+3]+s[3*src_stride+2]+s[3*src_stride+4]+s[4*src_stride+3];
				s_j=s[3*src_stride+3];
				
				
				rez= ( (s_a*c_a)+(s_b*c_b)+(s_c*c_c)+(s_d*c_d)+(s_e*c_e)+(s_f*c_f)+(s_g*c_g)+(s_h*c_h)+(s_i*c_i) +(s_j*c_j) );
				  rez>>=BILINEAR_LOWPASS_SHIFT;
*/
				const BYTE mask[8]={0xFF,0xFF,0xFF,0x00,0,0,0,0};
				__asm
				{
					pxor mm7,mm7
					mov esi, s
					mov eax, kernel
					//source 1
					movd mm0,[esi]
					movd mm3,[esi+4]

					//next row
					add esi, src_stride
				
					//source 2
					movd mm1,[esi]
					movd mm4,[esi+4]
					//next row
					add esi, src_stride
				
					//source 3
					movd mm2,[esi]
					movd mm5,[esi+4]
					//next row
					add esi, src_stride
					
					pand mm3,mask
					pand mm4,mask
					pand mm5,mask
			//		psrlq mm3,8
			//		psrlq mm4,8
			//		psrlq mm5,8

					//expand sources
					punpcklbw mm0,mm7
					punpcklbw mm1,mm7
					punpcklbw mm2,mm7
					punpcklbw mm3,mm7
					punpcklbw mm4,mm7
					punpcklbw mm5,mm7
					pshufw  mm3, mm3,0xC6
					pshufw  mm4, mm4,0xC6
					pshufw  mm5, mm5,0xC6
					paddw mm0,mm3
					paddw mm1,mm4
					paddw mm2,mm5
					
					//source 4
					movd mm3,[esi]
					movd mm4,[esi+4]
					//next row
					add esi, src_stride
					
					pand mm4,mask
				//	psrlq mm4,8

					punpcklbw mm3,mm7
					punpcklbw mm4,mm7
					pshufw  mm4, mm4,0xC6
					paddw  mm3,mm4

					//source 5
					movd mm4,[esi]
					movd mm5,[esi+4]
					//next row
					add esi, src_stride

					pand mm5,mask
			//		psrlq mm5,8


					punpcklbw mm4,mm7
					punpcklbw mm5,mm7
					pshufw  mm5,mm5, 0xC6
	
					paddw  mm2,mm4
					paddw mm2, mm5


					//source 6
					movd mm4,[esi]
					movd mm5,[esi+4]
					//next row
					add esi, src_stride
					pand mm5,mask
			//		psrlq mm5,8

					punpcklbw mm4,mm7
					punpcklbw mm5,mm7
					pshufw  mm5,mm5, 0xC6
	
					paddw  mm1,mm4
					paddw mm1, mm5



					//source 7
					movd mm4,[esi]
					movd mm5,[esi+4]
					//next row
					add esi, src_stride
					pand mm5,mask
			//		psrlq mm5,8


					punpcklbw mm4,mm7
					punpcklbw mm5,mm7
					pshufw  mm5,mm5, 0xC6
	
					paddw  mm0,mm4
					paddw mm0, mm5

//---------------------------------------------------------------
					//kernel
					movq mm4,[eax]
					//multiply and add by kernel
					pmaddwd mm0, mm4
				
					//kernel
					movq mm5,[eax+2*8]
					//multiply and add by kernel
					pmaddwd mm1, mm5
				
					//kernel
					movq mm6,[eax+2*16]
					//multiply and add by kernel
					pmaddwd mm2, mm6
				
					//kernel
					movq mm4,[eax+3*16]
					//multiply and add by kernel
					pmaddwd mm3, mm4
				

					//sum
					paddd mm0, mm1
					paddd mm0, mm2
					paddd mm0, mm3
				
					movq mm6,mm0

					psrlq mm0,32
					paddd mm0,mm6
					movd rez,mm0
			}

				rez>>=BILINEAR_LOWPASS_SHIFT;
			}
						
	//		pDest[vx]=(BYTE)rez;
	
			pDest[vx]=pClipTab[rez];	
		
			x_accum+=m_w_inc;
		}

		pDest+=nDestStride;
		
		y_accum+=m_h_inc;
	}
	__asm emms
}

#endif
