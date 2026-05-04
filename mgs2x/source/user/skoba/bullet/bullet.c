//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bullet.c
   銃の弾
   
   1999/07/19 M.Sonoyama
   $Id: bullet.c,v 1.1.1.3 2002/11/19 11:50:02 Yoshizawa1 Exp $
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

#include "bp_vector.h"
#include "BP_Misc.h"

#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | TARGET_CHILD|TARGET_GET_NORMAL )
#define	TARGET_CLASS2	(TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN|TARGET_GET_NORMAL )

#define	PRIM_FLAG	(DG_PRIM2_LINE | DG_PRIM2_SHADE | DG_PRIM2_ANTIALIASING)

#define	HIT_BIT		(0x01)
#define	WATER_BIT	(0x02)

/*------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int GM_WaterBulletSE;
extern	int	GM_SparkNoiseLevel ;
extern void *NewBulletBubble( FVECTOR *from, FVECTOR *to, float ratio );
/*------------------------------------------------------------*/


typedef struct	Work_bullet_t{ //BP_GENERAL - added name so you can debug on PC
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

static	void	Hit( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
    Work		*work ;
	FVECTOR		dc ;

    work = ( Work * )ptr ;

	/* ヒット点と防御中心の間をオンラインチェック */
	/* えねみーサイドのみ */	
	if ( off->side == ENEMY_SIDE ) {
		GM_TargetGetCenter( &dc, def ) ;
		if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &off->hit, &dc, HZX_CHK_ALL,
								    HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
								    HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) {
			GM_TargetHitCancel( off, def ) ;
			return ;
		}
	}

    if ( work->type & BUL_TYPE_VISIBLE ) {
		work->prim->pos[ work->prim->buffer_clock ][ 2 ] = off->hit ;
    }
    GV_DestroyActor( work ) ;
}

/* 弾痕 */
static	void	CallScar( work )
Work		*work ;
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
static	void	CallSpark( work ) 
Work		*work ;
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
    GM_CallSpark( seNo, &mat, &pole ) ;
	if ( work->type & BUL_TYPE_NOISE ) {
		noise = NOISE_S ;
		if ( work->type & BUL_TYPE_NOISE_SS ) noise = NOISE_SS ;
		if ( GM_SparkNoiseLevel >= NOISE_MM ) noise = GM_SparkNoiseLevel ;
		GM_SetNoise( noise, &work->to, work->control.map ) ;
	}
}

static	void	Act( work )
Work		*work ;
{
    TARGET	*t ;
    FVECTOR	from, to, *pos ;
    FVECTOR	diff ;
    //FVECTOR	w_pos1 ;
	int		w_flag = 0;

    /* 当たったので終了 */
    if ( work->flag & HIT_BIT ) {
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
	if (  -- work->time <= 0 ) {
		/* 最長到達点 */
		/* 動的ハザードに当たったかも */
		if ( work->control.n_touches > 0 ) {
			FVECTOR			dto ;
			float			len1, len2 ;

			HZX_GetOnlinePoint( &dto ) ;
			len1 = GV_VecLen3F2( &from, &work->to ) ;
			len2 = GV_VecLen3F2( &from, &dto ) ;
			if ( len2 < len1 ) {
				DG_COPY_VEC( &work->to, &dto ) ;
				HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
				work->seg_flag = HZX_GetOnlineHazardType() ;
			} else {
				DG_COPY_VEC( &to, &work->to ) ;
			}
		} else {
			DG_COPY_VEC( &to, &work->to ) ;
		}
		work->flag |= HIT_BIT ;
	} else if ( work->control.n_touches > 0 ) {
		/* 動的ハザードに当たった */
		HZX_GetOnlineHazard( &work->seg, &work->atr ) ;
		HZX_GetOnlinePoint( &( work->to ) ) ;
		work->seg_flag = HZX_GetOnlineHazardType() ;
		work->flag |= HIT_BIT ;
	}

	if( !(work->flag & WATER_BIT) ){
		/* 水面水飛沫 */
		_sceVu0SubVector( &diff, &to, &from ) ;
		if( PL_CheckBulletSplash( &from, &diff, 25.0F, 75.0F ) ){
			work->flag|= WATER_BIT;
			w_flag = 1;
			if(GM_WaterBulletSE!=0){
				GM_SeSetMode( GM_WaterBulletSE , &from, GM_SEMODE_NORMAL );
			}
		}
	}

#if 0
	if( work->flag & WATER_BIT ){	// 水中進行中 
		if( w_flag != 0 ){
			w_ratio = GM_WaterLevel + work->control.step.vy;
/*
			if( (from.vy > w_ratio)
			 && (to.vy   < w_ratio) ){
				
				_sceVu0SubVector( &w_pos0, &from, &work->control.step );
				DG_COPY_VEC( &w_pos1, &from );
				w_ratio = DG_FABS( (w_pos1.vy - GM_WaterLevel)/(w_pos1.vy - w_pos0.vy) );
				w_pos0.vx = w_pos0.vx + (w_pos1.vx - w_pos0.vx)* (1.0f - w_ratio);
				w_pos0.vy = GM_WaterLevel;
				w_pos0.vz = w_pos0.vz + (w_pos1.vz - w_pos0.vz)* (1.0f - w_ratio);
				NewBulletBubble( &w_pos0, &w_pos1, w_ratio );
			}
*/
			if( w_pos0.vy > GM_WaterLevel - 3000.0f ){
				w_ratio = DG_FABS( ( from.vy - to.vy ) / work->control.step.vy );
				NewBulletBubble( &from, &to, w_ratio );
			}
		}
	}
#else
	if( work->flag & WATER_BIT ){	// 水中進行中 
		if( work->attack.side == PLAYER_SIDE ){	// 敵の銃だけ 
			if( from.vy > GM_WaterLevel - 3000.0f ){
            static int bullet_bubble_clock = 0;

            bullet_bubble_clock ^= 1;

            if ( gAS_DG_HackArea != kDGHA_w32a || !bullet_bubble_clock )
            {
   				float w_ratio = DG_FABS( ( from.vy - to.vy ) / work->control.step.vy );
	   			NewBulletBubble( &from, &to, w_ratio );
            }
			}
		}
	}
#endif



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
		pos[ 0 ] = work->old_from ;
		pos[ 1 ] = from ;
		pos[ 2 ] = to ;
		work->old_from = from ;
    }
}

