/*
	ＩＰＵストリーミングドライバ

	2001/01/10 K.Takabe
	$Id: ipu.h,v 1.1.1.3 2002/11/19 11:41:51 Yoshizawa1 Exp $
*/

#ifndef __IPU_H__
#define __IPU_H__

/* 再生モード設定フラグ */
enum {
	GM_IPU_MODE_RGB32		= 0x0000,
	GM_IPU_MODE_RGB16		= 0x0001,
};

// BP - Changed to only support 32-bit image frames and force users to specify width and height instead of getting 
// it from the stream.
BP_Movie_Handle GM_StreamIpuDriverInit(int width, int height);

/* ＩＰＵストリームドライバ停止 */
void GM_StreamIpuDriverEnd( void );

/* ストリームの終了判定 */
int GM_StreamIsEndIpu( void );

/* ストリームの映像幅・高さ・最大フレーム数を取得 */
int GM_StreamGetIpuInfo( int *width, int *height, int *frame );

/* ストリームの再生フレーム数取得 */
int GM_StreamGetIpuFrame( void );

/* 純粋な展開後ＩＰＵマクロブロックデータへのポインタ取得 */
void* GM_StreamGetIpu( void );

/* 現在のデータを開放して次のデータを取得する */
void GM_StreamFreeIpu( void );

/* 展開後マクロブロックをラスタイメージに変換してコピーする */
void GM_StreamCopyIpuImage( void *dst, int dst_width, int dst_height, int dst_x, int dst_y, int flag );

extern void *NewMemStreamIpu( void *ipu_data, int mode, BP_Movie_Handle* pBP_Movie_Handle );

#endif
