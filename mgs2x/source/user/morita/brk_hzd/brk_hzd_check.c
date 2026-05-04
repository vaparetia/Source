//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  brk_hazard.c
  共有箱型ハザード
  
  2000/03/30 T. Morita
  2000/10/16 1.12 T.Morita 
  $Id: brk_hzd_check.c,v 1.1.1.3 2002/11/19 11:45:31 Yoshizawa1 Exp $
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


/* 出力を避ける */
//#define printf(args...)

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

#define AXIS(_x,_a) (((float*)(_x))[_a])


extern HZD_BOX *BRK_HZD_OneHazard  ;
extern int      BRK_HZD_OneFlag    ;
extern float    BRK_HZD_OneHitDist ;
extern int      BRK_HZD_OneAxis    ;
extern FVECTOR  BRK_HZD_OneNewDir  ;
#if DEBUG_MODE
extern int      BRK_HZD_DEBUG ;
#endif

/*

  判定結果をパラメータに保存

*/
void BRK_HZD_SetParameters( int flag, float d, int axis )
{
    if ( d > 0.01f || 1 ) /* 取り敢えずこの条件がまずっているので取る*/
    {
	BRK_HZD_OneFlag |= flag ;
	BRK_HZD_OneHitDist = d ;
	BRK_HZD_OneAxis    = axis ;

#if DEBUG_MODE
	if ( BRK_HZD_DEBUG )
	    printf( "%c HZD_HitDist%.3f dist%f HIT!!\n", (axis & 0x03) + 'X',  BRK_HZD_OneHitDist,  d ) ;
#endif
    }
#if DEBUG_MODE
    else if ( BRK_HZD_DEBUG )
	printf( "%c HZD_HitDist%.3f dist%f ZERO DIST!!\n", (axis & 0x03) + 'X',  BRK_HZD_OneHitDist,  d ) ;
#endif
}

/* (現在 外側)  (未来 外側)  移動     0x00 直線移動チェック*/
/* (現在 内側)  (未来 外側)  移動     0x0f 完全無視        */
/* (現在 外側)  (未来 内側)  跳ね     2    跳ね距離チェック*/
/* (現在 内側)  (未来 内側)  メリコミ 0    メリ込み回避    */
int BRK_HZD_SetOutSize( HZD_BOX *h,
			FVECTOR *npos, FVECTOR *lpos, FVECTOR *sphere,
			FVECTOR *lsize, FVECTOR *rsize )
{
    int     m ;
    FVECTOR v, n ;

    _sceVu0AddVector( lsize, &h->size, sphere ) ; /* 大きさの設定 */
    rsize->vx = 1.0f / lsize->vx ;
    rsize->vy = 1.0f / lsize->vy ;
    rsize->vz = 1.0f / lsize->vz ;
    _sceVu0MulVector( &v, lpos, rsize ) ;
    _sceVu0MulVector( &n, npos, rsize ) ;

    m  = ((int)n.vx ? 0: 0x00000001) | ((int)n.vy ? 0: 0x00000010) | ((int)n.vz ? 0: 0x00000100) ;
    m |= ((int)v.vx ? 0: 0x00010000) | ((int)v.vy ? 0: 0x00100000) | ((int)v.vz ? 0: 0x01000000) ;

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "OUT m%08x lp(%.0f %.0f %.0f) np(%.0f %.0f %.0f) ls(%.0f %.0f %.0f)\n",
		m,
		lpos->vx,lpos->vy,lpos->vz,
		npos->vx,npos->vy,npos->vz,
		lsize->vx,lsize->vy,lsize->vz ) ;

#endif

    return m ;
}

