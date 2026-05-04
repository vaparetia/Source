//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   put_mgz_ini.c
   雑誌壊れ 初期化

   2000/06/20 T. Morita
   $Id: put_bok_ini.c,v 1.1.1.3 2002/11/19 11:46:29 Yoshizawa1 Exp $
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

#include "put_book.h"



/* 動的ハザードコールバック */
static void DynamicHzdCallback( void *hzd, void *ptr, void *ptr2, int flag )
{
    Work *work = (Work *)ptr ;
    BOOK *p = &work->book ;

    /* 落とし穴がデキタヨー */
    if ( flag & (HZX_CALLBACK_FLAG_DESTROY|HZX_CALLBACK_FLAG_PITFALL) )
    {
	printf( "NewPutBook : Callback Called\n" ) ;

	/* 親に動いたことを知らせる */
	GV_CallParentSignalFunc( work, 0, 0 ) ;
	work->book.flag = 3 ;/* 落下 */
	work->fall      = 1 ;

	/* 挙動の初期化 */
	p->mov.pos_v.vy = 1.0f ;
	p->mov.rot_v.vx = (irnd()&0x00e0)-128 ;
	p->mov.rot_v.vy = (irnd()&0x00e0)-128 ;
	p->open_v = -DEG2RAD(20.0f) ;

	/* 当たり判定ありのアクト */
	p->flag = 12 ; /* collision immeadiately */

	/* 動的関数を解放する */
	HZX_RemoveDynamicCallback( work->seg.ptr, &work->dyn_clb ) ;
	work->seg.attribute &= ~(HZX_FLOOR_DYNAMIC | HZX_SEG_DYNAMIC) ;
    }
}

void PUT_BOK_InitHazard( Work *work )
{
    BOOK   *p = &work->book ;
    HZX_HZD flr[2] ;
    int     atr[2] ;

    if ( ( HZX_LevelHazardCheck( work->hzx, &p->mov.pos,
				 HZX_CHK_ALL, HZX_FLOOR_PITFALL ) ) & 1 ) 
    {
	HZX_GetLevelHazard( flr, atr ) ;
	work->seg = flr[0] ;

	printf( "Floor!!\n" ) ;

	/* 今はシナリオ起動のみ動的ハザード処理 */
	if ( HZX_IsDynamic( &work->seg ) )
	{
	    printf( "Set Callback!!\n" ) ;
	    HZX_SetDynamicCallback( work->seg.ptr, &work->dyn_clb,
				    DynamicHzdCallback, work, NULL ) ;
	}
    } 
}

int PUT_BOK_InitTarget( BOOK *bok, int where, DG_MDL *mdl, int idx )
{
    FVECTOR t_size, t_pos ;
    FVECTOR u = { mdl[idx].ux, mdl[idx].uy, mdl[idx].uz, 0.0f } ;
    FVECTOR l = { mdl[idx].lx, mdl[idx].ly, mdl[idx].lz, 0.0f } ;
    TARGET       *t = &bok->target[idx] ;
    POWER_TARGET *p = &bok->power[idx]  ;

    _sceVu0SubVector( &t_size, &u, &l ) ;
    _sceVu0ScaleVector( &t_size, &t_size, 0.5f ) ;
    _sceVu0AddVector( &t_pos, &u, &l ) ;
    _sceVu0ScaleVector( &t_pos , &t_pos , 0.5f ) ;

    GM_SetTarget( t,
		  TARGET_DEFENSE| TARGET_POWER| TARGET_ROTATE|
		  TARGET_NO_CLAYMORE,
		  where, BOTH_SIDE, &t_size, &t_pos ) ;
    GM_SetPowerTarget( t, p, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, PUT_BOK_TargetCallBack, bok ) ;
    GM_PutTarget( t ) ;
    GM_MoveTarget2( t, &bok->objs->objs[idx].world ) ;

    //NewTargetView( t,  200, 50, 32 ) ;

    return 0 ;
}

