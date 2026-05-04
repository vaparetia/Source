//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	赤外線カメラのぼんぼりのエフェクト (回転スクリプトバージョン）
	2000/01/27 H.TANAKA
	2000/03/31 S.Okajima
	$Id: gllspread.c,v 1.2 2002/11/25 01:54:46 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
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

/* ------------------------------------------------------------ */
#define	  FTOI12(_f)		 ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ------------------------------------------------------------- */
#define	  PRIM_TYPE		(DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define	  N_PRIMS		(1)
#define	  N_VERTS		(4)
#define	  N_NUM			(N_PRIMS * N_VERTS)
#define	  SPEED_Y		(3.0F)

#define	  COL_RGB		(255)
#define	  COL_A			(8)
#define	  COL_PARAM0	(COL_A * 2)
#define	  COL_PARAM1	(16)
#define	  MAX_COUNT		(COL_PARAM0 * COL_PARAM1)

//#define	  SIZE_MIN		(100)
//#define	  SIZE_RND		(500)
//#define	  SIZE_MAX		(1500)

#define	  SIZE_MIN		(1500)
#define	  SIZE_RND		(2500)
#define	  GLL_SIZE_MAX		(4000)

#define	  VEC_RND		(20.0f)
//#define	  GAS_GRAVITY	(0.3f)
#define	  GAS_GRAVITY	(4.0f)


/* 1 = 65536 */
#define	  EXPAND_RATIO	(65536 / 8)

typedef	struct
{
	GV_ACT_EX	 actor ;

	DG_TEX	 *tex ;
	DG_PRIM2   *prim ;
	
	FMATRIX	world ;
	int		count ;
	int		size[N_NUM] ;
	int		rad[N_NUM] ;
	FVECTOR	vec[N_NUM] ;

} Work ;


/* ------------------------------------------------------------------- */

static   void  Act( Work *work )
{
	DG_PRIM2		   *prim ;
	FVECTOR			*pre_pos,*pos ;
	FVECTOR				*vec ;
	DG_PRIM2_UVRGBWH   *pre_uvrgbwh, *uvrgbwh ;
	int				i,clock ;
	int				*size, *rad ;
   int buffSwitch;

	if( work->count > MAX_COUNT ){
		GV_DestroyActor( work );
		return ;
	}

	prim = work->prim ;
   //AR_PARTICLE_FULL
	buffSwitch = DG_SwitchBuffPrim2( prim ) ;
	clock = prim->buffer_clock;

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	pre_pos	 = prim->pos[ buffSwitch ^ clock ] ;
	pre_uvrgbwh = prim->uvrgb[ buffSwitch ^ clock ] ;

	pos = prim->pos[ clock ] ;
	uvrgbwh = prim->uvrgb[ clock ] ;
	
   size = work->size ;
	rad  = work->rad ;
	vec = work->vec;
	
   for(i = 0; i < N_NUM; i++){
		if( pre_uvrgbwh->a < 1 ){
			uvrgbwh->a = 0 ;
			uvrgbwh->w = 0 ;
			uvrgbwh->h = 0 ;
		}else if( i == work->count % COL_PARAM0 ){
			uvrgbwh->a = pre_uvrgbwh->a - 1;
			(*size) += (GLL_SIZE_MAX - (*size)) * EXPAND_RATIO / 65536 ;
			uvrgbwh->w = *size * cosf( (float)M_PI * (*rad) / 180.0F ) ;
			uvrgbwh->h = *size * sinf( (float)M_PI * (*rad) / 180.0F ) ;
		}else{
			uvrgbwh->a = pre_uvrgbwh->a;
			(*size) += (GLL_SIZE_MAX - (*size)) * EXPAND_RATIO / 65536 ;
			uvrgbwh->w = *size * cosf( (float)M_PI * (*rad) / 180.0F ) ;
			uvrgbwh->h = *size * sinf( (float)M_PI * (*rad) / 180.0F ) ;
		}

		pos->vx = pre_pos->vx + vec->vx ;
		pos->vy = pre_pos->vy + vec->vy ;
		pos->vz = pre_pos->vz + vec->vz ;

		vec->vy-= GAS_GRAVITY ;

		pos ++ ;
		pre_pos ++ ;
		vec++;
		uvrgbwh ++ ;
		pre_uvrgbwh ++ ;
		size ++ ;
		rad  ++ ;
	}

	work->count ++ ;
}

