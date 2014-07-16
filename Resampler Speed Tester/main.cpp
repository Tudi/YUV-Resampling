#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <conio.h>

struct Resolution
{
	int Width,Height;
};

int DoBenchmark1( int InWidth, int InHeight, int OutWidth, int OutHeight, char *Src, char *Dst, char *Conformance )
{
	//init
	int StartStamp;
	int EndStamp;
	//do the test
	StartStamp = GetTickCount();
	for( int i=0;i<TEST_REPEAT_COUNT;i++)
		ResampleYUV420Liniar( (unsigned char *)Src, (unsigned char *)Dst, InWidth, InHeight, OutWidth, OutHeight, false );
	EndStamp = GetTickCount();
	//check if output is correct 
	int ret = (EndStamp-StartStamp);
	if( Conformance != NULL )
	{
		int IsDifferent = memcmp( Dst, Conformance, OutWidth * OutHeight * 3 / 2 );
		if( IsDifferent )
			ret = 0x7FFFFFFF;
	}
	//deinit
	if( InWidth < OutWidth )
		sprintf( TestName, "LinC.Up" );
	else
		sprintf( TestName, "LinC.Down" );
	return ret;
}

int DoBenchmark2( int InWidth, int InHeight, int OutWidth, int OutHeight, char *Src, char *Dst, char *Conformance )
{
	//init
	int StartStamp;
	int EndStamp;
	//do the test
	StartStamp = GetTickCount();
	for( int i=0;i<TEST_REPEAT_COUNT;i++)
		ResampleYUV420LiniarSSE3( (unsigned char *)Src, (unsigned char *)Dst, InWidth, InHeight, OutWidth, OutHeight, false );
	EndStamp = GetTickCount();
	//check if output is correct 
	int ret = (EndStamp-StartStamp);
	if( Conformance != NULL )
	{
		int IsDifferent = memcmp( Dst, Conformance, OutWidth * OutHeight * 3 / 2 );
		if( IsDifferent )
		{
			ret = 0x7FFFFFFF;
			int ErrorCounter = 0;
			char *tdst = Dst;
			char *tconf = Conformance;
			for( int y=0;y<OutHeight;y++)
				for( int x=0;x<OutWidth;x++)
					if( tdst[y*OutWidth+x] != tconf[y*OutWidth+x] )
						ErrorCounter++;
			tdst += OutHeight * OutWidth;
			tconf += OutHeight * OutWidth;
			for( int y=0;y<OutHeight;y++)
				for( int x=0;x<OutWidth;x++)
					if( tdst[y*OutWidth+x] != tconf[y*OutWidth+x] )
						ErrorCounter++;
			tdst += OutHeight / 2 * OutWidth / 2;
			tconf += OutHeight / 2 * OutWidth / 2;
			for( int y=0;y<OutHeight;y++)
				for( int x=0;x<OutWidth;x++)
					if( tdst[y*OutWidth+x] != tconf[y*OutWidth+x] )
						ErrorCounter++;
		}
	}
	//deinit
	if( InWidth < OutWidth )
		sprintf( TestName, "LinSSE.Up" );
	else
		sprintf( TestName, "LinSSE.Down" );
	return ret;
}

int DoBenchmarkLiniar( int InWidth, int InHeight, int OutWidth, int OutHeight, char *Src, char *Dst, char *Conformance )
{
	//init
	int StartStamp;
	int EndStamp;
	//do the test
	StartStamp = GetTickCount();
	for( int i=0;i<TEST_REPEAT_COUNT;i++)
		ResampleYUV420Liniar( (unsigned char *)Src, (unsigned char *)Dst, InWidth, InHeight, OutWidth, OutHeight, false );
	EndStamp = GetTickCount();
	//check if output is correct 
	int ret = (EndStamp-StartStamp);
	//deinit
	return ret;
}

