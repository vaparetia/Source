//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_plt_ini.c
   皿壊れ

   2000/04/25 T. Morita
   $Id: brk_plt_ini.c,v 1.1.1.3 2002/11/19 11:45:40 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_plate.h"
#include "../brk_utl/brk_utl.x"



/* 破片の埃 効果  */
int BRK_PLT_InitPrimitive( Work *work )
{
    if ( !(work->dust = BRK_UTL_MakeSPRT( BRK_PLT_N_DUST,
					  GV_StrCode( "powder03_alp" ),
					  0 )) )
	PERROR( "No cannot make DG_PRIM2(work->dust)(May be no memory) :: NewPutPlateObject\n" ) ;
    work->n_dust = BRK_PLT_INACTIVE ;

    return 0 ;
}



/***

  壊れハザードの初期化

  ***/
int BRK_PLT_InitHazard( Work *work )
{
    int hzd_id ;

    hzd_id = GCL_GetOptionValue( 'h', BRK_HZD_W01D_KITCHEN ) ;
    if ( !(work->hzd = BRK_InitHazard( hzd_id )) )
	return -1 ;
    return 0 ;
}

/***

  ターゲットの初期化

  ***/
/* 中心及びサイズの計算 */
void BRK_PLT_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv )
{
    FVECTOR u = { uv[X],uv[Y],uv[Z],0 }, l = { lv[X],lv[Y],lv[Z],0 } ;

    _sceVu0SubVector( t_size, &u, &l ) ;
    _sceVu0ScaleVector( t_size, t_size, 0.5f ) ;
    _sceVu0AddVector( t_pos, &u, &l ) ;
    _sceVu0ScaleVector( t_pos , t_pos , 0.5f ) ;
}

