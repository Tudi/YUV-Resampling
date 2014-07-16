/*************************************************
*                                                *
*  EasyBMP Cross-Platform Windows Bitmap Library * 
*                                                *
*  Author: Paul Macklin                          *
*   email: macklin01@users.sourceforge.net       *
* support: http://easybmp.sourceforge.net        *
*                                                *
*          file: EasyBMPsample.cpp               * 
*    date added: 03-31-2006                      *
* date modified: 12-01-2006                      *
*       version: 1.06                            *
*                                                *
*   License: BSD (revised/modified)              *
* Copyright: 2005-6 by the EasyBMP Project       * 
*                                                *
* description: Sample application to demonstrate *
*              some functions and capabilities   *
*                                                *
*************************************************/

#include "bmp/EasyBMP.h"
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include "../Resampler/resampler.h"
#include <Windows.h>
#include <conio.h>
#include "color convert/colorconv.h"
#include "Color_conversion_20030115.h"

#define MAX_IMG_SIZE (4*15*1024*1024)
//#pragma comment(lib, "IPTVLib.lib")

BYTE BMP_Img_buffer_24[ MAX_IMG_SIZE ];
BYTE RGB_Img_buffer_24[ MAX_IMG_SIZE ];
TResampler			*m_pResampler;		//!< Resampler filter instance
CRatioResampler		*m_pRatioResampler;	// ewlee 20081111 - use two resampler because of supporting to keep source aspect ratio

void CopyBMPtoRGB(BMP &bmp_file,BYTE *RGB,int width,int height, int stride_dst, int stride_src)
{
	for(int i=0;i<height;i++)
	{
		for(int j=0;j<width;j++)
		{
			RGB[j*3 + 0 ] = bmp_file.GetPixel(j,i).Red;
			RGB[j*3 + 1 ] = bmp_file.GetPixel(j,i).Green;
			RGB[j*3 + 2 ] = bmp_file.GetPixel(j,i).Blue;
		}
		for(int j=width;j<stride_dst;j++)
		{
			RGB[j*3 + 0 ] = 0;
			RGB[j*3 + 1 ] = 0;
			RGB[j*3 + 2 ] = 0;
		}
		RGB += stride_src*3;
	}
}

void CopyBMPtoBGR(BMP &bmp_file,BYTE *RGB,int width,int height)
{
	for(int i=0;i<height;i++)
	{
		for(int j=0;j<width;j++)
		{
			RGB[j*3 + 2 ] = bmp_file.GetPixel(j,i).Red;
			RGB[j*3 + 1 ] = bmp_file.GetPixel(j,i).Green;
			RGB[j*3 + 0 ] = bmp_file.GetPixel(j,i).Blue;
		}
		RGB += width*3;
	}
}

void CopyRGBtoBMP(BMP &bmp_file,BYTE *RGB,int width,int height, int stride_dst, int stride_src, int height_stride)
{
	RGBApixel bmp_pixel;
	bmp_pixel.Alpha = 0;
	for(int i=0;i<height;i++)
	{
		for(int j=0;j<width;j++)
		{
			bmp_pixel.Red = RGB[j*3 + 0];
			bmp_pixel.Green = RGB[j*3 + 1];
			bmp_pixel.Blue = RGB[j*3 + 2];
			bmp_file.SetPixel(j,i,bmp_pixel);
		}
		bmp_pixel.Red = 0;
		bmp_pixel.Green = 0;
		bmp_pixel.Blue = 0;
		for(int j=width;j<stride_dst;j++)
			bmp_file.SetPixel(j,i,bmp_pixel);
		RGB += stride_src*3;
	}
	bmp_pixel.Red = 0;
	bmp_pixel.Green = 0;
	bmp_pixel.Blue = 0;
	for(int i=height;i<height_stride;i++)
		for(int j=0;j<stride_dst;j++)
			bmp_file.SetPixel(j,i,bmp_pixel);
}

