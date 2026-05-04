//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	f_bridge.c
	浮き橋オブジェクト

	2001/03/06 K.Takabe
	$Id: f_bridge.c,v 1.1.1.3 2002/11/19 11:51:14 Yoshizawa1 Exp $

*/
/*

chara 浮き橋[NewFlotingBridgeSet] $s:name \
	-pos $v:始点座標 $v:終点座標 \
	-n $w:配置個数 \
	-model $s:モデル名 \
	-flag $w:フラグ
mesg 浮き橋 $s:name 動的床強制更新禁止[0]
mesg 浮き橋 $s:name 動的床強制更新開始[1]
mesg 浮き橋 $s:name 表示状態[2] $b:ＯＮ・ＯＦＦ // ( 0:off 1:on )
// キャラ「プラント海面」が起動していなければならない。（w32a用）
// 指定した２点間に指定した個数の足場モデルを海面に浮かせる
// 浮き橋の上にＮＰＣなどを乗らせる場合には「動的床強制更新開始」
// メッセージを送らなければならない。動的床強制更新は処理が重たいので
// 必要がなくなったら「動的床強制更新禁止」メッセージを送る必要がある

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"



/* ---------------------------------------------------------------- */
	/*
		外部参照
	*/
extern int TAKABE_GetSeaSurfaceHeight( FVECTOR *res, FVECTOR *norm, int n_pos );
/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define _RND(n)	( ( (BP_PS2_rand()>>16) * n ) >> 15 )

/* ---------------------------------------------------------------- */
	/*
		プログラム使用定数の定義
	*/


/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
typedef struct {
	DG_OBJS		*objs ;
	FVECTOR		pos ;
	FMATRIX		base_world ;
	TARGET		trg ;
	HZX_D_FLOOR	*dynamic_floor ;
	int			damage_flag ;
	float		power ;
} ObjectWork ;

/* ---------------------------------------------------------------- */
//#define FLOOR_FLAG	(HZX_FLOOR_NO_BLOOD|HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_IK)
#define FLOOR_FLAG	(HZX_FLOOR_NO_BLOOD|HZX_FLOOR_NO_BULLETHOLE)
/* ---------------------------------------------------------------- */
	/*
		プログラムワーク
	*/
typedef	struct	{
	GV_ACT_EX	actor ;

	int			name ;
	int			map ;
	int			hzx_id ;
	int			n_models ;
	int			model_id ;
	int			flag ;
	float		parts_len ;
	FVECTOR		pos[2] ;
	FMATRIX		bound_world ;
	FVECTOR		bound[ 2 ];
	FVECTOR		base_pos[ 4 ] ;
	TARGET			target ;
	POWER_TARGET	power_target ;
	TARGET			*child_target ;
	FVECTOR			force ;

	ObjectWork	object[0] ;
} Work ;

enum {
	FLAG_UPDATE_FLOOR		= 0x0001,	/* ダイナミックフロア更新 */
	FLAG_INVISIBLE			= 0x0002,	/* モデル非表示 */
};

#define RND( _n )	( ( ( BP_PS2_rand() >> 16 ) * (_n) ) >> 15 )
#define RNDS( _n )	( ( ( ( BP_PS2_rand() >> 16 ) * (_n) ) >> 14 ) - ( _n ) )
/* ---------------------------------------------------------------- */
#if 0
static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		//if ( ( control->map & map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("%s: search faild control !!\n", __FILE__);
	return ( NULL );
}
#endif
/* ---------------------------------------------------------------- */
static void CalcFloorPos( Work *work, IVECTOR *hzx_pos, FMATRIX *world )
{
	FVECTOR	tmp_pos[4], *pos ;
	int		i ;

	DG_SetPos( world );
	DG_PutVector( work->base_pos, tmp_pos, 4 );
	pos = tmp_pos ;
	for ( i = 4 ; i > 0 ; i--, pos++, hzx_pos++ ){
		hzx_pos->vx = DG_FTOI( pos->vx );
		hzx_pos->vy = DG_FTOI( pos->vy );
		hzx_pos->vz = DG_FTOI( pos->vz );
	}
}
/* ---------------------------------------------------------------- */
#if 0
static void TargetCallback( TARGET *offence, TARGET *defence, ObjectWork *object )
{
	POWER_TARGET	*power ;
	power = offence->power ;
	switch ( power->type & POWER_CLASS ){
	  case POWER_EXPLODE:
		object->damage_flag |= 1 ;
		object->power += power->damage ;
		break ;
	  case POWER_ONCE:
	  case POWER_DECREASE:
	  case POWER_THRESHOLD:
	  case POWER_CONST:
	  default:
		object->damage_flag |= 1 ;
		object->power += power->damage ;
		break ;
	}
}
#endif
/* ---------------------------------------------------------------- */
	/*
		プログラムメイン処理
	*/
