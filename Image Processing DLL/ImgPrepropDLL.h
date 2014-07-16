#ifndef _IMG_PREPROP_DLL_H_INCLUDED_
#define _IMG_PREPROP_DLL_H_INCLUDED_

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IMGPREPROPDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IMGPREPROPDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef IMGPREPROPDLL_EXPORTS
	#define IMGPREPROPDLL_API __declspec(dllexport) 
#else
	#define IMGPREPROPDLL_API __declspec(dllimport)
#endif

#define ERR_BUFFER_NULL				-1L
#define ERR_INVALID_SIZE 			-2L
#define ERR_STRIDE_MISSMATCH		-3L
#define ERR_BOUNDSCHECK_FAILED		-4L
#define ERR_INTEGRITY_CHECK_FAILED	-5L

struct RGB24Color
{
	unsigned char R,G,B;
};

/*
ConvertYV12toRGB -> Convert a YUV420 planar colorspace into RGB24 color space
Parameters :
- sourceWidth : The width of the image in pixels. Does not take into count BPP
- sourceHeight : The height of the image in pixels. Does not take into count BPP
- sourceStride : The stride of the source buffer in Bytes. Does take into count BPP. Since input is YUV, stride is at least as large as image width
- sourcebuffer : pointer to the location of the input image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is YUV420 this should have the size of sourceWidth * sourceHeight * 3 / 2 + 32 at least
- targetbuffer : pointer to the location of the output image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is RGB24 this should have the size of sourceWidth * sourceHeight * 3 + 32 at least
Return value :
- 0 - No error
- ERROR_INVALID_ADDRESS
- ERROR_INVALID_PARAMETER
*/
extern "C"  IMGPREPROPDLL_API  int APIENTRY ConvertYV12toRGB(int width, int height, int src_stride, unsigned char *Src_IYUV, int Src_IYUV_Size, unsigned char *Dst_RGB, int Dst_RGB_Size, int dst_stride);


/*
ConvertYV12toBGR -> Convert a YUV420 planar colorspace into BGR24 color space
Parameters :
- sourceWidth : The width of the image in pixels. Does not take into count BPP
- sourceHeight : The height of the image in pixels. Does not take into count BPP
- sourceStride : The stride of the source buffer in Bytes. Does take into count BPP. Since input is YUV, stride is at least as large as image width
- sourcebuffer : pointer to the location of the input image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is YUV420 this should have the size of sourceWidth * sourceHeight * 3 / 2 + 32 at least
- targetbuffer : pointer to the location of the output image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is BGR24 this should have the size of sourceWidth * sourceHeight * 3 + 32 at least
Return value :
- 0 - No error
- ERROR_INVALID_ADDRESS
- ERROR_INVALID_PARAMETER
*/
extern "C" IMGPREPROPDLL_API int APIENTRY ConvertYV12toBGR(int width, int height, int src_stride, unsigned char *Src_IYUV, int Src_IYUV_Size, unsigned char *Dst_RGB, int Dst_RGB_Size, int dst_stride);


/*
ConvertRGBtoGrayscale -> Convert a RGB24 planar colorspace into RGB24 grayscale color space
Parameters :
- sourceWidth : The width of the image in pixels. Does not take into count BPP
- sourceHeight : The height of the image in pixels. Does not take into count BPP
- sourceStride : The stride of the source buffer in Bytes. Does take into count BPP. Since input is RGB, stride is at least as large as image width * 3
- sourcebuffer : pointer to the location of the input image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is RGB24 this should have the size of sourceWidth * sourceHeight * 3 + 32 at least
- GrayscaleLevels : number of color levels the output should have. Valid input is 1-255
Return value :
- 0 - No error
- ERROR_INVALID_ADDRESS
- ERROR_INVALID_PARAMETER
*/
extern "C" IMGPREPROPDLL_API int APIENTRY ConvertRGBtoGrayscale(int sourceWidth, int sourceHeight, int sourceStride, unsigned char *buffer, int GrayscaleLevels);


