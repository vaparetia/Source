//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	赤外線カメラのぼんぼりのエフェクト
	2000/10/18 S.Okajima
	2001/08/21 Masafumi Okuta : Z切断を回避追加
	$Id: inf_cam.c,v 1.1.1.3 2002/11/19 11:47:43 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"def_dma.h"
#include	"utl_dma.h"
#include	"camera.h"
#include	"../etc/ok_util.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define      FTOI12(_f)	     ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define      PRIM_TYPE       ( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define      N_PRIMS         1
#define      N_VERTS         2

#define	RAISE_SHIFT		(30.0f)

static       FVECTOR       big_shift_infcam[] = {
    { 31.0F, 42.0F, 261.0F, 1.0F}, 
    {-31.0F, 42.0F, 261.0F, 1.0F}
} ;

typedef struct
{
	GV_ACT_EX	actor ;
	DG_PRIM2    *prim ;
    FVECTOR     base[N_PRIMS * N_VERTS] ;

    FMATRIX      *world ;
    int         *mode ;
    int         *group_id ;

} Work ;

enum {
	INVISIBLE,
	OPEN,
	CLOSE
} ;

extern  void _BigScrCopy( void *dst, void *src, int size, int num );
extern  void _BigMemCopy( void *dst, void *src, int size, int num );
extern  void *AfterImage(FMATRIX *world,FVECTOR *shift, int *mode, int *group_id);

static  void  Act(Work *work)
{
	float	screen_near_x;
	float	screen_near_y;
	float	angle;
	float	physic_raise;

	DG_PRIM2         *prim ;
	FVECTOR          *pos ;
	GM_CameraSet     *cam ;
	FVECTOR          *cam_pos ;
	int              i ;
	FVECTOR             tmp ;
	FVECTOR* 		pvec;
	DG_PRIM2_UVRGBWH    *uvrgbwh ;
	float               len ;
	int                 size ;

	prim = work->prim ;
	/* カメラ位置をセット */
	cam = GM_GetCurrentCameraSet(0) ;  
	cam_pos = &cam->position ;

	if(work->mode != NULL){
		if((*(work->mode) == -1)){
			GV_DestroyActor(work) ;
			return ;
		}
		if(*(work->mode) == CLOSE){
			DG_VisiblePrim2(prim) ;
		}else{
			DG_InvisiblePrim2(prim) ;
		}
	}else{
		DG_VisiblePrim2(prim) ;
	}

	/* スイッチ切り替え */
	DG_SwitchBuffPrim2(prim) ;

	DG_SetPos(work->world) ;
	pos = prim->pos[prim->buffer_clock] ;
	DG_PutVector(&big_shift_infcam[0],pos,N_VERTS) ;

	angle = DG_Chanls->screen;
	screen_near_x = (ASPECT_X()                                           ) * angle;
	screen_near_y = (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) * angle;

	pvec = prim->pos[prim->buffer_clock];
	for ( i = 0; i < N_VERTS; i++){ // Z切断回避

	    /* 座標を透視変換する */
	    DG_SetPos( &DG_Chanls->eye_pers_no_offset ); //BP_CAMERA - replaced "eye_pers" with "eye_pers_no_offset" to fix projection/inverse projection
	    DG_PutVector( pvec, pvec, 1 );

	    if( pvec->vz < pvec->vw  &&  pvec->vw > RAISE_SHIFT + 51.0f ){
		physic_raise = pvec->vw - RAISE_SHIFT;

		pvec->vz = physic_raise;
		pvec->vx = physic_raise * pvec->vx / screen_near_x / pvec->vw;
		pvec->vy = physic_raise * pvec->vy / screen_near_y / pvec->vw;

		/* カメラ座標をワールド座標に変換 */
		DG_SetPos( &DG_Chanls->eye );
		DG_PutVector( pvec, pvec, 1 );
	    }

	    pvec++;
	}

	/* サイズを決定 */
	uvrgbwh = prim->uvrgb[prim->buffer_clock] ;
	for( i = 0 ; i < N_VERTS; i++){
		_sceVu0SubVector(&tmp, pos, cam_pos) ;
		len = GV_VecLen3F(&tmp) ;
		if(len <= 2000.0F){
			size = 15 ;
		}else if(len >= 4000.0F){
			size = 30 ;
		}else{
			size = (int)(15.0F + (30.0F - 15.0F) * (len - 2000.0F) / (4000.0F - 2000.0F)) ; 
		}
		uvrgbwh->w = uvrgbwh->h = size;
		pos ++ ;
		uvrgbwh ++ ;
	}
}

