/*
	XBOX CRI Stream Driver

	I Picture Only Stream,
	I Picture Only Memory Stream,
	MPEG Stream

	2002/05/20 M.Kobayashi
	$Id: cristream.c,v 1.3 2002/11/23 11:01:03 Yoshizawa1 Exp $
*/

//#include	<cri_mw.h>
//#include	<sfvsjd.h>
// 上記２つは後にinclude してはいけない( #define long 問題 )

#include	"mgs_type.h"
#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"libdg.h"
#include	"stream.h"
#include	"g_define.h"

#include	"strctrl.h"


//#define DECODE_OTHER_THREAD

typedef struct MPEG_HEADER {
	short	width;
	short	height;
	int		nFrames;
	float	fFps;
	int		pad;
} MPEG_HEADER;

// MPEG 時のリングバッファサイズ
#define RING_BUF_SIZE	(1024 * 128)
#define RING_BUF_EXTEND_SIZE	(1024 * 32)

#define	OPT_FLAG_PRECIOUS		0x80000000	// スキップ不可データ
#define	OPT_FLAG_NOT_MPEG		0x40000000	// MPEG ストリームではないデータ

#define GET_MPEG_STREAM_SIZE( p ) ( FS_STREAM_GET_OPTION( p ) & ~(OPT_FLAG_PRECIOUS | OPT_FLAG_NOT_MPEG ) )


/* ---------------------------------------------------------------- */

/* ストリームドライバ登録情報 */
void *NewStreamIpicDriver( GM_STREAM_CONTROL *ctrl, int type );
void *NewStreamCriMpegDriver( GM_STREAM_CONTROL *ctrl, int type );

static GM_STREAM_DRIVER driver = {
	NULL,
	0,
	NewStreamIpicDriver,
};

static GM_STREAM_DRIVER drivermpg = {
	NULL,
	0,
	NewStreamCriMpegDriver,
};

enum {
	STATE_READY,	// ヘッダ読み込み中
	STATE_DEC,		// デコード中
	STATE_END,		// 終了待ち
};


/* ---------------------------------------------------------------- */
typedef struct {
#if 0	//@ コンパイルがとおらなかったので修正
	GV_ACT_EX	actor;
	/* 通常ストリーム用ワーク */
	int			type;			/* ストリームタイプ */
	GM_STREAM_CONTROL* pctrl;	/* ストリームコントロール */
	/* ドライバ固有ワーク */
	SJ			sji;		// 入力ストリームジョイント
	char		sji_work[ SJUNI_CALC_WORK( 16 ) ];	// ジョイントワーク

	u_int		flag;
	int			state;
	MPEG_HEADER	mh;
	
	// IPIC 用
	
	SFVSJD 		sjd;		// デコーダ
	SJ			sjo;		// 出力ストリームジョイント
	char		sjo_work[ SJUNI_CALC_WORK( 16 ) ];	// ジョイントワーク
	Sint32		sjd_worksize;	// デコーダワークサイズ
	Sint8*		pSjd_work;		// デコーダワーク
//	char		yuv420_img[ 640 * 480 * 3 / 2 ];	// 450kb サイズが決まってから動的に確保したいのだが。。。
//	char		yuv420_img[ 256 * 256 * 3 / 2 ];
	char*		pYuv420Image;

	SJCK		ckout;		// 出力チャンク

	// OnMemory IPIC 用
	STREAM_TAG* 		pTopmem;
	STREAM_TAG* 		pCurrentmem;

	// MPEG 用
	MWPLY		ply;		// デコーダハンドル
	MWS_PLY_FRM	frm;		// 取得フレーム
//	void*		pRingBuf;	// リングバッファ
#endif
	void*		pPlyWork;	// デコーダワーク

} CRI_STR_WORK;

enum {
	FLAG_ON_MEMORY	= 0x01,
	FLAG_MPEG		= 0x02,
	FLAG_NO_SKIP	= 0x04,
};

static int	DriverRefCtr = 0 ;
static CRI_STR_WORK	*pWork = NULL ;


/* ---------------------------------------------------------------- */


/* ---------------------------------------------------------------- */
static BOOL CodeIsEnd( u_char* code ) {
	return code[ 0 ] == 0x00 &&
			code[ 1 ] == 0x00 &&
			code[ 2 ] == 0x01 &&
			code[ 3 ] == 0xb7;
}

