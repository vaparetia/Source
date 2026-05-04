//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	homsight2.c
	ホーミングカーソル
	2002/05/30 K.Sigeno
	$Id: homsight2.c,v 1.1.1.3 2002/11/19 11:49:47 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif


#include	"gameheader.h"
#include	"libutl.h"
//#include	"vr.h"
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

//テクスチャ
#define	HOM_TAG01		(15045893)	/*lockon_tag1_add_alp*/
#define	HOM_NAME_OCT	(11899535)	/*lockon_octa_add_alp*/

#define UWAGAKI (SCE_GS_SET_ALPHA(2,2,0,0,0))
#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))
#define KASAN2 (SCE_GS_SET_ALPHA(0,2,2,1,64))
#define ALPHA_HALF (SCE_GS_SET_ALPHA(0,1,0,1,0x80))


//#define LEN1		(1000.0f)		/*内周半径*/
#define LEN1		(750.0f)		/*内周半径*/

#define LEN2		(6000.0f)		/*軸線長さ*/
#define	START_DIF	(10000.0f)		/*軸線スタート位置*/
#define	FIX_LINE_TIME	DIRECT_TICK(12)		/*軸線到達時間*/
#define	LINE_NUM		(6)
#define	SPEED_01	(START_DIF/(float)FIX_LINE_TIME)	/*軸線速度*/
#define	NAME_NUM	(4)
#define NAME_DIV	(4) //必ず偶数
#define NAME_RATE	(0.7f) 
#define NAME_TIME	DIRECT_TICK(8) 

#define	NAME_H		(80.0f)
//#define	CIRCLE_DEV	(3)
#define	CIRCLE_DEV	(8) //必ず偶数
#define	CIRCLE_TIME	DIRECT_TICK(12)
#define	CIRCLE_RATE	(0.3f) 

#define	N_POLY_VERTS	(2+NAME_DIV*2)
#define	TAG_NUM	(3)
#define	N_TAG_POLY_VERTS	(4)

//#define PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_TEX)
#define PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA)
#define POLY_FLAG	(DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_TEX)
//#define POLY_FLAG	(DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_TEX|DG_PRIM_SORTONLY)

#define N_VERTS ((LINE_NUM*2+(CIRCLE_DEV+1)*NAME_NUM))


typedef	struct	{
	GV_ACT_EX	actor ;
	DG_PRIM2	*prim ;
	DG_PRIM2	*name ;
	DG_PRIM2	*tag ;
	FVECTOR	*pos ;
	FMATRIX	*world ;

	float hankei ;
	float y_len ;
	float y_low ;
	int		*flag ;
	int			cnt;

	short		tag_tex_w;
	short		tag_tex_h ;

} Work ;

/* プリミティブ座標設定関数*/
static void SetCirclePrim(Work *work){
	int j;
	float len,dir ;
	FVECTOR *pos ;
	pos = work->prim->pos[work->prim->buffer_clock] ;
	pos +=LINE_NUM*2 ;
	if(work->cnt < FIX_LINE_TIME ){
		len = 0.0f ;
	}else if(work->cnt < (CIRCLE_TIME+FIX_LINE_TIME)){
		len = (float)(work->cnt - FIX_LINE_TIME) /(float)CIRCLE_TIME ;
	}else {
		len = 1.0f;
	}
	len *= CIRCLE_RATE ;
	for(j=0;j<((CIRCLE_DEV+1)*NAME_NUM);j++){
#if 0
		/*一周*/
		int index ;
		index = (j%(CIRCLE_DEV+1)) +(j/(CIRCLE_DEV+1))*(CIRCLE_DEV) ;
		dir = ( float )M_PI * ((float)index/(float)(CIRCLE_DEV*NAME_NUM)) * 2.0f ;
		dir *= len ;
#endif
#if 0
		/*分割点から右回り*/
		float index ;
		index = ((float)(j%(CIRCLE_DEV+1))*len) +(float)((j/(CIRCLE_DEV+1))*(CIRCLE_DEV)) ;
		dir = ( float )M_PI * (index/(float)(CIRCLE_DEV*NAME_NUM)) * 2.0f ;
#endif
#if 1
		/*分割点から左右展開*/
		float index ;
//		index = ((float)(j%(CIRCLE_DEV+1))*len) +(float)((j/(CIRCLE_DEV+1))*(CIRCLE_DEV)) ;
		index = (float)((j/(CIRCLE_DEV+1))*(CIRCLE_DEV)) + (float)((j%(CIRCLE_DEV+1))-(CIRCLE_DEV/2)) * len ;

		dir = ( float )M_PI * (index/(float)(CIRCLE_DEV*NAME_NUM)) * 2.0f ;
#endif
		pos->vx = work->hankei*cosf(dir) ;
		pos->vz = work->hankei*sinf(dir) ;
		pos++;
	}
}


