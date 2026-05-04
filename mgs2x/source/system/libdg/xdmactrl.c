//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	xdmactrl.c
	ＸＢＯＸ用プッシュバッファ制御ルーチン

	2002/05/02 K.Takabe
	$Id: xdmactrl.c,v 1.4 2002/11/23 11:36:55 Yoshizawa1 Exp $

*/
/*
	ＰＳ２ＤＭＡ制御ルーチン
	　一つのＤＭＡ転送をＤＭＡタスクとして管理し、複数のＤＭＡタスクを
	　一度に発行することができるようにしたもの


	void		DG_DmaReset()

		パケット転送用ＤＭＡのリセット


	void DG_DmaReset( void );

		ＤＭＡ（ＶＩＦ１、ＧＩＦ）及びワークの初期化


	void DG_DmaClear( int which );
	int		which ;		ダブルバッファ選択

		ＤＭＡデータ構築用のワークを初期化


	void DG_DmaStart( int which );
	int		which ;		ダブルバッファ選択

		構築したＤＭＡパケットデータの転送開始


	int DG_DmaCheckEnd( void );

		転送したＤＭＡがすべて終了したかチェック


	void *DG_OpenDmaTask( int type, void *addr, int size );
	int		type ;		ＤＭＡタイプ(DG_OPEN_DMA_VIF1,DG_OPEN_DMA_GIF,DG_OPEN_DMA_STORE)
	void	*addr ;		DG_OPEN_DMA_STOREタイプを指定した場合のイメージストアアドレス
	int		size ;		DG_OPEN_DMA_STOREタイプを指定した場合のストアサイズ（qword単位）

		DG_DmaClear()関数で指定したバッファ上に新規ＤＭＡタスクとして
		ＤＭＡパケットを登録する


	void DG_CloseDmaTask( void );

		DG_OpenDmaTask()で開いたＤＭＡパケットに終端コードを入れ閉じる


*/

#ifdef KP_XBOX //BP

#ifdef KP_XBOX
#include <xtl.h>
//BP_RENDER #include <xgraphics.h>
#else
#include <d3dx8.h>
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"libgv.h"
#include	"libgv.cnf"
//#include	"break.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"


/* 確保するＤＭＡタスク数 */
#define DG_MAX_DMATASK		(64)


	/*
		個々のＤＭＡタスク情報
	*/
typedef struct {
	D3DPushBuffer	push_buffer ;	/* IDirect3DPushBuffer8 */
	short			dma_type ;		/* ＤＭＡの種類（0:VIF,1:GIF） */
	u_short			mark ;			/* デバッグ用マーク値 */
	void			*dma_ptr ;		/* 転送ＤＭＡのポインタ */
	void			*store_addr ;	/* ローカル＞ホスト転送の転送先 */
	int				store_size ;	/* ローカル＞ホスト転送の転送サイズ（qword単位） */
#ifdef DEBUG_MODE
	char			*fname ;
	void			*last_addr ;
	int				pad[2] ;
#endif
} DG_DMATASK;

	/*
		１フレーム分のＤＭＡタスク管理ワーク
	*/
typedef struct {
	int			n_task ;						/* 登録タスク数 */
	int			end_task ;						/* 実行終了タスク数 */
	int			terminate_flag ;				/* 終結済みフラグ */
	DG_DMATASK	task_list[ DG_MAX_DMATASK ] ;	/* 登録タスク */
} DG_DMATASK_SCHEDULE ;

	/*
		ＤＭＡパケット用メモリ
	*/

	/*
		パケットＤＭＡ転送管理タスク
	*/
static DG_DMATASK_SCHEDULE	DG_DmaTaskSchedule[ 2 ];	/* ダブルバッファで取る */
static DG_DMATASK_SCHEDULE	*CurrentSchedule ;			/* 作業用ワーク */
static DG_DMATASK_SCHEDULE	*ExecSchedule ;				/* 実行中ワーク */

static D3DPushBuffer	*PushBuffer ;	/* 現在使用中のプッシュバッファ */

	/*
		内部使用ワーク
	*/



/*----------------------------------------------------------------*/
#define PUSHBUFFER_SIZE		( 2 * 1024 * 1024 )
#define BUFFER_MARGIN		( 128 )

