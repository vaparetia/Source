//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	npcact.c
	NPCアクション

	2001/02/09 Y.Korekado
	$Id: npcact.c,v 1.1.1.3 2002/11/19 11:44:23 Yoshizawa1 Exp $
*/

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
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"npc.h"

/*----------------------------------------------------------------*/
#define	HANG_STRUGGLE_TIME COUNT_VMODE(180)	/* enemy.hと同じ*/
#define	HANG_OFF_TIME COUNT_VMODE(120)
/*----------------------------------------------------------------*/
void	NPC_Incline( CONTROL *ctrl )
{
    int		incline ;

    incline = GV_DiffDirS( ctrl->turn.vy, ctrl->rot.vy ) ;
    if ( incline > 128 ) incline = 128 ;
    else if ( incline < -128 ) incline = -128 ;
    ctrl->turn.vz = incline ;    
}
/* モーションの向きが正面か真後ろ向きの場合のみ使用すること！！ */
void NPC_SetSlopeRotX( CONTROL *ctrl )
{
	extern int ENE_GetGRot( CONTROL *, float ) ;
	int x ;
	
	if ( (ctrl->mov.vy - ctrl->levels[0]) > 500 ) return ;
	x = ENE_GetGRot( ctrl, 1000.0f ) ;
	if ( (x<-1200) || (x>1200) ) return ;
	ctrl->turn.vx = x ;

}
void NPC_ReSetSlopeRotX( CONTROL *ctrl )
{
	if ( (ctrl->mov.vy - ctrl->levels[0]) > 500 ) {
		ctrl->turn.vx = 0 ;
	}
}

static	void HangSetStep( CONTROL *ctrl, OBJECT *body, CAPTURE_TARGET *cap, FVECTOR *shift )
{
//    static FVECTOR 	Shift = { 34.0F, -465.0F, 263.0F } ;
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov, mov2 ;
    float	     	len ;

	SET_FLAG( body->flag, OBJECT_MOTIONSTEP_THROUGH ) ;
	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;

	/* 当たりチェック高さはプレイヤーと同じにする */
	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;
	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	DG_PutVector( shift, &mov, 1 ) ;

	/* 暫定 */
	if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &mov2, &mov,HZX_CHK_ALL,HZX_TYPE_PLAYER,HZX_FLOOR_NO_ENEMY ) ) {
	    /* 間に壁がある場合、壁の少し手前を引きずられ位置にする */
	    HZX_GetOnlineVector( &vec ) ;
	    len = GV_VecLen3F( &vec ) - 8.0F ;
	    if ( len <= 0.0F ) len = 0.0F ;
	    GV_LenVec3F( &vec, &vec, 1.0F, len ) ;
	    _sceVu0AddVector( &mov, &mov2, &vec ) ;
	}

//    ctrl->turn.vy = cap->capture->ctrl->rot.vy ;	    
    ctrl->turn.vy = MatToYRot( &BODYWORLD( pl_ctrl->object, HUMAN21_MUNE) ) ;
#if 0
	ctrl->step.vx += mov.vx - ctrl->mov.vx ;
	ctrl->step.vz += mov.vz - ctrl->mov.vz ;

	if ( ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		ctrl->mov.vy = GV_NearTimeF( ctrl->mov.vy, pl_ctrl->mov.vy+shift->vy, 0 ) ;
	}
#else
	KR_FMatToFvec( &BODYWORLD( pl_ctrl->object, HUMAN21_MUNE), &mov2 ) ;
	DG_SetPos2( &mov2, &ctrl->turn ) ;
	DG_PutVector( shift, &mov, 1 ) ;

	ctrl->step.vx += mov.vx - ctrl->mov.vx ;
	ctrl->step.vz += mov.vz - ctrl->mov.vz ;

	if ( ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		ctrl->mov.vy = GV_NearTimeF( ctrl->mov.vy, mov.vy, 0 ) ;
	}
#endif
}


static void NpcBlood( NPCWORK *npc, int mode )
{
   // Armature: commented out in the first version of the code we got (from 2010)
#if 0
   extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
   FMATRIX	*world ;
   NPCTARGET	*trg ;
   TARGET	*dam ;

	trg = &npc->target ;

	dam = trg->dam_trg ;
	world = &(BODYWORLD( npc->body, trg->dam_obj )) ;
	GV_SetActorChild( npc->character, NewBlood( world, &dam->center, &dam->power->force, mode, 0 ) ) ;
#endif
}

static int HangCheck( NPCWORK *npc )
{
	NPCTARGET	*trg ;
	TARGET	*def ;
	CAPTURE_TARGET	*cap ;
	NPCACT *act ;
	int	dam_child_num ;
	long64 weapon ;

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	cap = npc->target.capture ;
	if ( cap->flag & CAPTURE_HANG ) {	/* 絞め直し */
		cap->flag &= ~CAPTURE_HANG ;
		NPC_SetActMode( npc, NPC_ActHang ) ;
		return 1 ;
	}
	if ( cap->flag & CAPTURE_FREE ) {	/* 首絞め解除 */
		cap->capture->flag |= CAPTURE_FREE ;
		cap->capture = NULL ;	
		cap->flag = 0 ;
		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
		if ( act->faint > 0 ) {
			NPC_SetActMode( npc, NPC_ActHangFree ) ;
		} else {
			NPC_SetActMode( npc, NPC_ActHangDown ) ;
		}
		return 1 ;
	}
	if ( cap->flag & CAPTURE_BREAK ) {		/* 首折れる */
		cap->capture->flag |= CAPTURE_FREE ;
		cap->capture = NULL ;	
		cap->flag = 0 ;
		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
		NPC_SetActMode( npc, NPC_ActHangDie ) ;
		return 1 ;
	}

	/* ダメージ */
	weapon = 0 ;
	if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) {
		trg->dam_trg = trg->def_child + dam_child_num ;
		trg->dam_obj = trg->connect_obj[ dam_child_num ] ;
		weapon = trg->dam_trg->weapon_type ;
		NPC_DamageFlagClear( npc ) ;
		printf(" child damage [%d]/[%d]\n",dam_child_num,npc->target.child_trg_num ) ;
	}

	if ( TARGET_POWER & def->damaged ) {
		trg->dam_trg = def ;
		trg->dam_obj = HUMAN21_MUNE ;
		weapon = def->weapon_type ;
		NPC_DamageFlagClear( npc ) ;
		//printf("npc damage weapon type [%x]  \n",weapon);
	}

	if ( weapon ) {
		if ( weapon & (WP_BULLET|WP_BLOW ) ) {
			act->life -= act->down_dam ;
			if ( (weapon & WP_BULLET) && (npc->status & NPC_STATUS_NO_HANGDAM) ) {
				if ( act->life > 0 ) {
					NpcBlood( npc, 0 ) ;
				} else {
					NpcBlood( npc, 1 ) ;
				}
			}

			if ( act->life > 0 ) {
				if ( npc->status & NPC_STATUS_NO_HANGDAM ) {
					NPC_SetActMode( npc, NPC_ActHangDamageNoMot ) ;
				} else {
					NPC_SetActMode( npc, NPC_ActHangDamage ) ;
				}
			} else {
				cap->capture->flag |= CAPTURE_FREE ;
				cap->capture = NULL ;	
				cap->flag = 0 ;
				NPC_SetActMode( npc, NPC_ActHangDown ) ;
			}
			/* このフレームではダメージを受けない */
			NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

			return 1 ;
		}
	}

	return 0 ;
}


