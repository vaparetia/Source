//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shipworm.c
	メニュー取り付きフナムシ

	2001/02/24 K.Takabe
	$Id: shipworm.c,v 1.1.1.3 2002/11/19 11:45:14 Yoshizawa1 Exp $

*/
/*

chara メニューフナムシ[NewMenuShipwormSet] $s:名前 \
	-speed $w:レーション食いつぶしスピード（秒単位） \
	-proc $p:呼び出しProc
mesg メニューフナムシ $s:名前 remove[0]
mesg メニューフナムシ $s:名前 transfer[1]
// transferメッセージで取り付き開始
// removeメッセージで強制的に排除することも可能
// 呼び出しProcはレーションが消費される度に呼び出される


command メニューフナムシ取り付き状態取得[NewGetShipwormStatus]
// メニューにフナムシが取り付いている場合に１を返します


*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <math.h>
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
#include	"g_other.h"
#include	"menu.h"


/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	int			name ;
	int			where ;

	int			mode ;
	int			time ;			/* 侵食時間 */
	int			power ;			/* 振り落としパワー */
	int			max_time ;		/* 食いつぶし猶予時間 */
	int			old_move_dir ;
	int			old_x, old_y ;
	float		fall_x, fall_y ;
	float		force_x, force_y ;
	int			proc_id ;

} Work ;

static Work *gShipwormWork = NULL;

/* ---------------------------------------------------------------- */

void BP_shipworm_BSS_Init()
{
   gShipwormWork = NULL;
}

/* ---------------------------------------------------------------- */
	/*
		メニューレーション表示位置情報関連
	*/
extern int MENU_GetRationPanelInfo( int *x, int *y, int *alpha );

	/*
		表示制御用
	*/
extern int MENU_GetMenuDispStatus( void );
extern void MENU_SetMenuDispStatus( int status );
extern void MENU_ResetMenuDispStatus( int status );

/* ---------------------------------------------------------------- */

static void _shake_off_ration(Work *work, int ration_x, int ration_y)
{
   work->mode = 2 ;
   work->power = 0 ;
   work->fall_x = ration_x ;
   work->fall_y = ration_y ;
   work->force_x = 2.0f ;
   work->force_y = -8.0f ;
   MENU_ResetMenuDispStatus( MENU_DISP_SHIPWORM );
   GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_FNAESC01 ) ;
}

/* ---------------------------------------------------------------- */

// Arm fix:
// added this global function and a global work pointer
// to allow the vita menu system to shake the sea lice off the rations
// fix for MGSTWO-3395
void ShipwormShakeOffRation()
{
   if (gShipwormWork != NULL)
   {
      _shake_off_ration(gShipwormWork, gShipwormWork->old_x, gShipwormWork->old_y);
   }
}

