//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  brk_hazard.c
  共有箱型ハザード
  
  2000/03/30 T. Morita
  2000/10/16 1.14 T.Morita
  $Id: brk_hazard.c,v 1.1.1.3 2002/11/19 11:45:31 Yoshizawa1 Exp $
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

#include "brk_hazard.h"
#include "../include/util.h"


#if DEBUG_MODE

//#define SHOWBOXHZD //このデファインをコメントアウトすると枠が消える

#ifdef SHOWBOXHZD
static void  *dbg_work[BRK_N_HZD+BRK_N_HZD] ; /*debug code*/
#endif

#endif

/* ハザード検出 */
HZD_BOX *BRK_HZD_Hazard  ;
int      BRK_HZD_Flag    ;
float    BRK_HZD_HitDist ;
FVECTOR  BRK_HZD_HitPos  ;
float    BRK_HZD_Velocity ;
FVECTOR  BRK_HZD_Normal  ;
HZD_BOX *BRK_HZD_OneHazard  ;
int      BRK_HZD_OneFlag    ;
float    BRK_HZD_OneHitDist ;
int      BRK_HZD_OneAxis    ;
int      BRK_HZD_LogicTime  ;
FVECTOR  BRK_HZD_OneNewDir  ;

#if DEBUG_MODE
int      BRK_HZD_DEBUG = 0 ;
int      BRK_HZD_TMPCNT = 0 ;
#endif

FVECTOR  BRK_HZD_NoBounce = { 1.0000001f, 1.0000001f, 1.0000001f, 1.0f }  ;

/* ダイナミックハザード */
extern int      BRK_HZD_n_hzd ;
extern HZD_BOX *BRK_HZD_Pool[BRK_N_HZD] ;


/**********************************************

  ステージハザードの取得

  **********************************************/
void BRK_InitSize( HZD_BOX *h )
{
    h->size.vw = _sceVu0InnerProduct( &h->size, &h->size ) ;
}

HZD_BOX *BRK_InitHazard( enum stage_id_t where )
{
    int      i ;
    HZD_BOX *h ;

#ifdef DEBUG_MODE
    if ( where > BRK_HZD_NList )
    {
	printf( "No Such StageID [id(%d)>max(%d)] : BRK_InitHazard\n", where, BRK_HZD_NList ) ;
	return NULL ;
    }
#endif

    h = BRK_HZDList[where] ;
    for ( i=0 ; h->flag ; h++, i++ )
    {
	BRK_InitSize( h ) ;
	_sceVu0CopyMatrix( &h->work, &h->world ) ;
	_sceVu0InversMatrix( &h->invrs, &h->work ) ;

#ifdef DEBUG_MODE
#ifdef SHOWBOXHZD
	if ( h->flag & BRK_HZD_INSIDE )
	    dbg_work[BRK_N_HZD+i] = NewHzdBoxView( h, 50, 32, 200 ) ;
	else if ( h->flag & BRK_HZD_OUTSIDE )
	    dbg_work[BRK_N_HZD+i] = NewHzdBoxView( h, 32, 100,150 ) ;
	else
	    dbg_work[BRK_N_HZD+i] = NewHzdBoxView( h, 32, 200, 50 ) ;
#endif
#endif

    }

    /* グローバルの初期化も済ます */
    BRK_HZD_Hazard    = NULL ;
    BRK_HZD_Flag      = 0    ;
    BRK_HZD_LogicTime = 0    ;
    BRK_HZD_OneHazard = NULL ;
    BRK_HZD_OneFlag   = 0    ;

    return BRK_HZDList[where] ;
}




/**********************************************

  ハザードの登録／削除

  **********************************************/

