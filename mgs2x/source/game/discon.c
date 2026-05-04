//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  パッド抜け監視＆抜け表示

  2002/08/27 M.Kobayashi

  $Id: discon.c,v 1.1 2002/11/25 02:05:00 Yoshizawa1 Exp $

 */

#include "gameheader.h"
#include "font.h"
#include <stdio.h>

#include "mode/menu/xtextscn.h"

#define	DISP_CONTROL_PORT

#define 	VRAM_WIDTH		(384+64)
#ifdef DISP_CONTROL_PORT
#define		VRAM_HEIGHT		FONT_BUFFER_HEIGHT( 3, 0 )
#else
#define		VRAM_HEIGHT		FONT_BUFFER_HEIGHT( 2, 0 )
#endif

//#define		DISP_X			136
#define		CENTER_X		256
#define		DISP_Y			256
//#define		DISP_W			256
#ifdef DISP_CONTROL_PORT
#define		DISP_H			48
#else
#define		DISP_H			32
#endif

#define		SCREEN_Y( y )	( (y) * DRAW_HEIGHT / 384 )

//#define		FRAME_X			DIRECT_SCREEN_X( DISP_X	- 8 ) 
#define		FRAME_Y			((float)SCREEN_Y( DISP_Y - 4 ))
//#define		FRAME_W			DIRECT_SCREEN_X( DISP_W ) 
#define		FRAME_H			((float)SCREEN_Y( DISP_H + 8 ))



extern void *NewTextScreenControlEx( int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag );
extern void MENU_ConvertTexture( void* work_ptr );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void MENU_ClearTextTexture( void *work );



/////////////////////////////////////////////////////
typedef struct _WORK {	// ローカルなワークエリア宣言
	GV_ACT actor;
	void* 	pscrctrl;
	int		flag;
	u_int		ctr;

	DG_DMAPACK*	pdma;
	u_long128	buffer[ 8 ];

	int	width;

} WORK;

enum {
	FLAG_INIT			= 	0x01,
	FLAG_AFTER_DISCON	= 	0x02,
	FLAG_QUEUED			=	0x04,
};

static WORK* pWork = NULL;
static int nReq = 0;

////////
static int GetResources(WORK* pw)
{	// キャラの初期設定。
	pw->pscrctrl = NewTextScreenControlEx( VRAM_WIDTH, VRAM_HEIGHT, 255, 0x000b, 0/*bufferedTextFlag*/ );
	pw->flag = 0;
	pw->ctr = 0;
			
	pw->pdma = DG_MakeDmapack2( DG_DMAPACK_MENU | DG_DMAPACK_PRIVILEGE, DG_DMAPACK_PHASE_LAST , 254 );
#ifdef KP_XBOX //BP
	pw->pdma->autopacket = pw->buffer;
#endif
	DG_SetDmapackEnd( pw->buffer );

	return 0;	// リソース確保に失敗した場合は負の値を返す
}

