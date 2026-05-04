/*
   motion.h 
   ¥é¥¤¥Ç¥ó¡¿¥â¡¼¥·¥ç¥óÄêµÁ¥Õ¥¡¥¤¥ë
   
   1999/07/07 M.Sonoyama
   $Id: motion.h,v 1.1.1.3 2002/11/19 11:50:56 Yoshizawa1 Exp $			
   */

enum snake {
    none_stand,
    m92_stand,
    m92_ready,
    m92_fire,
    m92_reload,
    none_squat,
    none_squat_crouch,
    none_crouch,
    none_crouch_f,
    none_crouch_b,
    none_crouch_stand,
    none_damage_f,
    none_damage_b,
    none_damage_out,
    none_damage_near,
    none_down,
    none_down_stand,
    none_walk,
    none_run,
    none_hang,
    none_tie,
    none_kill,
    none_hang_walk,
    none_combo_ready,
    none_combo,
    none_carry_b_start_f,
    none_carry_b_start_b,
    none_carry_b,
    none_carry_b_walk,
    none_carry_b_end,
    none_carry_l_start,
    none_carry_l,
    none_carry_l_walk,
    none_carry_l_end,
    none_throw_l,
    none_throw_r,
    none_carry_miss,
    none_damage_crouch,
    none_damage_down,
    none_damage_down_out,
    m92_ready_crouch,
    m92_fire_crouch,
    m92_reload_crouch,
    none_stair_walk,
    none_stair_run,
    m92_walk,
    m92_run,
    m92_stair_walk,
    m92_stair_run,
    none_tumble,
    m92_squat,
    box_idle,
    box_run,
    box_walk,
	box_stop,
    none_caution,
    none_squat_caution,
    none_caution_l,
    none_caution_r,
    none_behind_l,
    none_behind_r,
    none_behind_lb_start,
    none_behind_lb,
    none_behind_rb_start,
    none_behind_rb,
    none_behind_lb_end,
    none_behind_rb_end,
    non_knock_r1,
    non_knock_r2,
    non_knock_l1,
    non_knock_l2,
	non_knock_sq1,
	non_knock_sq2,
	m4a_knock_sq1,
	m4a_knock_sq2,
	nkt_knock_sq1,
	nkt_knock_sq2,
	gm_knock_sq1,
	gm_knock_sq2,
    m92_caution,
    m92_squat_caution,
    m92_caution_l,
    m92_caution_r,
    m92_behind_l,
    m92_behind_r,
    m92_behind_lb_start,
    m92_behind_lb,
    m92_behind_rb_start,
    m92_behind_rb,
    m92_behind_lb_end,
    m92_behind_rb_end,
    m92_knock_r1,
    m92_knock_r2,
    m92_knock_l1,
    m92_knock_l2,
    m92_behind_at_r_s,
    m92_behind_at_r_e,
    m92_behind_at_l_s,
    m92_behind_at_l_e,
    m92_behind_r_sq,
    m92_behind_l_sq,
    m92_behind_rb_start_sq,
    m92_behind_rb_sq,
    m92_behind_rb_end_sq,
    m92_behind_lb_start_sq,
    m92_behind_lb_sq,
    m92_behind_lb_end_sq,
    m92_behind_at_r_s_sq,
    m92_behind_at_r_e_sq,
    m92_behind_at_l_s_sq,
    m92_behind_at_l_e_sq,
    non_tumble_end_cr,
    m92_caution_r_sq,
    m92_caution_l_sq,
    non_tumble_end_st,
    none_down_f,
    none_down_stand_f,
    none_crouch_f_fast,
	non_tumble_miss,
	none_damage_down_f,
	none_cr2sq_f,
	none_cr2sq_b,
	none_walk_on,
	non_push_walk_l_l,
	non_push_walk_l_r,
	non_pushed_lf,
	non_pushed_lb,
	non_damage_caution,
	non_out_caution,
	non_damage_caution_sq,
	non_out_caution_sq,
	non_squat_crouch_int,
	non_crouch_stand_int,
	non_hang_escape,
	non_hang_release,
	non_hang_miss,
	gm_idle,
	gm_squat,
	gm_walk,
	gm_run,
	non_caution_r_sq,
	non_caution_l_sq,
	gm_caution,
	gm_caution_r,
	gm_caution_l,
	gm_caution_sq,
	gm_caution_r_sq,
	gm_caution_l_sq,
	gm_knock_r1,
	gm_knock_r2,
	gm_knock_l1,
	gm_knock_l2,
	non_behind_r_sq,
	non_behind_l_sq,
	non_behind_rb_start_sq,
	non_behind_rb_sq,
	non_behind_rb_end_sq,
	non_behind_lb_start_sq,
	non_behind_lb_sq,
	non_behind_lb_end_sq,
	gm_behind_r,
	gm_behind_l,
	gm_behind_rb_start,
	gm_behind_rb,
	gm_behind_rb_end,
	gm_behind_lb_start,
	gm_behind_lb,
	gm_behind_lb_end,
	gm_behind_r_sq,
	gm_behind_l_sq,
	gm_behind_rb_start_sq,
	gm_behind_rb_sq,
	gm_behind_rb_end_sq,
	gm_behind_lb_start_sq,
	gm_behind_lb_sq,
	gm_behind_lb_end_sq,
	non_blow_soft,
	gm_fire_start,
	gm_fire_low_5pose,
	gm_fire_low_min,
	gm_fire_low_max,
    none_damage_down_out_f,
	non_dam_out_intrude,
	box_walk_on,
	gm_fire_start_bhl,
	gm_fire_min_bhl,
	gm_fire_max_bhl,
	gm_fire_5pose_bhl,
	gm_fire_start_bhr,
	gm_fire_min_bhr,
	gm_fire_max_bhr,
	gm_fire_5pose_bhr,
	non_idle_snz,
	non_crouch_snz,
	non_caution_snz,
	gm_fire_start_g, 
	gm_fire_min_g, 
	gm_fire_max_g, 
	gm_fire_5pose_g,
	scp_fire_p,
	scp_fire_p_cr,
	cam_fire_p,
	cam_fire_p_cr,
	/* °Ê²¼¥é¥¤¥Ç¥óÀìÍÑ */
	hfb_fire_p,
	hfb_walk,
	hfb_dash,
	non_damage_f_sps,
	non_damage_b_sps,
	psg_fire_stand,
	psg_fire_squat,
	psg_fire_crouch,
	m4a_idle,
	m4a_fire_p,
	m4a_fire,
	m4a_fire_end,
	m4a_fire_reload,
	m4a_crouch_fire_p,
	m4a_crouch_fire,
	m4a_crouch_fire_end,
	m4a_crouch_fire_reload,
	m4a_squat,
	m4a_walk,
	m4a_run,
	m4a_crouch_start,
	m4a_crouch_idle,
	m4a_crouch_move_f,
	m4a_crouch_move_b,
	m4a_crouch_move_f_fast,
	m4a_crouch_end,
	m4a_cr2sq_f,
	m4a_cr2sq_b,
	m4a_sq2intrude,
	m4a_intrude2stand,
	m4a_stair_walk,
	m4a_stair_run,
	m4a_behind_idle,
	m4a_behind_move_r,
	m4a_behind_move_l,
	m4a_behind_idle_sq,
	m4a_behind_move_r_sq,
	m4a_behind_move_l_sq,
	m4a_knock_r1,
	m4a_knock_r2,
	m4a_knock_l1,
	m4a_knock_l2,
	m4a_behind_look_r,
	m4a_behind_look_l,
	m4a_behind_look_rb_start,
	m4a_behind_look_rb_p,
	m4a_behind_look_rb_end,
	m4a_behind_look_lb_start,
	m4a_behind_look_lb_p,
	m4a_behind_look_lb_end,
	m4a_behind_look_r_sq,
	m4a_behind_look_l_sq,
	m4a_behind_look_rb_start_sq,
	m4a_behind_look_rb_p_sq,
	m4a_behind_look_rb_end_sq,
	m4a_behind_look_lb_start_sq,
	m4a_behind_look_lb_p_sq,
	m4a_behind_look_lb_end_sq,
	m4a_behind2wb_r_start,
	m4a_behind2wb_r_end,
	m4a_behind2wb_l_start,
	m4a_behind2wb_l_end,
	m4a_behind2wb_r_start_sq,
	m4a_behind2wb_r_end_sq,
	m4a_behind2wb_l_start_sq,
	m4a_behind2wb_l_end_sq,
	m4a_sneeze_idle,
	m4a_sneeze_crouch,
	m4a_sneeze_behind,
	nkt_idle,
	nkt_squat,
	nkt_walk,
	nkt_run,
	nkt_wallcheck_idle,
	nkt_wallcheck_move,
	nkt_stair_walk,
	nkt_stair_run,
	nkt_behind_idle,
	nkt_behind_move_r,
	nkt_behind_move_l,
	nkt_behind_idle_sq,
	nkt_behind_move_r_sq,
	nkt_behind_move_l_sq,
	nkt_knock_r1,
	nkt_knock_r2,
	nkt_knock_l1,
	nkt_knock_l2,
	nkt_behind_look_r,
	nkt_behind_look_l,
	nkt_behind_look_rb_start,
	nkt_behind_look_rb_p,
	nkt_behind_look_rb_end,
	nkt_behind_look_lb_start,
	nkt_behind_look_lb_p,
	nkt_behind_look_lb_end,
	nkt_behind_look_r_sq,
	nkt_behind_look_l_sq,
	nkt_behind_look_rb_start_sq,
	nkt_behind_look_rb_p_sq,
	nkt_behind_look_rb_end_sq,
	nkt_behind_look_lb_start_sq,
	nkt_behind_look_lb_p_sq,
	nkt_behind_look_lb_end_sq,
	nkt_fire_p,
	nkt_fire_p_sq,
	nkt_fire_p_cr,
	stg_fire_p,
	ak_reload,
	ak_reload_cr,
	c4_behind_idle,
	c4_behind_idle_sq,
	c4_set_behind,
	c4_set_behind_sq,
	c4_set_enemy,
	c4_set_floor,
	c4_set_floor_cr,
	clay_idle,
	clay_squat,
	clay_set_floor_start,
	clay_set_floor_idle,
	clay_set_floor_end,
	clay_set_floor_cr_start,
	clay_set_floor_cr_idle,
	clay_set_floor_cr_end,
	rgb_crouch_idle,
	rgb_fire_p,
	rgb_fire,
	rgb_reload,
	rgb_fire_p_cr,
	rgb_fire_cr,
	rgb_reload_cr,
	spr_fire_p,
	spr_fire_p_cr,
	mic_fire_p,
	mic_fire_p_cr,
	non_dam_trip_f,
	non_dam_trip_b,
	nkt_crouch_start,
	nkt_crouch_idle,
	nkt_crouch_move_f,
	nkt_crouch_move_b,
	nkt_crouch_move_f_fast,
	nkt_crouch_end,
	nkt_cr2sq_f,
	nkt_cr2sq_b,
	nkt_sq2intrude,
	nkt_intrude2stand,
	nkt_rise,
	nkt_rise_f,
	m4a_walk_on_corpse,
	m9_wallcheck_idle,
	m9_wallcheck_move,
	m4a_combo,
	nkt_fire_strike,
	non_hang_through,
    MAX_MOTIONS
};

