/*
    vamp.h

	$Id: vamp.h,v 1.1.1.3 2002/11/19 11:48:56 Yoshizawa1 Exp $

*/
#ifndef VAMP_H
#define VAMP_H


#define CLOCK_COUNT	(BP_BASE_TICK())
#define COUNT_VMODE(_a) (BP_AdjustTick(_a))

#define	SET_FLAG( a,b )		((a) |= (b))
#define	CHECK_FLAG( a,b )	((a) & (b))
#define	UNSET_FLAG( a,b )	((a) &= ~(b))
#define	CLEAR_FLAG( a ) 	((a) = 0)

#define	BODYWORLD(_body,_index) (((OBJECT*)_body)->objs->objs[(_index)].world)

#define	ACT_INTERP_FAST			(4*5)	// 1/300単位
#define	ACT_INTERP_DEF			(8*5)	// 1/300単位
#define	ACT_INTERP_M			(60*1)	// 1/300単位
#define	ACT_INTERP_SLOWLY		(60*5)	// 1/300単位
#define	ACT_INTERP_VERY_SLOWLY	(60*30)	// 1/300単位

//#define	TIME_BASE				(5)	// 1/300単位

#define CHILD_TARGET_NUM	(12)
#define VAMP_LIFE			(128)
#define VAMP_O2				(2048)
#define VAMP_FAINT			(10)
#define VAMP_FAINT_COUNT	(60*30)
#define VAMP_SLEEP_COUNT	(60*60*1 + 3)
#define VAMP_DOWN_DAM		(10)
#define VAMP_LIFEGAGE_LEVEL	(1)
#define VAMP_O2GAGE_LEVEL	(1)
#define HUMAN_JOINT			(21)

#define ALL_KNF_NAME		(GV_StrCode( "鞘とグリップ" ))
#define PARTS_KNF_NAME		(GV_StrCode( "鞘" ))

//思考側からアクションへのメッセージ
enum {
	THINK_MESG_NONE = 0,
	THINK_MESG_STAND,
	THINK_MESG_LOCKON,
	THINK_MESG_MUTEKI,
	THINK_MESG_MOVE,
	THINK_MESG_SLICE,
	THINK_MESG_THOROWL,
	THINK_MESG_THRWL_MUTEKI,
	THINK_MESG_THOROWL3,
	THINK_MESG_THRWMANY,
	// 10 
	THINK_MESG_THRWSHDW,
	THINK_MESG_MOVTHRWL,
	THINK_MESG_DIVE,
	THINK_MESG_DIVELOFT,
	THINK_MESG_JUMPOUT,
	THINK_MESG_JUMPOUTLOFT,
	THINK_MESG_SWIM,
	THINK_MESG_WALK,
	THINK_MESG_AVOID,
	THINK_MESG_AVOIDFULL,
	// 20
	THINK_MESG_ONESPIN,
	THINK_MESG_LOFTMOVE,
	THINK_MESG_LOFTJUMP,
	THINK_MESG_SLASH,
	THINK_MESG_SLASHSHDW,
	THINK_MESG_JUMPOUTWATER,
	THINK_MESG_FLOORJUMP,
	THINK_MESG_TOLOFTJUMP,
	THINK_MESG_TOFLOORJUMP,
	THINK_MESG_LOFTLOFTJUMP,
	// 30
	THINK_MESG_ONETIME,
	THINK_MESG_LONGRUN,
	THINK_MESG_NEARJUMP,
	THINK_MESG_SPECIAL,
	THINK_MESG_SLASHNEAR,
	THINK_MESG_WATEROUTWATER,
	THINK_MESG_SPIDER,
	THINK_MESG_TOSPIDERWATER,
	THINK_MESG_TOWATERSPIDER,
	THINK_MESG_TURNEND,
	// 40
	THINK_MESG_DIZZY,

	THINK_MESG_DEADEND,
	THINK_MESG_DAMAGE,
	THINK_MESG_SPIDERDAM,	

	THINK_MESG_JUMPOUTHNDRAIL,
	THINK_MESG_STANDTURN,
	THINK_MESG_PROVO,
	THINK_MESG_GROUND_ATT,

