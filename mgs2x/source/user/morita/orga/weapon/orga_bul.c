//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_bul.c
   銃の弾

   1999/07/19 M.Sonoyama
   2000/02/08 T.Morita
   $Id: orga_bul.c,v 1.1.1.3 2002/11/19 11:46:27 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"


#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | TARGET_CHILD)
#define	TARGET_CLASS2	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN)

#define ORG_BUL_PRIM_FLG  (DG_PRIM2_LINE        |\
			   DG_PRIM2_SHADE       |\
			   DG_PRIM2_ANTIALIASING|\
			   DG_PRIM2_TEX         |\
			   DG_PRIM2_ALPHA)
#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )

/*------------------------------------------------------------*/

extern	void	NewWallScar( FMATRIX *, HZX_SEG *seg, HZX_FLR *flr ) ;
extern	void	NewSpark( FMATRIX * ) ;

/*------------------------------------------------------------*/

typedef struct	{
    GV_ACT		actor ;
    CONTROL		control ;

    TARGET		attack ;
    POWER_TARGET	power ;

    FVECTOR		to ;
    FMATRIX		world ;

    DG_PRIM2		*prim ;
    u_short		type ;
    u_short		time ;
    u_short             time2 ;
    u_short		flag ;

    HZX_FLR		seg ;
    u_int		atr ;
    u_int		seg_flag ;
} Work ;


/*------------------------------------------------------------*/


/* 弾痕 */
static void CallScar( Work *work )
{
    HZX_SEG	*seg ;
    HZX_FLR	*flr ;
    FMATRIX	mat ;
    FVECTOR	pole ;
    int		flag, seNo ;

    flag = work->seg_flag ;
    if ( !(work->type & BUL_TYPE_SCAR) ||flag == 0 )
	return ;
    seg = ( HZX_SEG * )( &work->seg ) ;
    mat = work->world ;
    mat.m[ 3 ][ 0 ] = work->to.vx ;
    mat.m[ 3 ][ 1 ] = work->to.vy ;
    mat.m[ 3 ][ 2 ] = work->to.vz ;
    
    if ( flag == HZX_TYPE_FLOOR ) {	/* 床 */
	if ( work->atr & HZX_FLOOR_NO_BULLETHOLE ) return ;
	flr = ( HZX_FLR * )seg ;
	pole.vx = flr->p1.h ;
	pole.vy = flr->p3.h ;
	pole.vz = flr->p2.h ;
	seg = NULL ;
    } else {			/* 壁 */
	if ( work->atr & HZX_SEG_NO_BULLETHOLE ) return ;
	pole.vx = seg->p2.z - seg->p1.z ;
	pole.vy = 0.0F ;
	pole.vz = seg->p1.x - seg->p2.x ;
	flr = NULL ;
    }
    seNo = HZX_GetSeCode( work->atr ) ;
    GM_CallScar( seNo, &mat, seg, flr ) ;
}

/* 跳弾 */
static void CallSpark( Work *work )
{
    HZX_SEG	*seg ;
    HZX_FLR	*flr ;
    FMATRIX	mat ;
    FVECTOR	pole ;
    int		flag, seNo ;

    flag = work->seg_flag ;
    if ( !(work->type & BUL_TYPE_SPARK) || flag == 0 )
	return ;
    seg = ( HZX_SEG * )( &work->seg ) ;
    mat = work->world ;
    mat.m[ 3 ][ 0 ] = work->to.vx ;
    mat.m[ 3 ][ 1 ] = work->to.vy ;
    mat.m[ 3 ][ 2 ] = work->to.vz ;

    if ( flag == 2 ) {	/* 床 */
	if ( work->atr & HZX_FLOOR_NO_RECOIL ) return ;
	flr = ( HZX_FLR * )seg ;
	pole.vx = flr->p1.h ;
	pole.vy = flr->p3.h ;
	pole.vz = flr->p2.h ;
    } else {			/* 壁 */
	if ( work->atr & HZX_SEG_NO_RECOIL ) return ;
	pole.vx = seg->p2.z - seg->p1.z ;
	pole.vy = 0.0F ;
	pole.vz = seg->p1.x - seg->p2.x ;
    }
    _sceVu0Normalize( &pole, &pole ) ;

    seNo = HZX_GetSeCode( work->atr ) ;
    GM_CallSpark( seNo, &mat, &pole ) ;
}

