/*
	npc.h
	NPCメインヘッダ

	2001/02/08 Y.Korekado
	$Id: npc.h,v 1.1.1.3 2002/11/19 11:44:22 Yoshizawa1 Exp $
*/

#ifndef __NPC_____
#define __NPC_____ (1)

/*----------------------------------------------------------------*/
#include	"../conv/korekado.x"
#include	"../conv/define.h"
//#include	"../../shibata/t_headmark/headmark.h"
#include	"../../shibata/t_headmark/headmark2.h"
#include	"../../kano/attachment/attachment_called.h"
#include	"../../takabe/other/puppetik.h"
#include	"../../kira/face/face.h"
#include	"commdef.h"
#include	"../thing/item.h"
#include	"../enemy/route.h"
/*----------------------------------------------------------------*/
typedef	struct	{
	u_int	time ;		/* NPC_SetActModeで 0 */
	int		status ;	/* NPC_Actionで毎フレーム 0 */
	int		status_status ;	/* NPC_Actionで毎フレーム 0 */
	short	dir ;		/* NPC_PreProcessで毎フレーム -1 */
	short	act_end ;	/* NPC_Actionで毎フレーム 0 */
	int		pad ;		/* NPC_PreProcessで毎フレーム 0 */

	int		current_mar ;	/* 使用中の mar */
	short	current_mot ;	/* 再生中のモーション番号 */
	short	down_side ;		/* ダウン方向 */

	short	sleep ;			/* 眠り値(今は未使用) */
	short	life ;			/* ライフ値 */
	short	faint ;			/* 気絶値 */
	short	faint_count ;	/* 気絶カウント */
	u_short	faint_mode ;	/* 気絶モード 0:piyo 1:Zzz */
	u_short	faint_max ;		/* 最大気絶カウント */

	u_short	sleep_max ;		/* 最大眠りカウント */
	u_short	down_dam ;		/* ダウンダメージ */

	int		*headmark ;		/* ヘッドマーク操作 */
	short	headmark_num ;	/* Zzz, Ppp の個数 */
	short	face_anime ;		/* 顔表情 */

	int		set_mar ;		/* checkpad から mar 指定 */
	short	set_mot ;		/* checkpad から モーション番号指定 */
	short	set_pad ;		/* checkpad から pad 番号指定 */

	short	face_dir ;		/* 顔の向き。レーダーに反映 */
	short	eye_range ;		/* 視野。レーダーに反映 */
	short	eye_sight ;		/* 視力。レーダーに反映 */
	short	radar_color ;	/* レーダーの色 */
	short	heart_beat ;	/* 心臓音感覚 */
	short	padddddd ;

	int		adj_piku_time ;	/* プログラムピク */
   int      adj_piku_value ;  // Armature fix to not over vibrate the soldiers when being sprayed
	int		tmp_time ;		/* 汎用タイム */

	void	*ikwork ;		/* IK用ワーク */
	ITEM_PROC	item ;
	ITEM_PROC	dogtag_item ;
	int				dogtag_id ;
	int				sw_dogtag ;
	u_int	hang_time ;		/* */
} NPCACT ;

#define NPC_MAX_CHILD_TARGET	(32)
typedef	struct	{
    TARGET			*deftrg ;	/* 防御ターゲット */
	CAPTURE_TARGET	*capture ;	/* 捕まりターゲット */
    TARGET			*pushtrg ;	/* ぶつかりターゲット */

    TARGET			*def_child ;	/* 子ターゲット*/
	int				child_trg_num ;	/* 子ターゲット総数 */
    u_char			trg_level_num[4] ;	/* 各レベルのターゲット数 */
	u_char			connect_obj[NPC_MAX_CHILD_TARGET] ;	/* 子ターゲット基本オブジェクト位置 */

	FVECTOR			*target_size ;	/* 状態別ターゲットサイズ */
	FVECTOR			*drag_shift ;	/* 引き釣りシフト(モーション別) */
	FVECTOR			*hang_shift ;	/* 首締めシフト(モーション別) */

	TARGET			*dam_trg ;		/* ダメージを受けたターゲット */
	FVECTOR			force ;			/* ダメージフォース */
	u_short			dam_obj ;		/* ダメージを受けたオブジェ */
	short			tmp_count ;
	long64			weapon_type ;	/* ダメージＷＰタイプ */
} NPCTARGET ;

typedef	struct	{
	FVECTOR		aim_pos ;
	int			adj_status ;
	SVECTOR		adj_rot ;
} NPCADJUST ;