int DoBenchmarkOldResampler( int InWidth, int InHeight, int OutWidth, int OutHeight, char *Src, char *Dst, char *Conformance, int inframenum , char *InFileName, char *OutFileName)
{
	//init
	int StartStamp;
	int EndStamp;
	//TResampler			*m_pResampler = CreateResampler(RT_BILINEAR, OutWidth, OutHeight, InWidth, InHeight);
	CRatioResampler2	*m_resampler2 = new CRatioResampler2( InWidth, InHeight, OutWidth, OutHeight, false );
	FILE * input = fopen(InFileName, "rb" );
	FILE * output = fopen(OutFileName, "wb" );
	if(!output)
		printf("Cannot create output file");
	if(!input)
		printf("Cannot open input file");
	//do the test
	__int64 CounterStart = 0;
	__int64 sum = 0;
	LARGE_INTEGER li;
	double PCFreq = 0.0;
	QueryPerformanceFrequency(&li);
	 PCFreq = double(li.QuadPart)/1000.0;

	for( int i=0;i<inframenum;i++)
	{
		int ReadCount = fread( Src, 1, InWidth * InHeight * 3 / 2, input );

		if( ReadCount != InWidth * InHeight * 3 / 2 ){
			printf( "Warning. Could not read %d bytes, only %d available\n", InWidth * InHeight * 3 / 2, ReadCount );
			break;
		}
		QueryPerformanceCounter(&li);
		CounterStart = li.QuadPart;
//		ResampleYUV_I420( m_pResampler, (BYTE*)Dst, (BYTE*)Src, OutWidth, InWidth );
		printf("Start Frame : %d\n",i);
		m_resampler2->Process( (BYTE*)Src, (BYTE*)Dst, 0, CRatioResampler2::IYUV );
		printf("Finish Frame : %d\n",i);
		QueryPerformanceCounter(&li);
		sum += (li.QuadPart - CounterStart);

		int WriteCount = fwrite( Dst, 1, OutWidth * OutHeight * 3 / 2, output );
		if( WriteCount != OutWidth * OutHeight * 3 / 2 ){
			printf( "Warning. Could not write %d bytes, only %d available\n", OutWidth * OutHeight * 3 / 2, WriteCount );
			break;
	}
	}
	
	//deinit
	//(CBilinearResampler*)m_resampler2->m_pResampler->PrintfStat();
	return sum/PCFreq;
}

