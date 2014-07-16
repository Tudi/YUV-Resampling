#ifndef _COLOR_CONVERT_H_
#define _COLOR_CONVERT_H_

#define DIV_PRECISSION_BITS	12
#define DIV_PRECISION_VALUE ( 1 << DIV_PRECISSION_BITS )

void IYUV_to_RGB24_reference(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);

void IYUV_to_RGB24_1(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24_2(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24_3(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24_4(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24_5(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24_6(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24_7(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24_8(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_RGB24_9(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);

void RGB24_to_GrayScale_reference(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels);
void RGB24_to_GrayScale_1(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels);
void RGB24_to_GrayScale_2(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels);
void RGB24_to_GrayScale_3(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels);
void RGB24_to_GrayScale_4(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels);
void RGB24_to_GrayScale_5(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels);
void RGB24_to_GrayScale_6(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels);
void RGB24_to_GrayScale_7(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels);

float CompareRGBRegions_reference(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels );
float CompareRGBRegions_1(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels );
float CompareRGBRegions_2(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels );
float CompareRGBRegions_3(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels );
float CompareRGBRegions_4(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels );

void IYUV_to_BGR24_1(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);
void IYUV_to_BGR24_reference(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride);

float CompareBGRRegions_reference(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels );
float CompareBGRRegions_1(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels );
#endif