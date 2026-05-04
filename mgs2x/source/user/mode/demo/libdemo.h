/*
	libdemo.h
	デモ関連共有ライブラリ

	2000/07/26	K.Takabe
	$Id: libdemo.h,v 1.1.1.3 2002/11/19 11:45:10 Yoshizawa1 Exp $

*/

#ifndef __LIBDEMO_H__
#define __LIBDEMO_H__

#include <mgs_type.h>
#include <fmt_demo.h>
#include <libgv.h>
#include <libdg.h>
#include <libmt.h>

#include "eft_con.h"

#include "BP_Camera.h"

#ifdef __cplusplus
extern "C" {
#endif


/* ---------------------------------------------------------------- */
	/*
		一度に管理可能な最大数
	*/
#define DM_MAX_FRAME	(8)		/* フレーム最大数 */
#define DM_MAX_OBJECT	(256)	/* オブジェクト最大数 */
#define DM_MAX_MOTION	(64*4)	/* モーション最大数 */
#define DM_MAX_EFFECT	(512)	/* エフェクト最大数 */

#define DM_MAX_PROC	(64)	/* プロックリスト最大数 */
#define DM_MAX_RENAME	(32)	/* リネームリストの最大数 */

/* ---------------------------------------------------------------- */
	/*
		フレーム内部管理構造体
	*/
typedef struct _dm_frame {
	int			id ;			/* フレームのユニークＩＤ */
	int			no ;			/* カメラ番号 */
	int			chanl ;			/* チャンネル番号 */
	DEMO_FRAME	*frame_ptr ;	/* フレームパケットへのポインタ */
} DM_FRAME ;

/* ---------------------------------------------------------------- */
	/*
		オブジェクト内部管理構造体
	*/
typedef struct _dm_object {
	int			id ;			/* オブジェクトのユニークＩＤ */
	void		*work_ptr ;		/* オブジェクト管理アクターへのポインタ */
	int         flag;
} DM_OBJECT;

/* ---------------------------------------------------------------- */
	/*
		モーション内部管理構造体
	*/
typedef struct _dm_motion {
	int			object_id ;		/* 対象オブジェクトのユニークＩＤ */
	DEMO_MOTION	*motion_ptr ;	/* モーションパケットへのポインタ */
} DM_MOTION ;

/* ---------------------------------------------------------------- */
	/*
		エフェクト内部管理構造体
	*/
typedef struct _dm_effect {
	int			id ;			/* エフェクトのユニークＩＤ */
	void		*work_ptr ;		/* エフェクト管理アクターへのポインタ */
} DM_EFFECT;

/* ---------------------------------------------------------------- */
	/*
		デモ管理構造体
	*/
typedef struct _dm_work {
	GV_ACT_EX	*actor ;		/* デモ管理デーモンのアクター構造体へのポインタ */
	int			sequence_end ;	/* デモシーケンス終了フラグ */
	int			frame_end ;		/* フレーム処理終了フラグ */
	int			n_frame ;		/* 現在保持しているフレームの数（毎フレームクリア） */
	int			n_object ;		/* 管理中のオブジェクト数 */
	int			n_motion ;		/* 現在保持しているモーションの数 */
	int			n_effect ;		/* 管理中のエフェクト数 */
	DM_FRAME	frm_list[DM_MAX_FRAME] ;	/* フレームリスト（毎フレーム初期化される） */
	DM_OBJECT	obj_list[DM_MAX_OBJECT] ;	/* オブジェクトリスト */
	DM_MOTION	mtn_list[DM_MAX_MOTION] ;	/* モーションリスト */
	DM_EFFECT	eft_list[DM_MAX_EFFECT] ;	/* エフェクトリスト（消失をデモ側で管理するもののみ保持） */
	int		proc_id[DM_MAX_PROC] ;	/* プロックIDリスト（エフェクト側からプロックを呼び出すため） T.Morita Added 2000/08/30 */
	int		low_poly_demo ;
} DM_WORK ;


/* ---------------------------------------------------------------- */
	/*
		デモデバッグモード管理構造体
	*/
/* カメラ属性用 */
#define DEMO_CAMERA_NO_RESET    0x0001	// CAM_FLAG_PAD_ADJUST_NO_RESETを追加

#ifdef DEBUG_MODE

#define WAIT_COUNT 5
#define EFT_LIST_COUNT 50
#define VIEW_LIST_COUNT 100
#define VIEW_MODE_MAX 5
#define VIEW_BASE_Y   25

#if 0
#define VIEW_TYPE 1	// ローレゾ
#else
#define VIEW_TYPE 2	// ハイレゾ
#endif


#define BLANK_COUNT (BP_FRAMES_PER_SEC())		/* 1以上を指定 */

#define START_TIME 5

#define FLAG_DEMO           0x0000
#define FLAG_AMBIENT_COLOR  0x0001
#define FLAG_FOG_COLOR      0x0002
#define FLAG_FOG_LANGE      0x0004
#define FLAG_PARALLEL_COLOR 0x0008
#define FLAG_PARALLEL_DIR   0x0010

/* デバッグモードデファイン */
enum {
	MODE_PLAY = 0,
	MODE_DEBUG_PLAY,
	MODE_DEBUG_EDIT };

/* デバッグ表示レベルデファイン */
enum {
	VIEW_LEVEL0 = 0,
	VIEW_LEVEL1,
	VIEW_LEVEL2,
	VIEW_LEVEL3,
	VIEW_EFFECT };

/* 編集モードデファイン */
enum {
	EDIT_CAMERA_ROTATE = 0,
	EDIT_CAMERA_MOVE,
	EDIT_AMBIENT,
	EDIT_PARALLEL,
	EDIT_PARALLEL_ROT,
	EDIT_FOG,
	EDIT_FOG_PARAM
};


#if 1
/* エフェクトリスト */
typedef struct _dm_eft_list {
	char id[10];		/* エフェクトID */
	char name[50];	/* エフェクト名 */
} DM_EFT_LIST;

/* 表示用エフェクトリスト */
typedef struct _dm_eft_list_view {
	int  list_index;	/* 表示するリストのインデックス */
	int  start;			/* スタートしたフレーム */
	int  view;			/* 表示フラグ */
} DM_EFT_VIEW;
#else
/* エフェクトリスト */
typedef struct _dm_eft_list {
	char id[10];		/* エフェクトID */
	char name[50];	/* エフェクト名 */
	int  start;			/* スタートしたフレーム */
	int  view;			/* 表示フラグ */
} DM_EFT_LIST;
#endif


/* 初期値は０で統一しませう */
typedef struct _dm_debug {
	/* デバッグ用変数 */
	GV_PAD  *pad;			/* パッド : ２Pコントローラ */
	short     play_mode;		/* 0 : 通常再生 */
							/* 1 : ループ再生 */
	short     play_status;	/* 0 : 再生方向 → */
							/* 1 : 再生方向 ← */
	short     view_mode;		/* 表示切替用変数 */
							/* 3 : エフェクトリスト */
							/* 4 : 表示なし */
	short     view_color;
	short     mode1;	/* 0 : デモ再生モード */
						/* 1 : デモビューモード */
						/* 2 : デモフリーカメラモード */
	short     mode2;	/* 各モードのもう一つのモード */
	short     first;		/* カメラパラメータを一度しか取得しないためのフラグ */
						/* 0 : 未取得 */
	short     speed_type; /* 0 : Normal */
						/* 1 : High */
	short     edit_mode;	/* 0 : カメラ回転モード */
						/* 1 : カメラ移動モード */
						/* 2 : 環境光編集モード */
	short     n_camera;	/* 0 : カメラが存在していない */
	short     n_wait;		/* スロー再生・逆転再生時のウェイトカウント */
	FVECTOR ftrg_pos;	/* 現在のカメラのポジション */
	FVECTOR fcam_pos;	/* 現在のカメラのターゲット */
	float   zoom;		/* 現在のカメラのがかく */
	float   cam_len;	/* スクリーン間距離 */
	char    *string;	/* デバッグ文字列 */
	int     n_effect;	/* エフェクト数 */
	DM_EFT_LIST  effect_list[EFT_LIST_COUNT];	/* エフェクトリスト */
	int     n_eft_view;	/* 表示用エフェクトリストの登録数 */
	DM_EFT_VIEW  effect_view[VIEW_LIST_COUNT];	/* 表示用エフェクトリスト */
	int     frame;		/* 現在のフレームカウント */
	int     pre_tick;	/* 前回のDG_TickCountの値 */
	int     view_time;	/* リストを表示する時間 */
	int     half_frame_base;	/* 30フレーム表記のベースタイム */
	int     start_time;
	char    print_buffer[128*20] ;	/* 20行分の表示バッファ(T.Morita Added 2000/08/30) */

	/* カラー関連フラグ：ただしエフェクトの制御はこちらからはどうしようもない */
	/* 0 : デモのパラメータが設定値 */
	/* 0 以外 : デモのパラメータが設定値 */
	short sAmbientFlag;		/* 環境光用 */
	short sFogFlag;			/* フォグ用 */
	short sParallelFlag;	/* 平行光源用 */

	/* 環境光 */
	int     ambient_col_def[3];		/* デフォルト環境光 */
	int     ambient_col[3];			/* 修正した環境光 */
	short   ambient_status;			/* 編集する環境光の対象 */

	/* フォグ */
	int     fog_col_def[3];		/* デフォルトフォグカラー */
	int     fog_near_def;		/* デフォルト near = (255.0f - DG_FogParam2) / DG_FogParam1 */
	int     fog_far_def;		/* デフォルト far = near - (255.0f / DG_FogParam1)*/
	int     fog_col[3];			/* 修正したフォグカラー */
	short   fog_status;			/* 編集するフォグの	対象 */

	float   fog_near;			/* 修正した near = DG_FogParam1 */
	float   fog_far;			/* 修正した far = DG_FogParam2 */
	short   fog_param_status;	/* 編集するフォグの対象 */

	/* 平行光源 カラー */
	int     parallel_col_def[3];		/* デフォルト平行光源 */
	int     parallel_col[3];			/* 修正した平行光源 */
	short   parallel_status;			/* 編集する平行光源の対象 */

	int     parallel_vec_def[3];		/* デフォルト平行光源ベクトル */
	float   parallel_vec[3];			/* 修正した平行光源ベクトル */
	int     parallel_rot_def[2];		/* 平行光源回転 [0] = X, [1] = Y */
	int     parallel_rot[2];			/* 平行光源回転 [0] = X, [1] = Y */

	// パケットデバッグパラメーター
	short lpsPacketList[DEMO_PACKET_END+1][2];		/* 1フレームのパケット情報 */

} DM_DEBUG ;

extern DM_DEBUG dm_debug;


#endif

/* ---------------------------------------------------------------- */
	/*
		補助アクター用シグナル定義
	*/
enum{
	DM_SIGNAL_OBJECT_VISIBLE = 1,	/* オブジェクトの表示・非表示 */
	DM_SIGNAL_OBJECT_GET_CONTROL,	/* コントロール取得 */
	DM_SIGNAL_OBJECT_GET_OBJECT,	/* オブジェクト取得 */
	DM_SIGNAL_OBJECT_GET_MATRIX,	/* 関節マトリクスの取得 */
	DM_SIGNAL_OBJECT_GET_DGOBJS,	/* モデルの取得     */
	DM_SIGNAL_OBJECT_GET_DGOBJ,	/* 関節モデルの取得 */
	DM_SIGNAL_OBJECT_GET_LIGHTMTX,	/* ライトマトリックスの取得 */
	DM_SIGNAL_OBJECT_GET_EVM,		/* EVM obj*/
};

/* ---------------------------------------------------------------- */
	/*
		パケットストリーム処理テーブル構造体
	*/
typedef void (*DM_PACKET_FUNC)(DM_WORK*,void*) ;

/* ---------------------------------------------------------------- */
	/*
		エフェクト起動関数構造体
	*/
typedef void* (*DM_EFFECT_EXEC)( int id, void *param_ptr );

/* ---------------------------------------------------------------- */
	/*
		プロトタイプ宣言
	*/

/* demod.c */
extern DM_WORK* DM_GetDemoWork( void );
extern int DM_GetDemoActorMap() ;


/* demo_var.c */
extern int DM_FrameSkip ;  /* フレームスキップ スキップしたフレーム数が入る */
extern int DM_CameraFlag;	/* デモのカメラ起動時のGM_SetCameraTypeに */
							/* 属性を追加するかを決定するフラグ。     */

//BP_CAMERA - adding extra demo vars for tracking camera cuts
extern int  gBP_Demo_Active;
extern int  gBP_Demo_Id;
extern int  gBP_Demo_Frame;
extern char gBP_Demo_StreamName[];
//BP_CAMERA - adding extra demo vars for tracking camera cuts

/* demo_pkt.c */
extern void DM_ExecDemoStream( DM_WORK *work, void *packet_stream_ptr, int exec_flag );
extern void DM_Packet_TerminateSequence( DM_WORK *work, DEMO_PACKET *packet );
extern void DM_Packet_TerminateFrame( DM_WORK *work, DEMO_PACKET *packet );


/* demo_cmd.c */
extern void DM_Packet_Command( DM_WORK *work, DEMO_COMMAND *packet );


/* demo_frm.c */
extern void DM_Packet_Frame( DM_WORK *work, DEMO_FRAME *packet );
extern DEMO_FRAME*	DM_GetFrameData( int no );


/* demo_obj.c */
extern void DM_Packet_Object( DM_WORK *work, DEMO_OBJECT *packet );
extern void DM_DestroyObjectAll( void );
extern void DM_DestroyObject( int id );
extern void DM_SetObjectInvisible( int id, int invisible_flag );
extern void* DM_GetObjectHandle( int id );
extern void* DM_GetObjectHandleCore( int id ) ;
extern CONTROL* DM_GetObjectControl( void *handle );
extern OBJECT* DM_GetObjectObject( void *handle );
extern DG_OBJS* DM_GetObjectDgObjs( void *handle );
extern FMATRIX* DM_GetObjectMatrix( void *handle, int joint );
extern DG_OBJ* DM_GetObjectDgObj( void *handle, int joint );
extern FMATRIX* DM_GetObjectLightMtx( void *handle );


/* demo_lst.c */
extern void DM_RefreshRenameList( void ) ;
extern void DM_AddRenameList( int ext, int flag ) ;
extern int DM_n_RenameList ; /* リネームリストの長さ */
extern int DM_RenameFrom[DM_MAX_RENAME] ;
extern int DM_RenameTo[DM_MAX_RENAME] ;
extern int DM_RenameDispFlag[DM_MAX_RENAME] ;/* 描画フラグ */

extern int DM_n_EquipList ; /* 装備リストの長さ */
extern int DM_EquipTo[DM_MAX_RENAME] ;
extern int DM_EquipFlag[DM_MAX_RENAME] ;


/* demo_mtn.c */
extern void DM_Packet_Motion( DM_WORK *work, DEMO_MOTION *packet );
extern void DM_Packet_MotionPack( DM_WORK *work, DEMO_MOTION_PACK *packet );
extern DEMO_MOTION*	DM_GetMotionData( int object_id );
extern DEMO_MOTION*	DM_GetMotionDataNext( void );


/* demo_eft.c */
extern int DM_EffectInitName ;/* エフェクト初期君の名前 */
extern int DM_EffectInitFlag ;/* エフェクト初期君からのフラグ */
enum effect_init_t
{
    DM_EFFECT_INIT = 0x00000001,
} ;
extern void  DM_Packet_Effect( DM_WORK *work, DEMO_EFFECT *packet );
extern void  DM_DestroyEffectAll( void );
extern void  DM_DestroyEffect( int id );
extern void* DM_SendEffectMessage( int name,
				   int data0, int data1, int data2, int data3,
				   int data4, int data5, int data6, int data7,
				   int command, int num ) ;
extern void* DM_NULL() ;
extern void* DM_ChangeAmbient( IVECTOR *rgb ) ;
extern void* DM_ChangeParallel( IVECTOR *rgb, FVECTOR *dir ) ;
extern void* DM_DebugPrint( char *message ) ;
extern void* DM_SetClipParam( float near, float far ) ;

extern DG_EVMOBJ* DM_GetEvmObjObject( void *handle ); /* DF_EVMOBJ* */
extern FVECTOR* DM_GetObjectPos( void *handle, int joint ); /* DG_OBJからポジションゲット*/
extern int      DM_GetCurrentMap(); /*カレントマップを取得する*/
extern void DM_IVecToSVector( IVECTOR *i_vec, SVECTOR *s_vec );
extern void DM_GetRotToRadVec(FVECTOR *r,float x,float y,float z,float w);
extern void DM_InFVector(FVECTOR *r,float x,float y,float z,float w);
extern void DM_GetFMatrixM( FVECTOR *out, FVECTOR *rot, int num );
extern void DM_EftControlMatrix( EFTCONTROL *eft_con, FMATRIX *world ) ;

extern void DM_GetFMatrixM_ZXY( FVECTOR *out, FVECTOR *rot, int num );
extern void DM_GetFMatrixM_XYZ( FVECTOR *out, FVECTOR *rot, int num );
/* obj_act.c */
extern void *NewDemoObject( int id, int name, int evm_id, int kms_id, int flag );

/* cam_act.c */
extern void *NewDemoCamera( int id, int name, int flag );

/* eff_act.c */
extern void *NewDemoEffectLaunch( DEMO_EFFECT *packet, void *parent ) ;


/* ---------------------------------------------------------------- */

/* 進行時間単位 */
#ifndef TIME_BASE
#define TIME_BASE				(BP_BASE_TICK())
#endif

//#define DEMO_FILE_MEMORY	( (void*)0x02400000 )	/* 32+4MB~ */
#define DEMO_FILE_MEMORY	( (void*)0x04000000 )	/* 32+4MB~ */
/* ---------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __LIBDEMO_H__ */

