//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   book.c
   雑誌

   2001/05/17	M.Sonoyama
   $Id: book.c,v 1.1.1.3 2002/11/19 11:50:26 Yoshizawa1 Exp $
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


/*------------------------------------------------------------------*/

#define	BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

//static FVECTOR	Shift = { 113.0F, -226.0F, -2.0F } ;
//static SVECTOR  Rot = { 1024, 0, 30 * 1024 / 90 } ;
//static FVECTOR	Shift = { 63.0F, -266.0F, -2.0F } ;
static FVECTOR	Shift = { -4.0F, -277.0F, 72.0F } ;
static SVECTOR  Rot = { 1024, 0, 45 * 1024 / 90 } ;

/* Changed by Morita 2001.06.15 */
extern	void	*NewBulletBook( OBJECT *pbody, SVECTOR *rot ) ;

/*------------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX			actor ;
	OBJECT				weapon ;
	OBJECT				weapon_sub ;
	CONTROL				*ctrl ;
	OBJECT				**body ;
	int					*unit ;
	PL_StickWPWork		*trigger ;
	SVECTOR				open_rot1 ;
	SVECTOR				open_rot2 ;
	int					count ;
	int					flag ;
} Work ;

/*------------------------------------------------------------------*/

static	void	UpdatePos( Work *work, FMATRIX *pWorld, FVECTOR *shift, SVECTOR *rot )
{
	FVECTOR		mov ;
	FMATRIX		world, world1, world2 ;
	OBJECT		*weapon ;
	
	DG_SetPos( pWorld ) ;
	//DG_SetPos( &( ( *work->body )->objs->objs[ *work->unit ].world ) ) ;
	DG_PutVector( shift, &mov, 1 ) ;
	DG_RotatePos( rot ) ;
	DG_GetPos( &world ) ;
	GV_VecToMat( &mov, &world ) ;
	DG_SetPos2( &DG_ZeroVector, &work->open_rot1 ) ;
	DG_GetPos( &world1 ) ;
	_sceVu0MulMatrix( &world1, &world, &world1 ) ;
	DG_SetPos2( &DG_ZeroVector, &work->open_rot2 ) ;
	DG_GetPos( &world2 ) ;
	_sceVu0MulMatrix( &world2, &world, &world2 ) ;

	weapon = &work->weapon ;
	weapon->map_name = ( *work->body )->map_name ;
	GM_GroupObjs( weapon->objs, ( *work->body )->map_name ) ;
	DG_COPY_MAT( &weapon->objs->world, &world ) ;
	DG_COPY_MAT( &weapon->objs->objs[ 0 ].world, &world1 ) ;
	DG_COPY_MAT( &weapon->objs->objs[ 1 ].world, &world2 ) ;

	weapon = &work->weapon_sub ;
	weapon->map_name = ( *work->body )->map_name ;
	GM_GroupObjs( weapon->objs, ( *work->body )->map_name ) ;
	DG_COPY_MAT( &weapon->objs->world, &world ) ;
	DG_COPY_MAT( &weapon->objs->objs[ 0 ].world, &world1 ) ;
	DG_COPY_MAT( &weapon->objs->objs[ 1 ].world, &world2 ) ;	
}

static	int	GetGRot( HZX_HZD *floor, FVECTOR *mov, float height, int ry )
{
    FVECTOR	head, vec ;
    SVECTOR	rot ;
    float	hh, diff ;
    int		turn ;	

    if ( ( floor == NULL ) ||
		 ( floor->attribute & HZX_FLOOR_FLAT ) ) return 0 ;
    rot.vx = 0 ; rot.vy = ry ; rot.vz = 0 ;
    DG_SetPos2( mov, &rot ) ;
    head.vx = head.vy = 0.0F ;
    head.vz = 500.0F ; 
    DG_PutVector( &head, &head, 1 ) ;
    HZX_SlopeFloorLevel( &hh, &head, floor ) ;
    diff = hh - height ;
    vec.vx = diff ; 
    vec.vz = 500.0F ;
    turn = - GV_VecDir2( &vec ) ;
    if ( turn < -2048 ) turn += 4096 ;
    return turn ;
}

/*------------------------------------------------------------------*/

static	void	Act( Work *work )
{
	int			trg ;
	
	if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
		DG_InvisibleObjsChanl( work->weapon.objs, 0 ) ;
		DG_VisibleObjsChanl( work->weapon.objs, 1 ) ;

		DG_VisibleObjsChanl( work->weapon_sub.objs, 0 ) ;
	} else {
		DG_VisibleObjsChanl( work->weapon.objs, 0 ) ;
		DG_VisibleObjsChanl( work->weapon.objs, 1 ) ;
		DG_InvisibleObjsChanl( work->weapon_sub.objs, 0 ) ;
	}
#if 0
	if ( 0 ) {
		GV_PAD	*pad ;

		pad = GV_PadData + 1 ;
		if ( pad->status & PAD_U ) Shift.vx += 1.0F ;
		if ( pad->status & PAD_D ) Shift.vx -= 1.0F ;
		if ( pad->status & PAD_L ) Shift.vy += 1.0F ;
		if ( pad->status & PAD_R ) Shift.vy -= 1.0F ;
		if ( pad->status & PAD_A ) Shift.vz += 1.0F ;
		if ( pad->status & PAD_B ) Shift.vz -= 1.0F ;
		MENU_Printf( "%.0f %.0f %.0f\n",
					 Shift.vx, Shift.vy, Shift.vz ) ;
	}
