//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_gls_ini.c
   ガラス壊れ

   1999/11/26 T. Morita
   $Id: brk_b_gls_ini.c,v 1.1.1.3 2002/11/19 11:45:21 Yoshizawa1 Exp $
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

#include "brk_big_glass.h"

/*
  第５番目の点は,優先間違いを起こさないためのダミー頂点
*/
static int InitPrimitive(  DG_PRIM2 *p, DG_TEX *t, FVECTOR *pos )
{
    DG_PRIM2_UVRGB *u = p->uvrgb[0] ;
    FVECTOR        *v = p->pos[0]   ;
    FVECTOR cen = *pos ;
    int i ;

    DG_ConfigPrim2Tex( p, t ) ;
    cen.vw = 1.0f ;
    for ( i=BRK_BGLS_N_SCAR ; --i>=0 ; u+=5, v+=5 )
    {
        u[0].q = u[1].q = u[2].q = u[3].q = u[4].q = 4096 ;
        u[0].f = u[1].f =  u[4].f = 0x8fff ;    /* ダミー頂点も 非描画フラグを立てる */
        u[2].f = u[3].f = 0x0fff ;

        u[0].r = u[0].g = u[0].b = 
        u[1].r = u[1].g = u[1].b = 
        u[2].r = u[2].g = u[2].b = 
        u[3].r = u[3].g = u[3].b = 128 ;
        u[4].r = u[4].g = u[4].b = u[4].a = 0 ; /* ダミー頂点は 念のため色関係は0にする */

	v[0] = v[1] = v[2] = v[3] = v[4] = cen ;/* ソートは平均なので,頂点を中心に集めておく */
    }
    return 0 ;
}

/*
  第５番目の点は,優先間違いを起こさないためのダミー頂点
*/
int BRK_BGLS_InitPrimitive( Work *work )
{
    DG_TEX *t = DG_GetTexture( GV_StrCode("g_dankon_alp") ) ;
    FVECTOR ub = { work->objs->def->ux,
		   work->objs->def->uy,
		   work->objs->def->uz, 0 } ;
    FVECTOR lb = { work->objs->def->lx,
		   work->objs->def->ly,
		   work->objs->def->lz, 0 } ;
    FVECTOR pos ;

    _sceVu0AddVector( &pos, &ub, &lb ) ;
    _sceVu0ScaleVector( &pos, &pos, 0.5f ) ;
    _sceVu0AddVector( &pos, &pos, (FVECTOR *)work->objs->world.m[W] ) ;

    /*表面のプリミティブ*/
    if ( !(work->frac[0] = GM_MakePrim2( DG_PRIM2_POLY       |
					 DG_PRIM2_SHADE      |
					 DG_PRIM2_TEX        |
					 DG_PRIM2_SINGLEBUFF |
					 DG_PRIM2_ALPHA,
					 BRK_BGLS_N_SCAR, 5 )) )
	PERROR( "No cannot make DG_PRIM2(frac[0])(May be no memory) :: NewPutBigGlass\n" ) ;
    InitPrimitive( work->frac[0], t, &pos ) ;

    /*裏面のプリミティブ*/
    if ( !(work->frac[1] = GM_MakePrim2( DG_PRIM2_POLY |
					 DG_PRIM2_SHADE|
					 DG_PRIM2_TEX  |
					 DG_PRIM2_SINGLEBUFF |
					 DG_PRIM2_ALPHA,
					 BRK_BGLS_N_SCAR, 5 )) )
	PERROR( "No cannot make DG_PRIM2(frac[1])(May be no memory) :: NewPutBigGlass\n" ) ;
    InitPrimitive( work->frac[1], t, &pos ) ;

    work->n_frac = 0 ;

    return 0 ;
}

int BRK_BGLS_InitTarget( Work *work, int where )
{
    FVECTOR ub = { work->objs->def->ux, work->objs->def->uy, work->objs->def->uz, 0 } ;
    FVECTOR lb = { work->objs->def->lx, work->objs->def->ly, work->objs->def->lz, 0 } ;
    FVECTOR t_size, t_pos ;

    _sceVu0SubVector( &t_size, &ub, &lb ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &ub, &lb ) ;
    _sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &t_pos, (FVECTOR *)work->objs->world.m[W] ) ;

    GM_SetTarget( &work->target, TARGET_DEFENSE|TARGET_POWER|TARGET_THROUGH,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( &work->target, &work->power, POWER_ONCE, GM_Vitality, 0, 0,&DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->target, BRK_BGLS_TargetCallBack, work ) ;
    GM_PutTarget( &work->target ) ;

    //NewTargetView( &work->target,  200, 50, 32 ) ;

    return 0 ;
}


