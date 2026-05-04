//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dma_ctrl.c
	ＰＳ２ＤＭＡ制御ルーチン

	1999/07/07 K.Takabe
	$Id: dma_ctrl.c,v 1.1.1.3 2002/11/19 11:42:06 Yoshizawa1 Exp $

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

#include	"libgv.h"
#include	"libgv.cnf"
//BP_PS2 #include	"break.h"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

#if 0
// BP - DISABLED UNUSED MACRO
#  if 1
#     define TRANS_SIZE(n)	((n+7)&0xfff8)		/* ＤＭＡ転送バグ回避用 */
#  else
#     define TRANS_SIZE(n)	(n)		
#  endif
#endif 

/* パケットバッファの書き込み抑制用マージン（1qword単位で指定） */
#define BUFFER_MARGIN	(64*1024/sizeof(u_long128))


	/*
		個々のＤＭＡタスク情報
	*/
typedef struct {
	short			dma_type ;		/* ＤＭＡの種類（0:VIF,1:GIF） */
	u_short			mark ;			/* デバッグ用マーク値 */
	void		*dma_ptr ;		/* 転送ＤＭＡのポインタ */
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
ALIGN16_PRE u_long128	*DG_CurrentDmaAddr ALIGN16_POST;	/* ＤＭＡパケットの書き込みポインタ */
ALIGN16_PRE u_long128	*DG_CurrentDmaEnd ALIGN16_POST;	/* ＤＭＡパケットの書き込み限界アドレス */
u_long128	*DG_LastDmaAddr;			/* 使用メモリ量検出用 */
static u_long128	*DG_DmaBuffer[2] ;		/* パケットメモリダブルバッファ先頭アドレス */
static u_long128	*DG_DmaBufferEnd[2] ;	/* パケットメモリダブルバッファ終了アドレス */

	/*
		テクスチャ入れ替えワーク（現在未使用）
	*/
u_long128	**DG_CurrentTextureBuffer ;
u_long128	**DG_ExecTextureBuffer ;
static u_long128	*DG_TextureBuffer[2][128] ;

	/*
		パケットＤＭＡ転送管理タスク
	*/
static void	*Vif1Dma, *GifDma, *fromSprDma ;	/* ＤＭＡアドレス */
static DG_DMATASK_SCHEDULE	DG_DmaTaskSchedule[ 2 ];	/* ダブルバッファで取る */
static DG_DMATASK_SCHEDULE	*CurrentSchedule ;			/* 作業用ワーク */
static DG_DMATASK_SCHEDULE	*ExecSchedule ;				/* 実行中ワーク */
#ifdef DEBUG_MODE
static void	*last_dma_address ;							/* 最終ＤＭＡ開始アドレス（デバッグ用） */
#endif

	/*
		内部使用ワーク
	*/
static int	vif_dmac_hander_id ;			/* VIF1DMA終了割り込みハンドらＩＤ */
//static int	gif_dmac_hander_id ;			/* VIF1DMA終了割り込みハンドらＩＤ */
//static int	vif_intc_hander_id ;			/* VIF1割り込みハンドラＩＤ */
static int	gs_intc_hander_id ;				/* GS割り込みハンドラＩＤ */
static int	GsFinishFlag = 0 ;				/* ＧＳ描画終了チェックフラグ */
static int	LocalToHostFlag = 0 ;			/* ＧＳのローカル＞ホスト転送要求フラグ */
static void	*StoreImageAddr ;				/* イメージ保存先アドレス */
static int	StoreImageSize ;				/* イメージ保存サイズ */
static volatile int	DmaStartTime = 0 ;		/* ＤＭＡタイムアウト検出用 */



/*----------------------------------------------------------------*/
static void WriteFinishPacket( void );
extern void DG_DumpDmaStatus( void );
extern void DG_iDmaDone( void );

/*----------------------------------------------------------------*/

	/*
		ＶＩＦ＆ＧＩＦ共通転送終了コールバック
	*/
static void DmaCallbackFunc( void )
{
#if 0 //BP_PS2
	DG_DMATASK			*task ;
	int					n ;

	n = ExecSchedule->end_task ;
	task = &ExecSchedule->task_list[ n ] ;

	/* ＧＳのローカル＞ホスト転送要求チェック */
	if ( LocalToHostFlag ){
		switch ( LocalToHostFlag ){
		  case 1:	/* 逆方向ＶＩＦＤＭＡ開始 */
			/* ここではＤＭＡを発行せずにFINISHイベント発生時にＤＭＡ転送を開始する */
			return ;
		  case 2:	/* 逆方向ＶＩＦＤＭＡ終了 */
			/* １回で転送できなかったデータを追加転送する */
			if ( StoreImageSize ){
				if ( StoreImageSize < 32768 ){
					DPUT_D1_QWC( StoreImageSize );
					StoreImageSize = 0 ;
				} else {
					DPUT_D1_QWC( 16384 );
					StoreImageSize -= 16384 ;
				}
				//DPUT_D1_CHCR( 0x0100 ) ; /* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:0 */
				asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D1_CHCR),"r"(0x100) );	/* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:0 */
				return ;
			}
			*VIF1_STAT = 0x00000000;	/* VIF1のＩＦ方向を元に戻す */
			DPUT_GS_BUSDIR( 0 );		/* ＧＳのＩＦ方向を元に戻す */
			LocalToHostFlag = 0 ;		/* ＧＳのローカル＞ホスト転送要求フラグのクリア */
			break ;
		}
	}

