/*****************************************************************



******************************************************************/

#ifdef __WIN32_APP__
#include "stdafx.h"
#include "rect.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my.h"
#include "bmppos.h"


#ifdef __WIN32_APP__
#define printf  TRACE
#endif



#ifndef __WIN32_APP__
#include "bmp.h"
#include "fmt_kms.h"
#include "MDU_util.h"
#include "MDU_tex.h"
#include "makexti.h"

#endif

static void arrange_rect_bmp_pos(int num);

RECT_BMP_POS rect_bmp_pos[MAX_TEX_NUM]
#if 1
;
#else
={


	{0,	0,0,		200,40,	0},	
	{0,	0,0,		200,80,	0},
	{0,	0,0,		100,80,	0},	
	{0,	0,0,		80,80,	0},

	{0,	0,0,		10,40,	0},
	{0,	0,0,		10,30,	0},
	{0,	0,0,		10,20,	0},
	{0,	0,0,		10,10,	0},


};
#endif

//..候補地のワーク
RECT_BMP_POS rect_arrange_pos[ARRANGE_NUM];

long bmp_max_width;
long bmp_max_height;

//..テクスチャの枚数を数えるカウンタ
static int tex_cnt=0;



#ifdef __WIN32_APP__
inline int getRand(int min ,int max)
{	
	return (rand()%(max-min+1))+min;
	//return random(max-min+1)+min;
}
#endif


void cluc_area(RECT_BMP_POS *prmp,int num)
{
	int i;
	
	for(i=0; i<num; i++,prmp++)
		prmp->area=prmp->w * prmp->h;
	
}

void swap_RECT_BMP_POS(RECT_BMP_POS *prmp1,RECT_BMP_POS *prmp2)
{
	RECT_BMP_POS tmp;

	memmove(&tmp,prmp1,sizeof(RECT_BMP_POS));
	memmove(prmp1,prmp2,sizeof(RECT_BMP_POS));
	memmove(prmp2,&tmp,sizeof(RECT_BMP_POS));
}


void bubble_sort_RECT_BMP_POS(RECT_BMP_POS prmp[],int size)
{

	int i,j;
	for(i=0; i<size-1; i++){
		for(j=size-1; j>=i; j--){
			if(prmp[j].area < prmp[j-1].area){
				swap_RECT_BMP_POS(&prmp[j-1],&prmp[j]);
			}
		}
	}
}

void bubble_sort_RECT_BMP_POS_R(RECT_BMP_POS prmp[],int size)
{

	int i,j;
	for(i=0; i<size; i++){
		for(j=size-1; j>i; j--){
			//printf("i,j,j-1=%d,%d,%d\n",i,j,j-1);
			if(prmp[j].area > prmp[j-1].area){
				swap_RECT_BMP_POS(&prmp[j-1],&prmp[j]);
			}
		}
	}
}



/*
void bubble_sort(int a[],int size){

	int i,j,tmp;
	for(i=0; i<size-1; i++){
		for(j=size-1; j>=i; j--){
			if(a[j] < a[j-1]){
				swap(a[j-1],a[j]);
			}
		}
	}
}
*/
#if 0
//..重なっているかどうか判定
int kasanari(RECT_BMP_POS *prmp1,RECT_BMP_POS *prmp2)
{
	int i,j;
	
	for(j=prmp1->y; j<prmp1->y+prmp1->h; j++){
		for(i=prmp1->x; i<prmp1->x+prmp1->w; i++){
			
			if(	(prmp2->x <= i && i<prmp2->x + prmp2->w)
				&&
				(prmp2->y <= j && j<prmp2->y + prmp2->h)
			){
					return 1;
			}
		}
	}
	return 0;
}

#else
//..横線　縦線
inline int line_cross(int L1_x1,int L1_x2,int L1_y, int L2_x,int L2_y1,int L2_y2)
{
	if((L1_x1 <= L2_x && L2_x < L1_x2) && (L2_y1 <= L1_y && L1_y < L2_y2)) return 1;

	return 0;
}


