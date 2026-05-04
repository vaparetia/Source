/*
   rai_arm.h 
   ライデン／主観腕に関する定義

   1999/07/29 M.Sonoyama
   $Id: rai_arm.h,v 1.1.1.3 2002/11/19 11:51:00 Yoshizawa1 Exp $
*/

//#ifndef DESIGN_PREVIEW
//#define	ARM_RIGHT_HAND_UNIT	(4)
//#define	ARM_LEFT_HAND_UNIT	(7)
//#else
#define	ARM_RIGHT_HAND_UNIT	(HUMAN21_MIGI_TE)
#define	ARM_LEFT_HAND_UNIT	(HUMAN21_HIDARI_TE)
//#endif

/* 本体との通信用フラグ */
enum {
    ARM_NORMAL 	  = 0x0000,
    ARM_INVISIBLE = 0x0001,
    ARM_MOTION_IS_END = 0x0002,
	ARM_IS_COMBO =		0x0010,
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
    AM_MAX_CHANGE_MOTIONS
} ;

/* 共通モーション */
enum {
    AMcombo_p = AM_MAX_CHANGE_MOTIONS,
	AMcombo,
    AM_MAX_MOTIONS
} ;

#define	AM_NOACT	(AM_MAX_MOTIONS)

/* 手先モデルリスト */
enum {
    ARM_RIGHT_DEFAULT = 0,
    ARM_RIGHT_MAX
} ;

enum {
    ARM_LEFT_DEFAULT = 0,
    ARM_LEFT_MAX
} ;

extern	FVECTOR		ArmCamShift ;
extern	float		ArmCamRotX ;

extern	int			PL_GetUSPLightOn( void ) ;