HZD_BOX BRK_HZD_w01a1[] =
{
    { 0, {11800+BRK_SPHERE+3.0f, 500+BRK_SPHERE+3.0f, 6875+BRK_SPHERE+3.0f },      {1.0f/(11800.0f+BRK_SPHERE), 1.0f/(500.0f+BRK_SPHERE), 1.0f/(6875.0f+BRK_SPHERE) }, {    0, -500, -12875 }, UNIT },
    { 0, { 2000+BRK_SPHERE+3.0f, 750+BRK_SPHERE+3.0f,  250+BRK_SPHERE+3.0f },      {1.0f/( 2000.0f+BRK_SPHERE), 1.0f/(750.0f+BRK_SPHERE), 1.0f/( 250.0f+BRK_SPHERE) }, {    0,    0, -14250 }, UNIT },
    { 0, {  500+BRK_SPHERE+3.0f, 750+BRK_SPHERE+3.0f,  375+BRK_SPHERE+3.0f },      {1.0f/(  500.0f+BRK_SPHERE), 1.0f/(750.0f+BRK_SPHERE), 1.0f/( 375.0f+BRK_SPHERE) }, {-2500,    0, -14375 }, UNIT },
    { 0, {  500+BRK_SPHERE+3.0f, 750+BRK_SPHERE+3.0f,  375+BRK_SPHERE+3.0f },      {1.0f/(  500.0f+BRK_SPHERE), 1.0f/(750.0f+BRK_SPHERE), 1.0f/( 375.0f+BRK_SPHERE) }, { 2500,    0, -14375 }, UNIT },
    { 0, { 2900+BRK_SPHERE+3.0f, 850+BRK_SPHERE+3.0f,   50+BRK_SPHERE+3.0f },      {1.0f/( 2900.0f+BRK_SPHERE), 1.0f/(850.0f+BRK_SPHERE), 1.0f/(  50.0f+BRK_SPHERE) }, {    0,    0, -14250 }, UNIT },
} ;

int BRK_BGLS_InitHazard( Work *work, int map )
{
#if 0
    int    i ;

    for ( i=work->n_hzd ; --i>=0 ; )
    {
	static TARGET h[10] ; /*debug code*/
        GM_SetTarget( &h[i], TARGET_DEFENSE, 0, BOTH_SIDE, &work->hzd[i].size, &work->hzd[i].center ) ;
        GM_SetTarget( &work->hzd[i], TARGET_DEFENSE, 0, BOTH_SIDE,
		      &BRK_HZD_w01a1[i].size,
		      &BRK_HZD_w01a1[i].center ) ;
	NewTargetView( &h[i],  32, 200, 50 ) ;
    }
#endif

    work->n_hzd = sizeof(BRK_HZD_w01a1)/sizeof(HZD_BOX) ;
    work->hzd   = BRK_HZD_w01a1 ;

    return 0 ;
}