static void SetNamePrim(Work *work){
	int i,j,k;
	FVECTOR *pos ;
	float len_w,len_h ;
//NAME_TIME
	if(work->cnt < (CIRCLE_TIME+FIX_LINE_TIME) ){
		len_h = 0.0f ;
	}else if(work->cnt < (CIRCLE_TIME+FIX_LINE_TIME+NAME_TIME)){
		len_h = (float)(work->cnt - (CIRCLE_TIME+FIX_LINE_TIME)) /(float)NAME_TIME ;
	}else {
		len_h = 1.0f;
	}
//	len_w = len_h*NAME_RATE ;
	len_w = NAME_RATE ;

//	for(i= 0 ;i<2 ;i++){
	(i= work->prim->buffer_clock) ;
	{

		for(k= 0 ;k<NAME_NUM ;k++){
			pos = work->name->pos[i] ;
			pos += k*N_POLY_VERTS ;
			for (j= 0 ; j<(N_POLY_VERTS) ;j++){
		/*分割点から左右展開*/
				float dir;
#if 0
/*一周*/
				dir = (float)M_PI*2.0f*(((float)(j/2)/(float)(NAME_DIV*NAME_NUM))+((float)k/(float)NAME_NUM) );
#else
				dir = (float)M_PI*2.0f*
					(
					((float)((j/2)-(NAME_DIV/2))*len_w/(float)(NAME_DIV*NAME_NUM))
					+((float)k/(float)NAME_NUM) 
//					+0.25f
					+0.125f
					);
#endif
				pos->vx = work->hankei*cosf(-dir) ;
//				pos->vy = -(j&1)*1.0F * (float)tex_h ;
				pos->vy = -((float)(j&1)-0.5f)*1.0F * (float)NAME_H *len_h;
				pos->vz = work->hankei*sinf(-dir) ;
				pos++;
			}
		}
	}
}
static void SetTagPrim(Work *work){
	int i,j,k;
	FVECTOR *pos ;
	float len_w,len_h,len1,len2 ;
	len1 = 600.0f;
	len2 = 1000.0f;

//NAME_TIME
	if(work->cnt < (FIX_LINE_TIME) ){
		len_h = 0.0f ;
	}else if(work->cnt < (CIRCLE_TIME+FIX_LINE_TIME)){
		len_h = (float)(work->cnt - (FIX_LINE_TIME)) /(float)CIRCLE_TIME ;
	}else {
		len_h = 1.0f;
	}
//	len_w = len_h*NAME_RATE ;
	len_w = NAME_RATE ;
//	for(i= 0 ;i<2 ;i++){
	(i= work->tag->buffer_clock);
	{

		for(k= 0 ;k<TAG_NUM ;k++){
			pos = work->tag->pos[i] ;
			pos += k*N_TAG_POLY_VERTS ;
			for (j= 0 ; j<(N_TAG_POLY_VERTS) ;j++){
				SVECTOR rot = DG_ZeroSVector ;
		/*分割点から左右展開*/
#if 0
				float dir;
				dir = (float)M_PI*2.0f*((float)k/(float)NAME_NUM) ;
				len1 = work->hankei ;
				if(j/2) len1 += work->tag_tex_w ;
				pos->vx = len1*cosf(-dir) ;
				pos->vy = -((float)(j&1))*work->tag_tex_h * (float)len_h - 20.0f;
				pos->vz = len1*sinf(-dir) ;
				pos++;
#else
//				float dir;
				len1 = work->hankei ;

				if(k==2){
					/*縦ラインの位置は別指定*/
					len1 = work->y_len ;
				}

				if(j/2) len1 += work->tag_tex_w ;
				pos->vx = len1*cosf(0) ;
				pos->vy = -((float)(j&1))*work->tag_tex_h * (float)len_h - 20.0f;
				pos->vz = len1*sinf(0) ;
				switch(k){
					case 0 :
						break ;
					case 1 :
						rot.vy = 3072 ;
						break ;
					case 2 :
						rot.vz = 1024 ;
						break ;
				}
				DG_SetPos2( &DG_ZeroVector, &rot) ;
				DG_PutVector( pos, pos, 1 ) ;
				pos++;
#endif
			}
		}
	}
}
static void InitPacket( Work *work){
	int		i,j,k;
	FVECTOR		*pos;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_TEX		*name_tex ,*line_tex ;

	float	len1,len2,y_len1,y_len2;
	prim = work->prim ;
   prim->as_world = *work->world ;

   work->name->as_world = *work->world ;


//HOM_TAG01
//HOM_NAME_OCT 
	/* テクスチャ取得 */
	line_tex = DG_GetTexture( 8617368 );
	name_tex = DG_GetTexture( HOM_NAME_OCT );
	DG_ConfigPrim2Tex( work->name, name_tex );
	DG_ConfigPrim2Tex( work->prim, line_tex );

	DG_InvisiblePrim2(work->prim) ;
	DG_InvisiblePrim2(work->name) ;
	DG_InvisiblePrim2(work->tag) ;

	DG_SetPrim2Alpha( prim, KASAN );
	DG_SetPrim2Alpha( work->name, KASAN );
	DG_SetPrim2Alpha( work->tag, KASAN );

	len1 = work->hankei + START_DIF ;
	len2 = len1 + LEN2 ;

	y_len1 = work->y_len + START_DIF ;
	y_len2 = y_len1 + LEN2  ;

	for(i= 0 ;i<2;i++){
		uvrgb = (DG_PRIM2_UVRGB *)prim->uvrgb[i] ;

		for (j= 0 ; j<N_VERTS ;j++){
			/*線分は１頂点ごとに描画*/
			if(j<(LINE_NUM*2))
			{
				if ( (j&1) == 0 ){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
					uvrgb->r  = 0;
				} else {
					uvrgb->f = 0x0fff ;	/* 描画キックを行う */
					uvrgb->r  = 127;
				}
				uvrgb->a  = 128;
				uvrgb->g  = 0;
				uvrgb->b  = 0;
//				uvrgb->a  = 127;
				uvrgb->q = 4096 ;
				/*UV値*/
				uvrgb->u = FTOI12( 0.0F * line_tex->u_scale + line_tex->u_offset ) ;
				uvrgb->v = FTOI12( 0.0F * line_tex->v_scale + line_tex->v_offset ) ;
			}else {
				/*円周部はネーム表示部ごとに分割*/

//NAME_NUM	(4)
//CIRCLE_DEV	(8)
				if ( ((j-(LINE_NUM*2))%(CIRCLE_DEV+1)) == 0 ){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
				} else {
					uvrgb->f = 0x0fff ;	/* 描画キックを行う */
				}
				uvrgb->r  = 127;
				uvrgb->g  = 0;
				uvrgb->b  = 0;
				uvrgb->a  = 127;
			}
			uvrgb++ ;
		}

		pos = prim->pos[i] ;
		for (j= 0 ; j<(N_VERTS) ;j++){
//			pos = prim->pos[i] ;
			*pos = DG_ZeroVector ;
			pos->vw = 1.0f ;
			pos++;
		}
		/*線*/
		pos = prim->pos[i] ;
		pos->vy = y_len2 ;
		pos++;
		pos->vy = y_len1 ;
		pos++;
		pos->vx = -len2 ;
		pos++;
		pos->vx = -len1 ;
		pos++;
		pos->vx = len2 ;
		pos++;
		pos->vx = len1 ;
		pos++;

		pos->vy = - (work->y_len + START_DIF + LEN2) ;
		pos++;
		pos->vy = - (work->y_len + START_DIF) ;
//	y_len1 = work->y_len + START_DIF ;
//	y_len2 = y_len1 + LEN2  ;

		pos++;
		pos->vz = -len2 ;
		pos++;
		pos->vz = -len1 ;
		pos++;
		pos->vz = len2 ;
		pos++;
		pos->vz = len1 ;
		pos++;
	}
	/*ネーム部分*/
	for(i= 0 ;i<2;i++){
		short tex_w,tex_h ;
		for(k= 0 ; k<NAME_NUM;k++){
			uvrgb = (DG_PRIM2_UVRGB *)work->name->uvrgb[i] ;
			uvrgb += k*N_POLY_VERTS ;
			tex_w = FTOI12( name_tex->u_scale ) ;
			tex_h = FTOI12( name_tex->v_scale ) ;
			for (j= 0 ; j<N_POLY_VERTS ;j++){
				uvrgb->r  = 127;
				uvrgb->g  = 127;
				uvrgb->b  = 127;
				uvrgb->a  = 64;
				uvrgb->q = 4096 ;
				/*UV値*/
				uvrgb->u = FTOI12( ((j/2)/(float)NAME_DIV) * name_tex->u_scale + name_tex->u_offset ) ;
				uvrgb->v = FTOI12( (j&1)*1.0F * name_tex->v_scale + name_tex->v_offset ) ;
				/*描画フラグ*/
				if ( j == 0 ){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
				} else {
					uvrgb->f = 0x0fff ;	/* 描画キックを行う */
				}
				uvrgb++ ;
			}
			pos = work->name->pos[i] ;
			pos += k*N_POLY_VERTS ;
			for (j= 0 ; j<(N_POLY_VERTS) ;j++){
				*pos = DG_ZeroVector ;
				pos->vw = 1.0f ;
				pos++;
			}
		}
	}
	name_tex = DG_GetTexture( HOM_TAG01 );
	DG_ConfigPrim2Tex( work->tag, name_tex );
	for(i= 0 ;i<2;i++){
//		short tex_w,tex_h ;
		for(k= 0 ; k<TAG_NUM;k++){
			uvrgb = (DG_PRIM2_UVRGB *)work->tag->uvrgb[i] ;
			uvrgb += k*N_TAG_POLY_VERTS ;
			work->tag_tex_w = FTOI12( name_tex->u_scale ) ;
			work->tag_tex_h = FTOI12( name_tex->v_scale ) ;
			for (j= 0 ; j<N_TAG_POLY_VERTS ;j++){
				uvrgb->r  = 127;
				uvrgb->g  = 127;
				uvrgb->b  = 127;
				uvrgb->a  = 64;
				uvrgb->q = 4096 ;
				/*UV値*/
//				uvrgb->u = FTOI12( ((float)(j/2)) * name_tex->u_scale + name_tex->u_offset ) ;
//				uvrgb->v = FTOI12( (j&1)*1.0F * name_tex->v_scale + name_tex->v_offset ) ;
				if(j<2){
					uvrgb->u = FTOI12( 0.0f* name_tex->u_scale + name_tex->u_offset ) ;
				}else {
					uvrgb->u = FTOI12( 1.0f * name_tex->u_scale + name_tex->u_offset ) ;
				}
				uvrgb->v = FTOI12( (j&1)*1.0F * name_tex->v_scale + name_tex->v_offset ) ;
				/*描画フラグ*/
				if ( j == 0 ){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
				} else {
					uvrgb->f = 0x0fff ;	/* 描画キックを行う */
				}
				uvrgb++ ;
			}
			pos = work->tag->pos[i] ;
			pos += k*N_TAG_POLY_VERTS ;
			for (j= 0 ; j<(N_TAG_POLY_VERTS) ;j++){
				*pos = DG_ZeroVector ;
				pos->vw = 1.0f ;
				pos++;
			}
		}
	}
}

