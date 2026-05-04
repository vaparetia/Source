//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   wp_change2.c
   武器チェンジテスト（その２）

   2000/08/28 M.Sonoyama
   $Id: wp_chng2.c,v 1.1.1.3 2002/11/19 11:45:14 Yoshizawa1 Exp $
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
#include	"menu.h"
#undef OPEN_TIME_G
#undef OPEN_TIME_L
#define	OPEN_TIME_G		(6)		/* グループオープン時間 */
#define	OPEN_TIME_L		(6)		/* リストオーブン時間 */

/*------------------------------------------------------------------*/

extern	int	GM_WeaponChanged ;

#define	Weapons		GM_WeaponNames

enum {
	WG_None = 0,		/* 素手 */
	WG_HandGun,			/* 銃 */
	WG_Rifle,			/* ライフル */
	WG_Throw,			/* 投擲 */
	WG_Heavy,			/* 重火器 */
	WG_Blade,			/* ブレード */
	WG_Mic,				/* マイク */
	WG_Book,			/* 雑誌 */
	WG_Set,				/* 設置 */
	WG_Spray,			/* スプレー */
	MAX_GROUPS
} ;

#if 0
static	char	*Groups[] = {
	"None", "Guns", "Rifle", "Grenades", "Heavy", 
	"Blade", "Mic", "Magazine", "Setting", "Spray"
} ;
#endif
static	int		GrpNone[] = 	{ WP_None } ;
static	int		GrpGun[] = 		{ WP_m92, WP_Usp, WP_Socom } ;
static	int		GrpRifle[] = 	{ WP_m4, WP_Aks, WP_Psg1, WP_Psg1T } ;
static	int		GrpThrow[] = 	{ WP_Grenade, WP_ChaffGrenade, WP_StunGrenade, WP_Magazine } ;
static	int		GrpHeavy[] =    { WP_Rgb6, WP_Nikita, WP_Stinger } ;
static	int		GrpBlade[] =	{ WP_Blade } ;
static	int		GrpMic[] =		{ WP_Mic, WP_DemoMic } ;
static	int		GrpBook[] =		{ WP_Book } ;
static	int		GrpSet[] = 		{ WP_Claymore, WP_C4Bomb } ;
static	int		GrpSpray[] = 	{ WP_ColdSpray } ;

#define	MAX_GROUP_LISTS		(6)

typedef struct _group_info{
	int			n_lists ;		/* リスト内最大要素数 */
	int			*lists ;		/* リスト内要素データ配列へのポインタ */
} GroupInfo;

static GroupInfo	GroupInfoLists[] = {
	{ 1, GrpNone },
	{ 3, GrpGun },
	{ 4, GrpRifle},
	{ 4, GrpThrow },
	{ 3, GrpHeavy},
	{ 1, GrpBlade},
	{ 2, GrpMic},
	{ 1, GrpBook },
	{ 2, GrpSet},
	{ 1, GrpSpray},
};
static int	GroupOrder[] = {
	WG_Heavy,			/* 重火器 */
	WG_Throw,			/* 投擲 */
	WG_Rifle,			/* ライフル */
	WG_HandGun,			/* 銃 */
	WG_None,			/* 素手 */
	WG_Spray,			/* スプレー */
	WG_Set,				/* 設置 */
	WG_Book,			/* 雑誌 */
	WG_Mic,				/* マイク */
	WG_Blade,			/* ブレード */
};

#define GroupLists( _l, _n )	GroupInfoLists[ GroupOrder[ _l ] ].lists[ _n ]
#define ListNum( _l )			GroupInfoLists[ GroupOrder[ _l ] ].n_lists


/*------------------------------------------------------------------*/

#if 0
#define	EXP_WIN_X	(-120)
#define	EXP_WIN_Y	(-48*Y_ADJ)
#define	EXP_WIN_W	(240)
#define	EXP_WIN_H	(96*Y_ADJ)
#endif

//#define	NO_PAUSE

/*------------------------------------------------------------------*/
	/*
		２Ｄ表示関連設定など
	*/

#define MAX_PANEL_PRIM		((DISP_GROUPS+1)*(DISP_LISTS+1))/* 確保するパネルプリミティブ数 */
#define MAX_GROUP_PRIM		((DISP_GROUPS+1))/* 確保するグループ表示プリミティブ数 */

/*------------------------------------------------------------------*/

