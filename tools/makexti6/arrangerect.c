


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>

#include "my.h"
#include "bmp.h"
#include "arrangerect.h"

#include "MDU_util.h"
#include "MDU_tex.h"
#include "dds.h"
#include "fmt_cm2x.h"
#include "wrapfunc.h"
#include "dxtcmp.h"
#include "tex_utl.h"

#define LIMIT_OVER 1
#define KASANARI   2

#define MAX_W	512
#define MAX_H	512



static RECT_POS* base = NULL;
static int tex_num = 0;		//..読み込む矩形の総数

long bmp_max_width;
long bmp_max_height;
long limit_min_w;
long limit_min_h;
long limit_max_w;
long limit_max_h;


long get_bmp_max_width(void)
{
	return(bmp_max_width);
}

long get_bmp_max_height(void)
{
	return(bmp_max_height);
}


#if 0
//..矩形情報
RECT_BMP_POS rect_bmp_pos[MAX_TEX_NUM];
//..候補地のワーク
RECT_BMP_POS rect_arrange_pos[ARRANGE_NUM];
#else

RECT_POS *rect_bmp_pos;
//..とりあえず静的に確保
RECT_POS rect_arrange_pos[ARRANGE_NUM];

#endif


//..横線　縦線
inline int line_cross(int L1_x1,int L1_x2,int L1_y, int L2_x,int L2_y1,int L2_y2)
{
	if((L1_x1 <= L2_x && L2_x < L1_x2) && (L2_y1 <= L1_y && L1_y < L2_y2)) return 1;

	return 0;
}


//..矩形同士の重なりを調べる
//..重なる		:1
//..重ならない 	:0

#if 0
inline int kasanari(RECT_POS *prmp1,RECT_POS *prmp2)
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
#else
inline int kasanari(RECT_POS *prmp1,RECT_POS *prmp2)
{
	if ( prmp1->x >= ( prmp2->x + prmp2->w ) ) return ( 0 );
	if ( prmp1->y >= ( prmp2->y + prmp2->h ) ) return ( 0 );
	if ( ( prmp1->x + prmp1->w ) <= ( prmp2->x ) ) return ( 0 );
	if ( ( prmp1->y + prmp1->h ) <= ( prmp2->y ) ) return ( 0 );
	return 1;
}
#endif





//..矩形と線、重なっているかどうか判定。縦線と交差
static int kasanari_sen_V(RECT_POS *prmp1,int sx)
{
	
	if(prmp1->x <= sx  &&  sx < prmp1->x + prmp1->w){
		return prmp1->y+prmp1->h;
	}
	return 0;
}

//..矩形と線、重なっているかどうか判定。横線と交差
static int kasanari_sen_H(RECT_POS *prmp1,int sy)
{
	
	if(prmp1->y <= sy  &&  sy < prmp1->y + prmp1->h){
		return prmp1->x+prmp1->w;
	}
	return 0;
}


//..空きのワークを見つけ、番号を返し、埋める
int get_vacant_work(void)
{
	int i;
	for(i=0; i<ARRANGE_NUM; i++){
		if(rect_arrange_pos[i].flag==0){
			rect_arrange_pos[i].flag=1;
			return i;
		}
	}
	printf("空きのワークがありません\n");
	exit(1);
	//return(-1);
}





//..ワーク作成
//..戻り値 確保されたワークのアドレス
RECT_POS* ArrangeRect_CreateWork(int num ,int min, int max){
	
	base = (RECT_POS*)Malloczero( num * sizeof(RECT_POS) );
	tex_num = num;

#if 0	
	bmp_max_width=16;
	bmp_max_height=16;
#else
	bmp_max_width=min;
	bmp_max_height=min;
#endif

	limit_min_w=min;
	limit_min_h=min;
	limit_max_w=max;
	limit_max_h=max;
	

	//..候補地ワーク初期化
	memset(rect_arrange_pos,0,sizeof(RECT_POS)*ARRANGE_NUM);

	
	return(base);

}

