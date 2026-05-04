//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   wp_chng1.c
   武器チェンジテスト（その１）

   2000/08/28 M.Sonoyama
   2000/10/12 K.Takabe (custumized from wp_chng2.c)
   $Id: wp_chng1.c,v 1.1.1.3 2002/11/19 11:45:14 Yoshizawa1 Exp $
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

/*------------------------------------------------------------------*/

extern	int	GM_WeaponChanged ;

#define	Weapons		GM_WeaponNames

static	int		ListDatas[] = {
	WP_Rgb6, WP_Nikita, WP_Stinger,
	WP_Grenade, WP_ChaffGrenade, WP_StunGrenade, WP_Magazine,
	WP_m4, WP_Aks, WP_Psg1, WP_Psg1T,
	WP_m92, WP_Usp, WP_Socom,
	WP_None,
	WP_ColdSpray,
	WP_Claymore, WP_C4Bomb,
	WP_Book,
	WP_Mic, WP_DemoMic,
	WP_Blade,
	-1,
};


/*------------------------------------------------------------------*/

#if 0
#define	EXP_WIN_X	(-120)
#define	EXP_WIN_Y	(-48*Y_ADJ)
#define	EXP_WIN_W	(240)
#define	EXP_WIN_H	(96*Y_ADJ)
#endif

//#define	NO_PAUSE

/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/

typedef	struct	{
	GV_ACT				actor ;

	GV_PAD				*pad ;
	
	int					quick ;
	int					flag ;

	int					upper, lower ;
	int					start ;
	int					current ;

	int					open_time_l ;			/* リスト表示時間（OPEN_TIME_Lで完全に開く） */
	int					open_time_e ;			/* 説明文表示時間（OPEN_TIME_Eで完全に開く） */
	
	int					move_time ;

	int					max_list ;				/* 選択可能なリスト数 */
	int					current_list ;			/* 現在選択中のリスト番号 */
	int					list[ MAX_WEAPONS + 1 ] ;	/* 選択可能なリスト */

	u_char				rtime, ltime, utime, dtime ;
	u_char				rtime2, ltime2, utime2, dtime2 ;

	int					rpress ;
	int					rstat ;

	int					selected_time ;			/* 選択時点滅用カウンタ */

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
	int			i, n ;

	work->current_list = 0 ;
	work->max_list = 0 ;
	for ( i = 0; i < MAX_ITEMS; i ++ ) {
      // BP - Yet another case where the loop terminator up top should read ( ListDatas[i] != -1 )
      // instead, they pick a counter that is larger than the array and punt out right at the beginning
      // Scary but harmless.

		n = ListDatas[ i ] ;		/* アイテム番号取得 */
		if ( n == -1 ) break ;
		if ( ( GM_WeaponNum( n ) > 0 ) || ( ( GM_WeaponNum( n ) == 0 ) && ( GM_WeaponTypes[ n ] & WP_TYPE_BODY ) ) ){
			work->list[ work->max_list ] = n ;
			if ( work->current == n ) work->current_list = work->max_list ;
			work->max_list++ ;
		}
	}
	work->list[ work->max_list ] = work->list[ 0 ];	/* ループ対策（必要ないかも？） */

}

