//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	notice.c

	2000/04/01 Y.Korekado
	$Id: notice.c,v 1.1.1.3 2002/11/19 11:44:19 Yoshizawa1 Exp $
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

#include	"BP_Debug.h"   //BP_ENEMY

static void NoticeHoldUp( entk )
ENETHINK	*entk;
{
	if ( COM_GetNoiseNearID( entk  ) == NOISE_HOLD ) {
		entk->notice |= ENE_NOTICE_HOLDUP ;
	}
}

static void NoticeNoise( entk )
ENETHINK	*entk;
{
	switch ( COM_GetNoiseNearID( entk  ) ) {
		case NOISE_SCREAM :
		case NOISE_SS :
		case NOISE_S :
		case NOISE_MM :
			entk->notice |= ENE_NOTICE_NOISE ;
			break ;
		case NOISE_M :
			entk->notice |= ENE_NOTICE_NOISE_M ;
			break ;
		case NOISE_L :
			entk->notice |= ENE_NOTICE_NOISE_L ;
			break ;
	}
}

static void NoticeFound( entk )
ENETHINK	*entk;
{
	ENEFINDLIST	*efl ;
	ENEFIND *ef ;
	int 	dis, dir ;

	/* 既にオブジェクトモードならチェックしない。entk->found_ef_id書き換え防止 */
//	if ( entk->c_notice & ENE_NOTICE_FOUND ) return ;

	efl = &GM_EneFindList ;

	if( efl->start == NULL ) return ;

	ef = efl->start ;
	while( ef != NULL ) {
//printf("EF ef->id[%d]  ef->type[%x]\n",ef->id, ef->type)  ;
		if( !(ef->type & (EF_TYPE_NO_FIND|EF_TYPE_FOUND)) ) {
			if( ENE_EneFindSightIn( entk, ef, &dis, &dir ) ) {
				entk->notice |= ENE_NOTICE_FOUND ;
				entk->found_ef_id = ef->id ;
//printf("ChEEEEEEEEEEEEECK ef->id[%d]\n",ef->id)  ;

				return ;	/* 一個発見したら終わり */
			}
		}
		ef = ef->next ;
	}
}

static void NoticeFoundLink( entk )
ENETHINK	*entk;
{
	ENEFINDLISTLIST	*efll ;
	ENEFINDLIST		*efl ;
	ENEFIND 		*ef ;
	int 	dis, dir ;

	/* 既に追跡モードならチェックしない。entk->efl_id書き換え防止 */
	if ( entk->c_notice & ENE_NOTICE_TRACE ) return ;

	efll = &GM_EneFindListList ;

	if( efll->start == NULL ) return ;

	efl = efll->start ;
	while( efl != NULL ) {
		if( (efl->listtype & (EF_LIST_TYPE_FOOT|EF_LIST_TYPE_BLOOD)) 
			&& !(efl->listtype & EF_LSIT_TYPE_TRACE) ) {
			ef = efl->start ;
			while( ef != NULL ) {
				if( !(ef->type & EF_TYPE_FOUND) ) {
					if( ENE_EneFindSightIn( entk, ef, &dis, &dir ) ) {
						entk->notice |= ENE_NOTICE_TRACE ;
						entk->efl_id = efl->id ;
						entk->ef_id = ef->id ;
//printf("notice: go trace efl[%d] ef[%d] \n",entk->efl_id,entk->ef_id ) ;

						return ;	/* 一個発見したら終わり */
					}
				}
				ef = ef->next ;
			}
		}
		efl = efl->next ;
	}
}

/* NoticeUniformより先にチェックすること */
static void NoticeEnemyDamage( entk )
ENETHINK	*entk;
{
	ENETHINK	*dam_entk ;
	
	if ( ( dam_entk = COM_GetDamageEnemy( entk )) != NULL ) {
		if ( entk->status & ENE_STATUS_ACCDNT_REPO ) {
			if ( dam_entk->iknow_flag & IKNOW_HELP_ME_REPO ) return ;
			if ( dam_entk->act->status & ACT_STATUS_FAINT ) {
				entk->notice |= ENE_NOTICE_ACCDNT_REPO ;
				entk->dam_entk = dam_entk ;
			}
		} else {
			entk->notice |= ENE_NOTICE_ENE_DAMAGE ;
			entk->dam_entk = dam_entk ;
		}
//printf("[%d]Notice Damage Enemy[%d] [%d]\n",GV_Time, entk->id, entk->dam_entk->id ) ;
	}
}

