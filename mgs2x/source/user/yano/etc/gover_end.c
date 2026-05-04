//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  	gover_end.c
	ゲームオーパー時にEXITを選んだとき再度確認する
	2002/08/08 Y.Yano
	
	$Id: gover_end.c,v 1.3 2002/11/23 12:42:27 Yoshizawa1 Exp $
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

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"camera.h"
#include	"def_dma.h"
#include	"sprite_2d.h"
#include	"../../kira/2D_action/layout_2d.h"

extern void SPR_ForcePriorityChange( int chanl, int flag );

#define		YES_no 0 /* 1:のときYESにカーソルがあって表示される 0:の時はNOになる */

typedef struct _Work{
	GV_ACT_EX	actor;
	int 		*flag;	/* ゲームオーバー関数とのやりとりに使う変数 */
	int 		handle;	/* L2Dのハンドル */
	int			phase;	/* 動作制御 */
	int			y_n;	/* YES:0,NO:1 */
	int			se_cnt;	/* SEタイミングカウンタ */

	int			tri_code;	/* TRIのハンドラ */
	SPR_OBJ*	moji_obj;	/* 言語によってテクスチャが変わるスプライト部分 */
} Work;

enum {
	PH_FADEIN_START = 0,
	PH_FADEIN,
	PH_SELECT,
	PH_FADEOUT_START,
	PH_FADEOUT,
};
/*------------------------------------------------------------------------------------*/

static void Act( Work *work ){
	int stat;

	switch( work->phase ){
	  case PH_FADEIN_START:
		stat = L2D_ActionStatus( work->handle );
		if( stat == L2D_STAT_ACK ){
			/* DefaultActionが再生終了 */
			int res;
#if YES_no
			/* YESにカーソルがあっている */
			res = L2D_EvokeAction( work->handle, GV_StrCode("show") );
#else
			/* NOにカーソルがあっている */
			res = L2D_EvokeAction( work->handle, GV_StrCode("show_no") );
#endif
			if( res == L2D_STAT_ACK ){
				work->se_cnt = 0;
				work->phase = PH_FADEIN;
			}
		}
		break;
	  case PH_FADEIN:
		if( work->se_cnt == 70 ){
			/* 線が動き出すあたりで鳴らす */
			GM_SdSet( SD_S_WINOPNR1 );
		}
		work->se_cnt ++;
		stat = L2D_ActionStatus( work->handle );
		if( stat == L2D_STAT_ACK ){
			/* showが再生終了 */
			work->phase = PH_SELECT;
		}
		break;
	  case PH_SELECT:
		stat = L2D_ActionStatus( work->handle );
		if( stat != L2D_STAT_ACK ){
			/* 何かが再生中のときはパッド操作は受け付けない */
			break;
		}
		/* パッド監視 */
		if( GV_PadDataDirect[0].press & PAD_OK ){
			/* 選択された */
			work->phase = PH_FADEOUT_START;
			if( work->y_n == 0 ){
				*work->flag = 1;/* YES選択 */
				GM_SdSet( SD_S_START01 );
			} else {
				*work->flag = 2;/* NO選択 */
				GM_SdSet( SD_S_V_CANS02 );
			}
		} else if( GV_PadDataDirect[0].press & PAD_CANCEL ){
			/* 確認画面でキャンセルボタンを押してもゲームオーバー画面に戻る */
			work->phase = PH_FADEOUT_START;
			work->y_n = 1;
			*work->flag = 2;/* NO選択 */
			GM_SdSet( SD_S_V_CANS02 );
		} else if( GV_PadDataDirect[0].press & PAD_R && work->y_n == 0 ){
			/* YES->NOへ */
			work->y_n = 1;
			L2D_EvokeAction( work->handle, GV_StrCode("moov_rect_yes_no") );
			GM_SdSet( SD_S_CUR01 );
		} else if( GV_PadDataDirect[0].press & PAD_L && work->y_n == 1 ){
			/* NO->YESへ */
			work->y_n = 0;
			L2D_EvokeAction( work->handle, GV_StrCode("moov_rect_no_yes") );
			GM_SdSet( SD_S_CUR01 );
		}
		break;
	  case PH_FADEOUT_START:
		stat = L2D_ActionStatus( work->handle );
		if( stat == L2D_STAT_ACK ){
			/* なにも再生中でないのをチェック */
			int res;
			if( work->y_n == 0 ){
				/* yesが残るフェード */
				res = L2D_EvokeAction( work->handle, GV_StrCode("hide_yes") );
				if( res == L2D_STAT_ACK ){
					work->phase = PH_FADEOUT;
				}
			} else {
				/* NOが残るフェード */
				res = L2D_EvokeAction( work->handle, GV_StrCode("hide_no") );
				if( res == L2D_STAT_ACK ){
					work->phase = PH_FADEOUT;
				}
			}
		}
		break;
	  case PH_FADEOUT:
		stat = L2D_ActionStatus( work->handle );
		if( stat == L2D_STAT_ACK ){
			/* hide_yrs/no が再生終了した */
			if( GM_StreamIsPlay() != 0 ){
				/* ストリーム再生中だったら待つ */
				return ;
			}
			if( work->y_n == 0 ){
				/* yesのときはゲームオーバー画面に戻る必要がないのでここでリスタートをかける */
				GM_GameOverRestart( 1 /* EXIT */ );
			}
			GV_DestroyActor( work );
		}
		break;
	  default:
		printf("error!---------gover_end.c\n");
		break;
	}

	/* 多言語対応 */
	if( GM_Language != GM_LANG_ENGLISH ){
		if( GM_Language == GM_LANG_FRENCH ){
			SPR_ObjSetTexture( work->moji_obj, GV_StrCode("moji_frn_alp_ovl"), work->tri_code );
		} else if( GM_Language == GM_LANG_GERMANY ){
			SPR_ObjSetTexture( work->moji_obj, GV_StrCode("moji_grm_alp_ovl"), work->tri_code );
		} else if( GM_Language == GM_LANG_ITALY ){
			SPR_ObjSetTexture( work->moji_obj, GV_StrCode("moji_itl_alp_ovl"), work->tri_code );
		} else if( GM_Language == GM_LANG_SPANISH ){
			SPR_ObjSetTexture( work->moji_obj, GV_StrCode("moji_spn_alp_ovl"), work->tri_code );
		}
	}

	return;
}

