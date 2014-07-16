#include "File.h"
#include <stdio.h>
#include <string>

int LoadYUVImageFromFile( unsigned char *Buff, int Width, int Height, int Stride, int FrameNr, char *FileName )
{
	FILE *f ;
	errno_t er = fopen_s( &f, FileName, "rb" );
	if( f )
	{
		//jump to desired frame
		fseek( f, Width * Height * 3 / 2 * FrameNr, SEEK_SET );
		if( Width == Stride )
			fread( Buff, 1, Width * Height * 3 / 2, f );
		else
		{
			for( int i=0;i<Height;i++)
				fread( &Buff[i*Stride], 1, Width, f );
			for( int i=0;i<Height/2;i++)
				fread( &Buff[ Height * Stride + i * Stride / 2 ], 1, Width / 2, f );
			for( int i=0;i<Height/2;i++)
				fread( &Buff[ Height * Stride +  Height / 2 * Stride / 2 + i * Stride / 2 ], 1, Width / 2, f );
		}
		fclose( f );
		return 0;
	}
	return 1;
}

int SaveYUVImageToFile( unsigned char *Buff, int Width, int Height, int Stride, char *FileName, int y4mFormat )
{
	FILE *f ;
	errno_t er = fopen_s( &f, FileName, "wb" );
	if( f )
	{
		//write the y4m header
		if( y4mFormat )
		{
			char Header[500];
			sprintf_s( Header, 500, "YUV4MPEG2 W%d H%d\nFRAME\n",Width,Height);
			int WriteCount = fwrite( Header, 1, strlen( Header ), f );
		}
		//jump to desired frame
		if( Width == Stride )
			fwrite( Buff, 1, Width * Height * 3 / 2, f );
		else
		{
			for( int i=0;i<Height;i++)
				fwrite( &Buff[i*Stride], 1, Width, f );
			for( int i=0;i<Height/2;i++)
				fwrite( &Buff[ Height * Stride + i * Stride / 2 ], 1, Width / 2, f );
			for( int i=0;i<Height/2;i++)
				fwrite( &Buff[ Height * Stride +  Height / 2 * Stride / 2 + i * Stride / 2 ], 1, Width / 2, f );
		}
		fclose( f );
		return 0;
	}
	return 1;
}