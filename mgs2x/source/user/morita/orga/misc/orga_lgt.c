//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_lgt.c 
   オルガ オルガ戦専用プット投光器オブジェ

   1999/12/26 T.Morita
   $Id: orga_lgt.c,v 1.1.1.3 2002/11/19 11:46:26 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"
#include "../../brk_utl/brk_utl.x"

#include "../include/orga_se.h"


#define ORGA_N_VITAL 1

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;



typedef struct lgtwork_t
{
    GV_ACT       actor  ;

    FMATRIX      lights[2] ;

    DG_VERTS_ANIME vanm[2] ;
    DG_DEF      *off    ;
    DG_DEF      *on     ;
    DG_OBJS     *objs   ;
    CV2_DEF     *cvd    ;

    int         name    ;
    int         flag    ;
    int         bg_name ;
    int         lt_name ;
    int         pr_name ;

    short        turn_x ;
    short        turn_y ;
    short        rot_x  ;
    short        rot_y  ;
    FVECTOR      pos    ;

    LIT_DEF     *lit    ;

    int          map    ;
    int          vital  ;
    TARGET       target ;
    POWER_TARGET power  ;

    void        *lens   ;
    void        *rain   ;
    void        *spot   ;
    void        *volume ;
} Work ;

extern void  OK_SetDynamicLight( FVECTOR *pos, SVECTOR *rot ) ;       /* okajima/effect/lit_man.c   */
extern void  OK_RemoveDynamicLight( FVECTOR *pos );                   /* okajima/effect/lit_man.c   */
extern void *NewSpotRain() ;
extern void *NewOrgaLensFlr_Game( FMATRIX *world, FVECTOR *center, float size, int max_alpha ) ;
extern void  ORG_SendOrgaExitLightMessage() ;
extern void  ORG_SendOrgaChangeLightMessage() ;

extern void  TS_MakeMatrix( FMATRIX *world, FVECTOR *dir, FVECTOR *pos ) ;
extern void *NewSpark( FMATRIX *world ) ;



FVECTOR ORG_LGT_BreakPhase = { 0.0f, 0.0f, 500.0f, 1.0f } ;

static inline int SpeedCurve( int t, int r )
{
    for( ; t/2 && --r>=0 ; t/=2 ) ;

    return t ;
}

static void RotateMatrixXY( Work *work )
{
    FMATRIX *mtx ;
    DG_OBJ  *objs ;
    float z, y ;
    int i ;

    z = work->rot_x*M_PI/32768.0f ;
    y = work->rot_y*M_PI/32768.0f ;
    mtx = &work->objs->world ;

    _sceVu0RotMatrixY( mtx, &DG_UnitMatrix, y ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)mtx->m[3], &work->pos ) ;

    objs = work->objs->objs ;
    for ( i=0 ; i<(int)work->objs->def->n_x_models ; i++ )
    {
	if ( objs[i].parent < 0 )
	    mtx = &work->objs->world ;
	else
	    mtx = &objs[objs[i].parent].world ;
	if ( i== 1 ) {
	    _sceVu0RotMatrixZ( &objs[i].world, mtx, z ) ;
	} else {
	    _sceVu0CopyMatrix( &objs[i].world, mtx ) ;
	}
	_sceVu0ApplyMatrix( (FVECTOR *)objs[i].world.m[W], mtx, &objs[i].trans ) ;
    }
}


/* 目標回転 */
static void CalcDirection( Work *work, FVECTOR *pos, FVECTOR *aim, short *x, short *y )
{
    FVECTOR dir ;

    _sceVu0SubVector( &dir, aim, pos ) ;
    _sceVu0Normalize( &dir, &dir ) ;

    *y = (short)(32768.0f / M_PI * atan2f( dir.vx, dir.vz )) ;
    *x = (short)(32768.0f / M_PI * atan2f( dir.vy, bp_sqrtf( dir.vx*dir.vx + dir.vz*dir.vz ) )) ;  //BP_MATH - emulate PS2 sqrtf
}


