#ifndef _LINEAR_RESAMPLER_SSE3_H_
#define _LINEAR_RESAMPLER_SSE3_H_

//this was required for old hardware where float to int conversion is slow. In the future you might be able to use float indexing directly
#define FLOAT_PRECISSION_BITS		15		// values are on 16 bits, using more for float precission does not count. Make sure to not have this number too big !
#define FLOAT_PRECISSION			( 1 << FLOAT_PRECISSION_BITS )
#define FLOAT_PRECISSION_ROUNUPER	( FLOAT_PRECISSION - 1 ) //add this before division for roundup effect

//in this state this is not optimized for speed. Should be better then biliniar resampling tough
void ResampleYUV420LiniarSSE3( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int DestW, int DestH, bool KeepAspectRatio );
void ResampleYUV420LiniarSSE3( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int SrcStride, int DestW, int DestH, int DestStride, int DestBuffHeight, int DestStartY, int DestStartX );

//inherited from liniarresampler.cpp
void AddBlackBoxYUV420( unsigned char *dst, int bufWidth, int bufHeight, int BoxStartX, int BoxStartY, int boxWidth, int boxHeight );

#endif