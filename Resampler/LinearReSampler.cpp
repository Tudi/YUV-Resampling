#include "LinearReSampler.h"
#include <string.h>
#include <assert.h>

#ifndef MAX
	#define  MAX(a,b)		(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
	#define  MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

//note that this is mostly tested when in/out size is dividable by 16. Using Odd numbers might give issues at rounding
void ResampleYUV420Liniar( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int DestW, int DestH )
{
	//y
	unsigned int int_conv_y = (unsigned int)( SrcH * FLOAT_PRECISSION / DestH );
	unsigned int int_conv_x = (unsigned int)( SrcW * FLOAT_PRECISSION / DestW );

#ifdef _DEBUG
	assert( ( ( ( int_conv_y * DestH ) >> FLOAT_PRECISSION_BITS ) ) <= SrcH );
	assert( ( ( ( int_conv_x * DestW ) >> FLOAT_PRECISSION_BITS ) ) <= SrcW );
	assert( ( ( ( int_conv_y * DestH ) >> FLOAT_PRECISSION_BITS ) * ( (int_conv_x * DestW ) >> FLOAT_PRECISSION_BITS ) * 3 / 2 ) <= SrcW * SrcH * 3 / 2 );
#endif

	//downsampling row count
	if( DestH <= SrcH )
	{
		unsigned int stacking_precission_y = 0;
		for( int y=0;y<DestH;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char   *tsrc = src + converted_row_index * SrcW;
			stacking_precission_y += int_conv_y;

			unsigned int start = y * DestW;
			unsigned int end = start + DestW;
#ifdef _DEBUG
			assert( converted_row_index <= SrcH );
			assert( dst + end <= dst + DestW * DestH );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= SrcW );
#endif
				dst[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
		}
		//U
		stacking_precission_y = 0;
		unsigned char * dstU = dst + DestH * DestW;
		unsigned char * srcU = src + SrcH * SrcW;
		for( int y=0;y<DestH/2;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char *			tsrc = srcU + converted_row_index * ( SrcW / 2 );
			stacking_precission_y += int_conv_y;

			unsigned int start = y * ( DestW / 2 );
			unsigned int end = start + ( DestW / 2 );
#ifdef _DEBUG
			assert( converted_row_index <= ( SrcH / 2 ) );
			assert( dstU + end <= dst + DestW * DestH +  + ( DestW / 2 ) * ( DestH / 2 ) );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= ( SrcW / 2 ) );
#endif
				dstU[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
		}
		//V
		stacking_precission_y = 0;
		unsigned char * dstV = dstU +( DestH / 2 ) * ( DestW / 2 );
		unsigned char * srcV = srcU + ( SrcH / 2 ) * ( SrcW / 2 );
		for( int y=0;y<DestH/2;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char *			tsrc = srcV + converted_row_index * ( SrcW / 2 );
			stacking_precission_y += int_conv_y;

			unsigned int start = y * ( DestW / 2 );
			unsigned int end = start + ( DestW / 2 );
#ifdef _DEBUG
			assert( converted_row_index <= ( SrcH / 2 ) );
			assert( dstV + end <= dst + DestW * DestH * 3 / 2 );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= ( SrcW / 2 ) );
#endif
				dstV[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
		}
	}
	//upsampling
	else
	{
		unsigned int stacking_precission_y = 0;
		for( int y=0;y<DestH;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char   *tsrc = src + converted_row_index * SrcW;
			stacking_precission_y += int_conv_y;

			unsigned int start = y * DestW;
			unsigned int end = start + DestW;
#ifdef _DEBUG
			assert( converted_row_index <= SrcH );
			assert( dst + end <= dst + DestW * DestH );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= SrcW );
#endif
				dst[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
			//works only for upsampling. Which is what slows us down
			unsigned char *DuplicateRow = &dst[ start ];
			while( converted_row_index == ( stacking_precission_y >> FLOAT_PRECISSION_BITS ) && y + 1 < DestH )
			{
				y++;
				memcpy( dst + y * DestW, DuplicateRow, DestW );
				stacking_precission_y += int_conv_y;
			}
		}
		//U
		stacking_precission_y = 0;
		unsigned char * dstU = dst + DestH * DestW;
		unsigned char * srcU = src + SrcH * SrcW;
		for( int y=0;y<DestH/2;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char	*tsrc = srcU + converted_row_index * ( SrcW / 2 );
			stacking_precission_y += int_conv_y;

			unsigned int start = y * ( DestW / 2 );
			unsigned int end = start + ( DestW / 2 );
#ifdef _DEBUG
			assert( converted_row_index <= ( SrcH / 2 ) );
			assert( dstU + end <= dst + DestW * DestH +  + ( DestW / 2 ) * ( DestH / 2 ) );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= ( SrcW / 2 ) );
#endif
				dstU[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
			//works only for upsampling. Which is what slows us down
			unsigned char *DuplicateRow = &dstU[ start ];
			while( converted_row_index == ( stacking_precission_y >> FLOAT_PRECISSION_BITS ) && y + 1 < ( DestH / 2 ) )
			{
				y++;
				memcpy( dstU + y * ( DestW / 2 ), DuplicateRow, ( DestW / 2 ) );
				stacking_precission_y += int_conv_y;
			}
		}
		//V
		stacking_precission_y = 0;
		unsigned char * dstV = dstU + ( DestH / 2 ) * ( DestW / 2 );
		unsigned char * srcV = srcU + ( SrcH / 2 ) * ( SrcW / 2 );
		for( int y=0;y<DestH/2;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char *			tsrc = srcV + converted_row_index * ( SrcW / 2 );
			stacking_precission_y += int_conv_y;

			unsigned int start = y * ( DestW / 2 );
			unsigned int end = start + ( DestW / 2 );
#ifdef _DEBUG
			assert( converted_row_index <= ( SrcH / 2 ) );
			assert( dstV + end <= dst + DestW * DestH * 3 / 2 );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= ( SrcW / 2 ) );
#endif
				dstV[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
			//works only for upsampling. Which is what slows us down
			unsigned char *DuplicateRow = &dstV[ start ];
			while( converted_row_index == ( stacking_precission_y >> FLOAT_PRECISSION_BITS ) && y + 1 < ( DestH / 2 ) )
			{
				y++;
				memcpy( dstV + y * ( DestW / 2 ), DuplicateRow, ( DestW / 2 ) );
				stacking_precission_y += int_conv_y;
			}
		}
	}
}