static void Die( Work *work ){

	/* ゲームオーバー関数に渡している関数の更新 */
	if( work->y_n == 1 ){
		/* NO選択して、死んだ */
		*work->flag = 3;
	}
	printf("CONFIRM[%d]\n",*work->flag);
	/* L2Dデータの開放 */
	if( work->handle >= 0 ){
		printf("L2d release -----gover_end.c\n");
	    L2D_ReleaseLayout( work->handle );
	}
	if( GM_Configuration & GM_CONFIG_STORY_TANKER ){
		/* タンカー編の時のみ */
		/* 最強SPR表示をはずす */
		SPR_ForcePriorityChange( DG_CHANL_MENU, 0 );
	}
}


/*------------------------------------------------------------------------------------*/
static int GetResources( Work *work )
{
	int handle, res;

	if( GM_Configuration & GM_CONFIG_STORY_TANKER ){
		/* タンカー編の時のみ */
		/* 最強SPR表示(タンカー編ゲームオーバーdmapackより後) */
		SPR_ForcePriorityChange( DG_CHANL_MENU, 1 );
	}
	handle = L2D_LoadLayout2( GV_StrCode("end_game"), DG_CHANL_MENU, 0, SPR_FLAG_PRIV, GV_PAUSE_STOP );
	if( handle < 0 ){
		printf("l2d didn't load------error(gover_end.c)\n");
		return -1;
	}
	res = L2D_EvokeAction( handle, GV_StrCode("DefaultAction") );
	if( res != L2D_STAT_ACK ){
		printf("l2d didn't evoke------error(gover_end.c)\n");
		return -1;
	}
	work->handle = handle;
	printf("end handle [%d]\n",work->handle );

	work->moji_obj = L2D_GetObject(work->handle, GV_StrCode("moji_eng"));
	work->tri_code = SPR_LoadTexture( GV_StrCode("end_game") );

#if YES_no
	/* 最初はYESにカーソルがあっている */
	work->y_n = 0;
#else
	/* 最初はNOにカーソルがあっている */
	work->y_n = 1;
#endif

	return 0;
}

void *NewGameOverExitConfirm( int *flag  )
{
	Work		*work ;

	printf("G.O. confirmation![%d]\n", *flag );

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;

		*flag = -1;/* 起動中 */
		work->flag = flag;

		if ( GetResources( work ) < 0 ) {
			*flag = 0;/* 起動前の状態 */
			GV_DestroyActor( work );
			return NULL;
		}
	}
	return (void *)work ;
}

