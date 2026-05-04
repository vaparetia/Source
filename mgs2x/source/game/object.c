//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	object.c
	高レベルオブジェクト管理ルーチン

	1999/07/07 K.Takabe
	$Id: object.c,v 1.1.1.3 2002/11/19 11:41:54 Yoshizawa1 Exp $

*/
/*
	オブジェクト管理ルーチン

	------------------------------------------------

	void GM_InitObject( OBJECT *object, int model, int flag )
	OBJECT *object ;
	int model ;		モデル名
	int flag ;		処理フラグ
	int motion ;	モーション名

		初期化処理
		・光源は主光源のみ
		・マップは現在の GM_CurrentMap
		・モデル初期化する


	void GM_ActObject( OBJECT *object )
	OBJECT *object ;

		表示処理
		・定数回転マトリクス位置に配置
		・マップは現在の GM_CurrentMap に
		・モーションがあればモーション処理
		・下のGM_ActMotion()とGM_ActObject2()を連続呼びしたものと同じ
		　（モーションと移動量が１フレームずれてしまう）


	void GM_ActMotion( OBJECT *object )
	OBJECT *object ;

		モーション再生処理
		・絶対回転クォータニオン計算
		・移動量算出
		・GM_ActControl()の前に実行するとよい


	void GM_ActObject2( OBJECT *object )
	OBJECT *object ;

		表示処理
		・定数回転マトリクス位置に配置
		・マップは現在の GM_CurrentMap に
		・計算済み絶対回転クォータニオンから関節の表示マトリクス設定
		・GM_ActControl()の後に実行するとよい


	void GM_FreeObject( OBJECT *object )
	OBJECT  *object ;

		終了処理

	------------------------------------------------

	void GM_ConfigObjectFlags( OBJECT *object, int flag )
	OBJECT  *object ;
	int flag ;		処理フラグ

		処理フラグを設定


	void GM_ConfigObjectLight( OBJECT *object, FMATRIX *light )
	OBJECT  *object ;
	FMATRIX	*light ;	光源マトリクス FMATRIX×2

		モデルに光源マトリクスを設定する


	void GM_ConfigObjectRoot( OBJECT *object, OBJECT *org, int unit )
	OBJECT  *object ;
	OBJECT	*org ;		親モデル
	int		unit ;		関節番号

		他のモデルの関節に付随させる


	void GM_ConfigObjectStep( OBJECT *object, FVECTOR *step )
	OBJECT  *object ;
	FVECTOR *step ;		移動量格納アドレス

		モーションによる移動量を格納する場合
		格納アドレスを設定する
		但し、step->vyは変化なし、変わりにstep->vwにＹの移動量が入る


	void GM_ConfigObjectJoint( OBJECT *object, FVECTOR *rots )
	OBJECT  *object ;
	FVECTOR *rots ;		関節を制御する回転ベクトル配列

		回転関節を使用する


	void GM_ConfigObjectSlide( OBJECT *object )
	OBJECT  *object ;

		横すべり関節を使用する（object->slide で制御）


	void GM_ConfigObjectEvm( OBJECT *object, int model, int flag )
	OBJECT  *object ;
	int model ;			モデル名
	int flag ;			処理フラグ

		マルチウェイトエンベロープモデルを関連付けさせる
		（通常モデルのモーションに同期して動作する）

	------------------------------------------------

	void GM_ConfigObjectMotion( OBJECT *object, int n_layer, int motion, int flag )
	OBJECT  *object ;
	int n_layer ;		同時に制御するモーションレイヤー数
	int motion ;		モーションファイル名
	int	flag ;			モーション処理フラグ

		モーションを割り当てる
		・モーション再生を行うためにはモデルの処理フラグにDG_FLAG_FINISHCALCを設定する


	void GM_ConfigObjectAction( OBJECT *object, int n_layer, int action, int time, u_long64 mask, int interp )
	OBJECT  *object ;
	int n_layer ;		設定するモーションレイヤー番号
	int action ;		モーション番号
	int time ;			モーション再生開始時間
	u_long64 mask ;		再生マスク（下位ビットから関節０、関節１・・・となる。各ビットが１で有効）
	int interp ;		補間時間

		モーションを再生させる
		・時間指定は１／３００単位なので注意


	------------------------------------------------
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"
#include	"libhzx.h"
#include	"g_struct.h"
#include	"g_extern.h"
#include	"g_macro.h"
#ifdef KP_XBOX
#include	"g_define.h"
#endif

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/




/*----------------------------------------------------------------*/
void 	GM_InitObject( OBJECT *object, int model, int flag )
{
	DG_DEF *def ;

	/*
		model == 0 は禁止
	*/
	ASSERT( model != 0 ) ;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( model, 'k' ) );
