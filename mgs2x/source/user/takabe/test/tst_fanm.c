/*
	tst_fanm.c
	顔アニメ実験キャラ

	2000/06/14 Unknown
	$Id: tst_fanm.c,v 1.1.1.3 2002/11/19 11:51:31 Yoshizawa1 Exp $

*/
/*

シナリオ呼び出しインターフェイス
chara 顔アニメ実験キャラ[NewTestFaceAnimationSet] $s:name \
	-chara	$s:キャラクタ名
mesg 顔アニメ実験キャラ モーション再生[0] $s:モーション名 $w:補間時間
// 実験サンプル（マルチウェイトエンベロープモデル顔アニメキャラ）
// メッセージによりコントロールする（時間は全て1/300秒単位）
// ＜モーション再生コマンド捕捉＞
//  モーション名にはrmtファイルのファイル名を指定
//  


プログラム呼び出しインターフェイス
	void *NewTestFaceAnimation( int name, DG_EVMOBJ *evmobj );
	int			name ;		キャラ名
	DG_EVMOBJ	*evmobj ;	マルチウェイトエンベロープ型モデル

	ＥＶＭファイルに対して臨時フェイスアニメーション機構を付加する
	（以後メッセージにより制御）

	モーション再生開始メッセージ
	message[0] = 0(128で補間時間が1/1000秒単位に)
	message[1] = ＜モーションファイルＩＤ＞
	message[2] = 開始補間時間


※このキャラで使用するモーションファイルというのはモーション班に作成してもらった
　顔のみのモーションデータが含まれているｒｍｔファイル（無圧縮生モーションデータ）
　のことです。注意してください。


*/

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

/* ---------------------------------------------------------------- */
/* 各種定義 */
#ifndef PAL
#define TIME_BASE	(5)
#else
#define TIME_BASE	(6)
#endif

/* ---------------------------------------------------------------- */
/* 内部使用構造体 */

/* モーションファイル構造体 */
typedef struct {
	int			flags ;			/* 各種フラグ */
	int			motion_length ;	/* モーションの有効フレーム数 */
	int			motion_joints ;	/* モーションの関節数 */
	int			motion_tick ;	/* ベースクロック値（１／３００単位） */
	int			move_size ;		/* 移動量のサイズ((motion_length+1)*16) */
	int			rots_size ;		/* 関節回転データのサイズ(((motion_length+1)*motion_joints)*16) */
	int			trans_size ;	/* 関節移動量データへのサイズ(((motion_length+1)*motion_joints)*16) */
	int			minfo_size ;	/* 移動キー情報のサイズ((motion_length+1)*4) */
	int			jinfo_size ;	/* 関節キー情報のサイズ((motion_length+1)*motion_joints)*16) */
	int			sound_size ;	/* サウンド情報のサイズ（将来の拡張用） */
	int			anime_size ;	/* アニメーションのサイズ（将来の拡張用） */
	int			pad[4] ;
	int			error_angle ;	/* 圧縮時のエラー許容角度（ＰＳ固定小数点角度） */
} MTN_FILE_HEADER ;

/* モーションデータ管理構造体 */
typedef struct {
	int			name ;
	int			count ;
	int			length ;
	int			joints ;
	FVECTOR		*move ;
	FVECTOR		*rots ;
	FVECTOR		*trans ;
} MTN_DATA ;

/* スクラッチパッドワーク定義 */
typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
} ScrPadWork ;

/* ---------------------------------------------------------------- */
	/*
		ワーク構造体定義
	*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name ;				/* キャラクタ名 */
	DG_EVMOBJ	*evmobj ;			/* ＥＶＭオブジェクト */
	/* 顔モーション関係 */
	int			interp_count ;		/* 補間時間 */
	MTN_DATA	mtn_data ;			/* ＭＴＮファイル情報＆再生フレーム情報 */
	FVECTOR		face_skel_trans[ 35 ] ;	/* モーショントランスレーション配列 */
	FVECTOR		face_skel_rot[ 35 ] ;	/* モーションクォータニオン配列 */
	/* 視線制御関係 */
	int			eye_pos_type ;	/* 座標パラメータ基準タイプ */
	int			eye_interp_count ;	/* 視線制御補間時間 */
	FVECTOR		target_param ;		/* 座標パラメータ */
	FVECTOR		eye_target ;		/* 視線方向絶対座標 */
	FVECTOR		leye_rot ;			/* 左目回転量 */
	FVECTOR		reye_rot ;			/* 右目回転量 */
} Work ;