static void NoticeCorp( entk )
ENETHINK	*entk;
{
	int	uniq_id ;

	if ( !(entk->c_notice & ENE_NOTICE_CORP) ) {
		if ( (uniq_id = COM_GetCorpID( entk )) >= 0 ) {
			entk->notice |= ENE_NOTICE_CORP ;
			if ( entk->status & ENE_STATUS_ACCDNT_REPO ) {
				entk->notice |= ENE_NOTICE_ACCDNT_REPO ;
			}
			entk->corp_id = uniq_id ;
		}
	}
}

static void NoticeLight( entk )
ENETHINK	*entk;
{
	extern int PL_GetUSPLightOn( void ) ;
	SENSEPARAM *sens ;
	int	d_range, p2e_dir ;

	/* ライトはプレイヤーが持つ物に限る */
//	if ( !(GM_PlayerStatus & PLAYER_HOLD) || !(IT_GunLight2 == Ply_GetPlayerItem()) ) return ;
	if ( !PL_GetUSPLightOn( ) ) return ;

	sens = &entk->sense ;

	/* 直接なら見える距離 */
	if ( entk->pl_eyei.dis < EYE_LIGHT_DIS ) {
		/* 自分に光があたる距離 */
		if ( entk->pl_eyei.dis < EYE_LIGHT_NEARDIS ) {
			p2e_dir = 4095 & (entk->pl_eyei.dir + 2048) ;
			d_range = _DiffDirAbs( p2e_dir, GM_PlayerControl->rot.vy ) ;
			if ( d_range < EYE_LIGHT_RANGE ) {
				/* zonecheck */
				if ( ENE_EyeOnlineCheck( entk->ctrl->hzx_id, entk->pl_eyei.pos, &entk->ctrl->mov ) ){
					return ;
				}
				entk->notice |= ENE_NOTICE_LIGHT ;
//printf(" NEAR LIGHT!! \n");
				return ;
			}
		}
		if ( _DiffDirAbs( sens->facedir, entk->pl_eyei.dir ) < sens->eye_r ) {
			if ( ENE_EyeOnlineCheck( entk->ctrl->hzx_id, entk->pl_eyei.pos, &entk->ctrl->mov ) ){
				return ;
			}
			entk->notice |= ENE_NOTICE_LIGHT ;
//printf(" DIRECT LIGHT!! \n");
			return ;
		}
	}
}

static void NoticeTouch( entk )
ENETHINK	*entk;
{
	if ( entk->act->status & (ACT_STATUS_MASUI_SASARU|ACT_STATUS_MECABREAK) ) {
		entk->notice |= ENE_NOTICE_TOUCH ;
	}

	if ( (PL_GetPlayerItem() != IT_Uniform) ||
		 !(GM_PlayerStateFlag & PL_GBSCAP_EXIST) ) {
		if ( ENE_InRangeXYZ( &entk->ctrl->mov, &GM_PlayerPosition, 
				250, 1500, 250 ) ) {
			entk->notice |= ENE_NOTICE_NEAR_TOUCH ;
		}
	}

	if ( GM_PlayerStatus & PLAYER_GROUND ) {
		FVECTOR pos, floor_pos ;
		
		floor_pos = entk->ctrl->mov ;
		floor_pos.vy = entk->ctrl->levels[0] ;
		
		KR_FMatToFvec( &BODYWORLD( GM_PlayerBody, HUMAN21_ATAMA ), &pos ) ;
		if ( ENE_InRangeXYZ( &floor_pos, &pos, 250, 250, 250 ) ) {
			entk->notice |= ENE_NOTICE_NEAR_TOUCH ;
		}
		KR_FMatToFvec( &BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_ASHI2 ), &pos ) ;
		if ( ENE_InRangeXYZ( &floor_pos, &pos, 250, 250, 250 ) ) {
			entk->notice |= ENE_NOTICE_NEAR_TOUCH ;
		}
	}
	

}

/* プレイヤー部位発見モード */
static float	SubZoneY( addr1, addr2 )
int	addr1 ;
int addr2 ;
{
	HZX_ZON		*z1, *z2 ;

	z1 = ENE_HZX_GetZone( addr1 ) ;
	z2 = ENE_HZX_GetZone( addr2 ) ;
	
	return z1->y - z2->y ;
}