#define	FAINT_NODAM_WP	(WP_BLADEFAINT|WP_PUNCHR|WP_PUNCHL|WP_KICK|WP_KICK1|WP_STUNFAR|WP_STUNGRENADE)
static int	FaintTrgCheck( NPCWORK *npc )
{
	TARGET	*def ;
	NPCTARGET	*trg ;
	CAPTURE_TARGET	*cap ;
	NPCACT *act ;
	int	dam_child_num ;
	long64 weapon ;

	act = &npc->action ;
	trg = &npc->target ;
	def = npc->target.deftrg ;
	cap = npc->target.capture ;
	weapon = 0 ;

	if ( (dam_child_num = NPC_ChildTargetCheck( npc )) >= 0 ) {
		trg->dam_trg = trg->def_child + dam_child_num ;
		trg->dam_obj = trg->connect_obj[ dam_child_num ] ;
		weapon = trg->dam_trg->weapon_type ;
		NPC_DamageCaptureFlagClear( npc ) ;
		printf(" child damage [%d]/[%d]\n",dam_child_num,npc->target.child_trg_num ) ;
		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;

		if ( weapon & WP_M92 ) {
			NPC_SetNeedl( npc->body,trg->connect_obj[dam_child_num], &trg->dam_trg->hit, MASUIDAN_MODEL ) ;
			act->faint_count = act->sleep_max ;
			if ( act->faint_mode != NPC_FAINT_ZZZ ) {
				NPC_CallHeadMark( npc, HMK2_TYPE_PIYO_A  ) ;
				act->faint_mode = NPC_FAINT_ZZZ ;
			}
			NPC_ActStatus( act, NPC_ACT_STATUS_IK_PIKU ) ;
			return 0 ;
		} else if ( weapon & FAINT_NODAM_WP ) {
			if ( act->faint_count > COUNT_VMODE(100) ) act->faint_count -= COUNT_VMODE(100) ;
			NPC_ActStatus( act, NPC_ACT_STATUS_IK_PIKU ) ;
			return 0 ;
		} else {
			if ( trg->dam_obj == HUMAN21_ATAMA ) {
				act->life = 0 ;
			} else {
				act->life -= act->down_dam ;
			}
			if ( act->faint_count > COUNT_VMODE(100) ) act->faint_count -= COUNT_VMODE(100) ;
			NPC_SetActMode( npc, NPC_ActFaintDamage ) ;
		}

		return 1 ;
	}


	if ( TARGET_POWER & def->damaged ) {
		//printf("npc damage weapon type [%x]  \n",weapon);
		trg->dam_trg = def ;
		trg->dam_obj = HUMAN21_MUNE ;
		weapon = trg->dam_trg->weapon_type ;
		NPC_DamageCaptureFlagClear( npc ) ;
		/* このフレームではダメージを受けない */
		NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
		
		if ( weapon & (WP_COLDSPRAY) ) {
			if ( act->faint_count > COUNT_VMODE(100) ) act->faint_count -= COUNT_VMODE(100) ;

         // Arm fix:
         // adjust time and size of shake if running at 30
         // fix for MGSTWO-3181
         if (AS_IsRunningAt30Fps())
         {
            if (act->adj_piku_time == 0)
            {
               act->adj_piku_time = 3;
               // index into table of rotation amount of spine
               act->adj_piku_value = 1;
            }
         }
         else
         {
            act->adj_piku_time = 1 ;/* エマは間接補完をしているため効かない*/
         }
         return 0 ;

      } else if ( weapon & WP_THROWG ) {
			act->adj_piku_time = NPC_ADJ_PIKU_TIME ;/* エマは間接補完をしているため効かない*/
			NPC_ActStatus( act, NPC_ACT_STATUS_IK_PIKU ) ;
			return 0 ;
		} else if ( weapon & FAINT_NODAM_WP ) {
			if ( act->faint_count > COUNT_VMODE(100) ) act->faint_count -= COUNT_VMODE(100) ;
			NPC_ActStatus( act, NPC_ACT_STATUS_IK_PIKU ) ;
			return 0 ;
		} else if ( weapon & WP_BLAST ) {
			act->life -= (act->down_dam*2) ;
		} else {
			act->life -= act->down_dam ;
		}
		if ( act->faint_count > COUNT_VMODE(100) ) act->faint_count -= COUNT_VMODE(100) ;
		NPC_SetActMode( npc, NPC_ActFaintDamage ) ;

		return 1 ;
	}

    if ( cap->capture != NULL ) { /* 捕まった */
		NPC_DamageFlagClear( npc ) ;
		NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
		NPC_SetActMode( npc, NPC_ActFaintWakeup ) ;
		return 1 ;
	}

	if ( act->faint_count == 0 ) {
		NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
		NPC_SetActMode( npc, NPC_ActFaintEnd ) ;
		return 1 ;
	}

	return 0 ;
}

static void FloorThrough( NPCWORK *npc )
{
	CAPTURE_TARGET	*cap ;

	cap = npc->target.capture ;
	npc->ctrl->step = DG_ZeroVector ;
	if ( cap->capture != NULL ) {
		cap->capture->flag |= CAPTURE_FREE ;
		cap->capture = NULL ;	
		cap->flag &= ~CAPTURE_FREE ;
	}
	NPC_SetActMode( npc, NPC_ActFaintDown ) ;
}

static	void FaintSetPosition( CAPTURE_TARGET *cap, FVECTOR *shift, CONTROL *ctrl, FVECTOR *mov )
{
    CONTROL			*pl_ctrl ;
    FVECTOR			vec, mov2 ;
    float	     	len ;

	pl_ctrl = cap->capture->ctrl ;	/* 掴んだ人のコントロール */

	/* ひきずられ位置計算 */
	_sceVu0CopyVector( &mov2, &( pl_ctrl->mov ) ) ;

	/* 当たりチェック高さはプレイヤーと同じにする */
//	mov2.vy = pl_ctrl->hzx_base + pl_ctrl->hzx_height ;

	DG_SetPos2( &mov2, &pl_ctrl->rot ) ;
	DG_PutVector( shift, mov, 1 ) ;
	if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &mov2, mov,
				     HZX_CHK_ALL, HZX_TYPE_PLAYER, HZX_FLOOR_NO_ENEMY ) ) {
	    /* 間に壁がある場合、壁の少し手前を引きずられ位置にする */
	    HZX_GetOnlineVector( &vec ) ;
	    len = GV_VecLen3F( &vec ) - 8.0F ;
	    if ( len <= 0.0F ) len = 0.0F ;
	    GV_LenVec3F( &vec, &vec, 1.0F, len ) ;
	    _sceVu0AddVector( mov, &mov2, &vec ) ;
	} 
}