#define	NO_ACT	MAX_MOTIONS

/*--------Éð´ïËè¤ËÊÑ¹¹--------*/

#ifdef RAIDEN_C

/* ÁÇ¼ê */
static	short	NoneSet[] = {
    none_stand, none_combo_ready, NO_ACT, none_walk, none_run, NO_ACT, NO_ACT,
    none_stand, none_stair_walk, none_stair_run, none_walk_on,
	none_squat, none_crouch, none_crouch_f,
    none_crouch_b, none_crouch_f_fast,
    none_caution, none_squat_caution, none_caution_r, none_caution_l,
    non_caution_r_sq, non_caution_l_sq,
    none_behind_r, none_behind_rb_start, none_behind_rb, none_behind_rb_end, none_behind_l, 
    none_behind_lb_start, none_behind_lb, none_behind_lb_end, 
    NO_ACT, NO_ACT, NO_ACT, NO_ACT,
    non_behind_r_sq, non_behind_l_sq, non_behind_rb_start_sq, non_behind_rb_sq,
    non_behind_rb_end_sq, non_behind_lb_start_sq, non_behind_lb_sq, non_behind_lb_end_sq,
    NO_ACT, NO_ACT, NO_ACT, NO_ACT, 
    non_knock_r1, non_knock_l1, non_knock_r2, non_knock_l2,
	non_knock_sq1, non_knock_sq2,
    none_squat_crouch, none_crouch_stand, non_squat_crouch_int, non_crouch_stand_int,
	none_cr2sq_f, none_cr2sq_b
} ;

