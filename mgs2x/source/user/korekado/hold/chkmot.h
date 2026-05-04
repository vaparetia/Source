/*
	chkmot.h
	確認兵（船倉用）

	2000/03/28 Y.Korekado
	$Id: chkmot.h,v 1.1.1.3 2002/11/19 11:44:16 Yoshizawa1 Exp $
	
*/

#ifdef HOLD_MOT_NEW

enum {
//演説視聴用
	USHOLD_idle_1,
	USHOLD_idle_2,
	USHOLD_idle_3,
	USHOLD_idle_4,
	USHOLD_idle_5,
	USHOLD_idle_6,
	USHOLD_look_r,
	USHOLD_look_r_idle,
	USHOLD_look_l,
	USHOLD_look_l_idle,
	USHOLD_look_f,
	USHOLD_look_f_idle,
	USHOLD_2_to_4,
	USHOLD_1_uneune,
//敬礼
	USHOLD_listen_attention_start,
	USHOLD_listen_attention_idle,
	USHOLD_listen_attention_salute_crisp,
	USHOLD_listen_attention_salute_crisp_idle,
	USHOLD_listen_attention_salute_crisp_end,
//首体操
	USHOLD_listen_speech_refresh_r,
	USHOLD_listen_speech_refresh_d,
	USHOLD_listen_speech_refresh_idle,
	USHOLD_listen_speech_refresh_l,
	USHOLD_listen_speech_refresh_u,

	USHOLD_usl_fold_cameraman_pan_r,
	USHOLD_usl_fold_cameraman_idle,
	USHOLD_usa_m4a_listen_speech_sleep2down_f,
	USHOLD_usa_m4a_listen_speech_sleep2down_b,
	USHOLD_usa_m4a_listen_speech2cle,
	USHOLD_usa_m4a_listen_speech_r_turn,
	USHOLD_usa_m4a_listen_speech_l_turn,

//その他 メモリ展開はここまで
	USHOLD_gbs_ak_nom_idle_atk,
	USHOLD_gbs_ak_nom_walk,
	USHOLD_gbs_ak_nom_ijiiji,

/* スコット */
	SCOT_walk,
	SCOT_speech_idle_1,
	SCOT_speech_detect_sna_idle,
	SCOT_speech_detect_sna,

MAX_USHOLD_MOTION
} ;

/* gbs.mlsから船倉兵専用へのモーション変換リスト */
extern int use_standard_motion_num[][2] ;
extern int CheckEneMotionArray[ EM_STANDARD_MAX ] ;
extern int ScotMotionArray[ EM_STANDARD_MAX ] ;


#else	/* --------------------------OLD Mot Sysytem ----------------------------*/

