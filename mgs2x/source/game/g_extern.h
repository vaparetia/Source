/*
   g_extern.h
 	外部宣言

   1999/07/07 M.Sonoyama
   $Id: g_extern.h,v 1.1.1.3 2002/11/19 11:41:49 Yoshizawa1 Exp $

   *** PROJECT OF METAL GEAR SOLID2 ***
*/
#include "g_struct.h"
#ifndef _g_extern_h_
#define _g_extern_h_

#ifdef PSX2
#include <libcdvd.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


extern	int	GV_Time ; /* libgv/gvd.c */
extern	int	GM_CurrentMap ; /* １キャラ処理のマップ */
extern	int	GM_CurrentStageMap ;  /* 現在のマップ ( map.c ) */
extern	int	GM_ScriptCurrentMap ; /* charaコマンド時に渡される引数map */
extern	int	GM_CurrentChanlMap[];
extern	int	GM_StageMapAll ;	/* 現在のステージの全マップ */

extern	int	GM_PauseRequest ;
extern	int	GM_DisablePauseCount ;	/* > 0 のときはポーズ禁止 */

extern  int GM_PadResetDisable;		// パッドリセット禁止

extern	int		GM_GameOverVox ;

#ifdef KP_XBOX
extern	HZX_ZONE_ADD GM_CurrentAddr;	//カレントアドレス・ＳＥ遮蔽障害計算用
#endif

/* control.c */
extern	int		GM_N_WhereList ;
extern	CONTROL		*GM_WhereList[] ;

/* user/sonoyama/etc/gaged.c */
extern	int	GM_GageStatus ;
extern	int	GM_N_ActiveGages ;

/* 弾痕、跳弾エフェクト管理 */
extern	int		GM_SparkEntryFlag ;
extern	int		GM_ScarEntryFlag ;

/* ブラーによる不具合を回避するためのブラー禁止フラグ */
extern int		GM_BlurDisableFlag ;
/* VRおよびアナザーモード用状態管理*/
extern int		GM_VRStatus ;
/* VRおよびアナザーモード用 使用中プレイヤー*/
extern int		GM_AnotherPlayer ;
/*アナザーとＶＲで使用するルート番号のオフセット*/
extern int	GM_RouteOffset ; 
extern int	GM_ClearingOffset ;

extern int	VR_ENEMY_NUM ;	//倒すべき敵兵 残り
extern int	VR_ENEMY_MAX ;	//倒すべき敵兵 全体数
extern int	VR_DiscoverCount ;
extern int	VR_KillCount ;


/* map.c */
extern	MAP	*GM_GetMap( int map_id ) ;
extern	MAP	*GM_GetMap2( int name ) ;
extern	int	GM_GetMapID( int name ) ;
extern	int	GM_GetMapName( int map_id ) ;
extern	int	GM_GetMapIDfromHzxGroupID( HZX_GROUP_ID hzx_id ) ;
extern	int	GM_GetMapIDfromPos( int map, FVECTOR *pos ) ;
extern  int GM_ChanlTargetMap[];

EXTERN_INLINE void GM_SetChanlTargetMap( int chanl, int map )
{
	/* あるチャンネルのターゲットのマップを設定 */
	GM_ChanlTargetMap[ chanl ] = map;
}


/* gamed.c */

/* 現在時を記録 */
extern sceCdCLOCK G_rtc;


/* getitem.c */
extern	void		GM_SetCurrentItemSet( int which ) ;
extern	int			GM_IncrementWeapon( int id, int n ) ;
extern	int			GM_IncrementItem( int id, int n ) ;
extern	int			GM_DecrementWeapon( int id, int n ) ;
extern	int			GM_DecrementItem( int id, int n ) ;
extern	int			GM_WeaponNum( int id ) ;
extern	int			GM_ItemNum( int id ) ;
extern	int			GM_SetWeaponNum( int id, int n ) ;
extern	int			GM_SetItemNum( int id, int n ) ;
extern	int			GM_WeaponMaxNum( int id ) ;
extern	int			GM_ItemMaxNum( int id ) ;

extern	char	*GM_WeaponNames[] ;
extern	char	*GM_ItemNames[] ;
extern	int	GM_ItemTypes[] ;
extern	int	GM_WeaponTypes[] ;

/* no_use.c */
extern	int		*GM_NoUseWeaponsFromWeapon[] ;
extern	int		*GM_NoUseItemsFromWeapon[] ;
extern	int		*GM_NoUseWeaponsFromItem[] ;
extern	int		*GM_NoUseItemsFromItem[] ;
extern	int		GM_CheckNoUseWeaponFromWeapon( int from, int to ) ;
extern	int		GM_CheckNoUseWeaponFromItem( int from, int to ) ;
extern	int		GM_CheckNoUseItemFromWeapon( int from, int to ) ;
extern	int		GM_CheckNoUseItemFromItem( int from, int to ) ;

/* ------------------------------------------------------------- */
extern	int		GM_CautionLevel ;
extern	int		GM_JammingLevel ;
extern	int		GM_CautionLevelCount ;	/* 残り警戒フレーム数 */
extern	int		GM_OldAlertMode ; 		/* 前ステージのモード */
extern	int		GM_NoisePowerOn ;
extern	int		GM_NoisePower ;
extern	FVECTOR		GM_NoisePosition ;
extern	int		GM_NoiseMap ;
extern	int		GM_NoiseHzxID ;
extern	int		GM_NoiseZoneAddr ;
extern	int		GM_GameOverTimer ;
extern	int		GM_GameOverFlag ;
extern	int		GM_NoiseStatus ;
extern	int		GM_PlayerDarkLevel ;
extern	float	GM_WaterLevel ;
extern	int		GM_WaterBulletSE;
extern	int		GM_SightStatus ;
extern	int		GM_SightStatusScn ;

/* ------------------------------------------------------------- */
extern	void	GM_SetNoise( int, FVECTOR *, int ) ;
extern	void	GM_SetNoiseStatus( int, FVECTOR *, int, int ) ;
extern	int		GM_IsOneID( int id ) ;

extern	void	GM_PushGameStatus( void ) ;
extern	void	GM_PopGameStatus( void ) ;
extern	void	GM_PushMenuStatus( void ) ;
extern	void	GM_PopMenuStatus( void ) ;

extern	void	GM_ResetVariable() ;

extern	CONTROL	*GM_SearchWhere( u_int );

extern	ENEFINDLISTLIST	GM_EneFindListList ;
extern	ENEFINDLIST	GM_EneFindList ;

extern void GM_ReadError( int flag );

#ifdef __cplusplus
}
#endif
	
#endif
