//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   raiden.c 
   ライデン、スネーク共通
   プレイヤープログラム
   
   1999/07/07 M.Sonoyama
   $Id: raiden.c,v 1.4 2002/12/11 09:13:45 takaki Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"g_other.h"
#include	"raiden.h"
#include	"pl_inline.h"

#define	WorkL		PlayerWorkL
#define	Work		PlayerWork 
#define	ACTION		PL_ACTION
#define	OPERATE		PL_OPERATE
#define	MOTION_SET	PL_MOTION_SET

#define	RAIDEN_C

#include	"raiden.x"
#include	"weapon.h"
#include	"item.h"
#include	"motion.h"
#include	"game.h"
//#include	"rai_arm.h"
#include	"pl_arm.h"
#include	"../../kano/attachment/attachments.h"

#include "BP_Debug.h"//BP_INPUT - for debug options


extern int PL_COM_SetPadType( void );
extern int gBP_UseVitaControlOverrides;

int PL_PlayerInvisible = 0 ;
int PL_PlayerInvisiblePrev = 0 ;
/*---------------------------------------------------------------------*/

/* ローカル */
static 	float		HzxHeight[] = { NEAR_HEIGHT_STAND, NEAR_HEIGHT_SQUAT, 
									NEAR_HEIGHT_GROUND, NEAR_HEIGHT_GROUND } ;
static	float		FindHeight[] = { 1001.0F, 750.0F, 200.0F, 200.0F } ;
static	GV_PAD		   ReleasePad ;

//BP_INPUT - extra controller input processing for X360 etc

// Modified player pad data
BP_PlayerPad         PlayerPad;  

// Extra controller tweakables
float PL_PAD_ZOOM_LOCKER_FAST_TH   = 0.550f;
float PL_PAD_ZOOM_LOCKER_MEDIUM_TH = 0.130f;

float PL_ZOOM_LOCKER_FAST_SPEED    = 1.0f;
float PL_ZOOM_LOCKER_MEDIUM_SPEED  = 16.0f;
float PL_ZOOM_LOCKER_SLOW_SPEED    = 128.0f;

float PL_PAD_SPRAY_MIN_TH = 0.2f;
float PL_PAD_SPRAY_MAX_TH = 0.98f;

float PL_PAD_QUICK_HOLSTER_STICK_Y_TH = 0.9f;
float PL_PAD_QUICK_HOLSTER_STICK_X_TH = 0.2f;
float PL_PAD_QUICK_HOLSTER_TIME_TH    = 0.070f;

float PL_PAD_QUICK_HOLSTER_STICK_MAG_TH = 0.990f;

float PL_SCOPE_ZOOM_IN_SPEED = 2.0f;
float PL_SCOPE_ZOOM_OUT_SPEED = 2.5f;

int   PL_SQUAT_TO_GROUND_FRAMES = 30;

// Extra functions
static	inline	void	InitBPPlayerPad( void )
{
   GV_ZeroMemory( &PlayerPad, sizeof( PlayerPad ) ) ;
   PlayerPad.pad.dir = -1;

#if BP_360
   PlayerPad.enable = 1;
#endif
}

//BP_INPUT - extra controller input processing for X360 etc


static	int			   CurrentChanl = 0 ;


static	FVECTOR		PL_ObjShift ;

/* ホーミング（廃止？） */
static	float	HomingLen ;
static	int	HomingRange ;

/* 振動 */
u_char	PL_DamageVib1H[] = { 127, 4, 0, 0 } ;
u_char	PL_DamageVib1L[] = { 225, 1, 255, 2, 0, 0 } ;

u_char	PL_DamageVib1HHlf[] = { 127, 3, 0, 0 } ;
u_char	PL_DamageVib1LHlf[] = { 225*2/3, 1, 255*2/3, 2, 0, 0 } ;

u_char	PL_DamageVib2H[] = { 127, 6, 0, 0 } ;
u_char	PL_DamageVib2L[] = { 225, 1, 255, 4, 225, 2, 75, 8, 0, 0 } ;

#define	DamageVib1H		PL_DamageVib1H
#define	DamageVib1L		PL_DamageVib1L
#define	DamageVib2H		PL_DamageVib2H
#define	DamageVib2L		PL_DamageVib2L

#define	DamageVib1HHlf	PL_DamageVib1HHlf
#define	DamageVib1LHlf	PL_DamageVib1LHlf

#ifdef DEBUG_MODE
//static	int	PreAlert = 0 ;
#ifdef PAD_PRESSURE_CHECK
GM_GageSet 	PressureGage[ 12 ] ;
#endif
#ifdef PAD_MEMORY
#define	PAD_MEMORY_ADDR 	(0x00100000)
#define	PAD_MEMORY_ADDR_END	(PAD_MEMORY_ADDR + sizeof( GV_PAD ) * 60 * 300) /* 5分 */
static	GV_PAD	*MemPad = NULL ;
static	int	MemPadMode = 0 ;
static	int	PadMemoryEnd ;
#endif
#endif

/*---------------------------------------------------------------------*/

/* グローバル */
PL_ACTION	PL_StillMode[] = { StandStill, SquatStill, GroundStill, IntrudeStill } ;
PL_ACTION	PL_CB_BoxStill = CB_BoxStill ; 
PL_ACTION	PL_CB_BoxStop = CB_BoxStop ; 
PL_ACTION	PL_Dead = Dead ;
PL_ACTION	PL_Down = Down ;

PL_MOTION_SET	*PL_MotionSet ;
FORCE			*PL_Force ;		/* 強制モーション設定ワーク */
BEYOND			*PL_Beyond ;	/* エルードモード設定ワーク */

HZX_HZD			PL_KnockSegment ;	/* たたいた壁 */
int				PL_KnockJoint ;		/* たたいた関節 */

#define	StillMode	PL_StillMode

/*---------------------------------------------------------------------*/

#include	"routine.c"
#include	"adjust.c"
#include	"event.c"
#include	"subject.c"
#include	"normal.c"
#include	"peep.c"
#include	"attack.c"
#include	"equip.c"
#include	"special.c"
#include	"force.c"
#include	"dbg_menu.c"

/*---------------------------------------------------------------------*/

/* ライフ更新処理 */
static	void	UpdateLife( Work *work )
{
	int			p_vital ;

#if 0
	if ( GV_PadData[1].press & PAD_X ) /*ごめん、テストさせてー*/
	{
		int NewKillInvisibleChara() ;

		NewKillInvisibleChara() ;
	}
#endif

	p_vital = GM_Vitality ;

	if ( PlayerDebugMenuStatus & PDMS_MUTEKIMODE1 ) {
		if ( GM_VitalityAdjust < 0 ) GM_VitalityAdjust = 0 ;
		if ( GM_VitalityAdjustNoDead < 0 ) GM_VitalityAdjustNoDead = 0 ;
	}
	if ( Status( PLAYER_INVINCIBLE | PLAYER_INVINCIBLE_OUT ) ||
		 GM_CheckGameStatus( STATE_PLAY_DEMO ) ) {
		if ( GM_VitalityAdjust < 0 ) GM_VitalityAdjust = 0 ;
		if ( GM_VitalityAdjustNoDead < 0 ) GM_VitalityAdjustNoDead = 0 ;
	}
	if ( Status( PLAYER_NORECOVER ) ) {
		if ( GM_VitalityAdjust > 0 ) GM_VitalityAdjust = 0 ;
		if ( GM_VitalityAdjustNoDead > 0 ) GM_VitalityAdjustNoDead = 0 ;
	}
	if ( !Status( PLAYER_DEAD ) ) {
		work->power.vital += GM_VitalityAdjust ;
		/* 死なない調整値 */
		if ( GM_VitalityAdjustNoDead != 0 &&
			( work->power.vital + GM_VitalityAdjustNoDead > 0 ) ) {
			work->power.vital += GM_VitalityAdjustNoDead ;
		}
		/* レーション自動使用処理 */
		if ( !Flag( FLAG_CANNOT_USE_RATION ) &&
			!Status( PLAYER_NORECOVER ) &&
			work->power.vital <= 0 &&
			work->item == IT_Ration &&
			GM_ItemNum( IT_Ration ) > 0 ) {
			work->power.vital = GM_RATION_HEAL ;
			GM_DecrementItem( IT_Ration, 1 ) ;
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_KAIHUKU1 ) ;
			if ( ++ GM_RationUseCount > 30000 ) GM_RationUseCount = 30000 ;
			/* 出血トマル */
			UnsetStatus( PLAYER_BLOOD_DROP ) ;
		}
	}

	/* ダメージ値カウント */
	if ( GM_VitalityAdjust < 0 ) GM_DamageCount -= GM_VitalityAdjust ;
	if ( GM_VitalityAdjustNoDead < 0 ) GM_DamageCount -= GM_VitalityAdjustNoDead ;

	GM_VitalityAdjust = 0 ;
	GM_VitalityAdjustNoDead = 0 ;
    if ( work->power.vital > GM_VitalityMax ) work->power.vital = GM_VitalityMax ;
    if ( work->power.vital < 0 ) work->power.vital = 0 ;


	/*ステージクリアが確定している場合、かならず死なないための処理*/
	if ( GM_GameStatus & STATE_VR_ANOTHER && GM_VRStatus & GM_VR_CLEAR ){
		/*ステージクリアが確定している*/
		if ( work->power.vital <= 0 ) {
			work->power.vital = 1 ;
		}
	}

    GM_Vitality = work->power.vital ;
    work->life.value = GM_Vitality ;

    if ( work->power.vital <= 0 ) {
		/* 敵からのダメージ外の原因で死亡（炎など） */
		if ( !( work->def.damaged & TARGET_POWER ) && !Status( PLAYER_DEAD ) ) {
			if ( WeaponType( work ) & WP_TYPE_SUBJECT ) NoWeapon( work ) ;
			if ( ItemType( work ) & IT_TYPE_ABNORMAL ) NoEquip( work ) ;
			LeaveSubject( work ) ;
			LeaveCaution( work ) ;
			UnsetWeaponCamera( work ) ;
			work->control.skip_flag &= ~( CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_TRAP ) ;
			ClearCaptureTarget( work ) ;
			SetMode2( work, NULL ) ;
			SetArmAction( work, AMstand ) ;
			GM_ClearTargetDamage( &( work->def ) ) ;
			work->trigger = TRIG_FALL ;	/* グレネード落とし */
			if ( GM_NikitaAlive[ work->chanl ] == NKT_NORMAL ) {
				GM_NikitaAlive[ work->chanl ] = NKT_CANCEL ;
			}
			SetInvincible( work, 0 ) ;
			if ( !Status( PLAYER_BEYOND ) ) {
				PL_DeadMode		*list ;
				PL_ACTION		deadact ;
				int				( *cond )( Work *work ) ;

				/* 特殊ゲームオーバーアクション */
				list = PL_DeadModeList.next ;
				deadact = Dead ;
				while( list != NULL ) {
					cond = list->cond ;
					if ( cond == NULL || ( *cond )( work ) != 0 ) {
						deadact = list->action ;
						break ;
					}
					list = list->next ;
				}
				if ( deadact == Dead ) {
					PL_ChangeMotionArc( work, work->org_motion ) ;
					PL_SeSetSubject( SD_V_POUT0001, &work->control.mov, GM_SEMODE_BOMB ) ;
				}
				SetMode( work, deadact ) ;
			} else {
				PL_SeSetSubject( SD_V_POUT0001, &work->control.mov, GM_SEMODE_BOMB ) ;
			}
		}
		/* 死亡フラグセット */
		SetStatus( PLAYER_DEAD ) ;
		if ( !GM_IsGameOver() ) {
			GM_CallCallbackProc( GM_CALLBACK_PLAYER_DEAD ) ;
			GM_GameOverProcStart( &work->actor ) ;
		}
    }

	if ( GM_IsGameOver() ) {
		PL_SetDeadFlag( work ) ;
		/* プレイヤーの死亡でないときは、主観装備外す */
		if ( !Status( PLAYER_DEAD ) ) {

		    if ( !(GM_GameStatus & STATE_VR_ANOTHER && 
			   GM_VRStatus   & GM_VR_GAME_OVER /* &&
			   GM_VRStatus   & GM_VR_PHOTOGRAPH*/) ) {
			
			if ( PL_WeaponType( work ) & WP_TYPE_SUBJECT ) {
			    NoWeapon( work ) ;
			    SetMode( work, StillMode[ work->stance ] ) ;
			}
			if ( PL_ItemType( work ) & IT_TYPE_SUBJECT ) {
			    NoEquip( work ) ;
			    SetMode( work, StillMode[ work->stance ] ) ;
			}
		    }
		}
	}

    /* ゲージ表示／非表示 */