void ResampleYUV420LiniarInbox( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int SrcStride, int DestW, int DestH, int DestBuffStride, int DestBuffHeight, int DestSartX, int DestStartY )
{
	//y
	unsigned int int_conv_y = (unsigned int)( SrcH * FLOAT_PRECISSION / DestH );
	unsigned int int_conv_x = (unsigned int)( SrcW * FLOAT_PRECISSION / DestW );

#ifdef _DEBUG
	assert( ( ( ( int_conv_y * DestH ) >> FLOAT_PRECISSION_BITS ) ) <= SrcH );
	assert( ( ( ( int_conv_x * DestW ) >> FLOAT_PRECISSION_BITS ) ) <= SrcW );
	assert( ( ( ( int_conv_y * DestH ) >> FLOAT_PRECISSION_BITS ) * ( (int_conv_x * DestW ) >> FLOAT_PRECISSION_BITS ) * 3 / 2 ) <= SrcW * SrcH * 3 / 2 );
#endif

	//downsampling row count
	if( DestH <= SrcH )
	{
		unsigned int stacking_precission_y = 0;
		for( int y=0;y<DestH;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char   *tsrc = src + converted_row_index * SrcStride;
			stacking_precission_y += int_conv_y;

			unsigned int start = ( y + DestStartY ) * DestBuffStride + DestSartX;
			unsigned int end = start + DestW;
#ifdef _DEBUG
			assert( converted_row_index <= SrcH );
			assert( dst + end <= dst + DestBuffStride * DestBuffHeight );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= SrcW );
#endif
				dst[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
		}
		//U
		stacking_precission_y = 0;
		unsigned char * dstU = dst + DestBuffHeight * DestBuffStride;
		unsigned char * srcU = src + SrcH * SrcStride;
		for( int y=0;y<DestH/2;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char *			tsrc = srcU + converted_row_index * SrcStride / 2;
			stacking_precission_y += int_conv_y;

			unsigned int start = ( y + DestStartY / 2 ) * ( DestBuffStride / 2 ) + DestSartX / 2;
			unsigned int end = start + ( DestW / 2 );
#ifdef _DEBUG
			assert( converted_row_index <= ( SrcH / 2 ) );
			assert( dstU + end <= dst + DestBuffStride * DestBuffHeight +  + DestBuffStride / 2 * DestBuffHeight / 2 );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= ( SrcW / 2 ) );
#endif
				dstU[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
		}
		//V
		stacking_precission_y = 0;
		unsigned char * dstV = dstU + DestBuffHeight / 2 * DestBuffStride / 2;
		unsigned char * srcV = srcU + ( SrcH / 2 ) * SrcStride / 2;
		for( int y=0;y<DestH/2;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char *			tsrc = srcV + converted_row_index * SrcStride / 2;
			stacking_precission_y += int_conv_y;

			unsigned int start = ( y + DestStartY / 2 ) * ( DestBuffStride / 2 ) + DestSartX / 2;
			unsigned int end = start + ( DestW / 2 );
#ifdef _DEBUG
			assert( converted_row_index <= ( SrcH / 2 ) );
			assert( dstV + end <= dst + DestBuffStride * DestBuffHeight * 3 / 2 );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= ( SrcW / 2 ) );
#endif
				dstV[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
		}
	}
	//upsampling
	else
	{
		unsigned int stacking_precission_y = 0;
		for( int y=0;y<DestH;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char   *tsrc = src + converted_row_index * SrcStride;
			stacking_precission_y += int_conv_y;

			unsigned int start = ( y + DestStartY ) * DestBuffStride + DestSartX;
			unsigned int end = start + DestW;
#ifdef _DEBUG
			assert( converted_row_index <= SrcH );
			assert( dst + end <= dst + DestBuffStride * DestBuffHeight );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= SrcW );
#endif
				dst[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
			//works only for upsampling. Which is what slows us down
			unsigned char *DuplicateRow = &dst[ start ];
			while( converted_row_index == ( stacking_precission_y >> FLOAT_PRECISSION_BITS ) && y + 1 + DestStartY < DestH )
			{
				y++;
				memcpy( dst + ( y + DestStartY ) * DestBuffStride + DestSartX, DuplicateRow, DestW );
				stacking_precission_y += int_conv_y;
			}
		}
		//U
		stacking_precission_y = 0;
		unsigned char * dstU = dst + DestBuffHeight * DestBuffStride;
		unsigned char * srcU = src + SrcH * SrcStride;
		for( int y=0;y<(DestH+1)/2;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char *			tsrc = srcU + converted_row_index * SrcStride / 2;
			stacking_precission_y += int_conv_y;

			unsigned int start = ( y + DestStartY / 2 ) * ( DestBuffStride / 2 ) + DestSartX / 2;
			unsigned int end = start + ( DestW + 1 ) / 2;
#ifdef _DEBUG
			assert( converted_row_index <= ( SrcH / 2 ) );
			assert( dstU + end <= dst + DestBuffStride * DestBuffHeight +  + DestBuffStride / 2 * DestBuffHeight / 2 );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= ( SrcW / 2 ) );
#endif
				dstU[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
			//works only for upsampling. Which is what slows us down
			unsigned char *DuplicateRow = &dstU[ start ];
			while( converted_row_index == ( stacking_precission_y >> FLOAT_PRECISSION_BITS ) && y + 1 + DestStartY / 2 < ( DestH / 2 ) )
			{
				y++;
				memcpy( dstU + ( y + DestStartY / 2 ) * ( DestBuffStride / 2 ) + DestSartX / 2, DuplicateRow, ( DestW + 1 ) / 2 );
				stacking_precission_y += int_conv_y;
			}
		}
		//V
		stacking_precission_y = 0;
		unsigned char * dstV = dstU + DestBuffHeight / 2 * DestBuffStride / 2;
		unsigned char * srcV = srcU + ( SrcH / 2 ) * SrcStride / 2;
		for( int y=0;y<(DestH+1)/2;y++)
		{
			unsigned int stacking_precission_x = 0;

			unsigned int	converted_row_index = stacking_precission_y >> FLOAT_PRECISSION_BITS;
			unsigned char *			tsrc = srcV + converted_row_index * SrcStride / 2;
			stacking_precission_y += int_conv_y;

			unsigned int start = ( y + DestStartY / 2 ) * ( DestBuffStride / 2 ) + DestSartX / 2;
			unsigned int end = start + ( DestW + 1 ) / 2;
#ifdef _DEBUG
			assert( converted_row_index <= ( SrcH / 2 ) );
			assert( dstV + end <= dst + DestBuffStride * DestBuffHeight * 3 / 2 );
#endif

			for( unsigned int x=start;x<end;x++)
			{
				int converted_col_index = stacking_precission_x >> FLOAT_PRECISSION_BITS;
#ifdef _DEBUG
				assert( converted_col_index <= ( SrcW / 2 ) );
#endif
				dstV[ x ] = tsrc[ converted_col_index ];
				stacking_precission_x += int_conv_x;
			}
			//works only for upsampling. Which is what slows us down
			unsigned char *DuplicateRow = &dstV[ start ];
			while( converted_row_index == ( stacking_precission_y >> FLOAT_PRECISSION_BITS ) && y + 1 + DestStartY / 2 < ( DestH / 2 ) )
			{
				y++;
				memcpy( dstV + ( y + DestStartY / 2 ) * ( DestBuffStride / 2 ) + DestSartX / 2, DuplicateRow, ( DestW + 1 ) / 2 );
				stacking_precission_y += int_conv_y;
			}
		}
	}
}