//..矩形同士の重なりを調べる
//..重なる		:1
//..重ならない 	:0

inline int kasanari(RECT_BMP_POS *prmp1,RECT_BMP_POS *prmp2)
{
	//..
	if(line_cross(	prmp1->x, prmp1->x+prmp1->w-1, prmp1->y, 	
					prmp2->x, prmp2->y, prmp2->y+prmp2->h-1)){ return 1;}
	if(line_cross(	prmp1->x, prmp1->x+prmp1->w-1, prmp1->y+prmp1->h-1, 	
					prmp2->x, prmp2->y, prmp2->y+prmp2->h-1)){ return 1;};
	
	if(line_cross(	prmp1->x, prmp1->x+prmp1->w-1, prmp1->y, 	
					prmp2->x+prmp2->w-1, prmp2->y, prmp2->y+prmp2->h-1)){ return 1;};
	if(line_cross(	prmp1->x, prmp1->x+prmp1->w-1, prmp1->y+prmp1->h-1, 	
					prmp2->x+prmp2->w-1, prmp2->y, prmp2->y+prmp2->h-1)){ return 1;};

	if(line_cross(	prmp2->x, prmp2->x+prmp2->w-1, prmp2->y, 	
					prmp1->x, prmp1->y, prmp1->y+prmp1->h-1)){ return 1;};
	if(line_cross(	prmp2->x, prmp2->x+prmp2->w-1, prmp2->y+prmp2->h-1, 	
					prmp1->x, prmp1->y, prmp1->y+prmp1->h-1)){ return 1;};
	
	if(line_cross(	prmp2->x, prmp2->x+prmp2->w-1, prmp2->y, 	
					prmp1->x+prmp1->w-1, prmp1->y, prmp1->y+prmp1->h-1)){ return 1;};
	if(line_cross(	prmp2->x, prmp2->x+prmp2->w-1, prmp2->y+prmp2->h-1, 	
					prmp1->x+prmp1->w-1, prmp1->y, prmp1->y+prmp1->h-1)){ return 1;};
	
	//..矩形が一方を包むような重なりを調べる
	if( (prmp1->x <= prmp2->x && prmp1->y <= prmp2->y) && 
		(prmp2->x+prmp2->w <= prmp1->x+prmp1->w && prmp2->y+prmp2->h <= prmp1->y+prmp1->h) ) return 1;
	
	if( (prmp1->x >= prmp2->x && prmp1->y >= prmp2->y) && 
		(prmp2->x+prmp2->w >= prmp1->x+prmp1->w && prmp2->y+prmp2->h >= prmp1->y+prmp1->h) ) return 1;

	return 0;

}
#endif


//..空きのワークを見つけ、番号を返し、埋める
int get_vacant_work(void)
{
	int i;
	for(i=0; i<ARRANGE_NUM; i++){
		if(rect_arrange_pos[i].flag==NULL){
			rect_arrange_pos[i].flag=1;
			return i;
		}
	}
	printf("空きのワークがありません\n");
	exit(1);
	//return(-1);
}




//..矩形と線、重なっているかどうか判定。縦線と交差
static int kasanari_sen_V(RECT_BMP_POS *prmp1,int sx)
{
	
	if(prmp1->x <= sx  &&  sx < prmp1->x + prmp1->w){
		return prmp1->y+prmp1->h;
	}
	return 0;
}

//..矩形と線、重なっているかどうか判定。横線と交差
static int kasanari_sen_H(RECT_BMP_POS *prmp1,int sy)
{
	
	if(prmp1->y <= sy  &&  sy < prmp1->y + prmp1->h){
		return prmp1->x+prmp1->w;
	}
	return 0;
}




