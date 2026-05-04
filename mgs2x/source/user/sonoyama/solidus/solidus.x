/*
	solidus.x
	ソリダス／プロトタイプ宣言部
	
	2001/03/22	M.Sonoyama
	$Id: solidus.x,v 1.1.1.3 2002/11/19 11:51:07 Yoshizawa1 Exp $
*/

/* normal_sol.c */
static	void	StandStillK( Work *work, int time ) ;
static	void	StandWalkK( Work *work, int time ) ;
static	void	StandRunK( Work *work, int time ) ;
static	void	WalkBackK( Work *work, int time ) ;

static	void	FastAway( Work *work, int time ) ;
static	void	TriangleReady( Work *work, int time ) ;
static	void	TriangleJump( Work *work, int time ) ;

/* damage_sol.c */
static	void	SwayUpper( Work *work, int time ) ;
static	void	SwayLower( Work *work, int time ) ;

static	void	SlashFillipedR( Work *work, int time ) ;
static	void	SlashFillipedL( Work *work, int time ) ;
static	void	BladeGuard( Work *work, int time ) ;
static	void	BladeDamage( Work *work, int time ) ;
static	void	Blow( Work *work, int time ) ;
//static	void	Down( Work *work, int time ) ;
static	void	Rise( Work *work, int time ) ;
static	void	Dead( Work *work, int time ) ;
static	void	ArmUndoReady( Work *work, int time ) ;
static	void	ArmUndo( Work *work, int time ) ;

/* attack_sol.c */
static	void	SlashDownR( Work *work, int time ) ;
static	void	SlashDownL( Work *work, int time ) ;
static	void	SlashDouble( Work *work, int time ) ;
static	void	SlashThrust( Work *work, int time ) ;
static	void	Elbow( Work *work, int time ) ;
static	void	SpinKick( Work *work, int time ) ;
static	void	FastAttack( Work *work, int time ) ;

static	void	SlashCombo( Work *work, int time ) ;
static	void	Stamp( Work *work, int time ) ;

static	void	ShotArmMissile( Work *work, int time ) ;
static	void	LiftUpStart( Work *work, int time ) ;
static	void	LiftUpUp( Work *work, int time ) ;
static	void	LiftUpIdle( Work *work, int time ) ;
static	void	LiftUpFling( Work *work, int time ) ;
static	void	LiftUpEscaped( Work *work, int time ) ;

static	void	Trip( Work *work, int time ) ;
static	void	Claw( Work *work, int time ) ;
static	void	PursueWalk( Work *work, int time ) ;

static	void	SnakeArmAttackEnd( Work *work, int time ) ;

static	void	EludeAttackReady( Work *work, int time ) ;

/* careful_sol.c */
static	void	StandStillC( Work *work, int time ) ;
static	void	StandWalkC( Work *work, int time ) ;
static	void	WalkBackC( Work *work, int time ) ;