typedef	struct	{
	GV_ACT				actor ;

	GV_PAD				*pad ;
	
	int					quick ;
	int					flag ;

	int					current_g ;
	int					current_l[ MAX_GROUPS ] ;
	int					upper, lower ;
	int					start ;
	int					current ;

	int					open_time_g ;
	int					open_time_l ;
	int					open_time_e ;			/* 説明文表示時間（OPEN_TIME_Eで完全に開く） */
	
	int					move_time ;
	int					loop_group ;

	int					g_flag[ MAX_GROUPS ] ;
	int					l_flag[ MAX_GROUPS ][ MAX_GROUP_LISTS ] ;
	int					available_groups ;

	u_char				rtime, ltime, utime, dtime ;
	u_char				rtime2, ltime2, utime2, dtime2 ;

	int					rpress ;
	int					rstat ;

	int					selected_time ;			/* 選択時点滅用カウンタ */
	int					current_equip_group ;	/* 現在装備中の装備の含まれるグループ番号 */
	int					prev_equip_group ;	/* 以前装備していた装備の含まれるグループ番号 */

	int					padflag ;
} Work ;

enum {
	MENU_WIN_CLOSE = 0,
	MENU_WIN_CLOSING,
	MENU_WIN_OPEN,
	MENU_WIN_MOVE_GROUP_U,
	MENU_WIN_MOVE_GROUP_D,
	MENU_WIN_MOVE_LIST_L,
	MENU_WIN_MOVE_LIST_R,
} ;

static	void	DisplayList( Work *work, int g, int sx, int sy, int mode );

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/* 説明文テクスチャを更新する（必ず説明文テクスチャを描画していないときに行うこと） */
static void UpdateMessageTexture( Work *work )
{
#if 0
	/* 説明文テクスチャを更新 */
	void	*mes ;
	int		num ;
	num = work->current ;
	if ( num == WP_Socom && GM_PlayerStateFlag & PL_SOCOM_SPPRSR_ATTACHED ){
		num = MAX_WEAPONS + 2 ;
	}
	if ( num == WP_Aks && GM_PlayerStateFlag & PL_AK_SPPRSR_ATTACHED ){
		num = MAX_WEAPONS + 1 ;
	}
	if ( num == WP_Usp && GM_PlayerStateFlag & PL_USP_SPPRSR_ATTACHED ){
		num = MAX_WEAPONS + 0 ;
	}
	if ( num == WP_Mic && MENU_StatusFlag & MENU_STATUS_MIC_EXPLAIN2 ){
		num = MAX_WEAPONS + 3 ;
	}
	mes = GM_GetResource( RESOURCE_WEAPON_MENU_EXPLAIN, num );
	if ( mes == NULL ) return ;
	MENU_CreateExplainText( mes );
	/* テクスチャ展開時に多重変更をチェックしているので毎フレーム
	   説明文テクスチャ生成関数を呼び出しても問題ない */
#else
	MENU_UpdateWeaponExplainMessage( work->current );
#endif
}
/*------------------------------------------------------------------*/

static	inline	int	CountUpper( Work *work )
{
	int	 	upper ;

//	upper = ( work->available_groups < 3 ) ? 
//		work->available_groups : work->available_groups - 1 ;
	upper = work->available_groups ;
	if ( upper > UPPER_MAX ) upper = UPPER_MAX ;
	return upper ;
}
 
/*------------------------------------------------------------------*/

/* 変更不可武器のチェック */
static	int		_CheckDisableChanging( int weapon )
{
	return PL_CheckNoUseWeapon( weapon ) ;
}
static	int		CheckDisableChanging( void )
{
    if ( _CheckDisableChanging( GM_Weapon ) ) {
		GM_Weapon = WP_None ;
		return 1 ;
    }
	return 0 ;
}

static	inline	int	GetPadFlag( Work *work )
{
	return ( GM_CheckGameStatus( STATE_PAD_DEMO ) |
			( work->pad->flag & GV_PAD_RELEASE ) ) ;
}

static	inline	int	OpenEnable( work )
Work			*work ;
{
	if ( ( !GM_CheckPlayerStatus( PLAYER_MENU_OPEN ) || 
		  work->flag == MENU_WIN_CLOSING ) &&
		( work->pad->status & PAD_WP_CHANGE ) ) return 1 ;
	return 0 ;
}