enum {
USHOLD_idle_1,
USHOLD_idle_2,
USHOLD_idle_3,
USHOLD_idle_4,
USHOLD_idle_5,
USHOLD_idle_6,
USHOLD_look_r,
USHOLD_look_r_idle,
USHOLD_look_l,
USHOLD_look_l_idle,
USHOLD_look_f,
USHOLD_look_f_idle,

USHOLD_2_to_4,
USHOLD_1_uneune,
//USHOLD_burabura_3,
//USHOLD_dozing_5,
//USHOLD_iraira_4,
//USHOLD_shoulders_4,
//USHOLD_wake_up_5,

USHOLD_listen_attention_start,
USHOLD_listen_attention_idle,
USHOLD_listen_attention_salute_crisp,
USHOLD_listen_attention_salute_crisp_idle,
USHOLD_listen_attention_salute_crisp_end,

USHOLD_listen_speech_refresh_r,
USHOLD_listen_speech_refresh_d,
USHOLD_listen_speech_refresh_idle,
USHOLD_listen_speech_refresh_l,
USHOLD_listen_speech_refresh_u,

USHOLD_usl_fold_cameraman_pan_r,
USHOLD_usl_fold_cameraman_idle,
USHOLD_usa_m4a_listen_speech_sleep2down_f,
USHOLD_usa_m4a_listen_speech_sleep2down_b,

USHOLD_usa_m4a_listen_speech2cle,
USHOLD_usa_m4a_listen_speech_r_turn,
USHOLD_usa_m4a_listen_speech_l_turn,

/* その他（ここまでがMOTION_NUMの数） */
USHOLD_gbs_ak_nom_idle_atk,
USHOLD_gbs_ak_nom_walk,
USHOLD_gbs_ak_nom_ijiiji,

/* スコット */
SCOT_walk,
SCOT_speech_idle_1,
SCOT_speech_detect_sna_idle,
SCOT_speech_detect_sna,


USHOLD_gbs_ak_nom_run,
USHOLD_gbs_ak_nom_find_ply,
USHOLD_gbs_non_nom_idle_holdup,
USHOLD_gbs_ak_nom_surprised_bomb,
USHOLD_gbs_ak_nom_turn_b_t,
USHOLD_gbs_ak_nom_turn_l_t,
USHOLD_gbs_ak_nom_turn_r_t,

/* 攻撃*/
USHOLD_gbs_ak_nom_fire_p_high,

/*ダメージ関係 */
USHOLD_gbs_ak_nom_dam_gun_1,
USHOLD_gbs_ak_nom_dam_gun_2,
USHOLD_gbs_ak_nom_dam_gun_3,
USHOLD_gbs_ak_nom_dam_bom_f,
USHOLD_gbs_ak_nom_dam_down_f,
USHOLD_gbs_ak_nom_dam_out,
USHOLD_gbs_ak_nom_down_b,
USHOLD_gbs_ak_nom_iya_escape,
USHOLD_gbs_ak_nom_iya_hang,
USHOLD_gbs_ak_nom_iya_idle,
USHOLD_gbs_ak_nom_iya_release,
USHOLD_gbs_ak_nom_iya_release_down,
USHOLD_gbs_ak_nom_iya_resist,
USHOLD_gbs_ak_nom_iya_tie,
USHOLD_gbs_ak_nom_iya_walk,
USHOLD_gbs_ak_dam_escape,			/* その場で転倒 */
USHOLD_gbs_ak_nom_dam_bomb_fly_f,
USHOLD_gbs_ak_nom_dam_throw,
USHOLD_gbs_ak_nom_dam_kick_t,
USHOLD_gbs_ak_nom_dam_wall_down,
USHOLD_gbs_ak_nom_dam_wall_down_p,
USHOLD_gbs_ak_nom_dam_wall_move_d,
USHOLD_gbs_ak_nom_okiru_f,
USHOLD_gbs_ak_nom_okiru_f_slow,
USHOLD_gbs_ak_nom_okiru_b,
USHOLD_gbs_ak_nom_okiru_b_slow,
USHOLD_gbs_ak_nom_dam_panch_l,
USHOLD_gbs_ak_nom_dam_panch_r,
USHOLD_gbs_ak_nom_dam_fence_down,
USHOLD_gbs_ak_nom_dam_fence_fall,
USHOLD_gbs_ak_nom_dam_fence_start,
USHOLD_gbs_ak_nom_sleep_f,
USHOLD_gbs_ak_nom_sleep_b,
USHOLD_gbs_ak_nom_dam_bomb_fly_b,
//USHOLD_gbs_ak_nom_dam_bomb_fly_f,
USHOLD_gbs_ak_nom_dam_elude,
USHOLD_gbs_ak_nom_dam_out_bleed,
USHOLD_gbs_ak_nom_dam_okaji_end,
USHOLD_gbs_ak_nom_dam_okaji,
USHOLD_gbs_ak_nom_dam_stun_light,
USHOLD_gbs_ak_nom_dam_stun_down_b,
USHOLD_gbs_ak_nom_dam_fire_ex,
USHOLD_gbs_ak_nom_avoid_hang,
USHOLD_gbs_ak_nom_dam_punch2down,
USHOLD_gbs_ak_nom_dam_stair_start_b,


USHOLD_gbs_ak_nom_dam_fence_f_fall,
USHOLD_gbs_ak_nom_dam_fence_f_down,
USHOLD_gbs_ak_nom_dam_fence_f_start,
USHOLD_gbs_ak_nom_dam_stair_start,
USHOLD_gbs_ak_nom_dam_stair_roll_1,
USHOLD_gbs_ak_nom_dam_stair_roll_2,
USHOLD_gbs_ak_nom_dam_stair_end,
USHOLD_gbs_ak_nom_push_ply_r,
USHOLD_gbs_ak_nom_push_ply_l,
USHOLD_gbs_ak_nom_squeezed_f,
USHOLD_gbs_ak_nom_squeezed_b,


