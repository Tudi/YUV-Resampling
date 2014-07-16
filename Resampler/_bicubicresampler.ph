typedef struct 
{
	int index;
	int coeff[5];
}T_TABLE_DATA;

T_TABLE_DATA *m_pWidthTable;
T_TABLE_DATA *m_pHeightTable;
BYTE clip_tab[3][2048];	// clip_tab[0] 0-255 clip_tab[1] clipped range for luma clip_tab[2] cliplled range for chrmoa
int m_sw,m_sh,m_dw,m_dh;

void Resample_H(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride);
void Resample_V(PBYTE pSource, int nSourceStride, PBYTE pDest, int nDestStride, int clipindex);
BYTE *m_pInter;
UINT m_InterStride;