static void Die( Work *work )
{
    int  i ;

    OK_RemoveDynamicLight( (FVECTOR*)&work->objs->world.m[3] )  ;
    if ( work->objs )
    {
	DG_DequeueObjs( work->objs ) ;
	DG_FreeObjs( work->objs ) ;
    }

    for ( i=2 ; --i>=0 ; )
	if( work->vanm[i].mem[0] )
	    DG_FreeAnimVertsBuffer( &work->vanm[i] ) ;

    if ( work->spot )
	GV_DestroyOtherActor( work->spot ) ;
    if ( work->volume )
	GV_DestroyOtherActor( work->volume ) ;
}


static void ORG_LGT_OffSpotRainMessage( int name )
{
    GV_MSG msg ;
    int buffer = 0 ;/*スポット雨を消すメッセージID*/

    msg.address = name    ;
    msg.message = &buffer ;
    msg.message_len = 1   ;
    GV_SendMessage( &msg ) ;
}


static void ORG_LGT_DirectionChange( Work *work )
{
    if ( work->flag == 0 )
	return ;
    if ( !work->lens )
	work->lens = NewOrgaLensFlr_Game( &work->objs->objs[1].world,
					  &DG_ZeroVector,
					  50.0f, 48 ) ;

    GM_SeSetMode( SD_A_DAZZHIT1, &work->target.center, GM_SEMODE_BOMB ) ;

    CalcDirection( work, &work->pos, &GM_PlayerControl->mov,
		   &work->turn_x, &work->turn_y ) ;
    work->flag = 2 ;
}

/* 下が はみ出ないように変形させる */
static void ActVertexDeform( Work *work )
{
    static FVECTOR v0 = { -20000.0f, 0.0f, -14000.0f, 1.0f } ;
    static FVECTOR v1 = { -12000.0f, 0.0f, -12000.0f, 1.0f } ;
    FMATRIX  inv ;
    FVECTOR  v ;
    SVECTOR *scr = (void*)SCRPAD_ADDR ;
    int      i ;

    _sceVu0InversMatrix( &inv, &work->objs->objs[4].world ) ;

    for ( i=work->cvd->models[4].n_verts ; --i>=0 ; )
    {
	float z ;

	_sceVu0ApplyMatrix( &v,
			    &work->objs->objs[4].world,
			    &work->cvd->models[4].verts[i] ) ;
	z = (v.vx - v0.vx) * (v0.vz - v1.vz)/(v0.vx - v1.vx) + v0.vz ;
	v.vz = v.vz>z ? z : v.vz ;
	_sceVu0ApplyMatrix( &v, &inv, &v ) ;

	scr[i].vx = (short)v.vx ;
	scr[i].vy = (short)v.vy ;
	scr[i].vz = (short)v.vz ;
        scr[i].pad = 4096 ;
    }

    DG_RefineStripVertex( &work->vanm[0], work->cvd->models[4].verts_index ) ;
    DG_RefineStripVertex( &work->vanm[1], work->cvd->models[5].verts_index ) ;
}

static void Act( Work *work )
{
    GV_MSG *msg ;
    int i ;
    short x, y ;

    /* メッセージは どうですか？ */
    for ( i=GV_ReceiveMessage( work->name, &msg ) ; i>0 ; i--, msg++ )
        if ( !msg->message[0] )
            *((int **)msg->message[1]) = &work->flag ;
	else
	    ORG_LGT_DirectionChange( work ) ;/* 照明の向きを変える要求 */

    if ( work->flag == 2 )
    {
	/* 投光器の向きを変えるべきか */
	if ( !(GM_StagePlayTime & 0x1f) )
	{
	    CalcDirection( work, &work->pos, &GM_PlayerControl->mov, &x, &y ) ;

	    if ( (y - work->turn_y)/4096 )
		ORG_SendOrgaChangeLightMessage() ;
	}

	/* 投光器の向き指定の向きと現在の向きが異なると補間する */
	if ( work->turn_x != work->rot_x || work->turn_y != work->rot_y )
	{
	    work->rot_x += SpeedCurve( work->turn_x - work->rot_x, 2 ) ;
	    work->rot_y += SpeedCurve( work->turn_y - work->rot_y, 2 ) ;
	    RotateMatrixXY( work ) ;

	    /* はみ出る映り込みを削っている */
	    //if ( work->rot_y >= 0 )
		ActVertexDeform( work ) ;
	}
    }
}