static	void	Die( work )
Work		*work ;
{
    GM_FreeControl( ( CONTROL * )&work->control ) ;
    if ( work->type & BUL_TYPE_VISIBLE ) GM_FreePrim2( work->prim ) ;
}

/*------------------------------------------------------------*/

static	int	InitControl( work, length, speed, size )
Work		*work ;
u_int		length, speed, size ;
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
		/* onlineチェックのみ行う */
		ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
	}
	return 0 ;
}

static	void	InitTarget( work, side, size, damage, weapon, type )
Work		*work ;
u_int		side, size, damage, weapon, type ;
{
	TARGET		*t ;
	POWER_TARGET	*p ;
	FVECTOR		vec, force ;
	float		len ;

	t = &( work->attack ) ;
	p = &( work->power ) ;
	vec.vx = vec.vy = vec.vz = ( float )size ;
#if 0
	if ( side == ENEMY_SIDE ) {
		switch( weapon ) {
		case WP_Usp :
		case WP_m92 :
		case WP_Famas :
		case WP_Socom :
			if ( work->type & BUL_TYPE_HIT_CHILD ) {
				GM_SetTarget( t, TARGET_CLASS, GM_CurrentStageMap, 
							 side, &vec, &DG_ZeroVector ) ;
			} else {
				GM_SetTarget( t, TARGET_CLASS2, GM_CurrentStageMap, 
							 side, &vec, &DG_ZeroVector ) ;
			}
			break ;
		case WP_Psg1 :
			GM_SetTarget( t, TARGET_CLASS, GM_CurrentStageMap, 
						 side, &vec, &DG_ZeroVector ) ;
		}
	} else {
		GM_SetTarget( t, TARGET_CLASS2, GM_CurrentStageMap, side, &vec, &DG_ZeroVector ) ;
	}
#endif
	if ( work->type & BUL_TYPE_HIT_CHILD ) {
		GM_SetTarget( t, TARGET_CLASS, GM_CurrentStageMap, 
					  side, &vec, &DG_ZeroVector ) ;
	} else {
		GM_SetTarget( t, TARGET_CLASS2, GM_CurrentStageMap, 
					  side, &vec, &DG_ZeroVector ) ;
	}	
	GM_SetTargetName( t, weapon ) ;
	if ( type & BUL_TYPE_NO_PLAYER ) {
		GM_SetTargetWeaponType( t, ( I64(1) << weapon ) | WP_NOPLAYER ) ;
	} else {
		GM_SetTargetWeaponType( t, I64(1) << weapon ) ;
	}

	DG_COPY_VEC( &force, &work->control.step ) ;
	len = GV_VecLen3F( &force ) ;
	if ( len > 1000.0F ) GV_LenVec3F( &force, &force, 0.0F, 1000.0F ) ;
	
	GM_SetPowerTarget( t, p, POWER_ONCE, 255, 0, damage, &force ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
	//printf( "type %lx\n", t->weapon_type ) ;
}

static	int	InitPrim( work, world, length, type )
Work		*work ;
FMATRIX		*world ;
int		length ;
int		type ;
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR		*pos ;
	int			i ;

	prim = work->prim = GM_MakePrim2( PRIM_FLAG, 1, 3 ) ;
	if ( prim == NULL ) return -1 ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;
	for ( i = 0; i < 2; i ++ ) {
		uvrgb = prim->uvrgb[ i ] ;
		pos = prim->pos[ i ] ;
		pos[ 0 ] = pos[ 1 ] = pos[ 2 ] = DG_ZeroVector ;
		if ( type & BUL_TYPE_GREEN ) {
			uvrgb[ 0 ].r = uvrgb[ 1 ].r = uvrgb[ 2 ].r = 64 ;
			uvrgb[ 0 ].g = uvrgb[ 1 ].g = uvrgb[ 2 ].g = 164 ;
			uvrgb[ 0 ].b = uvrgb[ 1 ].b = uvrgb[ 2 ].b = 32 ;
			uvrgb[ 0 ].r = uvrgb[ 0 ].g = uvrgb[ 0 ].b = 0 ;	/* added by K.Takabe 2000/11/17 */
		}else if ( type & BUL_TYPE_RED ) {
			uvrgb[ 0 ].r = uvrgb[ 1 ].r = uvrgb[ 2 ].r = 255 ;
			uvrgb[ 0 ].g = uvrgb[ 1 ].g = uvrgb[ 2 ].g = 64 ;
			uvrgb[ 0 ].b = uvrgb[ 1 ].b = uvrgb[ 2 ].b = 64 ;
			uvrgb[ 0 ].r = uvrgb[ 0 ].g = uvrgb[ 0 ].b = 0 ;	/* added by K.Takabe 2000/11/17 */
		} else {
			//uvrgb[ 0 ].r = uvrgb[ 1 ].r = uvrgb[ 2 ].r = 196 ;
			//uvrgb[ 0 ].g = uvrgb[ 1 ].g = uvrgb[ 2 ].g = 128 ;
			//uvrgb[ 0 ].b = uvrgb[ 1 ].b = uvrgb[ 2 ].b = 64 ;
			uvrgb[ 0 ].r = uvrgb[ 1 ].r = uvrgb[ 2 ].r = 196*2/3 ;
			uvrgb[ 0 ].g = uvrgb[ 1 ].g = uvrgb[ 2 ].g = 128*2/3 ;
			uvrgb[ 0 ].b = uvrgb[ 1 ].b = uvrgb[ 2 ].b = 64*2/3 ;
			uvrgb[ 1 ].r = uvrgb[ 2 ].r / 2 ;
			uvrgb[ 1 ].g = uvrgb[ 2 ].g / 2 ;
			uvrgb[ 1 ].b = uvrgb[ 2 ].b / 2 ;
			uvrgb[ 0 ].r = uvrgb[ 0 ].g = uvrgb[ 0 ].b = 0 ;	/* added by K.Takabe 2000/11/17 */
		}
		uvrgb[ 0 ].a = uvrgb[ 1 ].a = uvrgb[ 2 ].a = 128 ;
		uvrgb[ 0 ].q = uvrgb[ 1 ].q = uvrgb[ 2 ].q = 4096 ;
		uvrgb[ 0 ].f = 0x8fff ;
		uvrgb[ 1 ].f = 0x0fff ;
		uvrgb[ 2 ].f = 0x0fff ;
	}
	prim->as_world = DG_UnitMatrix ;
	DG_InvisiblePrim2( prim ) ;
	return 0 ;
}

/*------------------------------------------------------------*/

static	int	GetResources( work, world, type, side, size, 
						 damage, length, speed, weapon )
Work		*work ;
FMATRIX		*world ;
u_int		type, side, size, damage, length, speed ;
int weapon;
{
	work->flag = 0;

	work->world = *world ;
	work->type = type ;
	if ( InitControl( work, length, speed, size ) < 0 ) return -1 ;
	InitTarget( work, side, size, damage, weapon, type ) ;
	if ( type & BUL_TYPE_VISIBLE ) {
		if ( InitPrim( work, world, length, type ) < 0 ) return -1 ;
	}
	return 0 ;
}

/* 弾起動 */
void	*NewBullet( world, type, side, size, damage, length, speed, weapon )
FMATRIX		*world ;
u_int		type ;
u_int		side ;
u_int		size ;
u_int		damage ;
u_int		length ;
u_int		speed ;
int			weapon ;
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
	return work ;
}