/* モーションが始まって１６フレーム目の総体座標に合わせる */
static int FaintSetStepNear(CAPTURE_TARGET *cap,OBJECT *body,CONTROL *ctrl, FVECTOR *shift, int side, int time )
{
	FVECTOR	mov, to ;

	body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	if ( cap->flag & CAPTURE_HEAD ) {
		shift += ( side == NPC_DOWN_SIDE_F )?
			NPC_DRAG_SHIFT_HEAD_F_START16 : NPC_DRAG_SHIFT_HEAD_B_START16 ;
	} else {
		shift += ( side == NPC_DOWN_SIDE_F )?
			NPC_DRAG_SHIFT_LEG_F_START16 : NPC_DRAG_SHIFT_LEG_B_START16 ;
	}
	FaintSetPosition( cap, shift, ctrl, &mov ) ;

	to.vx = GV_NearTimeF( ctrl->mov.vx, mov.vx, time ) ;
	to.vz = GV_NearTimeF( ctrl->mov.vz, mov.vz, time ) ;

	ctrl->step.vx += to.vx - ctrl->mov.vx ;
	ctrl->step.vz += to.vz - ctrl->mov.vz ;
	{
		ctrl->turn.vx = ENE_GetGRotFromPos( &ctrl->mov, &ctrl->rot, 1000.0f, 
					ctrl->hzx_id, HZX_CHK_ALL, ctrl->flr_flag ) ;
	}

	/* 高さ */
	if ( ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		to.vy = GV_NearTimeF( ctrl->mov.vy, mov.vy, time ) ;
		if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &to, &ctrl->mov,
					     HZX_CHK_ALL, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
			printf("!!!!!!!!!!!!!!!!!WARNING flor through !!!!!\n" ) ;
			return 1 ;
		}
		ctrl->mov.vy = to.vy ;
	}
	return 0 ;
}


enum {
	TAKE_TURN_NONE,
	TAKE_TURN_A,
	TAKE_TURN_B,
} ;

static int TakeTurn( int mot )
{
	if ( mot == NPC_MOT_DRAG_LEG_IDLE ||
		 mot == NPC_MOT_DRAG_LEG_WALK ) {
		return TAKE_TURN_A ;
	} else if ( 
		 mot == NPC_MOT_DRAG_BODY_IDLE ||
		 mot == NPC_MOT_DRAG_LEG_IDLE_B ||
		 mot == NPC_MOT_DRAG_BODY_WALK ||
		 mot == NPC_MOT_DRAG_LEG_WALK_B ) {
		return TAKE_TURN_B ;
	}
	return TAKE_TURN_NONE ;
}

static	int FaintSetStep( CAPTURE_TARGET *cap,OBJECT *body,CONTROL *ctrl, FVECTOR *shift, int side, int time, int turn, int current_mot )
{
	FVECTOR	mov, to ;
	int		pose ;
	
	body->flag |= OBJECT_MOTIONSTEP_THROUGH ;

	if ( cap->flag & CAPTURE_HEAD ) {
		shift += NPC_DRAG_SHIFT_HEAD_F ;
	} else {
		shift += ( side == NPC_DOWN_SIDE_F )?
			NPC_DRAG_SHIFT_LEG_F : NPC_DRAG_SHIFT_LEG_B ;
	}
	FaintSetPosition( cap, shift, ctrl, &mov ) ;
	
	/* 持ち上げた後 */
	pose = 0 ;
	if ( (current_mot == NPC_MOT_DRAG_LEG_IDLE ) ||
		 (current_mot == NPC_MOT_DRAG_LEG_START) ||
		 (current_mot == NPC_MOT_DRAG_LEG_WALK) ) {
		pose = 1 ;	/* 階段補正反対 */
	} else if ( (current_mot == NPC_MOT_DRAG_LEG_IDLE_B) ||
		 (current_mot == NPC_MOT_DRAG_LEG_WALK_B) ) {
		pose = 2 ;	/* 階段補正反対 */
	}

	if ( time >= 0 ) {
		if ( turn == TAKE_TURN_A ) {
		    ctrl->turn.vy = ctrl->rot.vy = cap->capture->ctrl->rot.vy + 2048 ;
		} else if ( turn == TAKE_TURN_B ) {
		    ctrl->turn.vy = ctrl->rot.vy = cap->capture->ctrl->rot.vy ;
		}
	}

	if ( time < 0 ) time = 0 ;
	to.vx = GV_NearTimeF( ctrl->mov.vx, mov.vx, time ) ;
	to.vz = GV_NearTimeF( ctrl->mov.vz, mov.vz, time ) ;

	ctrl->step.vx += to.vx - ctrl->mov.vx ;
	ctrl->step.vz += to.vz - ctrl->mov.vz ;

	if ( ctrl->skip_flag & CTRL_SKIP_FLR_CHECK ) {
		float	f, sub ;
		CONTROL	*pl ;
		
		pl = cap->capture->ctrl ;
		f = KR_GetGRotFromPos( &ctrl->mov, ctrl->hzx_id, ctrl->hzx_check_type, ctrl->flr_flag ) ;
		if ( (sub = f - pl->levels[0]) > 0.0f ) {
			if ( sub > 450.0f ) sub = 450.0f ;
			switch ( pose ) {
				case 0 :
					mov.vy += (sub / 2) ;
					ctrl->turn.vx = -384 * (sub/450) ;
				break ;
				case 1 :
					mov.vy += (sub / 2) ;
					ctrl->turn.vx = 384 * (sub/450) ;
				break ;
				default :
					mov.vy += (sub ) ;
					ctrl->turn.vx = -384 * (sub/450) * 2 ;
				break ;
			}
//printf("faint move sub+[%f]  vx[%d] pose[%d]\n",sub/2, ctrl->turn.vx, pose );
		} else {
			ctrl->rot.vx = GV_NearTimeF( ctrl->rot.vx, 0, time ) ;
		}
		to.vy = GV_NearTimeF( ctrl->mov.vy, mov.vy, time ) ;
		if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &to, &ctrl->mov,
					     HZX_CHK_ALL, HZX_TYPE_ENEMY, HZX_FLOOR_NO_ENEMY ) ) {
			printf("!!!!!!!!!!!!!!!!!WARNING flor through !!!!!\n" ) ;
			return 1 ;
		}
		ctrl->mov.vy = to.vy ;
	}
	return 0 ;
}

static	int FrontCheck( OBJECT *body )
{
	FMATRIX	mat, *w ;
	static FVECTOR Front = { 0.0, 0.0, 100.0 } ;

	w = &body->objs->objs[0].world ;
	DG_SetPos( w ) ;
	DG_MovePos( &Front ) ;
	DG_GetPos( &mat ) ;

	if ( mat.m[3][1] > w->m[3][1] ) {
		return 1 ;
	}

	return 0 ;
}
/*----------------------------------------------------------------*/
void NPC_ActStandStill( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_STAND  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, NPC_MOT_STAND ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;
	if ( npc->CheckPad( npc ) ) return ;

	if ( act->dir >= 0 ) {
		npc->ctrl->turn.vy = act->dir ;
		NPC_SetActMode( npc, NPC_ActMove ) ;
		return ;
	}
}

void NPC_ActMove( NPCWORK *npc, int time )
{
	CONTROL	*ctrl ;
	NPCACT *act ;

	act = &npc->action ;
	ctrl = npc->ctrl ;

	NPC_ActStatus( act, NPC_ACT_STATUS_STAND | NPC_ACT_STATUS_MOVE  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->base_mar, NPC_MOT_MOVE ) ;
	}

	if ( npc->CheckDamage( npc ) ) {
		ctrl->turn.vz = 0 ;
		return ;
	}
	if ( npc->CheckPad( npc ) ) {
		ctrl->turn.vz = 0 ;
		return ;
	}
	if ( act->dir < 0 ) {
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		ctrl->turn.vz = 0 ;
		return ;
	}

	/* 方向転換時の体の傾き */
	NPC_Incline( ctrl ) ;
	/* 進行方向 */
	ctrl->turn.vy = act->dir ;

