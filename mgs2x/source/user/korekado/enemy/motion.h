/*
	motion.h
	敵兵用、モーションデータ
	
	1997/07/07 Y.Korekado
	$Id: motion.h,v 1.1.1.3 2002/11/19 11:44:09 Yoshizawa1 Exp $
	
*/
/*
	敵兵共通モーションデータ 
	注）複数のmarファイルからモーションを選べるような仕様になれば
		変更する予定
*/
#ifdef MOTION_PLUGIN
enum {
	/* 基本モーション */
	gbs_ak_nom_idle_atk,
	gbs_ak_nom_walk,
	gbs_ak_nom_run,
	gbs_ak_nom_find_ply,
	gbs_ak_cau_mukade_idle,
	gbs_ak_nom_ijiiji,
	gbs_ak_nom_glasses,
	gbs_ak_nom_glasses_p,
	gbs_non_nom_idle_holdup,
	gbs_non_nom_idle_holdup_surprise,
	gbs_ak_armr_cle_call_break_l,
	gbs_ak_nom_cle_call_break_r,
	gbs_ak_nom_turn_b_t,
	gbs_ak_nom_turn_l_t,
	gbs_ak_nom_turn_r_t,
	gbs_ak_nom_akubi,
	gbs_ak_nom_senobi,
	gbs_ak_nom_zzz,
	gbs_ak_nom_walk_light,
	gbs_ak_nom_surprised_bomb,
	gbs_ak_nom_gogle_off,
	gbs_ak_nom_gogle_on,
	gbs_ak_nom_walk_pass,
	gbs_ak_armr_cle_call_team_l_1start,
	gbs_ak_armr_cle_call_team_l_2talk,
	gbs_ak_armr_cle_call_team_l_3end,
	gbs_ak_nom_cle_call_team_gun_l_1_start,
	gbs_ak_nom_cle_call_team_gun_l_2_talk,
	gbs_ak_nom_cle_call_team_gun_l_3_end,
	gbs_ak_nom_look_erotica,
	gbs_ak_nom_look_erotica_loop2,
	gbs_ak_nom_look_erotica_loop1,
	gbs_ak_nom_cle_call_team_gun_l_1_start_fast,
	gbs_ak_armr_cle_call_team_l_1_start_fast,
	gbs_non_nom_idle_holdup_item,
	gbs_non_nom_idle_holdup2idle,
	gbs_non_nom_idle_holdup_bend,
	gbs_ak_nom_box_look,
	gbs_ak_nom_box_look_idle,
	gbs_ak_nom_box_kick,
	gbs_ak_nom_box_look2stand,
	gbs_ak_nom_box_lift,
	gbs_ak_nom_box_lift_p,
	gbs_ak_nom_box_lift2throw,
	gbs_ak_nom_see_patient,

	gbs_ak_arml_cle_call_break_r,
	gbs_ak_arml_cle_call_team_r_1start,
	gbs_ak_arml_cle_call_team_r_2talk,
	gbs_ak_arml_cle_call_team_r_3end,

	gbs_ak_nom_stair_run_down,
	gbs_ak_nom_stair_walk_down,
	gbs_ak_nom_stair_run_up,
	gbs_ak_nom_stair_walk_up,

	gbs_ak_nom_check_disguise_end,
	gbs_ak_nom_check_disguise_idle,
	gbs_ak_nom_check_disguise_start,
	gbs_ak_nom_holdup_counter,
	gbs_ak_nom_cle_look_l_max,
	gbs_ak_nom_cle_look_r_max,