#ifdef DEBUG_MODE
    if ( GM_PlayerDebugMode == GM_PDM_NOGAGE ) {
		GM_GageStatus = GM_GAGE_STATE_DISAPPEAR ;
    } else {
#endif
		if ( p_vital == GM_Vitality &&
			GM_Vitality > GM_VitalityMax / 3 &&
			GM_N_ActiveGages == 1 ) {
			if ( ++ work->gagetime >= 500 ) {
				GM_GageStatus = GM_GAGE_STATE_DISAPPEAR ;
			}
		} else {
			work->gagetime = 0 ;
			if ( !GM_CheckMenuStatus( MENU_MENU_OPEN ) ) {
				GM_GageStatus = GM_GAGE_STATE_APPEAR ;
			}
		}
		if ( GM_CheckMenuStatus( MENU_MENU_OPEN ) ) work->gagetime = 0 ;
#ifdef DEBUG_MODE
    }
#endif


    if ( PL_SubjectMove ){
		/* 主観移動時には */
		GM_RadarSetFlag( &work->radar, RADAR_SIGHT ) ;

		/* 主観移動の時にゲームオーバーの時は主観がとれる */
		if ( GM_IsGameOver() ) {
			PL_SubjectMove = 0 ;
			LeaveSubject( work ) ;
#ifdef KP_WINDOWS
			GV_SetWindowsInputConfigulationMode(FALSE) ;
#endif
		}
	}

}

/* アクション制御部 */
static	inline	void	Action( Work *work )
{
    void	( *action )( Work *, int ) ;
    int		time ;
    FVECTOR	mov ;

    //BP_INPUT - Start with actual input since "CheckDirection" checks input status for weapon/item select timing
    memcpy( &PlayerPad.pad, GV_PadData + work->chanl, sizeof(GV_PAD) );
    //BP_INPUT - Start with actual input since "CheckDirection" checks input status for weapon/item select timing

	/* パッドデモ中は死体上動きしない */
	if ( GM_CheckGameStatus( STATE_PAD_DEMO ) ) {
		UnsetStatus( PLAYER_ON_CORPSE ) ;
	}


	/* ジアゼパム */
	CheckDiazepam( work ) ;

	/* 影制御１ */
	work->shadow = 1 ;

	/* 飛び散り血 */
	if ( Flag( FLAG_BLOOD_SPLASH ) ) {
		if ( work->power.vital > 0 ) {
			NewBlood( &work->body.objs->objs[ HUMAN21_KUBI ].world,
					  ( FVECTOR * )work->body.objs->objs[ HUMAN21_KUBI ].world.m[ 3 ],
					 &workL->blood_force, 0, 0 ) ;
		} else {
			FVECTOR		force = { 10.0F, 0.0F, 0.0F } ;

			/* 主観中はでない */
			if ( !Status( PLAYER_WATCH ) ) {
				DG_SetPos( &work->body.objs->world ) ;
				DG_RotVector( &force, &force, 1 ) ;
				NewBlood( &work->body.objs->objs[ HUMAN21_KUBI ].world,
						 ( FVECTOR * )work->body.objs->objs[ HUMAN21_KUBI ].world.m[ 3 ],
						 &force, 1, 0 ) ;
			}
		}
	}

    /* フラグリセットされる前にやっておくべき処理 */

    /* トリガリセット */
    work->trigger = TRIG_YET ;
	work->stick_wp_work.trigger = TRIG_YET ;
	work->body.m_ctrl->use_move_layer = 0 ; 

	/* 影チェック位置数 */
	GM_N_PlayerShadowPos = 0 ;

	/* ポーリング関数実行 */
	ExecPollingFunc( work ) ;

    /* メッセージ実行 */
    ExecMessage( work ) ;

    /* ビハインドチェック */
    CheckBehindCamera( work ) ;
    /* はりつき条件等をチェック */
    CheckDirection( work ) ;

	/* 変更不可武器・アイテムリセット */
	PL_MenuNoUseWeapon = I64(0) ;
	PL_MenuNoUseItem = I64(0) ;
	PL_MenuNoUseWeaponType = 0 ;
	PL_MenuNoUseItemType = 0 ;

    /* フラグリセット */
	UnsetFlag( FLAG_RESET_ALL ) ;
	UnsetFlag2( FLAG2_INTRUDE_EFFECT_ON | FLAG2_INTRUDE_EFFECT_OFF |
			    FLAG2_FORCE_VISIBLE | FLAG2_SUBJECT_MOVE ) ;

	/* 毎フレームクリアするステートもある */
	GM_ResetPlayerStatusEX( PLAYER_WEAPON_DISABLE | PLAYER_WEAPON_INVISIBLE |
						    PLAYER_ITEM_DISABLE | PLAYER_HOLD | PLAYER_MOVE |
						    PLAYER_DARK_AREA | PLAYER_ENEMY_HIDDEN |
						    PLAYER_WEAPON_QUICK_ONLY | PLAYER_ITEM_QUICK_ONLY |
						    PLAYER_EVENT_ENABLE | PLAYER_ENEMY_HANG | PLAYER_ENEMY_PULL | 
						    PLAYER_ROLLING | PLAYER_NARROW |
						    PLAYER_CB_BOX_HIDDEN | PLAYER_WALK | PLAYER_DASH |
						    PLAYER_CBBOX_RUN,
						    PLAYER2_ELUDE_FALL | PLAYER2_JUMP | PLAYER2_ARM_INVISIBLE |
						    PLAYER2_COMBO | PLAYER2_KNOCK_WALL | 
						    PLAYER2_NARROW_HIDDEN | PLAYER2_DEEP_STEALTH ) ;
	/* ポーリングでメニュー禁止特殊 */
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_MENU_DISABLE ) ) {
		GM_SetPlayerStatus( PLAYER_WEAPON_DISABLE | PLAYER_ITEM_DISABLE ) ;
		GM_ResetPlayerStatusEX( I64(0), PLAYER2_MENU_DISABLE ) ;
	}
	/* ポーリングでＸＺ移動禁止 */
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NO_MOTION_STEP_XZ ) ) {
		SetFlag( FLAG_NO_MOTION_STEP_XZ ) ;
		GM_ResetPlayerStatusEX( I64(0), PLAYER2_NO_MOTION_STEP_XZ ) ;
	}

	/* 死亡時フラグ再セット */
	PL_SetDeadFlag( work ) ;

    /* プレイヤーチェック部位リセット */
    GM_PlayerFindObj = 0 ;
    /* プレイヤーチェック座標セット。１フレーム遅れる */
    /* モーションも遅れるから調度いいのかも */
    /* 後で書き換えられることもある（主観中など） */
    GM_PlayerFindPos = work->control.mov ;
    GM_PlayerFindPos.vy = work->control.hzx_base + FindHeight[ work->stance ] ;
    
    /* トラップチェック */
    CheckTrapEvery( work ) ;

    /* アジャスト値をリセット */
	ResetRotAdjust( work ) ;
	/* 壁ＳＥコードリセット */
	work->seNoSeg = -1 ;
	work->seNoFlr = HZX_GetSeCode( work->floor_atr ) ;

    /* 頂点アニメフラグリセット */
    work->vanime_flag = 0 ;
	work->actblur_flag = 0 ;
	PL_ActionBlurObjs = work->body.objs ;
	PL_ActionBlurParent = work ;

    /* パッドＯＦＦ処理 */