#ifdef DEBUG_MODE
	if (0){/* ＤＭＡ終了時のVIF1_MARKの値をチェックし、正常終了しているかを確認する */
		static u_int	b[32][8] ;
		static int		ad = 0 ;
		static u_int	backup_t ;
		u_int	t ;
		int		count = 0 ;
		b[ad][0] = n ;
		b[ad][1] = ExecSchedule->n_task ;
		//b[ad][2] = t ;
		b[ad][3] = task->mark ;
		b[ad][4] = (int)task->dma_ptr ;
		//b[ad][5] = count ;
		b[ad][6] = backup_t ;
		b[ad][7] = task[1].mark ;
		do {
			t = DGET_VIF1_MARK() ;
			count++ ;
			if ( count > 10000 ){
				int	i, index ;
				for ( i = 0 ; i < 32 ; i++ ){
					index = ( ad + i ) & 31 ;
					scePrintf("dma ok!! (task %d/%d %d/%d %08x %d %d %d)\n",
							  b[index][0], b[index][1], b[index][2], b[index][3],
							  b[index][4], b[index][5], b[index][6], b[index][7] );
				}
				scePrintf("dma mark error!! (task %d/%d %d/%d %08x %d %d)\n",
						  n, ExecSchedule->n_task, t, task->mark, task->dma_ptr, count, backup_t );
				{
					int		last_tag, tag_addr, mem_addr, qwc, stat ;
					last_tag = DGET_D1_CHCR();
					tag_addr = DGET_D1_TADR();
					mem_addr = DGET_D1_MADR();
					qwc = DGET_D1_QWC();
					stat = DGET_D_STAT();
					printf("dma STAT:%08x CHCR:$%08x,TADR:$%08x,$MADR:%08x,QWC:%08x\n", stat,
						   last_tag, tag_addr, mem_addr, qwc );
				}
				HANGUP();
				break ;
			}
		}while ( t != task->mark );
		//b[ad][0] = n ;
		//b[ad][1] = ExecSchedule->n_task ;
		b[ad][2] = t ;
		//b[ad][3] = task->mark ;
		//b[ad][4] = task->dma_ptr ;
		b[ad][5] = count ;
		//b[ad][6] = backup_t ;
		//b[ad][7] = task[1].mark ;
		ad = ( ad + 1 ) & 31 ;
		//scePrintf("dma ok!! (task %d/%d %d/%d %08x %d %d)\n",
		//		  n, ExecSchedule->n_task, t, task->mark, task->dma_ptr, count, backup_t );
		backup_t = t ;
		DPUT_VIF1_MARK(0x0000);
	}
#endif

	ExecSchedule->end_task = ++n ;
	//scePrintf("dma end(%d)\n", ExecSchedule->end_task );

	GV_PROFILE_DRAW( n );
	if ( n >= ExecSchedule->n_task ){
		GV_PROFILE_DRAW_END( n );
		{
			extern int DG_FrameStartTime, DG_LastDrawTime;
			DG_LastDrawTime = DIFF_HSYNC_TIMER( GET_HSYNC_TIMER(), DG_FrameStartTime );
		}
		//DG_iDmaDone();	/* これはＧＳのFISHISHイベント割り込みで行う */
		return ;
	}

	/*
		次のＤＭＡ転送を開始する
	*/
	task++ ;
#ifdef DEBUG_MODE
	last_dma_address = task->dma_ptr ;
#endif
	switch ( task->dma_type ){
	  case DG_OPEN_DMA_VIF1:
		/* VIF */
		//sceDmaSend( Vif1Dma, task->dma_ptr );
		DPUT_D1_TADR( (int)task->dma_ptr );
		DPUT_D1_QWC( 0 );
		//DPUT_D1_CHCR( 0x0145 );	/* STR:1 TIE:0 TTE:1 ASP:0 MOD:1 DIR:1 */
		asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D1_CHCR),"r"(0x145) );
		break ;
	  case DG_OPEN_DMA_GIF:
		/* GIF */
		//sceDmaSend( GifDma, task->dma_ptr );
		DPUT_D2_TADR( (int)task->dma_ptr );
		DPUT_D2_QWC( 0 );
		//DPUT_D2_CHCR( 0x0105 );	/* STR:1 TIE:0 TTE:0 ASP:0 MOD:1 DIR:1 */
		asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D2_CHCR),"r"(0x105) );
		break ;
	  case DG_OPEN_DMA_STORE:
		/* VIF & LocalToHost */
		LocalToHostFlag = 1 ;		/* ＤＭＡ終了時に逆方向ＶＩＦＤＭＡを発動させるためフラグを立てる */
		//sceDmaSend( Vif1Dma, task->dma_ptr );
		DPUT_D1_TADR( (int)task->dma_ptr );
		DPUT_D1_QWC( 0 );
		StoreImageAddr = task->store_addr ;
		StoreImageSize = task->store_size ;
		//DPUT_D1_CHCR( 0x0145 );	/* STR:1 TIE:0 TTE:1 ASP:0 MOD:1 DIR:1 */
		asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D1_CHCR),"r"(0x145) );
		break ;
	}