static void arrange_rect_bmp_pos(int num)
{
	int tx,ty,tmp_area[ARRANGE_NUM],tmp_kasanari[ARRANGE_NUM];
	int i,j,min,max=0,near_num,near_area[ARRANGE_NUM],tmp_near=0,tmp_num=0;
	int n1,n2,ka,entyo_yoko,entyo_tate;
	RECT_BMP_POS t_rbp;
	
	yarinaosi:;
	
	memset(tmp_area,0,sizeof(int)*ARRANGE_NUM);
	memset(tmp_kasanari,0,sizeof(int)*ARRANGE_NUM);
	memset(near_area,0,sizeof(int)*ARRANGE_NUM);

	//..
	entyo_yoko=0;
	entyo_tate=0;

	
	printf("%d個目配置\n",num);
	//..num個目配置！！！
	
	
	//-------候補地検索-----------
	for(i=0; i<ARRANGE_NUM; i++){
		
		//..候補地検索
		if(rect_arrange_pos[i].flag){
			
			//..候補地に仮配置してみる
			tx=rect_arrange_pos[i].x+rect_bmp_pos[num].w;
			ty=rect_arrange_pos[i].y+rect_bmp_pos[num].h;
			
			{
				//..他の矩形と重ならないかどうかの判定
				ka=0;	//..重なりフラグ
				//..仮配置した矩形
				t_rbp.x=rect_arrange_pos[i].x;
				t_rbp.y=rect_arrange_pos[i].y;
				t_rbp.w=rect_bmp_pos[num].w;
				t_rbp.h=rect_bmp_pos[num].h;

				for(j=0;j<tex_cnt;j++){
				//for(j=0;j<MAX_TEX_NUM;j++){
					//..場所が決定している矩形
					if(rect_bmp_pos[j].flag==1){
						if((ka=kasanari(&t_rbp,&rect_bmp_pos[j]))){
							//..重なっていたら抜ける
							break;
						}
					}
				}
			}
			
			//..元の矩形のほうが大きい時は
			if(rect_arrange_pos[i].w>=tx)	tx=rect_arrange_pos[i].w;
			if(rect_arrange_pos[i].h>=ty)	ty=rect_arrange_pos[i].h;
			
			tmp_area[i]=tx * ty;
			if(tmp_area[i]>max) max=tmp_area[i];


			//..高さ幅の制限　制限越えたらフラグがたつ
			if(tx>bmp_max_width ) {
				if ( bmp_max_width < 4096) {
					ka=1;	entyo_yoko=1;
//printf( "koko?\n" ) ;
				}
			} else if(ty>bmp_max_height ) {
				if ( bmp_max_height < 4096){
					ka=1;	entyo_tate=1;
//printf( "tate ?\n" ) ;
				} else {
				}
			}
			tmp_kasanari[i]=ka;//..重なりフラグ代入
			
		}
	}
	
	min=max;
	//min=tmp_area[0];
	
	//-------もっとも効率のよい場所は-----------
	tmp_num=-1;
	for(i=0; i<ARRANGE_NUM; i++){
		if(rect_arrange_pos[i].flag){
			//dprint(i);dprint(min);dprint(tmp_area[i]);dprint(tmp_kasanari[i]);
			
			if(min>=tmp_area[i] && tmp_kasanari[i]==0){
				min=tmp_area[i];
				tmp_num=i;
#if 0
			printf("候補地%3d xy(%3d,%3d)wh(%3d,%3d):%4d=%4d*%4d : %c\n",
				i,rect_arrange_pos[i].x,rect_arrange_pos[i].y,
				rect_arrange_pos[i].w,rect_arrange_pos[i].h,
				tmp_area[i],tx,ty,(tmp_kasanari[i]==2)?'W':(tmp_kasanari[i]==1)?'X':'O');
#endif
			}
		}
	}
	if(tmp_num==-1){
		printf("矩形拡大 %d\n",num);

#if 1
#if 0
		//..右におおきくなりがち
		if(entyo_yoko ){
			bmp_max_width*=2;
			dprint(bmp_max_width);
			dprint(bmp_max_height);
		}
		else if(entyo_tate){
			bmp_max_height*=2;
			dprint(bmp_max_width);
			dprint(bmp_max_height);
		}
#else
		if ( entyo_yoko || entyo_tate ){
			if ( bmp_max_width < bmp_max_height ){
				bmp_max_width *= 2 ;
			} else {
				bmp_max_height *= 2 ;
			}
		}
#endif
#else	
		//..正方形
		if(bmp_max_width<bmp_max_height){
			bmp_max_width*=2;
			dprint(bmp_max_width);
			dprint(bmp_max_height);
		}else{
			bmp_max_height*=2;
			dprint(bmp_max_width);
			dprint(bmp_max_height);
		}
#endif
		goto yarinaosi;
		//return;
	}
	
	printf("最小の面積の地点.. %d:%d\n",tmp_num,tmp_area[tmp_num]);
	

	if(min==0){
		printf("ERROR: 最小の面積が０です。\n");
	}
	
	near_num=0;
	min=20000;
	//..最も原点に近い場所を算定
	for(i=0; i<ARRANGE_NUM; i++){
		if(rect_arrange_pos[i].flag){
			if(tmp_area[tmp_num]==tmp_area[i] && tmp_kasanari[i]==0){
				near_area[i]=rect_arrange_pos[i].x+rect_arrange_pos[i].y;
				if(min>=near_area[i]){
					min=near_area[i];
					tmp_near=i;
					//printf("near %2d:%4d\n",i,near_area[i]);
					
				}
				near_num++;
			}
		}
	}
#if 1	
	if(near_num>1){
	
		tmp_num=tmp_near;
	}	
#endif	
	
	
		
	//..矩形配置位置決定
	rect_bmp_pos[num].flag=1;
	rect_bmp_pos[num].x=rect_arrange_pos[tmp_num].x;
	rect_bmp_pos[num].y=rect_arrange_pos[tmp_num].y;
	
	//..次の候補位置算出
	n1=get_vacant_work();
	n2=get_vacant_work();

	//..配置後の全体の縦横を求める
	tx=rect_arrange_pos[tmp_num].x+rect_bmp_pos[num].w;
	ty=rect_arrange_pos[tmp_num].y+rect_bmp_pos[num].h;
	//..元の矩形のほうが大きい時は
	if(rect_arrange_pos[tmp_num].w>=tx)	tx=rect_arrange_pos[tmp_num].w;
	if(rect_arrange_pos[tmp_num].h>=ty)	ty=rect_arrange_pos[tmp_num].h;

	rect_arrange_pos[n1].x=rect_arrange_pos[tmp_num].x+rect_bmp_pos[num].w;
	rect_arrange_pos[n1].y=rect_arrange_pos[tmp_num].y;
	
	rect_arrange_pos[n2].x=rect_arrange_pos[tmp_num].x;
	rect_arrange_pos[n2].y=rect_arrange_pos[tmp_num].y+rect_bmp_pos[num].h;
	
#if 1
	{
	
	//..更なる候補地点
	int n3,n4,pos;

#if 1
	
	for(i=0;i<num;i++){
		//..場所が決定している矩形
		if(rect_bmp_pos[i].flag==1){
			if((pos=kasanari_sen_V(&rect_bmp_pos[num],rect_bmp_pos[i].x+rect_bmp_pos[i].w))){
//				if(pos<rect_arrange_pos[n1].y){
					//..延長線上の候補地点
					n3=get_vacant_work();
					rect_arrange_pos[n3].x=rect_bmp_pos[i].x+rect_bmp_pos[i].w;
					rect_arrange_pos[n3].y=pos;
					
					//dprint(i);dprint(rect_arrange_pos[n3].x);dprint(rect_arrange_pos[n3].y);
					
//				}
			}
		}
	}

	for(i=0;i<num;i++){
		//..場所が決定している矩形
		if(rect_bmp_pos[i].flag==1){
			if((pos=kasanari_sen_H(&rect_bmp_pos[num],rect_bmp_pos[i].y+rect_bmp_pos[i].h))){
//				if(pos<rect_arrange_pos[n1].y){
					//..延長線上の候補地点
					n3=get_vacant_work();
					rect_arrange_pos[n3].x=pos;
					rect_arrange_pos[n3].y=rect_bmp_pos[i].y+rect_bmp_pos[i].h;
					//dprint(i);dprint(rect_arrange_pos[n3].x);dprint(rect_arrange_pos[n3].y);
					
//				}
			}
		}
	}

#endif


#if 0
	for(i=0;i<MAX_TEX_NUM;i++){
		//..場所が決定している矩形
		if(rect_bmp_pos[i].flag==1){
			if((pos=kasanari_sen_V(&rect_bmp_pos[i],rect_arrange_pos[n1].x))){
				if(pos<rect_arrange_pos[n1].y){
					//..延長線上の候補地点
					n3=get_vacant_work();
					rect_arrange_pos[n3].x=rect_arrange_pos[n1].x;
					rect_arrange_pos[n3].y=pos;
				}
			}
		}
	}
#endif

#if 0
	for(i=0;i<MAX_TEX_NUM;i++){
		//..場所が決定している矩形
		if(rect_bmp_pos[i].flag==1){
			if((pos=kasanari_sen_H(&rect_bmp_pos[i],rect_arrange_pos[n2].y))){
				if(pos<rect_arrange_pos[n2].x){
					//..延長線上の候補地点
					n4=get_vacant_work();
					rect_arrange_pos[n4].x=pos;
					rect_arrange_pos[n4].y=rect_arrange_pos[n2].y;
				}
			}
		}
	}
#endif


	}
#endif
	
	//..埋まっている候補地点に新しいwh代入
	for(i=0; i<ARRANGE_NUM; i++)
		if(rect_arrange_pos[i].flag==1){
			rect_arrange_pos[i].w=tx;
			rect_arrange_pos[i].h=ty;
		}
	
	//dprint(tx);dprint(ty);

}




