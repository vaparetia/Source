//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   spray.c
   スプレー系
   
   2000/05/10 M.Sonoyama
   $Id: spray.c,v 1.1.1.3 2002/11/19 11:50:30 Yoshizawa1 Exp $
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

#include "BP_Misc.h"
#include "../sonoyama/raiden/pl_work.h"//BP_INPUT

/*------------------------------------------------------------------*/

/* 外部関数・変数宣言 */
extern	void	*NewBulletSpray( FMATRIX *world, int map, int side, int weapon ) ;
//extern   int     spray_num;

extern int PL_PAD_PRESS_WEAPON ; /* sonoyama/raiden/pl_pad.c */

/*------------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE)

#define	PRIM_FLAG2	(DG_PRIM2_SPRT | DG_PRIM2_TEX | DG_PRIM2_FOG | DG_PRIM2_ALPHA)
#define	TEXTURE2	TXT_LSIGHT_POINT
#define	N_PRIMS2	(1)
#define	N_VERTS2	(1)

#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_SEEK | TARGET_ONLINE | TARGET_ONLINE_MIN | \
						 TARGET_CHILD)

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))
#define HEADPHONES  (15757443) // dmp_headphones
#define MDL_MIKE     (3241187) // dmp_sub
#define MDL_ICE_SUB  (6386849)

// by koba4 
#define ICE_DUST    (15638496)
#define ICE_LOST    (6684577)

//#define SD_S_MICNOIZE 0x91
//#define SD_S_MICSWING 0x92    

enum {
	THIS_IS_COLD = 0,
	THIS_IS_MELT,
	THIS_IS_FIRE,
	THIS_IS_MIC,
} ;

enum {
	SK_SOUND_OFF = 0x0 ,
	SK_SOUND_ON  = 0x1 ,
	SK_SOUND_START  = 0x2 ,
};

enum {
	SK_EFFECT_NORMAL  = 0x1,
	SK_EFFECT_ADD     = 0x2,
	SK_EFFECT_KILL_OK = 0x4,
	SK_EFFECT_KILL_NO = 0x8,
	SK_EFFECT_DESTROY = 0x10,
	SK_EFFECT_HOKAN   = 0x20,
	SK_EFFECT_ADD_END  = 0x40,
};

enum {
	OPTION_OFF = 0x0 ,
	OPTION_ON  = 0x1
};

static	int	WpNum[] = {
	WP_ColdSpray,
	-1,		/* WP_MeltSpray */
	-1,		/* WP_FireExt */
	WP_Mic,
} ;

static	int	BodyDatas[] = {
	MDL_ICE_SUB,
	MDL_CHAFF_SUB,
	MDL_CHAFF_SUB,
	MDL_MIKE,
} ;

static	FVECTOR	NozzlePoints[] = {
	{ 13.0F, -323.0F, 61.0F },
	{ 13.0F, -323.0F, 61.0F },
	{ 13.0F, -323.0F, 61.0F },
	{ 13.0F, -323.0F, 61.0F }
} ;

enum {
	FLAG_NONE		=	0x0000,
	FLAG_START		=	0x0001,
} ;

/*------------------------------------------------------------------*/
typedef struct {
	SVECTOR  rot; // 前のフレームの向きの保存
	int    count; // フレームカウント
#ifdef KP_XBOX
	int  toggle ; // フリップする
#endif
} Sound;

typedef struct {
	int *num;
	int strcode;
	int alpha;
} SK_TEX_INFO;

typedef	struct _work {
	GV_ACT_EX	actor ;
	OBJECT		weapon ;
	OBJECT		weapon_chanl1 ;	/* 子画面用 */
	OBJECT      sub ;  // option	

	FVECTOR		hit ;

	CONTROL		*ctrl ;
	OBJECT		**body ;
	int			*unit ;
	u_int    *trigger ;

	short		this ;
	short		side ;
	int			flag ;
	GM_CameraSet		*camera ;

	int			chanl ;
	int			vflag ;
	int   		option_flag ;
	Sound      	sound ;
	void   		( *pAct )( struct _work * );
// 追加 by４
	int     ice_eff_count;
	int     ice_eff_tgl;
	int     effect_plus;
    char    effect_flag[ 2 ];
	void    *effect[ 2 ];
} Work ;

extern  void    *NewC4_Ice( FMATRIX * , FVECTOR * , SK_TEX_INFO * , int * , char * );

/*------------------------------------------------------------------*/
static int ReceiveSignal( void *work, int signal, int value )
{
	Work *pWork;

	pWork = ( Work * )work;

	printf("jkljlkjlkjlkjl\n");
	switch ( signal ){
	case GV_SIGNAL_KILL :
		printf("destroy child\n");
		GV_DestroyChild( pWork->effect[ 0 ] );
		GV_DestroyChild( pWork->effect[ 1 ] );
		((GV_ACT *)pWork)->class |= GV_CLASS_FOLLOW;
		GV_DestroyActor( pWork );
		return ( -1 );
	}
	return ( 0 );
}

