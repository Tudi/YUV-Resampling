#ifndef _C2DLINEARRESAMPLER_H_
#define _C2DLINEARRESAMPLER_H_

//
// Performs resampling for generic image size conversions
//
class C2DLinearResampler
{
public:
	// constructor with actual image sizes
	C2DLinearResampler(int width1, int height1, int width2, int height2,int lumalow =16, int lumahigh =235,int chromalow =16,int chromahigh =240);
	~C2DLinearResampler();

	// performs resampling of the source into destination, with the ratio defined in constructor
	void Resample(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	void ChangeColourTemprature(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride ,int clipindex);
	void ResampleRGB32(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	void ResampleRGB24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	void ResampleBW1Bit(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	
private:
	//
	// implementation-specific definitions
	//
	#include "_2DLinearResampler.ph"
};

#endif	/* _C2DLINEARRESAMPLER_H_ */

//
// Performs resampling for spatial scalability processing, as defines in MPEG4 spec (ISO/IEC 14496-2 $7.9.2.5, $7.9.2.6)
//