	//新ジャンプ関係
	THINK_MESG_LOFT2LOFT,
	THINK_MESG_LOFT2RAIL,
	THINK_MESG_LOFT2FLOOR,
	THINK_MESG_LOFT2WATER,
	
	THINK_MESG_RAIL2RAIL,
	THINK_MESG_RAIL2FLOOR,
	THINK_MESG_RAIL2WATER,

	THINK_MESG_FLOOR2LOFT,
	THINK_MESG_FLOOR2RAIL,
	THINK_MESG_FLOOR2FLOOR,

	THINK_MESG_WATER2LOFT,
	THINK_MESG_WATER2RAIL,
	THINK_MESG_WATER2WALL,
	
	THINK_MESG_ONESPINLOOP0,
	THINK_MESG_SPINLOOP0,
	THINK_MESG_ONESPINLOOP1,
	THINK_MESG_SPINLOOP1,

	THINK_MESG_SPIN,

	THINK_MESG_LOWBOW,
	THINK_MESG_LOWBOW_CANBREAK,
	THINK_MESG_RAPIDAVOID,
	THINK_MESG_INTOWATER,		//水上から水中へ

	THINK_MESG_FLR2FLR_F,
	THINK_MESG_FLR2WLL,

	THINK_MESG_FLR2WRT_B,
	
};

//モーションリスト
enum {
	//待ち
	VMP_MOT_NON_WAIT = 0,	//待ち
	VMP_MOT_KNF_WAIT,		//待ち
	VMP_MOT_RIL_WAIT,		//待ち
	//移動
	VMP_MOT_NON_WALK,			//歩き（仮）
	VMP_MOT_NON_MOVE,			//移動
	VMP_MOT_KNF_MOVE,			//移動
	VMP_MOT_KNF_RAIL,			//移動	
	VMP_MOT_NON_SWIM,			//泳ぎ
	VMP_MOT_NON_JUMPOUT,		//水から床
	VMP_MOT_NON_JUMPOUT30,		//水から床30
	//10
	VMP_MOT_NON_DIVE,			//床から水
	VMP_MOT_KNF_JUMP,			//ジャンプ
	VMP_MOT_KNF_SHDWATT_S,		//ジャンプ
	VMP_MOT_KNF_SHDWATT_L,		//ジャンプ
	VMP_MOT_KNF_WATER2WATER,	//飛び出し投げ
	VMP_MOT_NON_WATER2LOFT,		//飛び出し
	VMP_MOT_NON_FLOOR2LOFT,		//飛び出し
	VMP_MOT_NON_LOFT2WATER,		//飛び出し
	VMP_MOT_NON_WATER2RAIL,		//飛び出し
	//新飛び出し
	VMP_MOT_NON_START_WATER,
	//20
	VMP_MOT_NON_START_FLOOR,
	VMP_MOT_NON_START_RAIL,
	VMP_MOT_NON_START_LOFT,
	VMP_MOT_NON_START_WALL,

	VMP_MOT_NON_END_WATER,
	VMP_MOT_NON_END_FLOOR,
	VMP_MOT_NON_END_RAIL,
	VMP_MOT_NON_END_LOFT,
	VMP_MOT_NON_END_WALL,
/*
vmp_non_start_water2loft
vmp_non_start_floor2loft
vmp_non_start_rail2loft
vmp_non_start_loft2loft
vmp_non_start_wall2loft

vmp_non_end_water
vmp_non_end_floor
vmp_non_end_rail
vmp_non_end_loft
vmp_non_end_wall
*/
	VMP_MOT_KNF_FLR2FLR_F,
	// 30
	VMP_MOT_KNF_FLR2FLR_B,
	VMP_MOT_KNF_SPINJUMP,		//スピンジャンプ
	VMP_MOT_KNF_FLR2WLL,
	VMP_MOT_KNF_WLL2WTR_E,
	VMP_MOT_KNF_WLL2WTR_F,
//vmp/vmp_jump_short_f.mt3		vmp_knf_flr2flr_f
//vmp/vmp_jump_short_b.mt3		vmp_knf_flr2flr_b
//vmp/vmp_jump_floor2wall.mt3		vmp_knf_flr2wll
//vmp/vmp_escape_wall.mt3			vmp_knf_wll2wtr_e
//vmp/vmp_dam_wall_fall.mt3		vmp_knf_wll2wtr_f