/* 現在所持中のアイテムから表示に使用するリストを作成 */
static	void	CheckAvailable( Work *work )
{
	int			i, j, n, c ;

	work->current_equip_group = -1 ;
	work->prev_equip_group = -1 ;
	work->available_groups = 0 ;
	for ( i = 0; i < MAX_GROUPS; i ++ ) {
		c = 0 ;
		for ( j = 0; j < ListNum( i ); j ++ ) {
			n = GroupLists( i, j ) ;
			if ( GM_WeaponNum( n ) > 0 || 
				 ( GM_WeaponNum( n ) == 0 && ( GM_WeaponTypes[ n ] & WP_TYPE_BODY ) ) ) {
				work->l_flag[ i ][ j ] = 1 ;
				c ++ ;
				/* 切り替え対象装備の所属するグループを記憶 */
				if ( n == GM_Weapon ) work->current_equip_group = i ;
				if ( !( GM_Configuration & GM_CONFIG_MENU_QCHANGE_EX ) ){
					/* 旧形式クイックチェンジ時 */
					if ( GM_Weapon == WP_None ){
						if ( n == GM_WeaponPrev ) work->prev_equip_group = i ;
					} else {
						if ( n == WP_None ) work->prev_equip_group = i ;
					}
				} else {
					/* 新方式クイックチェンジ時 */
					if ( n == GM_WeaponPrev ) work->prev_equip_group = i ;
				}
			} else {
				work->l_flag[ i ][ j ] = 0 ;
				if ( work->current_l[ i ] == j ) {
					work->current_l[ i ] = -1 ;
				}
			}
		}
		if ( work->current_l[ i ] == -1 ) {
			work->current_l[ i ] = 0 ;
			for ( j = 0; j < ListNum( i ); j ++ ) {
				if ( work->l_flag[ i ][ j ] == 1 ) {
					work->current_l[ i ] = j ;
					break ;
				}
			}
		}
		work->g_flag[ i ] = c ;
		if ( c ) {
//printf( "available %s\n", Groups[ i ] ) ;
			work->available_groups ++ ;
		}
	}

}

/* 装備品番号から現在のグループ番号とリスト番号を取得する */
static	void	GetCurrentGroup( int wp, int *g, int *l )
{
	int			i, j ;

	for ( i = 0; i < MAX_GROUPS; i ++ ) {
		for ( j = 0; j < ListNum( i ); j ++ ) {
			if ( wp == GroupLists( i, j ) ) {
				*g = i ; *l = j ;
				return ;
			}
		}
	}
}

static	int		GetNextGroup( Work *work, int g, int dir )
{
	int			i, n ;

	i = 0 ; 
	while( ++ i < MAX_GROUPS ) {
		n = g + ( i * dir ) ;
		if ( n < 0 ) n += MAX_GROUPS ;
		else if ( n >= MAX_GROUPS ) n -= MAX_GROUPS ;
		if ( work->g_flag[ n ] > 0 ) return n ;
	}
	return g ;
}

static	int		GetNextList( Work *work, int g, int l, int dir )
{
	int			i, n ;

	i = 0 ; 
	while( ++ i < ListNum( g ) ) {
		n = l + ( i * dir ) ;
		if ( n < 0 ) n += ListNum( g ) ;
		else if ( n >= ListNum( g ) ) n -= ListNum( g ) ;
		if ( work->l_flag[ g ][ n ] > 0 ) return n ;
	}
	return l ;
}

/* 近い方の検索 */
static int GetNearGroupDir( Work *work, int from_g, int to_g )
{
	int		i, n, count, p_count = 0, m_count = 0, dir ;
	/* 正方向でのチェック */
	n = from_g ;
	for ( i = 0, count = 0 ; i < MAX_GROUPS ; i++, n++ ){
		if ( n >= MAX_GROUPS ) n -= MAX_GROUPS ;
		if ( n == to_g ){
			p_count = count ;
			break ;
		}
		if ( work->g_flag[ n ] > 0 ) count++ ;
	}
	/* 負方向でのチェック */
	n = from_g ;
	for ( i = 0, count = 0 ; i < MAX_GROUPS ; i++, n-- ){
		if ( n < 0 ) n += MAX_GROUPS ;
		if ( n == to_g ){
			m_count = count ;
			break ;
		}
		if ( work->g_flag[ n ] > 0 ) count++ ;
	}
	/* 近いほうを選択 */
	if ( m_count <= p_count ){
		dir = ( m_count != 0 ) ? -1 : 0 ;
	} else {
		dir = ( p_count != 0 ) ? 1 : 0 ;
	}
	return ( dir );
}
static int GetNearListDir( Work *work, int gp, int from_l, int to_l )
{
	int		i, n, count, p_count = 0, m_count = 0, dir, max ;
	max = ListNum( gp );
	/* 正方向でのチェック */
	n = from_l ;
	for ( i = 0, count = 0 ; i < max ; i++, n++ ){
		if ( n >= max ) n -= max ;
		if ( n == to_l ){
			p_count = count ;
			break ;
		}
		if ( work->l_flag[ gp ][ n ] > 0 ) count++ ;
	}
	/* 負方向でのチェック */
	n = from_l ;
	for ( i = 0, count = 0 ; i < max ; i++, n-- ){
		if ( n < 0 ) n += max ;
		if ( n == to_l ){
			m_count = count ;
			break ;
		}
		if ( work->l_flag[ gp ][ n ] > 0 ) count++ ;
	}
	/* 近いほうを選択 */
	if ( m_count <= p_count ){
		dir = ( m_count != 0 ) ? -1 : 0 ;
	} else {
		dir = ( p_count != 0 ) ? 1 : 0 ;
	}
	return ( dir );
}

/*------------------------------------------------------------------*/

