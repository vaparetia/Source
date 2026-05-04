//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	slinggun.c
	銃
	
	1999/10/19 Y.Korekado
	$Id: slinggun.c,v 1.1.1.3 2002/11/19 11:44:05 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewSlingGun( name, where )

----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#endif

#include	"gameheader.h"
#include	"define.h"
#include	"korekado/conv/korekado.x"
#include	"../../skoba/weapon_old/matsu.h"

extern	void *NewOpticalCamouflage( DG_OBJS *objs, int flag ) ;

/*----------------------------------------------------------------*/
#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_ONEPIECE)
#define	BODY_FLAG2	(DG_FLAG_TEXT|DG_FLAG_PAINT|DG_FLAG_FINISHCALC)

#define	LEFT_HAND	SW_FLAG_SWITCH1	/* 左手に持つ */
#define	FREE_HAND	SW_FLAG_SWITCH2	/* 放す */

#define	QUICK_FREE_HAND	SW_FLAG_SWITCH3	/* 瞬間に胸につける */

#define	MESS_INVISIBLE	SW_FLAG_SWITCH4	/* 銃消す */

#define	MESS_QUICK_FREE	SW_FLAG_SWITCH5	/* メッセージにより瞬間に胸につける */

//#define E_WP_SPS	(MDL_SPS)		/* (GV_StrCode( "sps" )) */
//#define E_WP_ABAKAN	(MDL_ABAKAN)		/*abk.kms*/
/*重野追加*/
#define GUN_TYPE_NORMAL		(0x00000000) //可動部無し
#define GUN_TYPE_MOVABLE	(0x00000001) //可動部あり


#define GUN_TYPE_ABAKAN		(0x00000002) //アバカン
#define GUN_TYPE_M4			(0x00000004) //M4


/**********/
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		*body ;
	OBJECT		*gun ;
	int			*flag ;
	
	FVECTOR		pos ;
	int			rot ;
	int			gun_type ; //sigeno add
	WEAPON_EF_CTRL *wctrl ; //sigeno add

	FVECTOR		*g_shift ;
	SVECTOR		*g_rot ;

	FVECTOR		quat_release ;
	FVECTOR		pos_release ;
	FVECTOR		quat_hand ;
	FVECTOR		pos_hand ;
	FMATRIX		gun_world ;

	int			think1 ;
	u_int			count ;
} Work ;

/*----------------------------------------------------------------*/
#define PRIO	0x40	/* 敵兵より後に処理 */

/* AK:タンカーゴル兵スリング */
static FVECTOR	AK_SHIFT = { -200.0, 110.0, 200.0 } ;
static SVECTOR	AK_ROT = { -338, 790, 130 } ;	/* Z->X->Y */

/* ABAKAN:プラントゴル兵スリング */
static FVECTOR	ABAKAN_SHIFT = { -200.0, 110.0, 200.0 } ;
static SVECTOR	ABAKAN_ROT = { -338, 790, 130 } ;	/* Z->X->Y */

/* M4:チョーカースリング */
static FVECTOR	M4_SHIFT = { -100.0, 0.0, 250.0 } ;
static SVECTOR	M4_ROT = { -258, 890, 100 } ;	/* Z->X->Y */

enum {
	SLING_TYPE_AK,
	SLING_TYPE_ABAKAN,
	SLING_TYPE_M4,
} ;

enum {
	HAND_IN, HAND_RELEASE, BODY_ON, GET_GUN, HAND_CATCH
} ;