int ArrangeRect_FreeWork(void){
	
	if(base)	free(base);
	else {
		printf("Assert\n");
	}

	tex_num = 0;
	return(0);

}




//..ワークにファイル名、テクスチャの大きさ(w,h)を代入していく
void ArrangeRect_InputData2Work(int order,int w,int h, char *filename ){
	RECT_POS* p;
	
	p = base + order;
#if 0
	p->w = w;
	p->h = h;
#else
	/* DXTテクスチャ圧縮を考慮しては位置情報ではサイズを４の倍数にそろえる */
	p->org_w=w;
	p->org_h=h;
	
	p->w = ( w + 3 ) & ~0x3 ;
	p->h = ( h + 3 ) & ~0x3 ;
#endif
	if(p->w > limit_max_w || p->h > limit_max_h){
		printf("ERROR!! 矩形上限より大きなテクスチャです。 %s\n",filename);
		dprint(limit_max_w );dprint(limit_max_h);
		dprint(p->w);dprint(p->h);
		exit(1);
	}
	
	
	ASSERT(sizeof(p->filename) > strlen(filename));
	strncpy(p->filename,filename,sizeof(p->filename));
	
	p->tex_id = MDU_GetStrCode(p->filename);
	
}


static void cluc_area(RECT_POS *prmp,int num)
{
	int i;
	
	for(i=0; i<num; i++,prmp++)
		prmp->area=prmp->w * prmp->h;
	
}

static void swap_RECT_BMP_POS(RECT_POS *prmp1,RECT_POS *prmp2)
{
	RECT_POS tmp;

	memmove(&tmp,prmp1,sizeof(RECT_POS));
	memmove(prmp1,prmp2,sizeof(RECT_POS));
	memmove(prmp2,&tmp,sizeof(RECT_POS));
}

static void bubble_sort_RECT_POS_R(RECT_POS *prmp,int size)
{

	int i,j;
	for(i=0; i<size; i++){
		for(j=size-1; j>i; j--){
			if(prmp[j].area > prmp[j-1].area){
				swap_RECT_BMP_POS(&prmp[j-1],&prmp[j]);
			}
		}
	}
}



