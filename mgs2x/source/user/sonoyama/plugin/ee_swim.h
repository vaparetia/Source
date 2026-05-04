/*
   ee_swim.h
   水中エマとライデンの通信用定義
   
   2001/06/15	M.Sonoyama
   $Id: ee_swim.h,v 1.1.1.3 2002/11/19 11:50:47 Yoshizawa1 Exp $
*/

#ifndef __ee_swim_h__
#define	__ee_swim_h__

extern	int		PL_EE_Exist ;
extern	int		PL_EE_Mode ;
extern	int		PL_EE_Flag ;
extern	int		PL_EE_Motion ;
extern	int		PL_EE_TurnEnd ;
extern	int		PL_EE_Damage ;
extern	int		PL_EE_O2Damage ;
extern	CONTROL	*PL_WaterCamControl ;

extern	GM_CameraSet	*PL_WaterCamera ;

extern	int		DiveSpeed ;

enum {
	DIVE_SPEED_ZERO	=		0,
	DIVE_SPEED_SLOW,
	DIVE_SPEED_LITTLE,
	DIVE_SPEED_MIDDLE,
	DIVE_SPEED_MAX,
} ;

enum {
	EE_MODE_OFF =		0,
	EE_MODE_FREE,					/* ライデンとＥＥは離れている */
	EE_MODE_STAND,					/* 立ちくっつき */
	EE_MODE_LIE,					/* 横泳ぎくっつき */
	EE_MODE_S2L,					/* 立ちから泳ぎ */
	EE_MODE_L2S,					/* 泳ぎから立ち */
	EE_MODE_TURN,		
	EE_MODE_RAI_DAM_STAND,	
	EE_MODE_RAI_DAM_LIE,
	EE_MODE_RAI_DEAD_STAND,
	EE_MODE_RAI_DEAD_LIE,
	EE_MODE_RAI_DEAD_IDLE,
	EE_MODE_RAI_NOAIR_STAND,
	EE_MODE_RAI_NOAIR_LIE,
	EE_MODE_FORCEACT,
} ;

enum {
	PL_EE_FLAG_NOTHING		=			0x0000,
	PL_EE_FLAG_STAND2LIE_NO_INTERP =	0x0001,
} ;

enum {
	EMstill = 0,
	EMdive_still,
	EMrai_dam_stand,
	EMrai_dam_lie,
	EMout_idle,
	EMrai_out_stand,
	EMrai_out_lie,
	EMout_lie,
	EMout_stand,
	EMrai_out_idle,
	EMrai_noair_stand,
	EMrai_noair_lie,
	EE_SWIM_MAX_MOTIONS
} ;

static	inline	void	SetEEMode( int mode )
{
	PL_EE_Mode = mode ;
}

static	inline	void	SetEEMotion( int motion )
{
	PL_EE_Motion = motion ;
}	

static	inline	void	SetEEFlag( int flag )
{
	PL_EE_Flag |= flag ;
}

extern	int			PL_GuideToBreathPoint( FVECTOR *step, FVECTOR *mov, float guide_len, 
										   float step_len, FVECTOR *bppos ) ;

#endif
