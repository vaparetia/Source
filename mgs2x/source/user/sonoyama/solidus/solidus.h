/*
   solidus.h
   ソリダスヘッダ

   2001/03/21	M.Sonoyama
   $Id: solidus.h,v 1.1.1.3 2002/11/19 11:51:07 Yoshizawa1 Exp $
*/

#ifndef	__solidus_h__
#define	__solidus_h__

#define	NTSC_TIME_BASE			(5)
#define	N_UNITS					(21)
#define	BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE|\
					 DG_FLAG_IRREACTION)

#define	GRAVITY					(96.0F)
#define	INVINCIBLE_TIME			(DIRECT_TICK(60))

#define	CHECK_SPHERE			(500)
#define	NORMAL_SPHERE			(480)
#define	NEAR_HEIGHT_STAND		(748.0F)
#define	NEAR_HEIGHT_SQUAT		(248.0F)
#define	NEAR_HEIGHT_GROUND		(248.0F)

#define	MAX_TARGET_CHILDREN		(13)


#define	MAAI_LEN				(1800.0F)

#define	SOL_BLADE_DAMAGE		(3)
#define	SOL_PUNCH_DAMAGE		(1)
#define	SOL_KICK_DAMAGE			(2)

#define	WALL_X					(15000.0F)		/* きめきめビル壁座標 */
#define	TRIANGLE_SHIFT			(4000.0F)		/* 三角飛び、スタート位置と壁の距離 */
#define	TRIANGLE_TURN			(1024)			/* 三角飛び、向き */
#define	TRIANGLE_DIFFH			(2781.1F - 868.7F)	/* 飛び位置と着地位置の床高さの差 */
#define	TRIANGLE_DIFFV			(-8561.6F)			/* 飛び位置と着地位置のＺ位置の差 */

/*----------------------------------------------------------------*/

/* 近距離思考 */
typedef	struct	{
	int			rot_diff_count ;
	int			maai_count ;
	int			ground_count ;
	int			middle_count ;
} NearThink ;

/* 遠距離思考 */
typedef	struct	{
	int			liftup_count ;
	int			missile_count ;
	int			triangle_count ;
	int			last_missile_time ;
} FarThink ;

/* 位置移動ワーク */
typedef	struct	{
	FVECTOR		aim_position ;
	float		aim_len ;
	int			aim_dir ;
	int			aim_way ;
} MoveThink ;

typedef	struct _Work {
	GV_ACT_EX				actor ;
	CONTROL					control ;
	OBJECT					body ;
	HOMING_TRG				homing_trg ;
	FMATRIX					lights[ 2 ] ;
	GM_GageSet				life_gage ;
	int						procs[ 8 ] ;
	int						motion1 ;
	int						motion2 ;
	int						mtime1 ;
	int						mtime2 ;

	FVECTOR					map_center ;
	FVECTOR					jump_bound1 ;
	FVECTOR					jump_bound2 ;
	FVECTOR					hit ;

	SVECTOR					turn_adj[ 24 ] ;
	SVECTOR					rot_adj[ 24 ] ;

	TARGET					defense ;
	TARGET					offense ;
	TARGET					offense2 ;
	POWER_TARGET			pt_defense ;
	POWER_TARGET			pt_offense ;
	POWER_TARGET			pt_offense2 ;

	/* level0 頭、心臓、股間 */
	/* level1 両腕、両足 */
	/* level2 腰、胸 */
	TARGET					child[ MAX_TARGET_CHILDREN ] ;	

	FVECTOR					fv1 ;
	FVECTOR					fv2 ;

	FVECTOR					pos_diff_player ;
	SVECTOR					rot_diff_player ;
	float					len_diff_player ;

	NearThink				near_think ;
	FarThink				far_think ;
	MoveThink				move_think ;

	long64					weapon_type ;	

	u_int					act_name ;
	void					( *action )( struct _Work *, int ) ;
	u_int					time ;
	int						ftime ;
	int						ftime_count ;

	u_int					act_name2 ;
	void					( *action2 )( struct _Work *, int ) ;
	u_int					time2 ;
	int						ftime2 ;
	int						ftime_count2 ;

	int						mode ;
	int						status ;
	int						flag ;
	int						data ;
	int						data2 ;
	int						idata ;
	int						idata2 ;
	float					fdata ;
	float					fdata2 ;
	int						seNoSeg ;
	int						seNoFlr ;
	int						invincible_time ;
	int						last_damage_time ;
	int						shadow ;
	int						se_tableID ;
	void					*attach_work ;
	float					va_t ;
	VERTEX_ANIME_WORK		*va_work[ 24 ] ;

	int						kwt_trg ;
	int						mst_trg ;

	int						maai_count ;
	int						guard_hit ;
	int						guard_last_time ;
	int						guard_mode ;
	int						cont_hit_count ;
	int						damage ;		/* コールバック内でセット、毎フレームクリアすること */
	int						faint_damage ;
	int						wakeup_time ;	/* 起動時間 */
	int						away_count ;	/* 加速逃げ連続回数 */

	int						down_dir ;		/* ダウン向き */

	int						careful_time ;	/* 間合いフェーズカウンタ */
	int						last_arm_attack ;
	int						last_elude_attack_time ;
	int						last_player_damage_se_time ;

	int						prev_mtime1 ;
	int						prev_mtime2 ;

	int						last_slash_time ;
	int						dash_fire_no ;
	int						dash_fire_current ;
	int						dash_fire_flag[ 8 ] ;

    int	energy_mode;
    int	pre_pl_status;

	DG_OBJS					*gantai ;
	DG_OBJS					*faceguard ;
	DG_OBJS					*kwt_sht ;
	DG_OBJS					*mst_sht ;

	char					*caption1 ;
	char					*caption2 ;
	int						caption_time1 ;
	int						caption_time2 ;
	int						caption_end ;

	int						vib_time ;
} Work ;