/* 近い方の検索 */
static int GetNearListDir( Work *work, int from_i, int to_i )
{
	int		i, p_count = 0, m_count = 0, dir, max ;
	int		from_n = 0, to_n = 0 ;
	max = work->max_list ;
	for ( i = 0 ; i < max ; i++ ){
		if ( work->list[ i ] == from_i ) from_n = i ;
		if ( work->list[ i ] == to_i ) to_n = i ;
	}
	if ( to_n > from_n ){
		p_count = to_n - from_n ;
	} else {
		p_count = to_n + max - from_n ;
	}
	if ( from_n > to_n ){
		m_count = from_n - to_n ;
	} else {
		m_count = from_n + max - to_n ;
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

static	void	DisplayList( Work *work, int shift, int mode )
{
	int			time, x, y, n ;
	int			i, a1, a2, start, end ;
	int			alpha_buffer[2] ;

	time = work->open_time_l ;
	if ( time > 0 ) {

		{/* アルファの調整 */
			/* タイトル部のアルファ計算 */
			a2 = 128 * time / OPEN_TIME_L ;
#if 0
			if ( a2 < 0 ) a2 = 0 ;
			/* アイコンウィンドウのアルファ計算（タイトル部の影響を受ける） */
			a1 = ( PY - ( y + sy ) ) ;
			if ( a1 < 0 ) a1 = -a1 ;
			if ( a1 > PH ) a1 = PH ;
			a1 = a2 - a2 * a1 / PH ;
			/* やっぱりタイトル部はアイコン部との平均を取ってみる */
			a2 = ( a1 + a2 ) / 2 ;
			//a1 = a2 - ( a2 - a1 ) * 6 / 8 ;
#endif
			a1 = a2 ;
		}

		/* 表示範囲の決定 */
		start = 0 - work->max_list / 2 ;	/* startは含む */
		end = work->max_list + start ;		/* endは含まない */
		if ( start < -( DISP_GROUPS - 1 ) ) start = -( DISP_GROUPS - 1 ) ;
		if ( end >= ( DISP_LISTS ) ) end = DISP_LISTS ;

		/* スクロール端パネルの表示（選択行の場合のみ） */
		if ( shift != 0 ){
			int		blend ;
			if ( work->flag == MENU_WIN_MOVE_LIST_L ){
				blend = ( shift < 0 ) ? -shift : shift ;
				blend = 128 * blend / ( SHIFT_TIME ) ;
				if ( blend > 128 ) blend = 128 ;
				alpha_buffer[0] = blend ;		/* i==start */
				alpha_buffer[1] = 128 - blend ;	/* i==(end-1) */
				start-- ;
			} else if ( work->flag == MENU_WIN_MOVE_LIST_R ){
				blend = ( shift < 0 ) ? -shift : shift ;
				blend = 128 * blend / ( SHIFT_TIME ) ;
				if ( blend > 128 ) blend = 128 ;
				alpha_buffer[0] = 128 - blend ;	/* i==start */
				alpha_buffer[1] = blend ;		/* i==(end-1) */
				end++ ;
			}
		}

		/* 通常表示 */
		for ( i = start ; i < end ; i ++ ) {
			int	flag, aa1, aa2 ;

			/* アルファの設定 */
			if ( shift == 0 ){
				aa1 = a1 ;
				aa2 = a2 ;
			} else {
				if ( i == start ){
					aa1 = a1 * alpha_buffer[0] / 128 ;
					aa2 = a2 * alpha_buffer[0] / 128 ;
				} else if ( i == ( end - 1 ) ){
					aa1 = a1 * alpha_buffer[1] / 128 ;
					aa2 = a2 * alpha_buffer[1] / 128 ;
				} else {
					aa1 = a1 ;
					aa2 = a2 ;
				}
			}

			n = i + work->current_list ;
			/* 表示パーツの決定 */
			if ( n >= work->max_list ) n -= work->max_list ;
			if ( n < 0 ) n += work->max_list ;

			/* フラグの設定（選択中のアイテムにフラグを立てる） */
			flag = ( ( shift == 0 ) &&  ( i == 0 ) ) ? 1/* 選択中 */ : 2/* 非選択中 */ ;
			if ( _CheckDisableChanging( work->list[n] ) ) flag |= PANEL_FLAG_NO_USE ;

			/* 表示 */
			if ( i < 0 || ( i == 0 && shift > 0 ) ){
				/* 縦列の表示 */
				x = WP_PX ;
				y = PY + ( ( PH + STEP_Y ) * i ) * time / OPEN_TIME_L ;
				y -= ( PH + STEP_Y ) * shift / SHIFT_TIME ;
				MENU_PutWeaponPanel( x, y, work->list[n], aa1, aa2, flag );
			} else {
				/* 横列の表示 */
				y = PY ;
				x = WP_PX - ( ( PW + STEP_X ) * i ) * time / OPEN_TIME_L ;
				x += ( PW + STEP_X ) * shift / SHIFT_TIME ;
				MENU_PutWeaponPanel( x, y, work->list[n], aa1, aa2, flag );
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
	int			rstat ;

	if ( work->quick == 0 && OpenEnable( work ) ) {
		/* 初期化 */
		work->start = work->current = GM_Weapon ;
#if 0
		/* 現在の武器をチェック */
		CheckAvailable( work ) ;
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
			/* 現在の武器をチェック */
			CheckAvailable( work ) ;
		} 
		if ( work->open_time_l < OPEN_TIME_L ) work->open_time_l ++ ;

		DisplayList( work, 0, 0 ) ;
		/* 説明分表示 */
		if ( work->flag == MENU_WIN_OPEN && work->open_time_l == OPEN_TIME_L ){
			if ( work->open_time_e < OPEN_TIME_E ) work->open_time_e ++ ;
			if ( work->open_time_e == OPEN_TIME_E && work->current != WP_None ){
				UpdateMessageTexture( work );
				MENU_PutExplainText( EXP_WIN_WX, EXP_WIN_Y, 1 );
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
			press = 0 ;
			if ( work->current != MENU_AutoSelectWeapon ){
				if ( GetNearListDir( work, work->current, MENU_AutoSelectWeapon ) < 0 ){
					press = PAD_D ;
				} else {
					press = PAD_U ;
				}
			} else {
				MENU_AutoSelectWeapon = -1 ;
			}
		} else {
			press = work->pad->press ;
		}

		if ( ( press & PAD_U ) || work->utime > 30 ) {
			if ( work->max_list > 1 ) {
				work->flag = MENU_WIN_MOVE_LIST_R ;
				work->move_time = 0 ;
				work->open_time_e = 0 ;
			}
		} else if ( ( press & PAD_R ) || work->rtime > 30 ) {
			if ( work->max_list > 1 ) {
				work->flag = MENU_WIN_MOVE_LIST_R ;
				work->move_time = 0 ;
				work->open_time_e = 0 ;
			}
		} else if ( ( press & PAD_D ) || work->dtime > 30 ) {
			if ( work->max_list > 1 ) {
				work->flag = MENU_WIN_MOVE_LIST_L ;
				work->move_time = 0 ;
				work->open_time_e = 0 ;
			}
		} else if ( ( press & PAD_L ) || work->ltime > 30 ) {
			if ( work->max_list > 1 ) {
				work->flag = MENU_WIN_MOVE_LIST_L ;
				work->move_time = 0 ;
				work->open_time_e = 0 ;
			}
		}
	}
}

/* リストの回転 */
static	void	MoveList( Work *work )
{
	int			time ;
	int			flag ;
	int			shift ;

	time = work->move_time ;
	work->move_time ++ ;

	flag = work->flag ;

	shift = time ;
	if ( flag == MENU_WIN_MOVE_LIST_L ) shift = 0 - shift ;

	DisplayList( work, shift, 0 ) ;

	if ( time == SHIFT_TIME ) {
		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_ISEL02 ) ;
		if ( flag == MENU_WIN_MOVE_LIST_R ) {
			work->current_list++ ;
			if ( work->current_list >= work->max_list ) work->current_list = 0 ;
		} else {
			work->current_list-- ;
			if ( work->current_list < 0 ) work->current_list = work->max_list - 1 ;
		}
		work->current = work->list[ work->current_list ] ;
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
		if ( work->open_time_l > 0 ) {
			GM_SetMenuStatus( MENU_WEAPON_OPEN ) ;
			work->flag = MENU_WIN_CLOSING ;
			DisplayList( work, 0, 0 ) ;
			if ( -- work->open_time_l == 0 ) {
				GM_ResetPlayerStatus( PLAYER_PAD_OFF | PLAYER_MENU_OPEN ) ;
//				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01 ) ;
				GM_GageStatus = GM_GAGE_STATE_APPEAR ;
#ifndef NO_PAUSE
				GV_PauseOffActorSystem( GV_PAUSE_MENU ) ;
#endif
				work->flag = MENU_WIN_CLOSE ;

				/* 選択不可能アイテムを選択した場合の処理 */
				//if ( work->current != WP_None && _CheckDisableChanging( work->current ) ){
				//	work->current = work->start ;
				//}

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

void	*NewWeaponChange1( int which )
{
#if 0
	Work			*work ;

	if ( !( GM_Configuration & GM_CONFIG_OLD_TYPE_MENU ) ) return ( NewWeaponChange2( which ) );

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
#else
	NewMenuPrimControl();
	return ( NULL );
#endif
}


void	*NewWeaponChange( int which )
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
