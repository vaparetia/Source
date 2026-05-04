//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   nkt.c
   ニキータ、ＲＧＢ６
   
   2000/02/24 M.Sonoyama
   $Id: nkt.c,v 1.1.1.3 2002/11/19 11:50:29 Yoshizawa1 Exp $
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
#include	"../weapon_old/matsu.h"


/*------------------------------------------------------------------*/

/* 外部関数・変数宣言 */
extern	void	*NewCartridgeControl( int *, OBJECT *, OBJECT *, int ) ;

extern	void	UTL_StartSprToMem( void *, void *, int ) ;
extern	void	UTL_StartMemToSpr( void *, void *, int ) ;
extern	void	UTL_EndSprToMem( void ) ;
extern	void	UTL_EndMemToSpr( void ) ;

extern	void	*NewBulletNikita( FMATRIX *, CONTROL *, int, int ) ;
extern	void	*NewBulletRGB6( FMATRIX *, CONTROL *, int, int ) ;
extern  void	*NewNikitaSight( void );

/*------------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_ONEPIECE)
#define	BODY_FLAG_SUB	(DG_FLAG_SHADE | DG_FLAG_ONEPIECE)

#define	TEXTURE		TXT_NIKITA_LIGHT

#define	PRIM_FLAG	(DG_PRIM2_SPRT | DG_PRIM2_TEX | DG_PRIM2_FOG | DG_PRIM2_ALPHA)
#define	N_PRIMS		(1)
#define	N_VERTS		(1)

#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

#define	MOTION_NAME		(120130)	/* rgb.mar */

#define	RGB_AMO_GRIP		(13087836)	/* rgb_amo_grip */
#define	RGB_AMO_MOUNTED		(15680088)	/* rgb_amo_mounted */

/*------------------------------------------------------------------*/

enum {
    THIS_IS_NKT = 0,
    THIS_IS_RGB
} ;

static	u_int	Bodys[] = {
	MDL_NIKITA,
	MDL_NIKITA,
	MDL_RGB,
	MDL_RGB_SUB
} ;

static	FVECTOR	Shoot[] = {
    { 17.5F, -498.0F, 138.7F },
    { 17.5F, -635.0F, 73.7F } 
} ;

static	int	WpNum[] = {
    WP_Nikita,
    WP_Rgb6
} ;

/*------------------------------------------------------------------*/

typedef	struct {
    GV_ACT_EX	actor ;
    OBJECT		weapon ;
    OBJECT		weapon_sub ;
    
	DG_OBJS		*amo_grip ;
	DG_OBJS		*amo_mounted ;

    CONTROL		*ctrl ;
    OBJECT		**body ;

    WEAPON_EF_CTRL	bctrl ;
    WEAPON_EF_CTRL	bctrl_sub ;
    
    int			*unit ;
    u_int		*trigger ;
    int			side ;
    int			chanl ;
	int			vflag ;

    DG_TEX		*tex ;
    DG_PRIM2		*prim ;

    GM_CameraSet	*camera ;
    int			this ;
	int			count ;

	int			motion_set ;
	int			amo_visible ;
	// koba4
	int			crtrgctrl ;
	int			crtrgctrl_sub ;
} Work ;

#define	RGB_COUNT		(DIRECT_TICK(60))

/*------------------------------------------------------------------*/

static	inline	void	SprToMem( dst, src, size, num )
void		*dst, *src ;
int		size, num ;
{
    UTL_StartSprToMem( dst, src, size * num / sizeof( u_long128 ) ) ;
    UTL_EndSprToMem() ;
}

static	inline	void	MemToSpr( dst, src, size, num )
void		*dst, *src ;
int		size, num ;
{
    UTL_StartMemToSpr( dst, src, size * num / sizeof( u_long128 ) ) ;
    UTL_EndMemToSpr() ;
}

/*------------------------------------------------------------------*/

/* マップ更新 */
static	inline	void	UpdateMap( work )
Work		*work ;
{
    int		map ;

    map = GM_CurrentMap = work->ctrl->map ;
    work->weapon.map_name = map ;
    GM_GroupObjs( work->weapon.objs, map ) ;
    work->weapon_sub.map_name = map ;
    GM_GroupObjs( work->weapon_sub.objs, map ) ;
	if ( work->this == THIS_IS_RGB ) {
		if ( work->amo_grip != NULL ) GM_GroupObjs( work->amo_grip, map ) ;
		if ( work->amo_mounted != NULL ) GM_GroupObjs( work->amo_mounted, map ) ;
	}
}

