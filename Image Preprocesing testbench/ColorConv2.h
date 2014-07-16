#ifndef _COLOR_CONVERT2_H_
#define _COLOR_CONVERT2_H_

#define DIV_PRECISSION_BITS	12
#define DIV_PRECISION_VALUE ( 1 << DIV_PRECISSION_BITS )

struct RGB24Color;
/*
struct RGB24Color
{
	unsigned char R,G,B;
};/**/

void RGB24_to_IYUV_reference(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_IYUV, int dst_stride);
void RGB24_to_IYUV_reference1(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_IYUV, int dst_stride);
void RGB24_to_IYUV_1(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_IYUV, int dst_stride);
void RGB24_to_IYUV_2(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_IYUV, int dst_stride);

void InsertWatermark_reference(unsigned char *Src_IYUV, int Src_width, int Src_height, int Src_stride, int Src_start_x, int Src_start_y, int Src_copy_width, int Src_copy_height, unsigned char *Dst_IYUV, int Dst_width, int Dst_height, int Dst_stride, int Dst_start_x, int Dst_start_y );

float GetColorValues_reference(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *Reference, RGB24Color *Tolerance, RGB24Color *CurrentColor);
float GetColorValues_1(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *Reference, RGB24Color *Tolerance, RGB24Color *CurrentColor);
float GetColorValues_2(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *Reference, RGB24Color *Tolerance, RGB24Color *CurrentColor);
float GetColorValues_3(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *Reference, RGB24Color *Tolerance, RGB24Color *CurrentColor);
float GetColorValues_4(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *Reference, RGB24Color *Tolerance, RGB24Color *CurrentColor);
float GetColorValues_5(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *Reference, RGB24Color *Tolerance, RGB24Color *CurrentColor);
float GetColorValues_6(unsigned char *pbuff, int Stride, int StartX, int StartY, int RegionWidth, int RegionHeight, RGB24Color *Reference, RGB24Color *Tolerance, RGB24Color *CurrentColor);

#endif