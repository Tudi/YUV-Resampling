//#include "SObjects.h"
//#include <windows.h>
#include "stdafx.h"
#include <stdlib.h>
#include "BicubicResampler.h"

#define MAXBYTE 0xff


#define SPACC   5               /* SubPixelACCuracy = 5 */
#define SCA 4096
#define SHIFT   12
#define HALF    (1<<11)

double cubic_coeff(double x)
{
		const double A=-0.6;
        
		if (x<0)
                x = -x;
        if (x>=2)
                return (0);
        else if (x>=1)
                return (double)(A*x*x*x -5*A*x*x +8*A*x -4*A);
        else
                return (double)((A+2)*x*x*x -(A+3)*x*x +1);
}

void scale_table(double c[5],int v[5])
{

	int i;
	for(i=0;i<5;i++)
	{
		v[i]=(int)(c[i]*SCA);
		
	}

}



CBicubicResampler::CBicubicResampler(int sw, int sh, int dw, int dh,int lumalow , int lumahigh ,int chromalow ,int chromahigh )
{
	
	double x_ratio;
	double y_ratio;
	int i,j;
	
	m_sw=sw;
	m_sh=sh;
	m_dw=dw;
	m_dh=dh;
	
	x_ratio=(double)sw/(double)dw;
	y_ratio=(double)sh/(double)dh;
	
	UINT dw_32;
	dw_32=(dw+31) & ~31;

	//m_pInter=(BYTE*)MemCalloc((sh+4)*dw_32,sizeof(BYTE));
	m_pInter=(BYTE*)calloc((sh+4)*dw_32,sizeof(BYTE));
	if(!m_pInter)
	{
		throw(-1);
		return;
	}
	
	m_InterStride=dw_32;
	m_pInter+=2*dw_32;

	//m_pWidthTable=(T_TABLE_DATA*)MemCalloc(dw,sizeof(T_TABLE_DATA));
	//m_pHeightTable=(T_TABLE_DATA*)MemCalloc(dh,sizeof(T_TABLE_DATA));
	m_pWidthTable=(T_TABLE_DATA*)calloc(dw,sizeof(T_TABLE_DATA));
	m_pHeightTable=(T_TABLE_DATA*)calloc(dh,sizeof(T_TABLE_DATA));
	
	
	if( (!m_pWidthTable)||(!m_pHeightTable))
	{
		if(m_pWidthTable)
		{
			//MemFree(m_pWidthTable);
			free(m_pWidthTable);
			m_pWidthTable=NULL;
		}
		if(m_pHeightTable)
		{
			//MemFree(m_pHeightTable);
			free(m_pHeightTable);
			m_pHeightTable=NULL;
		}
		
		throw(-1);
		//should not get here..
		return;
	}
	
	for(i=0;i<dw;i++)
	{
		double tmp;
		double dist;
		tmp=(x_ratio*(double)i);
		m_pWidthTable[i].index=	(int)tmp ;

/*		if(m_pWidthTable[i].index<2)
			m_pWidthTable[i].index=2;

		if(m_pWidthTable[i].index>(width1-3))
			m_pWidthTable[i].index=width1-3;
*/

		dist= tmp- (double)((int)tmp);


		double frac =dist;
		double sub;
		/* find the closest subpixel position for frac */
		sub = (double) ((int)(frac*SPACC + 0.5)/(double)SPACC);

/*		m_pWidthTable[i].coeff[0] = (int) (cubic_coeff(-sub-2)*SCA);
		m_pWidthTable[i].coeff[1] = (int) (cubic_coeff(-sub-1)*SCA);
		m_pWidthTable[i].coeff[2] = (int) (cubic_coeff(sub)*SCA);
		m_pWidthTable[i].coeff[3] = (int) (cubic_coeff(1-sub)*SCA);
		m_pWidthTable[i].coeff[4] = (int) (cubic_coeff(2-sub)*SCA);

*/
		double c[5];
		for(j=0;j<5;j++)
		{
			if ( ( (m_pWidthTable[i].index -2+j )<0)||
			 	 ( (m_pWidthTable[i].index -2+j )>(m_sw-1))
			   )
				c[j]=0;
			else
				c[j]=cubic_coeff(-sub+j-2);
		}
		scale_table(c,m_pWidthTable[i].coeff);
	
	}

	for(i=0;i<dh;i++)
	{	
		double tmp;
		double dist;
		tmp=(y_ratio*(double)i);
		m_pHeightTable[i].index= (int)tmp ;
/*
		if(m_pHeightTable[i].index < 2)
			m_pHeightTable[i].index=2;

		if(m_pHeightTable[i].index>(height1-3))
			m_pHeightTable[i].index=height1-3;
*/
		dist= tmp- (double)((int)tmp);

	
		double frac = dist;
		double sub;
		/* find the closest subpixel position for frac */
		sub = (double) ((int)(frac*SPACC + 0.5)/(double)SPACC);
/*		m_pHeightTable[i].coeff[0] = (int) (cubic_coeff(-sub-2)*SCA);
		m_pHeightTable[i].coeff[1] = (int) (cubic_coeff(-sub-1)*SCA);
		m_pHeightTable[i].coeff[2] = (int) (cubic_coeff(sub)*SCA);
		m_pHeightTable[i].coeff[3] = (int) (cubic_coeff(1-sub)*SCA);
		m_pHeightTable[i].coeff[4] = (int) (cubic_coeff(2-sub)*SCA);
*/
		double c[5];
		for(j=0;j<5;j++)
		{
			if ( ( (m_pHeightTable[i].index -2+j )<0)||
			 	 ( (m_pHeightTable[i].index -2+j )>(m_sh-1))
			   )
				c[j]=0;
			else
				c[j]=cubic_coeff(-sub+j-2);
		}
		scale_table(c,m_pHeightTable[i].coeff);

	}




	for( i = 0; i < 2048; i++ )
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

CBicubicResampler::~CBicubicResampler()
{

	if(m_pWidthTable)
		//MemFree(m_pWidthTable);
		free(m_pWidthTable);
	if(m_pHeightTable)
		//MemFree(m_pHeightTable);
		free(m_pHeightTable);
	
	if(m_pInter)
	{
		m_pInter-=2*m_InterStride;
		//MemFree(m_pInter);
		free(m_pInter);
	}
};

void CBicubicResampler::Resample(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride,int clipindex)
{
	
	Resample_H(pSource, nSourceStride, m_pInter, m_InterStride);
	Resample_V(m_pInter, m_InterStride, pDest, nDestStride,clipindex);
}





//
//
// dest is intermediate buffer
//
//
//
void CBicubicResampler::Resample_H(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride)
{
	int vx,vy;
	int dw=m_dw;
	int sh=m_sh;

	for(vy = 0;vy < sh; vy++)  
	{
		for(vx = 0;vx < dw; vx++)
		{
			int  result;
			int rx;
			int  a,b,c,d,e;
			//index
			rx =  m_pWidthTable[vx].index;
			
			a= pSource[rx-2];
			b= pSource[rx-1];
			c= pSource[rx];
			d= pSource[rx+1];
			e= pSource[rx+2];
			
			/* compute convolution using info from htable */
			result =  
				  (int) a  * m_pWidthTable[vx].coeff[0]
				+ (int) b  * m_pWidthTable[vx].coeff[1]
				+ (int) c  * m_pWidthTable[vx].coeff[2]
				+ (int) d  * m_pWidthTable[vx].coeff[3]
				+ (int) e  * m_pWidthTable[vx].coeff[4];

			/* convert to 8-bits */
			result = (result + HALF)>>SHIFT;
			if (result < 0) result = 0;
			if (result > MAXBYTE) result = MAXBYTE;
			
			pDest[vx]=(BYTE)result;

		}
		pSource+=nSourceStride;
		pDest+=nDestStride;
	}
	

}





void CBicubicResampler::Resample_V(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex)
{
	int vx,vy;
	int dw=m_dw;
	int dh=m_dh;

	for(vy = 0;vy < dh; vy++)  
	{
		BYTE *s0,*s1,*s2,*s3,*s4;
		
		//upper line 
		s0=pSource+m_pHeightTable[vy].index*nSourceStride-2*nSourceStride;
		s1=s0+nSourceStride;
		s2=s1+nSourceStride;
		s3=s2+nSourceStride;
		s4=s3+nSourceStride;
		
		for(vx = 0;vx < dw; vx++)
		{
			int result;
			int  a,b,c,d,e;
			
			a= s0[vx];
			b= s1[vx];
			c= s2[vx];
			d= s3[vx];
			e= s4[vx];
			
		
			/* compute convolution using info from htable */
			result =  
				  (int) a * m_pHeightTable[vy].coeff[0]
				+ (int) b * m_pHeightTable[vy].coeff[1]
				+ (int) c * m_pHeightTable[vy].coeff[2]
				+ (int) d * m_pHeightTable[vy].coeff[3]
				+ (int) e * m_pHeightTable[vy].coeff[4];

			/* convert to 8-bits */
			result = (result + HALF)>>SHIFT;
			if (result < 0) result = 0;
			if (result > MAXBYTE) result = MAXBYTE;
			
			pDest[vx]=(BYTE)result;

		}
	
		pDest+=nDestStride;
	}
	

}