int BRK_BGLS_InitPieces( Work *work, DG_DEF *def )
{
    int i ;
    DG_COMDL_POS *pos_s, *pos_l ;
    PIECE  *p ;
    FMATRIX light[2] ;

    if ( !(work->piece_l = DG_MakeComdl( def->models[2/*0*/].packs,
					 DG_COMDL_SEMITRANS, BRK_N_PIECE/2, 0)) )
	PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutBigGlass\n" ) ;
    DG_QueueComdlObjs( work->piece_l ) ;
    GM_GroupObject( work->piece_l, work->where ) ;

    if ( !(work->piece_s = DG_MakeComdl( def->models[2].packs,
					 DG_COMDL_SEMITRANS, BRK_N_PIECE/2, 0)) )
	PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutBigGlass\n" ) ;
    DG_QueueComdlObjs( work->piece_s ) ;
    GM_GroupObject( work->piece_s, work->where ) ;
    DG_GetLightMatrix( (FVECTOR*)work->objs->world.m[3], light ) ;

    if ( !(work->piece = GV_Malloc( sizeof(PIECE) * BRK_N_PIECE )) )
	PERROR( "Can't Alloc memory for PIECE moving(May be no memory) :: NewPutBigGlass\n" ) ;

    pos_l = work->piece_l->pos ;
    pos_s = work->piece_s->pos ;
    p = work->piece ;
    for ( i=BRK_N_PIECE/2 ; --i>=0 ; pos_s++, pos_l++ )
    {
	static IVECTOR def_color = { 64, 64, 64, 64 } ;

	p->world = &pos_s->world ;
	p->flag  = 0             ;
	p++ ;
	p->world = &pos_l->world ;
	p->flag  = 0             ;
	p++ ;
        pos_l->world    = pos_s->world    = DG_UnitMatrix ;
        pos_l->color.vx = pos_s->color.vx = (int)(light[1].m[0][X]*light[0].m[W][0]) ;
        pos_l->color.vy = pos_s->color.vy = (int)(light[1].m[0][Y]*light[0].m[W][0]) ;
        pos_l->color.vz = pos_s->color.vz = (int)(light[1].m[0][Z]*light[0].m[W][0]) ;
	pos_l->color = pos_s->color = def_color ;
    }
    work->n_piece = BRK_BGLS_INACTIVE ;

    return 0 ;
}

FMATRIX BRK_BGLS_MatrixTable[16][16] ;

int BRK_BGLS_InitMatrixTable( Work *work )
{
    int      x, z ;
    FMATRIX *m ;

    for ( z=16 ; --z>=0 ; )
	for ( x=16 ; --x>=0 ; )
	{
	    m = &BRK_BGLS_MatrixTable[x][z] ;
	    _sceVu0RotMatrixX( m, &DG_UnitMatrix, (float)(2.0f * (float)M_PI * x /16 - (float)M_PI) ) ;
	    _sceVu0RotMatrixZ( m, m             , (float)(2.0f * (float)M_PI * z /16 - (float)M_PI) ) ;
	}
    return 0 ;
}

int BRK_BGLS_InitPart( Work *work )
{
    int      i, j ;
    PART    *p ;
    FVECTOR  c ;/* 左上の端の点 */
    int      w_num, h_num ;
    DG_DEF  *def = work->def ;

    c.vx = work->target.offset.vx - work->target.size.vx + BRK_COMDL_W/2 - BRK_COMDL_M/2 ;
    c.vy = work->target.offset.vy - work->target.size.vy + BRK_COMDL_H/2 - BRK_COMDL_M/2 ;
    c.vz = work->target.offset.vz - work->target.size.vz ;

    w_num = DG_FTOI( (work->target.size.vx*2 + BRK_COMDL_W - 1) / BRK_COMDL_W ) ; /* 横のリピート回数 */
    h_num = DG_FTOI( (work->target.size.vy*2 + BRK_COMDL_H - 1) / BRK_COMDL_H ) ; /* 縦のリピート回数 */

    work->n_comdl = def->n_models ;
    work->n_parts = work->n_comdl * w_num * h_num ;
    if ( !(work->comdl = GV_Malloc( sizeof(DG_COMDL *) * work->n_comdl )) )
	PERROR( "Can't alocate memory :: NewPutBigGlass\n" ) ;
    if ( !(work->parts = GV_Malloc( sizeof(PART) * work->n_parts )) )
    {
	BRK_BGLS_FreeComdl( work ) ;
	PERROR( "Can't alocate memory :: NewPutBigGlass\n" ) ;
    }
    p = work->parts ;
    for ( i=work->n_comdl ; --i>=0 ; )
    {
	if ( !(work->comdl[i] = DG_MakeComdl( def->models[i].packs, DG_COMDL_SEMITRANS,
					      w_num * h_num, 0 )) )
	{
	    while( --i>=0 )
		work->comdl[i] = NULL ;
	    BRK_BGLS_FreeComdl( work ) ;
	    PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutBigGlass\n" ) ;
	}
	DG_QueueComdlObjs( work->comdl[i] ) ;
	GM_GroupObject( work->comdl[i], work->where ) ;
	for ( j=w_num*h_num ; --j>=0 ; p++ )
	{
	    static IVECTOR def_color = { 64, 64, 64, 0 } ;

	    work->comdl[i]->pos[j].color = def_color ;
	    p->pos        = &work->comdl[i]->pos[j] ;
	    p->pos->world = DG_UnitMatrix ;
	    p->flag       = i&1 ? 0x8000 : 0 ;
	    p->mov.pos.vx = c.vx + def->models[i].tx + (j % w_num) * BRK_COMDL_W ;
	    p->mov.pos.vy = c.vy + def->models[i].ty + (j / w_num) * BRK_COMDL_H ;
	    p->mov.pos.vz = c.vz + def->models[i].tz ;
	    p->mov.pos_v  = DG_ZeroVector ;
	    p->mov.rot_x = p->mov.rot_y = p->mov.rot_vx = p->mov.rot_vy = 0 ;
	}
    }
#if DEBUG_MODE
    printf( "w%d h%d n_parts%d :: NewPutBigGlass\n", w_num,  h_num, work->n_parts ) ;
#endif

    return 0 ;
}

