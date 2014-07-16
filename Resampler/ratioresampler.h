#pragma once

#include "resampler.h"

class CRatioResampler
{
public:

	enum RAW_DATA_TYPE
	{
		RGB24,
		IYUV
	};

	CRatioResampler(int nSrcWidth, 
					int nSrcHeight,
					int nDesWidth,
					int nDesHeight,
					bool bKeepAspectRatio);

	virtual ~CRatioResampler(void);

	void Process(BYTE *pSrc, BYTE *pDes, double * pAspectRatio = NULL, RAW_DATA_TYPE eRawDataType = RGB24 );

	int m_nSrcWidth;
	int m_nSrcHeight;
	int m_nDesWidth;
	int m_nDesHeight;

protected:

	int m_nResampleHeight;
	int m_nOffset;
	float m_fSrcRatio;
	double m_AspectRatio;
	bool m_bKeepAspectRatio;
	TResampler *m_pResampler;
};