void SaveImgToFile( char *OutputBuffer, int Width, int Height, char *TestType )
{
	char Name[500];
	sprintf_s( Name, 500, "%dx%d_%s.y4m",Width, Height, TestType );
	SaveFrameToFile( Name, OutputBuffer, Width, Height, 1 );
	sprintf_s( Name, 500, "%dx%d_%s.yuv",Width, Height, TestType );
	SaveFrameToFile( Name, OutputBuffer, Width, Height, 0 );
}
//F:\XOE\testset\yuv\big_bunny_1080p.yuv 1920 1080 100 320 240
//F:\XOE\testset\yuv\motor_720x576.yuv 720 576 100 320 240
void main(int argc, char *argv[])
{
	char *Src = (char *)malloc( MAX_WIDTH * MAX_HEIGHT * 3 );
	char *Dst = (char *)malloc( MAX_WIDTH * MAX_HEIGHT * 3 );
	char *Conf = (char *)malloc( MAX_WIDTH * MAX_HEIGHT * 3 );
	char *Conf2 = (char *)malloc( MAX_WIDTH * MAX_HEIGHT * 3 );
	for( int i=0;i<MAX_WIDTH * MAX_HEIGHT * 3;i++)
	{
		Src[i] = i;
		Dst[i] = i+1;
		Conf[i] = i+2;
		Conf[i] = i+3;
	}
	//printf("Example : abc.yuv inwidth inheight inframenum outwidth outheight\n");
	int inwidth = atoi(argv[3]);
	int inheight = atoi(argv[4]);
	int inframe = atoi(argv[5]);
	int outwidth = atoi(argv[6]);
	int outheight = atoi(argv[7]);
	LoadFrameFromFile( argv[1], Src, inwidth, inheight, inframe );
//	Resolution InputResolutions[] = { { 176 * 2, 144 * 2 }, {0,0} };
//	Resolution OutPutResolutions[] = { { 1 * 176, 1 * 144 }, {0,0} };
//	Resolution InputResolutions[] = { { 1920, 1080 }, { 1920, 1080 }, { 1920, 1080 }, { 1920, 1080 }, { 0, 0 } };
	//Resolution InputResolutions[] = { { 720, 480 }, { 0, 0 } };
	//Resolution OutPutResolutions[] = { { 1280, 720 }, { 720, 576 }, { 720, 480 }, { 320, 240 }, { 130, 98 }, { 0, 0 } };
	//Resolution OutPutResolutions[] = {  { 640, 480 }, { 0, 0 } };

/*	Resolution InputResolutions[40];
	Resolution OutPutResolutions[40];
	memset( InputResolutions, 0, sizeof( InputResolutions ) );
	memset( OutPutResolutions, 0, sizeof( OutPutResolutions ) );
	int ind = 0;
	for( float i=1;i<7;i+=0.3f)
	{
		InputResolutions[ind].Width = ((int)(176 * i)) & ~0x03;
		InputResolutions[ind].Height = ((int)(144 * i)) & ~0x03;
		OutPutResolutions[ind].Width = ((int)(176 * i)) & ~0x03;
		OutPutResolutions[ind].Height = ((int)(144 * i)) & ~0x03;
		ind++;
	}/**/
	printf("Resampler tester start \n");
	int SumDur[4],dur;
	memset( SumDur, 0, sizeof( SumDur ) );
	int TestCount = 0;
	int InResItr = 0;
	//while( InputResolutions[InResItr].Height != 0 )
	{
		int OutResItr = 0;
	//	while( OutPutResolutions[OutResItr].Height != 0 )
		{
			//benchmark old alg
			int InWidth = inwidth;
			int InHeight = inheight;
			int OutWidth = outwidth;
			int OutHeight = outheight;
			OutResItr++;
			
			//prepare conformance 
			//ResampleYUV420Liniar( (unsigned char *)Src, (unsigned char *)Conf, InWidth, InHeight, OutWidth, OutHeight, false );
			//ResampleYUV420Liniar( (unsigned char *)Src, (unsigned char *)Conf2, OutWidth, OutHeight, InWidth, InHeight, false );

			/*dur = DoBenchmark2( InWidth, InHeight, OutWidth, OutHeight, Src, Dst, Conf );
			SumDur[0] += dur;
			printf("Test 1 took %d for resolution %dx%d -> %dx%d = %f FPS\n", dur, InWidth, InHeight, OutWidth, OutHeight, 1000.0f/((float)dur/TEST_REPEAT_COUNT) );
			SaveImgToFile( Dst, OutWidth, OutHeight, "Lin" );*/

			dur = DoBenchmarkOldResampler( InWidth, InHeight, OutWidth, OutHeight, Src, Dst, Conf , inframe, argv[1], argv[2]);
			SumDur[1] += dur;
			printf("Test 2 took %d for resolution %dx%d -> %dx%d = %f second %f FPS\n", dur, InWidth, InHeight, OutWidth, OutHeight, (float)dur/1000.0f, 1000.0f/((float)dur/inframe) );
			SaveImgToFile( Dst, OutWidth, OutHeight, "BiLin" );

			/*dur = DoBenchmark2( OutWidth, OutHeight, InWidth, InHeight, Src, Dst, Conf2 );
			SumDur[2] += dur;
			printf("Test 3 took %d for resolution %dx%d -> %dx%d = %f FPS\n", dur, OutWidth, OutHeight, InWidth, InHeight, 1000.0f/((float)dur/TEST_REPEAT_COUNT) );
			SaveImgToFile( Dst, InWidth, InHeight, "Lin" );*/

			/*dur = DoBenchmarkOldResampler( OutWidth, OutHeight, InWidth, InHeight, Src, Dst, Conf2 );
			SumDur[3] += dur;
			printf("Test 4 took %d for resolution %dx%d -> %dx%d = %f FPS\n", dur, OutWidth, OutHeight, InWidth, InHeight, 1000.0f/((float)dur/TEST_REPEAT_COUNT) );
			SaveImgToFile( Dst, InWidth, InHeight, "BiLin" );*/
			
			//printf("============================================\n");
			TestCount++;
		}
		InResItr++;
	}
	/*for( int i = 0; i<4; i++ )
		printf("Test %d took %d milliseconds. -> %f FPS ( Avg )\n", i, SumDur[i], 1000.0f/((float)SumDur[i]/TEST_REPEAT_COUNT/TestCount) );
	printf("Avg Speed improvement %f\n", (float)(SumDur[1]+SumDur[3]) / (float)(SumDur[0]+SumDur[2]) );
	_getch();*/
}