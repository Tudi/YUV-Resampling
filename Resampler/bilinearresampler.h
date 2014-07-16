#ifndef _BILINEAR_RESAMPLER_H_
#define _BILINEAR_RESAMPLER_H_

//Hoi Ming x64
#include <emmintrin.h>
//end

//
// Performs resampling for generic image size conversions
//
class CBilinearResampler
{
public:
	// constructor with actual image sizes
	CBilinearResampler(int sw, int sh, int dw, int dh,int precise=1,int lumalow =16, int lumahigh =235,int chromalow =16,int chromahigh =240);
	~CBilinearResampler();

	// performs resampling of the source into destination, with the ratio defined in constructor
	void Resample(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	void ChangeColourTemprature(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride ,int clipindex);
	void ResampleRGB24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	void ResampleRGB32(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	void ResampleBW1Bit(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);

	//Hoi Ming YUV resizer
	void ResampleYUV420(PBYTE pSourceY, PBYTE pSourceU, PBYTE pSourceV, int nSourceStride, 
										PBYTE pDestY, PBYTE pDestU, PBYTE pDestV, int nDestStride, int clipindex =0);
	//end

	/////////////////
	//{ vos 4249 , [jerry:2010-11-09]
	void CopyRGB( BYTE * pDst, BYTE * pSrc, int nSourceStride  );
	void CopyYUV420( BYTE * pDst, BYTE * pSrcY, BYTE * pSrcU, BYTE * pSrcV, int nSourceStride );
	//}
	////////////////
	
private:
	//Hoi Ming test 4May2009
	int absArray[255+256];
	
	//end
	//Hoi Ming Sinc Filter
	//[0]: 1/4; [1]: 1/2; [2]: 3/4 -- /1024
	//Hoi Ming X64
	short SincFilter[7][4];
	short SincFilter_mix[7][7][4][4];
	short SincFilter_mix_conv_1_1[7][7][8][8];
	short SincFilter_mix_conv_1_1_h[7][3][8];
	short SincFilter_mix_conv_1_1_v[7][6][8];
	short Tab_P_Q[16][16][4];
	short Tab_P_Q_mix_conv_1_1[16][16][4][4];
	short Tab_P_Q_mix_conv_1_1_h[16][3][4];

	short SincFilter_mix_conv_2_2[7][7][8][8];
	short SincFilter_mix_conv_2_2_h[7][5][8];
	short SincFilter_mix_conv_2_2_v[7][8][8];	//only [0:6][0:7][0:4] will use

	//int SincFilter[7][4];
	
	//Hoi Ming TBB
	int m_step;
	int m_num_cores;
	//end

	PBYTE SourceBackup;
	//int SincFilter[3][6];
	//end
	//
	// implementation-specific definitions
	//
	BOOL m_bIsNeedCopy;
	PBYTE m_pSourceBuffer;
	#include "_BilinearResampler.ph"
};



#endif