//#define VIEWZONE (1)
static void NoticeRegion( entk )
ENETHINK	*entk;
{
	u_int	findobj, region_detect ;
	int		objnum, zone, myzone, near, grpid ;
	FVECTOR	pos ;

#ifdef VIEWZONE
DBG_ViewZone( HZX_Zone1(GM_PlayerControl->addr), GM_PlayerControl->hzx_id, 64, 128, 64 ) ;
#endif

	region_detect = 0 ;

	/* プレイヤーの情報のみチェック */
	if ( entk->pl_eyei.pos != &GM_PlayerFindPos ) 	goto end ;

	/* 同マップのみチェック */
	if ( GM_PlayerControl->hzx_id != entk->ctrl->hzx_id )	goto end ;

	/* 変装中はノーチェック */
	if ( (PL_GetPlayerItem() == IT_Uniform) &&
		 (GM_PlayerStateFlag & PL_GBSCAP_EXIST) )	goto end ;

	/* エルード中は無し */
	if ( GM_PlayerStatus & (PLAYER_BEYOND) ) goto end ;

	/* 部位チェック */
	zone = HZX_Zone1( GM_PlayerControl->addr ) ;
	myzone = HZX_Zone1( entk->ctrl->addr ) ;
	grpid = entk->ctrl->hzx_id ;
	findobj = GM_PlayerFindObj ;

	/* プレイヤーとの間に障害物が有って見えない場合ではないと影のみチェック */
	if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_OUT_HZD )	goto shadow ;

	while( findobj ) {
		objnum = GV_GetNo( findobj ) ;
		KR_FMatToFvec( &BODYWORLD( GM_PlayerBody, objnum ), &pos ) ;

		/* 部位が隣接ゾーンに入っているかチェック */
		if ( (near = ENE_InNearZone( &pos, zone, grpid )) >= 0 ) {

			/* 今いるゾーンから目標のゾーンが見えるかチェック */
			if ( !(ENE_ReadOnlinInfo( entk->ctrl->addr, HZX_Address(grpid,near,near))) ) {
#if 1
				if ( !(ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &pos, &entk->ctrl->mov )) ){
//printf("kore0\n");
					region_detect = 1 ;
#ifdef VIEWZONE
DBG_ViewZone( myzone, grpid, 200, 8, 64 ) ;
#endif
					goto end ;
				}
#else
				if ( ENE_ABSf(SubZoneY( entk->ctrl->addr, HZX_Address(grpid,near,near) ) ) > 1500.0F ) {
					if ( !(ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &pos, &entk->ctrl->mov )) ){
//printf("kore1\n");
						region_detect = 1 ;
#ifdef VIEWZONE
DBG_ViewZone( myzone, grpid, 200, 8, 64 ) ;
#endif
						goto end ;
					}
				} else {
//printf("kore2\n");
					region_detect = 1 ;
#ifdef VIEWZONE
DBG_ViewZone( myzone, grpid, 200, 8, 64 ) ;
#endif
					goto end ;
				}
#endif
			}
			else {
#ifdef VIEWZONE
DBG_ViewZone( myzone, grpid, 200, 200, 200 ) ;
DBG_ViewZone( near, grpid, 200, 200, 200 ) ;
#endif
			}
		} else {
			/* 入ってなければOnlineチェック */
			if ( !(ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &pos, &entk->ctrl->mov )) ){
//printf("kore3\n");
				region_detect = 1 ;
#ifdef VIEWZONE
DBG_ViewZone( myzone, grpid, 200, 8, 64 ) ;
#endif
				goto end ;
			}
		}
		findobj = GV_DeletBit( findobj, objnum ) ;
	}

