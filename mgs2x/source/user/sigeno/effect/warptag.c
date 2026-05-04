//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	warptag.c
	VR 光降る
	2002/04/19 K.Sigeno
	$Id: warptag.c,v 1.1.1.3 2002/11/19 11:49:50 Yoshizawa1 Exp $
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


#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

//テクスチャ
#define	HOM_TAG01		(15045893)	/*lockon_tag1_add_alp*/
#define	HOM_NAME_OCT	(11899535)	/*lockon_octa_add_alp*/
//(A-B)*C>>7+D
#define UWAGAKI (SCE_GS_SET_ALPHA(2,2,0,0,0))
#define KASAN (SCE_GS_SET_ALPHA(0,2,0,1,0))
#define KASAN2 (SCE_GS_SET_ALPHA(0,2,2,1,64))
#define ALPHA_HALF (SCE_GS_SET_ALPHA(0,1,0,1,0x80))
#define GENSAN (SCE_GS_SET_ALPHA(2,0,0,1,0))
#define NEGA	(SCE_GS_SET_ALPHA(0,1,2,2,128))	//FIX 0
//#define NEGA	(SCE_GS_SET_ALPHA(0,1,2,1,60))	//FIX フレーム
//#define PRIM_RGB	(255)
//#define PRIM_RGB	(128)
#define PRIM_RGB	(200)


//#define	CIRCLE_DIV	(16)
#define	CIRCLE_DIV	(32)

#define N_POLY		(1)
#define N_VERTS		((CIRCLE_DIV+1)*2)


//#define N_FUTA			(1)
#define N_FUTA			(2)

#define N_FUTA_VERTS	(CIRCLE_DIV+1)

#define N_FUTA_VERTS_HALF	(CIRCLE_DIV*2+1)

//#define PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_ALPHA|DG_PRIM2_TEX)
//|DG_PRIM2_SHADE
#define PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA)
#define POLY_FLAG	(DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_TEX)

enum {
	MODE_X = 0 ,
	MODE_Y ,
} ;
//#define		RAISE_Y (0.0f)
//#define		RAISE_Y (+20.0f)
#define		RAISE_Y (+20.0f)
#define		RAISE_DRAW (+150.0f)

//DG_PRIM2_CULLPOLY
#define CYCLE (8)
typedef	struct	{
	GV_ACT_EX	actor ;
	DG_PRIM2	*prim ;

	DG_PRIM2	*futa ;

	FVECTOR	pos ;
	FVECTOR	*pos_buf ;

	float hankei ;
	int		flag ;
	int			cnt;

	float	height ;
	int		time ;
	int		time_a ;
	int		time_b ;

	short		tag_tex_w;
	short		tag_tex_h ;
	float		speed ;
} Work ;