static void InitPos( Work *work){
//extern void PosBox(FVECTOR * ,float ,SVECTOR * );
	int		i;
	FVECTOR		*pos;
	DG_PRIM2	*prim ;
	FVECTOR	tmp ;
	float len1 ,len2 , y_len1, y_len2 , line_dis01;

	GV_MatToVec( work->world, &tmp ) ;

	DG_SetPos2( &tmp, &DG_ZeroSVector) ;
	DG_GetPos(&work->prim->as_world ) ;
	DG_GetPos(&work->name->as_world ) ;
	DG_GetPos(&work->tag->as_world ) ;

	prim = work->prim ;

	line_dis01 = (START_DIF - (float)work->cnt*SPEED_01) ;
	if(line_dis01< 0.0f) line_dis01 = 0.0f ;
	len1 = work->hankei + line_dis01 ;
	len2 = len1 + LEN2 ;


//printf("work->y_len[%f]\n",work->y_len);
//printf("line_dis01[%f]\n",line_dis01);
	y_len1 = work->y_len + line_dis01 ;
	y_len2 = y_len1 + LEN2  ;
	
//	for(i= 0 ;i<2;i++){
	(i= work->prim->buffer_clock);
	{
#if 0
		for (j= 0 ; j<(N_VERTS) ;j++){
			pos = prim->pos[i] ;
			*pos = DG_ZeroVector ;
			pos->vw = 1.0f ;
			pos++;
		}
#endif
		pos = prim->pos[i] ;
//PosBox(pos ,250.0f ,NULL );
//printf("y_len2[%f]\n",y_len2);
		pos->vy = y_len2;
		pos++;
//PosBox(pos ,250.0f ,NULL );
//printf("y_len1[%f]\n",y_len1);
		pos->vy = y_len1 ;
		pos++;
		pos->vx = -len2 ;
		pos++;
		pos->vx = -len1 ;
		pos++;
		pos->vx = len2 ;
		pos++;
		pos->vx = len1 ;
		pos++;

		pos->vy = -(work->y_low + line_dis01 + LEN2) ;
		pos++;
		pos->vy = -(work->y_low + line_dis01) ;
		pos++;
		pos->vz = -len2 ;
		pos++;
		pos->vz = -len1 ;
		pos++;
		pos->vz = len2 ;
		pos++;
		pos->vz = len1 ;
		pos++;
	}
	SetCirclePrim(work) ;
	SetNamePrim(work) ;
	SetTagPrim(work) ;
}

