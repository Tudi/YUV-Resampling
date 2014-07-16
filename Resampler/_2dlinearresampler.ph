// **********************************************************************************************
//                              Copyright (C) 2000  - SPLASH Software 
//
//                    Use or duplication without written consent prohibitted
// **********************************************************************************************

//
//	Sets up the lookup tables. Tables are allocated dynamically 
//	and their size depends on the dimensions of the target picture
//
//	NOTE: This function must be called BEFORE using the resampling function
//
int Setup_Linear_Tables();

//
//	Deallocates the space allocated in the previous function
//	Function should be called after the resampling job is done
//
void Destroy_Linear_Tables();


int width1, height1;		// dimensions of the source picture
int width2, height2;		// dimensions of the destination picture
int stride1, stride2;	// stride of source/destination picture
int lumalow, lumahigh,chromalow,chromahigh; // range for luma and chro cliping

// Linear resampling
// Luxxon
UINT *inX, *inY;		// current output and input (x,y) coords
int *diffX;				// i * subfactor - inX[i]
int *diffY;				// j * subfactor - inX[j]
BYTE clip_tab[3][2048];	// clip_tab[0] 0-255 clip_tab[1] clipped range for luma clip_tab[2] cliplled range for chrmoa

