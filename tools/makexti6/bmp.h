#ifndef BMP_H_
#define BMP_H_

/*********************************************************************
	
	BMPﾌｧｲﾙ用構造体定義
	
	1999/06/16		NISINO Motoaki

**********************************************************************/
#include <sys/types.h>
//#include "my.h"

typedef struct{

	u_short	bf_type;		//..ASCII "BM"
	
	u_short bf_size_l;
	u_short bf_size_h;
	//u_long	bf_size;		//..ﾌｧｲﾙのｻｲｽﾞ
	
	u_short	bf_reserved1;	//..0
	u_short	bf_reserved2;	//..0
	
	u_short bf_off_bits_l;
	u_short bf_off_bits_h;
	//u_long	bf_off_bits;	//..ｲﾒｰｼﾞの開始位置

}BITMAP_FILEHEADER;

typedef struct{

	u_long	bi_size;		//..ﾍｯﾀﾞのｻｲｽﾞ 40ﾊﾞｲﾄ
	long	bi_width;		//..ｲﾒｰｼﾞの幅
	long	bi_height;		//..ｲﾒｰｼﾞの高さ
	u_short	bi_planes;		//..ｲﾒｰｼﾞﾌﾟﾚｰﾝ数 必ず1
	u_short	bi_bitcount;	//..ﾋﾟｸｾﾙあたりのﾋﾞｯﾄ数 1,4,8,24
	u_long	bi_compression;
	u_long	bi_sizeimage;
	long	bi_xpelspermeter;
	long	bi_ypelspermeter;
	u_long	bi_clrused;
	u_long	bi_clrimportant;

}BITMAP_INFO_HEADER;

typedef struct{
	u_char	b;
	u_char	g;
	u_char	r;	
	//u_char	pad;

}RGB_QUAD;

typedef struct{
	short x,y;
	u_short w,h;
}RECT;


typedef struct{
	u_char *pimg;	//..ｲﾒｰｼﾞが置かれているｱﾄﾞﾚｽ
	short x,y;		//..mallocで確保されたﾒﾓﾘの先頭ｱﾄﾞﾚｽが入る
	u_short w,h;	//..ピクセルサイズ
	short pitch;	//..ピッチ（バイト単位での横幅）
	short byte;		//..1ピクセルあたりのバイト数 (24bit->3 32bi->4)
	long buffer_size;
}RECT2;


typedef struct _rgba {
	unsigned char		b, g, r, a ;
}RGBA ;



extern int  get_pitch(int w,int byte);

extern void b_hai(char *dst, int size);
extern short bmp_pad(short w);
extern short byte_width(short yoko);
extern char* bmp2buffer(char *bmpfilename,/*char *p,*/RECT *prect);

extern short paste_bmpimg(
	RECT *prect_s,u_char *ps,
	RECT *prect_u,u_char *pu,
	short x,short y
	);
extern short paste_bmpimg_semi(
	RECT *prect_s,u_char *ps,
	RECT *prect_u,u_char *pu,
	short x,short y,short semi
	);
extern short paste_rgb_semi(
	short x,short y,
	RECT *prect_s,u_char *ps,
	RECT *prect_u,
	RGB_QUAD rgb,short semi
	);
extern char* bmp2buffer2(char *bmpfilename,RECT2 *prect);
extern short paste_bmpimg_semi2(
	RECT2 *prect_s, RECT2 *prect_u,
	short x,short y,short semi
	);

extern short paste_bmpimg_semi32(RECT2 *prect_s, RECT2 *prect_u,short x,short y,short semi);

extern short settei_bmpheader(FILE *wfp,short tate,short yoko);
extern short paste_rgb_semi2(
	short x,short y,
	RECT2 *prect_s,
	RECT2 *prect_u,
	RGB_QUAD rgb,short semi
	);
extern short draw_yoko_line(short x,short y,short nagasa,RGB_QUAD quad,RECT2 *prect);
extern short draw_dot(short x,short y,RGB_QUAD quad,RECT2 *prect);
extern short copy_bmpimg(short u,short v,short w,short h,RECT2 *pmoto,RECT2 *psaki);
extern short rect2file(RECT2 *prect,u_char *filename);
extern short rect2file32(RECT2 *prect,u_char *filename);
extern short rect2file_revers(RECT2 *prect,u_char *filename);

extern void number_font_load(void);
extern void print_number(short px,short py,RECT2 *pr,int number,int type);

#define SetRect2(p,tx,ty,tw,th) { (p)->x=tx; (p)->y=ty; (p)->w=tw; (p)->h=th;}

extern void RECT2UpsideDown( RECT2 *prect );
extern void RECT2Create(RECT2 *prect, int w, int h, int byte);
extern int RECT2PenetratetionCheckRGBA(RECT2 *prect);





#endif