static void ORG_LGT_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work    *work = (Work*)ptr ;
    extern void *NewBreakLight( FVECTOR *pos, FVECTOR *dir, int where ) ;
    FVECTOR  p ;

    if ( def->damaged & TARGET_POWER )
    {
	/* clear damage */
	GM_ClearTargetDamage( def ) ;

	/* オルガが撃った(USP)ので,向きが変わる */
	if ( off->side & PLAYER_SIDE && off->weapon_type & WP_USP )
	{
	    FMATRIX world ;

	    ORG_LGT_DirectionChange( work ) ;
	    TS_MakeMatrix( &world, &off->power->force, &def->hit ) ;
	    NewSpark( &world ) ;

	    return ;
	}

	/* 既に壊れているか 銃の弾以外は壊れない */
	if ( !(off->weapon_type & (WP_BULLET|WP_M92)) || work->vital <= 0 )
	    return ;
	BRK_UTL_CallOffenceWhenThrough( off, def ) ;

	if ( --work->vital > 0 )
	{
	    if ( GM_GameLevel >= GM_LEVEL_EXTREME )
		work->vital++ ;

	    GM_SetNoise( NOISE_S, &work->target.center, work->map ) ;
	    GM_SeSetMode( SD_A_LHIBI01, &work->target.center, GM_SEMODE_BOMB ) ;
	}
	else
	{
	    DG_OBJS *objs = work->objs ;
	    int i ;

	    if ( work->lens )
	    {
		ORG_SendOrgaExitLightMessage() ;
		GV_DestroyOtherActor( work->lens ) ;
		work->lens = NULL ;
	    }

	    DG_DestroyLightSphere( &work->target.center, 4000.f ) ;

	    if ( (work->objs = DG_MakeObjs( work->off,
					    DG_FLAG_FINISHCALC| DG_FLAG_SHADE,
					    0 )) )
	    {
		GM_SeSetMode( ORGA_SE_BRK_LIGHT,
			      &work->target.center,
			      GM_SEMODE_BOMB ) ;
		GM_SetNoise( NOISE_MM, &work->target.center, work->map ) ;

		work->flag = 0 ;
		if ( work->pr_name )
		    GCL_ExecProc( work->pr_name, NULL ) ;
		DG_QueueObjs( work->objs ) ;
		RotateMatrixXY( work ) ;

		DG_SetLightMatrix( work->objs, work->lights ) ;
		DG_GetLightMatrix( &work->pos, work->lights ) ;

		/* スポットライトを消す */
		if ( work->spot )
		{
		    GV_DestroyOtherActor( work->spot ) ;
		    work->spot = NULL ;
		}
		/* ボリューミックライトを消す */
		if ( work->volume )
		{
		    GV_DestroyOtherActor( work->volume ) ;
		    work->volume = NULL ;
		}

		ORG_LGT_OffSpotRainMessage( work->lt_name ) ;

		/* 頂点アニメを解放 */
		for ( i=2 ; --i>=0 ; )
		    if( work->vanm[i].mem[0] )
			DG_FreeAnimVertsBuffer( &work->vanm[i] ), work->vanm[i].mem[0] = NULL ;

		DG_DequeueObjs( objs ) ;
		DG_FreeObjs( objs ) ;

		_sceVu0ApplyMatrix( &p, &work->objs->objs[1].world, &ORG_LGT_BreakPhase ) ;
		NewBreakLight( &p, &p, work->map ) ;
		p.vz +=  50.0f ;
		p.vy += 100.0f ;
		NewBreakLight( &p, &p, work->map ) ;
		p.vz += 100.0f ;
		NewBreakLight( &p, &p, work->map ) ;
	    }
	}
    }
}

