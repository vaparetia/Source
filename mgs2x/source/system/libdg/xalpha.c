//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xalpha.cpp

	ＸＢＯＸ用アルファブレンディング関連関数

	2002/03/28 K.Takabe
	$Id: xalpha.c,v 1.4 2002/11/23 11:36:54 Yoshizawa1 Exp $
*/

#ifdef KP_XBOX //BP

#ifndef KP_XBOX
#include <d3dx8.h>
#else
#include <xtl.h>
#endif

#include <stdio.h>
#include "libgv.h"
#include "libdg.h"
#include "private.h"


/*
 * SCE_GS_SET_ALPHAについて
 *
 *  アルファブレンディングを行った場合の出力カラー Cv は
 *  以下の式で求めることが出来る。
 *
 *  Cv = (A - B) * C >> 7 + D
 *  ~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  この式における A, B, C, D は、SCE_GS_SET_ALPHA(a, b, c, d, fix) によって
 *  次のように定まる。
 *
 *  フレームバッファのカラーを Cd, アルファ値を Ad,
 *  描画ソースのカラーを       Cs, アルファ値を As,
 *  出力されるカラーを         Cv と置くと、
 *
 *  a == 0 : Cs, 1 : Cd,  2 : 0
 *  b == 0 : Cs, 1 : Cd,  2 : 0
 *  c == 0 : As, 1 : Ad,  2 : fix
 *  d == 0 : Cs, 1 : Cd,  2 : 0
 *
 *  fix  : c == 2 の時に参照されるアルファ値を設定する。
 *  0〜255の値を取り、128の時に 1.0 を表す。
 *
 *  ※ 16bit, 24bit カラーの場合、As は TEXAレジスタを参照して定められる。
 *
 * これに対し、DirectXでは
 * SRCBLEND, DSTBLEND の設定と
 * D3DBLENDOPの設定(ADD, SUBTRACT(src-dst), REVSUBSTRACT(dst-src))
 * を設定することで可能になる。
 * SRC に対する fix 値は ピクセルシェーダでなんとかなるが、
 * DST に対する fix 値は どうしようもない。
 *
 *  D3DBLEND_ZERO         : (0, 0, 0, 0) 
 *  D3DBLEND_ONE          : (1, 1, 1, 1)
 *  D3DBLEND_SRCCOLOR     : (Rs, Gs, Bs, As)
 *  D3DBLEND_INVSRCCOLOR  : (1-Rs, 1-Gs, 1-Bs, 1-As)
 *  D3DBLEND_SRCALPHA     : (As, As, As, As)
 *  D3DBLEND_INVSRCALPHA  : (1-As, 1-As, 1-As, 1-As)
 *  D3DBLEND_DESTALPHA    : (Ad, Ad, Ad, Ad)
 *  D3DBLEND_INVDESTALPHA : (1-Ad, 1-Ad, 1-Ad, 1-Ad)
 *  D3DBLEND_DESTCOLOR    : (Rd, Gd, Bd, Ad)
 *  D3DBLEND_INVDESTCOLOR : (1-Rd, 1-Gd, 1-Bd, 1-Ad)
 *  D3DBLEND_SRCALPHASAT  : (f, f, f, 1), f = min(As, 1-Ad)
 *
 *
 *      SCE_GS_SET_ALPHA(2, 0, 0, 1) ... ( 0 - Cs) *  As + Cd
 *                   Cd - a * Cs     = SRCALPHA, ONE, REVSUBSTRACT
 *      SCE_GS_SET_ALPHA(2, 2, 2, 1)      ... ( 0 -  0) * fix + Cd
 *                   Cd              = ZERO, ONE
 *      SCE_GS_SET_ALPHA(0, 1, 2, 1)      ... (Cs - Cd) * fix + Cd
 *                   無理
 *      SCE_GS_SET_ALPHA(1, 2, 2, 2)      ... (Cd -  0) * fix + 0
 *                   Cd              = ZERO, ONE
 *      SCE_GS_SET_ALPHA(1, 0, 2, 2)      ... (Cd - Cs) * fix + 0
 *                   Cd - Cs         = ONE, ONE, BLENDOP_REVSUBTRACT
 *      SCE_GS_SET_ALPHA(0, 1, 2, 2, 128) ... (Cs - Cd) * 128 + 0
 *                   Cs - Cd         = ONE, ONE, BLENDOP_SUBTRACT
 *      SCE_GS_SET_ALPHA(0, 1, 0, 1, fix)  ... (Cs - Cd) * As  + Cd
 *                 a Cs + (1 - a) Cd = SRCALPHA, INVSRCALPHA
 *      SCE_GS_SET_ALPHA(0, 2, 0, 1, fix)  ... (Cs -  0) * As  + Cd
 *                 a Cs + Cd         = SRCALPHA, ONE
 */