static short GetCameraDir(Work *work){
	float     tmp ;
	DG_CHANL  *cp ;
extern short	GV_VecDir2FromTo( FVECTOR *from, FVECTOR *to ) ;

//	FMATRIX		eye_pers ;			/* カメラ透視変換マトリクス（=pers*eye_inv） */
//	FMATRIX		eye_inv ;			/* カメラ逆行列				*/
//	FMATRIX		eye ;				/* カメラ行列				*/
//	FMATRIX		pers ;				/* 透視変換マトリクス */
	cp = DG_Chanl(0) ;
	return 	GV_VecDir2FromTo( &work->pos, (FVECTOR *) &cp->eye.m[3] ) ;
}
static void InitPacket( Work *work){
	int		i,j,k;
	FVECTOR		*pos;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_TEX		*tag_tex ;

	float	len1,len2,y_len1,y_len2,dir;
	prim = work->prim ;


	/* テクスチャ取得 */
	tag_tex = DG_GetTexture( 8617368 );

	DG_ConfigPrim2Tex( work->prim, tag_tex );
	DG_ConfigPrim2Tex( work->futa, tag_tex );


	DG_InvisiblePrim2(work->prim) ;
	DG_SetPrim2Alpha( prim, NEGA );
//	DG_SetPrim2Alpha( prim, KASAN );
	DG_InvisiblePrim2(work->futa) ;


	DG_SetPrim2Alpha( work->futa, NEGA );
//	DG_SetPrim2Alpha( work->futa, KASAN );

//	DG_RaisePrim( prim, 4000 ) ;
//	DG_RaisePrim( work->futa, 4000 ) ;
	prim->raise =  RAISE_DRAW ;
	work->futa->raise = RAISE_DRAW ;



	for(i= 0 ;i<2;i++){
		short tex_w,tex_h ;
		for(k= 0 ; k<N_POLY;k++){
			uvrgb = (DG_PRIM2_UVRGB *)work->prim->uvrgb[i] ;
			uvrgb += k*N_VERTS ;
			pos = work->prim->pos[i] ;
			pos += k*N_VERTS ;
			tex_w = FTOI12( tag_tex->u_scale ) ;
			tex_h = FTOI12( tag_tex->v_scale ) ;
			for (j= 0 ; j<N_VERTS ;j++){
				uvrgb->r  = PRIM_RGB ;
				uvrgb->g  = PRIM_RGB ;
				uvrgb->b  = PRIM_RGB ;
				uvrgb->a  = 128 ;
				uvrgb->q = 4096 ;
				/*UV値*/
				uvrgb->u = FTOI12( tag_tex->u_offset ) ;
				uvrgb->v = FTOI12(tag_tex->v_offset ) ;
				/*描画フラグ*/
#if 0
				if ( j == 0 ){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
				} else {
					uvrgb->f = 0x0fff ;	/* 描画キックを行う */
				}
#else
				if(j > (N_VERTS/2)){
//				if(((j/2) > 1)&&(!((j/2)&1))){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
				}else if ( j == 0 ){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
				} else {
					uvrgb->f = 0x0fff ;	/* 描画キックを行う */
				}
#endif
				uvrgb++ ;
			}
			for (j= 0 ; j < N_VERTS ;j++){
				*pos = DG_ZeroVector ;

//				dir = ( float )M_PI *2.0f*(j/2)/CIRCLE_DIV ;
				dir = ( float )M_PI *4.0f*(j/2)/CIRCLE_DIV ;

				pos->vx = work->hankei*cosf(dir) ;
				pos->vz = work->hankei*sinf(dir) ;
//				pos->vx = 0.0f;
//				pos->vz = 0.0f;

				if(j&1){
//					pos->vy -= 250.0f;
					pos->vy += work->height ;
				}
				pos->vw = 1.0f ;
				pos++;
			}
		}
	}
	DG_SetPos2( &work->pos, &DG_ZeroSVector) ;
	DG_GetPos(&work->prim->as_world ) ;

	/*ふた*/
	for(i= 0 ;i<2;i++){
		short tex_w,tex_h ;
		for(k= 0 ; k<N_FUTA;k++){
			uvrgb = (DG_PRIM2_UVRGB *)work->futa->uvrgb[i] ;
			uvrgb += k*N_FUTA_VERTS ;
			pos = work->futa->pos[i] ;
			pos += k*N_FUTA_VERTS ;
			tex_w = FTOI12( tag_tex->u_scale ) ;
			tex_h = FTOI12( tag_tex->v_scale ) ;
			for (j= 0 ; j<N_FUTA_VERTS ;j++){
				uvrgb->r  = PRIM_RGB ;
				uvrgb->g  = PRIM_RGB ;
				uvrgb->b  = PRIM_RGB ;
				uvrgb->a  = 128 ;
				uvrgb->q = 4096 ;
				/*UV値*/
				uvrgb->u = FTOI12( tag_tex->u_offset ) ;
				uvrgb->v = FTOI12(tag_tex->v_offset ) ;
				/*描画フラグ*/
				if ( j <= 1 ){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
				} else if(j&1){
					uvrgb->f = 0x8fff ;	/* 描画キックしない */
				}else {
					uvrgb->f = 0x0fff ;	/* 描画キックを行う */
				}
				uvrgb++ ;
			}
			for (j= 0 ; j < N_FUTA_VERTS ;j++){
				pos[j].vx = pos[j].vz = 0.0f;
				pos[j].vy = work->height * (float)k ;
				pos[j].vw = 1.0f ;
			}
		}
	}
	DG_SetPos2( &work->pos, &DG_ZeroSVector) ;
	DG_GetPos(&work->futa->as_world ) ;

}

