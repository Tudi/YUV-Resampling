#ifndef _HDResampler_H_
#define _HDResampler_H_

//
// Performs resampling for generic image size conversions
//
// Raw resampling does not perform aditional image quality improvement. It simply drops / adds n'th pixel
// HD - High Definition -> it refers that speed is critical. Sacrifice as less visual effects possible to gain speed

class HDResampler
{
public:
	// constructor with actual image sizes
	HDResampler(int width1, int height1, int width2, int height2,int lumalow =16, int lumahigh =235,int chromalow =16,int chromahigh =240);
	~HDResampler(){	if( conv_table ) free( conv_table ); };

	// performs resampling of the source into destination, with the ratio defined in constructor
	void ResampleRGB24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
	void ResampleYUV_I420(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
	
private:
	//convert whatever into whatever -> slow but good for testing
	void ResampleRGB24_anyscale(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
	//source is bigger then output
	void ResampleRGB24_downscale(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
	//source is smaller then output
	void ResampleRGB24_upscale(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);

	//convert whatever into whatever -> slow but good for testing
	void ResampleYUV_I420_anyscale(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
	//source is bigger then output
	void ResampleYUV_I420_downscale(PBYTE src, unsigned int *lookup_table, PBYTE dest, int destw, int   dest_stride );
	//source is smaller then output
	void ResampleYUV_I420_upscale(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);

	float			conv_x;			// precision with which pixels are converted
	float			conv_y;			// precision with which pixels are converted
	unsigned int	in_w,in_h;		// size of input "image"
	unsigned int	out_w,out_h;	// size of input "image"

	void			InitConvTable(unsigned int src_stride, unsigned int dst_stride);		// populate the conversion table
	unsigned int	*conv_table;	// conversion table that translates any dst pos to a src pos
	unsigned int	src_stride,dst_stride;
};

#endif	/* _HDResampler_H_ */