#if 0
//..ツール用ワーク初期化関数
//..init_rect_arrangeへ移動
void init_bmppos(void)
{
	tex_cnt=0;
	memset(rect_bmp_pos,0,sizeof(RECT_BMP_POS)*MAX_TEX_NUM);
}
#endif


//..初期化
void init_rect_arrange(void)
{
	
	printf("init_rect_arrange()\n");
	
	tex_cnt=0;
	
	bmp_max_width=16;
	bmp_max_height=16;

	//..候補地ワーク初期化
	memset(rect_arrange_pos,0,sizeof(RECT_BMP_POS)*ARRANGE_NUM);

	memset(rect_bmp_pos,0,sizeof(RECT_BMP_POS)*MAX_TEX_NUM);

#if 0
	//..TEST
	for(i=0; i<MAX_TEX_NUM; i++){
		rect_bmp_pos[i].w=getRand(30,80);
		rect_bmp_pos[i].h=getRand(20,80);
	}
#endif	

}


//..座標代入
void input_bmppos(int w,int h, char *filename,unsigned int mozi_code,unsigned int tex_flag)
{
	if(filename==NULL){
		printf("ERROR: Nullpointer in input_bmppos()\n");
		exit(1);
	}
	//printf("filename=%s\n",filename);
	strcpy((char*)rect_bmp_pos[tex_cnt].filename,filename);
	rect_bmp_pos[tex_cnt].org_w = w ;
	rect_bmp_pos[tex_cnt].org_h = h ;
#if 1
	/* DXTテクスチャ圧縮を考慮しては位置情報ではサイズを４の倍数にそろえる */
	w = ( w + 3 ) & ~0x3 ;
	h = ( h + 3 ) & ~0x3 ;
#endif
	rect_bmp_pos[tex_cnt].w=w;
	rect_bmp_pos[tex_cnt].h=h;
	rect_bmp_pos[tex_cnt].tex_id=mozi_code;
	rect_bmp_pos[tex_cnt].tex_flag=tex_flag;
	tex_cnt++;

}



