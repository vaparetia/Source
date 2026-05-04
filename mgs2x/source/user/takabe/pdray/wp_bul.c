//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   wp_bul.c
   ＲＡＹ用バルカン（銃の弾を改造したもの）
   
   1999/07/19 M.Sonoyama
   2001/06/06 K.Takabe
   $Id: wp_bul.c,v 1.1.1.3 2002/11/19 11:51:27 Yoshizawa1 Exp $
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

#include	"../other/vec_util.h"

#include "BP_Misc.h"

#ifdef KP_XBOX
#define GM_SeSetMode( _a, _b, _c ) GM_SeSetModeAddr( _a, _b, _c, GM_INVALID_ADDR )
#endif

#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | TARGET_CHILD)
#define	TARGET_CLASS2	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN)

//#define	PRIM_FLAG	(DG_PRIM2_LINE | DG_PRIM2_SHADE | DG_PRIM2_ANTIALIASING)
#define	PRIM_FLAG	(DG_PRIM2_POLY | DG_PRIM2_SHADE | DG_PRIM2_TEX | DG_PRIM2_ALPHA )
//#define TEXTURE		(14071854)		/* "flare_r1_msk" */
//#define TEXTURE		(4458455)		/* "muzzlefrash_01_alp" */
//#define TEXTURE		(9776993)			/* "drop01_alp2_mod1021" */
#define TEXTURE		(3594043)			/* "drop01_msk" */

#define	SET_UVRGB_UV( _p, _u, _v )	{ (_p)->u = (_u) ; (_p)->v = (_v) ; (_p)->q = 4096 ; }
#define	SET_UVRGB_COL( _p, _r, _g, _b, _a )	{ (_p)->r = (_r) ; (_p)->g = (_g) ; (_p)->b = (_b) ; (_p)->a = (_a) ; }

/* 汎用マクロ */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )

/*------------------------------------------------------------*/

typedef struct	{
    GV_ACT		actor ;
    CONTROL		control ;

    TARGET		attack ;
    POWER_TARGET	power ;

    FVECTOR		to ;
	FVECTOR		old_from ;		/* added by K.Takabe 2000/11/17 */
    FMATRIX		world ;

    DG_PRIM2		*prim ;
    u_short		type ;
    u_short		time ;
    u_short		time2 ;    
    u_short		flag ;

    HZX_FLR		seg ;
    u_int		atr ;
    u_int		seg_flag ;
} Work ;


/*------------------------------------------------------------*/
static void CalcPrimPos( FVECTOR *pos, FVECTOR *from, FVECTOR *to, float size )
{
#if 0
	/* カメラから見て必ず厚みがあるように */
	FVECTOR		vec1, vec2 ;
	FVECTOR		vec ;
	GTE_LoadMatrix( &( DG_Chanl( 0 )->eye_pers ) );
	GTE_PutVector1( &vec1, from );
	GTE_PutVector1( &vec2, to );
	vec.vx = vec2.vy * vec1.vw - vec1.vy * vec2.vw ;
	vec.vy = vec1.vx * vec2.vw - vec2.vx * vec1.vw ;
	vec.vz = 0 ;
	vec.vw = 0 ;
	GTE_Normalize( &vec, &vec );
	GTE_LoadMatrix( &( DG_Chanl( 0 )->eye ) );
	GTE_RotVector1( &vec, &vec );
#if 1
	GTE_ScaleVector( &vec, &vec, size );
	GTE_AddVector( pos + 0, to, &vec );
	GTE_SubVector( pos + 1, to, &vec );
	GTE_AddVector( pos + 2, from, &vec );
	GTE_SubVector( pos + 3, from, &vec );
#else
	/* 画面から見て常に一定の太さの場合 */
	GTE_ScaleVector( &vec1, &vec, vec1.vw / 128 );
	GTE_ScaleVector( &vec2, &vec, vec2.vw / 128 );
	GTE_AddVector( pos + 0, to, &vec2 );
	GTE_SubVector( pos + 1, to, &vec2 );
	GTE_AddVector( pos + 2, from, &vec1 );
	GTE_SubVector( pos + 3, from, &vec1 );
#endif
#else

	/* カメラから見て必ず厚みがあるように */
	FVECTOR		vec1, vec2 ;
	FVECTOR		vec ;
	GTE_LoadMatrix( &( DG_Chanl( 0 )->eye_pers ) );
	GTE_PutVector1( &vec1, from );
	GTE_PutVector1( &vec2, to );
	vec.vx = vec2.vy * vec1.vw - vec1.vy * vec2.vw ;
	vec.vy = vec1.vx * vec2.vw - vec2.vx * vec1.vw ;
	vec.vz = 0 ;
	vec.vw = 0 ;
	GTE_Normalize( &vec, &vec );
	vec1.vx = -vec.vy ;
	vec1.vy = vec.vx ;
	vec1.vz = 0 ;
	vec1.vw = 0 ;
	GTE_LoadMatrix( &( DG_Chanl( 0 )->eye ) );
	GTE_RotVector1( &vec, &vec );
	GTE_RotVector1( &vec1, &vec1 );

	GTE_ScaleVector( &vec, &vec, size );
	GTE_ScaleVector( &vec1, &vec1, size );
	GTE_AddVector( pos + 0, to, &vec );
	GTE_SubVector( pos + 1, to, &vec );
	GTE_AddVector( pos + 2, from, &vec );
	GTE_SubVector( pos + 3, from, &vec );
	GTE_AddVector( pos + 0, pos + 0, &vec1 );
	GTE_AddVector( pos + 1, pos + 1, &vec1 );
	GTE_SubVector( pos + 2, pos + 2, &vec1 );
	GTE_SubVector( pos + 3, pos + 3, &vec1 );

#endif
}
/*------------------------------------------------------------*/

