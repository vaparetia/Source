#ifndef _DXTCMP_H_
#define _DXTCMP_H_



/* ３２ビットテクスチャイメージの圧縮（DXT3） */
void *CompressImageDXT3( unsigned int *image, int width, int height, void **compress_buff, int *compress_size );

/* ３２ビットテクスチャイメージの圧縮（DXT1） */
void *CompressImageDXT1( unsigned int *image, int width, int height, void **compress_buff, int *compress_size );


#endif