#if 0
	if ( speed != 0.0F ) {
		act->body->flag |= OBJECT_MOTIONSTEP_THROUGH ;
		ctrl->step.vx = speed * _RsinF( (int)act->dir ) ;
		ctrl->step.vz = speed * _RcosF( (int)act->dir ) ;
	}
#endif
}


/* ループモーション */
void NPC_ActLoopMotion( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
#if 0
	/* 注！！ループモーションは移動しない */
	SET_FLAG( npc->body->flag, OBJECT_MOTIONSTEP_THROUGH ) ;
	npc->ctrl->step = DG_ZeroVector ;
#endif

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;

}

/* １回こっきりモーション */
void NPC_ActOneTimeMotion( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	}

	if ( npc->CheckDamage( npc ) ) return ;

	if ( act->pad != act->set_pad ) {
		if ( !npc->CheckPad( npc ) ) {
			NPC_SetActMode( npc, NPC_ActStandStill ) ;
		}
		return ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		return ;
	}
	if ( act->dir >= 0 ) npc->ctrl->turn.vy = act->dir ;
}


/* 投げられ */
void NPC_ActThrow( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStStatus( act, NPC_ACT_STST_UNDER_NEARCHECK ) ;

	if ( (npc->ctrl->mov.vy - npc->ctrl->levels[0]) < 500 ) {
		NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN  ) ;
	}

	if ( time == 0 ) {
		NPC_SetActMotion( npc, act->set_mar, act->set_mot ) ;
	    npc->ctrl->turn.vy = npc->target.capture->capture->ctrl->rot.vy  ;	    
	}

	NPC_SetSlopeRotX( npc->ctrl ) ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		NPC_CaptureFlagClear( npc ) ;
		if ( act->faint > 0 ) {
			NPC_SetActMode( npc, NPC_ActFaintEnd ) ;
			npc->action.down_side = NPC_DOWN_SIDE_F ;
		} else {
			NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
			NPC_CallHeadMark( npc, HMK2_TYPE_PIYO  ) ;
			NPC_SetActMode( npc, NPC_ActFaint ) ;

			npc->target.capture->flag = CAPTURE_FRONT ;
			npc->action.down_side = NPC_DOWN_SIDE_F ;
		}
		return ;
	}
}

/* 首締められ */
void NPC_ActHang( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	
	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_TIE ) ;
		if ( npc->status & NPC_STATUS_HANG_LIFEDAM ) {
//			攻撃ターゲットを受けるはず ;
		} else {
			act->faint -- ;
		}
		npc->target.tmp_count = 0 ;
		SET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_FLR_CHECK ) ;
	}

	if ( HangCheck( npc ) ) return ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		NPC_SetActMode( npc, NPC_ActHangIdle ) ;
		return ;
	}

	HangSetStep( npc->ctrl, npc->body, npc->target.capture, npc->target.hang_shift ) ;
}
void NPC_ActHangIdle( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	
	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_IDLE ) ;
	}

	if ( HangCheck( npc ) ) return ;

	if ( npc->target.tmp_count > HANG_STRUGGLE_TIME  ) {
		if ( act->faint > 0 ) {
			NPC_SetActMode( npc, NPC_ActHangStruggle ) ;
			return ;
		}
	}
	
	/* 手離し処理 */
	if ( npc->target.tmp_count == (HANG_STRUGGLE_TIME+HANG_OFF_TIME  ) ) {
		npc->target.capture->capture->flag |= CAPTURE_FREE ;
	}

	if ( npc->target.capture->flag & CAPTURE_MOVE ) {
		NPC_SetActMode( npc, NPC_ActHangMove ) ;
		return ;
	}

	HangSetStep( npc->ctrl, npc->body, npc->target.capture, npc->target.hang_shift ) ;
	npc->target.tmp_count ++ ;
}
void NPC_ActHangMove( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_MOVE ) ;
	}

	if ( HangCheck( npc ) ) return ;

	if ( npc->target.tmp_count > HANG_STRUGGLE_TIME  ) {
		if ( act->faint > 0 ) {
			NPC_SetActMode( npc, NPC_ActHangStruggle ) ;
			return ;
		}
	}

	/* 手離し処理 */
	if ( npc->target.tmp_count == (HANG_STRUGGLE_TIME+HANG_OFF_TIME  ) ) {
		npc->target.capture->capture->flag |= CAPTURE_FREE ;
	}

	if ( !(npc->target.capture->flag & CAPTURE_MOVE) ) {
		NPC_SetActMode( npc, NPC_ActHangIdle ) ;
		return ;
	}

	HangSetStep( npc->ctrl, npc->body, npc->target.capture, npc->target.hang_shift ) ;
	npc->target.tmp_count ++ ;
}
void NPC_ActHangStruggle( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	
	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_RESISIT ) ;
	}

	if ( HangCheck( npc ) ) return ;

	if ( time == HANG_OFF_TIME  ) {
		npc->target.capture->capture->flag |= (CAPTURE_FREE|CAPTURE_ESCAPE) ;
		npc->target.capture->capture = NULL ;	
		npc->target.capture->flag = 0 ;
/*ACTが変わり無敵になる場合は変わる前のフレームから無敵にする事！！*/
//		AT_SetActStatus( act, ACT_STATUS_PUSHT_SKIP ) ;
		NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
		NPC_SetActMode( npc, NPC_ActHangEscape ) ;
		return ;
	}

	HangSetStep( npc->ctrl, npc->body, npc->target.capture, npc->target.hang_shift ) ;
	npc->target.tmp_count ++ ;
}

void NPC_ActHangDamage( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	
	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_DAMAGE ) ;
		npc->target.tmp_count = 0 ;
	}

	if ( HangCheck( npc ) ) return ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		NPC_SetActMode( npc, NPC_ActHangIdle ) ;
		return ;
	}

	HangSetStep( npc->ctrl, npc->body, npc->target.capture, npc->target.hang_shift ) ;
}

void NPC_ActHangDamageNoMot( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	
	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_MOVE ) ;/*試しに*/
		npc->target.tmp_count = 0 ;
	}

	if ( HangCheck( npc ) ) return ;

//	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
	if ( time > COUNT_VMODE(8) ) {
		NPC_SetActMode( npc, NPC_ActHangIdle ) ;
		return ;
	}

	HangSetStep( npc->ctrl, npc->body, npc->target.capture, npc->target.hang_shift ) ;
}

void NPC_ActHangEscape( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	
	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_ESCAPE ) ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		UNSET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_FLR_CHECK ) ;
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		return ;
	}
}
void NPC_ActHangFree( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	
	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_RELEASE ) ;
		UNSET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_FLR_CHECK ) ;
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		NPC_SetActMode( npc, NPC_ActStandStill ) ;
		return ;
	}
}
void NPC_ActHangDown( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	NPC_ActStStatus( act, NPC_ACT_STST_UNDER_NEARCHECK ) ;
	
	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_RELEASE_DOWN ) ;
		UNSET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_FLR_CHECK ) ;
	}

	NPC_SetSlopeRotX( npc->ctrl ) ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		NPC_SetFaintCount( npc, act->faint_max, NPC_FAINT_PIYO ) ;
		NPC_ChangeTargetSize( npc, NPC_TARGET_SIZE_DOWN ) ;
		NPC_CallHeadMark( npc, HMK2_TYPE_PIYO  ) ;
		NPC_SetActMode( npc, NPC_ActFaint ) ;
		return ;
	}
}
void NPC_ActHangDie( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_HANG  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_DEATH  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	NPC_ActStStatus( act, NPC_ACT_STST_UNDER_NEARCHECK ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_DIE ) ;
		UNSET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_FLR_CHECK ) ;
	}

	NPC_SetSlopeRotX( npc->ctrl ) ;

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		act->act_end = 1 ;
		NPC_ChangeTargetSize( npc, NPC_TARGET_SIZE_DOWN ) ;
		NPC_SetActMode( npc, NPC_ActDeath ) ;
		npc->action.down_side = NPC_DOWN_SIDE_F ;
		return ;
	}
}