static int ArrangeRect_SetPos(int num)
{
	int tx,ty,tmp_area[ARRANGE_NUM],tmp_kasanari[ARRANGE_NUM];
	int i,j,min,max=0,near_area[ARRANGE_NUM],tmp_near=0,best_num=0;
	int n1,n2;
	int		min_x, min_y ;
	RECT_POS t_rbp;
	
	rect_bmp_pos=base;
	
	memset(tmp_area,0,sizeof(int)*ARRANGE_NUM);
	memset(tmp_kasanari,0,sizeof(int)*ARRANGE_NUM);
	memset(near_area,0,sizeof(int)*ARRANGE_NUM);
	min_x = bmp_max_width ;
	min_y = bmp_max_height ;
	best_num = -1 ;

	
	dprintf("%d個目配置\n",num);
	//..num個目配置！！！
	
	
	//-------候補地検索-----------
	for(i=0; i<ARRANGE_NUM; i++){
		
		//..候補地検索
		if(rect_arrange_pos[i].flag){
			
			//printf("候補... %d %d\n", rect_arrange_pos[i].x, rect_arrange_pos[i].y );
			//..候補地に仮配置してみる
			tx=rect_arrange_pos[i].x+rect_bmp_pos[num].w;
			ty=rect_arrange_pos[i].y+rect_bmp_pos[num].h;
			
			{
				//..他の矩形と重ならないかどうかの判定
				tmp_kasanari[i]=0;	//..重なりフラグ
				//..仮配置した矩形
				t_rbp.x=rect_arrange_pos[i].x;
				t_rbp.y=rect_arrange_pos[i].y;
				t_rbp.w=rect_bmp_pos[num].w;
				t_rbp.h=rect_bmp_pos[num].h;
				for(j=0;j<tex_num;j++){
					//..場所が決定している矩形
					//if(rect_bmp_pos[j].flag==1){
					if(rect_bmp_pos[j].atomawasi_flg==KARIKETTEI){
#if 1
						if(kasanari(&t_rbp,&rect_bmp_pos[j])){
							//..重なっていたら抜ける
							tmp_kasanari[i] = KASANARI;
							break;
						}
#endif
					}
				}
			}
			
			//..元の全体矩形のほうが大きい時はそのまま
			//..配置して矩形が小さくなることはないから
			//if(rect_arrange_pos[i].w>=tx)	tx=rect_arrange_pos[i].w;
			//if(rect_arrange_pos[i].h>=ty)	ty=rect_arrange_pos[i].h;
			
			//tmp_area[i]=tx * ty;
			//tmp_area[i] = ty ;
			//if(tmp_area[i]>max) max=tmp_area[i];
			
			//..はみ出し判定　制限越えたらフラグがたつ
			if(tx>bmp_max_width || ty>bmp_max_height){
				//..矩形内からはみ出た
				tmp_kasanari[i]=LIMIT_OVER;
			}

			if ( tmp_kasanari[i] == 0 ){
				if ( rect_arrange_pos[i].y < min_y ){
					best_num = i ;
					min_x = rect_arrange_pos[i].x ;
					min_y = rect_arrange_pos[i].y ;
				} else if ( rect_arrange_pos[i].y == min_y && rect_arrange_pos[i].x < min_x ){
					best_num = i ;
					min_x = rect_arrange_pos[i].x ;
					min_y = rect_arrange_pos[i].y ;
				}
			}

			
		}
	}//..すべての候補地に代入完了

	
	//..残念！、入りきらなかったので矩形を拡大しましょう。
	if(best_num==-1){
		
		//..正方形
		if(bmp_max_width<bmp_max_height && bmp_max_width < limit_max_w ){
			bmp_max_width*=2;
			dprintf("矩形拡大 %d\n",num);
		}else if(bmp_max_height < limit_max_h){
			bmp_max_height*=2;
			dprintf("矩形拡大 %d\n",num);
		}else{
			//..入りきらないのでこのテクスチャは後回し。
			dprintf(" 後回し %d\n",num);
			rect_bmp_pos[num].atomawasi_flg=ATOMAWASI;
			return(1);
		}
		
		dprint(bmp_max_width);
		dprint(bmp_max_height);
		
		return(-1);

	}
	
	//printf("最小の面積の地点.. %d:%d\n",best_num,tmp_area[tmp_num]);
	

	//if(min==0){
	//	printf("ERROR: 最小の面積が０です。\n");
	//	exit(1);
	//}

	//..矩形配置位置決定
	//rect_bmp_pos[num].flag=1;
	rect_bmp_pos[num].x=rect_arrange_pos[best_num].x;
	rect_bmp_pos[num].y=rect_arrange_pos[best_num].y;
	rect_bmp_pos[num].atomawasi_flg=KARIKETTEI;
	//printf("仮配置 %d %d\n", rect_arrange_pos[best_num].x, rect_arrange_pos[best_num].y );
	
	
	/*-------配置終了-------*/
	


	//..配置後の全体の縦横を求める
	tx=rect_arrange_pos[best_num].x+rect_bmp_pos[num].w;
	ty=rect_arrange_pos[best_num].y+rect_bmp_pos[num].h;
	//..元の矩形のほうが大きい時は
	if(rect_arrange_pos[best_num].w>=tx)	tx=rect_arrange_pos[best_num].w;
	if(rect_arrange_pos[best_num].h>=ty)	ty=rect_arrange_pos[best_num].h;

	/* ワーク解放 */
	rect_arrange_pos[best_num].flag = 0 ;
	
	/* 配置した矩形と重なる候補点を削除 */
	for ( i = 0 ; i < ARRANGE_NUM ; i++ ){
		if ( ( rect_arrange_pos[i].x >= rect_bmp_pos[num].x )
			 && ( rect_arrange_pos[i].x < ( rect_bmp_pos[num].x + rect_bmp_pos[num].w ) )
			 && ( rect_arrange_pos[i].y >= rect_bmp_pos[num].y )
			 && ( rect_arrange_pos[i].y < ( rect_bmp_pos[num].y + rect_bmp_pos[num].h ) ) ){
			rect_arrange_pos[i].flag = 0 ;
		}
	}

	/*---次の候補位置算出----*/
	
	//..矩形の右上、左下の候補地点を作成
	n1=get_vacant_work();
	n2=get_vacant_work();

	rect_arrange_pos[n1].x=rect_bmp_pos[num].x+rect_bmp_pos[num].w;
	rect_arrange_pos[n1].y=rect_bmp_pos[num].y;

	rect_arrange_pos[n2].x=rect_bmp_pos[num].x;
	rect_arrange_pos[n2].y=rect_bmp_pos[num].y+rect_bmp_pos[num].h;




#if 1
	//..x軸y軸に候補地を作る
	n1=get_vacant_work();
	n2=get_vacant_work();
	rect_arrange_pos[n1].x=rect_bmp_pos[num].x+rect_bmp_pos[num].w;
	rect_arrange_pos[n1].y=0;
	rect_arrange_pos[n2].x=0;
	rect_arrange_pos[n2].y=rect_bmp_pos[num].y+rect_bmp_pos[num].h;

#endif

	
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
	
	return(0);

}



