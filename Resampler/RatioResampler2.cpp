#include "StdAfx.h"
#include "RatioResampler2.h"

void CRatioResampler2::ComputeResampleDimension(	int * pResampleWidth, int * pResampleHeight, 
													int * pTop, int * pBottom, int * pLeft, int * pRight, 
													double fSrcRatio, double fDstRatio, int DstWidth, int DstHeight )
{
	int ResampleWidth;
	int ResampleHeight;
	*pTop = 0;
	*pBottom = 0;
	*pLeft = 0;
	*pRight = 0;

	if( fSrcRatio >= fDstRatio )
	{
		int Top, Bottom;
		////////////////////
		// Add Letter Box
		ResampleWidth = DstWidth;

		ResampleHeight = (int)(DstWidth / fSrcRatio);
		ResampleHeight -= (ResampleHeight % 4); // 

		Top = (DstHeight-ResampleHeight)/2;
		Top -= (Top%2);
		Bottom = DstHeight-(Top+ResampleHeight);

		if( Top <= 0 )
		{
			ResampleWidth = DstWidth;
			ResampleHeight = DstHeight;
			Bottom = 0;
		}
		*pTop = Top;
		*pBottom = Bottom;		
	}
	else
	{
		int Left, Right;
		/////////////
		// Add Pillar Box
		ResampleHeight = DstHeight;			

		ResampleWidth = (int)(ResampleHeight*fSrcRatio);
		ResampleWidth -= (ResampleWidth % 4); // 

		Left = (DstWidth-ResampleWidth)/2;
		Left -= (Left%2);
		Right = DstWidth-(Left+ResampleWidth);
		if( Left <= 0 )
		{
			ResampleWidth = DstWidth;
			ResampleHeight = DstHeight;
			Right = 0;
		}
		*pLeft = Left;
		*pRight = Right;		
	}

	*pResampleWidth = ResampleWidth;
	*pResampleHeight = ResampleHeight;
}

CRatioResampler2::CRatioResampler2(int nSrcWidth, int nSrcHeight, int nDesWidth, int nDesHeight, bool bKeepAspectRatio)
: m_nSrcWidth(nSrcWidth)
, m_nSrcHeight(nSrcHeight)
, m_nDesWidth(nDesWidth)
, m_nDesHeight(nDesHeight)
, m_nResampleHeight( nDesHeight )
, m_bKeepAspectRatio(bKeepAspectRatio)
, m_AspectRatio( 0.0 )
, m_bAddLetterBox(false)
, m_bAddPillarBox(false)
{
	m_fSrcRatio = (double)m_nSrcWidth / (double)m_nSrcHeight;
	m_fDstRatio = (double)m_nDesWidth / (double)m_nDesHeight;

	if( bKeepAspectRatio )
	{
		m_nResampleStride = m_nDesWidth;

		ComputeResampleDimension( &m_nResampleWidth, &m_nResampleHeight, &m_nOffsetTop, &m_nOffsetBottom, &m_nOffsetLeft, &m_nOffsetRight,
									m_fSrcRatio, m_fDstRatio, m_nDesWidth, m_nDesHeight );

		if( m_fSrcRatio >= m_fDstRatio )
		{			
			////////////////////
			// Add Letter Box
			if( m_nOffsetTop > 0 )
				m_bAddLetterBox = true;
		}
		else
		{
			/////////////
			// Add Pillar Box
			if( m_nOffsetLeft > 0 )
				m_bAddPillarBox = true;
		}
	}
	else
	{
		m_nResampleStride = m_nDesWidth;
		m_nResampleWidth = m_nDesWidth;
		m_nResampleHeight = m_nDesHeight;
	}
	m_pResampler = CreateResampler(RT_BILINEAR, m_nResampleWidth, m_nResampleHeight, m_nSrcWidth, m_nSrcHeight);
}

CRatioResampler2::~CRatioResampler2()
{
	if(m_pResampler)
		ReleaseResampler(m_pResampler);
}

void CRatioResampler2::FillBox( BYTE * pDstData,  RAW_DATA_TYPE eRawDataType )
{
	if( m_bAddLetterBox || m_bAddPillarBox )
	{
		if( eRawDataType == RGB24 )
		{
			memset( pDstData, 0, m_nDesWidth*m_nDesHeight*3 );
		}
		else
		{
			memset(pDstData, 16, m_nDesWidth * m_nDesHeight);
			memset(pDstData + m_nDesWidth*m_nDesHeight, 128, m_nDesWidth/2 * m_nDesHeight/2);
			memset(pDstData + (m_nDesWidth*m_nDesHeight*5/4), 128, m_nDesWidth/2 * m_nDesHeight/2);			
		}
	}	
}

