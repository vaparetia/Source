/*
	eve_a.h
	長廊下兵&コマンダ 共通宣言

	2000/03/23 K.Sigeno
	$Id: eve_a.h,v 1.1.1.3 2002/11/19 11:49:10 Yoshizawa1 Exp $
*/

#define	EVE_A_OBSERVE	(0x01)	/*目撃した本人 危険レベルに関係なく攻撃*/
#define	EVE_A_DEATH			(0x02)	/*死んでる*/
#define	EVE_A_WAIT			(0x04)	/*物陰で待機*/
#define	EVE_A_SUPORT_WAIT	(0x08)	/*応援コールまで待機*/
#define	EVE_A_NO_DEF		(0x10)	/*追いかけモード*/
#define	EVE_A_ATTAK_END		(0x20)	/*攻撃済み*/

#define		DEF_DYNAMIC_SIGHT	(2500)

/*攻撃兵コマンダ 守備モード管理*/
#if 1
enum {
	AT_COM_DEF_NORMAL,	
	AT_COM_DEF_WAIT,	/*待機中*/
	AT_COM_DEF_DYNAMIC	/*全員突撃*/
};
#endif

/*長廊下コマンダーのモード*/
enum {
	DEF_COM_TYPE_A ,
	DEF_COM_TYPE_B
};
/*長廊下兵のモード*/
enum {
	DEF_TYPE_A ,
	DEF_TYPE_B ,
	DEF_TYPE_C ,
	DEF_TYPE_D
};
//アナザー追加処理
enum {
	ANOTHER_NORMAL	= 0 ,
	ANOTHER_DARK_CAMP = 1 ,
};

/*長廊下コマンダのステータス*/
#define	DEF_COM_ST_RNDPOS	(0x0001) //守備位置選定優先がランダム



/*イベント時の守備ポイント最大数*/
//#define EVENT_DEF_POINT_MAX (8)
#define EVENT_DEF_POINT_MAX (16)
#define	EVENT_SUPPORT_TIME	DIRECT_TICK(60*30)

#if 0
#define		DEF_MODE_CNT	DIRECT_TICK(60*30)
#define		DEF_GRD_CNT		DIRECT_TICK(60*15)	/*左舷*/
#else
#define		DEF_MODE_CNT	DIRECT_TICK(60*15)
#define		DEF_GRD_CNT		DIRECT_TICK(60*5)	/*左舷*/
#endif


//#define DEF_STAY_LIMIT	DIRECT_TICK(60*10) /*右舷動かなければグレネード*/
#define DEF_STAY_LIMIT	DIRECT_TICK(60*5) /*右舷動かなければグレネード*/
#define DEF_STAY_DECAY	DIRECT_TICK(300)

/*飛び出し間隔*/
#define		DEF_ATTACK_CYCLE	(180)
/*残り人数減ってからは出入り頻繁*/
#define		DEF_ATTACK_CYCLE_LAST	(12)	

 /*グレネード投げ後の他の兵士待機時間*/
#define		DEF_ATTACK_GRD_CYCLE	(60*5)


/*長廊コマンダ専用*/
typedef	struct	{
	u_char			def_mode ; /*動作モード*/
	u_char			defense_num; /*守備人数*/
	u_char		order ;
	u_char		trgmode ;	/*目標切り替え*/

	char	def_use[EVENT_DEF_POINT_MAX];	/* 使用中守備位置管理 */

	int			def_mapbit[EVENT_DEF_POINT_MAX] ; /* 守備位置のマップビット */
	FVECTOR		def_pos[EVENT_DEF_POINT_MAX];		/* 守備位置リスト*/
	int			def_zonedis[EVENT_DEF_POINT_MAX];
	int			def_act_num[EVENT_DEF_POINT_MAX]; /*アクション番号*/
	int			def_pos_num ;		/* 守備位置設定数 */
	FVECTOR		final_pos;
	int			final_addr;
	int			pl_fi_dis;	/*プレイヤとfinalposとのゾーン距離*/
	FVECTOR		trgpos2[2];	/*ライト破壊時などの目標座標 */

	short		count;		/*ライト破壊時間*/
	u_short		seq_cont ;	/*同期用カウンタ*/

	short		use_num ;	/*左舷用 使用済み管理番号*/
	short		res_pos_num ; /*再発生位置番号*/

	u_short		mode_cnt ; /*左舷第2フェイズ用*/
	u_short		grd_cnt	;	/*左舷ＧＲＤ間隔*/
	int			retire_num ;
	int			destroy		;
	int			wait_num ;
	int			max_res_num ;
	int			res_num ;
} DEF_COM ;

/*イベントＡ兵のみのENETHINK追加メンバ*/
typedef	struct	{
	DEF_COM		*def_com ;

	int			def_type;	/*守備位置のタイプ*/

	u_char		def_seq;	/*どのアクションリストを読むか？*/
	u_char		seq_index;	/*アクションリスト中の何番めか？*/
	char		def_pos_num;	/*守備位置番号*/
	char		def_pos_req;	/*次に向かう守備位置のリクエスト*/

	short		seq_time;	/*アクション持続時間*/
	char		seq_pad;	/*padバッファ*/
	char		type ; 

	short		det_time;	/*銃撃戦状態経過時間*/
	char		status;		
	char		video;		/*撮影用特種モード*/

	short		shot_rnd;	/*銃ブレの幅*/
	char		level;		/*攻撃レベル*/
	char		shot_delay ;	/*発砲までの時間*/

	char		shot_time ;	/*発砲時間*/
	char		wait_time ;	/*隙*/
	short 		seq_count ; /*1アクション管理*/

	int			another_flag ;
	int			shuffle_time ; /*席がえ周期管理*/	
} ENTK_TYPE_A ;