//..テクスチャ配置
void ArrangeRect_DecidePostion(void){
	
	int i , restart = 1, res, giveup ;
	//..面積計算
	cluc_area(base, tex_num);
	
	//..並び替え
	bubble_sort_RECT_POS_R(base,tex_num);
	

	/* 配置 */
	while ( restart ){

		restart = 0 ;
		memset(rect_arrange_pos,0,sizeof(RECT_POS)*ARRANGE_NUM);
		/* 仮決定がある場合には未定に */
		for(i=0;i<tex_num;i++)	{
			if ( base[i].atomawasi_flg == KARIKETTEI ){
				rect_bmp_pos[i].flag = 0 ;
				base[i].atomawasi_flg = MITEI ;
			}
			//if ( base[i].atomawasi_flg == ATOMAWASI ){
			//	rect_bmp_pos[i].flag = 0 ;
			//	base[i].atomawasi_flg = MITEI ;
			//}
		}
	
		/* 最初の候補地(0,0) */
		rect_arrange_pos[0].flag=1;

		/* 並べる */
		giveup = 1 ;
		for(i=0;i<tex_num;i++)	{
			if ( ( res = ArrangeRect_SetPos(i) ) < 0 ){
				restart = 1 ;
				break ;
			}
			/* 1枚でも配置が出来たら処理は続行可能と見なす */
			if ( res == 0 ){
				giveup = 0 ;
			}
		}
		/* すべてが後回しになってしまった場合はエラーとする */
		if ( ( restart == 0 ) && ( giveup != 0 ) ){
			printf("ERROR: 最小の面積が０です。\n");
			exit(1);
		}
	}
	/* 仮決定を決定に */
	for(i=0;i<tex_num;i++)	{
		if ( base[i].atomawasi_flg == KARIKETTEI ){
			rect_bmp_pos[i].flag = 1 ;
			base[i].atomawasi_flg = KETTEI ;
		}
	}
}





