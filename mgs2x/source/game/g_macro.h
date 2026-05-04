/*
   g_macro.h

   マクロ定義ファイル

   METAL GEAR SOLID 2 project ( from 1999 )
*/
#include "g_extern.h"
#ifndef _g_macro_h_
#define _g_macro_h_

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------*/

/* PALとNTSCの相互変換 */

#ifdef PSX2
/*
	ＰＳ２の場合
*/
extern int BP_AdjustTick(int _tick);
#define DIRECT_TICK( _tick )	(BP_AdjustTick(_tick))

#ifdef NTSC
#define DIRECT_SCREEN_X(_x)		(_x)		/* ((_x) * DRAW_WIDTH / 512 ) = (_x) */
#define DIRECT_SCREEN_Y(_y)		(_y)		/* ((_y) * DRAW_HEIGHT / 448 ) = (_y) */
#endif

#ifdef PAL
#define DIRECT_SCREEN_X(_x)		(_x)		/* ((_x) * DRAW_WIDTH / 512 ) = (_x) */
#define DIRECT_SCREEN_Y(_y)		((_y) * 256 / 224)
#endif

#else

/*
	ＸＢＯＸの場合
*/
#define DIRECT_SCREEN_X(_x)		((_x) * DRAW_WIDTH / 512 )
#define DIRECT_SCREEN_Y(_y)		((_y) * DRAW_HEIGHT / 448 )

#ifndef PAL
#define DIRECT_TICK( _tick )	(_tick)
#else
#define DIRECT_TICK( _tick )	(((_tick)*5+1)/6)
#endif

#endif

/*----------------------------------------------------------------*/

	/*
		OBJECT関連定義
	*/

/* モーションの終了及び終了１フレーム前の検出 */
#define GM_GetObjectMotionEnd( _o, _n ) ( (_o)->m_ctrl->mt3_ctrl[_n].flag & ( MT3_PLAYEND | MT3_PLAYLAST1 ) )

/* モーションの終了１フレーム前の検出（ＭＧＳ１のis_endチェックに相当） */
#define GM_CheckObject_IsEnd( _o, _n ) ( ( (_o)->m_ctrl->mt3_ctrl[_n].flag & MT3_PLAYLAST1 ) != 0 )

/* モーションの終了の検出 */
#define GM_CheckObject_PlayEnd( _o, _n ) ( ( (_o)->m_ctrl->mt3_ctrl[_n].flag & MT3_PLAYEND ) != 0 )


#ifndef __GAME_H__
extern	int		GM_GameStatus ;
extern	int		GM_GameStatusScn ;
extern	int		GM_MenuStatus ;
extern	int		GM_MenuStatusScn ;
#endif

#ifndef	_game_x_
extern	int		GM_GetDGGroupID( int ) ;
#endif

/* 表示グループ設定 */
static	inline	void	GM_GroupObjs( DG_OBJS *objs, int map_id )
{
    objs->group_id = GM_GetDGGroupID( map_id ) ;
}

#ifdef PSX2
static	inline	void	GM_GroupPrim( DG_PRIM *prim, int map_id )
{
    prim->group_id = GM_GetDGGroupID( map_id ) ;
}
#endif

static	inline	void	GM_GroupPrim2( DG_PRIM2 *prim, int map_id )
{
    prim->group_id = GM_GetDGGroupID( map_id ) ;
}

#define GM_GroupObject( _obj, _map_id ) {\
										   extern int	GM_GetDGGroupID( int ) ;\
										   (_obj)->group_id = GM_GetDGGroupID( _map_id );\
									 }

#define GM_ACTOR_WAIT_MAP_CHANGE		GV_CLASS_WAIT_USER0

EXTERN_INLINE void GM_WaitMapChange( void *work )
{
	GV_SleepActor( work, GM_ACTOR_WAIT_MAP_CHANGE );
}

/* ステータス変数のセット・リセット・チェック */
#ifdef DEBUG_MODE
extern	void	GM_LockGameStatus( int, char * ) ;
extern	void	GM_UnlockGameStatus( int, char * ) ;
extern	void	GM_LockMenuStatus( int, char * ) ;
extern	void	GM_UnlockMenuStatus( int, char * ) ;
#endif

EXTERN_INLINE	int	GM_CheckGameStatus( int state )
{
	return ( ( GM_GameStatus | GM_GameStatusScn ) & state ) ;
}

#ifdef DEBUG_MODE
#define	GM_SetGameStatus( _s )		GM_LockGameStatus( _s, __FILE__ )
#define	GM_ResetGameStatus( _s )	GM_UnlockGameStatus( _s, __FILE__ )
#else
EXTERN_INLINE	void	GM_SetGameStatus( int state )
{
	GM_GameStatus |= state ;
}

EXTERN_INLINE	void	GM_ResetGameStatus( int state )
{
	GM_GameStatus &= ~state ;
}
#endif

EXTERN_INLINE	void	GM_SetGameStatusScn( int state )
{
	GM_GameStatusScn |= state ;
}

EXTERN_INLINE	void	GM_ResetGameStatusScn( int state )
{
	GM_GameStatusScn &= ~state ;
}

#ifdef DEBUG_MODE
#define	GM_SetMenuStatus( _s )		GM_LockMenuStatus( _s, __FILE__ )
#define	GM_ResetMenuStatus( _s )	GM_UnlockMenuStatus( _s, __FILE__ )
#else
EXTERN_INLINE	void	GM_SetMenuStatus( int state )
{
	GM_MenuStatus |= state ;
}

EXTERN_INLINE	void	GM_ResetMenuStatus( int state )
{
	GM_MenuStatus &= ~state ;
}
#endif

EXTERN_INLINE	void	GM_SetMenuStatusScn( int state )
{
	GM_MenuStatusScn |= state ;
}

EXTERN_INLINE	void	GM_ResetMenuStatusScn( int state )
{
	GM_MenuStatusScn &= ~state ;
}

EXTERN_INLINE	int		GM_CheckMenuStatus( int state )
{
	int		status ;

	status = GM_MenuStatus | GM_MenuStatusScn ;
	return ( status & state ) ;
}

static	inline	int		GM_CheckSightStatus( int state )
{
	return ( GM_SightStatus & state ) ;
}

static	inline	void	GM_SetSightStatus( int state )
{
	GM_SightStatus |= state ;
}

static	inline	void	GM_ResetSightStatus( int state )
{
	GM_SightStatus &= ~state ;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
	
#endif