#define		NPC_ADJ_ON			0x00000001
#define		NPC_ADJ_KUBIFURI	0x00000002	/* 口パク代わりの首振り*/
#define		NPC_ADJ_GME_BDMH	0x00000004	/* メカゴルびっくりどっきりメカ発進 */

typedef	struct	_NPCWORK {
	OBJECT	*body ;
	CONTROL	*ctrl ;
	FMATRIX	*lights ;
	int		lod ;			/* マルチウェイトモデル使用時、切替えるカメラとの距離 */

	NPCACT		action ;
	NPCTARGET	target ;
	void	( *actmode_call )( struct _NPCWORK *, int );	/* アクション */
	int		( *CheckDamage)( struct _NPCWORK * ) ;	/* ダメージチェック関数 */
	int		( *CheckPad )( struct _NPCWORK * ) ;	/* パッドチェック関数 */
	float 	old_body_height ;	/* 1フレ前の高さ。高さ移動量に使用 */

	NAVIGATE 	*navi ;
	NAVITARGET	*nvtrg ;
	RADAR_CTRL	*rctrl ;
	FC_EVM_FACE	*fcanim ;
	NPCADJUST	*nadj ;
	HOMING_TRG		*hom ;
	ROUTENAVI	*rnavi ;

	int		base_mar ;			/* 1番目に「立ち」、2番目に「歩き」が入っているmar */
	int		damage_mar ;		/* 各キャラのダメージモーション */
	int		capture_mar ;		/* システム指定のキャプチャーダメージモーション */
	int		drag_mar ;			/* システム指定の引き摺りモーション */
	int		inf_name ;			/* ストリーミング口パク用インフ名 */

	int		se_id ;
	int		status ;

	void	*headmark ;
	
	void	*character ;		/* キャラクターワーク */
} NPCWORK ;

typedef	void	( *NPCACTMODE )( NPCWORK *, int ) ;
typedef	int	( *NPCCHECK )( NPCWORK * ) ;
/*----------------------------------------------------------------*/
/* status */
#define NPC_STATUS_NO_HANGDAM	0x00000001	/* ハング時 バレットダメージモーションなし */
#define NPC_STATUS_HANG_LIFEDAM	0x00000002	/* ハングされる度にライフ値減少  */
#define NPC_STATUS_HOSTAGE		0x00000004	/* 人質 */


/*----------------------------------------------------------------*/
/* heart_beat */
#define NPC_HEART_BEAT_NORMAL	COUNT_VMODE(120)
/*----------------------------------------------------------------*/
#define	NPC_ADJ_PIKU_TIME	COUNT_VMODE(6)

#define NPC_GAGE_LEVEL	(1)

#define	MOTION_MASK_FULL	0xffffff
#define	MOTION_MASK_UPPER	0x1ffe
#define	MOTION_MASK_LOWER	0x1fe001

#define	ACT_INTERP_DEF		8*5	/* 1/300単位 */
#define	ACT_INTERP_M		60*1 /* 1/300単位 */
#define	ACT_INTERP_SLOWLY	60*5 /* 1/300単位 */
#define	ACT_INTERP_VERY_SLOWLY	60*30 /* 1/300単位 */

#define	LAYER_BASE	0
#define	LAYER_OVER1	1
#define	LAYER_OVER2	2

#define DEF_TARGET_CLASS	(TARGET_CAPTURE|TARGET_LOCKON|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_CHILD)	/* 標準ターゲットクラス */
#define FAINT_TARGET_CLASS	(TARGET_CAPTURE|TARGET_DEFENSE|TARGET_SEEK|TARGET_POWER|TARGET_CHILD|TARGET_DEAD)	/* 気絶ターゲットクラス */

/* 麻酔弾モデル */
#define MASUIDAN_MODEL	(2512988) /* GV_StrCode("m92_bul2") */

/* base_mar */
enum {
	NPC_MOT_STAND,	/* base_mar の 1番目のモーション */
	NPC_MOT_MOVE,	/* base_mar の 2番目のモーション */

	NPC_BASE_MOT_END
} ;

/* capture_mar */
enum {
	NPC_MOT_DOWN_F,		/* 仰向けダウンアイドル */
	NPC_MOT_THROW,		/* なげられモーション */

	NPC_MOT_HANG_DIE,	/* 首締め殺し */
	NPC_MOT_HANG_IDLE,	/* 首締めアイドル */
	NPC_MOT_HANG_MOVE,	/* 首締め移動 */
	NPC_MOT_HANG_RELEASE,	/* 首締め開放 */
	NPC_MOT_HANG_RELEASE_DOWN,	/* 首締め解放後ダウン */
	NPC_MOT_HANG_RESISIT,	/* 首締め抵抗 */
	NPC_MOT_HANG_TIE,	/* 首締め */
	NPC_MOT_HANG_ESCAPE,	/* 首締め脱出 */
	NPC_MOT_HANG_DAMAGE,	/* 首締めダメージ */