static void Act( Work *work )
{
    FVECTOR from, to, *pos ;

    /* 当たったので終了 */
    if ( work->flag == 1 )
    {
	/* 跳弾エフェクト等に渡すマップ */
	GM_SetCurrentMap( GM_CurrentStageMap ) ;
	CallSpark( work ) ;
	CallScar( work ) ;
	GV_DestroyActor( work ) ;
	return ;
    }

    from = work->control.mov ;
    GM_ActControl( &work->control ) ;
    to   = work->control.mov ;
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
	DG_SwitchBuffPrim2( work->prim ) ;
	pos = work->prim->pos[ work->prim->buffer_clock ] ;
	pos[1] = to   ;
        if ( ++work->time2 < 2 )
	    pos[0] = from ;
	else
	    _sceVu0ScaleVector( &pos[0],  &work->control.step, -2.0f ),
		_sceVu0AddVector( &pos[0], &pos[0], &pos[1] ) ;
	if ( work->flag == 1 )
	{
	    DG_TEX *t = DG_GetTexture( GV_StrCode( "plasma_msk" ) ) ;
	    DG_PRIM2_UVRGB *uvs = work->prim->uvrgb[ work->prim->buffer_clock ] ;

	    uvs[1].v = FTOI12( 0.5f * t->v_scale + t->v_offset ) ;
	}
    }
}

static void Die( Work *work )
{
    if ( work->prim )
	GM_FreePrim2( work->prim ) ;
    GM_FreeControl( &work->control ) ;
}

/*------------------------------------------------------------*/

static int InitControl( Work *work, u_int length, u_int speed, u_int size )
{
    float    len   ;
    CONTROL *ctrl  ;
    FVECTOR  to    ;
    FMATRIX *world ;
    int	     type  ;

    type  = work->type ;
    world = &work->world ;
    ctrl  = &work->control ; 
    if ( GM_InitControl( ctrl, 0, GM_CurrentStageMap ) < 0 )
	return -1 ;
    ctrl->mov.vx = world->m[ 3 ][ 0 ] ;
    ctrl->mov.vy = world->m[ 3 ][ 1 ] ;
    ctrl->mov.vz = world->m[ 3 ][ 2 ] ;
    ctrl->rot = ctrl->turn = DG_ZeroSVector ;

    if ( !( type & BUL_TYPE_NOT_XROT ) )
    {
	/* ステップ値を計算 */
	to.vx = to.vz = 0.0F ;
	to.vy = -( float )speed ; 
	DG_SetPos( world ) ;
	DG_RotVector( &to, &to, 1 ) ;
	GV_LenVec3F( &to, &ctrl->step, GV_VecLen3F( &to ), ( float )speed ) ;
	/* 到達点を予め計算 */
	to.vx = to.vz = 0.0F ;
	to.vy = -( float )length ; 
	DG_PutVector( &to, &to, 1 ) ;
    }
    else
    {
	/* ステップ値を計算 */
	to.vx = to.vy = 0.0F ;
	to.vz = ( float )speed ; 
	DG_SetPos( world ) ;
	DG_RotVector( &to, &to, 1 ) ;
	GV_LenVec3F( &to, &ctrl->step, GV_VecLen3F( &to ), ( float )speed ) ;
	/* 到達点を予め計算 */
	to.vx = to.vy = 0.0F ;
	to.vz = ( float )length ; 
	DG_PutVector( &to, &to, 1 ) ;
    }

    if ( ( work->seg_flag = HZX_OnlineHazardCheck( ctrl->hzx_id, &ctrl->mov, &to, 
						   HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR, 
						   HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
						   HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) )
    {
	HZX_GetOnlineVector( &to ) ;
	len = GV_VecLen3F( &to ) ;
	HZX_GetOnlinePoint( &( work->to ) ) ;
	HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
    }
    else
    {
	len = ( float )length ;
	work->to = to ;
	work->atr = 0 ;
    }

    work->time = ( int )( len / ( float)speed ) + 1 ;
    /* 動的ハザードのみをチェックする */
    GM_ConfigControlHazard( ctrl, size, size, size * 2 ) ;
    ctrl->hzx_check_type = HZX_CHK_D_SEGMENT | HZX_CHK_D_FLOOR ;
    ctrl->seg_flag |= HZX_SEG_NO_BULLET ;
    ctrl->flr_flag |= HZX_FLOOR_NO_BULLET ;
    /* onlineチェックのみ行う */
    ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;

    return 0 ;
}

static void BulletTargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    Work *work = (Work *)ptr ;
    DG_PRIM2_UVRGB *uvs ;
    FVECTOR        *pos ;
    DG_TEX         *t ;

    if ( work->type & BUL_TYPE_VISIBLE )
    {
	t = DG_GetTexture( GV_StrCode( "plasma_msk" ) ) ;
	pos = work->prim->pos[ work->prim->buffer_clock ] ;
	uvs = work->prim->uvrgb[ work->prim->buffer_clock ] ;
	pos[1] = off->hit ;
	uvs[1].v = FTOI12( 0.5f * t->v_scale + t->v_offset ) ;
    }
    GV_DestroyActor( work ) ;
}
static int InitTarget( Work *work, u_int side, u_int size, u_int damage, u_int weapon )
{
    TARGET	 *t = &work->attack ;
    POWER_TARGET *p = &work->power  ;
    FVECTOR	  vec ;

    vec.vx = vec.vy = vec.vz = ( float )size ;
    GM_SetTarget( t, TARGET_CLASS, GM_CurrentStageMap, side, &vec, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( t, I64(1) << weapon ) ;
    GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, damage, &work->control.step ) ;
    GM_SetTargetCallBack( t, BulletTargetCallBack, work ) ;

    return 0 ;
}