#ifdef DEBUG_MODE
	if ( def == NULL ) {
	    printf( "model %d not found\n", model ) ;
	    ASSERT( 0 ) ;	    
	}
#endif
	object->objs = DG_MakeObjs( def, flag, 0 );
	if ( object->objs == NULL ) {
#ifdef DEBUG_MODE
	    printf( "warning : model %x make objs failed\n", model ) ;
#endif
		return ;
	}
	DG_QueueObjs( object->objs );
	object->flag = 0 ;
	object->map_name = 0x7fffffff ;
}

void GM_ActObject( OBJECT *object )
{
	/*
		定数回転マトリクス位置に配置
	*/
	DG_PutObjs( object->objs );

	/*
		マップ変更処理
	*/
	if ( object->map_name != GM_CurrentMap ) {
	    object->map_name = GM_CurrentMap ;
		/* シングルモデルは必ず要るのでＮＵＬＬチェックなし */
	    GM_GroupObjs( object->objs, GM_CurrentMap ) ;
		/* マルチはＮＵＬＬチェックが要る */
		if ( object->evmobj != NULL ){
			GM_GroupObject( object->evmobj, GM_CurrentMap ) ;
		}
	}

	/*
		モーション処理
	*/
	if ( object->m_ctrl != NULL ){
		DG_SetScratchStack( SCRPAD_ADDR + 16*1024-16 );
		MT_ActMotion( object->m_ctrl, object->objs, object->evmobj );
		DG_ResetScratchStack();
		MT_ActMotion2( object->m_ctrl, object->objs );
		object->height = object->m_ctrl->height ;
		if ( object->step != NULL && !(object->flag & OBJECT_MOTIONSTEP_THROUGH) ){
		    object->step->vx = object->m_ctrl->step.vx ;
		    object->step->vz = object->m_ctrl->step.vz ;
		    object->step->vw = object->m_ctrl->step.vw ;
		}
		if ( object->evmobj != NULL )
      {

			/* 中心座標の設定 */
			DG_GetPos( &object->evmobj->world );
			/* マトリクスダブルバッファの切り替え */
         DG_Arm_SwitchEvmBuffer(object->evmobj);
			/* 関節型オブジェクト用モーションを割り当てる */
			MT_EvmActMotion( object->m_ctrl, object->evmobj );
		}
	}
}

void GM_ActMotion( OBJECT *object )
{
	/*
		モーション処理
	*/
	if ( object->m_ctrl != NULL ){
		DG_SetScratchStack( SCRPAD_ADDR + 16*1024-16 );
		MT_ActMotion( object->m_ctrl, object->objs, object->evmobj );
		DG_ResetScratchStack();
		object->height = object->m_ctrl->height ;
		if ( object->step != NULL && !(object->flag & OBJECT_MOTIONSTEP_THROUGH) ){
		    object->step->vx += object->m_ctrl->step.vx ;
		    object->step->vz += object->m_ctrl->step.vz ;
		    object->step->vw += object->m_ctrl->step.vw ;
		}
	}
}

void GM_ActObject2( OBJECT *object )
{
	/*
		定数回転マトリクス位置に配置
	*/
	DG_PutObjs( object->objs );

	/*
		マップ変更処理
	*/
	if ( object->map_name != GM_CurrentMap ) {
	    object->map_name = GM_CurrentMap ;
		/* シングルモデルは必ず要るのでＮＵＬＬチェックなし */
	    GM_GroupObjs( object->objs, GM_CurrentMap ) ;
		/* マルチはＮＵＬＬチェックが要る */
		if ( object->evmobj != NULL ){
			GM_GroupObject( object->evmobj, GM_CurrentMap ) ;
		}
	}

	/*
		モーション処理
	*/
	if ( object->m_ctrl != NULL ){
		MT_ActMotion2( object->m_ctrl, object->objs );
		if ( object->step != NULL ){
			object->step->vx = 0 ;
			object->step->vz = 0 ;
			object->step->vw = 0 ;
		}
		if ( object->evmobj != NULL ){
			/* 中心座標の設定 */
			DG_GetPos( &object->evmobj->world );
			/* マトリクスダブルバッファの切り替え */
         DG_Arm_SwitchEvmBuffer(object->evmobj);
			/* 関節型オブジェクト用モーションを割り当てる */
			MT_EvmActMotion( object->m_ctrl, object->evmobj );
		}
	}
#ifdef KP_XBOX
	GM_CurrentAddr = GM_INVALID_ADDR;
#endif			
}