	//攻撃
	VMP_MOT_KNF_SLICE,		//ナイフ切り
	VMP_MOT_KNF_THOROW,		//ナイフ投げ
	VMP_MOT_KNF_SLASH,		//中距離
	VMP_MOT_KNF_STOMP,		//踏みつけ

	//ダメージ
	//VMP_MOT_KNF_GUNDAM,		//銃ダメージ
	VMP_MOT_KNF_DAM_HEAD,		//ダメージ頭
	VMP_MOT_KNF_DAM_FLANK,		//ダメージわき腹
	VMP_MOT_KNF_DAM_FRONT,		//ダメージ前から
	VMP_MOT_KNF_DAM_BACK,		//ダメージ後ろから
	VMP_MOT_KNF_DAM_CROUCH,		//ダメージしゃがみ時
//vmp/vmp_dam_crouch.mt3			vmp_knf_dam_crouch
	
	VMP_MOT_KNF_DAM_WALL,		//壁ダメージ
	VMP_MOT_KNF_DAM_BLOW,		//爆発ダメージ
//vmp/vmp_dam_wall.mtn			vmp_knf_dam_wall
//vmp/vmp_dam_blow.mtn			vmp_knf_dam_blow
	
	//特殊
	VMP_MOT_KNF_AVOID,		//緊急回避（仮）
	VMP_MOT_NON_SPINJUMP,	//スピンジャンプ（仮）
	VMP_MOT_NON_GRASP,		//ナイフ抜き
	VMP_MOT_NON_RETURN,		//ナイフ差し
	VMP_MOT_NON_LONGRUN,	//ダンス投げ
	VMP_MOT_NON_SPECIAL,	//スチャッ→クルクルヘイ！
	VMP_MOT_NON_TURNEND,	//くるっ！パンパン
	VMP_MOT_NON_END,		//

	VMP_MOT_NON_SPIDER,		//張り付き
	VMP_MOT_NON_PROVO,		//挑発

	VMP_MOT_NON_SPIN_START,		//
	VMP_MOT_NON_SPIN_LOOP0,		//
	VMP_MOT_NON_SPIN_LOOP1,		//
	VMP_MOT_NON_SPIN_END,		//

	VMP_MOT_NON_SPDR_ATT,	//張り付き攻撃

	VMP_MOT_KNF_THROW_R,		//ナイフ投げ
	VMP_MOT_KNF_THROW_L,		//ナイフ投げ
	
	VMP_MOT_KNF_RAPID_AVOID,	//連続回避
	VMP_MOT_KNF_LOWBOW,			//お辞儀
	VMP_MOT_KNF_INWATER,			//水の上から水へ

	VMP_MOT_KNF_DIZZY,				//ふらつき
	VMP_MOT_KNF_DAM_LAST_FLR,		//ラストダメージ床
	VMP_MOT_KNF_DAM_LAST_LFT,		//ラストダメージ２階
/*	
vmp/vmp_spin_dizzy.mt3				vmp_knf_dizzy
vmp/vmp_bow.mt3						vmp_knf_bow
vmp/vmp_handclap.mt3				vmp_knf_handclap
vmp/vmp_dance.mt3					vmp_floordance
*/
};

#define ZONE_INFO_SAME			(0)
#define ZONE_INFO_NEXT			(1)
#define ZONE_INFO_NEAR			(2)
#define ZONE_INFO_LINE			(3)
#define ZONE_INFO_TOIMEN		(4)
#define ZONE_INFO_CONTRAST		(5)
#define ZONE_INFO_FAR			(6)

#define VMP_FLAGS_STOP			(0x00000001)
#define VMP_FLAGS_GROUND		(0x00000002)
#define VMP_FLAGS_BEYOND		(0x00000004)
#define VMP_FLAGS_SQUAT			(0x00000008)
#define VMP_FLAGS_WATCH			(0x00000010)
#define VMP_FLAGS_LOCKON		(0x00000020)
#define VMP_FLAGS_FIRE			(0x00000040)
#define VMP_FLAGS_LOFT			(0x00000080)	// ２階
#define VMP_FLAGS_RAIL			(0x00000100)	// １階手摺