void CopyBGRtoBMP(BMP &bmp_file,BYTE *RGB,int width,int height)
{
	RGBApixel bmp_pixel;
	bmp_pixel.Alpha = 0;
	for(int i=0;i<height;i++)
	{
		for(int j=0;j<width;j++)
		{
			bmp_pixel.Red = RGB[j*3 + 2];
			bmp_pixel.Green = RGB[j*3 + 1];
			bmp_pixel.Blue = RGB[j*3 + 0];
			bmp_file.SetPixel(j,i,bmp_pixel);
		}
		RGB += width*3;
	}
}

void CopyYUVtoBMP(BMP &bmp_file,BYTE *YUV,int width,int height, int stride_dst, int stride_src, int height_stride)
{
	RGBApixel bmp_pixel;
	bmp_pixel.Alpha = 0;
	PBYTE Y,U,V;
	Y = YUV;
	U = Y + stride_src * height;
	V = U + stride_src * height / 4;
	for(int i=0;i<height;i++)
	{
		for(int j=0;j<width;j++)
		{
			int CY,CU,CV;
			CY = Y[ j ];
			CU = U[ j >> 1 ];
			CV = V[ j >> 1 ];
			int R,G,B;
			int C,D,E;
			C = CY - 16;
			D = CU - 128;
			E = CV - 128;
			C = 298 * C;
			R = ( C           + 409 * E + 128) >> 8;
			G = ( C - 100 * D - 208 * E + 128) >> 8;
			B = ( C + 516 * D           + 128) >> 8;

			if( R < 0 ) 				R = 0;
			if( R > 255 ) 				R = 255;
			if( G < 0 ) 				G = 0;
			if( G > 255 ) 				G = 255;
			if( B < 0 ) 				B = 0;
			if( B > 255 ) 				B = 255; 
			bmp_pixel.Red = R;
			bmp_pixel.Green = G;
			bmp_pixel.Blue = B;
			bmp_file.SetPixel(j,i,bmp_pixel);
		}
		Y += stride_src;
		if( i % 2 == 0 )
		{
			U += stride_src >> 1;
			V += stride_src >> 1;
		}
	}
}