/* プロック */
enum {
	SOL_PROC_DEAD = 0,
	SOL_PROC_ARMUNDO_MOVE_START,
	SOL_PROC_ARMUNDO_START,
	SOL_PROC_ARMUNDO_END,
} ;

/* 攻撃モード */
enum {
	SOL_MODE_BLADE_AND_SNAKEARM 	=	0,
	SOL_MODE_BLADE_ONLY,
} ;

/* ガードもーど */
enum {
	GUARD_LEFT = 0,
	GUARD_RIGHT,
	GUARD_DOUBLE,
	GUARD_SWAY_U,
	GUARD_SWAY_D,
} ;

/* モーションリスト */
enum {
	Mkwt_idle		=	0,				/* 刀静止 */
	Mkwt_walk,							/* 刀歩き */
	Mkwt_run,							/* 刀走り */
	Mwalk_b,
	
	Mkwt_fire_ready,
	Mkwt_fire_idle,	
	Mkwt_fire_end,
	Mslash_r,
	Mslash_l,
	Mslash_w,
	
	Mkwt_dam_f,				

	Mdam_f_m,
	Mdam_f_l,
	Mdam_f_r,
	Mdam_b_m,
	Mdam_b_l,
	Mdam_b_r,

	Mblow_f_m,
	Mblow_f_l,
	Mblow_f_r,
	Mblow_b_m,
	Mblow_b_l,
	Mblow_b_r,
	
	Mrise_f,
	Mrise_b,

	Mdam_filliped_r,
	Mdam_filliped_l,

	Mguard_u,
	Mguard_d,
	Mguard_r,
	Mguard_l,
	Mguard_w,

	Mdash_ready,
	Mdash_f,
	Mdash_b,
	Mdash_l,
	Mdash_r,
	Mdash_end_f,
	Mdash_end_b,
	Mdash_end_l,
	Mdash_end_r,

	Mthrust,
	Mspinkick,
	Melbow,

	Mseize_start,
	Mseize_liftup,
	Mseize_liftidle,
	Mseize_fling,
	Mseize_miss,
	Mseize_undo,

	Mclaw_ready,
	Mclaw_fire,
	Mclaw_end,

	Mtrip_ready,
	Mtrip_r,
	Mtrip_l,

	Mmissle_ready,
	Mmissle_fire,
	Mmissle_fire_idle,
	Mmissle_end,
	
	Mtriangle,
	
	Marm_undo_start,
	Marm_undo_end,

