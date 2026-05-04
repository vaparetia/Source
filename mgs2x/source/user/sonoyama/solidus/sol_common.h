/*
   sol_common.h
   ソリダス共通

   2001/04/06	M.Sonoyama
   $Id: sol_common.h,v 1.1.1.3 2002/11/19 11:51:06 Yoshizawa1 Exp $
*/

extern	int			SOL_SolStatus ;			/* solidus.c */
extern	int			SOL_GameLevel ;			/* solidus.c */
extern	CONTROL		*SOL_SolControl ;
extern	FVECTOR		SOL_SnakeArmHangPos ;	/* snakearm.c */

enum {
	SOL_CS_SNAKEARM_LIFTUP_SUCCESS =		0x00000001,
	SOL_CS_SNAKEARM_LIFTUP_END =			0x00000002,
	SOL_CS_SNAKEARM_LIFTUP_FLING =			0x00000004,
	SOL_CS_SNAKEARM_LIFTUP_FLING_GROUND = 	0x00000008,
	SOL_CS_SNAKEARM_LIFTUP_CANCEL =			0x00000010,
	SOL_CS_SNAKEARM_LIFTUP_CANCEL_ENABLE =	0x00000020,
	SOL_CS_SNAKEARM_LIFTUP_DAMAGED = 		0x00000040,
	SOL_CS_SNAKEARM_TRIP_SUCCESS =			0x00000080,
	SOL_CS_SNAKEARM_CLAW_SUCCESS =			0x00000100,
} ;

/* ライデンの刀振り種別 */
enum {
	ATK_DOWN_L = 0,
	ATK_UP_L,
	ATK_DOWN_R,
	ATK_UP_R,
	ATK_VERTICAL,
	ATK_RIGHT,
	ATK_LEFT,
	ATK_THRUST,
} ;

static	inline	void	SetSolStatus( int state )
{
	SOL_SolStatus |= state ;
}

static	inline	void	ResetSolStatus( int state )
{
	SOL_SolStatus &= ~state ;
}

static	inline	int		CheckSolStatus( int state )
{
	return ( SOL_SolStatus & state ) ;
}

extern	int		PL_SlashNow( void ) ;
extern	int		PL_SlashGuardNow( void ) ;
extern	void	PL_AttackGuardBlade( FVECTOR *mov ) ;
extern	int		PL_LiftupNow( void ) ;

extern	void	*NewSnakeArmMissile( FMATRIX *world, int map ) ;

extern	void *NewSolidusDunkSmoke(
								  FVECTOR *center,			// ライデンの座標
								  float          radius		// 煙の広がる半径
								  ) ;
/*
   ソリダスが蛇手でライデンを叩きつけたときの煙と破片です。
   ライデンが叩きつけられた瞬間に呼んでやってください。
   ライデンが多少浮いていても地面検索をしているので問題ないです。
*/

extern	void *NewSolidusSnakearmFlow(
									 DG_EVMOBJ *evmobj,		// 蛇手モデル
									 int		*flag			// フラグ
									 ) ;
/* 
   ソリダスの蛇手に纏うオーラエフェクトです。
   蛇手初期化時に子エフェクトとして呼んでやってください。
   常駐型で、ライデンを掴んだときにフラグを１に、
   和げ終わり、または振りほどかれたときに０にしてやってください。
*/

extern	void *NewSolidusSnakearmPlasma(
									   DG_EVMOBJ *evmobj		// 蛇手モデル
									   ) ;
/*
   蛇手で掴んだ時のプラズマエフェクトです。
   このエフェクトは蛇手オーラの中でフラグがたった時に自動的に呼ばれるので
   呼んでやる必要はないです。
   現段階でプラズマはストリップで組んでいるので
   後にスプライトで組み直して主観に耐えれるようにする予定です。
*/

extern	void *NewSolidusMissileFire(
									FVECTOR *pos,			// ミサイル位置
									FVECTOR *vec,			// ミサイル進行方向（Z軸）
									int		*flag
									) ;
/*
   ソリダスのミサイルの炎です。
   位置と進行方向のポインタを渡してやってください。
   ミサイルの子エフェクトとして呼んでください。
*/

extern	void *NewSolidusMissileSmoke(
									 FVECTOR *pos,
									 FVECTOR *vec,
									 float   size,
									 int     *flag
									 ) ;
/*
   ソリダスのミサイルの軌跡煙です。
   ミサイルが着弾しても煙は残るのでミサイル初期化時に普通にエフェクトを呼んでください。
   ミサイルが着弾、焼失した時にフラグを１にしてもらえれば
   あとはフェードアウトします。
   初期化時はフラグは０にしてください。
*/

extern	void *NewSolidusDashFire(
								 FVECTOR *pos,
								 int	 *flag,
								 int	 time
								 ) ;
/*
   ソリダスダッシュ時の炎です。
   地面に擦れている方の足の座標を渡してやってください。
   多少足が埋もれたり浮いていたりしても地面チェックをしているので問題ないです。
   ダッシュに入る瞬間に呼んでやってください。
*/

extern	void *NewBladeSparkEffect(
							FMATRIX *mat,
							int flag
							) ;
/*
   刀エフェクトです。
   刀を刀で受けたりした時に衝突点を中心とするマトリクスを呼んでやってください。
   マトリクスにしたのは今後飛び散る火花に指向性を持たせるときのためです。
*/

extern	void *NewSolidusDamageSpark( 
									OBJECT *body 
									) ;


