/*
   raiden.x 
   ライデン／プロトタイプ宣言

   1999/07/07 M.Sonoyama
   $Id: raiden.x,v 1.1.1.3 2002/11/19 11:51:02 Yoshizawa1 Exp $			
*/

/* normal.c */
static	void	StandStill( Work *, int ) ;
static	void	StandRun( Work *, int ) ;
static	void	RunToSquat( Work *, int ) ;
static	void	SquatStill( Work *, int ) ;
static	void	SquatToGround( Work *, int ) ;
static	void	GroundStill( Work *, int ) ;
static	void	GroundMoveFront( Work *, int ) ;
static	void	GroundMoveBack( Work *, int ) ;
static	void	GroundToStand( Work *, int ) ;
//static	void	StandCautionStill( Work *, int ) ; /* グローバル化 */
static	void	StandCautionLeft( Work *, int ) ;
static	void	StandCautionRight( Work *, int ) ;
static	void	SquatCautionStill( Work *, int ) ;
static	void	SquatCautionLeft( Work *, int ) ;
static	void	SquatCautionRight( Work *, int ) ;
static	void	GroundFromToIntrude( Work *, int ) ;
static	void	IntrudeStill( Work *, int ) ;
static	void	IntrudeMoveFront( Work *, int ) ;
static	void	IntrudeMoveBack( Work *, int ) ;
static	void	GroundToSquatFront( Work *, int ) ;
static	void	GroundToSquatBack( Work *, int ) ;

/* peep.c */
static	void	StandCautionPeepL( Work *, int ) ;
static	void	StandCautionPeepR( Work *, int ) ;
static	void	SquatCautionPeepL( Work *, int ) ;
static	void	SquatCautionPeepR( Work *, int ) ;
static	void	BehindAttackR( Work *, int ) ;
static	void	BehindAttackL( Work *, int ) ;
static	void	SquatBehindAttackR( Work *, int ) ;
static	void	SquatBehindAttackL( Work *, int ) ;

static	void	ThrowGrenadeBehindR( Work *work, int time ) ;
static	void	ThrowGrenadeBehindL( Work *work, int time ) ;

/* attack.c */
static	void	ShootBullet( Work *, int ) ;
static	void	PullBody( Work *, int ) ;
static	void	HangStart( Work *, int ) ;
static	void	HangStill( Work *, int ) ;
static	void	HangTie( Work *, int ) ;
static	void	HangRun( Work *, int ) ;
static	void	HangRelease( Work *, int ) ;
static	void	HangBreak( Work *, int ) ;
static	void	Throw( Work *, int ) ;
static	void	Combo( Work *, int ) ;
static	void	NikitaStrike( Work *, int ) ;
static	void	ShootNikita( Work *, int ) ;
static	void	SetC4BombCheck( Work *, int ) ;
static	void	SetC4BombEnemy( Work *, int ) ;
static	void	SetC4BombFloor( Work *, int ) ;
static	void	SetC4BombWall( Work *, int ) ;
static	void	SetClaymore( Work *, int ) ;
static	void	SetBook( Work *, int ) ;
static	void	ShootPsg1( Work *, int ) ;
static	void	ShootStinger( Work *, int ) ;
static	void	ThrowGrenade( Work *, int ) ;
static	void	JetSpray( Work *, int ) ;
static	void	SetMic( Work *work, int time ) ;

static	void	ReleasePsg1( Work *, int ) ;

/* equip.c */
static	void	CB_BoxStill( Work *, int ) ;
static	void	CB_BoxMove( Work *, int ) ;
static	void	CB_BoxStop( Work *, int ) ;
static	void	CB_BoxCanceled( Work *, int ) ;

static	void	SetScope( Work *, int ) ;

/* special.c */
static	void	PushOrPushed( Work *, int ) ;
static	void	Shrink( Work *, int ) ;
static	void	CautionShrink( Work *, int ) ;
static	void	DownDamage( Work *, int ) ;
static	void	Blow( Work *, int ) ;
static	void	Down( Work *, int ) ;
static	void	Rise( Work *, int ) ;
static	void	Dead( Work *, int ) ;
static	void	KnockWallLeft( Work *, int ) ;
static	void	KnockWallLeftAgain( Work *, int ) ;
static	void	KnockWallRight( Work *, int ) ;
static	void	KnockWallRightAgain( Work *, int ) ;

/* force.c */
static	void	ForceAct( Work *, int ) ;

static	void	EludeStart( Work *work, int time ) ;
static	void	EludeReturn( Work *work, int time ) ;
static	void	EludeStill( Work *work, int time ) ;
static	void	EludeMoveRight( Work *work, int time ) ;
static	void	EludeMoveLeft( Work *work, int time ) ;
static	void	EludeFall( Work *work, int time ) ;
static	void	EludeDamage( Work *work, int time ) ;
static	void	EludeTouchDownSafe( Work *work, int time ) ;
static	void	EludeTouchDownDamage( Work *work, int time ) ;
static	void	EludeTouchDownDead( Work *work, int time ) ;
static	void	EludeChinUp( Work *work, int time ) ;

//static	void	BeyondMode( Work *, int ) ;

/*-----------------------------------------------------*/