	/* 攻撃基本モーション */
	gbs_ak_nom_run_atk,
	gbs_ak_nom_fire_p_high,
	gbs_ak_nom_fire_reload,
	gbs_ak_nom_squat_fire_reload,
	gbs_ak_nom_slide_fire_l,
	gbs_ak_nom_slide_fire_r,
	gbs_ak_nom_tobidasi_l_t,
	gbs_ak_nom_tobidasi_r_t,
	gbs_ak_nom_squat_fire,
	gbs_ak_nom_squat_hide_idle,
	gbs_ak_nom_squat_roll_l,
	gbs_ak_nom_squat_roll_r,
	gbs_ak_nom_step_l,
	gbs_ak_nom_step_r,
	gbs_ak_nom_behind_idle,
	gbs_ak_nom_walk_b_atk,
	gbs_ak_nom_stomp,
	gbs_ak_nom_signe_go_atk,
	gbs_gm_fire_high,
	gbs_gm_fire_low,
	gbs_gm_fire_korokoro,
	gbs_ak_cle_gm_fire,
	gbs_ak_sign_mukade_go,
	gbs_ak_nom_attack_near_kick,
	gbs_ak_nom_attack_near_kick_r,
	gbs_ak_nom_attack_near_kick_l,
	gbs_ak_nom_attack_near_punch_r, 
	gbs_ak_nom_attack_near_punch_l, 
	gbs_ak_nom_snipe_fire_p,
	gbs_ak_nom_turn_out,
	gbs_ak_nom_fire_intrude,
	gba_ak_nom_call_team_l,

	/* 警戒基本モーション */
	gbs_ak_cle_signe_idle,
	gbs_ak_armr_cle_call_team_l,
	gbs_ak_nom_cle_call_team_r,
	gbs_ak_cle_signe_go,
	gbs_ak_nom_cle_look_l,
	gbs_ak_nom_cle_look_r,
	gbs_ak_nom_cle_seach_r2l,
	gbs_ak_cle_walk,
	gbs_ak_nom_cle_walk_low,
	gbs_ak_nom_cle_slidewalk_l,
	gbs_ak_nom_cle_slidewalk_r,
	gbs_ak_nom_cle_run,
	gbs_ak_cle_signe_clear,
	gbs_ak_cle_signe_move,
	gbs_ak_cle_signe_stop,
	gbs_ak_nom_cle_locker_open,
	gbs_ak_cle_go_ahead,

	/* ダメージモーション */
	gbs_ak_nom_dam_gun_1,
	gbs_ak_nom_dam_gun_2,
	gbs_ak_nom_dam_gun_3,
	gbs_ak_nom_dam_down_f,
	gbs_ak_nom_dam_out,
	gbs_ak_nom_down_b,
	gbs_ak_nom_iya_escape,
	gbs_ak_nom_iya_hang,
	gbs_ak_nom_iya_idle,
	gbs_ak_nom_iya_release,
	gbs_ak_nom_iya_release_down,
	gbs_ak_nom_iya_resist,
	gbs_ak_nom_iya_tie,
	gbs_ak_nom_iya_walk,
	gbs_ak_dam_escape,			/* その場で転倒 */
	gbs_ak_nom_dam_bom_f,
	gbs_ak_nom_dam_throw,
	gbs_ak_nom_dam_kick_t,
	gbs_ak_nom_dam_wall_down,
	gbs_ak_nom_dam_wall_down_p,
	gbs_ak_nom_dam_wall_move_d,
	gbs_ak_nom_okiru_f,
	gbs_ak_nom_okiru_f_slow,
	gbs_ak_nom_okiru_b,
	gbs_ak_nom_okiru_b_slow,
	gbs_ak_nom_dam_panch_l,
	gbs_ak_nom_dam_panch_r,
	gbs_ak_nom_dam_fence_down,
	gbs_ak_nom_dam_fence_fall,
	gbs_ak_nom_dam_fence_start,
	gbs_ak_nom_sleep_f,
	gbs_ak_nom_sleep_b,
	gbs_ak_nom_dam_bomb_fly_b,
	gbs_ak_nom_dam_bomb_fly_f,
	gbs_ak_nom_dam_elude,
	gbs_ak_nom_dam_out_bleed,
	gbs_ak_nom_dam_okaji_end,
	gbs_ak_nom_dam_okaji,
	gbs_ak_nom_dam_stun_light,
	gbs_ak_nom_dam_stun_down_b,
	gbs_ak_nom_dam_fire_ex,
	gbs_ak_nom_avoid_hang,
	gbs_ak_nom_dam_punch2down,
	gbs_ak_nom_dam_stair_start_b,

