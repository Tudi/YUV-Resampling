#ifndef _LINEAR_RESAMPLER_H_
#define _LINEAR_RESAMPLER_H_

//this was required for old hardware where float to int conversion is slow. In the future you might be able to use float indexing directly
#define FLOAT_PRECISSION_BITS	15		// values are on 16 bits, using more for float precission does not count. Make sure to not have this number too big !
#define FLOAT_PRECISSION		( 1 << FLOAT_PRECISSION_BITS )

//in this state this is not optimized for speed. Should be better then biliniar resampling tough
extern "C" void ResampleYUV420Liniar( unsigned char *src, unsigned char *dst, int SrcW, int SrcH, int DestW, int DestH, bool KeepAspectRatio );

#endif