#if 0
int PUT_BOK_InitPiece( Work *work, int where )
{
    int     i ;
    DG_DEF *def ;
    PIECE  *p ;
    DG_COMDL_POS *pos_s, *pos_l ;

    if ( !(def = (DG_DEF*)GV_GetCache( GV_CacheID( 6602911/*book_hahen*/, 'k' ))) )
	PERROR( "No Dust KMS-MODEL (not in data.cnf) :: NewPutBookObject\n" ) ;

    if ( !(work->piece_l = DG_MakeComdl( def->models[0].packs,
					 DG_COMDL_SEMITRANS, PUT_BOK_N_PIECE/2, 0)) )
	PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutBookObject\n" ) ;
    DG_QueueComdlObjs( work->piece_l ) ;
    GM_GroupObject( work->piece_l, where ) ;
    work->piece_l->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;

    if ( !(work->piece_s = DG_MakeComdl( def->models[1].packs,
					 DG_COMDL_SEMITRANS, PUT_BOK_N_PIECE/2, 0)) )
	PERROR( "Can't Make DG_COMDL(May be no memory) :: NewPutBookObject\n" ) ;
    DG_QueueComdlObjs( work->piece_s ) ;
    GM_GroupObject( work->piece_s, where ) ;
    work->piece_s->flag |= DG_FLAG_INVISIBLE1|DG_FLAG_INVISIBLE2|DG_FLAG_INVISIBLE3 ;

    pos_l = work->piece_l->pos ;
    pos_s = work->piece_s->pos ;
    for ( i=PUT_BOK_N_PIECE/2, p=work->piece ; --i>=0 ; p+=2 )
    {
	p[0].pos  = pos_s++ ;
	p[0].flag = 0 ;
	p[1].pos  = pos_l++ ;
	p[1].flag = 0 ;
    }
    work->n_piece = PUT_BOK_INACTIVE ;

    return 0 ;
}
#endif

int PUT_BOK_InitPage( Work *work )
{
    int   i ;
    PAGE *p ;

    if ( !(work->n_def = (DG_DEF *)GV_GetCache( GV_CacheID( 9315933 /*book_naka*/, 'k' ))) )
	PERROR( "No Page KMS-MODEL (not in data.cnf) :: NewPutBookObject\n" ) ;
    if ( !(work->c_def = (CV2_DEF*)GV_GetCache( GV_CacheID( 9315933 /*book_naka*/, 'c' ))) )
	PERROR( "No Page CV2-MODEL (not in data.cnf) :: NewPutBookObject\n" ) ;

    for ( i=PUT_BOK_N_PAGE, p=work->page ; --i>=0 ; p++ )
    {
	p->flag = -PUT_BOK_SPOOLTIME-1 ;
	p->anime = NULL ;
	p->objs  = NULL ;
    }
    work->n_page |= PUT_BOK_INACTIVE ;

    return 0 ;
}


#if 0
#define XZY 1
extern int MatToEularType ;

