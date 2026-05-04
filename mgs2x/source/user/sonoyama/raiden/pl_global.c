//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_global.c
   プレイヤー、別キャラから使用する関数

   2000/06/12 M.Sonoyama
   $Id: pl_global.c,v 1.1.1.3 2002/11/19 11:50:58 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
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
#include	"g_other.h"
#include	"raiden.h"

/* 武器・アイテム変更不可チェック */
int			PL_CheckNoUseWeapon( int weapon )
{
	extern	int	PL_GetPlayerItem( void ) ;	
	extern	int	PL_GetPlayerWeapon( void ) ;
	int			type, p_weapon, item ;

	item = PL_GetPlayerItem() ;
	p_weapon = PL_GetPlayerWeapon() ;
	
	if ( GM_CheckNoUseWeaponFromWeapon( p_weapon, weapon ) ) return 1 ;
	if ( GM_CheckNoUseWeaponFromItem( item, weapon ) ) return 1 ;
	/* 念の為 */
	if ( GM_CheckNoUseWeaponFromWeapon( GM_Weapon, weapon ) ) return 1 ; /* これは意味がない */
	if ( GM_CheckNoUseWeaponFromItem( GM_Item, weapon ) ) return 1 ;

	type = GM_WeaponTypes[ weapon ] ;
	if ( PL_MenuNoUseWeapon & ( I64(1) << weapon ) ) return 1 ;
	if ( PL_MenuNoUseWeaponScn & ( I64(1) << weapon ) ) return 1 ;
	if ( type & PL_MenuNoUseWeaponType ) return 1 ;
	return 0 ;
}

int			PL_CheckNoUseItem( int item )
{
	extern	int	PL_GetPlayerItem( void ) ;	
	extern	int	PL_GetPlayerWeapon( void ) ;
	int			type ;
	int			weapon, p_item ;

	p_item = PL_GetPlayerItem() ;
	weapon = PL_GetPlayerWeapon() ;
	
	if ( GM_CheckNoUseItemFromWeapon( weapon, item ) ) return 1 ;
	if ( GM_CheckNoUseItemFromItem( p_item, item ) ) return 1 ;
	/* 念の為 */
	if ( GM_CheckNoUseItemFromWeapon( GM_Weapon, item ) ) return 1 ;
	if ( GM_CheckNoUseItemFromItem( GM_Item, item ) ) return 1 ;	/* これは意味がない */

	type = GM_ItemTypes[ item ] ;
	if ( PL_MenuNoUseItem & ( I64(1) << item ) ) return 1 ;
	if ( PL_MenuNoUseItemScn & ( I64(1) << item ) ) return 1 ;
	if ( type & PL_MenuNoUseItemType ) return 1 ;
	return 0 ;
}

/* シナリオからＮｏＵｓｅを設定 */
int			PL_COM_SetNoUse( void )
{
	int		item ;

	if ( GCL_GetOption( 'c' ) != NULL ) {
		PL_MenuNoUseWeaponScn = I64(0) ;
		PL_MenuNoUseItemScn = I64(0) ;
		return 0 ;
	}

	GCL_GetOption( 'w' ) ;
	while( GCL_NextStr() != NULL ) {
		item = GCL_GetNextInt() ;
		PL_MenuNoUseWeaponScn |= I64(1) << item ;
	}
	GCL_GetOption( 'i' ) ;
	while( GCL_NextStr() != NULL ) {
		item = GCL_GetNextInt() ;
		PL_MenuNoUseItemScn |= I64(1) << item ;
	}
	return 0 ;
}

/* 主観操作反転 */
void		PL_ShukanReverse( float *dy, short *rx )
{
	if ( GM_CheckGameStatus( STATE_PAD_DEMO ) ) return ;
	if ( GM_Configuration & GM_CONFIG_SHUKAN_REVERSE ) {
		*dy = 0.0F - *dy ;
		if ( rx != NULL ) *rx = 0 - *rx ;
	}
}

/* マガジンリロード */
void		PL_SetMagazine( wp, max )
int			wp, max ;
{
	if ( max > 0 ) GM_MagazineMax = max ;
	GM_Magazine = ( ( GM_WeaponTypes[ wp ] & WP_TYPE_TACTICAL ) &&
				    GM_Magazine > 0 ) ? GM_MagazineMax + 1 : GM_MagazineMax ;
	if ( GM_Magazine > GM_WeaponNum( wp ) ) {
		GM_Magazine = GM_WeaponNum( wp ) ;
	}
}