/* ---------------------------------------------------------------- */
/* レーション侵食中 */
static void ErodeRation( Work *work )
{
	int		ration_x, ration_y, ration_alpha, flag, dir ;

   /* メニューのレーション表示位置取得 */
   flag = MENU_GetRationPanelInfo( &ration_x, &ration_y, &ration_alpha );
	/* 画面に表示されていたらフナムシを表示 */
	if ( flag ){
      // int y = ration_y * DRAW_HEIGHT / VR_HEIGHT ;
      // MENU_Locate( ration_x - 12, y - 12, 0 );
      // MENU_Color( 192, 192, 192, ration_alpha );
      // MENU_Printf( "FUNA !! %d", work->time  );
	}

	/* アイテムメニューを開いていなければ終了 */
	if ( !GM_CheckMenuStatus( MENU_ITEM_OPEN ) ){
		work->old_x = ration_x ;
		work->old_y = ration_y ;
		work->old_move_dir = -1 ;
		return ;
	}

	/* フナムシ振り落とし判定処理 */
	if ( work->old_x != ration_x || work->old_y != ration_y ){
		int	dx, dy ;
		dx = work->old_x - ration_x ;
		dy = work->old_y - ration_y ;
		dir = 0 ;
		if ( dx > 0 && dx < PW ) dir |= 1 ;
		if ( dx < 0 && dx > -PW ) dir |= 2 ;
		if ( dy > 0 && dy < PH ) dir |= 4 ;
		if ( dy < 0 && dy > -PH ) dir |= 8 ;
		if ( dir != 0 ){
			if ( work->old_move_dir != dir ){
				//work->power += 16 ;
				work->power += 20 ;
				if ( work->power > 50 ){
					/* 振り落としに成功した場合 */
               _shake_off_ration(work, ration_x, ration_y);
				}
			}
			work->old_move_dir = dir ;
		}
	}

	work->old_x = ration_x ;
	work->old_y = ration_y ;
}
/* メニューからはなれる */
static void Fall( Work *work )
{
	//int		x, y ;
	work->fall_x += work->force_x ;
	work->fall_y += work->force_y ;
	work->force_y += 0.5f ;
#if 0
	x = work->fall_x ;
	y = work->fall_y * DRAW_HEIGHT / VR_HEIGHT ;
	MENU_Locate( x, y, 0 );
	MENU_Color( 192, 192, 192, 128 );
	MENU_Printf( "FUNA FUNA !!!!" );
#else
	MENU_PutShipwormPanel( work->fall_x, work->fall_y );
#endif
	if ( work->fall_y > ( DRAW_HEIGHT + 30 ) ){
		work->mode = 0 ;
		GM_ShipwormFlag = 0 ;
	}
}
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	{/* メッセージチェック */
		GV_MSG *msg;
		int n;
		if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
			for ( ; n > 0 ; n--, msg++ ){
				switch ( msg->message[0] ){
				  case 0:/* remove */
					work->mode = 0 ;
					GM_ShipwormFlag = 0 ;
					MENU_ResetMenuDispStatus( MENU_DISP_SHIPWORM );
					break ;
				  case 1:/* transfer */
					if ( work->mode == 0 ){
						work->mode = 1 ;
						GM_ShipwormFlag = 3 ;	/* フラグを最大消費数と兼用する */
						work->time = work->max_time ;
						GM_ShipwormCorrode = work->max_time / 300 ;
						work->power = 0 ;
						MENU_SetMenuDispStatus( MENU_DISP_SHIPWORM );
						/* アイテム装備中のみ取り付きＳＥを鳴らす */
						if ( GM_Item == IT_Ration ){
							GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_FNAPET01 ) ;
						}
					}
					break ;
				}
			}
		}
	}

	/* 振り落としパワー減衰 */
	if ( work->power > 0 ){
		work->power-- ;
	}

	switch ( work->mode ){
	  case 0:/* 何もしない */
		MENU_SetShipwormOffset( 0, 0 );
		break ;
	  case 1:/* レーションに取り付き中 */
		if ( GV_PauseLevel == 0 ){
			if ( GM_ItemNum( IT_Ration ) > 0 ){
				work->time -= TIME_BASE ;
				if ( work->time < 0 ){
					work->time = work->max_time ;
					/* レーション１つ消滅 */
					GM_DecrementItem( IT_Ration, 1 ) ;
					GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_FNAEAT01 ) ;
					/* 指定回レーションを消費した場合には自動的に外れる */
					GM_ShipwormFlag-- ;
					if ( GM_ShipwormFlag <= 0 ){
						work->mode = 0 ;
						GM_ShipwormFlag = 0 ;
						MENU_ResetMenuDispStatus( MENU_DISP_SHIPWORM );
						GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_FNAESC01 ) ;
					}
				}
			}
		}
		GM_ShipwormCorrode = work->time / 300 ;
		ErodeRation( work );
#if !BP_VITA
		if ( work->old_move_dir & 1 ){
			MENU_SetShipwormOffset( work->power/3, 0 );
		} else if ( work->old_move_dir & 2 ){
			MENU_SetShipwormOffset( -work->power/3, 0 );
		} else if ( work->old_move_dir & 4 ){
			MENU_SetShipwormOffset( 0, work->power/3 );
		} else if ( work->old_move_dir & 8 ){
			MENU_SetShipwormOffset( 0, -work->power/3 );
		}
#endif
		break ;
	  case 2:/* 落下 */
		MENU_SetShipwormOffset( 0, 0 );
		Fall( work );
		break ;
	}
	
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	/* 念のためクリアしておく */
	MENU_ResetMenuDispStatus( MENU_DISP_SHIPWORM );

   gShipwormWork = NULL;
}

/* ---------------------------------------------------------------- */
static int GetResources( Work *work, int name, int where )
{
	work->name = name ;
	work->where = where ;

	if ( GCL_GetOption( 's' ) != NULL ){
		work->max_time = GCL_GetNextInt() * 300 ;
	} else {
		work->max_time = 1 * 60 * 300 ;	/* １分 */
	}

	if ( GCL_GetOption( 'p' ) ){
		work->proc_id = GCL_GetNextInt();
	}

	if ( GM_ShipwormFlag ){
		work->mode = 1 ;
		work->time = GM_ShipwormCorrode * 300 ;
		MENU_SetMenuDispStatus( MENU_DISP_SHIPWORM );
	} else {
		MENU_ResetMenuDispStatus( MENU_DISP_SHIPWORM );
	}
   
   gShipwormWork = work;

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewMenuShipwormSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	//work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	/* 装備メニューよりも後に実行されるようにプライオリティを調整 */
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
 									sizeof( Work ), PLAYER_MENU_ACTOR_PRIO + 1 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* フナムシ取り付き状態の取得 */
int NewGetShipwormStatus( void )
{
	int		res = 0 ;
#if 0
	if ( MENU_GetMenuDispStatus() & MENU_DISP_SHIPWORM ){
		res = 1 ;
	}
#else
	if ( GM_ShipwormFlag ){
		res = 1 ;
	}
#endif
	return ( res );
}