void AddBlackBoxYUV420( unsigned char *dst, int bufWidth, int bufHeight, int BoxStartX, int BoxStartY, int boxWidth, int boxHeight )
{
#ifdef _DEBUG
	assert( bufWidth >= BoxStartX + boxWidth && bufHeight >= boxHeight + BoxStartY );
	assert( BoxStartX >= 0 && BoxStartY >= 0 );
	assert( bufWidth >= boxWidth && bufHeight >= boxHeight );
#endif
	//avoid out of bounds buffer processing if possible
	if( bufWidth < BoxStartX + boxWidth )
		BoxStartX = bufWidth - boxWidth;
	if( bufHeight < boxHeight + BoxStartY )
		BoxStartY = bufHeight - boxHeight;
	//get the params we need
	unsigned char *dstY = dst + bufWidth * BoxStartY + BoxStartX;
	unsigned char *dstU = dst + bufWidth * bufHeight + bufWidth / 2 * BoxStartY / 2 + BoxStartX / 2;
	unsigned char *dstV = dstU + bufWidth / 2 * bufHeight / 2;
	//process the buffer
	if( boxWidth == bufWidth )
	{
		memset( dstY, 16, boxWidth * boxHeight );
		memset( dstU, 128, boxWidth / 2 * boxHeight / 2 );
		memset( dstV, 128, boxWidth / 2 * boxHeight / 2 );
	}
	else
	{
		for( int y = 0; y < boxHeight; y++ )
			memset( dstY + y * bufWidth, 16, boxWidth );
		for( int y = 0; y < boxHeight / 2; y++ )
		{
			memset( dstU + y * bufWidth / 2, 128, boxWidth / 2 );
			memset( dstV + y * bufWidth / 2, 128, boxWidth / 2 );
		}
	}
}