/* マガジンへらし */
int			PL_DecrementMagazine( void )
{
	/* 無限処理 */
	//if ( GM_ItemTypes[ PL_GetPlayerItem() ] & IT_TYPE_MUGEN ) return GM_Magazine ;
	GM_Magazine -- ;
	return GM_Magazine ;
}

/* 無敵にする */
int		PL_PlayerSetInvincible( void )
{
	GM_SetPlayerStatus( PLAYER_INVINCIBLE_PRG ) ;
	return 1 ;
}

/* 無敵を解除する */
int		PL_PlayerResetInvincible( void )
{
	if ( GM_CheckPlayerStatus( PLAYER_INVINCIBLE_PRG ) ) {
		GM_ResetPlayerStatus( PLAYER_INVINCIBLE_PRG ) ;
	} else {
		if ( GM_CheckPlayerStatus( PLAYER_INVINCIBLE ) ) {
			if ( GM_PlayerWork->invincible_time == 0 ) return 0 ;
			GM_ResetPlayerStatus( PLAYER_INVINCIBLE ) ;
			GM_PlayerWork->invincible_time = 0 ;
		}
	}
	return 1 ;
}

/* 不可視にする */
void	PL_InvisiblePlayer( void )
{
	if ( GM_PlayerWork != NULL ) {
		GM_PlayerWork->flag |= FLAG_DEMO_INVISIBLE_ORDER ;
	}
}

/* 可視に戻す */
void	PL_VisiblePlayer( void )
{
	if ( GM_PlayerWork != NULL ) {
		GM_PlayerWork->flag &= ~FLAG_DEMO_INVISIBLE_ORDER ;
	}
}

/* シナリオからFVECTORを取得 */
void	PL_GetNextFV( FVECTOR *v )
{
	v->vx = ( float )GCL_GetNextInt() ;
	v->vy = ( float )GCL_GetNextInt() ;
	v->vz = ( float )GCL_GetNextInt() ;
	v->vw = 1.0F ;
}

int		PL_GetOptionFV( char ops, FVECTOR *v )
{
	if ( GCL_GetOption( ops ) == NULL ) return 0 ;
	PL_GetNextFV( v ) ;
	return 1 ;
}

/* シナリオからSVECTORを取得 */
void	PL_GetNextSV( SVECTOR *v )
{
	v->vx = GCL_GetNextInt() ;
	v->vy = GCL_GetNextInt() ;
	v->vz = GCL_GetNextInt() ;
	v->pad = 0 ;
}

int		PL_GetOptionSV( char ops, SVECTOR *v )
{
	if ( GCL_GetOption( ops ) == NULL ) return 0 ;
	PL_GetNextSV( v ) ;
	return 1 ;
}

#if 0
/* 主観カメラとの距離チェック */
static	inline	void	MakeBound( FVECTOR *b1, FVECTOR *b2,
								  FVECTOR *v1, FVECTOR *v2 ) {
    asm __volatile__("
	lqc2    	vf4,0x0(%2)
	lqc2    	vf5,0x0(%3)
    vmini.xyzw	vf6,vf4,vf5
    vmax.xyzw	vf7,vf4,vf5
	sqc2    	vf6,0x0(%0)
	sqc2    	vf7,0x0(%1)
	": : "r" (b1), "r" (b2), "r" (v1), "r" (v2) : "memory" ) ;
}

int		PL_CheckIntoPlayerCamera( DG_OBJS *objs, float chk_len )
{
	DG_OBJ			*obj ;
	FVECTOR			*cpos ;
	FVECTOR			b1, b2 ;
	int				i ;
	float			len ;

	if ( GM_PlayerWork == NULL ) return 0 ;
	if ( !GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE ) ) return 0 ;

	cpos = &( ( GM_GetCurrentCamera( GM_PlayerWork->chanl ) )->position ) ;
	
	len = GV_VecLen3F2( cpos, ( FVECTOR * )objs->world.m[ 3 ] ) ;

	if ( chk_len < len ) return 0 ;

	obj = objs->objs ;
	for ( i = 0; i < objs->n_models; i ++, obj ++ ) {
		DG_SetPos( &obj->world ) ;
		DG_PutVector( &obj->bound_min, &b1, 1 ) ;
		DG_PutVector( &obj->bound_max, &b2, 1 ) ;
		MakeBound( &b1, &b2, &b1, &b2 ) ;
		if ( cpos->vx < b1.vx || cpos->vx > b2.vx ||
			 cpos->vy < b1.vy || cpos->vy > b2.vy ||
			 cpos->vz < b1.vz || cpos->vz > b2.vz ) continue ;
		return 1 ;
	}
	return 0 ;
}
#endif

/*----------------------------------------------------------------

  汎用カメラ揺らし

----------------------------------------------------------------*/

