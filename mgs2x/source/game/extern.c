//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   extern.c
   外部宣言

   1999/07/07 M.Sonoyama
   $Id: extern.c,v 1.1.1.3 2002/11/19 11:41:48 Yoshizawa1 Exp $

   *** PROJECT OF METAL GEAR SOLID2 ***
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"
#include	"libhzx.h"
#include	"game.h"
#include	"g_define.h"
#include	"g_struct.h"
#include	"g_extern.h"

#ifdef KP_XBOX
#include "libgcl.h"
#include "game.x"
#endif

/* ------------------------------------------------------------- */
//int		GM_AlertLevel ;/* linkvar.hに移動 */
//int		GM_AlertMode ; /* linkvar.hに移動 */
int		GM_CautionLevel ;		/* バー表示用。残り警戒フレーム数/警戒レベル最大値 */
int		GM_JammingLevel ;

int		GM_CautionLevelCount ;	/* 残り警戒フレーム数 */
int		GM_OldAlertMode ; 		/* 前ステージのモード */

int		GM_GameOverVox ;

int		GM_NoisePowerOn ;
int		GM_NoisePower ;
FVECTOR	GM_NoisePosition ;
int		GM_NoiseMap ;
int		GM_NoiseHzxID ;
int		GM_NoiseZoneAddr ;
int		GM_NoiseStatus ;

int		GM_PlayerDarkLevel ;
float	GM_WaterLevel ;
int		GM_WaterBulletSE;

/* サイトの表示管理( GM_SightStatus ) */
/* ビットがたっていれば表示不可 */
int		GM_SightStatus ;
int		GM_SightStatusScn ;

/* 弾痕、跳弾エフェクト管理 */
int		GM_SparkEntryFlag ;
int		GM_ScarEntryFlag ;

/* ブラーによる不具合を回避するためのブラー禁止フラグ */
int		GM_BlurDisableFlag ;
/*シゲノ追加　VRモード用 */
int		GM_VRStatus ;
int		GM_AnotherPlayer ; /*アナザーとＶＲでの使用中キャラ*/
int		GM_RouteOffset ; /*アナザーとＶＲで使用するルート番号のオフセット*/
int		GM_ClearingOffset ;

int		VR_ENEMY_NUM ;
int		VR_ENEMY_MAX ;
//VRキルカウント＆見つかったカウント
int		VR_DiscoverCount ;
int		VR_KillCount ;
void VR_SurrenderEnemy(void){
	VR_ENEMY_NUM++ ;
}
void VR_AddEnemy(void){
	VR_ENEMY_MAX++ ;
}



/* ------------------------------------------------------------- */

void GM_Scn_SetRouteOffset(void){
	int offset ;

	offset = GCL_GetNextInt() ;
	GM_RouteOffset = (u_int)HZX_CurrentHzx->def->vr_pat_offset[offset] ; 
	offset = GCL_GetOptionValue( 'c', offset ) ;
	GM_ClearingOffset = (u_int)HZX_CurrentHzx->def->vr_clr_offset[offset] ; 
}

/* ------------------------------------------------------------- */

#define GMDEF_NOISE_MAP_RENEW	(1)

/* 注意！！！！！！！！   */
/* ノイズのmapは map_id !!! hzx_idではない！！！！！！！   */
void	GM_SetNoise( int power, FVECTOR *pos, int map )
{
	if ( power > GM_NoisePowerOn ) {
#ifdef GMDEF_NOISE_MAP_RENEW
		int	addr ;

		GM_NoisePowerOn = power ;
		GM_NoisePosition = *pos ;
		GM_NoiseMap = map ;
		GM_NoiseHzxID = HZX_GetHzxIDbyZone( GM_GetHzxGroupID(map), pos, &addr ) ;
		GM_NoiseZoneAddr = addr ;
		GM_NoiseStatus = 0 ;
		//printf("set nosie: GM_NoiseHzxID[%x] map[%x] GM_GetHzxGroupID(%x)\n",GM_NoiseHzxID,map,GM_GetHzxGroupID(map));
#else
		if ( !(GM_IsOneID( map )) ) {
			int	addr ;
			printf("WARNING!!: GM_SetNoise ANY MAPS [%x]\n",map ) ;
			map = HZX_GetHzxIDbyZone( HZX_GetHzxIDbyZone( map ), pos, &addr ) ;
		}

		GM_NoisePowerOn = power ;
		GM_NoisePosition = *pos ;
		GM_NoiseMap = map ;
		GM_NoiseStatus = 0 ;
#endif
	}
}