/* £Í£¹£² */
static	short	M92Set[] = {
    m92_stand, m92_stand, NO_ACT, m92_walk, m92_run, NO_ACT, NO_ACT,
    m92_stand, m92_stair_walk, m92_stair_run, none_walk_on,
	m92_squat, m4a_crouch_idle, m4a_crouch_move_f,
    m4a_crouch_move_b, m4a_crouch_move_f_fast,
    m92_caution, m92_squat_caution, m92_caution_r, m92_caution_l,
    m92_caution_r_sq, m92_caution_l_sq,
    m92_behind_r, m92_behind_rb_start, m92_behind_rb, m92_behind_rb_end, m92_behind_l, 
    m92_behind_lb_start, m92_behind_lb, m92_behind_lb_end, 
    m92_behind_at_r_s, m92_behind_at_r_e, m92_behind_at_l_s, m92_behind_at_l_e,
    m92_behind_r_sq, m92_behind_l_sq, m92_behind_rb_start_sq, m92_behind_rb_sq,
    m92_behind_rb_end_sq, m92_behind_lb_start_sq, m92_behind_lb_sq,m92_behind_lb_end_sq,
    m92_behind_at_r_s_sq, m92_behind_at_r_e_sq, m92_behind_at_l_s_sq, m92_behind_at_l_e_sq, 
    m92_knock_r1, m92_knock_l1, m92_knock_r2, m92_knock_l2,
	non_knock_sq1, non_knock_sq2,
    none_squat_crouch, none_crouch_stand, non_squat_crouch_int, non_crouch_stand_int,
	none_cr2sq_f, none_cr2sq_b
} ;

