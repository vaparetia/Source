//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   psg.c
   ＰＳＧ－１
   
   2000/02/01 M.Sonoyama
   $Id: psg.c,v 1.1.1.3 2002/11/19 11:50:29 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"


/*------------------------------------------------------------------*/

/* 外部関数・変数宣言 */
extern	void	*NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;

/* ぼかし制御 */
extern	void	*NewFarFocusControl( GM_CameraSet * ) ;
/* ズームカメラ (sonoyama/etc/zoomcam.c) */
extern	void		*NewCameraZoomControl( GM_CameraSet **res, GM_CameraSet *parent, GV_PAD *pad, 
										  float angle_min, float angle_max,
										  float angle_step, int mode, int name ) ;
extern  void	*NewPsg1Sight( int );

//extern  void    *NewLattice( int , int ); // サイトエフェクト用
extern void SK_OutAngleControl( float angle );
extern void SK_OutAngleControlInitialize( void );

/*------------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE)
#define	BODY_DATA	MDL_PSG
#define	BULLET_TYPE	(BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_HIT_CHILD|BUL_TYPE_NOISE_SS)
#define	LENGTH		(200000)
#define	MAGAZINE	(5)
#define	RELOAD_TIME	(90)
#define	PSG1_ANGLE	(48.0F)

/*------------------------------------------------------------------*/

//static	FVECTOR	ShootPoint = { 17.5, -954, 89.7 } ;
//static	FVECTOR	ShootPoint = { 17.5F, 600.0F, 89.7F } ;
static	FVECTOR	ShootPoint = { 0.0F, 600.0F, 0.0F } ;

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT_EX		actor ;
    OBJECT			weapon ;
    OBJECT			weapon_sub ;
    CONTROL			*ctrl ;
    OBJECT			**body ;
    int				*unit ;
    u_int			*trigger ;
    int				side ;
    int				reload_time ;
    GM_CameraSet	*camera ;
	GM_CameraSet	*zoom ;
    float			default_angle ;
    int				default_step ;

	int				weaponNo ;
	int				weaponType ;
	int				chanl ;
	int				vflag ;
	float           angle;
	float           zoom_back;
	char            flag;
	char            local_weapon_fire;

	void			*focus ;
} Work ;

enum {
	SK_SET = 0x1 , 
	SK_GO = 0x2 ,
	SK_CHANCEL = 0x4 ,
};

/*------------------------------------------------------------------*/
static void Psg1ZoomControl( Work *pWork )
{
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( 0 );	// カメラの取得
	if ( ( pWork->local_weapon_fire & 0x1 ) && pWork->flag & SK_SET ){
		pWork->zoom_back = pCam->angle;
		pWork->angle = pCam->angle;
		pWork->flag = SK_GO;
		SK_OutAngleControlInitialize(); // 一度初期化する
	}
	if ( pWork->flag & SK_GO ){
		if ( pWork->local_weapon_fire & 0x1 ){
			pCam->angle = pWork->zoom_back;
			pWork->angle = pCam->angle = pWork->zoom_back - ( pCam->angle / 2.0f );
			pWork->local_weapon_fire = 0x0;
		} else if ( pWork->angle < pWork->zoom_back ){
			pWork->angle += ( pWork->angle / 4.0f );
			SK_OutAngleControl( pWork->angle ); // 間接操作
			if ( pWork->angle > pWork->zoom_back ){
				pCam->angle = pWork->zoom_back;
				pWork->angle = pWork->zoom_back;
				SK_OutAngleControl( pWork->zoom_back );
				SK_OutAngleControlInitialize(); // 一度初期化する
			}
		} else {
			pCam->angle = pWork->zoom_back;
			pWork->angle = pWork->zoom_back;
			pWork->flag = SK_SET;
		}
	}
}

#if 1 //BP - enabling corrected 3D sound calculation globally.
      //On PS2, rifle sounds came from the player's feet.