/*
CompareBitmapRegions -> Compare RGB24 to RGB24 bitmap regions and return image similarity PCT
Parameters :
- width : The width of the image in pixels. Does not take into count BPP
- height : The height of the image in pixels. Does not take into count BPP
- src_stride : The stride of the source buffer in Bytes. Does take into count BPP. Since input is RGB24, stride is at least as large as image width * 3
- regionx : Value in pixel indicating the starting column of the region to be compared
- regiony : Value in pixel indicating the starting row of the region to be compared
- regionwidth : Value in pixel indicating the sub image width to be compared
- regionheight : Value in pixel indicating the sub image height to be compared
- referencebuffer : pointer to the location of the output image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is RGB24 this should have the size of sourceWidth * sourceHeight * 3 + 32 at least
- samplebuffer : pointer to the location of the input image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is RGB24 this should have the size of sourceWidth * sourceHeight * 3 + 32 at least
- highlightidenticalpixels : if value is non 0 then the R component of the sample image is set to 255 when the 2 pixels at the same position match
*/
extern "C" IMGPREPROPDLL_API float APIENTRY CompareBitmapRegions(int width, int height, int src_stride, int regionx, int regiony, int regionwidth, int regionheight, unsigned char *referencebuffer, unsigned char *samplebuffer, char highlightidenticalpixels );


/*
CompareBitmapRegions -> Compare BGR24 to BGR24 bitmap regions and return image similarity PCT
Parameters :
- width : The width of the image in pixels. Does not take into count BPP
- height : The height of the image in pixels. Does not take into count BPP
- src_stride : The stride of the source buffer in Bytes. Does take into count BPP. Since input is BGR24, stride is at least as large as image width * 3
- regionx : Value in pixel indicating the starting column of the region to be compared
- regiony : Value in pixel indicating the starting row of the region to be compared
- regionwidth : Value in pixel indicating the sub image width to be compared
- regionheight : Value in pixel indicating the sub image height to be compared
- referencebuffer : pointer to the location of the output image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is BGR24 this should have the size of sourceWidth * sourceHeight * 3 + 32 at least
- samplebuffer : pointer to the location of the input image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is BGR24 this should have the size of sourceWidth * sourceHeight * 3 + 32 at least
- highlightidenticalpixels : if value is non 0 then the R component of the sample image is set to 255 when the 2 pixels at the same position match
*/
extern "C" IMGPREPROPDLL_API float APIENTRY CompareBGRBitmapRegions(int width, int height, int src_stride, int regionx, int regiony, int regionwidth, int regionheight, unsigned char *referencebuffer, unsigned char *samplebuffer, char highlightidenticalpixels );

/*
InsertBuffer -> Insert a smaller YUV image into a larger one at a specific position
Parameters :
- TargetBuffer : pointer to the location of the destination image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is YUV this should have the size of dst_stride * sourceHeight * 3 / 2 + 32 at least
- yv12Width : The width of the image in pixels. Does not take into count BPP
- yv12Height : The height of the image in pixels. Does not take into count BPP
- dst_stride : The stride of the destination buffer in Bytes. Does take into count BPP. Since input is YUV, stride is at least as large as image width
- insertbuffer : pointer to the location of the output image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is YUV this should have the size of stride * height * 3 + 32 at least
- width : The width of the image in pixels. Does not take into count BPP
- height : The height of the image in pixels. Does not take into count BPP
- stride : The stride of the destination buffer in Bytes. Does take into count BPP. Since input is YUV, stride is at least as large as image width
- x : the starting pixel in horizontal axis where the insertbuffer image will be inserted to
- y : the starting pixel in vertical axis where the insertbuffer image will be inserted to
*/
extern "C" IMGPREPROPDLL_API void APIENTRY InsertBuffer(unsigned char *targetBufferYUV, int targetBufferSize, int targetWidth, int targetHeight, int targetStride, unsigned char *sourceBufferYUV, int sourceBufferSize, int sourceWidth, int sourceHeight, int Src_stride, int Dst_start_x, int Dst_start_y );

/*
ConvertBGRtoYV12 -> Convert a BGR24 colorspace into YUV420 planar color space 
Parameters :
- sourcebuffer : pointer to the location of the input image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is YUV420 this should have the size of sourceWidth * sourceHeight * 3 / 2 + 32 at least
- targetbuffer : pointer to the location of the output image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is BGR24 this should have the size of sourceWidth * sourceHeight * 3 + 32 at least
- dst_stride : The stride of the source buffer in Bytes. Does take into count BPP. Since destination is YUV, stride is at least as large as image stride
- sourceWidth : The width of the image in pixels. Does not take into count BPP
- sourceHeight : The height of the image in pixels. Does not take into count BPP
- sourceStride : The stride of the source buffer in Bytes. Does take into count BPP. Since input is RGB, stride is at least as large as image stride ( 3 * width )
*/
extern "C" IMGPREPROPDLL_API void APIENTRY ConvertBGRtoYV12(unsigned char *sourceBufferRGB, int sourceBufferSize, int width, int height, int sourceStride, unsigned char *targetBufferYUV, int targetBufferSize, int targetStride);