//..配置ワークをもとにTEXPACKヘッダのイメージを作成する
void ArrangeRect_Make_TEXPACK_HeaderImg(void **pimg,int *header_size,int format){
	TEXPACK_HEADER *p_header;
	TEXPACK_INFO *p_info;
	RECT_POS *prp;
	int size,i,cnt=0;
	RGBA	*clut ;
	
	prp=base;
	
	//..配置ワークの中から挿入するべきファイルを数える
	for(i=0; i<tex_num; i++){
		if(prp[i].atomawasi_flg == KETTEI){
			cnt++;
		}
	}
		
	//..ヘッダ + DG_TEX * テクスチャ数 分のメモリを確保
	//size = sizeof(TEXPACK_HEADER) + tex_num * sizeof(TEXPACK_INFO);
	size = sizeof(TEXPACK_HEADER) + cnt * sizeof(TEXPACK_INFO);
	if ( format == TEXPACK_FORMAT_PAL8 ){
		size += sizeof(RGBA) * 256 * cnt ;
	}
	p_header = (TEXPACK_HEADER *)Malloc(size);
	memset( p_header, 0, size );
	
	p_header->file_type = 0;	//..未定
	p_header->width = bmp_max_width;			/* テクスチャ幅 */
	p_header->height= bmp_max_height ;		/* テクスチャ高さ */
	p_header->format= format ;		/* テクスチャフォーマット */
	p_header->flag  = 0;		//..未定	/* 各種フラグ */
	p_header->n_info= cnt ;		/* テクスチャ情報数 */
	p_header->info  = (TEXPACK_INFO*)sizeof(TEXPACK_HEADER);/* テクスチャ情報へのオフセット */
	p_header->texel_addr = (void*)size;	/* テクセルデータへのオフセット */
	p_header->mip_level =0;		/* 最大ミップマップレベル（通常０） */
	switch ( format ){
	case TEXPACK_FORMAT_PAL8:
		p_header->clut_addr = (void*)( sizeof(TEXPACK_HEADER) + cnt * sizeof(TEXPACK_INFO) );
		clut = (RGBA*)( (char*)p_header + (int)p_header->clut_addr );
		break ;
	default:
		clut = NULL ;
		break ;
	}
	
	p_info = (TEXPACK_INFO *)(p_header+1);
	
	for(i=0; i<tex_num; i++){
		if(prp[i].atomawasi_flg == KETTEI){
char *c ;
			p_info->id = prp[i].tex_id;	/* 24bit strcode ID */
			
			p_info->flag = 0;			/* 各種フラグ */

	c=strstr( prp[i].filename,"_mod");
			if( c && 
(*(c+4)>='0'&&*(c+4)<='9') &&
(*(c+5)>='0'&&*(c+5)<='9') &&
(*(c+6)>='0'&&*(c+6)<='9') &&
(*(c+7)>='0'&&*(c+7)<='9') ){	/* 都合により一番最初にチェック */
				extern unsigned int GetModFlg(char *filename);
				p_info->flag |= TEXINFO_FLAG_BLEND_AMODE;
				p_info->flag |= GetModFlg(prp[i].filename);
			}
			else if( strstr( prp[i].filename,"_add")){
				p_info->flag |= TEXINFO_FLAG_BLEND_ADD;
			}
			else if( strstr( prp[i].filename,"_sub")){
				p_info->flag |= TEXINFO_FLAG_BLEND_SUB;
			}
			else if( strstr( prp[i].filename,"_hlf")){
				p_info->flag |= TEXINFO_FLAG_BLEND_HLF;
			}
			else
			{

				if( strstr( prp[i].filename,"_bld")){
				
					p_info->flag |= TEXINFO_FLAG_BLEND_HLF;
				}

				/*
				if( strstr( prp[i].filename,"_ovl")){
					//??
					//p_info->flag |= ;
				}*/

				if( strstr( prp[i].filename,"_alp")){
					p_info->flag |= TEXINFO_FLAG_BLEND_HLF;
				}
				
				if( strstr( prp[i].filename,"_alpx2")){
					p_info->flag |= TEXINFO_FLAG_BLEND_HLF;
				}
			}

			if( strstr( prp[i].filename,"_decal")){
				p_info->flag |= TEXINFO_FLAG_DECAL;
			}
			
			if( strstr( prp[i].filename,"_bmap")){
				p_info->flag |= TEXINFO_FLAG_BMAP;
			}
			else if( strstr( prp[i].filename,"_emap")){
				p_info->flag |= TEXINFO_FLAG_EMAP;
			}
			else if( strstr( prp[i].filename,"_smap")){
				p_info->flag |= TEXINFO_FLAG_SMAP;
			}

	
			p_info->x = prp[i].x;
			p_info->y = prp[i].y;		/* テクスチャ配置座標 */
#if 0
			p_info->w = prp[i].w;
			p_info->h = prp[i].h;		/* テクスチャサイズ */
#else
			p_info->w = prp[i].org_w;
			p_info->h = prp[i].org_h;		/* テクスチャサイズ */
#endif
			/* ＣＬＵＴデータのコピー */
			if ( clut != NULL ){
				p_info->clut_addr = (void*)( (int)clut - (int)p_header ) ;	/* パレットアドレス */
				memcpy( clut, prp[i].clut, sizeof(RGBA) * 256 );
				clut += 256 ;
			} else {
				p_info->clut_addr = 0;		/* パレットアドレス */
			}
			
			p_info++;

		}
	}
	
	*pimg=p_header;
	*header_size = size;

}