void main_rect_arrange(void)
{
	int i;
	//..mainに	
	//..矩形の面積計算
	cluc_area(rect_bmp_pos,tex_cnt);
	
	//..面積の大きい順に並び替え
	bubble_sort_RECT_BMP_POS_R(rect_bmp_pos,tex_cnt);

#if 0	
	//..
	for(i=0; i<MAX_TEX_NUM; i++){
		printf("%3d:%4d wh(%4d,%4d) xy(%4d,%4d)\n",
			i,rect_bmp_pos[i].area,
			rect_bmp_pos[i].w,rect_bmp_pos[i].h,
			rect_bmp_pos[i].x,rect_bmp_pos[i].y);
	}
#endif

	rect_arrange_pos[0].flag=1;
	
	//..並べる
	for(i=0;i<tex_cnt;i++)
		arrange_rect_bmp_pos(i);
	
	//..
	printf("結果\n");
	dprint(rect_arrange_pos[0].w);
	dprint(rect_arrange_pos[0].h);
	dprint(bmp_max_width);
	dprint(bmp_max_height);
}


//..配置後の結果を出力
void arrange_result(void)
{
	int i;
	dprint(tex_cnt);
	for(i=0; i<tex_cnt; i++){
		printf("%s \t: xy(%4d,%4d) wh(%4d,%4d)area(%5d),%x\n",
			rect_bmp_pos[i].filename,
			rect_bmp_pos[i].x,
			rect_bmp_pos[i].y,
			rect_bmp_pos[i].w,
			rect_bmp_pos[i].h,
			rect_bmp_pos[i].area,
			rect_bmp_pos[i].tex_id
			
		);
	}
	//..
	printf("一体化ファイル結果\n");
	dprint(rect_arrange_pos[0].w);
	dprint(rect_arrange_pos[0].h);
	dprint(bmp_max_width);
	dprint(bmp_max_height);
}