/* ---------------------------------------------------------------- */
	/*
		ローカル関数群
	*/
/* ＭＴＮデータのセットアップ */
static void SetMtnMotionData( MTN_DATA *mtn_data, int name_id )
{
	MTN_FILE_HEADER	*mtn_file_header ;
	int				size ;

	/* ロード済みデータから該当データを検索 */
	mtn_file_header = GV_GetCache( GV_CacheID( name_id, 'r' ) );	/* 拡張子は.r??とする */
	if ( mtn_file_header != NULL ){
		/* データが見つかった場合、管理構造体を初期化する */
		mtn_data->length = mtn_file_header->motion_length ;
		mtn_data->joints = mtn_file_header->motion_joints ;
		size = 0 ;
		mtn_data->move = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size = mtn_file_header->move_size ;
		mtn_data->rots = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size += mtn_file_header->rots_size ;
		mtn_data->trans = (FVECTOR*)( (char*)&mtn_file_header[1] + size );
		size += mtn_file_header->trans_size ;
		mtn_data->name = name_id ;
		mtn_data->count = 1 ;
	} else {
		/* 失敗 */
		printf("rmt file open error !!\n");
		mtn_data->name = 0 ;
	}

}

/* ＥＶＭオブジェクトの一部に直接モーションを設定 */
static void EvmActMotionImmediate( DG_EVMOBJ *evmobj, int first, int n_joints, FVECTOR *skel_trans, FVECTOR *skel_rot )
{
	int			i ;
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;
	FVECTOR		vec ;
	FMATRIX		*mats = scrpad->mats ;
	EVM_SKEL	*skel ;

	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/* オブジェクトにマトリクスを設定する */
	skel = &evmobj->def->skeleton[ first ] ;
	vec.vw = 1.0F ;
	/* モデル情報から親子関係を取得して求める */
	for ( i = first ; i < ( first + n_joints ) ; i++ ){
		FMATRIX	*parent ;
		MT_QuatToMat( mats, skel_rot );
		mats->m[3][0] = skel->rt_tx + skel_trans->vx ;
		mats->m[3][1] = skel->rt_ty + skel_trans->vy ;
		mats->m[3][2] = skel->rt_tz + skel_trans->vz ;
		parent = &skel_mats[ skel->parent ] ;
		_sceVu0MulMatrix( mats, parent, mats ) ;
		vec.vx = -skel->rt_tx ;
		vec.vy = -skel->rt_ty ;
		vec.vz = -skel->rt_tz ;
		_sceVu0ApplyMatrix( &mats->m[3][0], mats, &vec );
		skel_mats[i] = *mats ;

		skel_rot++ ;
		skel_trans++ ;
		skel++ ;
		mats++ ;
	}
}

/* ベクトルの線形補間 */
static void InterpVector( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
{
#if 1
	asm ("
		mfc1		$8,%3
		lqc2		vf1,0(%1)
		lqc2		vf2,0(%2)
		qmtc2		$8,vf3
		vmulax.xyz	ACC,vf2,vf3
		vmsubax.xyz	ACC,vf1,vf3
		vmaddw.xyz	vf1,vf1,vf0
		sqc2		vf1,0(%0)
	"::"r"(res),"r"(from),"r"(to),"f"(t): "$8" );
#else
	res->vx = ( to->vx - from->vx ) * t + from->vx ;
	res->vy = ( to->vy - from->vy ) * t + from->vy ;
	res->vz = ( to->vz - from->vz ) * t + from->vz ;
#endif
}

/* 指定した名前のコントロールを検索 */
static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}
	printf("search faild control !!\n");
	return ( NULL );
}


/* ---------------------------------------------------------------- */
	/*
		キャラアクター関数
	*/