static void MovePos( Work *work,float speed){
	int		i,k,j;
	FVECTOR		*pos;
	float		hankei ,dir,height;
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb;

	FVECTOR	tmp ;
	float len1 ,len2 , y_len1, y_len2 , line_dis01;
	int rot ;
	DG_SetPos2( &work->pos, &DG_ZeroSVector) ;
	DG_GetPos(&work->prim->as_world ) ;


	if(work->flag == MODE_Y){
//		if(work->cnt > work->time_a+work->time_b ){
		if(work->cnt > work->time_b ){
			height = 0.0f;
//		}else if(work->cnt > work->time_a ){
		}else {
//			height = ((float)(work->cnt-work->time_a) /(float)(work->time_b) ) ;
         if (work->time_b == 0.0f)
            height = 1.0f;
         else
            height = ((float)(work->cnt) /(float)(work->time_b) ) ;
			height = 1.0f - height;
			height *= work->height;
//		}else {
//			height = work->height;
		}
		hankei = work->hankei;
	}else {
		if(work->cnt > work->time_a+work->time_b){
         if (work->time_a == 0)
            hankei = 0.0f;
         else
            hankei = (float)(work->time_a -(work->cnt- (work->time_a+work->time_b )))
               * (float)work->hankei/(float)work->time_a;
		}else if(work->cnt > work->time_a){
			hankei = work->hankei;
		}else {
         if (work->time_a == 0)
            hankei = 0.0f;
         else
            hankei = ((float)work->cnt /(float)work->time_a )* work->hankei;
		}
		height = 0.0f;
	}


	prim = work->prim ;

//	for(i= 0 ;i<2;i++){
	i= work->prim->buffer_clock ; {
		for (k= 0 ; k<N_POLY ;k++){
			pos = prim->pos[i] ;
			pos += k*N_VERTS ;

			uvrgb = (DG_PRIM2_UVRGB *)prim->uvrgb[i] ;
			uvrgb += k*N_VERTS ;
			for (j= 0 ; j<N_VERTS ;j++){
				/*線分は１頂点ごとに描画*/

//				dir = ( float )M_PI *2.0f*(j/2)/CIRCLE_DIV ;
				dir = ( float )M_PI *4.0f*(j/2)/CIRCLE_DIV ;

//				pos->vy += speed ;
				pos->vx = hankei*cosf(dir) ;
				if((j&1)==0){
					pos->vy = height ;
				}
				pos->vz = hankei*sinf(dir) ;
//				uvrgb->a  = 255*work->cnt / work->time ;
				pos++ ;
				uvrgb++ ;
			}
		}
	}

	i= work->prim->buffer_clock ; {
//	for(i= 0 ;i<2;i++){
		short tex_w,tex_h ;
		for(k= 0 ; k<N_FUTA;k++){
			pos = work->futa->pos[i] ;
			pos += k*N_FUTA_VERTS ;
			for (j= 0 ; j < N_FUTA_VERTS ;j++){
//				pos[j].vx = pos[j].vz = 0.0f;
//				pos[j].vy = work->height ;
				pos[j].vw = 1.0f ;
				if(k==0){
					pos[j].vy = height ;
				}
				if(j==0){
					continue ;
				}else if(j==1){
					dir = 0 ;
				}else if(!(j&1)){
//					dir = ( float )M_PI *2.0f*(j/2.0f)/CIRCLE_DIV ;
					dir = ( float )M_PI *4.0f*(j/2.0f)/CIRCLE_DIV ;
				}else if(j&1){
					continue ;
				}
				pos[j].vx = hankei*cosf(dir) ;
				pos[j].vz = hankei*sinf(dir) ;
			}
		}
	}

}

static	void	Act( work )
Work		*work ;
{
//	int model, i ;
	SVECTOR rot;

//	work->pos = *work->pos_buf ;

	rot.vx = rot.vz = 0 ;

//	rot.vy = work->cnt*16 ;
	rot.vy = GetCameraDir(work) ;


	DG_VisiblePrim2(work->prim) ;
	DG_VisiblePrim2(work->futa) ;
	MovePos(work,16.0f) ;
#if 0
	work->pos.vy +=work->speed ;
#endif
//	DG_SetPos2( &work->pos, &DG_ZeroSVector) ;
	DG_SetPos2( &work->pos, &rot) ;

	DG_GetPos(&work->prim->as_world ) ;
	DG_GetPos(&work->futa->as_world ) ;
	work->cnt--;
	if(work->cnt<=0){
		GV_DestroyActor( work ) ;
		return ;
	}

}

static	void	Die( work )
Work		*work ;
{
   // JA(AS) -- Voodoo attempt to kill triangle man
   DG_WaitForThreadedRenderComplete();
	DG_DequeuePrim2( work->prim );
	DG_FreePrim2( work->prim );

	DG_DequeuePrim2( work->futa );
	DG_FreePrim2( work->futa );


}




/*----------------------------------------------------------------*/
static	int	GetResources( Work	*work ){

//	work->prim = GM_MakePrim2( PRIM_FLAG, 1, N_VERTS );
	work->prim = GM_MakePrim2( POLY_FLAG, N_POLY, N_VERTS );
	work->futa = GM_MakePrim2( POLY_FLAG, N_FUTA, N_FUTA_VERTS );

   work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
   work->futa->flag |= DG_PRIM_AS_CUSTOMWORLD;

	InitPacket( work );
//	InitPos(work) ;

	return 0 ;
}
void	*NewVR_WarpTag( int flag,FVECTOR *pos,float hankei ,float height,int time_a,int time_b)
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		work->flag = flag ;
		work->pos = *pos ;
		work->pos_buf = pos ;
		work->pos.vy += RAISE_Y ;
		work->hankei = (float)hankei ;
		work->height = height ;
		work->cnt = work->time = DIRECT_TICK(time_a+time_b + time_a);
		work->time_a = DIRECT_TICK(time_a);
		work->time_b = DIRECT_TICK(time_b);

		work->speed =work->height/work->cnt;

		if ( GetResources( work) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