HZD_BOX *BRK_MakeHazard( int flag, FMATRIX *root, FMATRIX *world,
			 FVECTOR *size, FVECTOR *center, TARGET *t )
{
    int i ;
    HZD_BOX *h ;
    static FVECTOR def_size = { 1.0f, 1.0f, 1.0f, 0.0f } ;

    if ( BRK_HZD_n_hzd < BRK_N_HZD )
	for ( i=0 ; i<BRK_N_HZD ; i++ )
	    if ( BRK_HZD_Pool[i] == NULL )
		if ( (h = GV_Malloc( sizeof(HZD_BOX) )) )
		{
		    /* ハザードの初期化 */
		    h->flag = flag ;
		    h->root = root ;
		    if ( !world )
			world = &DG_UnitMatrix ;
		    _sceVu0CopyMatrix( &h->world, world ) ;
		    if ( center )
			_sceVu0CopyVectorXYZ( (FVECTOR *)h->world.m[3], center ) ;
		    _sceVu0CopyMatrix( &h->work, &h->world ) ;
		    _sceVu0InversMatrix( &h->invrs, &h->work ) ;
		    h->size = size ? *size : def_size ;
		    BRK_InitSize( h ) ;

		    /* プールに登録 */
		    BRK_HZD_Pool[i] = h ;
		    BRK_HZD_n_hzd++ ;

#ifdef DEBUG_MODE
#ifdef SHOWBOXHZD
		    if ( h->flag & BRK_HZD_INSIDE )
			dbg_work[i] = NewHzdBoxView( h, 50, 32, 200 ) ;
		    else if ( h->flag & BRK_HZD_OUTSIDE )
			dbg_work[i] = NewHzdBoxView( h, 32, 100,150 ) ;
		    else
			dbg_work[i] = NewHzdBoxView( h, 32, 200, 50 ) ;
#endif
#endif
		    return h ;
		}

#if DEBUG_MODE
    printf( BRK_HZD_n_hzd < BRK_N_HZD ? "No Memory" : "Brk Hazard Que Overfull" ) ;
    printf( ": BRK_MakeHazard\n" ) ;
#endif

    return NULL ;
}

int BRK_FreeHazard( HZD_BOX *h )
{
    int  i ;

    if ( h )
	for ( i=BRK_N_HZD ; --i>=0 ; )
	    if ( BRK_HZD_Pool[i] == h )
	    {
		BRK_HZD_n_hzd-- ;
		BRK_HZD_Pool[i] = NULL ;
		GV_Free( h ) ;

#if DEBUG_MODE
#ifdef SHOWBOXHZD
		if ( dbg_work[i] )
		    GV_DestroyOtherActor( dbg_work[i] ), dbg_work[i] = NULL ;
#endif
#endif

		return 0 ;
	    }

#if DEBUG_MODE
    if ( h )
	printf( "Hazard Not Found!!! So didn't GV_Free()!!! : BRK_FreeHazard\n" ) ;
#endif

    return 1 ;
}


/**********************************************

  一つの 箱当たりチェック関数

  **********************************************/
//#define printf(args...)

static void BRK_CheckOneHazard( HZD_BOX *h, FVECTOR *pos, FVECTOR *pos_v,
				FVECTOR *bounce, FVECTOR *sphere,
				float radius )
{
    FVECTOR *npos  = (FVECTOR *)(SCRPAD_ADDR + 1024*16) - 1 ;/*ローカル座標の新しい位置 */
    FVECTOR *lpos  = (FVECTOR *)(SCRPAD_ADDR + 1024*16) - 2 ;/*ローカル座標の現在の位置 */
    FVECTOR *ldir  = (FVECTOR *)(SCRPAD_ADDR + 1024*16) - 3 ;/*ローカル座標の現在の方向 */
    FMATRIX *w = &h->work ;

    /* pos を箱のローカル座標に変換する→ lpos        */
    if ( h->flag & BRK_HZD_ROTATE ) /* 回転ありの場合 */
	_sceVu0ApplyMatrix( lpos, &h->invrs, pos ) ;
    else
	_sceVu0SubVector( lpos, pos, (FVECTOR *)h->world.m[3] ) ;

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "CheckOneHzdStart flg%x (%.0f>%.0f+%.0f)%d  world.m[3](%.0f %.0f %.0f) (%.0f %.0f %.0f %.0f)\n",
		h->flag,
		_sceVu0InnerProduct( lpos, lpos ), radius, h->size.vw,
		_sceVu0InnerProduct( lpos, lpos ) > radius + h->size.vw,
		h->world.m[3][0],h->world.m[3][1],h->world.m[3][2],
		lpos->vx,lpos->vy,lpos->vz,lpos->vw ) ;
