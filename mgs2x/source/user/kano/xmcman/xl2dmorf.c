/*
  L2D の部品を制御(L2Dシステム内でやるべき。。。)

  2002/05/08 M.Kobayashi

  $Id: xl2dmorf.c,v 1.1.1.3 2002/11/19 11:43:47 Yoshizawa1 Exp $
 */

#include "gameheader.h"
#include "xmcman.h"
#include "private.h"	
#include	"../../kira/2D_action/layout_2d.h"

static MCX_L2D_MORF* pTop = NULL;

void MCX_L2DMorfCreate(MCX_L2D_MORF* pmorf, int l2d_handle, int codeobj )
{	// クラスを初期化
	pmorf->codeobj = codeobj;
	pmorf->codestart = 0;
	pmorf->codeend = 0;
	pmorf->l2d_handle = l2d_handle;
			
	// 自動補間の設定
	pmorf->intrp_rate.fCurrent = 0.f;
	pmorf->intrp_rate.target = 0;
	pmorf->intrp_rate.ctr = 0;
	MCX_AddInterpService( &pmorf->intrp_rate );
	
	// リストに登録
	pmorf->pNext = pTop;
	pTop = pmorf;
}

void MCX_L2DMorfDelete(MCX_L2D_MORF* pmorf )
{
	// 補間サービス終了
	MCX_DeleteInterp( &pmorf->intrp_rate );

	// リストから削除
	if( pTop == pmorf ) {
		pTop = pmorf->pNext;
	} else {
		MCX_L2D_MORF* pmorfcur;
		for( pmorfcur = pTop; pmorfcur->pNext != NULL ; pmorfcur = pmorfcur->pNext ) {
			if( pmorfcur->pNext == pmorf ) {
				pmorfcur->pNext = pmorf->pNext;
				break;
			}
		}
	}
}

void MCX_L2DMorfUpdate( void )
{	// 画面に反映
	MCX_L2D_MORF* pm;
	for( pm = pTop; pm != NULL ; pm = pm->pNext ) {
		if( pm->codestart != 0 && pm->l2d_handle >= 0 ) {
			L2D_MorfObject( L2D_GetParts( pm->l2d_handle, pm->codeobj ), pm->codestart, pm->codeend,
							pm->intrp_rate.fCurrent );
//			if( pm->intrp_rate.ctr == 0 ) pm->codestart = pm->codeend = 0;
		}
	}
}


void MCX_L2DMorfSet( MCX_L2D_MORF* pmorf, int codestart, int codeend, int ctr )
{	// ctr かけてモーフィング
	pmorf->codestart = codestart;
	pmorf->codeend = codeend;
	pmorf->intrp_rate.fCurrent = 0.f;
	pmorf->intrp_rate.target = 1;
	pmorf->intrp_rate.ctr = ctr;
	// イニシャライズしておく
	if( pmorf->l2d_handle >= 0 ){
		L2D_MorfObject( L2D_GetParts( pmorf->l2d_handle, pmorf->codeobj ), pmorf->codestart, pmorf->codestart,
						1.f );
	}
}

void MCX_L2DMorfExit( void )
{	// システムの終了・現在登録中のものを全て消す
	while( pTop != NULL ) {
		MCX_L2DMorfDelete( pTop );
	}
}