typedef struct	{
	GV_ACT_EX		actor ;
	int				time ;
	int				chanl ;
	long64			status ;
} Work1 ;

static	void	Act1( Work1 *work )
{
	FVECTOR		adj ;
	int			time ;

	if ( -- work->time <= 0 ) {
		GV_DestroyActor( work ) ;
		return ;
	}

	if ( work->status != I64(0) &&
		 !GM_CheckPlayerStatus( work->status ) ) return ;

	time = work->time ;
	adj.vx = ( float )( ( BP_PS2_rand() % time * 2 ) - time ) ;
	adj.vy = ( float )( ( BP_PS2_rand() % time * 2 ) - time ) ;
	GM_SetCameraAdjust( work->chanl, &adj ) ;
}

void	*PL_DamageCamera( int time, long64 status )
{
	Work1	*work ;

	work = ( Work1 * )GV_NewActor( GV_ACTOR_PLAYER, sizeof( Work1 ) ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act1, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	work->time = time ;
	work->status = status ;
	return work ;
}

/*----------------------------------------------------------------

  足跡マネージャー

----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT_EX			actor ;
	MOTION_CONTROL		*m_ctrl ;
	void				*child ;
	int					foot_flag ;
	int					item ;
	short				flag ;
	short				count ;
	short				left ;
	short				right ;
	float				t ;
} FootPrintWork ;

static	void	FootPrintAct( FootPrintWork *work )
{
	float		t ;
	int			noenefind, item ;

	item = PL_GetPlayerItem() ;
	noenefind = ( item == IT_Uniform ) ? 0x4 : 0x0 ;

	if ( work->flag ) {
		if ( work->left < 0 ) {
			t = work->m_ctrl->mt3_ctrl[ 0 ].play_time 
				/ work->m_ctrl->mt3_ctrl[ 0 ].motion_total_time ;	
			if ( work->t > t ) {
				work->foot_flag = 0x02 | noenefind ;
			} else if ( t >= 0.55F && work->t < 0.55F ) {
				work->foot_flag = 0x01 | noenefind ;
			}
			work->t = t ;
		} else {
			if ( work->left < work->right ) {
				if ( MT_CHECK_MOTION_TIME( work->m_ctrl, 0, work->left * NTSC_TIME_BASE ) ) {
					//printf( "left %d\n", work->left ) ;
					work->foot_flag = 0x02 | noenefind ;				
				} else if ( MT_CHECK_MOTION_TIME( work->m_ctrl, 0, work->right * NTSC_TIME_BASE ) ) {
					//printf( "right %d\n", work->right ) ;
					work->foot_flag = 0x01 | noenefind ;				
				} 
			} else {
				if ( MT_CHECK_MOTION_TIME( work->m_ctrl, 0, work->right * NTSC_TIME_BASE ) ) {
					//printf( "right2 %d\n", work->right ) ;
					work->foot_flag = 0x01 | noenefind ;				
				} else if ( MT_CHECK_MOTION_TIME( work->m_ctrl, 0, work->left * NTSC_TIME_BASE ) ) {
					//printf( "left2 %d\n", work->left ) ;
					work->foot_flag = 0x02 | noenefind ;
				} 
			}
		}
		work->flag = 0 ;
	} else if ( ++ work->count >= 300 / TIME_BASE * 10 ) {
		work->t = 0.0F ;
		GV_SleepActor( &work->actor, GV_CLASS_WAITING ) ;
	}
	if ( work->item != item &&
		( work->item == IT_Uniform || item == IT_Uniform ) ) {
		work->foot_flag |= 0x08 ;
	}
	work->item = item ;
}

void	*PL_FootPrintManager( PlayerWork *pwork )
{
	FootPrintWork	*work ;

	work = ( FootPrintWork * )GV_NewActor( GV_ACTOR_PLAYER, sizeof( FootPrintWork ) ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, FootPrintAct, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	work->m_ctrl = pwork->body.m_ctrl ;
	work->child = NewFootPrint( &pwork->body, &pwork->control, &work->foot_flag, 3000 ) ;
	if ( work->child == NULL ) return work ; // KARI-
	ASSERT( work->child != NULL ) ;
	GV_SetActorChild( work, work->child ) ;
	GV_SleepActor( &work->actor, GV_CLASS_WAITING ) ;
	work->item = PL_GetPlayerItem() ;
	return work ;
}

void	PL_FootPrintAct( void *ptr, int init, int left, int right )
{
	FootPrintWork		*work ;

	if ( ptr == NULL ) return ;
	work = ( FootPrintWork * )ptr ;
	work->count = 0 ;
	work->flag = 1 ;
	work->left = left ;
	work->right = right ;
	if ( init ) work->t = 0.0F ;
	GV_WakeupActor( &work->actor, GV_CLASS_WAITING ) ;
}

void	PL_FootPrintWakeUp( void *ptr )
{
	FootPrintWork		*work ;

	if ( ptr == NULL ) return ;
	work = ( FootPrintWork * )ptr ;
	GV_WakeupActor( &work->actor, GV_CLASS_WAITING ) ;
}

/* 強制足跡 */
void	PL_FootPrintForce( void *ptr, int which ) 
{
	FootPrintWork		*work ;
	int					item, noenefind ;

	if ( ptr == NULL ) return ;
	work = ( FootPrintWork * )ptr ;
	item = PL_GetPlayerItem() ;
	noenefind = ( item == IT_Uniform ) ? 0x4 : 0x0 ;
	work->foot_flag = 0 ;
	if ( which & 1 ) work->foot_flag |= 0x02 | noenefind ;	/* 左 */
	if ( which & 2 ) work->foot_flag |= 0x01 | noenefind ;	/* 右 */
}

/*----------------------------------------------------------------*/

/* コールバック系のクリア */
void	PL_ClearSpecialCallback( void )
{
	if ( GM_PlayerWork == NULL ) return ;
	PL_TargetCallbackFunc = NULL ;
	PL_DamageFunc = NULL ;
	//PL_CheckAttackFunc = NULL ;
	PL_ReturnModes[ 0 ] = NULL ;
	PL_ReturnModes[ 1 ] = NULL ;
	PL_ReturnModes[ 2 ] = NULL ;
	PL_ReturnModeMotionArc = 0 ;
}

/*----------------------------------------------------------------*/

/* エルード範囲トラップの名前取得 */
int		PL_GetEludeTrapCode( void )
{
	if ( GM_PlayerWork == NULL ) return 0 ;
	return GM_PlayerWork->work_l.beyond.trap_id ;
}


/*----------------------------------------------------------------*/

/* モデルの頭首表示／非表示 */
void	PL_VisibleHead( int mode, DG_OBJS *objs, int chanl )
{	
	int			i, vflag ;

	if ( chanl < 0 ) vflag = DG_FLAG_INVISIBLE ;
	else			 vflag = DG_FLAG_INVISIBLE0 << chanl ;

	if ( mode == 0 ) {
		objs->objs[ HUMAN21_KUBI ].flag |= vflag ;
		objs->objs[ HUMAN21_ATAMA ].flag |= vflag ;
		for ( i = 0; i < objs->def->n_x_models; i ++ ) {
			if ( objs->objs[ i ].parent == HUMAN21_KUBI ||
				 objs->objs[ i ].parent == HUMAN21_ATAMA ) {
				objs->objs[ i ].flag |= vflag ;
			}
		}
	} else {
		objs->objs[ HUMAN21_KUBI ].flag &= ~vflag ;
		objs->objs[ HUMAN21_ATAMA ].flag &= ~vflag ;
		for ( i = 0; i < objs->def->n_x_models; i ++ ) {
			if ( objs->objs[ i ].parent == HUMAN21_KUBI ||
				 objs->objs[ i ].parent == HUMAN21_ATAMA ) {
				objs->objs[ i ].flag &= ~vflag ;
			}
		}
	}
}

/*----------------------------------------------------------------*/

/* 主観時はカメラ位置から出るＳＥ */
void	PL_SeSetSubject( int se, FVECTOR *pos, int mode )
{
	GM_CameraSet	*cam ;

	if ( GM_PlayerBody == NULL ) return ;
	if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
#if 1 //BP - enabling corrected 3D sound calculation globally.
      //On PS2, gun sounds came from exactly the camera's position.
//#ifdef	KP_XBOX
		// XBOX の場合カメラの位置に出すと不安定に聞こえる
		// （ 次のフレームでカメラが動いた場合など）
		// カメラより少し前にだす
		FVECTOR	vTmp;
		float	fScale;
		cam = GM_GetCurrentCamera( 0 ) ;
		_sceVu0SubVector( &vTmp, &cam->target, &cam->position );
		fScale = 500.f / GV_VecLen3F( &vTmp );	// 50cm 
		_sceVu0ScaleVector( &vTmp, &vTmp, fScale );
		_sceVu0AddVector( &vTmp, &vTmp, &cam->position );
		GM_SeSetMode( se, &vTmp, mode ) ;
#else
		cam = GM_GetCurrentCamera( 0 ) ;
		GM_SeSetMode( se, &cam->position, mode ) ;
#endif		
		return ;
	}
	GM_SeSetMode( se, pos, mode ) ;
}