shadow :
	if ( entk->pl_eyei.sight >= EYE_INFO_SIGHT_BLURR )	goto end ;

	/* 影チェック */
	/* 体験版後段ボールの影なら見つけない様にする */
	findobj = GM_N_PlayerShadowPos ;
	while( findobj-- ) {
//printf("shadow check num[%d]\n",findobj ) ;
		pos = GM_PlayerShadowPos[ findobj ]  ;

		if ( entk->pl_eyei.sight != EYE_INFO_SIGHT_OUT_HZD ) {
			EYEINFO		eyei ;

			ENE_SetEyeInfo( &eyei, &pos, &GM_PlayerHzxID, 0, &GM_PlayerHzxID ) ;
			ENE_EyeInfoCheck( entk, &eyei ) ;

			//printf(" sight[%d] dis[%d] dir[%d] \n",eyei.sight, eyei.dis, eyei.dir ) ;
			if ( eyei.sight != EYE_INFO_SIGHT_IN || eyei.dis > entk->sense.eye_s ) goto end ;
		}

#ifdef DEBUG_MODE
#if 0
{
	extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
	FVECTOR	lin[2] ;
	lin[0] = pos ;
	KR_FMatToFvec( &BODYWORLD(entk->act->body, HUMAN21_ATAMA), &lin[1] ) ;
	NewLineView(  &lin[0],1,0,0,255) ;
}
#endif
#endif
		/* 部位が隣接ゾーンに入っているかチェック */
		if ( (near = ENE_InNearZone( &pos, zone, grpid )) >= 0 ) {
			/* 今いるゾーンから目標のゾーンが見えるかチェック */
			if ( !(ENE_ReadOnlinInfo( entk->ctrl->addr, HZX_Address(grpid,near,near))) ) {
				if ( ENE_ABSf(SubZoneY( entk->ctrl->addr, HZX_Address(grpid,near,near) ) ) > 1500.0F ) {
					if ( !(ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &pos, &entk->ctrl->mov )) ){
						region_detect = 1 ;
						goto end ;
					}
				} else {
					region_detect = 1 ;
					goto end ;
				}
			}
		} else {
			/* 入ってなければOnlineチェック */
			if ( !(ENE_EyeOnlineCheck( entk->ctrl->hzx_id, &pos, &entk->ctrl->mov )) ){
				region_detect = 1 ;
				goto end ;
			}
		}
	}

end :

	if ( region_detect ) {
		if ( ++entk->region_count > 10000 ) entk->region_count = 10000 ;
		if ( entk->region_count > ENE_REGION_DETECT_COUNT ) {
			entk->notice |= ENE_NOTICE_REGION ;
		}
	} else {
		entk->region_count = 0 ;
	}
}

/*ダンボールぶつかりダメージの距離は
	player : 250
	enemy : 50
	max : 424.264068(300*1.41421356)
*/
static void NoticeBox( entk )
ENETHINK	*entk;
{
	if ( !(GM_PlayerStatus & PLAYER_CB_BOX) )	return ;	/* 段ボール被っていない */
	if ( GM_PlayerStatus & PLAYER_CB_BOX_STAND )	return ;	/* ダンボール被って立っている */
	if ( GM_PlayerStatus & PLAYER_CBBOX_RUN )	return ;	/* ダンボール被走っている */
	if ( GM_PlayerStatus & PLAYER_FORCE )	return ;	/* 強制モーション中*/

	entk->pl_eyei.sight = EYE_INFO_SIGHT_OUT ;				/* 見えなかった事にする */

	if ( entk->status & ENE_STATUS_ACCDNT_REPO ) return ;

	if ( ENE_InRangeXYZ( &entk->ctrl->mov, &GM_PlayerPosition, 
			1200, 1500, 1200 ) ) {
		if ( !(ENE_BodyOnlineCheck( entk->ctrl->hzx_id, &GM_PlayerPosition, &entk->ctrl->mov )) ){
			if ( ENE_ABSf( entk->ctrl->levels[0] - GM_PlayerControl->levels[0] ) < 900.0f ) {
				if ( _DiffDirAbs( entk->ctrl->rot.vy, entk->pl_eyei.dir ) < 200 ) {
					entk->notice |= ENE_NOTICE_NEAR_BOX ;
					return ;
				}
				if ( ENE_InRangeXYZ( &entk->ctrl->mov, &GM_PlayerPosition, 
						800, 1500, 800 ) ) {
						entk->notice |= ENE_NOTICE_NEAR_BOX ;
						return ;
				}
			}
		} else {
//printf("body online Hit!!!\n");			
		}
#ifdef DEBUG_MODE
if ( entk->g_id == 111 && entk->id == 0 ) {
printf("mov [%f][%f][%f] pl[%f][%f][%f]\n",
	entk->ctrl->mov.vx,entk->ctrl->mov.vy,entk->ctrl->mov.vz, 
	GM_PlayerPosition.vx,GM_PlayerPosition.vy,GM_PlayerPosition.vz ) ;
}
#endif
	}

	if ( entk->pl_eyei.sight_real < EYE_INFO_SIGHT_BLURR ) return ;

	if ( GM_PlayerStatus & PLAYER_MOVE )	{		/* 動いた */
		entk->notice |= ENE_NOTICE_BOX ;
		return ;
	}

#if 0
	/* 周りのダンボールと柄がちがったら */
	if ( !(entk->c_notice & ENE_NOTICE_BOX) ) {
//		if ( GM_PlayerStatus & PLAYER_CB_BOX_HIDDEN )	{
			entk->notice |= ENE_NOTICE_BOX ;
			return ;
//		}
	}
#else
//	if ( GM_AlertMode == ALERT_MODE_AVOID )	{
	if ( (GM_AlertMode == ALERT_MODE_AVOID) && (GM_GameStatus & STATE_CLEARING) ) {
		if ( GM_PlayerStatus & PLAYER_CB_BOX_HIDDEN )	{
			return ;
		}
		entk->notice |= ENE_NOTICE_BOX ;
	}
#endif
}