void f1_simple_biliniar_and_NN_resize()
{
 BMP BMP_file;
 int bmp_hight,bmp_width,new_width;
 char new_filename[70];
 float scale;

 for( scale = 2;scale>0.2f;scale -= 0.3f )
 {
	BMP_file.ReadFromFile("testimage.bmp");
	bmp_hight = BMP_file.TellHeight();
	 bmp_width = BMP_file.TellWidth();
	 CopyBMPtoRGB(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width,bmp_width);
	printf("copied to RGB buffer\n");
	if( m_pResampler )   delete m_pResampler;
	m_pResampler = CreateResampler(RT_BILINEAR, (int)(bmp_width * scale), (int)(bmp_hight * scale), bmp_width, bmp_hight);
	ResampleRGB24(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24);
	printf("Resized\n");
	BMP_file.SetSize( (int)(bmp_width * scale), (int)(bmp_hight * scale) );
	CopyRGBtoBMP(BMP_file,RGB_Img_buffer_24, (int)(bmp_width * scale), (int)(bmp_hight * scale), (int)(bmp_width * scale), (int)(bmp_width * scale), (int)(bmp_hight * scale) );
	printf("Copied to BMP buffer\n");
	sprintf(new_filename,"BILIN_BMP_Resized_%04f.bmp",scale);
	BMP_file.WriteToFile( new_filename );
	printf("Saved to file\n");
 }/**/
 
 for( scale = 2;scale>0.2f;scale -= 0.3f )
 {
	BMP_file.ReadFromFile("testimage.bmp");
	bmp_hight = BMP_file.TellHeight();
	 bmp_width = BMP_file.TellWidth();
	 new_width = (int)(bmp_width * scale);
	 if( new_width % 4 != 0 )
		 new_width = new_width - new_width % 4;
	 CopyBMPtoRGB(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width,bmp_width);
	printf("copied to RGB buffer\n");
	if( m_pResampler )   delete m_pResampler;
	m_pResampler = CreateResampler(RT_HDRAW, new_width, (int)(bmp_hight * scale), bmp_width, bmp_hight);
	ResampleRGB24(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24);
	printf("Resized\n");
	BMP_file.SetSize( (int)(bmp_width * scale), (int)(bmp_hight * scale) );
	CopyRGBtoBMP(BMP_file,RGB_Img_buffer_24, (int)(bmp_width * scale), (int)(bmp_hight * scale), (int)(bmp_width * scale), (int)(bmp_width * scale), (int)(bmp_hight * scale) );
	printf("Copied to BMP buffer\n");
	sprintf(new_filename,"NN_BMP_Resized_%04f.bmp",scale);
	BMP_file.WriteToFile( new_filename );
	printf("Saved to file\n");
 }/**/
 //just dump the same BMP into YUV (don't ask why)
 FILE *YUV_file;
 BMP_file.ReadFromFile("testimage.bmp");
 bmp_hight = BMP_file.TellHeight();
 bmp_width = BMP_file.TellWidth();/**/

 ColorConverter CC;
 CopyBMPtoRGB(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width,bmp_width);
 YUV_file=fopen("testimage.YUV","wb");
 for(int i=0;i<bmp_hight;i++)
	for(int j=0;j<bmp_width;j++)
	{
		BYTE YUV_C = CC.rgb_to_y(BMP_file.GetPixel(j,i).Red,BMP_file.GetPixel(j,i).Green,BMP_file.GetPixel(j,i).Blue);
		fwrite(&YUV_C,1,1,YUV_file);
	}
 for(int i=0;i<bmp_hight;i+=2)
	for(int j=0;j<bmp_width;j+=2)
	{
		BYTE YUV_C = CC.rgb_to_u(BMP_file.GetPixel(j,i).Red,BMP_file.GetPixel(j,i).Green,BMP_file.GetPixel(j,i).Blue);
		fwrite(&YUV_C,1,1,YUV_file);
	}
 for(int i=0;i<bmp_hight;i+=2)
	for(int j=0;j<bmp_width;j+=2)
	{
		BYTE YUV_C = CC.rgb_to_v(BMP_file.GetPixel(j,i).Red,BMP_file.GetPixel(j,i).Green,BMP_file.GetPixel(j,i).Blue);
		fwrite(&YUV_C,1,1,YUV_file);
	}
 printf("Converted image to YUV\n");
 fclose(YUV_file); /**/

 YUV_file = fopen("testimage.YUV","rb");
 fread(BMP_Img_buffer_24,bmp_hight*bmp_width*3/2,1,YUV_file);
 fclose(YUV_file);/**/

 for( scale = 2;scale>0.2f;scale -= 0.3f )
 {
	if( m_pResampler )   delete m_pResampler;
	m_pResampler = CreateResampler(RT_BILINEAR, (int)(bmp_width * scale), (int)(bmp_hight * scale), bmp_width, bmp_hight);
	ResampleYUV_I420(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24,(int)(bmp_width * scale),bmp_width);
	printf("Resized\n");
	sprintf(new_filename,"BILIN_YUV_Resized_%04f.YUV",scale);
	BMP_file.SetSize( (int)(bmp_width * scale), (int)(bmp_hight * scale) );
	CopyRGBtoBMP(BMP_file,RGB_Img_buffer_24, (int)(bmp_width * scale), (int)(bmp_hight * scale), (int)(bmp_width * scale), (int)(bmp_width * scale), (int)(bmp_hight * scale) );
	printf("Copied to BMP buffer\n");
	sprintf(new_filename,"BILIN_YUV_Resized_%04f.bmp",scale);
	BMP_file.WriteToFile( new_filename );
	printf("Saved to file\n");
 }/**/
 //scale = 0.2;
 for( scale = 2;scale>0.2f;scale -= 0.3f )
 {
	if( m_pResampler )   delete m_pResampler;
	m_pResampler = CreateResampler(RT_HDRAW, (int)(bmp_width * scale), (int)(bmp_hight * scale), bmp_width, bmp_hight);
	ResampleYUV_I420(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24,(int)(bmp_width * scale),bmp_width);
	printf("Resized\n");
	sprintf(new_filename,"NN_YUV_Resized_%04f.YUV",scale);
	BMP_file.SetSize( (int)(bmp_width * scale), (int)(bmp_hight * scale) );
	CopyRGBtoBMP(BMP_file,RGB_Img_buffer_24, (int)(bmp_width * scale), (int)(bmp_hight * scale), (int)(bmp_width * scale), (int)(bmp_width * scale), (int)(bmp_hight * scale) );
	printf("Copied to BMP buffer\n");
	sprintf(new_filename,"NN_YUV_Resized_%04f.bmp",scale);
	BMP_file.WriteToFile( new_filename );
	printf("Saved to file\n");
 }
}

 void f2_simple_biliniar_and_NN_resize_QCIF()
 {
  BMP BMP_file;
 int bmp_hight,bmp_width,out_width,out_hight;
 char new_filename[70];
 float scale;

 scale = 0.5;
 {
	BMP_file.ReadFromFile("testimage2.bmp");
	bmp_hight = BMP_file.TellHeight();
	 bmp_width = BMP_file.TellWidth();
//	 out_width = bmp_width * scale;
//	 out_hight = bmp_hight * scale;
	 out_width = 174;
	 out_hight = 144;
	 CopyBMPtoRGB(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width,bmp_width);
	printf("copied to RGB buffer\n");
	if( m_pResampler )   delete m_pResampler;
	m_pResampler = CreateResampler(RT_BILINEAR, out_width, out_hight, bmp_width, bmp_hight);
	ResampleRGB24(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24);
	printf("Resized\n");
	BMP_file.SetSize( out_width, out_hight );
	CopyRGBtoBMP(BMP_file,RGB_Img_buffer_24, out_width, out_hight, out_width, out_width, out_hight);
	printf("Copied to BMP buffer\n");
	sprintf(new_filename,"BILIN_BMP_Resized_%04f.bmp",scale);
	BMP_file.WriteToFile( new_filename );
	printf("Saved to file\n");
 }/**/
 
  scale = 0.5;
 {
	BMP_file.ReadFromFile("testimage2.bmp");
	bmp_hight = BMP_file.TellHeight();
	 bmp_width = BMP_file.TellWidth();
//	 out_width = bmp_width * scale;
//	 out_hight = bmp_hight * scale;
	 out_width = 174;
	 out_hight = 144;
	 CopyBMPtoRGB(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width,bmp_width);
	printf("copied to RGB buffer\n");
	if( m_pResampler )   delete m_pResampler;
	m_pResampler = CreateResampler(RT_HDRAW, out_width, out_hight, bmp_width, bmp_hight);
	ResampleRGB24(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24);
	printf("Resized\n");
	BMP_file.SetSize( out_width, out_hight );
	CopyRGBtoBMP(BMP_file,RGB_Img_buffer_24, out_width, out_hight, out_width, out_width, out_hight);
	printf("Copied to BMP buffer\n");
	sprintf(new_filename,"NN_BMP_Resized_%04f.bmp",scale);
	BMP_file.WriteToFile( new_filename );
	printf("Saved to file\n");

 }/**/
 //just dump the same BMP into YUV (don't ask why)
 FILE *YUV_file;
 BMP_file.ReadFromFile("testimage2.bmp");
 bmp_hight = BMP_file.TellHeight();
 bmp_width = BMP_file.TellWidth();/**/

 ColorConverter CC;
 CopyBMPtoRGB(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width,bmp_width);
 YUV_file=fopen("testimage2.YUV","wb");
 for(int i=0;i<bmp_hight;i++)
	for(int j=0;j<bmp_width;j++)
	{
		BYTE YUV_C = CC.rgb_to_y(BMP_file.GetPixel(j,i).Red,BMP_file.GetPixel(j,i).Green,BMP_file.GetPixel(j,i).Blue);
		fwrite(&YUV_C,1,1,YUV_file);
	}
 for(int i=0;i<bmp_hight;i+=2)
	for(int j=0;j<bmp_width;j+=2)
	{
		BYTE YUV_C = CC.rgb_to_u(BMP_file.GetPixel(j,i).Red,BMP_file.GetPixel(j,i).Green,BMP_file.GetPixel(j,i).Blue);
		fwrite(&YUV_C,1,1,YUV_file);
	}
 for(int i=0;i<bmp_hight;i+=2)
	for(int j=0;j<bmp_width;j+=2)
	{
		BYTE YUV_C = CC.rgb_to_v(BMP_file.GetPixel(j,i).Red,BMP_file.GetPixel(j,i).Green,BMP_file.GetPixel(j,i).Blue);
		fwrite(&YUV_C,1,1,YUV_file);
	}
 printf("Converted image to YUV\n");
 fclose(YUV_file); /**/

 YUV_file = fopen("testimage2.YUV","rb");
 fread(BMP_Img_buffer_24,bmp_hight*bmp_width*3/2,1,YUV_file);
 fclose(YUV_file);/**/

 scale = 0.5;
 {
	if( m_pResampler )   delete m_pResampler;
//	 out_width = bmp_width * scale;
//	 out_hight = bmp_hight * scale;
	 out_width = 174;
	 out_hight = 144;
	m_pResampler = CreateResampler(RT_BILINEAR, out_width, out_hight, bmp_width, bmp_hight);
	ResampleYUV_I420(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24,out_width,bmp_width);
	printf("Resized\n");
	BMP_file.SetSize( out_width, out_hight );
	CopyYUVtoBMP(BMP_file,RGB_Img_buffer_24, out_width, out_hight, out_width, out_width, out_hight);
	printf("Copied to BMP buffer\n");
	sprintf(new_filename,"BILIN_YUV_Resized_%04f.bmp",scale);
	BMP_file.WriteToFile( new_filename );
	printf("Saved to file\n");
 }/**/
  scale = 0.5;
 {
	if( m_pResampler )   delete m_pResampler;
//	 out_width = bmp_width * scale;
//	 out_hight = bmp_hight * scale;
	 out_width = 174;
	 out_hight = 144;
	m_pResampler = CreateResampler(RT_HDRAW, out_width, out_hight, bmp_width, bmp_hight);
	ResampleYUV_I420(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24,out_width,bmp_width);
	printf("Resized\n");
	BMP_file.SetSize( out_width, out_hight );
	CopyYUVtoBMP(BMP_file,RGB_Img_buffer_24, out_width, out_hight, out_width, out_width, out_hight);
	printf("Copied to BMP buffer\n");
	sprintf(new_filename,"NN_YUV_Resized_%04f.bmp",scale);
	BMP_file.WriteToFile( new_filename );
	printf("Saved to file\n");
 }/**/
}

