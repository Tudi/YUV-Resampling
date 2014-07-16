// **********************************************************************************************
//                              Copyright (C) 2000  - SPLASH Software 
//
//                    Use or duplication without written consent prohibitted
// **********************************************************************************************
//#include "SObjects.h"
//#include <windows.h>
#include "stdafx.h"
#include <stdlib.h>


#include "2DLinearResampler.h"

C2DLinearResampler::C2DLinearResampler(int _width1, int _height1, int _width2, int _height2,int _lumalow , int _lumahigh , int _chromalow ,int  _chromahigh )
					: width1(_width1)
					, width2(_width2)
					, height1(_height1)
					, height2(_height2)
					,lumalow(_lumalow)
					,lumahigh(_lumahigh)
					,chromalow(_chromalow)
					,chromahigh(_chromahigh)
{
	if(Setup_Linear_Tables()==0)
		throw(-1);
		
}

C2DLinearResampler::~C2DLinearResampler()
{
	Destroy_Linear_Tables();
}


int C2DLinearResampler::Setup_Linear_Tables()
{
	int i, j;
	int pad_in_x, pad_in_y;
	double subfactor_x;
	double subfactor_y;
/*
	inX = (UINT*) MemAlloc(width2*sizeof(UINT));
	inY = (UINT*) MemAlloc(height2*sizeof(UINT));
	diffX = (int*) MemAlloc(width2*sizeof(int));
	diffY = (int*) MemAlloc(height2*sizeof(int));
*/
	inX = (UINT*) malloc(width2*sizeof(UINT));
	inY = (UINT*) malloc(height2*sizeof(UINT));
	diffX = (int*) malloc(width2*sizeof(int));
	diffY = (int*) malloc(height2*sizeof(int));
	if( (inX==NULL) || (inY==NULL) || (diffX==NULL) || (diffY==NULL) )
	{
		if(inX)
		{
			delete inX;
			inX=NULL;
		}
		if(inY)
		{
			delete inY;
			inY=NULL;
		}
		if(diffX)
		{
			delete diffX;
			diffX=NULL;
		}
		if(diffY)
		{
			delete diffY;
			diffY=NULL;
		}
		return 0;
		//fprintf(stderr, "malloc error in Setup_Linear_Resampling");
	}

	subfactor_x = (double)width1/(double)width2;
	subfactor_y = (double)height1/(double)height2;

	pad_in_x = 0;
	pad_in_y = 0;

	//memset(clip_tab[0], 0,2048);
	//memset(clip_tab[1],0, 2048);
	//memset(clip_tab[2],0, 2048);


	for( j = 0; j < height2; j++ )
	{
		inY[j] = (int)(j * subfactor_y) + pad_in_y;
		diffY[j] = (int)((double)(j * subfactor_y - inY[j])*65536);
	}

	for( i = 0; i < width2; i++ )
	{
		inX[i] = (int)(i * subfactor_x ) + pad_in_x; // note, we need the full multiplication here; do not use +=
		diffX[i] = (int)((double)(i * subfactor_x - inX[i])*65536);
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
	return 1;
}

void C2DLinearResampler::Destroy_Linear_Tables()
{
	/*
	if(inX)
		MemFree(inX);
	if(inY)
		MemFree(inY);
	if(diffX)
		MemFree(diffX);
	if(diffY)
		MemFree(diffY);
		*/
	if(inX)
		free(inX);
	if(inY)
		free(inY);
	if(diffX)
		free(diffX);
	if(diffY)
		free(diffY);
}

//
// Performs generic linear resampling of two blocks of byte values.
//
void C2DLinearResampler::Resample(PBYTE address1, // base address of block 1
								  int   stride1,  // stride of block 1
								  PBYTE address2, // base address of block 2
								  int   stride2, // stride of block 2
								  int clipindex // index to be used for pClipTab
								  )  
{
	int d_dx, d_dy;
	int i, j;
	int in_pixel;
	int deltax1=1, deltay1=1; 
	register int inx, iny;	// current output and input (x,y) coords
	//int w_a_s1, w_a_s2;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);

	//w_a_s1 = width1+stride1;
	//w_a_s2 = width2+stride2;

	for (j = 0; j < height2; j++)
	{
		iny = inY[j];
		(iny < height1-1) ? (deltay1 = 1) : (deltay1 = 0);
		iny *= stride1;
		for (i = 0; i < width2; i++)
		{
			register INT tmp;

			inx = inX[i];
			in_pixel = iny + inx;
			(inx < width1-1) ? (deltax1 = 1) : (deltax1 = 0);
			// [f(x+dx,y) - f(x,y)]/dx
			d_dx = (int)(address1[in_pixel+deltax1] - address1[in_pixel]);	// deltax; - division left out because deltax=1
			// [f(x,y+dy) - f(x,y)]/dy
			d_dy = (int)(address1[in_pixel+ stride1 * deltay1] - address1[in_pixel]);	//deltay; - same reason
			
			tmp = (INT)(address1[in_pixel] + (int)((d_dx * diffX[i] + d_dy * diffY[j])>>16));
			address2[i] = pClipTab[tmp];
		}
		address2 += stride2;
	}
}
void C2DLinearResampler::ChangeColourTemprature(PBYTE address1, // base address of block 1
								  int   stride1,  // stride of block 1
								  PBYTE address2, // base address of block 2
								  int   stride2, // stride of block 2
								  int clipindex // index to be used for pClipTab
								  ) 
{
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	int i,j;
	for (j = 0; j < height2; j++)
	{
	
		for (i = 0; i < width2; i++)
		{
			address2[i] = pClipTab[address1[i]];
			/*if((address1[i] >=16) &&  (address1[i] <=235))
				if( address1[i] != address2[i])
					__asm int 3; */
			/* if(clipindex==1)
			{
		    	if(address1[i] > 240 )
			    	address2[i] = 240;
		    	else if(address1[i] < 16 )
                    address2[i] = 16;
			    else
				    address2[i] = address1[i];
			}
			else
			if(clipindex==2)
			{
		    	if(address1[i] > 240 )
			    	address2[i] = 240;
		    	else if(address1[i] < 16 )
                    address2[i] = 16;
			    else
				    address2[i] = address1[i];
			}
			else
				address2[i] = address1[i]; */




		}
		address2 += stride2;
		address1 += stride1;
	}


}

//
// Performs generic linear resampling of two blocks of byte values.
//
void C2DLinearResampler::ResampleRGB32(PBYTE address1, // base address of block 1
								  int   stride1,  // stride of block 1
								  PBYTE address2, // base address of block 2
								  int   stride2,
								  int clipindex // index to be used for pClipTab
								)  // stride of block 2
{
	int d_dx, d_dy;
	int i, j,di;
	int in_pixel;
//	int deltax1=1, deltay1=1; 
	register int inx, iny;	// current output and input (x,y) coords
	int next_y_pixel;
	int next_x_pixel;

	//int w_a_s1, w_a_s2;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//w_a_s1 = width1+stride1;
	//w_a_s2 = width2+stride2;

	for (j = 0; j < height2; j++)
	{
		iny = inY[j];
		iny *= stride1;
	
		if(iny < height1-1) 
			next_y_pixel = iny + stride1;
		else
			next_y_pixel = 0;
	
		for (di=0,i = 0; i < width2; i++,di+=4)
		{
			register INT tmp;

			inx = inX[i];
			if(inx < width1-1) 
				next_x_pixel = 4;
			else next_x_pixel = 0;
			
			
			for(int q=0;q<4; q++)
			{
				in_pixel = iny + 4*inx+q;
				// [f(x+dx,y) - f(x,y)]/dx
				d_dx = (int)(address1[in_pixel + next_x_pixel] - address1[in_pixel]);	// deltax; - division left out because deltax=1
				// [f(x,y+dy) - f(x,y)]/dy
				d_dy = (int)(address1[in_pixel + next_y_pixel] - address1[in_pixel]);	//deltay; - same reason
			
				tmp = (INT)(address1[in_pixel] + (int)((d_dx * diffX[i] + d_dy * diffY[j])>>16));
				address2[di+q] = pClipTab[tmp];
			}
			//alpha
			//address2[di+3]=0xFF;
		}
		
		address2 += stride2;
	}
}
//
// Performs generic linear resampling of two blocks of byte values.
//
void C2DLinearResampler::ResampleRGB24(PBYTE address1, // base address of block 1
								  int   stride1,  // stride of block 1
								  PBYTE address2, // base address of block 2
								  int   stride2,
								  int clipindex // index to be used for pClipTab
								)  // stride of block 2
{
	int d_dx, d_dy;
	int i, j,di;
	int in_pixel;
//	int deltax1=1, deltay1=1; 
	register int inx, iny;	// current output and input (x,y) coords
	int next_y_pixel;
	int next_x_pixel;

	//int w_a_s1, w_a_s2;
	BYTE *pClipTab = &(clip_tab[clipindex][1024]);
	
	//w_a_s1 = width1+stride1;
	//w_a_s2 = width2+stride2;

	for (j = 0; j < height2; j++)
	{
		iny = inY[j];
		iny *= stride1;
	
		if(iny < height1-1) 
			next_y_pixel = iny + stride1;
		else
			next_y_pixel = 0;
	
		for (di=0,i = 0; i < width2; i++,di+=3)
		{
			register INT tmp;

			inx = inX[i];
			if(inx < width1-1) 
				next_x_pixel = 3;
			else next_x_pixel = 0;
			
			
			for(int q=0;q<3; q++)
			{
				in_pixel = iny + 3*inx+q;
				// [f(x+dx,y) - f(x,y)]/dx
				d_dx = (int)(address1[in_pixel + next_x_pixel] - address1[in_pixel]);	// deltax; - division left out because deltax=1
				// [f(x,y+dy) - f(x,y)]/dy
				d_dy = (int)(address1[in_pixel + next_y_pixel] - address1[in_pixel]);	//deltay; - same reason
			
				tmp = (INT)(address1[in_pixel] + (int)((d_dx * diffX[i] + d_dy * diffY[j])>>16));
				address2[di+q] = pClipTab[tmp];
			}
			//alpha
			//address2[di+3]=0xFF;
		}
		
		address2 += stride2;
	}
}
//
// Performs generic linear resampling of two blocks of byte values.
//
void C2DLinearResampler::ResampleBW1Bit(PBYTE address1, // base address of block 1
								  int   stride1,  // stride of block 1
								  PBYTE address2, // base address of block 2
								  int   stride2, // stride of block 2
								  int clipindex // index to be used for pClipTab
								  )  
{
	int d_dx, d_dy;
	int i, j;
	int in_pixel;
	int deltax1=1, deltay1=1; 
	register int inx, iny;	// current output and input (x,y) coords
	//int w_a_s1, w_a_s2;
//	BYTE *pClipTab = &(clip_tab[clipindex][1024]);

	//w_a_s1 = width1+stride1;
	//w_a_s2 = width2+stride2;

	for (j = 0; j < height2; j++)
	{
		iny = inY[j];
		(iny < height1-1) ? (deltay1 = 1) : (deltay1 = 0);
		iny *= stride1;
		for (i = 0; i < width2; i++)
		{
			register INT tmp;

			inx = inX[i];
			in_pixel = iny + inx;
			(inx < width1-1) ? (deltax1 = 1) : (deltax1 = 0);
			// [f(x+dx,y) - f(x,y)]/dx
			d_dx = (int)(address1[in_pixel+deltax1] - address1[in_pixel]);	// deltax; - division left out because deltax=1
			// [f(x,y+dy) - f(x,y)]/dy
			d_dy = (int)(address1[in_pixel+ stride1 * deltay1] - address1[in_pixel]);	//deltay; - same reason
			
			tmp = (INT)(address1[in_pixel] + (int)((d_dx * diffX[i] + d_dy * diffY[j])>>16));
			if(tmp)
				address2[i] = 1;
			else
				address2[i] = 0;
		}
		address2 += stride2;
	}
}