#endif

    /* 距離が遠かったら処理を行なわない */
    if ( _sceVu0InnerProduct( lpos, lpos ) > radius + h->size.vw )
	return ;

    /* pos_v を箱のローカル座標に変換する→ ldir      */
    if ( h->flag & BRK_HZD_ROTATE ) /* 回転ありの場合 */
	_sceVu0ApplyMatrix( ldir, &h->invrs, pos_v ) ;
    else
	_sceVu0CopyVector( ldir, pos_v ) ;
    _sceVu0AddVector( npos, lpos, ldir  ) ;

    BRK_HZD_OneFlag    = 0    ;
    BRK_HZD_OneHitDist = 1.0f ;    
    BRK_HZD_OneAxis    = 0    ;

    switch( h->flag & BRK_HZD_FLGMSK )
    {
	int  m ;
	FVECTOR lsize ;
	FVECTOR rsize ;

    case BRK_HZD_OUTSIDE:
	m = BRK_HZD_SetOutSize( h, npos, lpos, sphere, &lsize, &rsize ) ;
	BRK_HZD_OutsideBoxCheck( h, w, npos, lpos, ldir, &lsize, &rsize, m ) ;
	break ;				       

    case BRK_HZD_INSIDE:    
	m = BRK_HZD_SetInSize( h, npos, lpos, sphere, &lsize, &rsize ) ;
	BRK_HZD_InsideBoxCheck( h, w, npos, lpos, ldir, &lsize, &rsize, m ) ;
	break ;

	/* 箱にX軸正方向が空いている当たり */
    case BRK_HZD_X_BOX:
	HZX_OpenBoxCheck( h, w, npos, lpos, ldir, sphere, 0 ) ;
	break ;					    

	/* 箱にY軸正方向が空いている当たり */	    
    case BRK_HZD_Y_BOX:
	HZX_OpenBoxCheck( h, w, npos, lpos, ldir, sphere, 1 ) ;
	break ;					    

	/* 箱にZ軸正方向が空いている当たり */	    
    case BRK_HZD_Z_BOX:
	HZX_OpenBoxCheck( h, w, npos, lpos, ldir, sphere, 2 ) ;
	break ;
    }

    if ( BRK_HZD_OneFlag )
	if ( (BRK_HZD_Flag & BRK_HZD_THROUGH ? 
	      (BRK_HZD_HitDist < BRK_HZD_OneHitDist) :/*既にメリ込んでいるのでメリ込みが*/
	      (BRK_HZD_HitDist > BRK_HZD_OneHitDist || BRK_HZD_OneFlag & BRK_HZD_THROUGH)) ||
	     BRK_HZD_OneAxis & 0x4000 )
	{
	    BRK_HZD_Hazard  = h                  ;
	    BRK_HZD_Flag   |= BRK_HZD_OneFlag    ;
	    BRK_HZD_HitDist = BRK_HZD_OneHitDist ;
	    if ( BRK_HZD_OneAxis & 0x4000 )
		_sceVu0CopyVector( &BRK_HZD_Normal, &BRK_HZD_OneNewDir ) ;
	    else
		_sceVu0ScaleVector( &BRK_HZD_Normal,
				    (FVECTOR*)w->m[ BRK_HZD_OneAxis & 0x0003 ],
				    (BRK_HZD_OneAxis & 0x8000 ? 1.0f : -1.0f) ) ;
#if DEBUG_MODE
	    if ( BRK_HZD_DEBUG )
		printf( "Change HZD_HitDist%f\n",	BRK_HZD_HitDist  ) ;
#endif
	}