#endif
}

	/*
		ＤＭＡ転送終了例外割り込みハンドラ
	*/
static int DmacHandler( int ch )
//static int DmacHandler( int ch, void *arg, void *addr )
{
#if 0 //BP_PS2

//	int		last_tag, tag_addr, mem_addr, qwc, stat ;

	switch ( ch ){
	  case DMAC_VIF1:
#if 0
#ifdef DEBUG_MODE
		last_tag = DGET_D1_CHCR();
		tag_addr = DGET_D1_TADR();
		mem_addr = DGET_D1_MADR();
		qwc = DGET_D1_QWC();
		stat = DGET_D_STAT();
		//DPUT_D_STAT( 0x002 );	/* ライブラリ側でやってくれているのか？ */
#endif
#endif
		break ;
	  case DMAC_GIF:
#if 0
#ifdef DEBUG_MODE
		last_tag = DGET_D2_CHCR();
		tag_addr = DGET_D2_TADR();
		mem_addr = DGET_D2_MADR();
		qwc = DGET_D2_QWC();
		stat = DGET_D_STAT();
		//DPUT_D_STAT( 0x004 );	/* ライブラリ側でやってくれているのか？ */
#endif
#endif
		break ;
	  default:
		ExitHandler();	/* EEバグ回避用 */
		return ( 1 );
	}
#if 0
#ifdef DEBUG_MODE
	if ( ( last_tag & 0xf0000000 ) != DMATAG_ID_END ){
		scePrintf("dma error!!(%d)[%08x](CHCR:$%08x,TADR:$%08x,$MADR:%08x,QWC:%08x)\n", ch, stat,
				  last_tag, tag_addr, mem_addr, qwc );
		scePrintf("%08x %08x %08x %08x\n", 0[(int*)tag_addr], 1[(int*)tag_addr], 2[(int*)tag_addr], 3[(int*)tag_addr]);
		//if ( ch == 1 )
		//  sceDmaSend( Vif1Dma, tag_addr );
		ExitHandler();	/* EEバグ回避用 */
		return ( 0 );
	}
#endif
#endif
#if 0
	else if ( GV_PadData[1].press & PAD_A ){
		scePrintf("dma end(%d)[%08x](CHCR:$%08x,TADR:$%08x,$MADR:%08x,QWC:%08x)\n", ch, stat,
				  last_tag, tag_addr, mem_addr, qwc );
		scePrintf("%08x %08x %08x %08x\n", 0[(int*)tag_addr], 1[(int*)tag_addr], 2[(int*)tag_addr], 3[(int*)tag_addr]);
	}
#endif
	DmaCallbackFunc();
	ExitHandler();	/* EEバグ回避用 */
#endif
	return (1);
}

#if 0
	/*
		ＶＩＦ割り込み取得ハンドラ
	*/
static int VifHandler( int ca )
{
	if ( ca != INTC_VIF1 ){
		ExitHandler();		/* EEバグ回避用 */
		return ( 0 ) ;
	}

#if 0
	if ( *DG_ExecTextureBuffer != NULL ){
		/* 入れ替えテクスチャの転送処理 */
		if (GV_PadData[1].status & PAD_A ){
			sceDmaSend( GifDma, *DG_ExecTextureBuffer );
		}
		DG_ExecTextureBuffer++ ;
	}
#endif

	DPUT_VIF1_FBRST(0x08);
	ExitHandler();		/* EEバグ回避用 */
	return ( -1 );
}
#endif

	/*
		ＧＳ割り込み取得ハンドラ
	*/
static int GsHandler( int ca )
{
#if 0 //BP_PS2

	int		stat ;
	//u_long64 UserIMR;

	if ( ca != INTC_GS ){
		ExitHandler();	/* EEバグ回避用 */
		return ( 0 ) ;
	}
	/* 全割り込みをマスク */
	//UserIMR = sceGsPutIMR(0xff00);

	stat = DGET_GS_CSR();
	if ( !( stat & 0x00000002 ) ){	/* FINISHイベント判定 */
//		/* 割り込みマスクの解除 */
//		sceGsPutIMR(UserIMR);
		ExitHandler();	/* EEバグ回避用 */
		return ( 0 ) ;
	}
	DPUT_GS_CSR( 0x00000002 );		/* FINISHイベントのクリア */
#if 0
	scePrintf("dma (task %d/%d %d %08x )\n",
			  ExecSchedule->end_task, ExecSchedule->n_task,
			  ExecSchedule->task_list[ExecSchedule->end_task-1].mark,
			  ExecSchedule->task_list[ExecSchedule->end_task-1].dma_ptr );
#endif

	/* ＧＳローカル＞ホスト転送チェック */
	if ( LocalToHostFlag == 1 ){
		DG_DMATASK			*task ;
		int		n ;
		n = ExecSchedule->end_task ;
		task = &ExecSchedule->task_list[ n ] ;
		while ( DGET_D1_CHCR() & 0x100 );	/* 念のためＤＭＡ転送終了待ち */
		LocalToHostFlag = 2 ;
		/* VIF1からメモリへノーマルＤＭＡ転送を行う */
		*VIF1_STAT = 0x00800000;	/* VIF1のＩＦ方向を逆転させる */
		DPUT_GS_BUSDIR( 1 );		/* ＧＳのＩＦ方向を逆転させる */
		DPUT_D1_MADR( (int)StoreImageAddr );
		if ( StoreImageSize < 32768 ){
			DPUT_D1_QWC( StoreImageSize );
			StoreImageSize = 0 ;
		} else {
			DPUT_D1_QWC( 16384 );
			StoreImageSize -= 16384 ;
		}
		//DPUT_D1_CHCR( 0x0100 ) ; /* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:0 */
		asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D1_CHCR),"r"(0x100) );	/* STR:1 TIE:0 TTE:0 ASP:0 MOD:0 DIR:0 */