	gbs_ak_nom_dam_fence_f_fall,
	gbs_ak_nom_dam_fence_f_down,
	gbs_ak_nom_dam_fence_f_start,
	gbs_ak_nom_dam_stair_start,
	gbs_ak_nom_dam_stair_roll_1,
	gbs_ak_nom_dam_stair_roll_2,
	gbs_ak_nom_dam_stair_end,
	gbs_ak_nom_push_ply_r,
	gbs_ak_nom_push_ply_l,
	gbs_ak_nom_squeezed_f,
	gbs_ak_nom_squeezed_b,
	gbs_ak_nom_dam_cliff_b,
	gbs_ak_nom_dam_cliff_f,

	gbs_ak_nom_dam_out_f_cap01,
	gbs_ak_nom_dam_out_f_cap02,
	gbs_ak_nom_dam_out_b_cap01,
	gbs_ak_nom_dam_out_b_cap02,
	gbs_ak_nom_dam_out_b_cap03,
	gbs_ak_nom_dam_out_b_cap04,
	gbs_ak_nom_dam_ball_cap01,

	/* 部位ダメージ */
	gbs_ak_nom_dam_arm_l,
	gbs_ak_nom_dam_leg_l,
	gbs_ak_nom_dam_legs,
	gbs_ak_nom_dam_ball,
	gbs_ak_arml_idle,
	gbs_ak_arml_run,
	gbs_ak_arml_walk,
	gbs_ak_arms_idle,
	gbs_ak_arms_run,
	gbs_ak_arms_walk,
	gbs_ak_legl_idle,
	gbs_ak_legl_run,
	gbs_ak_legl_walk,
	gbs_ak_legs_idle,
	gbs_ak_legs_okiru,
	gbs_ak_legs_dam_out,
	gbs_ak_armr_morph_arm,
	gbs_ak_legr_morph_legr,
	gbs_ak_legl_morph_legl,
	gbs_ak_legs_morph_arm,
	gbs_ak_legs_morph_leg,
	gbs_ak_nom_dam_arm_r,
	gbs_ak_nom_dam_leg_r,
	gbs_ak_armr_idle,
	gbs_ak_armr_run,
	gbs_ak_armr_walk,
	gbs_ak_legr_idle,
	gbs_ak_legr_run,
	gbs_ak_legr_walk,
	gbs_arml_nom_idle_holdup,
	gbs_armr_nom_idle_holdup,
	gbs_arms_nom_idle_holdup,

	/* 死体モーション */
	gbs_ak_dead_carry_body_end,
	gbs_ak_dead_carry_body_idle,
	gbs_ak_dead_carry_body_start_b,
	gbs_ak_dead_carry_body_start_f,
	gbs_ak_dead_carry_body_walk,
	gbs_ak_nom_down_f,
	gbs_ak_dead_carry_leg_end_b,
	gbs_ak_dead_carry_leg_end_f,
	gbs_ak_dead_carry_leg_idle_b,
	gbs_ak_dead_carry_leg_idle_f,
	gbs_ak_dead_carry_leg_start_b,
	gbs_ak_dead_carry_leg_start_f,
	gbs_ak_dead_carry_leg_walk_b,
	gbs_ak_dead_carry_leg_walk_f,
	gbs_ak_dead_carry_body_put_locker, 
	gbs_ak_dead_carry_put_toilet,
	gbs_ak_dead_carry_locker_idle,
	gbs_ak_dead_carry_locker_surprise,
	gbs_ak_dead_carry_leg_put_locker_f,
	gbs_ak_dead_carry_leg_put_locker_b,

	gbs_ak_nom_toilet_knock,
	gbs_ak_nom_toilet_peep_u,
	gbs_ak_nom_toilet_peep_d,


	// おしっこ -DDEL_SHOUBEN=DUMMY
	gbs_ak_nom_toilet_ready,
	gbs_ak_nom_toilet_putback,
	gbs_ak_nom_toilet_idle,

	// ウォークマン -D DEL_WALKMAN
	gbs_ak_nom_nori_nori,
	gbs_ak_nom_norinori_sleep,
	gbs_ak_nom_norinori_idle,
	gbs_ak_nom_norinori_walk,