/*----------------------------------------------------------------*/
extern int		DG_MaxVertexBufferSize ;
extern void		*DG_VertexBuffer[2] ;


/*----------------------------------------------------------------*/

	/*
		パケット転送ＤＭＡの初期化
	*/
void DG_DmaReset( void )
{
	/*
		ワークなどの初期化
	*/
	//DG_DmaBuffer[0] = PACK_ADDR0 ;
	//DG_DmaBuffer[1] = PACK_ADDR1 ;
	//DG_DmaBufferEnd[0] = (u_long128*)( (int)DG_DmaBuffer[0] + PACK_SIZE ) ;
	//DG_DmaBufferEnd[1] = (u_long128*)( (int)DG_DmaBuffer[1] + PACK_SIZE ) ;

	//DG_DmaBuffer[0] = (void*)( (char*)DG_VertexBuffer[0] + DG_MaxVertexBufferSize );
	//DG_DmaBuffer[1] = (void*)( (char*)DG_VertexBuffer[1] + DG_MaxVertexBufferSize );
	//DG_DmaBufferEnd[0] = (u_long128*)( (char*)DG_DmaBuffer[0] + 3 * 1024 * 1024 ) ;
	//DG_DmaBufferEnd[1] = (u_long128*)( (char*)DG_DmaBuffer[1] + 3 * 1024 * 1024 ) ;
	//printf("%p %p %p %p\n", DG_DmaBuffer[0], DG_DmaBufferEnd[0], DG_DmaBuffer[1], DG_DmaBufferEnd[1] );

	DG_DmaTaskSchedule[ 0 ].n_task = 0 ;
	DG_DmaTaskSchedule[ 0 ].end_task = 0 ;
	DG_DmaTaskSchedule[ 0 ].terminate_flag = 0 ;
	DG_DmaTaskSchedule[ 1 ].n_task = 0 ;
	DG_DmaTaskSchedule[ 1 ].end_task = 0 ;
	DG_DmaTaskSchedule[ 1 ].terminate_flag = 0 ;
	CurrentSchedule = &DG_DmaTaskSchedule[ 0 ] ;
	ExecSchedule = &DG_DmaTaskSchedule[ 1 ] ;
}

	/*
		ＤＭＡ転送管理タスクの初期化
	*/
void DG_DmaClear( int which )
{
	CurrentSchedule = &DG_DmaTaskSchedule[ which ] ;
	CurrentSchedule->n_task = 0 ;
	CurrentSchedule->end_task = 0 ;
	CurrentSchedule->terminate_flag = 0 ;
}

	/*
		ＤＭＡ転送管理タスクのスタート
	*/
void DG_DmaStart( int which )
{
	DG_DMATASK			*task ;
	int					i ;

	ExecSchedule = &DG_DmaTaskSchedule[ which ];

	ExecSchedule->end_task = 0 ;

	if ( ExecSchedule->n_task <= 0 ){
		return ;
	}

	/*
		実行スケジュールの設定
	*/
	task = &ExecSchedule->task_list[ 0 ] ;

	//DmaStartTime = DG_TickCount ;

	/*
		スタティックプッシュバッファを実行させる

	*/
	DG_SetDrawStartMark();
	DG_SetDrawMark();
	for ( i = 0 ; i < ExecSchedule->n_task ; i++, task++ ){
#ifndef KP_WINDOWS
		//printf("%s:%p ~ %p\n", task->fname, task->dma_ptr, task->last_addr );
		if ( task->push_buffer.Size == 0 ){
#ifdef DEBUG_MODE
			printf("%s:zero size push-buffer %s\n", __FILE__, task->fname );
#endif
			continue ;
		}
		DG_SetDrawMark();
#if 0 //BP_RENDER
		IDirect3DDevice8_RunPushBuffer( g_pd3dDevice, &task->push_buffer, NULL );
#endif
#endif
	}
	DG_SetDrawEndMark();
	/* キックをして描画を強制的に開始 */
#if 0 //BP_RENDER
	IDirect3DDevice8_KickPushBuffer( g_pd3dDevice );
#endif

}

	/*
		ＤＭＡ転送処理のスキップ（描画キャンセル用）
	*/