/*----------------------------------------------------------------*/
void	EneSlingResetPosition( OBJECT *body, OBJECT *weapon, int flag, int wp )
{
	FVECTOR *g_shift ;
	SVECTOR *g_rot ;

	if ( flag & (FREE_HAND|QUICK_FREE_HAND) ) {
		switch (wp){
			case MDL_SPS :
				g_shift = &AK_SHIFT ;
				g_rot = &AK_ROT ;
				break ;
			case MDL_ABAKAN :
				g_shift = &ABAKAN_SHIFT ;
				g_rot = &ABAKAN_ROT ;
				break ;
			case MDL_M4B_GRD :
				g_shift = &M4_SHIFT ;
				g_rot = &M4_ROT ;
				break ;
			default :
				g_shift = &AK_SHIFT ;
				g_rot = &AK_ROT ;
				break ;
		}
		DG_SetPos( &BODYWORLD( body, HUMAN21_MUNE ) ) ;
		DG_MovePos( g_shift ) ;
		DG_RotatePos( g_rot ) ;
		DG_PutObjs( weapon->objs ) ;
	} else if ( flag & LEFT_HAND ) {
		DG_SetPos( &BODYWORLD( body, HUMAN21_HIDARI_TE ) ) ;
		DG_PutObjs( weapon->objs ) ;
	} else {
		DG_SetPos( &BODYWORLD( body, HUMAN21_MIGI_TE ) ) ;
		DG_PutObjs( weapon->objs ) ;
	}

if ( 0 ){
	FVECTOR pos ;
	
	KR_FMatToFvec( &weapon->objs->world, &pos ) ;
	printf("Resettttttttttt sling pos [%f][%f][%f]\n",pos.vx,pos.vy,pos.vz);
}
}
/*----------------------------------------------------------------*/
static	void	ReleaseStyle( work )
Work		*work ;
{
	FMATRIX	mat ;

	DG_SetPos( &work->body->objs->objs[HUMAN21_MUNE].world ) ;
	DG_MovePos( work->g_shift ) ;
	DG_RotatePos( work->g_rot ) ;
	DG_GetPos( &mat ) ;
#if 1
	MT_MatToQuat( &work->quat_release, &mat ) ;
#else
{
	extern void UTL_MakeQuatFromOBJ(FVECTOR *q,DG_DEF *def,FVECTOR *t,SVECTOR *r,
						 OBJECT *target,int objnum,FVECTOR *x,int objnum2,FVECTOR *x2);
	static FVECTOR saki = { 0.0f, -300.0f, 0.0f, 0.0f } ;
	static FVECTOR s2 = { 150.0f, 0.0f, 50.0f, 0.0f } ;
	UTL_MakeQuatFromOBJ( &work->quat_release, work->gun->objs->def, &saki, NULL,
			work->body, 2, work->g_shift, 0, &s2) ;

}
#endif
	KR_FMatToFvec( &mat, &work->pos_release ) ;
}

static	void	HandStyle( work )
Work		*work ;
{
	FMATRIX	mat ;

	if ( *work->flag & LEFT_HAND ) {
		mat = work->body->objs->objs[HUMAN21_HIDARI_TE].world ;
	} else {
		mat = work->body->objs->objs[HUMAN21_MIGI_TE].world ;
	}
	MT_MatToQuat( &work->quat_hand, &mat ) ;
	KR_FMatToFvec( &mat, &work->pos_hand ) ;
}

#if 0
static	void	SetGunPos( work, s, p )
Work		*work ;
int			s ;
float		p ;
#else
static	void	SetGunPos( Work *work, int s, float p )
#endif
{
	FVECTOR	q, res, pos ;
	FMATRIX	*w ;

	w = &work->gun->objs->world ;

	/* 目標座標、角度 計算 */
	ReleaseStyle( work ) ;

	/* 座標補完 */
	KR_FMatToFvec( w, &pos ) ;
//	GV_NearExp8VF( &pos, &work->pos_release, 3 ) ;
	GV_NearTimeVF( &pos, &work->pos_release, s, 3 ) ;

	/* クォータニオン補完 */
	MT_MatToQuat( &q, w ) ;
	MT_QuatSlerp( &res, &q, &work->quat_release, p ) ;
	MT_QuatNormalize( &res, &res ) ;
	MT_QuatToMat( &work->gun_world , &res ) ;

	KR_FvecToMat( &pos, &work->gun_world ) ;
}

static	void	SetGunHandPos( work )
Work		*work ;
{
	FVECTOR	q, res, pos ;
	FMATRIX	*w ;

	w = &work->gun->objs->world ;

	/* 目標座標、角度 計算 */
	HandStyle( work ) ;

	/* 座標補完 */
	KR_FMatToFvec( w, &pos ) ;
//	GV_NearExp8VF( &pos, &work->pos_hand, 3 ) ;
//	GV_NearTimeVF( &pos, &work->pos_hand, 4, 3 ) ;
	GV_NearTimeVF( &pos, &work->pos_hand, COUNT_VMODE(8), 3 ) ;

	/* クォータニオン補完 */
	MT_MatToQuat( &q, w ) ;
	MT_QuatSlerp( &res, &q, &work->quat_hand, 0.5 ) ;
	MT_QuatNormalize( &res, &res ) ;
	MT_QuatToMat( &work->gun_world , &res ) ;

	KR_FvecToMat( &pos, &work->gun_world ) ;
}

/*----------------------------------------------------------------*/