static int InitPrimitive( Work *work )
{
    DG_PRIM2       *prim ;
    DG_PRIM2_UVRGB *uv0, *uv1 ;
    DG_TEX         *t ;

    if ( !(prim = work->prim = GM_MakePrim2( ORG_BUL_PRIM_FLG, 1, 2 ) ) )
	return -1 ;
    DG_ConfigPrim2Tex( prim, t=DG_GetTexture( GV_StrCode( "plasma_msk" ) ) ) ;
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA(0,2,0,1,0) ) ;
    uv0 = prim->uvrgb[0] ;
    uv1 = prim->uvrgb[1] ;
    uv0[0].u = uv1[0].u = FTOI12( 0.5f * t->u_scale + t->u_offset ) ;
    uv0[1].u = uv1[1].u = FTOI12( 0.5f * t->u_scale + t->u_offset ) ;
    uv0[0].v = uv1[0].v = FTOI12( 1.0f * t->v_scale + t->v_offset ) ;
    uv0[1].v = uv1[1].v = FTOI12( 0.0f * t->v_scale + t->v_offset ) ;
    uv0[0].r = uv1[0].r = uv0[1].r = uv1[1].r = 196 ;
    uv0[0].g = uv1[0].g = uv0[1].g = uv1[1].g = 128 ;
    uv0[0].b = uv1[0].b = uv0[1].b = uv1[1].b =  64 ;
    uv0[0].a = uv1[0].a = 128 ;
    uv0[1].a = uv1[1].a = 16 ;
    uv0[0].q = uv1[0].q = uv0[1].q = uv1[1].q = 4096 ;
    uv0[0].f = uv1[0].f = 0x8fff ;
    uv0[1].f = uv1[1].f = 0x0fff ;
    _sceVu0CopyVector( &prim->pos[0][0], &work->control.mov ) ;
    _sceVu0CopyVector( &prim->pos[0][1], &work->control.mov ) ;
    _sceVu0CopyVector( &prim->pos[1][0], &work->control.mov ) ;
    _sceVu0CopyVector( &prim->pos[1][1], &work->control.mov ) ;

    return 0 ;
}

/*------------------------------------------------------------*/

static int GetResources( Work *work, FMATRIX *world, u_int type, u_int side, u_int size,
			 u_int damage, u_int length, u_int speed, u_int weapon )
{
    work->world = *world ;
    work->type  = type   ;
    if ( InitControl( work, length, speed, size ) < 0 )
	return -1 ;
    if ( InitTarget( work, side, size, damage, weapon ) < 0 )
	return -1 ;
    if ( type & BUL_TYPE_VISIBLE )
	if ( InitPrimitive( work ) )
	    return -1 ;
    return 0 ;
}

/* 弾起動 */
void	*NewOrgaBullet( FMATRIX *world, u_int type, u_int side, u_int size, 
			u_int damage, u_int length, u_int speed, int weapon )
{
    Work *work ;

    work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
    if ( work != NULL )
    {
	GV_SetActor( &work->actor, Act, Die ) ;
	if ( GetResources( work, world, type, side, size, damage, length, speed, weapon ) < 0 )
	{
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }
    return work ;
}