/* £Õ£Ó£Ð */
static	short	USPSet[] = {
    m92_stand, m92_stand, NO_ACT, m92_walk, m92_run, NO_ACT, NO_ACT,
    m92_stand, m92_stair_walk, m92_stair_run, none_walk_on,
	none_squat, none_crouch, none_crouch_f,
    none_crouch_b, none_crouch_f_fast,
    m92_caution, m92_squat_caution, m92_caution_r, m92_caution_l,
    m92_caution_r_sq, m92_caution_l_sq,
    m92_behind_r, m92_behind_rb_start, m92_behind_rb, m92_behind_rb_end, m92_behind_l, 
    m92_behind_lb_start, m92_behind_lb, m92_behind_lb_end, 
    m92_behind_at_r_s, m92_behind_at_r_e, m92_behind_at_l_s, m92_behind_at_l_e,
    m92_behind_r_sq, m92_behind_l_sq, m92_behind_rb_start_sq, m92_behind_rb_sq,
    m92_behind_rb_end_sq, m92_behind_lb_start_sq, m92_behind_lb_sq,m92_behind_lb_end_sq,
    m92_behind_at_r_s_sq, m92_behind_at_r_e_sq, m92_behind_at_l_s_sq, m92_behind_at_l_e_sq, 
    m92_knock_r1, m92_knock_l1, m92_knock_r2, m92_knock_l2,
	m4a_knock_sq1, m4a_knock_sq2,
    none_squat_crouch, none_crouch_stand, non_squat_crouch_int, non_crouch_stand_int,
	none_cr2sq_f, none_cr2sq_b
} ;