#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "\n" ) ;
#endif

}


static inline void BRK_CalcWorkMatrix( HZD_BOX *h )
{
    if ( h->flag & BRK_HZD_ROTATE ) /* 回転ありの場合 */
	if ( h->root ) /* 親がいれば考慮する */
	{
	    _sceVu0MulMatrix( &h->work, h->root, &h->world ) ;
	    _sceVu0InversMatrix( &h->invrs, &h->work ) ;
	}
}

#define BRK_HZD_MINVEL 200.0f


int BRK_CheckHazard( HZD_BOX *fix, FVECTOR *pos, FVECTOR *pos_v, FVECTOR *bounce, FVECTOR *sphere )
{
    HZD_BOX **que ;
    HZD_BOX *h ;
    int     i ;
    float   sq_vel ;

#ifdef SHOWBOXHZD
#if 0
    int j=0 ;

    que = BRK_HZD_Pool ;
    for ( i=BRK_HZD_n_hzd ; i>0 ; j++, que++ )
	if ( *que )
	    if ( (*que)->root )
		GM_MoveTarget2( &dbg_hzd[j], (*que)->root ), i-- ;
#endif
#endif
    pos_v->vw = 0.0f ;/* 位置を反映させたくない 良くないが勝手に変えちゃう */
    pos->vw   = 1.0f ;/* 位置を反映させたい     良くないが勝手に変えちゃう */
    sq_vel = _sceVu0InnerProduct( pos_v, pos_v ) ;
    BRK_HZD_Hazard   = NULL ;
    BRK_HZD_Flag    &= BRK_HZD_USRMASK ;
    BRK_HZD_HitDist  = 1.0f ;
    BRK_HZD_Velocity = sceVu0Sqrt( sq_vel ) ;
    sq_vel += _sceVu0InnerProduct( sphere, sphere ) ;
    if ( sq_vel < BRK_HZD_MINVEL*BRK_HZD_MINVEL*3.0f )
	sq_vel = BRK_HZD_MINVEL*BRK_HZD_MINVEL*3.0f ;

    /* ワークの計算 */
    /* 同じハザードは,2度以上ワークを通らない */
    if ( BRK_HZD_LogicTime != GM_StagePlayTime + (int)fix )
    {
	BRK_HZD_LogicTime = GM_StagePlayTime + (int)fix ;

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "Calc HazardWork\n" ) ;
#endif

	/*ダイナミック ハザード*/
	if ( !(BRK_HZD_Flag & BRK_HZD_FIXONLY) )
	    for ( que=BRK_HZD_Pool, i=BRK_HZD_n_hzd ; i>0 ; que++ )
		if ( *que )
		    BRK_CalcWorkMatrix( *que ), i-- ;
    }

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "DYNAMIC HazardCheckStart FLG%d N%d sq_v%.2f\n", BRK_HZD_Flag & BRK_HZD_FIXONLY, BRK_HZD_n_hzd, sq_vel ) ;
#endif
    /*ダイナミック ハザード*/
    if ( !(BRK_HZD_Flag & BRK_HZD_FIXONLY) )
	for ( que=BRK_HZD_Pool, i=BRK_HZD_n_hzd ; i>0 ; que++ )
	    if ( *que )
		BRK_CheckOneHazard( *que, pos, pos_v, bounce, sphere, sq_vel ), i-- ;

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "FIX HazardCheckStart\n" ) ;
#endif
    /*フィックス ハザード*/
    for ( h=fix ; h->flag ; h++ )
	BRK_CheckOneHazard( h, pos, pos_v, bounce, sphere, sq_vel ) ;

    if ( BRK_HZD_Flag )
    {
	FVECTOR v ;

#if DEBUG_MODE
	if ( BRK_HZD_DEBUG )
	{
	    if ( !(BRK_HZD_Flag & BRK_HZD_THROUGH) )
		printf( "HAZUMI%08x BRK_HZD_HitDist%f\n", BRK_HZD_Flag, BRK_HZD_HitDist ) ;
	    else 
		printf( "MERIKOMI%08x BRK_HZD_HitDist%.2f MaxVel%.2f<50\n", BRK_HZD_Flag,  BRK_HZD_HitDist, BRK_HZD_Velocity * BRK_HZD_HitDist ) ;
	    printf( "  siz   %.2f %.2f %.2f\n", sphere->vx  , sphere->vy  , sphere->vz   ) ;
	    /* !(int)(BRK_HZD_Velocity * BRK_HZD_HitDist * (1.0f/50.0f))*/
	    printf( "  pos   %.2f %.2f %.2f\n", pos->vx  , pos->vy  , pos->vz   ) ;
	    printf( "  pos_v %.2f %.2f %.2f\n", pos_v->vx, pos_v->vy, pos_v->vz ) ;
	}
#endif

	/* 弾む場所を計算する。メリ込みなら 現在位置から一番近い場所 */
	_sceVu0ScaleVector( &v,
			    BRK_HZD_Flag & BRK_HZD_THROUGH ? &BRK_HZD_Normal : pos_v,
			    BRK_HZD_HitDist ) ;
	_sceVu0AddVector( pos, pos, &v ) ;
	_sceVu0CopyVector( &BRK_HZD_HitPos, pos ) ;

	/* 速度を計算 */
	if ( BRK_HZD_Flag & BRK_HZD_THROUGH && !(int)(BRK_HZD_HitDist*(1.0f/1000.0f)) )
	{
	    _sceVu0Normalize( &BRK_HZD_Normal, &BRK_HZD_Normal ) ;
	    _sceVu0ScaleVector( pos_v, &BRK_HZD_Normal, BRK_HZD_Velocity*0.9f ) ;
	}
	else if ( (v.vw = _sceVu0InnerProduct( pos_v, &BRK_HZD_Normal )) < 0.0f )
	{
	    /*****  跳ね返り計算 *****/
	    _sceVu0ScaleVector( &v, &BRK_HZD_Normal, v.vw ) ;/* 跳ね返り方向の速度 */ ;
	    _sceVu0MulVector( &v, &v, bounce ) ;
	    _sceVu0SubVector( pos_v, pos_v, &v ) ;
	}

#if DEBUG_MODE
	if ( BRK_HZD_DEBUG )
	{
	    printf( "  pos   %.2f %.2f %.2f\n"      , pos->vx  , pos->vy  , pos->vz         ) ;
	    printf( "  pos_v %.2f %.2f %.2f  vw%f\n", pos_v->vx, pos_v->vy, pos_v->vz, v.vw ) ;
	    printf( "  nrm   %.2f %.2f %.2f\n", BRK_HZD_Normal.vx, BRK_HZD_Normal.vy, BRK_HZD_Normal.vz ) ;
	    printf( "  hzd   %x cen%.2f %.2f %.2f\n", BRK_HZD_Hazard->flag, BRK_HZD_Hazard->world.m[3][0], BRK_HZD_Hazard->world.m[3][1], BRK_HZD_Hazard->world.m[3][2] ) ;
	}
#endif

	return BRK_HZD_Normal.vy > 0.7f  ? 1/*床*/ : 2/*壁*/ ;
    }

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "\n" ) ;
#endif

    return 0 ;
}


