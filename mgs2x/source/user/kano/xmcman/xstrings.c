/*
  ＭＣ内GUI文字列クラス
  2002/04/11 M.Kobayahi

  $Id: xstrings.c,v 1.1.1.3 2002/11/19 11:43:49 Yoshizawa1 Exp $

 */

#include "gameheader.h"
#include "xmcman.h"
#include "private.h"	

#define	STR_WIDTH			(18) // strcode ではなく文字の長さ
#define STR_HEIGHT			(14) // strcode ではなく文字の長さ

#define FONT_WIDTH			(18.0f-2.0f)
#define FONT_HEIGHT			(14.0f-2.0f)

extern void SK_PrintfNormal2( u_char *ascci , SPR_OBJ **pObj , int u , int v , int str_width , int str_height , int num );
extern void SK_PrintfChengColor2( SPR_OBJ **pObj , u_char r , u_char g , u_char b , u_char a , int num ); // 一行いっきに書き換え
extern void SK_AllShow( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え
extern void SK_AllHide( SPR_OBJ **pObj , int num ); // 一行いっきに書き換え

static SPR_OBJ** ppObjTop;
static int objbuf_len;
static int thandle;
static int tstrcode;
static int cur_objbuf;
static MCX_STRINGS* pStrTop = NULL;
static int base_u;
static int base_v;

void MCX_StringsInitBuffer( SPR_OBJ** ppObj, int len,
							int	tex_handle,
							int tex_strcode )
{	// 使用するプールなどを決定
	ppObjTop = ppObj;
	objbuf_len = len;
	thandle = tex_handle;
	tstrcode = tex_strcode;
	cur_objbuf = 0;
}


void MCX_StringsCreate(MCX_STRINGS* pStrings, char* str, short x, short y, int col, u_char nofocus_a )
{	// クラスを初期化
	int i;
	SPR_OBJ** ppObj = ppObjTop + cur_objbuf;
	pStrings->ppObj = ppObj;
	pStrings->len = strlen( str );
	pStrings->col = col;
	pStrings->nofocus_a = nofocus_a;
	pStrings->x = x;
	pStrings->y = y;
	pStrings->str = str;
	for( i = 0 ; i < pStrings->len; i++, ppObj++, cur_objbuf++ ) {
		if( cur_objbuf >= objbuf_len ) HANGUP();
		*ppObj = SPR_Create_2D_Object( SP_SPRITE, DG_CHANL_MENU, NULL );
		SPR_ObjSetTexture( *ppObj, tstrcode, thandle );
		SPR_SetPriority( *ppObj, 7 );
		(*ppObj)->head.flags |= SPR_FLAG_PRIV | SPR_FLAG_ALPHA;
		(*ppObj)->head.alpha = SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0 ); // Cs As + ( 1 - Cd ) As
		(*ppObj)->sprite.pos.x = x;
		(*ppObj)->sprite.pos.y = y;
		(*ppObj)->sprite.dw = FONT_WIDTH;
		(*ppObj)->sprite.dh = FONT_HEIGHT;
	}
	ppObj = pStrings->ppObj;
	base_u = (int)( (*ppObj)->sprite.head.tex.u * 16.f );
	base_v = (int)( (*ppObj)->sprite.head.tex.v * 16.f );
	
	SK_PrintfNormal2( NULL, ppObj,
					 base_u, base_v,
					 STR_WIDTH, STR_HEIGHT, pStrings->len );
	SK_PrintfNormal2( str, ppObj,
					 base_u, base_v,
					 STR_WIDTH, STR_HEIGHT, pStrings->len );
	SK_PrintfChengColor2( ppObj, col & 0xff, ( col >> 8 ) & 0xff, ( col >> 16 ) & 0xff , 0,
						  pStrings->len );
	SK_AllShow( ppObj, pStrings->len );

	// 自動補間の設定
	pStrings->intrp_alpha.fCurrent = 0.f;
	pStrings->intrp_alpha.target = 0;
	pStrings->intrp_alpha.ctr = 0;

	MCX_AddInterpService( &pStrings->intrp_alpha );

	// リストに登録
	pStrings->pNext = pStrTop;
	pStrTop = pStrings;
}