static inline int ORG_LGT_InitTarget( Work *work, int map ) 
{
    FVECTOR t_size, t_pos ;
    FVECTOR uv = { work->off->models[1].ux, work->off->models[1].uy, work->off->models[1].uz, 0 } ;
    FVECTOR lv = { work->off->models[1].lx, work->off->models[1].ly, work->off->models[1].lz, 0 } ;
    FMATRIX *mtx ;
    TARGET       *t = &work->target ;
    POWER_TARGET *p = &work->power  ;

    mtx =  &work->objs->objs[1].world ;
    _sceVu0ApplyMatrix( &uv, mtx, &uv ) ;
    _sceVu0ApplyMatrix( &lv, mtx, &lv ) ;
    t_size.vx = fpu_Abs(uv.vx - lv.vx)*0.5f ;
    t_size.vy = fpu_Abs(uv.vy - lv.vy)*0.5f ;
    t_size.vz = fpu_Abs(uv.vz - lv.vz)*0.5f ;
    _sceVu0AddVector  ( &t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
    _sceVu0AddVector  ( &t_pos, &t_pos, (FVECTOR *)mtx->m[W] ) ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH, map,
		  BOTH_SIDE, &t_size, &DG_ZeroVector ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, ORG_LGT_TargetCallBack, work ) ;
    GM_MoveTarget( t, &t_pos ) ;
    GM_PutTarget( t ) ;
    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

static int GetResources( Work *work, int name, int where )
{
    int      i, buf[3] ;
    int      mdl_id ;
    FVECTOR  aim    ;

    if ( (i=GCL_GetOptionValue( 'l', 0 )) )
	work->lit = GV_GetCache( GV_CacheID( i, 'l') ) ;
    else
	return -1 ;

    if ( GCL_GetOption( 'm' ) )
    {
	mdl_id = GCL_GetNextInt() ;
	work->on  = GV_GetCache( GV_CacheID( mdl_id, 'k' ) ) ;
	work->cvd = GV_GetCache( GV_CacheID( mdl_id, 'c' ) ) ;
	mdl_id = GCL_GetNextInt() ;
	work->off = GV_GetCache( GV_CacheID( mdl_id, 'k' ) ) ;

	work->objs = DG_MakeObjs( work->on, DG_FLAG_FINISHCALC| DG_FLAG_SHADE, 0 ) ;
	DG_QueueObjs( work->objs ) ;
	DG_SetLightMatrix( work->objs, work->lights ) ;

	/* 照り返し効果を頂点アニメさせる */
	DG_MakeAnimVertsBuffer( &work->vanm[0], &work->objs->objs[4],
				DG_VANIME_VERTS|DG_VANIME_SINGLE ) ;
	DG_MakeAnimVertsBuffer( &work->vanm[1], &work->objs->objs[5],
				DG_VANIME_VERTS|DG_VANIME_SINGLE ) ;
    }
    else
	return -1 ;

    if ( GCL_GetOption( 'n' ) )
    {
	work->bg_name = GCL_GetNextInt() ;
	work->lt_name = GCL_GetNextInt() ;
    }
    else
	return -1 ;
    work->pr_name = GCL_GetOptionValue( 'b', 0 ) ;

    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &work->pos ) ;
	DG_GetLightMatrix( &work->pos, work->lights ) ;
    }

    if ( GCL_GetOption( 'a' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &aim ) ;

	CalcDirection( work, &work->pos, &aim, &work->turn_x, &work->turn_y ) ;

	/* 目標回転 */
	work->rot_y = work->turn_y ;
	work->rot_x = work->turn_x ;
    }
    RotateMatrixXY( work ) ;

    OK_SetDynamicLight( (FVECTOR*)&work->objs->objs[1].world.m[3], &DG_ZeroSVector ) ;

    {
	extern void *NewCircleLightSpotCalled( int name, int where,
					       FMATRIX *root, FVECTOR *pos, FVECTOR *aim,
					       int length, int width, int n_poly ) ;
	FVECTOR off_pos = { 0.0f, -1000.0f, 500.0f, 1.0f } ;

	work->volume = NewCircleLightSpotCalled( work->lt_name, where, 
						 &work->objs->objs[1].world, &off_pos, NULL,
						 15000, 1000, 16 ) ;
    }

    {
	extern void *NewSpotLight( FMATRIX *mat,
				   float umbra, float penumbra,
				   float range, int color ) ;

	work->spot = NewSpotLight( &work->objs->objs[1].world, 45.0f, 45.0f, 15000.0f, 0x807f7f64 ) ;
    }

    ORG_LGT_InitTarget( work, where ) ;
    work->vital = GM_GameLevel >= GM_LEVEL_EXTREME ? 8 : ORGA_N_VITAL ;
    work->name  = name  ;
    work->map   = where ;
    work->flag  = 1     ; 

    return 0 ;
}


void *NewOrgaPutSearchLightObject( int name, int where )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if( work != NULL )
    {
        GV_SetActor( &work->actor, Act, Die ) ;
        if( GetResources( work, name, where ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