static	void	DisplayGroup( Work *work, int sx, int sy, int mode )
{
	int		time, used, upper, lower ;
	int		i, n, y, a ; 
	int		list[MAX_GROUPS+1] ;

	time = work->open_time_g ;
	used = 0 ;
	upper = work->upper ;
	lower = -work->lower ;

	/* 表示用リストの作成 */
	for ( i = work->current_g; i > work->current_g - MAX_GROUPS; i -- ) {
		n = i ;
		if ( n < 0 ) n += MAX_GROUPS ;
		if ( work->g_flag[ n ] > 0 ) {
			list[ used ] = n ;
			used++ ;
		}
	}
	/* ループ対処 */
	list[ used ] = list[ 0 ] ;

	/* スクロール時の追加表示処理 */
	if ( work->flag == MENU_WIN_MOVE_GROUP_U ){
		lower-- ;
	} else if ( work->flag == MENU_WIN_MOVE_GROUP_D ){
		upper++ ;
	}

	/* アルファの設定 */
	a = 128 * time / OPEN_TIME_G ;

	/* 表示処理 */
	for ( i = lower ; i < upper ; i++ ) {
		int		g_col ;		/* グループパネル表示カラー */
		n = i ;
		if ( n >= used ) n -= used ;
		if ( n < 0 ) n += used ;
		y = PY - ( ( PY - GY ) + ( PH + STEP_Y ) * i ) * time / OPEN_TIME_G ;
		/* グループ名の表示 */
		g_col = 0x000000 ;
		if ( list[n] == work->prev_equip_group ) g_col = MENU_GRP_PREV_COL ;
		if ( list[n] == work->current_equip_group ) g_col = MENU_GRP_CURRENT_COL ;
		MENU_PutWeaponGroup( WP_PX + sx + PW - GW, y + sy, GroupOrder[ list[n] ], a, g_col );
		if ( mode == 0 || list[n] != work->current_g ) {
			/* グループ直下のリストの表示 */
			//DisplayList( work, list[n], 0, y + sy - PY, 0 ) ;
			DisplayList( work, list[n], 0, y + sy - GY, 0 ) ;
		} else {
			if ( mode == 2 ) {
				/* リスト選択可能状態の時には非表示 */
			} else {
				/* グループ直下のリストの表示 */
				//DisplayList( work, list[n], 0, y + sy - PY, 0 ) ;
				DisplayList( work, list[n], 0, y + sy - GY, 0 ) ;
			}
		}
		/* 選択中のものは常に表示 */
		if ( ( list[n] == work->current_g ) && ( work->open_time_l <= 0 ) ){
			int		x ;
			x = WP_PX - ( ( PW + STEP_X ) * 0 ) - ( GW + STEP_X ) ;
			MENU_PutWeaponPanel( x, GY + y + sy - GY, GM_Weapon, a, a, 1/* 選択中 */ );
		}
	}
}