void MCX_StringsPrintf(MCX_STRINGS* pStrings, const char* fmt, ... )
{	// 既存のスプライト文字列を変える
#define BUF_MAX 256
	char buf[ BUF_MAX ];
	int len;
	int i;
	va_list ap;
	va_start( ap, fmt );
	_vsnprintf( buf, BUF_MAX, fmt, ap );
	len = strlen( buf );
	if( len > pStrings->len ) {
		printf("xstrings: too long strings\n");
		return;
	}
	// 高さ、幅をここであわせておく
	for( i = 0 ; i < pStrings->len ; i++ ) {
		(*(pStrings->ppObj + i))->sprite.dh = (*pStrings->ppObj)->sprite.dh;
		(*(pStrings->ppObj + i))->sprite.dw = (*pStrings->ppObj)->sprite.dw;
	}
	SK_AllHide( pStrings->ppObj, pStrings->len );
	SK_AllShow( pStrings->ppObj, len );
	SK_PrintfNormal2( buf, pStrings->ppObj, 
					 base_u, base_v, 
					 STR_WIDTH, STR_HEIGHT, pStrings->len );
}

void MCX_StringsMove( MCX_STRINGS* pStrings, int n, int x )
{	// 既に作成された文字列に対して
	// n 番目以降のスプライトの座標をあわせる

	int i;
	int diff = (int)( x - (*(pStrings->ppObj + n))->sprite.pos.x);
	for( i = n; i < pStrings->len; i ++ ) {
		(*(pStrings->ppObj + i))->sprite.pos.x += diff;
	}
}


void MCX_StringsDelete(MCX_STRINGS* pStrings)
{	// オブジェクトを消去
	int i;
	SPR_OBJ** ppObj = pStrings->ppObj;
	for( i = 0 ; i < pStrings->len; i++, ppObj++ ) {
		SPR_Destroy_2D_Object( *ppObj );
	}
	MCX_DeleteInterp( &pStrings->intrp_alpha );

	// リストから削除
	if( pStrTop == pStrings ) {
		pStrTop = pStrings->pNext;
	} else {
		MCX_STRINGS* pst;
		for( pst = pStrTop; pst->pNext != NULL ; pst = pst->pNext ) {
			if( pst->pNext == pStrings ) {
				pst->pNext = pStrings->pNext;
				break;
			}
		}
	}
}

void MCX_StringsUpdate( void ) {
	MCX_STRINGS* pst;
	for( pst = pStrTop; pst != NULL ; pst = pst->pNext ) {
		SK_PrintfChengColor2( pst->ppObj, pst->col & 0xff,  ( pst->col >> 8 ) & 0xff,
							  ( pst->col >> 16 ) & 0xff , (u_char)pst->intrp_alpha.fCurrent,
							  pst->len );
	}
}


void MCX_StringsShow( MCX_STRINGS* pStrings, int ctr )
{	// ctr かけて見えるようにする
	if( pStrings->intrp_alpha.ctr != 0 ) {
		if( pStrings->intrp_alpha.target == 0 ) {	// 消えている途中だった
#if 0	
			ctr = (int)(ctr * pStrings->intrp_alpha.fCurrent / (( pStrings->col >> 24 ) & 0xff ));
#endif	
		} else if( pStrings->intrp_alpha.target != pStrings->nofocus_a ) {	
			return;
		}
	}
	pStrings->intrp_alpha.ctr = ctr;
	pStrings->intrp_alpha.target = ( pStrings->col >> 24 ) & 0xff;
}

void MCX_StringsHide( MCX_STRINGS* pStrings, int ctr )
{	// ctr かけて見えないようにする
	if( pStrings->intrp_alpha.ctr != 0 ) {
		if( pStrings->intrp_alpha.target == 0 ) {	// 消えている途中だった
			return;
		} else {
#if 0	
			int alpha = (( pStrings->col >> 24 ) & 0xff );
			ctr = (int)(ctr * ( alpha - pStrings->intrp_alpha.fCurrent ) / alpha);
#endif	
		}
	}
	pStrings->intrp_alpha.ctr = ctr;
	pStrings->intrp_alpha.target = 0;
}

void MCX_StringsNofocus( MCX_STRINGS* pStrings, int ctr )
{	// ctr かけてNofocusアルファにする
	if( pStrings->intrp_alpha.ctr != 0 ) {
		if( pStrings->intrp_alpha.target == (short) pStrings->nofocus_a ) {	// 途中だった
			return;
		} 
	}
	pStrings->intrp_alpha.ctr = ctr;
	pStrings->intrp_alpha.target = (short) pStrings->nofocus_a;
}
