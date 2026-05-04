//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  utl_eft_bound.c
  エフェクトバウンドチェック

  2001/06/08 T. Morita
  $Id: utl_eft_bound.c,v 1.1.1.3 2002/11/19 11:43:00 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifndef KP_XBOX
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"


#define MAX_FUNC  16
#define MAX_BOUND 8

typedef	void (*EFT_FUNC)( void *ptr ) ;

typedef struct _eft_ctrl_t
{
    EFT_FUNC  func ;
    void     *ptr  ;
} EFFECT ;

static EFFECT  EFT_FuncList[MAX_FUNC] ;
static int     n_EFT_FuncList = 0 ;
static FVECTOR EFT_Bound[MAX_BOUND][2] ;
static int     n_EFT_Bound = 0 ;


/*
  リストやバウンドの初期化
*/
void UTL_EFT_InitBound()
{
    printf( "InitEffectBound Previous n_Func%d n_Bound%d\n", n_EFT_FuncList, n_EFT_Bound ) ;
    n_EFT_FuncList = 0 ;
    n_EFT_Bound = 0 ;
}

/*
  バウンドの個数を返します
*/
int UTL_EFT_NBound()
{
    return n_EFT_Bound ;
}

/*
  バウンドの情報を返します
*/
FVECTOR *UTL_EFT_GetBound( int idx )
{
    return EFT_Bound[idx] ;
}

/*
  コールバック関数を返します

  返り値は,現在のリストの総数です(０もありえます！！)。
*/
void *UTL_EFT_GetFunc( int idx )
{
    return EFT_FuncList[idx].func ;
}


/*
  全てのコールバック関数を実行します。
*/
void UTL_EFT_ExecCallback()
{
    int  i ;

    printf( "UTL_EFT_ExecCallback %d\n", n_EFT_FuncList ) ;

    /* リストから該当するものを捜す */
    for ( i=0 ; i<n_EFT_FuncList ; i++ )
    {
	if ( EFT_FuncList[i].ptr && EFT_FuncList[i].func )
	    (EFT_FuncList[i].func)( EFT_FuncList[i].ptr ) ;
    }
}


/*
  リストからコールバック関数を削除します。

  返り値は,現在のリストの総数です(０もありえます！！)。
  失敗した場合は,-1を返します。
*/
int UTL_EFT_DelCallback( void *p )
{
    int  i ;

    /* リストから該当するものを捜す */
    for ( i=0 ; i<n_EFT_FuncList ; i++ )
	if ( EFT_FuncList[i].ptr == p )
	{
	    n_EFT_FuncList-- ;
	    /* 配列に穴が空かないよう,ずらしていく */
	    for ( ; i<n_EFT_FuncList ; i++ )
	    {
		EFT_FuncList[i].func = EFT_FuncList[i+1].func ;
		EFT_FuncList[i].ptr  = EFT_FuncList[i+1].ptr  ;
	    }
	    EFT_FuncList[i].func = NULL ;
	    EFT_FuncList[i].ptr  = NULL ;
	    return n_EFT_FuncList ;
	}
    return -1 ;
}

/*
  リストからコールバック関数を追加します。

  返り値は,現在のリストの総数です(０もありえます！！)。
  失敗した場合は,-1を返します。
*/
int UTL_EFT_AddCallback( void *f, void *p )
{
    int  i ;

    if ( n_EFT_FuncList >= MAX_FUNC )
	return -1 ;

    for ( i=n_EFT_FuncList ; --i>=0 ; )
	if ( EFT_FuncList[i].ptr == p )
	    return -1 ;
    EFT_FuncList[n_EFT_FuncList].func = f ;
    EFT_FuncList[n_EFT_FuncList].ptr  = p ;

    return ++n_EFT_FuncList ;
}

/*
  与えられた位置のバウンドチェックします。
  成功した場合は,そのインデックスを返します。

  返り値は,バウンドのインデックスです(０もありえます！！)。
  失敗した場合は,-1を返します。
*/
int UTL_EFT_CheckBound( FVECTOR *pos )
{
    int  i ;

    for( i=n_EFT_Bound ; --i>=0 ; )
	if ( vu0_CheckBoundingBox( pos, &EFT_Bound[i][0], &EFT_Bound[i][1] ) )
	    break ;
    return i ;
}

/*
  バウンドを追加します。
  同じフレームの場合のみ追加していき,
  異なったフレームの場合はクリアされ,コールバックが実行されます。

  返り値は,バウンドの総数です。
*/
int UTL_EFT_AddBound( FVECTOR *p0, FVECTOR *p1, int flag )
{
    /* 同一フレームの時は足されていく */
    if ( flag )
	n_EFT_Bound = 0 ;

    _sceVu0CopyVector( &EFT_Bound[n_EFT_Bound][0], p0 ) ;
    _sceVu0CopyVector( &EFT_Bound[n_EFT_Bound][1], p1 ) ;

    n_EFT_Bound++ ;
    if ( n_EFT_Bound >= MAX_BOUND )
    {
#if DEBUG_MODE
	printf( "UTL_EFT_AddBound : Too many Effect Bound!!! MAX_BOUND %d\n", n_EFT_Bound ) ;
#endif
		n_EFT_Bound = MAX_BOUND-1 ;
    }

    return n_EFT_Bound ;
}

/*
  GCLよりバウンドを追加します。
*/
int UTL_EFT_AddBoundFromGCL( void )
{
    FVECTOR p0, p1 ;
    int flag = 0 ;

    /* バウンダリをセット */
    if ( GCL_NextStr() )
	p0.vx = (float)GCL_GetNextInt() ; /* p0座標の取得 */
    if ( GCL_NextStr() )
	p0.vy = (float)GCL_GetNextInt() ; /* p0座標の取得 */
    if ( GCL_NextStr() )
	p0.vz = (float)GCL_GetNextInt() ; /* p0座標の取得 */

    /* バウンダリをセット */
    if ( GCL_NextStr() )
	p1.vx = (float)GCL_GetNextInt() ; /* p1座標の取得 */
    if ( GCL_NextStr() )
	p1.vy = (float)GCL_GetNextInt() ; /* p1座標の取得 */
    if ( GCL_NextStr() )
	p1.vz = (float)GCL_GetNextInt() ; /* p1座標の取得 */


    /* フラグをセットする */
    if ( GCL_NextStr() )
	flag = GCL_GetNextInt() ; /* フラグデータの取得 */

    /* バウンドを追加する */
    UTL_EFT_AddBound( &p0, &p1, flag ) ;

    return 1 ;
}