static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
    Work	*work ;

    work = ( Work * )ptr ;
    if ( work->type & BUL_TYPE_VISIBLE ) {
		work->prim->pos[ work->prim->buffer_clock ][ 2 ] = off->hit ;
    }
    GV_DestroyActor( work ) ;
}

/* 弾痕 */
static	void	CallScar( Work *work )
{
    HZX_SEG	*seg ;
    HZX_FLR	*flr ;
    FMATRIX	mat ;
    FVECTOR	pole ;
    int		flag, seNo ;

    flag = work->seg_flag ;
    if ( !( work->type & BUL_TYPE_SCAR ) ||
		flag == 0 ) return ;
    seg = ( HZX_SEG * )( &work->seg ) ;
    mat = work->world ;
    mat.m[ 3 ][ 0 ] = work->to.vx ;
    mat.m[ 3 ][ 1 ] = work->to.vy ;
    mat.m[ 3 ][ 2 ] = work->to.vz ;

    if ( flag == 2 ) {			/* 床 */
		if ( work->atr & HZX_FLOOR_NO_BULLETHOLE ) return ;
		flr = ( HZX_FLR * )seg ;
		pole.vx = flr->p1.h ;
		pole.vy = flr->p3.h ;
		pole.vz = flr->p2.h ;
		seg = NULL ;
    } else {					/* 壁 */
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
static	void	CallSpark( Work *work ) 
{
    HZX_SEG	*seg ;
    HZX_FLR	*flr ;
    FMATRIX	mat ;
    FVECTOR	pole ;
    int		flag ;
    int		seNo, noise ;

    flag = work->seg_flag ;
    if ( !( work->type & BUL_TYPE_SPARK ) ||
		flag == 0 ) return ;
    seg = ( HZX_SEG * )( &work->seg ) ;
    mat = work->world ;
    mat.m[ 3 ][ 0 ] = work->to.vx ;
    mat.m[ 3 ][ 1 ] = work->to.vy ;
    mat.m[ 3 ][ 2 ] = work->to.vz ;

    if ( flag == 2 ) {			/* 床 */
		if ( work->atr & HZX_FLOOR_NO_RECOIL ) return ;
		flr = ( HZX_FLR * )seg ;
		pole.vx = flr->p1.h ;
		pole.vy = flr->p3.h ;
		pole.vz = flr->p2.h ;
    } else {					/* 壁 */
		if ( work->atr & HZX_SEG_NO_RECOIL ) return ;
		pole.vx = seg->p2.z - seg->p1.z ;
		pole.vy = 0.0F ;
		pole.vz = seg->p1.x - seg->p2.x ;
    }
    _sceVu0Normalize( &pole, &pole ) ;

    seNo = HZX_GetSeCode( work->atr ) ;
    //GM_CallSpark( seNo, &mat, &pole ) ;
	if ( work->type & BUL_TYPE_NOISE ) {
		noise = NOISE_S ;
		if ( work->type & BUL_TYPE_NOISE_SS ) noise = NOISE_SS ;
		GM_SetNoise( noise, &work->to, work->control.map ) ;
	}
	/*
		BUL_TYPE_SPARK_SMOKEフラグをＶＲ破壊エフェクト禁止フラグとして運用する
	*/
	{
		extern void	*NewRayValcanSpark( FMATRIX *world );
		FMATRIX		world ;
		float		len ;
		world = mat ;
		/* 反射マトリクス生成 */
		len = world.m[0][0] * pole.vx + world.m[0][1] * pole.vy + world.m[0][2] * pole.vz ;
		len *= 2.0f ;
		world.m[0][0] -= len * pole.vx ;
		world.m[0][1] -= len * pole.vy ;
		world.m[0][2] -= len * pole.vz ;
		len = world.m[1][0] * pole.vx + world.m[1][1] * pole.vy + world.m[1][2] * pole.vz ;
		len *= 2.0f ;
		world.m[1][0] -= len * pole.vx ;
		world.m[1][1] -= len * pole.vy ;
		world.m[1][2] -= len * pole.vz ;
		len = world.m[2][0] * pole.vx + world.m[2][1] * pole.vy + world.m[2][2] * pole.vz ;
		len *= 2.0f ;
		world.m[2][0] -= len * pole.vx ;
		world.m[2][1] -= len * pole.vy ;
		world.m[2][2] -= len * pole.vz ;
		
		GTE_ScaleVector( (FVECTOR*)world.m[0], (FVECTOR*)world.m[0], -1.0f );
		GTE_ScaleVector( (FVECTOR*)world.m[1], (FVECTOR*)world.m[1], -1.0f );
		GTE_ScaleVector( (FVECTOR*)world.m[2], (FVECTOR*)world.m[2], -1.0f );

		NewRayValcanSpark( &world );
	}
	/* ＳＥ */
	GM_SeSetMode( SD_E_ARMREB01, &work->to, GM_SEMODE_BOMB ) ;
}

static	void	Act( Work *work )
{
    TARGET	*t ;
    FVECTOR	from, to, *pos ;

    /* 当たったので終了 */
    if ( work->flag != 0 ) {
		if ( ( work->atr & HZX_FLOOR_DYNAMIC ) == 0 ){/* 動的ハザードでなければ */
			if ( !( work->type & BUL_TYPE_SPARK_SMOKE ) ){
				FMATRIX	mat ;
				extern void *NewVRFloorBreak( FMATRIX *parent, float scale );
				mat = DG_UnitMatrix ;
				*(FVECTOR*)mat.m[3] = work->to ;
				mat.m[3][3] = 1.0f ;
				NewVRFloorBreak( &mat, 1.0f );
			}
		}
		/* 跳弾エフェクト等に渡すマップ */
		GM_SetCurrentMap( GM_CurrentStageMap ) ;
		CallSpark( work ) ;
		CallScar( work ) ;
		/* 動的ハザード処理 */
		GM_TargetHitDynamicHazard( &work->seg, &work->attack, work->seg_flag - 1 ) ;
		//if ( work->type & BUL_TYPE_VISIBLE ) DG_InvisiblePrim2( work->prim );	/* あったほうがいい？ */
		GV_DestroyActor( work ) ;	
		return ;
    }

    from = work->control.mov ;
    GM_ActControl( ( CONTROL * )&( work->control ) ) ;

    to = work->control.mov ;
    if ( work->control.n_touches > 0 ) {
		/* 動的ハザードに当たった */
		HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
		HZX_GetOnlinePoint( &( work->to ) ) ;
		work->seg_flag = HZX_GetOnlineHazardType() ;
		work->flag = 1 ;
    } else if (  -- work->time <= 0 ) {
		/* 最長到達点 */
		to = work->to ;
		work->flag = 1 ;
    }

    if ( !( work->type & BUL_TYPE_NO_ATTACK ) ) {
		t = &( work->attack ) ;
		GM_MoveOnlineTarget( t, &from, &to ) ;
		GM_PutTarget( t ) ;
    } 

    if ( work->type & BUL_TYPE_VISIBLE ) {
		from.vw = 1.0f ;
		to.vw = 1.0f ;
#if 1
		/* 弾発射位置と銃口の見た目が違う場合を考慮して軌跡表示開始を遅らせる場合 */
		if ( work->time2 < 2 ) {
			if ( ++ work->time2 == 2 ) DG_VisiblePrim2( work->prim ) ;
			work->old_from = from ;
		}
#else
		/* 弾発射位置と銃口の見た目が同じ場合 */
		if ( work->time2 < 1 ) {
			work->time2++ ;
			work->old_from = from ;
			DG_VisiblePrim2( work->prim );
		}
#endif
		DG_SwitchBuffPrim2( work->prim ) ;
		pos = work->prim->pos[ work->prim->buffer_clock ] ;
		//pos[ 0 ] = work->old_from ;
		//pos[ 1 ] = from ;
		//pos[ 2 ] = to ;
		//CalcPrimPos( pos, &from, &to, 20.0f );
		CalcPrimPos( pos, &work->old_from, &to, 20.0f + RND(5) );
		work->old_from = from ;
    }
}

static	void	Die( Work *work )
{
    GM_FreeControl( ( CONTROL * )&work->control ) ;
    if ( work->type & BUL_TYPE_VISIBLE ) GM_FreePrim2( work->prim ) ;
}

/*------------------------------------------------------------*/

static	int	InitControl( Work *work, u_int length, u_int speed, u_int size )
{
    float	len ;
    CONTROL	*ctrl ;
    FVECTOR	to ;
    FMATRIX	*world ;
    int		type ;
	
    type = work->type ;
    world = &( work->world ) ;
    ctrl = ( CONTROL * )&( work->control ) ; 
    if ( GM_InitControl( ctrl, 0, GM_CurrentStageMap ) < 0 ) return -1 ;
    ctrl->mov.vx = world->m[ 3 ][ 0 ] ;
    ctrl->mov.vy = world->m[ 3 ][ 1 ] ;
    ctrl->mov.vz = world->m[ 3 ][ 2 ] ;
    ctrl->rot = ctrl->turn = DG_ZeroSVector ;
	
    if ( !( type & BUL_TYPE_NOT_XROT ) ) {
		/* ステップ値を計算 */
		to.vx = to.vz = 0.0F ;
		to.vy = -( float )speed ; 
		DG_SetPos( world ) ;
		DG_RotVector( &to, &to, 1 ) ;
		GV_LenVec3F( &to, &( ctrl->step ), GV_VecLen3F( &to ), ( float )speed ) ;
		/* 到達点を予め計算 */
		to.vx = to.vz = 0.0F ;
		to.vy = -( float )length ; 
		DG_PutVector( &to, &to, 1 ) ;
    } else {
		/* ステップ値を計算 */
		to.vx = to.vy = 0.0F ;
		to.vz = ( float )speed ; 
		DG_SetPos( world ) ;
		DG_RotVector( &to, &to, 1 ) ;
		GV_LenVec3F( &to, &( ctrl->step ), GV_VecLen3F( &to ), ( float )speed ) ;
		/* 到達点を予め計算 */
		to.vx = to.vy = 0.0F ;
		to.vz = ( float )length ; 
		DG_PutVector( &to, &to, 1 ) ;
    }

	if ( type & BUL_TYPE_NO_HZD ) {
		/* あたり見ない */
		len = ( float )length ;
		work->to = to ;
		work->atr = 0 ;
		work->seg_flag = 0 ;
	} else if ( ( work->seg_flag 
				 = HZX_OnlineHazardCheck( ctrl->hzx_id, &( ctrl->mov ), &to, 
										  HZX_CHK_F_SEGMENT | HZX_CHK_F_FLOOR, 
										  HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE, 
										  HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) ) {
		HZX_GetOnlineVector( &to ) ;
		len = GV_VecLen3F( &to ) ;
		HZX_GetOnlinePoint( &( work->to ) ) ;
		HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
	} else {
		len = ( float )length ;
		work->to = to ;
		work->atr = 0 ;
	}
		
	work->time = ( int )( len / ( float )speed ) + 1 ;
	if ( type & BUL_TYPE_NO_HZD ) {
		/* あたり見ない */
		ctrl->skip_flag |= CTRL_SKIP_HZX ;
	} else {
		/* 動的ハザードのみをチェックする */
		GM_ConfigControlHazard( ctrl, size, size, size * 2 ) ;
		ctrl->hzx_check_type = HZX_CHK_D_SEGMENT | HZX_CHK_D_FLOOR ;
		ctrl->seg_flag |= HZX_SEG_NO_BULLET ;
		ctrl->flr_flag |= HZX_FLOOR_NO_BULLET ;
		//ctrl->flr_flag |= HZX_FLOOR_NO_MISSILE ;
		/* onlineチェックのみ行う */
		ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
	}
	return 0 ;
}

static	void	InitTarget( Work *work, u_int side, u_int size, u_int damage, u_int weapon )
{
	TARGET		*t ;
	POWER_TARGET	*p ;
	FVECTOR		vec ;

	t = &( work->attack ) ;
	p = &( work->power ) ;
	vec.vx = vec.vy = vec.vz = ( float )size ;
	if ( work->type & BUL_TYPE_HIT_CHILD ) {
		GM_SetTarget( t, TARGET_CLASS, GM_CurrentStageMap, 
					  side, &vec, &DG_ZeroVector ) ;
	} else {
		GM_SetTarget( t, TARGET_CLASS2, GM_CurrentStageMap, 
					  side, &vec, &DG_ZeroVector ) ;
	}	

	GM_SetTargetWeaponType( t, I64(1) << weapon ) ;
	GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, damage, &( work->control.step ) ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
	//printf( "type %lx\n", t->weapon_type ) ;
}

static	int	InitPrim( Work *work, FMATRIX *world, int length, int type )
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	DG_TEX			*tex ;
	FVECTOR		*pos ;
	int			i ;
	float		u0, v0, u1, v1 ;

	tex = DG_GetTexture( TEXTURE );
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset ) ;
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset ) ;

	prim = work->prim = GM_MakePrim2( PRIM_FLAG, 1, 4 ) ;
	if ( prim == NULL ) return -1 ;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;
	for ( i = 0; i < 2; i ++ ) {
		uvrgb = prim->uvrgb[ i ] ;
		pos = prim->pos[ i ] ;
		pos[ 0 ] = pos[ 1 ] = pos[ 2 ] = pos[ 3 ] = DG_ZeroVector ;
		SET_UVRGB_COL( &uvrgb[0], 196, 128, 64, 128 );
		SET_UVRGB_COL( &uvrgb[1], 196, 128, 64, 128 );
		SET_UVRGB_COL( &uvrgb[2], 196, 128, 64, 128 );
		SET_UVRGB_COL( &uvrgb[3], 196, 128, 64, 128 );
		SET_UVRGB_UV( uvrgb + 0, u0, v0 );	/* 左上 */
		SET_UVRGB_UV( uvrgb + 1, u1, v0 );	/* 右上 */
		SET_UVRGB_UV( uvrgb + 2, u0, v1 );	/* 左下 */
		SET_UVRGB_UV( uvrgb + 3, u1, v1 );	/* 右下 */
		uvrgb[ 0 ].f = 0x8fff ;
		uvrgb[ 1 ].f = 0x8fff ;
		uvrgb[ 2 ].f = 0x0fff ;
		uvrgb[ 3 ].f = 0x0fff ;
	}
	prim->as_world = DG_UnitMatrix ;
	DG_InvisiblePrim2( prim ) ;
	return 0 ;
}