static	void	DisplayList( Work *work, int g, int sx, int sy, int mode )
{
	int			time, max, used, x, y, n ;
	int			i, a1, a2, start, end ;
	int			list[ MAX_GROUP_LISTS + 1 ] ;
	int			flag ;

	time = work->open_time_l ;
	max = ListNum( g ) ;
	used = 0 ;
	if ( time > 0 ) {
        y = GY ;
		/* 表示データリストの作成 */
		for ( i = work->current_l[ g ] ; i < ( work->current_l[ g ] + max ) ; i++ ){
			n = i ;
			if ( n >= max ) n -= max ;
			if ( work->l_flag[ g ][ n ] > 0 ) {
				list[ used ] = GroupLists( g, n ) ;
				used++ ;
			}
		}
		list[ used ] = list[ 0 ] ;/* ループ用 */

		{/* アルファの調整 */
			/* タイトル部のアルファ計算 */
			a2 = 128 * time / OPEN_TIME_L ;
			if ( a2 < 0 ) a2 = 0 ;
			/* アイコンウィンドウのアルファ計算（タイトル部の影響を受ける） */
			//a1 = ( PY - ( y + sy ) ) ;
			a1 = ( GY - ( y + sy ) ) ;
			if ( a1 < 0 ) a1 = -a1 ;
			if ( a1 > PH ) a1 = PH ;
			a1 = a2 - a2 * a1 / PH ;
			/* やっぱりタイトル部はアイコン部との平均を取ってみる */
			a2 = ( a1 + a2 ) / 2 ;
			//a1 = a2 - ( a2 - a1 ) * 6 / 8 ;
		}

		/* 表示範囲の決定 */
		start = 0 ;
		end = work->g_flag[ g ] ;

		/* スクロール端パネルの表示（選択行の場合のみ） */
		if ( g == work->current_g && sx != 0 ){
			if ( work->flag == MENU_WIN_MOVE_LIST_L ){
				int	blend ;

				blend = ( sx < 0 ) ? -sx : sx ;
				blend = 128 * blend / ( PW + STEP_X ) ;
				if ( blend > 128 ) blend = 128 ;

				/* 先頭パネルの表示 */
				flag = 2 ;	/* 非選択中フラグ */
				if ( _CheckDisableChanging( list[used-1] ) ) flag |= PANEL_FLAG_NO_USE ;
				x = WP_PX - ( ( PW + STEP_X ) * (-1) ) - ( GW + STEP_X ) ;
				MENU_PutWeaponPanel( x + sx, y + sy, list[used-1], a1 * blend / 128, a2 * blend / 128, flag );

				blend = 128 - blend ;
				/* 最終パネルの表示 */
				flag = 2 ;	/* 非選択中フラグ */
				if ( _CheckDisableChanging( list[end-1] ) ) flag |= PANEL_FLAG_NO_USE ;
				x = WP_PX - ( ( PW + STEP_X ) * (end-1) ) - ( GW + STEP_X ) ;
				MENU_PutWeaponPanel( x + sx, y + sy, list[end-1], a1 * blend / 128, a2 * blend / 128, flag );
				end-- ;
			} else if ( work->flag == MENU_WIN_MOVE_LIST_R ){
				int	blend ;

				blend = ( sx < 0 ) ? -sx : sx ;
				blend = 128 * blend / ( PW + STEP_X ) ;
				if ( blend > 128 ) blend = 128 ;

				/* 最終パネルの表示 */
				flag = 2 ;	/* 非選択中フラグ */
				if ( _CheckDisableChanging( list[end] ) ) flag |= PANEL_FLAG_NO_USE ;
				x = WP_PX - ( ( PW + STEP_X ) * (end) ) - ( GW + STEP_X ) ;
				MENU_PutWeaponPanel( x + sx, y + sy, list[end], a1 * blend / 128, a2 * blend / 128, flag );

				blend = 128 - blend ;
				/* 先頭パネルの表示 */
				flag = 2 ;	/* 非選択中フラグ */
				if ( _CheckDisableChanging( list[0] ) ) flag |= PANEL_FLAG_NO_USE ;
				x = WP_PX - ( ( PW + STEP_X ) * (0) ) - ( GW + STEP_X ) ;
				MENU_PutWeaponPanel( x + sx, y + sy, list[0], a1 * blend / 128, a2 * blend / 128, flag );

				start++ ;
			}
		}

		/* 通常表示 */
		for ( i = start ; i < end ; i ++ ) {
			int	flag ;
			/* フラグの設定（選択中のアイテムにフラグを立てる） */
			flag = ( ( g == work->current_g ) && ( sx == 0 ) &&  ( i == 0 ) ) ? 1/* 選択中 */ : 2/* 非選択中 */ ;
			if ( _CheckDisableChanging( list[i] ) ) flag |= PANEL_FLAG_NO_USE ;
			/* パネルの表示 */
			x = WP_PX - ( ( PW + STEP_X ) * i ) * time / OPEN_TIME_L - ( GW + STEP_X ) ;
			if ( g == work->current_g && i == 0 && work->flag == MENU_WIN_OPEN ){
				/* 選択中のものは常に表示 */
				MENU_PutWeaponPanel( x + sx, y + sy, list[i], 128, 128, flag );
			} else {
				MENU_PutWeaponPanel( x + sx, y + sy, list[i], a1, a2, flag );
			}
		}
	}
}

static	int		AnalogRStat( pad )
GV_PAD			*pad ;
{
	int			status ;
	u_char		dx, dy ;

    status = 0 ;
    dx = pad->right_dx ;
    dy = pad->right_dy ;
    if ( dx < 128 - ANALOG_MARGIN ) status |= PAD_L ;
    else if ( dx > 128 + ANALOG_MARGIN ) status |= PAD_R ;
    if ( dy < 128 - ANALOG_MARGIN ) status |= PAD_U ;
    else if ( dy > 128 + ANALOG_MARGIN ) status |= PAD_D ;
    return status ;	
}

/*------------------------------------------------------------------*/