//..ワークをダンプ
void ArrangeRect_dump(void){
	
	int i;
	RECT_POS *p;
	p=base;
	
	for(i=0; i<tex_num; i++,p++){
		dprint(i);
		dprints(p->filename);
		//dprint(p->tex_id);
		dprintx(p->flag);
		dprint(p->x);
		dprint(p->y);
		dprint(p->w);
		dprint(p->h);
		//dprint(p->area);
		dprint(p->atomawasi_flg);
	}
	dprint(tex_num);
	dprint(bmp_max_width);
	dprint(bmp_max_height);
	printf("\n");
}




//..ワークを元に一体化テクスチャイメージを作成する
void* ArrangeRect_MakeImg32bit(void **p_tex_img,int *img_size){

	int i;
	RECT2 rect_s,rect_u0;
	
	//..一体化テクスチャ確保
	RECT2Create(&rect_s, bmp_max_width, bmp_max_height, 4);
	
	for(i=0; i<tex_num; i++){
		MDU_Tex *tex;
		BYTE *p;
		
		//..後回しなら
		if(rect_bmp_pos[i].atomawasi_flg == ATOMAWASI){
			continue;
		}
		
		
        tex = (MDU_Tex *)LoadTex((char*)rect_bmp_pos[i].filename);
		if (tex == NULL) {
			printf("file \"%s\" : LoadTex failed. %s(%d)\n", 
				rect_bmp_pos[i].filename,__FILE__,__LINE__);
			continue;
		}
		/* CLUT付きで読み込まれたテクスチャを32bitに変換する */
		MakeFullColorTex(tex);
		/* 32bit/または24bitのBMPに変換する */
		p = (char*)MakeFullColorBmp(tex, 32);

		RECT2Create(&rect_u0, tex->width, tex->height, 4);
		memcpy(rect_u0.pimg, p + 54, rect_u0.buffer_size);
		RECT2UpsideDown( &rect_u0 );
		
		/* 実際に変換するデータはすでにtexconvにてαが設定されているのでなにもする必要なし */
#if 0
		if ( strstr( rect_bmp_pos[i].filename, "_msk" ) ){
			RECT2PenetratetionCheckRGBA(&rect_u0);
		}
#endif
		
		//..大きい矩形に貼り付けていく
		paste_bmpimg_semi32(&rect_s,&rect_u0,
			rect_bmp_pos[i].x,rect_bmp_pos[i].y,0);
		
		Free(rect_u0.pimg);
	}
	
	//RectImg4Outputfile(&rect_s,"bmp32.bmp");
	
	*p_tex_img=rect_s.pimg;
	*img_size=rect_s.buffer_size;
	
	return(p_tex_img);

}