//..BMPファイルをひとつに
//..rect_arrangeの次に呼ぶ
void make_one_bmp(u_char *new_filename)
{
	int i,w,h;
	RECT2 rect_s,rect_u0;


#if 1
	//..2の乗数の場合
	rect_s.w=bmp_max_width;
	rect_s.h=bmp_max_height;
#else	
	//..必要最低限の大きさ
	rect_s.w=rect_arrange_pos[0].w;
	rect_s.h=rect_arrange_pos[0].h;
#endif
	
	w=byte_width(rect_s.w);
	h=rect_s.h;
	
	rect_s.pimg=(u_char*)malloc(w * h);
	if(rect_s.pimg==NULL){
		printf("メモリが確保できません。\n");exit(1);
	}
	//..クリア
	b_hai((char*)rect_s.pimg,w * h);	
	
	//..新矩形に貼り付け
	for(i=0; i<tex_cnt; i++){
		//dprints(rect_bmp_pos[i].filename);

#if 0
		bmp2buffer2(rect_bmp_pos[i].filename,&rect_u0);
#else
		{
	        MDU_Tex *tex;
			BYTE *p;
			//void *p;
			
	        tex = LoadTex((char*)rect_bmp_pos[i].filename);
			if (tex == NULL) {
				printf("file \"%s\" : LoadTex failed\n", rect_bmp_pos[i].filename);
				continue;
			}
			/* CLUT付きで読み込まれたテクスチャを32bitに変換する */
			MakeFullColorTex(tex);
			/* 32bit/または24bitのBMPに変換する */
			p = (char*)MakeFullColorBmp(tex, 24);

			rect_u0.x=0;
			rect_u0.y=0;
			rect_u0.w = tex->width;
			rect_u0.h = tex->height;

			{
			int w2;
			//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
			w2=byte_width(tex->width);
		
			//..ﾒﾓﾘ確保
			rect_u0.pimg=(u_char*)malloc(w2 * tex->height);
		
			if(rect_u0.pimg==NULL){
				printf("メモリが確保できません。\n");exit(1);
			}
		
			//..クリア
			b_hai((char*)rect_u0.pimg,w2 * tex->height);	
			memcpy(rect_u0.pimg, p + 54, w2 * tex->height);
			
			}

		}
#endif

#if 1		
		paste_bmpimg_semi2(&rect_s,&rect_u0,
			rect_bmp_pos[i].x,
			rect_bmp_pos[i].y,0);
#endif		
		free(rect_u0.pimg);

	}
	rect2file(&rect_s,new_filename);	
}




