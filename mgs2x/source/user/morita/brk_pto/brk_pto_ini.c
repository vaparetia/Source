//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_pto_ini.c
   じゃがいも壊れ

   1999/12/13 T. Morita
   2000/10/16 1.28 T.Morita 
   $Id: brk_pto_ini.c,v 1.1.1.3 2002/11/19 11:45:42 Yoshizawa1 Exp $
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

#include "brk_potato.h"
#include "../brk_utl/brk_utl.x"


int BRK_PTO_InitEneFind( Work *work )
{
    int grp, zone ;
    int addr ;
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;

    HZX_Pos2Zone( &work->box.target.center, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, &work->box.target.center, addr, EF_TYPE_NO_FIND ) ;

    return 0 ;
}


#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
static inline void InitPacket( DG_PRIM2 *p, DG_TEX *tex, int i, u_int rgba )
{
    DG_PRIM2_UVRGB *uvrgb = p->uvrgb[0] ;
    FVECTOR        *pos   = p->pos[0] ;

    DG_ConfigPrim2Tex( p, tex ) ;
    DG_SetPrim2Alpha( p, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) ) ;
    for ( ; --i>=0 ; uvrgb+=4, pos+=4 )
    {
	uvrgb[0].u = uvrgb[2].u = FTOI12( 0.0f*tex->u_scale + tex->u_offset ) ;
	uvrgb[1].u = uvrgb[3].u = FTOI12( 1.0f*tex->u_scale + tex->u_offset ) ;
	uvrgb[0].v = uvrgb[1].v = FTOI12( 0.0f*tex->v_scale + tex->v_offset ) ;
	uvrgb[2].v = uvrgb[3].v = FTOI12( 1.0f*tex->v_scale + tex->v_offset ) ;
	uvrgb[0].q = uvrgb[1].q = uvrgb[2].q = uvrgb[3].q = 4096 ;
	uvrgb[0].f = uvrgb[1].f = 0x8fff ;
	uvrgb[2].f = uvrgb[3].f = 0x0fff ;
	uvrgb[0].r = uvrgb[1].r = uvrgb[2].r = uvrgb[3].r = (rgba>> 0) & 0xff ;
	uvrgb[0].g = uvrgb[1].g = uvrgb[2].g = uvrgb[3].g = (rgba>> 8) & 0xff ;
	uvrgb[0].b = uvrgb[1].b = uvrgb[2].b = uvrgb[3].b = (rgba>>16) & 0xff ;
	uvrgb[0].a = uvrgb[1].a = uvrgb[2].a = uvrgb[3].a = (rgba>>24) & 0xff ;
	_sceVu0CopyVector( &pos[0], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[1], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[2], &DG_ZeroVector ) ;
	_sceVu0CopyVector( &pos[3], &DG_ZeroVector ) ;
    }
}

int BRK_PTO_InitPrims( Work *work )
{
    int i ;
    DG_TEX *t = DG_GetTexture( GV_StrCode( "dankon2_alp" ) ) ;
    DG_PRIM2 *p ;

    for ( i=5 ; --i>=0 ; p++ )
    {
	if ( !(p = GM_MakePrim2( DG_PRIM2_POLY| DG_PRIM2_TEX| DG_PRIM2_ALPHA| DG_PRIM2_SINGLEBUFF,
				 BRK_N_PRIM, BRK_N_VERTS )) )
	    PERROR( "Cannot make DG_PRIM2(Maybe no memory) :: brk_potato.c" ) ;
	InitPacket( p, t, BRK_N_SCAR, 0x00808080 ) ;
	p->root = &work->box.objs->objs[i].world ;
	work->box.scar[i] = p ;
    }
    return 0 ;
}


void BRK_PTO_MakeSizeAndCenter( FVECTOR *t_size, FVECTOR *t_pos, float *uv, float *lv )
{
    FVECTOR u = { uv[X],uv[Y],uv[Z],0 }, l = { lv[X],lv[Y],lv[Z],0 } ;

    _sceVu0SubVector( t_size, &u, &l ) ;
    _sceVu0ScaleVector( t_size, t_size, 0.5f ) ;
    _sceVu0AddVector( t_pos, &u, &l ) ;
    _sceVu0ScaleVector( t_pos , t_pos , 0.5f ) ;
}

int BRK_PTO_InitHazard( Work *work )
{
    int hzd_id ;

    hzd_id = GCL_GetOptionValue( 'D', BRK_HZD_W01D_STORERAGE ) ;
    work->hzd = BRK_InitHazard( hzd_id ) ;
    work->box_hzd[0] = BRK_MakeHazard( BRK_HZD_OUTSIDE| BRK_HZD_ROTATE ,
				       &work->box.objs->objs[0].world, NULL,
				       &work->box.target.size,
				       &work->box.target.offset,
				       NULL ) ;
    return 0 ;
}