/* £Ó£Ï£Ã£Ï£Í */
static	short	SocomSet[] = {
    m92_stand, m92_stand, m9_wallcheck_idle, m92_walk, m92_run, NO_ACT, m9_wallcheck_move,
    m92_stand, m92_stair_walk, m92_stair_run, none_walk_on,
	none_squat, none_crouch, none_crouch_f,
    none_crouch_b, none_crouch_f_fast,
    m92_caution, m92_squat_caution, m92_caution_r, m92_caution_l,
    m92_caution_r_sq, m92_caution_l_sq,
    m92_behind_r, m92_behind_rb_start, m92_behind_rb, m92_behind_rb_end, m92_behind_l, 
    m92_behind_lb_start, m92_behind_lb, m92_behind_lb_end, 
    m92_behind_at_r_s, m92_behind_at_r_e, m92_behind_at_l_s, m92_behind_at_l_e,
    m92_behind_r_sq, m92_behind_l_sq, m92_behind_rb_start_sq, m92_behind_rb_sq,
    m92_behind_rb_end_sq, m92_behind_lb_start_sq, m92_behind_lb_sq,m92_behind_lb_end_sq,
    m92_behind_at_r_s_sq, m92_behind_at_r_e_sq, m92_behind_at_l_s_sq, m92_behind_at_l_e_sq, 
    m92_knock_r1, m92_knock_l1, m92_knock_r2, m92_knock_l2,
	non_knock_sq1, non_knock_sq2,
    none_squat_crouch, none_crouch_stand, non_squat_crouch_int, non_crouch_stand_int,
	none_cr2sq_f, none_cr2sq_b
} ;

/* £Í£´ */
static	short	M4Set[] = {
    m4a_idle, m4a_idle, NO_ACT, m4a_walk, m4a_run, NO_ACT, NO_ACT,
    m4a_idle, m4a_stair_walk, m4a_stair_run, m4a_walk_on_corpse,
	m4a_squat, m4a_crouch_idle, m4a_crouch_move_f,
    m4a_crouch_move_b, m4a_crouch_move_f_fast,
    m4a_behind_idle, m4a_behind_idle_sq, m4a_behind_move_r, m4a_behind_move_l,
	m4a_behind_move_r_sq, m4a_behind_move_l_sq,
    m4a_behind_look_r, m4a_behind_look_rb_start, m4a_behind_look_rb_p, m4a_behind_look_rb_end, 
    m4a_behind_look_l, m4a_behind_look_lb_start, m4a_behind_look_lb_p, m4a_behind_look_lb_end, 
	m4a_behind2wb_r_start, m4a_behind2wb_r_end,	m4a_behind2wb_l_start, m4a_behind2wb_l_end,
    m4a_behind_look_r_sq, m4a_behind_look_l_sq, 
	m4a_behind_look_rb_start_sq, m4a_behind_look_rb_p_sq, m4a_behind_look_rb_end_sq, 
	m4a_behind_look_lb_start_sq, m4a_behind_look_lb_p_sq, m4a_behind_look_lb_end_sq, 
	m4a_behind2wb_r_start_sq, m4a_behind2wb_r_end_sq, m4a_behind2wb_l_start_sq, m4a_behind2wb_l_end_sq,
    m4a_knock_r1, m4a_knock_l1, m4a_knock_r2, m4a_knock_l2,
	m4a_knock_sq1, m4a_knock_sq2,
    m4a_crouch_start, m4a_crouch_end, m4a_sq2intrude, m4a_intrude2stand,
	m4a_cr2sq_f, m4a_cr2sq_b
} ;