void NPC_ActFaint( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN|NPC_ACT_STATUS_FAINT  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	if ( time == 0 ) {
	    if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
			NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_DOWN_F ) ;
			npc->target.capture->flag = CAPTURE_FRONT ;
		} else {
			NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_DOWN_B ) ;
			npc->target.capture->flag = CAPTURE_BACK ;
		}
		npc->target.deftrg->class = FAINT_TARGET_CLASS ;
		NPC_ChangeTargetSize( npc, NPC_TARGET_SIZE_DOWN ) ;
	}

	if ( FaintTrgCheck( npc ) ) return ;
}

static int CaptureCheck( NPCWORK *npc )
{
	CAPTURE_TARGET	*cap ;

	cap = npc->target.capture ;
	if ( cap->flag & CAPTURE_FREE ) { /* ひきずられ終わり */
		cap->capture = NULL ;	
		cap->flag &= ~CAPTURE_FREE ;
		NPC_SetActMode( npc, NPC_ActFaintDown ) ;
	    return 1 ;
	}

	if ( npc->action.faint_count < COUNT_VMODE(10) ) {
		cap->capture->flag |= CAPTURE_FREE ;
		cap->capture = NULL ;	
		cap->flag &= ~CAPTURE_FREE ;
		NPC_SetActMode( npc, NPC_ActFaintDown ) ;
	    return 1 ;
	}

    return 0 ;
}

void NPC_ActFaintWakeup( NPCWORK *npc, int time )
{
	CAPTURE_TARGET	*cap ;
	NPCACT *act ;
	int	left ;

//printf("NPC_ActFaintWakeup[%d]\n",time);
	act = &npc->action ;
	cap = npc->target.capture ;

	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_FAINT  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;

    if ( cap->flag & CAPTURE_HEAD ) {
		NPC_ActStatus( act, NPC_ACT_STATUS_IK_FOOT  ) ;
	} else {
		NPC_ActStatus( act, NPC_ACT_STATUS_IK_HAND  ) ;
	}

	if ( time == 0 ) {
		FVECTOR	vec ;
		int		dir, mot ;

		_sceVu0SubVector( &vec, &cap->capture->ctrl->mov, &npc->ctrl->mov ) ;
		dir = _FVecDir2( &vec ) ;
		mot = NPC_MOT_DRAG_BODY_START ;
		act->tmp_time = COUNT_VMODE(50) ;
	    if ( cap->flag & CAPTURE_HEAD ) {
		    if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
				mot = NPC_MOT_DRAG_BODY_START ;
			    npc->ctrl->turn.vy = dir + 2048 ;
			} else {
				mot = NPC_MOT_DRAG_BODY_START_B ;
				npc->action.down_side = NPC_DOWN_SIDE_F ;
			    npc->ctrl->turn.vy = dir ;
				act->tmp_time = COUNT_VMODE(90) ;
			}
	    } else {
		    if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
				mot = NPC_MOT_DRAG_LEG_START ;
			    npc->ctrl->turn.vy = dir ;
			} else {
				mot = NPC_MOT_DRAG_LEG_START_B ;
			    npc->ctrl->turn.vy = dir + 2048 ;
			}
	    }
		NPC_SetActMotion( npc, npc->drag_mar, mot ) ;
		NPC_ReSetSlopeRotX( npc->ctrl ) ;
	}

	if ( time == act->tmp_time ) SET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_FLR_CHECK ) ;

	if ( CaptureCheck( npc ) ) {
		UNSET_FLAG( npc->ctrl->skip_flag,  CTRL_SKIP_FLR_CHECK ) ;
		return ;
	}

	if ( time < 16 ){
	    if ( FaintSetStepNear( cap, npc->body, npc->ctrl, npc->target.drag_shift, 
			      npc->action.down_side, 16 - time ) ) {
			FloorThrough( npc ) ;
			return ;
		}
	}else{
		left = KR_LeftMotion( &npc->body->m_ctrl->mt3_ctrl[ 0 ] ) ;
		if ( FaintSetStep( cap, npc->body, npc->ctrl, npc->target.drag_shift,
				npc->action.down_side, left, TakeTurn( act->current_mot ), act->current_mot ) ) {
			FloorThrough( npc ) ;
			return ;
		}
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		NPC_SetActMode( npc, NPC_ActFaintTake ) ;
		return ;
	}
}


void NPC_ActFaintTake( NPCWORK *npc, int time  )
{
	CAPTURE_TARGET	*cap ;
	NPCACT *act ;

//printf("NPC_ActFaintTake[%d]\n",time);
	act = &npc->action ;
	cap = npc->target.capture ;

	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_FAINT  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;

    if ( cap->flag & CAPTURE_HEAD ) {
		NPC_ActStatus( act, NPC_ACT_STATUS_IK_FOOT  ) ;
	} else {
		NPC_ActStatus( act, NPC_ACT_STATUS_IK_HAND  ) ;
	}

	if ( time == 0 ) {
		int	mot ;
		if ( npc->target.capture->flag & CAPTURE_HEAD ) {
		    mot = NPC_MOT_DRAG_BODY_IDLE ;
		} else {
		    if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
			    mot = NPC_MOT_DRAG_LEG_IDLE ;
		    } else {
			    mot = NPC_MOT_DRAG_LEG_IDLE_B ;
		    }
		}
		NPC_SetActMotion( npc, npc->drag_mar, mot ) ;
	}

	if ( CaptureCheck( npc ) ) {
		UNSET_FLAG( npc->ctrl->skip_flag,  CTRL_SKIP_FLR_CHECK ) ;
		return ;
	}

	if ( time == 0 ) {
		if ( FaintSetStep( cap, npc->body, npc->ctrl, npc->target.drag_shift, 
			npc->action.down_side, -1, TakeTurn( act->current_mot ), act->current_mot ) ) {
			FloorThrough( npc ) ;
			return ;
		}
	} else {
		if ( FaintSetStep( cap, npc->body, npc->ctrl, npc->target.drag_shift, 
			npc->action.down_side, 0, TakeTurn( act->current_mot ), act->current_mot ) ) {
			FloorThrough( npc ) ;
			return ;
		}
	}

	if ( cap->flag & CAPTURE_MOVE ) {
		NPC_SetActMode( npc, NPC_ActFaintMove ) ;
	}
}