static void Act( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg, i ;
	FVECTOR		poss[ 32 ], norms[ 32 ];

	GM_CurrentMap = work->map ;

	/* メッセージチェック */
	if ( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){
			  case 0:/* 動的床更新禁止 */
				work->flag &= FLAG_UPDATE_FLOOR ;
				break ;
			  case 1:/* 動的床更新開始 */
				work->flag |= FLAG_UPDATE_FLOOR ;
				break ;
			  case 2:/* 表示ＯＮ・ＯＦＦ */
				if ( msg->message[ 1 ] )	work->flag &= ~FLAG_INVISIBLE ;
				else						work->flag |= FLAG_INVISIBLE ;
				break ;
			}
		}
	}

#if 0
	{/* 動的床のマップ切り替えチェック */
		CONTROL		*ctrl ;
		ctrl = SearchControl( 5584254 /*GV_StrCode("エマ")*/ );
		if ( ctrl == NULL || ( ctrl != NULL && ctrl->map != work->map ) ){
			/* エマが存在しないかマップが違う場合には全体バウンディングに応じて更新を制御する */
			//extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
			//NewBoundingBoxView_1( &work->bound_world, &work->bound[0], &work->bound[1], 0x80c06060 );
			if ( DG_BoundCheck( &work->bound_world, &work->bound[0], &work->bound[1] ) == -1 ) return ;
		}
	}
#else
	if ( !( work->flag & FLAG_UPDATE_FLOOR ) ){
		/* 更新許可がない場合には全体バウンディングに応じて更新を制御する */
		//extern void *NewBoundingBoxView_1( FMATRIX *world, float *max, float *min, int color );
		//NewBoundingBoxView_1( &work->bound_world, &work->bound[0], &work->bound[1], 0x80c06060 );
		if ( DG_BoundCheck( &work->bound_world, &work->bound[0], &work->bound[1] ) == -1 ) return ;
	}
#endif

	/* 海面位置計算チェック */
	for ( i = 0 ; i < work->n_models ; i++ ){
		poss[ i ] = *(FVECTOR*)&work->object[ i ].base_world.m[3][0] ;
		/* 表示・非表示チェック */
		if ( work->flag & FLAG_INVISIBLE ){
			work->object[ i ].objs->flag |= DG_FLAG_INVISIBLE ;
		} else {
			work->object[ i ].objs->flag &= ~DG_FLAG_INVISIBLE ;
		}
	}

   if ( !DG_Arm_SkipThisFrame() )
   {
	   TAKABE_GetSeaSurfaceHeight( poss, norms, work->n_models );

	   /* オブジェクトのマトリクス確定＆動的床更新 */
	   for ( i = 0 ; i < work->n_models ; i++ ){
		   ObjectWork	*object ;
		   FVECTOR		pos, norm, vec_x, vec_z ;
		   IVECTOR		hzx_pos[4] ;

		   object = &work->object[ i ];
		   /* マトリクス設定 */
		   pos = poss[ i ];
		   norm = norms[ i ];
		   object->objs->world.m[3][1] = pos.vy ;
		   _sceVu0OuterProduct( &vec_x, &norm, (FVECTOR*)&object->base_world.m[2][0] );
		   _sceVu0OuterProduct( &vec_z, &vec_x, &norm );
		   vec_x.vw = 0.0f ; norm.vw = 0.0f ; vec_z.vw = 0.0f ;
		   *(FVECTOR*)&object->objs->world.m[0][0] = vec_x ;
		   *(FVECTOR*)&object->objs->world.m[1][0] = norm ;
		   *(FVECTOR*)&object->objs->world.m[2][0] = vec_z ;

		   /* 動的床設定 */
		   CalcFloorPos( work, hzx_pos, &object->objs->world );
		   HZX_MoveDynamicFloor( object->dynamic_floor, &hzx_pos[0], &hzx_pos[1], &hzx_pos[2], &hzx_pos[3] );
		   //HZX_ViewDynamicFloor( object->dynamic_floor, 4 ) ;
	   }
   }