static	void	OpenMenu( Work *work )
{
	int			upper, lower ;
	int			n, g, rstat ;

	if ( work->quick == 0 && OpenEnable( work ) ) {
		/* 初期化 */
		work->start = work->current = GM_Weapon ;
#if 0
		GetCurrentGroup( GM_Weapon, &work->current_g, &n ) ;
		work->current_l[ work->current_g ] = n ;
		/* 現在の武器をチェック */
		CheckAvailable( work ) ;
		upper = CountUpper( work ) ;
		lower = work->available_groups - upper ;
		if ( lower > LOWER_MAX ) lower = LOWER_MAX ;
		work->upper = upper ;
		work->lower = lower ;
#endif
		/* カウンタをクリア */
		work->rtime = work->ltime = work->utime = work->dtime = 0 ;
		work->rtime2 = work->ltime2 = work->utime2 = work->dtime2 = 0 ;

		work->quick = 1 ;
		work->open_time_e = 0 ;
		work->padflag = GetPadFlag( work ) ;
	}
    if ( 0 < work->quick && work->quick < QUICK_TIME && 
		( !( GM_CheckPlayerStatus( PLAYER_MENU_OPEN ) ) || 
		 work->flag == MENU_WIN_CLOSING ) ) ++ work->quick ;

    /* クイックチェンジのみ可の場合リターン */
    if ( GM_CheckPlayerStatus( PLAYER_WEAPON_QUICK_ONLY ) ) {
		if ( GM_Weapon != WP_None ) {
			MENU_PutWeaponPanel( WP_PX, PY, GM_Weapon, 128, 128, 0 );
		}
		return ;
    }

	if ( work->quick == QUICK_TIME ) {
		int		press ;
		if ( work->flag != MENU_WIN_OPEN ) {
			/* 開きはじめ */
			GM_SetPlayerStatus( PLAYER_PAD_OFF | PLAYER_MENU_OPEN ) ;
#ifndef NO_PAUSE
			GV_PauseOnActorSystem( GV_PAUSE_MENU ) ;
#endif		
			work->flag = MENU_WIN_OPEN ;
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDISP02 ) ;

			/* 初期化 */
			work->start = work->current = GM_Weapon ;
			GetCurrentGroup( GM_Weapon, &work->current_g, &n ) ;
			work->current_l[ work->current_g ] = n ;
			/* 現在の武器をチェック */
			CheckAvailable( work ) ;
			upper = CountUpper( work ) ;
			lower = work->available_groups - upper ;
			if ( lower > LOWER_MAX ) lower = LOWER_MAX ;
			work->upper = upper ;
			work->lower = lower ;
		}
		if ( work->open_time_g < OPEN_TIME_G ) work->open_time_g ++ ;
		else if ( work->open_time_l < OPEN_TIME_L ) work->open_time_l ++ ;

		g = work->current_g ;

		DisplayGroup( work, 0, 0, 1 ) ;
		//DisplayList( work, g, 0, 0, 0 ) ;
		/* 説明分表示 */
		if ( work->flag == MENU_WIN_OPEN && work->open_time_l == OPEN_TIME_L ){
			if ( work->open_time_e < OPEN_TIME_E ) work->open_time_e ++ ;
			if ( work->open_time_e == OPEN_TIME_E && work->current != WP_None ){
				UpdateMessageTexture( work );
				MENU_PutExplainText( EXP_WIN2_WX, EXP_WIN2_Y, 1 );
			}
		}

		if ( work->pad->status & PAD_U ) work->utime ++ ;
		else 							 work->utime = 0 ;
		if ( work->pad->status & PAD_D ) work->dtime ++ ;
		else 							 work->dtime = 0 ;
		if ( work->pad->status & PAD_L ) work->ltime ++ ;
		else 							 work->ltime = 0 ;
		if ( work->pad->status & PAD_R ) work->rtime ++ ;
		else 							 work->rtime = 0 ;

		rstat = AnalogRStat( work->pad ) ;
		if ( rstat & PAD_U ) work->utime2 ++ ;
		else 				 work->utime2 = 0 ;
		if ( rstat & PAD_D ) work->dtime2 ++ ;
		else 				 work->dtime2 = 0 ;
		if ( rstat & PAD_L ) work->ltime2 ++ ;
		else 				 work->ltime2 = 0 ;
		if ( rstat & PAD_R ) work->rtime2 ++ ;
		else 				 work->rtime2 = 0 ;

		work->rpress = ( rstat & ~work->rstat ) ;
		work->rstat = rstat ;

		/* 自動選択処理 */
		if ( work->flag == MENU_WIN_OPEN && work->open_time_l == OPEN_TIME_L && MENU_AutoSelectWeapon != -1 ){
			int		cg, cl, tg, tl ;
			GetCurrentGroup( MENU_AutoSelectWeapon, &tg, &tl );
			GetCurrentGroup( work->current, &cg, &cl );
			press = 0 ;
			if ( tg != cg ){
				/* グループを合わせる */
				if ( GetNearGroupDir( work, cg, tg ) < 0 ){
					press = PAD_D ;
				} else {
					press = PAD_U ;
				}
			} else {
				if ( cl != tl ){
					/* リストを合わせる */
					if ( GetNearListDir( work, cg, cl, tl ) < 0 ){
						press = PAD_L ;
					} else {
						press = PAD_R ;
					}
				} else {
					MENU_AutoSelectWeapon = -1 ;
				}
			}
		} else {
			press = work->pad->press ;
		}

		if ( ( press & PAD_U ) || work->utime > 30 ) {
			work->flag = MENU_WIN_MOVE_GROUP_U ;
			work->move_time = 0 ;
			work->open_time_e = 0 ;
		} else if ( ( press & PAD_R ) || work->rtime > 30 ) {
			if ( work->g_flag[ g ] > 1 ) {
				work->flag = MENU_WIN_MOVE_LIST_R ;
				work->move_time = 0 ;
				work->open_time_e = 0 ;
			}
		} else if ( ( press & PAD_D ) || work->dtime > 30 ) {
			work->flag = MENU_WIN_MOVE_GROUP_D ;
			work->move_time = 0 ;
			work->open_time_e = 0 ;
		} else if ( ( press & PAD_L ) || work->ltime > 30 ) {
			if ( work->g_flag[ g ] > 1 ) {
				work->flag = MENU_WIN_MOVE_LIST_L ;
				work->move_time = 0 ;
				work->open_time_e = 0 ;
			}
		}
	}
}

