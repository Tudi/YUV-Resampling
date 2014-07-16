#ifndef _NX_Color_Conversion_H_
#define _NX_Color_Conversion_H_

/*************************************************************/
/*****              For MPEG-4 Codec                    ******/
/*************************************************************/
// XX_to_YUV(IYUV)
#ifndef __cplusplus
// for Encoder
/*
extern void DIB_RGB24_to_IYUV(unsigned char * out, unsigned char * in,int w, int h); // RGB(x bits) to YUV420으로 될 것 같음.??
extern void RGB24_to_IYUV(unsigned char * out, unsigned char * in,int w, int h); // RGB(x bits) to YUV420으로 될 것 같음.??
extern void YVU9_to_IYUV(unsigned char * out, unsigned char * in, int w, int h);
extern void IYUV_to_YVU9(unsigned char * out, unsigned char * in, int w, int h);
extern void YUY2_to_IYUV(unsigned char * out, unsigned char * in, int w, int h);
extern void YV12_to_IYUV(unsigned char * out, unsigned char * in, int w, int h);
// for Decoder
extern void IYUV_to_RGB16(unsigned char *out, int width, int height, int pitch,
				   unsigned char *py, unsigned char *pu, unsigned char *pv);
extern void IYUV_to_RGB16_555(unsigned char *out, int width, int height, int pitch,
				   unsigned char *py, unsigned char *pu, unsigned char *pv);
extern void IYUV_to_RGB24( unsigned char *out, int width, int height, int pitch,
				   unsigned char *pY, unsigned char *pU, unsigned char *pV);
extern void IYUV_to_RGB32( unsigned char *out, int width, int height, int pitch,
				   unsigned char *pY, unsigned char *pU, unsigned char *pV);
extern void DIB_IYUV_to_RGB24(unsigned char *out, int width, int height, int pitch,
				   unsigned char *py, unsigned char *pu, unsigned char *pv);
*/
#else	//_cplusplus
extern "C" void DIB_RGB24_to_IYUV(unsigned char * out, unsigned char * in,int w, int h); // RGB(x bits) to YUV420으로 될 것 같음.??
extern "C" void RGB24_to_IYUV(unsigned char * out, unsigned char * in,int w, int h); // RGB(x bits) to YUV420으로 될 것 같음.??
extern "C" void YVU9_to_IYUV(unsigned char * out, unsigned char * in, int w, int h);
extern "C" void IYUV_to_YVU9(unsigned char * out, unsigned char * in, int w, int h);
extern "C" void YUY2_to_IYUV(unsigned char * out, unsigned char * in, int w, int h);
extern "C" void YV12_to_IYUV(unsigned char * out, unsigned char * in, int w, int h);
// for Decoder
extern "C" void IYUV_to_RGB16(unsigned char *out, int width, int height, int pitch,
				   unsigned char *py, unsigned char *pu, unsigned char *pv);
extern "C" void IYUV_to_RGB16_555(unsigned char *out, int width, int height, int pitch,
				   unsigned char *py, unsigned char *pu, unsigned char *pv);
extern "C" void IYUV_to_RGB24( unsigned char *out, int width, int height, int pitch,
				   unsigned char *pY, unsigned char *pU, unsigned char *pV);
extern "C" void IYUV_to_RGB32( unsigned char *out, int width, int height, int pitch,
				   unsigned char *pY, unsigned char *pU, unsigned char *pV);
extern "C" void DIB_IYUV_to_RGB24(unsigned char *out, int width, int height, int pitch,
				   unsigned char *py, unsigned char *pu, unsigned char *pv);

#endif	//_cplusplus

#endif	// _Color_Conversion_H_