static	void	HandIn( work )
Work		*work ;
{
	if ( *work->flag & FREE_HAND ) {
		work->think1 = HAND_RELEASE ;
		work->count = 0 ;
		return ;
	} else if ( *work->flag & QUICK_FREE_HAND ) {
		SetGunPos( work, 1, 1.0f ) ;
		work->think1 = BODY_ON ;
		work->count = 0 ;
		return ;
	} else if ( *work->flag & LEFT_HAND ) {
		work->gun_world = BODYWORLD( work->body, HUMAN21_HIDARI_TE ) ;
	} else {
		work->gun_world = BODYWORLD( work->body, HUMAN21_MIGI_TE ) ;
	}

	work->count ++ ;
}

static	void	HandRelease( work )
Work		*work ;
{
	SetGunPos( work, COUNT_VMODE(8)-work->count, 0.2f ) ;
	if ( work->count >= COUNT_VMODE(8) ) {
		work->think1 = BODY_ON ;
		work->count = 0 ;
	}
	work->count ++ ;
}

static	void	BodyOn( work )
Work		*work ;
{
	SetGunPos( work, 1, 1.0f ) ;
	if ( !(*work->flag & (FREE_HAND|QUICK_FREE_HAND)) ) {
//		work->think1 = HAND_IN ;
		work->think1 = HAND_CATCH ;
		work->count = 0 ;
	}
	work->count ++ ;
}

static	void	HandCatch( work )
Work		*work ;
{
	SetGunHandPos( work ) ;
	if ( *work->flag & FREE_HAND ) {
		work->think1 = HAND_RELEASE ;
		work->count = 0 ;
		return ;
	} else if ( *work->flag & QUICK_FREE_HAND ) {
		SetGunPos( work, 1, 1.0f ) ;
		work->think1 = BODY_ON ;
		work->count = 0 ;
		return ;
	}
	if ( work->count > 1 ) {
		work->think1 = HAND_IN ;
		work->count = 0 ;
	}
	work->count ++ ;
}

static	void	Think( work )
Work		*work ;
{
	switch( work->think1 ) {
		case HAND_IN:
			HandIn( work ) ;
		break ;
		case HAND_RELEASE:
			HandRelease( work ) ;
		break ;
		case BODY_ON:
			BodyOn( work ) ;
		break ;
		case HAND_CATCH:
			HandCatch( work ) ;
		break ;

		case GET_GUN :
		break ;
	}
}

static	void	SetInvisible( work )
Work	*work ;
{
	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ) {
		work->gun->objs->flag |= DG_FLAG_INVISIBLE ;
	} else {
		if ( *(work->flag) & SW_FLAG_VISIBLE ) {
			work->gun->objs->flag &= ~DG_FLAG_INVISIBLE ;
if ( 0 ){
	FVECTOR pos ;
	
	KR_FMatToFvec( &work->gun->objs->world, &pos ) ;
	printf("sling pos [%f][%f][%f]\n",pos.vx,pos.vy,pos.vz);
}
		} else {
			work->gun->objs->flag |= DG_FLAG_INVISIBLE ;
		}
	}
	KR_GroupObject( work->gun, work->body->map_name ) ;

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
		if ( work->body->objs->flag & DG_FLAG_OPTCMF ) {
			SET_FLAG ( work->gun->objs->flag, DG_FLAG_OPTCMF ) ;
		} else {
			UNSET_FLAG ( work->gun->objs->flag, DG_FLAG_OPTCMF ) ;
		}
	}
}

#if 0
static	void	StatusCheck( Work *work )
{
	
	if ( 0 & 0 ) {
	}
}
#endif

static	void	Act( work )
Work		*work ;
{
//	StatusCheck( work ) ;

	Think( work ) ;
	
	SetInvisible( work ) ;
	DG_SetPos( &work->gun_world ) ;
	if(work->gun_type & GUN_TYPE_MOVABLE ){
		/*重野追加*/
		/*パーツ制御はWeaponEfActにまかせる*/
		WeaponEfAct( work->wctrl );
		/*WeaponEfActで現在位置が変更されるときがあるので再設定*/
		DG_SetPos( &work->gun_world ) ;
		DG_PutObjs( work->gun->objs ) ;
	}else {
		DG_PutObjs( work->gun->objs ) ;
	}
}