/* 危険モードでのダンボール処理 */
static void NoticeBoxAlert( entk )
ENETHINK	*entk;
{
	if ( COM_AlertStatus() & COM_ALERT_PLAYER_DETECT ) return ;

	/* プレイヤーを発見していない場合 */

	if ( !(GM_PlayerStatus & PLAYER_CB_BOX) )	return ;	/* 段ボール被っていない */
	if ( entk->pl_eyei.sight < EYE_INFO_SIGHT_BLURR )	return ;
	if ( GM_PlayerStatus & PLAYER_CB_BOX_STAND )	return ;	/* ダンボール被って立っている */
	if ( GM_PlayerStatus & PLAYER_CBBOX_RUN )	return ;	/* ダンボール被走っている */
	if ( GM_PlayerStatus & PLAYER_FORCE )	return ;	/* 強制モーション中*/
	if ( GM_PlayerStatus & PLAYER_MOVE )	return ;		/* 動いた */

	entk->pl_eyei.sight = EYE_INFO_SIGHT_OUT ;				/* 見えなかった事にする */
}

static void NoticeUniform( entk )
ENETHINK	*entk;
{
	if ( !(PL_GetPlayerItem() == IT_Uniform) ) return ;		/* ユニフォーム着ていない */

	if ( entk->iknow_flag & IKNOW_DRESSED ) return ;	/* 変装見破っている */

	if ( entk->iknow_flag & IKNOW_DETECT ) return ;	/* 発見した */

	/* キャップ無しは視力を2/3にして通常判定 */
	if ( !(GM_PlayerStateFlag & PL_GBSCAP_EXIST) ) return ;

	/* 首しめ、引き摺り */
	if ( GM_PlayerStatus & (PLAYER_ENEMY_HANG|PLAYER_ENEMY_PULL) ) return ;

	/* ホールドアップ実行中 */
	if ( entk->notice & ENE_NOTICE_ENE_DAMAGE ) {
		if ( ENE_PlayerHoldCheck( entk->dam_entk ) ) return ;
		/* ホールドアップじゃなく */
		if ( !(entk->dam_entk->act->status & ACT_STATUS_HOLD_UP) ) {
			/* 見破っていたら */
			if ( entk->dam_entk->iknow_flag & IKNOW_DRESSED ) {
				return ;
			}
		}
	}

	/* プラントゴルステージ */
	if ( COM_StageKind() & ENE_STAGE_GPS ) {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			/* 出血、エルード は発見 */
			if ( (GM_PlayerStatus & PLAYER_BLOOD_DROP) 			/* 出血している */
				 || (GM_PlayerStatus & PLAYER_BEYOND) ) {			/* エルード */
				return ;
			}
			/* しゃがみ、匍匐、回転、構え は発見 */
			if ( (GM_PlayerStatus & (PLAYER_SQUAT|PLAYER_GROUND|PLAYER_ROLLING|PLAYER_HOLD)) ||
				GM_CheckPlayerStatusEX( I64(0), (PLAYER2_COMBO|PLAYER2_KNOCK_WALL|PLAYER2_JUMP) ) ) {
				return ;
			}
			/* AKじゃなかったら 発見 */
			if ( PL_GetPlayerWeapon() != WP_Aks ) {
				return ;
			}
			/* 視力1/2以内なら発見 */
			if ( entk->pl_eyei.dis <= entk->pl_eyei.sight_dis/2 ) {
				return ;
			}

			entk->notice |= ENE_NOTICE_UNIFORM ;
			entk->pl_eyei.sight = EYE_INFO_SIGHT_OUT ;	/* 見えなかった事にする */
			return ;
		}
	} else {
		if ( entk->pl_eyei.sight == EYE_INFO_SIGHT_IN ) {
			if ( !(PL_GetPlayerWeapon() == WP_Aks)  				/* AKを装備していない */
				 || (GM_PlayerStatus & PLAYER_BLOOD_DROP) 			/* 出血している */
				 || (GM_PlayerStatus & PLAYER_BEYOND) ) {			/* エルード */
				if ( entk->pl_eyei.dis > entk->pl_eyei.sight_dis/2 ) {
					entk->notice |= ENE_NOTICE_UNIFORM ;
					entk->pl_eyei.sight = EYE_INFO_SIGHT_OUT ;	/* 見えなかった事にする */
				}
				return ;
			}
			if ( (GM_PlayerStatus & (PLAYER_SQUAT|PLAYER_GROUND|PLAYER_ROLLING))
				|| (GM_CheckPlayerStatusEX( I64(0), (PLAYER2_COMBO|PLAYER2_KNOCK_WALL|PLAYER2_JUMP) ))
				|| ((PL_GetPlayerWeapon() == WP_Aks)&&(GM_PlayerStatus&PLAYER_HOLD)) ) {
				entk->notice |= ENE_NOTICE_UNIFORM ;
				entk->pl_eyei.sight = EYE_INFO_SIGHT_OUT ;	/* 見えなかった事にする */
				return ;
			}
		}
		if ( (entk->iknow_flag & IKNOW_HIDDEN_PLAYER) ) {
			UNSET_FLAG( entk->iknow_flag, IKNOW_HIDDEN_PLAYER ) ;
			entk->notice |= ENE_NOTICE_UNIFORM ;
			return ;
		}
	}

	entk->pl_eyei.sight = EYE_INFO_SIGHT_OUT ;				/* 見えなかった事にする */
}