/*
	マイクを向けられて呼びかけに応える範囲内にいるかチェック
	*/

#define IN_MIC_DIS		(2000.0f)
#define IN_MIC_COS_15	(0.9659258262891f)

int WP_InMic( FVECTOR *pos )
{
	GM_CameraSet	*cam ;
    FVECTOR	d1, d2 ;
	float f, dis ;

	/* カメラを装備している */
	if ( Ply_GetPlayerWeapon() != WP_Mic ) return 0 ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/
    _sceVu0SubVector( &d1, &cam->position, pos ) ;
	dis = GV_VecLen3F( &d1 ) ;

	/* 距離チェック */
	if ( dis > IN_MIC_DIS ) return 0 ;

    _sceVu0SubVector( &d2, &cam->position, &cam->target ) ;
    _sceVu0Normalize( &d1, &d1 ) ;
    _sceVu0Normalize( &d2, &d2 ) ;
    f = _sceVu0InnerProduct( &d1, &d2) ;

	/* 角度チェック */
	if ( f < IN_MIC_COS_15 ) return 0 ;

	return 1 ;
}
/*------------------------------------------------------------------*/

static  void    c4Act( Work *work )
{
	GM_CameraSet *pCam;
	int		chanl, vflag ;
	int		this ;
	u_int	trg ;
	FVECTOR	from, to ;
	FMATRIX	*root, world ;
	char    sound_flag;
	
	chanl = work->chanl ;
	vflag = work->vflag ;
	sound_flag = 0x0;

	GM_SetCurrentMap( work->ctrl->map ) ;
	GM_GroupObjs( work->weapon.objs, work->ctrl->map ) ;
	GM_ResetPlayerStatus( PLAYER_SIGHT_LOCKON ) ;

	this = work->this ;
	GM_ConfigObjectRoot( &( work->weapon ), *( work->body ), *( work->unit ) ) ;
	root = work->weapon.objs->root ;

	pCam = GM_GetCurrentCameraSet( work->chanl );	

	/* 主観専用なので */
	if ( ( work->flag & FLAG_START ) || !( GM_PlayerBody->objs->flag & vflag ) ) {
		DG_InvisibleObjsChanl( work->weapon.objs, chanl ) ;		
		work->flag &= ~FLAG_START ;
		return ;
	} else {
		DG_VisibleObjsChanl( work->weapon.objs, chanl ) ;
	}

	trg = *( work->trigger ) ;
	switch ( trg ) {
	case TRIG_FIRE :
		DG_SetPos( root ) ;
		DG_COPY_VEC( &from, &NozzlePoints[ this ] ) ;
		DG_PutVector( &from, &from, 1 ) ;
		DG_COPY_VEC( &to, &NozzlePoints[ this ] ) ;
		to.vy -= 3000.0F ;			/* 射程 ３ｍ */
		DG_PutVector( &to, &to, 1 ) ;	

		DG_COPY_MAT( &world, root ) ;
		GV_VecToMat( &from, &world ) ;

		work->ice_eff_tgl = 1;

		// サウンド
		if ( work->ice_eff_count == 0 ){
//			spray_num = 0;
			sound_flag = SK_SOUND_START;
		} else if ( !( work->ice_eff_count % DIRECT_TICK( 9 ) ) ){
			sound_flag = SK_SOUND_ON;
		}
		// エフェクト
		work->effect_plus = PlayerPad.pad.pressure[  PL_PAD_PRESS_WEAPON ] / 127; // 6段階  //BP_INPUT - use remapped X360 pad
	    work->effect_plus = work->effect_plus > 5 ? 5 : work->effect_plus;
		if ( work->effect_plus <= 0 ){
			break;
		}
		if ( work->ice_eff_count < DIRECT_TICK( 600 ) ){ // 10秒
			NewBulletSpray( &world, work->ctrl->map, ENEMY_SIDE, WpNum[ this ] );
			if ( sound_flag & SK_SOUND_START ){
				GM_SeSetMode( SD_I_SPRAY01 , (FVECTOR *)world.m[ 3 ] , GM_SEMODE_BOMB );
			}
			if ( sound_flag & SK_SOUND_ON ){
				GM_SeSetMode( SD_I_SPRAY02 , (FVECTOR *)world.m[ 3 ] , GM_SEMODE_BOMB );
			}
			work->effect_flag[ 0 ] = SK_EFFECT_ADD;
			work->effect_flag[ 1 ] = SK_EFFECT_ADD;
			work->ice_eff_count++;
			GM_SetWeaponFire( WP_ColdSpray ) ;
		} else if ( work->ice_eff_count < DIRECT_TICK( 700 ) ){
			{ 
				work->effect_plus = 5;
				if ( ( work->ice_eff_count % 5 ) == 0 ){
					if ( sound_flag & SK_SOUND_ON ){
						GM_SeSetMode( SD_I_SPRAY02 , (FVECTOR *)world.m[ 3 ] , GM_SEMODE_BOMB );
					}
					work->effect_flag[ 0 ] = SK_EFFECT_ADD;
					work->effect_flag[ 1 ] = SK_EFFECT_ADD;
				}
			}
			work->ice_eff_count++;
		}
		break;
	default :
		work->ice_eff_count = 0;
		if ( work->ice_eff_tgl != 0 ){
			work->ice_eff_tgl = 0;
		}
		break;
	}
}

