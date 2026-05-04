/*
	water.x
	水中モードのプロトタイプ宣言

	2001/04/23	M.Sonoyama
	$Id: water.x,v 1.1.1.3 2002/11/19 11:50:51 Yoshizawa1 Exp $
*/

static	void	AdjustObject( PlayerWork *work, FVECTOR *shift ) ;

static	void	TreadStill( PlayerWork *work, int time ) ;
static	void	TreadMove( PlayerWork *work, int time ) ;
static	void	Tread2Dive( PlayerWork *work, int time ) ;
static	void	Dive2Tread( PlayerWork *work, int time ) ;
static	void	SeabedStill( PlayerWork *work, int time ) ;
static	void	SeabedWalk( PlayerWork *work, int time ) ;
static	void	Seabed2Dive( PlayerWork *work, int time ) ;

static	int		DiveCommon( PlayerWork *work ) ;
static	void	DiveInit( PlayerWork *work ) ;

static	void	DiveStill( PlayerWork *work, int time ) ;
static	void	DiveSlowMove( PlayerWork *work, int time ) ;
static	void	DiveLittle( PlayerWork *work, int time ) ;
static	void	DiveMiddle( PlayerWork *work, int time ) ;
static	void	DiveMax( PlayerWork *work, int time ) ;

static	void	DiveBreakStand( PlayerWork *work, int time ) ;
static	void	DiveBreak( PlayerWork *work, int time ) ;

static	void	Dive2WallTouch( PlayerWork *work, int time ) ;
static	void	WallTouch( PlayerWork *work, int time ) ;
static	void	TouchTurnR( PlayerWork *work, int time ) ;
static	void	TouchTurnL( PlayerWork *work, int time ) ;
static	void	TouchTurnB( PlayerWork *work, int time ) ;

static	void	WaterDamageCallback( PlayerWork *work, TARGET *off, TARGET *def ) ;
static	void	WaterCheckDamage( PlayerWork *work, long64 weapon_type, int dead ) ;

static	void	WaterDamageStand( PlayerWork *work, int time ) ;
static	void	WaterDamageLie( PlayerWork *work, int time ) ;
static	void	WaterDeadStand( PlayerWork *work, int time ) ;
static	void	WaterDeadLie( PlayerWork *work, int time ) ;
static	void	WaterDozaemon( PlayerWork *work, int time ) ;

static	void	IntoWater( PlayerWork *work, int time ) ;
static	void	LeaveWater( PlayerWork *work, int time ) ;

static	void	IntoWaterStair( PlayerWork *work, int time ) ;
static	void	LeaveWaterStair( PlayerWork *work, int time ) ;

static	void	EludeFallDeadWater( PlayerWork *work, int time ) ;

/* 溺れゲームオーバー関連 */
static	int		WaterDeadModeConditionFunc( PlayerWork *work ) ;
static	void	WaterDeadModeNoO2( PlayerWork *work, int time ) ;

/* 戻りモード */
static	void	ReturnSeabedStill( PlayerWork *work, int time ) ;