	// 盾兵モーション -D DEL_SHIELD
	gbs_shl_nom_dam_gun,
	gbs_shl_nom_idle,
	gbs_shl_nom_squat_fire,
	gbs_shl_nom_walk,
	gbs_shl_nom_run,
	gbs_shl_nom_override,
	gbs_shl_nom_defence_idle,
	gbs_shl_nom_defence_squat,
	gbs_shl_nom_attack_near_kick,
	gbs_shl_nom_fire_side_p,
	gbs_shl_nom_signe_go,
	gbs_shl_nom_attack_near_shl,
	gbs_shl_nom_stomp,
	gbs_shl_nom_mkr_reload,

	gbs_shl_cle_walk,
	gbs_shl_cle_run,
	gbs_shl_nom_cle_slidewalk_l,
	gbs_shl_nom_cle_slidewalk_r,
	gbs_shl_nom_walk_b_atk,
	gbs_shl_nom_cle_look_l,
	gbs_shl_nom_cle_look_r,
	gbs_shl_cle_signe_idle,
	gbs_shl_nom_cle_seach_r2l,
	gbs_shl_nom_stair_run_down,


	//ショットガン -D DEL_SHOTGUN
	gbs_sps_nom_fire,
	gbs_sps_nom_fire_p,
	gbs_sps_nom_fire_reload,

	/*銃撃戦用モーション*/
	gbs_ak_nom_peek_r_start,
	gbs_ak_nom_peek_r_end,
	gbs_ak_nom_peek_r_squat_end,
	gbs_ak_nom_peek_r_squat_start,
	gbs_ak_nom_peek_r_squat_fire,
	gbs_ak_nom_peek_r_fire,
	gbs_ak_nom_fire_blind,
	gbs_ak_nom_fire_blind_squat,
	gbs_ak_nom_peek_l_start,
	gbs_ak_nom_peek_l_end,
	gbs_ak_nom_peek_l_fire,

/* 削除モーション 後で削除 */
	gbs_ak_nom_ottotto,
	gbs_ak_nom_poripori,
	gbs_ak_nom_break_seasick,
	gbs_ak_nom_guruguru,

	/* ステージ固有・keep */
	___EM_watch_pose_1,
	___EM_watch_pose_2,
	/* ステージ固有・OneTime */
	___EM_watch_OneTime_1,
	___EM_watch_OneTime_2,
	___EM_watch_OneTime_3,

	ene_standard_end
} ;

/* 重野君が対応後消す */
enum {
gbs_ak_nom_attack_near_shoulder= ene_standard_end,
gbs_ak_nom_attack_near_punch,
} ;