/* 省略形定義 */
#define ON			1
#define OFF			0
#define OP_ADD		D3DBLENDOP_ADD
#define OP_SUB		D3DBLENDOP_SUBTRACT
#define OP_RSUB		D3DBLENDOP_REVSUBTRACT
#define ONE			D3DBLEND_ONE
#define ZERO		D3DBLEND_ZERO
#define SRC			D3DBLEND_SRCALPHA
#define DST			D3DBLEND_DESTALPHA
#define INVSRC		D3DBLEND_INVSRCALPHA
#define INVDST		D3DBLEND_INVDESTALPHA
#define FIX			D3DBLEND_BLENDFACTOR
#define INVFIX		D3DBLEND_INVBLENDFACTOR
#define SRCCOL		D3DBLEND_SRCCOLOR
#define DSTCOL		D3DBLEND_DESTCOLOR
#define INVSRCCOL	D3DBLEND_INVSRCCOLOR
#define INVDSTCOL	D3DBLEND_INVDESTCOLOR

static inline void SetBlendIM( int mode, int op, int src, int dst )
{
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_ALPHABLENDENABLE, mode );
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_BLENDOP, op );
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_SRCBLEND, src );
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_DESTBLEND, dst );
}
#define SetBlend( _m, _o, _s, _d ) { state->mode = _m ; state->op = _o ; state->src = _s ; state->dst = _d ; }


typedef struct _alpha_state{
	int			mode, op, src, dst ;
	int			fix_flag, init_flag ;
} DG_ALPHA_STATE ;

static DG_ALPHA_STATE		DG_AlphaState[ 256 ];