//..ワークを元に一体化テクスチャイメージを作成する
void* ArrangeRect_MakeImg24bit(void **p_tex_img,int *img_size){

	int i;
	RECT2 rect_s,rect_u0;
	
	//..一体化テクスチャ確保
	RECT2Create(&rect_s, bmp_max_width, bmp_max_height, 3);
	
	for(i=0; i<tex_num; i++){
		MDU_Tex *tex;
		BYTE *p;

		//..後回しなら
		if(rect_bmp_pos[i].atomawasi_flg == ATOMAWASI){
			continue;
		}

        tex = (MDU_Tex *)LoadTex((char*)rect_bmp_pos[i].filename);
		if (tex == NULL) {
			printf("file \"%s\" : LoadTex failed. %s(%d)\n", 
				rect_bmp_pos[i].filename,__FILE__,__LINE__);
			continue;
		}
		/* CLUT付きで読み込まれたテクスチャを32bitに変換する */
		MakeFullColorTex(tex);
		/* 32bit/または24bitのBMPに変換する */
		p = (char*)MakeFullColorBmp(tex, 24);

		RECT2Create(&rect_u0, tex->width, tex->height, 3);
		memcpy(rect_u0.pimg, p + 54, rect_u0.buffer_size);
		RECT2UpsideDown( &rect_u0 );

		//..大きい矩形に貼り付けていく
		paste_bmpimg_semi2(&rect_s,&rect_u0,
			rect_bmp_pos[i].x,rect_bmp_pos[i].y,0);
		
		Free(rect_u0.pimg);
	}

	//RectImg4Outputfile(&rect_s,"bmp24.bmp");

	*p_tex_img=rect_s.pimg;
	*img_size=rect_s.buffer_size;
	
	return(p_tex_img);

}


//..配置ワークを元に一体化テクスチャイメージを作成する
//..確保したワークは解放しないので注意
void* ArrangeRect_MakeImgDXTn(void **p_tex_img,int *img_size,int DXTn){

	int i;
	int	compress_size ;
	void *pcmpimg;

	RECT2 rect_s,rect_u0;
	
	//..一体化テクスチャ確保 32bit
	RECT2Create(&rect_s, bmp_max_width, bmp_max_height, 4);
	
	for(i=0; i<tex_num; i++){
		MDU_Tex *tex;
		BYTE *p;

		//..後回しなら
		if(rect_bmp_pos[i].atomawasi_flg == ATOMAWASI){
			continue;
		}

        tex = (MDU_Tex *)LoadTex((char*)rect_bmp_pos[i].filename);
		if (tex == NULL) {
			printf("file \"%s\" : LoadTex failed. %s(%d)\n", 
				rect_bmp_pos[i].filename,__FILE__,__LINE__);
			continue;
		}
		/* CLUT付きで読み込まれたテクスチャを32bitに変換する */
		MakeFullColorTex(tex);
		/* 32bit/または24bitのBMPに変換する */
		p = (char*)MakeFullColorBmp(tex, 32);

		RECT2Create(&rect_u0, tex->width, tex->height, 4);
		memcpy(rect_u0.pimg, p + 54, rect_u0.buffer_size);
		RECT2UpsideDown( &rect_u0 );
		
		//RECT2CheckRGBA(&rect_u0);
		
		//..大きい矩形に貼り付けていく
		paste_bmpimg_semi32(&rect_s,&rect_u0,
			rect_bmp_pos[i].x,rect_bmp_pos[i].y,0);
		
		Free(rect_u0.pimg);
	}

	//..bmp→DXTn
	if(DXTn == D3DFMT_DXT1){
		CompressImageDXT1((u_int*)rect_s.pimg, bmp_max_width, bmp_max_height, &pcmpimg, &compress_size );
	}else if(DXTn == D3DFMT_DXT3){
		CompressImageDXT3((u_int*)rect_s.pimg, bmp_max_width, bmp_max_height, &pcmpimg, &compress_size );
	}else if(DXTn == D3DFMT_DXT5){
		CompressImageDXT5((u_int*)rect_s.pimg, bmp_max_width, bmp_max_height, &pcmpimg, &compress_size );
	}else{
		ASSERT(!"Invalid DXTn\n");
	}
	
	*p_tex_img=pcmpimg;
	*img_size=compress_size;
	
	//return(p_tex_img);
	return(rect_s.pimg);

}