/*------------------------------------------------------------------*/
// sound
static  void    SoundManage( Work *pWork , int this )
{
	GM_SetNoise( NOISE_M, &pWork->ctrl->mov, pWork->ctrl->map ) ;
	switch( this ) {
	case THIS_IS_NKT :
		GM_SeSetMode( SD_W_NIKFIRE1 , &pWork->ctrl->mov, GM_SEMODE_BOMB ) ; 
		break ;
	case THIS_IS_RGB :
		//GM_SeSetMode( SD_W_LANTURE2 , &pWork->ctrl->mov, GM_SEMODE_BOMB ) ; 
		PL_SeSetSubject( SD_W_LANTURE2 , &pWork->ctrl->mov, GM_SEMODE_BOMB ) ; 
		break ;
	}
}

static	void	SetMotion( Work *work )
{
	work->amo_visible = 0 ;
	if ( work->motion_set != 0 ) return ;
	work->motion_set = 1 ;
	GM_ConfigObjectAction( &work->weapon, 0, 0, 0, 0xfffff, 0 ) ;
	GM_ConfigObjectAction( &work->weapon_sub, 0, 0, 0, 0xfffff, 0 ) ;
	work->weapon.m_ctrl->mt3_ctrl[ 0 ].flag |= MT3_SLEEP ;
	work->weapon_sub.m_ctrl->mt3_ctrl[ 0 ].flag |= MT3_SLEEP ;
}

static	void	StartMotion( Work *work ) 
{
	work->weapon.m_ctrl->mt3_ctrl[ 0 ].flag &= ~MT3_SLEEP ;
	work->weapon_sub.m_ctrl->mt3_ctrl[ 0 ].flag &= ~MT3_SLEEP ;
	work->motion_set = 0 ;
}