//		/* 割り込みマスクの解除 */
//		sceGsPutIMR(UserIMR);
		ExitHandler();	/* EEバグ回避用 */
		return ( -1 );
	}

	//scePrintf("dma end\n");

	/* 通常描画終了通知 */
	GsFinishFlag = 1 ;	/* ＧＳ終了フラグを立てる */
	DG_iDmaDone();

//	/* 割り込みマスクの解除 */
//	sceGsPutIMR(UserIMR);
	ExitHandler();	/* EEバグ回避用 */
#endif
	return ( -1 );
}


/*----------------------------------------------------------------*/

	/*
		パケット転送ＤＭＡの初期化
	*/
void DG_DmaReset( void )
{
#if 0 //BP_PS2
	/*
		ＤＭＡ初期化
	*/
	sceDevVif1Reset();
	sceDevVu1Reset();
	Vif1Dma    = sceDmaGetChan( 1 /* VIF1 */ );
	GifDma     = sceDmaGetChan( 2 /* GIF */ );
	fromSprDma = sceDmaGetChan( 8 /* fromSPR */ );
	Vif1Dma->chcr.TTE = 1 ;		/* DMA TAGも転送 */
	GifDma->chcr.TTE = 0 ;		/* DMA TAGは転送しない */
	DPUT_D_STAT( DGET_D_STAT() & ( 1 << ( 8 + 16 ) ) );/* fromSPR 割り込み禁止 */
	DPUT_D_STAT( DGET_D_STAT() & ( 1 << ( 9 + 16 ) ) );/* toSPR 割り込み禁止 */

	/*
		割り込みハンドラの設定
	*/
	/* ＶＩＦ１ＤＭＡ転送終了割り込み設定 */
	vif_dmac_hander_id = AddDmacHandler( DMAC_VIF1, DmacHandler, -1 );
	//vif_dmac_hander_id = AddDmacHandler2( DMAC_VIF1, DmacHandler, -1, (void*)0x1234567 );
	EnableDmac( DMAC_VIF1 );
	/* ＧＩＦＤＭＡ転送終了割り込み設定（未使用） */
	//AddDmacHandler( DMAC_GIF, DmacHandler, -1 );
	//gif_dmac_hander_id = AddDmacHandler( DMAC_GIF, DmacHandler, -1 );
	//EnableDmac( DMAC_GIF );
	/* ＶＩＦ割り込み設定（並列テクスチャ転送用：未使用） */
	//vif_intc_hander_id = AddIntcHandler( INTC_VIF1, VifHandler, -1 );
	//DPUT_VIF1_ERR(0x06);		/* 不要な割り込みの禁止＆iビット割り込み許可 */
	//DPUT_VIF1_FBRST(0x08);		/* ストールフラグのクリア */
	//EnableIntc( INTC_VIF1 );
	/* ＧＳ割り込み設定（FISNISHイベント取得用） */
	gs_intc_hander_id = AddIntcHandler( INTC_GS, GsHandler, -1 );
	DPUT_GS_CSR( 0x00000002 );		/* FINISHイベントのクリア */
	sceGsPutIMR( ~0x0200 );			/* FINISHによる割り込み許可 */
	//DPUT_GS_IMR( ~0x0200 ) ;
	EnableIntc( INTC_GS );

	/*
		その他設定
	*/
	//DPUT_GIF_MODE( 0x4 );/* image転送時の断続転送モード許可 */
#endif
	/*
		ワークなどの初期化
	*/
	DG_DmaBuffer[0] = PACK_ADDR0 ;
	DG_DmaBuffer[1] = PACK_ADDR1 ;
	DG_DmaBufferEnd[0] = (u_long128*)( (int)DG_DmaBuffer[0] + PACK_SIZE ) ;
	DG_DmaBufferEnd[1] = (u_long128*)( (int)DG_DmaBuffer[1] + PACK_SIZE ) ;
	//DG_DmaBuffer[0] = 0x02000000 + 0x00100000 * 3 ;	/* 32M+3M */
	//DG_DmaBuffer[1] = 0x02000000 + 0x00100000 * 6 ;	/* 32M+6M */
	//DG_DmaBuffer[0] = 0x04000000 + 0x00100000 * 3 ;	/* 32M+3M */
	//DG_DmaBuffer[1] = 0x04000000 + 0x00100000 * 6 ;	/* 32M+6M */
	DG_CurrentDmaAddr = DG_DmaBuffer[ 0 ] ;
	DG_CurrentDmaEnd = DG_DmaBufferEnd[ 0 ] - BUFFER_MARGIN ;
	*DG_TextureBuffer[0] = NULL ;
	*DG_TextureBuffer[1] = NULL ;
	DG_CurrentTextureBuffer = DG_TextureBuffer[0] ;
	DG_DmaTaskSchedule[ 0 ].n_task = 0 ;
	DG_DmaTaskSchedule[ 0 ].end_task = 0 ;
	DG_DmaTaskSchedule[ 0 ].terminate_flag = 0 ;
	DG_DmaTaskSchedule[ 1 ].n_task = 0 ;
	DG_DmaTaskSchedule[ 1 ].end_task = 0 ;
	DG_DmaTaskSchedule[ 1 ].terminate_flag = 0 ;
	CurrentSchedule = &DG_DmaTaskSchedule[ 0 ] ;
	ExecSchedule = &DG_DmaTaskSchedule[ 1 ] ;
	GsFinishFlag = 1 ;	/* ＧＳ終了フラグを立てる */
	LocalToHostFlag = 0 ;	/* ＧＳローカル＞ホスト転送要求フラグをクリア */
}

	/*
		ＤＭＡ転送管理タスクの初期化
	*/