static  void    MicAct( Work *work )
{
	GM_CameraSet *pCam;
	SVECTOR       stmp;
	float         ftmp;
	float            r;
	int            vol;
	int			   chanl, vflag ;
	const int TIME = DIRECT_TICK( 7 );

	chanl = work->chanl ;
	vflag = work->vflag ;

	GM_SetCurrentMap( work->ctrl->map ) ;
	GM_ConfigObjectRoot( &( work->weapon ), *( work->body ), *( work->unit ) ) ;

	pCam = GM_GetCurrentCameraSet( work->chanl );	

	if ( ( work->flag & FLAG_START ) || !( GM_PlayerBody->objs->flag & vflag ) ) {
		DG_InvisibleObjsChanl( work->weapon.objs, chanl ) ;		
		work->flag &= ~FLAG_START ;
		return ;
	} else {
		DG_VisibleObjsChanl( work->weapon.objs, chanl ) ;
	}


   if( (BP_IsPAL()==TRUE && !(work->sound.count % 10) ) ||    //#define NOIZE_CNT 10
	    (BP_IsPAL()!=TRUE && !(work->sound.count % 14)) )      //#define NOIZE_CNT 14

   {

#ifdef KP_XBOX
		if ( (work->sound.toggle ^= 1) ) {
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_MICNOIZE );
		} else {
			GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_MICSWING );
		}
#else
		GM_SeSet( GM_PAN_CENTER , GM_MAX_VOL , SD_S_MICNOIZE );
#endif
	}

	/* この音はPSX2のみになりました */
#ifdef PSX2
	if ( !(work->sound.count % TIME) ){ // 速度を出す
		stmp.vx = pCam->rotate.vx - work->sound.rot.vx; // 移動した角度を入力
		stmp.vy = pCam->rotate.vy - work->sound.rot.vy; // 移動した角度を入力
		stmp.vx = stmp.vx < 0 ? -stmp.vx : stmp.vx;
		stmp.vy = stmp.vy < 0 ? -stmp.vy : stmp.vy;
		if ( stmp.vx == 0 || stmp.vy == 0 ){ // 一次元か二次元か
			r = 63.f;
		} else {
			r = 31.f;
		}
		ftmp = 2 * M_PI * r; // 31半径のこと(円周をだす)
		stmp.vz  = ( ( ftmp * stmp.vx ) / 360 ) / TIME;  
		stmp.vz += ( ( ftmp * stmp.vy ) / 360 ) / TIME;
		work->sound.rot = pCam->rotate;
		vol = ( int )( stmp.vz * 1.53 ); // 1.53はただの色
		vol = ( vol > 0x3f ? 0x3f : vol );
		/*printf("vol %d\n" , vol);*/
		GM_SeSet( GM_PAN_CENTER , vol , SD_S_MICSWING );
	}
#endif
	work->sound.count++;
	work->sound.count &= 0x7fff;
}

static	void	Act( work )
Work		*work ;
{
	work->pAct( work );
}

static  void SubDie( Work *pWork )
{
	if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, pWork->sub.objs ) ;
	}
	GM_FreeObject( &( pWork->sub ) ) ;
}

static	void	Die( work )
Work		*work ;
{
	// by koba4 2001/06/02
	work->effect_flag[ 0 ] = SK_EFFECT_DESTROY;
	work->effect_flag[ 1 ] = SK_EFFECT_DESTROY;

	if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon_chanl1.objs ) ;
	}
	if ( work->option_flag == OPTION_ON ){
		SubDie( work );
	}
	GM_FreeObject( &( work->weapon ) ) ;
	GM_FreeObject( &( work->weapon_chanl1 ) ) ;	
	GM_PlayerWeaponBody = NULL ;
	GM_PlayerSubWeaponBody = NULL ;
}

/*------------------------------------------------------------------*/

static void InitSound( Work *work )
{
	GM_CameraSet *pCam;

	pCam = GM_GetCurrentCameraSet( GM_CurrentCameraChanl );	
	work->sound.rot = pCam->rotate;
	work->sound.count = 0;
#ifdef KP_XBOX
	work->sound.toggle = 0 ;
#endif
}

