#include "ColorConv.h"
#include <tmmintrin.h>
#include <string.h>

#ifndef CLIP3
	#define CLIP3(a,b,c)	((c) < (a) ? (a) : ((c) > (b) ? (b) : (c))) // MIN, MAX, val
#endif

void IYUV_to_RGB24_reference(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{

	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	for( j = 0; j < height; j++)
	{
		for( i=0; i < width; i++)
		{
			int Y, U, V, R, G, B;
			int iX3 = i+i+i;
			Y = py[j*src_stride + i];
			U = pu[(j/2)*(src_stride/2) + i/2];
			V = pv[(j/2)*(src_stride/2) + i/2];
			R = (300*(Y-16) + 517*(U-128) + 128 ) / 256;
			G = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) / 256;
			B = (300*(Y-16) + 409*(V-128) + 128 ) / 256;
			Dst_RGB[j*dst_stride + iX3 + 0] = CLIP3(0, 255, R);
			Dst_RGB[j*dst_stride + iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[j*dst_stride + iX3 + 2] = CLIP3(0, 255, B);
		}
	}
}

static int Y_300_128_Lookup[255]={-4672,-4372,-4072,-3772,-3472,-3172,-2872,-2572,-2272,-1972,-1672,-1372,-1072,-772,-472,-172,128,428,728,1028,1328,1628,1928,2228,2528,2828,3128,3428,3728,4028,4328,4628,4928,5228,5528,5828,6128,6428,6728,7028,7328,7628,7928,8228,8528,8828,9128,9428,9728,10028,10328,10628,10928,11228,11528,11828,12128,12428,12728,13028,13328,13628,13928,14228,14528,14828,15128,15428,15728,16028,16328,16628,16928,17228,17528,17828,18128,18428,18728,19028,19328,19628,19928,20228,20528,20828,21128,21428,21728,22028,22328,22628,22928,23228,23528,23828,24128,24428,24728,25028,25328,25628,25928,26228,26528,26828,27128,27428,27728,28028,28328,28628,28928,29228,29528,29828,30128,30428,30728,31028,31328,31628,31928,32228,32528,32828,33128,33428,33728,34028,34328,34628,34928,35228,35528,35828,36128,36428,36728,37028,37328,37628,37928,38228,38528,38828,39128,39428,39728,40028,40328,40628,40928,41228,41528,41828,42128,42428,42728,43028,43328,43628,43928,44228,44528,44828,45128,45428,45728,46028,46328,46628,46928,47228,47528,47828,48128,48428,48728,49028,49328,49628,49928,50228,50528,50828,51128,51428,51728,52028,52328,52628,52928,53228,53528,53828,54128,54428,54728,55028,55328,55628,55928,56228,56528,56828,57128,57428,57728,58028,58328,58628,58928,59228,59528,59828,60128,60428,60728,61028,61328,61628,61928,62228,62528,62828,63128,63428,63728,64028,64328,64628,64928,65228,65528,65828,66128,66428,66728,67028,67328,67628,67928,68228,68528,68828,69128,69428,69728,70028,70328,70628,70928,71228,71528,};
static int U_517_Lookup[255]={-66176,-65659,-65142,-64625,-64108,-63591,-63074,-62557,-62040,-61523,-61006,-60489,-59972,-59455,-58938,-58421,-57904,-57387,-56870,-56353,-55836,-55319,-54802,-54285,-53768,-53251,-52734,-52217,-51700,-51183,-50666,-50149,-49632,-49115,-48598,-48081,-47564,-47047,-46530,-46013,-45496,-44979,-44462,-43945,-43428,-42911,-42394,-41877,-41360,-40843,-40326,-39809,-39292,-38775,-38258,-37741,-37224,-36707,-36190,-35673,-35156,-34639,-34122,-33605,-33088,-32571,-32054,-31537,-31020,-30503,-29986,-29469,-28952,-28435,-27918,-27401,-26884,-26367,-25850,-25333,-24816,-24299,-23782,-23265,-22748,-22231,-21714,-21197,-20680,-20163,-19646,-19129,-18612,-18095,-17578,-17061,-16544,-16027,-15510,-14993,-14476,-13959,-13442,-12925,-12408,-11891,-11374,-10857,-10340,-9823,-9306,-8789,-8272,-7755,-7238,-6721,-6204,-5687,-5170,-4653,-4136,-3619,-3102,-2585,-2068,-1551,-1034,-517,0,517,1034,1551,2068,2585,3102,3619,4136,4653,5170,5687,6204,6721,7238,7755,8272,8789,9306,9823,10340,10857,11374,11891,12408,12925,13442,13959,14476,14993,15510,16027,16544,17061,17578,18095,18612,19129,19646,20163,20680,21197,21714,22231,22748,23265,23782,24299,24816,25333,25850,26367,26884,27401,27918,28435,28952,29469,29986,30503,31020,31537,32054,32571,33088,33605,34122,34639,35156,35673,36190,36707,37224,37741,38258,38775,39292,39809,40326,40843,41360,41877,42394,42911,43428,43945,44462,44979,45496,46013,46530,47047,47564,48081,48598,49115,49632,50149,50666,51183,51700,52217,52734,53251,53768,54285,54802,55319,55836,56353,56870,57387,57904,58421,58938,59455,59972,60489,61006,61523,62040,62557,63074,63591,64108,64625,65142,};
static int V_208_Lookup[255]={-26624,-26416,-26208,-26000,-25792,-25584,-25376,-25168,-24960,-24752,-24544,-24336,-24128,-23920,-23712,-23504,-23296,-23088,-22880,-22672,-22464,-22256,-22048,-21840,-21632,-21424,-21216,-21008,-20800,-20592,-20384,-20176,-19968,-19760,-19552,-19344,-19136,-18928,-18720,-18512,-18304,-18096,-17888,-17680,-17472,-17264,-17056,-16848,-16640,-16432,-16224,-16016,-15808,-15600,-15392,-15184,-14976,-14768,-14560,-14352,-14144,-13936,-13728,-13520,-13312,-13104,-12896,-12688,-12480,-12272,-12064,-11856,-11648,-11440,-11232,-11024,-10816,-10608,-10400,-10192,-9984,-9776,-9568,-9360,-9152,-8944,-8736,-8528,-8320,-8112,-7904,-7696,-7488,-7280,-7072,-6864,-6656,-6448,-6240,-6032,-5824,-5616,-5408,-5200,-4992,-4784,-4576,-4368,-4160,-3952,-3744,-3536,-3328,-3120,-2912,-2704,-2496,-2288,-2080,-1872,-1664,-1456,-1248,-1040,-832,-624,-416,-208,0,208,416,624,832,1040,1248,1456,1664,1872,2080,2288,2496,2704,2912,3120,3328,3536,3744,3952,4160,4368,4576,4784,4992,5200,5408,5616,5824,6032,6240,6448,6656,6864,7072,7280,7488,7696,7904,8112,8320,8528,8736,8944,9152,9360,9568,9776,9984,10192,10400,10608,10816,11024,11232,11440,11648,11856,12064,12272,12480,12688,12896,13104,13312,13520,13728,13936,14144,14352,14560,14768,14976,15184,15392,15600,15808,16016,16224,16432,16640,16848,17056,17264,17472,17680,17888,18096,18304,18512,18720,18928,19136,19344,19552,19760,19968,20176,20384,20592,20800,21008,21216,21424,21632,21840,22048,22256,22464,22672,22880,23088,23296,23504,23712,23920,24128,24336,24544,24752,24960,25168,25376,25584,25792,26000,26208,};
static int U_100_Lookup[255]={-12800,-12700,-12600,-12500,-12400,-12300,-12200,-12100,-12000,-11900,-11800,-11700,-11600,-11500,-11400,-11300,-11200,-11100,-11000,-10900,-10800,-10700,-10600,-10500,-10400,-10300,-10200,-10100,-10000,-9900,-9800,-9700,-9600,-9500,-9400,-9300,-9200,-9100,-9000,-8900,-8800,-8700,-8600,-8500,-8400,-8300,-8200,-8100,-8000,-7900,-7800,-7700,-7600,-7500,-7400,-7300,-7200,-7100,-7000,-6900,-6800,-6700,-6600,-6500,-6400,-6300,-6200,-6100,-6000,-5900,-5800,-5700,-5600,-5500,-5400,-5300,-5200,-5100,-5000,-4900,-4800,-4700,-4600,-4500,-4400,-4300,-4200,-4100,-4000,-3900,-3800,-3700,-3600,-3500,-3400,-3300,-3200,-3100,-3000,-2900,-2800,-2700,-2600,-2500,-2400,-2300,-2200,-2100,-2000,-1900,-1800,-1700,-1600,-1500,-1400,-1300,-1200,-1100,-1000,-900,-800,-700,-600,-500,-400,-300,-200,-100,0,100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500,1600,1700,1800,1900,2000,2100,2200,2300,2400,2500,2600,2700,2800,2900,3000,3100,3200,3300,3400,3500,3600,3700,3800,3900,4000,4100,4200,4300,4400,4500,4600,4700,4800,4900,5000,5100,5200,5300,5400,5500,5600,5700,5800,5900,6000,6100,6200,6300,6400,6500,6600,6700,6800,6900,7000,7100,7200,7300,7400,7500,7600,7700,7800,7900,8000,8100,8200,8300,8400,8500,8600,8700,8800,8900,9000,9100,9200,9300,9400,9500,9600,9700,9800,9900,10000,10100,10200,10300,10400,10500,10600,10700,10800,10900,11000,11100,11200,11300,11400,11500,11600,11700,11800,11900,12000,12100,12200,12300,12400,12500,12600,};
static int V_409_Lookup[255]={-52352,-51943,-51534,-51125,-50716,-50307,-49898,-49489,-49080,-48671,-48262,-47853,-47444,-47035,-46626,-46217,-45808,-45399,-44990,-44581,-44172,-43763,-43354,-42945,-42536,-42127,-41718,-41309,-40900,-40491,-40082,-39673,-39264,-38855,-38446,-38037,-37628,-37219,-36810,-36401,-35992,-35583,-35174,-34765,-34356,-33947,-33538,-33129,-32720,-32311,-31902,-31493,-31084,-30675,-30266,-29857,-29448,-29039,-28630,-28221,-27812,-27403,-26994,-26585,-26176,-25767,-25358,-24949,-24540,-24131,-23722,-23313,-22904,-22495,-22086,-21677,-21268,-20859,-20450,-20041,-19632,-19223,-18814,-18405,-17996,-17587,-17178,-16769,-16360,-15951,-15542,-15133,-14724,-14315,-13906,-13497,-13088,-12679,-12270,-11861,-11452,-11043,-10634,-10225,-9816,-9407,-8998,-8589,-8180,-7771,-7362,-6953,-6544,-6135,-5726,-5317,-4908,-4499,-4090,-3681,-3272,-2863,-2454,-2045,-1636,-1227,-818,-409,0,409,818,1227,1636,2045,2454,2863,3272,3681,4090,4499,4908,5317,5726,6135,6544,6953,7362,7771,8180,8589,8998,9407,9816,10225,10634,11043,11452,11861,12270,12679,13088,13497,13906,14315,14724,15133,15542,15951,16360,16769,17178,17587,17996,18405,18814,19223,19632,20041,20450,20859,21268,21677,22086,22495,22904,23313,23722,24131,24540,24949,25358,25767,26176,26585,26994,27403,27812,28221,28630,29039,29448,29857,30266,30675,31084,31493,31902,32311,32720,33129,33538,33947,34356,34765,35174,35583,35992,36401,36810,37219,37628,38037,38446,38855,39264,39673,40082,40491,40900,41309,41718,42127,42536,42945,43354,43763,44172,44581,44990,45399,45808,46217,46626,47035,47444,47853,48262,48671,49080,49489,49898,50307,50716,51125,51534,};

void IYUV_to_RGB24_4(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{

	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) __m128i Const128;
	Const128 = _mm_set1_epi16(128);

	for( j = 0; j < height - 2; j++)
	{
		//11.54
/*		for( i=0; i < width; i++)
		{
			int Y, U, V;
			int iX3 = i+i+i;
			Y = py[i];
			U = pu[i>>1];
			V = pv[i>>1];
			Dst_RGB[iX3] = CLIP3(0, 255, (300*(Y-16) + 517*(U-128) + 128 ) >> 8);
			Dst_RGB[iX3+1] = CLIP3(0, 255, (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) >> 8);
			Dst_RGB[iX3+2] = CLIP3(0, 255, (300*(Y-16) + 409*(V-128) + 128 ) >> 8);
		}/**/
		//11.84
/*		for( i=0; i < width; i++)
		{
			int Y, U, V;
			int iX3 = i+i+i;
			Y = py[i];
			U = pu[i>>1];
			V = pv[i>>1];
			int r = (300*(Y-16) + 517*(U-128) + 128 ) >> 8;
			int g = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) >> 8;
			int b = (300*(Y-16) + 409*(V-128) + 128 ) >> 8;
			r = CLIP3(0, 255, r);
			g = CLIP3(0, 255, g);
			b = CLIP3(0, 255, b);
			int write = b | ( g << 8 ) | ( r << 16 );
			*((int *)(&Dst_RGB[iX3])) = write;
		}/**/
		//11.86
/*		{
			//8.12
			for( i=0; i < width; i++)
			{
				int Y, U, V;
				int iX3 = i+i+i;
				Y = py[i];
				U = pu[i>>1];
				V = pv[i>>1];
				Dst_RGB[iX3] = (300*(Y-16) + 517*(U-128) + 128 ) >> 8;
				Dst_RGB[iX3+1] = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) >> 8;
				Dst_RGB[iX3+2] = (300*(Y-16) + 409*(V-128) + 128 ) >> 8;
			} 
			//4.38
			for( i=0; i < width; i++)
			{
				int iX3 = i+i+i;
				Dst_RGB[iX3] = CLIP3(0, 255, Dst_RGB[iX3]);
				Dst_RGB[iX3+1] = CLIP3(0, 255, Dst_RGB[iX3+1]);
				Dst_RGB[iX3+2] = CLIP3(0, 255, Dst_RGB[iX3+2]);
			}
		}/**/
		//8.12
/*		{
			for( i=0; i < width; i++)
			{
				int Y, U, V;
				int iX6 = i*6;
				Y = py[i];
				U = pu[i>>1];
				V = pv[i>>1];
				*(short*)&Dst_RGB[iX6+0] = (300*(Y-16) + 517*(U-128) + 128 ) >> 8;
				*(short*)&Dst_RGB[iX6+2] = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) >> 8;
				*(short*)&Dst_RGB[iX6+4] = (300*(Y-16) + 409*(V-128) + 128 ) >> 8;
			} 
			for( i=0; i < width; i+=6)
			{
				int iX3 = i*3;
				int iX6 = i*6;
				__declspec(align(16)) __m128i rgb = _mm_loadu_si128((__m128i*)(&Dst_RGB[iX6]));
				rgb = _mm_packus_epi16(rgb, zero);
				_mm_storel_epi64((__m128i*)(&Dst_RGB[iX3]), rgb);
			}
		}/**/
		//7.48
/*		{
			//8.12
			for( i=0; i < width; i++)
			{
				int Y, U, V;
				int iX6 = i*6;
				Y = py[i];
				U = pu[i>>1];
				V = pv[i>>1];
				*(short*)&Dst_RGB[iX6+0] = (300*(Y-16) + 517*(U-128) );
				*(short*)&Dst_RGB[iX6+2] = (300*(Y-16) - 208*(V-128) - 100*(U-128) );
				*(short*)&Dst_RGB[iX6+4] = (300*(Y-16) + 409*(V-128) );
			} 
			//4.38
			for( i=0; i < width; i+=6)
			{
				int iX3 = i*3;
				int iX6 = i*6;
				__declspec(align(16)) __m128i rgb = _mm_loadu_si128((__m128i*)(&Dst_RGB[iX6]));
				rgb = _mm_add_epi16(rgb, Const128);
				rgb = _mm_srai_epi32(rgb, 8);
				rgb = _mm_packus_epi16(rgb, zero);
				_mm_storel_epi64((__m128i*)(&Dst_RGB[iX3]), rgb);
			}
		}/**/
		//11.86
/*		{
			//11.22
			for( i=0; i < width; i++)
			{
				unsigned char Y, U, V;
				int iX6 = i*6;
				Y = py[i];
				U = pu[i>>1];
				V = pv[i>>1];
				*(short*)&Dst_RGB[iX6+0] = Y_300_128_Lookup[Y] + U_517_Lookup[U];
				*(short*)&Dst_RGB[iX6+2] = Y_300_128_Lookup[Y] - V_208_Lookup[V] - U_100_Lookup[U];
				*(short*)&Dst_RGB[iX6+4] = Y_300_128_Lookup[Y] + V_409_Lookup[V];
			} 
			//0.32
			for( i=0; i < width; i+=6)
			{
				int iX3 = i*3;
				int iX6 = i*6;
				__declspec(align(16)) __m128i rgb = _mm_loadu_si128((__m128i*)(&Dst_RGB[iX6]));
				rgb = _mm_srai_epi32(rgb, 8);
				rgb = _mm_packus_epi16(rgb, zero);
				_mm_storel_epi64((__m128i*)(&Dst_RGB[iX3]), rgb);
			}
		}/**/
		//14.36
/*		{
			for( i=0; i < width; i+=2)
			{
				unsigned int Y1, Y2, U, V, UV;
				int iX3 = i*3;
				Y1 = py[i];
				Y2 = py[i+1];
				U = pu[i>>1];
				V = pv[i>>1];

				Y1 = Y_300_128_Lookup[Y1];
				Y2 = Y_300_128_Lookup[Y2];
				UV = V_208_Lookup[V] + U_100_Lookup[U];
				U = U_517_Lookup[U];
				V = V_409_Lookup[V];

				int R1 = Y1 + U;
				int G1 = Y1 - UV;
				int B1 = Y1 + V;

				__declspec(align(16)) char buff1[16];
				*(int*)&buff1[0] = R1;
				*(int*)&buff1[4] = G1;
				*(int*)&buff1[8] = B1;

				int R2 = Y2 + U;
				int G2 = Y2 - UV;
				int B2 = Y2 + V;

				__declspec(align(16)) char buff2[16];
				*(int*)&buff2[0] = R2;
				*(int*)&buff2[4] = G2;
				*(int*)&buff2[8] = B2;

				__declspec(align(16)) __m128i rgb1 = _mm_loadu_si128((__m128i*)(&buff1));
				__declspec(align(16)) __m128i rgb2 = _mm_loadu_si128((__m128i*)(&buff2));
				rgb1 = _mm_srai_epi32(rgb1, 8);
				rgb2 = _mm_srai_epi32(rgb2, 8);
				rgb1 = _mm_packs_epi32(rgb1, rgb2);
				rgb1 = _mm_packus_epi16(rgb1, zero);
				_mm_storel_epi64((__m128i*)(&Dst_RGB[iX3]), rgb1);
			} 
		}/**/
		//9.36
/*		{
			//11.22
			for( i=0; i < width; i+=2)
			{
				unsigned int Y1, Y2, U, V, UV;
				int iX3 = i*3;
				Y1 = py[i];
				Y2 = py[i+1];
				U = pu[i>>1];
				V = pv[i>>1];

				Y1 = Y_300_128_Lookup[Y1];
				Y2 = Y_300_128_Lookup[Y2];
				UV = V_208_Lookup[V] + U_100_Lookup[U];
				U = U_517_Lookup[U];
				V = V_409_Lookup[V];

				int R1 = Y1 + U;
				int G1 = Y1 - UV;
				int B1 = Y1 + V;
				Dst_RGB[iX3+0] = CLIP3( 0, 255, R1 >> 8 );
				Dst_RGB[iX3+1] = CLIP3( 0, 255, G1 >> 8 );
				Dst_RGB[iX3+2] = CLIP3( 0, 255, B1 >> 8 );

				int R2 = Y2 + U;
				int G2 = Y2 - UV;
				int B2 = Y2 + V;
				Dst_RGB[iX3+3] = CLIP3( 0, 255, R2 >> 8 );
				Dst_RGB[iX3+4] = CLIP3( 0, 255, G2 >> 8 );
				Dst_RGB[iX3+5] = CLIP3( 0, 255, B2 >> 8 );
			} 
		}/**/
		//10.62
/*		{
			//11.22
			for( i=0; i < width; i++)
			{
				unsigned int Y1, U, V, UV;
				int iX3 = i*3;
				Y1 = py[i];
				U = pu[i>>1];
				V = pv[i>>1];

				Y1 = Y_300_128_Lookup[Y1];
				UV = V_208_Lookup[V] + U_100_Lookup[U];
				U = U_517_Lookup[U];
				V = V_409_Lookup[V];

				int R1 = Y1 + U;
				int G1 = Y1 - UV;
				int B1 = Y1 + V;
				Dst_RGB[iX3+0] = CLIP3( 0, 255, R1 >> 8 );
				Dst_RGB[iX3+1] = CLIP3( 0, 255, G1 >> 8 );
				Dst_RGB[iX3+2] = CLIP3( 0, 255, B1 >> 8 );
			} 
		}/**/
		//8.74
/*		{
			//11.22
			for( i=0; i < width; i+=2)
			{
				unsigned int Y[4], U, V, UV;
				int iX3 = i*3;
				Y[0] = py[i];
				Y[1] = py[i+1];
				Y[2] = py[i+src_stride];
				Y[3] = py[i+src_stride+1];
				U = pu[i>>1];
				V = pv[i>>1];

				Y[0] = Y_300_128_Lookup[Y[0]];
				Y[1] = Y_300_128_Lookup[Y[1]];
				Y[2] = Y_300_128_Lookup[Y[2]];
				Y[3] = Y_300_128_Lookup[Y[3]];
				UV = V_208_Lookup[V] + U_100_Lookup[U];
				U = U_517_Lookup[U];
				V = V_409_Lookup[V];

				int R,G,B;
				R = Y[0] + U;
				G = Y[0] - UV;
				B = Y[0] + V;
				Dst_RGB[iX3+0] = CLIP3( 0, 255, R >> 8 );
				Dst_RGB[iX3+1] = CLIP3( 0, 255, G >> 8 );
				Dst_RGB[iX3+2] = CLIP3( 0, 255, B >> 8 );

				R = Y[1] + U;
				G = Y[1] - UV;
				B = Y[1] + V;
				Dst_RGB[iX3+3] = CLIP3( 0, 255, R >> 8 );
				Dst_RGB[iX3+4] = CLIP3( 0, 255, G >> 8 );
				Dst_RGB[iX3+5] = CLIP3( 0, 255, B >> 8 );

				R = Y[2] + U;
				G = Y[2] - UV;
				B = Y[2] + V;
				Dst_RGB[iX3+dst_stride+0] = CLIP3( 0, 255, R >> 8 );
				Dst_RGB[iX3+dst_stride+1] = CLIP3( 0, 255, G >> 8 );
				Dst_RGB[iX3+dst_stride+2] = CLIP3( 0, 255, B >> 8 );

				R = Y[3] + U;
				G = Y[3] - UV;
				B = Y[3] + V;
				Dst_RGB[iX3+dst_stride+3] = CLIP3( 0, 255, R >> 8 );
				Dst_RGB[iX3+dst_stride+4] = CLIP3( 0, 255, G >> 8 );
				Dst_RGB[iX3+dst_stride+5] = CLIP3( 0, 255, B >> 8 );
			} 
			py += src_stride;
			j++;
		}/**/
		//5.3
/*		{
			for( i=0; i < width; i++)
			{
				unsigned char Y, U, V;
				int iX3 = i*3;
				Y = py[i];
				U = pu[i>>1];
				V = pv[i>>1];

				Dst_RGB[iX3+0] = Y;
				Dst_RGB[iX3+1] = U;
				Dst_RGB[iX3+2] = V;
			} 
		}/**/
		//3.74
/*		{
			for( i=0; i < width; i+=2)
			{
				unsigned char Y[4], U, V, UV;
				int iX3 = i*3;
				Y[0] = py[i];
				Y[1] = py[i+1];
				Y[2] = py[i+src_stride];
				Y[3] = py[i+src_stride+1];
				U = pu[i>>1];
				V = pv[i>>1];

				Dst_RGB[iX3+0] = Y[0];
				Dst_RGB[iX3+1] = U;
				Dst_RGB[iX3+2] = V;

				Dst_RGB[iX3+3] = Y[1];
				Dst_RGB[iX3+4] = U;
				Dst_RGB[iX3+5] = V;

				Dst_RGB[iX3+dst_stride+0] = Y[2];
				Dst_RGB[iX3+dst_stride+1] = U;
				Dst_RGB[iX3+dst_stride+2] = V;

				Dst_RGB[iX3+dst_stride+3] = Y[3];
				Dst_RGB[iX3+dst_stride+4] = U;
				Dst_RGB[iX3+dst_stride+5] = V;
			} 
			py += src_stride;
			j++;
		}/**/
		//11.24
		{
			for( i=0; i < width; i+=2)
			{
				unsigned char Y[4], U, V;
				int iX3 = i*3;
				Y[0] = py[i];
				Y[1] = py[i+1];
				Y[2] = py[i+src_stride];
				Y[3] = py[i+src_stride+1];
				U = pu[i>>1];
				V = pv[i>>1];

				Dst_RGB[iX3+0] = Y[0];
				Dst_RGB[iX3+1] = U;
				Dst_RGB[iX3+2] = V;

				Dst_RGB[iX3+3] = Y[1];
				Dst_RGB[iX3+4] = U;
				Dst_RGB[iX3+5] = V;

				Dst_RGB[iX3+dst_stride+0] = Y[2];
				Dst_RGB[iX3+dst_stride+1] = U;
				Dst_RGB[iX3+dst_stride+2] = V;

				Dst_RGB[iX3+dst_stride+3] = Y[3];
				Dst_RGB[iX3+dst_stride+4] = U;
				Dst_RGB[iX3+dst_stride+5] = V;
			} 
			for( i=0; i < width; i++)
			{
				int iX3 = i*3;
				__declspec(align(16)) __m128i YUV = _mm_loadu_si128((__m128i*)&Dst_RGB[iX3]);
				__declspec(align(16)) __m128 YUVF;
				YUVF = _mm_cvtepi32_ps( YUV );
//				YUV = _mm_mul_ps( YUV, zero );
				YUV = _mm_cvtps_epi32( YUVF );
				_mm_storeu_si128((__m128i*)&Dst_RGB[iX3], YUV);
			} 
			py += src_stride;
			j++;
		}/**/
		py += src_stride;
		if( (j&1) == 1 )
		{
			pu += (src_stride>>1);
			pv += (src_stride>>1);
		}
		Dst_RGB += src_stride;
	}
}
//5.62
void IYUV_to_RGB24_1(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{

	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	for( j = 0; j < height; j++)
	{
		for( i = 0; i < (width-8); i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sU, sV, temp0, temp1, temp2, temp3, AccLo, AccHi;
			__declspec(align(16)) unsigned char result[16];

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));
			sY = _mm_unpacklo_epi8(sY, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			//B
			AccLo = AccHi = _mm_setzero_si128();

			temp0 = _mm_set1_epi16(-16);
			temp3 = _mm_add_epi16(sY, temp0);
			temp0 = _mm_set1_epi16(300);
			temp1 = _mm_mullo_epi16(temp3, temp0);
			temp2 = _mm_mulhi_epi16(temp3, temp0);
			AccLo = _mm_unpacklo_epi16(temp1, temp2);
			AccHi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(-128);
			temp3 = _mm_add_epi16(sU, temp0);
			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(temp3, temp0);
			temp2 = _mm_mulhi_epi16(temp3, temp0);
			temp0 = _mm_unpacklo_epi16(temp1, temp2);
			AccLo = _mm_add_epi32(AccLo, temp0);
			temp0 = _mm_unpackhi_epi16(temp1, temp2);
			AccHi = _mm_add_epi32(AccHi, temp0);

			temp0 = _mm_set1_epi32(128);
			AccLo = _mm_add_epi32(AccLo, temp0);
			AccHi = _mm_add_epi32(AccHi, temp0);
			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3] = result[0]; 
			Dst_RGB[iX3+3] = result[1]; 
			Dst_RGB[iX3+6] = result[2]; 
			Dst_RGB[iX3+9] = result[3];
			Dst_RGB[iX3+12] = result[4]; 
			Dst_RGB[iX3+15] = result[5]; 
			Dst_RGB[iX3+18] = result[6]; 
			Dst_RGB[iX3+21] = result[7];

			//G
			AccLo = AccHi = _mm_setzero_si128();

			temp0 = _mm_set1_epi16(-16);
			temp3 = _mm_add_epi16(sY, temp0);
			temp0 = _mm_set1_epi16(300);
			temp1 = _mm_mullo_epi16(temp3, temp0);
			temp2 = _mm_mulhi_epi16(temp3, temp0);
			AccLo = _mm_unpacklo_epi16(temp1, temp2);
			AccHi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(-128);
			temp3 = _mm_add_epi16(sV, temp0);
			temp0 = _mm_set1_epi16(-208);
			temp1 = _mm_mullo_epi16(temp3, temp0);
			temp2 = _mm_mulhi_epi16(temp3, temp0);
			temp0 = _mm_unpacklo_epi16(temp1, temp2);
			AccLo = _mm_add_epi32(AccLo, temp0);
			temp0 = _mm_unpackhi_epi16(temp1, temp2);
			AccHi = _mm_add_epi32(AccHi, temp0);

			temp0 = _mm_set1_epi16(-128);
			temp3 = _mm_add_epi16(sU, temp0);
			temp0 = _mm_set1_epi16(-100);
			temp1 = _mm_mullo_epi16(temp3, temp0);
			temp2 = _mm_mulhi_epi16(temp3, temp0);
			temp0 = _mm_unpacklo_epi16(temp1, temp2);
			AccLo = _mm_add_epi32(AccLo, temp0);
			temp0 = _mm_unpackhi_epi16(temp1, temp2);
			AccHi = _mm_add_epi32(AccHi, temp0);

			temp0 = _mm_set1_epi32(128);
			AccLo = _mm_add_epi32(AccLo, temp0);
			AccHi = _mm_add_epi32(AccHi, temp0);
			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3+1] = result[0]; 
			Dst_RGB[iX3+1+3] = result[1]; 
			Dst_RGB[iX3+1+6] = result[2]; 
			Dst_RGB[iX3+1+9] = result[3];
			Dst_RGB[iX3+1+12] = result[4]; 
			Dst_RGB[iX3+1+15] = result[5]; 
			Dst_RGB[iX3+1+18] = result[6]; 
			Dst_RGB[iX3+1+21] = result[7];

			//R
			AccLo = AccHi = _mm_setzero_si128();

			temp0 = _mm_set1_epi16(-16);
			temp3 = _mm_add_epi16(sY, temp0);
			temp0 = _mm_set1_epi16(300);
			temp1 = _mm_mullo_epi16(temp3, temp0);
			temp2 = _mm_mulhi_epi16(temp3, temp0);
			AccLo = _mm_unpacklo_epi16(temp1, temp2);
			AccHi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(-128);
			temp3 = _mm_add_epi16(sV, temp0);
			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(temp3, temp0);
			temp2 = _mm_mulhi_epi16(temp3, temp0);
			temp0 = _mm_unpacklo_epi16(temp1, temp2);
			AccLo = _mm_add_epi32(AccLo, temp0);
			temp0 = _mm_unpackhi_epi16(temp1, temp2);
			AccHi = _mm_add_epi32(AccHi, temp0);

			temp0 = _mm_set1_epi32(128);
			AccLo = _mm_add_epi32(AccLo, temp0);
			AccHi = _mm_add_epi32(AccHi, temp0);
			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3+2] = result[0]; 
			Dst_RGB[iX3+2+3] = result[1]; 
			Dst_RGB[iX3+2+6] = result[2]; 
			Dst_RGB[iX3+2+9] = result[3];
			Dst_RGB[iX3+2+12] = result[4]; 
			Dst_RGB[iX3+2+15] = result[5]; 
			Dst_RGB[iX3+2+18] = result[6]; 
			Dst_RGB[iX3+2+21] = result[7];

		}
		for( ; i < width; i++)
		{
			unsigned char Y, U, V;
			int iX3 = i+i+i;
			Y = py[i];
			U = pu[i>>1];
			V = pv[i>>1];
			Dst_RGB[iX3] = CLIP3(0, 255, (300*(Y-16) + 517*(U-128) + 128 ) >> 8);
			Dst_RGB[iX3+1] = CLIP3(0, 255, (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) >> 8);
			Dst_RGB[iX3+2] = CLIP3(0, 255, (300*(Y-16) + 409*(V-128) + 128 ) >> 8);
		}
		py += src_stride;
		if( (j&1) == 1 )
		{
			pu += (src_stride>>1);
			pv += (src_stride>>1);
		}
		Dst_RGB += src_stride;
	}
}
//5.0
void IYUV_to_RGB24_2(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) __m128i Const16;
	Const16 = _mm_set1_epi16(-16);
	__declspec(align(16)) __m128i Const128;
	Const128 = _mm_set1_epi16(128);