	USHOLD_gbs_ak_nom_dam_out_f_cap01,
	USHOLD_gbs_ak_nom_dam_out_f_cap02,
	USHOLD_gbs_ak_nom_dam_out_b_cap01,
	USHOLD_gbs_ak_nom_dam_out_b_cap02,
	USHOLD_gbs_ak_nom_dam_out_b_cap03,
	USHOLD_gbs_ak_nom_dam_out_b_cap04,
	USHOLD_gbs_ak_nom_dam_ball_cap01,

	/* 死体モーション */
	USHOLD_gbs_ak_dead_carry_body_end,
	USHOLD_gbs_ak_dead_carry_body_idle,
	USHOLD_gbs_ak_dead_carry_body_start_b,
	USHOLD_gbs_ak_dead_carry_body_start_f,
	USHOLD_gbs_ak_dead_carry_body_walk,
	USHOLD_gbs_ak_nom_down_f,
	USHOLD_gbs_ak_dead_carry_leg_end_b,
	USHOLD_gbs_ak_dead_carry_leg_end_f,
	USHOLD_gbs_ak_dead_carry_leg_idle_b,
	USHOLD_gbs_ak_dead_carry_leg_idle_f,
	USHOLD_gbs_ak_dead_carry_leg_start_b,
	USHOLD_gbs_ak_dead_carry_leg_start_f,
	USHOLD_gbs_ak_dead_carry_leg_walk_b,
	USHOLD_gbs_ak_dead_carry_leg_walk_f,

} ;

