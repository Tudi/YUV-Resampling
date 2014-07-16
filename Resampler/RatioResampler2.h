#pragma once

#include "resampler.h"

class CRatioResampler2
{
public:

	enum RAW_DATA_TYPE
	{
		RGB24,
		IYUV
	};

	CRatioResampler2(int nSrcWidth, 
		int nSrcHeight,
		int nDesWidth,
		int nDesHeight,
		bool bKeepAspectRatio);

	virtual ~CRatioResampler2(void);

	void Process(BYTE *pSrc, BYTE *pDes, double * pAspectRatio = NULL, RAW_DATA_TYPE eRawDataType = RGB24 );
	void Crop(BYTE *pSrc, BYTE *pDes, RAW_DATA_TYPE eRawDataType );
	void SetCropParams( int nOffsetTop, int nOffsetLeft, int nWidth, int nHeight )
	{
		m_nOffsetTop = nOffsetTop;
		m_nOffsetLeft = nOffsetLeft;
		m_nDesWidth = nWidth;
		m_nDesHeight = nHeight;
		if( m_nOffsetTop < 0 )
			m_nOffsetTop = 0;
		if( m_nOffsetTop > m_nSrcHeight )
			m_nOffsetTop = m_nSrcHeight;
		if( nOffsetLeft < 0 )
			nOffsetLeft = 0;
		if( nOffsetLeft > m_nSrcWidth )
			nOffsetLeft = m_nSrcWidth;
		if( nOffsetLeft + m_nDesWidth > m_nSrcWidth )
			m_nDesWidth = m_nSrcWidth - nOffsetLeft;
		if( m_nOffsetTop + m_nDesHeight > m_nSrcHeight )
			m_nDesHeight = m_nSrcHeight - m_nOffsetTop;
	}

	int m_nSrcWidth;
	int m_nSrcHeight;
	int m_nDesWidth;
	int m_nDesHeight;
private:
	void ComputeResampleDimension(	int * pResampleWidth, int * pResampleHeight, 
									int * pTop, int * pBottom, int * pLeft, int * pRight, 
									double fSrcRatio, double fDstRatio, int DstWidth, int DstHeight );
	void FillBox( BYTE * pDstData,  RAW_DATA_TYPE eRawDataType );

protected:	
	double m_fSrcRatio;
	double m_fDstRatio;
	double m_AspectRatio;
	bool m_bKeepAspectRatio;
	TResampler *m_pResampler;
	bool m_bAddPillarBox;
	bool m_bAddLetterBox;
	int m_nOffsetTop;
	int m_nOffsetBottom;
	int m_nOffsetLeft;
	int m_nOffsetRight;
	int m_nResampleWidth;
	int m_nResampleHeight;	
	int m_nResampleStride;
};