void	GM_SetNoiseStatus( int power, FVECTOR *pos, int map, int status )
{
	if ( power > GM_NoisePowerOn ) {
#ifdef GMDEF_NOISE_MAP_RENEW
		int	addr ;

		GM_NoisePowerOn = power ;
		GM_NoisePosition = *pos ;
		GM_NoiseMap = map ;
		GM_NoiseHzxID = HZX_GetHzxIDbyZone( GM_GetHzxGroupID(map), pos, &addr ) ;
		GM_NoiseZoneAddr = addr ;
		GM_NoiseStatus = status ;
#else
		if ( !(GM_IsOneID( map )) ) {
			int	addr ;
			printf("WARNING!!: GM_SetNoise ANY MAPS [%x]\n",map ) ;
			map = HZX_GetHzxIDbyZone( HZX_GetHzxIDbyZone( map ), pos, &addr ) ;
		}

		GM_NoisePowerOn = power ;
		GM_NoisePosition = *pos ;
		GM_NoiseMap = map ;
		GM_NoiseStatus = status ;
#endif
	}
}

/* ------------------------------------------------------------- */

/* ロック付きステータスセット */
#ifdef DEBUG_MODE
int		GM_GameStatusLocked[ 32 ] ;
int		GM_MenuStatusLocked[ 32 ] ;

void	GM_LockGameStatus( int state, char *file )
{
	int		code, i, t, f ;

	f = GV_StrCode( file ) ;
	for ( i = 0; i < 32; i ++ ) {
		t = ( 0x1 << i ) ;
		if ( t & state ) {
			code = GM_GameStatusLocked[ i ] ;
			if ( code != 0 && code != f ) {
				//printf( "warning : GameStatus[%x] is already setted by %x\n", t, code ) ;
				//continue ;
				/* セットできる */
			} 
			GM_GameStatusLocked[ i ] = f ;
			GM_GameStatus |= t ;
		}
		state &= ~t ;
		if ( state == 0 ) break ;
	}
}

void	GM_UnlockGameStatus( int state, char *file )
{
	int		code, i, t, f ;

	f = GV_StrCode( file ) ;
	for ( i = 0; i < 32; i ++ ) {
		t = ( 0x1 << i ) ;
		if ( t & state ) {
			code = GM_GameStatusLocked[ i ] ;
			if ( code == 0 || code == f ) {
				GM_GameStatus &= ~t ;				
				GM_GameStatusLocked[ i ] = 0 ;
			} else {
				printf( "warning : GameStatus[%x] is locked by %x\n", t, code ) ;
				/* 解除はする */
				GM_GameStatus &= ~t ;				
				GM_GameStatusLocked[ i ] = 0 ;
			}
		}
		state &= ~t ;
		if ( state == 0 ) break ;
	}
}

void	GM_LockMenuStatus( int state, char *file )
{
	int		code, i, t, f ;

	f = GV_StrCode( file ) ;
	for ( i = 0; i < 32; i ++ ) {
		t = ( 0x1 << i ) ;
		if ( t & state ) {
			code = GM_MenuStatusLocked[ i ] ;
			if ( code != 0 && code != f ) {
				//printf( "warning : MenuStatus[%x] is already setted by %x\n", t, code ) ;
				//continue ;
				/* セットできる */
			} 
			GM_MenuStatusLocked[ i ] = f ;
			GM_MenuStatus |= t ;
		}
		state &= ~t ;
		if ( state == 0 ) break ;
	}
}

void	GM_UnlockMenuStatus( int state, char *file )
{
	int		code, i, t, f ;

	f = GV_StrCode( file ) ;
	for ( i = 0; i < 32; i ++ ) {
		t = ( 0x1 << i ) ;
		if ( t & state ) {
			code = GM_MenuStatusLocked[ i ] ;
			if ( code == 0 || code == f ) {
				GM_MenuStatus &= ~t ;				
				GM_MenuStatusLocked[ i ] = 0 ;
			} else {
				printf( "warning : MenuStatus[%x] is locked by %x\n", t, code ) ;
				/* 解除はする */
				GM_MenuStatus &= ~t ;				
				GM_MenuStatusLocked[ i ] = 0 ;
			}
		}
		state &= ~t ;
		if ( state == 0 ) break ;
	}
}
#endif

/* ------------------------------------------------------------- */

/* ゲームステータスのＰｕｓｈ＆Ｐｏｐ */
/* メニューステータスのＰｕｓｈ＆Ｐｏｐ */