void DG_DmaClear( int which )
{
	/* デバッグ用に直前のCurrentDmaAddrを待避 */
	DG_LastDmaAddr = DG_CurrentDmaAddr;

	DG_CurrentDmaAddr = DG_DmaBuffer[ which ] ;
	DG_CurrentDmaEnd = DG_DmaBufferEnd[ which ] - BUFFER_MARGIN ;
	CurrentSchedule = &DG_DmaTaskSchedule[ which ] ;
	DG_CurrentTextureBuffer = DG_TextureBuffer[which] ;
	*DG_CurrentTextureBuffer = NULL ;
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

	ExecSchedule = &DG_DmaTaskSchedule[ which ];

	/* GS終了検出用パケットを設定 */
	GsFinishFlag = 0 ;				/* ＧＳ終了フラグをクリア */

#if 0 //BP_PS2
   DPUT_GS_CSR( 0x00000002 );		/* FINISHイベントのクリア */
	sceGsPutIMR( ~0x0200 );			/* FINISHによる割り込み許可 */
#endif

	WriteFinishPacket();			/* FINISHイベント発生パケット登録 */
	ExecSchedule->end_task = 0 ;

	if ( ExecSchedule->n_task <= 0 ){
		return ;
	}

	FlushCache(0);						/* キャッシュのフラッシュ */
	//while ( sceGsSyncPath( 0, 0 ) ) ;	/* ＤＭＡ終了ウェイト */

#if 0
	{/* 最後に設定されたVIF1_MARKの値を表示 */
		int	t ;
		t = DGET_VIF1_MARK() ;
		DPUT_VIF1_MARK(0x0000);
		if ( t != 123 ){
			printf("dma exec error!! (%d)\n", t);
		}
	}
#endif

	/*
		テクスチャ入れ替えスタックの設定
	*/
	DG_ExecTextureBuffer = DG_TextureBuffer[ 1- which ] ;

	/*
		実行スケジュールの設定
	*/
	task = &ExecSchedule->task_list[ 0 ] ;

	/*
		最初のテクスチャ転送を開始する
	*/
	//VifHandler( INTC_VIF1 );

	DmaStartTime = DG_TickCount ;

#if 0 //BP_PS2

#if 1
	/*
		最初のＤＭＡ転送を開始する
	*/
#ifdef DEBUG_MODE
	last_dma_address = task->dma_ptr ;
#endif
#ifdef DEBUG_MODE
	DPUT_VIF1_MARK(0x0000);
#endif
	Vif1Dma->chcr.TTE = 1 ;		/* DMA TAGも転送 */
	GifDma->chcr.TTE = 0 ;		/* DMA TAGは転送しない */
	switch ( task->dma_type ){
	  case DG_OPEN_DMA_VIF1:
		/* VIF */
		//sceDmaSend( Vif1Dma, task->dma_ptr );
		DPUT_D1_TADR( (int)task->dma_ptr );
		DPUT_D1_QWC( 0 );
		//DPUT_D1_CHCR( 0x0145 );	/* STR:1 TIE:0 TTE:1 ASP:0 MOD:1 DIR:1 */
		asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D1_CHCR),"r"(0x145) );
		break ;
	  case DG_OPEN_DMA_GIF:
		/* GIF */
		//sceDmaSend( GifDma, task->dma_ptr );
		DPUT_D2_TADR( (int)task->dma_ptr );
		DPUT_D2_QWC( 0 );
		//DPUT_D2_CHCR( 0x0105 );	/* STR:1 TIE:0 TTE:0 ASP:0 MOD:1 DIR:1 */
		asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D2_CHCR),"r"(0x105) );
		break ;
	  case DG_OPEN_DMA_STORE:
		/* VIF & LocalToHost */
		LocalToHostFlag = 1 ;		/* ＤＭＡ終了時に逆方向ＶＩＦＤＭＡを発動させるためフラグを立てる */
		//sceDmaSend( Vif1Dma, task->dma_ptr );
		DPUT_D1_TADR( (int)task->dma_ptr );
		DPUT_D1_QWC( 0 );
		StoreImageAddr = task->store_addr ;
		StoreImageSize = task->store_size ;
		//DPUT_D1_CHCR( 0x0145 );	/* STR:1 TIE:0 TTE:1 ASP:0 MOD:1 DIR:1 */
		asm volatile( "sync.l;sw %1, 0(%0)" :: "r"(D1_CHCR),"r"(0x145) );
		break ;
	}
	//while ( DG_DmaCheckEnd() == 0 ) ;	/* ＤＭＡによる描画が終了するまでウェイト（デバッグ用） */