#else
enum {
	gbs_ak_nom_find_ply,			/*0*/
	gbs_ak_armr_cle_call_break_l,
	gbs_ak_armr_cle_call_team_l,
	gbs_ak_cau_mukade_idle,
	gbs_ak_cle_signe_go,
	gbs_ak_cle_signe_idle,
	gbs_ak_cle_walk,
	gbs_ak_dead_carry_body_end,
	gbs_ak_dead_carry_body_idle,
	gbs_ak_dead_carry_body_start_b,
	gbs_ak_dead_carry_body_start_f,	/* 10 */
	gbs_ak_dead_carry_body_walk,
	gbs_ak_dead_carry_leg_end_b,
	gbs_ak_dead_carry_leg_end_f,
	gbs_ak_dead_carry_leg_idle_b,
	gbs_ak_dead_carry_leg_idle_f,
	gbs_ak_dead_carry_leg_start_b,
	gbs_ak_dead_carry_leg_start_f,
	gbs_ak_dead_carry_leg_walk_b,
	gbs_ak_dead_carry_leg_walk_f,
	gbs_ak_dead_idle_f,				/* 20 */
	gbs_ak_nom_akubi,
	gbs_ak_nom_attack_near,
	gbs_ak_nom_attack_near_p,
	gbs_ak_nom_cle_call_break_r,
	gbs_ak_nom_cle_call_team_r,
	gbs_ak_nom_cle_look_l,
	gbs_ak_nom_cle_look_r,
	gbs_ak_nom_cle_run,
	gbs_ak_nom_cle_slidewalk_l,
	gbs_ak_nom_cle_slidewalk_r,		/* 30 */
	gbs_ak_nom_cle_walk_low,
	gbs_ak_nom_dam_bom_f,
	gbs_ak_nom_dam_fence_down,
	gbs_ak_nom_dam_fence_fall,
	gbs_ak_nom_dam_fence_start,
	gbs_ak_nom_dam_kick_t,
	gbs_ak_nom_dam_panch_l,
	gbs_ak_nom_dam_panch_r,
	gbs_ak_nom_dam_throw,
	gbs_ak_nom_dam_wall_down,		/* 40 */
	gbs_ak_nom_dam_wall_down_p,
	gbs_ak_nom_dam_wall_move_d,
	gbs_ak_nom_down_b,
	gbs_ak_nom_down_f,
	gbs_ak_nom_fire_p_high,
	gbs_ak_nom_fire_reload,
	gbs_ak_nom_glasses,
	gbs_ak_nom_glasses_p,
	gbs_ak_nom_idle_atk,
	gbs_ak_nom_ijiiji,			/* 50 */
	gbs_ak_nom_iya_escape,
	gbs_ak_nom_iya_hang,
	gbs_ak_nom_iya_idle,
	gbs_ak_nom_iya_release,
	gbs_ak_nom_iya_release_down,
	gbs_ak_nom_iya_resist,
	gbs_ak_nom_iya_tie,
	gbs_ak_nom_iya_walk,
	gbs_ak_nom_okiru_b,
	gbs_ak_nom_okiru_b_slow,				/* 60 */
	gbs_ak_nom_okiru_f,
	gbs_ak_nom_okiru_f_slow,
	gbs_ak_nom_run,
	gbs_ak_nom_run_atk,
	gbs_ak_nom_senobi,
	gbs_ak_nom_slide_fire_l,
	gbs_ak_nom_slide_fire_r,
	gbs_ak_nom_squat_fire,
	gbs_ak_nom_squat_fire_reload,
	gbs_ak_nom_squat_hide_idle,		/* 70 */
	gbs_ak_nom_squat_roll_l,
	gbs_ak_nom_squat_roll_r,
	gbs_ak_nom_step_l,
	gbs_ak_nom_step_r,
	gbs_ak_nom_tobidasi_l_t,
	gbs_ak_nom_tobidasi_r_t,
	gbs_ak_nom_turn_b_t,
	gbs_ak_nom_turn_l_t,
	gbs_ak_nom_turn_r_t,
	gbs_ak_nom_walk,					/* 80 */
	gbs_ak_nom_zzz,
	gbs_non_nom_idle_holdup,
	gbs_ak_nom_walk_light,
	gbs_ak_nom_ottotto,
	gbs_ak_nom_poripori,
	gbs_ak_nom_nori_nori,
	gbs_ak_nom_dam_gun_1,
	gbs_ak_nom_dam_gun_2,
	gbs_ak_nom_dam_gun_3,
	gbs_ak_nom_sleep_f,				/* 90 */
	gbs_ak_nom_sleep_b,
	gbs_ak_nom_behind_idle,
	gbs_ak_nom_walk_b_atk,
	gbs_ak_nom_dam_bomb_fly_b,
	gbs_ak_nom_dam_bomb_fly_f,
	gbs_ak_nom_cle_seach_r2l,
	gbs_ak_nom_dam_down_f,
	gbs_ak_nom_dam_down_b,
	gbs_ak_nom_dam_out,
	gbs_ak_mukade_run1,			/* 100 */
	gbs_ak_mukade_run4,
	gbs_ak_mukade_run2,
	gbs_ak_mukade_run3,
	gbs_ak_sign_mukade_go,
	gbs_ak_armr_cle_call_team_l_1start,
	gbs_ak_armr_cle_call_team_l_2talk,
	gbs_ak_armr_cle_call_team_l_3end,
	gbs_ak_nom_break_seasick,
	gbs_ak_nom_cle_call_team_gun_l_1_start,
	gbs_ak_nom_cle_call_team_gun_l_2_talk	/* 110 */,
	gbs_ak_nom_cle_call_team_gun_l_3_end,
	gbs_ak_nom_look_erotica,
	gbs_ak_nom_look_erotica_loop2,
	gbs_ak_nom_look_erotica_loop1,