//	__declspec(align(16)) __m128i Const128_x32;
//	Const128_x32 = _mm_set1_epi32(128);
	for( j = 0; j < height; j++)
	{
		for( i = 0; i < width; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi;
			__declspec(align(16)) unsigned char result[16];

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));
			sY = _mm_unpacklo_epi8(sY, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_add_epi16(sY, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp0 = _mm_set1_epi16(300);
			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3] = result[0]; 
			Dst_RGB[iX3+3] = result[1]; 
			Dst_RGB[iX3+6] = result[2]; 
			Dst_RGB[iX3+9] = result[3];
			Dst_RGB[iX3+12] = result[4]; 
			Dst_RGB[iX3+15] = result[5]; 
			Dst_RGB[iX3+18] = result[6]; 
			Dst_RGB[iX3+21] = result[7];

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sV208Lo);
			AccHi = _mm_sub_epi32(sY300Hi, sV208Hi);
			AccLo = _mm_sub_epi32(AccLo, sU100Lo);
			AccHi = _mm_sub_epi32(AccHi, sU100Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3+1] = result[0]; 
			Dst_RGB[iX3+1+3] = result[1]; 
			Dst_RGB[iX3+1+6] = result[2]; 
			Dst_RGB[iX3+1+9] = result[3];
			Dst_RGB[iX3+1+12] = result[4]; 
			Dst_RGB[iX3+1+15] = result[5]; 
			Dst_RGB[iX3+1+18] = result[6]; 
			Dst_RGB[iX3+1+21] = result[7];

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3+2] = result[0]; 
			Dst_RGB[iX3+2+3] = result[1]; 
			Dst_RGB[iX3+2+6] = result[2]; 
			Dst_RGB[iX3+2+9] = result[3];
			Dst_RGB[iX3+2+12] = result[4]; 
			Dst_RGB[iX3+2+15] = result[5]; 
			Dst_RGB[iX3+2+18] = result[6]; 
			Dst_RGB[iX3+2+21] = result[7];
		}
		py += src_stride;
		if( (j&1) == 1 )
		{
			pu += (src_stride/2);
			pv += (src_stride/2);
		}
		Dst_RGB += src_stride;
	}
}
//4.98
void IYUV_to_RGB24_3(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) __m128i Const16;
	Const16 = _mm_set1_epi16(-16);
	__declspec(align(16)) __m128i Const128;
	Const128 = _mm_set1_epi16(128);
	for( j = 0; j < height; j+=2)
	{
		for( i = 0; i < width; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sY2, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi, sY300Lo2, sY300Hi2;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi;
			__declspec(align(16)) unsigned char result[16];

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sY2 = _mm_loadl_epi64((__m128i*)(&py[i+src_stride]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));
			sY = _mm_unpacklo_epi8(sY, zero);
			sY2 = _mm_unpacklo_epi8(sY2, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_add_epi16(sY, Const16);
			sY2 = _mm_add_epi16(sY, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp0 = _mm_set1_epi16(300);
			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp1 = _mm_mullo_epi16(sY2, temp0);
			temp2 = _mm_mulhi_epi16(sY2, temp0);
			sY300Lo2 = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi2 = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3] = result[0]; 
			Dst_RGB[iX3+3] = result[1]; 
			Dst_RGB[iX3+6] = result[2]; 
			Dst_RGB[iX3+9] = result[3];
			Dst_RGB[iX3+12] = result[4]; 
			Dst_RGB[iX3+15] = result[5]; 
			Dst_RGB[iX3+18] = result[6]; 
			Dst_RGB[iX3+21] = result[7];

			AccLo = _mm_add_epi32(sY300Lo2, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[dst_stride+iX3] = result[0]; 
			Dst_RGB[dst_stride+iX3+3] = result[1]; 
			Dst_RGB[dst_stride+iX3+6] = result[2]; 
			Dst_RGB[dst_stride+iX3+9] = result[3];
			Dst_RGB[dst_stride+iX3+12] = result[4]; 
			Dst_RGB[dst_stride+iX3+15] = result[5]; 
			Dst_RGB[dst_stride+iX3+18] = result[6]; 
			Dst_RGB[dst_stride+iX3+21] = result[7];

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sV208Lo);
			AccHi = _mm_sub_epi32(sY300Hi, sV208Hi);
			AccLo = _mm_sub_epi32(AccLo, sU100Lo);
			AccHi = _mm_sub_epi32(AccHi, sU100Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3+1] = result[0]; 
			Dst_RGB[iX3+1+3] = result[1]; 
			Dst_RGB[iX3+1+6] = result[2]; 
			Dst_RGB[iX3+1+9] = result[3];
			Dst_RGB[iX3+1+12] = result[4]; 
			Dst_RGB[iX3+1+15] = result[5]; 
			Dst_RGB[iX3+1+18] = result[6]; 
			Dst_RGB[iX3+1+21] = result[7];

			AccLo = _mm_sub_epi32(sY300Lo2, sV208Lo);
			AccHi = _mm_sub_epi32(sY300Hi2, sV208Hi);
			AccLo = _mm_sub_epi32(AccLo, sU100Lo);
			AccHi = _mm_sub_epi32(AccHi, sU100Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[dst_stride+iX3+1] = result[0]; 
			Dst_RGB[dst_stride+iX3+1+3] = result[1]; 
			Dst_RGB[dst_stride+iX3+1+6] = result[2]; 
			Dst_RGB[dst_stride+iX3+1+9] = result[3];
			Dst_RGB[dst_stride+iX3+1+12] = result[4]; 
			Dst_RGB[dst_stride+iX3+1+15] = result[5]; 
			Dst_RGB[dst_stride+iX3+1+18] = result[6]; 
			Dst_RGB[dst_stride+iX3+1+21] = result[7];

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[iX3+2] = result[0]; 
			Dst_RGB[iX3+2+3] = result[1]; 
			Dst_RGB[iX3+2+6] = result[2]; 
			Dst_RGB[iX3+2+9] = result[3];
			Dst_RGB[iX3+2+12] = result[4]; 
			Dst_RGB[iX3+2+15] = result[5]; 
			Dst_RGB[iX3+2+18] = result[6]; 
			Dst_RGB[iX3+2+21] = result[7];

			AccLo = _mm_add_epi32(sY300Lo2, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			AccLo = _mm_packus_epi16(AccLo, zero);
			_mm_storel_epi64((__m128i*)result, AccLo);

			Dst_RGB[dst_stride+iX3+2] = result[0]; 
			Dst_RGB[dst_stride+iX3+2+3] = result[1]; 
			Dst_RGB[dst_stride+iX3+2+6] = result[2]; 
			Dst_RGB[dst_stride+iX3+2+9] = result[3];
			Dst_RGB[dst_stride+iX3+2+12] = result[4]; 
			Dst_RGB[dst_stride+iX3+2+15] = result[5]; 
			Dst_RGB[dst_stride+iX3+2+18] = result[6]; 
			Dst_RGB[dst_stride+iX3+2+21] = result[7];
		}
		py += (src_stride*2);
		pu += (src_stride/2);
		pv += (src_stride/2);
		Dst_RGB += (src_stride*2);
	}
}
void IYUV_to_RGB24_5(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) __m128i Const16;
	Const16 = _mm_set1_epi16(16);
	__declspec(align(16)) __m128i Const128;
	Const128 = _mm_set1_epi16(128);
	for( j = 0; j < height; j+=2)
	{
		for( i = 0; i < width; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sY2, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi, sY300Lo2, sY300Hi2;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi, sUVLo, sUVHi;
			__declspec(align(16)) __m128i R[4],G[4],B[4];

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sY2 = _mm_loadl_epi64((__m128i*)(&py[i+src_stride]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));
			sY = _mm_unpacklo_epi8(sY, zero);
			sY2 = _mm_unpacklo_epi8(sY2, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_sub_epi16(sY, Const16);
			sY2 = _mm_sub_epi16(sY, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp0 = _mm_set1_epi16(300);
			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp1 = _mm_mullo_epi16(sY2, temp0);
			temp2 = _mm_mulhi_epi16(sY2, temp0);
			sY300Lo2 = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi2 = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			sUVLo = _mm_add_epi32(sV208Lo, sU100Lo);
			sUVHi = _mm_add_epi32(sV208Hi, sU100Hi);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
R[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
R[1] = _mm_packus_epi16(AccLo, zero);

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
G[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_sub_epi32(sY300Lo2, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi2, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
G[1] = _mm_packus_epi16(AccLo, zero);

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
B[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
B[1] = _mm_packus_epi16(AccLo, zero);

R[2] = _mm_unpacklo_epi8(R[0], G[0]);
R[3] = _mm_unpacklo_epi8(R[1], G[1]);
B[2] = _mm_unpacklo_epi8(B[0], zero);
B[3] = _mm_unpacklo_epi8(B[1], zero);
__declspec(align(16)) __m128i RGB32Lo[2],RGB32Hi[2];
RGB32Lo[0] = _mm_unpacklo_epi16(R[2], B[2]);
RGB32Hi[0] = _mm_unpackhi_epi16(R[2], B[2]);
RGB32Lo[1] = _mm_unpacklo_epi16(R[3], B[3]);
RGB32Hi[1] = _mm_unpackhi_epi16(R[3], B[3]);
_mm_storel_epi64((__m128i*)&Dst_RGB[iX3+0], RGB32Lo[0]);
_mm_storel_epi64((__m128i*)&Dst_RGB[iX3+12], RGB32Hi[0]);
_mm_storel_epi64((__m128i*)&Dst_RGB[dst_stride+iX3+0], RGB32Lo[1]);
_mm_storel_epi64((__m128i*)&Dst_RGB[dst_stride+iX3+12], RGB32Hi[1]);
RGB32Lo[0] = _mm_srli_si128( RGB32Lo[0], 4 );
RGB32Hi[0] = _mm_srli_si128( RGB32Hi[0], 4 );
RGB32Lo[1] = _mm_srli_si128( RGB32Lo[1], 4 );
RGB32Hi[1] = _mm_srli_si128( RGB32Hi[1], 4 );
_mm_storel_epi64((__m128i*)&Dst_RGB[iX3+3], RGB32Lo[0]);
_mm_storel_epi64((__m128i*)&Dst_RGB[iX3+15], RGB32Hi[0]);
_mm_storel_epi64((__m128i*)&Dst_RGB[dst_stride+iX3+3], RGB32Lo[1]);
_mm_storel_epi64((__m128i*)&Dst_RGB[dst_stride+iX3+15], RGB32Hi[1]);
RGB32Lo[0] = _mm_srli_si128( RGB32Lo[0], 4 );
RGB32Hi[0] = _mm_srli_si128( RGB32Hi[0], 4 );
RGB32Lo[1] = _mm_srli_si128( RGB32Lo[1], 4 );
RGB32Hi[1] = _mm_srli_si128( RGB32Hi[1], 4 );
_mm_storel_epi64((__m128i*)&Dst_RGB[iX3+6], RGB32Lo[0]);
_mm_storel_epi64((__m128i*)&Dst_RGB[iX3+18], RGB32Hi[0]);
_mm_storel_epi64((__m128i*)&Dst_RGB[dst_stride+iX3+6], RGB32Lo[1]);
_mm_storel_epi64((__m128i*)&Dst_RGB[dst_stride+iX3+18], RGB32Hi[1]);
RGB32Lo[0] = _mm_srli_si128( RGB32Lo[0], 4 );
RGB32Hi[0] = _mm_srli_si128( RGB32Hi[0], 4 );
RGB32Lo[1] = _mm_srli_si128( RGB32Lo[1], 4 );
RGB32Hi[1] = _mm_srli_si128( RGB32Hi[1], 4 );
_mm_storel_epi64((__m128i*)&Dst_RGB[iX3+9], RGB32Lo[0]);
_mm_storel_epi64((__m128i*)&Dst_RGB[iX3+21], RGB32Hi[0]);
_mm_storel_epi64((__m128i*)&Dst_RGB[dst_stride+iX3+9], RGB32Lo[1]);
_mm_storel_epi64((__m128i*)&Dst_RGB[dst_stride+iX3+21], RGB32Hi[1]);
		}
		py += (src_stride*2);
		pu += (src_stride/2);
		pv += (src_stride/2);
		Dst_RGB += (src_stride*2);
	}
}
//2.58
void IYUV_to_RGB24_6(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) __m128i Const16;
	Const16 = _mm_set1_epi16(16);
	__declspec(align(16)) __m128i Const128;
	Const128 = _mm_set1_epi16(128);
	__declspec(align(16)) char c_shufflemask[16] = { 0,1,2,4,5,6,8,9,10,12,13,14 };
	for( j = 0; j < height; j+=2)
	{
		for( i = 0; i < width; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sY2, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi, sY300Lo2, sY300Hi2;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi, sUVLo, sUVHi;
			__declspec(align(16)) __m128i R[4],G[4],B[4];

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sY2 = _mm_loadl_epi64((__m128i*)(&py[i+src_stride]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));
			sY = _mm_unpacklo_epi8(sY, zero);
			sY2 = _mm_unpacklo_epi8(sY2, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_sub_epi16(sY, Const16);
			sY2 = _mm_sub_epi16(sY2, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp0 = _mm_set1_epi16(300);
			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp1 = _mm_mullo_epi16(sY2, temp0);
			temp2 = _mm_mulhi_epi16(sY2, temp0);
			sY300Lo2 = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi2 = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi32(128);
			sY300Lo = _mm_add_epi32(sY300Lo, temp0);
			sY300Hi = _mm_add_epi32(sY300Hi, temp0);
			sY300Lo2 = _mm_add_epi32(sY300Lo2, temp0);
			sY300Hi2 = _mm_add_epi32(sY300Hi2, temp0);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			sUVLo = _mm_add_epi32(sV208Lo, sU100Lo);
			sUVHi = _mm_add_epi32(sV208Hi, sU100Hi);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[1] = _mm_packus_epi16(AccLo, zero);

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_sub_epi32(sY300Lo2, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi2, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[1] = _mm_packus_epi16(AccLo, zero);

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[1] = _mm_packus_epi16(AccLo, zero);

			R[2] = _mm_unpacklo_epi8(R[0], G[0]);
			R[3] = _mm_unpacklo_epi8(R[1], G[1]);
			B[2] = _mm_unpacklo_epi8(B[0], zero);
			B[3] = _mm_unpacklo_epi8(B[1], zero);
			__declspec(align(16)) __m128i RGB32Lo[2],RGB32Hi[2];
			RGB32Lo[0] = _mm_unpacklo_epi16(R[2], B[2]);
			RGB32Hi[0] = _mm_unpackhi_epi16(R[2], B[2]);
			RGB32Lo[1] = _mm_unpacklo_epi16(R[3], B[3]);
			RGB32Hi[1] = _mm_unpackhi_epi16(R[3], B[3]);
			__declspec(align(16)) __m128i shufflemask;
			shufflemask = _mm_load_si128((__m128i*)c_shufflemask);
			__declspec(align(16)) __m128i pix123,pix456,pix123_,pix456_;
			pix123 = _mm_shuffle_epi8(RGB32Lo[0], shufflemask);
			pix456 = _mm_shuffle_epi8(RGB32Hi[0], shufflemask);
			pix123_ = _mm_shuffle_epi8(RGB32Lo[1], shufflemask);
			pix456_ = _mm_shuffle_epi8(RGB32Hi[1], shufflemask);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+0], pix123);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+12], pix456);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+dst_stride+0], pix123_);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+dst_stride+12], pix456_);
		}
		py += (src_stride*2);
		pu += (src_stride/2);
		pv += (src_stride/2);
		Dst_RGB += (dst_stride*2);
	}
}
//3.09
void IYUV_to_RGB24_7(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) __m128i Const16;
	Const16 = _mm_set1_epi16(16);
	__declspec(align(16)) __m128i Const128;
	Const128 = _mm_set1_epi16(128);
	__declspec(align(16)) char c_shufflemask[16] = { 0,1,2,4,5,6,8,9,10,12,13,14 };
	for( j = 0; j < height; j++)
	{
		for( i = 0; i < width; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi, sUVLo, sUVHi;
			__declspec(align(16)) __m128i R[4],G[4],B[4];

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));
			sY = _mm_unpacklo_epi8(sY, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_sub_epi16(sY, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp0 = _mm_set1_epi16(300);
			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi32(128);
			sY300Lo = _mm_add_epi32(sY300Lo, temp0);
			sY300Hi = _mm_add_epi32(sY300Hi, temp0);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			sUVLo = _mm_add_epi32(sV208Lo, sU100Lo);
			sUVHi = _mm_add_epi32(sV208Hi, sU100Hi);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[0] = _mm_packus_epi16(AccLo, zero);

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[0] = _mm_packus_epi16(AccLo, zero);

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[0] = _mm_packus_epi16(AccLo, zero);

			R[2] = _mm_unpacklo_epi8(R[0], G[0]);
			B[2] = _mm_unpacklo_epi8(B[0], zero);
			__declspec(align(16)) __m128i RGB32Lo[2],RGB32Hi[2];
			RGB32Lo[0] = _mm_unpacklo_epi16(R[2], B[2]);
			RGB32Hi[0] = _mm_unpackhi_epi16(R[2], B[2]);
			__declspec(align(16)) __m128i shufflemask;
			shufflemask = _mm_load_si128((__m128i*)c_shufflemask);
			__declspec(align(16)) __m128i pix123,pix456;
			pix123 = _mm_shuffle_epi8(RGB32Lo[0], shufflemask);
			pix456 = _mm_shuffle_epi8(RGB32Hi[0], shufflemask);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+0], pix123);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+12], pix456);
		}
		py += src_stride;
		if( (j&1) == 1 )
		{
			pu += (src_stride/2);
			pv += (src_stride/2);
		}
		Dst_RGB += dst_stride;
	}
}
//2.54
void IYUV_to_RGB24_8(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) char c_shufflemask[16] = { 0,1,2,4,5,6,8,9,10,12,13,14 };
	int SSEHeight = height & (~0x01);
	int SSEWidth = (width-8) & (~0x07); //write 32 bytes instead 24(12+12) leads to bad output
	for( j = 0; j < SSEHeight; j+=2)
	{
		for( i = 0; i < SSEWidth; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sY2, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi, sY300Lo2, sY300Hi2;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi, sUVLo, sUVHi;
			__declspec(align(16)) __m128i R[4],G[4],B[4];
			__declspec(align(16)) __m128i Const16;
			__declspec(align(16)) __m128i Const128;

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sY2 = _mm_loadl_epi64((__m128i*)(&py[i+src_stride]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));

			Const16 = _mm_set1_epi16(16);
			Const128 = _mm_set1_epi16(128);
			temp0 = _mm_set1_epi16(300);

			sY = _mm_unpacklo_epi8(sY, zero);
			sY2 = _mm_unpacklo_epi8(sY2, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_sub_epi16(sY, Const16);
			sY2 = _mm_sub_epi16(sY2, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp1 = _mm_mullo_epi16(sY2, temp0);
			temp2 = _mm_mulhi_epi16(sY2, temp0);
			sY300Lo2 = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi2 = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi32(128);
			sY300Lo = _mm_add_epi32(sY300Lo, temp0);
			sY300Hi = _mm_add_epi32(sY300Hi, temp0);
			sY300Lo2 = _mm_add_epi32(sY300Lo2, temp0);
			sY300Hi2 = _mm_add_epi32(sY300Hi2, temp0);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			sUVLo = _mm_add_epi32(sV208Lo, sU100Lo);
			sUVHi = _mm_add_epi32(sV208Hi, sU100Hi);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[1] = _mm_packus_epi16(AccLo, zero);

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_sub_epi32(sY300Lo2, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi2, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[1] = _mm_packus_epi16(AccLo, zero);

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[1] = _mm_packus_epi16(AccLo, zero);

			R[2] = _mm_unpacklo_epi8(R[0], G[0]);
			R[3] = _mm_unpacklo_epi8(R[1], G[1]);
			B[2] = _mm_unpacklo_epi8(B[0], zero);
			B[3] = _mm_unpacklo_epi8(B[1], zero);
			__declspec(align(16)) __m128i RGB32Lo[2],RGB32Hi[2];
			RGB32Lo[0] = _mm_unpacklo_epi16(R[2], B[2]);
			RGB32Hi[0] = _mm_unpackhi_epi16(R[2], B[2]);
			RGB32Lo[1] = _mm_unpacklo_epi16(R[3], B[3]);
			RGB32Hi[1] = _mm_unpackhi_epi16(R[3], B[3]);
			__declspec(align(16)) __m128i shufflemask;
			shufflemask = _mm_load_si128((__m128i*)c_shufflemask);
			__declspec(align(16)) __m128i pix1234,pix5678,pix1234_,pix5678_;
			pix1234 = _mm_shuffle_epi8(RGB32Lo[0], shufflemask);
			pix5678 = _mm_shuffle_epi8(RGB32Hi[0], shufflemask);
			pix1234_ = _mm_shuffle_epi8(RGB32Lo[1], shufflemask);
			pix5678_ = _mm_shuffle_epi8(RGB32Hi[1], shufflemask);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+0], pix1234);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+12], pix5678);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+dst_stride+0], pix1234_);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+dst_stride+12], pix5678_);
		}
		for( ;i < width; i++)
		{
			int Y, Y2, U, V, R, G, B, R2, G2, B2;
			int iX3 = i+i+i;
			Y = py[i];
			Y2 = py[i+src_stride];
			U = pu[i/2];
			V = pv[i/2];
			int T1 = 517*(U-128) + 128;
			int T2 = -208*(V-128) - 100*(U-128) + 128;
			int T3 = 409*(V-128) + 128;
			R = (300*(Y-16) + T1 ) / 256;
			G = (300*(Y-16) + T2 ) / 256;
			B = (300*(Y-16) + T3 ) / 256;
			R2 = (300*(Y2-16) + T1 ) / 256;
			G2 = (300*(Y2-16) + T2 ) / 256;
			B2 = (300*(Y2-16) + T3 ) / 256;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, R);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, B);
			Dst_RGB[dst_stride + iX3 + 0] = CLIP3(0, 255, R2);
			Dst_RGB[dst_stride + iX3 + 1] = CLIP3(0, 255, G2);
			Dst_RGB[dst_stride + iX3 + 2] = CLIP3(0, 255, B2);
		}/**/
		py += (src_stride*2);
		pu += (src_stride/2);
		pv += (src_stride/2);
		Dst_RGB += (dst_stride*2);
	}
	if( height & 0x01 )
	{
		for( i=0; i < width; i++)
		{
			int Y, U, V, R, G, B;
			int iX3 = i+i+i;
			Y = py[i];
			U = pu[i/2];
			V = pv[i/2];
			R = (300*(Y-16) + 517*(U-128) + 128 ) / 256;
			G = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) / 256;
			B = (300*(Y-16) + 409*(V-128) + 128 ) / 256;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, R);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, B);
		}
	}/**/
}
//2.465
void IYUV_to_RGB24_9(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) char c_shufflemask[16] = { 0,1,2,4,5,6,8,9,10,12,13,14 };
	for( j = 0; j < height; j+=2)
	{
		for( i = 0; i < width-8; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sY2, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi, sY300Lo2, sY300Hi2;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi, sUVLo, sUVHi;
			__declspec(align(16)) __m128i R[4],G[4],B[4];
			__declspec(align(16)) __m128i Const16;
			__declspec(align(16)) __m128i Const128;

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sY2 = _mm_loadl_epi64((__m128i*)(&py[i+src_stride]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));

			Const16 = _mm_set1_epi16(16);
			Const128 = _mm_set1_epi16(128);
			temp0 = _mm_set1_epi16(300);

			sY = _mm_unpacklo_epi8(sY, zero);
			sY2 = _mm_unpacklo_epi8(sY2, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_sub_epi16(sY, Const16);
			sY2 = _mm_sub_epi16(sY2, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp1 = _mm_mullo_epi16(sY2, temp0);
			temp2 = _mm_mulhi_epi16(sY2, temp0);
			sY300Lo2 = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi2 = _mm_unpackhi_epi16(temp1, temp2);

//			temp0 = _mm_set1_epi32(128);
//			sY300Lo = _mm_add_epi32(sY300Lo, temp0);
//			sY300Hi = _mm_add_epi32(sY300Hi, temp0);
//			sY300Lo2 = _mm_add_epi32(sY300Lo2, temp0);
//			sY300Hi2 = _mm_add_epi32(sY300Hi2, temp0);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			sUVLo = _mm_add_epi32(sV208Lo, sU100Lo);
			sUVHi = _mm_add_epi32(sV208Hi, sU100Hi);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[1] = _mm_packus_epi16(AccLo, zero);

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_sub_epi32(sY300Lo2, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi2, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[1] = _mm_packus_epi16(AccLo, zero);

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[1] = _mm_packus_epi16(AccLo, zero);

			R[2] = _mm_unpacklo_epi8(R[0], G[0]);
			R[3] = _mm_unpacklo_epi8(R[1], G[1]);
			B[2] = _mm_unpacklo_epi8(B[0], zero);
			B[3] = _mm_unpacklo_epi8(B[1], zero);
			__declspec(align(16)) __m128i RGB32Lo[2],RGB32Hi[2];
			RGB32Lo[0] = _mm_unpacklo_epi16(R[2], B[2]);
			RGB32Hi[0] = _mm_unpackhi_epi16(R[2], B[2]);
			RGB32Lo[1] = _mm_unpacklo_epi16(R[3], B[3]);
			RGB32Hi[1] = _mm_unpackhi_epi16(R[3], B[3]);
			__declspec(align(16)) __m128i shufflemask;
			shufflemask = _mm_load_si128((__m128i*)c_shufflemask);
			__declspec(align(16)) __m128i pix1234,pix5678,pix1234_,pix5678_;
			pix1234 = _mm_shuffle_epi8(RGB32Lo[0], shufflemask);
			pix5678 = _mm_shuffle_epi8(RGB32Hi[0], shufflemask);
			pix1234_ = _mm_shuffle_epi8(RGB32Lo[1], shufflemask);
			pix5678_ = _mm_shuffle_epi8(RGB32Hi[1], shufflemask);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+0], pix1234);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+12], pix5678);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+dst_stride+0], pix1234_);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+dst_stride+12], pix5678_);
		}
		for( ;i < width; i++)
		{
			int Y, Y2, U, V, R, R2, G, B;
			int iX3 = i+i+i;
			Y = py[i];
			Y2 = py[i+src_stride];
			U = pu[i/2];
			V = pv[i/2];
			R = (300*(Y-16) + 517*(U-128) + 128 ) >> 8;
			R2 = (300*(Y2-16) + 517*(U-128) + 128 ) >> 8;
			G = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) >> 8;
			B = (300*(Y-16) + 409*(V-128) + 128 ) >> 8;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, R);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, B);
			Dst_RGB[dst_stride + iX3 + 0] = CLIP3(0, 255, R);
			Dst_RGB[dst_stride + iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[dst_stride + iX3 + 2] = CLIP3(0, 255, B);
		}
		py += (src_stride*2);
		pu += (src_stride/2);
		pv += (src_stride/2);
		Dst_RGB += (dst_stride*2);
	}
}
//2.5
void IYUV_to_RGB24__(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
//	memcpy( Dst_RGB, Src_IYUV, height *  width * 3 / 2 );
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	for( int j = 0; j < height; j++)
	{
		for( int i = 0; i < width; i++)
		{
			Dst_RGB[3*i+0] = py[i];
			Dst_RGB[3*i+1] = pu[i/2];
			Dst_RGB[3*i+2] = pv[i/2];
		}
		Dst_RGB += dst_stride;
		py += src_stride;
	}
}
//3.42
void IYUV_to_RGB24___(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	for( int j = 0; j < height; j+=2)
	{
		for( int i = 0; i < width; i++)
		{
			Dst_RGB[3*i+0] = py[i];
			Dst_RGB[3*i+1] = pu[i/2];
			Dst_RGB[3*i+2] = pv[i/2];
			Dst_RGB[dst_stride+3*i+0] = py[i+src_stride];
			Dst_RGB[dst_stride+3*i+1] = pu[i/2];
			Dst_RGB[dst_stride+3*i+2] = pv[i/2];
		}
		Dst_RGB += dst_stride;
		py += src_stride;
	}
}

