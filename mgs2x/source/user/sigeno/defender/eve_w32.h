/*
	eve_w32.h
	防衛型イベント敵兵関連ヘッダ
	2001/04/17 K.Sigeno
	$Id: eve_w32.h,v 1.1.1.3 2002/11/19 11:49:10 Yoshizawa1 Exp $
*/


#define W32_ATTACKPOS_MAX 4
typedef	struct	{
	int	find_flag ;
	int	old_find_flag ;
	int	dis_rank ;
	int	zone_dis ;
	int	dis_dif ;
	FVECTOR		esc_pos ; /*逃げ場所*/
	int			esc_map ;
	int			esc_cnt ;

	FVECTOR		attack_pos[W32_ATTACKPOS_MAX] ; /*攻撃場所*/
	int			attack_map[W32_ATTACKPOS_MAX] ;
	short		attack_max ;
	short		attack_num ;

	u_char		team ;
	u_char		mode ;
	short		fix_dir ;

	long64 front_st ;
	int		st_flag ;
	int		Pl_StayTime;	/*プレイヤの静止時間*/
	int		OldPlAddr;	/*前フレームでのプレイヤアドレス待避*/
} EVENT_W32 ;
/*find_flag member */
#define W32_FIND			(0x01) /*目標補足*/
#define W32_ESC_POS			(0x02)
#define W32_ATTACK_POS		(0x04)
#define W32_ALERT			(0x08) /*警戒行動開始*/
#define W32_FAKE			(0x10) /*強制見つけ*/

/*st_flag member*/
#define	W32_ST_CONTACT	(0x01)   /*ダメージ後の反撃時に仲間も攻撃開始*/
#define	W32_ST_SKIP_DMG	(0x02)   /*ダメージ後無敵*/

/***********/
#define W32_ESC_TIME	(120)
#define W25_DEF_EYE_RANGE (256) /*小窓から見つかりにくくするため小さ目*/
enum {
	DEF_MODE_W32_A = 0,
	DEF_MODE_W32_B ,
	DEF_MODE_W32_C,
	DEF_MODE_W25_A,
	DEF_MODE_W25_B,
	DEF_MODE_W25_C
};
