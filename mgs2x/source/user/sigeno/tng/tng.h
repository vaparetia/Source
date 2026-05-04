/*
	tng.h
	$Id: tng.h,v 1.1.1.3 2002/11/19 11:49:52 Yoshizawa1 Exp $
*/


/*天狗 と 刀 通信用フラグ*/
#define TNG_KATANA_ON			(0x01)
#define TNG_KATANA_BULLET_HIT	(0x02)
#define TNG_KATANA_BLUR_ON		(0x04)
#define TNG_KATANA_FALL			(0x08)

#define	EVE_TNG_OBSERVE		(0x0001)	/*目撃した本人 危険レベルに関係なく攻撃*/
//#define	EVE_TNG_DEATH	(0x0002)	/*死んでる*/
#define	EVE_TNG_WAIT		(0x0004)	/*物陰で待機*/
#define	EVE_TNG_SUPORT_WAIT	(0x0008)	/*応援コールまで待機*/
#define	EVE_TNG_NO_DEF		(0x0010)	/*追いかけモード*/
#define	EVE_TNG_ATTAK_END	(0x0020)	/*攻撃済み*/
#define	EVE_TNG_RETIRE		(0x0040)	/*退却*/
#define	EVE_TNG_HANG		(0x0080)	/*首絞め攻撃さん*/
#define	EVE_TNG_ACTIVE		(0x0100)	/* 戦闘可能 */ 
#define	EVE_TNG_RES_REQ		(0x0200)	/* 再発生すべきです */ 
#define	EVE_TNG_DAM_WAIT	(0x0400)	/* ダメージ状態から復帰してない 保険 */ 

//#define		DEF_DYNAMIC_SIGHT	(2500)
#define		DEF_DYNAMIC_SIGHT	(6000)

#define		KATANA_GUARD_INTR	(8)
#define		KATANA_ROLL_GUARD_INTR	DIRECT_TICK(20)

//#define HANG_TEST
/*攻撃兵コマンダ 守備モード管理*/
#if 1
enum {
	AT_COM_DEF_NORMAL,	
	AT_COM_DEF_WAIT,	/*待機中*/
	AT_COM_DEF_DYNAMIC	/*全員突撃*/
};
#endif

#define TNG_JUNP_DIS	(487.6841F)
//#define TNG_JUNP_DIS_VR	(495.0000F)
#define TNG_JUNP_DIS_VR	(510.0000F)

//#define TNG_JUNP_DIS	(1500.0F)
//#define TNG_JUNP_DIS	(800.0F)
#define TNG_JUNP_DIS_LEVEL (6000.0F)

enum {
	TNG_COM_TYPE_A ,
	TNG_COM_TYPE_B
};
/*天狗兵のモード*/
enum {
	TNG_TYPE_A ,	/*w42a裸ライデン対応*/
	TNG_TYPE_B ,	/*w44a*/
	TNG_TYPE_C ,	/*w45a銃*/
	TNG_TYPE_D		/*w45a刀*/
};
/*イベント時の守備ポイント最大数*/
//#define EVENT_DEF_POINT_MAX (8)
//#define EVENT_DEF_POINT_MAX (16)
#define EVENT_DEF_POINT_MAX (32)
#define	EVENT_SUPPORT_TIME	(60*30)

#define		DEF_MODE_CNT	(60*30)
#define		DEF_GRD_CNT		(60*15)
#define		DEF_ATTACK_CYCLE	(180)
 /*グレネード投げ後の他の兵士待機時間*/
#define		DEF_ATTACK_GRD_CYCLE	(60*5)

#define		MAX_TNG_RES_POS		(16)

#define		KATANA_DIS		(1500)
//#define		HANG_DIS		(1500)
//#define		HANG_DIS		(500)
#define		HANG_DIS		(1000)

#define		KATANA_ATTACK_NUM (2) /*同時に突撃する刀人数*/
#define		KATANA_GUARD_DEF	(30)	
//#define		KATANA_GUARD_DEF	(1)	
#define		KATANA_JUMP_ATTACK_DIS	(6500)



