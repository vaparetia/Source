/****
	"at_thk.h"
	攻撃兵独自思考構造体
	K.sigeno 2000.04.28
$Id: at_thk.h,v 1.1.1.3 2002/11/19 11:48:59 Yoshizawa1 Exp $
****/

#define	AT_THK_RATE	(BP_BASE_TICK())

/*ダメージ時のプレイヤ無敵時間　実体ではない　あくまで敵兵制御のための目安*/
#define	PL_MUTEKI_TIME		DIRECT_TICK(60)

#define	AT_ST_SQUAT			(0x0001) /*しゃがみ姿勢*/
#define	AT_ST_DEFENSE		(0x0002) /*守備位置設定済み*/
#define	AT_ST_DEFENSE_MOVE	(0x0004) /*守備時の移動攻撃制御*/
#define	AT_ST_FEEL			(0x0008) /*プレイヤの隣接ゾーンが見える*/
#define	AT_ST_NEXT			(0x0010) /*次のゾーンでは発見するぞ*/
#define	AT_ST_DODGE			(0x0020) /*待避位置設定済み*/
#define	AT_ST_WAKE			(0x0040) /*起こし兵決定済み*/
#define	AT_ST_LAST			(0x0080) /*行動可能で最後尾兵*/
#define	AT_ST_LOW_SQUAT		(0x0100) /*対匍匐攻撃*/
#define	AT_ST_IN_COVER		(0x0200) /*カバーエリア内にいる*/
#define	AT_ST_FEEL_LOW		(0x0400) /*しゃがみで隠れているのを察知*/
#define	AT_ST_ABS_ESCAPE	(0x0800) /*一目散に逃げる*/
#define	AT_ST_IN_BOUND		(0x1000) /*プレイヤへの攻撃に参加している*/
#define	AT_ST_NO_SHOT		(0x2000) /*発砲禁止命令発信*/
#define	AT_ST_TMP_TRG		(0x4000) /*一時目標設定 サブスタンスのみ*/


/*at_check.c shot系関数で参照 射撃制御フラグ*/
#define	AT_SHT_NORMAL		(0x00)	/*残弾判定無し*/
#define	AT_SHT_NO_RELOAD	(0x01)	/*残弾判定無し*/
#define	AT_SHT_BLIND		(0x02)	/*視界判定無し*/
#define	AT_SHT_NO_RAND		(0x04)	/*定間隔で発砲*/



/*行動不能なので隊列検索除外する条件*/
//#define AT_NO_ACTIVE (ACT_STATUS_FAINT|ACT_STATUS_DEATH)
/*気絶or死亡or転びor首閉め*/
/*死亡は消滅まで待つ*/
#define AT_NO_ACTIVE (ACT_STATUS_FAINT|ACT_STATUS_DOWN|ACT_STATUS_CAPTURE)
//#define AT_NO_ACTIVE (ACT_STATUS_FAINT|ACT_STATUS_CAPTURE)


/** 攻撃兵 行動パターン **/
#define	AT_THK_NO_FIRE	(0x10) /*発砲禁止*/



#define X_FIRE_DIR (128) /*前方兵との角座標で発砲禁止*/
//ENE_AlertGameLevel
/*各アクション解禁レベル*/
//#define	AT_ESC_LEVEL		(3)	/*逃げるレベル*/
#define	AT_ESC_LEVEL			(0)	/*逃げるレベル*/
#define	AT_SQUAT_LEVEL			(1)	/*しゃがみ攻撃使うレベル*/
//#define	AT_SURPORT_LEVEL	(2)	/*援護射撃するレベル*/
#define	AT_SURPORT_LEVEL		(3)	/*援護射撃するレベル*/
#define	AT_APROACH_LEVEL		(0)	/*先頭進行*/
//#define	AT_GRD_LEVEL		(AT_APROACH_LEVEL+1) /*グレネード攻撃するレベル*/
#define	AT_GRD_LEVEL		(0) /*グレネード攻撃するレベル*/

#define	AT_PRUDENCE_LEVEL	(0)	/*後方動き回り*/
#define	AT_PEEK_LEVEL	(0)	/* 実験用に最初から覗き攻撃*/

#define	AT_DYNAMIC_LEVEL	(6)	/*未ダイナミックエントリー*/


#define AT_MAX_LEVEL 8
#define AT_APPROACH_COUNT (AT_THK_RATE*10)/*接近威嚇攻撃の長さ*/

