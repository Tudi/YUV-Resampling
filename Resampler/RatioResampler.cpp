#include "StdAfx.h"
#include "RatioResampler.h"

CRatioResampler::CRatioResampler(int nSrcWidth, int nSrcHeight, int nDesWidth, int nDesHeight, bool bKeepAspectRatio)
: m_nSrcWidth(nSrcWidth)
, m_nSrcHeight(nSrcHeight)
, m_nDesWidth(nDesWidth)
, m_nDesHeight(nDesHeight)
, m_nResampleHeight( nDesHeight )
, m_bKeepAspectRatio(bKeepAspectRatio)
, m_AspectRatio( 0.0 )
{
	if(bKeepAspectRatio)
	{
		m_fSrcRatio = (float)m_nSrcHeight / (float)m_nSrcWidth;
		m_nResampleHeight = (int)(m_nDesWidth * m_fSrcRatio);
		m_nResampleHeight  -= (m_nResampleHeight % 4); // Make multifly 4
		if( m_nResampleHeight > m_nDesHeight )
			m_nResampleHeight = m_nDesHeight;

		m_nOffset = (m_nDesHeight-m_nResampleHeight )/2; // Letter Box's height

		//m_nDesHeight = (int)(m_nDesWidth * m_fSrcRatio);
		//m_nDesHeight -= (m_nDesHeight % 4); // Make multifly 4
		//if(m_nDesHeight > nDesHeight)
		//	m_nDesHeight = nDesHeight;
		//m_nOffset = (nDesHeight - m_nDesHeight) / 2; // Letter Box's height
	}

	//m_pResampler = CreateResampler(RT_BILINEAR, m_nDesWidth, m_nDesHeight, m_nSrcWidth, m_nSrcHeight);
	m_pResampler = CreateResampler(RT_BILINEAR, m_nDesWidth, m_nResampleHeight, m_nSrcWidth, m_nSrcHeight);
	_ASSERTE(m_pResampler);
}

CRatioResampler::~CRatioResampler()
{
	if(m_pResampler)
		ReleaseResampler(m_pResampler);
}

void CRatioResampler::Process(BYTE *pSrc, BYTE *pDes, double * pAspectRatio, RAW_DATA_TYPE eRawDataType)
{
	if(!pSrc ||
		!pDes ||
		!m_pResampler)
		return;

	//if( pAspectRatio && *pAspectRatio > 0.0 )
	if( m_bKeepAspectRatio && pAspectRatio && *pAspectRatio > 0.0 ) //Fix for the "Mpeg2-ts dump for anamorphic" [jerry:2010-11-04]
	{
		double AspectRatio = *pAspectRatio;

		if( m_AspectRatio != AspectRatio )
		{
			int Offset, Height;
			TResampler * pResampler;			

			Height = (int)(m_nDesWidth / AspectRatio);
			Height -= (Height % 4); // Make multifly 4
			if( Height > m_nDesHeight )
				Height = m_nDesHeight;

			Offset = (m_nDesHeight - Height)/2;
			pResampler = CreateResampler(RT_BILINEAR, m_nDesWidth, Height, m_nSrcWidth, m_nSrcHeight);
			if( pResampler )
			{
				//delete m_pResampler;
				//m_pResampler = NULL;
				if( m_pResampler ) // 2010-04-23, jerry fix memory leak. 
				{
					ReleaseResampler( m_pResampler );
					m_pResampler = NULL;
				}

				m_nResampleHeight = Height;
				m_nOffset = Offset;
				m_AspectRatio = AspectRatio;
				if( !m_bKeepAspectRatio )
					m_bKeepAspectRatio = true;
				m_pResampler = pResampler;
			}
		}
	}

	switch(eRawDataType)
	{
		case RGB24:
			if(m_bKeepAspectRatio)
			{
				//Insert letter box
				if(m_nOffset > 0)
				{
					// Top
					memset(pDes, 0, (m_nOffset * m_nDesWidth * 3));
					// Bottom
					//memset(pDes + ((m_nOffset + m_nDesHeight) * m_nDesWidth * 3), 0, (m_nOffset * m_nDesWidth * 3));
					memset( pDes + ((m_nOffset+m_nResampleHeight)*m_nDesWidth*3), 0, (m_nOffset * m_nDesWidth * 3) );
				}
				// Resized source
				ResampleRGB24(m_pResampler, pDes + (m_nOffset * m_nDesWidth * 3), pSrc);
			}
			else
			{
				//Hoi Ming YUV resizer
				/*unsigned char* temp_src_buffer = (unsigned char*)calloc(m_nSrcWidth*m_nSrcHeight*3/2, sizeof(unsigned char));
				unsigned char* temp_dst_buffer = (unsigned char*)calloc(m_nDesWidth*m_nDesHeight*3/2, sizeof(unsigned char));
				NEW_DIB_RGB24_to_IYUV(temp_src_buffer, pSrc, m_nSrcWidth, m_nSrcHeight);
				ResampleYUV420(m_pResampler, temp_dst_buffer, temp_dst_buffer+(m_nDesWidth*m_nDesHeight), temp_dst_buffer+(m_nDesWidth*m_nDesHeight*5/4),
				temp_src_buffer, temp_src_buffer+(m_nSrcWidth*m_nSrcHeight), temp_src_buffer+(m_nSrcWidth*m_nSrcHeight*5/4));
				NEW_DIB_IYUV_to_RGB24(pDes, m_nDesWidth, m_nDesHeight, m_nDesWidth, m_nDesWidth, 
				temp_dst_buffer, temp_dst_buffer+(m_nDesWidth*m_nDesHeight), temp_dst_buffer+(m_nDesWidth*m_nDesHeight*5/4));
				free(temp_src_buffer);
				free(temp_dst_buffer);*/

				ResampleRGB24(m_pResampler, pDes, pSrc);
			}
			break;
		case IYUV:
			if(m_bKeepAspectRatio)
			{
				if(m_nOffset > 0)
				{

					memset(pDes, 16, m_nDesWidth * m_nDesHeight);
					memset(pDes + m_nDesWidth*m_nDesHeight, 128, m_nDesWidth/2 * m_nDesHeight/2);
					memset(pDes + (m_nDesWidth*m_nDesHeight*5/4), 128, m_nDesWidth/2 * m_nDesHeight/2);
				}

				ResampleYUV420(m_pResampler, 
							   pDes + m_nDesWidth*m_nOffset,
							   pDes + m_nDesWidth*m_nDesHeight + m_nDesWidth/2*m_nOffset/2,
							   pDes + (m_nDesWidth*m_nDesHeight*5/4) + m_nDesWidth/2*m_nOffset/2,
							   pSrc,
							   pSrc + m_nSrcWidth*m_nSrcHeight,
							   pSrc + (m_nSrcWidth*m_nSrcHeight*5/4));
			}
			else
			{
				ResampleYUV420(m_pResampler, 
							   pDes,
							   pDes + m_nDesWidth*m_nDesHeight,
							   pDes + (m_nDesWidth*m_nDesHeight*5/4),
							   pSrc,
							   pSrc + m_nSrcWidth*m_nSrcHeight,
							   pSrc + (m_nSrcWidth*m_nSrcHeight*5/4));
			}
			break;
	}
}