#else
	{/* 割り込みを使わずにＤＭＡを処理する（デバッグ用） */
		int		i ;
		int		last_tag, tag_addr, mem_addr, qwc, stat ;
		DisableDmac( DMAC_VIF1 );
		for ( i = ExecSchedule->n_task ; i > 0 ; task++, i-- ){
			if ( task->dma_type == 0 ){
				/* VIF */
				sceDmaSend( Vif1Dma, task->dma_ptr );
				while ( sceGsSyncPath( 0, 0 ) ) ;	/* ＤＭＡ終了ウェイト */
				last_tag = DGET_D1_CHCR();
				tag_addr = DGET_D1_TADR();
				mem_addr = DGET_D1_MADR();
				qwc = DGET_D1_QWC();
				stat = DGET_D_STAT();
			} else {
				/* GIF */
				sceDmaSend( GifDma, task->dma_ptr );
				while ( sceGsSyncPath( 0, 0 ) ) ;	/* ＤＭＡ終了ウェイト */
				last_tag = DGET_D2_CHCR();
				tag_addr = DGET_D2_TADR();
				mem_addr = DGET_D2_MADR();
				qwc = DGET_D2_QWC();
				stat = DGET_D_STAT();
			}
			if ( ( last_tag & 0xf0000000 ) != DMATAG_ID_END ){
				scePrintf("dma error!!(%d)[%08x](CHCR:$%08x,TADR:$%08x,$MADR:%08x,QWC:%08x)\n", task->dma_type, stat,
						  last_tag, tag_addr, mem_addr, qwc );
				scePrintf("%08x %08x %08x %08x\n",
						  0[(int*)tag_addr], 1[(int*)tag_addr], 2[(int*)tag_addr], 3[(int*)tag_addr]);
				//if ( ch == 1 )
				//  sceDmaSend( Vif1Dma, tag_addr );
			}
		}
		ExecSchedule->end_task = ExecSchedule->n_task ;
	}
#endif

#endif
}

	/*
		ＤＭＡ転送処理のスキップ（描画キャンセル用）
	*/
void DG_DmaSkip( int which )
{
	ExecSchedule = &DG_DmaTaskSchedule[ which ];
	ExecSchedule->end_task = ExecSchedule->n_task ;
	DmaStartTime = DG_TickCount ;
	GsFinishFlag = 1 ;		/* ＧＳ終了フラグを立てる */
#if 0 //BP_PS2
	DPUT_GS_CSR( 0x00000002 );		/* FINISHイベントのクリア */
#endif
}

	/*
		ＤＭＡ転送の終了チェック
	*/
int DG_DmaCheckEnd( void )
{
#if 0 //BP_PS2
	if ( *(volatile int*)&ExecSchedule->end_task < *(volatile int*)&ExecSchedule->n_task ){
		//printf("wait sync\n");
#ifdef DEBUG_MODE
		/* ＤＭＡタイムアウトチェック */
		if ( ( DG_TickCount - DmaStartTime ) > 60 ){
			DG_DumpDmaStatus();
			while ( 1 );
			DI();
			DPUT_D1_CHCR( DGET_D_STAT() & ( ~0x100 ) );
			sceDevVif1Reset();
			sceDevVu1Reset();
			EI();
			return ( 1 );
		}
#endif
		return ( 0 );
	}
	/* GSのFINISHイベントが発生したかチェック */
	if ( GsFinishFlag == 0 ){
		/* ＤＭＡタイムアウトチェック */
		if ( ( DG_TickCount - DmaStartTime ) > 60 ){
			DG_DumpDmaStatus();
		}
		return ( 0 );
	}
#endif
	return ( 1 );
}

/*----------------------------------------------------------------*/

	/*
		ＤＭＡ転送タスク登録開始
	*/