void f3_test_color_conversion()
{
 BMP BMP_file;
 int bmp_hight,bmp_width;
 float scale = 1;
 BMP_file.ReadFromFile("testimage2.bmp");
 bmp_hight = BMP_file.TellHeight();
 bmp_width = BMP_file.TellWidth();/**/

 ColorConverter CC;
 CopyBMPtoBGR(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight);

 CC.BGR24_to_YUV(BMP_Img_buffer_24,RGB_Img_buffer_24,bmp_width,bmp_hight);
// CC.BGR24_to_YUV_2x2_smooth(BMP_Img_buffer_24,RGB_Img_buffer_24,bmp_width,bmp_hight);
 //this seems to add extra column to small lines
// CC.BGR24_to_YUV_interpolate(BMP_Img_buffer_24,RGB_Img_buffer_24,bmp_width,bmp_hight);
 //seems to ok
 CC.YUV_to_BGR24(RGB_Img_buffer_24,BMP_Img_buffer_24,bmp_width,bmp_hight);
 //the one from XLE
/* unsigned char *y,*u,*v;
 y = RGB_Img_buffer_24;
 u = y + bmp_hight*bmp_width;
 v = u + bmp_hight*bmp_width / 4;
 DIB_IYUV_to_RGB24((unsigned char*)BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width, y,u,v);/**/

 CopyBGRtoBMP(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight);
 BMP_file.WriteToFile( "Color_converted_normal.bmp" );
}

