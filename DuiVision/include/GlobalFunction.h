#pragma once

// 加载图片
BOOL LoadImage(const CString strPathFile, CBitmap &bitmap, CSize &size);
// 读取图片
BOOL LoadImage(UINT nID, CBitmap &bitmap, CSize &size, CString strType);
// 从资源中加载图片
BOOL ImageFromIDResource(UINT nID, CString strType, Image * & pImg);
// 加载图片文件到内存中
BOOL ImageFromFile(CString strFile, BOOL useEmbeddedColorManagement, Image * & pImg);
// 从内存中加载图片文件
BOOL ImageFromMem(BYTE* pByte, DWORD dwSize, BOOL useEmbeddedColorManagement, Image * & pImg);
// 取得图片平均颜色
BOOL GetAverageColor(CDC *pDC, CBitmap &bitmap, const CSize &sizeImage, COLORREF &clrImage);
// 取得图片大小
BOOL GetSize(CBitmap &bitmap, CSize &size);
// 取得字体大小
Size GetTextBounds(const Gdiplus::Font& font,const StringFormat& strFormat,const CString& strText);
// 取得字体大小
Size GetTextBounds(const Gdiplus::Font& font,const StringFormat& strFormat, int nWidth, const CString& strText);
// 取得字体大小
Size GetTextBounds(const Gdiplus::Font& font,const CString& strText);
// 取得位置
CPoint GetOriginPoint(int nWidth, int nHeight, int nChildWidth, int nChildHeight, UINT uAlignment = DT_CENTER, UINT uVAlignment = DT_VCENTER);
// 取得位置
CPoint GetOriginPoint(CRect rc, int nChildWidth, int nChildHeight, UINT uAlignment = DT_CENTER, UINT uVAlignment = DT_VCENTER);
// 转换数字
CString DecimalFormat(int nNumber);


// 绘画函数

// 画垂直过渡
int DrawVerticalTransition(CDC &dcDes, CDC &dcSrc, const CRect &rcDes, const CRect &rcSrc, int nBeginTransparent = 0, int nEndTransparent = 100);
// 画水平过渡
int DrawHorizontalTransition(CDC &dcDes, CDC &dcSrc, const CRect &rcDes, const CRect &rcSrc, int nBeginTransparent = 0, int nEndTransparent = 100);
// 画右下角过渡
void DrawRightBottomTransition(CDC &dc, CDC &dcTemp, CRect rc, const int nOverRegio, const COLORREF clrBackground);
// 画图片边框
void DrawImageFrame(Graphics &graphics, Image *pImage, const CRect &rcControl, int nX, int nY, int nW, int nH, int nFrameSide = 4);
// 画图片边框(可指定九宫格的位置,nX/nY是原图的左上角,nWH/nHL是左上角点坐标,nWR/nHR是右下角点坐标)
void DrawImageFrameMID(Graphics &graphics, Image *pImage, const CRect &rcControl, int nX, int nY, int nW, int nH, int nWL, int nHL, int nWR, int nHR);
// 画过渡圆角矩形
void DrawRectangle(CDC &dcDes, const CRect &rcDes, BOOL bUp = TRUE, int nBeginTransparent = 60, int nEndTransparent = 90);

// 创建位图
HBITMAP	DuiCreateCompatibleBitmap(CRect& rcClient);