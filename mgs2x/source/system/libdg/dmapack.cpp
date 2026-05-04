/*
	dmapack.c
	ＤＭＡパケット接続型オブジェクト管理ルーチン

	2000/02/29 K.Takabe
	$Id: dmapack.cpp,v 1.1.1.3 2002/11/19 11:42:07 Yoshizawa1 Exp $

*/

#if 0

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#else

#ifdef KP_XBOX
#include <xtl.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "libgv.h"
#include "libdg.h"

//#include	"gameheader.h"

#include "dmapack.h"

//#include "nisi.h"


#endif


/* ---------------------------------------------------------------- */
	/*
		プラグイン固有設定
	*/

/* メインチャンネルプラグイン用設定 */
	/* プラグイン固有ＩＤ */
#define PLUGIN_ID		(4827819)	/* "dmapack" */
	/* プラグイン初期化フラグ */
#define PLUGIN_FLAG		(DG_PLUGIN_FLAG_OBJBUFFER|DG_PLUGIN_FLAG_ENABLE|DG_PLUGIN_FLAG_ENABLEMENU)
	/* プラグイン実行フェーズ指定 */
#define PLUGIN_PHASE	(DG_PLUGIN_PHASE_FIRST|DG_PLUGIN_PHASE_NORMAL|DG_PLUGIN_PHASE_AFTER|DG_PLUGIN_PHASE_LAST)
	/* プラグインプライオリティ */
#define PLUGIN_PRIO		(DG_PLUGIN_PRIO_NORMAL)
	/* 最大使用オブジェクトキューサイズ */
#define PLUGIN_USE_QUEUE	(64)



/* ---------------------------------------------------------------- */
	/*
		プラグイン固有変数
	*/

	/* プラグイン初期化フラグ */
static int	PluginStartFlag = 0 ;
static DG_PLUGIN	DmapackPlugin ;

/* ---------------------------------------------------------------- */
	/*
		プラグイン内部使用関数
	*/

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status );

/* ---------------------------------------------------------------- */
	/*
		プラグイン起動
	*/
	
void PluginTest(void)
//void DG_AddPluginDmapack( void )
{
	if ( PluginStartFlag != 0 ) return ;

	/* プラグイン作成 */
	DG_MakePlugin( &DmapackPlugin, PLUGIN_ID, PLUGIN_FLAG, PLUGIN_PHASE, PLUGIN_PRIO, PluginActor, PLUGIN_USE_QUEUE );
	DG_AddPlugin( &DmapackPlugin );

	PluginStartFlag = 1 ;
	DG_PrivilegeMode = 0 ;
}

	/*
		プラグイン終了
	*/
void DG_DeletePluginDmapack( void )
{
	while ( PluginStartFlag != 0 ){
		DG_DeletePlugin( &DmapackPlugin );
		DG_FreePlugin( &DmapackPlugin );
	}
	PluginStartFlag = 0 ;
}

/* ---------------------------------------------------------------- */
	/*
		オブジェクト作成
	*/
DG_DMAPACK* DG_MakeDmapack2( int flag, int phase, int priority )
{
	DG_DMAPACK	*dmapack ;

	flag |= DG_DMAPACK_INVISIBLE2 | DG_DMAPACK_INVISIBLE3 ;
	dmapack = (DG_DMAPACK*)GV_Malloc( sizeof(DG_DMAPACK) );
	GV_ZeroMemory( dmapack, sizeof(DG_DMAPACK) );

	dmapack->flag = flag ;
	dmapack->phase = phase ;
	dmapack->priority = priority ;

	return ( dmapack );
}

DG_DMAPACK* DG_MakeDmapack( int flag, int phase )
{
	return DG_MakeDmapack2( flag, phase, 128 );
}

	/*
		オブジェクト開放
	*/
void DG_FreeDmapack( DG_DMAPACK *dmapack )
{
	GV_Free( dmapack );
}

	/*
		オブジェクト登録
	*/
