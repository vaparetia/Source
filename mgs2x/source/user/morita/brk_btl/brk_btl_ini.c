//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
   brk_brk_btl_ini.c
   瓶壊れ 初期化

   1999/12/02 T. Morita
   $Id: brk_btl_ini.c,v 1.2 2003/01/30 01:28:00 takaki Exp $
*/
#endif

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

#include "brk_bottle.h"
#include "../include/util.h"



void BRK_BTL_InitEneFind( Work *work )
{
    int grp, zone ;
    int addr ;
    extern void HZX_Pos2Zone( FVECTOR *pos, int *g, int *z ) ;

    HZX_Pos2Zone( &work->bottles->pos, &grp, &zone ) ;
    addr = HZX_AddressNo( grp, zone, zone ) ;
    GM_SetEneFind( &work->ene_find, &work->bottles->pos, addr, EF_TYPE_NO_FIND ) ;
    GM_PutEneFind( &work->ene_find ) ;
}



int BRK_BTL_InitPieces( Work *work, int where )
{
    int      i   ;
    DG_DEF  *def ;
    DG_COMDL_POS *pos_s ;
    DG_COMDL_POS *pos_l ;
    PIECE   *p   ;

    /* コモデルの初期化 */
    if ( !(def = (DG_DEF *)GV_GetCache( GV_CacheID( GV_StrCode( "btl_piece" ), 'k' ) )) )
	PERROR( "Cannot find Model (Not in data.cnf) :: NewPutBottleObject\n" ) ;
    if ( !(work->piece_l = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, BRK_N_PIECE/2, 0)) )
	PERROR( "Can't make DG_COMDL (Maybe no memory) :: NewPutBottleObject\n" ) ;
    DG_QueueComdlObjs( work->piece_l ) ;
    GM_GroupObject( work->piece_l, where ) ;
    if ( !(work->piece_s = DG_MakeComdl( def->models[1].packs, DG_COMDL_SEMITRANS, BRK_N_PIECE/2, 0)) )
	PERROR( "Can't make DG_COMDL (Maybe no memory) :: NewPutBottleObject\n" ) ;
    DG_QueueComdlObjs( work->piece_s ) ;
    GM_GroupObject( work->piece_s, where ) ;

    /* 子画面には見えないようにする */
    work->piece_s->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;
    work->piece_l->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;

    /* 位置,色の初期化 */
    pos_s = work->piece_s->pos ;
    pos_l = work->piece_l->pos ;
    p     = work->piece_prof ;
    for ( i=BRK_N_PIECE/2 ; --i>=0 ; pos_s++, pos_l++ )
    {
        p->cmdl_p     = pos_s         ;
        p->flag       = 0             ;
        pos_s->world  = DG_UnitMatrix ;
        pos_s->color.vx = pos_s->color.vy = pos_s->color.vz = 128 ;
	pos_s->color.vw = 0 ;
	p++ ;

        p->cmdl_p     = pos_l         ;
        p->flag       = 0             ;
        pos_l->world  = DG_UnitMatrix ;
        pos_l->color.vx = pos_l->color.vy = pos_l->color.vz = 128 ;
	pos_l->color.vw = 0 ;
	p++ ;
    }
    work->n_piece = 0 ;

    return 0 ;
}