//3.43
void RGB24_to_GrayScale_reference(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels)
{
	//we are using 3 levels of gray : 4, 8, 16
	//we have 3 color channels so we multiply by 3
	int multiplier = ( DIV_PRECISION_VALUE * GrayscaleLevels ) / ( 256 * 3 );
	for( int j = 0; j < height; j++)
	{
		for( int i = 0; i < width*3; i += 3 )
		{
			int R,G,B;
			R = Src_RGB[i+0];
			G = Src_RGB[i+1];
			B = Src_RGB[i+2];
			int res = (R+G+B)*multiplier;
			res = res >> DIV_PRECISSION_BITS;
			res = res * GrayscaleLevels;
			Dst_RGB[i+0] = res;
			Dst_RGB[i+1] = res;
			Dst_RGB[i+2] = res;
		}
		Src_RGB += src_stride;
		Dst_RGB += dst_stride;
	}
}

void RGB24_to_GrayScale_1(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels)
{
	//we are using 3 levels of gray : 4, 8, 16
	//we have 3 color channels so we multiply by 3
	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	for( int j = 0; j < height; j++)
	{
		for( int i = 0; i < width*3; i += 3*8 )
		{
			__declspec(align(16)) __m128i R,G,B,RL,GL,BL,RH,GH,BH,RGBL,RGBH;
			//load up 8 values
			R = _mm_loadu_si128((__m128i*)(&Src_RGB[i]));
			G = _mm_loadu_si128((__m128i*)(&Src_RGB[i+1]));
			B = _mm_loadu_si128((__m128i*)(&Src_RGB[i+2]));
			//unpack
			RL = _mm_unpacklo_epi8(R, zero);
			GL = _mm_unpacklo_epi8(G, zero);
			BL = _mm_unpacklo_epi8(B, zero);
			RH = _mm_unpackhi_epi8(R, zero);
			GH = _mm_unpackhi_epi8(G, zero);
			BH = _mm_unpackhi_epi8(B, zero);
			//add them up
			RGBL = _mm_add_epi16(RL, GL);
			RGBL = _mm_add_epi16(RGBL, BL);
			RGBH = _mm_add_epi16(RH, GH);
			RGBH = _mm_add_epi16(RGBH, BH);
			//divide to get only 3 levels
			RGBL = _mm_srai_epi16(RGBL, 8);
			RGBH = _mm_srai_epi16(RGBH, 8);
			//pack
			RGBL = _mm_unpacklo_epi16(RGBL, RGBH);
			//scale them up as requested
			__declspec(align(16)) __m128i one = _mm_set1_epi16(1);
			RGBL = _mm_add_epi16(RGBL, one);
			RGBL = _mm_slli_epi32(RGBL, 2);
			//store
			_mm_storeu_si128((__m128i*)&Dst_RGB[i], RGBL);

		}
		Src_RGB += src_stride;
		Dst_RGB += dst_stride;
	}
}
void RGB24_to_GrayScale_2(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels)
{
	//we are using 3 levels of gray : 4, 8, 16
	//we have 3 color channels so we multiply by 3
	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	for( int j = 0; j < height; j++)
	{
		for( int i = 0; i < width*3; i += 3*7 )
		{
			__declspec(align(16)) __m128i R,G,B,RL,GL,BL,RH,GH,BH,RGBL,RGBH;
			//load up 8 values
			R = _mm_loadu_si128((__m128i*)(&Src_RGB[i]));
			G = _mm_srli_si128(R, 1);
			B = _mm_srli_si128(R, 2);
			//unpack
			RL = _mm_unpacklo_epi8(R, zero);
			GL = _mm_unpacklo_epi8(G, zero);
			BL = _mm_unpacklo_epi8(B, zero);
			RH = _mm_unpackhi_epi8(R, zero);
			GH = _mm_unpackhi_epi8(G, zero);
			BH = _mm_unpackhi_epi8(B, zero);
			//add them up
			RGBL = _mm_add_epi16(RL, GL);
			RGBL = _mm_add_epi16(RGBL, BL);
			RGBH = _mm_add_epi16(RH, GH);
			RGBH = _mm_add_epi16(RGBH, BH);
			//divide to get only 3 levels
			RGBL = _mm_srai_epi16(RGBL, 8);
			RGBH = _mm_srai_epi16(RGBH, 8);
			//pack
			RGBL = _mm_unpacklo_epi16(RGBL, RGBH);
			//scale them up as requested
			__declspec(align(16)) __m128i one = _mm_set1_epi16(1);
			RGBL = _mm_add_epi16(RGBL, one);
			RGBL = _mm_slli_epi32(RGBL, 2);
			//store
			_mm_storeu_si128((__m128i*)&Dst_RGB[i], RGBL);

		}
		Src_RGB += src_stride;
		Dst_RGB += dst_stride;
	}
}
//1.99
void RGB24_to_GrayScale_3(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels)
{
	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();

	int multiplier = ( DIV_PRECISION_VALUE * GrayscaleLevels ) / ( 256 * 3 );
	__declspec(align(16)) __m128i InvMull;
	InvMull = _mm_set1_epi16( multiplier );

	__declspec(align(16)) unsigned char c_shufflemask[16] = { 0,3,6,9,12,15,0xF0,0xF0,0xF00,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask;
	shufflemask = _mm_load_si128((__m128i*)c_shufflemask);

	for( int j = 0; j < height; j++)
	{
		for( int i = 0; i < width*3; i += 3*4 )
		{
			__declspec(align(16)) __m128i R,G,B,R1,G1,B1,RGBL,RGBH,temp1,temp2;
			//load up 8 values
			R = _mm_loadu_si128((__m128i*)(&Src_RGB[i]));
			G = _mm_loadu_si128((__m128i*)(&Src_RGB[i+1]));
			B = _mm_loadu_si128((__m128i*)(&Src_RGB[i+2]));
			//filter used values
			R = _mm_shuffle_epi8(R, shufflemask);
			G = _mm_shuffle_epi8(G, shufflemask);
			B = _mm_shuffle_epi8(B, shufflemask);
			//unpack
			R1 = _mm_unpacklo_epi8(R, zero);
			G1 = _mm_unpacklo_epi8(G, zero);
			B1 = _mm_unpacklo_epi8(B, zero);

//			R2 = _mm_unpackhi_epi8(R, zero);
//			G2 = _mm_unpackhi_epi8(G, zero);
//			B2 = _mm_unpackhi_epi8(B, zero);
			//add them up
			RGBL = _mm_add_epi16(R1, G1);
			RGBL = _mm_add_epi16(RGBL, B1);
//			RGBH = _mm_add_epi16(RH, GH);
//			RGBH = _mm_add_epi16(RGBH, BH);
			//divide to get only N levels
			temp1 = _mm_mullo_epi16(RGBL, InvMull);
			temp2 = _mm_mulhi_epi16(RGBL, InvMull);
			RGBL = _mm_unpacklo_epi16(temp1, temp2);
			RGBH = _mm_unpackhi_epi16(temp1, temp2);
			RGBL = _mm_srai_epi32(RGBL, DIV_PRECISSION_BITS);
			RGBH = _mm_srai_epi32(RGBH, DIV_PRECISSION_BITS);
			//pack
			RGBL = _mm_packs_epi32(RGBL, RGBH);
			RGBL = _mm_packus_epi16(RGBL, zero);
			//store
//			_mm_storeu_si128((__m128i*)&Dst_RGB[i/3], RGBL);
			_mm_storel_epi64((__m128i*)&Dst_RGB[i/3], RGBL);

		}
		Src_RGB += src_stride;
		Dst_RGB += dst_stride;
	}
}
//1.79
void RGB24_to_GrayScale_4(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels)
{
	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();

	int multiplier = ( DIV_PRECISION_VALUE * GrayscaleLevels ) / ( 256 * 3 );
	__declspec(align(16)) __m128i InvMull;
	InvMull = _mm_set1_epi16( multiplier );

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,3,6,9,0xF0,0xF0,0xF0,0xF0,0xF00,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask2[16] = { 1,4,7,10,0xF0,0xF0,0xF0,0xF0,0xF00,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask3[16] = { 2,5,8,11,0xF0,0xF0,0xF0,0xF0,0xF00,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask1,shufflemask2,shufflemask3;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);
	shufflemask2 = _mm_load_si128((__m128i*)c_shufflemask2);
	shufflemask3 = _mm_load_si128((__m128i*)c_shufflemask3);

	for( int j = 0; j < height; j++)
	{
		for( int i = 0; i < width*3; i += 3*4 )
		{
			__declspec(align(16)) __m128i RGB,R1,G1,B1,RGBL,RGBH,temp1,temp2;
			//load up 8 values
			RGB = _mm_loadu_si128((__m128i*)(&Src_RGB[i]));
			//filter used values
			R1 = _mm_shuffle_epi8(RGB, shufflemask1);
			G1 = _mm_shuffle_epi8(RGB, shufflemask2);
			B1 = _mm_shuffle_epi8(RGB, shufflemask3);
			//unpack
			R1 = _mm_unpacklo_epi8(R1, zero);
			G1 = _mm_unpacklo_epi8(G1, zero);
			B1 = _mm_unpacklo_epi8(B1, zero);
			//add them up
			RGBL = _mm_add_epi16(R1, G1);
			RGBL = _mm_add_epi16(RGBL, B1);
			//divide to get only N levels
			temp1 = _mm_mullo_epi16(RGBL, InvMull);
			temp2 = _mm_mulhi_epi16(RGBL, InvMull);
			RGBL = _mm_unpacklo_epi16(temp1, temp2);
			RGBH = _mm_unpackhi_epi16(temp1, temp2);
			RGBL = _mm_srai_epi32(RGBL, DIV_PRECISSION_BITS);
			RGBH = _mm_srai_epi32(RGBH, DIV_PRECISSION_BITS);
			//pack
			RGBL = _mm_packs_epi32(RGBL, RGBH);
			RGBL = _mm_packus_epi16(RGBL, zero);
			//store
			_mm_storel_epi64((__m128i*)&Dst_RGB[i/3], RGBL);

		}
		Src_RGB += src_stride;
		Dst_RGB += dst_stride;
	}
}
//1.38
void RGB24_to_GrayScale_5(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels)
{
	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();

	int multiplier = ( DIV_PRECISION_VALUE * GrayscaleLevels ) / ( 256 * 3 );
	__declspec(align(16)) __m128i InvMull;
	InvMull = _mm_set1_epi16( multiplier );

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,3,6,9,0xF0,0xF0,0xF0,0xF0,0xF00,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask2[16] = { 1,4,7,10,0xF0,0xF0,0xF0,0xF0,0xF00,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask3[16] = { 2,5,8,11,0xF0,0xF0,0xF0,0xF0,0xF00,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask1,shufflemask2,shufflemask3;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);
	shufflemask2 = _mm_load_si128((__m128i*)c_shufflemask2);
	shufflemask3 = _mm_load_si128((__m128i*)c_shufflemask3);

	for( int j = 0; j < height; j++)
	{
		for( int i = 0; i < width*3; i += 3*8 )
		{
			__declspec(align(16)) __m128i RGB1,RGB2,R1,G1,B1,RGBL,RGBH,temp1,temp2,R2,G2,B2,RGBL2,RGBH2;
			//load up 8 values
			RGB1 = _mm_loadu_si128((__m128i*)(&Src_RGB[i]));
			RGB2 = _mm_loadu_si128((__m128i*)(&Src_RGB[i+12]));
			//filter used values
			R1 = _mm_shuffle_epi8(RGB1, shufflemask1);
			G1 = _mm_shuffle_epi8(RGB1, shufflemask2);
			B1 = _mm_shuffle_epi8(RGB1, shufflemask3);
			R2 = _mm_shuffle_epi8(RGB2, shufflemask1);
			G2 = _mm_shuffle_epi8(RGB2, shufflemask2);
			B2 = _mm_shuffle_epi8(RGB2, shufflemask3);
			//unpack
			R1 = _mm_unpacklo_epi8(R1, zero);
			G1 = _mm_unpacklo_epi8(G1, zero);
			B1 = _mm_unpacklo_epi8(B1, zero);
			R2 = _mm_unpacklo_epi8(R2, zero);
			G2 = _mm_unpacklo_epi8(G2, zero);
			B2 = _mm_unpacklo_epi8(B2, zero);
			//add them up
			RGBL = _mm_add_epi16(R1, G1);
			RGBL2 = _mm_add_epi16(R2, G2);
			RGBL = _mm_add_epi16(RGBL, B1);
			RGBL2 = _mm_add_epi16(RGBL2, B2);
			//divide to get only N levels
			temp1 = _mm_mullo_epi16(RGBL, InvMull);
			temp2 = _mm_mulhi_epi16(RGBL, InvMull);
			RGBL = _mm_unpacklo_epi16(temp1, temp2);
			RGBH = _mm_unpackhi_epi16(temp1, temp2);
			RGBL = _mm_srai_epi32(RGBL, DIV_PRECISSION_BITS);
			RGBH = _mm_srai_epi32(RGBH, DIV_PRECISSION_BITS);

			temp1 = _mm_mullo_epi16(RGBL2, InvMull);
			temp2 = _mm_mulhi_epi16(RGBL2, InvMull);
			RGBL2 = _mm_unpacklo_epi16(temp1, temp2);
			RGBH2 = _mm_unpackhi_epi16(temp1, temp2);
			RGBL2 = _mm_srai_epi32(RGBL2, DIV_PRECISSION_BITS);
			RGBH2 = _mm_srai_epi32(RGBH2, DIV_PRECISSION_BITS);
			//pack
			RGBL = _mm_packs_epi32(RGBL, RGBL2);
			RGBL = _mm_packus_epi16(RGBL, zero);
			//store
			_mm_storel_epi64((__m128i*)&Dst_RGB[i/3], RGBL);

		}
		Src_RGB += src_stride;
		Dst_RGB += dst_stride;
	}
}
//1.373
void RGB24_to_GrayScale_6(unsigned char *Src_RGB, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride,int GrayscaleLevels)
{
	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();

	int multiplier = ( DIV_PRECISION_VALUE * GrayscaleLevels ) / ( 256 * 3 );
	__declspec(align(16)) __m128i InvMull;
	InvMull = _mm_set1_epi16( multiplier );

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,3,6,9,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask2[16] = { 1,4,7,10,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask3[16] = { 2,5,8,11,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask4[16] = { 0,0,0,1,1,1,2,2,2,3,3,3,4,4,4,5 };
	__declspec(align(16)) unsigned char c_shufflemask5[16] = { 5,5,6,6,6,7,7,7,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask1,shufflemask2,shufflemask3;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);
	shufflemask2 = _mm_load_si128((__m128i*)c_shufflemask2);
	shufflemask3 = _mm_load_si128((__m128i*)c_shufflemask3);

	int SSEWidth = ( width * 3 ) & ~0x07;
	for( int j = 0; j < height; j++)
	{
		int i;
		for( i = 0; i < SSEWidth; i += 3*8 )
		{
			__declspec(align(16)) __m128i RGB1,RGB2,R1,G1,B1,RGBL,RGBH,temp1,temp2,R2,G2,B2,RGBL2,RGBH2;
			//load up 8 values
			RGB1 = _mm_loadu_si128((__m128i*)(&Src_RGB[i]));
			RGB2 = _mm_loadu_si128((__m128i*)(&Src_RGB[i+12]));
			//filter used values
			R1 = _mm_shuffle_epi8(RGB1, shufflemask1);
			G1 = _mm_shuffle_epi8(RGB1, shufflemask2);
			B1 = _mm_shuffle_epi8(RGB1, shufflemask3);
			R2 = _mm_shuffle_epi8(RGB2, shufflemask1);
			G2 = _mm_shuffle_epi8(RGB2, shufflemask2);
			B2 = _mm_shuffle_epi8(RGB2, shufflemask3);
			//unpack
			R1 = _mm_unpacklo_epi8(R1, zero);
			G1 = _mm_unpacklo_epi8(G1, zero);
			B1 = _mm_unpacklo_epi8(B1, zero);
			R2 = _mm_unpacklo_epi8(R2, zero);
			G2 = _mm_unpacklo_epi8(G2, zero);
			B2 = _mm_unpacklo_epi8(B2, zero);
			//add them up
			RGBL = _mm_add_epi16(R1, G1);
			RGBL2 = _mm_add_epi16(R2, G2);
			RGBL = _mm_add_epi16(RGBL, B1);
			RGBL2 = _mm_add_epi16(RGBL2, B2);
			//divide to get only N levels
			temp1 = _mm_mullo_epi16(RGBL, InvMull);
			temp2 = _mm_mulhi_epi16(RGBL, InvMull);
			RGBL = _mm_unpacklo_epi16(temp1, temp2);
			RGBH = _mm_unpackhi_epi16(temp1, temp2);
			RGBL = _mm_srai_epi32(RGBL, DIV_PRECISSION_BITS);
			RGBH = _mm_srai_epi32(RGBH, DIV_PRECISSION_BITS);

			temp1 = _mm_mullo_epi16(RGBL2, InvMull);
			temp2 = _mm_mulhi_epi16(RGBL2, InvMull);
			RGBL2 = _mm_unpacklo_epi16(temp1, temp2);
			RGBH2 = _mm_unpackhi_epi16(temp1, temp2);
			RGBL2 = _mm_srai_epi32(RGBL2, DIV_PRECISSION_BITS);
			RGBH2 = _mm_srai_epi32(RGBH2, DIV_PRECISSION_BITS);
			//pack to smaller type
			RGBL = _mm_packs_epi32(RGBL, RGBL2);
			//scale up again
			__declspec(align(16)) __m128i Mull;
			Mull = _mm_set1_epi16( GrayscaleLevels );
			temp1 = _mm_mullo_epi16(RGBL, Mull);
			RGBL = _mm_packus_epi16(temp1, zero);
			__declspec(align(16)) __m128i bloatmask1,bloatmask2;
			bloatmask1 = _mm_load_si128((__m128i*)c_shufflemask4);
			bloatmask2 = _mm_load_si128((__m128i*)c_shufflemask5);
			R1 = _mm_shuffle_epi8(RGBL, bloatmask1);
			G1 = _mm_shuffle_epi8(RGBL, bloatmask2);
			//store
			_mm_storeu_si128((__m128i*)&Dst_RGB[i+0], R1);
			_mm_storel_epi64((__m128i*)&Dst_RGB[i+16], G1);
		}
		for( ; i < width*3; i += 3 )
		{
			int R,G,B;
			R = Src_RGB[i+0];
			G = Src_RGB[i+1];
			B = Src_RGB[i+2];
			int res = (R+G+B)*multiplier;
			res = res >> DIV_PRECISSION_BITS;
			res = res * GrayscaleLevels;
			Dst_RGB[i+0] = res;
			Dst_RGB[i+1] = res;
			Dst_RGB[i+2] = res;
		}
		Src_RGB += src_stride;
		Dst_RGB += dst_stride;
	}
}
//4.88
float CompareRGBRegions_reference(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels )
{
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	int PixelMismatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	for( int j = 0; j < BoxHeight; j++)
		for( int i = 0; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ j*src_stride + i + 0 ] != Ref_RGB[ j*src_stride + i + 0 ]
				|| Src_RGB[ j*src_stride + i + 1 ] != Ref_RGB[ j*src_stride + i + 1 ]
				|| Src_RGB[ j*src_stride + i + 2 ] != Ref_RGB[ j*src_stride + i + 2 ] )
				{
					PixelMismatches++;
				}
			else if( HighlightPixels )
				Src_RGB[ j * src_stride + i + 0 ] = 255;
	return ((float)PixelMismatches/(float)PixelsToInvestigate);
}
//4.24-2.34
float CompareRGBRegions_1(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels )
{
	int PixelMatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,0xF0,3,0xF0,6,0xF0,9,0xF0,12,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask2[16] = { 0,0xF0,0xF0,3,0xF0,0xF0,6,0xF0,0xF0,9,0xF0,0xF0,12,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask1,shufflemask2;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);
	shufflemask2 = _mm_load_si128((__m128i*)c_shufflemask2);

	int SSEWidth = ( ( BoxWidth / 5) * 5 ) * 3;
	for( int j = 0; j < BoxHeight; j++)
	{
		int i;
		__declspec(align(16)) __m128i SumDiff;
		SumDiff = _mm_setzero_si128();
		for( i = 0; i < SSEWidth; i += 3*5 )
		{
			__declspec(align(16)) __m128i SR,SG,SB,RR,RG,RB,T,T1;
			//load up 8 values from sec
			SR = _mm_loadu_si128((__m128i*)(&Src_RGB[i+0]));
			SG = _mm_loadu_si128((__m128i*)(&Src_RGB[i+1]));
			SB = _mm_loadu_si128((__m128i*)(&Src_RGB[i+2]));
			//load up 8 values from ref
			RR = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+0]));
			RG = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+1]));
			RB = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+2]));

			//0xFF - match, 0x00 - no match
			T = _mm_cmpeq_epi8(SR,RR);
			SG = _mm_cmpeq_epi8(SG,RG);
			SB = _mm_cmpeq_epi8(SB,RB);

			//see if all 3 match : 0xFF - match, 0x00 - no match
			T = _mm_and_si128(T,SG);
			T = _mm_and_si128(T,SB);

			//we are interested in every 3rd value only
			T1 = _mm_shuffle_epi8(T, shufflemask1);
			if( HighlightPixels )
			{
				T = _mm_shuffle_epi8(T, shufflemask2);
				SR = _mm_or_si128( SR, T );
				_mm_storeu_si128((__m128i*)(&Src_RGB[i+0]), SR);
			}

			//1 bit if match
			T = _mm_set1_epi8( 1 );
			T = _mm_and_si128(T, T1);
			SumDiff = _mm_add_epi16(T, SumDiff);
		}
		for( ; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ i + 0 ] == Ref_RGB[ i + 0 ]
				&& Src_RGB[ i + 1 ] == Ref_RGB[ i + 1 ]
				&& Src_RGB[ i + 2 ] == Ref_RGB[ i + 2 ] )
				{
					PixelMatches++;
					if( HighlightPixels )
						Src_RGB[ i + 0 ] = 255;
				}
		Src_RGB += src_stride;
		Ref_RGB += src_stride;
		__declspec(align(16)) unsigned short tstore[16];
		_mm_storeu_si128((__m128i*)(tstore), SumDiff);
		PixelMatches += tstore[0]+tstore[1]+tstore[2]+tstore[3]+tstore[4];
	}
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	return ((float)(PixelsToInvestigate-PixelMatches)/(float)PixelsToInvestigate);
}
//3.19-2.34
float CompareRGBRegions_2(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels )
{
	int PixelMatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,0xF0,3,0xF0,6,0xF0,9,0xF0,12,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask1;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);

	int SSEWidth = ( ( BoxWidth / 5) * 5 ) * 3;
	for( int j = 0; j < BoxHeight; j++)
	{
		int i;
		__declspec(align(16)) __m128i SumDiff;
		SumDiff = _mm_setzero_si128();
		for( i = 0; i < SSEWidth; i += 3*5 )
		{
			__declspec(align(16)) __m128i SR,SG,SB,RR,RG,RB,T,T1;
			//load up 8 values from sec
			SR = _mm_loadu_si128((__m128i*)(&Src_RGB[i+0]));
			SG = _mm_loadu_si128((__m128i*)(&Src_RGB[i+1]));
			SB = _mm_loadu_si128((__m128i*)(&Src_RGB[i+2]));
			//load up 8 values from ref
			RR = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+0]));
			RG = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+1]));
			RB = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+2]));

			//0xFF - match, 0x00 - no match
			T = _mm_cmpeq_epi8(SR,RR);
			SG = _mm_cmpeq_epi8(SG,RG);
			SB = _mm_cmpeq_epi8(SB,RB);

			//see if all 3 match : 0xFF - match, 0x00 - no match
			T = _mm_and_si128(T,SG);
			T = _mm_and_si128(T,SB);

			//we are interested in every 3rd value only
			T1 = _mm_shuffle_epi8(T, shufflemask1);
			if( HighlightPixels )
			{
				if( T.m128i_i8[0] )
					Src_RGB[i+0] = 255;
				if( T.m128i_i8[3] )
					Src_RGB[i+3] = 255;
				if( T.m128i_i8[6] )
					Src_RGB[i+6] = 255;
				if( T.m128i_i8[9] )
					Src_RGB[i+9] = 255;
				if( T.m128i_i8[12] )
					Src_RGB[i+12] = 255;
			}

			//1 bit if match
			T = _mm_set1_epi8( 1 );
			T = _mm_and_si128(T, T1);
			SumDiff = _mm_add_epi16(T, SumDiff);
		}
		for( ; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ i + 0 ] == Ref_RGB[ i + 0 ]
				&& Src_RGB[ i + 1 ] == Ref_RGB[ i + 1 ]
				&& Src_RGB[ i + 2 ] == Ref_RGB[ i + 2 ] )
				{
					PixelMatches++;
					if( HighlightPixels )
						Src_RGB[ i + 0 ] = 255;
				}
		Src_RGB += src_stride;
		Ref_RGB += src_stride;
		__declspec(align(16)) unsigned short tstore[16];
		_mm_storeu_si128((__m128i*)(tstore), SumDiff);
		PixelMatches += tstore[0]+tstore[1]+tstore[2]+tstore[3]+tstore[4];
	}
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	return ((float)(PixelsToInvestigate-PixelMatches)/(float)PixelsToInvestigate);
}
//3.22-2.32
float CompareRGBRegions_3(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels )
{
	int PixelMatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,0xF0,3,0xF0,6,0xF0,9,0xF0,12,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) unsigned char c_shufflemask2[16] = { 0,0xF0,0xF0,3,0xF0,0xF0,6,0xF0,0xF0,9,0xF0,0xF0,12,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask1,shufflemask2;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);
	shufflemask2 = _mm_load_si128((__m128i*)c_shufflemask2);

	int SSEWidth = ( BoxWidth & ( ~0x05 ) ) * 3;
	for( int j = 0; j < BoxHeight; j++)
	{
		int i;
		__declspec(align(16)) __m128i SumDiff;
		SumDiff = _mm_setzero_si128();
		for( i = 0; i < SSEWidth; i += 3*5 )
		{
			__declspec(align(16)) __m128i SR,SG,SB,RR,RG,RB,T,T1;
			//load up 8 values from sec
			SR = _mm_loadu_si128((__m128i*)(&Src_RGB[i+0]));
			SG = _mm_loadu_si128((__m128i*)(&Src_RGB[i+1]));
			SB = _mm_loadu_si128((__m128i*)(&Src_RGB[i+2]));
			//load up 8 values from ref
			RR = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+0]));
			RG = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+1]));
			RB = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+2]));

			//0xFF - match, 0x00 - no match
			T = _mm_cmpeq_epi8(SR,RR);
			SG = _mm_cmpeq_epi8(SG,RG);
			SB = _mm_cmpeq_epi8(SB,RB);

			//see if all 3 match : 0xFF - match, 0x00 - no match
			T = _mm_and_si128(T,SG);
			T = _mm_and_si128(T,SB);

			//we are interested in every 3rd value only
			T1 = _mm_shuffle_epi8(T, shufflemask2);
			if( HighlightPixels )
			{
				__declspec(align(16)) unsigned char tstore[16];
				_mm_storeu_si128((__m128i*)(tstore), T1);
				int *t1=(int *)tstore;
				int *t2=(int *)Src_RGB;
				t2[0] = t2[0] | t1[0];
				t2[1] = t2[1] | t1[1];
				t2[2] = t2[2] | t1[2];
			}

			//1 bit if match
			T = _mm_set1_epi8( 1 );
			T = _mm_and_si128(T, T1);
			SumDiff = _mm_add_epi16(T, SumDiff);
		}
		for( ; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ i + 0 ] == Ref_RGB[ i + 0 ] && Src_RGB[ i + 1 ] == Ref_RGB[ i + 1 ]
				&& Src_RGB[ i + 2 ] == Ref_RGB[ i + 2 ] )
				{
					PixelMatches++;
					if( HighlightPixels )
						Src_RGB[ i + 0 ] = 255;
				}
		Src_RGB += src_stride;
		Ref_RGB += src_stride;
		__declspec(align(16)) unsigned short tstore[16];
		_mm_storeu_si128((__m128i*)(tstore), SumDiff);
		PixelMatches += tstore[0]+tstore[1]+tstore[2]+tstore[3]+tstore[4];
	}
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	return ((float)(PixelsToInvestigate-PixelMatches)/(float)PixelsToInvestigate);
}
//3.88-4.99
float CompareRGBRegions_4(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels )
{
	int PixelMatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	int SSEWidth = ( BoxWidth & ( ~0x05 ) ) * 3;
	for( int j = 0; j < BoxHeight; j++)
	{
		int i;
		for( i = 0; i < SSEWidth; i += 3*5 )
		{
			__declspec(align(16)) __m128i SR,RR,T;
			//load up 8 values from sec
			SR = _mm_loadu_si128((__m128i*)(&Src_RGB[i+0]));
			//load up 8 values from ref
			RR = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+0]));

			//0xFF - match, 0x00 - no match
			T = _mm_cmpeq_epi8(SR,RR);

			__declspec(align(16)) unsigned char tstore[16];
			_mm_storeu_si128((__m128i*)(tstore), T);

			if(( *(int*)&tstore[0] & 0x00FFFFFF ) == 0x00FFFFFF )
			{
				PixelMatches++;
				if( HighlightPixels )
					Src_RGB[ i + 0 ] = 255;
			}
			if(( *(int*)&tstore[3] & 0x00FFFFFF ) == 0x00FFFFFF )
			{
				PixelMatches++;
				if( HighlightPixels )
					Src_RGB[ i + 3 ] = 255;
			}
			if(( *(int*)&tstore[6] & 0x00FFFFFF ) == 0x00FFFFFF )
			{
				PixelMatches++;
				if( HighlightPixels )
					Src_RGB[ i + 6 ] = 255;
			}
			if(( *(int*)&tstore[9] & 0x00FFFFFF ) == 0x00FFFFFF )
			{
				PixelMatches++;
				if( HighlightPixels )
					Src_RGB[ i + 9 ] = 255;
			}
			if(( *(int*)&tstore[12] & 0x00FFFFFF ) == 0x00FFFFFF )
			{
				PixelMatches++;
				if( HighlightPixels )
					Src_RGB[ i + 12 ] = 255;
			}
		}
		for( ; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ i + 0 ] == Ref_RGB[ i + 0 ]
				&& Src_RGB[ i + 1 ] == Ref_RGB[ i + 1 ]
				&& Src_RGB[ i + 2 ] == Ref_RGB[ i + 2 ] )
				{
					PixelMatches++;
					if( HighlightPixels )
						Src_RGB[ i + 0 ] = 255;
				}
		Src_RGB += src_stride;
		Ref_RGB += src_stride;
	}
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	return ((float)(PixelsToInvestigate-PixelMatches)/(float)PixelsToInvestigate);
}





