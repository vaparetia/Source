//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_tv.c
   TV壊れ

   1999/12/13 T. Morita
   $Id: brk_tv_ini.c,v 1.1.1.3 2002/11/19 11:45:49 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_tv.h"




int BRK_TV_InitPrimitive( DG_PRIM2 **prim, DG_TEX *t, int i, int alpha,
			  float raise, float *uv, /*u_max,u_min,v_max,v_minの順番*/
			  float lz, float uz, float ly, float uy, float lx )
{
    DG_PRIM2_UVRGB *u0, *u1 ;
    FVECTOR *p0, *p1 ;

    if ( !(*prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_SHADE, i, 4 )) )
	PERROR( "Cannot make PRIM2 ( May be no memory ) :: NewPutTVObject\n" ) ;

    u0 = (*prim)->uvrgb[0], u1 = (*prim)->uvrgb[1] ;
    p0 = (*prim)->pos[0]  , p1 = (*prim)->pos[1]   ;
    DG_ConfigPrim2Tex( *prim, t ) ;
    DG_SetPrim2Alpha( *prim, alpha ) ;
//    DG_ConfigPrim2Tex( *prim, t ) ;
    (*prim)->raise = (int)(raise*4.0f) ;
    for ( ; --i>=0 ; u0 += 4, u1 += 4, p0 += 4, p1 += 4 )
    {
        u0[0].u = u0[2].u = u1[0].u = u1[2].u = FTOI12( uv[0] * t->u_scale + t->u_offset ) ;
        u0[1].u = u0[3].u = u1[1].u = u1[3].u = FTOI12( uv[1] * t->u_scale + t->u_offset ) ;
        u0[0].v = u0[1].v = u1[0].v = u1[1].v = FTOI12( uv[2] * t->v_scale + t->v_offset ) ;
        u0[2].v = u0[3].v = u1[2].v = u1[3].v = FTOI12( uv[3] * t->v_scale + t->v_offset ) ;
        u0[0].q = u0[1].q = u0[2].q = u0[3].q = u1[0].q = u1[1].q = u1[2].q = u1[3].q = 4096 ;
        u0[0].f = u0[1].f = u1[0].f = u1[1].f = 0x8fff ;
        u0[2].f = u0[3].f = u1[2].f = u1[3].f = 0x0fff ;

        u0[0].r = u0[1].r = u0[2].r = u0[3].r = u1[0].r = u1[1].r = u1[2].r = u1[3].r = 
        u0[0].g = u0[1].g = u0[2].g = u0[3].g = u1[0].g = u1[1].g = u1[2].g = u1[3].g = 
        u0[0].b = u0[1].b = u0[2].b = u0[3].b = u1[0].b = u1[1].b = u1[2].b = u1[3].b = 
        u0[0].a = u0[1].a = u0[2].a = u0[3].a = u1[0].a = u1[1].a = u1[2].a = u1[3].a = 128 ;

	p0[0].vz = p0[2].vz = p1[0].vz = p1[2].vz = lz ;
	p0[1].vz = p0[3].vz = p1[1].vz = p1[3].vz = uz ;
	p0[0].vy = p0[1].vy = p1[0].vy = p1[1].vy = ly ;
	p0[2].vy = p0[3].vy = p1[2].vy = p1[3].vy = uy ;
	p0[0].vx = p0[1].vx = p0[2].vx = p0[3].vx =
	    p1[0].vx = p1[1].vx = p1[2].vx = p1[3].vx = lx + raise*0.25f ;
    }
    return 0 ;
}




static void BRK_TV_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    static float wipe_uv[]   = { 1.0f, 0.0f, 1.0f, 0.0f } ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	if ( off->weapon_type & (WP_BULLET|WP_M92) )
	    _sceVu0CopyVector( &work->hit, &def->hit ) ;
	else if ( off->weapon_type & (WP_BLOW|WP_BLAST|WP_PUNCHALL) )
	    _sceVu0CopyVector( &work->hit, &def->offset ) ;
	else
	    return ;

	/* 弾を止める */
	BRK_UTL_CallOffenceWhenThrough( off, def ) ;

	if ( !work->count )
	{
	    BRK_TV_InitPrimitive( &work->wipe,
				  DG_GetTexture( GV_StrCode( BRK_TV_WIPE ) ),
				  1,
				  SCE_GS_SET_ALPHA(2,0,0,1,0),
				  75.0f, wipe_uv,
				  def->hit.vz-50.0f, def->hit.vz+50.0f,
				  def->hit.vy-50.0f, def->hit.vy+50.0f,
				  work->def->lx ) ;
	    GM_SeSetMode( SD_A_MONITA01, &def->hit, GM_SEMODE_BOMB ) ;
	}
	else
	    return ;
	work->wipe->raise = 100 ;

	/* 壊れプロックを呼び出す */
	if ( work->proc_id )
	    GCL_ExecProc( work->proc_id, NULL ) ;

	work->count = 1 ;
#if 0
	if ( work->scn_iv )
	    GM_FreePrim2( work->scn_iv ), work->scn_iv = NULL ;
	if ( work->scn_al )
	    GM_FreePrim2( work->scn_al ), work->scn_al = NULL ;
	if ( work->inv    )
	    GM_FreePrim2( work->inv    ), work->inv    = NULL ;