static	void	Act( Work *work )
{
	int		chanl, vflag ;
    u_int	trg ;
    int		this, weapon ;
	int		cancel ;
    FMATRIX	world, *root ;
    FVECTOR	mov ;
    SVECTOR	*rot ;

	chanl = work->chanl ;
	vflag = work->vflag ;
    this = work->this ;
    weapon = WpNum[ this ] ;

    UpdateMap( work ) ;

    if ( GM_PlayerBody->objs->flag & vflag ) {
		DG_InvisibleObjsChanl( work->weapon.objs, 0 ) ;
		DG_VisibleObjsChanl( work->weapon.objs, 1 ) ;
		DG_VisibleObjsChanl( work->weapon_sub.objs, 0 ) ;
		rot = &work->camera->rotate ;
    } else {
		DG_VisibleObjsChanl( work->weapon.objs, 0 ) ;
		DG_VisibleObjsChanl( work->weapon.objs, 1 ) ;
		DG_InvisibleObjsChanl( work->weapon_sub.objs, 0 ) ;
		rot = &work->ctrl->rot ;
    }

	/* ニキータの握り位置が違うためスネークのときだけずらすことにする */
	/* スネークのニキータ以外はここでルートなどが設定される */
	if ( GM_PlayerBody->objs->flag & vflag ) {
		GM_ConfigObjectRoot( &( work->weapon_sub ), 
							*( work->body ), *( work->unit ) ) ;
		root = work->weapon_sub.objs->root ;
	} else {
		if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) && this == THIS_IS_NKT ) {
			static FVECTOR offset = { 19.511, -58.498, 4.480, 1.0f } ;
			FMATRIX *weapon = &work->weapon.objs[0].world ;
			FMATRIX *hand   = &(*work->body)->objs->objs[ *work->unit ].world ;
			
			work->weapon.objs->root = NULL ;
			_sceVu0CopyMatrix( weapon, hand ) ;
			_sceVu0ApplyMatrix( weapon->m[3], hand, &offset ) ;
			root = hand ;
		} else {
			GM_ConfigObjectRoot( &( work->weapon ), 
								*( work->body ), *( work->unit ) ) ;
			root = work->weapon.objs->root ;
		}		
	}


	if ( work->count > 0 ) -- work->count ;

	if ( this == THIS_IS_NKT ) {
		/* サイト表示管理 */
		if ( GM_NikitaAlive[ 0 ] == NKT_NORMAL ) GM_ResetSightStatus( SGT_Nikita ) ;	
		else									 GM_SetSightStatus( SGT_Nikita ) ;	
	} else if ( this == THIS_IS_RGB ) {
		if ( work->count > 0 || GM_N_RGB6S >= GM_CURRENT_RGB6_MAX ) {
			GM_WeaponAlive |= WP_CANNOT_FIRE_RGB6 ;
		} else {
			GM_WeaponAlive &= ~WP_CANNOT_FIRE_RGB6 ;
		}
	}

	cancel = 0 ;

    trg = *( work->trigger ) ;
    switch ( trg ) {
	case TRIG_RELOAD_START :
		StartMotion( work ) ;
		break ;
    case TRIG_SET :
		GM_NikitaLife = GM_NikitaLifeMax ;
		if ( this == THIS_IS_NKT && 
			( GM_PlayerBody->objs->flag & vflag ) ) {
			/* サイト表示許可 */
			GM_ResetSightStatus( SGT_Nikita ) ;
		}
		break ;
    case TRIG_FIRE :
		if ( GM_WeaponNum( weapon ) == 0 ) {
			/* から撃ち */
            //GM_SeSetMode( SD_W_EMPTY01, &work->ctrl->mov, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_W_EMPTY01, &work->ctrl->mov, GM_SEMODE_BOMB ) ;
			break ;
		}
		DG_SetPos( root ) ;
		DG_MovePos( &Shoot[ this ] ) ;
		DG_GetPos( &world ) ;
		switch( this ) {
		case THIS_IS_NKT :
			/* サイト表示許可 */
			GM_ResetSightStatus( SGT_Nikita ) ;
			/* カメラ位置から発射 */
			rot = &GM_CameraDir ;
			DG_COPY_VEC( &mov, &GM_PlayerSubjectCamera[ 0 ]->position ) ;
			DG_SetPos2( &mov, rot ) ;
			DG_GetPos( &world ) ;
			NewBulletNikita( &world, work->ctrl, BOTH_SIDE, work->chanl ) ;	    
			break ;
		case THIS_IS_RGB :
			if ( GM_WeaponAlive & WP_CANNOT_FIRE_RGB6 ) break ;
			//GV_MatToVec( &world, &mov ) ;	
			//DG_SetPos2( &mov, rot ) ;
			//DG_GetPos( &world ) ;
			NewBulletRGB6( &world, work->ctrl, BOTH_SIDE, work->chanl ) ;
			work->count = RGB_COUNT ;
			// 追加こば４
			work->bctrl.flag = WPEF_FLG_START;
			work->bctrl_sub.flag = WPEF_FLG_START;
			break ;
		} 
		SoundManage( work , this );
		break ;
	case TRIG_MAG_FALL : // あるかも
		/* マガジン落下 */
		work->crtrgctrl = 4 ;
		work->crtrgctrl_sub = 4 ;
		break ;
	case TRIG_MAG_LEFT_HAND :
		/* 左手もち */
		work->crtrgctrl = 2 ;
		work->crtrgctrl_sub = 2 ;
		break ;
	case TRIG_MAG_INVISIBLE :
		work->crtrgctrl = 0 ;
		work->crtrgctrl_sub = 0 ;
		work->amo_visible = 0x1 | 0x2 ;
		break ;		
	case TRIG_RGB_AMOGRIP_INVISIBLE :
		work->amo_visible &= ~0x1 ;
		break ;
	case TRIG_FALL :
		cancel = 1 ;
		work->crtrgctrl = 0 ;
		work->crtrgctrl_sub = 0 ;
		work->amo_visible = 0 ;
		break ;
    default :
		break ;
    }

	switch ( this ) {
	case THIS_IS_RGB :
		if ( work->motion_set == 0 ) {
			GM_ActMotion( &work->weapon ) ;
			GM_ActMotion( &work->weapon_sub ) ;
			DG_SetPos( root ) ;
			GM_ActObject2( &work->weapon ) ;
			GM_ActObject2( &work->weapon_sub ) ;
			if ( MT_CHECK_END( work->weapon.m_ctrl, 0 ) || cancel ) {
				SetMotion( work ) ;
			}
		} else {
			WeaponEfAct( &( work->bctrl ) ) ;
			WeaponEfAct( &( work->bctrl_sub ) ) ;
		}
		/* マガジンの不可視 */
		if ( GM_CheckPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ) {
			work->crtrgctrl = 0 ;
			work->crtrgctrl_sub = 0 ;
		}
		/* 主観でないときは、主観カートリッジは不可視 */
		if ( !( GM_PlayerBody->objs->flag & vflag ) ) { 
			work->crtrgctrl_sub = 0 ;
		}
		/* マガジン付随モデルの制御 */
		if ( work->motion_set == 1 || 
			 work->amo_visible == 0 ||
			 GM_CheckPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ) {
			if ( work->amo_grip != NULL ) DG_InvisibleObjs( work->amo_grip ) ;
			if ( work->amo_mounted != NULL ) DG_InvisibleObjs( work->amo_mounted ) ;
			break ;
		}		
		if ( work->amo_visible & 0x1 ) {
			/* グリップ */
			if ( work->amo_grip != NULL ) {
				DG_VisibleObjsChanl( work->amo_grip, 0 ) ;
				work->amo_grip->root = &( *work->body )->objs->objs[ HUMAN21_HIDARI_TE ].world ;
			}
		} else {
			if ( work->amo_grip != NULL ) {
				DG_InvisibleObjs( work->amo_grip ) ;
			}
		}
		if ( work->amo_visible & 0x2 ) {
			/* グリップなしマガジン */
			if ( work->amo_mounted != NULL ) {
				DG_VisibleObjsChanl( work->amo_mounted, 0 ) ;
				if ( work->weapon.objs->flag & DG_FLAG_INVISIBLE0 ) {
					/* 主観 */
					work->amo_mounted->root = &work->weapon_sub.objs->objs[ 2 ].world ;
				} else {
					/* 客観 */
					work->amo_mounted->root = &work->weapon.objs->objs[ 2 ].world ;
				}
			}
		} else {
			if ( work->amo_mounted != NULL ) {
				DG_InvisibleObjs( work->amo_mounted ) ;
			}
		}
		break;
	}
}