static void Act( Work *work )
{
	int		n_msg, i ;
	GV_MSG	*msg ;
	DG_EVMOBJ	*evmobj ;

	/* メッセージチェック */
	if( n_msg = GV_ReceiveMessage( work->name, &msg ) ){
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){

			  case 0:/* モーション発動 */
			  case 128:
				SetMtnMotionData( &work->mtn_data, msg->message[1] );
				work->interp_count = msg->message[2] ;
				if ( msg->message[0] >= 128 ){
					work->interp_count = msg->message[2] * 300 / 1000 ;
				}
				break ;

			  case 1:/* 視線変更 */
			  case 129:
				work->eye_pos_type = msg->message[1] ;
				work->eye_interp_count = msg->message[2] ;
				if ( msg->message[0] >= 128 ){
					work->interp_count = msg->message[2] * 300 / 1000 ;
				}
				work->target_param.vx = msg->message[3] ;
				work->target_param.vy = msg->message[4] ;
				work->target_param.vz = msg->message[5] ;
				work->target_param.vw = 1.0f ;
				break ;

			}
		}
	}

	evmobj = work->evmobj ;

	/* 顔に対するモーション再生 */
	if ( work->mtn_data.name != 0 ){
		float	t ;

		work->interp_count -= TIME_BASE ;
		if ( work->interp_count < TIME_BASE ) work->interp_count = TIME_BASE ;
		/* 補間定数を算出 */
		t = (float)TIME_BASE / work->interp_count ;

		if ( work->interp_count <= TIME_BASE ){
			/* 補間なし */
			for ( i = 0 ; i < work->mtn_data.joints ; i++ ){
				if ( i >= 35 ) break ;
				work->face_skel_trans[i] =
				  work->mtn_data.trans[ ( work->mtn_data.length + 1 ) * i + work->mtn_data.count ];
				work->face_skel_rot[i] =
				  work->mtn_data.rots[ ( work->mtn_data.length + 1 ) * i + work->mtn_data.count ];
			}
		} else {
			/* 補間あり */
			for ( i = 0 ; i < work->mtn_data.joints ; i++ ){
				if ( i >= 35 ) break ;
				/* 平行移動量補間 */
				InterpVector( &work->face_skel_trans[i],
							 &work->face_skel_trans[i],
							 &work->mtn_data.trans[ ( work->mtn_data.length + 1 ) * i + work->mtn_data.count ],
							 t );
				/* 回転量補間 */
				MT_QuatSlerp( &work->face_skel_rot[i],
							 &work->face_skel_rot[i],
							 &work->mtn_data.rots[ ( work->mtn_data.length + 1 ) * i + work->mtn_data.count ],
							 t );
				MT_QuatNormalize( &work->face_skel_rot[i], &work->face_skel_rot[i] );
			}
		}

		/* モーションの再生カウントのインクリメント */
		work->mtn_data.count++ ;
		if ( work->mtn_data.count > work->mtn_data.length ) work->mtn_data.count = work->mtn_data.length ;
	}

	/* 顔モーション設定 */
	//EvmActMotionImmediate( work->evmobj, 21, work->mtn_data.joints, work->face_skel_trans, work->face_skel_rot );
	EvmActMotionImmediate( work->evmobj,
						  21,						/* 顔のマトリクスは２１番目から（固定） */
						  32,						/* 顔用マトリクス数は３２個（固定） */
						  work->face_skel_trans,	/* マトリクスの平行移動成分（×３２個） */
						  work->face_skel_rot );	/* マトリクスの回転成分（×３２個） */
	
}

/* ---------------------------------------------------------------- */
	/*
		キャラアクター終了関数
	*/
static void Die( Work *work )
{
}

/* ---------------------------------------------------------------- */
	/*
		キャラクタ初期化関数
	*/
static int GetResources_S( Work *work, int name, int where )
{
	int		chara_id ;
	CONTROL	*ctrl ;
	OBJECT	*object ;

	work->name = name ;

	/* キャラクタ取得 */
	if ( GCL_GetOption( 'c' ) == NULL ) return ( -1 );
	chara_id = GCL_GetNextInt() ;
	ctrl = SearchControl( chara_id );
	if ( ctrl == NULL ){
		printf("e3_fanim.c: search error control\n");
		return ( -1 );
	}
	if ( ctrl->object == NULL ){
		printf("e3_fanim.c: control initialize miss error\n");
		return ( -1 );
	}
	object = ctrl->object ;
	if ( object->evmobj == NULL ){
		printf("e3_fanim.c: error: no support chara\n");
		return ( -1 );
	}

	work->evmobj = object->evmobj ;

	work->eye_pos_type = -1 ;
	//work->eye_pos_type = 2 ;

	return ( 0 );
}

static int GetResources_P( Work *work, int name, DG_EVMOBJ *evmobj )
{
	work->name = name ;
	work->evmobj = evmobj ;

	work->eye_pos_type = -1 ;

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		シナリオインターフェイス
	*/
void *NewTestFaceAnimationSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources_S( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------- */
	/*
		プログラムインターフェイス
	*/
void *NewTestFaceAnimation( int name, DG_EVMOBJ *evmobj )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources_P( work, name, evmobj ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