static void *StreamGetData( CRI_STR_WORK *pw )
{
#if 0	//@ コンパイルがとおらなかったので修正
	if( pw->flag & FLAG_ON_MEMORY ) {
//		return NULL;
		if( pw->pCurrentmem == NULL ) return NULL;
		return pw->pCurrentmem + 1;
	} else {
		/* フレームデータ取得中の場合 */
		void	*data_ptr = NULL, *old_ptr = NULL ;
		/* タイムアジャスト処理 */
		while ( 1 ){
			/* とりあえずデータの取得 */
			data_ptr = FS_StreamGetData( pw->pctrl->stream_h, pw->type );
			if ( data_ptr == NULL ) return ( old_ptr );

			/* 時間の整合チェック */
			if ( FS_STREAM_GET_TIME( data_ptr ) > pw->pctrl->tick ){
				/* 先のデータを取得してしまったのでそれを戻し、前のデータポインタを返す */
				FS_StreamUngetData( pw->pctrl->stream_h, data_ptr );
				return ( old_ptr );
			}

			// MPEG時は全データが必要 IPicのときも最初のフレームは飛ばせない
			if ( (pw->flag & ( FLAG_MPEG | FLAG_NO_SKIP ) ) ) return data_ptr; 
				 
			if( (FS_STREAM_GET_OPTION( data_ptr ) & OPT_FLAG_PRECIOUS) ) {
				// スキップ不可データ
				if ( old_ptr != NULL ) {
					FS_StreamFreeData( pw->pctrl->stream_h, old_ptr );
				}
				return data_ptr;
			}
			
			//printf("%s:stream time = %d\n", __FILE__, FS_STREAM_GET_TIME( data_ptr ) );
			/* 取得したデータには問題がないので前のデータポインタを破棄する */
			if ( old_ptr != NULL ){
				printf("%s:frame skip! %d/%d\n", __FILE__, FS_STREAM_GET_TIME( old_ptr ), pw->pctrl->tick );
//				printf( "skip%d/%d!", FS_STREAM_GET_TIME( old_ptr ), pw->pctrl->tick );
				FS_StreamFreeData( pw->pctrl->stream_h, old_ptr );
				/* 一つスキップしたことになるので内部処理フレーム数もインクリメントする */
				//if ( FS_STREAM_GET_OPTION( data_ptr ) != 0 ) pw->frame++ ;
				//pw->frame++ ;
			}
			/* データポインタのバックアップを保存する */
			old_ptr = data_ptr ;
		}
	}		
#endif
}
static void StreamFreeData( CRI_STR_WORK* pw, void* pData )
{
#if 0	//@ コンパイルがとおらなかったので修正
	if ( pw->flag & FLAG_ON_MEMORY ){
		pw->pCurrentmem = (STREAM_TAG*)( (int)pw->pCurrentmem + pw->pCurrentmem->size );
		// リピート処理
		if( CodeIsEnd( (u_char*)(pw->pCurrentmem + 1 ))) pw->pCurrentmem = pw->pTopmem;
		return ;
	} else {
		FS_StreamFreeData( pw->pctrl->stream_h, pData );
	}
#endif
}
static int StreamIsEnd( CRI_STR_WORK* pw )
{
#if 0	//@ コンパイルがとおらなかったので修正
	if ( pw->flag & FLAG_ON_MEMORY ){
		return 0;
	} else {
		return ( GM_IS_STREAM_END( pw->pctrl ) );
	}
#endif
}

static void ReleaseRes( CRI_STR_WORK* pw )
{	// Act 中に確保したリソースの解放
#if 0	//@ コンパイルがとおらなかったので修正
	if( pw->flag & FLAG_MPEG ) {
		if( pw->ply != NULL ) mwPlyDestroy( pw->ply );
		if( pw->pPlyWork != NULL ) GV_Free( pw->pPlyWork );
		pw->ply = NULL;
		pw->pPlyWork = NULL;
	} else {
		if( pw->sjd != NULL ) SFVSJD_Destroy( pw->sjd );
		if( pw->sji != NULL ) SJ_Destroy( pw->sji );
		if( pw->sjo != NULL ) SJ_Destroy( pw->sjo );
		if( pw->pSjd_work != NULL) GV_Free( pw->pSjd_work );
		if( pw->pYuv420Image != NULL ) GV_Free( pw->pYuv420Image );
		pw->sjd = NULL;
		pw->sji = pw->sjo = NULL;
		pw->pSjd_work = NULL;
		pw->pYuv420Image = NULL;
	}
#endif
}