//..BMPファイルをひとつに
//..rect_arrangeの次に呼ぶ
void *make_one_bmp32(char *new_filename)
{
	int i,w,h;
	RECT2 rect_s,rect_u0;


#if 1
	//..2の乗数の場合
	rect_s.w=bmp_max_width;
	rect_s.h=bmp_max_height;
#else	
	//..必要最低限の大きさ
	rect_s.w=rect_arrange_pos[0].w;
	rect_s.h=rect_arrange_pos[0].h;
#endif
	
	w=rect_s.w*4;
	h=rect_s.h;
	
	rect_s.pimg=(u_char*)malloc(w * h);
	if(rect_s.pimg==NULL){
		printf("メモリが確保できません。\n");exit(1);
	}
	//..クリア
	b_hai((char*)rect_s.pimg,w * h);	
	
	//..新矩形に貼り付け
	for(i=0; i<tex_cnt; i++){
		//dprints(rect_bmp_pos[i].filename);

#if 0
		bmp2buffer2(rect_bmp_pos[i].filename,&rect_u0);
#else
		{
	        MDU_Tex *tex;
			BYTE *p;
			//void *p;
			
	        tex = LoadTex((char*)rect_bmp_pos[i].filename);
			if (tex == NULL) {
				printf("file \"%s\" : LoadTex failed.(bmppos.cpp)\n", rect_bmp_pos[i].filename);
				continue;
			}
			/* CLUT付きで読み込まれたテクスチャを32bitに変換する */
			MakeFullColorTex(tex);
			/* 32bit/または24bitのBMPに変換する */
			p = (char*)MakeFullColorBmp(tex, 32);

			rect_u0.x=0;
			rect_u0.y=0;
			rect_u0.w = tex->width;
			rect_u0.h = tex->height;

			{
			int w2;
			//..ﾒﾓﾘ上の幅..ﾊﾞｲﾄ単位
			w2=tex->width*4;
		
			//..ﾒﾓﾘ確保
			rect_u0.pimg=(u_char*)malloc(w2 * tex->height);
		
			if(rect_u0.pimg==NULL){
				printf("メモリが確保できません。\n");exit(1);
			}
		
			//..クリア
			b_hai((char*)rect_u0.pimg,w2 * tex->height);
			
			
#if 1
			{
			//..逆にコピー
			int j;
			for(j=0; j<tex->height; j++){
				memcpy(rect_u0.pimg+j*w2, p + 54 + (tex->height-1-j)*w2, w2);
			}
	
			}
#else
			memcpy(rect_u0.pimg, p + 54, w2 * tex->height);
#endif
			}

		}
#endif
		//..大きい矩形に貼り付けていく
		paste_bmpimg_semi32(&rect_s,&rect_u0,
			rect_bmp_pos[i].x,rect_bmp_pos[i].y,0);

		free(rect_u0.pimg);

	}
	rect2file32(&rect_s,(u_char*)new_filename);
	
	return(rect_s.pimg);
		
}
