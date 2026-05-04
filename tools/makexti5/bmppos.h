#ifndef __BMPPOS_H__
#define __BMPPOS_H__

;typedef struct{
	u_char filename[64];
	short	flag;		//..配置が決定されたら１
	short	pad ;
	short	x,y;		//..座標
	short	w,h;		//..横幅、縦幅
	short	org_w, org_h ;		/* 配置に使用するサイズと実際のサイズが異なるため */
	unsigned long area;		//..面積
	unsigned int tex_id;	//..テクスチャID
    unsigned int tex_flag ;
}RECT_BMP_POS;


#define MAX_TEX_NUM 512
#define ARRANGE_NUM 10000

#define MAX_WIDTH	800
#define MAX_HEIGHT	2000

extern void init_rect_arrange(void);
extern void input_bmppos(int w,int h, char *filename,unsigned int mozi_code,unsigned int flag);
extern void main_rect_arrange(void);
extern void arrange_result(void);
extern void make_one_bmp(u_char *new_filename);
extern void *make_one_bmp32(char *new_filename);



#endif
