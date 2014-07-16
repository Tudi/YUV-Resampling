#ifndef _BICUBIC_RESAMPLER_H_
#define _BICUBIC_RESAMPLER_H_

//
// Performs resampling for generic image size conversions
//
class CBicubicResampler
{
public:
	// constructor with actual image sizes
	CBicubicResampler(int sw, int sh, int dw, int dh,int lumalow =16, int lumahigh =235,int chromalow =16,int chromahigh =240);
	~CBicubicResampler();

	// performs resampling of the source into destination, with the ratio defined in constructor
	void Resample(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex = 0);
	
	void ChangeColourTemprature(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride ,int clipindex);
	void ResampleRGB32(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	void ResampleBW1Bit(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex =0);
	
private:
	//
	// implementation-specific definitions
	//
	#include "_BicubicResampler.ph"
};



#endif

