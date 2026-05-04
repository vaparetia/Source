#ifndef __ARRANGERECT_H__
#define __ARRANGERECT_H__

typedef struct{
	char filename[512];
	short	flag;		//..配置が決定されたら１
	short	pad ;
	short	x,y;		//..座標
	short	w,h;		//..横幅、縦幅
	short	org_w, org_h ;		/* 配置に使用するサイズと実際のサイズが異なるため */
	unsigned long area;		//..面積
	unsigned int tex_id;	//..テクスチャID
    unsigned int tex_flag ;
	int		atomawasi_flg;	//..これがたつと後回し
	RGBA	clut[256];
}RECT_POS;

#define ARRANGE_NUM 10000



enum{
	MITEI = 0,
	KETTEI ,
	ATOMAWASI,
	KARIKETTEI,
};


long get_bmp_max_width(void);
long get_bmp_max_height(void);

RECT_POS* ArrangeRect_CreateWork(int num ,int min, int max);
int ArrangeRect_FreeWork(void);

void ArrangeRect_InputData2Work(int order,int w,int h, char *filename );

void ArrangeRect_DecidePostion(void);
void ArrangeRect_Make_TEXPACK_HeaderImg(void **pimg,int *header_size,int format);
void ArrangeRect_dump(void);
void* ArrangeRect_MakeImg32bit(void **p_tex_img,int *img_size);
void* ArrangeRect_MakeImg24bit(void **p_tex_img,int *img_size);
void* ArrangeRect_MakeImgDXTn(void **p_tex_img,int *img_size,int DXTn);
int ArrangeRect_Nokori(void);

#endif