static void ActIpic( CRI_STR_WORK* pw )
{
#if 0	//@ コンパイルがとおらなかったので修正
	void* pData;
	SJCK	ck;
#if 0
	int intime, outtime;
	intime = GET_HSYNC_TIMER();
#endif	
	
	switch( pw->state ) {
	case STATE_READY:
		// ヘッダ読み込み待ち
		if ( !(pw->flag & FLAG_ON_MEMORY) && StreamIsEnd( pw ) ) {
			GV_DestroyActor( pw );
			break;
		}
		if ( (pData = (STREAM_TAG*)StreamGetData( pw ) )!= NULL ) {
			SJCK ck;
			int imagebufsize;
			
			memcpy( &pw->mh, pData, sizeof( pw->mh ) );
			printf("CRI Stream %dx%d, %ffps\n", pw->mh.width, pw->mh.height, pw->mh.fFps );
			
			pw->state = STATE_DEC;
			pw->flag |= FLAG_NO_SKIP;	// 最初のデータはスキップしない
			
			StreamFreeData( pw, pData );
			if( pw->flag & FLAG_ON_MEMORY ) {
				// ループの戻り先をヘッダを抜かした位置に設定する
				pw->pTopmem = pw->pCurrentmem;	
			}
			// サイズが決まったのでメモリ準備
//			imagebufsize = pw->mh.width * pw->mh.height * 3 / 2;
			// 32の倍数の横サイズにしか対応してないっぽい
			imagebufsize = ((pw->mh.width + 0x1f) & ~0x1f) * pw->mh.height * 3 / 2;
				
			pw->pYuv420Image = GV_Malloc( imagebufsize );
			pw->sjd_worksize = SFVSJD_CalcWork( NULL );
			pw->pSjd_work = (Sint8*)GV_Malloc( (int)pw->sjd_worksize );
			if( pw->pYuv420Image == NULL || pw->pSjd_work == NULL ) {
				// メモリ確保できず
				GV_DestroyActor( pw );
				return;
			}
		
			pw->sji = SJUNI_Create( SJUNI_MODE_JOIN, pw->sji_work, sizeof( pw->sji_work ) );
			pw->sjo = SJUNI_Create( SJUNI_MODE_SEPA, pw->sjo_work, sizeof( pw->sjo_work ) );
			ck.data = (Sint8*)pw->pYuv420Image;
			ck.len = imagebufsize;
			SJ_PutChunk( pw->sjo, SJ_LIN_FREE, &ck );
			
			pw->sjd = SFVSJD_Create( pw->sji, pw->sjo, NULL, pw->pSjd_work, pw->sjd_worksize );
			
			pw->ckout.data = NULL;
			pw->ckout.len = 0;
			
			SFVSJD_Start( pw->sjd );
		} else break;
	case STATE_DEC:
		// 通常デコード
		if ( (pData = (STREAM_TAG*)StreamGetData( pw ) )!= NULL ) {
			ck.data = (Sint8*)pData;
			ck.len = GET_MPEG_STREAM_SIZE( pData );	// ここに実際のサイズが入ってる
			pw->flag &= ~FLAG_NO_SKIP;	// スキップ可能にする
#if 0			
			if( CodeIsEnd( (char*)pData ) ) printf("code end\n");
#endif			
					
			SJ_PutChunk( pw->sji, SJ_LIN_DATA, &ck );
			
			// 出力データ処理
			if( pw->ckout.len > 0 ) {
				// 前回のが残っていたらフリーする
				// 通常ここにはこないはず
				SJ_PutChunk( pw->sjo, SJ_LIN_FREE, &pw->ckout );
				pw->ckout.len = 0;
				pw->ckout.data = NULL;
			}
				
			// デコード実行
			SFVSJD_ExecHndl( pw->sjd );
			// データをフリー
			if( !(pw->flag & FLAG_ON_MEMORY) ) {
				StreamFreeData( pw, pData );
			}
		
			SJ_GetChunk( pw->sjo, SJ_LIN_DATA, 0x7fffffff, &pw->ckout );
			// 入力データ回収
			SJ_Reset( pw->sji );

			if( SFVSJD_GetStat( pw->sjd ) == SFVSJD_STAT_DECEND ) {
				pw->state = STATE_END;
			}
		} 
		break;
	case STATE_END:	// 終了待ち
		if( pw->ckout.len == 0 ) {
			// 最後のデータが取り出された
			printf("IPIC stream end\n");
			// 最初に戻って待機
			pw->state = STATE_READY;
			ReleaseRes( pw );
			break;
		}
//		FS_StreamDump( pw->pctrl->stream_h );
	}

#if 0	
	outtime = GET_HSYNC_TIMER();
	printf("%3d ", DIFF_HSYNC_TIMER( outtime, intime ) );
#endif	
#endif
}