static void Act(WORK* pw)
{	// 1フレームに一回呼ばれます

	extern int	GV_PadMask[ 4 * 2 ] ; 	/* マスク指定用 */
	int bDisp = FALSE;
	
	if( ( GV_PauseLevel & ~GV_PAUSE_PAUSE ) == 0
#if 0
		&& ( GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON
								 | MENU_STREAM_CH_0 | MENU_STREAM_CH_1 ) == 0 )
#else
		&& ( GM_CheckMenuStatus( MENU_WEAPON_OPEN | MENU_ITEM_OPEN | MENU_RADIO_ON ) == 0 )
#endif
		&& ( GM_GameStatus & ( STATE_DEMO | STATE_PAUSE_DISABLE
							   | STATE_PAD_DEMO | STATE_PRG_DEMO
							   | STATE_SCN_DEMO | STATE_GAMEOVER ) ) == 0
		&& ( DG_UnDrawFrameCount == 0 ) 
		&& ( GM_PauseRequest == 0 ) 
		&& ( GV_PadData[ 0 ].flag & GV_PAD_DISCONNECT )
		&& ( GM_DisablePauseCount <= 0 )
		&& !( GV_PadData[ 0 ].flag & GV_PAD_RELEASE )	// パッドリリース中はポーズできない
		// スタートがマスクされてる間はポーズできない
		&& !( (GV_PadData[ 0 ].flag & GV_PAD_MASK_PRG) && ( (GV_PadMask[ 0 ] & PAD_STA ) != PAD_STA ) )
		&& !( (GV_PadData[ 0 ].flag & GV_PAD_MASK_SCN) && ( (GV_PadMask[ 1 ] & PAD_STA ) != PAD_STA ) )
		) {
		if( ++pw->ctr > DIRECT_TICK( 30 ) ){
			// TCRでは 1秒以内にポーズしてメッセージ出せ、とのこと。
			// 0.5 秒間連続してパッドが抜けてたらポーズすることにする。
			GM_SdSet( SNG_PAUSEON ) ;
			GV_PauseOnActorSystem( GV_PAUSE_PAUSE ) ;
			pw->ctr = 0;
		}
	} else {
		pw->ctr = 0;
	}

	if( (GV_PauseLevel & GV_PAUSE_PAUSE) && (GV_PadData[ 0 ].flag & GV_PAD_DISCONNECT )) {
		if( !(pw->flag & FLAG_INIT) ) {
			u_long128* pbuf;
			char str[ 0x200 ];
#if 0			
			MENU_CreateTextTexture( pw->pscrctrl, 0, 0 , VRAM_WIDTH, FONT_SIZE_H, 0, 0, 0,
									"Please reconnect the controller");
			sprintf( str, "       to controller port %d", GV_GetXPadPort( 0 ) + 1 );
			MENU_CreateTextTexture( pw->pscrctrl, 0, FONT_SIZE_H, VRAM_WIDTH, FONT_SIZE_H * 2, 0, 0, 0, str );
			MENU_CreateTextTexture( pw->pscrctrl, 0, FONT_SIZE_H * 2 , VRAM_WIDTH, FONT_SIZE_H * 3, 0, 0, 0,
									" and press START to continue" );
#else
			sprintf( str, GM_GetResource( 5, 14 ), GV_GetXPadPort( 0 ) + 1 );
			MENU_ClearTextTexture( pw->pscrctrl );
			pw->width = MENU_CreateTextTextureNoConvert( pw->pscrctrl, 0, 0 , VRAM_WIDTH, VRAM_HEIGHT, 0, 0, 0,
														 str );
			MENU_ConvertTexture( pw->pscrctrl );
#endif			


			pbuf = DG_SetDmapackAlpha( pw->buffer, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ) );
			pbuf = DG_SetDmapackBox( pbuf, (float)DIRECT_SCREEN_X( CENTER_X - pw->width / 3 - 8), FRAME_Y,
									 (float)DIRECT_SCREEN_X( CENTER_X + pw->width / 3 + 8), FRAME_Y + FRAME_H,
									 DG_MakeDmaPackColorFromInt( 0x60000000 ) );
			pbuf = DG_SetDmapackEnd( pbuf );
			
			pw->flag |= FLAG_INIT;
		}
		if( (GV_Time % 90 < 50) || ( pw->flag & FLAG_AFTER_DISCON ) ) {
			MENU_PutTextScreen( pw->pscrctrl, CENTER_X - pw->width / 3, DISP_Y,
								CENTER_X + pw->width / 3, DISP_Y + DISP_H,
								0, 0, pw->width, VRAM_HEIGHT, 0xffffffff);

			bDisp = TRUE;
			if( GV_Time % 90 < 50 ) {
				pw->flag &= ~FLAG_AFTER_DISCON;
			}
		} 
	} else {
		pw->flag &= ~FLAG_INIT;
	}
	
	if( bDisp ) {
		if( !(pw->flag & FLAG_QUEUED) ) {
			DG_QueueDmapack( pw->pdma );
			pw->flag |= FLAG_QUEUED;
		}
	} else {
		if( pw->flag & FLAG_QUEUED ) {
			DG_DequeueDmapack( pw->pdma );
			pw->flag &= ~FLAG_QUEUED;
		}
	}
}

static void Die(WORK* pw)
{	// 死ぬときに呼ばれる関数。デストラクタにあたる
	GV_DestroyOtherActor( pw->pscrctrl );
	if( pw->flag & FLAG_QUEUED ) {
		DG_DequeueDmapack( pw->pdma );
	}
	DG_FreeDmapack( pw->pdma );
	pWork = NULL;
	nReq = 0;
}

/////////////////
void* GM_CreatePadDisconnect( void )
{
	WORK* pw;

	if( pWork == NULL ) {
		// アクター用メモリを確保
		if ((pw = (WORK *)GV_NewActor(GV_ACTOR_AFTER2, sizeof(WORK))) != NULL){
			// メモリが確保できれば、関数を設定する
			GV_SetActor( &(pw->actor ), Act, Die ) ;
			if ( GetResources( pw ) < 0 ) {	// リソースを確保する
				GV_DestroyActor( pw ) ;
				return NULL ;
			}
		}
		pWork = pw;
    }
	++nReq;
    return (void *)pWork ;
}

void GM_ReleasePadDisconnect( void )
{
	if( pWork != NULL && --nReq <= 0 ) GV_DestroyOtherActorQuick( pWork );
}