#if 1
    if ( Status( PLAYER_PAD_OFF ) ||
		( !GM_CheckGameStatus( STATE_PAD_DEMO ) &&
		  ( GV_PadData[ work->chanl ].flag & GV_PAD_RELEASE ) ) ) {
#else
    if ( Status( PLAYER_PAD_OFF ) ) {
#endif
		/* 主観の入りがトグルになっている */
		if ( PL_SubjectToggle && Status( PLAYER_WATCH ) &&
			!(WeaponType( work ) & WP_TYPE_SUBJECT) &&
			!(ItemType( work ) & IT_TYPE_SUBJECT) ) {
			LeaveSubject( work ) ;
#ifdef CAMERA_TURN
			work->camdir.vx = work->g_rot ;
#else
			work->control.turn.vx = work->g_rot ;
#endif
#ifndef KP_WINDOWS
			PL_SubjectToggle = 0 ;
#endif
		}

		work->pad = &ReleasePad ;

#if defined(BP_VITA)
      work->gesturePadFilterType = kGPR_AlwaysFail;
#endif
    } 
    else 
    {
      //BP_INPUT - use player pad structure for extra controller input logic processing

      // Enabled?
      if( PlayerPad.enable )
      {
         // Lookup item type
         const int   itemType = ItemType( work );
         const u_int weaponType = work->weapon;

         // Lookup left stick info
         const float leftStickX = (PlayerPad.pad.left_dx - 128.0f) / 128.0f;
         const float leftStickY = (128.0f - PlayerPad.pad.left_dy) / 128.0f;
         const float leftStickForce = bp_sqrtf( (leftStickX * leftStickX) + (leftStickY * leftStickY) ); //BP_MATH - emulate PS2 sqrtf

         const float deltaTime = 1.0f/(float)BP_FRAMES_PER_SEC();

         // Default states
         int weaponState    = PlayerPad.weaponState;
         int weaponPressure = PlayerPad.pad.pressure[PL_PAD_PRESS_WEAPON];
         int buttonState    = BS_Default;

         // Point to player pad so we can apply logic modifications
         work->pad = &PlayerPad.pad;

#if defined(BP_VITA)
         work->pad->status |= GestureGetTriggerButtonMask(kGesture_TouchScreen_Front, PL_PadGetGestureRelease(work));
         work->pad->press |= GestureGetTriggerButtonMask(kGesture_TouchScreen_Front, PL_PadGetGestureRelease(work));
#endif

         // Holding a camera or other item that has 1st person zoom and disabled weapon?
         if( itemType & IT_TYPE_SUBJECT )
         {
            // Skip weapon holster/draw switching update
         }

         // AS MCampbell - Leave the input unaffected when no weapon is equipped but allow the button state to be 
		 // modified when the Nikita is equipped. The shootnikita code has been modified to cancel the firing state
		 // if the holster weapon button is pressed. This is consistent with the behavior of pistols.
         // Bug: MGSTWO-2939
         else if (weaponType == WP_None)
         {
            // Leave input as is so player:
            // - doesn't hold onto an enemy that has been picked up when releasing X
         }

         // AS MCampbell - Make sure pressing the dpad down button does not fire the weapon.
         // Bug: MGSTWO-2939
         else if( (weaponType == WP_Psg1) || (weaponType == WP_Psg1T) || (weaponType == WP_Stinger) )
         {
            if (gBP_UseVitaControlOverrides && !(GM_GameStatus & STATE_PAD_DEMO))
            {
               if ((PlayerPad.pad.press & PAD_D) && PlayerPad.pad.pressure[PAD_PRESS_D])
               {
                  buttonState = BS_Clear;
               }
            }
         }

         // Spray?
         else if( weaponType == WP_ColdSpray )
         {
#if !defined(BP_VITA)
            // Read right stick input: +1 = up, 0=middle, -1=down
            float stick = ((255.0f - PlayerPad.pad.right_dy) - 128.0f)/128.0f;

            // Scale [PL_PAD_SPRAY_MIN_TH, PL_PAD_SPRAY_MAX_TH] -> [0,1]
            stick = (stick - PL_PAD_SPRAY_MIN_TH) / (PL_PAD_SPRAY_MAX_TH - PL_PAD_SPRAY_MIN_TH);
            if( stick < 0.0f ) stick = 0.0f;
            if( stick > 1.0f ) stick = 1.0f;

            // Scale [0,1] -> [0,255] and store as weapon button to activate spray
            weaponPressure = (int)((stick*255.0f)+0.5f);
            buttonState = (stick > 0.0f) ? BS_PressureHold : BS_Clear;
#else
            // On Vita, we use square to spray coolant. Instead of varying pressure, we just spray at full pressure.
            if (work->pad->status & PAD_Y)
            {
               weaponPressure = 255;
               buttonState = BS_PressureHold;
            }
            else
            {
               weaponPressure = 0;
               buttonState = BS_Clear;
            }
#endif
         }
         // Force a punch?
         else if( PlayerPad.forcePunch )
         {
            // Fake a punch press
            PlayerPad.forcePunch--;
            PlayerPad.pad.press |= PL_PAD_PUNCH;
            PlayerPad.pad.release &= PL_PAD_PUNCH;
            PlayerPad.pad.status |= PL_PAD_PUNCH;

            // Fake holster
            weaponState = WS_Holster;
            buttonState = BS_Clear;
         }
         // Release fire always holsters weapon
         else if(PlayerPad.pad.release & PL_PAD_WEAPON)
         {
            weaponState = WS_Holster;
         }
         // Pressing fire always draws weapon
         else if(PlayerPad.pad.press & PL_PAD_WEAPON)
         {
            weaponState = WS_Draw;
         }
         else
         {
            // Quick holster activated?
            int quickHolster = 0;
            int holster_weapon_button_pressed = 0;

            if (gBP_UseVitaControlOverrides && !(GM_GameStatus & STATE_PAD_DEMO))
            {
               holster_weapon_button_pressed = (PlayerPad.pad.press & PAD_D) && PlayerPad.pad.pressure[PAD_PRESS_D];
            }
            else
            {
               holster_weapon_button_pressed =  (PlayerPad.pad.press & PL_PAD_DRAW_HOLSTER_WEAPON);
            }

            // Pushed stick past threshold in 3rd person?
            if(    ( gBP_Input_X360Remap_QuickHolsterMethod == 0 )
               && (!(PlayerPad.pad.status & (PL_PAD_WEAPON | PL_PAD_LOCKON))) // Arm fix: added PAD_LOCKON to fix MGSTWO-3319
               && PL_SubjectMove != 1 // \ Arm fix: if we can move in subject mode, don't holster when moving left stick
               && PL_SubjectMove != 2 // / fix for MGSTWO-3345
               && ( !Status( PLAYER_WATCH ) )
               && ( leftStickForce > PL_PAD_QUICK_HOLSTER_STICK_MAG_TH ) )
            {
               quickHolster = 1;
            }

            // Pushed stick forward quickly in 3rd person?
            if(    ( gBP_Input_X360Remap_QuickHolsterMethod == 1 )
               && (!(PlayerPad.pad.status & (PL_PAD_WEAPON | PL_PAD_LOCKON))) // Arm fix: added PAD_LOCKON to fix MGSTWO-3319
               && ( !Status( PLAYER_WATCH ) )
               && PL_SubjectMove != 1 // \ Arm fix: if we can move in subject mode, don't holster when moving left stick
               && PL_SubjectMove != 2 // / fix for MGSTWO-3345
               && ( leftStickY >= PL_PAD_QUICK_HOLSTER_STICK_Y_TH ) 
               && ( fabs(leftStickX) < PL_PAD_QUICK_HOLSTER_STICK_X_TH ) 
               && ( PlayerPad.leftStickNonZeroTime < PL_PAD_QUICK_HOLSTER_TIME_TH ) )
            {
               quickHolster = 1;
            }

            // Process weapon state
            switch( weaponState )
            {
            case WS_Holster:
            case WS_HolsterQuick:
               // Draw?
               if( (!quickHolster) && holster_weapon_button_pressed)
               {
                  // Switch to draw
                  weaponState = WS_Draw;
                  buttonState = BS_SoftPress;
               }
               else
               {
                  // Continue holster
                  buttonState = BS_Clear;
               }
               
               break;

            case WS_Draw:
               // Pressing punch should holster weapon so player can punch immediately
               if( PlayerPad.pad.press & PL_PAD_PUNCH )
               {
                  // Switch to holster without firing
                  weaponState = WS_HolsterQuick;
                  buttonState = BS_SoftRelease;

                  // Force a punch next 2 frames for PL_CheckAttack logic to pick up
                  PlayerPad.forcePunch = 2;
               }
               // Holster?
               else if( holster_weapon_button_pressed )
               {
                  // Switch to holster
                  weaponState = WS_Holster;
                  buttonState = BS_SoftRelease;
               }
               // Quick holster?
               else if( quickHolster )
               {
                  // Switch to quick holster
                  weaponState = WS_HolsterQuick;
                  buttonState = BS_SoftRelease;
               }
               // Hold draw?
               else if(!(PlayerPad.pad.status & PL_PAD_WEAPON))
               {
                  // Continue draw
                  buttonState = BS_SoftHold;
               }
               else
               {
                  // Allow default fire behavior....
                  //    Pistol = fire upon release
                  //    Auto   = fire 
               }
               break;
            }
         }

         // Set new states
         PlayerPad.weaponState = weaponState;
         PlayerPad.buttonState = buttonState;

         // Update weapon button state/pressure
         switch( buttonState )
         {
         // Simulate soft press
         case BS_SoftPress:
            PlayerPad.pad.status  |= PL_PAD_WEAPON;
            PlayerPad.pad.press   |= PL_PAD_WEAPON;
            PlayerPad.pad.release &= ~PL_PAD_WEAPON;
            PlayerPad.pad.pressure[PL_PAD_PRESS_WEAPON] = PL_PAD_WEAPON_TH;
            break;

         // Simulate soft hold
         case BS_SoftHold:
            PlayerPad.pad.status  |= PL_PAD_WEAPON;
            PlayerPad.pad.press   &= ~PL_PAD_WEAPON;
            PlayerPad.pad.release &= ~PL_PAD_WEAPON;
            PlayerPad.pad.pressure[PL_PAD_PRESS_WEAPON] = PL_PAD_WEAPON_TH;
            break;

         // Simulate soft release
         case BS_SoftRelease:
            PlayerPad.pad.status  |= PL_PAD_WEAPON;
            PlayerPad.pad.press   &= ~PL_PAD_WEAPON;
            PlayerPad.pad.release |= PL_PAD_WEAPON;
            PlayerPad.pad.pressure[PL_PAD_PRESS_WEAPON] = PL_PAD_WEAPON_TH;
            break;

         // Simulate analog pressure hold
         case BS_PressureHold:
            PlayerPad.pad.status  |= PL_PAD_WEAPON;
            PlayerPad.pad.press   &= ~PL_PAD_WEAPON;
            PlayerPad.pad.release &= ~PL_PAD_WEAPON;
            PlayerPad.pad.pressure[PL_PAD_PRESS_WEAPON] = weaponPressure;
            break;

         // Clear fire state
         case BS_Clear:
            PlayerPad.pad.status  &= ~PL_PAD_WEAPON;
            PlayerPad.pad.press   &= ~PL_PAD_WEAPON;
            PlayerPad.pad.release &= ~PL_PAD_WEAPON;
            PlayerPad.pad.pressure[PL_PAD_PRESS_WEAPON] = 0;
            break;
         }

         // Update left stick non zero time
         if( fabs(leftStickY) > 0.1f )
         {
            PlayerPad.leftStickNonZeroTime += deltaTime;
         }
         else
         {
            PlayerPad.leftStickNonZeroTime = 0.0f;
         }
      }
      else
      {
         work->pad = GV_PadData + work->chanl ;
      }

#if defined(BP_VITA)
      work->gesturePadFilterType = kGPR_Filtered;
#endif
      //BP_INPUT - use player pad structure for extra controller input logic processing
    }
    if ( work->pad->press != 0 ) {
		UnsetStatus( PLAYER_NEED_NEW_PRESS ) ;
		if ( work->pad->press & PL_PAD_SUBJECT ) SetFlag( FLAG_SUBJECT_NEWPRESS ) ;
	}

    PL_UpdatePressSquat( work ); //BP_INPUT - Adding MGS3 hold "X" for squat to ground functionality


#ifdef DEBUG_MODE
    MrAdjust( work ) ;
#endif
    /* ダメージチェック */
    CheckDamage( work ) ;

	/* ダメージチェックのあとに
	   リセットされるステータスもある */
    UnsetStatus( PLAYER_BEHIND_ATTACK | PLAYER_CB_BOX | PLAYER_CB_BOX_STAND ) ;

    /* 主観可視制御 */
    CheckVWait2( work ) ;

    /* アクション */
    time = work->time ;
    work->time += ONE_CLOCK ;
    work->ftime = work->ftime_count ;
    work->ftime_count ++ ;
    action = work->action ;
    ( *action )( work, time ) ;
	/* アクション１後にリセットされるフラグもある */
	UnsetFlag( FLAG_RELOADING ) ;
	if ( Flag( FLAG_HOMING ) ) {
		if ( GM_CameraDir.vx == work->camdir.vx &&
			 work->control.turn.vy == work->control.rot.vy ) {
			UnsetFlag( FLAG_HOMING ) ;
		}
	}

	/* 変更不可武器・アイテムのセット */
	SetNoUse( work ) ;

    /* 武器チェンジチェック */
    CheckChangeWeapon( work ) ;

    /* アイテムチェンジチェック */
    CheckChangeItem( work ) ;

	/* 変更チェック後にリセット */
	PL_UnsetFlag2( FLAG2_NO_TIMERESET_WHEN_CHANGE |
				   FLAG2_CANNOT_CHANGE_WEAPON2 |
				   FLAG2_CANNOT_CHANGE_ITEM2 ) ;

    /* 追加アクション */
    action = work->action2 ;
    if ( action != NULL ) {
		time = work->time2 ;
		work->time2 += ONE_CLOCK ;
		work->ftime2 = work->ftime2_count ;
		work->ftime2_count ++ ;
		( *action )( work, time ) ;	
    }
    /* 主観モードチェック */
    CheckWatch( work ) ;
	/* 主観タクティカルリロード */
	SubjectTacticalReload( work ) ;

    /* 張り付き半径計算 */
    work->control.r_sphere
		= GV_NearExp4( work->control.r_sphere, work->r_sphere ) ;

	/* イントルードエフェクト管理 */
	if ( Status( PLAYER_INTRUDE ) ) {
		if ( Flag2( FLAG2_INTRUDE_EFFECT_OFF ) ) {
			NewIntrudeInDuct2D_2( 1, 0 ) ;			
		} else {
			NewIntrudeInDuct2D_2( 0, 0 ) ;
		}
	} else {
		if ( Flag2( FLAG2_INTRUDE_EFFECT_ON ) ) {
			NewIntrudeInDuct2D_2( 0, 0 ) ;		
		} else {
			NewIntrudeInDuct2D_2( 1, 0 ) ;			
		}
	}

	
    /* 付随状態更新 */
    if ( Status( PLAYER_BLOOD_DROP ) ) {
#ifdef BLOOD_EVERY
		/* ぽたぽた血中 */
		if ( ++ work->blood_drop_count >= BLOOD_DROP_TIME ) {
			FVECTOR		pos ;

			DG_COPY_VEC( &pos, &work->control.mov ) ;
			if ( !GM_CheckGameStatus( STATE_PLAY_DEMO ) ) {
				/* デモ中はでない */
				NewBloodDrops( &pos, 2000, 0, work->control.addr ) ;
			}
			work->blood_drop_count = 0 ;
		}
		work->life.flag |= GM_GAGE_WARNING ;
#else
		/* 体力が１／３まで回復していれば出血はとまる */
		if ( GM_Vitality >= GM_VitalityMax / 3 ) {
			UnsetStatus( PLAYER_BLOOD_DROP ) ;
		} else {
			/* ぽたぽた血中 */
	        if ( ++ work->blood_drop_count >= BLOOD_DROP_TIME ) {
				FVECTOR		pos ;
				if ( !Status( PLAYER_IN_THE_WATER ) ) {
					int			addr ;

					if ( GM_CheckPlayerStatus( PLAYER_INTRUDE | 
											   PLAYER_ENEMY_HIDDEN | PLAYER_NARROW ) ) addr = -1 ;
					else   	addr = work->control.addr ;
					DG_COPY_VEC( &pos, &work->control.mov ) ;
					NewBloodDrops( &pos, 2000, 0, addr ) ;
				}
				work->blood_drop_count = 0 ;
			}
			work->life.flag |= GM_GAGE_WARNING ;
			if ( work->item == IT_Stealth ) GM_SetPlayerStatusEX( I64(0), PLAYER2_DEEP_STEALTH ) ;
		}
#endif
    } else {
		work->life.flag &= ~GM_GAGE_WARNING ;
	}
	/* 静止していれば体力回復 */
	RecoverLife( work ) ;

    /* 敵の方を向く */
    HeadToEnemy( work ) ;

    /* ターゲット移動 */
	MoveTarget( work ) ;
	
	/* 交差ターゲット */
	if ( work->touch_invincible_time > 0 ) {
		-- work->touch_invincible_time ;
	}
	if ( work->touch_invincible_time <= 0 && 
		!Status( PLAYER_CAUTION | PLAYER_FORCE | PLAYER_BEYOND |
				 PLAYER_DAMAGED | PLAYER_DOWNED ) &&
		work->stance != GROUND && !Flag( FLAG_NO_TOUCH_DAMAGE ) ) {
		GM_MoveTarget( &work->touch, &work->control.mov ) ;
		GM_PutTarget( &work->touch ) ;
#ifdef DEBUG_MODE
		if ( PlayerDebugMenuStatus & PDMS_TARGETVIEW ) {
			NewTargetView2( &work->touch, 232, 232, 32 ) ;
		}
#endif
	}
    /* 大域変数の更新 */
    mov = work->control.mov ; mov.vy = work->control.levels[ 0 ] ;
//    GM_RadarSetCenter( &( work->control.mov ) ) ;
    GM_RadarSetCenter( &GM_PlayerFindPos ) ;
	if ( Status( PLAYER_WATCH ) ) {
		GM_RadarSetSight( &( work->radar ), work->camdir.vy, 768, 6000.0F,
						  RADAR_COLOR_PLAYER ) ;
	} else {
		GM_RadarSetSight( &( work->radar ), work->control.rot.vy, 768, 6000.0F,
						  RADAR_COLOR_PLAYER ) ;
	}
    GM_PlayerAddress = work->control.addr ;
    GM_PlayerMap = work->control.map ;
    GM_PlayerPosition = work->control.mov ;
    GM_PlayerPosX = work->control.mov.vx ;
    GM_PlayerPosY = work->control.mov.vy ;
    GM_PlayerPosZ = work->control.mov.vz ;
    GM_PlayerDir = ( work->control.rot.vy & 4095 ) ;
    GM_PlayerMotion = work->motion1 ;
	GM_PlayerStance = Status( PLAYER_INTRUDE ) ? INTRUDE : work->stance ;
	GM_SubjectVMaxTmp[ 0 ] = GM_SubjectVMaxTmp[ 1 ] = 0 ;
	GM_SubjectHMaxTmp[ 0 ] = GM_SubjectHMaxTmp[ 1 ] = 0 ;
	GM_SubjectVStepTmp = GM_SubjectHStepTmp = -1 ;
	GM_SubjectAngleTmp = -1.0F ;

	GM_SetChanlTargetMap( GM_MAP_CHANL_MAIN, GM_PlayerMap ) ;

	/* 振動センサー */
    if ( work->item	== IT_VibSensor ) {
		GM_SetGameStatus( STATE_VIB_PAUSE0 ) ;
	} else {
		GM_ResetGameStatus( STATE_VIB_PAUSE0 ) ;
	}

    /* プレイヤーチェック部位 */
    if ( Status( PLAYER_GROUND ) ) {
		AddFindObj( HUMAN21_MIGI_TSUMASAKI ) ;
		AddFindObj( HUMAN21_ATAMA ) ;
    }
	/* 最後にクリアするステータスもある */
	UnsetStatus( PLAYER_ON_CORPSE ) ;
	GM_ResetPlayerStatusEX( I64(0), PLAYER2_SQUAT_ONLY ) ;
	PL_JumpGravity = 0.0F ;

	/* 風邪引きカウンタ */
	PL_UpdateColdCount() ;

	/* 影制御２ */
	if ( ( work->body.objs->flag & DG_FLAG_INVISIBLE ) ||	
		 ( work->control.flr_atrs[ 0 ] & HZX_FLOOR_STEP ) ||
		 ( work->body.height < 300.0F ) ||
		 ( Status( PLAYER_STEALTH ) ) ||
		 ( Flag( FLAG_MSG_SHADOW_OFF ) ) ) work->shadow = 0 ;
    /* デバッグ用 */
#if 0
    /* Ｌ２ボタン押し中は壁みない */
    if ( work->pad->status & PAD_L2 ) {
		work->control.skip_flag |= CTRL_SKIP_SEG_CHECK ;
    } else {
		work->control.skip_flag &= ~CTRL_SKIP_SEG_CHECK ;
    }
#endif
#ifdef DEBUG_MODE
	if ( GM_DebugModeEnable ) {
		if ( GV_PadData->press & PAD_AR ) GM_DumpControlList() ;
	}
#endif
}