/* £Ã£´ */
static	short	C4Set[] = {
    gm_idle, none_combo_ready, NO_ACT, none_walk, none_run, NO_ACT, NO_ACT,
    gm_idle, none_stair_walk, none_stair_run, none_walk_on,
	none_squat, none_crouch, none_crouch_f,
    none_crouch_b, none_crouch_f_fast,
    gm_caution, gm_caution_sq, gm_caution_r, gm_caution_l,
    gm_caution_r_sq, gm_caution_l_sq,
    gm_behind_r, none_behind_rb_start, none_behind_rb, none_behind_rb_end, gm_behind_l, 
    gm_behind_lb_start, gm_behind_lb, gm_behind_lb_end, 
    NO_ACT, NO_ACT, NO_ACT, NO_ACT,
    gm_behind_r_sq, gm_behind_l_sq, non_behind_rb_start_sq, non_behind_rb_sq,
    non_behind_rb_end_sq, gm_behind_lb_start_sq, gm_behind_lb_sq, gm_behind_lb_end_sq,
    NO_ACT, NO_ACT, NO_ACT, NO_ACT, 
    gm_knock_r1, gm_knock_l1, gm_knock_r2, gm_knock_l2,
	gm_knock_sq1, gm_knock_sq2,
    none_squat_crouch, none_crouch_stand, non_squat_crouch_int, non_crouch_stand_int,
	none_cr2sq_f, none_cr2sq_b
} ;

/* ¥¯¥ì¥¤¥â¥¢ */
static	short	ClaymoreSet[] = {
    gm_idle, none_combo_ready, NO_ACT, gm_walk, gm_run, NO_ACT, NO_ACT,
    gm_idle, none_stair_walk, none_stair_run, none_walk_on,
	gm_squat, nkt_crouch_idle, nkt_crouch_move_f,
    nkt_crouch_move_b, nkt_crouch_move_f_fast,
    none_caution, none_squat_caution, none_caution_r, none_caution_l,
    non_caution_r_sq, non_caution_l_sq,
    none_behind_r, none_behind_rb_start, none_behind_rb, none_behind_rb_end, none_behind_l, 
    none_behind_lb_start, none_behind_lb, none_behind_lb_end, 
    NO_ACT, NO_ACT, NO_ACT, NO_ACT,
    non_behind_r_sq, non_behind_l_sq, non_behind_rb_start_sq, non_behind_rb_sq,
    non_behind_rb_end_sq, non_behind_lb_start_sq, non_behind_lb_sq, non_behind_lb_end_sq,
    NO_ACT, NO_ACT, NO_ACT, NO_ACT, 
    non_knock_r1, non_knock_l1, non_knock_r2, non_knock_l2,
	non_knock_sq1, non_knock_sq2,
    none_squat_crouch, none_crouch_stand, non_squat_crouch_int, non_crouch_stand_int,
	none_cr2sq_f, none_cr2sq_b
} ;

/* ¥°¥ì¥Í¡¼¥É */
static	short	GrenadeSet[] = {
    gm_idle, none_combo_ready, NO_ACT, gm_walk, gm_run, NO_ACT, NO_ACT,
    gm_idle, none_stair_walk, none_stair_run, none_walk_on,
	gm_squat, none_crouch, none_crouch_f,
    none_crouch_b, none_crouch_f_fast,
    gm_caution, gm_caution_sq, gm_caution_r, gm_caution_l,
    gm_caution_r_sq, gm_caution_l_sq,
    gm_behind_r, gm_behind_rb_start, gm_behind_rb, gm_behind_rb_end, gm_behind_l, 
    gm_behind_lb_start, gm_behind_lb, gm_behind_lb_end, 
    NO_ACT, NO_ACT, NO_ACT, NO_ACT,
    gm_behind_r_sq, gm_behind_l_sq, gm_behind_rb_start_sq, gm_behind_rb_sq,
    gm_behind_rb_end_sq, gm_behind_lb_start_sq, gm_behind_lb_sq, gm_behind_lb_end_sq,
    NO_ACT, NO_ACT, NO_ACT, NO_ACT, 
    gm_knock_r1, gm_knock_l1, gm_knock_r2, gm_knock_l2,
	gm_knock_sq1, 	gm_knock_sq2,
    none_squat_crouch, none_crouch_stand, non_squat_crouch_int, non_crouch_stand_int,
	none_cr2sq_f, none_cr2sq_b
} ;