#ifndef DEBUG_MODE
void *DG_OpenDmaTask( int type, void *addr, int size )
{
	DG_DMATASK_SCHEDULE	*schedule ;
	DG_DMATASK			*task ;

	schedule = CurrentSchedule ;
	task = &schedule->task_list[ schedule->n_task ] ;
#ifdef DEBUG_MODE
	task->mark = BP_PS2_rand() & 0xffff ;
#endif
	task->dma_type = type ;
	task->dma_ptr = DG_CurrentDmaAddr ;
	task->store_addr = addr ;
	task->store_size = size ;
	schedule->n_task++ ;
	return ( (void*)task->dma_ptr );
}
#else
void *_DG_OpenDmaTask( int type, void *addr, int size, char *fname )
{
	DG_DMATASK_SCHEDULE	*schedule ;
	DG_DMATASK			*task ;

	schedule = CurrentSchedule ;
	task = &schedule->task_list[ schedule->n_task ] ;
#ifdef DEBUG_MODE
	task->mark = BP_PS2_rand() & 0xffff ;
	task->fname = fname ;
#endif
	task->dma_type = type ;
	task->dma_ptr = (sceDmaTag*)DG_CurrentDmaAddr ;
	task->store_addr = addr ;
	task->store_size = size ;
	schedule->n_task++ ;
	return ( (void*)task->dma_ptr );
}
#endif

	/*
		ＤＭＡ転送タスク登録終了
	*/
void DG_CloseDmaTask( void )
{
	DG_DMATAG	*dmatag ;
	/* ＤＭＡの終端タグを設定 */
	dmatag = (DG_DMATAG*)DG_CurrentDmaAddr ;
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 0 ) ;
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	dmatag->vifcode[1] = SCE_VIF1_SET_NOP( 0 );
#ifdef DEBUG_MODE
	dmatag->vifcode[1] = SCE_VIF1_SET_MARK( CurrentSchedule->task_list[ CurrentSchedule->n_task-1 ].mark, 0 );
	CurrentSchedule->task_list[ CurrentSchedule->n_task-1 ].last_addr = DG_CurrentDmaAddr ;
#endif
	dmatag++ ;
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_END, 0 ) ;
	dmatag->vifcode[0] = SCE_VIF1_SET_NOP( 0 );
	dmatag->vifcode[1] = SCE_VIF1_SET_NOP( 0 );
	DG_CurrentDmaAddr += 8 ;	/* 場合によってはある程度ずらさないとだめ。原因不明 */
	//DG_CurrentDmaAddr += 2 ;	/* 場合によってはある程度ずらさないとだめ。原因不明 */
	//FlushCache( 0 );

	/* パケットメモリオーバーチェック */
	//ASSERT( (int)DG_CurrentDmaAddr < (int)DG_CurrentDmaEnd );
}



/*----------------------------------------------------------------*/

	/*
		テクスチャ入れ替えパケットの登録
	*/
void DG_SetLoadTexturePacket( void *addr )
{
	*DG_CurrentTextureBuffer++ = addr ;
	*DG_CurrentTextureBuffer = NULL ;
}

/* ---------------------------------------------------------------------- */
/*
	パケットメモリのサイズを再設定する。
*/