int DG_QueueDmapack( DG_DMAPACK *dmapack )
{
	if ( PluginStartFlag == 0 ){
		return (-1);
	}
	//DG_QueueUserObject( DmapackPlugin.obj_buffer, dmapack );

	{
		int		n, i ;
		DG_OBJ_BUFFER	*obj_buff = DmapackPlugin.obj_buffer ;
		DG_DMAPACK	**objs ;

		if ( dmapack == NULL ) return (-1);

		/*
			プライオリティを考慮し、登録順番をくずさないようにキューに追加する
		*/
		if ( obj_buff->n_queue >= obj_buff->max_queue ){
         //printf("DG_QueueDmapack: queue buffer over!!\n");
			//return ;
			return -1 ;
		}
		//..キューに追加核心
		objs = (DG_DMAPACK**)&obj_buff->queue[ obj_buff->n_queue - 1 ] ;
		for ( n = obj_buff->n_queue - 1 ; n >= 0 ; objs--, n-- ){
			if ( (*objs)->priority > dmapack->priority ){
				objs[1] = objs[0] ;
			} else {
				break ;
			}
		}
		objs++ ;
		*objs = dmapack ;
		obj_buff->n_queue++ ;
#if 0
		for ( n = 0 ; n < 16 ; n++ ){
			printf("%08x ", obj_buff->queue[n] );
		}
		printf(" (%d)\n", obj_buff->n_queue);
#endif
	}
	return ( 0 );
}

	/*
		オブジェクト削除
	*/
void DG_DequeueDmapack( DG_DMAPACK *dmapack )
{
	if ( PluginStartFlag == 0 ){
		return ;
	}
	DG_DequeueUserObject( DmapackPlugin.obj_buffer, dmapack );
}

/* ---------------------------------------------------------------- */
	/*
		プラグイン実行アクター
		暫定版
	*/


struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw; // The transformed position for the vertex
    DWORD color;        // The vertex color
    float tu,tv;
};
//extern CUSTOMVERTEX VertTest[];
extern CUSTOMVERTEX *VertTest;
extern IDirect3DBaseTexture8* pTexture;
extern DrawTest_cnt;

extern LPDIRECT3DVERTEXBUFFER8  vb;

static void PluginActor( DG_CHANL *cp, int which, DG_OBJ_BUFFER *obj_buff, int status )
{
#if 0 ///	
	if(pTexture){
	    //..メニュー
	    if(DrawTest_cnt>0){
			g_pd3dDevice->SetTexture(0,pTexture);
			g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			
			//..普通の半透明
			g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
			g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
/*
#if 0
			//..アルファ値
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
			//g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
#else
			//..テクスチャの色と頂点の色をアルファ値として使う
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			g_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
#endif
			//..加算半透明
			//g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
			//g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
*/			
			g_pd3dDevice->DrawPrimitiveUP( 
		    	D3DPT_TRIANGLELIST, DrawTest_cnt*2,VertTest, sizeof(CUSTOMVERTEX)  );
		}
	}

#if 0
	//..頂点バッファ描画テスト
	g_pd3dDevice->SetStreamSource( 0, vb, sizeof(CUSTOMVERTEX) );
	g_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
#endif


#ifdef __PS2__
	DG_DMAPACK	*dmapack, **que ;
	int			i, size, exec_flag = 0 ;
	int			invisible_flag, chanl_flag ;

	DG_DMATAG	*tag ;

	MARK( "dmapack.c" );
	if ( ( i = obj_buff->n_queue ) == 0 ) return ;

	/* 非表示チェック用フラグ設定 */
	invisible_flag = DG_DMAPACK_INVISIBLE0 << cp->chanl_num ;
	chanl_flag = ( cp->chanl_num != 4 ) ? DG_DMAPACK_NORMAL : DG_DMAPACK_MENU ;


	que = obj_buff->queue ;
	for ( ; i > 0 ; i-- ){
		dmapack = (*que++) ;
		if ( !( dmapack->flag & chanl_flag ) ) continue ;
		if ( dmapack->flag & invisible_flag ) continue ;
		if ( DG_PrivilegeMode && !( dmapack->flag & DG_DMAPACK_PRIVILEGE ) ) continue ;
		if ( !( dmapack->phase & status ) ) continue ;
		if ( exec_flag == 0 ){
			/* ＤＭＡバッファオープン(VIF1) */
			DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );
			exec_flag = 1 ;
		}

		/* ＤＭＡ接続処理 */
		tag = (DG_DMATAG*)DG_CurrentDmaAddr ;
		tag->qwc = DMATAG_SET_QWC( DMATAG_ID_CALL, 0 ) ;
		tag->addr = dmapack->packet[ which ] ;
		tag->vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 ) ;
		tag->vifcode[1] = SCE_VIF1_SET_NOP( 0 ) ;
		DG_CurrentDmaAddr++ ;
	}

	if ( exec_flag != 0 ){
		size = DG_WriteObjsPacketEnd( DG_CurrentDmaAddr );
		DG_CurrentDmaAddr += size ;

		/* ＤＭＡバッファクローズ */
		DG_CloseDmaTask();
	}
#endif

#endif ///
}




/* ---------------------------------------------------------------- */

