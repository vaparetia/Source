//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   stg.c
   スティンガー 
   
   2000/03/01 M.Sonoyama
   $Id: stg.c,v 1.1.1.3 2002/11/19 11:50:30 Yoshizawa1 Exp $
   */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include <libutl.h>
#include	"utl_dma.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"camera.h"
#include    "../test/etc.h"

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

/*------------------------------------------------------------------*/

/* 外部関数・変数宣言 */
extern	void	*NewBulletStinger( FMATRIX *, CONTROL *, int, int ) ;
extern	void	*NewLockonSight( void ) ;
extern  void    *NewStgSight( int );
extern  void    *NewSthingerUVSight( void );

extern int PL_PAD_WEAPON  ; /* sonoyama/raiden/pl_pad.c */
extern int PL_PAD_SUBJECT ; /* sonoyama/raiden/pl_pad.c */

/*------------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_ONEPIECE)
//#define	BODY_DATA	(121575)	/* stg */
#define	BODY_DATA		MDL_STINGER_SUB

/*------------------------------------------------------------------*/

static	FVECTOR ShootPoint = { 17.5F, -641.0F, 112.7F } ;
//static	FVECTOR	ShootPoint = { 0.0F, 600.0F, 0.0F } ;

/*------------------------------------------------------------------*/

typedef	struct {
	GV_ACT_EX	actor ;
	OBJECT		weapon ;
	OBJECT		weapon_sub ;

	CONTROL		*ctrl ;
	OBJECT		**body ;
	int			*unit ;
	u_int		*trigger ;

	int			side ;
	int			chanl ;
	int			vflag ;
	GM_CameraSet	*camera ;

	DG_PRIM2	*prim ;
	float		alpha;
	int			count;
	float		fcount;

	FVECTOR slide_point;
	char       togle;
	void		*sight ;
	void		*lockon_sight ;
} Work ;


#define FLOAT_ADD	(0.05f)
#define TIME_SCALE	(40)

#define	COL_R				(255)
#define	COL_G				(16)
#define	COL_B				(16)
#define	MAX_ALPHA			(255.0f)
#define	DECAY_RATIO			(0.90f)


#define	RAISE					(0)
#define	SIZE		(3)
#if 0
#define	SHIFT_X		( 144.0f)
#define	SHIFT_Y		(-160.0f)
#define	SHIFT_Z		( 193.0f)
#endif
#define	SHIFT_X		( 144.0f)
#define	SHIFT_Y		(-160.0f)
#define	SHIFT_Z		( 201.0f)

//#define	LEN			(4.0f)
#define	LEN			(8.0f)

#define	N_PRIMS			(1)
#define	N_VERTS			(1)
#define	N_VP			(N_PRIMS * N_VERTS)
//static FVECTOR s_pos[N_VP];

#define SLIDE_VY ( 1000.0f )
#define SLIDE_FRAME ( DIRECT_TICK( 5.0f ) )

enum {
	SK_STHINGER_MODEL_OFF = 0x4, // モデルなし通常モードにいこう 
	SK_STHINGER_MODEL_ON = 0x8, // モデル有直ねらいモードにいこう 
};

/*------------------------------------------------------------------*/
static void ModelSlide( Work *pWork )
{
	FVECTOR slide;
	FVECTOR fvtmp;
	FVECTOR tmp;

	slide = DG_ZeroVector;
	slide.vy = SLIDE_VY;

	// 加速度の算出（慣性） 
	if ( pWork->body == NULL ){
		return;
	}
	DG_SetPos( &( *pWork->body )->objs->objs[ HUMAN21_MIGI_TE ].world );
	DG_RotVector( &pWork->slide_point , &fvtmp , 1 );
	// 打てるか打てないか 
	if ( fvtmp.vx + fvtmp.vy + fvtmp.vz <= 1.0f ){
		GM_WeaponAlive &= ~WP_CANNOT_FIRE_STINGER;
	} else {
		GM_WeaponAlive |= WP_CANNOT_FIRE_STINGER;
	}
	// 修正 
	_sceVu0DivVector( &tmp , &pWork->slide_point , SLIDE_FRAME );
	_sceVu0SubVector( &pWork->slide_point , &pWork->slide_point , &tmp );
	_sceVu0AddVector( ( FVECTOR * )pWork->weapon_sub.objs->world.m[ 3 ] ,
					  ( FVECTOR * )( *pWork->body )->objs->objs[ HUMAN21_MIGI_TE ].world.m[ 3 ] , &fvtmp );
}