/* グループの回転 */
static	void	MoveGroup( Work *work )
{
	int			time ;
	int			flag, n ;
	int			used, upper, lower, shift ;

	time = work->move_time ;
	work->move_time ++ ;
	
	flag = work->flag ;
	if ( time == 0 ) {
		if ( flag == MENU_WIN_MOVE_GROUP_U ) {		
			n = work->lower + 1 ;
			work->loop_group = work->current_g ;
			while( -- n >= 0 ) {
				work->loop_group = GetNextGroup( work, work->loop_group, 1 ) ;
			}
		} else {
			n = work->upper ;
			work->loop_group = work->current_g ;
			while( -- n >= 0 ) {
				work->loop_group = GetNextGroup( work, work->loop_group, -1 ) ;
			}
		}
	}
	used = 0 ;
	upper = work->upper ;
	lower = work->lower ;
	shift = ( PH + STEP_Y ) * time / SHIFT_TIME ;
	if ( flag == MENU_WIN_MOVE_GROUP_U ) shift = 0 - shift ;

	DisplayGroup( work, 0, shift, 1 ) ;

	if ( time == SHIFT_TIME ) {
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_ISEL02 ) ;
		if ( flag == MENU_WIN_MOVE_GROUP_U ) {
			work->current_g = GetNextGroup( work, work->current_g, 1 ) ;
		} else {
			work->current_g = GetNextGroup( work, work->current_g, -1 ) ;
		}
		work->current = GroupLists( work->current_g, work->current_l[ work->current_g ] ) ;
		//work->open_time_l = -4 ;	/* リスト表示のリセットは行わない */
		work->flag = MENU_WIN_OPEN ;
	}
}

/* リストの回転 */
static	void	MoveList( Work *work )
{
	int			time, g, i ;
	int			flag ;
	int			shift ;

	g = work->current_g ;
	time = work->move_time ;
	work->move_time ++ ;

	flag = work->flag ;
	if ( time == 0 ) {
		if ( flag == MENU_WIN_MOVE_LIST_L ) {
			work->loop_group = GetNextList( work, g, work->current_l[ g ], -1 ) ;
		} else {
			work->loop_group = work->current_l[ g ] ;
			for ( i = 0; i < work->g_flag[ g ]; i ++ ) {
				work->loop_group = GetNextList( work, g, work->loop_group, 1 ) ;
			}
		}
	}

	shift = ( PW + STEP_X ) * time / SHIFT_TIME ;
	if ( flag == MENU_WIN_MOVE_LIST_L ) shift = 0 - shift ;

	DisplayGroup( work, 0, 0, 2 ) ;
	DisplayList( work, g, shift, 0, 0 ) ;

	if ( time == SHIFT_TIME ) {
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_ISEL02 ) ;
		if ( flag == MENU_WIN_MOVE_LIST_L ) {
			work->current_l[ g ] = GetNextList( work, g, work->current_l[ g ], -1 ) ;
		} else {
			work->current_l[ g ] = GetNextList( work, g, work->current_l[ g ], 1 ) ;
		}
		work->current = GroupLists( g, work->current_l[ g ] ) ;
		work->flag = MENU_WIN_OPEN ;
	}
}

/*------------------------------------------------------------------*/