static void ActMpeg( CRI_STR_WORK* pw )
{
#if 0	//@ コンパイルがとおらなかったので修正
	void* pData;
	SJCK	ck;
#if 0
	int intime, outtime;

	intime = GET_HSYNC_TIMER();
#endif	
	
	switch( pw->state ) {
	case STATE_READY:
		// ヘッダ読み込み待ち
		if ( (pData = (STREAM_TAG*)StreamGetData( pw ) )!= NULL ) {
			MWS_PLY_CPRM_SFD cprm;		//	Creating parameter
			
			memcpy( &pw->mh, pData, sizeof( pw->mh ) );
			printf("CRI Stream %dx%d, %ffps\n", pw->mh.width, pw->mh.height, pw->mh.fFps );
			pw->state = STATE_DEC;
			StreamFreeData( pw, pData );
			// サイズが決まったのでメモリ準備
			
			mwPlyInitYcc420plnToArgb8888();		// Initialize convert table
		
			memset(&cprm, 0, sizeof(cprm));
			cprm.ftype			= MWD_PLY_FTYPE_MPV;
			cprm.max_bps		= 5*1000*1000;
			cprm.max_width		= pw->mh.width;
			cprm.max_height		= pw->mh.height;
			cprm.nfrm_pool_wk	= 4;
			cprm.wksize			= mwPlyCalcWorkCprmSfd(&cprm);
			cprm.work			= (Sint8 *)pw->pPlyWork = GV_Malloc(cprm.wksize);
			if( cprm.work == NULL ) {
				// メモリ確保できず
				GV_DestroyActor( pw );
				return;
			}
				
			pw->ply				= mwPlyCreateSofdec(&cprm);
			
			// リングバッファ取得
			pw->sji = mwPlyGetInputSj( pw->ply );
			
			mwPlyStartSj( pw->ply, pw->sji );
		} else {
			if ( !(pw->flag & FLAG_ON_MEMORY) && StreamIsEnd( pw ) ) {
				GV_DestroyActor( pw );
			}
			break;
		}
	case STATE_DEC:
		// 通常デコード
		if ( (pData = (STREAM_TAG*)StreamGetData( pw ) )!= NULL ) {
//			printf("%s:frame %d/%d\n", __FILE__, FS_STREAM_GET_TIME( pData ), pw->pctrl->tick );
			// 入力データあり
			
			// データをリングバッファにコピー
			int size = GET_MPEG_STREAM_SIZE( pData );
			char* pDataCur = (char*)pData;

			if( SJ_GetNumData( pw->sji, SJ_LIN_FREE ) < size ) {
				FS_StreamUngetData( pw->pctrl->stream_h, pData );
			} else {
				do {
					SJ_GetChunk( pw->sji, SJ_LIN_FREE, size, &ck );
//					printf("free %x-%x\n", ck.data, ck.len);
					memcpy( ck.data, pDataCur, ck.len );
					SJ_PutChunk( pw->sji, SJ_LIN_DATA, &ck );
					size -= ck.len;
					pDataCur += ck.len;
				} while ( size > 0 );
				// データをフリー
				StreamFreeData( pw, pData );
			}
		}
#if 0
		{	// test
			SJ_GetChunk( pw->sji, SJ_LIN_DATA, 0x7fffffff, &ck );
			printf("data %x-%x\n", ck.data, ck.len);
			SJ_UngetChunk( pw->sji, SJ_LIN_DATA, &ck );
		}
#endif
		
//		mwPlyRelCurFrm( pw->ply );
		ADXM_ExecMain();	//	decoding image data when you select MAIN decode mode
		mwPlyGetCurFrm( pw->ply, &pw->frm );

		if( mwPlyGetStat( pw->ply ) == MWE_PLY_STAT_PLAYEND ) {
			pw->state = STATE_END;
		} else { // デコーダは信頼できないので自分で終了コードを見る
#if 1			// 最終フレームがBピクチャじゃないと起こる？
			u_char code[4];
			if( SJ_GetNumData( pw->sji, SJ_LIN_FREE ) >=4 ) {
				SJ_GetChunk( pw->sji, SJ_LIN_DATA, 4, &ck );
				memcpy( code, ck.data, ck.len );
				if( ck.len < 4 ) {
					SJCK ck2;
					SJ_GetChunk( pw->sji, SJ_LIN_DATA, 4 - ck.len , &ck2 );
					memcpy( code + ck.len, ck2.data, ck2.len );
					SJ_UngetChunk( pw->sji, SJ_LIN_DATA, &ck2 ); 
				}
				SJ_UngetChunk( pw->sji, SJ_LIN_DATA, &ck );
				if( CodeIsEnd( code ) ) pw->state = STATE_END;
			}
#endif			
		}
		break;
	case STATE_END:	// 終了待ち
		if( pw->frm.bufadr == NULL ) {
			printf("MPEG stream end\n");
			// 最初に戻って待機
			pw->state = STATE_READY;
			ReleaseRes( pw );
//			GV_DestroyActor( pw );
			break;
		}
//		FS_StreamDump( pw->pctrl->stream_h );
	}
#if 0	
	outtime = GET_HSYNC_TIMER();
	printf("%3d ", DIFF_HSYNC_TIMER( outtime, intime ) );
#endif	
#endif
}