void speed_test_BMP_Billiniar()
{
	BMP BMP_file;
	int bmp_hight,bmp_width;
	float scale = 0.5;
	BMP_file.ReadFromFile("testimage.bmp");
	bmp_hight = BMP_file.TellHeight();
	bmp_width = BMP_file.TellWidth();
	CopyBMPtoRGB(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width,bmp_width);
	if( m_pResampler )   delete m_pResampler;
	m_pResampler = CreateResampler(RT_BILINEAR, (int)(bmp_width * scale), (int)(bmp_hight * scale), bmp_width, bmp_hight);
	ResampleRGB24(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24);
}

void speed_test_BMP_NN()
{
	BMP BMP_file;
	int bmp_hight,bmp_width;
	float scale = 0.5;
	int new_width;
	BMP_file.ReadFromFile("testimage.bmp");
	bmp_hight = BMP_file.TellHeight();
	bmp_width = BMP_file.TellWidth();
	new_width = (int)(bmp_width * scale);
	if( new_width % 4 != 0 )
		 new_width = new_width - new_width % 4;
	CopyBMPtoRGB(BMP_file,BMP_Img_buffer_24,bmp_width,bmp_hight,bmp_width,bmp_width);
	if( m_pResampler )   delete m_pResampler;
	m_pResampler = CreateResampler(RT_HDRAW, new_width, (int)(bmp_hight * scale), bmp_width, bmp_hight);
	ResampleRGB24(m_pResampler, RGB_Img_buffer_24, BMP_Img_buffer_24);
}