static	void	Die( work )
Work		*work ;
{
    if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
    }
	if ( work->amo_grip != NULL ) {
		DG_DequeueObjs( work->amo_grip ) ;
		DG_FreeObjs( work->amo_grip ) ;
	}
	if ( work->amo_mounted != NULL ) {
		DG_DequeueObjs( work->amo_mounted ) ;
		DG_FreeObjs( work->amo_mounted ) ;
	}
    GM_FreeObject( &( work->weapon ) ) ;
    GM_FreeObject( &( work->weapon_sub ) ) ;
    GM_PlayerWeaponBody = NULL ;
    GM_PlayerSubWeaponBody = NULL ;
	if ( GM_Weapon != WP_None &&
		 GM_Weapon != WpNum[ work->this ] ) GM_Magazine = 0 ;
}

/*------------------------------------------------------------------*/

static	int	GetResources( work, body, unit, this )
Work		*work ;
OBJECT		**body ;
int		*unit ;
int		this ;
{
    OBJECT	*weapon ;

    weapon = &( work->weapon ) ;
	switch( this ) {
	case THIS_IS_NKT :
	    GM_InitObject( weapon, Bodys[ this * 2 ], BODY_FLAG ) ;
		if ( weapon->objs == NULL ) return -1 ;
		if ( !GM_CheckPlayerStatus( PLAYER_SNAKE ) ) { /*スネークでなければ*/
			GM_ConfigObjectRoot( weapon, *body, *unit ) ;
		}
	    weapon = &( work->weapon_sub ) ;
	    GM_InitObject( weapon, Bodys[ this * 2 + 1 ], BODY_FLAG_SUB ) ;
		if ( weapon->objs == NULL ) return -1 ;
	    GM_ConfigObjectRoot( weapon, *body, *unit ) ;

		break;
	case THIS_IS_RGB :
		WeaponEfInitObject( &( work->bctrl ), &( work->weapon ), *body, *unit, 
						   WpNum[ this ], Bodys[ this * 2 ] ) ;
		WeaponEfInitObject( &( work->bctrl_sub ), &( work->weapon_sub ), *body, *unit, 
						   WpNum[ this ], Bodys[ this * 2 + 1 ] ) ;
		if ( work->weapon.objs == NULL ) return -1 ;
		if ( work->weapon_sub.objs == NULL ) return -1 ;
		GM_ConfigObjectMotion( &work->weapon, 1, MOTION_NAME, MT_FLAG_HUMAN2 ) ;
		GM_ConfigObjectMotion( &work->weapon_sub, 1, MOTION_NAME, MT_FLAG_HUMAN2 ) ;
		SetMotion( work ) ;
		// マガジン by koba4 2001/07/11
		work->crtrgctrl = 1 ;
		work->crtrgctrl_sub = 1 ;
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl, 
													 GM_PlayerBody, &work->weapon, 10 ) ) ;
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl_sub, 
													 GM_PlayerArmBody, &work->weapon_sub, 10 ) ) ;
		work->crtrgctrl = 0 ;
		work->crtrgctrl_sub = 0 ;
		
		/* 装填弾用モデル */
		{
			DG_DEF			*def ;
			DG_OBJS			*objs ;

			def = ( DG_DEF * )GV_GetCache( GV_CacheID( RGB_AMO_GRIP, 'k' ) ) ;
			ASSERT( def != NULL ) ;
			work->amo_grip = objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
			if ( objs == NULL ) return -1 ;
			objs->root = &( *body )->objs->objs[ HUMAN21_HIDARI_TE ].world ;
			objs->light = GM_PlayerBody->objs->light ;
			objs->group_id = GM_PlayerBody->objs->group_id ;
			DG_QueueObjs( objs ) ;
			DG_InvisibleObjs( objs ) ;

			def = ( DG_DEF * )GV_GetCache( GV_CacheID( RGB_AMO_MOUNTED, 'k' ) ) ;
			ASSERT( def != NULL ) ;
			work->amo_mounted = objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
			if ( objs == NULL ) return -1 ;
			objs->root = &work->weapon.objs->objs[ 2 ].world ;
			objs->light = GM_PlayerBody->objs->light ;
			objs->group_id = GM_PlayerBody->objs->group_id ;
			DG_QueueObjs( objs ) ;
			DG_InvisibleObjs( objs ) ;

			work->amo_visible = 0 ;
		}
		break;
	}

	GM_PlayerWeaponModel = Bodys[ this * 2 ] ;
	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon_sub ;

    DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
    work->camera = GM_PlayerSubjectCamera[ GM_CurrentCameraChanl ] ;    

	if ( this == THIS_IS_NKT ){
		// サイトの起動
		GV_SetActorChild( work , NewNikitaSight() );
	}

	DG_InvisibleObjs( work->weapon_sub.objs ) ;
    return 0 ;
}

/*------------------------------------------------------------------*/

static	void	*New( ctrl, body, unit, trigger, side, this )
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
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->side = side ;
		work->chanl = GM_CurrentCameraChanl ;
		work->vflag = ( DG_FLAG_INVISIBLE0 << work->chanl ) ;
		work->this = this ;
		work->count = 0 ;
		if ( GetResources( work, body, unit, this ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		if ( this == THIS_IS_NKT ) {
			GM_MagazineMax = 0 ;
			GM_Magazine = 0 ;
		} else {
			PL_SetMagazine( WP_Rgb6, 6 ) ;
		}
    }
    return work ;
}

/* ニキータ */
void		*NewNikita( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
	GM_SetSightStatus( SGT_Nikita ) ;
    return New( ctrl, body, unit, trigger, side, THIS_IS_NKT ) ;
}

/* ＲＧＢ－６ */
void		*NewRGB6( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
    return New( ctrl, body, unit, trigger, side, THIS_IS_RGB ) ;
}

/*----------------------------------------------------------------*/

/* シナリオで時間せってい */
int		NewSetNikitaTimer( void )
{
	GM_NikitaLifeMax = DIRECT_TICK( GCL_GetOptionValue( 't', 1200 ) ) ;
	return 0 ;
}