static void Die( CRI_STR_WORK* pw )
{
#if 0	//@ コンパイルがとおらなかったので修正
	ReleaseRes( pw );

	if( pw->flag & FLAG_MPEG ) {
	//	Shutting down configurations of file system, sound system and thread mechanism
		mwPlyFinishSfdFx();
#ifdef DECODE_OTHER_THREAD		
		ADXM_ShutdownThrd();
		ADXXB_ShutdownSound();
		ADXXB_ShutdownDvdFs();
#endif		
	} else {
		SFVSJD_Finish();
	}

	if ( pWork == pw ){
		pWork = NULL ;
	}
#endif	
}


void errfunc(void *obj, char *msg)
{
//@	printf(msg);
//	HANGUP();
}


static void GetResources( CRI_STR_WORK* pw, GM_STREAM_CONTROL* pctrl, int type )
{
#if 0	//@ コンパイルがとおらなかったので修正

	GV_ActorEX( &pw->actor );
	pWork = pw ;
	pw->type = type;
	pw->pctrl = pctrl;

	pw->pSjd_work = NULL;
	pw->pYuv420Image = NULL;
	pw->pPlyWork = NULL;
	
	pw->sji = pw->sjo = NULL;
	pw->sjd = NULL;
	pw->ply = NULL;
	
	pw->frm.bufadr = NULL;
	pw->ckout.len = 0;

	pw->state = STATE_READY;
#endif				
}

void *NewStreamIpicDriver( GM_STREAM_CONTROL *pctrl, int type )
{
	CRI_STR_WORK* pw = NULL ;
#if 0	//@ コンパイルがとおらなかったので修正

	if( ( pw = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM, sizeof( CRI_STR_WORK ), 0xf2 ) ) != NULL ){
		
		GV_SetActor( &pw->actor, ActIpic, Die );
		GetResources( pw, pctrl, type );
		
		pw->flag = 0;
		
		// SFVSJD イニシャライズ
		SFVSJD_Init();

	}
#endif
	return pw;
}


