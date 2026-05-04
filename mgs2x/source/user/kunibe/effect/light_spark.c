//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   light_spark.c
   ソリダス用照り返し火花 ( スローパラメータ対応済 )
   2001/05/20 Yuuta Kunibe	
   $Id: light_spark.c,v 1.1.1.3 2002/11/19 11:44:44 Yoshizawa1 Exp $
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
#include	"../../okajima/etc/ok_util.h"


/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
/* ---------------------------------------------------------------- */

#define	SCR_LENGTH   	( 0x3000 )	/* 本来は 0x4000 */
#define N_VERTS	  	( 2 )  
#define MEM_ADDR1	((void *)( SCRPAD_ADDR + 0x0000))
#define MEM_ADDR2	((void *)( SCRPAD_ADDR + 0x2000))
#define PRIM_TYPE	( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA|DG_PRIM2_NOMSAA )

#define SEARCH_LENGTH	(500.f)

#define LIGHT_ALPHA	(8)



typedef struct
{
	GV_ACT_EX	 actor;
	DG_PRIM2   	*prim;
    DG_PRIM2		*prim_light;

    FVECTOR		vec_x;
    FVECTOR		vec_z;
    
	int		n_prims;	/* 火花の数 */
	float		length;		/* 長さの割合 */
	float		gravity;	/* 重力 */
	float		count;
	float		count_max;

	FVECTOR		color;		/* 色 */
	FVECTOR		speed[0];	/* スピード */

} Work ;


/* ---------------------------------------------------------------- */
	/*
		プログラム使用サブルーチン
	*/

/* メインメモリからスクラッチパッドへの転送 */
extern void  _BigScrCopy( void *dst, void *src, int size, int num) ;

/* スクラッチパッドからメインメモリへの転送 */
extern void  _BigMemCopy( void *dst, void *src, int size, int num ) ;

/*-------------------------------------------------------------------*/
extern void *NewPointDebug( FVECTOR *pos );

extern float	OK_slow_param;



static   void  Act_Scr(Work *work)
{

    static FVECTOR	n_vector = { 3000.0f, 12500.0f, 0.0f };
    static FVECTOR	floor_pos = { 0.0f, 18000.0f, 23000.0f };
    
	FVECTOR		*pos;
	FVECTOR		*pos_pre;
	FVECTOR		*speed;
	int		n_prims;
	int		i;
	float		length;
	FVECTOR		vec;
	FVECTOR		vectmp;
	FVECTOR		*position;
	FVECTOR		speed_tmp;

	float		width;
	FVECTOR		width_x;
	FVECTOR		width_z;

	n_prims = work->n_prims;
	pos     = MEM_ADDR1;
	pos_pre = work->prim->pos[1-work->prim->buffer_clock];
	speed = work->speed;
	length = work->length * 2.0f;

	position = work->prim_light->pos[work->prim_light->buffer_clock];

	for ( i = 0; i < n_prims ; i++ ) {

	    speed->vy -= work->gravity * OK_slow_param;
	    _sceVu0ScaleVector( &speed_tmp, speed, OK_slow_param );
	    _sceVu0AddVector( &pos[0], &pos[0], &speed_tmp );

	    /* 床判定 */
	    if ( speed->vy < 0.0f && pos->vx <= 12500.0f && pos->vy <= floor_pos.vy && pos->vz <= 23000.0f ) {
		_sceVu0SubVector( &vec, pos, &floor_pos );
		if ( _sceVu0InnerProduct( &vec, &n_vector ) < 0.0f ) {
		    DG_COPY_VEC( &pos[0], &pos_pre[0] );
		    pos[0].vy += 20.0f;
		    speed->vy = -speed->vy;
		    speed->vx *= 0.40f + frnd()*0.60f;
		    speed->vy *= 0.40f + frnd()*0.10f;
		    speed->vz *= 0.40f + frnd()*0.60f;
		}
	    }
	    
	    pos[1].vx = pos[0].vx - speed->vx * length;
	    pos[1].vy = pos[0].vy - speed->vy * length;
	    pos[1].vz = pos[0].vz - speed->vz * length;
	    pos[0].vw = 1.0F ;
	    pos[1].vw = 1.0F ;

	    /* 照り返し */
	    vectmp.vx = pos->vx;
	    vectmp.vy = 15250.0f + 2750.0f * ( 12500.0f - vectmp.vx ) / 12500.0f;
	    vectmp.vz = pos->vz;
	    //NewPointDebug( &vectmp );

	    if ( vectmp.vx <= 12500.0f && vectmp.vz <= 23000.0f ) {
		if ( vectmp.vy > pos->vy ) {
		    width = 120.0f;
		}
		else if ( pos->vy - vectmp.vy < 1000.0f ) {
		    width = 120.0f * ( 1000.0f - pos->vy + vectmp.vy ) / 1000.0f;
		}
		else {
		    width = 0.0f;
		}
	    }
	    else {
		width = 0.0f;
	    }


	    _sceVu0ScaleVector( &width_x, &work->vec_x, width );
	    _sceVu0ScaleVector( &width_z, &work->vec_z, width );

	    DG_COPY_VEC( position, &vectmp );
	    _sceVu0AddVector( position, position, &width_x );
	    _sceVu0AddVector( position, position, &width_z );
	    position++;

	    DG_COPY_VEC( position, &vectmp );
	    _sceVu0AddVector( position, position, &width_x );
	    _sceVu0SubVector( position, position, &width_z );
	    position++;

	    DG_COPY_VEC( position, &vectmp );
	    _sceVu0SubVector( position, position, &width_x );
	    _sceVu0AddVector( position, position, &width_z );
	    position++;

	    DG_COPY_VEC( position, &vectmp );
	    _sceVu0SubVector( position, position, &width_x );
	    _sceVu0SubVector( position, position, &width_z );
	    position++;
	    
	    speed++;
	    pos     += 2;
	    pos_pre += 2;

	}	

}	