void IYUV_to_BGR24_reference(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{

	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	for( j = 0; j < height; j++)
	{
		for( i=0; i < width; i++)
		{
			int Y, U, V, R, G, B;
			int iX3 = i+i+i;
			Y = py[j*src_stride + i];
			U = pu[(j/2)*(src_stride/2) + i/2];
			V = pv[(j/2)*(src_stride/2) + i/2];
			R = (300*(Y-16) + 517*(U-128) + 128 ) / 256;
			G = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) / 256;
			B = (300*(Y-16) + 409*(V-128) + 128 ) / 256;
			Dst_RGB[j*dst_stride + iX3 + 0] = CLIP3(0, 255, B);
			Dst_RGB[j*dst_stride + iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[j*dst_stride + iX3 + 2] = CLIP3(0, 255, R);
		}
	}
}

void IYUV_to_BGR24_1(unsigned char *Src_IYUV, int width, int height, int src_stride, unsigned char *Dst_RGB, int dst_stride)
{
	unsigned char *py = Src_IYUV;
	unsigned char *pu = py + height * src_stride;
	unsigned char *pv = pu + ( height / 2 ) * ( src_stride / 2 );
	int i,j;

	__declspec(align(16)) __m128i zero;
	zero = _mm_setzero_si128();
	__declspec(align(16)) char c_shufflemask[16] = { 0,1,2,4,5,6,8,9,10,12,13,14 };
	int SSEHeight = height & (~0x01);
	int SSEWidth = (width-8) & (~0x07); //write 32 bytes instead 24(12+12) leads to bad output
	for( j = 0; j < SSEHeight; j+=2)
	{
		for( i = 0; i < SSEWidth; i+=8)
		{

			int iX3 = i*3;
			__declspec(align(16)) __m128i sY, sY2, sU, sV, temp0, temp1, temp2, AccLo, AccHi;
			__declspec(align(16)) __m128i sY300Lo, sY300Hi, sY300Lo2, sY300Hi2;
			__declspec(align(16)) __m128i sU517Lo, sU517Hi, sU100Lo, sU100Hi, sV208Lo, sV208Hi, sV409Lo, sV409Hi, sUVLo, sUVHi;
			__declspec(align(16)) __m128i R[4],G[4],B[4];
			__declspec(align(16)) __m128i Const16;
			__declspec(align(16)) __m128i Const128;

			sY = _mm_loadl_epi64((__m128i*)(&py[i]));
			sY2 = _mm_loadl_epi64((__m128i*)(&py[i+src_stride]));
			sU = _mm_loadl_epi64((__m128i*)(&pu[i>>1]));
			sV = _mm_loadl_epi64((__m128i*)(&pv[i>>1]));

			Const16 = _mm_set1_epi16(16);
			Const128 = _mm_set1_epi16(128);
			temp0 = _mm_set1_epi16(300);

			sY = _mm_unpacklo_epi8(sY, zero);
			sY2 = _mm_unpacklo_epi8(sY2, zero);
			sU = _mm_unpacklo_epi8(sU, zero);
			sV = _mm_unpacklo_epi8(sV, zero);
			sU = _mm_unpacklo_epi16(sU, sU);
			sV = _mm_unpacklo_epi16(sV, sV);
			
			sY = _mm_sub_epi16(sY, Const16);
			sY2 = _mm_sub_epi16(sY2, Const16);
			sU = _mm_sub_epi16(sU, Const128);
			sV = _mm_sub_epi16(sV, Const128);

			temp1 = _mm_mullo_epi16(sY, temp0);
			temp2 = _mm_mulhi_epi16(sY, temp0);
			sY300Lo = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp1 = _mm_mullo_epi16(sY2, temp0);
			temp2 = _mm_mulhi_epi16(sY2, temp0);
			sY300Lo2 = _mm_unpacklo_epi16(temp1, temp2);
			sY300Hi2 = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi32(128);
			sY300Lo = _mm_add_epi32(sY300Lo, temp0);
			sY300Hi = _mm_add_epi32(sY300Hi, temp0);
			sY300Lo2 = _mm_add_epi32(sY300Lo2, temp0);
			sY300Hi2 = _mm_add_epi32(sY300Hi2, temp0);

			temp0 = _mm_set1_epi16(517);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU517Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU517Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(100);
			temp1 = _mm_mullo_epi16(sU, temp0);
			temp2 = _mm_mulhi_epi16(sU, temp0);
			sU100Lo = _mm_unpacklo_epi16(temp1, temp2);
			sU100Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(208);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV208Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV208Hi = _mm_unpackhi_epi16(temp1, temp2);

			temp0 = _mm_set1_epi16(409);
			temp1 = _mm_mullo_epi16(sV, temp0);
			temp2 = _mm_mulhi_epi16(sV, temp0);
			sV409Lo = _mm_unpacklo_epi16(temp1, temp2);
			sV409Hi = _mm_unpackhi_epi16(temp1, temp2);

			sUVLo = _mm_add_epi32(sV208Lo, sU100Lo);
			sUVHi = _mm_add_epi32(sV208Hi, sU100Hi);

			//R
			AccLo = _mm_add_epi32(sY300Lo, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sU517Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sU517Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			R[1] = _mm_packus_epi16(AccLo, zero);

			//G
			AccLo = _mm_sub_epi32(sY300Lo, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_sub_epi32(sY300Lo2, sUVLo);
			AccHi = _mm_sub_epi32(sY300Hi2, sUVHi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			G[1] = _mm_packus_epi16(AccLo, zero);

			//B
			AccLo = _mm_add_epi32(sY300Lo, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[0] = _mm_packus_epi16(AccLo, zero);

			AccLo = _mm_add_epi32(sY300Lo2, sV409Lo);
			AccHi = _mm_add_epi32(sY300Hi2, sV409Hi);

			AccLo = _mm_srai_epi32(AccLo, 8);
			AccHi = _mm_srai_epi32(AccHi, 8);
			AccLo = _mm_packs_epi32(AccLo, AccHi);
			B[1] = _mm_packus_epi16(AccLo, zero);

			B[2] = _mm_unpacklo_epi8(B[0], G[0]); //R is swapped with B !
			B[3] = _mm_unpacklo_epi8(B[1], G[1]); //R is swapped with B !
			R[2] = _mm_unpacklo_epi8(R[0], zero);
			R[3] = _mm_unpacklo_epi8(R[1], zero); //R is swapped with B !
			__declspec(align(16)) __m128i RGB32Lo[2],RGB32Hi[2];
			RGB32Lo[0] = _mm_unpacklo_epi16(B[2], R[2]);
			RGB32Hi[0] = _mm_unpackhi_epi16(B[2], R[2]);
			RGB32Lo[1] = _mm_unpacklo_epi16(B[3], R[3]);
			RGB32Hi[1] = _mm_unpackhi_epi16(B[3], R[3]);
			__declspec(align(16)) __m128i shufflemask;
			shufflemask = _mm_load_si128((__m128i*)c_shufflemask);
			__declspec(align(16)) __m128i pix1234,pix5678,pix1234_,pix5678_;
			pix1234 = _mm_shuffle_epi8(RGB32Lo[0], shufflemask);
			pix5678 = _mm_shuffle_epi8(RGB32Hi[0], shufflemask);
			pix1234_ = _mm_shuffle_epi8(RGB32Lo[1], shufflemask);
			pix5678_ = _mm_shuffle_epi8(RGB32Hi[1], shufflemask);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+0], pix1234);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+12], pix5678);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+dst_stride+0], pix1234_);
			_mm_storeu_si128((__m128i*)&Dst_RGB[iX3+dst_stride+12], pix5678_);
		}
		for( ;i < width; i++)
		{
			int Y, Y2, U, V, R, G, B, R2, G2, B2;
			int iX3 = i+i+i;
			Y = py[i];
			Y2 = py[i+src_stride];
			U = pu[i/2];
			V = pv[i/2];
			int T1 = 517*(U-128) + 128;
			int T2 = -208*(V-128) - 100*(U-128) + 128;
			int T3 = 409*(V-128) + 128;
			R = (300*(Y-16) + T1 ) / 256;
			G = (300*(Y-16) + T2 ) / 256;
			B = (300*(Y-16) + T3 ) / 256;
			R2 = (300*(Y2-16) + T1 ) / 256;
			G2 = (300*(Y2-16) + T2 ) / 256;
			B2 = (300*(Y2-16) + T3 ) / 256;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, B);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, R);
			Dst_RGB[dst_stride + iX3 + 0] = CLIP3(0, 255, B2);
			Dst_RGB[dst_stride + iX3 + 1] = CLIP3(0, 255, G2);
			Dst_RGB[dst_stride + iX3 + 2] = CLIP3(0, 255, R2);
		}/**/
		py += (src_stride*2);
		pu += (src_stride/2);
		pv += (src_stride/2);
		Dst_RGB += (dst_stride*2);
	}
	if( height & 0x01 )
	{
		for( i=0; i < width; i++)
		{
			int Y, U, V, R, G, B;
			int iX3 = i+i+i;
			Y = py[i];
			U = pu[i/2];
			V = pv[i/2];
			R = (300*(Y-16) + 517*(U-128) + 128 ) / 256;
			G = (300*(Y-16) - 208*(V-128) - 100*(U-128) + 128 ) / 256;
			B = (300*(Y-16) + 409*(V-128) + 128 ) / 256;
			Dst_RGB[iX3 + 0] = CLIP3(0, 255, B);
			Dst_RGB[iX3 + 1] = CLIP3(0, 255, G);
			Dst_RGB[iX3 + 2] = CLIP3(0, 255, R);
		}
	}/**/
}