void NPC_ActFaintMove( NPCWORK *npc, int time  )
{
	CAPTURE_TARGET	*cap ;
	NPCACT *act ;

	act = &npc->action ;
	cap = npc->target.capture ;

	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_FAINT  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;

    if ( cap->flag & CAPTURE_HEAD ) {
		NPC_ActStatus( act, NPC_ACT_STATUS_IK_FOOT  ) ;
	} else {
		NPC_ActStatus( act, NPC_ACT_STATUS_IK_HAND  ) ;
	}

	if ( time == 0 ) {
		int	mot ;
		if ( npc->target.capture->flag & CAPTURE_HEAD ) {
		    mot = NPC_MOT_DRAG_BODY_WALK ;
		} else {
		    if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
			    mot = NPC_MOT_DRAG_LEG_WALK ;
		    } else {
			    mot = NPC_MOT_DRAG_LEG_WALK_B ;
		    }
		}
		NPC_SetActMotion( npc, npc->drag_mar, mot ) ;
	}
	if ( CaptureCheck( npc ) ) {
		UNSET_FLAG( npc->ctrl->skip_flag,  CTRL_SKIP_FLR_CHECK ) ;
		return ;
	}

	if ( FaintSetStep( cap, npc->body, npc->ctrl, npc->target.drag_shift, 
		npc->action.down_side, 0, TakeTurn( act->current_mot ), act->current_mot ) ) {
		FloorThrough( npc ) ;
		return ;
	}

	if ( !( cap->flag & CAPTURE_MOVE ) ) {
		NPC_SetActMode( npc, NPC_ActFaintTake ) ;
	    return ;
	}
}

void NPC_ActFaintDown( NPCWORK *npc, int time  )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_FAINT  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;
	NPC_ActStStatus( act, NPC_ACT_STST_UNDER_NEARCHECK ) ;

	if ( time == 0 ) {
		int	mot ;
		if ( npc->target.capture->flag & CAPTURE_HEAD ) {
		    mot = NPC_MOT_DRAG_BODY_END ;
		} else {
		    if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
			    mot = NPC_MOT_DRAG_LEG_END ;
		    } else {
			    mot = NPC_MOT_DRAG_LEG_END_B ;
		    }
		}
		NPC_SetActMotion( npc, npc->drag_mar, mot ) ;
		MT_SetMotionSpeed( npc->body->m_ctrl, ( float )TIME_BASE*2.0f ) ;
		UNSET_FLAG( npc->ctrl->skip_flag,  CTRL_SKIP_FLR_CHECK ) ;
	}
//printf("faint down time[%d] \n",time);

	NPC_SetSlopeRotX( npc->ctrl ) ;

	if ( (npc->ctrl->mov.vy - npc->ctrl->levels[0]) < 500 ) {
//		SetSlopeRotX( act ) ;
		NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN  ) ;
	}
	if ( time == COUNT_VMODE(30/2) ) {
		if ( act->dogtag_id >= 0 && act->dogtag_item.c_proc == 0 ) {
			KRTH_PutDogTagItemProc( &act->dogtag_item, &npc->ctrl->mov, 0, 1.0f, act->dogtag_id ) ;
			act->sw_dogtag = -1 ;
		} else {
			float n ;
			
			n = ( npc->target.capture->flag & CAPTURE_HEAD )? 1.0f : 2.0f ;
			KRTH_PutItemProc( &act->item, &npc->ctrl->mov, 0, n ) ;
		}
	}

	if ( GM_CheckObject_IsEnd( npc->body, 0 ) ) {
		NPC_SetActMode( npc, NPC_ActFaint ) ;
	    if ( FrontCheck( npc->body ) ){
			npc->target.capture->flag = CAPTURE_FRONT ;
			npc->action.down_side = NPC_DOWN_SIDE_F ;
		} else {
			npc->target.capture->flag = CAPTURE_BACK ;
			npc->action.down_side = NPC_DOWN_SIDE_B ;
		}
	    return ;
	}
}

void NPC_ActFaintDamage( NPCWORK *npc, int time  )
{
	NPCACT *act ;

	act = &npc->action ;

	NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN|NPC_ACT_STATUS_FAINT  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;

	/* ダメージモーション中はダメージを受けない */
	NPC_ActStatus( act, NPC_ACT_STATUS_DAMAGE|NPC_ACT_STATUS_TRG_OFF  ) ;
//	SetSlopeRotX( act ) ;

	if ( act->life <= 0 ) {
		if ( time<COUNT_VMODE(32)  && !(time%COUNT_VMODE(8)) ) {
			NPC_ActStatus( act, NPC_ACT_STATUS_IK_PIKU ) ;
		}
	} else {
		if ( time == 0 ) {
			NPC_ActStatus( act, NPC_ACT_STATUS_IK_PIKU ) ;
		}
		if ( FaintTrgCheck( npc ) ) return ;
	}

	if ( act->life > 0 ) {
		if ( time>COUNT_VMODE(30)  ) {
//			act->bodyp.faint_time -= DOWN_FAINT_DAMAGE ;
			NPC_SetActMode( npc, NPC_ActFaint ) ;
			return ;
		}
	} else {
		if ( time>COUNT_VMODE(120)  ) {
			NPC_SetActMode( npc, NPC_ActDeath ) ;
			return ;
		}
	}
}

void NPC_ActFaintEnd( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN|NPC_ACT_STATUS_FAINT_END  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;

	if ( time == 0 ) {
		NPC_CallHeadMark( npc, HMK2_TYPE_KILL  ) ;

	    if ( npc->action.down_side == NPC_DOWN_SIDE_F ) {
			NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_DOWN_F ) ;
		} else {
			NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_DOWN_B ) ;
		}
		npc->target.deftrg->class = DEF_TARGET_CLASS ;
	}

	if ( npc->CheckPad( npc ) ) return ;
}

void NPC_ActDeath( NPCWORK *npc, int time )
{
	NPCACT *act ;

	act = &npc->action ;
	NPC_ActStatus( act, NPC_ACT_STATUS_NPCSYS_DAM  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_IK_DOWN|NPC_ACT_STATUS_DEATH  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_TRG_OFF  ) ;
	NPC_ActStatus( act, NPC_ACT_STATUS_EYE_CLOSE  ) ;

	if ( time == 0 ) {
		NPC_SetActMotion( npc, npc->capture_mar, NPC_MOT_DOWN_F ) ;
		npc->target.capture->flag = CAPTURE_FRONT ;
		npc->target.deftrg->class = FAINT_TARGET_CLASS ;
	}

	if ( npc->CheckPad( npc ) ) return ;
}
/*----------------------------------------------------------------*/
void NPC_Action( NPCWORK *npc )
{
	int	time ;
	NPCACT *act ;

	act = &npc->action ;

	act->act_end = 0 ;
//	npc->old_status = act->status ;
	act->status = 0 ;
	act->status_status = 0 ;
	UNSET_FLAG( npc->body->flag, OBJECT_MOTIONSTEP_THROUGH ) ;

	time = act->time ++ ;

	if ( npc->actmode_call == NULL ) return ;
	( *npc->actmode_call )( npc, time ) ;
}

/*----------------------------------------------------------------*/
static void NPC_MotionAdjustAim( body, trg_rot, dir, parts, interp )
OBJECT		*body ;
SVECTOR		*trg_rot ;
int			dir ;
int			parts ;
int			interp ;
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR		quat ;
	SVECTOR		rot ;

	m_ctrl = body->m_ctrl ;

	rot.vx = 0 ;
	rot.vy = 0 ;
	rot.vz = 0 ;

	if ( parts & ADJ_PARTS_HEAD ) {
		rot.vx = trg_rot->vx/2 ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ HUMAN21_MUNE ] = quat ;
		m_ctrl->adjust_flag |= 1 << HUMAN21_MUNE ;


		rot.vy = trg_rot->vy ;
		rot.vx = trg_rot->vx*2 ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ HUMAN21_KUBI ] = quat ;
		m_ctrl->adjust_flag |= 1 << HUMAN21_KUBI ;
		rot.vy = trg_rot->vy ;
		rot.vx = trg_rot->vx ;
		GM_RotToQuat( &rot, &quat ) ;
		m_ctrl->adjust[ HUMAN21_ATAMA ] = quat ;
		m_ctrl->adjust_flag |= 1 << HUMAN21_ATAMA ;
	}
}

