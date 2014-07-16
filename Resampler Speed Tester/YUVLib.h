#ifndef _YUVLIB_H_
#define _YUVLIB_H_

int LoadFrameFromFile( char *FileName, char *OutputBuffer, int Width, int Height, int FrameNumber );
int SaveFrameToFile( char *FileName, char *InputBuffer, int Width, int Height, int y4mFormat );

#endif