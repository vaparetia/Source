//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_bul.c
   銃の弾

   1999/07/19 M.Sonoyama
   2000/06/14 T.Morita revised
   $Id: orga_bul2.c,v 1.1.1.3 2002/11/19 11:46:28 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"


enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | TARGET_CHILD)

#define	PRIM_FLAG	(DG_PRIM2_LINE | DG_PRIM2_SHADE | DG_PRIM2_ANTIALIASING)

/*------------------------------------------------------------*/

extern	void	NewWallScar( FMATRIX *, HZX_SEG *seg, HZX_FLR *flr ) ;
extern	void	NewSpark( FMATRIX * ) ;

/*------------------------------------------------------------*/

typedef struct	{
    GV_ACT	 actor ;
    CONTROL	 control ;
		 
    TARGET	 attack ;
    POWER_TARGET power ;
		 
    FVECTOR	 from ;
    FVECTOR	 to ;
		 
    DG_PRIM2	*prim ;
    u_short	 type ;
    u_short	 time ;
    u_short	 time2 ;    
    u_short	 flag ;
		 
    HZX_FLR	 seg ;
    u_int	 atr ;
    u_int	 seg_flag ;
} Work ;


/*------------------------------------------------------------*/

static void Hit( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;

    if ( work->type & BUL_TYPE_VISIBLE )
	work->prim->pos[work->prim->buffer_clock][1] = off->hit ;
    GV_DestroyActor( work ) ;
}

static void CallHitEffect( Work *work )
{
    FMATRIX	mat ;
    FVECTOR	pole ;

    if ( work->atr ^ (HZX_FLOOR_NO_BULLETHOLE| HZX_FLOOR_NO_RECOIL) ||
	 !(work->type & (BUL_TYPE_SCAR| BUL_TYPE_SPARK)) ||
	 work->seg_flag == 0 )
	return ;

    _sceVu0UnitMatrix( &mat ) ;
    _sceVu0CopyVector( (FVECTOR *)&mat.m[W], &work->from ) ;
    _sceVu0SubVector ( (FVECTOR *)&mat.m[Y], &work->from, &work->to ) ;
    _sceVu0Normalize ( (FVECTOR *)&mat.m[Y], (FVECTOR*)&mat.m[Y] ) ;

    if ( work->seg_flag == HZX_TYPE_FLOOR )
    {	/* 床 */
	HZX_FLR *flr = (HZX_FLR *)&work->seg ;
	pole.vx = flr->p1.h ;
	pole.vy = flr->p3.h ;
	pole.vz = flr->p2.h ;
	if ( !(work->atr & HZX_FLOOR_NO_BULLETHOLE) && work->type & BUL_TYPE_SCAR )
	     NewWallScar( &mat, NULL, flr ) ;
    }
    else
    {			/* 壁 */
	HZX_SEG *seg = (HZX_SEG *)&work->seg ;
	pole.vx = seg->p2.z - seg->p1.z ;
	pole.vy = 0.0F ;
	pole.vz = seg->p1.x - seg->p2.x ;
	if ( !(work->atr & HZX_FLOOR_NO_BULLETHOLE) && work->type & BUL_TYPE_SCAR )
	    NewWallScar( &mat, seg, NULL ) ;
    }
    if ( !(work->atr & HZX_FLOOR_NO_BULLETHOLE) && work->type & BUL_TYPE_SPARK )
    {
	_sceVu0Normalize( &pole, &pole ) ;
	DG_ReflectMatrix( &pole, &mat, &mat ) ;
	GM_SeSetMode( SD_W_RICOCH02, &work->to, GM_SEMODE_BOMB ) ;
	NewSpark( &mat ) ;
    }
}

static void Act( Work *work )
{
    FVECTOR	from, to, *pos ;

    /* 当たったので終了 */
    if ( work->flag == 1 )
    {
	/* 跳弾エフェクト等に渡すマップ */
	GM_SetCurrentMap( GM_CurrentStageMap ) ;
	CallHitEffect( work ) ;
	GV_DestroyActor( work ) ;
	return ;
    }

    from = work->control.mov ;
    GM_ActControl( &work->control ) ;
    to = work->control.mov ;
    GM_MoveOnlineTarget( &work->attack, &from, &to ) ;
    GM_PutTarget( &work->attack ) ;

    if ( work->control.n_touches > 0 )
    {
	/* 動的ハザードに当たった */
	HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
	HZX_GetOnlinePoint( &to ) ;
	work->seg_flag = HZX_GetOnlineHazardType() ;
	work->flag = 1 ;
    }
    else if ( --work->time <= 0 )
    {
	/* 最長到達点 */
	to = work->to ;
	work->flag = 1 ;
    }
    if ( work->type & BUL_TYPE_VISIBLE )
    {
	if ( work->time2 < 2 )
	{
	    if ( ++work->time2 == 2 )
		DG_VisiblePrim2( work->prim ) ;
	}
	DG_SwitchBuffPrim2( work->prim ) ;
	pos = work->prim->pos[work->prim->buffer_clock] ;
	pos[0] = from ;
	pos[1] = to   ;
    }
}