static	void	Die( work )
Work		*work ;
{
	extern void DG_DisconnectObjs( DG_OBJS *, DG_OBJS * ) ;
	
	if ( !(GV_IsStageDestroy( work )) ) {
		DG_DisconnectObjs( work->body->objs, work->gun->objs ) ;
	}
    GM_FreeObject( work->gun ) ;
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, body, gun, data, flag, wctrl )
Work	*work ;
OBJECT	*body ;	/* ボディーオブジェクト */
OBJECT	*gun ;	/* 銃オブジェクト */
int		data ;
int		*flag ;
WEAPON_EF_CTRL *wctrl ;
{
	extern void DG_ConnectObjs( DG_OBJS *, DG_OBJS * ) ;

	work->body = body ;
	work->gun = gun ;
	work->flag = flag ;
	work->wctrl = wctrl ;
	/* 武器 */


	if ( *(work->flag) & SW_FLAG_SWITCH6 ) {
	/*重野追加*/
		/*死体から呼ばれたのでワンピースモデルとして呼び出し*/
		work->gun_type = GUN_TYPE_NORMAL ;
		GM_InitObject( gun, data, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );

	}else {
		switch (data){
			case MDL_SPS :
				WeaponEfInitObject( wctrl, gun, body, 6, WP_ShotGun_Near, data ) ;
				SET_FLAG( work->gun_type, GUN_TYPE_MOVABLE ) ;
				break ;
			case MDL_ABAKAN :
			/*可動部のある武器*/
				WeaponEfInitObject( wctrl, gun, body, 6, WP_Abk, data ) ;
				SET_FLAG( work->gun_type, (GUN_TYPE_MOVABLE|GUN_TYPE_ABAKAN) ) ;
				break ;
			case MDL_M4B_GRD :
				WeaponEfInitObject( wctrl, gun, body, 6, WP_M4_Grd_Htc, data ) ;
				SET_FLAG( work->gun_type, (GUN_TYPE_MOVABLE|GUN_TYPE_M4) ) ;
				break ;
			default :
				work->gun_type = GUN_TYPE_NORMAL ;
				GM_InitObject( gun, data, DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
				break ;
		}
	}

	switch (data){
		case MDL_SPS :
			work->g_shift = &AK_SHIFT ;
			work->g_rot = &AK_ROT ;
			break ;
		case MDL_ABAKAN :
			work->g_shift = &ABAKAN_SHIFT ;
			work->g_rot = &ABAKAN_ROT ;
			break ;
		case MDL_M4B_GRD :
			work->g_shift = &M4_SHIFT ;
			work->g_rot = &M4_ROT ;
			break ;
		default :
			work->g_shift = &AK_SHIFT ;
			work->g_rot = &AK_ROT ;
			break ;
	}

	if ( *work->flag & (FREE_HAND|QUICK_FREE_HAND) ) {
		DG_SetPos( &work->body->objs->objs[HUMAN21_MUNE].world ) ;
		DG_MovePos( work->g_shift ) ;
		DG_RotatePos( work->g_rot ) ;
		DG_PutObjs( work->gun->objs ) ;
		work->think1 = HAND_RELEASE ;
	} else if ( *work->flag & LEFT_HAND ) {
		DG_SetPos( &BODYWORLD( work->body, HUMAN21_HIDARI_TE ) ) ;
		DG_PutObjs( work->gun->objs ) ;
		work->think1 = HAND_IN ;
	} else {
		DG_SetPos( &BODYWORLD( work->body, HUMAN21_MIGI_TE ) ) ;
		DG_PutObjs( work->gun->objs ) ;
		work->think1 = HAND_IN ;
	}
	DG_SetLightMatrix( gun->objs, body->objs->light );

	DG_ConnectObjs( body->objs, gun->objs ) ;

	if(work->gun_type & GUN_TYPE_MOVABLE){
		/*重野追加*/
		/*パーツ制御はWeaponEfActにまかせる*/
		work->wctrl->pobjs->root = &work->gun_world ;
		WeaponEfAct( work->wctrl );
	}

if ( 0 ){
	FVECTOR pos ;
	
	KR_FMatToFvec( &work->body->objs->objs[HUMAN21_MUNE].world, &pos ) ;
	printf("starrrrrrrrrrrrt sling pos [%f][%f][%f]\n",pos.vx,pos.vy,pos.vz);
}

	work->count = 0 ;

	if ( body->objs->flag & DG_FLAG_OPTCMF ) {
		GV_SetActorChild( work, NewOpticalCamouflage( gun->objs, 0 ) ) ;
	}

	return 0 ;
}

void	*NewSlingGun( body, gun, data, flag )
OBJECT	*body ;	/* ボディーオブジェクト */
OBJECT	*gun ;	/* 銃オブジェクト */
int		data ;
int		*flag ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, gun, data, flag, NULL ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

void	*NewSlingGun2( body, gun, data, flag, wctrl )
OBJECT	*body ;	/* ボディーオブジェクト */
OBJECT	*gun ;	/* 銃オブジェクト */
int		data ;
int		*flag ;
WEAPON_EF_CTRL *wctrl ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, body, gun, data, flag, wctrl ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