static	int		GameStatusStack[ 8 ] ;
static	int		GameStatusStackScn[ 8 ] ;
static	int		GM_GameStatusStackLevel ;

static	int		MenuStatusStack[ 8 ] ;
static	int		MenuStatusStackScn[ 8 ] ;
/*static	*/ int		GM_MenuStatusStackLevel ;

void	GM_PushGameStatus( void )
{	
	int		level ;

	ASSERT( GM_GameStatusStackLevel < 8 ) ;
	level = GM_GameStatusStackLevel ;
	GameStatusStack[ level ] = GM_GameStatus ;
	GameStatusStackScn[ level ] = GM_GameStatusScn ;
	GM_GameStatusStackLevel ++ ;
}

void	GM_PopGameStatus( void )
{
	int		level ;

	ASSERT( GM_GameStatusStackLevel > 0 ) ;
	GM_GameStatusStackLevel -- ;
	level = GM_GameStatusStackLevel ;
	GM_GameStatus = GameStatusStack[ level ] ;
	GM_GameStatusScn = GameStatusStackScn[ level ] ;
}

void	GM_PushMenuStatus( void )
{	
	int		level ;

	ASSERT( GM_MenuStatusStackLevel < 8 ) ;
	level = GM_MenuStatusStackLevel ;
	MenuStatusStack[ level ] = GM_MenuStatus ;
	MenuStatusStackScn[ level ] = GM_MenuStatusScn ;
	GM_MenuStatusStackLevel ++ ;
}

void	GM_PopMenuStatus( void )
{
	int		level ;
	int		menu ;

	ASSERT( GM_MenuStatusStackLevel > 0 ) ;
	GM_MenuStatusStackLevel -- ;
	level = GM_MenuStatusStackLevel ;
	menu = MenuStatusStack[ level ] & ~( MENU_STREAM_CH_0 | MENU_STREAM_CH_1 ) ;
	menu |= GM_MenuStatus & ( MENU_STREAM_CH_0 | MENU_STREAM_CH_1 ) ;
	GM_MenuStatus = menu ;
	GM_MenuStatusScn = MenuStatusStackScn[ level ] ;
}

/* ------------------------------------------------------------- */

/* ＩＤビットが単一しか立っていないかチェック */
int		GM_IsOneID( int id )
{
	int			no ;

	no = GV_GetNo( id ) ;
	if ( id == GV_GetBit( no ) ) return 1 ;
	return 0 ;
}

/* ------------------------------------------------------------- */
	/*
		大域変数をリセット
	*/
void	GM_ResetVariable()
{
	GM_AlertLevel = 0 ;
	GM_AlertMode = 0 ;
	GM_CautionLevelCount = GM_CautionLevel ;
	GM_CautionLevel = 0 ;
	GM_JammingLevel = 0;
	GM_PlayerDarkLevel = 0 ;
	GM_WaterLevel = -1000000.0F ; /* GM_WORLD_LIMIT_BOTTOM */
	GM_WaterBulletSE = 0;
	GM_SightStatus = 0 ;
	GM_SightStatusScn = 0 ;

	GM_NoisePowerOn = 0 ;
	GM_NoisePower = 0 ;
	GM_NoisePosition = DG_ZeroVector;
	GM_NoiseMap = 0 ;
	GM_NoiseHzxID = 0 ;
	GM_NoiseZoneAddr = -1 ;

	GM_SparkEntryFlag = 0 ;
	GM_ScarEntryFlag = 0 ;

	GM_BlurDisableFlag = 0 ;

	GM_StagePlayTime = 0 ;
	GM_CurrentStage = 0 ;
	GM_GameStatus = 0 ;
	GM_GameStatusScn = 0 ;
	GM_MenuStatus = 0 ;
	GM_MenuStatusScn = 0 ;
	GM_GameOverTimer = 0 ;
	GM_GameOverFlag = 0 ;
	GM_GameOverVox = -1 ;
	GM_PauseRequest = 0 ;
	GM_DisablePauseCount = 0 ;

	GM_GameStatusStackLevel = 0 ;
	GM_MenuStatusStackLevel = 0 ;
	GM_VRStatus = 0 ;
	GM_RouteOffset = 0 ;
	GM_ClearingOffset = 0 ;
#ifdef DEBUG_MODE
	GM_Debug2PMode = 0 ;
	GV_ZeroMemory( GM_GameStatusLocked, sizeof( GM_GameStatusLocked ) ) ;
	GV_ZeroMemory( GM_MenuStatusLocked, sizeof( GM_MenuStatusLocked ) ) ;
#endif
}

