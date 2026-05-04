/*
	jpeg.h
*/

#ifndef PI
#define PI 3.1415926f
#endif
#define Y  0
#define C  1
#define BLEND 0
#define X_WAKU 0
#define Y_WAKU 0

/* encode */

typedef struct {
	void *vram;				// 入力バッファ width * height * 4
	void *work;				// テンポラリバッファ input_vramと同サイズ
	void *code;				// 出力バッファ 適当に大きく
	int code_size;			// 出力バッファサイズ
	short width;			// 入力バッファの幅。32の倍数
	short height;			// 入力バッファの高さ。32の倍数
} UTL_JPEGWORK;

int UTL_JpegEncode( UTL_JPEGWORK *jpegwork, int quality );
int UTL_JpegDecode( void *image, int width, int height, void *code );

void UTL_MakeThumbnail( UTL_JPEGWORK *jpegwork
					   , void *thumb_buf, int width, int height );