/*---------------------------------------------------------------------*/

static	void 	Act( work )
Work		*work ;
{
    extern	void	Leg_IKcalc( CONTROL *, OBJECT * ) ;
    extern	void	Leg_IKcalcForRaiden( CONTROL *, OBJECT * ) ;
    float	p_height, p_ctrl_height ;


    if ( Status( PLAYER_STOP ) ) return ;

	/* デモ中などで不可視から復帰 */
	if ( Flag( FLAG_DEMO_INVISIBLE ) ) {
		DG_VisibleObjsChanl( work->body.objs, 0 ) ;
		UnsetFlag( FLAG_DEMO_INVISIBLE ) ;
	}
	/* カットインデモで強制可視から復帰 */
	if ( Flag2( FLAG2_CUTIN_DEMO_VISIBLE ) ) {
		DG_InvisibleObjsChanl( work->body.objs, 0 ) ;
		work->weapon_body = GM_PlayerArmBody ;
		UnsetFlag2( FLAG2_CUTIN_DEMO_VISIBLE ) ;
	}
	if ( Flag2( FLAG2_CUTIN_DEMO_VISIBLE_ALLOBJS ) ) {
		PL_InvisibleObjsAll( work->body.objs, 0 ) ;
		UnsetFlag2( FLAG2_CUTIN_DEMO_VISIBLE_ALLOBJS ) ;
	}
	if ( Flag2( FLAG2_CUTIN_DEMO_INVISIBLE_ARM ) ) {	
		work->arm_trigger &= ~ARM_INVISIBLE ;
		UnsetFlag2( FLAG2_CUTIN_DEMO_INVISIBLE_ARM ) ;
	}

    GM_CurrentCameraChanl = work->chanl ; 
	CurrentChanl = work->chanl ;
	PL_MotionSet = &MS ;
	PL_Force = &ForceWork ;
	PL_Beyond = &Beyond ;

#ifdef PAD_MEMORY
    if ( MemPadMode == 0 ) {
		if ( ( u_int )MemPad - sizeof( GV_PAD ) < PAD_MEMORY_ADDR_END ) {
            *( MemPad ++ ) = *GV_PadData ;
			PadMemoryEnd = ( u_int )MemPad ;
		} else {
			printf( "Pad Data Recording End\n" ) ;
			MemPadMode = 2 ;
		}
    } else if ( MemPadMode == 1 ) {
		GV_PAD	pad ;
	
		if ( ( u_int )MemPad == PadMemoryEnd ) {
			printf( "Pad Data replay End\n" ) ;
			MemPadMode = 2 ;
		} else {
			pad = *( MemPad ++ ) ;
			pad.press &= ~( PAD_STA | PAD_AR ) ;
			pad.status &= ~( PAD_STA | PAD_AR ) ;
			pad.release &= ~( PAD_STA | PAD_AR ) ;
			pad.press |= GV_PadData->press & ( PAD_STA | PAD_AR ) ;
			pad.status |= GV_PadData->status & ( PAD_STA | PAD_AR ) ;
			pad.release |= GV_PadData->release & ( PAD_STA | PAD_AR ) ;
			pad.right_dx = GV_PadData->right_dx ;
			pad.right_dy = GV_PadData->right_dy ;
			*GV_PadData = pad ;
		}
    } else {
		/* なにもしない */
    }
#endif

	/* パッドデモ中はＰＳ２互換の配置で */
	if ( GM_CheckGameStatus( STATE_PAD_DEMO ) ) {
		PL_SetPadType( 0 ) ;
	} else {
		PL_SetPadType( 1 ) ;
	}



	/* ライフ更新処理 */
	UpdateLife( work ) ;
    if ( Status( PLAYER_DEAD ) ) printf( "SNAKE Die 3\n" ) ; //KARI

    /* メッセージ受信 */
    ReceiveMessage( work ) ;

    p_height = work->body.height ;
	p_ctrl_height = work->control.height ;
	if ( !Flag( FLAG_MOTION_VIB_OFF ) && 
		 !( work->body.m_ctrl->mt3_ctrl->flag & MT3_SLEEP ) ) {
		GM_ActVctrl( &work->vctrl ) ;	/* ActMotionの上 */
	}
	if ( Flag( FLAG_MOTION_STEP_OVERRIDE ) && work->motion2 >= 0 ) {
		work->body.m_ctrl->use_move_layer = 1 ;
	}

#if 0
	{
		float	v1, v2 ;

		v1 = work->body.m_ctrl->mt3_ctrl[ 0 ].play_time_base ;
		v2 = work->body.m_ctrl->mt3_ctrl[ 1 ].play_time_base ;
		work->body.m_ctrl->mt3_ctrl[ 0 ].play_time_base *= PL_MotionVelocity ;
		work->body.m_ctrl->mt3_ctrl[ 1 ].play_time_base *= PL_MotionVelocity ;

		GM_ActMotion( &( work->body ) ) ;

		work->body.m_ctrl->mt3_ctrl[ 0 ].play_time_base = v1 ;
		work->body.m_ctrl->mt3_ctrl[ 1 ].play_time_base = v2 ;

		PL_MotionVelocity = 1.0F ;
	}
#else
	GM_ActMotion( &( work->body ) ) ;
#endif
	/* 腰高さ補完なし */
    if ( Flag( FLAG_NO_WAIST_INTERP ) ) {
		int		use_layer ;

		use_layer = work->body.m_ctrl->use_move_layer ;
		work->body.height 
			= work->body.m_ctrl->height 
			= work->body.m_ctrl->old_height 
			= work->body.m_ctrl->mt3_ctrl[ use_layer ].move->step.vw ;
		work->body.m_ctrl->root_height 
			= work->body.m_ctrl->root_old_height 
			= work->body.m_ctrl->mt3_ctrl[ use_layer ].root->step.vw ;
		UnsetFlag( FLAG_NO_WAIST_INTERP ) ;		
	}


    if ( !Flag( FLAG_CTRL_HEIGHT_NOT_UPDATE ) ) {
		work->control.height = work->body.height ;
	}
    /* 床当たりチェック値調整 */
	if ( Flag( FLAG_HZX_HEIGHT_QUICK ) ) {
		work->control.hzx_height = work->hzx_height ;
	} else {
		work->control.hzx_height = GV_NearExp4( work->control.hzx_height, 
											    work->hzx_height ) ;
	}
    work->control.hzx_base += work->hzx_base_adjust ;
    work->control.height += work->hzx_height_adjust ;
    work->hzx_base_adjust = work->hzx_height_adjust = 0.0F ;

    if ( Flag( FLAG_CTRL_HEIGHT_IS_OBJECT_HEIGHT ) ) {
		work->control.height = ObjectHeight( work ) ;
//		work->control.height = GV_NearExp4F( p_ctrl_height, ObjectHeight( work ) ) ;
		work->control.hzx_base = work->control.mov.vy - work->control.height ;
    } 

    /* 床あたりチェックしてないときはステップ値をいれる */
    if ( work->control.skip_flag & CTRL_SKIP_FLR_CHECK ) {
        if ( !Flag( FLAG_NO_MOTION_STEP_Y ) ) {
//			work->control.step.vy = work->body.step->vw ;
			work->control.step.vy = work->body.height - p_height ;
		}
    }

    /* ステップ調整 */
    CheckStep( work ) ;

    /* 強制終わり１ */
    if ( Flag( FLAG_FORCE_END ) ) {
		/* 腰高さの補完をなくす */
		work->body.m_ctrl->height = work->control.height ;
		work->body.m_ctrl->root_height = work->control.height ;
		/* モーションファイルチェンジ */
		if ( !( ForceWork.flag & FA_USE_DEFAULT ) ) {
			PL_ChangeMotionArc( work, work->org_motion ) ;
		}	
    }

    /* ルート位置と実位置との差 */
    if ( !( work->body.m_ctrl->mt3_ctrl->flag & MT3_SLEEP ) &&
		 !Flag( FLAG_SET_FORCE_NOW ) ) {
		work->root_diff.vx += work->body.m_ctrl->root_step.vx - work->body.m_ctrl->step.vx ;
		work->root_diff.vz += work->body.m_ctrl->root_step.vz - work->body.m_ctrl->step.vz ;
		work->root_diff.vy = work->body.m_ctrl->root_height - work->body.m_ctrl->height ;
    } 

    /* 前回位置の記録 */
    DG_COPY_VEC( &work->pre_mov, &work->control.mov ) ;
	GV_MatToVec( &work->body.objs->world, &work->pre_body_mov ) ;

    work->pre_turn = work->control.rot ;
//printf( "<%d> %f %f %f\n", GV_Time, work->control.step.vx,
//	     work->control.step.vy, work->control.step.vz ) ;

#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_HAZARDCHECK ) {
		extern int DG_OnePieceSkip ;

		DG_OnePieceSkip = 1 ;
		HZX_NearDebug = 1 ;
		HZX_LevelDebug = 1 ;
	} 
	if ( PlayerDebugMenuStatus & PDMS_HAZARDCHECK_R ) {
		extern int DG_OnePieceSkip ;
		DG_OnePieceSkip = 1 ;
	}	