/* ¥Ë¥­¡¼¥¿ */
static	short	NikitaSet[] = {
    nkt_idle, nkt_idle, nkt_wallcheck_idle, nkt_walk, nkt_run, NO_ACT, nkt_wallcheck_move,
    nkt_idle, nkt_stair_walk, nkt_stair_run, none_walk_on,
	nkt_squat, nkt_crouch_idle, nkt_crouch_move_f,
    nkt_crouch_move_b, nkt_crouch_move_f_fast,
    nkt_behind_idle, nkt_behind_idle_sq, nkt_behind_move_r, nkt_behind_move_l,
    nkt_behind_move_r_sq, nkt_behind_move_l_sq, 
	nkt_behind_look_r, nkt_behind_look_rb_start, nkt_behind_look_rb_p, nkt_behind_look_rb_end,
	nkt_behind_look_l, nkt_behind_look_lb_start, nkt_behind_look_lb_p, nkt_behind_look_lb_end,
    NO_ACT, NO_ACT, NO_ACT, NO_ACT,
	nkt_behind_look_r_sq, nkt_behind_look_l_sq, nkt_behind_look_rb_start_sq, nkt_behind_look_rb_p_sq,
	nkt_behind_look_rb_end_sq, nkt_behind_look_lb_start_sq, 
	nkt_behind_look_lb_p_sq, nkt_behind_look_lb_end_sq,
    NO_ACT, NO_ACT, NO_ACT, NO_ACT,
    nkt_knock_r1, nkt_knock_l1, nkt_knock_r2, nkt_knock_l2,
	nkt_knock_sq1, 	nkt_knock_sq2,
    nkt_crouch_start, nkt_crouch_end, nkt_sq2intrude, nkt_intrude2stand,
	nkt_cr2sq_f, nkt_cr2sq_b
} ;

#define	RGBSet		M4Set
#define	StingerSet	C4Set
#define	BladeSet	NoneSet
#define	AksSet		M4Set

static	short	*ChangeSets[] = {
    NoneSet, M92Set, USPSet, SocomSet, NoneSet, 
	RGBSet, NikitaSet, StingerSet, ClaymoreSet, C4Set, 
	GrenadeSet, GrenadeSet, NoneSet, BladeSet, NoneSet, 
    AksSet, GrenadeSet, GrenadeSet,	M4Set, NoneSet,
	NoneSet, ClaymoreSet
} ;

/*--------Éð´ï¹¶·âÍÑ--------*/

/* £Í£¹£² */
static	short	M92AtkSet[] = {
    m92_ready, m92_fire, m92_reload, NO_ACT,
    m92_ready_crouch, m92_fire_crouch, m92_reload_crouch, NO_ACT
} ;

/* £Õ£Ó£Ð */
static	short	USPAtkSet[] = {
    m92_ready, m92_fire, m92_reload, NO_ACT,
    m92_ready_crouch, m92_fire_crouch, m92_reload_crouch, NO_ACT
} ;

/* £Ó£Ï£Ã£Ï£Í */
static	short	SocomAtkSet[] = {
    m92_ready, m92_fire, m92_reload, NO_ACT,
    m92_ready_crouch, m92_fire_crouch, m92_reload_crouch, NO_ACT
} ;

