//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	複数赤外線センサー  main.c
	2001/03/14 S.Kobayashi
	$Id: newirs.c,v 1.1.1.3 2002/11/19 11:50:25 Yoshizawa1 Exp $
*/
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"
#include    "../../kano/attachment/attachment_called.h"
#include    "newirs.h"
//#include    "Senser.h"
#include    "../test/etc.h"

typedef struct _work {
	GV_ACT_EX       actor;

	OBJECT           body;  // 通常モデル
	OBJECT      BreakBody;  // 壊れモデル
	FMATRIX   lights[ 2 ];
	FVECTOR           pos;
	SVECTOR           rot;
	TARGET         target;
	POWER_TARGET	power;
	CONTROL         *ctrl;  // くっつけ用
	int              name;
	int         attach_id;
	int        attach_put; // くっつけたい間接番号
	int        attach_flg;
	int       attach_name;
	int           hit_flg;
	int               map;

    int        cold_count;
	int   model_name[ 3 ];

	u_long64    my_bit_name; // 最大64個登録可能 scnからどれが破壊されたかしらべるときのID
    int              proc;
    int    ( *act )( struct _work * );
} Work;

typedef struct {
	int *put_name;
	int *point_name;
	int *prt_id;
	int num;
} Attach;

enum {
	SK_PARENT_CALL = 0x02 ,
	SK_SENSER_DIE  = 0x04 ,
	SK_ATTACH_DIE  = 0x08 , 
	SK_ALL_DIE     = 0x10 , 
	SK_SINGLE_DIE  = 0x20 , 
	SK_PARENT_CALL_CHILD_NAME = 0x40 ,
	SK_PROC_FREE   = 0x80 ,
};

// プロトタイプ
extern void *NewIRS_Box_Break_Effect(FVECTOR  *, SVECTOR  *);
extern void  *NewSpark1(int n_packets, FVECTOR *center, float min_speed, 
                       float speed_wide, float gravity, SVECTOR *rot, 
		       SVECTOR *rot_wide, FVECTOR *color, float length, int count );
extern void *NewIrs_Lamp( CONTROL * , FVECTOR * , SVECTOR * , int , int , int );

static void PutObj( Work * , int , int ); // くっつけ作業

static void IrsCheck( Work *pWork ) // 壊れていたら総数から引く
{
	if( pWork->hit_flg == 1 ){
	  	pWork->hit_flg = 2;
		// 親に送信
		GV_CallParentSignalFunc( pWork , SK_PARENT_CALL , pWork->my_bit_name );
		GV_CallParentSignalFunc( pWork , SK_PARENT_CALL_CHILD_NAME , pWork->name );
		// proc execute
		if ( pWork->proc != 0 ){
			GM_ExecProc( pWork->proc , NULL );
		}
		GV_CallParentSignalFunc( pWork , SK_PROC_FREE , 0 );
	} 
}

static int CtrlCheck( Work *pWork , int name )
{
	CONTROL *ctrl;
	int i;

	for ( i = 0 ; i < GM_N_WhereList ; i++ ) {
		ctrl = GM_WhereList[ i ];
		if ( ctrl->name == name ){
			return ( 1 );
		}
	}
	KillAttachment_called( pWork->attach_id );
//	SK_Err("せーふてぃー\0");
	GV_DestroyActor( pWork );
	return ( -1 );
}

static int NameSearch( Work *pWork , int name )
{
	if ( pWork->attach_flg == ATTACH_ON && pWork->attach_name == name ){
		KillAttachment_called( pWork->attach_id );
		return ( 0 );
	}
	return ( -1 );
}

static void BreakBOX_Act( Work *pWork )
{
}