/* (現在 外側)  (未来 外側)  メリコミ0 */
/* (現在 内側)  (未来 外側)  跳ね    2 */
/* (現在 外側)  (未来 内側)  移動    1 */
/* (現在 内側)  (未来 内側)  移動    3 */
int BRK_HZD_SetInSize( HZD_BOX *h,
		       FVECTOR *npos, FVECTOR *lpos, FVECTOR *sphere,
		       FVECTOR *lsize, FVECTOR *rsize )
{
    int m ;
#ifndef BP_PSX2_ASM
    FVECTOR v, n ;

    _sceVu0SubVector( lsize, &h->size, sphere ) ; /* 大きさの設定 */
    rsize->vx = 1.0f / lsize->vx ;
    rsize->vy = 1.0f / lsize->vy ;
    rsize->vz = 1.0f / lsize->vz ;
    _sceVu0MulVector( &v, lpos, rsize ) ;
    _sceVu0MulVector( &n, npos, rsize ) ;

    m  = ((int)n.vx ? 0x00001000 : 0) | ((int)n.vy ? 0x00000100 : 0) | ((int)n.vz ? 0x00000010 : 0) ;
    m |= ((int)v.vx ? 0x10000000 : 0) | ((int)v.vy ? 0x01000000 : 0) | ((int)v.vz ? 0x00100000 : 0) ;
#else

    _sceVu0SubVector( lsize, &h->size, sphere ) ; /* 大きさの設定 */
    asm volatile ("
    lqc2        vf1,0x00(%0)
    lqc2        vf2,0x00(%1)
    lqc2        vf3,0x00(%2)
    vmulx.w     vf16,vf0,vf1
    vmuly.w     vf17,vf0,vf1
    vmulz.w     vf18,vf0,vf1
    vclipw.xyz  vf2xyz,vf16w
    vclipw.xyz  vf2xyz,vf17w
    vclipw.xyz  vf2xyz,vf18w
    vnop
    vnop
    vnop
    vnop
    cfc2        $8,$vi18
    andi        $8,$8,0x3330
    sll         $9,$8,16       /*上位１６ビットが現在の位置判定*/
    vclipw.xyz  vf3xyz,vf16w
    vclipw.xyz  vf3xyz,vf17w
    vclipw.xyz  vf3xyz,vf18w
    vnop
    vnop
    vnop
    vnop
    cfc2        $8,$vi18
    andi        $8,$8,0x3330
    or          $8,$8,$9       /*下位１６ビットが未来の位置判定*/
    sw          $8,0(%3)
    " : : "r"(lsize), "r"(lpos), "r"(npos), "r"(&m) :  "$8","$9","memory" );
#endif

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "IN m%08x lp(%.0f %.0f %.0f) np(%.0f %.0f %.0f) ls(%.0f %.0f %.0f)\n",
		m,
		lpos->vx,lpos->vy,lpos->vz,
		npos->vx,npos->vy,npos->vz,
		lsize->vx,lsize->vy,lsize->vz ) ;
#endif

    return m ;
}

/**********************************************

  各場合のハザード計算

  **********************************************/