/*

  スクラッチパッド化

*/
void BRK_MoveHazardToScr( HZD_BOX *fix )
{
    HZD_BOX **que ;
    HZD_BOX *h ;
    int     i ;
    HZD_BOX *scr = (void*)SCRPAD_ADDR ;

    /*ダイナミック ハザード*/
    for ( que=BRK_HZD_Pool, i=BRK_HZD_n_hzd ; i>0 ; que++ )
	if ( *que )
	{
	    BRK_CalcWorkMatrix( *que ) ;
	    i-- ;
	    *scr++ = **que ;
	}

    /*フィックス ハザード*/
    for ( h=fix ; h->flag ; h++ )
    {
	BRK_CalcWorkMatrix( h ) ;
	*scr++ = *h ;
    }
    *scr = *h ;
    BRK_HZD_LogicTime = GM_StagePlayTime ;
}

void BRK_ExitHazardToScr( void )
{
    BRK_HZD_LogicTime-- ;
    BRK_HZD_Flag &= ~BRK_HZD_FIXONLY ;
}

int BRK_CheckHazardScr( FVECTOR *pos, FVECTOR *pos_v, FVECTOR *bounce, FVECTOR *sphere )
{
    BRK_HZD_Flag |= BRK_HZD_FIXONLY ;

    return BRK_CheckHazard( (void*)SCRPAD_ADDR, pos, pos_v, bounce, sphere ) ;
}