	gbs_ak_nom_look_erotica_stand, 
	gbs_ak_nom_look_erotica_stand_loop2, 
	gbs_ak_nom_look_erotica_stand_loop1, 

	gbs_ak_nom_cle_call_team_gun_l_1_start_fast,
	gbs_ak_armr_cle_call_team_l_1_start_fast,
	gbs_ak_nom_attack_near_shoulder,
	gbs_ak_nom_attack_near_punch,
	gbs_ak_nom_attack_near_kick,
	gbs_ak_nom_attack_near_kick_r,	/*120*/
	gbs_ak_nom_attack_near_kick_l,
	gbs_ak_nom_attack_near_punch_r, 
	gbs_ak_nom_attack_near_punch_l, 
	gbs_ak_nom_snipe_fire_p,
	gbs_non_nom_idle_holdup_surprise,
	gbs_ak_nom_turn_out,
	gbs_ak_dead_carry_body_put_locker, 
	gbs_ak_dead_carry_locker_idle,
	gbs_ak_dead_carry_leg_put_locker_f,
	gbs_ak_dead_carry_leg_put_locker_b,	/*130*/
	gbs_non_nom_idle_holdup_item,

	gbs_ak_arml_cle_call_break_r,
	gbs_ak_arml_cle_call_team_r_1start,
	gbs_ak_arml_cle_call_team_r_2talk,
	gbs_ak_arml_cle_call_team_r_3end,

	gbs_ak_nom_fire_intrude,
	gbs_ak_nom_box_look,
	gbs_ak_nom_box_look_idle,
	gbs_ak_nom_box_kick,
	gbs_ak_nom_box_look2stand, 
	gbs_ak_nom_box_lift,	
	gbs_ak_nom_box_lift_p,
	gbs_ak_nom_box_lift2throw,
	gbs_ak_nom_see_patient,
	gbs_ak_nom_dam_punch2down,
	gbs_ak_nom_dam_stair_start_b,


	gbs_ak_nom_dam_arm_l,			/*140*/
	gbs_ak_nom_dam_leg_l,
	gbs_ak_nom_dam_legs,
	gbs_ak_nom_dam_ball,
	gbs_ak_arml_idle,
	gbs_ak_arml_run,
	gbs_ak_arml_walk,
	gbs_ak_arms_idle,
	gbs_ak_arms_run,
	gbs_ak_arms_walk,
	gbs_ak_legl_idle,			/*150*/
	gbs_ak_legl_run,
	gbs_ak_legl_walk,
	gbs_ak_legs_idle,
	gbs_ak_legs_okiru,
	gbs_ak_legs_dam_out,
	gbs_ak_armr_morph_arm,
	gbs_ak_legr_morph_legr,
	gbs_ak_legl_morph_legl,
	gbs_ak_legs_morph_arm,
	gbs_ak_legs_morph_leg,		/*160*/
	gbs_ak_nom_dam_arm_r,
	gbs_ak_nom_dam_leg_r,
	gbs_ak_armr_idle,
	gbs_ak_armr_run,
	gbs_ak_armr_walk,
	gbs_ak_legr_idle,
	gbs_ak_legr_run,
	gbs_ak_legr_walk,
	gbs_ak_nom_dam_elude,
	gbs_ak_nom_dam_out_bleed,	/*170 */
	gbs_ak_dam_escape,			/* その場で転倒 */
	gbs_ak_nom_dam_okaji_end,
	gbs_ak_nom_dam_okaji,
	gbs_ak_nom_dam_stun_light,
	gbs_ak_nom_dam_stun_down_b,
	gbs_ak_nom_dam_fire_ex,
	gbs_ak_nom_avoid_hang,