//#ifdef KP_XBOX
static void SetSe( int se, int mode ) {
	// XBOX ではカメラの少し前で音を出す
	// (カメラの後ろで出していると音質に問題が出る)

	GM_CameraSet	*cam ;
	FVECTOR	vTmp;
	float	fScale;
	cam = GM_GetCurrentCamera( 0 ) ;
	_sceVu0SubVector( &vTmp, &cam->target, &cam->position );
	fScale = 500.f / GV_VecLen3F( &vTmp );	// 50cm
	
	_sceVu0ScaleVector( &vTmp, &vTmp, fScale );
	_sceVu0AddVector( &vTmp, &vTmp, &cam->position );
	GM_SeSetMode( se, &vTmp, mode ) ;
}

#define GM_SeSetMode( se, pos, mode ) SetSe( se, mode )
#endif

static	void	Act( Work * work )
{
	int		chanl, vflag ;
    u_int	trg ;
    FVECTOR	from ;
    FMATRIX	world, *root ;

	chanl = work->chanl ;
	vflag = work->vflag ;
    GM_SetCurrentMap( work->ctrl->map ) ;
    GM_GroupObjs( work->weapon.objs, work->ctrl->map ) ;

    if ( GM_PlayerBody->objs->flag & vflag ) {
		void	*child ;

		DG_InvisibleObjsChanl( work->weapon.objs, chanl ) ;
		if ( work->zoom == NULL ) {
			child = NewCameraZoomControl( &work->camera, GM_PlayerSubjectCamera[ 0 ], GV_PadData,
										  8.0F, 60.0F, 0.15F, 0, PL_PSG1_ZOOMCAMERA_NAME ) ;
			if ( child != NULL ) {
				GV_SetActorChild( work, child ) ;
				work->zoom = child ;
			}
			/* 仮サイト */
			//GV_SetActorChild( work, NewLattice( 0, GV_StrCode( "psg" ) ) ) ;
			GM_ResetSightStatus( SGT_Psg1 ) ;
			GV_SetActorChild( work, NewPsg1Sight( 0 ) ) ;
		}
		if ( work->focus == NULL ) {
			child = NewFarFocusControl( work->camera ) ;
			if ( child != NULL ) {
				GV_SetActorChild( work, child ) ;
				work->focus = child ;
			}
		}

    } else {
		DG_VisibleObjsChanl( work->weapon.objs, chanl ) ;
		return ;
    }

	if ( work->zoom == NULL ) {	
		printf( "warning : zoom camera failed!!\n" ) ;
		return ;
	}

    root = work->weapon.objs->root ;
    DG_SetPos( root ) ;
    _sceVu0CopyMatrix( &world, root ) ;

	Psg1ZoomControl( work );
    if ( work->reload_time > 0 ) {
		/* リロード中 */
		if ( -- work->reload_time == 0 ) {
			PL_SetMagazine( work->weaponNo, 0 ) ;
			if ( GM_Magazine > 0 ) {
				GM_SeSetMode( SD_W_EQUIP03, &work->ctrl->mov, GM_SEMODE_BOMB ) ;
			}
		}
		return ;
    }

    trg = *( work->trigger ) ;
    switch ( trg ) {
    case TRIG_FIRE :
		if ( GM_WeaponNum( work->weaponNo ) == 0 ) break ;
		{
			SVECTOR		rot ;

			rot = work->camera->rotate ; rot.vx -= 1024 ;
			DG_SetPos2( &work->camera->position, &rot ) ;
			DG_GetPos( &world ) ;
			ShootPoint.vx = 0.0F ;
			ShootPoint.vy = 0.0F ;
			ShootPoint.vz = 0.0F ;
		}
		DG_PutVector( &ShootPoint, &from, 1 ) ;
		world.m[ 3 ][ 0 ] = from.vx ;
		world.m[ 3 ][ 1 ] = from.vy ;
		world.m[ 3 ][ 2 ] = from.vz ;
		NewPadVibration2( GV_StrCode( "rai_psg_fire" ), 0 ) ;
		NewBullet( &world, BULLET_TYPE, ENEMY_SIDE, 25, 5, 
				  LENGTH, 5500, work->weaponType ) ;
		if ( work->weaponNo == WP_Psg1 ) {
			GM_SeSetMode( SD_W_RIFLE02 , &work->ctrl->mov, GM_SEMODE_BOMB ) ; 
			GM_SetNoise( NOISE_M, &work->ctrl->mov, work->ctrl->map ) ;
		} else {
			GM_SeSetMode( SD_W_RIFSIR01 , &work->ctrl->mov, GM_SEMODE_BOMB ) ; 	    
			GM_SetNoise( NOISE_SS, &work->ctrl->mov, work->ctrl->map ) ;
		}
		/* 発砲数 */
		if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;
		GM_DecrementWeapon( work->weaponNo, 1 ) ;
		if ( !PL_DecrementMagazine() ){
			work->reload_time = RELOAD_TIME ;
		}
		work->local_weapon_fire = 0x1;
		break ;
    default :
	  ;
    }
}