/*
int BRK_CheckTargetHazard( HZD_BOX *h, FMATRIX *new, FVECTOR *pos, FVECTOR *pos_v,
			   FVECTOR *bounce, TARGET *t ) ;
*/
int BRK_CheckTargetHazard( HZD_BOX *h, FMATRIX *new, FVECTOR *pos_v, FVECTOR *bounce, TARGET *t )
{
    int     flag, i ;
    FVECTOR p, s, v ;
    FVECTOR min={ 60000,60000,60000 }, max={ -60000,-60000,-60000 } ;

    _sceVu0ApplyMatrix( &p, &t->world, &t->offset ) ;
    _sceVu0AddVector( &p, &p, &t->center ) ;
    _sceVu0CopyVector( pos_v, &p ) ;
    _sceVu0CopyVector( &v, &t->offset ) ;
    v.vw= 1.0f ;
    _sceVu0ApplyMatrix( &v, new, &v ) ;
    _sceVu0AddVector( &v, &v, &t->center ) ;

    _sceVu0SubVector( &v, &v, &p ) ;
    s.vw = 0.0f ;
    for ( i=8 ; --i>=0 ; )
    {
	s.vx = i&1 ? t->size.vx : -t->size.vx ;
	s.vy = i&2 ? t->size.vy : -t->size.vy ;
	s.vz = i&4 ? t->size.vz : -t->size.vz ;
	_sceVu0ApplyMatrix( &s, new, &s ) ;

#if BP_PSX2_ASM
	asm volatile ( "
	    lqc2        vf3, 0(%2)
	    lqc2        vf1, 0(%0)
	    lqc2        vf2, 0(%1)
	    vmini.xyz   vf1, vf1, vf3
	    vmax.xyz    vf2, vf2, vf3
	    sqc2        vf1, 0(%0)
	    sqc2        vf2, 0(%1)
	    ": : "r"(&min), "r"(&max), "r"(&s) ) ;
#else
	MinMaxVector( &min, &max, &s ) ;
#endif
    }

    _sceVu0SubVector( &s, &max, &min ) ;
    _sceVu0ScaleVector( &s, &s, 0.5f ) ;

#if 0
    if ( BRK_HZD_DEBUG )
    printf( "sz(%.2f %.2f %.2f)",	s.vx, s.vy, s.vz    ) ;
    printf( "vel(%.2f %.2f %.2f)",	v.vx, v.vy, v.vz    ) ;
    printf( "Now(%.2f %.2f %.2f)\n",p.vx, p.vy, p.vz    ) ;
    AN_Test_Eye2( &p, 1 ) ;
#endif

    flag = BRK_CheckHazard( h, &p, &v, bounce, &s ) ;
    _sceVu0SubVector( pos_v, &p, pos_v ) ;

#if 0
    printf( "\tNew(%.2f %.2f %.2f)=  V%.2f %.2f %.2f flag %d\n",
	    p.vx, p.vy, p.vz,
	    pos_v->vx, pos_v->vy, pos_v->vz, flag ) ;
#endif

    return flag ;
}