#define VMP_FLAGS_NIKITA		(0x00000200)	// ニキータ
#define VMP_FLAGS_STG			(0x00000400)	// スティンガー
#define VMP_FLAGS_MISS_INSIGHT	(0x00000800)	// ミサイル系視界に入った
#define VMP_FLAGS_CAN_SPPATT	(0x00001000)	// びっくり攻撃可
#define VMP_FLAGS_KNFATT		(0x00002000)	// ナイフ攻撃した
#define VMP_FLAGS_NO_ZONE		(0x00004000)	// プレイヤがゾーン内にいない
#define VMP_FLAGS_NO_BLOW		(0x00008000)	// 吹っ飛ばされない




#define VMP_STATUS_STAND		(0x00000001)	// 立ち
#define VMP_STATUS_MOVE			(0x00000002)	// 移動
#define VMP_STATUS_DAMEGE		(0x00000004)	// ダメージ処理によりactionが奪われ中
//#define VMP_STATUS_NO_BLOW		(0x00000008)	// 吹っ飛ばされない
#define VMP_STATUS_NO_SETACT	(0x00000010)	// モーション割り込み禁止
#define VMP_STATUS_NO_DIR		(0x00000020)	// 方向入力禁止
#define VMP_STATUS_ATTACK		(0x00000040)	// 攻撃中
#define VMP_STATUS_ACT_END		(0x00000080)	// action終了
#define VMP_STATUS_SWIM			(0x00000100)	// 泳ぎ
#define VMP_STATUS_WAIT			(0x00000200)	// ウェイト
#define VMP_STATUS_AVOID		(0x00000400)	// 回避
#define VMP_STATUS_BLAST		(0x00000800)	// 爆発にまきこまれん
#define VMP_STATUS_NOWAVOID		(0x00001000)	// 回避中
#define VMP_STATUS_FLY			(0x00002000)	// 飛んでます
#define VMP_STATUS_ADD_STEP		(0x00004000)	// ステップ値に加える
#define VMP_STATUS_MUL_STEP		(0x00008000)	// ステップ値スケール
#define VMP_STATUS_SPIDER		(0x00010000)	// 張り付き中
#define VMP_STATUS_MOTAVOID		(0x00020000)	// モーションに回避アリ
#define VMP_STATUS_ACT_SEMIEND	(0x00040000)	// action終了
#define VMP_STATUS_CAN_BREAK	(0x00080000)	// 割り込みＯＫ
#define VMP_STATUS_STOMP		(0x00100000)	// 割り込みの踏みつけ
#define VMP_STATUS_MISS			(0x00200000)	// 割り込みのミサイル系
#define VMP_STATUS_SPPATT		(0x00400000)	// 割り込みのびっくり攻撃
#define VMP_STATUS_RESET_H		(0x00800000)	// ctrl->height書き換え
#define VMP_STATUS_RAPIDAVOID	(0x01000000)	// 連続避け発動中
#define VMP_STATUS_THRWKNF		(0x02000000)	// ナイフ投げた
#define VMP_STATUS_BLOW			(0x04000000)	// 吹っ飛ばされー
#define VMP_STATUS_AVOID2		(0x08000000)	// 回避2
#define VMP_STATUS_GO_LAND		(0x10000000)	// 着地へ
#define VMP_STATUS_DEATH		(0x20000000)	// 死亡

#define VMP_STATUS_DAM_MUL		(0x40000000)	// ダメージ率補正


//#define VMP_STATUS_FAINT		0x00000008	// 気絶居眠り中
//#define VMP_STATUS_IK_FOOT		0x00000020	// 引き摺り足ＩＫ
//#define VMP_STATUS_IK_HAND		0x00000040	// 引き摺り手ＩＫ
//#define VMP_STATUS_IK_PIKU		0x00000080	// ダウンピクピクＩＫ
//#define VMP_STATUS_TRG_OFF		0x00000100	// ターゲットオフ
//#define VMP_STATUS_FAINT_END	0x00000200	// 気絶居眠り終了
//#define VMP_STATUS_HANG			0x00000800	// 首締められ中