#if 0
	{/* あたり判定チェック */
		TARGET		*child ;
		ObjectWork	*object ;
		extern int TAKABE_AddWaveForce( FVECTOR *pos, float drop_force );

		for ( i = 0 ; i < work->n_models ; i++ ){
			object = &work->object[ i ];
			child = &work->child_target[ i ] ;
			GM_MoveTarget2( child, &object->objs->world );
			if ( object->damage_flag ){
				TAKABE_AddWaveForce( (FVECTOR*)object->base_world.m[3], 1000.0f );
				object->damage_flag = 0 ;
				object->power = 0 ;
			}
		}
	}
#endif
}

/* ---------------------------------------------------------------- */
	/*
		プログラム終了処理
	*/
static void Die( Work *work )
{
	int		i ;
#if 0
	GM_FreeTarget( &work->target );
	//GM_TargetDisconnectChild( &work->target, work->child_target );
	GV_Free( work->child_target );
#endif
	for ( i = 0 ; i < work->n_models ; i++ ){
		ObjectWork	*object ;
		object = &work->object[ i ];
		DG_DequeueObjs( object->objs );
		DG_FreeObjs( object->objs );
		HZX_RemoveDynamicFloor( object->dynamic_floor );
	}
}

/* ---------------------------------------------------------------- */
	/*
		ワーク初期化処理
	*/
static int GetResources( Work *work, int name, int where )
{
	DG_DEF	*def ;
	DG_OBJS	*objs ;
	int		i ;
	FVECTOR	vec ;
	float	dir ;
	FMATRIX	mat ;
	float	len ;

	work->name = name ;
	work->map = where ;
	GM_CurrentMap = where ;
	work->hzx_id = GM_GetHzxGroupID( where );

	/* テクスチャ設定読み込み */
	if ( GCL_GetOption( 'm' ) != NULL ) {
		work->model_id = GCL_GetNextInt() ;
		/* モデルデータ取得 */
		def = GV_GetCache( GV_CacheID( work->model_id, 'k' ) ) ;
		if ( def == NULL ){
			printf("%s:model not found!!(%d)\n", __FILE__, work->model_id );
			return ( -1 );
		}
	} else {
		return ( -1 );
	}

	/* 設置座標読み込み */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		work->pos[0].vx = (float)GCL_GetNextInt() ;
		work->pos[0].vy = (float)GCL_GetNextInt() ;
		work->pos[0].vz = (float)GCL_GetNextInt() ;
		work->pos[1].vx = (float)GCL_GetNextInt() ;
		work->pos[1].vy = (float)GCL_GetNextInt() ;
		work->pos[1].vz = (float)GCL_GetNextInt() ;
		work->pos[0].vw = work->pos[1].vw = 1.0f ;
	} else {
		return ( -1 );
	}
	_sceVu0SubVector( &vec, &work->pos[1], &work->pos[0] );
	dir = atan2f( vec.vz, vec.vx );
	len = DG_SQRT( vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz );
	_sceVu0UnitMatrix( &mat );
	_sceVu0RotMatrixY( &mat, &mat, -dir );
	*(FVECTOR*)&mat.m[3][0] = work->pos[0] ;
	work->parts_len = len / work->n_models ;

	/* 制御フラグの読み込み */
	if ( GCL_GetOption( 'f' ) != NULL ){
		work->flag = GCL_GetNextInt() ;
	}

	{/* あたり判定用基準座標生成 */
		static FVECTOR	base_pos[4] = {
			{ -2250.0f-250, 500.0f, -500.0f },
			{  2250.0f+250, 500.0f, -500.0f },
			{  2250.0f+250, 500.0f,  500.0f },
			{ -2250.0f-250, 500.0f,  500.0f },
		};
		for ( i = 0 ; i < 4 ; i++ ) work->base_pos[ i ] = base_pos[ i ] ;
		base_pos[0].vx = - work->parts_len * 0.5f - 250.0f ;
		base_pos[1].vx =   work->parts_len * 0.5f + 250.0f ;
		base_pos[2].vx =   work->parts_len * 0.5f + 250.0f ;
		base_pos[3].vx = - work->parts_len * 0.5f - 250.0f ;
	}

	/* オブジェクトの初期化 */
	for ( i = 0 ; i < work->n_models ; i++ ){
		ObjectWork	*object ;
		IVECTOR		hzx_pos[4] ;
		
		object = &work->object[ i ];
		/* モデル初期化、登録、マップ設定 */
		object->objs = objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
		if ( objs == NULL ) return ( -1 );
		DG_QueueObjs( objs );
	    GM_GroupObjs( objs, GM_CurrentMap ) ;
		/* 表示位置ベースマトリクス設定 */
		object->base_world = mat ;
		object->base_world.m[3][0] += vec.vx * ( i * 2 + 1 ) / ( work->n_models * 2 ) ;
		object->base_world.m[3][2] += vec.vz * ( i * 2 + 1 ) / ( work->n_models * 2 ) ;
		objs->world = object->base_world ;

		CalcFloorPos( work, hzx_pos, &objs->world );
		object->dynamic_floor = HZX_AddDynamicFloor( work->hzx_id,
													&hzx_pos[0], &hzx_pos[1], &hzx_pos[2], &hzx_pos[3], 
													4, FLOOR_FLAG );
	}

	/* 全体バウンディングの設定 */
	work->bound_world = mat ;
	work->bound[0].vx = -1000.0f ;
	work->bound[0].vy = -3000.0f ;
	work->bound[0].vz = -750.0f ;
	work->bound[1].vx = 1000.0f + len ;
	work->bound[1].vy = 3000.0f ;
	work->bound[1].vz = 750.0f ;
	work->bound[0].vw = work->bound[1].vw = 1.0f ;