static	void	Die( work )
Work		*work ;
{
    if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
    }
    GM_FreeObject( &( work->weapon ) ) ;
    GM_FreeObject( &( work->weapon_sub ) ) ;
//    work->camera->angle = work->default_angle ;
//    GM_SubjectVStep = GM_SubjectHStep = work->default_step ;
	
	GM_PlayerWeaponBody = GM_PlayerSubWeaponBody = NULL ;
	SK_OutAngleControlInitialize(); // 初期化する 
	GV_DestroyOtherActorQuick( work->zoom ) ;
}

/*------------------------------------------------------------------*/

static	int	GetResources( work, body, unit )
Work		*work ;
OBJECT		**body ;
int		*unit ;
{
    OBJECT	*weapon ;

    weapon = &( work->weapon ) ;
    GM_InitObject( weapon, BODY_DATA, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
    GM_ConfigObjectRoot( weapon, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
    DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;

	/* 主観用はダミー */
    weapon = &( work->weapon_sub ) ;
    GM_InitObject( weapon, BODY_DATA, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;

    GM_ConfigObjectRoot( weapon, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
	DG_InvisibleObjs( weapon->objs ) ;

	GM_PlayerWeaponModel = BODY_DATA ;
	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon_sub ;

	{
		//void	*other ;
		/*
		other = NewLattice( 0 , 7967233 ); // サイト
		if ( other != NULL ) {
			GV_SetActorChild( work, other ) ;
		}
		other = NewLattice( 0 , 8113294 ); // サイト
		if ( other != NULL ) {
			GV_SetActorChild( work, other ) ;
		}
		*/
		
	}
	work->flag = SK_SET; // by koba4
	SK_OutAngleControlInitialize();

	GM_SetSightStatus( SGT_Psg1 ) ;

    return 0 ;
}

/*------------------------------------------------------------------*/

static	void	*New( CONTROL *ctrl, OBJECT **body, int *unit, 
					  u_int *trigger, int side, int weaponNo, int weaponType,
					  int 	magazine )
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
		work->weaponNo = weaponNo ;
		work->weaponType = weaponType ;
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->side = side ;
		PL_SetMagazine( work->weaponNo, magazine ) ;
		work->chanl = GM_CurrentCameraChanl ;
		work->vflag = ( DG_FLAG_INVISIBLE0 << work->chanl ) ;
    }
    return work ;
}

/* ＰＳＧ－１ */
void		*NewPsg1( CONTROL *ctrl, OBJECT **body, int *unit, 
					  u_int *trigger, int side )
{
	return New( ctrl, body, unit, trigger, side, WP_Psg1, WP_Psg1, 20 ) ;
}

/* ＰＳＧ－１Ｔ（麻酔） */
void		*NewPsg1T( CONTROL *ctrl, OBJECT **body, int *unit, 
					  u_int *trigger, int side )
{
	return New( ctrl, body, unit, trigger, side, WP_Psg1T, WP_m92, 5 ) ;
} 