int BRK_PTO_InitTarget( TARGET *t, POWER_TARGET *p, int map, int flag,
			float *uv, float *lv, FVECTOR *pos,
			void (*callback)( TARGET *off, TARGET *def, void *ptr ), void *ptr )
{
    FVECTOR t_size, t_pos ;

    BRK_PTO_MakeSizeAndCenter( &t_size, &t_pos, uv, lv ) ;
    GM_SetTarget( t, TARGET_DEFENSE|TARGET_POWER|TARGET_SEEK|flag, map, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, callback, ptr ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget( t, pos ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

/*

  プラグにより壊れ前か壊れ後を決める。

*/
int BRK_PTO_InitPotato( POTATO *p, int where, DG_DEF *def, int flag, void *act )
{
    int i ;
    DG_OBJ  *o ;
    DG_OBJS *objs = p->objs ;

    if ( p->act == BRK_PTO_ActOnFoot )
	return -1 ;

    p->n_bounce = 0 ;

    p->pos.vw = p->pos_v.vx = p->pos_v.vy = p->pos_v.vz = 0.0f ;
    p->rot_vx = p->rot_vy   = 0 ;
    p->rot_x = irnd()&4095 ;
    p->rot_y = irnd()&4095 ;
    p->act = act  ;
    if ( !(p->objs = DG_MakeObjs( def, flag, 0 )) )
	PERROR( "Can't make DG_OBJS(no memory) :: brk_potato\n" ) ;
    DG_SetLightMatrix( p->objs, p->lights ) ;
    DG_QueueObjs( p->objs ) ;
    GM_GroupObjs( p->objs, where ) ;

    RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    TransMatrix( &p->objs->world, &p->pos ) ;
    if ( flag & DG_FLAG_FINISHCALC )
    {
	p->n_parts = p->objs->n_models ;
	for ( i=p->n_parts, o=p->objs->objs ; --i>=0 ; o++ )
	{
	    /* 子画面には見えないようにする */
	    o->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;
	    o->world = p->objs->world ;
	}
    }
#if MAKING
    {
	void *NewDrawWireframe( DG_OBJ * ) ;
	p->wireframe = NewDrawWireframe( &p->objs->objs[0] ) ;
    }
#endif
    if ( objs )
	DG_DequeueObjs( objs ),	DG_FreeObjs( objs ) ;

    return  0 ;
}

int BRK_PTO_InitBox( Work *work, int where )
{
    DG_DEF  *def ;
    BOX     *b = &work->box ;
    int      buf[3], i ;
    FVECTOR  enlarge = { 1000.0f, 1000.0f, 1000.0f, 0.0f } ;

    /* プロックIDの取得 */
    work->proc_id = GCL_GetOptionValue( 'C', 0 ) ;

    if ( (i = GCL_GetOptionValue( 'b', 0 )) )
	def = (DG_DEF*)GV_GetCache( GV_CacheID( i, 'k' ) ) ;
    else
	PERROR( "No Model ID (-b option missing) :: brk_potato\n" ) ;

    if ( GCL_GetOption( 'p' ) )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR *)buf, &b->pos ) ;
    }
    else
	PERROR( "No Position (-p option missing) :: brk_potato\n" ) ;

    if ( GCL_GetOption( 'r' ) )
    {
	b->brot.vy = GCL_GetNextInt() ;
	b->r_max   = GCL_GetNextInt() ;
	b->r_min   = GCL_GetNextInt() ;
    }

    b->flag = GCL_GetOptionValue( 'f', 0 ) ;

    /* モデルの初期化 */
    if ( !(b->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_FINISHCALC, 0 )) )
	PERROR( "Can't initialize DG_OBJS(Maybe box model missing) :: brk_potato\n" ) ;
    DG_QueueObjs( b->objs ) ;
    GM_GroupObjs( b->objs, where ) ;
    BRK_UTL_GetLightMatrix( &b->pos, b->lights, 1.0f, work->where ) ;
    DG_SetLightMatrix( b->objs, b->lights ) ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		extern int	C4MAN_Regist( int name, int *map, DG_OBJS *objs, FMATRIX *world, FVECTOR *shift, SVECTOR *rot ) ;

		static FVECTOR	shift={0.0, 0.0, 0.0} ;
		static SVECTOR	rot={-1024, 1024, 0} ;
		C4MAN_Regist( work->name, &work->where, b->objs, &b->objs->world, &shift, &rot ) ;
	}

    /* モデル情報よりターゲットを設定 */
    BRK_PTO_InitTarget( &b->target, &b->power, where, TARGET_ROTATE|TARGET_THROUGH,
			&b->objs->def->models[0].ux, &b->objs->def->models[0].lx,
			&b->pos, BRK_PTO_BoxTargetCallBack, work ) ;
    /* マトリックスの初期化 */
    RotateMatrix( &b->objs->world, &DG_UnitMatrix, &b->brot ) ;
    TransMatrix( &b->objs->world, &b->pos ) ;
    for ( i=b->objs->n_models ; --i>=0 ; )
    {
	FMATRIX *m = &b->objs->objs[i].world ;

	m->m[W][X] = b->objs->def->models[i].tx ;
	m->m[W][Y] = b->objs->def->models[i].ty ;
	m->m[W][Z] = b->objs->def->models[i].tz ;
	_sceVu0MulMatrix( m, &b->objs->world, m ) ;
	b->rot[i] = b->rot_v[i] = 0 ;
    }
    /* 各変数の初期化 */
    b->floor    = b->pos.vy            ;
    b->vitality = BRK_PTO_BOXVITALITY  ;
    b->act      = NULL ;
    b->non_dmg  = 0 ;
    b->n_scar[0] = b->n_scar[1] = b->n_scar[2] =
	b->n_scar[3] = b->n_scar[4] = 0 ;

    /* うまく表示するようバウンダリを調整 */
    _sceVu0AddVector( &b->objs->bound_max, &b->objs->bound_max, &enlarge ) ;
    _sceVu0SubVector( &b->objs->bound_min, &b->objs->bound_min, &enlarge ) ;

    return 0 ;
}