//#define VMP_STATUS_EYE_CLOSE	0x00002000	// 視界ＯＦＦ
//#define VMP_STATUS_CAPTURE_OFF	0x00004000	// 掴みターゲットオフ

//#define VMP_STATUS_STAND		(0x0001)
//#define VMP_STATUS_MOVE		(0x0002)
//#define VMP_STATUS_DAMEGE		(0x0004)
//#define VMP_STATUS_SLICE		(0x0008)
//#define VMP_STATUS_THOROW		(0x0010)

#define VMP_ACTFLAG_TURN			(0x0001)
#define VMP_ACTFLAG_SHDW			(0x0002)
#define VMP_ACTFLAG_CHANGE_MUL		(0x0004)
#define VMP_ACTFLAG_ENDSPIN			(0x0008)
#define VMP_ACTFLAG_ONESPIN			(0x0010)
#define VMP_ACTFLAG_SPINATT			(0x0020)
#define VMP_ACTFLAG_NXTAVOID		(0x0040)
#define VMP_ACTFLAG_RPDDAM			(0x0080)
#define VMP_ACTFLAG_COUNTER			(0x0100)
//#define VMP_ACTFLAG_DEAD			(0x0200)

//反応とダメージ値が同じ物をまとめる
enum {
	VMP_WPDM_PUNCH = 0,		//パンチ	（親にくる）
	VMP_WPDM_KICK,			//キック	（親にくる）
	VMP_WPDM_BLAST,			//爆発		（親にくる）
	VMP_WPDM_STUN,			//スタン

	VMP_WPDM_M9,			//m9
	VMP_WPDM_PSG1,			//ライフル
	VMP_WPDM_BULLET,		//弾丸
	VMP_WPDM_BLADE,			//刀
	VMP_WPDM_SPRAY,			//スプレー

	VMP_WPDM_AVOID1,		//連続回避へ
	VMP_WPDM_AVOID2,		//移動回避へ
	VMP_WPDM_RPD_AVD,		//さらに連続避けして

	VMP_WPDM_SPDR_DAM,		//張り付き時ダメージ
	VMP_WPDM_SWIM_DAM,		//水中ダメージ

	VMP_WPDM_DAM_NONACT,	//ダメージモーション発動しない
};

//#define VMP_DAMFLAG_CHANGE


//dmg_flags
//[ 0 <->  3] 0<->15 ダメージ種類  	4
//[ 4 <->  8] 0<->32 間接			5
//[ 9 <-> 14] 0<->64 ダメージ値		6
//[15 <-> 20] 0<->64 気絶値			6
//[21 <-> 24] 0<->15				4

#define VMP_SET_DMFLAG( _k, _j, _d, _f, _i ) \
				( ( ((_k)&0xf) )|( ((_j)&0x1f)<<4 )|( ((_d)&0x3f)<<9 )|( ((_f)&0x3f)<<15 )|( ((_i)&0x0f)<<21 )  )

#define VMP_GET_DMFLAG_KINDS( _data )	( (_data)      & 0x0f)
#define VMP_GET_DMFLAG_JOINT( _data )	(((_data)>> 4) & 0x1f)
#define VMP_GET_DMFLAG_DAMGE( _data )	(((_data)>> 9) & 0x3f)
#define VMP_GET_DMFLAG_FAINT( _data )	(((_data)>>15) & 0x3f)
#define VMP_GET_DMFLAG_INDEX( _data )	(((_data)>>21) & 0x0f)


#define N_MAX_VMP_VOX	(16)

