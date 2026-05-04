//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	hang_act.c
	首絞め関連
	2001/04/23 K.Sigeno
	$Id: hang_act.c,v 1.1.1.3 2002/11/19 11:49:06 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"
#include	"korekado/enemy/motion.h"
#include	"libutl.h"


static	void	ActHangStill( ACTION *, int  ) ;
static	void	ActHangTie( ACTION *, int  ) ;

/* 首絞めヒット時コールバック */
static	void	HangCaptureCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
printf("HANG CALLBACK!!!!\n");
	/* つかめない場合 */
	if ( 
	( off->capture->capture != NULL )  /* もう捕まえたやつがいる */
	||( def->class & TARGET_DEAD )  /* 死んでいる */
	||(def->capture->flag & CAPTURE_HANG )	/*2001.10.22*/
	/* 壁がある */
//	||CheckBetweenHazard( work, &( off->capture->ctrl->mov ),&( def->capture->ctrl->mov ) ) 
	) {
printf("HANG FAILED!!!!\n");
		def->damaged &= ~TARGET_CAPTURE ;
		def->weapon_type &= ~( off->weapon_type ) ;
		if ( off->capture->capture == NULL ){
			off->damaged &= ~TARGET_CAPTURE ;
		}
		return ;
	}
printf("HANG SACCESS!!!!\n");
	GM_CaptureTarget( off, def ) ;
	off->capture->capture->flag |= CAPTURE_HANG ;
}


/****
SetCaptureTarget( act,(TARGET_OFFENSE|TARGET_CHECK_ONE,
	300.0F, 450.0F, 300.0F,0.0F, 0.0F, 0.0F,
	HUMAN21_KOSHI, 0.0F, 0.0F, 300.0F, 
	WP_HANG, HangCaptureCallBack ) ;
*****/

static	void	SetCaptureTarget( ACTION *act,
	FVECTOR	*size,FVECTOR *shift,
	int objNo, TARGET_CALLBACK callback )
{
	FVECTOR		mov, force ;
	FMATRIX		mat ;
	TARGET		*t ;
	CAPTURE_TARGET	*c ;

	force = DG_ZeroVector ;
	DG_SetPos( &act->body->objs->objs[ objNo ].world ) ;
	DG_MovePos( shift ) ;
	DG_GetPos( &mat ) ;
	GV_MatToVec( &mat, &mov ) ;
	t = &( act->offense ) ; /*こっちは実体*/
	c = act->cap_attack ;	/*こっちはポインタ*/
	GM_SetTarget( t, TARGET_OFFENSE|TARGET_CHECK_ONE, GM_CurrentMap,
		PLAYER_SIDE,size, &force ) ;
	GM_SetTargetWeaponType( t, WP_HANG|WP_NOPLAYER ) ;
	GM_SetCaptureTarget( t, c, act->ctrl, act->body ) ;
	if ( callback != NULL ) GM_SetTargetCallBack( t, callback, act ) ;
	GM_MoveTarget( t, &mov ) ;
	GM_PutTarget( t ) ;
//	NewTargetView2( t, 32, 232, 186 ) ;
}

/* 敵つかみ解除 */
static void		ClearCaptureTarget( act )
ACTION	*act ;
{
    CAPTURE_TARGET	*cap_attack ;

    cap_attack = act->cap_attack ;
    if ( cap_attack->capture != NULL ) {
		cap_attack->capture->flag |= CAPTURE_FREE ;
		cap_attack->capture->flag &= ~CAPTURE_HANG ;

		cap_attack->capture = NULL ;
#if 0
		work->def.damaged &= ~TARGET_CAPTURE ;
#else
//		act->deftrg.damaged &= ~TARGET_CAPTURE ;
#endif
		cap_attack->flag = 0 ;
    }
}