static int MakePositionFromBox( Work *work, BOX *b, POTATO *p )
{
    int     i, j, k ;
    FVECTOR pos ;
    float   dx, dz ;

#define BRK_BOX_THICKNESS 10.0f
#define BRK_POTATO_RAD_M (BRK_POTATO_RAD+10.0f)

    pos.vy = b->target.size.vy + BRK_POTATO_RAD_M/2.0f + 8.0f ;
    k = 0 ;
    while( 1 )
    {
	i = (int)(2*b->target.size.vx/BRK_POTATO_RAD_M) ;
	dx =  BRK_POTATO_RAD_M + (b->target.size.vx*2-i*BRK_POTATO_RAD_M-BRK_BOX_THICKNESS*2) / i ;
	pos.vx = -b->target.size.vx + dx*0.5f + BRK_BOX_THICKNESS ;
	for ( ; --i>=0 ; pos.vx += dx )
	{
	    j = (int)(2*b->target.size.vz/BRK_POTATO_RAD_M) ;
	    dz =  BRK_POTATO_RAD_M + (b->target.size.vz*2-j*BRK_POTATO_RAD_M-BRK_BOX_THICKNESS*2) / j ;
	    pos.vz = -b->target.size.vz + dz*0.5f + BRK_BOX_THICKNESS ;
	    for ( ; --j>=0 ; pos.vz += dz )
	    {
		if ( !(irnd() & 0x1101) )
		    continue ;
		if ( ++k > work->n_potato )
		    return 0 ;
		p->pos.vx = pos.vx + frnd() ;
		p->pos.vy = pos.vy + frnd() ;
		p->pos.vz = pos.vz + frnd() ;
		p++ ;
	    }
	}
	pos.vy += BRK_POTATO_RAD_M + 1.0f ;
    }
    return 0 ;
}


void BRK_PTO_InitBroken( Work *work )
{
    if ( GCL_GetOptionValue( 'B', 0 ) )
	BRK_PTO_AlreadyMessy( work ) ;
}

int BRK_PTO_GetOptions( Work *work, int where )
{
    int      i ;
    POTATO  *p ;

    /* モデル */
    if ( GCL_GetOption( 'm' ) )
    {
	if ( !(work->m_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	    PERROR( "Cannnot find POTATO KMS-MODEL( not in data.cnf ) :: brk_potato.c\n" ) ;
	if ( !(work->p_def = (DG_DEF*)GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ))) )
	    PERROR( "Cannnot find BROKEN POTATO KMS-MODEL( not in data.cnf ) :: brk_potato.c\n" ) ;
	work->p_def->ux =  BRK_BOUND_X ; work->p_def->lx = -BRK_BOUND_X ;
	work->p_def->uy =  BRK_BOUND_Y ; work->p_def->ly = -BRK_BOUND_Y ;
	work->p_def->uz =  BRK_BOUND_Z ; work->p_def->lz = -BRK_BOUND_Z ;
    }
    else
	PERROR( "No Potato Model :: brk_potato.c\n" ) ;

    /* 中身の種類 */
    work->type = GCL_GetOptionValue( 't', 0 ) ;

    work->max_foot = GCL_GetOptionValue( 'h', 0 ) ;
    if ( !(work->n_potato = GCL_GetOptionValue( 'n', 0 )) )
	PERROR( "The number of potato need more than one (-n option maybe missing) :: brk_potato.c\n" ) ;

    if ( !(p = work->potato = GV_Malloc( sizeof(POTATO) * (work->n_potato + work->max_foot * 2) )) )
	PERROR( "Cannot allocate memory for <work->potato>  :: brk_potato.c\n" ) ;
    MakePositionFromBox( work, &work->box, p ) ;
    for ( i=work->n_potato ; --i>=0 ; p++ )
    {
	p->objs    = NULL ; /* mallocからの初期化なのでNULLにする */
	p->act     = NULL ;
	p->n_parts = 0    ;
	p->work    = work ;
    }
    work->foot   = p ;
    work->n_foot = 0 ;

    return 0 ;
}