static  void  Die(Work *work)
{
	work->prim = OK_FreePrim2( work->prim ) ;
}

static  int  GetResources(Work *work, FMATRIX *world, int *mode, int *group_id)
{
    DG_PRIM2    *prim ;
    DG_TEX      *tex ;
    FVECTOR     *pos_a, *pos_b ;
    DG_PRIM2_UVRGBWH *uvrgbwh_a, *uvrgbwh_b ;
    int         i ;

    /* 四角形ポリゴンの実験 */
    prim = work->prim = GM_MakePrim2(PRIM_TYPE, N_PRIMS, N_VERTS) ;
    if(prim == NULL)
	return -1 ;
    /* テクスチャを取得 */
    tex = DG_GetTexture( 3908076 /*"light08_msk"*/ ) ;
    if(tex == NULL)
	return -1 ;
    DG_ConfigPrim2Tex(prim, tex) ;
    DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0,2,0,1,0x00)) ;
//    prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0x00) ;

    prim->group_id = *group_id ;
    work->world    = world ;
    work->mode     = mode ;
    work->group_id = group_id ;
    if(work->mode == NULL)
	DG_VisiblePrim2(prim) ;
    else
    {
	if(*(work->mode))
	    DG_VisiblePrim2(prim) ;
	else
	    DG_InvisiblePrim2(prim) ;
    }

    pos_a = prim->pos[0] ;
    pos_b = prim->pos[1] ;
    uvrgbwh_a = prim->uvrgb[0] ;
    uvrgbwh_b = prim->uvrgb[1] ;

    DG_SetPos(world) ;
    DG_PutVector(big_shift_infcam,pos_a,N_VERTS) ;
    DG_PutVector(big_shift_infcam,pos_b,N_VERTS) ;
    
//    printf("vx %f, vy %f, vz %f vw %f\n",pos_a->vx,pos_a->vy,pos_a->vz, pos_a->vw) ;
    
    for(i = 0; i < N_VERTS; i++)
    {
	uvrgbwh_a->u0 = FTOI12(0.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh_a->v0 = FTOI12(0.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh_a->u1 = FTOI12(1.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh_a->v1 = FTOI12(1.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh_a->q0 = 4096 ;
	uvrgbwh_a->q1 = 4096 ;
	uvrgbwh_a->f0 = 0x0fff ;
	uvrgbwh_a->f1 = 0x0fff ;
	uvrgbwh_a->w = 30 ;
	uvrgbwh_a->h = 30 ;
	uvrgbwh_a->r = 200 ;
	uvrgbwh_a->g = 128 ;
	uvrgbwh_a->b = 128 ;
	uvrgbwh_a->a = 128 ;
	*uvrgbwh_b = *uvrgbwh_a ;
	uvrgbwh_a ++ ;
	uvrgbwh_b ++ ;
    }
    


	GV_SetActorChild( work, AfterImage(world, &big_shift_infcam[0], mode, group_id) );
	GV_SetActorChild( work, AfterImage(world, &big_shift_infcam[1], mode, group_id) );
    DG_InvisiblePrim2(prim) ;
    return 0 ;

}


/* --------------------------------------------------------- */
/* 赤外線カメラ */
void     *InfaredCamera_Light(FMATRIX *world,int *mode,int *group_id)
{
	Work         *work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, world, mode, group_id) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