static   void  Act(Work *work)
{

	DG_PRIM2	*prim;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*uvrgb_light;
	int		num;
	float		bright;
	SVECTOR		color;
	int		i;

	
	work->count -= OK_slow_param;
	if ( work->count <= 0.0f ) {
		DG_InvisiblePrim2(work->prim);
		DG_InvisiblePrim2(work->prim_light);
		GV_DestroyActor(work);
		return;
	}

	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap );
	GM_GroupPrim2( work->prim_light, GM_CurrentStageMap );

	num = work->n_prims * N_VERTS;

	/* 位置とスピードを更新する */
	_BigScrCopy( MEM_ADDR1, prim->pos[prim->buffer_clock], sizeof(FVECTOR), num );

	DG_SwitchBuffPrim2( prim );
	DG_SwitchBuffPrim2( work->prim_light );

	Act_Scr(work);
	_BigMemCopy( prim->pos[prim->buffer_clock], MEM_ADDR1, sizeof(FVECTOR), num );

	bright = work->count / work->count_max;
	color.vx = (u_short)( work->color.vx * bright );
	color.vy = (u_short)( work->color.vy * bright );
	color.vz = (u_short)( work->color.vz * bright );

	uvrgb = prim->uvrgb[prim->buffer_clock];
	uvrgb_light = work->prim_light->uvrgb[prim->buffer_clock];

	for( i = 0 ; i < work->n_prims; i++ ) {
		/* 先頭のみ更新 */
		uvrgb->r = color.vx;
		uvrgb->g = color.vy;
		uvrgb->b = color.vz;
		uvrgb += N_VERTS;

		uvrgb_light->a = LIGHT_ALPHA * bright;
		uvrgb_light++;
		uvrgb_light->a = LIGHT_ALPHA * bright;
		uvrgb_light++;
		uvrgb_light->a = LIGHT_ALPHA * bright;
		uvrgb_light++;
		uvrgb_light->a = LIGHT_ALPHA * bright;
		uvrgb_light++;

	}
	
}

static   void  Die(Work *work)
{
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim ) ;
    if ( work->prim_light ) work->prim_light = OK_FreePrim2( work->prim_light ) ;
}


/*--------------------------------------------------------------------------*/
static void Init_Tex( Work *work, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb;
	int		n_prims;
	int		i;
	int		n_verts ;

	n_prims = work->n_prims;
	uvrgb = MEM_ADDR1;

	n_verts = ( n_prims * N_VERTS + 63 ) & ~63 ;
	for ( i = 0; i < n_verts ; i+=2 ) {	/* for ( i = 0; i < n_prims ; i++ ) */

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = (u_short)work->color.vx;
		uvrgb->g = (u_short)work->color.vy;
		uvrgb->b = (u_short)work->color.vz;
		uvrgb->a = 128;

		uvrgb[1].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
		uvrgb[1].v = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
		uvrgb[1].q = 4096;
		uvrgb[1].f = 0x0fff;
		uvrgb[1].r = 0;
		uvrgb[1].g = 0;
		uvrgb[1].b = 0;
		uvrgb[1].a = 128;

		if ( i >= ( n_prims * N_VERTS ) ){
			uvrgb[0].f = 0x8fff;
			uvrgb[1].f = 0x8fff;
		}
		uvrgb += N_VERTS;


	}

}