void *NewMemStreamIpic( void *ipu_data, int mode )
{
	CRI_STR_WORK* pw = NULL ;
#if 0	//@ コンパイルがとおらなかったので修正

	if( ( pw = GV_NewActorPrio( GV_ACTOR_MANAGER, sizeof( CRI_STR_WORK ), 0xf2 ) ) != NULL ){
		
		GV_SetActor( &pw->actor, ActIpic, Die );
		GetResources( pw, NULL, 0 );
		
		pw->flag = FLAG_ON_MEMORY;
		
		pw->pCurrentmem = ipu_data ;
		pw->pTopmem = NULL;
		
		// SFVSJD イニシャライズ
		SFVSJD_Init();
		
	}
#endif
	return pw;
}



void *NewStreamCriMpegDriver( GM_STREAM_CONTROL *pctrl, int type )
{	// 通常ＭＰＥＧの入り口
	CRI_STR_WORK* pw = NULL ;
#if 0	//@ コンパイルがとおらなかったので修正

	if( ( pw = GV_CreateActor( GV_ACTOR_MANAGER, GV_CLASS_SYSTEM, sizeof( CRI_STR_WORK ), 0xf2 ) ) != NULL ){
		extern LPDIRECTSOUND  p_DSound;                          // DirectSound object
		MWS_PLY_INIT_SFD iprm;
		
		GV_SetActor( &pw->actor, ActMpeg, Die );
		GetResources( pw, pctrl, type );
		
		pw->flag = FLAG_MPEG;
#ifdef	DECODE_OTHER_THREAD	
		ADXXB_SetupDvdFs(NULL);
		ADXXB_SetupSound(p_DSound);
		ADXM_SetupThrd(NULL);
#endif		
		ADXM_SetCbErr(errfunc, NULL);
		
		memset(&iprm, 0, sizeof(iprm));
#ifdef PAL        //BP JG - not used as it's if #0
		iprm.vhz			= MWSFD_VHZ_50_00;
#else		
		iprm.vhz			= MWSFD_VHZ_59_94;
#endif		
		iprm.disp_cycle		= 1;
		iprm.disp_latency	= 2;
#ifdef DECODE_OTHER_THREAD		
		iprm.dec_svr		= MWSFD_DEC_SVR_IDLE;
#else		
		iprm.dec_svr		= MWSFD_DEC_SVR_MAIN;
#endif		
		mwPlyInitSfdFx(&iprm);
	}
#endif
	return pw;
}

/* ---------------------------------------------------------------------- */
/*
	ドライバ登録 
*/

/* IPIC,MPEGストリームドライバ登録 */

void GM_StreamCriDriverInit( int dummy )
{
#if 0	//@ コンパイルがとおらなかったので修正
	if( DriverRefCtr++ == 0 ) {
		GM_StreamAddDriver( &driver, CHANK_TYPE_CRI_IPIC );
		GM_StreamAddDriver( &drivermpg, CHANK_TYPE_CRI_MPEG );
	}
#endif
}

/* IPIC,MPEGストリームドライバ停止 */
void GM_StreamCriDriverEnd( void )
{	// Init を呼ばずに End だけ呼ばれることがある
#if 0	//@ コンパイルがとおらなかったので修正
	if( DriverRefCtr > 0 && --DriverRefCtr == 0 ) {
		GM_StreamRemoveDriver( &drivermpg );
		GM_StreamRemoveDriver( &driver );
	}
#endif
}

#if 0
static void AvoidCriBug( void )
{	// 幅が32の倍数でないものを処理。。。
	if( pWork->mh.width & 0x10 ) {
		int srcwidth = pWork->mh.width + 16;
		Uint8* pCurDst;
		Uint8* pCurSrc;
		Uint8* pDataTop;
		int i;
		
		pDataTop = (Uint8*)pWork->ckout.data;
		
		for( i = pWork->mh.height - 1 ; i > 0; i-- ) {
			pCurDst = pDataTop + i * srcwidth;
			pCurSrc = pDataTop + (i & ~7) * srcwidth
					+ (i & 7) * pWork->mh.width;
			memmove( pCurDst, pCurSrc, pWork->mh.width );
		}
		pDataTop += pWork->mh.height * srcwidth;
		for( i = pWork->mh.height / 2 - 1 ; i > 0; i-- ) {
			pCurDst = pDataTop + i * srcwidth / 2;
			pCurSrc = pDataTop + ((i & ~7) * srcwidth
								  + (i & 7) * pWork->mh.width) / 2;
			memmove( pCurDst, pCurSrc, pWork->mh.width / 2 );
		}
		pDataTop += pWork->mh.height * srcwidth / 4;
		for( i = pWork->mh.height / 2 - 1 ; i > 0; i-- ) {
			pCurDst = pDataTop + i * srcwidth / 2;
			pCurSrc = pDataTop + ((i & ~7) * srcwidth
								  + (i & 7) * pWork->mh.width) / 2;
			memmove( pCurDst, pCurSrc, pWork->mh.width / 2 );
		}
	}
}
#else
#define AvoidCriBug()
#endif