	Mslash_combo,
	Mstamp,

	Mdam_out,				//死に仮
	MAX_MOTIONS,
} ;

/* ステータス */
enum {
	SOL_STATE_NORMAL	  	=			0x00000000,

	SOL_STATE_RESET_ALL		=			0x0000ffff,

	SOL_STATE_INVINCIBLE 	=			0x00010000,
	SOL_STATE_DAMAGED 		=			0x00020000,
} ;

/* フラグ */
enum {
	SOL_FLAG_NORMAL			=			0x00000000,
	SOL_FLAG_BLADE_GUARD	=			0x00000001,		/* ブレード攻撃をガードできる */
	SOL_FLAG_BLADE_ATTACK_ENABLE =		0x00000002,		/* ブレード攻撃できる間合い */
	SOL_FLAG_COMBO_GUARD   	=			0x00000004,
	SOL_FLAG_LIFTUP_ENABLE =			0x00000008,
	SOL_FLAG_NO_MOTION_STEP_XZ =		0x00000010,
	SOL_FLAG_MISSILE_ATTACK_ENABLE =	0x00000020,
	SOL_FLAG_FAST_AWAY =				0x00000040,
	SOL_FLAG_NO_IK =					0x00000080,
	SOL_FLAG_BLADE_ATTACK_W =			0x00000100,
	SOL_FLAG_NO_PUSH =					0x00000200,
	SOL_FLAG_ATTACK			=			0x00000400,
	SOL_FLAG_SLASH			=			0x00000800,
	SOL_FLAG_SLASH_L		=			0x00001000,
	SOL_FLAG_ELBOW_ENABLE	=			0x00002000,
	SOL_FLAG_COMBO_IGNORE	=			0x00004000,
	SOL_FLAG_NO_GRAVITY 	=			0x00008000,
	SOL_FLAG_NO_WAIST_INTERP = 			0x00010000,
	SOL_FLAG_RESET_ALL		=			0x000fffff,

	SOL_FLAG_FILLIP_NOW		=			0x00100000,
	SOL_FLAG_CHOUHATSU		=			0x00200000,

	SOL_FLAG_CAREFUL		=			0x10000000,
} ;

typedef	void	( *ACTION )( Work *, int ) ;

#define	EndMotion( _w )	MT_CHECK_LAST1( (_w)->body.m_ctrl, 0 )

extern	void	*NewPadVibration( char *, int ) ;
extern	void	*NewKyouwaBlade( CONTROL *ctrl, OBJECT *body, int unit, int *trigger ) ;
extern	void	*NewMinshuBlade( CONTROL *ctrl, OBJECT *body, int unit, int *trigger ) ;
extern	void	*NewShadow( DG_OBJ *, DG_OBJ *, CONTROL *, FMATRIX *, int * ) ;
extern	void	PL_FillipSlash( void ) ;
extern	int		PL_SlashNow( void ) ;
extern	int		PL_SlashMode( void ) ;

extern	void	Leg_IKcalc( CONTROL *, OBJECT * ) ;
extern	void	SOL_SnakeArmFireMissile( int dir ) ;

extern	void	*NewSolidusSnakeArm( int model, OBJECT *pbody, int unit ) ;
extern	void	SOL_SetSnakeArmMotion( int nNo, int interp ) ;
extern	void	SOL_SetSnakeArmMotionEX( int nNo, int interp, int mode ) ;
extern	void	SOL_SnakeArmAttackOnline( int joint1, int joint2 ) ;
extern	void	SOL_SnakeArmAttackTrip( int which ) ;
extern	void	SOL_SnakeArmAttackClaw( void ) ;
extern	void	SOL_SnakeArmSetDefenseTarget( void ) ;
extern	void	SOL_SnakeArmSetFlowFlag( int flag ) ;
extern	void	SOL_SetBladeDamage( int damage, float lenplus ) ;
extern	void	SOL_SetBladeWeaponType( long64 weapon_type ) ;
extern	void	SOL_SetBladeForce( float v ) ;
extern	void	SOL_SnakeArmDestroy( void ) ;
extern	void	SOL_SnakeArmUndo( void ) ;

extern	int		PL_PluginLiftup( void ) ;

#endif