	NPC_MOT_DOWN_B,		/* うつ伏せダウンアイドル */
} ;

/* drag_mar */
enum {
	NPC_MOT_DRAG_BODY_START,
	NPC_MOT_DRAG_BODY_IDLE,
	NPC_MOT_DRAG_BODY_WALK,
	NPC_MOT_DRAG_BODY_END,

	NPC_MOT_DRAG_LEG_START,
	NPC_MOT_DRAG_LEG_IDLE,
	NPC_MOT_DRAG_LEG_WALK,
	NPC_MOT_DRAG_LEG_END,

	NPC_MOT_DRAG_LEG_START_B,
	NPC_MOT_DRAG_LEG_IDLE_B,
	NPC_MOT_DRAG_LEG_WALK_B,
	NPC_MOT_DRAG_LEG_END_B,

	NPC_MOT_DRAG_BODY_START_B,
} ;

/* npc->target.target_size */
enum {
	NPC_TARGET_SIZE_STAND,	/* 立ち状態 */
	NPC_TARGET_SIZE_DOWN,	/* ダウン状態 */
	/* ここまでシステムにより固定,これ以降は各キャラクターのお好みに */
} ;

/* npc->target.drag_shift */
enum {
	NPC_DRAG_SHIFT_HEAD_F_START16,	/* 仰向け頭 スタート16f後 */
	NPC_DRAG_SHIFT_HEAD_F,			/* 仰向け頭 */
	NPC_DRAG_SHIFT_LEG_F_START16,	/* 仰向け足 スタート16f後 */
	NPC_DRAG_SHIFT_LEG_F,			/* 仰向け足 */
	NPC_DRAG_SHIFT_LEG_B_START16,	/* うつ伏せ足 スタート16f後 */
	NPC_DRAG_SHIFT_LEG_B,			/* うつ伏せ足 */
	NPC_DRAG_SHIFT_HEAD_B_START16,	/* うつ伏せ頭 */
} ;

/* down_side */
enum {
	NPC_DOWN_SIDE_F,
	NPC_DOWN_SIDE_B,
} ;

/* faint_mode */
enum {
	NPC_FAINT_PIYO,	/* 気絶 */
	NPC_FAINT_ZZZ,	/* 眠り */
} ;

/* face_anime */
enum {
	NPC_FCANIM_NORMAL,	/* 通常（目パチ） */
	NPC_FCANIM_EYE_CLOSE,	/* 目つむる */
	NPC_FCANIM_SUFFER,	/* 苦しむ */
	NPC_FCANIM_SMILE,	/* 微笑む */
} ;

/*----------------------------------------------------------------*/
static inline void NPC_SetActMotionEX( NPCWORK	*npc,
									  int		n_layer ,	/* レイヤー番号 */
									  int		mar ,		/* mar */
									  int		mot ,		/* モーション番号 */
									  u_long64	mask ,		/* マスクビット */
									  int		interp ,	/* 補完時間 */
									  float	speed )		/* 再生スピード */
{
	NPCACT *act ;

	act = &npc->action ;
	if ( act->current_mar != mar ) {
		act->current_mar = mar ;
		npc->body->m_ctrl->motion_arc = MT_GetMotionArchives( mar ) ;
		MT_FreeSequence( npc->body->m_ctrl->sar_ctrl ) ;
		npc->body->m_ctrl->sar_ctrl = MT_InitSequence( npc->body->m_ctrl->n_layer, mar, 0 ) ;
	}
	if ( n_layer == 0 ) act->current_mot = mot ;

	GM_ConfigObjectAction( npc->body, n_layer, mot, 0, mask, interp ) ;
	MT_SetMotionSpeed( npc->body->m_ctrl, speed ) ;

	SET_FLAG(npc->body->m_ctrl->mt3_ctrl[ n_layer ].flag, MT3_ACTIVE ) ;
}

static	inline void NPC_MotionLayerOverOff( NPCWORK *npc, int layer )
{
	MT_ResetMotionData( npc->body->m_ctrl, layer ) ;
//	npc->body->m_ctrl->mt3_ctrl[ layer ].flag &= ~MT3_ACTIVE ;
}