static	void	Act( Work *work )
{
	int		status ;
	int		change = 0 ;

	GM_ResetMenuStatus( MENU_WEAPON_OPEN ) ;

	change = MENU_DefaultCheckWeapon();

	/* ポーズ中 */
    if ( ( GV_PauseLevel & GV_PAUSE_PAUSE ) ) {
		if ( work->quick < QUICK_TIME ) work->quick = 0 ;
		return ;
	}
	/* 非表示設定 */
	if ( GM_CheckMenuStatus( MENU_WEAPON_OFF | MENU_ITEM_OPEN ) ||
		( GM_CheckPlayerStatus( PLAYER_FORCE | PLAYER_LOCKER ) ) ) {
		if ( work->quick < QUICK_TIME ) work->quick = 0 ;
		return ;
    }	
	/* 押し直し必要 */
	if ( GM_CheckMenuStatus( MENU_MENU_NEWPRESS ) &&
		!( work->pad->press & PAD_WP_CHANGE ) ) {
		if ( GM_Weapon != WP_None ) {
			MENU_PutWeaponPanel( WP_PX, PY, GM_Weapon, 128, 128, 0 );
		}
		return ;
	}

	GM_ResetMenuStatus( MENU_MENU_NEWPRESS ) ;

	status = work->pad->status ;

	/* メニューが閉じている状態 */
    if ( ( !( status & PAD_WP_CHANGE ) && work->flag <= MENU_WIN_OPEN ) ||
		( GM_CheckPlayerStatus( PLAYER_WEAPON_DISABLE ) ) ||
		GM_CheckMenuStatus( MENU_WEAPON_DISABLE ) ) {
		if ( ( GM_CheckPlayerStatus( PLAYER_WEAPON_DISABLE ) ) ||
			GM_CheckMenuStatus( MENU_WEAPON_DISABLE ) ) {
			work->quick = 0 ;
		}
		if ( work->flag == MENU_WIN_CLOSE && 
			work->quick > 0 && 
			work->quick < QUICK_TIME ) {
			if ( work->padflag != GetPadFlag( work ) ) {
printf( "quick change cancel!!\n" ) ;
				goto quick_change_cancel ;
			}
			/* クイックチェンジ */
			change = MENU_QuickChangeWeapon();
		}
		quick_change_cancel :
		/* ウインドウ閉じる処理 */
		if ( work->open_time_g > 0 ) {
			GM_SetMenuStatus( MENU_WEAPON_OPEN ) ;
			work->flag = MENU_WIN_CLOSING ;
			work->open_time_l = -4 ;
			DisplayGroup( work, 0, 0, 1 ) ;
			if ( -- work->open_time_g == 0 ) {
				GM_ResetPlayerStatus( PLAYER_PAD_OFF | PLAYER_MENU_OPEN ) ;
//				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01 ) ;
				GM_GageStatus = GM_GAGE_STATE_APPEAR ;
#ifndef NO_PAUSE
				GV_PauseOffActorSystem( GV_PAUSE_MENU ) ;
#endif
				work->flag = MENU_WIN_CLOSE ;

				if ( work->start != work->current ) {
#if 0
					if ( work->start != WP_None ) {
						GM_WeaponPrev = work->start ;
					}
					GM_Weapon = work->current ;
					GM_WeaponChanged = 1 ;
#else
					MENU_ChangeWeapon( work->current );
#endif
					change = 1 ;
					work->selected_time = 32 ;		/* 選択用カウンタ */
				}
			}
		}

		work->quick = 0 ;
		/* 変更不可武器のチェック */
		if ( change == 1 ) {
			if ( !CheckDisableChanging() ) GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01 ) ;
		}
		/* 選択中のアイテム表示 */
		if ( work->selected_time ){
			work->selected_time-- ;
			/* 選択直後は点滅表示を行う */
			if ( ( work->selected_time & 4 ) == 0 ) MENU_PutWeaponPanel( WP_PX, PY, GM_Weapon, 128, 128, 0 );
		} else {
			/* 通常時は無装備以外の */
			if ( GM_Weapon != WP_None ) {
				MENU_PutWeaponPanel( WP_PX, PY, GM_Weapon, 128, 128, 0 );
			}
		}
		return ;
	}

	switch( work->flag ) {
	case MENU_WIN_OPEN :
	case MENU_WIN_CLOSING :
		GM_SetMenuStatus( MENU_WEAPON_OPEN ) ;
	case MENU_WIN_CLOSE :
		OpenMenu( work ) ;
		break ;
	case MENU_WIN_MOVE_GROUP_U :
	case MENU_WIN_MOVE_GROUP_D :
		GM_SetMenuStatus( MENU_WEAPON_OPEN ) ;
		MoveGroup( work ) ;
		break ;
	case MENU_WIN_MOVE_LIST_L :
	case MENU_WIN_MOVE_LIST_R :
		GM_SetMenuStatus( MENU_WEAPON_OPEN ) ;
		MoveList( work ) ;
		break ;
	}
	/* ゲージ消す */
	if ( GM_CheckMenuStatus( MENU_WEAPON_OPEN ) ) GM_GageStatus = GM_GAGE_STATE_DISAPPEAR ;
}

static	void	Die( Work *work )
{
#ifndef NO_PAUSE
	GV_PauseOffActorSystem( GV_PAUSE_MENU );
#endif
}

/*------------------------------------------------------------------*/

static	int		GetResources( Work *work, int which ) 
{
	extern void *NewMenuPrimControl( void );
	work->pad = GV_PadData + which ;

	//NewMenuPrimControl();

	return 1 ;
}

/*------------------------------------------------------------------*/

void	*NewWeaponChange2( int which )
{
	Work			*work ;

    work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, which ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
    return work ;
}
