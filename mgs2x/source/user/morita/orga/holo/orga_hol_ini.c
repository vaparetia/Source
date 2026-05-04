//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_hol.c 
   オルガ オルガ戦専用プットホロオブジェ

   2000/01/21 T.Morita
   $Id: orga_hol_ini.c,v 1.4 2002/11/23 12:28:38 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "orga_holo.h"


/* 当たりを張る */
int ORG_HOL_InitTarget( TARGET       *t,
			POWER_TARGET *p,
			Work *work, int map, int flag,
			float *l, float *u, FMATRIX *mtx ) 
{
    FVECTOR t_size, t_pos ;
    FVECTOR uv = { u[X], u[Y], u[Z], 0 } ;
    FVECTOR lv = { l[X], l[Y], l[Z], 0 } ;

    _sceVu0ApplyMatrix( &uv, mtx, &uv ) ;
    _sceVu0ApplyMatrix( &lv, mtx, &lv ) ;
    t_size.vx = fpu_Abs(uv.vx - lv.vx)*0.5f ;
    t_size.vy = fpu_Abs(uv.vy - lv.vy)*0.5f ;
    t_size.vz = fpu_Abs(uv.vz - lv.vz)*0.5f ;
    _sceVu0AddVector  ( &t_pos, &uv, &lv ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
    _sceVu0AddVector  ( &t_pos, &t_pos, (FVECTOR *)mtx->m[W] ) ;

    GM_SetTarget( t, flag, map, BOTH_SIDE, &t_size, &DG_ZeroVector ) ;
    if ( p )
	GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    if ( work )
	GM_SetTargetCallBack( t, ORG_HOL_TargetCallBack, work ) ;
    GM_MoveTarget( t, &t_pos ) ;
    GM_PutTarget( t ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

/* モデルを複写する */
static void DuplicateObjs( DG_OBJS *dst, DG_OBJS *src )
{
    int i ;
    DG_OBJ *p=dst->objs, *o=src->objs ;

#ifdef PSX2
    for ( i=src->n_models ; --i>=0 ; p++, o++ )
	  p->verts = o->verts, p->norms = o->norms ;
#else
#ifndef KP_WINDOWS
    for ( i=src->n_models ; --i>=0 ; p++, o++ )
	  p->vbuff = o->vbuff ;
#else
    for ( i=src->n_models ; --i>=0 ; p++, o++ )
	{
		p->vbuff     = o->vbuff ;
		p->d3d_vbuff = o->d3d_vbuff ;
	}
#endif
#endif
}

int ORG_HOL_InitModel( Work *work, DG_DEF *def, FMATRIX *mtx )
{
    /* おお元を変更しちゃう（正常なクリッピング処理のための変更） */
    def->n_models = def->n_x_models = 1 ;
    def->lx = -3000.0f, def->lz = -20000.0f ;/*DG_DEF*/
    def->ux =  3000.0f, def->uz =  40000.0f ;
    def->models->lx = -3000.0f, def->models->lz = -20000.0f ;/*DG_MDL*/
    def->models->ux =  3000.0f, def->models->uz =  20000.0f ;

    work->objs   = DG_MakeObjs( def, DG_FLAG_SHADE| DG_FLAG_ONEPIECE| DG_FLAG_SHADOWMAKE, 0 ) ;
    work->objs_r = DG_MakeObjs( def, DG_FLAG_SHADE| DG_FLAG_ONEPIECE, 0 ) ;
    work->objs->world   = *mtx ;
    _sceVu0ScaleVector( (FVECTOR *)&mtx->m[Y], (FVECTOR *)&mtx->m[Y], -1.0f ) ;/* マトリックスを反転 */
    work->objs_r->world = *mtx ;

    DG_MakeAnimVertsBuffer( &work->v_anm, work->objs->objs, DG_VANIME_VERTS|DG_VANIME_NORMS ) ;

    DuplicateObjs( work->objs_r, work->objs ) ;
    DG_QueueObjs( work->objs   ) ;
    DG_QueueObjs( work->objs_r ) ;

    return 0 ;
}

int ORG_HOL_InitHoloParam( Work *work )
{
    work->himo_l.vx = work->himo_r.vx = -16000 ;
    work->himo_l.vy = work->himo_r.vy = 12870 ;
    work->himo_l.vz = -15850 ;
    work->himo_r.vz = -18130 ;
    _sceVu0CopyVector( &work->vel, &DG_ZeroVector ) ;

    work->flag = ORGA_HOL_COVERED ;
    work->key_idx = 0 ;
    work->key   = 0   ;
    work->tics  = 0   ;
    work->tgl   = 0   ;
    work->ratio = 0   ;
    work->rate  = 128 ;

    work->life_l = work->life_r = 8 ; /* エクストリーム用耐久値 */

    return 0 ;
}

int ORG_HOL_InitLzsh( Work *work, int id )
{
    work->buffer[0] = GV_Malloc( ORGA_HOL_BUFSIZE*4 ) ;
    work->buffer[1] = work->buffer[0] + ORGA_HOL_N_VERTS*XYZ*2 ;

    work->lzh   = UTL_LzshDecodeInit( GV_GetCache( GV_CacheID( id, 'a') ),
				      work->buffer[0], ORGA_HOL_BUFSIZE*2 ) ;
    if ( !work->lzh )
	PERROR( "Can't initialize LZSH handler!! :NewPutHoloObject\n" ) ;
    return 0 ;
}

static float BRK_HOLO_Lower[] = { -1110.001465f, 107.001724f, -1423.437378f } ;
static float BRK_HOLO_Upper[] = {  1116.406250f, 1753.125f  ,  1504.687256f } ;
static float BRK_HOLO_Lower2[]= { -1110.001465f, 0.0f       , -1500.0f } ;
static float BRK_HOLO_Upper2[]= {  1116.406250f, 1753.125f  ,  1600.0f } ;

int ORG_HOL_GetOptions( Work *work, int name, int where )
{
    int   i, buf[3] ;
    char *c ;
    FMATRIX mtx ;
    DG_DEF  *def ;

    work->pr_name = GCL_GetOptionValue( 'b', 0 ) ;

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No model ID!! :NewPutHoloObject\n" ) ;
    for ( i=0 ; (c=GCL_NextStr()) ; i++ )
	work->id[i] = GCL_GetInt( c ) ;
    work->next = 0 ;

    if ( GCL_GetOption( 'r' ) )
    {
	i = GCL_GetNextInt() & 0x0fff ;
	i -= 4096 & (i<<1) ;
	_sceVu0RotMatrixY( &mtx, &DG_UnitMatrix, i*(float)M_PI/2048.0F ) ;
	/*mtx = DG_UnitMatrix ;
	mtx.m[Y][Y] *= -1 ;
	_sceVu0RotMatrixY( &mtx, &work->objs->world, i*(float)M_PI/2048.0F ) ;*/
    }
    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR *)&mtx.m[W] ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &work->pos ) ;
	mtx.m[W][W] = 1.0f ;
#if DEBUG_MODE
	_sceVu0CopyMatrix( &work->org_mtx, (FVECTOR *)&mtx ) ;
#endif
    }

    if ( !(i = GCL_GetOptionValue( 'c', 0 )) )
	PERROR( "No CV2 ID!! :NewPutHoloObject\n" ) ;
    if ( !(def  = GV_GetCache( GV_CacheID(i, 'k') )) )
	PERROR( "No KMS-DG_DEF!! :NewPutHoloObject\n" ) ;
    if ( !(work->cdef = GV_GetCache( GV_CacheID(i, 'c') ) ) )
	PERROR( "No Cv2-CV2_DEF!! :NewPutHoloObject\n" ) ;
    if ( ORG_HOL_InitTarget( &work->bul_wall, NULL,
			     NULL, where, TARGET_DEFENSE,
			     BRK_HOLO_Lower, BRK_HOLO_Upper, &mtx ) < 0 )
	return -1 ;
    if ( ORG_HOL_InitTarget( &work->target, &work->power,
			     work, where, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH|TARGET_SEEK,
			     BRK_HOLO_Lower2, BRK_HOLO_Upper2, &mtx ) < 0 )
	return -1 ;

    if ( ORG_HOL_InitModel( work, def, &mtx ) < 0 )
	return -1 ;

    work->splash = NewObjectSplash( work->objs, i, &work->v_anm ) ;

    return 0 ;
}
