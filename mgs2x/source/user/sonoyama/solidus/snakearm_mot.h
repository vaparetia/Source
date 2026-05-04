/*
   snakearm_mot.h
   ソリダス／スネークアームモーション

   2001/03/28	M.Sonoyama
   $Id: snakearm_mot.h,v 1.1.1.3 2002/11/19 11:51:06 Yoshizawa1 Exp $
*/

#ifndef __snakearm_mot_h__
#define	__snakearm_mot_h__

enum {
	AMkwt_idle		=	0,				/* 刀静止 */
	AMkwt_walk,							/* 刀歩き */
	AMkwt_run,							/* 刀走り */
	AMwalk_b,
	
	AMkwt_fire_ready,
	AMkwt_fire_idle,	
	AMkwt_fire_end,
	AMslash_r,
	AMslash_l,
	AMslash_w,
	
	AMkwt_dam_f,

	AMdam_f_m,
	AMdam_f_l,
	AMdam_f_r,
	AMdam_b_m,
	AMdam_b_l,
	AMdam_b_r,

	AMblow_f_m,
	AMblow_f_l,
	AMblow_f_r,
	AMblow_b_m,
	AMblow_b_l,
	AMblow_b_r,

	AMrise_f,
	AMrise_b,

	AMdam_filliped_r,
	AMdam_filliped_l,

	AMguard_u,
	AMguard_d,
	AMguard_r,
	AMguard_l,
	AMguard_w,

	AMdash_ready,
	AMdash_f,
	AMdash_b,
	AMdash_l,
	AMdash_r,
	AMdash_end_f,
	AMdash_end_b,
	AMdash_end_l,
	AMdash_end_r,

	AMthrust,
	AMspinkick,
	AMelbow,

	AMseize_start,
	AMseize_liftup,
	AMseize_liftidle,
	AMseize_fling,
	AMseize_miss,
	AMseize_undo,

	AMclaw_ready,
	AMclaw_fire,
	AMclaw_end,

	AMtrip_ready,
	AMtrip_r,
	AMtrip_l,

	AMmissle_ready,
	AMmissle_fire,
	AMmissle_fire_idle,
	AMmissle_end,
	
	AMtriangle,
	
	AMarm_undo_start,
	AMarm_undo_end,

	AMslash_combo,
	AMstamp,
	MAX_ARM_MOTIONS
} ;

#endif