static int ReceiveSignal2( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;
	switch ( signal ){
	case SK_ATTACH_DIE : // 装置破壊
		if ( NameSearch( pWork , value ) < 0 ){
			return ( 0 );
		}
		pWork->act  = (void *)BreakBOX_Act;
//		NewIRS_Box_Break_Effect( &pWork->pos , &pWork->rot);
		GV_CallChildSignalFunc( pWork , 0x04 , 0 );
		pWork->hit_flg = 1;
		pWork->attach_flg = ATTACH_FREE;
		IrsCheck( pWork );
		GV_DestroyActor( pWork );
		break;
	case SK_SINGLE_DIE : // 指名殺し
		if ( pWork->name != value ){
			break;
		}
	case SK_ALL_DIE : // 装置破壊壊れモデルにかえる
		if ( pWork->hit_flg == 0 ){
			pWork->hit_flg = 1;
			pWork->act  = (void *)BreakBOX_Act;
			// 壊れ時のエフェクト発動 前のをそのまま流用
			// スクラッチパット使用 
			//			NewIRS_Box_Break_Effect( &pWork->pos , &pWork->rot);
			GV_CallChildSignalFunc( pWork , 0x04 , 0 );
			// 通常モデルの登録を破棄してフリーする
			if ( pWork->attach_flg == ATTACH_ON ){
				KillAttachment_called( pWork->attach_id );
			} else {
				GM_FreeObject( &pWork->body );
			}
			/* 壊れオブジェクトを表示 */
			/* 壊れモデル表示 */
			/* 大きさは変わらないので、前回のターゲットをそのまま残す */
			if ( pWork->attach_flg == ATTACH_ON ){
				GM_SetCurrentMap( pWork->map );    // カレントマップの設定
				PutObj( pWork , pWork->model_name[ ModelBreakBox ] , 0 ); // くっつけ作業単体
			} else {
				DG_SetPos2( &pWork->pos , &pWork->rot );
				GM_ActObject( &pWork->BreakBody ); // 配置
				DG_GetLightMatrix( &pWork->pos , pWork->lights );
				DG_VisibleObjs( pWork->BreakBody.objs );
//				DG_InvisibleObjs( pWork->body.objs );
			}
		}
		break;
	case GV_SIGNAL_KILL :
		((GV_ACT *)work)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( work );
		return ( -1 );
	}
	return ( 0 );
}

static void PutObj( Work *pWork , int model , int onoff ) // くっつけ作業
{
	CONTROL *ctrl;

	if ( pWork->attach_flg == ATTACH_ON ){
		ctrl = pWork->ctrl;
		pWork->attach_id = MakeAttachment3A_called( model , &pWork->rot , ctrl->object , pWork->attach_put , &pWork->pos , 0);
		TurnOnOff_IR_called( pWork->attach_id , onoff ); // ir_on
		pWork->map = pWork->ctrl->map;
	}
}