#if 0
	{/* ターゲット関連初期化 */
		FVECTOR		size, offset ;
		int			class ;

		class = TARGET_POWER|TARGET_CHILD_ALWAYS|TARGET_DEFENSE|TARGET_ROTATE ;
		size.vx = ( work->bound[1].vx - work->bound[0].vx ) * 0.5f ;
		size.vy = ( work->bound[1].vy - work->bound[0].vy ) * 0.5f ;
		size.vz = ( work->bound[1].vz - work->bound[0].vz ) * 0.5f ;
		GM_SetTarget( &work->target, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
		GM_SetPowerTarget( &work->target, &work->power_target, POWER_CONST, 1, 0, 0, &work->force );
		GM_MoveTarget2( &work->target, &work->bound_world );
		GM_PutTarget( &work->target );

		/* 子ターゲットの登録 */
		work->child_target = GV_Malloc( sizeof(TARGET) * work->n_models ) ;
		size.vx = work->parts_len * 0.5f + 250.0f ;
		size.vy = 500.0f ;
		size.vz = 500.0f ;
		for ( i = 0 ; i < work->n_models ; i++ ){
			ObjectWork	*object ;
			TARGET		*child ;
			object = &work->object[ i ];
			child = &work->child_target[ i ] ;
			GM_SetTarget( child, class, work->map, ENEMY_SIDE, &size, &DG_ZeroVector );
			GM_SetPowerTarget( child, &work->power_target, POWER_CONST, 1, 0, 0, &work->force );
			GM_MoveTarget2( child, &object->objs->world );
			GM_SetTargetCallBack( child, (TARGET_CALLBACK)TargetCallback, object );
		}
		GM_TargetConnectChild( &work->target, work->child_target, work->n_models, 0 );
	}
#endif

	return (0);
}


/* ---------------------------------------------------------------- */
	/*
		シナリオ起動インターフェイス
	*/
void *NewFlotingBridgeSet( int name, int where )
{
	Work		*work ;
	int			n ;

	OPERATOR() ;

	if ( GCL_GetOption( 'n' ) != NULL ) {
		n = GCL_GetNextInt() ;
	} else {
		return ( NULL );
	}

	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) + sizeof(ObjectWork) * n ) ;
	work->n_models = n ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