void BRK_BGLS_InitEneFind( Work *work )
{
    int grp, zone ;
    int addr ;
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;

    HZX_Pos2Zone( &work->target.offset, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, &work->target.offset, addr, EF_TYPE_NO_FIND ) ;
    GM_PutEneFind( &work->ene_find ) ;
}

void BRK_BGLS_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_BGLS_AlreadyMessy( work ) ;
}

int BRK_BGLS_GetOptions( Work *work, int where )
{
    int      lit, buf[3] ;
    SVECTOR  rot ;
    FMATRIX *mtx ;
    FVECTOR  scale, pos = {0,0,0,0} ;
    DG_DEF  *def ;

    if ( GCL_GetOption( 'm' ) )
    {
	if ( !(def = (DG_DEF*) GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) )) )
	    PERROR( "Can't find Model(May be not specified in data.cnf) :: NewPutBigGlass\n" ) ;
	if ( !(work->objs = DG_MakeObjs( def, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, 0 )) )
	    PERROR( "Can't Make DG_OBJS(May be no memory) :: NewPutBigGlass\n" ) ;
	DG_QueueObjs( work->objs ) ;
	GM_GroupObjs( work->objs, where ) ;

	if ( !(def = (DG_DEF *)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	    PERROR( "Can't find DUST-KMS(May be not specified in data.cnf) :: NewPutBigGlass\n" ) ;
	if ( BRK_BGLS_InitPieces( work, def ) )
	    PERROR( "PieceInit Failed :: NewPutBigGlass\n" ) ;
	if ( !(work->def = (DG_DEF *)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	    PERROR( "Can't find FRACT-KMS(May be not specified in data.cnf) :: NewPutBigGlass\n" ) ;
    }
    else
	PERROR( "No Model ID(-m options missing) :: NewPutBigGlass\n" ) ;

    mtx = &work->objs->world ;
    if ( GCL_GetOption( 's' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &scale ) ;
	mtx->m[X][X] *= scale.vx/100.0F ;
	mtx->m[Y][Y] *= scale.vy/100.0F ;
	mtx->m[Z][Z] *= scale.vz/100.0F ;
    }
    if ( GCL_GetOption( 'r' ) != NULL )
    {
	GCL_GetSV( GCL_NextStr(), (short*)&rot ) ;
	RotateMatrix( mtx, &DG_UnitMatrix, &rot ) ;
    }
    if ( GCL_GetOption( 'p' ) != NULL )
    {
        GCL_GetIV( GCL_NextStr(), buf ) ;
        vu0_IV0toFV( (IVECTOR *)buf, &pos ) ;
	TransMatrix( mtx, &pos ) ;
    }
    work->pr_name = GCL_GetOptionValue( 'c', 0 ) ;

    if ( (lit = GCL_GetOptionValue( 'l', 0 )) != 0 )
        DG_MakePreshade( work->objs, GV_GetCache( GV_CacheID( lit, 'l' ) ) ) ;
    else
	PERROR( "No Light ID(-l options missing) :: NewPutBigGlass\n" ) ;

    work->life = 10 ;
    work->act  = NULL ;

    return 0 ;
}