int BRK_HZD_OutsideBoxCheck( HZD_BOX *h, FMATRIX *w,
			     FVECTOR *npos, FVECTOR *lpos, FVECTOR *ldir,
			     FVECTOR *lsize, FVECTOR *rsize, int m )
{
    int     i, through ;
    float   d ;
    FVECTOR v ;

    if ( m == 0x01110111 )/*メリ込み*/
    {
	for ( i=XYZ ; --i>=0 ; )
	{
	    d = AXIS(lpos,i) > 0.0f ? AXIS(lpos,i) - AXIS(lsize,i) - 0.1f :
		                      AXIS(lpos,i) + AXIS(lsize,i) + 0.1f ;
#if DEBUG_MODE
	    if ( BRK_HZD_DEBUG )
		printf( "%c HZD_HitDist%.3f lpos%.3f -lsiz%.3f = d%.3f\n", (i & 0x03) + 'X',  BRK_HZD_OneHitDist, AXIS(lpos,i), AXIS(lsize,i), d ) ;
#endif

	    if ( (!(BRK_HZD_OneFlag & BRK_HZD_THROUGH) ) ||  /* 初回 */
		 (!(int)(d / BRK_HZD_OneHitDist) ) )         /* メリコミ回避条件 */
	    {
#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
		printf( " OUT MERI " ) ;
#endif
		BRK_HZD_SetParameters( BRK_HZD_THROUGH,
				       AXIS(lpos,i) > 0.0f ?       -d : d,
				       AXIS(lpos,i) > 0.0f ? 0x8000|i : i ) ;
	    }
	}
	return 0 ;
    }

    /* つき抜けの可能性 */
    _sceVu0MulVector( &v, lpos, npos ) ; 
    through = (v.vx<0.0f || v.vy<0.0f || v.vz<0.0f) ? 1 : 0 ;

    if ( (m & 0x0111) == 0x0111 || through )/*弾み or つき抜け */
    {
	/* 各軸に対して平面の交点を調べる（ローカル座標で計算） */
	for ( i=XYZ ; --i>=0 ; )
	    if ( AXIS(ldir,i) != 0.0f )  /* 0 だったら平面と直線は平行なので関係なくなる */
	    {
		if ( AXIS(ldir, i) < 0.0f )
		    d = AXIS(lpos, i) - AXIS(lsize, i) - 0.1f ;
		else
		    d = AXIS(lpos, i) + AXIS(lsize, i) + 0.1f ;
		d /= -AXIS(ldir, i) ;

		/*if ( d >= 0.0f && d < BRK_HZD_OneHitDist )*/
		if ( BRK_HZD_Velocity * d >= -50.0f && d < BRK_HZD_OneHitDist )
		{
		    if ( through )
		    {
			_sceVu0ScaleVector( &v, ldir, d ) ;
			_sceVu0AddVector( &v, &v, lpos  ) ;
			_sceVu0MulVector( &v, &v, rsize ) ;
#if DEBUG_MODE
			if ( BRK_HZD_DEBUG )
			    printf( " lv%f %f %f\n", v.vx,v.vy,v.vz ) ;
#endif

#define ONE 1.000001f
			if ( (i==X && (v.vy> ONE || v.vz> ONE ||
				       v.vy<-ONE || v.vz<-ONE) ) ||
			     (i==Y && (v.vx> ONE || v.vz> ONE ||
				       v.vx<-ONE || v.vz<-ONE) ) ||
			     (i==Z && (v.vx> ONE || v.vy> ONE ||
				       v.vx<-ONE || v.vy<-ONE) ) )
			    continue ;
		    }

#if DEBUG_MODE
		    if ( BRK_HZD_DEBUG )
			printf( through ? " OUT TUKI " : " OUT HAZU " )  ;
#endif

		    BRK_HZD_SetParameters( BRK_HZD_HITONE, d,
					   AXIS(ldir,i) < 0.0f ? 0x8000|i : i ) ;
		}
	    }
    }

    return m ;
}