#if 0
/* £Æ£Á£Í£Á£Ó */
static	short	FamasAtkSet[] = {
    fms_ready, fms_fire, fms_reload, fms_fire_end,
    m92_ready_crouch, m92_fire_crouch, m92_reload_crouch, fms_fire_end
} ;
#endif

/* £Í£´ */
static	short	M4AtkSet[] = {
    m4a_fire_p, m4a_fire, m4a_fire_reload, m4a_fire_end,
    m4a_crouch_fire_p, m4a_crouch_fire, m4a_crouch_fire_reload, m4a_crouch_fire_end
} ;

/* £Á£Ë£Ó */
static	short	AksAtkSet[] = {
    m4a_fire_p, m4a_fire, ak_reload, m4a_fire_end,
    m4a_crouch_fire_p, m4a_crouch_fire, ak_reload_cr, m4a_crouch_fire_end
} ;

/* £Ç£ò£å£î£á£ä£å */
static	short	GrnAtkSet[] = {
	gm_fire_start, gm_fire_low_min, gm_fire_low_max, gm_fire_low_5pose,
	gm_fire_start_g, gm_fire_min_g, gm_fire_max_g, gm_fire_5pose_g,
	gm_fire_start_bhl, gm_fire_min_bhl, gm_fire_max_bhl, gm_fire_5pose_bhl,
	gm_fire_start_bhr, gm_fire_min_bhr,	gm_fire_max_bhr, gm_fire_5pose_bhr
} ;

/* £Ã£´ */
static	short	C4AtkSet[] = {
	NO_ACT, NO_ACT
} ;

/* £Ò£Ç£Â */
static	short	RGBAtkSet[] = {
    rgb_fire_p, rgb_fire, rgb_reload, NO_ACT,
    rgb_fire_p_cr, rgb_fire_cr, rgb_reload_cr, NO_ACT,
} ;

#define	NikitaAtkSet	USPAtkSet
#define	ChaffAtkSet		GrnAtkSet
#define	StunAtkSet		GrnAtkSet
#define	MagazineAtkSet	GrnAtkSet
#define	StingerAtkSet	USPAtkSet

static	short	*AttackSets[] = {
    NULL, M92AtkSet, USPAtkSet, SocomAtkSet, NULL,
    RGBAtkSet, NikitaAtkSet, StingerAtkSet, NULL, C4AtkSet, 
	ChaffAtkSet, StunAtkSet, NULL, NULL, NULL, 
    AksAtkSet, MagazineAtkSet, GrnAtkSet, M4AtkSet, NULL,
	NULL, NULL
} ;

/*--------¶¦ÄÌ--------*/

static	short	SharedSet[] = {
	none_tumble, NO_ACT, non_tumble_end_cr,
    non_tumble_end_st, NO_ACT, non_tumble_miss,
    none_damage_f, none_damage_b, non_damage_f_sps, non_damage_b_sps,
	non_dam_trip_f, non_dam_trip_b,
    none_damage_out, none_damage_near, non_blow_soft, none_down, none_down_stand, 
    none_down_f, none_down_stand_f, none_damage_crouch, 
    none_damage_down, none_damage_down_f, none_damage_down_out, none_damage_down_out_f, 
	non_damage_caution, non_damage_caution_sq, non_out_caution, non_out_caution_sq, 
	non_dam_out_intrude,
	non_push_walk_l_l, non_push_walk_l_r, non_pushed_lf, non_pushed_lb,
	none_hang, none_tie, none_kill, none_hang_walk, 
	non_hang_escape, non_hang_release, non_hang_miss,
    none_throw_r, none_throw_l, none_combo, none_carry_b_start_f, none_carry_b_start_b, 
    none_carry_b, none_carry_b_walk, none_carry_b_end, none_carry_l_start, none_carry_l, 
    none_carry_l_walk, none_carry_l_end, none_carry_miss,
    box_idle, NO_ACT, box_run, box_walk, box_stop, box_walk_on,
	non_idle_snz, NO_ACT, non_crouch_snz, non_caution_snz
} ;

#endif

/*-------------------------------------------------------------------*/