void DG_DmaSkip( int which )
{
	ExecSchedule = &DG_DmaTaskSchedule[ which ];
	ExecSchedule->end_task = ExecSchedule->n_task ;
	//DmaStartTime = DG_TickCount ;
}

	/*
		ＤＭＡ転送の終了チェック
	*/
int DG_DmaCheckEnd( void )
{
	DG_DrawSync();
	return ( 1 );
#if 0
	if ( *(volatile int*)&ExecSchedule->end_task < *(volatile int*)&ExecSchedule->n_task ){
		//printf("wait sync\n");
		return ( 0 );
	}
	/* GSのFINISHイベントが発生したかチェック */
	if ( GsFinishFlag == 0 ){
		return ( 0 );
	}
	return ( 1 );
#endif
}

/*----------------------------------------------------------------*/
	/*
		ＤＭＡ転送タスク登録開始
	*/
#ifndef DEBUG_MODE
void DG_OpenDmaTask( void )
#else
void _DG_OpenDmaTask( char *fname )
#endif
{
	DG_DMATASK_SCHEDULE	*schedule ;
	DG_DMATASK			*task ;

	schedule = CurrentSchedule ;
	task = &schedule->task_list[ schedule->n_task ] ;
#ifdef DEBUG_MODE
	task->fname = fname ;
#endif
	schedule->n_task++ ;

	/* プッシュバッファの作成 */
	task->dma_ptr = DG_OpenPushBufferMemory( &task->push_buffer );
	/* スタティックプッシュバッファへの記録を開始 */
#if 0 //BP_RENDER
	IDirect3DDevice8_BeginPushBuffer( g_pd3dDevice, &task->push_buffer );
#endif

}

	/*
		ＤＭＡ転送タスク登録終了
	*/
void DG_CloseDmaTask( void )
{
	DG_DMATASK			*task ;
	int					size ;

	/* スタティックプッシュバッファへの記録を終了 */
#if 0 //BP_RENDER
	IDirect3DDevice8_EndPushBuffer( g_pd3dDevice );
#endif
	task = &CurrentSchedule->task_list[ CurrentSchedule->n_task - 1 ] ;

	//DG_CurrentDmaAddr += 16 ;

	size = DG_ClosePushBufferMemory( &task->push_buffer );
#ifdef DEBUG_MODE
	CurrentSchedule->task_list[ CurrentSchedule->n_task-1 ].last_addr = (void*)( (char*)task->dma_ptr + size ) ;
#endif


}



/*----------------------------------------------------------------*/


#if 0
/* ---------------------------------------------------------------------- */
/*
	パケットメモリのサイズを再設定する。
*/

void *DG_ResizePacketMemory( int size )
{
	void *res;
	/* 描画が終了するまでウェイトを入れる */
	while ( DG_DmaCheckEnd() == 0 );
	sceGsSyncPath( 0, 0 );			/* 念のため */

	if( size == 0 ){
		/* 元に戻す */
		DG_DmaBuffer[0] = PACK_ADDR0 ;
		DG_DmaBuffer[1] = PACK_ADDR1 ;
		DG_DmaBufferEnd[0] = PACK_ADDR0 + PACK_SIZE ;
		DG_DmaBufferEnd[1] = PACK_ADDR1 + PACK_SIZE ;

		res = NULL;
	} else {
		int psize;
		psize = ( PACK_SIZE * 2 - size ) / 2;
		DG_DmaBuffer[ 0 ] = PACK_ADDR0;
		DG_DmaBuffer[ 1 ] = PACK_ADDR0 + psize;
		DG_DmaBufferEnd[ 0 ] = DG_DmaBuffer[ 0 ] + psize ;
		DG_DmaBufferEnd[ 1 ] = DG_DmaBuffer[ 1 ] + psize;

		res = PACK_ADDR0 + psize * 2;
	}
	DG_DmaClear( DG_Clock );

	return res;
}

/* ---------------------------------------------------------------------- */

/* ---------------------------------------------------------------------- */
/*
	他のタスクからDMAパケット領域を使用する
*/

void *DG_OpenDmaPacketArea( void )
{
	return DG_CurrentDmaAddr;
}

void DG_CloseDmaPacketArea( void *addr )
{
	DG_CurrentDmaAddr = addr;
}

/* ---------------------------------------------------------------- */
#endif

#endif