static void FreePutObj( Work *pWork ) // くっつけた奴を解放
{
	if ( pWork->attach_flg == ATTACH_ON ){
		KillAttachment_called( pWork->attach_id );
	}
}
	// テンプライト色決定 
	/*	vx = pObj->box.color.vx / 10;
	vy = pObj->box.color.vy / 10;
	vz = pObj->box.color.vz / 10;
	if ( pObj->attach_flg == ATTACH_ON ){
		world = pObj->box.prim->world;
		pos = pObj->box.pos;
		vu0_Ldv0( &pos );
		vu0_Ldm0( &world );
		vu0_Mulv0m0v0();
		pos.vx = world.m[ 3 ][ 0 ] + pos.vx;
		pos.vy = world.m[ 3 ][ 1 ] + pos.vy;
		pos.vz = world.m[ 3 ][ 2 ] + pos.vz;
	} else {
		pos = pObj->box.pos;
	}
	DG_SetTmpLight2(
			&pos,
			400.0f,
			800.0f,
			vx | vy << 8 | vz << 16,
			LIT_FLAG_CHARAONLY|LIT_FLAG_BGONLY ) ;

// メッセージを受けとると殺す敵が死んだ時に利用
static int MsgDie( Work *pWork )
{
	GV_MSG		*msg;
	int      msg_num;
	int          num;
	int         name;
	int            i;

	i = 0;
	msg_num = GV_ReceiveMessage( pWork->name , &msg );
	msg += msg_num - 1;
	while( --msg_num >= 0 ){
		num = msg->message[ 0 ];
		if( num == SAIFA_DIE_FLG ){
			name = msg->message[ 1 ];
			if ( (name = NameSearch( pWork , name )) < 0 ){
				msg--;
				continue;
			}
			pWork->act  = (void *)BreakBOX_Act;
//			NewIRS_Box_Break_Effect( &pWork->pos , &pWork->rot);
			GV_CallChildSignalFunc( pWork , 0x04 , 0 );
			pWork->hit_flg = 1;
			pWork->attach_flg = ATTACH_FREE;
			IrsCheck( pWork );
			return ( -1 );
		}
		msg--;
	}
	return ( 0 );
}
*/
static void BoxTargetCallBack( TARGET *off, TARGET *def, void *pbuf )
{
	Work *pWork;
	FVECTOR fvtmp;

	pWork = ( Work * )pbuf;

	if( pWork->hit_flg == 0 ){
		if( def->damaged & TARGET_POWER ){ 
			if ( def->weapon_type & WP_BULLET ){
				pWork->hit_flg = 1;
				pWork->act  = (void *)BreakBOX_Act;
				// 壊れ時のエフェクト発動 前のをそのまま流用
				// スクラッチパット使用 
				NewIRS_Box_Break_Effect( &pWork->pos , &pWork->rot);
				GV_CallChildSignalFunc( pWork , 0x02 , 0 );
				// 通常モデルの登録を破棄してフリーする
				if ( pWork->attach_flg == ATTACH_ON ){
					KillAttachment_called( pWork->attach_id );
				} else {
					GM_FreeObject( &pWork->body );
				}
				/* 壊れオブジェクトを表示 */
				/* 壊れモデル表示 */
				/* 大きさは変わらないので、前回のターゲットをそのまま残す */
				if ( pWork->attach_flg == ATTACH_ON ){
					GM_SetCurrentMap( pWork->map );    // カレントマップの設定
					PutObj( pWork , pWork->model_name[ ModelBreakBox ] , 0 ); // くっつけ作業単体
					DG_SetPos2( &pWork->ctrl->mov , &pWork->ctrl->rot );
					DG_RotVector( &pWork->pos , &fvtmp , 1 );
					_sceVu0AddVector( &fvtmp , &fvtmp , &pWork->ctrl->mov );
				} else {
					GM_SetCurrentMap( pWork->map );    // カレントマップの設定
					DG_SetPos2( &pWork->pos , &pWork->rot );
					GM_ActObject( &pWork->BreakBody ); // 配置
					DG_GetLightMatrix( &pWork->pos , pWork->lights );
					DG_VisibleObjs( pWork->BreakBody.objs );
	//				DG_InvisibleObjs( pWork->body.objs );
					DG_COPY_VEC( &fvtmp , &pWork->pos );
				}
				// sound
				GM_SeSetMode( SD_E_ECODBR01 , &fvtmp , GM_SEMODE_BOMB );
			}
#if 0
				 else if ( def->weapon_type & WP_COLDSPRAY ) {
				if ( pWork->cold_count++ >= COLD_TIME ) { // 機能停止
					pWork->hit_flg = 1;
					GV_CallChildSignalFunc( pWork , 0x02 , 0 );
				}
			}
#endif

		}
	}
}