static   void  Die( Work *work )
{
	if(work->prim != NULL) GM_FreePrim2(work->prim) ;
}
/* ---------------------------------------------------------------------*/

static   int   GetResources(Work *work,FMATRIX  *world)
{
	DG_PRIM2			*prim ;
	DG_TEX				*tex ;
	FVECTOR				*pos_a,*pos_b ;
	FVECTOR				*vec ;
	DG_PRIM2_UVRGBWH	*uvrgbwh_a, *uvrgbwh_b ;
	int					i;
	int					*size;
	int					*rad ;

	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS);
	if(prim == NULL) return -1 ;

//	tex = work->tex = DG_GetTexture( GV_StrCode("powfog03_add")) ;
//	tex = work->tex = DG_GetTexture( GV_StrCode("powder02_alp")) ;
	tex = work->tex = DG_GetTexture(9998494) ;
//	tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
	tex = DG_GetTexture(9868771);

	if(tex == NULL)
	return -1 ;
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 )) ;

	work->count = 0 ;
	work->world = *world ;

	pos_a = prim->pos[0] ;
	pos_b = prim->pos[1] ;
	uvrgbwh_a = prim->uvrgb[0] ;
	uvrgbwh_b = prim->uvrgb[1] ;

	size = work->size ;
	rad  = work->rad ;
	vec = work->vec;
	for(i = 0; i < N_NUM; i++){
		vec->vx = rnd() * VEC_RND - VEC_RND*0.5f;
		vec->vy = rnd() * VEC_RND;
		vec->vz = rnd() * VEC_RND - VEC_RND*0.5f;
		pos_a->vx = world->m[3][0] + vec->vx ;
		pos_a->vy = world->m[3][1] + vec->vy;
		pos_a->vz = world->m[3][2] + vec->vz ;
		pos_a->vw = 1.0F ;
		*pos_b = *pos_a ;
		pos_a ++ ;
		pos_b ++ ;

		uvrgbwh_a->u0 = FTOI12(tex->u_offset) ;
		uvrgbwh_a->v0 = FTOI12(tex->v_offset) ;
		uvrgbwh_a->u1 = FTOI12(tex->u_scale + tex->u_offset) ;
		uvrgbwh_a->v1 = FTOI12(tex->v_scale + tex->v_offset) ;
		uvrgbwh_a->q0 = 4096 ;
		uvrgbwh_a->q1 = 4096 ;
		uvrgbwh_a->f0 = 0x0fff ;
		uvrgbwh_a->f1 = 0x0fff ;
		*size = SIZE_MIN + irnd() % SIZE_RND ;
		*rad  = irnd() % 360 ;
		uvrgbwh_a->w = *size * cosf( (float)M_PI * (*rad) / 180.0F ) ; 
		uvrgbwh_a->h = *size * sinf( (float)M_PI * (*rad) / 180.0F ) ;
		uvrgbwh_a->r = uvrgbwh_a->g = uvrgbwh_a->b = COL_RGB ;
		uvrgbwh_a->a = COL_A ;

		*uvrgbwh_b = *uvrgbwh_a ;

		vec++;
		uvrgbwh_a ++ ;
		uvrgbwh_b ++ ;
		size ++ ;
		rad  ++ ;
	}

	return 0 ;
}

void  *NewGllSpreadFlour(FMATRIX *world )
{
	Work  *work ;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER,sizeof(Work)) ;
	if(work != NULL){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(GetResources( work, world ) < 0){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