static	void	Act( Work *work )
{
	int		j;
	int		chanl, vflag ;
	u_int	trg ;
	FVECTOR plus_trans;
	FMATRIX	world, *root ;
	int	itemp;
	int	clock;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	chanl = work->chanl ;
	vflag = work->vflag ;
	GM_SetCurrentMap( work->ctrl->map ) ;
	GM_GroupObjs( work->weapon.objs, work->ctrl->map ) ;
	GM_GroupObjs( work->weapon_sub.objs, work->ctrl->map ) ;

	if ( GM_PlayerBody->objs->flag & vflag ) {
		DG_InvisibleObjsChanl( work->weapon.objs, chanl ) ;
		GM_ResetSightStatus( SGT_Stinger ) ;
		// sthingerモデル 
		DG_VisibleObjsChanl( work->weapon_sub.objs, chanl ) ;
		DG_InvisiblePrim2( work->prim ) ;
	} else {
		DG_InvisibleObjsChanl( work->weapon_sub.objs, chanl ) ;
		GM_SetSightStatus( SGT_Stinger ) ;
		DG_VisibleObjsChanl( work->weapon.objs, chanl ) ;	
	}
	{
		FVECTOR fvtmp;
		FVECTOR default_pos = { 0.0f , 1000.0f , 0.0f , 0.0f };

		// 初期位置の設定 腕から離す（主観モデルのみ） 
		work->weapon_sub.objs->world = (*work->body)->objs->objs[ HUMAN21_MIGI_TE ].world;
		DG_SetPos( &work->weapon_sub.objs->world );
		DG_RotVector( &default_pos , &fvtmp , 1 );
		_sceVu0AddVector( ( FVECTOR * )work->weapon_sub.objs->world.m[ 3 ] , ( FVECTOR * )work->weapon_sub.objs->world.m[ 3 ] , &fvtmp );
	}
 	if ( GV_PadData[ 0 ].status & PL_PAD_SUBJECT ){ // 押しつづけている間のみ実行 
		if ( work->togle == SK_STHINGER_MODEL_OFF ){ // 恐喝 
			GM_SetNoise( NOISE_HOLD , ( FVECTOR * )work->weapon_sub.objs->world.m[ 3 ] , work->ctrl->map );
			work->prim->root = &work->weapon_sub.objs->world;
		}
		// model move 
		ModelSlide( work );
		// etc
		work->togle = SK_STHINGER_MODEL_ON;
		GV_CallChildSignalFunc( work , work->togle , 0 );
		DG_VisiblePrim2( work->prim ) ;
	}
	if ( work->togle == SK_STHINGER_MODEL_ON && !( GV_PadData[ 0 ].status & PL_PAD_SUBJECT ) ){
		// initialize
		work->slide_point = DG_ZeroVector;
		work->slide_point.vy = SLIDE_VY;
		// etc
		work->togle = SK_STHINGER_MODEL_OFF;
		GV_CallChildSignalFunc( work , work->togle , 0 );
		work->prim->root = &work->weapon.objs->world;
		GM_WeaponAlive &= ~WP_CANNOT_FIRE_STINGER;
		DG_InvisiblePrim2( work->prim ) ;
	}
	if ( work->togle == SK_STHINGER_MODEL_OFF ){ // 弾の出現する位置の修正 
	    //root = &(*work->body)->objs->objs[ HUMAN21_MIGI_TE ].world;
		root = &work->weapon_sub.objs->world;//&( GM_PlayerBody->objs->objs[ HUMAN21_MIGI_TE ].world ) ;
		plus_trans = DG_ZeroVector;
		plus_trans.vy = -SLIDE_VY;
	} else {
		root = &work->weapon_sub.objs->world ;
		plus_trans = DG_ZeroVector;
	}
	_sceVu0CopyMatrix( &world, root ) ;

	trg = *( work->trigger ) ;

	switch ( trg ) {
	case TRIG_FIRE :
		if ( GM_WeaponAlive & ( WP_ALIVE_STINGER | WP_CANNOT_FIRE_STINGER ) ) break ;
		if ( GM_WeaponNum( WP_Stinger ) == 0 ) {
			/* から撃ち */
			GM_SeSetMode( SD_W_EMPTY01, &work->ctrl->mov, GM_SEMODE_BOMB ) ;
			break ;
		}
		DG_SetPos( root ) ;
		DG_MovePos( &ShootPoint );
		if ( work->togle == SK_STHINGER_MODEL_OFF ){ // さらに動かす 
			DG_MovePos( &plus_trans );
		}
		DG_GetPos( &world ) ;

		if( NewBulletStinger( &world, work->ctrl, BOTH_SIDE, work->chanl )
				 != NULL ){
			// 発射部に移動 
			//		GM_SeSetMode( SD_W_MISSIL01, &work->ctrl->mov, GM_SEMODE_BOMB ) ; 
			//		GM_SetNoise( NOISE_M, &work->ctrl->mov, work->ctrl->map ) ;
		}
		break ;
	default :
	  ;

	}
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;


	uvrgbwh = work->prim->uvrgb[clock];
	work->count--;
	if( work->count < 0 ){
		work->fcount += FLOAT_ADD;
		if( work->fcount >= TPI ) work->fcount -= TPI;
		work->count = (int)( sinf( work->fcount ) * TIME_SCALE );
		work->alpha = MAX_ALPHA;

//		GM_SeSetMode( SD_W_SIGNAL01, &work->ctrl->mov, GM_SEMODE_BOMB ) ;
	}else{
		work->alpha *= DECAY_RATIO;
	}

	itemp = (int)work->alpha;
	for( j=0; j<N_VP; j++ ){
		uvrgbwh->a = itemp;
		uvrgbwh++;
	}


}