typedef	struct _WORK{
	GV_ACT_EX		actor;

    // システム用データ
    CONTROL			control;						// 移動制御
    OBJECT			body;							// モデル
    FMATRIX			lights[2];						// ライト
    TARGET			target;							// 防御 部位ターゲット
    POWER_TARGET	power;							// 攻撃
    TARGET			def_child[CHILD_TARGET_NUM];	// 子ターゲット
    TARGET			def_water;						// 水中の当たり
    HOMING_TRG		homing;							// ホーミング部位ターゲット(現在 BODY21_ATAMA)
    ALIGN16_PRE RADAR_CTRL		radar ALIGN16_POST;					// レーダー
	//int				radar_pad;					// レーダーのパディング
	NAVIGATE		navigate;						//ナビ
	NAVITARGET		navitrg;						//ナビ
    GM_GageSet		gage;							// ライフ
    GM_GageSet		oxygen;							// Ｏ２

	DG_OBJS			*shdw_objs;
	void			*ik_work;						//IKワーク
	short			life,faint;						// ライフと酸素
	int				name;							//なまえ
	int				map;							//マップ
	int				sys_timer;						// タイマー
	int				o2;
#if 0
	//テスト
    OBJECT			body1;							// モデル
    OBJECT			body2;							// モデル
#endif
	// モーション関係
	void			(*ActFunc_call)( void*, int );		//モーションアクト関数
	void			(*Next_ActFunc_call)( void*, int );		//モーションアクト関数
	
	float			old_body_height;					//前の高さ
	float			reset_height;						//リセット用高さ
	short			dir;								//向きたい向き
	short			dir_x;
	short			dir_z;
	short			inv_timer;
	short			adjust_rot_x;						//アジャスト用Ｘ軸回転
	u_short			act_flags;
	char			active_mot;							//今のモーション
	char			next_mot;							//次のモーション
	char			land_mot;							//着地モーション
	char			main_mot;							//メインモーシ

	int				act_timer;							//モーション用タイマー
	int				status;								//ただいまの状況
	int				adjust_flags;						//アジャストフラグ
	u_int			dmg_flags;							//ダメージ状況

	short			n_spin0;							//スピン０の回転数
	short			n_spin1;							//スピン１の回転数

	ALIGN16_PRE SVECTOR			to_adjust[HUMAN_JOINT] ALIGN16_POST;		//目指すアジャスト
	ALIGN16_PRE SVECTOR			from_adjust[HUMAN_JOINT] ALIGN16_POST;	//今のアジャスト(こいつはアライン気をつける)

	// 思考関係
	short			think_main;			//メインステップ
	short			think_sub;			//サブステップ
	short			think_local;		//ローカルステップ
	short			think_pre_sub;		//前のローカルステップ
	short			think_turn;			//２階にいる時右回りか左か
	short			think_n_att;		//連続攻撃回数
	int				think_timer;		//思考タイマー
	int				think_now_main_w;	//
	int				think_now_main_f;	//
	
	int				think_flags;		//各種状況
	int				think_mesg;			//思考側からアクトへのモーション受け渡し
	int				think_dir;			//プレイヤーの方向
	u_short			think_ntbl;
	u_short			think_max_att;
	short			think_temp0;		//異なる思考ステップ間の保存の保証なし
	short			think_temp1;		//異なる思考ステップ間の保存の保証なし
	short			think_temp2;		//異なる思考ステップ間の保存の保証なし
	short			think_temp3;		//異なる思考ステップ間の保存の保証なし

	short			think_p_fzn;		//プレイヤーがいる場所(床)
	short			think_v_fzn;		//俺様がいる場所(床)
	short			think_to_fzn;		//目的の場所（床）
	short			think_p_wzn;		//プレイヤーがいる場所(水)
	short			think_v_wzn;		//俺様がいる場所(水)
	short			think_to_wzn;		//目的の場所（水）
	short			think_pre_p_fzn;	//プレイヤーがいた場所(床)
	short			think_pre_v_wzn;	//俺様がいた場所(水)
	
	short			think_dive_pos;		//飛び込み位置
	short			think_jumpout_pos;	//飛び出し位置
	short			think_damnum;		//連続ダメージ回数
	short			think_damtime;		//ダメージタイマー
	short			think_raitimer;		//ライデン止ってるか判定用タイマー
	short			think_v_loft;		//俺様がいる場所(２階のＩＤ)
	short			think_wait_time;
	short			think_move_cnt;
	short			think_ground_timer;
	short			think_beyond_timer;
	
	float			think_len;					//プレイヤーとの距離
	float			spin_scale_vec;				//回転時の移動量
	int				think_tmp_max_att;

	float			think_mul_x;
	float			think_mul_y;
	float			think_mul_z;

//	long64			think_firekind ALIGN16;		//ライデンが撃った物の種類
	FVECTOR			think_mul_step;				//ステップ値のスケール
	FVECTOR			think_mot_step;				//移動無視モーション時のステップ
	FVECTOR			think_init_pos;				//
	// セクシーナイフ
    TARGET			knf_trgt;			// ナイフ防御
    POWER_TARGET	knf_pow;			// ナイフ攻撃
	FVECTOR			knf_pos[2];			// エフェクト用
	FVECTOR			knf_att_pos[2];		// 攻撃用
	int				knf_alpha;
	DG_OBJS			*knf_objs;			// ナイフオブジェ
	FMATRIX			*knf_root;			// ナイフroot

	TARGET			att_trgt;			// 汎用攻撃ターゲット
    POWER_TARGET	att_pow;			// 汎用攻撃ターゲット
	FVECTOR			off_force;
	FVECTOR			off_hit;
	// ストーリーミング関係
	int				vox_id[N_MAX_VMP_VOX];
	int				vox_len[N_MAX_VMP_VOX];
	short			vox_count[N_MAX_VMP_VOX];
	short			now_vox,pre_vox;
	int				vox_timer;
	int				n_vox;
	int				stream_handler;
	int				last_call;				//最後に呼んだ音のフェーズ
	short			dam_count;
	short			dam_timer;
	short			dam_max_count;
	short			piku_timer;

	short			clmr_timer;
	short			pad0;
	int				breath;
	int				end_proc;			//エンドプロック
	int				game_diff;			//難易度
	void			*holawork;			//
	void			*motblur;
	int				motblur_flag;

	void			*all_knf_work;
	void			*knf_work;
	int				vib_time;
	short			wtr_blast_count,wtr_blast_timer;
	
} Work;

