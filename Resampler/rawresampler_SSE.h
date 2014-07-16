#ifndef _RAWRESAMPLER_SSE_H_
#define _RAWRESAMPLER_SSE_H_

//
// Performs resampling for generic image size conversions
//
// Raw resampling does not perform aditional image quality improvement. It simply drops / adds n'th pixel
// !! this is not an acurate methode. It uses 1 point float precission to decide output result
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!! not finished !!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class RawResamplerSSE
{
public:
	// constructor with actual image sizes
	RawResamplerSSE(int width1, int height1, int width2, int height2,int lumalow =16, int lumahigh =235,int chromalow =16,int chromahigh =240);

	// performs resampling of the source into destination, with the ratio defined in constructor
	void ResampleRGB24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
	
private:
	//convert whatever into whatever -> slow but good for testing
	void ResampleRGB24_anyscale(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
	//source is bigger then output
	void ResampleRGB24_downscale(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
	//source is smaller then output
	void ResampleRGB24_upscale(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);

	float			conv_x;			// precision with which pixels are converted
	float			conv_y;			// precision with which pixels are converted
	unsigned int	in_w,in_h;		// size of input "image"
	unsigned int	out_w,out_h;	// size of input "image"
};

#endif	/* _RAWRESAMPLER_H_ */