/*プレイヤが防衛地点に近づくと守備開始*/
#define AT_DEF_START_DIS_PL		(10000)
#define AT_DEF_START_DIS_ENE	(AT_DEF_START_DIS_PL+2000)
#define	AT_DIS_MIN	(2500) /*プレイヤに近づき過ぎると後退*/
/*他の兵が近づくと守備位置を空ける*/
#define AT_DEF_DODGE_DIS	(2000)
#define	AT_PL_HIDE	(PLAYER_INTRUDE|PLAYER_GROUND|PLAYER_LOCKER|PLAYER_BEYOND)

#define	AT_GRD_COUNT	(AT_THK_RATE*10*8)
#define	AT_DYN_COUNT	(AT_THK_RATE*10*15)

#define	AT_ROLL_COUNT	(AT_THK_RATE*10*8)  /*転がり間隔*/
#define	AT_WAIT_COUNT	(AT_THK_RATE*10*4)  /*待ち時間デフォルト*/


#define BEYOND_DIS (500.0f)



/*(u_int) at_com.watch_statusにセット*/
/*攻撃兵 STATUS*/
/*コマンダから全体制御に使用*/
#define AT_COM_WATCH_INTRUDE	(0x0001) /*イントルード瞬間見た*/
#define AT_COM_WATCH_BEYOND		(0x0002) /*ぶら下がり瞬間見た*/
#define AT_COM_WATCH_LAST		(0x0004) /*見失い地点に到達*/
#define AT_COM_WATCH_SEARCH		(0x0008) /*探索開始命令*/
#define AT_COM_WATCH_HOSTAGE	(0x0010) /*仲間が捕まってるの発見*/
#define AT_COM_WATCH_DEATHBED	(0x0020) /*仲間の死に際を見た*/
#define AT_COM_WATCH_APPROACH	(0x0040) /*接近状態*/
#define AT_COM_WATCH_COVER		(0x0080) /*遮蔽モード*/
#define AT_COM_WATCH_LOCKER		(0x0100) /*ロッカーモード*/
#define AT_COM_WATCH_NO_AVOID	(0x0200) /*行き止まりに追い詰めた*/
#define AT_COM_WATCH_NO_SHOT	(0x0400) /*発砲禁止*/



#define AT_COM_SCN_STATUS_WARP		(0x0001) /*ワープ可能*/


#define AT_SURP_TIME		COUNT_VMODE(120)	/*ビックリ間隔*/



/*気絶兵 消し時間*/
#define RETIRE_NO_CHECK_TIME	(180)
#define RETIRE_CHECK_TIME		(120)
#define RETIRE_CHECK_TIME_ABS (RETIRE_NO_CHECK_TIME+RETIRE_CHECK_TIME)
#define RETIRE_CHECK_HALH	(120)


typedef struct {
	FVECTOR	subtrg;		/** プレイヤ以外の目標座標 **/
	int		escaddr;	/** 後退時目標ゾーン **/ 
	int		safeaddr;	/** 使用可能安地 **/
	u_int	safetype;	/** 指定された安地のタイプ **/
	int		found_addr ; /*攻撃兵が最後に見たプレイヤ*/

	short	at_tmptime;	/** 攻撃兵汎用タイム **/
	short	form_dir; 	/** コマンダからの指定位置 **/

	int		dis_dif;	/** 味方とのゾーン距離 **/
	int		zone_dis;	/** ゾーン探索による距離**/

	short	at_status;	/** 攻撃兵専用ステイタス**/
	short	sight_time; /** 敵兵がプレイヤに見られてる時間 **/

	long64	front_st;	/** 手前兵の状態 **/

	char	equip_req;	/** 装備品リクエスト**/
	char	dis_rank;	/** 距離順位 **/
	/*sight_timeと間違えるな*/
	u_short		in_sight;	/** プレイヤを連続補足してる時間 **/

	short	life_buf ;
	char	th2_buf ; /*シンクモードの行き先予約*/
	char	th3_buf ;

	int		def_addr ;	/*守備位置のアドレス*/
	FVECTOR	def_pos2 ;	/* 守備位置を空けるときの待避場所 */

	int		surprised ; /*ビックリするものを見た　個別管理用*/

	FVECTOR *ef_pos ;	/*付随型エフェクト座標*/
	int		blur_sw ;	/*付随型エフェクト 表示制御*/
} AT_THK ;

enum {
SHL_ST_ENE_DEATH = -2,	/*持ってる人が死んだ*/
SHL_ST_BREAK,			/*壊れた瞬間を敵本体に通知*/
SHL_ST_NOP,				/*盾 無効*/
SHL_ST_ACTIVE,			/*盾 有効*/
SHL_ST_MAKE_NEW ,
} ;