#endif
	UpdatePos( work, &GM_PlayerBody->objs->objs[ *work->unit ].world, &Shift, &Rot ) ;

	if ( work->count > 0 ) {
		-- work->count ;
		DG_InvisibleObjs( work->weapon.objs ) ;		
		DG_InvisibleObjs( work->weapon_sub.objs ) ;		
		return ;
	}
	trg = work->trigger->trigger ;

	if ( trg == TRIG_OPEN_START ) {
		work->flag = 1 ;
	} else {
		work->open_rot1.vy += 192 ;
		if ( work->open_rot1.vy > 0 ) {
			work->flag = 0 ;
			work->open_rot1.vy = 0 ;
		}
	}

	if ( work->flag == 1 ) {
		work->open_rot1.vy -= 96 ;
		work->open_rot2.vy = 0 ;
		if ( work->open_rot1.vy < - 1800 ) work->open_rot1.vy = - 1800 ;
	}

	if ( trg == TRIG_FIRE ) {
		FVECTOR		mov ;
		//SVECTOR		rot ;
		SVECTOR		prot ;/* Added by Morita 2001.06.15 */
		//FMATRIX		world ;
		
		if ( GM_WeaponNum( WP_Book ) <= 0 ) return ;
		if ( GM_N_Books >= GM_BOOK_MAX ) return ;

		/* 地面に置く */
		//DG_COPY_VEC( &mov, &work->ctrl->mov ) ;
		//mov.vy = work->ctrl->levels[ 0 ] + 10.0F ;
		GV_MatToVec( &work->weapon.objs->world, &mov ) ;
		mov.vy += 250.0F ;
		if ( HZX_LevelHazardCheck( work->ctrl->hzx_id, &mov, HZX_CHK_ALL, 0 ) & 1 ) {
			float		levels[ 2 ] ;

			HZX_GetLevelHeight( levels ) ;
			mov.vy = levels[ 0 ] + 10.0F ;
		} 

		/* Changed by Morita 2001.06.15 */
		prot.vy = work->ctrl->rot.vy ;
		prot.vx = GetGRot( &work->trigger->hzd, &mov, mov.vy - 10.0F, prot.vy ) ;
		prot.vz = 0 ;
#if 0		
		DG_SetPos2( &mov, &prot ) ;
		DG_GetPos( &world ) ;
		rot.vx = -1024 ; rot.vy = 2048 ; rot.vz = 0 ;
		work->open_rot1.vy = 1024 + 64 ;
		work->open_rot2.vy = -1024 - 64 ;
		UpdatePos( work, &world, &DG_ZeroVector, &rot ) ;
#endif
		GV_VecToMat( &mov, &work->weapon.objs->world ) ;
		DG_InvisibleObjs( work->weapon.objs ) ;
		DG_InvisibleObjs( work->weapon_sub.objs ) ;

		work->open_rot1.vy = 0 ;
		work->open_rot2.vy = 0 ;
		work->flag = 0 ;
		GM_SeSetMode( SD_W_BOOKPUT1 , &work->ctrl->mov , GM_SEMODE_BOMB );
		NewBulletBook( &work->weapon, &prot ) ;/* Changed by Morita 2001.06.15 */
		work->count = DIRECT_TICK( 40 ) ;
		GM_DecrementWeapon( WP_Book, 1 ) ;
	}
}

static	void	Die( Work *work )
{
	if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
	}
	GM_FreeObject( &work->weapon ) ;
	GM_FreeObject( &work->weapon_sub ) ;
	GM_PlayerWeaponBody = NULL ;
	GM_PlayerSubWeaponBody = NULL ;
}

/*------------------------------------------------------------------*/

static	int		GetResources( Work *work, OBJECT **body, int *unit )
{
	OBJECT		*weapon ;

	weapon = &work->weapon ;
	GM_InitObject( weapon, MDL_BOOK_MSL, BODY_FLAG ) ;
	if ( weapon->objs == NULL ) return -1 ;
	GM_PlayerWeaponModel = MDL_BOOK_MSL ;
	GM_ConfigObjectLight( weapon, ( *body )->objs->light ) ;
	DG_ConnectObjs( GM_PlayerBody->objs, weapon->objs ) ;
	weapon->map_name = ( *body )->map_name ;
	GM_GroupObjs( weapon->objs, ( *body )->map_name ) ;

	weapon = &work->weapon_sub ;
	GM_InitObject( weapon, MDL_BOOK_MSL, BODY_FLAG ) ;	
	if ( weapon->objs == NULL ) return -1 ;
	GM_ConfigObjectLight( weapon, ( *body )->objs->light ) ;
	weapon->map_name = ( *body )->map_name ;
	GM_GroupObjs( weapon->objs, ( *body )->map_name ) ;

	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon_sub ;

	if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
		GM_SetPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ;
	}

	DG_InvisibleObjs( work->weapon_sub.objs ) ;

	UpdatePos( work, &GM_PlayerBody->objs->objs[ *work->unit ].world, &Shift, &Rot ) ;

	return 0 ;
}

/*------------------------------------------------------------------*/

void		*NewBook( CONTROL *ctrl, OBJECT **body, int *unit,
					  u_int *trigger, int side )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = ( PL_StickWPWork * )trigger ;
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work, body, unit ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


