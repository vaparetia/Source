/*
   pl_arm.h
   新・主観腕ヘッダ

   2000/09/14 M.Sonoyama
   $Id: pl_arm.h,v 1.1.1.3 2002/11/19 11:50:57 Yoshizawa1 Exp $
*/

/* 本体との通信用フラグ */
enum {
    ARM_NORMAL 	  = 		0x0000,
    ARM_INVISIBLE = 		0x0001,
    ARM_MOTION_IS_END = 	0x0002,
	ARM_MOTION_SET_OVER =	0x0004,
	ARM_IS_COMBO = 			0x0010,	
	ARM_IS_WALL =			0x0020,
	ARM_IS_RELOAD = 		0x0040,
	ARM_ACTOBJECT_EX =		0x0080,
	ARM_BLADE =				0x0100,
	ARM_NEAR_WALL =			0x0200,
	ARM_NEAR_WALL2 =		0x0400,
	ARM_BLADE_MODE =		0xf000,
} ;

#define	ARM_VISIBLE_WAIT_TIME	(4)

/* 武器毎に変更モーション */
enum {
    AMstand = 0,
    AMready,
    AMfire,
    AMreload,
    AMfire_end,
	AMwall,
    AMcrouch,
    AMcrouch_ready,
    AMcrouch_fire,
    AMcrouch_reload,
    AMcrouch_fire_end,
	AMcrouch_wall,
    AM_MAX_CHANGE_MOTIONS
} ;

/* グレネード特殊 */
enum {
	AMgm_stand = 0,
	AMgm_start,
	AMgm_min,
	AMgm_max,
	AMgm_5pose,
	AMgm_start_g,
	AMgm_min_g,
	AMgm_max_g,
	AMgm_5pose_g
} ;

/* ブレード主観カメラ */
enum {
	ARM_BLADE_SLASH = 	0,
	ARM_BLADE_CHANGE,
	ARM_BLADE_GUARD,
	ARM_BLADE_ROLL,
	ARM_BLADE_COMBO,
	ARM_BLADE_THRUST,
} ;

#define	AM_CROUCH_SHIFT	(AMcrouch - AMstand)

/* 共通モーション */
enum {
    AMcombo = AM_MAX_CHANGE_MOTIONS,
	AMhfb_combo,
	AMnkt_strike,
	AMnkd_combo,
	AMm4a_combo,
    AM_MAX_MOTIONS
} ;

extern	int		PL_GetUSPLightOn( void ) ;
