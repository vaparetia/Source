/*

  ＭＣ内文字列クラス（MENU_系)
  2002/04/18 M.Kobayashi
  $Id: xmenustr.c,v 1.1.1.3 2002/11/19 11:43:48 Yoshizawa1 Exp $
  
 */
#include "gameheader.h"
#include "xmcman.h"
#include "private.h"	

#include "mode/menu/xtextscn.h"

static MCX_MENU_STRINGS* pstrTop = NULL;
static void* pFontMan;
static BOOL bDirty = FALSE;

#define NOFOCUS_A	0x30

extern void MENU_ClearTextTexture( void *work );
extern void MENU_PutTextScreen( void *work, int x1, int y1, int x2, int y2, int u1, int v1, int u2, int v2, int col );
extern void MENU_ClearPartTextTexture( void *work_ptr, int start_line, int height );
extern void MENU_ConvertTexture( void *work_ptr );

void MCX_MStringsInit( void* pfm )
{
	pFontMan = pfm;
	if( pfm != NULL ) MENU_ClearTextTexture( pfm );
}

void MCX_MStringsCreate( MCX_MENU_STRINGS* pstr,
						 char* pRes,
						 short x1, short y1,
						 short w, short h,
						 short u1, short v1,
						 short uw, short vh,
						 u_int	col )
{	// MENU_系パラメータ設定＆リスト登録
	if( pFontMan == NULL ) return;
	pstr->pRes = pRes;
	pstr->x1 = x1; pstr->y1 = y1;
	pstr->w = w; pstr->h = h;
	pstr->u1 = u1; pstr->v1 = v1;
	pstr->uw = uw; pstr->vh = vh;
	pstr->col = col;
	pstr->intrp_alpha.ctr = 0;
	pstr->intrp_alpha.target = 0;
	pstr->intrp_alpha.fCurrent = 0.f;
	pstr->pNext = pstrTop;
	pstrTop = pstr;
	MCX_AddInterpService( &pstr->intrp_alpha );
	// 実際に文字を作る
	MENU_CreateTextTextureNoConvert( pFontMan, u1, v1, u1+uw, v1+vh, 0, 0, 0, pRes );
	bDirty = TRUE;
}

void MCX_MStringsDelete( MCX_MENU_STRINGS* pstr )
{
	if( pFontMan == NULL ) return;
	// 補間サービス終了
	MCX_DeleteInterp( &pstr->intrp_alpha );

	// リストから削除
	if( pstrTop == pstr ) {
		pstrTop = pstr->pNext;
	} else {
		MCX_MENU_STRINGS* ps;
		for( ps = pstrTop; ps->pNext != NULL ; ps = ps->pNext ) {
			if( ps->pNext == pstr ) {
				ps->pNext = pstr->pNext;
				break;
			}
		}
	}
}

void MCX_MStringsUpdate( void )
{	// 必要な文字を画面に表示する
	MCX_MENU_STRINGS* pstr;
	if( pFontMan == NULL ) return;
	if( bDirty ) {
		MENU_ConvertTexture( pFontMan );
		bDirty = FALSE;
	}
	for( pstr = pstrTop ; pstr != NULL; pstr = pstr->pNext ) {
		if( pstr->intrp_alpha.fCurrent != 0.f ) {
			MENU_PutTextScreen( pFontMan, pstr->x1, pstr->y1, pstr->x1 + pstr->w, pstr->y1 + pstr->h,
								pstr->u1, pstr->v1, pstr->u1 + pstr->uw, pstr->v1 + pstr->vh,
								(pstr->col & 0xffffff) | ((int)pstr->intrp_alpha.fCurrent << 24) );
		}						
	}
}

void MCX_MStringsShow( MCX_MENU_STRINGS* pstr, int ctr )
{
	if( pFontMan == NULL ) return;
	pstr->intrp_alpha.ctr = ctr;
	pstr->intrp_alpha.target = pstr->col >> 24;
}

void MCX_MStringsHide( MCX_MENU_STRINGS* pstr, int ctr )
{
	if( pFontMan == NULL ) return;
	pstr->intrp_alpha.ctr = ctr;
	pstr->intrp_alpha.target = 0;
}

void MCX_MStringsNofocus( MCX_MENU_STRINGS* pstr, int ctr )
{
	if( pFontMan == NULL ) return;
	pstr->intrp_alpha.ctr = ctr;
	pstr->intrp_alpha.target = NOFOCUS_A;
}

void MCX_MStringsPrintf(MCX_MENU_STRINGS* pms, const char* fmt, ... )
{	// 既存のスプライト文字列を変える
#define BUF_MAX 256
	char buf[ BUF_MAX ];
	va_list ap;

	if( pFontMan == NULL ) return;
	va_start( ap, fmt );
	_vsnprintf( buf, BUF_MAX, fmt, ap );

	MENU_CreateTextTextureNoConvert( pFontMan, pms->u1, pms->v1, pms->u1 + pms->uw, pms->v1 + pms->vh, 0, 0, 0, buf );
	bDirty = TRUE;
}

void MCX_MStringsClearLine( MCX_MENU_STRINGS* pms )
{	// テクスチャがある場所のラインを消す
	if( pFontMan == NULL ) return;
	MENU_ClearPartTextTexture( pFontMan, pms->v1, pms->vh );
}