/* 首絞め開始 */
void	ActHangStart( act, time )
ACTION	*act ;
int		time ;
{
//	FVECTOR	size ={300.0F, 450.0F, 300.0F,0.0F},
//			shift ={0.0F, 0.0F, 300.0F,0.0F};
	FVECTOR	size ={2000.0F, 450.0F, 2000.0F,0.0F},
			shift ={0.0F, 0.0F, 900.0F,0.0F};
	TARGET			*target ;
	CAPTURE_TARGET	*capture ;

	target = &( act->offense ) ;
	capture = ( act->cap_attack ) ;

	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_PUSHT_SKIP|ACT_STATUS_GUN_FREE ) ;

	if ( act->CheckDamage( act ) ) {
		ClearCaptureTarget( act ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		ClearCaptureTarget( act ) ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->CheckPad( act ) ) {
		ClearCaptureTarget( act ) ;
		return ;
	}
    if ( time == 0 ) {
printf("HANG TRY!!!!\n");
/*ここで攻撃ターゲット発生
コールバックが起動すれば次のフレームで拾える
*/
		ClearCaptureTarget( act ) ;
		AT_SetAction( act, 0, (EM_sna_non_hang_miss), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
	/*攻撃あたり*/
		ENE_SetBodyTargetSize( &(act->bodyp.deftrg),
		ENE_BODYTRGSIZE_STAND ) ;
		/* つかみ判定 */
		SetCaptureTarget( act,&size,&shift,
			HUMAN21_KOSHI,HangCaptureCallBack ) ;
		return ;
    }
    if ( time == 1 ) {
		if(
		 ( !( target->damaged & TARGET_CAPTURE ) )
		||(  act->cap_attack->flag & CAPTURE_ESCAPE  )
		 ) {
		/* つかめなかった */
printf("HANG FREE!!!! IN TIME[%d]\n",time);
			ClearCaptureTarget( act ) ;
			act->act_end = 1 ;
			AT_SetMode( act, ENE_ActStandStill ) ;
			return ;
		}
		/*つかみ成功*/
printf("HANG ACT SACCESS!!!!\n");
		act->cap_attack->capture->flag |= CAPTURE_HANG ;
		AT_SetMode( act, ActHangStill ) ;
		return ;
	}
}

/* つかみ静止 */
static	void	ActHangStill( act, time )
ACTION		*act ;
int			time ;
{

//	act->cap_attack->capture->flag |= CAPTURE_HANG ;
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_PUSHT_SKIP|ACT_STATUS_GUN_FREE ) ;
	if ( act->CheckDamage( act ) ) {
		ClearCaptureTarget( act ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		//printf("DIR -1 TO STILL\n");
		//printf("AT_ACT LINE %d\n",__LINE__);
		act->act_end = 1 ;
		ClearCaptureTarget( act ) ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->CheckPad( act ) ) {
		ClearCaptureTarget( act ) ;
		return ;
	}
	if ( time == 0 ) {
		AT_SetAction( act, 0, (EM_sna_non_hang_idle), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
    }	
#if 0
	if ( time == 480 ) {
		act->cap_attack->capture->flag |= CAPTURE_BREAK ;
	}
#endif
//	if((time % DIRECT_TICK(120))==0){
	if(time >= DIRECT_TICK(120)){
		AT_SetMode( act, ActHangTie ) ;
	}
#if 0
	if ( time >= 960 ) {
		/* 首折りへ */
		act->cap_attack->capture->flag |= CAPTURE_BREAK ;
		AT_SetMode( act, ActHangBreak ) ;
		return ;
	}
#endif
    /* 被害者の命令による離し */
    if ( act->cap_attack->flag & CAPTURE_FREE ) {
printf("ESCAPE !!!!\n");
		ClearCaptureTarget( act ) ;
		act->act_end = 1;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
    }
}
static	void	ActHangTie( act, time )
ACTION		*act ;
int			time ;
{

//	act->cap_attack->capture->flag |= CAPTURE_HANG ;
	AT_SetActStatus( act, ACT_STATUS_STAND|ACT_STATUS_PUSHT_SKIP|ACT_STATUS_GUN_FREE ) ;
	if ( act->CheckDamage( act ) ) {
		ClearCaptureTarget( act ) ;
		return ;
	}
	if ( act->dir >= 0 ) {
		act->ctrl->turn.vy = act->dir ;
	}else	{
		//printf("DIR -1 TO STILL\n");
		//printf("AT_ACT LINE %d\n",__LINE__);
		act->act_end = 1 ;
		ClearCaptureTarget( act ) ;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
	}
	if ( act->CheckPad( act ) ) {
		ClearCaptureTarget( act ) ;
		return ;
	}
	if ( time == 0 ) {
		act->cap_attack->capture->flag |= CAPTURE_HANG ;
		AT_SetAction( act, 0, (EM_sna_non_hang_tie), 0, MOTION_MASK_FULL, ACT_INTERP_DEF ) ;
    }	
	if ( time == 2 ) {
		FVECTOR size = {500.0f,500.0f,500.0f,0.0f} ;
//		ENE_SetOffenseTarget2( act,WP_BULLET,HUMAN21_MIGI_UDE2,&size ) ;
		ENE_SetOffenseTarget2( act,WP_SOCOM,HUMAN21_MIGI_UDE2,&size ) ;
    }	

    /* 被害者の命令による離し */
    if ( act->cap_attack->flag & CAPTURE_FREE ) {
printf("ESCAPE !!!!\n");
		ClearCaptureTarget( act ) ;
		act->act_end = 1;
		AT_SetMode( act, ENE_ActStandStill ) ;
		return ;
    }
	if ( GM_GetObjectMotionEnd( act->body, 0 ) ) {
		AT_SetMode( act, ActHangStill ) ;
		return ;
	}
}

#if 0
static	void	CheckAttack( work )
Work		*work ;
{
    int		press, status ;
    ACTION	func ;
    PL_WeaponSet	*wpset ;

	/* 武器独自の攻撃チェックがある */
	if ( PL_CheckAttack != NULL ) {
		void	( *chkat )( PlayerWork *work ) ;
		chkat = PL_CheckAttack ;
		( *chkat )( work ) ;
		return ;
	}
    if ( work->action2 != NULL ) return ;
    press = work->pad->press ;
    status = work->pad->status ;
    wpset = ( PL_WeaponSet * )work->wp_set ;
    if ( wpset->type & WP_TYPE_PRESS ) status = press ;
    if ( status & PAD_WEAPON ) {
		/* 押し判定アナログ有り */
		if ( wpset->type & WP_TYPE_PRESSURE &&
			work->pad->pressure[ PAD_PRESS_WEAPON ] < PAD_WEAPON_TH ) {
			return ;
		}
		func = ( ( PL_WeaponSet * )( work->wp_set ) )->shoot ;
		if ( func != NULL ) {
			if ( wpset->type & WP_TYPE_MODE1FUNC ) {
				SetMode( work, func ) ;
			} else {
				SetMode2( work, func ) ;
			}
		} else if ( work->weapon == WP_None ) {				/* 素手 */
			if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
				/* はだか */
				return ;
			}
			switch( work->stance ) {
			case STAND :
				if ( PadForce > 0 ) {
					/* なげ */
					SetMode( work, Throw ) ;
				} else {
					/* 首絞め */
					SetMode( work, HangStart ) ;
				}
				SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM ) ;
				break ;
			case SQUAT :
				/* 死体つかみ */
				SetMode( work, PullBody ) ;
				SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM ) ;
				break ;
			default :

			}
		}
    } else if ( press & PAD_PUNCH ) {
		/* コムボ */
		if ( work->stance == STAND ) {
			SetMode( work, Combo ) ;
			SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM ) ;
		}
    }
}
static	void	HangRelease( work, time )
Work		*work ;
int		time ;
{
    SetFlag( FLAG_CANNOT_CHANGE_WEAPON | FLAG_CANNOT_CHANGE_ITEM | 
			FLAG_CANNOT_PEEP | FLAG_NO_HEADTOENEMY ) ;
	SetStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
	PL_ObjPos( work, HUMAN21_ATAMA, &work->camera ) ;
	if ( time == 0 ) {
		if ( work->data == 1 ) {
			SetAction( work, MS.shared[ Mhang_release ], 6 ) ;
		} else {
			SetAction( work, MS.shared[ Mhang_escape ], 6 ) ;
		}
	}
	if ( EndMotion( work ) ) {
		SetMode( work, StandStill ) ;
	}
	GM_ReverseSlideCameraDir( work->chanl ) ;
}
#endif