/* スイッチのターゲットを用意する */
static void InitBoxTarget( Work *pWork )
{
	TARGET   *pTrg;
	FVECTOR  size;
	FVECTOR  pos;
	FMATRIX		world ;

	pTrg = &pWork->target;
	// ターゲットのサイズを決定
	size.vx = 124.f;
	size.vy = 190.f;
	size.vz = 100.f;
	size.vw = 1.0f;

	// ターゲットの位置
	pos.vx = 0.0F;
	pos.vy = size.vy;
	pos.vz = 0.0F;

	if ( pWork->attach_flg == ATTACH_ON ){
		DG_SetPos( &pWork->ctrl->object->objs->objs[ pWork->attach_put ].world ) ;
		DG_MovePos( &pWork->pos ) ;
		DG_RotatePos( &pWork->rot ) ;
		DG_GetPos( &world ) ;
	} else {
		DG_SetPos2( &pWork->pos , &pWork->rot );
		DG_GetPos( &world ) ;
	}

	// ターゲット配置
	GM_SetTarget(pTrg, TARGET_DEFENSE | TARGET_ROTATE | TARGET_POWER | TARGET_SEEK, 
				 pWork->map , ENEMY_SIDE , &size, &pos);
	GM_MoveTarget2(pTrg , &world) ;
	GM_SetPowerTarget(pTrg , &pWork->power , POWER_ONCE , 1 , 0 , 0 , &DG_ZeroVector);
	GM_PutTarget(pTrg);
	GM_SetTargetCallBack(pTrg , BoxTargetCallBack , pWork );
}

//  普通時の動作
static int NORMALBOX_Act( Work *pWork )
{
	FMATRIX fmt;

	// ターゲットの移動
	if ( pWork->attach_flg == ATTACH_ON ){
#if 0
		fmt = pWork->ctrl->object->objs->objs[ pWork->attach_put ].world;
		WorldPos( &pos , &fmt , &pWork->pos );
		GM_MoveTarget( &pWork->target , &pos );
#else	/* 園山修正 */
		DG_SetPos( &pWork->ctrl->object->objs->objs[ pWork->attach_put ].world ) ;
		DG_MovePos( &pWork->pos ) ;
		DG_RotatePos( &pWork->rot ) ;
		DG_GetPos( &fmt ) ;
		GM_MoveTarget2( &pWork->target, &fmt ) ;
#endif
	} else {
		DG_GetLightMatrix( &pWork->pos , pWork->lights );            // 位置からライトマトリックスの計算
	}

	return 0 ;
}

static void InitPut( Work *pWork , Attach *pAttach ) // キャラクターにセンサーをつける時
{
	CONTROL *ctrl;
	int i , j;

	for ( i = 0 ; i < pAttach->num ; i++ ){
		for ( j = 0 ; j < GM_N_WhereList ; j++ ) {
			ctrl = GM_WhereList[ j ];
			// 操作対象のオブジェクトを探す
			if ( ctrl->name == pAttach->put_name[ i ] ) {
				if ( pWork->name == pAttach->point_name[ i ] ){
					pWork->ctrl = ctrl; // copy
					pWork->attach_name = ctrl->name;
					pWork->attach_put = pAttach->prt_id[ i ];
					pWork->attach_flg = ATTACH_ON;
					PutObj( pWork , pWork->model_name[ ModelBox ] , 1 );
					return;
				}
			}
		}
	}
	pWork->attach_flg = ATTACH_OFF;
}