static	inline void NPC_SetActMotion( NPCWORK *npc, int mar, int mot_num )
{
	NPC_SetActMotionEX( npc, LAYER_BASE,
		mar, mot_num, MOTION_MASK_FULL, ACT_INTERP_DEF, (float)TIME_BASE ) ;
}

static inline void NPC_SetActMode( NPCWORK *npc, NPCACTMODE actmode )
{
	npc->actmode_call = actmode ;
	npc->action.time = 0 ;
}

static inline void NPC_SetModeFromPad( NPCWORK *npc, NPCACTMODE actmode, int mar, int mot, int pad )
{
	NPCACT *act ;

	act = &npc->action ;
	act->set_mar = mar ;
	act->set_mot = mot ;
	act->set_pad = pad ;
	NPC_SetActMode( npc, actmode ) ;
}
/*----------------------------------------------------------------*/

#define	NPC_ActStatus( a,b )		(a)->status |= (b)
#define	NPC_ActStStatus( a,b )		(a)->status_status |= (b)

//npc->act->status
// システムでは下位１６ビット使う、上位はそれぞれ好きなように
#define NPC_ACT_STATUS_STAND		0x00000001	/* 立ち */
#define NPC_ACT_STATUS_MOVE			0x00000002	/* 移動 */
#define NPC_ACT_STATUS_DAMAGE		0x00000004	/* ダメージ処理によりactionが奪われ中 */
#define NPC_ACT_STATUS_FAINT		0x00000008	/* 気絶居眠り中 */
#define NPC_ACT_STATUS_IK_DOWN		0x00000010	/* ダウンＩＫ */
#define NPC_ACT_STATUS_IK_FOOT		0x00000020	/* 引き摺り足ＩＫ */
#define NPC_ACT_STATUS_IK_HAND		0x00000040	/* 引き摺り手ＩＫ */
#define NPC_ACT_STATUS_IK_PIKU		0x00000080	/* ダウンピクピクＩＫ */
#define NPC_ACT_STATUS_TRG_OFF		0x00000100	/* ターゲットオフ */
#define NPC_ACT_STATUS_FAINT_END	0x00000200	/* 気絶居眠り終了 */
#define NPC_ACT_STATUS_NPCSYS_DAM	0x00000400	/* ＮＰＣシステムによるダメージ処理中 */
#define NPC_ACT_STATUS_HANG			0x00000800	/* 首締められ中 */
#define NPC_ACT_STATUS_DEATH		0x00001000	/* 死亡 */
#define NPC_ACT_STATUS_EYE_CLOSE	0x00002000	/* 視界ＯＦＦ */
#define NPC_ACT_STATUS_CAPTURE_OFF	0x00004000	/* 掴みターゲットオフ */
#define NPC_ACT_STATUS_HOMING_OFF	0x00008000	/* ホーミングオフ */

//npc->act->status_status
#define NPC_ACT_STST_UNDER_NEARCHECK	0x00000001	/* 低い壁チェック*/

/*----------------------------------------------------------------*/
//npc.c
void	NPC_DamageFlagClear( NPCWORK	*npc ) ;
void	NPC_CaptureFlagClear( NPCWORK	*npc ) ;
void	NPC_DamageCaptureFlagClear( NPCWORK	*npc ) ;
void	NPC_CallHeadMark( NPCWORK *npc, int mark  ) ;
void	NPC_SetFaintCount( NPCWORK *npc, int count, int mode  ) ;
int	NPC_ChildTargetCheck( NPCWORK	*npc ) ;
void NPC_SetFaintCount( NPCWORK *npc, int count, int mode  ) ;
void NPC_SetNeedl( OBJECT *body, int n_obj, FVECTOR *pos, int model ) ;
void NPC_SetNeedlV( OBJECT *body, int n_obj, FVECTOR *pos, int model ) ;
void NPC_ClearNeedl( OBJECT *body ) ;

void NPC_PreProcess( NPCWORK *npc ) ;
void NPC_AfterProcess( NPCWORK *npc ) ;
void NPC_ActControl( NPCWORK *npc ) ;
void NPC_ActControlExe( NPCWORK *npc, int scale ) ;
void NPC_Gravitation( NPCWORK *npc ) ;

void NPC_SetCheckPad( NPCWORK *npc, NPCCHECK checkpad ) ;
void NPC_SetCheckDamage( NPCWORK *npc, NPCCHECK checkdamage ) ;
void NPC_ChangeTargetSize( NPCWORK *npc, int n ) ;