static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
}

/*------------------------------------------------------------*/

static	int	InitControl( Work *work, float speed, u_int size )
{
    FVECTOR  v ;
    CONTROL *ctrl = &work->control ; 

    if ( GM_InitControl( ctrl, 0, GM_CurrentStageMap ) < 0 )
	return -1 ;
    ctrl->mov = work->from ;
    ctrl->rot = ctrl->turn = DG_ZeroSVector ;

    if ( ( work->seg_flag = HZX_OnlineHazardCheck( ctrl->hzx_id, &work->from, &work->to, 
						   HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR, 
						   HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
						   HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) )
    {
	HZX_GetOnlinePoint( &work->to ) ;
	HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
    }
    else
	work->atr = 0 ;

    /* ステップ値を計算 */
    _sceVu0SubVector( &v, &work->to, &work->from ) ;
    _sceVu0Normalize( &ctrl->step, &v ) ;
    _sceVu0ScaleVector( &ctrl->step, &ctrl->step, speed ) ;

    work->time = ( int )( GV_VecLen3F( &v ) / ( float)speed ) + 1 ;

    /* 動的ハザードのみをチェックする */
    GM_ConfigControlHazard( ctrl, size, size, size * 2 ) ;
    ctrl->hzx_check_type = HZX_CHK_D_SEGMENT | HZX_CHK_D_FLOOR ;

    /* onlineチェックのみ行う */
    ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;

    return 0 ;
}

static int InitTarget( Work  *work, u_int side, u_int size, u_int damage, u_int weapon )
{
    TARGET  *t = &work->attack ;
    FVECTOR  vec ;

    vec.vx = vec.vy = vec.vz = (float)size ;

    GM_SetTarget( t, TARGET_CLASS, GM_CurrentStageMap, side, &vec, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, I64(1) << weapon ) ;
    GM_SetPowerTarget( t, &work->power, POWER_ONCE, 255, 0, damage, &work->control.step ) ;
    GM_SetTargetCallBack( t, Hit, work ) ;

    return 0 ;
}

static int InitPrim( Work *work )
{
    DG_PRIM2	   *prim ;
    DG_PRIM2_UVRGB *uvrgb ;
    FVECTOR	   *pos ;
    int		    i ;

    if ( !(prim = work->prim = GM_MakePrim2( DG_PRIM2_LINE| DG_PRIM2_SHADE| DG_PRIM2_ANTIALIASING,
					     1, 2 ) ) )
	return -1 ;
    prim->tex_trans.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ;
    for ( i=0 ; i<2 ; i++ )
    {
	uvrgb = prim->uvrgb[ i ] ;
	pos = prim->pos[ i ] ;
	pos[ 0 ] = pos[ 1 ] = DG_ZeroVector ;
	uvrgb[ 0 ].r = uvrgb[ 1 ].r = 196 ;
	uvrgb[ 0 ].g = uvrgb[ 1 ].g = 128 ;
	uvrgb[ 0 ].b = uvrgb[ 1 ].b = 64 ;
	uvrgb[ 0 ].a = uvrgb[ 1 ].a = 128 ;
	uvrgb[ 0 ].q = uvrgb[ 1 ].q = 4096 ;
	uvrgb[ 0 ].f = 0x8fff ;
	uvrgb[ 1 ].f = 0x0fff ;
    }
    DG_InvisiblePrim2( prim ) ;

    return 0 ;
}

/*------------------------------------------------------------*/

static int GetResources( Work *work, FVECTOR *from, FVECTOR *to,
			 u_int type, u_int side, u_int size, u_int damage, u_int speed, int weapon )
{
    work->from = *from ;
    work->to   = *to  ;
    work->type = type ;
    if ( InitControl( work, (float)speed, size ) < 0 )
	return -1 ;
    if ( InitTarget( work, side, size, damage, weapon ) < 0 )
	return -1 ;
    if ( type & BUL_TYPE_VISIBLE )
	InitPrim( work ) ; /* 表示はでなくても弾は出る */

    return 0 ;
}

/* 弾起動 */
void *NewOrgaBullet2( FVECTOR *from, FVECTOR *to,
		      u_int type,   u_int side,   u_int size,
		      u_int damage, u_int speed, int weapon )
{
    Work	*work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	if ( GetResources( work, from, to, type, side, size, damage, speed, weapon ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
