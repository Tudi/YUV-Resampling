#ifndef _STDAFX_H_
#define _STDAFX_H_

#include "File.h"
#include "Resize.h"
#include "Allocator.h"
#include "ColorConv.h"
#include "ColorConv2.h"

#undef IMGPREPROPDLL_EXPORTS
#include "../../../Common/ImageProcessing/ImgPrepropDLL.h"

#define UnAllignAddress( x ) ( x + ( ( (int)x & 0x0F )?(16-((int)x & 0x0F )+1):1) )
#define AllignAddress( x ) ( x + ( ( (int)x & 0x0F )?(16-((int)x & 0x0F )):0) )

#endif