/**** 箱の内側当たり計算 *******/
int BRK_HZD_InsideBoxCheck( HZD_BOX *h, FMATRIX *w,
			    FVECTOR *npos, FVECTOR *lpos, FVECTOR *ldir, 
			    FVECTOR *lsize, FVECTOR *rsize, int m )
{
    int     i ;
    FVECTOR v ;
    float   d ;

    /* 弾み */
    if ( m <= 0x3330 && m > 0 )
    {
	/* 各軸に対して平面の交点を調べる（ローカル座標で計算） */
	for ( i=XYZ ; --i>=0 ; )
	    if ( AXIS(ldir,i) != 0.0f && m & (0x3000>>(i*4)) )  /* 0 なら平面と直線は平行なので除外 */
	    {
		if ( AXIS(ldir, i) > 0.0f )
		    d = AXIS(lpos, i) - AXIS(lsize, i) + 0.1f ;
		else
		    d = AXIS(lpos, i) + AXIS(lsize, i) - 0.1f ;
		d /= -AXIS(ldir, i) ;

		if ( BRK_HZD_Velocity * d >= -50.0f && d < BRK_HZD_OneHitDist )
		{
#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( " IN HAZU lpos%.3f lsiz%.3f ",AXIS(lpos, i),AXIS(lsize, i) ) ;
#endif
		    BRK_HZD_SetParameters( BRK_HZD_HITONE, d,
					   AXIS(ldir,i)>0.0f ? i:(0x8000 | i)  ) ;
		}
	    }
    }

    /*メリ込み*/
    else if ( m & 0x3330 )
    {
	/* 各軸に対して平面の距離を計算しメリ込み回避の移動量(BRK_HZD_OneNewDir)にする */
	_sceVu0CopyVector( &BRK_HZD_OneNewDir, &DG_ZeroVector ) ;
	for ( i=XYZ ; --i>=0 ; )
	    if ( m & (0x3000>>(i*4)) )/*はみ出ている軸全てに補正を掛ける*/
	    {
		d = AXIS(lpos, i) > 0.0f ? (AXIS(lpos, i) - AXIS(lsize, i) + 0.1f) :
		                           (AXIS(lpos, i) + AXIS(lsize, i) - 0.1f) ;
		//if ( !(int)(d*100.0f) ) d = AXIS(lpos, i)>0.0f ? 0.001f : -0.001f ;
		_sceVu0ScaleVector( &v, (FVECTOR*)w->m[i], -d ) ;
		_sceVu0AddVector( &BRK_HZD_OneNewDir, &BRK_HZD_OneNewDir, &v ) ;
#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( " IN MERI lpos%.3f lsiz%.3f d%f ",AXIS(lpos, i),AXIS(lsize, i), d ) ;
#endif
		BRK_HZD_SetParameters( BRK_HZD_THROUGH, 1.0f,
				       (0x4000 | i) ) ;
	    }

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "newdir %.3f %.3f %.3f\n", BRK_HZD_OneNewDir.vx, BRK_HZD_OneNewDir.vy, BRK_HZD_OneNewDir.vz ) ;
#endif
    }

    return m ;
}


/**** 箱の何れかの軸正方向が空いている場合の当たり計算 *******/
int HZX_OpenBoxCheck( HZD_BOX *h, FMATRIX *w,
		      FVECTOR *npos, FVECTOR *lpos, FVECTOR *ldir, FVECTOR *sphere,
		      int axis )
{
    int     out, in ;
    int     outmsk, inmsk ;
    FVECTOR o_lsize, o_rsize ;
    FVECTOR i_lsize, i_rsize ;

    in  = BRK_HZD_SetInSize( h, npos, lpos, sphere, &i_lsize, &i_rsize ) ;
    out = BRK_HZD_SetOutSize( h, npos, lpos, sphere, &o_lsize, &o_rsize ) ;

    /* axis の正は,開いているので当たりをとらないためのマスク設定*/
    if ( AXIS(npos, axis) > 0.0f && out >= 0x01110000 )
	outmsk=~(0x00010001 << (axis*4)), inmsk = ~(0x00100010 << (axis*4)) ;
    else
	outmsk = 0x01110111, inmsk = 0x33303330 ;

#if DEBUG_MODE
    if ( BRK_HZD_DEBUG )
	printf( "HAKO (OUT%08x IN%08x)=>(OUT%08x IN%08x)",
		out,in,
		out & outmsk, in & inmsk ) ;
#endif

    if ( out >= 0x01110000 )
    {
#if DEBUG_MODE
	if ( BRK_HZD_DEBUG ) printf( "CHCK IN\n"  ) ;
#endif
	BRK_HZD_InsideBoxCheck( h, w, npos, lpos, ldir, &i_lsize, &i_rsize, in & inmsk ) ;
    }
    else if ( (out&0x0111)==0x0111 && in==0 )
    {
#if DEBUG_MODE
    if ( BRK_HZD_DEBUG ) printf( "CHCK OUT\n"  ) ;
#endif
	BRK_HZD_OutsideBoxCheck( h, w, npos, lpos, ldir, &o_lsize, &o_rsize, out & outmsk ) ;
    }
#if DEBUG_MODE
    else if ( BRK_HZD_DEBUG ) printf( "\n"  ) ;
#endif

    return 0 ;
}


/**** 箱の外側当たり計算 *******/
int HZX_PlaneCheck( HZD_BOX *h, FMATRIX *w,
		    FVECTOR *npos, FVECTOR *lpos, FVECTOR *ldir, FVECTOR *sphere )
{

    return 0 ;
}