/*------------------------------------------------------------*/

static	int	GetResources( Work *work, FMATRIX *world, u_int type, u_int side, u_int size, 
						 u_int damage, u_int length, u_int speed, int weapon )
{
	work->world = *world ;
	work->type = type ;
	if ( InitControl( work, length, speed, size ) < 0 ) return -1 ;
	InitTarget( work, side, size, damage, weapon ) ;
	if ( type & BUL_TYPE_VISIBLE ) InitPrim( work, world, length, type ) ;
	return 0 ;
}

/* 弾起動 */
void	*NewRayValcanBullet( FMATRIX *world, u_int type, u_int side, u_int size, u_int damage, u_int length, u_int speed, int weapon )
{
	Work	*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;

      if ( BP_IsPAL()==TRUE )
      {
         /* 速度を上げる */
		   speed = ( int )( ( float )speed * 1.20F ) ;
      }
		if ( GetResources( work, world, type, side, size, 
						  damage, length, speed, weapon ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		GM_SetWeaponFire( weapon ) ;
	}
	{
		extern void	*NewRayValcanMuzzleFlash( FMATRIX *world );
		NewRayValcanMuzzleFlash( world );
	}
	return work ;
}

/* お手軽呼び出し版 */
/*
	FMATRIX		*world ;		手の先のマトリクス
	int			side ;			0:右手 1:左手
	int			type ;			0,1,2 銃口位置のセレクト
*/
void *NewRayValcanFire( FMATRIX *world, int side, int type )
{
	static FVECTOR	offset_r_vec[3] = {{ -1700, -500, -200, 1 },{ -1700, -500, 0, 1 },{ -1700, -500, 200, 1 }};
	static FVECTOR	offset_l_vec[3] = {{ 1700, -500, -200, 1 },{ 1700, -500, 0, 1 },{ 1700, -500, 200, 1 }};
	FVECTOR	*offset_vec ;
	FMATRIX		shot_mat, rot_mat ;

	offset_vec = ( side == 0 ) ? offset_r_vec : offset_l_vec ;
	
	shot_mat = *world ;
	GTE_ApplyMatrix( (FVECTOR*)shot_mat.m[3], &shot_mat, &offset_vec[type] );
	if ( side == 0 ){
		GTE_MakeRotate( &rot_mat, 0.0f, 0.0f, 1.0f, GTE_PS2RAD(-1024) );
	} else {
		GTE_MakeRotate( &rot_mat, 0.0f, 0.0f, 1.0f, GTE_PS2RAD(1024) );
	}
	GTE_MulMatrix( &shot_mat, &shot_mat, &rot_mat );
#if 0
	{/* 発射方向乱数散らし */
		int		angle ;
		FVECTOR	axis ;
		FMATRIX	error_mat ;
		axis.vx = RND( 256 ) - 128 ;
		axis.vy = RND( 256 ) - 128 ;
		axis.vz = RND( 256 ) - 128 ;
		angle = 6 ;
		GTE_Normalize( &axis, &axis );
		GTE_MakeRotateAxis( &error_mat, &axis, GTE_PS2RAD( angle ) );
		GTE_MulMatrix( &shot_mat, &shot_mat, &error_mat );
	}
#endif
	return ( NewRayValcanBullet( &shot_mat, BUL_TYPE_VISIBLE|BUL_TYPE_SPARK, PLAYER_SIDE, 100, 16, 40000, 1800, WP_ShotGun_Far ) );
}

void *NewRayValcanFireNoVRArea( FMATRIX *world, int side, int type )
{
	static FVECTOR	offset_r_vec[3] = {{ -1700, -500, -200, 1 },{ -1700, -500, 0, 1 },{ -1700, -500, 200, 1 }};
	static FVECTOR	offset_l_vec[3] = {{ 1700, -500, -200, 1 },{ 1700, -500, 0, 1 },{ 1700, -500, 200, 1 }};
	FVECTOR	*offset_vec ;
	FMATRIX		shot_mat, rot_mat ;

	offset_vec = ( side == 0 ) ? offset_r_vec : offset_l_vec ;
	
	shot_mat = *world ;
	GTE_ApplyMatrix( (FVECTOR*)shot_mat.m[3], &shot_mat, &offset_vec[type] );
	if ( side == 0 ){
		GTE_MakeRotate( &rot_mat, 0.0f, 0.0f, 1.0f, GTE_PS2RAD(-1024) );
	} else {
		GTE_MakeRotate( &rot_mat, 0.0f, 0.0f, 1.0f, GTE_PS2RAD(1024) );
	}
	GTE_MulMatrix( &shot_mat, &shot_mat, &rot_mat );
	return ( NewRayValcanBullet( &shot_mat, BUL_TYPE_VISIBLE|BUL_TYPE_SPARK|BUL_TYPE_SPARK_SMOKE, PLAYER_SIDE, 100, 16, 40000, 1800, WP_ShotGun_Far ) );
}
