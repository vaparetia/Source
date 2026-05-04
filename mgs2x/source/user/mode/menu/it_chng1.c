//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   it_chng1.c
   アイテムチェンジテスト（その１）

   2000/09/27 M.Sonoyama
   2000/10/12 K.Takabe (custumized from it_chng2.c)
   $Id: it_chng1.c,v 1.1.1.3 2002/11/19 11:45:12 Yoshizawa1 Exp $
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

#define	Items		GM_ItemNames

static	int		ListDatas[] = {
	IT_Jacket, IT_Uniform,
	IT_CBBox, IT_CBBoxWet, IT_CBBoxB, IT_CBBoxC, IT_CBBoxD, IT_CBBoxE,
	IT_BombSenserA, IT_BombSenserB, IT_MineDetector, IT_VibSensor,
	IT_NightVision, IT_Thermal,
	IT_Scope, IT_DummyScope, IT_Camera, IT_TnkCamera, IT_DummyTnkCamera,
	IT_Ration,
	IT_None,
	IT_Styptic, IT_Diazepam, IT_Medicine,
	IT_Tabacco,
	IT_Card, IT_MODisc,
	IT_SocomSpprsr, IT_AKSpprsr, IT_UspSpprsr,
	IT_Shaver, IT_PHS,
	IT_DogTag,
	IT_MugenBandana, IT_MugenWig, IT_Stealth,
	IT_WigA, IT_WigB, IT_WigC, IT_WigD,
	-1,
};

extern	int		GM_ItemChanged ;

#define	MAGAZINE_PAGE_MAX	(256)
//static	int	MagazinePage = 1 ;

/*------------------------------------------------------------------*/

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
	int					list[ MAX_ITEMS + 1 ] ;	/* 選択可能なリスト */

	u_char				rtime, ltime, utime, dtime ;
	u_char				rtime2, ltime2, utime2, dtime2 ;

	int					rpress ;
	int					rstat ;

	int					selected_time ;			/* 選択時点滅用カウンタ */

	int					shipworm_erode ;		/* フナムシ侵食度 */
	int					struggle_power ;		/* メニュー振り回し累積パワー */

	int					padflag ;	/* パッドデモキャンセルクイックチェンジ防止用 */
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

static	Work	*ItemMenuWork ;


/*------------------------------------------------------------------*/


/*------------------------------------------------------------------*/
/*------------------------------------------------------------------*/
/* 説明文テクスチャを更新する（必ず説明文テクスチャを描画していないときに行うこと） */
static void UpdateMessageTexture( Work *work )
{
	MENU_UpdateItemExplainMessage( work->current );
}
/*------------------------------------------------------------------*/

/* 変更不可のチェック */
static	int		_CheckDisableChanging( int item )
{
	/* 無い */
	if ( item != IT_None && GM_ItemNum( item ) == 0 ) {
		return 1 ;
	}
	return PL_CheckNoUseItem( item ) ;
}
static	int		CheckDisableChanging( void )
{
	if ( _CheckDisableChanging( GM_Item ) ){
		GM_Item = IT_None ;
		return ( 1 );
	}
	return ( 0 );
}

static	inline	int	OpenEnable( work )
Work			*work ;
{
	if ( ( !GM_CheckPlayerStatus( PLAYER_MENU_OPEN ) || 
		  work->flag == MENU_WIN_CLOSING ) &&
		( work->pad->status & PAD_IT_CHANGE ) ) return 1 ;
	return 0 ;
}

static	inline	int	GetPadFlag( Work *work )
{
	return ( GM_CheckGameStatus( STATE_PAD_DEMO ) |
			( work->pad->flag & GV_PAD_RELEASE ) ) ;
}