#endif
	/* 床に付いていないときは、
	   トラップチェック位置腰
	   （強制中を除く）*/
	/* 主観で移動量なし指定のときも */
	if ( !Status( PLAYER_FORCE ) && 
		 !( work->control.skip_flag & CTRL_SKIP_FLR_CHECK ) &&
		 !( work->control.grounded & 1 ) ) {
		work->control.root_offset = NULL ;
	}
	if ( Status( PLAYER_WATCH ) && !Flag2( FLAG2_SUBJECT_MOVE ) ) {
		work->control.root_offset = NULL ;
	}

	if ( work->control.root_offset == NULL || 
		 work->control.link_zone != NULL ) {
		SetFlag2( FLAG2_TRPCHECK_NOROOT ) ;
	} else {
		UnsetFlag2( FLAG2_TRPCHECK_NOROOT ) ;
	}

	/* トラップチェック位置と腰とのオンラインチェック */
	/* 水平方向のみ */
	if ( !Flag2( FLAG2_TRPCHECK_NOROOT ) && !Status( PLAYER_FORCE | PLAYER_IN_THE_WATER ) ) {
		FVECTOR		root ;
		float		len ;

		_sceVu0AddVector( &root, &work->control.mov, &work->root_diff ) ;
		root.vy = work->control.mov.vy ;
		if ( HZX_OnlineHazardCheck( work->control.hzx_id, &work->control.mov, &root,
								    HZX_CHK_SEGMENT, HZX_SEG_NO_PLAYER, 0 ) ) {
			printf( "mov - root online\n" ) ;
			printf( "%f %f %f - %f %f %f\n", 
				    work->control.mov.vx, work->control.mov.vy, work->control.mov.vz,
				    root.vx, root.vy, root.vz ) ;				   
			HZX_GetOnlineVector( &root ) ;
			len = GV_VecLen3F( &root ) ;
			if ( len > 1.0F ) GV_LenVec3F( &root, &work->root_shift, 0.0F, len - 1.0F ) ;
			work->root_shift.vy = work->root_diff.vy ;
			work->control.root_offset = &work->root_shift ;
		}
	}


    GM_ActControl( &( work->control ) ) ;
#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & ( PDMS_HAZARDCHECK | PDMS_HAZARDCHECK_R ) ) {
		extern int DG_OnePieceSkip ;

		HZX_NearDebug = 0 ;
		HZX_LevelDebug = 0 ;
		DG_OnePieceSkip = 2 ;
	}
#endif

	/* トラップチェック位置初期化 */
	work->control.root_offset = &work->root_diff ; 

    work->floor_atr = work->control.flr_atrs[ 0 ] ;
    /* ＳＥ変換用 */
    MT_SetMotionSeTable( work->body.m_ctrl, GM_CurrentMap, work->se_tableID, 
						 work->seNoFlr, work->seNoSeg ) ;
    /* 強制終わり２ */
    if ( Flag( FLAG_FORCE_END ) ) {
		/* 当たりチェック再開 */
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK ) ;
		if ( !( ForceWork.flag & FA_NO_RECHECK_TRP ) ) {
			work->control.skip_flag &= ~CTRL_SKIP_TRAP ;
		}
		/* 向き変更 */
		if ( ForceWork.e_turn >= 0 ) {
			work->control.turn.vy = work->control.rot.vy = ForceWork.e_turn ;
			ForceWork.e_turn = -1 ;
		}
		UnsetStatus( PLAYER_FORCE ) ;
		ForceWork.flag = 0 ;
    }
	if ( Flag( FLAG_RECHECK_HAZARD_READY ) ) {
		UnsetFlag( FLAG_RECHECK_HAZARD_READY ) ;
		work->control.skip_flag &= ~( CTRL_SKIP_SEG_CHECK | CTRL_SKIP_FLR_CHECK |
									  CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_ONLINE_CHECK ) ;
	}

    MoveHumanMA( work->left_leg, work->vanime_flag ) ;
    
    if ( work->stance != GROUND && !Flag( FLAG_NO_IK ) && 
		 work->stance != SQUAT && 
		 ( work->control.grounded & 1 ) ) {
		if ( Status( PLAYER_SNAKE ) ) Leg_IKcalc( &( work->control ), &( work->body ) ) ;
		else 						  Leg_IKcalcForRaiden( &( work->control ), &( work->body ) ) ;
    }

	/* サイドステップ時のモーション付けＳＥ対策 */
	if ( Flag2( FLAG2_OBJ_SHIFT ) ) {
		FMATRIX		mat ;

		DG_GetPos( &mat ) ;
		_sceVu0AddVector( ( FVECTOR * )mat.m[ 3 ], ( FVECTOR * )mat.m[ 3 ], &PL_ObjShift ) ;
		DG_SetPos( &mat ) ;
	}

	GM_ActObject2( &( work->body ) ) ;

	/* サイドステップ時のモーション付けＳＥ対策リセット */
	if ( Flag2( FLAG2_OBJ_SHIFT ) ) {
		int			i ;
		DG_OBJ		*obj ;

		_sceVu0SubVector( ( FVECTOR * )work->body.objs->world.m[ 3 ], 
						  ( FVECTOR * )work->body.objs->world.m[ 3 ], &PL_ObjShift ) ;
		obj = work->body.objs->objs ;
		for ( i = 0; i < N_UNITS; i ++, obj ++ ) {
			_sceVu0SubVector( ( FVECTOR * )obj->world.m[ 3 ], 
							  ( FVECTOR * )obj->world.m[ 3 ], &PL_ObjShift ) ;
		}
		UnsetFlag2( FLAG2_OBJ_SHIFT ) ;
	}

	if ( Flag( FLAG_RESET_MOTION2_AT_END ) ) {
		if ( ( work->work_l.motion2cutframe <= 0 && EndMotion2( work ) ) ||
			 ( work->work_l.motion2cutframe > 0 && 
			   PL_MotionTime2( work ) >= work->work_l.motion2cutframe ) ) {
			SetAction2( work, -1, 6, 0x01ffe ) ;
			SetArmAction( work, AMstand ) ;
		}
	}

    DG_GetLightMatrix( ( FVECTOR * )( &( work->control.mov ) ), work->lights ) ;

    if ( work->control.grounded & 1 ) work->control.step.vy = 0.0F ;
    if ( !Flag( FLAG_NO_GRAVITY ) ) work->control.step.vy -= GRAVITY ;
    SetCamera( work ) ; 

#if defined(BP_VITA)
    if (Status(PLAYER_LOCKER))
    {
      GestureConfigureDeadZone(kGesture_DeadZone_None);
    }
    else
    {
       GestureConfigureDeadZone(kGesture_DeadZone_Default);
    }
#endif

    Action( work ) ;

	/* カメラセット */
    PutCamera( work ) ;
    PushEnemy( work ) ;

	/* 回転・高さ調整 */
	RotAndHeightAdjust( work ) ;

	if ( work->control.grounded & 1 ) {
		PL_PlayerFloorHeight = work->control.levels[ 0 ] ;
	}

	/* カットインデモで強制可視 */
	if ( ( GM_CheckGameStatus( STATE_CUT_IN ) || Flag2( FLAG2_FORCE_VISIBLE ) ) &&
		( work->body.objs->flag & DG_FLAG_INVISIBLE0 ) &&
		(PL_SubjectMove ? !GM_IsGameOver() : 1) /*主観移動モードではゲームオーバー時に入らない*/ ) {
		SetFlag2( FLAG2_CUTIN_DEMO_VISIBLE ) ;
		DG_VisibleObjsChanl( work->body.objs, 0 ) ;
		work->weapon_body = GM_PlayerBody ;
		if ( work->body.objs->objs[ 0 ].flag & DG_FLAG_INVISIBLE0 ) {
			SetFlag2( FLAG2_CUTIN_DEMO_VISIBLE_ALLOBJS ) ;
			PL_VisibleObjsAll( work->body.objs, 0 ) ;
			/* ゴルキャップ中は頭首関節非表示 */	
			if ( work->item == IT_Uniform &&
				( GM_PlayerStateFlag & PL_GBSCAP_EXIST ) ) {
				PL_VisibleHead( 0, work->body.objs, -1 ) ;
#if 0
				work->body.objs->objs[ HUMAN21_KUBI ].flag |= DG_FLAG_INVISIBLE ;
				work->body.objs->objs[ HUMAN21_ATAMA ].flag |= DG_FLAG_INVISIBLE ;
#endif
			}
		}
		/* 主観腕は消す */
		if ( !( work->arm_trigger & ARM_INVISIBLE ) ) {
			SetFlag2( FLAG2_CUTIN_DEMO_INVISIBLE_ARM ) ;
			work->arm_trigger |= ARM_INVISIBLE ;
		}
		GM_DisablePauseCount = 2 ;	/* ポーズ禁止 */
	}


	if( PL_PlayerInvisible != PL_PlayerInvisiblePrev ) {
		if( PL_PlayerInvisible  ) {
			work->body.objs->flag |=  DG_FLAG_INVISIBLE ;
		} else {
			work->body.objs->flag &= ~DG_FLAG_INVISIBLE ;
		}
			PL_PlayerInvisiblePrev = PL_PlayerInvisible ;
	}



	
	/* デモ中などで不可視 */
	if ( Flag( FLAG_DEMO_INVISIBLE_ORDER ) &&
		 !( work->body.objs->flag & DG_FLAG_INVISIBLE0 ) ) {
		SetFlag( FLAG_DEMO_INVISIBLE ) ;
		DG_InvisibleObjsChanl( work->body.objs, 0 ) ;
		/* 主観腕も消す */
		if ( !( work->arm_trigger & ARM_INVISIBLE ) ) {
			SetFlag2( FLAG2_CUTIN_DEMO_INVISIBLE_ARM ) ;
			work->arm_trigger |= ARM_INVISIBLE ;
		}
		/* 武器も消す */
		GM_SetPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ;
	}


#ifdef DEBUG_MODE
	ActPlayerDebugMenu( work ) ;
#endif
#if 0
	if ( Status( PLAYER_SNAKE ) ) {
		/* ステルスのとき装備品を消す（スネーク） */
		if ( Status( PLAYER_STEALTH ) ) {
			if ( work->attach_work != NULL ) {
				GV_DestroyOtherActor( work->attach_work ) ;
				work->attach_work = NULL ;
			}
		} else {
			if ( work->attach_work == NULL ) {
				static int InitEquip( Work *, int ) ;
				InitEquip( work, work->control.name ) ;
			}
		}
	}
#endif

#ifdef KP_XBOX
	/*主観移動用モードのみ　武器によって曲線を変える*/
	if ( PL_SubjectMove ) {
		if ( ((WeaponSet *)work->wp_set)->type & WP_TYPE_PRESSURE ){
			GV_SetChangePressure( PAD_PRESS_R1, 2 ) ;
		} else {
			GV_SetChangePressure( PAD_PRESS_R1, 1 ) ;
		}
	}
#endif
}