/* ターゲットの初期化 */
int BRK_PLT_InitTarget( TARGET *t, POWER_TARGET *p, int map,
			DG_DEF *def, FVECTOR *pos,
			void (*callback)( TARGET *off, TARGET *def, void *ptr ), void *ptr )
{
    FVECTOR t_size, t_pos ;

    BRK_PLT_MakeSizeAndCenter( &t_size, &t_pos, &def->ux, &def->lx ) ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK, map, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, callback, ptr ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget( t, pos ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

/***

  破片の初期化

  ***/
/* 小さい破片の初期化 */
int BRK_PLT_InitPiece( Work *work, int where )
{
    int    i ;
    PART  *p ;
    DG_COMDL_POS *pos_s, *pos_l ;

    if ( work->d_def )
    {
	if ( !(work->piece_l = DG_MakeComdl( work->d_def->models[0].packs,
					     /*DG_COMDL_DEFAULT*/DG_COMDL_SEMITRANS, BRK_PLT_N_PIECE/2, 0)) )
	    PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutPlateObject\n" ) ;
	DG_QueueComdlObjs( work->piece_l ) ;
	GM_GroupObject( work->piece_l, where ) ;
	if ( !(work->piece_s = DG_MakeComdl( work->d_def->models[1].packs,
					     /*DG_COMDL_DEFAULT*/DG_COMDL_SEMITRANS, BRK_PLT_N_PIECE/2, 0)) )
	    PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutPlateObject\n" ) ;
	DG_QueueComdlObjs( work->piece_s ) ;
	GM_GroupObject( work->piece_s, where ) ;

	pos_l = work->piece_l->pos ;
	pos_s = work->piece_s->pos ;
	for ( i=BRK_PLT_N_PIECE/2, p=work->piece ; --i>=0 ; p+=2 )
	{
	    p[0].pos  = pos_s++ ;
	    p[0].flag = 0 ;
	    p[1].pos  = pos_l++ ;
	    p[1].flag = 0 ;
	}
	work->n_piece = BRK_PLT_INACTIVE ;
    }
    return 0 ;
}

/* 皿破片の初期化 */
int BRK_PLT_InitPart( Work *work, int where )
{
    int   i, j, size ;
    int   n_comdl, n_plate ;
    PART *p    ;

    n_comdl = work->p_def->n_models ;
    n_plate = (work->n_piled & ~BRK_PLT_INACTIVE) * BRK_PLT_N_PLATE ;
    size =  ((sizeof(DG_COMDL *) * n_comdl + 0xf) & ~0xf) + sizeof(PART) * n_comdl * n_plate ;
    if ( !(work->comdl = GV_Malloc( size )) )
        PERROR( "Can't alocate memory :: NewPutPlateObject\n" ) ;
    for ( i=n_comdl ; --i>=0 ; )/* mallocからのメモリなので初期化しておく */
	work->comdl[i] = NULL ;
    for ( i=n_comdl ; --i>=0 ; )
    {
        if ( !(work->comdl[i] = DG_MakeComdl( work->p_def->models[i].packs, DG_COMDL_SEMITRANS/*DG_COMDL_DEFAULT*/,
					      n_plate, 0 )) )
            PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutPlateObject\n" ) ;
        DG_QueueComdlObjs( work->comdl[i] ) ;
	GM_GroupObject( work->comdl[i], where ) ;
    }

    /* アラインメントを考えてメモリを配置する */
    work->part = (PART *)( ( (int)(work->comdl + n_comdl) + 0xf ) & ~0xf ) ;
    for ( j=0, p=work->part ; j<n_plate ; j++ )
	for ( i=0 ; i<n_comdl ; p++, i++ )
        {
	    work->comdl[i]->pos[j].color.vw = 0 ;
	    p->mdl  = &work->p_def->models[i] ;
            p->pos  = &work->comdl[i]->pos[j] ;
	    p->flag = 0 ;
        }
    work->n_part = BRK_PLT_INACTIVE ;

    return  0 ;
}

/* 一枚皿の初期化 */
int BRK_PLT_InitPlate( Work *work, int where )
{
    int    i, n_plate ;
    PLATE *p ;
    PART  *parts = work->part ;

    n_plate = (work->n_piled & ~BRK_PLT_INACTIVE) * BRK_PLT_N_PLATE ;
    if ( !(work->plate = GV_Malloc( sizeof(PLATE) * n_plate)) )
        PERROR( "Can't alocate memory :: NewPutPlateObject\n" ) ;
    for ( i=n_plate, p=work->plate ; --i>=0 ; )/* mallocからのメモリなので初期化しておく */
	p->objs = NULL ;

    for ( i=n_plate, p=work->plate ; --i>=0 ; p++, parts += work->p_def->n_models )
    {
	if ( !(p->objs = DG_MakeObjs( work->m_def, DG_FLAG_SHADE| DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Can't initialize DG_OBJS(Maybe plate model missing) :: NewPutPlateObject\n" ) ;

	DG_SetLightMatrix( p->objs, p->lights ) ;
	DG_QueueObjs( p->objs ) ;
	DG_InvisibleObjs( p->objs ) ;
	GM_GroupObjs( p->objs, where ) ;
	p->work  = work  ;
	p->parts = parts ;
    }
    work->n_plate = BRK_PLT_INACTIVE ;

    return  0 ;
}

static int BRK_PLT_InitPiled( Work *work, PILED *p, DG_DEF *def, int where,
			      FVECTOR *pos, SVECTOR *rot, int flag )
{
    if ( !(p->objs = DG_MakeObjs( def, DG_FLAG_SHADE| DG_FLAG_ONEPIECE, 0 )) )
	PERROR( "Can't initialize DG_OBJS(Maybe no memory) :: NewPutPlateObject\n" ) ;

    RotateMatrix( &p->objs->world, &DG_UnitMatrix, rot ) ;
    _sceVu0CopyVector( (FVECTOR *)&p->objs->world.m[W], pos ) ;
    p->objs->world.m[W][W] = 1.0f ;
    DG_QueueObjs( p->objs ) ;
    GM_GroupObjs( p->objs, where ) ;
    DG_SetLightMatrix( p->objs, p->lights ) ;
    BRK_UTL_GetLightMatrix( pos, p->lights, 1.0f, work->where ) ;

    BRK_PLT_InitTarget( &p->target, &p->power, where,
			def, (FVECTOR *)&p->objs->world.m[W],
			BRK_PLT_PiledTargetCallBack, p ) ;
    p->flag  = flag ;
    p->work  = work ;
    p->vel_y = 0.0f ;

    return 0 ;
}

void BRK_PLT_InitEneFind( Work *work )
{
    int grp, zone ;
    int addr ;
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;

    HZX_Pos2Zone( &work->piled->target.center, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, &work->piled->target.center, addr, EF_TYPE_NO_FIND ) ;
    GM_PutEneFind( &work->ene_find ) ;
}

void BRK_PLT_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_PLT_AlreadyMessy( work ) ;
}

/***

  オプションの取得

  ***/
/* オプションの取得 及び 積み上げ皿の初期化 */
int BRK_PLT_GetOptions( Work *work, int where )
{
    int      i, j ;
    PILED   *p ;
    DG_DEF  *def ;

    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No plate Model ID (-model option missing) :: NewPutPlateObject\n" ) ;
    if ( !(def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannnot find PILED-PLATE KMS-MODEL( not in data.cnf ) :: NewPutPlateObject\n" ) ;
    if ( !(work->m_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannnot find PLATE KMS-MODEL( not in data.cnf ) :: NewPutPlateObject\n" ) ;
    if ( !(work->p_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	PERROR( "Cannnot find BROKEN PLATE KMS-MODEL( not in data.cnf ) :: NewPutPlateObject\n" ) ;
    //if ( GCL_NextStr() )
	if ( !(work->d_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	    PERROR( "Cannnot find BROKEN DUST KMS-MODEL( not in data.cnf ) :: NewPutPlateObject\n" ) ;

    if ( !(work->n_piled = GCL_GetOptionValue( 'n', 0 )) )
	PERROR( "The number of plate need more than one (-number option maybe missing) :: NewPutPlateObject\n" ) ;

    if ( !(work->piled = GV_Malloc( sizeof(PILED) * work->n_piled )) )
	PERROR( "Cannot allocate memory for <work->piled> :: NewPutPlateObject\n" ) ;
    for ( i=work->n_piled, p=work->piled ; --i>=0 ; p++ ) /* malloc からの初期化なので NULL にする */
	p->objs = NULL ;

    if ( !GCL_GetOption( 'p' ) )
	PERROR( "No rotations and positions (-param option missing) :: NewPutPlateObject\n" ) ;
    for ( p=work->piled ; GCL_NextStr() ; )
    {
	IVECTOR  buf ;
	SVECTOR  rot ;
	FVECTOR  pos ;

	j = GCL_GetNextInt() - 1 ;
        GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;
        GCL_GetIV( GCL_NextStr(), (int  *)&buf ) ;
        vu0_IV0toFV( &buf, &pos ) ;

	BRK_PLT_InitPiled( work, p++, def, where, &pos, &rot, BRK_F_IS_PARENT ) ;
	while( --j>=0 )
	{
	    (p-1)->flag |= BRK_F_HAS_CHILD ;
	    pos.vy += (p-1)->target.size.vy * 2 ;
	    BRK_PLT_InitPiled( work, p++, def, where, &pos, &rot, 0 ) ;
	}
    }
    //work->n_piled |= BRK_PLT_INACTIVE ;

    return 0 ;
}