unsigned int number_of_samples = 1;
float runtest(int inw,int inh,int new_ver=0)
{
   printf("Testing conv with input resolution : %u x %u \n",inw,inh);
   ColorConverter CC;
   unsigned int ts_start = GetTickCount();
   for(unsigned int i=0;i<number_of_samples;i++)
	   if( new_ver )
		 speed_test_BMP_Billiniar();
		// f2_simple_biliniar_and_NN_resize_QCIF();
		//	f3_test_color_conversion();
	   else
		 speed_test_BMP_NN();
		// f2_simple_biliniar_and_NN_resize_QCIF();
		//	f3_test_color_conversion();
   unsigned int ts_end = GetTickCount();
   unsigned int test_duration = ts_end - ts_start;

   printf("Resize time %f Milliseconds / frame \n",(float)test_duration/number_of_samples);
   return ((float)test_duration/number_of_samples);
}

int main( int argc, char* argv[] )
{
 cout << endl
      << "Using EasyBMP Version " << _EasyBMP_Version_ << endl << endl
      << "Copyright (c) by the EasyBMP Project 2005-6" << endl
      << "WWW: http://easybmp.sourceforge.net" << endl << endl;

#define testcount 8
	float total_time;
	total_time = 0;
	for(int i=1;i<=testcount;i++)
		total_time += runtest( 176*i,144*i,0 );
	total_time /= testcount;
   printf("CC AVG time %f Milliseconds / frame \n",total_time);
   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	total_time = 0;
	for(int i=1;i<=testcount;i++)
		total_time += runtest( 176*i,144*i,1 );
	total_time /= testcount;
   printf("CC AVG time %f Milliseconds / frame \n",total_time);
   printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

   char c=getch();

 return 0;
}