/* 現在所持中のアイテムから表示に使用するリストを作成 */
static	void	CheckAvailable( Work *work )
{
	int			i, n ;

	work->current_list = 0 ;
	work->max_list = 0 ;
	for ( i = 0; i < MAX_ITEMS; i ++ ) {
		n = ListDatas[ i ] ;		/* アイテム番号取得 */
		if ( n == -1 ) break ;
		if ( GM_ItemNum( n ) > 0 ||
			( ( n == IT_Ration ) && ( ( MENU_GetMenuDispStatus() & MENU_DISP_SHIPWORM ) != 0 ) ) ){
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
			if ( work->flag == MENU_WIN_MOVE_LIST_R ){
				blend = ( shift < 0 ) ? -shift : shift ;
				blend = 128 * blend / ( SHIFT_TIME ) ;
				if ( blend > 128 ) blend = 128 ;
				alpha_buffer[0] = blend ;		/* i==start */
				alpha_buffer[1] = 128 - blend ;	/* i==(end-1) */
				start-- ;
			} else if ( work->flag == MENU_WIN_MOVE_LIST_L ){
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

			/* 表示パーツの決定 */
			n = i + work->current_list ;
			if ( n >= work->max_list ) n -= work->max_list ;
			if ( n < 0 ) n += work->max_list ;

			/* フラグの設定（選択中のアイテムにフラグを立てる） */
			flag = ( ( shift == 0 ) &&  ( i == 0 ) ) ? 1/* 選択中 */ : 2/* 非選択中 */ ;
			if ( _CheckDisableChanging( work->list[n] ) ) flag |= PANEL_FLAG_NO_USE ;

			/* 表示 */
			if ( i < 0 || ( i == 0 && shift < 0 ) ){
				/* 縦列パネルの表示 */
				x = IT_PX ;
				y = PY + ( ( PH + STEP_Y ) * i ) * time / OPEN_TIME_L ;
				y += ( PH + STEP_Y ) * shift / SHIFT_TIME ;
				MENU_PutItemPanel( x, y, work->list[n], aa1, aa2, flag );
			} else {
				/* 横列パネルの表示 */
				y = PY ;
				x = IT_PX + ( ( PW + STEP_X ) * i ) * time / OPEN_TIME_L ;
				x += ( PW + STEP_X ) * shift / SHIFT_TIME ;
				MENU_PutItemPanel( x, y, work->list[n], aa1, aa2, flag );
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

/* アイテム使用処理 */
static	void	UseItem( work )
Work		*work ;
{
	extern int	NewPlayerRecoverCold( void ) ;

    switch ( work->current ) {
    case IT_Ration :			/* レーション */
		if ( GM_ItemNum( IT_Ration ) > 0 &&
			work->pad->press & PL_PAD_ITEMUSE && 
			( ( GM_Vitality + GM_VitalityAdjust < GM_VitalityMax ) ||
			 ( GM_CheckPlayerStatus( PLAYER_BLOOD_DROP ) ) ) ) {
			GM_DecrementItem( IT_Ration, 1 ) ;
			GM_VitalityAdjust += GM_RATION_HEAL ;
			GM_ResetPlayerStatus( PLAYER_BLOOD_DROP ) ;
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_KAIHUKU1 ) ;
			if ( ++ GM_RationUseCount > 30000 ) GM_RationUseCount = 30000 ;
		}
		break ;
	case IT_Styptic :			/* 止血剤 */
		if ( GM_ItemNum( IT_Styptic ) > 0 &&
			work->pad->press & PL_PAD_ITEMUSE &&
			 GM_CheckPlayerStatus( PLAYER_BLOOD_DROP ) ) {
			GM_DecrementItem( IT_Styptic, 1 ) ;
			GM_ResetPlayerStatus( PLAYER_BLOOD_DROP ) ;
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDEC02 ) ;
		}
		break ;
	case IT_Medicine :			/* 風邪薬 */
		if ( GM_ItemNum( IT_Medicine ) > 0 &&
			 work->pad->press & PL_PAD_ITEMUSE &&
			 GM_CheckPlayerStatus( PLAYER_COLD ) ) {
			GM_DecrementItem( IT_Medicine, 1 ) ;
			NewPlayerRecoverCold() ;
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDEC02 ) ;
		}
		break ;
    case IT_Diazepam :			/* ジアゼパム */
		if ( GM_ItemNum( IT_Diazepam ) > 0 &&
			work->pad->press & PL_PAD_ITEMUSE ) {
			GM_DecrementItem( IT_Diazepam, 1 ) ;
			/* 重ねがけＯＫ */
			GM_DiazepamCount += GM_DIAZEPAM_COUNT ;
			if ( GM_DiazepamCount > 1000000 ) GM_DiazepamCount = 1000000 ;
			GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_IDEC02 ) ;
		}
		break ;	
#if 0
	case IT_Magazine :			/* 雑誌 */
		if ( GM_ItemNum( IT_Magazine ) > 0 ) {
			if ( work->pad->status & PAD_A ) {
				if ( ++ MagazinePage >= MAGAZINE_PAGE_MAX ) {
					MagazinePage = MAGAZINE_PAGE_MAX ;
				}
			} else if ( work->pad->status & PAD_B ) {
				if ( -- MagazinePage <= 1 ) {
					MagazinePage = 1 ;
				}
			}
		}
#endif
    }
}

/*------------------------------------------------------------------*/

static	void	OpenMenu( Work *work )
{
	int			rstat ;

	if ( work->quick == 0 && OpenEnable( work ) ) {
		/* 初期化 */
		work->start = work->current = GM_Item ;
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
    if ( GM_CheckPlayerStatus( PLAYER_ITEM_QUICK_ONLY ) ) {
		if ( GM_Item != IT_None ) {
			MENU_PutItemPanel( IT_PX, PY, GM_Item, 128, 128, 0 );
		}
		return ;
    }

printf("debug %s %d\n", __FILE__, __LINE__ );
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
			work->start = work->current = GM_Item ;
			/* 現在の武器をチェック */
			CheckAvailable( work ) ;
		} 
		if ( work->open_time_l < OPEN_TIME_L ) work->open_time_l ++ ;

		DisplayList( work, 0, 0 ) ;
		/* 説明分表示 */
		if ( work->flag == MENU_WIN_OPEN && work->open_time_l == OPEN_TIME_L ){
			if ( work->open_time_e < OPEN_TIME_E ) work->open_time_e ++ ;
			if ( work->open_time_e == OPEN_TIME_E && work->current != IT_None ){
				UpdateMessageTexture( work );
				MENU_PutExplainText( EXP_WIN_IX, EXP_WIN_Y, 0 );
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
		UseItem( work ) ;

		/* 自動選択処理 */
		if ( work->flag == MENU_WIN_OPEN && work->open_time_l == OPEN_TIME_L && MENU_AutoSelectItem != -1 ){
			press = 0 ;
			if ( work->current != MENU_AutoSelectItem ){
				if ( GetNearListDir( work, work->current, MENU_AutoSelectItem ) < 0 ){
					press = PAD_D ;
				} else {
					press = PAD_U ;
				}
			} else {
				MENU_AutoSelectItem = -1 ;
			}
		} else {
			press = work->pad->press ;
		}

		if ( ( press & PAD_U ) || work->utime > 30 ) {
			if ( work->max_list > 1 ) {
				work->flag = MENU_WIN_MOVE_LIST_L ;
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
				work->flag = MENU_WIN_MOVE_LIST_R ;
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
		if ( flag == MENU_WIN_MOVE_LIST_L ) {
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
	int		c = 0 ;

	GM_ItemChanged = 0 ;

	GM_ResetMenuStatus( MENU_ITEM_OPEN ) ;

	c = MENU_DefaultCheckItem();

	/* ポーズ中 */
    if ( ( GV_PauseLevel & GV_PAUSE_PAUSE ) ) {
		if ( work->quick < QUICK_TIME ) work->quick = 0 ;
		return ;
	}
	/* 振り回しチェック */
	if ( work->struggle_power > 0 ) work->struggle_power-- ;
	/* 非表示設定 */
	if ( GM_CheckMenuStatus( MENU_ITEM_OFF | MENU_WEAPON_OPEN ) ||
		( GM_CheckPlayerStatus( PLAYER_FORCE | PLAYER_LOCKER ) ) ) {
		if ( work->quick < QUICK_TIME ) work->quick = 0 ;
		return ;
    }	
	/* 押し直し必要 */
	if ( GM_CheckMenuStatus( MENU_MENU_NEWPRESS ) &&
		!( work->pad->press & PAD_IT_CHANGE ) ) {
		if ( GM_Item != IT_None ) {
			MENU_PutItemPanel( IT_PX, PY, GM_Item, 128, 128, 0 );
		}
		return ;
	}

	GM_ResetMenuStatus( MENU_MENU_NEWPRESS ) ;

	status = work->pad->status ;

	/* メニューが閉じている状態 */
    if ( ( !( status & PAD_IT_CHANGE ) && work->flag <= MENU_WIN_OPEN ) ||
		( GM_CheckPlayerStatus( PLAYER_ITEM_DISABLE ) ) ||
		GM_CheckMenuStatus( MENU_ITEM_DISABLE ) ) {
		if ( ( GM_CheckPlayerStatus( PLAYER_ITEM_DISABLE ) ) ||
			GM_CheckMenuStatus( MENU_ITEM_DISABLE ) ) {
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
			c = MENU_QuickChangeItem();
		}
		quick_change_cancel :
		/* ウインドウ閉じる処理 */
		if ( work->open_time_l > 0 ) {
			GM_SetMenuStatus( MENU_ITEM_OPEN ) ;
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
				//if ( work->current != IT_None && _CheckDisableChanging( work->current ) ){
				//	work->current = work->start ;
				//}

				if ( work->start != work->current ) {
#if 0
					if ( work->start != IT_None ) {
						GM_ItemPrev = work->start ;
					}
					GM_Item = work->current ;
					GM_ItemChanged = 1 ;
#else
					MENU_ChangeItem( work->current );
#endif
					c = 1 ;
					work->selected_time = 32 ;		/* 選択用カウンタ */
				}
			}
		}

		work->quick = 0 ;
		/* 変更不可武器のチェック */
		if ( c == 1 ) {
			if ( !CheckDisableChanging() ) {
				GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_S_EQUIP01 ) ;
			}
#if 0
			/* 雑誌特殊 */
			if ( GM_Item == IT_Magazine ) {
				extern void	*NewEquipMagazine( FMATRIX *world, int page ) ;
				FMATRIX		world ;

				DG_COPY_MAT( &world, &GM_PlayerBody->objs->world ) ;
				world.m[ 3 ][ 1 ] = GM_PlayerControl->levels[ 0 ] + 250.0F ;
				NewEquipMagazine( &world, MagazinePage ) ;

				GM_DecrementItem( IT_Magazine, 1 ) ;
				GM_Item = IT_None ;
				MagazinePage = 1 ;
			}			
#endif
		}
		/* 選択中のアイテム表示 */
		if ( work->selected_time ){
			work->selected_time-- ;
			/* 選択直後は点滅表示を行う */
			if ( ( work->selected_time & 4 ) == 0 ) MENU_PutItemPanel( IT_PX, PY, GM_Item, 128, 128, 0 );
		} else {
			/* 通常時は無装備以外の */
			if ( GM_Item != IT_None ) {
				MENU_PutItemPanel( IT_PX, PY, GM_Item, 128, 128, 0 );
			}
		}
		return ;		
	}

	switch( work->flag ) {
	case MENU_WIN_OPEN :
	case MENU_WIN_CLOSING :
		GM_SetMenuStatus( MENU_ITEM_OPEN ) ;
	case MENU_WIN_CLOSE :
		OpenMenu( work ) ;
		break ;
	case MENU_WIN_MOVE_LIST_L :
	case MENU_WIN_MOVE_LIST_R :
		GM_SetMenuStatus( MENU_ITEM_OPEN ) ;
		MoveList( work ) ;
		break ;
	}
	/* ゲージ消す */
	if ( GM_CheckMenuStatus( MENU_ITEM_OPEN ) ) GM_GageStatus = GM_GAGE_STATE_DISAPPEAR ;
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

void	*NewItemChange1( int which )
{
#if 0
	Work			*work ;

	if ( !( GM_Configuration & GM_CONFIG_OLD_TYPE_MENU ) ) return ( NewItemChange2( which ) );

    work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO ) ;
    if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, which ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
    }
	ItemMenuWork = work ;
    return work ;
#else
	NewMenuPrimControl();
	return ( NULL );
#endif
}

void	*NewItemChange( int which )
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
	ItemMenuWork = work ;
    return work ;
}