void CRatioResampler2::Process(BYTE *pSrc, BYTE *pDes, double * pAspectRatio, RAW_DATA_TYPE eRawDataType)
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
			int Top, Bottom, Left,Right;
			int ResampleWidth, ResampleHeight;			
			TResampler * pResampler;		

			ComputeResampleDimension( &ResampleWidth, &ResampleHeight, &Top,&Bottom,&Left,&Right, AspectRatio, m_fDstRatio, m_nDesWidth, m_nDesHeight );

			pResampler = CreateResampler(RT_BILINEAR, ResampleWidth, ResampleHeight, m_nSrcWidth, m_nSrcHeight);			
			if( pResampler )
			{
				//delete m_pResampler;
				//m_pResampler = NULL;
				if( m_pResampler ) // 2010-04-23, jerry fix memory leak. 
				{
					ReleaseResampler( m_pResampler );
					m_pResampler = NULL;
				}
				m_nResampleWidth = ResampleWidth;
				m_nResampleHeight = ResampleHeight;
				m_nOffsetTop = Top;
				m_nOffsetBottom = Bottom;
				m_nOffsetLeft = Left;
				m_nOffsetRight = Right;			

				m_AspectRatio = AspectRatio;

				m_bAddLetterBox = false;
				m_bAddPillarBox = false;

				if( m_AspectRatio >= m_fDstRatio )
				{
					////////////////////
					// Add Letter Box
					if( m_nOffsetTop > 0 )
						m_bAddLetterBox = true;					
				}
				else
				{
					////////////////////
					// Add Pillar Box
					if( m_nOffsetLeft > 0 )
						m_bAddPillarBox = true;					
				}
				m_pResampler = pResampler;
			}
		}
	}

#if 1 
	switch(eRawDataType)
	{
	case RGB24:
		if(m_bKeepAspectRatio)
		{
			FillBox( pDes, RGB24 );			
			// Resized source			
			ResampleRGB24_2(m_pResampler, pDes + (m_nOffsetTop*m_nResampleStride*3) + m_nOffsetLeft, m_nResampleStride*3, pSrc, m_nSrcWidth*3 );			
		}
		else
		{
			ResampleRGB24(m_pResampler, pDes, pSrc);
		}
		break;
	case IYUV:
		if(m_bKeepAspectRatio)
		{
			FillBox( pDes, IYUV );
			// Resized source	
			BYTE * pDstY, *pDstU, * pDstV;
			BYTE * pSrcY, *pSrcU, * pSrcV;
			int UVDstStride = m_nResampleStride/2;

			pDstY = pDes;
			pDstU = pDes + m_nDesWidth*m_nDesHeight;
			pDstV = pDstU + m_nDesWidth*m_nDesHeight/4;

			pSrcY = pSrc;
			pSrcU = pSrc + m_nSrcWidth*m_nSrcHeight;
			pSrcV = pSrcU + m_nSrcWidth*m_nSrcHeight/4;

			ResampleYUV420_2( m_pResampler, 
							pDstY+(m_nOffsetTop*m_nResampleStride) + m_nOffsetLeft,
							pDstU+(m_nOffsetTop/2*UVDstStride) + m_nOffsetLeft/2,
							pDstV+(m_nOffsetTop/2*UVDstStride) + m_nOffsetLeft/2,
							m_nResampleStride,
							pSrcY, pSrcU, pSrcV, m_nSrcWidth );		
			
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
#endif 
}

void CRatioResampler2::Crop(BYTE *pSrc, BYTE *pDes, RAW_DATA_TYPE eRawDataType )
{
	//sanity checks on the fly. Makes sense if someone forgot to initialize us
	if( m_nDesHeight > m_nSrcHeight || m_nDesWidth > m_nSrcWidth 
		|| m_nOffsetTop < 0 || m_nOffsetLeft < 0 
		|| m_nDesHeight + m_nOffsetTop > m_nSrcHeight || m_nDesWidth + m_nOffsetLeft > m_nSrcWidth 
		)
	{
		return;
	}
	switch(eRawDataType)
	{
/*		case RGB24:
			{
				BYTE * pDstY, *pDstU, * pDstV;
				BYTE * pSrcY, *pSrcU, * pSrcV;
				int UVDstStride = m_nResampleStride/2;

				pDstY = pDes;

				pSrcY = pSrc + ( m_nOffsetTop * m_nSrcWidth + m_nOffsetLeft ) * 3;
				//plane Y
				for(int y = 0; y < m_nDesHeight; y++ )
				{
					memcpy( pDstY, pSrcY, m_nDesWidth * 3 );
					pDstY += m_nDesWidth * 3;
					pSrcY += m_nSrcWidth * 3;
				}
			}break; */
		case IYUV:
			{
				BYTE * pDstY, *pDstU, * pDstV;
				BYTE * pSrcY, *pSrcU, * pSrcV;

				pDstY = pDes;
				pDstU = pDes + m_nDesWidth*m_nDesHeight;
				pDstV = pDstU + m_nDesWidth*m_nDesHeight/4;

				pSrcY = pSrc;
				pSrcY += m_nOffsetTop * m_nSrcWidth + m_nOffsetLeft;
				pSrcU = pSrc + m_nSrcWidth*m_nSrcHeight;
				pSrcU += m_nOffsetTop / 2 * m_nSrcWidth / 2 + m_nOffsetLeft / 2;
				pSrcV = pSrcU + m_nSrcWidth / 2 * m_nSrcHeight / 2;
				pSrcV += 0;
				//plane Y
				for(int y = 0; y < m_nDesHeight; y++ )
				{
					memcpy( pDstY, pSrcY, m_nDesWidth );
					pDstY += m_nDesWidth;
					pSrcY += m_nSrcWidth;
				}
				//plane U
				for(int y = 0; y < m_nDesHeight / 2; y++ )
				{
					memcpy( pDstU, pSrcU, m_nDesWidth / 2 );
					pDstU += m_nDesWidth / 2;
					pSrcU += m_nSrcWidth / 2;
				}
				//plane V
				for(int y = 0; y < m_nDesHeight / 2; y++ )
				{
					memcpy( pDstV, pSrcV, m_nDesWidth / 2 );
					pDstV += m_nDesWidth / 2;
					pSrcV += m_nSrcWidth / 2;
				}
			}break;
	};
}