void GM_FreeObject( OBJECT *object )
{
	if ( object->m_ctrl != NULL ){
		MT_FreeMotion( object->m_ctrl );
		object->m_ctrl = NULL ;
	}
	if ( object->objs != NULL ){
		DG_DequeueObjs( object->objs ) ;
		DG_FreeObjs( object->objs );
		object->objs = NULL ;
	}
	if ( object->evmobj != NULL ){
		DG_DequeueEvmObj( object->evmobj );
		DG_FreeEvmObj( object->evmobj );
		object->evmobj = NULL ;
	}
} 

/*----------------------------------------------------------------*/

void GM_ConfigObjectFlags( OBJECT *object, int flag )
{
	DG_OBJS		*objs ;
	DG_OBJ		*obj ;
	int			i ;

	objs = object->objs ;
	ASSERT( objs != NULL ) ;
	objs->flag = flag ;
	obj = objs->objs ;
	for ( i = objs->n_models ; i > 0 ; i-- ){
		obj->flag = flag ;
		obj++ ;
	}
}

void GM_ConfigObjectLight( OBJECT *object, FMATRIX *light )
{
	DG_OBJS		*objs ;

	objs = object->objs ;
	ASSERT( objs != NULL ) ;
	DG_SetLightMatrix( objs, light );
	if ( object->evmobj != NULL ){
		object->evmobj->light = light ;
	}
}

void GM_ConfigObjectStep( OBJECT *object, FVECTOR *step )
{
	object->step = step ;
}

void GM_ConfigObjectRoot( OBJECT *object, OBJECT *org, int unit )
{
	DG_OBJS		*objs1, *objs0 ;

	objs1 = object->objs ;
	ASSERT( objs1 != NULL ) ;
	if ( org == NULL || unit < 0 ) {
		objs1->root = NULL ;
		DG_SetLightMatrix( objs1, NULL );
	} else {
		objs0 = org->objs ;
		ASSERT( objs0 != NULL ) ;
		objs1->root = &( objs0->objs[ unit ].world ) ;
		DG_SetLightMatrix( objs1, objs0->light );
	}
}

void GM_ConfigObjectJoint( OBJECT *object, FVECTOR *rots )
{
	DG_OBJS		*objs ;

	objs = object->objs ;
	ASSERT( objs != NULL ) ;
	objs->rots = rots ;
}

void GM_ConfigObjectSlide( OBJECT *object )
{
	DG_OBJS		*objs ;

	objs = object->objs ;
	ASSERT( objs != NULL ) ;
	objs->movs = &object->slide ;
}

void GM_ConfigObjectEvm( OBJECT *object, int model, int flag )
{
	EVM_DEF		*def ;
	DG_EVMOBJ	*evmobj ;

	if ( ( def = GV_GetCache( GV_CacheID( model, 'e' ) ) ) == NULL ) ASSERT( 0 ) ;
	object->evmobj = evmobj = DG_MakeEvmObj( def, flag, 0 );
	if ( evmobj != NULL ) DG_QueueEvmObj( evmobj );
}

/*----------------------------------------------------------------*/

void GM_ConfigObjectMotion( OBJECT *object, int n_layer, int motion, int flag )
{
	if ( object->m_ctrl != NULL ){
		MT_FreeMotion( object->m_ctrl );
	}
	if ( n_layer <= 0 ) n_layer = 1 ;
	object->m_ctrl = MT_InitMotion( object->objs, n_layer, motion, flag );
	ASSERT( object->m_ctrl != NULL );
}

void GM_ConfigObjectAction( OBJECT *object, int n_layer, int action, int time, u_long64 mask, int interp )
{
	/*
		モーションが設定されていなければ何もしない
	*/
	if ( object->m_ctrl == NULL ) return ;
	/*
		アクション変更
	*/
	if ( n_layer == 0 ) mask = I64(0xffffffffffffffff) ;
	if ( action >= 0 ){
		MT_SetMotionData( object->m_ctrl, n_layer, action, time, mask );
	} else {
		MT_ResetMotionData( object->m_ctrl, n_layer );
	}
	/*
		補間の設定
	*/
	if ( interp != 0 ){
		MT_SetMotionInterp( object->m_ctrl, interp, mask );
	}
}


/*----------------------------------------------------------------*/