void ResampleYUV420Liniar( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int DestW, int DestH, bool KeepAspectRatio  )
{
	if( KeepAspectRatio == true )
	{
		// using int for the sake of rounding
		int ScaleBoth;
		int ScaleWidth = ( DestW * FLOAT_PRECISSION ) / SrcW;
		int ScaleHeigth = ( DestH * FLOAT_PRECISSION ) / SrcH;
		if( ScaleWidth < ScaleHeigth )
			ScaleBoth = ScaleWidth;
		else
			ScaleBoth = ScaleHeigth;
		// calc up / down , left / right box size
		int NewHeight = MIN( ( SrcH * ScaleBoth + FLOAT_PRECISSION / 2 ) / FLOAT_PRECISSION, DestH );
		int NewWidth = MIN( ( SrcW * ScaleBoth + FLOAT_PRECISSION / 2 ) / FLOAT_PRECISSION, DestW );
		int RemainingGapSizeHeight = DestH - NewHeight;
		int RemainingGapSizeWidth = DestW - NewWidth;
		//apply boxes if needed. Worst case we could init the whole output image. That would be bugfree and slower in some cases
		if( RemainingGapSizeHeight > 0 )
		{
			AddBlackBoxYUV420( dst, DestW, DestH, 0, 0, DestW, RemainingGapSizeHeight / 2 );
			AddBlackBoxYUV420( dst, DestW, DestH, 0, RemainingGapSizeHeight / 2 + NewHeight, DestW, DestH - NewHeight - RemainingGapSizeHeight / 2 );
		}
		else if( RemainingGapSizeWidth > 0 )
		{
			AddBlackBoxYUV420( dst, DestW, DestH, 0, 0, RemainingGapSizeWidth / 2, DestH );
			AddBlackBoxYUV420( dst, DestW, DestH, RemainingGapSizeWidth / 2 + NewWidth, 0, DestW - NewWidth - RemainingGapSizeWidth / 2, DestH );
		} 
		// scale the picture in the remaining box
		ResampleYUV420LiniarInbox( src, dst, SrcW, SrcH, SrcW, NewWidth, NewHeight, DestW, DestH, RemainingGapSizeWidth / 2, RemainingGapSizeHeight / 2 );
	}
	else 
//		if( KeepAspectRatio == false )
	{
		ResampleYUV420Liniar( src, dst, SrcW, SrcH, DestW, DestH );
	}
}