float CompareBGRRegions_reference(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels )
{
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	int PixelMismatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	for( int j = 0; j < BoxHeight; j++)
		for( int i = 0; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ j*src_stride + i + 0 ] != Ref_RGB[ j*src_stride + i + 0 ]
				|| Src_RGB[ j*src_stride + i + 1 ] != Ref_RGB[ j*src_stride + i + 1 ]
				|| Src_RGB[ j*src_stride + i + 2 ] != Ref_RGB[ j*src_stride + i + 2 ] )
				{
					PixelMismatches++;
				}
			else if( HighlightPixels )
				Src_RGB[ j * src_stride + i + 2 ] = 255;
	return ((float)PixelMismatches/(float)PixelsToInvestigate);
}

float CompareBGRRegions_1(unsigned char *Src_RGB, int width, int height, int src_stride, int Startx, int Starty, int BoxWidth, int BoxHeight, unsigned char *Ref_RGB, char HighlightPixels )
{
	int PixelMatches = 0;

	Src_RGB = Src_RGB + Starty * src_stride + Startx * 3;
	Ref_RGB = Ref_RGB + Starty * src_stride + Startx * 3;

	__declspec(align(16)) unsigned char c_shufflemask1[16] = { 0,0xF0,3,0xF0,6,0xF0,9,0xF0,12,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0 };
	__declspec(align(16)) __m128i shufflemask1;
	shufflemask1 = _mm_load_si128((__m128i*)c_shufflemask1);

	int SSEWidth = ( ( BoxWidth / 5) * 5 ) * 3;
	for( int j = 0; j < BoxHeight; j++)
	{
		int i;
		__declspec(align(16)) __m128i SumDiff;
		SumDiff = _mm_setzero_si128();
		for( i = 0; i < SSEWidth; i += 3*5 )
		{
			__declspec(align(16)) __m128i SR,SG,SB,RR,RG,RB,T,T1;
			//load up 8 values from sec
			SB = _mm_loadu_si128((__m128i*)(&Src_RGB[i+0]));
			SG = _mm_loadu_si128((__m128i*)(&Src_RGB[i+1]));
			SR = _mm_loadu_si128((__m128i*)(&Src_RGB[i+2]));
			//load up 8 values from ref
			RB = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+0]));
			RG = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+1]));
			RR = _mm_loadu_si128((__m128i*)(&Ref_RGB[i+2]));

			//0xFF - match, 0x00 - no match
			T = _mm_cmpeq_epi8(SR,RR);
			SG = _mm_cmpeq_epi8(SG,RG);
			SB = _mm_cmpeq_epi8(SB,RB);

			//see if all 3 match : 0xFF - match, 0x00 - no match
			T = _mm_and_si128(T,SG);
			T = _mm_and_si128(T,SB);

			//we are interested in every 3rd value only
			T1 = _mm_shuffle_epi8(T, shufflemask1);
			if( HighlightPixels )
			{
				if( T.m128i_i8[2] )
					Src_RGB[i+2] = 255;
				if( T.m128i_i8[5] )
					Src_RGB[i+5] = 255;
				if( T.m128i_i8[8] )
					Src_RGB[i+8] = 255;
				if( T.m128i_i8[11] )
					Src_RGB[i+11] = 255;
				if( T.m128i_i8[14] )
					Src_RGB[i+14] = 255;
			}

			//1 bit if match
			T = _mm_set1_epi8( 1 );
			T = _mm_and_si128(T, T1);
			SumDiff = _mm_add_epi16(T, SumDiff);
		}
		for( ; i < BoxWidth * 3; i+=3 )
			if( Src_RGB[ i + 0 ] == Ref_RGB[ i + 0 ]
				&& Src_RGB[ i + 1 ] == Ref_RGB[ i + 1 ]
				&& Src_RGB[ i + 2 ] == Ref_RGB[ i + 2 ] )
				{
					PixelMatches++;
					if( HighlightPixels )
						Src_RGB[ i + 2 ] = 255;
				}
		Src_RGB += src_stride;
		Ref_RGB += src_stride;
		__declspec(align(16)) unsigned short tstore[16];
		_mm_storeu_si128((__m128i*)(tstore), SumDiff);
		PixelMatches += tstore[0]+tstore[1]+tstore[2]+tstore[3]+tstore[4];
	}
	int PixelsToInvestigate = BoxWidth * BoxHeight;
	return ((float)(PixelsToInvestigate-PixelMatches)/(float)PixelsToInvestigate);
}