static int InitBox( Work *pWork  , FVECTOR *pPos , SVECTOR *pRot , int *model_name , Attach *pAttach , int name , int where)
{
	pWork->pos = *pPos;	
	pWork->rot = *pRot;
	pWork->name = name;          // NAMEの取得
	pWork->hit_flg = 0;
	pWork->ctrl = NULL;
	pWork->attach_flg = ATTACH_OFF;
	pWork->model_name[ 0 ] = model_name[ 0 ];
	pWork->model_name[ 1 ] = model_name[ 1 ];
	pWork->model_name[ 2 ] = model_name[ 2 ];
	pWork->cold_count = 0;

	InitPut( pWork , pAttach ); // くっつきモデルか

	// 位置,方向の設定
	if ( pWork->attach_flg == ATTACH_ON ) { // くっつける場合は登録しない
	    GM_SetCurrentMap( pWork->map ); // 表示設定
		GV_SetActorChild( pWork , NewIrs_Lamp( pWork->ctrl , pPos , &pWork->rot , pWork->attach_flg , pWork->attach_put , where ) );
		InitBoxTarget( pWork );
		pWork->act  = (void *)NORMALBOX_Act;
		return ( 0 );
	}
	GM_InitObject( &pWork->body , model_name[ ModelBox ] , BODY_FLAG );           // モデルの初期化
	GM_ConfigObjectLight( &pWork->body , pWork->lights );          // ライト設定
	DG_VisibleObjs( pWork->body.objs );

	GM_InitObject( &pWork->BreakBody , model_name[ ModelBreakBox ] , BODY_FLAG2 ); // モデルの初期化
	GM_ConfigObjectLight( &pWork->BreakBody , pWork->lights );     // ライト設定
	DG_InvisibleObjs( pWork->BreakBody.objs );

	DG_SetPos2( &pWork->pos , &pWork->rot );
	if ( !GM_IsOneID( where ) ){
		where = GM_GetMapIDfromPos( where , &pWork->pos ); // 場所から今自分のいる場所を割出す
	}
	GM_SetCurrentMap( where );    // カレントマップの設定
	GM_ActObject( &pWork->body ); // 配置

	DG_GetLightMatrix( &pWork->pos , pWork->lights );            // 位置からライトマトリックスの計算
	pWork->map = where;
	pWork->act  = (void *)NORMALBOX_Act;
	GV_SetActorChild( pWork , NewIrs_Lamp( pWork->ctrl , pPos , &pWork->rot , pWork->attach_flg , 0 , where ) );

	InitBoxTarget( pWork );
	
	return (0);
}

static int GetResources( Work *pWork , FVECTOR *pPos , SVECTOR *pRot , int *model_name , Attach *pAttach , int name , int where )
{
	// シグナルの登録
	GV_SetActorSignalFunc( pWork , ReceiveSignal2 );
	// ボックスの初期化
	if ( InitBox( pWork , pPos , pRot , model_name , pAttach , name , where ) < 0 ){
		return ( -1 );
	}
	return 0;
}

static void Act( Work *pWork )
{
    GM_SetCurrentMap( pWork->map ); // 表示設定
	if ( pWork->attach_flg == ATTACH_ON ){ // コントロールの更新
		GM_ActControl( pWork->ctrl );
	}
	if ( pWork->attach_flg == ATTACH_ON ){
		if ( CtrlCheck( pWork , pWork->attach_name ) < 0 ){
			return;
		}
	}
	pWork->act( pWork );
	IrsCheck( pWork );
}

static void Die( Work *pWork )
{
	if ( pWork->hit_flg == 0 && pWork->attach_flg == ATTACH_OFF ){
		GM_FreeObject( &pWork->body ); // オブジェクトの解放
	}
	if ( pWork->attach_flg == ATTACH_OFF ){
		GM_FreeObject( &pWork->BreakBody ); // オブジェクトの解放
	}
	GM_ClearTargetDamage( &pWork->target );// ターゲットの解放
	GM_FreeTarget( &pWork->target );
	FreePutObj( pWork );
}

void *NewIrsBox( FVECTOR *pPos , SVECTOR *pRot , int *model_name , Attach *pAttach , int name , int where , int proc , u_long64 my_bit_name )
{
	Work *pWork;

	pWork = GV_NewActorPrio( GV_ACTOR_USER, sizeof( Work ), 0x40 ) ;
	if ( pWork != NULL ) {
		GV_SetActor( &pWork->actor , Act , Die );
		GV_ActorEX( &pWork->actor );
		pWork->my_bit_name = my_bit_name;
		pWork->proc = proc;
		if ( GetResources( pWork , pPos , pRot , model_name , pAttach , name , where ) != 0 ) {
			GV_DestroyActor( pWork );
			return NULL;
		}
	}
	return pWork;
}