static void Init_Tex2( Work *work, DG_TEX *tex )
{
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	DG_PRIM2_UVRGB	*uvrgb;
	int		n_prims;
	int		i;
	int		n_verts ;

	n_prims = work->n_prims;
	pos0  = work->prim_light->pos[0];
	pos1  = work->prim_light->pos[1];
	uvrgb = MEM_ADDR1;

	n_verts = ( n_prims * 4 + 63 ) & ~63 ;
	for ( i = 0; i < n_verts; i+=4 ) {		/* for ( i = 0; i < n_prims; i++ ) */

		DG_COPY_VEC( pos0, &DG_ZeroVector ); pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector ); pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector ); pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector ); pos0++;

		DG_COPY_VEC( pos1, &DG_ZeroVector ); pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector ); pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector ); pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector ); pos1++;
		
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = (u_short)work->color.vx;
		uvrgb->g = (u_short)work->color.vy;
		uvrgb->b = (u_short)work->color.vz;
		uvrgb->a = 0;//LIGHT_ALPHA;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = (u_short)work->color.vx;
		uvrgb->g = (u_short)work->color.vy;
		uvrgb->b = (u_short)work->color.vz;
		uvrgb->a = 0;//LIGHT_ALPHA;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = (u_short)work->color.vx;
		uvrgb->g = (u_short)work->color.vy;
		uvrgb->b = (u_short)work->color.vz;
		uvrgb->a = 0;//LIGHT_ALPHA;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = (u_short)work->color.vx;
		uvrgb->g = (u_short)work->color.vy;
		uvrgb->b = (u_short)work->color.vz;
		uvrgb->a = 0;//LIGHT_ALPHA;
		uvrgb++;

		if ( i >= ( n_prims * 4 ) ){
			uvrgb[0-4].f = 0x8fff;
			uvrgb[1-4].f = 0x8fff;
			uvrgb[2-4].f = 0x8fff;
			uvrgb[3-4].f = 0x8fff;
		}

	}

}




static   int  GetResources( Work *work )
{
	DG_PRIM2	*prim;
	DG_TEX		*tex;
	int		n_prims;
	int		num;
	int		ex_n_prims ;


	n_prims = work->n_prims;

#if 0
	if ( ( prim = work->prim = GM_MakePrim2( PRIM_TYPE, n_prims, N_VERTS ) ) == NULL ) {
	    return -1 ;
	}
#else
	ex_n_prims = ( n_prims * N_VERTS + 63 ) / 64 ;
	if ( ( prim = work->prim = GM_MakePrim2( PRIM_TYPE, ex_n_prims, 64 ) ) == NULL ) {
	    return -1 ;
	}
#endif

	/* 照り返し*/
#if 0
	if ( ( work->prim_light = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
						n_prims, 4 ) ) == NULL ) {
	    return -1 ;
	}
#else
	ex_n_prims = ( n_prims * 4 + 63 ) / 64 ;
	if ( ( work->prim_light = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
						ex_n_prims, 64 ) ) == NULL ) {
	    return -1 ;
	}
#endif

	/* 位置と速度情報を移す */
#if 0
	num = N_VERTS * n_prims ;
#else
	ex_n_prims = ( n_prims * N_VERTS + 63 ) / 64 ;
	num = ex_n_prims * 64 ;