static	void	Die( work )
Work		*work ;
{
	DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;

	GM_FreeObject( &( work->weapon ) ) ;
	GM_FreeObject( &( work->weapon_sub ) ) ;

	GM_PlayerWeaponBody = NULL ;
	GM_PlayerSubWeaponBody = NULL ;

	GM_WeaponAlive &= ~WP_CANNOT_FIRE_STINGER;
	if( work->prim != NULL )  GM_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int n_prims, int n_verts )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise=RAISE;

	//-------------------------------
	for ( i=0; i<2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		for ( j=0; j<n_prims; j++ ){
			for ( k=0; k<n_verts; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h = uvrgbwh->w = SIZE ;
				uvrgbwh->r = COL_R;
				uvrgbwh->g = COL_G;
				uvrgbwh->b = COL_B;
				uvrgbwh->a = 0 ;
				uvrgbwh ++ ;
			}
		}
	}
}

/*------------------------------------------------------------------*/
#define STR_LIGHT10_MSK (12296685)
static	int	GetResources( Work *pWork, OBJECT **body, int *unit )
{
	OBJECT	*weapon , *weapon_sub ;
	FVECTOR	    *fpv;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	//FVECTOR     s_pos[ 2 ];
	int	i;

	pWork->count  = 0;
	pWork->fcount = 0.0f;

	weapon = &( pWork->weapon ) ;
	GM_InitObject( weapon, BODY_DATA , BODY_FLAG ) ;
	if ( weapon->objs == NULL ){
		return ( -1 );
	}
	// root の設定 
	GM_ConfigObjectRoot( weapon, GM_PlayerBody, HUMAN21_MIGI_TE ) ;

	weapon_sub = &( pWork->weapon_sub ) ;
	GM_InitObject( weapon_sub, BODY_DATA , BODY_FLAG ) ;
	if ( weapon_sub->objs == NULL ){
		return ( -1 );
	}
	// root の設定 
	pWork->weapon_sub.objs->root = NULL;

	// default pos
	pWork->weapon.objs->world = GM_PlayerBody->objs->world;
//	work->weapon_sub.objs->world = GM_PlayerBody->objs->world;
	pWork->camera = GM_PlayerSubjectCamera[ GM_CurrentCameraChanl ] ;
	DG_InvisibleObjsChanl( weapon->objs, pWork->chanl ) ;
	DG_InvisibleObjsChanl( weapon_sub->objs, pWork->chanl ) ;
//	DG_InvisibleObjs( weapon->objs ) ;
//	DG_InvisibleObjs( weapon_sub->objs ) ;

	DG_ConnectObjs( GM_PlayerBody->objs, pWork->weapon.objs ) ;

	GM_PlayerWeaponModel = BODY_DATA ;
	GM_PlayerWeaponBody = &pWork->weapon ;
	GM_PlayerSubWeaponBody = &pWork->weapon_sub ;

	// その他の初期設定 
	pWork->slide_point = DG_ZeroVector;
	pWork->slide_point.vy = SLIDE_VY;
	pWork->togle = SK_STHINGER_MODEL_OFF;
#if 0
	weapon = &( work->weapon ) ;
	weapon_sub = &( work->weapon_sub ) ;
	GM_InitObject( weapon, BODY_DATA, BODY_FLAG ) ;
	GM_ConfigObjectRoot( weapon, *body, *unit ) ;
	work->weapon.objs->world = GM_PlayerBody->objs->world;
//	DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
	work->camera = GM_PlayerSubjectCamera[ GM_CurrentCameraChanl ] ;
	DG_InvisibleObjs( weapon->objs ) ;
	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon ;
#endif
	//-----------------------------
	tex = DG_GetTexture( STR_LIGHT10_MSK );
	if ( tex == NULL ){
		SK_Err("Non Tex light10_msk\0");
		return ( -1 );
	}
	prim = pWork->prim = GM_MakePrim2( DG_PRIM2_SPRT | DG_PRIM2_TEX | DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if( prim == NULL ){
		printf("null prim\n");
		return -1;
	}
	InitPacket( pWork, prim, tex, N_PRIMS, N_VERTS );

	pWork->prim->root = &pWork->weapon.objs->world;
	DG_SetPos( pWork->prim->root );

#if 0
	for ( i = 0 ; i < N_VP ; i++ ){
		s_pos[i].vx = SHIFT_X;
		s_pos[i].vy = SHIFT_Y;
		s_pos[i].vz = SHIFT_Z + (float)(i) * LEN / (float)(N_VP-1);
	}
	DG_RotVector( s_pos, pWork->prim->pos[0], N_VP );
	DG_RotVector( s_pos, pWork->prim->pos[1], N_VP );
#else
	fpv = pWork->prim->pos[0];
	for ( i = 0 ; i < N_VP ; i++ ){
		fpv->vx = SHIFT_X;
		fpv->vy = SHIFT_Y;
		fpv->vz = SHIFT_Z;
	}
	fpv = pWork->prim->pos[1];
	for ( i = 0 ; i < N_VP ; i++ ){
		fpv->vx = SHIFT_X;
		fpv->vy = SHIFT_Y;
		fpv->vz = SHIFT_Z;
	}
#endif
	GM_SetSightStatus( SGT_Stinger ) ;

	GV_SetActorChild( pWork , NewStgSight( 0xffffffff ) );

	return 0 ;
}

/*------------------------------------------------------------------*/

/* スティンガー */
void	*NewStinger( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
	Work	*work ;


	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, unit ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->side = side ;
		work->chanl = GM_CurrentCameraChanl ;
		work->vflag = ( DG_FLAG_INVISIBLE0 << work->chanl ) ;
	}
	return work ;
}