static void NPC_SitMotionAdjust( NPCWORK *npc )
{
//	static	ROTLIMIT	rlim = { -1024, 1024, -768, 768 } ;
	static	ROTLIMIT	rlim = { -512, 512, -768, 768 } ;
	NPCADJUST	*nadj ;
	FVECTOR		trg, pos, vec ;
	SVECTOR		rot ;
	FMATRIX		*w ;
	int			flag, interp, face_x, face_y ;

	nadj = npc->nadj ;
	if ( nadj->adj_status & NPC_ADJ_ON ) {
		w = &(npc->body->objs->objs[HUMAN21_KUBI].world) ;
//		w = &(npc->body->objs->objs[HUMAN21_ATAMA].world) ;

		trg = nadj->aim_pos ;
		KR_FMatToFvec( w, &pos ) ;
		face_x = MatToXRot( w ) ;
		face_y = MatToYRot( w ) ;

		_sceVu0SubVector(  &vec, &trg, &pos ) ;
		_FVecToRotXY( &vec, &rot ) ;

		rot.vx -= 1024 ;
		if ( rot.vx < rlim.upper ) rot.vx = rlim.upper ;
		if ( rot.vx > rlim.lower ) rot.vx = rlim.lower ;

		/* 体の向きからの角度に変換 */
		rot.vy = (short)GV_DiffDirS( npc->ctrl->rot.vy, rot.vy ) ;
		face_y = (short)GV_DiffDirS( npc->ctrl->rot.vy, face_y ) ;

		if ( rot.vy < rlim.right ) rot.vy = rlim.right ;
		if ( rot.vy > rlim.left )  rot.vy = rlim.left ;

		rot.vx -= face_x ;
		rot.vy -= face_y ;

		if ( nadj->adj_status & NPC_ADJ_KUBIFURI ) {
			rot.vx += (float)KR_RandS( 32 ) ;
		}

		nadj->adj_rot.vx = GV_NearTime( nadj->adj_rot.vx, rot.vx, 8 ) ;
		nadj->adj_rot.vy = GV_NearTime( nadj->adj_rot.vy, rot.vy, 8 ) ;

		flag = ADJ_PARTS_HEAD ;
		interp = 0 ;
		NPC_MotionAdjustAim( npc->body, &nadj->adj_rot, npc->ctrl->rot.vy, flag , interp) ;
	} else {
		if ( nadj->adj_rot.vx != 0 || nadj->adj_rot.vy != 0 ) {
			nadj->adj_rot.vx = GV_NearExp8( nadj->adj_rot.vx, 0 ) ;
			nadj->adj_rot.vy = GV_NearExp8( nadj->adj_rot.vy, 0 ) ;
			flag = ADJ_PARTS_HEAD ;
			interp = 0 ;
			NPC_MotionAdjustAim( npc->body, &nadj->adj_rot, npc->ctrl->rot.vy, flag , interp) ;
		} else {
			npc->body->m_ctrl->adjust_flag = 0 ;
		}
	}
}

static void NPC_MotionAdjust( NPCWORK *npc )
{
//	static	ROTLIMIT	rlim = { -1024, 1024, -768, 768 } ;
	static	ROTLIMIT	rlim = { -512, 512, -768, 768 } ;
	NPCADJUST	*nadj ;
	FVECTOR		trg, pos, vec ;
	SVECTOR		rot ;
	FMATRIX		*w ;
	int			flag, interp, face_x, face_y ;

	nadj = npc->nadj ;
	if ( nadj->adj_status & NPC_ADJ_ON ) {
		w = &(npc->body->objs->objs[HUMAN21_KUBI].world) ;
//		w = &(npc->body->objs->objs[HUMAN21_ATAMA].world) ;

		trg = nadj->aim_pos ;
		KR_FMatToFvec( w, &pos ) ;
		face_x = MatToXRot( w ) ;
		face_y = MatToYRot( w ) ;

		_sceVu0SubVector(  &vec, &trg, &pos ) ;
		_FVecToRotXY( &vec, &rot ) ;

		rot.vx -= 1024 ;
		if ( rot.vx < rlim.upper ) rot.vx = rlim.upper ;
		if ( rot.vx > rlim.lower ) rot.vx = rlim.lower ;

		/* 体の向きからの角度に変換 */
		rot.vy = (short)GV_DiffDirS( npc->ctrl->rot.vy, rot.vy ) ;
		face_y = (short)GV_DiffDirS( npc->ctrl->rot.vy, face_y ) ;

		if ( rot.vy < rlim.right ) rot.vy = rlim.right ;
		if ( rot.vy > rlim.left )  rot.vy = rlim.left ;

		rot.vx -= face_x ;
		rot.vy -= face_y ;

		if ( nadj->adj_status & NPC_ADJ_KUBIFURI ) {
			rot.vx += (float)KR_RandS( 128 ) ;
		}

		nadj->adj_rot.vx = GV_NearTime( nadj->adj_rot.vx, rot.vx, 8 ) ;
		nadj->adj_rot.vy = GV_NearTime( nadj->adj_rot.vy, rot.vy, 8 ) ;

		flag = ADJ_PARTS_HEAD ;
		interp = 0 ;
		GM_MotionAdjustAim( npc->body, &nadj->adj_rot, npc->ctrl->rot.vy, flag , interp) ;
	} else {
		if ( nadj->adj_rot.vx != 0 || nadj->adj_rot.vy != 0 ) {
			nadj->adj_rot.vx = GV_NearExp8( nadj->adj_rot.vx, 0 ) ;
			nadj->adj_rot.vy = GV_NearExp8( nadj->adj_rot.vy, 0 ) ;
			flag = ADJ_PARTS_HEAD ;
			interp = 0 ;
			GM_MotionAdjustAim( npc->body, &nadj->adj_rot, npc->ctrl->rot.vy, flag , interp) ;
		} else {
			npc->body->m_ctrl->adjust_flag = 0 ;
		}
	}
}

static void AdjustPiku( NPCWORK *npc )
{
	static short asjpiku_buff[]={ 16, 64, 128, 192, 256, 128} ;
	SVECTOR	rot ;
	NPCACT	*act ;
   int adj_piku_value;
	act = &npc->action ;

   // Arm fix (for 30fps):
   // if value > 0, use it, otherwise use time
   // part of a fix for MGSTWO-3181
   if (act->adj_piku_value > 0)
   {
      adj_piku_value = act->adj_piku_value;
   }
   else
   {
      adj_piku_value = act->adj_piku_time;
   }

	act->adj_piku_time -- ;
   adj_piku_value -- ;

   // Arm fix (for 30fps):
   // reset the piku_value
   // part of a fix for MGSTWO-3181
   if (act->adj_piku_time == 0)
   {
      act->adj_piku_value = 0;
   }

	if ( act->adj_piku_time > 5 ) return ;

	rot.vx = -asjpiku_buff[ adj_piku_value ] ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	GM_AdjustRotBody( npc->body, &rot, 1  ) ;
	rot.vx = asjpiku_buff[ adj_piku_value ] ;
	GM_AdjustRotBody( npc->body, &rot, 11  ) ;
	GM_AdjustRotBody( npc->body, &rot, 3  ) ;
	GM_AdjustRotBody( npc->body, &rot, 7  ) ;
}