#endif
	_BigMemCopy( prim->pos[0], MEM_ADDR1, sizeof(FVECTOR), num );
	_BigMemCopy( prim->pos[1], MEM_ADDR1, sizeof(FVECTOR), num );
	_BigMemCopy( work->speed, MEM_ADDR2, sizeof(FVECTOR), n_prims );

	/* テクスチャ関係の初期化 */
	tex = DG_GetTexture( GV_StrCode("col128_add") );

	DG_ConfigPrim2Tex( prim, tex ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	Init_Tex(work,tex) ;
	_BigMemCopy( prim->uvrgb[0], MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), num );
	_BigMemCopy( prim->uvrgb[1], MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), num );



	//tex = DG_GetTexture( GV_StrCode("rcm_l_msk") );
	//tex = DG_GetTexture( GV_StrCode("svc_bonbori_r") );
	tex = DG_GetTexture( GV_StrCode("drop01_msk") );
	DG_ConfigPrim2Tex( work->prim_light, tex );
	DG_SetPrim2Alpha( work->prim_light, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

#if 0
	num = 4 * n_prims ;
#else
	ex_n_prims = ( n_prims * 4 + 63 ) / 64 ;
	num = ex_n_prims * 64 ;
#endif
	Init_Tex2(work,tex) ;
	_BigMemCopy( work->prim_light->uvrgb[0], MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), num );
	_BigMemCopy( work->prim_light->uvrgb[1], MEM_ADDR1, sizeof(DG_PRIM2_UVRGB), num );


	DG_COPY_VEC( &work->vec_z, &DG_ZeroVector );
	work->vec_z.vx = 0.0f;
	work->vec_z.vy = 0.0f;
	work->vec_z.vz = 1.0f;
	work->vec_z.vw = 1.0f;

	work->vec_x.vx = 12500.0f;
	work->vec_x.vy =-2750.0f;
	work->vec_x.vz = 0.0f;
	work->vec_x.vw = 1.0f;

	_sceVu0Normalize( &work->vec_x, &work->vec_x );
	
	return 0;

}


static  void Init_Scr2(
		       Work    *work ,
		       float   min_speed,
		       float   speed_wide,
		       FMATRIX *world,
		       SVECTOR *rot,
		       SVECTOR *rot_wide )
{

	SVECTOR		local_rot;
	FVECTOR		*pos;
	FVECTOR		*speed;
	int		i;
	int		n_prims;
	float		length;

	pos   = MEM_ADDR1;
	speed = MEM_ADDR2;

	n_prims = work->n_prims;
	length  = work->length;

	for ( i = 0; i < n_prims; i++ ) {

	    local_rot.vx = rot->vx + irnd() % rot_wide->vx;
	    local_rot.vy = rot->vy + irnd() % rot_wide->vy;
	    local_rot.vz = rot->vz + irnd() % rot_wide->vz;

	    DG_SetPos( world );
	    DG_RotatePos( &local_rot );

	    speed->vx = 0.0F;
	    speed->vy = 0.0F;
	    speed->vz = min_speed + rnd() * speed_wide;
	    speed->vw = 1.0F;

	    DG_RotVector( speed, speed, 1 );
	
	    /* 位置決定 */
	    _sceVu0AddVector( &pos[1], speed, (FVECTOR *)world->m[3] );
	    pos[1].vw = 1.0F;

	    _sceVu0AddVector( &pos[0], speed, &pos[1] );
	    pos[0].vw = 1.0F;

	    pos += 2;
	    speed++;

	}
	
}



/*-------- NewLightSpark : 火花呼び出し関数 ----------
     int	n_prims		: 火花数
     FMATRIX	*world		: 親マトリクス
     float	min_speed 	: 最小スピード
     float	speed_wide	: スピード幅
     float	gravity		: 重力
     SVECTOR	*rot		: 回転角度
     SVECTOR	*rot_wide	: 回転幅
     FVECTOR	*color		: 色
     float	length		: 幅
     int	count		: 生存時間
------------------------------------------------------*/
void *NewLightSpark(
		int	  n_prims,
		FMATRIX	  *world,
		float	  min_speed,
		float	  speed_wide,
		float	  gravity,
		SVECTOR	  *rot,
		SVECTOR	  *rot_wide,
		FVECTOR	  *color,
		float	  length,
		int	  count )
{

	Work *work;
	int  data_size;
	int  max_num;

	data_size = sizeof(FVECTOR) * 3;
	max_num = SCR_LENGTH / data_size;
	if ( n_prims > max_num ) {
		n_prims = max_num;
	}

	data_size = sizeof(Work) + sizeof(FVECTOR) * n_prims;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, data_size );

	if ( work != NULL ) {
		GV_SetActor( &(work->actor), Act, Die );
		GV_ActorEX( &work->actor );

		work->n_prims   = n_prims;
		work->count_max = work->count = (float)count;
		work->length    = length;
		work->color.vx  = color->vx;
		work->color.vy  = color->vy;
		work->color.vz  = color->vz;
		work->color.vw  = color->vw;
		work->gravity   = gravity;
		Init_Scr2( work, min_speed, speed_wide, world, rot, rot_wide );

		/* ワークの情報を初期化 */	
		if( GetResources(work) < 0 ) {
			GV_DestroyActor(work);
			return NULL;
		}

	}

	return work ;

}