/* アルファブレンディングモードを設定する。 */
/*
	SCE_GS_SET_ALPHAで正直に分岐する。
	※このままだと死ぬ程重いのでは…
	※Windows版のHALでは、D3DBLENDOP サポートありません。
	（ＰＳ２形式のビット並びを直で指定しているので注意！）
*/
void SetAlphaMode( DG_ALPHA_STATE *state, u_long64 type)
{
	int				alpha_fix_mode = 0 ;
	int				param ;

	state->init_flag = 1 ;
	param = (int)(type & SCE_GS_SET_ALPHA(3,3,3,3,0));
	switch (param) {
	  case SCE_GS_SET_ALPHA(0,0,0,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(0,0,1,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(0,0,2,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(1,1,0,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(1,1,1,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(1,1,2,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(2,2,0,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(2,2,1,1,0): /* DST */
	  case SCE_GS_SET_ALPHA(2,2,2,1,0): /* DST */
#if 0
		SetBlend( ON, OP_ADD, ZERO, ONE );
#else
		state->init_flag = 2 ;				/* カラーマスクを有効に */
		SetBlend( OFF, OP_ADD, ONE, ZERO );
#endif
		break ;
	  case SCE_GS_SET_ALPHA(0,0,0,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(0,0,1,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(0,0,2,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(1,1,0,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(1,1,1,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(1,1,2,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(2,2,0,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(2,2,1,0,0): /* SRC */
	  case SCE_GS_SET_ALPHA(2,2,2,0,0): /* SRC */
		SetBlend( OFF, OP_ADD, ONE, ZERO );
		break ;
	  case SCE_GS_SET_ALPHA(0,2,2,2,0): /* SRC * fix */
		//SetBlend( ON, OP_ADD, SRC, ZERO );
		SetBlend( ON, OP_ADD, FIX, ZERO );
		alpha_fix_mode = 1 ;
	  case SCE_GS_SET_ALPHA(1,2,2,2,0): /* DST * fix */
		//SetBlend( ON, OP_ADD, ZERO, SRC );
		SetBlend( ON, OP_ADD, ZERO, FIX );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(1,0,2,2,0): /* ( DST - SRC ) * fix */
		//SetBlend( ON, OP_RSUB, SRC, SRC );
		SetBlend( ON, OP_RSUB, FIX, FIX );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(0,1,2,2,0): /* ( SRC - DST ) * fix */
		//SetBlend( ON, OP_SUB, SRC, SRC );
		SetBlend( ON, OP_SUB, FIX, FIX );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(1,0,0,2,0): /* ( DST - SRC ) * alpha */
		SetBlend( ON, OP_RSUB, SRC, SRC );
		break;
	  case SCE_GS_SET_ALPHA(0,1,0,2,0): /* ( SRC - DST ) * alpha */
		SetBlend( ON, OP_SUB, SRC, SRC );
		break;
	  case SCE_GS_SET_ALPHA(0,2,0,2,0): /* SRC * alpha */
		SetBlend( ON, OP_ADD, SRC, ZERO );
		break;
	  case SCE_GS_SET_ALPHA(1,2,0,2,0): /* DST * alpha */
		SetBlend( ON, OP_ADD, ZERO, SRC );
		break;
	  case SCE_GS_SET_ALPHA(2,0,0,0,0): /* SRC * ( 1 - alpha ) */
		SetBlend( ON, OP_ADD, INVSRC, ZERO );
		break;
	  case SCE_GS_SET_ALPHA(2,0,1,0,0): /* SRC * ( 1 - dstalpha ) */
		SetBlend( ON, OP_ADD, INVDST, ZERO );
		break;
	  case SCE_GS_SET_ALPHA(2,0,2,0,0): /* SRC * ( 1 - fix ) */
		//SetBlend( ON, OP_ADD, INVSRC, ZERO );
		SetBlend( ON, OP_ADD, INVFIX, ZERO );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(2,1,0,1,0): /* DST * ( 1 - alpha ) */
		SetBlend( ON, OP_ADD, ZERO, INVSRC );
		break;
	  case SCE_GS_SET_ALPHA(2,1,1,1,0): /* DST * ( 1 - dstalpha ) */
		SetBlend( ON, OP_ADD, ZERO, INVDST );
		break;
	  case SCE_GS_SET_ALPHA(2,1,2,1,0): /* DST * ( 1 - fix ) */
		//SetBlend( ON, OP_ADD, ZERO, INVSRC );
		SetBlend( ON, OP_ADD, ZERO, INVFIX );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(0,1,0,1,0): /* 通常の半透明 */
		SetBlend( ON, OP_ADD, SRC, INVSRC );
		break;
	  case SCE_GS_SET_ALPHA(1,0,0,0,0): /* 通常の半透明 */
		SetBlend( ON, OP_ADD, INVSRC, SRC );
		break;
	  case SCE_GS_SET_ALPHA(0,1,1,1,0): /* フレームバッファアルファを使用した半透明 */
		SetBlend( ON, OP_ADD, DST, INVDST );
		break;
	  case SCE_GS_SET_ALPHA(1,0,1,0,0): /* フレームバッファアルファを使用した半透明 */
		SetBlend( ON, OP_ADD, INVDST, DST );
		break;
	  case SCE_GS_SET_ALPHA(0,1,2,1,0): /* 通常の半透明 SRC * fix + DST * ( 1 - fix ) */
		//SetBlend( ON, OP_ADD, SRC, INVSRC );
		SetBlend( ON, OP_ADD, FIX, INVFIX );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(1,0,2,0,0): /* 通常の半透明 SRC * ( 1 - fix ) + DST * fix */
		//SetBlend( ON, OP_ADD, INVSRC, SRC );
		SetBlend( ON, OP_ADD, INVFIX, FIX );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(0,2,0,1,0): /* 加算 SRC * alpha + DST */
		SetBlend( ON, OP_ADD, SRC, ONE );
		break;
	  case SCE_GS_SET_ALPHA(2,0,0,1,0): /* 減算 DST - alpha * SRC */
		SetBlend( ON, OP_RSUB, SRC, ONE );
		break;
	  case SCE_GS_SET_ALPHA(0,2,1,1,0): /* 加算 SRC * dstalpha + DST */
		SetBlend( ON, OP_ADD, DST, ONE );
		break;
	  case SCE_GS_SET_ALPHA(2,0,1,1,0): /* 減算 DST - dstalpha * SRC */
		SetBlend( ON, OP_RSUB, DST, ONE );
		break;
	  case SCE_GS_SET_ALPHA(0,2,2,1,0): /* 加算 SRC * fix + DST */
		//SetBlend( ON, OP_ADD, SRC, ONE );
		SetBlend( ON, OP_ADD, FIX, ONE );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(2,0,2,1,0): /* 減算 DST - fix * SRC */
		//SetBlend( ON, OP_RSUB, SRC, ONE );
		SetBlend( ON, OP_RSUB, FIX, ONE );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(1,2,0,0,0): /* 逆加算 DST * alpha + SRC */
		SetBlend( ON, OP_ADD, ONE, SRC );
		break;
	  case SCE_GS_SET_ALPHA(1,2,1,0,0): /* 逆加算 DST * dstalpha + SRC */
		SetBlend( ON, OP_ADD, ONE, DST );
		break;
	  case SCE_GS_SET_ALPHA(1,2,2,0,0): /* 逆加算 DST * fix + SRC */
		//SetBlend( ON, OP_ADD, ONE, SRC );
		SetBlend( ON, OP_ADD, ONE, FIX );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(2,1,0,0,0): /* 逆減算 SRC - DST * alpha */
		SetBlend( ON, OP_SUB, ONE, SRC );
		break;
	  case SCE_GS_SET_ALPHA(2,1,1,0,0): /* 逆減算 SRC - DST * dstalpha */
		SetBlend( ON, OP_SUB, ONE, DST );
		break;
	  case SCE_GS_SET_ALPHA(2,1,2,0,0): /* 逆減算 SRC - DST * fix */
		//SetBlend( ON, OP_SUB, ONE, SRC );
		SetBlend( ON, OP_SUB, ONE, FIX );
		alpha_fix_mode = 1 ;
		break;
	  case SCE_GS_SET_ALPHA(2,0,0,2,0): /* -SRC * alpha == 0 *//* 意味あるのか？ */
	  case SCE_GS_SET_ALPHA(2,1,0,2,0): /* -DST * alpha == 0 *//* 意味あるのか？ */
		SetBlend( ON, OP_ADD, ZERO, ZERO );
		break;
	  case SCE_GS_SET_ALPHA(1,2,0,1,0): /* DST * alpha + DST *//* テクスチャカラーが白なら再現可能 */
		SetBlend( ON, OP_ADD, DSTCOL, SRC );
		break;

	  case SCE_GS_SET_ALPHA(1,2,2,1,0): /* DST * fix + DST *//* 再現できない */
		SetBlend( ON, OP_ADD, ONE, ONE );
		state->init_flag = 0 ;
		break;
	  case SCE_GS_SET_ALPHA(0,1,2,0,0): /* ( SRC - DST ) * fix + SRC *//* 再現できない */
		SetBlend( ON, OP_ADD, ONE, ONE );
		state->init_flag = 0 ;
		break;
	  case SCE_GS_SET_ALPHA(0,2,0,0,0): /* SRC * alpha + SRC *//* 再現できない */
		SetBlend( ON, OP_ADD, ONE, ONE );
		state->init_flag = 0 ;
		break;
	  case SCE_GS_SET_ALPHA(1,0,0,1,0): /* ( DST - SRC ) * alpha + DST *//* 再現できない */
		SetBlend( ON, OP_ADD, ONE, ONE );
		state->init_flag = 0 ;
		break;
	  default:
		/* サポートしていないモードはここに来る */
		SetBlend( ON, OP_ADD, ONE, ONE );
		//printf("SCE_GS_SET_ALPHA : wrong type (%08lx)\n", type);
		state->init_flag = 0 ;
		break;
	}

	state->fix_flag = alpha_fix_mode ;

	return ;

}

/* アルファブレンディングのパラメータを作成する */
void DG_InitAlphaState( void )
{
	int			i ;

	for ( i = 0 ; i < 256 ; i++ ){
		SetAlphaMode( &DG_AlphaState[ i ], i );
	}
}

void DG_SetAlphaMode(u_long64 type)
{
	static int		reset_chanl_mask = 0 ;
	unsigned int	alpha = 0x80000000 ;
	int				alpha_fix_mode = 0 ;
	int				param ;
#ifdef WIRE_MODE
	//SetAlphaBlend(FALSE);
	SetBlendIM( OFF, OP_ADD, ONE, ZERO );
	return;
#endif
	/* まず0かどうかでαブレンディングのON/OFFを決める */
	if (type == 0) {
		//SetAlphaBlend(FALSE);
		SetBlendIM( OFF, OP_ADD, ONE, ZERO );
		/* カラーマスク復元チェック */
		if ( reset_chanl_mask ){
#ifndef KP_WINDOWS
			IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA );
#endif
			reset_chanl_mask = 0 ;
		}
		return;
	}
	//SetAlphaBlend(TRUE);

	/* 固定アルファチェック */
	switch ( ( type >> 4 ) & 3 ){
	  case 2:
		alpha = ( type >> 32 ) & 0xff ;
		break ;
	  default:
		alpha = 128 ;
		break ;
	}

	param = (int)(type & SCE_GS_SET_ALPHA(3,3,3,3,0));
	{
		DG_ALPHA_STATE	*state ;
		state = &DG_AlphaState[ param ] ;
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_ALPHATESTENABLE, state->mode );
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_ALPHABLENDENABLE, state->mode );
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_BLENDOP, state->op );
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_SRCBLEND, state->src );
		IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_DESTBLEND, state->dst );
		alpha_fix_mode = state->fix_flag ;
		if ( state->init_flag == 2 ){
			/* カラーマスク有効化 */
#ifndef KP_WINDOWS
			IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA );
#endif
			reset_chanl_mask = 1 ;
		} else {
			/* カラーマスク復元チェック */
			if ( reset_chanl_mask ){
#ifndef KP_WINDOWS
				IDirect3DDevice9_SetRenderState( g_pd3dDevice, D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_ALPHA );
#endif
				reset_chanl_mask = 0 ;
			}
		}
#ifdef DEBUG_MODE
		if ( state->init_flag == 0 ){
			printf("no support alpha mode %08x\n", type );
		}
#endif
	}

	if ( alpha_fix_mode ){
		alpha <<= 1 ;
		if ( alpha > 255 ) alpha = 255 ;
#ifndef KP_WINDOWS


		IDirect3DDevice9_SetRenderState( g_pd3dDevice/*BP: 0*/, D3DRS_BLENDFACTOR, alpha << 24 );
#endif
	}

}

#endif