static	void Die( Work *work )
{
   // Armature add to clear up knocking touch detector
   PL_LeaveCaution(work);

   GM_PlayerStance = Status( PLAYER_INTRUDE ) ? INTRUDE : work->stance ;
    if ( work->wp_act != NULL ) GV_DestroyOtherActorQuick( work->wp_act ) ;
    if ( work->it_act != NULL ) GV_DestroyOtherActorQuick( work->it_act ) ;
    ExitHumanMA( work->left_leg ) ;
    if(work->attach_work!=NULL) GV_DestroyOtherActor(work->attach_work);
    GM_RemoveGageSet( &work->life ) ;
    GM_RemoveGageSet( &work->hand_power ) ;
    GM_FreeRadarControl( &( work->radar ) ) ;
    GM_FreeControl( &( work->control ) ) ;
	if ( work->shadow_obj != NULL ) DG_FreeObjs( work->shadow_obj ) ;
    GM_FreeObject( &( work->body ) ) ;
    GM_FreeTarget( &( work->def ) ) ;
    GM_DeleteCamera( work->subject_camera ) ;
    GM_DeleteCamera( work->behind_camera ) ;
    GM_DeleteCamera( work->weapon_camera ) ;
    GM_PlayerBody = NULL ;
    GM_PlayerControl = NULL ;
    GM_PlayerTarget = NULL ;
    GM_PlayerSubjectCamera[ work->chanl ] = NULL ;
#ifdef PAD_PRESSURE_CHECK
	{
		GM_GageSet	  	*pgs ;
		int			i ;

		pgs = PressureGage ;
	
		for ( i = 0; i < 12; i ++ ) {
			GM_RemoveGageSet( pgs ) ;
			pgs ++ ;
		}
	}
#endif
    GM_PlayerWeaponBody = NULL ;
    GM_PlayerSubWeaponBody = NULL ;
	GM_PlayerWork = NULL ;

	PL_ObjectFlag  = 0 ;/*クリアしておく T.Morita*/
	PL_SubjectMove = 0 ;
#ifdef KP_WINDOWS
	GV_SetWindowsInputConfigulationMode(FALSE) ;
#endif

	PL_SaveStatus() ;
}

/*----------------------------------------------------------------*/

/* コントロール初期化 */
static	int		InitControl( work, name )
Work		*work ;
int		name ;
{
    CONTROL	*ctrl ;
	int	   	flag ;

    ctrl = &( work->control ) ;
    if ( GM_InitControl( ctrl, RAIDEN_NAME, 0 ) < 0 ) return -1 ;
    ctrl->root_offset = &work->root_diff ; 
    ctrl->name = name ;
    ctrl->levels[ 0 ] = 0.0F ;
    ctrl->levels[ 1 ] = 32000.0F ;
    ctrl->mov = DG_ZeroVector ; 
    ctrl->step = DG_ZeroVector ;
    ctrl->rot = DG_ZeroSVector ;
    ctrl->turn = DG_ZeroSVector ;

    GM_ConfigControlObject( ctrl, &( work->body ) ) ;
    GM_ConfigControlHazard( ctrl, 1000, NORMAL_SPHERE, CHECK_SPHERE ) ;
    GM_ConfigControlTrapCheck( ctrl ) ;
	GM_ConfigControlAttribute( ctrl, CTRL_ATR_ITEMCHECK | CTRL_ATR_PITFALLCHECK | 
							   CTRL_ATR_NEARONLINE ) ;
    HZX_SetEvent( &( ctrl->evt ), name, &ctrl->mov, &ctrl->rot ) ;
	ctrl->evt.flag |= HZX_EVT_FLAG_CHECKALL ; /* ブロック外のときは近い奴を検索 */

	//    GM_ConfigControlMessageCheck( ctrl ) ;
    GM_ConfigControlCameraTrapCheck( ctrl ) ;
    GM_ConfigControlMapCheck( ctrl ) ;

    ctrl->seg_flag |= HZX_SEG_NO_PLAYER ;
    ctrl->flr_flag |= HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ;
    ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;

    GM_PlayerControl = ctrl ;

	flag = GCL_GetOptionValue( 'f', 0 ) & ( START_FLAG_IS_SNAKE | START_FLAG_IS_RAIDEN ) ;

    if ( !( flag & START_FLAG_IS_RAIDEN ) && 
		( name != RAIDEN_NAME || ( flag & START_FLAG_IS_SNAKE ) ) ) {
		SetStatus( PLAYER_SNAKE ) ;
		M92Set[ 2 ] = NO_ACT ;
		M92Set[ 6 ] = NO_ACT ;
		M92Set[ Mcrouch ] = none_crouch ;
		M92Set[ Mcrouch_f ] = none_crouch_f ;
		M92Set[ Mcrouch_b ] = none_crouch_b ;
		M92Set[ Mcrouch_f_fast ] = none_crouch_f_fast ;
		M92Set[ Mknock_sq1 ] = m4a_knock_sq1 ;
		M92Set[ Mknock_sq2 ] = m4a_knock_sq2 ;
	} else {
		/* ライデンはＭ９壁チェックモーションあり */
		M92Set[ 2 ] = m9_wallcheck_idle ;
		M92Set[ 6 ] = m9_wallcheck_move ;
		M92Set[ Mcrouch ] = m4a_crouch_idle ;
		M92Set[ Mcrouch_f ] = m4a_crouch_move_f ;
		M92Set[ Mcrouch_b ] = m4a_crouch_move_b ;
		M92Set[ Mcrouch_f_fast ] = m4a_crouch_move_f_fast ;
		M92Set[ Mknock_sq1 ] = non_knock_sq1 ;
		M92Set[ Mknock_sq2 ] = non_knock_sq2 ;
	}

	return 0 ;
}

/* 装備品を初期化 */
static	int		InitEquip( Work *work, int name )
{
    work->attach_work = NULL ;
    if ( name != RAIDEN_NAME && 
		 GV_GetCache( GV_CacheID( 0x2bc559, 'k' ) ) != NULL ) {
		static const FVECTOR mag1 = { 95.0f,85.0f,107.5f,0.0f } ;
		static const FVECTOR mag2 = { -95.0f,85.0f,107.5f,0.0f } ;
		static const FVECTOR mag3 = { 72.5f,37.5f,120.0f,0.0f } ;
		static const FVECTOR mag4 = { -72.5f,37.5f,120.0f,0.0f } ;

		static const ATTACHMENT_ARGUMENT3 attch[] = {
			{
				0x2bc559		/* GV_StrCode("sna_mag1") */ ,
				NULL,
				2,				/* 胸 */
				( FVECTOR * )&mag1,
				3,				/* frames */
			},
			{
				0x2bc55a		/* GV_StrCode("sna_mag2") */ ,
				NULL,
				2,				/* 胸 */
				( FVECTOR * )&mag2,
				3,				/* frames */
			},
			{
				0x2bc55b		/* GV_StrCode("sna_mag3") */,
				NULL,
				2,				/* 胸 */
				( FVECTOR * )&mag3,
				3,				/* frames */
			},
			{
				0x2bc55c		/* GV_StrCode("sna_mag4") */,
				NULL,
				2,				/* 胸 */
				( FVECTOR * )&mag4,
				3,				/* frames */
			},
		};

		work->attach_work
			= NewAttachments_called(&(work->body),
									NULL, 0,
									NULL, 0,
									attch, sizeof(attch)/sizeof(attch[0]) ) ;
    }
	return 0 ;
}

/* オブジェクトを初期化 */
static	int		InitObject( work, name )
Work			*work ;
int				name ;
{
    OBJECT	*body ;
    int		model ;

	printf( "raiden.c : INITOBJECT\n" );

    body = &( work->body ) ;
    if ( GCL_GetOption( 'm' ) == NULL ) {
		GM_InitObject( body, MODEL_NAME, OBJECT_FLAG| PL_ObjectFlag ) ;
		model = MODEL_NAME ;
    } else {
		model = GCL_GetNextInt() ;
		GM_InitObject( body, model, OBJECT_FLAG| PL_ObjectFlag ) ;
    }
	if ( body->objs == NULL ) return -1 ;

    GM_ConfigObjectLight( body, work->lights ) ;

    if ( GCL_GetOption( 'a' ) == NULL ) {
		work->org_motion = MOTION_NAME ;
		GM_ConfigObjectMotion( body, 2, MOTION_NAME, MT_FLAG_HUMAN2 | MT_FLAG_MOVE_EX ) ;
    } else {
		work->org_motion = GCL_GetNextInt() ;
		GM_ConfigObjectMotion( body, 2, work->org_motion, MT_FLAG_HUMAN2 | MT_FLAG_MOVE_EX ) ;	

    }

#if 0
	body->m_ctrl->mt3_ctrl[ 1 ].move = &work->override_move ;
	body->m_ctrl->mt3_ctrl[ 1 ].move2 = &work->override_move2 ;
	body->m_ctrl->mt3_ctrl[ 1 ].root = &work->override_root ;
#endif
	work->current_mar = work->org_motion ;
    GM_ConfigObjectStep( body, &( work->control.step ) ) ;
    GM_PlayerBody = body ;

	printf( "raiden.c : INITOBJECT end\n" );

	/* 振動 */
	GM_InitVarControl( &work->actor, &work->vctrl, work->org_motion ) ;
	GM_ConfigVctrlMctrl( &work->vctrl, work->body.m_ctrl ) ;

    /* 頂点アニメ */
    {
		CV2_DEF		*cv2_def ;

		cv2_def = GV_GetCache( GV_CacheID( model, 'c' ) ) ;
		if ( Status( PLAYER_SNAKE ) ) {
			work->left_leg = InitHumanLegMA( work->body.objs, cv2_def, 
											HUMAN21_HIDARI_ASHI1, 
											DG_VANIME_VERTS | DG_VANIME_NORMS,	
											4 ) ;
		} else {
			work->left_leg = InitHumanLegMA( work->body.objs, cv2_def, 
											HUMAN21_MIGI_ASHI1, 
											DG_VANIME_VERTS | DG_VANIME_NORMS,	
											4 ) ;
		}
		work->vanime_flag = 0 ;
    }
	/* ライデンモーションブラー（スネークもやる） */
	//if ( !Status( PLAYER_SNAKE ) )
	{
		extern void  *PL_PlayerActionBlurManager( OBJECT *body, u_long64 mask, int *enable_flag ) ;
		GV_SetActorChild( work, 
						  PL_PlayerActionBlurManager( &work->body, 0x1fe770, &work->actblur_flag ) ) ;
	}

    /* 装備品 */
	InitEquip( work, name ) ;

	/* ローポリ影 */
	{
		DG_DEF			*def ;
		DG_OBJS			*objs ;
		int				name ;
		
		name = Status( PLAYER_SNAKE ) ?
		  GV_StrCode( "sna_shadow" ) :
		  GV_StrCode( "rai_shadow" ) ;

		def = ( DG_DEF * )GV_GetCache( GV_CacheID( name, 'k' ) ) ;
		if ( !def ) {
			printf( "raiden.c : No shadow model<%d>\n", name ) ;
			return  0 ;
		}
		objs = work->shadow_obj = DG_MakeObjs( def, DG_FLAG_FINISHCALC, 0 ) ;
		if ( !objs ) {
			printf( "raiden.c : Cannot make shadow model(no memory)\n" ) ;
			return  0 ;
		}
		DG_SetLowObjs( work->body.objs, objs ) ;
		if ( work->body.objs->low == NULL ) {
			/* 接続失敗 */
			DG_FreeObjs( objs ) ;
			work->shadow_obj = NULL ;
		}
	}

	return 0 ;
}

