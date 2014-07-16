#include <stdio.h>
#include <string>

int LoadFrameFromFile( char *FileName, char *OutputBuffer, int Width, int Height, int FrameNumber )
{
	FILE *f;
	errno_t e = fopen_s( &f, FileName, "rb" );
	if( f == NULL )
	{
		printf("Could not open input file");
		return 1;
	}
	fseek( f, Width * Height * 3 / 2, SEEK_SET );
	int ReadCount = fread( OutputBuffer, 1, Width * Height * 3 / 2, f );
	if( ReadCount != Width * Height * 3 / 2 )
		printf( "Warning. Could not read %d bytes, only %d available\n",Width * Height * 3 / 2, ReadCount );
	fclose( f );
	return 0;
}

int SaveFrameToFile( char *FileName, char *InputBuffer, int Width, int Height, int y4mFormat )
{
	FILE *f;
	errno_t e = fopen_s( &f, FileName, "wb" );
	if( f == NULL )
	{
		printf("Could not open output file");
		return 1;
	}
	//write the y4m header
	if( y4mFormat )
	{
		char Header[500];
		sprintf_s( Header, 500, "YUV4MPEG2 W%d H%d\nFRAME\n",Width,Height);
		int WriteCount = fwrite( Header, 1, strlen( Header ), f );
	}
	 
	int WriteCount = fwrite( InputBuffer, 1, Width * Height * 3 / 2, f );
	if( WriteCount != Width * Height * 3 / 2 )
		printf( "Warning. Could not write %d bytes, only %d available\n",Width * Height * 3 / 2, WriteCount );
	fclose( f );
	return 0;
}