static	void	Act( work )
Work		*work ;
{
//	int model, i ;

	if(*work->flag == 0){
		work->cnt = 0;
		DG_InvisiblePrim2(work->prim) ;
		DG_InvisiblePrim2(work->name) ;
		DG_InvisiblePrim2(work->tag) ;

//		GV_DestroyActor( work ) ;
		return ;
	}else {
		DG_VisiblePrim2(work->prim) ;
		DG_VisiblePrim2(work->name) ;
		DG_VisiblePrim2(work->tag) ;
	}
	InitPos(work) ;
	if(work->cnt == 0){
		GM_SeSetMode(SD_A_V_LOCKST,(FVECTOR *)&work->world->m[3],GM_SEMODE_BOMB) ;
	}
	if(work->cnt == FIX_LINE_TIME){
		GM_SeSetMode(SD_A_V_LOCKON,(FVECTOR *)&work->world->m[3],GM_SEMODE_BOMB) ;
	}

	if(work->cnt < DIRECT_TICK(600)){
		work->cnt++;
	}
}

static	void	Die( work )
Work		*work ;
{
	DG_DequeuePrim2( work->prim );
	DG_FreePrim2( work->prim );
	DG_DequeuePrim2( work->name );
	DG_FreePrim2( work->name );
	DG_DequeuePrim2( work->tag );
	DG_FreePrim2( work->tag );
}




/*----------------------------------------------------------------*/
static	int	GetResources( Work	*work ,FMATRIX *world,int	*flag,float hankei ,float y_len,float y_low){

	work->cnt = 0 ;
	work->flag = flag ;
	work->world = world ;
	work->hankei = hankei ;
	work->y_len = y_len ;
	work->y_low = y_low ;

	work->prim = GM_MakePrim2( PRIM_FLAG, 1, N_VERTS );
	work->name = GM_MakePrim2( POLY_FLAG, NAME_NUM, N_POLY_VERTS );
	work->tag = GM_MakePrim2( POLY_FLAG, TAG_NUM, N_TAG_POLY_VERTS );

   work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
   work->name->flag |= DG_PRIM_AS_CUSTOMWORLD;
   work->tag->flag |= DG_PRIM_AS_CUSTOMWORLD;


	InitPacket( work );

	return 0 ;
}
void	*NewHomingSight2( int *flag,FMATRIX *world,float hankei ,float y_len,float y_low)
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, world,flag,hankei , y_len,y_low) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