#endif
    }
}

int BRK_TV_InitTarget( Work *work, DG_DEF *def, int where )
{
    FVECTOR t_size, t_pos ;

    t_size.vx = (def->ux - def->lx)*0.5f ;
    t_size.vy = (def->uy - def->ly)*0.5f ;
    t_size.vz = (def->uz - def->lz)*0.5f ;
    t_pos.vx  = def->tx + (def->ux + def->lx)*0.5f ;
    t_pos.vy  = def->ty + (def->uy + def->ly)*0.5f ;
    t_pos.vz  = def->tz + (def->uz + def->lz)*0.5f ;

    GM_SetTarget( &work->target, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( &work->target, &work->power,
		       POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->target, BRK_TV_TargetCallBack, work ) ;
    GM_PutTarget( &work->target ) ;

//    NewTargetView( &work->target,  200, 50, 32 ) ;

    return 0 ;
}

#if 0
static int BRK_TV_InitModel( DG_OBJS **objs, int id, int where, FMATRIX *mtx, LIT_DEF *lit )
{
    DG_DEF  *def  ;

    if ( !(def = GV_GetCache( GV_CacheID( id, 'k' ) )) )
	PERROR( "Cannot find KMSModel( Not in data.cnf ) :: NewPutTVObject\n" ) ;
    if ( !(*objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Cannot create DG_OBJS( Maybe no memory ) :: NewPutTVObject\n" ) ;
    DG_MakePreshade( *objs, lit ) ;
    DG_QueueObjs( *objs ) ;
    GM_GroupObjs( *objs, where ) ;
    (*objs)->world = *mtx ;

    return 0 ;
}
#endif

int BRK_TV_GetOptions( Work *work, int name, int where )
{
    int      buf[3], id ;
    SVECTOR  rot ;
    FMATRIX  mtx = DG_UnitMatrix ;
    LIT_DEF *lit ;
    static float screen_uv[] = { 0.5f, 0.0f, 1.0f, 0.0f } ;
    static float spec_uv[]   = { 1.0f, 0.0f, 1.0f, 0.0f } ;

    work->scroll = 0.0f ;
    work->speed  = GCL_GetOptionValue( 's', 100 ) * 0.000005f ;
    work->count  = 0 ;

    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;

    if ( GCL_GetOption( 'r' ) != NULL )
    {
	GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;
	RotateMatrix( &mtx, &DG_UnitMatrix, &rot ) ;
    }
    if ( GCL_GetOption( 'p' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, (FVECTOR*)&mtx.m[W]   ) ;
    }
    if ( !(id = GCL_GetOptionValue( 'l', 0 )) )
	PERROR( "No light ID(-l option missing) :: NewPutTVObject\n" ) ;
    if ( !(lit = GV_GetCache( GV_CacheID( id, 'l' ) )) )
	PERROR( "Cannot find Lt2File( Not in data.cnf ) :: NewPutTVObject\n" ) ;

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No model ID(-m option missing) :: NewPutTVObject\n" ) ;
    if ( !(work->def = GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	PERROR( "Cannot find KMSModel( Not in data.cnf ) :: NewPutTVObject\n" ) ;

    BRK_TV_InitPrimitive( &work->scn_al,
			  DG_GetTexture( GV_StrCode( BRK_TV_SCREEN ) ),
			  1,
			  SCE_GS_SET_ALPHA(0,2,0,2,0),
			  50.0f, screen_uv,
			  work->def->lz, work->def->uz, work->def->ly, work->def->uy,
			  work->def->lx ) ;
    work->scn_al->raise = -1000 ;
    BRK_TV_InitPrimitive( &work->scn_iv,
			  DG_GetTexture( GV_StrCode( BRK_TV_SCREEN ) ),
			  1,
			  SCE_GS_SET_ALPHA(0,2,0,2,0),
			  0.0f, screen_uv,
			  work->def->lz, work->def->uz, work->def->ly, work->def->uy,
			  work->def->lx ) ;
    work->scn_iv->raise = -1000 ;
    BRK_TV_InitPrimitive( &work->inv,
			  DG_GetTexture( GV_StrCode( BRK_TV_WHITE  ) ),
			  1,
			  SCE_GS_SET_ALPHA(0,1,0,2,0),
			  25.0f, spec_uv,
			  work->def->lz, work->def->uz, work->def->ly, work->def->uy,
			  work->def->lx ) ;
    work->inv->raise = -1000 ;
    BRK_TV_InitPrimitive( &work->spec,
			  DG_GetTexture( GV_StrCode( BRK_TV_SPECULAR ) ),
			  1,
			  SCE_GS_SET_ALPHA(0,2,0,1,0),
			  100.0f, spec_uv,
			  work->def->lz, work->def->uz, work->def->ly, work->def->uy,
			  work->def->lx ) ;
    work->spec->raise = 100 ;

    //DG_InvisiblePrim2( work->scn_al ) ;
    //DG_InvisiblePrim2( work->scn_iv ) ;
    //DG_InvisiblePrim2( work->inv ) ;

    return 0 ;
}