/* 初期設定値を取得 */
static	void	GetOptionValue( work )
Work		*work ;
{
    int		buf[ 3 ], stance, flag ;
    float	level ;

    /* 初期位置 */
    if ( GCL_GetOption( 'p' ) != NULL ) {
		GCL_GetIV( GCL_NextStr(), buf ) ;
		IVtoFV( buf, ( float * )&( work->control.mov ) ) ;
    } 
    DG_COPY_VEC( &work->pre_mov, &work->control.mov ) ;
	DG_COPY_VEC( &work->pre_body_mov, &work->control.mov ) ;
    work->root_diff = DG_ZeroVector ;
    GM_ConfigControlMapID( &work->control ) ;
    GM_PlayerAddress = work->control.addr ;

    /* 初期方向 */
    if ( GCL_GetOption( 'd' ) != NULL ) {
		GCL_GetIV( GCL_NextStr(), buf ) ;
		work->control.rot.vx = work->control.turn.vx = buf[ 0 ] ;
		work->control.rot.vy = work->control.turn.vy = buf[ 1 ] ;
		work->control.rot.vz = work->control.turn.vz = buf[ 2 ] ;
    }    
    work->pre_turn = work->control.rot ;

    /* 初期床高さ */
#if 0
    if ( GCL_GetOption( 'l' ) != NULL ) {
		level = ( float )GCL_GetNextInt() ;
    } else {
		level = 0.0F ;
    }
#else
    if ( HZX_LevelHazardCheck( work->control.hzx_id,
							  &( work->control.mov ),
							  ( HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR ), 
							  HZX_FLOOR_NO_PLAYER | HZX_FLOOR_IK ) ) {
		level = HZX_GetFloorLevel() ;
    } else {
		level = 0.0F ;
    }
#endif

    /* 初期姿勢 */
	SetFlag2( FLAG2_STAGESTART ) ;
	work->stance = -1 ;
	stance = GCL_GetOptionValue( 's', GM_PlayerStance ) ;
	PL_ChangeStance( work, ( stance == INTRUDE ) ? GROUND : stance ) ;
	UnsetFlag2( FLAG2_STAGESTART ) ;
    GM_ConfigControlHzxHeight( &( work->control ), HzxHeight[ stance ], level ) ;
    work->hzx_height = HzxHeight[ stance ] ;
    /* 初期武器 */
    GM_PlayerWeaponBody = NULL ;
    GM_PlayerSubWeaponBody = NULL ;
    work->weapon_body = &( work->body ) ;
    SetWeapon( work, 0 ) ;
    SetMotionSet( work->weapon ) ;
    /* 初期アイテム */
    work->item_body = &( work->body ) ;
    SetItem( work, &PL_ItemSets[ GM_Item ], 0 ) ;

    /* 初期モーション */
    GM_ConfigObjectAction( &( work->body ), 0, StanceAct( stance ), 0, 0xfffff, 0 ) ;
    work->motion1 = StanceAct( stance ) ;
    work->motion2 = -1 ;

    /* ＳＥ変換テーブルＩＤ */
    if ( GCL_GetOption( 'i' ) != NULL ) {    
		work->se_tableID = GCL_GetNextInt() ;
    } else {
		work->se_tableID = 0 ;
    }
    /* 強制モーション */
    if ( GCL_GetOption( 'o' ) != NULL ) {    
		ForceWork.marfile = GCL_GetNextInt() ;
    } else {
		ForceWork.marfile = -1 ;
    }
	PL_ClearForceActProc() ;

    /* 初期モード */
	if ( !( WeaponType( work ) & WP_TYPE_CALLFUNC ) &&
		 !( ItemType( work ) & IT_TYPE_CALLFUNC ) ) {
		SetMode( work, StillMode[ stance ] ) ;
		if ( stance == INTRUDE ) IntoIntrude( work ) ;
	}
    SetMode2( work, NULL ) ;

	GM_PlayerStance = Status( PLAYER_INTRUDE ) ? INTRUDE : work->stance ;

    /* 開始時フラグによる設定 */
    flag = 0 ;
    if ( GCL_GetOption( 'f' ) != NULL ) flag = GCL_GetNextInt() ;
    if ( flag & START_FLAG_BODYSPLASH ) {
		/* 飛沫 */
		NewBodySplash( work->body.objs, &work->control, 3075579 ) ;
		NewFootSplash( &work->body, &work->control ) ;
    }
    if ( !( flag & START_FLAG_NOFOOTSHADOW ) ) {
		/* 影起動 */
		GV_SetActorChild( work, NewShadow( &work->body.objs->objs[16], &work->body.objs->objs[20],   
										  &work->control, work->lights, &work->shadow ) ) ;
#if 0
		GV_SetActorChild( work, NewDropShadow( &work->body,   
											  &work->control, work->lights, &work->shadow ) ) ;
#endif
		work->shadow = 1 ;
    }
    /* 敵ライト影 */
    NewBodyShadow( work->body.objs ) ;
	/* 足跡 */
	work->foot_work = PL_FootPrintManager( work ) ;
	GV_SetActorChild( work, work->foot_work ) ;
	
    /* ホーミング設定 */
    if ( GCL_GetOption( 'h' ) != NULL ) {
		HomingLen = ( float )GCL_GetNextInt() ;
		HomingRange = GCL_GetNextInt() ;
    } else {
		HomingLen = 4500.0F ;
		HomingRange = 768 ;
    }
}

/* ターゲット初期化 */
static	void	InitTarget( work, name )
Work		*work ;
int		name ;
{
    TARGET	*t ;
    FVECTOR	size ;
    FMATRIX	mat ;

    t = &( work->def ) ;
    GM_PlayerTarget = t ;
    size.vx = size.vz = 100.0F ; size.vy = 100.0F ;
    GM_SetTarget( t, TARGET_DEFENSE | TARGET_ROTATE, 0, PLAYER_SIDE,
				 &size, &DG_ZeroVector ) ;
    GM_SetTargetName( t, name ) ;
    GM_SetPowerTarget( t, &( work->power ), POWER_DECREASE, GM_Vitality, 0, 0,
					  &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, TargetCallBack, work ) ;
    DG_SetPos2( &work->control.mov, &work->control.rot ) ;
    DG_GetPos( &mat ) ;
    GM_MoveTarget2( t, &mat ) ;
    GM_PutTarget( t ) ;
	//NewTargetView( &( work->def ), 32, 232, 186 ) ;

	t = &( work->touch ) ;
	size.vx = size.vz = 160.0F ; size.vy = 400.0F ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_TOUCH | TARGET_CHECK_ONE,
				  0, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_BODY ) ;
	GM_SetTargetCallBack( t, TouchCallback, work ) ;
}

/* レーダー初期化 */
static	void	InitRadar( work )
Work		*work ;
{
//    GM_InitRadarControl( &( work->radar ), &( work->control.mov ), 
//						RADAR_VISIBLE, -1 ) ;
    GM_InitRadarControl( &( work->radar ), &GM_PlayerFindPos, 
						RADAR_VISIBLE, -1 ) ;
}

/* カメラ関係 */
/* そのうちシステムへ */
static	void	InitCamera( work )
Work		*work ;
{
    GM_CameraTarget = work->control.mov ;
}

/* 主観カメラ用補完関数 */
static	void	SubjectCameraInterpFuncIn( chanl, cam1, cam2 )
GM_CameraChanl	*chanl ;
GM_CameraSet	*cam1, *cam2 ;
{
    if ( cam1->level == GM_CAMERA_SUBJECT ) {
		GM_CameraInterpExp4( chanl, cam1, cam2 ) ;
    } else {
		GM_CameraInterpIntoSubject( chanl, cam1, cam2 ) ;
    }
}

/* 主観カメラ初期化 */
static	void	InitSubjectCamera( work )
Work		*work ;
{
    GM_CameraSet	*cam ;
    int			name ;

    name = 7532342 ;// GV_StrCode( "主観カメラ" ) ;
    name += GM_CurrentCameraChanl ;
    work->subject_camera = cam = NewProgramCamera( name, GM_CurrentCameraChanl, 
												  GM_CAMERA_SUBJECT, SUBJECT_CAMERA_PRIO ) ;
    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_ROTATE, 
					 CAM_FLAG_DIR_TRACE | CAM_FLAG_PAD_ADJUST ) ;
    GM_SetCameraRotate( cam, &( work->control.rot ) ) ;
    GM_SetCameraTrack( cam, 1000 ) ;
    GM_SetCameraAngle( cam, 2.0F ) ;
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
		/* はだかのときは入りをクイックに */
		GM_SetCameraInterpMode( cam, GM_CAM_INTERP_QUICK,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
	} else {
		GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_SUBJECT, 
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
	}
    GM_PlayerSubjectCamera[ GM_CurrentCameraChanl ] = cam ;
}

/* ビハインドカメラ初期化 */
static	void	InitBehindCamera( work )
Work		*work ;
{
    GM_CameraSet	*cam ;
    int			name ;

    name = 11911073 ; //GV_StrCode( "ビハインドカメラ" ) ;
    work->behind_camera = cam = NewProgramCamera( name, GM_CurrentCameraChanl, 
												 GM_CAMERA_BEHIND, 0 ) ;
    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_TARGET, 
		      CAM_FLAG_FIX | CAM_FLAG_PAD_ADJUST_NO_RESET ) ;
    GM_SetCameraAngle( cam, 2.0F ) ;
    GM_SetCameraInterpMode( cam, GM_CAM_INTERP_EXP8, GM_CAM_INTERP_BEHIND,
			    0, 0 ) ;
}