//モーション関係
//vmp_action.c
extern void VMP_PreAct( Work *work );
extern void VMP_AfterAct( Work *work );
extern void VMP_InitAction( Work *work );

//思考関係
//vmp_think.c
extern void VMP_ThinkInit( Work *work );
extern void VMP_PreThink( Work *work );
extern void VMP_AfterThink( Work *work );
extern void VMP_ThinkMain( Work *work );
extern void ThrowKnife( Work *work, FVECTOR *pos, FVECTOR *hit, FVECTOR *shift, int index, int mode, int shdw );

//ヴァンプゾーン関係
//vmp_zone.c
extern void VMP_DebugZoneView( int p, int v, int, int );
extern int VMP_GetZoneNumFromPosFloor( FVECTOR *pos, int pre_num );
extern int VMP_GetZoneInfo( int a, int b );		//a->bのinfo
extern int VMP_GetDivePosNum( int a );
extern void VMP_DivePos( FVECTOR *out, int a );
extern void VMP_JumpOutPos( FVECTOR *out, int a );
extern int VMP_RaiMayBeNextZone( int now, int pre, int flag );
extern int VMP_GetFromZoneToZoneDirFloor( FVECTOR *from_vec, int from, int to, float len, FVECTOR *to_pos );
extern int VMP_GetRndZone( int a, int info );
extern int VMP_GetRndZoneNoSame( int a, int info, int now );
extern int VMP_GetZoneNumFromPosWater( FVECTOR *pos, int pre_num );
extern int VMP_GetFromZoneToZoneDirWater( FVECTOR *from_vec, int from, int to, float len, int pre );
extern int VMP_GetNextIndexZone( int a, int index );
extern int VMP_GetNearWaterZoneFromFloor( int a, int pre );
extern int VMP_GetLoftJumpOutPos( FVECTOR *out, FVECTOR *pos, int wzn );
extern int VMP_GetLoftJumpOutPos2( FVECTOR *out, FVECTOR *pos, int wzn );
extern float VMP_GetSpinScale( FVECTOR *pos, int num );
extern int VMP_GetLoftDir( short *pdir, short *pv, FVECTOR *pos, int mode );
extern void VMP_GetLoftFixPos( short v, FVECTOR *pos );
extern void VMP_GetLoftFixPosEx( short v, FVECTOR *pos  );
extern void VMP_GetLinePos( FVECTOR *out, int a );
extern int VMP_GetWaterJumpOutPos( FVECTOR *out, int wzn );