static	int	GetResources( work, body, unit, this )
Work		*work ;
OBJECT		**body ;
int		*unit ;
int		this ;
{
	OBJECT	*weapon ;
	OBJECT  *sub ;
	SK_TEX_INFO tex;
	FVECTOR shift = { 13.0f, -303.0f, 61.0f, 0.0f };

	weapon = &( work->weapon ) ;
	sub    = &( work->sub ) ;

	GM_InitObject( weapon, BodyDatas[ this ], BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( weapon, GM_PlayerArmBody, *unit ) ;

	/* 子画面用 */
	GM_InitObject( &work->weapon_chanl1, BodyDatas[ this ], BODY_FLAG ) ;
	if ( work->weapon_chanl1.objs == NULL ) return -1 ;
	GM_ConfigObjectRoot( &work->weapon_chanl1, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
	DG_InvisibleObjs( work->weapon_chanl1.objs ) ;
	DG_VisibleObjsChanl( work->weapon_chanl1.objs, 1 ) ;
	work->weapon_chanl1.objs->objs[ 0 ].flag |= DG_FLAG_INVISIBLE0 ;
	DG_ConnectObjs( GM_PlayerBody->objs, work->weapon_chanl1.objs ) ;

	//work->camera = GM_PlayerSubjectCamera[ GM_CurrentCameraChanl ] ;
	work->camera = GM_FindCameraSet( GV_StrCode( "武器カメラ" ), 0 ) ;

	work->flag = 0 ;
	work->ice_eff_tgl = 0;
	work->ice_eff_count = 0;
	DG_InvisibleObjs( weapon->objs ) ;

	GM_PlayerWeaponModel = BodyDatas[ this ] ;
	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon ;
	InitSound( work );
	switch ( this ){
	case THIS_IS_MIC : // ヘッドホンのついか
		work->pAct = ( void * )MicAct;
		GM_InitObject( sub , HEADPHONES , BODY_FLAG ); // ヘッドホンモデル
		if ( sub->objs == NULL ) return -1 ;
		GM_ConfigObjectRoot( sub , GM_PlayerBody, HUMAN21_ATAMA ) ;
		DG_ConnectObjs( GM_PlayerBody->objs , work->sub.objs ) ;
		DG_InvisibleObjs( sub->objs );	
		DG_VisibleObjsChanl( sub->objs, 1 ) ;
		work->option_flag = OPTION_ON;
		break;
	default :
		// signal add
		GV_SetActorSignalFunc( work , ReceiveSignal );
		// etc
		work->pAct = ( void * )c4Act;
		work->option_flag = OPTION_OFF;
		// 氷Effect
		tex.alpha = 3;
		tex.num = &work->effect_plus;
		tex.strcode = ICE_DUST;
		work->effect_flag[ 0 ] = SK_EFFECT_NORMAL;
		work->effect[ 0 ] = NewC4_Ice( &work->weapon.objs->world , &shift , &tex , work->trigger , &work->effect_flag[ 0 ] );
		GV_SetActorChild( work , work->effect[ 0 ] );
		// 氷Effect2
		tex.alpha = 4;
		tex.num = &work->effect_plus;
		tex.strcode = ICE_LOST;
		work->effect_flag[ 1 ] = SK_EFFECT_NORMAL;
		work->effect[ 1 ] = NewC4_Ice( &work->weapon.objs->world , &shift , &tex , work->trigger , &work->effect_flag[ 1 ] );
		GV_SetActorChild( work , work->effect[ 1 ] );
		break;
	}
	work->flag |= FLAG_START ;
	return 0 ;
}

static	void	*New( ctrl, body, unit, trigger, side, this )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side, this ;
{
	Work	*work ;
  
	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->trigger = trigger ;
		if ( GetResources( work, body, unit, this ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->this = this ;
		work->side = side ;
		work->chanl = GM_CurrentCameraChanl ;
		work->vflag = ( DG_FLAG_INVISIBLE0 << work->chanl ) ;
	}
	return work ;
}

/*------------------------------------------------------------------*/

/* Cold Spray */
void	*NewColdSpray( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int			*unit ;
u_int		*trigger ;
int		side ;
{
	return New( ctrl, body, unit, trigger, side, THIS_IS_COLD ) ;
}

/* Melt Spray */
void	*NewMeltSpray( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
	return New( ctrl, body, unit, trigger, side, THIS_IS_MELT ) ;
}

/* Fire Extinguisher*/
void	*NewFireExtinguisher( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
	return New( ctrl, body, unit, trigger, side, THIS_IS_FIRE ) ;
}

/* マイク */
void	*NewMic( CONTROL *ctrl, OBJECT **body, int *unit, u_int *trigger, int side )
{
	return New( ctrl, body, unit, trigger, side, THIS_IS_MIC ) ;
}
