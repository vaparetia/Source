//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   TestBox.c
   センサー付き箱
   
   2001/03/13 S.Kobayashi
   $Id: TestBox.c,v 1.1.1.3 2002/11/19 11:50:23 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

extern void	*NewTargetView2( TARGET			*targ, u_char r, u_char g, u_char b );

#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

typedef	struct _BELTOBJ_LIST {
	struct _BELTOBJ_LIST	*next ;
	OBJECT					*body ;
} BELTOBJ_LIST ;

typedef	struct	{
	GV_ACT_EX		actor;
	OBJECT			body;        // モデル情報
	FMATRIX			lights[ 2 ]; // ライト二つ
	FVECTOR			pos;         // 表示位置
	TARGET			target;      // ターゲット情報
	int				map ;
	int				name ;
	void			*objhzx ;
//	BELTOBJ_LIST	list ;
	int             proc;
} Work ;

// プロトタイプ
static void ExecProc( TARGET * , TARGET * , void * );
static void SetTarget( Work * );

static int count = 0;

static void ExecProc( TARGET *pOff , TARGET *pDef , void *pWork )
{
	Work *pWork2;
	GCL_ARGS args;
	int buf[ 2 ];

	pWork2 = ( Work * )pWork;
	if ( pWork2->proc == 0 ){
		return;
	}
	args.argc = 2;
	args.argv = buf;

	buf[ 0 ] = pWork2->name;
	buf[ 1 ] = ++count;
	GM_ExecProc( pWork2->proc , &args );
}

static void SetTarget( Work *pWork )
{
	TARGET *pTrg;
	FVECTOR size;

	pTrg = &pWork->target;
	size.vx = size.vy = size.vz = 100.F;             // ターゲットの大きさ
	GM_SetTarget( pTrg , TARGET_DEFENSE  | TARGET_POWER | POWER_ONCE , pWork->map , BOTH_SIDE , &size , &DG_ZeroVector );
	GM_MoveTarget( pTrg , &pWork->pos );             // ターゲットのトランス
	GM_SetTargetCallBack( pTrg , ExecProc , pWork ); // コールバックの設定
	GM_PutTarget( pTrg );							 // ターゲットの配置
}

static void Act( Work *pWork )
{
	GM_SetCurrentMap( pWork->map ); // 表示設定
	NewTargetView2( &pWork->target , 128 , 128 , 160 ); // ターゲットの表示
//	GM_MoveTarget( &pWork->target, &pWork->pos );       // ターゲットのトランス
}

static void Die( Work *pWork )
{
	GM_FreeObject( &pWork->body ); // オブジェクトの解放
}

static int GetResources( Work *pWork , int name , int where )
{
	int     model;
	FVECTOR pos;
	SVECTOR rot;

	pWork->proc = GCL_GetOptionValue( 'e' , 0 );          // procの読み込み
	model = GCL_GetOptionValue( 'm' , 0 );                // モデルNAMEの取得
	GM_InitObject( &pWork->body , model , BODY_FLAG );    // モデルの初期化
	GM_ConfigObjectLight( &pWork->body , pWork->lights ); // ライト設定
	GCL_GetOption( 'p' );                                 // 位置の取得
	// 位置情報の引きだし（格納）
	pos.vx = ( float )GCL_GetNextInt();	
	pos.vy = ( float )GCL_GetNextInt();	
	pos.vz = ( float )GCL_GetNextInt();
	GCL_GetOption( 'r' );                                 // 方向の取得
	// 方向情報の引きだし
	rot.vx = GCL_GetNextInt();
	rot.vy = GCL_GetNextInt();
	rot.vz = GCL_GetNextInt();
	// 位置,方向の設定
	DG_SetPos2( &pos , &rot );
	
	if ( !GM_IsOneID( where ) ){
		where = GM_GetMapIDfromPos( where , &pos ); // 場所から今自分のいる場所を割出す
	}
	GM_SetCurrentMap( where );    // カレントマップの設定
	GM_ActObject( &pWork->body ); // 配置
	DG_GetLightMatrix( &pos , pWork->lights );            // 位置からライトマトリックスの計算

	pWork->name = name;
	pWork->map = where;

	DG_COPY_VEC( &pWork->pos, &pos );  // 位置のコピー

//	pWork->list.body = &pWork->body;
	SetTarget( pWork );
	return 0;
}

void *NewTestBox( int name , int where )
{
	Work *pWork;
	
	pWork = ( Work * )GV_NewActor( GV_ACTOR_USER , sizeof( Work ) );
	if ( pWork != NULL ) {
		GV_SetActor( &pWork->actor , Act , Die );
		if ( GetResources( pWork , name , where ) != 0 ) {
			GV_DestroyActor( pWork );
			return NULL;
		}
	}
	return pWork;
}