static	int	use_standard_motion_num[][2] = {
/* 基本 */
	{ EM_stand, USHOLD_gbs_ak_nom_idle_atk },
	{ EM_walk, USHOLD_gbs_ak_nom_walk },
	{ EM_run, USHOLD_gbs_ak_nom_run },
	{ EM_find_ply, USHOLD_gbs_ak_nom_find_ply },
	{ EM_idle_holdup, USHOLD_gbs_non_nom_idle_holdup },
	{ EM_surprised_bomb, USHOLD_gbs_ak_nom_surprised_bomb },	/* 物音びっくり */
	{ EM_turn_l_t, USHOLD_gbs_ak_nom_turn_l_t },	/* 物音びっくり */
	{ EM_turn_r_t, USHOLD_gbs_ak_nom_turn_r_t },	/* 物音びっくり */
	{ EM_turn_b_t, USHOLD_gbs_ak_nom_turn_b_t },	/* 物音びっくり */

	{ EM_ijiiji, USHOLD_gbs_ak_nom_ijiiji },	/* 下覗き込み */

/* 攻撃*/
	{ EM_kamae_gun_high, USHOLD_gbs_ak_nom_fire_p_high },

/*ダメージ関係 */
	{ EM_dam_nokezori, USHOLD_gbs_ak_nom_dam_gun_1 },
	{ EM_dam_gun_2, USHOLD_gbs_ak_nom_dam_gun_2 },
	{ EM_dam_gun_3, USHOLD_gbs_ak_nom_dam_gun_3 },
	{ EM_dam_out_f, USHOLD_gbs_ak_nom_dam_down_f },
	{ EM_dam_out, USHOLD_gbs_ak_nom_dam_out },
	{ EM_dam_out_pause, USHOLD_gbs_ak_nom_down_b },
	{ EM_nom_iya_escape, USHOLD_gbs_ak_nom_iya_escape },
	{ EM_nom_iya_hang, USHOLD_gbs_ak_nom_iya_hang },
	{ EM_nom_iya_idle, USHOLD_gbs_ak_nom_iya_idle },
	{ EM_iya_release, USHOLD_gbs_ak_nom_iya_release },
	{ EM_iya_release_down, USHOLD_gbs_ak_nom_iya_release_down },
	{ EM_nom_iya_resist, USHOLD_gbs_ak_nom_iya_resist },
	{ EM_nom_iya_tie, USHOLD_gbs_ak_nom_iya_tie },
	{ EM_nom_iya_walk, USHOLD_gbs_ak_nom_iya_walk },
	{ EM_dam_bomb_b, USHOLD_gbs_ak_dam_escape },/*モーションできたら修正 */
	{ EM_dam_bomb_f, USHOLD_gbs_ak_nom_dam_bom_f },
	{ EM_dam_throw, USHOLD_gbs_ak_nom_dam_throw },
	{ EM_dam_kick_t, USHOLD_gbs_ak_nom_dam_kick_t },
	{ EM_dam_wall_down, USHOLD_gbs_ak_nom_dam_wall_down },
	{ EM_dam_wall_down_p, USHOLD_gbs_ak_nom_dam_wall_down_p },
	{ EM_dam_wall_move_d, USHOLD_gbs_ak_nom_dam_wall_move_d },
	{ EM_okiru_aomuke, USHOLD_gbs_ak_nom_okiru_f },
	{ EM_okiru_aomuke_slow, USHOLD_gbs_ak_nom_okiru_f_slow },
	{ EM_okiru_utubuse, USHOLD_gbs_ak_nom_okiru_b },
	{ EM_okiru_utubuse_slow, USHOLD_gbs_ak_nom_okiru_b_slow },
	{ EM_dam_punch_l, USHOLD_gbs_ak_nom_dam_panch_l },
	{ EM_dam_punch_r, USHOLD_gbs_ak_nom_dam_panch_r },
	{ EM_dam_fence_down, USHOLD_gbs_ak_nom_dam_fence_down },
	{ EM_dam_fence_fall, USHOLD_gbs_ak_nom_dam_fence_fall },
	{ EM_dam_fence_start, USHOLD_gbs_ak_nom_dam_fence_start },
	{ EM_sleep_f, USHOLD_gbs_ak_nom_sleep_f },
	{ EM_sleep_b, USHOLD_gbs_ak_nom_sleep_b },
	{ EM_dam_bomb_fly_b, USHOLD_gbs_ak_nom_dam_bomb_fly_b },
	{ EM_dam_bomb_fly_f, USHOLD_gbs_ak_nom_dam_bomb_fly_f },
	{ EM_dam_elude,	USHOLD_gbs_ak_nom_dam_elude },	/* 上から乗っかられてダウン */
	{ EM_dam_out_bleed,	USHOLD_gbs_ak_nom_dam_out_bleed },/* 出血多量死 */
	{ EM_dam_okaji_end,	USHOLD_gbs_ak_nom_dam_okaji_end },/* 呆けから復帰 */
	{ EM_dam_okaji,	USHOLD_gbs_ak_nom_dam_okaji },	/* 呆け */
	{ EM_dam_stun_far, USHOLD_gbs_ak_nom_dam_stun_light },	/* スタン、ファーダメージ */
	{ EM_dam_stun_near, USHOLD_gbs_ak_nom_dam_stun_down_b },	/* スタン、ニアダメージ */
	{ EM_dam_fire_ex, USHOLD_gbs_ak_nom_dam_fire_ex },		/* 水蒸気ダメージ */
	{ EM_avoid_hang, USHOLD_gbs_ak_nom_avoid_hang },		/* 首締め避け */
	{ EM_dam_punch2down, USHOLD_gbs_ak_nom_dam_punch2down },
	{ EM_dam_stair_start_b, USHOLD_gbs_ak_nom_dam_stair_start_b },	/* 階段転がり後ろ向きから */

	{ EM_dam_fence_f_fall, USHOLD_gbs_ak_nom_dam_fence_f_fall },	/* 手摺り落ち中 */
	{ EM_dam_fence_f_down, USHOLD_gbs_ak_nom_dam_fence_f_down },	/* 手摺り落ちダウン */
	{ EM_dam_fence_f_start, USHOLD_gbs_ak_nom_dam_fence_f_start },	/* 手摺り落ちスタート */
	{ EM_dam_stair_start, USHOLD_gbs_ak_nom_dam_stair_start },		/* 階段落ちスタート */
	{ EM_dam_stair_roll_1, USHOLD_gbs_ak_nom_dam_stair_roll_1 },		/* 階段落ち転がり */
	{ EM_dam_stair_roll_2, USHOLD_gbs_ak_nom_dam_stair_roll_2 },		/* 階段落ち転がり */
	{ EM_dam_stair_end, USHOLD_gbs_ak_nom_dam_stair_end },		/* 階段落ち最後 */
	{ EM_push_ply_r, USHOLD_gbs_ak_nom_push_ply_r },			/* スネーク押し右 */
	{ EM_push_ply_l, USHOLD_gbs_ak_nom_push_ply_l },			/* スネーク押し左 */
	{ EM_squeezed_f, USHOLD_gbs_ak_nom_squeezed_f },			/* 押され前 */
	{ EM_squeezed_b, USHOLD_gbs_ak_nom_squeezed_b },			/* 押され後ろ */

	{ EM_dam_out_f_cap01, USHOLD_gbs_ak_nom_dam_out_f_cap01 },
	{ EM_dam_out_f_cap02, USHOLD_gbs_ak_nom_dam_out_f_cap02 },
	{ EM_dam_out_b_cap01, USHOLD_gbs_ak_nom_dam_out_b_cap01 },
	{ EM_dam_out_b_cap02, USHOLD_gbs_ak_nom_dam_out_b_cap02 },
	{ EM_dam_out_b_cap03, USHOLD_gbs_ak_nom_dam_out_b_cap03 },
	{ EM_dam_out_b_cap04, USHOLD_gbs_ak_nom_dam_out_b_cap04 },
	{ EM_dam_ball_cap01, USHOLD_gbs_ak_nom_dam_ball_cap01 },

	{ EM_dead_carry_body_end, USHOLD_gbs_ak_dead_carry_body_end },
	{ EM_dead_carry_body_idle, USHOLD_gbs_ak_dead_carry_body_idle },
	{ EM_dead_carry_body_start_b, USHOLD_gbs_ak_dead_carry_body_start_b },
	{ EM_dead_carry_body_start_f, USHOLD_gbs_ak_dead_carry_body_start_f },
	{ EM_dead_carry_body_walk, USHOLD_gbs_ak_dead_carry_body_walk },
	{ EM_dead_idle_f, USHOLD_gbs_ak_nom_down_f },
	{ EM_dead_carry_leg_end_b, USHOLD_gbs_ak_dead_carry_leg_end_b },
	{ EM_dead_carry_leg_end_f, USHOLD_gbs_ak_dead_carry_leg_end_f },
	{ EM_dead_carry_leg_idle_b, USHOLD_gbs_ak_dead_carry_leg_idle_b },
	{ EM_dead_carry_leg_idle_f, USHOLD_gbs_ak_dead_carry_leg_idle_f },
	{ EM_dead_carry_leg_start_b, USHOLD_gbs_ak_dead_carry_leg_start_b },
	{ EM_dead_carry_leg_start_f, USHOLD_gbs_ak_dead_carry_leg_start_f },
	{ EM_dead_carry_leg_walk_b, USHOLD_gbs_ak_dead_carry_leg_walk_b },
	{ EM_dead_carry_leg_walk_f, USHOLD_gbs_ak_dead_carry_leg_walk_f },

	{-1, -1},
} ;
#endif