void *DG_ResizePacketMemory( int size )
{
	void *res;
	/* 描画が終了するまでウェイトを入れる */
	while ( DG_DmaCheckEnd() == 0 );
#if 0 //BP_PS2
	sceGsSyncPath( 0, 0 );			/* 念のため */
#endif
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
	/*
		描画終了の検出に使用するパケットを書き込む
	*/
static void WriteFinishPacket( void )
{
	DG_DMATAG	*dmatag ;
	DG_GIFTAG	*giftag ;
	DG_GSREG	*gsreg ;

	if ( CurrentSchedule->terminate_flag ) return ;

	/* ＤＭＡバッファオープン(VIF1) */
	DG_OpenDmaTask( DG_OPEN_DMA_VIF1, NULL, 0 );

	dmatag = (DG_DMATAG*)DG_CurrentDmaAddr++ ;
	dmatag->qwc = DMATAG_SET_QWC( DMATAG_ID_CNT, 2 );
	dmatag->vifcode[0] = SCE_VIF1_SET_FLUSHA( 0 );
	dmatag->vifcode[1] = SCE_VIF1_SET_DIRECT( 2, 0 );

	giftag = (DG_GIFTAG*)DG_CurrentDmaAddr++ ;
	giftag->tag = SCE_GIF_SET_TAG( 1, 1, 0, 0, SCE_GIF_PACKED, 1 );
	giftag->regs = 0xe ;

	gsreg = (DG_GSREG*)DG_CurrentDmaAddr++ ;
	gsreg->data = 0 ;
	gsreg->reg = SCE_GS_FINISH ;

	/* ＤＭＡバッファクローズ */
	DG_CloseDmaTask();
	
	CurrentSchedule->terminate_flag = 1 ;
}

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
/* ＤＭＡ動作状態のデバッグ出力 */
void DG_DumpDmaStatus( void )
{
#ifdef DEBUG_MODE
	printf("dma time out!!\n");
	{/* ＤＭＡ関連ステータス表示＆ＤＭＡタグ周辺メモリ内容ダンプ */
		int		last_tag, tag_addr, mem_addr, qwc, stat ;
		printf("*** DMA status ***\n");
		/* データキャプチャ */
		last_tag = DGET_D1_CHCR();
		tag_addr = DGET_D1_TADR();
		mem_addr = DGET_D1_MADR();
		qwc = DGET_D1_QWC();
		stat = DGET_D_STAT();
		/* 表示 */
		printf("dma STAT:%08x CHCR:$%08x,TADR:$%08x,$MADR:%08x,QWC:%08x\n", stat,
			   last_tag, tag_addr, mem_addr, qwc );
		/* 関連メモリ領域のダンプ */
		printf("TADR %08x: %08x %08x %08x %08x\n",tag_addr,
			   0[(int*)tag_addr], 1[(int*)tag_addr], 2[(int*)tag_addr], 3[(int*)tag_addr]);
		printf("%08x: %08x %08x %08x %08x\n",(int)mem_addr-16,
			   (0-4)[(int*)mem_addr], (1-4)[(int*)mem_addr], (2-4)[(int*)mem_addr], (3-4)[(int*)mem_addr]);
		printf("%08x: %08x %08x %08x %08x\n",mem_addr,
			   0[(int*)mem_addr], 1[(int*)mem_addr], 2[(int*)mem_addr], 3[(int*)mem_addr]);
		printf("%08x: %08x %08x %08x %08x\n",(int)mem_addr+16,
			   (0+4)[(int*)mem_addr], (1+4)[(int*)mem_addr], (2+4)[(int*)mem_addr], (3+4)[(int*)mem_addr]);
	}
	{/* ＶＩＦ関連ステータス表示 */
		sceDevVif1Cnd	vif_cnd ;
		int				flag ;
		printf("*** VIF1 status ***\n");
		/* データキャプチャ */
		flag = sceDevVif1Pause( 0 );
		sceDevVif1GetCnd( &vif_cnd );
		if ( flag ) sceDevVif1Continue();
		/* 表示 */
		printf("VIF1 STAT:%08x CODE:%08x NUM:%08x ERROR:%08x MARK:%08x \n",
			   vif_cnd.stat, vif_cnd.code, vif_cnd.num, vif_cnd.error, vif_cnd.mark );
		printf("     BASE:%08x OFST:%08x ITOP:%08x ITOPS:%08x MODE:%08x \n",
			   vif_cnd.base, vif_cnd.offset, vif_cnd.top, vif_cnd.tops, vif_cnd.cmod );
	}
	{/* ＶＵ１関連ステータス表示 */
		sceDevVu1Cnd	vu1_cnd ;
		int				flag, vu1_addr, vu_stat, i ;
		printf("*** VU1 status ***\n");
		/* データキャプチャ */
		asm volatile ("cfc2 %0,$29":"=r"(vu_stat) );
		flag = sceDevVu1Pause();
		asm volatile ("
					ctc2	%1,$1
					vlqi	vf1,(vi1++)
					qmfc2	%0,vf1
				":"=r"(vu1_addr):"r"(0x43a));
		sceDevVu1GetCnd( &vu1_cnd );
		if ( flag ) sceDevVu1Continue();
		/* 表示 */
		printf("VU1 status:%08x TPC:%08x(%08x)\n",
			   vu_stat, vu1_addr, 0x11008000+vu1_addr*8 );
		/* 整数レジスタダンプ */
		for ( i = 0 ; i < 8 ; i++ ) printf("%02d:%04x ", i, vu1_cnd.vi[i] ); printf("\n");
		for ( i = 8 ; i < 16 ; i++ ) printf("%02d:%04x ", i, vu1_cnd.vi[i] ); printf("\n");
	}
	{/* ＧＩＦ関連表示 */
		sceDevGifCnd	gif ;
		DG_GIFTAG		*giftag ;
		int		flag ;
		flag = sceDevGifPause();
		sceDevGifGetCnd( &gif );
		if ( flag ) sceDevGifContinue();
		giftag = (void*)&gif.tag ;
		printf("*** GIF status ***\n");
		printf("STAT:%08x CNT:%08x P3CNT:%08x P3TAG:%08x\n", gif.stat, gif.count, gif.p3count, gif.p3tag);
		printf("%08x-%08x-%08x-%08x\n",
			   ( giftag->regs >> 32 ) & 0xffffffff, giftag->regs & 0xffffffff,
			   ( giftag->tag >> 32 ) & 0xffffffff, giftag->tag & 0xffffffff);
	}
	{/* ＤＭＡスケジューリング関連表示 */
		int		i ;
		printf("*** DMA scheduling info ***\n");
		printf("dma shcedule end_task:%d, total_task:%d  last dma start address: %08x\n",
			   ExecSchedule->end_task, ExecSchedule->n_task, last_dma_address );
		printf("mark:%08x\n", DGET_VIF1_MARK() );
		for ( i = 0 ; i < ExecSchedule->n_task ; i++ ){
			printf("%d) start_addr:%08x, last_addr:%08x, mark:%04x , (%s)\n", i,
				   ExecSchedule->task_list[ i ].dma_ptr,
				   ExecSchedule->task_list[ i ].last_addr,
				   ExecSchedule->task_list[ i ].mark & 0xffff,
				   ExecSchedule->task_list[ i ].fname );
		}
	}
	{/* ＧＳ割り込み関連表示 */
		printf("GS_CSR:%08x GsFinishFlag:%d\n", DGET_GS_CSR(), GsFinishFlag );

	}
#endif
}

void DG_EndDmaCtrl( void )
{
#if 0 //BP_PS2
	RemoveDmacHandler( DMAC_VIF1, vif_dmac_hander_id );
	RemoveIntcHandler( INTC_GS, gs_intc_hander_id );
#endif
}