void MatToEular( SVECTOR *rot, FMATRIX *mat )
{
    float rx, ry, rz ;

    switch( MatToEularType )
    {
    case 0:
	/* XZY */
	ry = atan2f( mat->m[2][0], mat->m[0][0] ) ;
	rz = asinf( -mat->m[1][0] ) ;
	rx = atan2f( mat->m[1][2], mat->m[1][1] ) ;
	printf("XZY%d %d %d\n",  RAD2ANG( rx ), RAD2ANG( ry ), RAD2ANG( rz ) ) ;
	break ;

    case 1:
	/* ZXY */
	ry = atan2f( -mat->m[0][2], mat->m[2][2] ) ;
	rx = asinf( mat->m[1][2] ) ;
	rz = atan2f( -mat->m[1][0], mat->m[1][1] ) ;

	ry = atan2f( -mat->m[2][0], -mat->m[2][2] ) ;
	rx = acosf( -mat->m[2][1] ) ;
	rz = atanf( mat->m[0][1]/-mat->m[1][1] ) ;

	printf("ZXY%d %d %d %.3f\n",  RAD2ANG( rx ), RAD2ANG( ry ), RAD2ANG( rz ), mat->m[2][1] ) ;
	break ;

    case 2:
	/* ZXY' */
	ry = -atan2f( -mat->m[2][0], -mat->m[2][2] ) ;
	rz = atanf( mat->m[0][1]/-mat->m[1][1] ) ;
	rx = atan2f( sinf( ry )*mat->m[2][1], mat->m[2][2] )  ;
	printf("ZXY'%d %d %d %.3f\n",  RAD2ANG( rx ), RAD2ANG( ry ), RAD2ANG( rz ), mat->m[2][1] ) ;
	break ;

    case 3:
	/* YXZ */
	ry = atan2f( mat->m[2][0], mat->m[2][2] ) ;
	rx = asinf( -mat->m[2][1] ) ;
	rz = atan2f( mat->m[0][1], mat->m[1][1] ) ;
	printf("YXZ%d %d %d\n",  RAD2ANG( rx ), RAD2ANG( ry ), RAD2ANG( rz ) ) ;
	break ;

    case 4:
	/* ZYX */
	rx = atan2f( mat->m[1][2], mat->m[2][2] ) ;
	ry = asinf( -mat->m[0][2] ) ;
	rz = atan2f( mat->m[0][1], mat->m[0][0] ) ;
	printf("ZYX%d %d %d\n",  RAD2ANG( rx ), RAD2ANG( ry ), RAD2ANG( rz ) ) ;
	break ;

    default:
	/* YZX */
	rx = atan2f( -mat->m[2][1], mat->m[1][1] ) ;
	rz = asinf( mat->m[0][1] ) ;
	ry = atan2f( -mat->m[0][2], mat->m[0][0] ) ;
	printf("YZX%d %d %d\n",  RAD2ANG( rx ), RAD2ANG( ry ), RAD2ANG( rz ) ) ;
    }

    rot->vx = RAD2ANG( rx ) ;
    rot->vy = RAD2ANG( ry ) ;
    rot->vz = RAD2ANG( rz ) ;
}
#endif

int PUT_BOK_InitParam( Work *work, DG_OBJS *objs, int where, SVECTOR *rot )
{
    BOOK   *p = &work->book ;
    extern void TS_MatToRot( SVECTOR *rot, FMATRIX *world ) ;

    work->floor = objs->world.m[W][Y] ;

#if 1
    if ( !(p->objs = DG_MakeObjs( objs->def, DG_FLAG_SHADE| DG_FLAG_FINISHCALC, 0 )) )
	PERROR( "Can't initialize DG_OBJS(Maybe no memory) :: NewPutBookObject\n" ) ;
    DG_QueueObjs( p->objs ) ;
    GM_GroupObjs( p->objs, where ) ;
    DG_SetLightMatrix( p->objs, p->lights ) ;
    DG_GetLightMatrix( (FVECTOR*)objs->world.m[W], p->lights ) ;

#else
    p->objs = objs ;
#endif

    p->flag     = 0    ;
    p->work     = work ;
    p->y_offset = 4.0f*rnd() ;
    p->y_pos    = objs->world.m[W][Y] ;
    p->open     = ANG2RAD((1024-64)*2) ;
    p->open_v   = DEG2RAD(0.0f) ;
    work->fall  = 0 ;

    //MatToEular( &p->mov.rot, &objs->world ) ;
    //TS_MatToRot( &p->mov.rot, &objs->world ) ;
    p->rot.vx = p->mov.rot.vx = rot->vx + 1024 ;
    p->rot.vy = p->mov.rot.vy = rot->vy ;
    p->rot.vz = p->mov.rot.vz = rot->vz ;
    _sceVu0CopyVector( &p->mov.pos, (FVECTOR *)objs->world.m[W] ) ;

    //RotateMatrixZY( &p->objs->world, &DG_UnitMatrix, p->mov.rot.vx, p->mov.rot.vy ) ;
    RotateMatrix( &p->objs->world, &DG_UnitMatrix, &p->mov.rot ) ;
    TransMatrix( &p->objs->world, &p->mov.pos ) ;
    PUT_BOK_DispBook( p ) ;

    PUT_BOK_InitTarget( p, where, objs->def->models, 0 ) ;
    PUT_BOK_InitTarget( p, where, objs->def->models, 1 ) ;

    return 0 ;
}