extern int VMP_GetToShdwAttNum( int wzn );
extern int VMP_GetRndZoneFar( int p, int v, int info, int far );
extern int VMP_GetLinePosFromFloorLoft( FVECTOR *out, int pf, int vl, FVECTOR* );
extern int VMP_GetLinePosFromFloorLoft_long( FVECTOR *out, int pf, int vl );

extern int GetNearAttRndPos( FVECTOR *out, FVECTOR *player, int map );

extern int GetGoSplashWaterNum( int wzn );
extern int GetGoSplashWaterNum_1st( int to_wzn );
extern int GetGoSplashWaterNum_2nd( int to_wzn );

extern int GetTriAttNum( FVECTOR *pos );
extern int GetTriAttDammyNum( int tri_att_num );
extern void GetTriAttPosition( FVECTOR *out, int tri_att_num );

extern void GetHndRailPosition( FVECTOR *out, int hndrl_num );
extern int GetHndRailPositionRnd( FVECTOR *out, int hndrl_num );
extern int GetHandRailDammyNum( int hndrl_num );
extern int VMP_GetHndRailDir( FVECTOR *pos, int hndrl_num, int turn );


extern void GetLoftPosition_Temp( FVECTOR *out, int loft_num );
extern int VMP_GetLoftDir_Temp( FVECTOR *pos, int loft_num, int turn );
extern void VMP_GetLoftFixPos_Temp( int loft_num, FVECTOR *pos );


extern int VMP_GetZoneLen( int a, int b );
extern int VMP_GetZoneRndFromLen( int p, int len, int v );


extern int VMP_GetTurnToPos( FVECTOR *pos, FVECTOR *to, int vl );

extern int VMP_GetZoneNumFromForce( FVECTOR *out, FVECTOR *pos, FVECTOR *pforce );
extern void VMP_GetBlowWaterPos( FVECTOR *out, FVECTOR *pos, FVECTOR *pforce );
extern int VMP_GetLoftDirNoChangeLoftNum( short *pdir, short *pv, FVECTOR *pos, int mode );
//その他もろもろ
//vmp_etc.c
extern void SetAjustToPlayer( Work *work );
extern void ClearAjustToPlayer( Work *work );
extern void GetInitialVelocity( FVECTOR *step, FVECTOR *from, FVECTOR *to, float time );
extern int GetOutRandNum( int from, int to, int out );

//投げナイフ
//vmp_knife.c
extern void VmpThrowKnifeSet( FVECTOR *from, FVECTOR *to, int *map, int mode );

//影ターゲット
//vmp_shdwtrgt.c
extern void VmpShdwClear();
extern int VMP_CheckShdw();
extern void VMP_GetShdwPos( FVECTOR *shdw );
extern int VMP_CheckShdwBind();

/*vamp.c sigeno追加*/
extern int VMP_LifeCheck ;


#ifdef DEBUG_MODE
#define TS_DEBUG_MODE
#endif

#ifdef TS_DEBUG_MODE

#ifdef PSX2 //yano
#define VMP_PRINTF( fmt... )	 printf("vamp: " ##fmt )
#else //
#include <stdarg.h>
static inline void VMP_PRINTF( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, f );
	vprintf( f, argptr );
	va_end( argptr );
}
#endif //

#define	PRINT_PFVEC(_i,_fv) \
			printf("vamp: [%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)
#else

#ifdef PSX2 //yano
#define VMP_PRINTF( ... )
#else //
#include <stdarg.h>
static void inline VMP_PRINTF( char *f, ... )
{
	va_list		argptr ;
	va_start( argptr, f );
	//vprintf( f, argptr );
	va_end( argptr );
}
#endif //

#define	PRINT_PFVEC(_i,_fv)
#endif
#endif