void NPC_InitNPC( NPCWORK *npc, void *work, OBJECT *body, CONTROL *ctrl, FMATRIX *lights ) ;
void NPC_InitControl( NPCWORK *npc, int name ) ;
void NPC_InitObject( NPCWORK *npc, int modelname ) ;
void NPC_InitMWObject( NPCWORK *npc, int modelname, int lod ) ;
void NPC_InitMotion( NPCWORK *npc, int base, int damage, int capture, int drag ) ;
void NPC_InitNavi( NPCWORK *npc, CONTROL *ctrl, NAVIGATE *navi, NAVITARGET *nvtrg ) ;
void NPC_InitDefenceTarget( NPCWORK *npc,TARGET*deftrg, int side, FVECTOR *size, FVECTOR *drag_shift, FVECTOR *shift ) ;
void	NPC_SetHangShift( NPCWORK *npc, FVECTOR *shift ) ;
void NPC_InitDefenceChildTarget( NPCWORK *npc, TARGET *child_trg,
			int side, FVECTOR *size, FVECTOR *shift, int *level_num, int *connect ) ;
void NPC_InitDefenceCapture( NPCWORK *npc,CAPTURE_TARGET *capture, CONTROL *ctrl, OBJECT *body ) ;
void NPC_InitRader( NPCWORK *npc, RADAR_CTRL *rctrl, FVECTOR *mov,
			float upper, float lower, int range, int sight, int color ) ;
void NPC_InitPose( NPCWORK *npc, FVECTOR *pos, int dir, int mar, int mot ) ;
void NPC_FreeResources( NPCWORK *npc ) ;
void NPC_InitHeadMark(void *w,NPCWORK *npc,int *flag, OBJECT *body, CONTROL *ctrl, TARGET *trg ) ;
int NPC_InitFaceAnime( NPCWORK *npc, DG_EVMOBJ * evm, int mot_name, int inf_name ) ;
void NPC_InitMotionAdjust ( NPCWORK *npc, NPCADJUST *nadj ) ;
void NPC_InitHomingTarget( NPCWORK *npc, HOMING_TRG *hom, OBJECT *body, CONTROL *ctrl,TARGET *trg ) ;
void NPC_SetSeID ( NPCWORK *npc, int se_id ) ;
void NPC_SetStatus ( NPCWORK *npc, int status ) ;
void NPC_InitRouteNavi ( NPCWORK *npc, ROUTENAVI *rnavi, int route ) ;

//npcact.c
void NPC_Incline( CONTROL *ctrl ) ;
void NPC_ActStandStill( NPCWORK *npc, int time ) ;
void NPC_ActMove( NPCWORK *npc, int time ) ;
void NPC_ActLoopMotion( NPCWORK *npc, int time ) ;
void NPC_ActOneTimeMotion( NPCWORK *npc, int time ) ;

void NPC_ActThrow( NPCWORK *npc, int time ) ;
void NPC_ActHang( NPCWORK *npc, int time ) ;
void NPC_ActHangIdle( NPCWORK *npc, int time ) ;
void NPC_ActHangMove( NPCWORK *npc, int time ) ;
void NPC_ActHangStruggle( NPCWORK *npc, int time ) ;
void NPC_ActHangDamage( NPCWORK *npc, int time ) ;
void NPC_ActHangDamageNoMot( NPCWORK *npc, int time ) ;
void NPC_ActHangEscape( NPCWORK *npc, int time ) ;
void NPC_ActHangFree( NPCWORK *npc, int time ) ;
void NPC_ActHangDown( NPCWORK *npc, int time ) ;
void NPC_ActHangDie( NPCWORK *npc, int time ) ;
void NPC_ActFaint( NPCWORK *npc, int time ) ;
void NPC_ActFaintWakeup( NPCWORK *npc, int time ) ;
void NPC_ActFaintTake( NPCWORK *npc, int time  ) ;
void NPC_ActFaintMove( NPCWORK *npc, int time  ) ;
void NPC_ActFaintDown( NPCWORK *npc, int time  ) ;
void NPC_ActFaintDamage( NPCWORK *npc, int time  ) ;
void NPC_ActFaintEnd( NPCWORK *npc, int time ) ;
void NPC_ActDeath( NPCWORK *npc, int time ) ;

void NPC_ActStatusCheck( NPCWORK *npc ) ;
void NPC_Action( NPCWORK *npc ) ;
void NPC_InitAction( NPCWORK *npc, NPCACT *act ) ;
void NPC_SetActionParam( NPCACT *act, int life, int faint, int faint_max, int sleep_max, int down_dam ) ;
void NPC_SetHeartBeat( NPCWORK *npc, int beat ) ;
#endif