static void AddFloatToFvector( FVECTOR *r, float *a, float *b )
{
    r->vx = a[X] + b[X] ;
    r->vy = a[Y] + b[Y] ;
    r->vz = a[Z] + b[Z] ;
}
static void SubFloatToFvector( FVECTOR *r, float *a, float *b )
{
    r->vx = a[X] - b[X] ;
    r->vy = a[Y] - b[Y] ;
    r->vz = a[Z] - b[Z] ;
}
int BRK_BTL_InitTarget( BOTTLE *b, DG_DEF *def, int map )
{
    FVECTOR       t_size, t_pos ;
    int           i = def->n_models ;
    DG_MDL       *m = def->models   ;
    TARGET       *tr ;
    POWER_TARGET *pw ;
    PART         *pr ;

    //BP - this may or may not have been a bugfix that should apply to all versions not just Windows.
    //can't hurt to use the code that allocates the greater of the two amounts. (not worth the time to figure it out)
#if 0 //BP ndef KP_WINDOWS
    if ( !(tr = GV_Malloc( sizeof(void *) + (sizeof(TARGET)+sizeof(POWER_TARGET)) * (i+1) + sizeof(PART)*i )) )
	return -1 ;
#else
	{
		unsigned int	size ;

		size = sizeof(void *)
			+ ((sizeof(TARGET)+sizeof(POWER_TARGET)+sizeof(PART)) * (i+1)) ;
				// PARTSの最終要素の次のアドレスにアクセスに行って死亡する事が
				// あるので、余分に取っておく。

	    if ( !(tr = GV_Malloc( size )) ) return -1 ;

		GV_ZeroMemory(tr, size) ;	// 安全策
	}
#endif
    b->target = tr ;
    b->power  = pw = (POWER_TARGET *)(tr + i + 1) ;
    b->parts  = pr = (PART         *)(pw + i + 1) ;
    b->pos_v  = DG_ZeroVector  ;
    b->rot_v  = DG_ZeroSVector ;
    b->act    = NULL ;
    t_size.vx = 800.0f ;
    t_size.vy = 300.0f ;
    t_size.vz = 800.0f ;
    AddFloatToFvector( &t_pos , &def->ux, &def->lx ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;
    GM_SetTarget( tr, TARGET_DEFENSE|TARGET_POWER|TARGET_CHILD|TARGET_CHILD_ALWAYS,
		  map, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( tr, pw++, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( tr, BRK_BTL_TargetCallBack, pr+(irnd()%i) ) ;
    GM_SetTargetParts( tr, tr+1, i, 0 ) ;
    GM_MoveTarget( tr, &b->pos ) ;
    GM_PutTarget( tr++ ) ;
    for ( ; --i>=0 ; m++, tr++, pw++, pr++ )
    {
	SubFloatToFvector( &t_size, &m->ux, &m->lx ) ;
	_sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
	AddFloatToFvector( &t_pos, &m->ux, &m->lx ) ;
	_sceVu0ScaleVector( &t_pos, &t_pos, 0.5f ) ;
	GM_SetTarget( tr, TARGET_DEFENSE|TARGET_POWER|TARGET_ROTATE|TARGET_ONLINE|TARGET_CHILD_ALWAYS,
		      map, BOTH_SIDE, &t_size, &t_pos );
	GM_SetPowerTarget( tr, pw, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( tr, BRK_BTL_TargetCallBack, pr ) ;
	GM_MoveTarget( tr, &b->pos ) ;

	pr->bottle = b  ;
	pr->target = tr ;
    }

    return 0 ;
}

#if 1
static BTL_TYP BottleType[] = 
{
    { NULL, NULL, "btl_01", "btl_01brk", 0x00271313, SD_A_BOTTLE01 },
    { NULL, NULL, "btl_02", "btl_02brk", 0x00320303, SD_A_BOTTLE01 },
    { NULL, NULL, "btl_03", "btl_03brk", 0x00000f0a, SD_A_BOTTLE01 },
    { NULL, NULL, "btl_04", "btl_04brk", 0x000f0f1f, SD_A_BOTTLE01 },
    { NULL, NULL, "btl_05", "btl_05brk", 0x00060503, SD_A_BOTTLE01 },
    { NULL, NULL, "btl_06", "btl_06brk", 0x00031003, SD_A_BOTTLE01 },
    { NULL, NULL, "btl_07", "btl_07brk", 0x001a0413, SD_A_BOTTLE01 },
    { NULL, NULL, "gls_03", "gls_03brk", 0x00070707, SD_A_BOTTLE02 },
} ;
void BRK_BTL_InitBottleType()
{
    int      i, c=0 ;
    BTL_TYP *b = BottleType ;

    for ( i=sizeof(BottleType)/sizeof(BTL_TYP) ; --i>=0 ; b++ )
    {
	b->bf_def = GV_GetCache( GV_CacheID( c, 'k' ) ) ;
	b->af_def = GV_GetCache( GV_CacheID( c, 'k' ) ) ;
	if ( b->af_def )
	{
	    b->af_def->lx = -5000.0f, b->af_def->ly = -5000.0f, b->af_def->lz = -5000.0f ;
	    b->af_def->ux =  5000.0f, b->af_def->uy =  5000.0f, b->af_def->uz =  5000.0f ;
	}
    }
}
#endif

int BRK_BTL_InitModel( BOTTLE *b, int map, DG_DEF *def, int flag, LIT_DEF *lit )
{
    int     i ;
    DG_OBJ *o ;
    PART   *p ;
    DG_MDL *m ;
    DG_OBJS *objs = b->objs ;

    if ( flag & DG_FLAG_FINISHCALC )
    {
	def->lx = -5000.0f, def->ly = -5000.0f, def->lz =-5000.0f ;
	def->ux =  5000.0f, def->uy =  5000.0f, def->uz = 5000.0f ;
    }
    if ( !(b->objs = DG_MakeObjs( def, flag, 0 )) )
	PERROR( "Can't make DG_OBJS (Maybe no memory) :: NewPutBottleObject\n" ) ;
    GM_GroupObjs( b->objs, map ) ;

    RotateMatrix( &b->objs->world, &DG_UnitMatrix, &b->rot ) ;
    TransMatrix( &b->objs->world, &b->pos ) ;


    if ( flag & DG_FLAG_FINISHCALC )
    {
	m = def->models ;
	p = b->parts ;
	o = b->objs->objs ;
	for ( i=b->objs->n_models ; --i>=0 ; o++, p++, m++ )/*瓶の部分も一緒に初期化してしまう*/
	{
	    (p->obj = o)->world = b->objs->world ;
	    p->pos    = b->pos ;
	    p->pos_v  = DG_ZeroVector  ;
	    p->rot    = b->rot ;
	    p->rot_v  = DG_ZeroSVector ;

#if MAKING
	    p->wireframe = NewDrawWireframe( o ) ;
#endif

	}
    }
    //DG_GetLightMatrixFix( &b->pos, b->lights ) ;
    //DG_SetLightMatrix( b->objs, b->lights ) ;
    DG_MakePreshade( b->objs, lit ) ;
    DG_QueueObjs( b->objs ) ;

    if ( objs )
    {
#if MAKING
	if ( b->wireframe )
	    GV_DestroyOtherActor( b->wireframe ), b->wireframe = NULL ;
#endif

	/* メモリを確保できそうなのでモデルを消す */
	DG_FreePreshade( objs ) ;
	DG_DequeueObjs( objs ) ;
	DG_FreeObjs( objs ) ;
    }

#if MAKING
    if ( !(flag & DG_FLAG_FINISHCALC) )
	b->wireframe = NewDrawWireframe( &b->objs->objs[0] ) ;
#endif

    return 0 ;
}

#if DEBUG_MODE

static int BRK_BTL_ShowDbg = 0 ;
static TARGET BRK_BTL_DbgTrg[20] ;
static int BRK_BTL_N_DbgTrg = 0 ;

#endif


int BRK_BTL_InitHazard( Work *work, int where )
{
    int      i   ;
    IVECTOR  buf ;
    char    *c   ;
    FVECTOR *f, siz, pos ;

    work->hzd_id  = GCL_GetOptionValue( 'h', BRK_HZD_W01A1_BAR ) ;
    work->hzd_box = BRK_InitHazard( work->hzd_id ) ;

    /*ハザードタイプ ( 0:円盤 -1:長方形最小点 最大点 -2:筒形最小点 最大点)*/
    i = GCL_GetOptionValue( 's', 0 ) ;
    f = work->hzd = GV_Malloc( sizeof(FVECTOR) * (i+1) ) ;
    if ( i ) 
	while( (c=GCL_NextStr()) )
	    switch( i=GCL_GetInt( c ) )
	    {
	    case 0:
		f->vw = (float)GCL_GetNextInt() ;/* 半径 */
		f->vx = (float)GCL_GetNextInt() ;
		f->vy = (float)GCL_GetNextInt() ;
		f->vz = (float)GCL_GetNextInt() ;
		f++ ;
		break ;

	    case 1:
		GCL_GetNextIV( (int*)&buf ) ;
		vu0_IV0toFV( &buf, f ) ;
		GCL_GetNextIV( (int*)&buf ) ;
		vu0_IV0toFV( &buf, f+1 ) ;
		_sceVu0SubVector( &siz, f+1, f ) ;
		_sceVu0ScaleVector( &siz, &siz, 0.5f ) ;
		_sceVu0AddVector( &pos, f+1, f ) ;
		_sceVu0ScaleVector( f, &pos, 0.5f ) ;
		(f+1)->vx =  fpu_Abs( siz.vx ) ;
		(f+1)->vy =  fpu_Abs( siz.vy ) ;
		(f+1)->vz =  fpu_Abs( siz.vz ) ;
		(f+0)->vw = BTK_BTL_HZD_BOX ;/* 直方体 */
#if DEBUG_MODE
		if ( BRK_BTL_ShowDbg )
		{
		    GM_SetTarget( &BRK_BTL_DbgTrg[BRK_BTL_N_DbgTrg], TARGET_DEFENSE, 0, BOTH_SIDE, f+1, f );
		    NewTargetView( &BRK_BTL_DbgTrg[BRK_BTL_N_DbgTrg++],  200, 50, 20 ) ;
		}
#endif
		f+=2 ;
		break ;

	    case 2:
		GCL_GetNextIV( (int*)&buf ) ;
		vu0_IV0toFV( &buf, f ) ;
		GCL_GetNextIV( (int*)&buf ) ;
		vu0_IV0toFV( &buf, f+1 ) ;
		_sceVu0SubVector( &siz, f+1, f ) ;
		_sceVu0ScaleVector( &siz, &siz, 0.5f ) ;
		_sceVu0AddVector( &pos, f+1, f ) ;
		_sceVu0ScaleVector( f, &pos, 0.5f ) ;
		(f+1)->vx =  fpu_Abs( siz.vx ) ;
		(f+1)->vy =  fpu_Abs( siz.vy ) ;
		(f+1)->vz =  fpu_Abs( siz.vz ) ;
		(f+0)->vw = BTK_BTL_HZD_TUNNEL ;/* くりぬき直方体 */
		(f+1)->vw = (float)GCL_GetNextInt() ;/*opened axis(as X:0, Y:1, Z:2) */

#if DEBUG_MODE
		if ( BRK_BTL_ShowDbg )
		{
		    GM_SetTarget( &BRK_BTL_DbgTrg[BRK_BTL_N_DbgTrg], TARGET_DEFENSE, 0, BOTH_SIDE, f+1, f );
		    NewTargetView( &BRK_BTL_DbgTrg[BRK_BTL_N_DbgTrg++],  200, 50, 20 ) ;
		}
#endif
		f+=2 ;
		break ;

	    default:
		break ;
	    }
    f->vw = -255.0f ;
    return 0 ;
}

int BRK_BTL_GetOptions( Work *work, int where )
{
    int      i, buf[3], id ;
    char    *c ;
    BOTTLE  *b ;


    if ( !(work->n_bottles = GCL_GetOptionValue( 'n', 0 )) )
	PERROR( "No number of bottles (-n option missing) :: NewPutBottleObject\n" ) ;
    if ( !(work->bottles = GV_Malloc( sizeof(BOTTLE) * work->n_bottles )) )
	PERROR( "Cannot allocate memory for <work->bottle> :: NewPutBottleObject\n" ) ;
    if ( !GCL_GetOption( 'p' ) )
	PERROR( "No Position (-p option missing) :: NewPutBottleObject\n" ) ;
    for ( i=0, b=work->bottles ; (c=GCL_NextStr()) ; i++, b++ )
    {
	GCL_GetSV( c, (short*)&b->rot ) ;
	GCL_GetIV( GCL_NextStr(), buf ) ;
	b->flag = GCL_GetNextInt() ;
	b->objs = NULL ;
	vu0_IV0toFV( (IVECTOR *)buf, &b->pos ) ;
#if DEBUG_MODE
	if ( b->flag & 0x02000000 )
	    BRK_BTL_ShowDbg = 1 ;
#endif
    }

#if DEBUG_MODE
    if ( i != work->n_bottles )
    {
	printf( i<work->n_bottles ? "BOTTLE:Does not have enough " : "BOTTLE:Too many " ) ;
	printf( "position arguments!!!(POSs%d/bottles%d)\n", i, work->n_bottles ) ;
	return -1 ;
    }
#endif

    /* 壊れプロック */
    if ( GCL_GetOption( 'C' ) )
    {
	if ( GCL_NextStr() )
	    work->proc_half = GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    work->proc_full = GCL_GetNextInt() ;
    }

    if ( !(i = GCL_GetOptionValue( 'l', 0 )) )
	PERROR( "No light ID (-l option missing) :: NewPutBottleObject\n" ) ;
    if ( !(work->lit = GV_GetCache( GV_CacheID( i, 'l' )) ) )
	PERROR( "No light-data exist(Not in data.cnf) :: NewPutBottleObject\n" ) ;

    if ( !GCL_GetOption( 'm' ) )
	PERROR( "No Model ID (-m option missing) :: NewPutBottleObject\n" ) ;
    for ( b=work->bottles, i=0 ; GCL_NextStr() ; b++, i++ )
    {
	id = GCL_GetNextInt() ;
	b->brk_bf = GV_GetCache( GV_CacheID( id, 'k' ) ) ;
	if ( !b->brk_bf )
	    PERROR( "No BeforeBreak Model<%d> :: NewPutBottleObject\n", id ) ;

	id = GCL_GetNextInt() ;
	b->brk_af = GV_GetCache( GV_CacheID( id, 'k' ) ) ;
	if ( !b->brk_af )
	    PERROR( "No AfterBreak Model<%d> :: NewPutBottleObject\n", id ) ;

	if ( BRK_BTL_InitModel( b, where, b->brk_bf, DG_FLAG_PAINT|DG_FLAG_ONEPIECE, work->lit ) )
	    return -1 ;
	if ( BRK_BTL_InitTarget( b, b->brk_af, where ) )
	    return -1 ;
	b->work = work ;
    }

#if DEBUG_MODE
    if ( i != work->n_bottles )
    {
	printf( i<work->n_bottles ? "BOTTLE:Does not have enough " : "BOTTLE:Too many " ) ;
	printf( "MODEL-ID arguments exist!!!(IDs%d/bottles%d)\n", i, work->n_bottles ) ;
	return -1 ;
    }
#endif

    work->n_bottles |= BRK_BOTTLE_INACTIVE ; 
    
#if MAKING
    if (0)
    {
	/*プレーヤーをワイヤーフレームにする。*/
	extern void *NewPlayerWireframe() ; 
	NewPlayerWireframe() ; 
    }
#endif
    return 0 ;
}


void BRK_BTL_InitBroken( Work *work )
{
    int break_msk ;

    if ( GCL_GetOption( 'B' ) )
    {
	break_msk = -1 ;
	if ( GCL_GetNextInt() )
	    break_msk = 0x10 ;
	if ( GCL_GetNextInt() )
	    break_msk = 0 ;
	if ( break_msk >= 0 )
	    BRK_BTL_AlreadyMessy( work, break_msk ) ;
    }
}
