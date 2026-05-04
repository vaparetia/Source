//-----------------------------------------------------------------------------
// BMPファイルヘッダ
//   (from  "Program Files\Microsoft Visual Studio\VC98\Include\WINGDI.h")
//-----------------------------------------------------------------------------

#ifndef __FMT_BMP_H__
#define __FMT_BMP_H__

typedef struct tagBITMAPFILEHEADER {
	WORD    bfType;      // ファイルタイプ
	//DWORD   bfSize;	 // ファイルサイズ
	WORD   bfSize0;
	WORD   bfSize1;
	WORD    bfReserved0; // 予約
	WORD    bfReserved1; // 予約
	//DWORD   bfOffBits;   // ファイル先頭から画像データまでのオフセット
	WORD   bfOffBits0;   // ファイル先頭から画像データまでのオフセット
	WORD   bfOffBits1;   // ファイル先頭から画像データまでのオフセット	
} BITMAPFILEHEADER, FAR *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;

// OS/2 Bitmap
typedef struct tagBITMAPCOREHEADER {
	DWORD   bcSize;      // この構造体のサイズ(byte)
	WORD    bcWidth;     // 画像幅(ピクセル)
	WORD    bcHeight;    // 画像高さ
	WORD    bcPlanes;    // プレーン数(常に1)
	WORD    bcBitCount;  // 1画素あたりのデータサイズ(bit) 1,4,8,16,24,32
} BITMAPCOREHEADER, FAR *LPBITMAPCOREHEADER, *PBITMAPCOREHEADER;

// Windows Bitmap
typedef struct tagBITMAPINFOHEADER {
	DWORD      biSize;          // この構造体のサイズ(通常40)
	LONG       biWidth;         // 画像幅(ピクセル)
	LONG       biHeight;        // 画像高さ(ピクセル)
	WORD       biPlanes;        // プレーン数(常に1)
	WORD       biBitCount;      // 1画素あたりのデータサイズ(bit) 1,4,8,16,24,32
	DWORD      biCompression;   // 圧縮形式(※)
	DWORD      biSizeImage;     // 画像データのサイズ(byte)
	LONG       biXPelsPerMeter; // 横方向解像度(1mあたりの画素数)
	LONG       biYPelsPerMeter; // 縦方向解像度(1mあたりの画素数)
	DWORD      biClrUsed;       // パレットの色数(0なら最大)
	DWORD      biClrImportant;  // 重要なパレットのインデックス
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

/*
 * ※ 圧縮形式
 * #define BI_RGB        0L (無圧縮)
 * #define BI_RLE8       1L (RunLength 8bits/pixel)
 * #define BI_RLE4       2L (Runlength 4bits/pixel)
 * #define BI_BITFIELDS  3L (Bitfields)
 */

// ピクセルフォーマット
typedef struct tagRGBTRIPLE {
	BYTE    rgbtBlue;
	BYTE    rgbtGreen;
	BYTE    rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;

// カラーテーブル付き
typedef struct BITMAPINFO {
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD          bmiColors[1];
} BITMAPINFO;

#endif // __FMT_BMP_H__
