
BYTE clip_tab[3][2048];	// clip_tab[0] 0-255 clip_tab[1] clipped range for luma clip_tab[2] cliplled range for chrmoa

int m_sw,m_sh,m_dw,m_dh;

int m_w_inc;
int m_h_inc;
double m_x_ratio;
double m_y_ratio;
int m_precise;
void SetupLowpassKernels();

typedef enum {BILINEAR,BILINEAR_3,BILINEAR_5,BILINEAR_7 }BILINEAR_ACTION;

BILINEAR_ACTION m_action;
unsigned short m_LowpassKernel_3[3][8];
unsigned short m_LowpassKernel_5[3][8];
unsigned short m_LowpassKernel_7[4][8];

void resample_bilinear(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex=0);
void resample_lowpass_3x3(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex=0);
void resample_lowpass_5x5(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex=0);
void resample_lowpass_7x7(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex=0);


void resample_bilinear_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex=0);
void resample_lowpass_3x3_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex=0);
void resample_lowpass_5x5_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex=0);
void resample_lowpass_7x7_24(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex=0);

//Hoi Ming YUV resizer
void resample_bilinear_yuv420(PBYTE pSourceY, PBYTE pSourceU, PBYTE pSourceV, int nSourceStride, 
PBYTE pDestY, PBYTE pDestU, PBYTE pDestV, int nDestStride, int clipindex=0);
void resample_bilinear_yuv420_downsample(PBYTE pSourceY, PBYTE pSourceU, PBYTE pSourceV, int nSourceStride, 
PBYTE pDestY, PBYTE pDestU, PBYTE pDestV, int nDestStride, int clipindex=0);
void LowPassFilter_SingleColour_2_2(PBYTE pSource, int nSourceStride, int sw, int sh);
void LowPassFilter_SingleColour_1_1(PBYTE pSource, int nSourceStride, int sw, int sh);

void LowPassFilter_2_2(PBYTE pSource, int nSourceStride);
void LowPassFilter_1_1(PBYTE pSource, int nSourceStride);

/*void CBilinearResampler::LowPassFilter_SingleColour_2_2(PBYTE pSource, int nSourceStride, int sw, int sh);
void CBilinearResampler::LowPassFilter_SingleColour_1_1(PBYTE pSource, int nSourceStride, int sw, int sh);


void CBilinearResampler::LowPassFilter_2_2(PBYTE pSource, int nSourceStride);
void CBilinearResampler::LowPassFilter_2_1(PBYTE pSource, int nSourceStride);
void CBilinearResampler::LowPassFilter_2_0(PBYTE pSource, int nSourceStride);
void CBilinearResampler::LowPassFilter_1_2(PBYTE pSource, int nSourceStride);
void CBilinearResampler::LowPassFilter_1_1(PBYTE pSource, int nSourceStride);
void CBilinearResampler::LowPassFilter_1_0(PBYTE pSource, int nSourceStride);
void CBilinearResampler::LowPassFilter_0_2(PBYTE pSource, int nSourceStride);
void CBilinearResampler::LowPassFilter_0_1(PBYTE pSource, int nSourceStride);*/
	