/*
GetColorValues -> Get color statistics on a region of an image 
Parameters :
- pbuff : pointer to the location of the input image in memory. It is advised to be 16 byte aligned and SSE friendly ( at least 32 byte padded ). Since input is BGR24 this should have the size of sourceWidth * sourceHeight * 3  + 32 at least
- Stride : The stride of the source buffer in Bytes. Does take into count BPP. Since destination is BGR24, stride is at least as large as image stride ( 3*width)
- StartX : starting position on horizontal axis of the region that we will sample
- StartY : starting position on vertical axis of the region that we will sample
- RegionWidth : the width of the sub image that we will sample
- RegionHeight : the height of the sub image that we will sample
- Reference : BGR value used in sampling formula
- Tolerance : BGR value used in sampling formula
- CurrentColor : BGR value used in sampling formula
*/
extern "C" IMGPREPROPDLL_API float APIENTRY GetColorValues( unsigned char *pbuff, int bufferSize, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *Reference, RGB24Color *Tolerance, RGB24Color *CurrentColor );

/*
AllocateRGBMemory -> Allocate memory to be used internally or externally. Memory will be at least 16 byte alligned and will be capable of transaction Lock + bounds checking 
Parameters :
- OutputBuffer : pointer to a pointer where the newly allocated buffer pointer will be stored
- BufferSize : Size of the buffer to be allocated Since destination is BGR24, At least as large as ( 3*width) * height
- Width : Width of the buffer. Used for security checks
- Height : Height of the buffer. Used for security checks
- Stride : The stride of the source buffer in Bytes. Does take into count BPP. Since destination is BGR24, stride is at least as large as image stride ( 3*width)
*/
extern "C" IMGPREPROPDLL_API int AllocateRGBMemory( unsigned char **OutputBuffer, int BufferSize, int Width, int Height, int Stride );

/*
AllocateYUVMemory -> Allocate memory to be used internally or externally. Memory will be at least 16 byte alligned and will be capable of transaction Lock + bounds checking 
Parameters :
- OutputBuffer : pointer to a pointer where the newly allocated buffer pointer will be stored
- BufferSize : Size of the buffer to be allocated Since destination is BGR24, At least as large as width * height * / 2
- Width : Width of the buffer. Used for security checks
- Height : Height of the buffer. Used for security checks
- Stride : The stride of the source buffer in Bytes. Does take into count BPP. Since destination is YUV, stride is at least as large as image stride width
*/
extern "C" IMGPREPROPDLL_API int AllocateYUVMemory( unsigned char **OutputBuffer, int BufferSize, int Width, int Height, int Stride );

/*
FreeMemory -> Free memory that we allocated with AllocateRGBMemory or AllocateYUVMemory
Parameters :
- buff : pointer where the buffer was allocated
*/
extern "C" IMGPREPROPDLL_API int FreeMemory( void *buff );

/*
FreeMemory -> Free memory that we allocated with AllocateRGBMemory or AllocateYUVMemory. This function will lock (in case buffer is locked) until it can proceed with delete
Parameters :
- buff : pointer where the buffer was allocated
*/
extern "C" IMGPREPROPDLL_API int FreeMemoryWaitForTransactions( void *buff );
/*
MemorySpinLockEnterTransaction -> Lock the buffer to avoid delete ( or maybe if you want to avoid double write )
Parameters :
- buff : pointer where the buffer was allocated with AllocateRGBMemory or AllocateYUVMemory
*/
extern "C" IMGPREPROPDLL_API int MemorySpinLockEnterTransaction( void *buff );
/*
MemorySpinLockExitTransaction -> UnLock the buffer to avoid delete ( or maybe if you want to avoid double write )
Parameters :
- buff : pointer where the buffer was allocated with AllocateRGBMemory or AllocateYUVMemory
*/
extern "C" IMGPREPROPDLL_API int MemorySpinLockExitTransaction( void *buff );

#endif //_IMG_PREPROP_DLL_H_INCLUDED_