static void NoticeAccident( entk )
ENETHINK	*entk;
{
	int	id ;
	
	id = COM_Accident( ) ;
	if ( id >= 0 && id != entk->uniq_id ) {
		entk->notice |= ENE_NOTICE_ACCIDENT ;
	}
}

#if 0//no use
/* 調査モード */
static void NoticeInvestigated( entk )
ENETHINK	*entk;
{
	if ( COM_GetInvestigatedNearID( entk ) ) {
		entk->notice |= ENE_NOTICE_INVESTIGATED ;
	}
}
#endif
/* ----------------------------------------------- */
void ENE_NoticeCheck( ENETHINK *entk )
{
	entk->notice = 0 ;

	if ( COM_StageKind() & ENE_STAGE_NO_NOTICE ) return ;	/* 反応無し */

	if ( entk->act->status & ACT_STATUS_DAMAGE ) return ;

	/* 危険モードでノイズ聞こえたらアラートマックスにする為 */
	if ( !(entk->act->status & ACT_STATUS_UNREAL) ) NoticeNoise( entk ) ;

	if ( GM_AlertMode == ALERT_MODE_ALERT ) {
		NoticeBoxAlert( entk ) ;
		return ;
	}

	NoticeAccident( entk ) ;
	if ( entk->act->status & ACT_STATUS_UNREAL ) return ;

	NoticeHoldUp( entk ) ;
	NoticeFound( entk ) ;
	NoticeFoundLink( entk ) ;
/*sigeno 反応制御*/
	if(!(entk->act->bodyp.type & ENE_TYPE_EVENT_32)){
		NoticeEnemyDamage( entk ) ;
	}
	if(!(entk->act->bodyp.type & ENE_TYPE_EVENT_32)){
		NoticeCorp( entk ) ;
	}

	NoticeLight( entk ) ;
	NoticeTouch( entk ) ;
	NoticeRegion( entk ) ;
	NoticeBox( entk ) ;
	NoticeUniform( entk ) ;
//	NoticeInvestigated( entk ) ;
}
