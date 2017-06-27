#include<vector>
using namespace std;


vector<RECT> vecMonitor;


int dspNum = 0;


BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFO monitorInfo;
	monitorInfo.cbSize  = sizeof(MONITORINFO);

	if (GetMonitorInfo(hMonitor, &monitorInfo))
	{
		vecMonitor.push_back(monitorInfo.rcWork);
	}
	//MONITORINFOF_PRIMARY
	dspNum += 1;
	return TRUE;
}


HBITMAP CopyScreenToBitmap(LPRECT lpRect)
{
	HDC hScrDC,hMemDC;
	HBITMAP hBitmap,hOldBitmap;
	int nX1,nX2,nY1,nY2;
	int nWidth,nHeight;
	if (IsRectEmpty(lpRect))
	{
		return FALSE;
	}
	//为屏幕创建设备描述表
	hScrDC = CreateDC("DISPLAY",NULL,NULL,NULL);
	//为屏幕设备描述表创建兼容的内存设备描述表
	hMemDC = CreateCompatibleDC(hScrDC);
	// 获得选定区域坐标
	nX1 = lpRect->left;
	nY1 = lpRect->top;
	nX2 = lpRect->right;
	nY2 = lpRect->bottom;

	nWidth = nX2 - nX1;
	nHeight = nY2 - nY1;
	// 创建一个与屏幕设备描述表兼容的位图
	hBitmap = CreateCompatibleBitmap(hScrDC,nWidth,nHeight);
	// 把新位图选到内存设备描述表中
	hOldBitmap = (HBITMAP)SelectObject(hMemDC,hBitmap);
	// 把屏幕设备描述表拷贝到内存设备描述表中
	BitBlt(hMemDC,0,0,nWidth,nHeight,hScrDC,nX1,nY1,SRCCOPY);
	//得到屏幕位图的句柄
	hBitmap = (HBITMAP)SelectObject(hMemDC,hOldBitmap);
	//清除
	DeleteDC(hScrDC);
	DeleteDC(hMemDC);
	return hBitmap;
}


int SaveBitmapToFile(HBITMAP hBitmap, LPCTSTR lpFileName) //hBitmap 为刚才的屏幕位图句柄
{
	//lpFileName为位图文件名
	HDC hDC; 
	//设备描述表
	int iBits; 
	//当前显示分辨率下每个像素所占字节数
	WORD wBitCount; 
	//位图中每个像素所占字节数
	//定义调色板大小， 位图中像素字节大小 ， 位图文件大小 ， 写入文件字节数
	DWORD dwPaletteSize=0,dwBmBitsSize,dwDIBSize, dwWritten;
	BITMAP Bitmap; 
	//位图属性结构
	BITMAPFILEHEADER bmfHdr; 
	//位图文件头结构
	BITMAPINFOHEADER bi; 
	//位图信息头结构 
	LPBITMAPINFOHEADER lpbi; 
	//指向位图信息头结构
	HANDLE fh, hDib, hPal;
	HPALETTE hOldPal=NULL;
	//定义文件，分配内存句柄，调色板句柄

	//计算位图文件每个像素所占字节数
	hDC = CreateDC("DISPLAY",NULL,NULL,NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * 
		GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	if (iBits <= 1)
		wBitCount = 1;
	else if (iBits <= 4)
		wBitCount = 4;
	else if (iBits <= 8)
		wBitCount = 8;
	else if (iBits <= 24)
		wBitCount = 24;
	else
		wBitCount = 32;
	//计算调色板大小
	if (wBitCount <= 8)
		dwPaletteSize=(1<<wBitCount)*sizeof(RGBQUAD);

	//设置位图信息头结构
	GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&Bitmap);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap.bmWidth;
	bi.biHeight = Bitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	dwBmBitsSize = ((Bitmap.bmWidth*wBitCount+31)/32)*4*Bitmap.bmHeight;
	//为位图内容分配内存

	/*xxxxxxxx计算位图大小分解一下(解释一下上面的语句)xxxxxxxxxxxxxxxxxxxx 
	//每个扫描行所占的字节数应该为4的整数倍，具体算法为:
	int biWidth = (Bitmap.bmWidth*wBitCount) / 32;
	if((Bitmap.bmWidth*wBitCount) % 32)
	biWidth++; //不是整数倍的加1
	biWidth *= 4;//到这里，计算得到的为每个扫描行的字节数。
	dwBmBitsSize = biWidth * Bitmap.bmHeight;//得到大小
	xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*/


	hDib = GlobalAlloc(GHND,dwBmBitsSize+dwPaletteSize+sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;
	// 处理调色板 
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal)
	{
		hDC = ::GetDC(NULL);
		hOldPal=SelectPalette(hDC,(HPALETTE)hPal,FALSE);
		RealizePalette(hDC);
	}
	// 获取该调色板下新的像素值
	GetDIBits(hDC,hBitmap,0,(UINT)Bitmap.bmHeight,(LPSTR)lpbi+sizeof(BITMAPINFOHEADER)+dwPaletteSize, (BITMAPINFO *)lpbi,DIB_RGB_COLORS);
	//恢复调色板 
	if (hOldPal)
	{
		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		::ReleaseDC(NULL, hDC);
	}
	//创建位图文件 
	fh=CreateFile(lpFileName, GENERIC_WRITE,0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh==INVALID_HANDLE_VALUE)
		return FALSE;
	// 设置位图文件头
	bmfHdr.bfType = 0x4D42; // "BM"
	dwDIBSize=sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize; 
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER)+dwPaletteSize;
	// 写入位图文件头
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	// 写入位图文件其余内容
	WriteFile(fh, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)+dwPaletteSize+dwBmBitsSize , &dwWritten, NULL); 
	//清除 
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}


void SceenShoot()
{
	
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, NULL);
	//int dspNum = ::GetSystemMetrics(SM_CMONITORS);

	
	int            dspNum           = 0;
	DISPLAY_DEVICE ddDisplay;
	DEVMODE        dmDevMode;

	ZeroMemory(&ddDisplay, sizeof(ddDisplay));
	ddDisplay.cb = sizeof(ddDisplay);
	ZeroMemory(&dmDevMode, sizeof(dmDevMode));
	dmDevMode.dmSize = sizeof(dmDevMode);
	BOOL bFlag = true;
	while (bFlag)
	{
		bFlag = EnumDisplayDevices(NULL, dspNum, &ddDisplay, 0);
		bFlag = bFlag & EnumDisplaySettings(ddDisplay.DeviceName, ENUM_CURRENT_SETTINGS, &dmDevMode);
		if (bFlag)
		{
			dspNum += 1;
		}
	}

	HBITMAP g_screenBmp=NULL;

	vector<RECT>::iterator it = vecMonitor.begin();
	
	RECT rc;
	rc.left = (*it).left;
	rc.top = (*it).top;
	rc.right = (*it).right;
	rc.bottom = (*it).bottom;
	g_screenBmp = CopyScreenToBitmap(&rc);
		
	SaveBitmapToFile(g_screenBmp,"d:\\hideme\\screen.bmp");
	
}