/* ---------------------------------------------------------------------- */
/*
	外部呼び出しインターフェイス
*/

/* ストリームの映像幅・高さ・最大フレーム数を取得 */
int GM_StreamGetCriInfo( int *width, int *height, int *frame )
{
	if( pWork == NULL ) return -1;
#if 0	//@ コンパイルがとおらなかったので修正
	if( pWork->state == STATE_READY ) return -1;
	if ( width != NULL ) *width = pWork->mh.width;
	if ( height != NULL ) *height = pWork->mh.height;
	if ( frame != NULL ) *frame = 0;	// 返せない
	
	/* 終了なら真を返す */
#endif
	return FALSE;
}

void* GM_StreamGetCri( void )
{
	if ( pWork == NULL ) return ( NULL );
#if 0	//@ コンパイルがとおらなかったので修正
	if ( pWork->flag & FLAG_MPEG ) {
		return ( pWork->frm.bufadr );
	} else {
		return ( pWork->ckout.data );
	}
#endif
}

/* 展開後Ycc420をラスタイメージに変換 */
void GM_StreamCopyCriImage( void *dst, int dst_width, int dst_height, int dst_x, int dst_y, int flag )
{
	if ( pWork == NULL ) return ;
#if 0	//@ コンパイルがとおらなかったので修正

#if 0	
	if( dst_width - dst_x < pWork->mh.width ||
		dst_height - dst_y < pWork->mh.height ) return ;
#endif	
	if( pWork->flag & FLAG_MPEG ) {
		if ( pWork->frm.bufadr != NULL ) {
			mwPlyYcc420plnToArgb8888( (Uint8*)pWork->frm.bufadr, (Uint8*)dst + dst_x * 4 + dst_y * dst_width * 4,
									  pWork->mh.width, pWork->mh.height, dst_width * 4 );
		}
	} else {
		if ( pWork->ckout.len > 0 ) {
			// 32の倍数の横サイズにしか対応してないっぽい
			AvoidCriBug();
#if 0
			mwPlyYcc420plnToArgb8888( (Uint8*)pWork->ckout.data, (Uint8*)dst + dst_x * 4 + dst_y * dst_width * 4,
									  pWork->mh.width, pWork->mh.height, dst_width * 4 );
#else
			/* PS2版がバグっていたため同じ挙動になるように修正 */
			mwPlyYcc420plnToArgb8888( (Uint8*)pWork->ckout.data, (Uint8*)dst + dst_x * 4 + dst_y * dst_width * 4 / 2,
									  pWork->mh.width, pWork->mh.height, dst_width * 4 );
#endif
		}
	}
#endif
}

/* 展開後バッファの開放 */
void GM_StreamFreeCri( void )
{
	if ( pWork == NULL ) return ;
#if 0	//@ コンパイルがとおらなかったので修正
	if ( pWork->flag & FLAG_MPEG ) {
		if ( pWork->frm.bufadr != NULL ) {
			mwPlyRelCurFrm( pWork->ply );
			pWork->frm.bufadr = NULL;
		}
	} else {		
		if ( pWork->ckout.len > 0 ) {
			SJ_PutChunk( pWork->sjo, SJ_LIN_FREE, &pWork->ckout );
			pWork->ckout.data = NULL;
			pWork->ckout.len = 0;
		}
		if( pWork->flag & FLAG_ON_MEMORY ) {
			// オンメモリの場合はここでポインタを進める
			StreamFreeData( pWork, NULL );
		}
	}
#endif
}

BOOL	GM_StreamIsEndCri( void )
{
	if( pWork == NULL ) return TRUE;
//@@@@	return pWork->state != STATE_DEC;
}
