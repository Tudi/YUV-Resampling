#ifndef _FILE_IMAGE_H_
#define _FILE_IMAGE_H_

//!! this function has very limited safety checks, use it with care !
int LoadYUVImageFromFile( unsigned char *Buff, int Width, int Height, int Stride, int FrameNr, char *FileName );
int SaveYUVImageToFile( unsigned char *Buff, int Width, int Height, int Stride, char *FileName, int y4mFormat );

#endif