	gbs_ak_nom_stomp,
	gbs_ak_nom_surprised_bomb,
	gbs_ak_nom_signe_go_atk,	/*180*/
	gbs_ak_nom_norinori_sleep,
	gbs_ak_nom_norinori_idle,
	gbs_ak_nom_norinori_walk,
	gbs_ak_nom_gogle_off,
	gbs_ak_nom_gogle_on,
	gbs_ak_nom_push_ply_r,
	gbs_ak_nom_push_ply_l,
	gbs_ak_nom_squeezed_f,
	gbs_ak_nom_squeezed_b,

	gbs_ak_nom_watch_pose1,	/*190*/
	gbs_ak_nom_watch_pose2,

	gbs_ak_nom_watch_onetime1,
	gbs_ak_nom_watch_onetime2,
	gbs_ak_nom_watch_onetime3,

	gbs_ak_nom_dam_out_f_cap01,
	gbs_ak_nom_dam_out_f_cap02,
	gbs_ak_nom_dam_out_b_cap01,
	gbs_ak_nom_dam_out_b_cap02,
	gbs_ak_nom_dam_out_b_cap03,
	gbs_ak_nom_dam_out_b_cap04,
	gbs_ak_nom_dam_ball_cap01,

	gbs_gm_fire_high,
	gbs_gm_fire_low,
	gbs_gm_fire_korokoro,		/*200*/
	gbs_ak_nom_dam_fence_f_fall,
	gbs_ak_nom_dam_fence_f_down,
	gbs_ak_nom_dam_fence_f_start,
	gbs_ak_nom_walk_pass,

	gbs_ak_nom_dam_stair_start,
	gbs_ak_nom_dam_stair_roll_1,
	gbs_ak_nom_dam_stair_roll_2,
	gbs_ak_nom_dam_stair_end,
	gbs_ak_dead_carry_locker_surprise,
	gbs_ak_nom_stair_run_down,		/*210*/
	gbs_ak_nom_stair_walk_down,
	gbs_ak_nom_stair_run_up,
	gbs_ak_nom_stair_walk_up,

	gbs_ak_cle_signe_clear,
	gbs_ak_cle_signe_move,
	gbs_ak_cle_signe_stop,
	gbs_ak_nom_cle_locker_open,
	gba_ak_nom_call_team_l,

	gbs_ak_cle_go_ahead,
	gbs_arml_nom_idle_holdup,
	gbs_armr_nom_idle_holdup,
	gbs_arms_nom_idle_holdup,
	gbs_ak_nom_guruguru,

	gbs_shl_nom_dam_gun,
	gbs_shl_nom_idle,			/*220*/
	gbs_shl_nom_squat_fire,
	gbs_shl_nom_walk,
	gbs_shl_nom_run,
	gbs_shl_nom_override,
	gbs_shl_nom_defence_idle,
	gbs_shl_nom_defence_squat,
	gbs_shl_nom_attack_near_kick,
	gbs_shl_nom_fire_side_p,
	gbs_shl_nom_signe_go,
	gbs_shl_nom_attack_near_shl,	/*230*/
	gbs_shl_nom_stomp,
	gbs_shl_nom_mkr_reload,


	gbs_ak_nom_peek_r_start,
	gbs_ak_nom_peek_r_end,
	gbs_ak_nom_peek_r_squat_end,
	gbs_ak_nom_peek_r_squat_start,
	gbs_ak_nom_peek_r_squat_fire,
	gbs_ak_nom_peek_r_fire,
	gbs_ak_nom_fire_blind,
	gbs_ak_nom_fire_blind_squat,	/*240*/

	gbs_ak_nom_peek_l_start,
	gbs_ak_nom_peek_l_end,
	gbs_ak_nom_peek_l_fire,

	gbs_sps_nom_fire,
	gbs_sps_nom_fire_p,
	gbs_sps_nom_fire_reload,		/*256*/

/* 削除モーション 後で削除 */
	gbs_ak_nom_ottotto,
	gbs_ak_nom_poripori,
	gbs_ak_nom_break_seasick,
	gbs_ak_nom_guruguru,

	/* ステージ固有・keep */
	___EM_watch_pose_1,
	___EM_watch_pose_2,
	/* ステージ固有・OneTime */
	___EM_watch_OneTime_1,
	___EM_watch_OneTime_2,
	___EM_watch_OneTime_3,

	ene_standard_end
} ;
#endif