/* 武器カメラ初期化 */
static	void	InitWeaponCamera( work )
Work		*work ;
{
    GM_CameraSet	*cam ;
    int			name ;

    name = 1832632 ; //GV_StrCode( "武器カメラ" ) ;
    work->weapon_camera = cam = NewProgramCamera( name, GM_CurrentCameraChanl, 
												 GM_CAMERA_SUBJECT, WEAPON_CAMERA_PRIO ) ;
    GM_SetCameraType( cam, GM_CAM_TYPE_CAMERA_AND_ROTATE, 
					 CAM_FLAG_PAD_ADJUST ) ;
    GM_SetCameraRotate( cam, &( work->control.rot ) ) ;
    GM_SetCameraTrack( cam, 1000 ) ;
    GM_SetCameraAngle( cam, 2.0F ) ;
	if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
		/* はだかのときは入りをクイックに */
		GM_SetCameraInterpMode( cam, GM_CAM_INTERP_QUICK,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
	} else {
		GM_SetCameraInterpMode( cam, GM_CAM_INTERP_INTO_SUBJECT,  
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
	}
    cam->interp_func1 = SubjectCameraInterpFuncIn ;
}

/* 腕起動 */
static	inline	void	InitArm( work ) 
Work		*work ;
{
	extern	void	*NewPlayerArm( CONTROL *, OBJECT *, int, int, int, 
								   u_char *, u_char *, short *, int *, int ) ;
	static 	int		ArmDatas[] = {
		/* スネーク */
		5518597,	/* snh_def_mh_mt */
		13298953,	/* snh_def_dummy */
		2697888,	/* sna_arm_mh */
		/* ライデン */
		5512837,	/* rah_def_mh_mt */
		13293193,	/* rah_def_dummy */
		16542367,	/* rai_arm_mh */
		/* はだか */
		14449978,	/* rah_naked_mh_mt */
		13293193,	/* rah_def_dummy */
		16542367,	/* rai_arm_mh */
	} ;
	int		*datas, armevm ;
	int		buf[ 4 ] ;

	/* 腕起動時に位置と方向がいるので
	   初期位置設定だけここでやってしまう */

    /* 初期位置 */
    if ( GCL_GetOption( 'p' ) != NULL ) {
		GCL_GetIV( GCL_NextStr(), buf ) ;
		IVtoFV( buf, ( float * )&( work->control.mov ) ) ;
    } 
    /* 初期方向 */
    if ( GCL_GetOption( 'd' ) != NULL ) {
		GCL_GetIV( GCL_NextStr(), buf ) ;
		work->control.rot.vx = work->control.turn.vx = buf[ 0 ] ;
		work->control.rot.vy = work->control.turn.vy = buf[ 1 ] ;
		work->control.rot.vz = work->control.turn.vz = buf[ 2 ] ;
    }    

    work->arm_trigger = ARM_NORMAL | ARM_INVISIBLE ;
	/* 新主観腕 */	
	if ( Status( PLAYER_SNAKE ) ) datas = ArmDatas ;
	else if ( GM_CheckPlayerStatusEX( I64(0), PLAYER2_NUDE ) ) {
		datas = ArmDatas + 6 ;
	} else {
		datas = ArmDatas + 3 ;
	}

	armevm = GCL_GetOptionValue( 'R', datas[ 0 ] ) ;

	if ( GV_GetCache( GV_CacheID( datas[ 0 ], 'e' ) ) != NULL ) {
		work->arm = NewPlayerArm( &work->control, &work->body, 
								 armevm,
								 datas[ 1 ],
								 datas[ 2 ],
								 &work->arm_motion,
								 &work->arm_interp,
								 &work->arm_start_time,
								 &work->arm_trigger,
								 work->chanl ) ;
		GV_SetActorChild( work, work->arm ) ;
	}
}

/* ゲージ */
static	void	InitGage( work ) 
Work		*work ;
{
    GM_GageSet	*gs ;
	char		g_name[ 32 ] ;
	char		r1, g1, b1, r2, g2, b2 ;

    gs = &work->life ;
    GM_InitGageSet( gs, "LIFE", 16, 160, GM_DEFAULT_GAGE_HEIGHT, 
				   GM_Vitality, GM_VitalityMax, 0, 30, GM_GAGE_LEVEL_PLAYER_LIFE ) ;
	GM_SetGageColorType( gs, GM_GAGE_COLOR_TYPE_PLAYER_LIFE ) ;
    //GM_SetGageColor( gs, 0, 0, 0, 40, 128, 118, 110, 190, 118, 255, 0, 0 ) ;

	/* Ｍ９ゲージテスト用 */
	//GM_InitGageM9( gs, 100, 100, 0, 30 ) ;

    GM_AppendGageSet( gs ) ;
	GM_VisibleGage( gs ) ;

	ELUDE_HAND_POWER_MAX = ELUDE_HAND_POWER_MAX_MAX ;
	if ( Status( PLAYER_SNAKE ) ) {
		if ( GM_SnakeChin_Up < ELUDE_HAND_POWER_LEVELUP1 ) {
			ELUDE_HAND_POWER_MAX = GM_SnakeGripMax[ 0 ] ;
			strcpy( g_name, "GRIP Lv1" ) ;
			r1 = 13 ; g1 = 78 ; b1 = 167 ;
			r2 = 14 ; g2 = 154 ; b2 = 175 ;
		} else if ( GM_SnakeChin_Up < ELUDE_HAND_POWER_LEVELUP2 ) {
			ELUDE_HAND_POWER_MAX = GM_SnakeGripMax[ 1 ] ;
			strcpy( g_name, "GRIP Lv2" ) ;
			r1 = 191 ; g1 = 102 ; b1 = 0 ;
			r2 = 220 ; g2 = 227 ; b2 = 100 ;
		} else {
			ELUDE_HAND_POWER_MAX = GM_SnakeGripMax[ 2 ] ;
			strcpy( g_name, "GRIP Lv3" ) ;
			r1 = 90 ; g1 = 90 ; b1 = 92 ;
			r2 = 150 ; g2 = 150 ; b2 = 152 ;
		}
	} else {
		if ( GM_RaidenChin_Up < ELUDE_HAND_POWER_LEVELUP1 ) {
			ELUDE_HAND_POWER_MAX = GM_RaidenGripMax[ 0 ] ;
			strcpy( g_name, "GRIP Lv1" ) ;
			r1 = 13 ; g1 = 78 ; b1 = 167 ;
			r2 = 14 ; g2 = 154 ; b2 = 175 ;
		} else if ( GM_RaidenChin_Up < ELUDE_HAND_POWER_LEVELUP2 ) { 
			ELUDE_HAND_POWER_MAX = GM_RaidenGripMax[ 1 ] ;
			strcpy( g_name, "GRIP Lv2" ) ;
			r1 = 191 ; g1 = 102 ; b1 = 0 ;
			r2 = 220 ; g2 = 227 ; b2 = 100 ;
		} else {
			ELUDE_HAND_POWER_MAX = GM_RaidenGripMax[ 2 ] ;
			strcpy( g_name, "GRIP Lv3" ) ;
			r1 = 90 ; g1 = 90 ; b1 = 92 ;
			r2 = 150 ; g2 = 150 ; b2 = 152 ;
		}
	}

    gs = &work->hand_power ;
    GM_InitGageSet( gs, g_name, 16, 196, 3, DIRECT_TICK( ELUDE_HAND_POWER_MAX ), 
				   DIRECT_TICK( ELUDE_HAND_POWER_MAX ), 0, 30, GM_GAGE_LEVEL_PLAYER_GRIP ) ;
//    GM_SetGageColor( gs, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 ) ;
    GM_SetGageColor( gs, 0, 0, 0, r1, g1, b1, r2, g2, b2, 255, 0, 0 ) ;
    GM_AppendGageSet( gs ) ;    
#ifdef PAD_PRESSURE_CHECK
    {
		static char	*Names[] = {
			"R", "L", "U", "D", "X", "A", "B", "Y",
			"L1", "R1", "L2", "R2"
		} ;
		GM_GageSet	  	*pgs ;
		int			i ;

		pgs = PressureGage ;
	
		for ( i = 0; i < 12; i ++ ) {
			GM_InitGageSet( pgs, Names[ i ], 16, 128, 3, 0, 255, 0, 0, 255 ) ;
			GM_SetGageColor( pgs, 0, 0, 0, 0, 142, 176, 0, 230, 220, 0, 0, 0 ) ;
			GM_AppendGageSet( pgs ) ;
			pgs ++ ;
		}
    }
#endif
}

/* プレイヤーパッドリリース用 */
static	inline	void	InitReleasePad( void )
{
    GV_ZeroMemory( &ReleasePad, sizeof( GV_PAD ) ) ;
    ReleasePad.dir = -1 ;
}



/* 資源を獲得 */
static	int 	GetResources( Work		*work, int name )
{
	int			which ;
   extern void *NewMenuPrimControl( int );

	if ( GM_GameStatus & STATE_VR_ANOTHER ) {
printf( "------------------------ change substans item buff!!!\n" ) ;
		GM_SetCurrentItemSet( 1 ) ;
	}

	/* パッド設定 */
	PL_COM_SetPadType() ;

	/* 主観移動用パッド設定 */
	if ( PL_SubjectMove ) {
	    void PL_SetPadTypeSubjectMove( void ) ;/* pl_pad.c */
	    PL_SetPadTypeSubjectMove() ;
	}
	
	which = ( name == RAIDEN_NAME ) ? 1 : 0 ;
    GM_PlayerWork = work ;
    /* 保険 */
    if ( GM_Vitality == 0 ) GM_Vitality = 1 ;
    /* 画面 */
    work->chanl = 0 ;			/* 今は０固定 */
    GM_CurrentCameraChanl = 0 ;

	CurrentChanl = work->chanl ;
	PL_MotionSet = &MS ;
	PL_Force = &ForceWork ;
	PL_Beyond = &Beyond ;

    work->pad = GV_PadData ;
    work->motion1 = 0 ;
    work->flag = 0 ;
    work->r_sphere = NORMAL_SPHERE ;
    if ( InitControl( work, name ) < 0 ) {ASSERT(0) ; return -1 ;}
    if ( InitObject( work, name ) < 0 ) {ASSERT(0) ; return -1 ;}
    SetCamera( work ) ;
    InitSubjectCamera( work ) ;
    InitBehindCamera( work ) ;
    InitWeaponCamera( work ) ;
    InitArm( work ) ;

	/* 主観カメラ制御 */
	GV_SetActorChild( work, PL_SubjectCameraManager( work->subject_camera,
													 work->weapon_camera, PL_SubjectMove ) ) ; /* 引数増やしたT.Morita Added*/
    GetOptionValue( work ) ;

    InitTarget( work, name ) ;
    InitRadar( work ) ;
    InitCamera( work ) ;
    InitGage( work ) ;
    InitReleasePad() ;
    InitBPPlayerPad();  //BP_INPUT - init player pad structure for extra controller input logic processing
	PL_LoadStatus() ;

    GM_PlayerMap = GM_CurrentMap ;

#ifdef DEBUG_MODE
    {
		extern void *NewPlayerDebugView( Work *work ) ;
		NewPlayerDebugView( work ) ;
    }
#ifdef PAD_MEMORY
    MemPad = ( GV_PAD * )PAD_MEMORY_ADDR ;
    if ( GV_PadData->status & PAD_L1 ) {
		MemPadMode = 1 ;
    } else if ( GV_PadData->status & PAD_AR ) {
		MemPadMode = 2 ;
    } else {
		MemPadMode = 0 ;
    }
#endif
	StartPlayerDebugMenu( work ) ;
#endif
    /* 武器用設定 */
    work->weapon_body = &( work->body ) ;
    work->weapon_unit = RIGHT_HAND_UNIT ;
    GM_PlayerPosition = work->control.mov ;
	DG_COPY_VEC( &GM_PlayerFirstPosition, &work->control.mov ) ;

    work->name = name ;

    /* ビヨンドトラップ初期化 */
    Beyond.trap_id = -1 ;
	/* ダメージコールバック初期化 */
	work->dmg_callback = NULL ;
	/* 壁床SE */
	work->seNoSeg = -1 ;
	work->seNoFlr = -1 ;

	/* イントルードエフェクト */
	GV_SetActorChild( work, NewIntrudeInDuct2D_2( 0, 0 ) ) ;
	NewIntrudeInDuct2D_2( 1, 0 ) ;

	/* 初期時に主観ならクイック */
	if ( Status( PLAYER_WATCH | PLAYER_INTRUDE ) ) {
		SetFlag2( FLAG2_SUBJECT_START ) ;
		GM_SetCameraInterpMode( work->subject_camera, GM_CAM_INTERP_QUICK,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;		  		
		GM_SetCameraInterpMode( work->weapon_camera, GM_CAM_INTERP_QUICK,
							    GM_CAM_INTERP_OUT_SUBJECT, 0, 0 ) ;
	}





//    GV_ResetPadSystem() ;	/* gamed.cに移行 */
    NewMenuPrimControl( 0 );

    if ( NewWeaponManager() == NULL ) {
		ASSERT(0) ;
		return NULL ;
	}

    /* 

       装備関係

       */
    /*ライデンのみ起動 */
    {
	void	*PL_RaidenNinjaManager( OBJECT *body, int name ) ;
	int flag = GCL_GetOptionValue( 'f', 0 ) ;
	void *NewConnectEquip( DG_OBJS *objs, int joint, int kms, int *flag, int disp_flag ) ;

	if ( !Status( PLAYER_SNAKE ) ) {
	    
	    if ( flag & START_FLAG_IS_NINJA ) {
		GV_SetActorChild( work,
				  PL_RaidenNinjaManager( &work->body, 
							 PL_EQUIPMENT_MANAGER_NAME ) ) ;
	    } else {
		GV_SetActorChild( work, PL_RaidenEquipmentManager( &work->body, 
								   PL_EQUIPMENT_MANAGER_NAME ) ) ;
		if ( GV_GetCache( GV_CacheID( GV_StrCode( "rai_hair_shadow_mt" ), 'k' ) ) != NULL ) {
			GV_SetActorChild( work, PL_RaidenShadowHair( &work->body ) ) ;
		}
	    }
	} else {
	    if ( flag & START_FLAG_IS_PLISKIN ) {
	    GV_SetActorChild( work, NewConnectEquip( work->body.objs, HUMAN21_KOSHI, GV_StrCode( "iro_mag" ), NULL, 0 ) ) ;
	    }
	}
    }

//	NewDebugTarget() ;
#ifdef DEBUG_MODE
	if ( GM_DebugModeEnable ) {
		/* パッド録画 */
		NewPadRecorder( 0 ) ;
	}
#endif


	/*主観移動用  Added by T.Morita */
	if ( PL_SubjectMove )
	  PL_IntoSubject( work ) ;


   work->inCautionMode = FALSE;
   work->knockHappened = FALSE;

    return 0 ;
}






/* ライデン起動 */
void *NewRaiden( int name, int where )
{
    Work	*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, 
									 sizeof( Work ), PLAYER_BODY_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name ) < 0 ) {

			printf( "raiden.c : Player FAILED\n" ) ;
			ASSERT(0) ; 

			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }

    return ( void * )work ;
}

//BP_INPUT - Adding MGS3 hold "X" for squat to ground functionality
void PL_UpdatePressSquat( PlayerWork *work )
{
   // Clear triggered state
   work->squat_held_triggered = 0;

   // Holding "squat" button?
   if( work->pad->status & PL_PAD_SQUAT )
   {
      // Check for holding down?
      if( work->squat_held_count >= 0 )
      {
         // Held down for set amount of time?
         if( ++work->squat_held_count > DIRECT_TICK(PL_SQUAT_TO_GROUND_FRAMES) )
         {
            // Wait for release
            work->squat_held_count = -1;

            // Flag as triggered
            work->squat_held_triggered = 1;
         }
      }
   }
   else
   {
      // Reset timer
      work->squat_held_count = 0;
   }
}

int PL_IsPressSquat( PlayerWork *work )
{
   // Enabled?
   if( gBP_Input_HoldSquatToGround_Enable )
   {
      // Return state
      return work->squat_held_triggered;
   }
   else
   {
      return 0;
   }
}
//BP_INPUT - Adding MGS3 hold "X" for squat to ground functionality

int PL_PadGetGestureRelease( PlayerWork *pl_work )
{
   // Player always uses "filtered" gestures

#if defined(BP_VITA)
   return pl_work->gesturePadFilterType;
#else
   return 0;
#endif
}