//..ワークを元に一体化テクスチャイメージを作成する
void* ArrangeRect_MakeImg8bit(void **p_tex_img,int *img_size){

	int i;
	RECT2 rect_s,rect_u0;
	
	/* 一体化テクスチャ確保（３２ビットとして確保） */
	RECT2Create(&rect_s, bmp_max_width, bmp_max_height, 4 );
	
	for(i=0; i<tex_num; i++){
		MDU_Tex *tex;
		BYTE *p;
		
		//..後回しなら
		if(rect_bmp_pos[i].atomawasi_flg == ATOMAWASI){
			continue;
		}
		
		
        tex = (MDU_Tex *)LoadTex((char*)rect_bmp_pos[i].filename);
		if (tex == NULL) {
			printf("file \"%s\" : LoadTex failed. %s(%d)\n", 
				rect_bmp_pos[i].filename,__FILE__,__LINE__);
			continue;
		}
		/* CLUT付きで読み込まれたテクスチャのαをPS２対応に変更 */
		MakeColorTex(tex);
		/* CLUTデータをコピー */
		memcpy( rect_bmp_pos[i].clut, tex->clut.data, sizeof(RGBA) * 256 );
		/* 32bit/または24bitのBMPに変換する */
		p = (char*)MakeFullColorBmp(tex, 32);

		RECT2Create(&rect_u0, tex->width, tex->height, 4);
		memcpy(rect_u0.pimg, p + 54, rect_u0.buffer_size);
		RECT2UpsideDown( &rect_u0 );
		
		/* 実際に変換するデータはすでにtexconvにてαが設定されているのでなにもする必要なし */
#if 0
		if ( strstr( rect_bmp_pos[i].filename, "_msk" ) ){
			RECT2PenetratetionCheckRGBA(&rect_u0);
		}
#endif
		
		//..大きい矩形に貼り付けていく
		paste_bmpimg_semi32(&rect_s,&rect_u0,
			rect_bmp_pos[i].x,rect_bmp_pos[i].y,0);
		
		Free(rect_u0.pimg);
	}
	
	//RectImg4Outputfile(&rect_s,"bmp32.bmp");
	
	*p_tex_img=rect_s.pimg;
	*img_size=rect_s.buffer_size;
	
	return(p_tex_img);

}

void *CompressImagePAL8( unsigned int *image, int width, int height, void **compress_buff, int *compress_size )
{
	unsigned char	*data, *src, *dst ;
	int				x, y ;

	*compress_size = width * height ;
	data = malloc( *compress_size );
	memset( data, 0, *compress_size );
	if ( compress_buff != NULL ) *compress_buff = data ;

	/* １ピクセル４バイトで記録してるインデックスを１バイトに詰める */
	src = image ;
	dst = data ;
	for ( y = 0 ; y < height ; y++ ){
		for ( x = 0 ; x < width ; x++ ){
			*dst++ = src[0] ;
			src += 4 ;
		}
	}
	return ( data );
}


//..後回しのテクスチャを処理する
int ArrangeRect_Nokori(void)
{
	int i,cnt=0,n_cnt;
	RECT_POS* prect = NULL;
	
	//..後回しを数える
	for(i=0; i<tex_num; i++){
		if(rect_bmp_pos[i].atomawasi_flg == ATOMAWASI){
			cnt++;
		}
	}
	if(cnt == 0)	return(0);

	prect = (RECT_POS*)Malloczero( cnt * sizeof(RECT_POS) );
	
	//..配置ワーク再作成
	n_cnt=0;
	for(i=0; i<tex_num; i++){
		if(rect_bmp_pos[i].atomawasi_flg == ATOMAWASI){
			memcpy(&prect[n_cnt], &rect_bmp_pos[i], sizeof(RECT_POS));
			prect[n_cnt].atomawasi_flg=MITEI;
			n_cnt++;
		}
	}
	
	tex_num = cnt;
	
	//..いったん解放
	if(base)	free(base);
	else {
		ASSERT(base);
	}
	
	base = prect;

	//..候補地ワーク初期化
	memset(rect_arrange_pos,0,sizeof(RECT_POS)*ARRANGE_NUM);

	bmp_max_width= limit_min_w;
	bmp_max_height=limit_min_h;


	return(tex_num);


}