void NPC_ActStatusCheck( NPCWORK *npc )
{
	NPCACT	*act ;
	CONTROL *ctrl ;
	int	status, correct ;

	act = &npc->action ;
	status = act->status ;
	ctrl = npc->ctrl ;

	/* ターゲット判定のＯＮ，ＯＦＦ */
	if ( status & NPC_ACT_STATUS_TRG_OFF ) {
		SET_FLAG( npc->target.deftrg->class, TARGET_SKIP ) ;
	} else {
		UNSET_FLAG( npc->target.deftrg->class, TARGET_SKIP ) ;
	}

	/* キャプチャーターゲットのＯＮ，ＯＦＦ */
	if ( npc->target.capture != NULL ) {
		if ( status & NPC_ACT_STATUS_CAPTURE_OFF ) {
			UNSET_FLAG( npc->target.deftrg->class, TARGET_CAPTURE ) ;
		} else {
			SET_FLAG( npc->target.deftrg->class, TARGET_CAPTURE ) ;
		}
	} else {
		UNSET_FLAG( npc->target.deftrg->class, TARGET_CAPTURE ) ;
	}
	/* 心臓の音 */
	if ( act->heart_beat != 0 ) {
		if ( !(GM_PlayTime%act->heart_beat) ) {
			GM_SeSetMode( SD_E_EHEART02, &npc->ctrl->mov, GM_SEMODE_MIC ) ;
		}
	}

	/* 気絶値の更新 */
	if ( act->faint_count > 0 ) {
		if ( act->faint_mode == NPC_FAINT_ZZZ ) {
			if ( act->headmark_num == 4+1 ) {
				act->headmark_num = 4 ;
			} else if ( act->faint_count < COUNT_VMODE(60*60) &&
				 act->headmark_num != 1 ) {
				*act->headmark = HMK2_TYPE_VALUE|0 ;
				act->headmark_num = 1 ;
				CallActHeadMarks( npc->headmark, *act->headmark );
printf("HEAD Mark Anes 1 !! \n");
			} else if ( act->faint_count >= COUNT_VMODE(60*60) &&
						act->faint_count < COUNT_VMODE(60*60*2) &&
						act->headmark_num != 2 ) {
				*act->headmark = HMK2_TYPE_VALUE|1 ;
				act->headmark_num = 2 ;
				CallActHeadMarks( npc->headmark, *act->headmark );
printf("HEAD Mark Anes 2 !! \n");
			} else if ( act->faint_count >= COUNT_VMODE(60*60*2) &&
						act->faint_count < COUNT_VMODE(60*60*3) &&
						act->headmark_num != 3 ) {
				*act->headmark = HMK2_TYPE_VALUE|2 ;
				act->headmark_num = 3 ;
				CallActHeadMarks( npc->headmark, *act->headmark );
printf("HEAD Mark Anes 3 !! \n");
			} else if ( act->faint_count >= COUNT_VMODE(60*60*3) &&
						act->headmark_num != 4 ) {
				*act->headmark = HMK2_TYPE_VALUE|3 ;
				act->headmark_num = 4 ;
				CallActHeadMarks( npc->headmark, *act->headmark );
printf("HEAD Mark Anes 4 !! \n");
			}
		} else {
			if ( act->headmark_num == 3+1 ) {
				act->headmark_num = 3 ;
			} else if ( act->faint_count < (act->faint_max/3) && act->headmark_num != 1 ) {
printf("head mark num change 1 !! \n");
				*act->headmark = HMK2_TYPE_PIYO_VALUE|1 ;
				act->headmark_num = 1 ;
				CallActHeadMarks( npc->headmark, *act->headmark );
			} else if ( act->faint_count >= (act->faint_max/3) && 
						act->faint_count < (act->faint_max*2/3) &&
						act->headmark_num != 2 ) {
printf("head mark num change 2 !! \n");
				*act->headmark = HMK2_TYPE_PIYO_VALUE|2 ;
				act->headmark_num = 2 ;
				CallActHeadMarks( npc->headmark, *act->headmark );
			}
		}
		act->faint_count -- ;
	}

	/* ターンフラグ */
	if( ( correct = npc->body->m_ctrl->rot_correct ) ){
		printf("correct[%d] rot[%d]\n",correct, ctrl->rot.vy);
		ctrl->rot.vy += correct;
		ctrl->rot.vy &=4095;
		ctrl->turn.vy = ctrl->rot.vy ;

		/* X軸方向の傾きは体が正面を向いている場合しかしない */
		if ( correct > 1024 || correct < -1024) {
			ctrl->rot.vx = ctrl->turn.vx = - ctrl->turn.vx ;
		}
		npc->body->m_ctrl->rot_correct = 0;
	}

	if ( npc->nadj != NULL ) {
		if ( npc->status & NPC_STATUS_HOSTAGE ) {
			NPC_SitMotionAdjust( npc ) ;
		} else {
			NPC_MotionAdjust( npc ) ;
		}
	}

	if ( act->adj_piku_time > 0 ) {
		/* すべてのアジャスト操作後 */
		AdjustPiku( npc ) ;
	}

	if ( npc->hom != NULL ) {
		if ( status & NPC_ACT_STATUS_HOMING_OFF ) {
			SET_FLAG( npc->hom->status, HOMING_SKIP ) ;
		} else {
			UNSET_FLAG( npc->hom->status, HOMING_SKIP ) ;
		}
	}
	/* 倒れたときの低い床抜けチェック */
	/* 倒れたときの低い床抜けチェック */
	if ( act->status_status & (NPC_ACT_STST_UNDER_NEARCHECK) ) {
		GM_ConfigControlNearCheck2( npc->ctrl, 195 ) ;
	}
}
/*----------------------------------------------------------------*/
void NPC_InitAction( NPCWORK *npc, NPCACT *act )
{
	npc->actmode_call = NPC_ActStandStill ;

	act->time = 0 ;
	act->act_end = 0 ;
	act->status = 0 ;

	act->sleep = 0 ;
	act->life = 0 ;
	act->faint = 0 ;
	act->faint_count = 0 ;
	act->faint_mode = 0 ;
	act->faint_max = 0 ;
	act->sleep_max = 0 ;
	act->down_dam = 0 ;

	act->face_dir = 0 ;
	act->eye_range = 0 ;
	act->eye_sight = 0 ;
	act->radar_color = 0 ;

	act->ikwork = NULL ;
	act->headmark = NULL ;
	act->face_anime = 0 ;

	act->item.n_proc = 0 ;
	act->item.c_proc = 0 ;
	act->heart_beat = NPC_HEART_BEAT_NORMAL ;

   // Arm fix:
   // initialize these values
   act->adj_piku_time = 0;
   act->adj_piku_value = 0;
}

void NPC_SetActionParam( NPCACT *act, int life, int faint, int faint_max, 
		int sleep_max, int down_dam )
{
	act->sleep = 0 ;
	act->life = life ;
	act->faint = faint ;
	act->faint_max = faint_max ;

	act->sleep_max = sleep_max ;
	act->down_dam = down_dam ;
}

void NPC_SetHeartBeat( NPCWORK *npc, int beat )
{
	npc->action.heart_beat = beat ;
}