enum {
	TNG_PH_WAIT = 0,
	TNG_PH_ALART ,
	TNG_PH_SHUFFLE,
	TNG_PH_END,
	TNG_PH_DESTROY
};
/*天狗コマンダ専用*/
#define	TNG_COM_NORMAL			(0x00)
#define TNG_COM_DEBUG			(0x01)	//デバッグ用表示ON
#define TNG_COM_MULTITEX_AUTO	(0x02)	//主観時以外マルチテクスチャOFF
#define	TNG_COM_W41 			(0x04)

#define TNG_PL_WAIST_H	(300.0F)

#define TNG_JUMP_ATTACK_CNT DIRECT_TICK(60*5)

typedef	struct	{

	int			def_route[EVENT_DEF_POINT_MAX];

	u_char		tng_respos_num[EVENT_DEF_POINT_MAX];
	u_char		active_lv[EVENT_DEF_POINT_MAX];

	int			def_pos_num ;		/* 守備位置設定数 */
	FVECTOR		final_pos ;
	FVECTOR		esc_pos ;
	int			final_addr ;
	int			pl_fi_dis ;	/*プレイヤとfinalposとのゾーン距離*/

	u_int		res_trap ;	/*再発生トラップ*/

	short		phase ;		
	u_short		seq_cont ;	/*同期用カウンタ*/

	FVECTOR		tng_respos[MAX_TNG_RES_POS] ;
	int			tng_resmap[MAX_TNG_RES_POS] ;

	char		now_level ; /*プレイヤ進行度*/
	char		max_level ;
	char		use_num ;	/*左舷用 使用済み管理番号*/
	char		res_pos_num ; /*再発生位置番号*/

	u_char		shot_delay ; /*登場から発砲まで*/
	u_char		shot_time ; /*発砲時間*/
	u_char		wait_time ; /*発砲後の隙*/
	u_char		destroy ; /*排除された敵人数*/ 

	CONTROL		*subtrg ; /*プレイヤ以外の目標キャラ*/

	u_char		guard_max ; /*連続防御限界*/
	u_char		retire_num ; /*死んでないが行動不能な人数*/
	char		hang_num ; /*首絞め許可*/
	u_char		now_hang_num ; /*首絞め行動中人数*/

	FVECTOR		avoid_pos ; /*回避モード時の捜索場所*/
	float		base_floor ; /*標準床高さ*/
	short		mode ;		/*とりあえず拷問部屋だけ区別*/
	short		jump_attack ;
	int			quiet_time ;
	int			quiet_time2 ;	/*保険*/
	int			max_res_num ;
	int			res_num ;
} TNG_COM ;



/*天狗兵のみのENETHINK追加メンバ*/
typedef	struct	{
	TNG_COM		*tng_com ;

	int			def_type;	/*守備位置のタイプ*/

	u_char		def_seq;	/*どのアクションリストを読むか？*/
	u_char		seq_index;	/*アクションリスト中の何番めか？*/
	char		def_pos_num;	/*守備位置番号*/
	char		def_pos_req;	/*次に向かう守備位置のリクエスト*/

	short		seq_time;	/*アクション持続時間*/
	char		seq_pad;	/*padバッファ*/
	char		type ; 

	short		det_time;	/*銃撃戦状態経過時間*/
	short		status;		
//	char		video;		/*撮影用特種モード*/

	short		shot_rnd;	/*銃ブレの幅*/
	short 		seq_count ; /*1アクション管理*/
	short		res_pos ;
	short		def_level ;

	int			level;		/*攻撃レベル*/

	short			katana ;
	short			guard_n ;	/*連続防御回数管理*/ 

	int			blur_sw ;	/*頭の飾りスイッチ*/

} ENTK_TENG_A ;


extern void TngKatanaOn(ENETHINK *) ;
extern void TngKatanaOff(ENETHINK *) ;

