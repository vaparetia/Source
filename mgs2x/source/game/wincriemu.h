//-----------------------------------------------------------------------------
// File: DShowTextures.h
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 8.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef	_WINCRIEMU_H_
#define	_WINCRIEMU_H_

// MPEG 時のリングバッファサイズ
#define RING_BUF_SIZE	(1024 * 128)
#define RING_BUF_EXTEND_SIZE	(1024 * 32)

#define	OPT_FLAG_PRECIOUS		0x80000000	// スキップ不可データ
#define	OPT_FLAG_NOT_MPEG		0x40000000	// MPEG ストリームではないデータ

#define GET_MPEG_STREAM_SIZE( p ) ( FS_STREAM_GET_OPTION( p ) & ~(OPT_FLAG_PRECIOUS | OPT_FLAG_NOT_MPEG ) )


/*	データラインの定義		*/
/*	Data line definition	*/
#define	SJ_LIN_FREE	(0)			/*	データチャンクライン				*/
								/*	Data chunk line						*/
#define	SJ_LIN_DATA	(1)			/*	フリーチャンクライン				*/
								/*	Free chunk line						*/

#define SJ_ERR_FATAL		(-1)
#define SJ_ERR_INTERNAL		(-2)
#define SJ_ERR_PRM			(-3)

#ifndef UUID_DEFINED
#define UUID_DEFINED
typedef struct _UUID {
    Uint32 Data1;
    Uint16 Data2;
    Uint16 Data3;
    Uint8 Data4[8];
} UUID;
#define uuid_t UUID
#endif

/*	データチャンク	*/
/*	Data chunk		*/
#ifndef SJCK_DEFINED
#define SJCK_DEFINED
typedef struct {
	char *data;					/*	開始アドレス						*/
								/*	Begin address						*/
	int len;					/*	バイト数							*/
								/*	Size of chunk length				*/
} SJCK;
#endif

/*	ライブラリハンドル	*/
/*	SJ handle			*/
typedef struct {
	struct _sj_vtbl *vtbl;		/*	インタフェース						*/
								/*	Interfaces							*/
} SJ_OBJ;
#ifndef SJ_DEFINED
#define SJ_DEFINED
typedef SJ_OBJ *SJ;
#endif


/*	インタフェース関数  				*/
/*	Interface functions					*/
typedef struct _sj_vtbl {
	void (*QueryInterface)();		/*		for COM compatibility			*/
	void (*AddRef)();				/*		for COM compatibility			*/
	void (*Release)();				/*		for COM compatibility			*/
	void (*Destroy)(SJ sj);			/*		ハンドルの消去					*/
									/*		Destroys specified SJ handle	*/
	UUID *(*GetUuid)(SJ sj);		/*		UUIDの取得						*/
									/*		Get a UUID						*/
	void (*Reset)(SJ sj);			/*		リセット						*/
									/*		Resets SJ						*/
	void (*Lock)(SJ sj);			/*		リセット						*/
									/*		Resets SJ						*/
	void (*Unlock)(SJ sj);			/*		リセット						*/
									/*		Resets SJ						*/
	/*	チャンクの取得	(FIFOの先頭から取得)								*/
	/*	Get a chunk	(Get chunk from top of FIFO)							*/
	void (*GetChunk)(SJ sj, int id, int nbyte, SJCK *ck);
	/*	チャンクを戻す　(FIFOの先頭に挿入)									*/
	/*	Unget a chunk	(Insert chunk in top of FIFO)						*/
	void (*UngetChunk)(SJ sj, int id, SJCK *ck);
	/*	チャンクを挿入	(FIFOの最後に挿入)									*/
	/*	Put a chunk	(Insert chunk in last of FIFO)							*/
	void (*PutChunk)(SJ sj, int id, SJCK *ck);
	/*	取得できるデータのバイト数の取得									*/
	/*	Get the data number which can be got								*/
	int (*GetNumData)(SJ sj, int id);
	/*	チャンクの取得可能の判定											*/
	/*	Check whether can get chunk											*/
	int (*IsGetChunk)(SJ sj, int id, int nbyte, int *rbyte);
	/*	エラー発生時に起動する関数の登録									*/
	/*	Entry error callback function										*/
	void (*EntryErrFunc)(SJ sj,
					void (*func)(void *obj, int ecode), void *obj);
} SJ_IF;
typedef SJ_IF *SJIF;
/*	チャンクの結合モード		*/
/*	Combination mode of chunk	*/
#define	SJUNI_MODE_SEPA			(0)
#define	SJUNI_MODE_JOIN			(1)

#define	SJUNI_CALC_WORK(nck)	((nck)*16)

#define MWSFD_VHZ_59_94		(59.94f)
#define MWSFD_VHZ_50_00		(50.00f)

/* デコードサーバ */
/* Decode Server  */
typedef enum {
	MWSFD_DEC_SVR_IDLE		= 0,	/* メインの余り時間でデコードする。 */
									/* Decode in idel thread            */
	MWSFD_DEC_SVR_MAIN		= 1,	/* メイン処理内でデコードする。		*/
									/* Decode in main thread            */

	MWSFD_DEC_SVR_END
} MwsfdDecSvr;

/* Sofdecの初期化パラメータ構造体							*/
/* Parameter structure of Sofdec initialization function	*/
typedef struct {
	float		vhz;			/* 垂直同期周波数[Hz]						*/
								/* Vsync frequency[Hz]						*/
	int		disp_cycle;		/* 表示更新周期[v]				 			*/
								/* Display update cycle[v]					*/
	int		disp_latency;	/* 表示レイテンシ[v]						*/
								/* Display latency[v]						*/
	MwsfdDecSvr	dec_svr;		/* デコードサーバ				 			*/
								/* Decode Server							*/
	int		rsv[4];			/* 予約 (全て0を設定して下さい) 			*/
								/* Reserved(Please set 0 in all of area)	*/
} MWS_PLY_INIT_SFD;

/* ---------------------------------------------------------------- */
/* フレーム情報構造体 */
/* Frame Information  */
typedef struct {
	BYTE		*bufadr;			/* MPEGフレームバッファアドレス		*/
//	IMediaSample *sample;			/* MPEGフレームバッファアドレス		*/
										/* Frame Buffer Address				*/
	DWORD		buffmt;				/* MPEGフレームバッファデータ形式	*/
										/* Frame Buffer Format				*/
	int			width;				/* 横ピクセル数						*/
										/* Width by the pixel				*/
	int			height;				/* 縦ピクセル数						*/
										/* Height by the pixel				*/
	int			x_mb;				/* 横マクロブロック数				*/
										/* Width by the macroblock			*/
	int			y_mb;				/* 縦マクロブロック数				*/
										/* Height by the macroblock			*/
	DWORD		ptype;				/* ピクチャタイプ					*/
										/* Picture type						*/
	int			fps;				/* フレームレート[fps * 1000]		*/
										/* Frame rate [fps * 1000]			*/
	int			fno;				/* フレーム番号						*/
										/* Count of frames					*/
	int			time;				/* 表示時刻							*/
										/* Time of disp						*/
	int			tunit;				/* 表示時刻単位						*/
										/* Unit of display time				*/
	int			concat_cnt;			/* 連結処理回数						*/
										/* Count of concatenation			*/
	int			fno_per_file;		/* ファイル毎のフレーム番号			*/
										/* Count of frames per file			*/
	int			time_per_file;		/* ファイル毎の再生時刻				*/
										/* Playtime per file				*/
	int			errcnt;				/* データエラー発生回数				*/
										/* Count of data error				*/
	int			rcvcnt;				/* データエラー回復回数				*/
										/* Count of error recovery			*/
} MWS_PLY_FRM;

enum {
	FLAG_ON_MEMORY	= 0x01,
	FLAG_MPEG		= 0x02,
	FLAG_NO_SKIP	= 0x04,
};

typedef	enum _mwe_ply_stat {
	MWE_PLY_STAT_STOP		= 0,		/*	stoped						*/
	MWE_PLY_STAT_PREP		= 1,		/*	preparing					*/
	MWE_PLY_STAT_PLAYING	= 2,		/*	playing						*/
	MWE_PLY_STAT_PLAYEND	= 3,		/*	end of playing				*/
	MWE_PLY_STAT_ERROR		= 4,		/*	error was occured			*/

	MWE_PLY_STAT_END
} MWE_PLY_STAT;




/* ファイルタイプ		*/
/* File type		*/
enum {
	MWD_PLY_FTYPE_NON,
	MWD_PLY_FTYPE_SFD,
	MWD_PLY_FTYPE_MPV
};



/*	ライブラリハンドル	*/
/*	SJ handle			*/
typedef struct {
	struct _sj_vtbl *vtbl;		/*	インタフェース						*/
								/*	Interfaces							*/
	/* デコーダー用 */
} SFVSJD_OBJ;
typedef SFVSJD_OBJ *SFVSJD;



enum {
	SFVSJD_STAT_DECEND
};



#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


extern void SJ_Reset( SJ sjo );

#define SJ_GetChunk(sj, id, nbyte, ck) \
		(*(sj)->vtbl->GetChunk)(sj, id, nbyte, ck)
#define SJ_PutChunk(sj, id, ck) \
		(*(sj)->vtbl->PutChunk)(sj, id, ck)
#define SJ_GetNumData(sj, id) \
		(*(sj)->vtbl->GetNumData)(sj, id)
#define SJ_UngetChunk(sj, id, ck) \
		(*(sj)->vtbl->UngetChunk)(sj, id, ck)
#define SJ_Destroy(sj) \
		(*(sj)->vtbl->Destroy)(sj)
#define SJ_Reset(sj) \
		(*(sj)->vtbl->Reset)(sj)

#define SJ_Lock(sj) \
		(*(sj)->vtbl->Lock)(sj)
#define SJ_Unlock(sj) \
		(*(sj)->vtbl->Unlock)(sj)





extern int SFVSJD_CalcWork( LPVOID );
extern SJ SJUNI_Create( int mode, char *work, int size );
extern SFVSJD SFVSJD_Create( SJ sji , SJ sjo, LPVOID, char *pSjd_work, int sjd_worksize );
extern void SFVSJD_Start( SFVSJD sjd );
extern void SFVSJD_ExecHndl( SFVSJD sjd );
extern int SFVSJD_GetStat( SFVSJD sjd );



extern void SFVSJD_Finish(void);
extern void SFVSJD_Init(void);
extern void SFVSJD_Destroy(SFVSJD 	sj);




#ifdef __cplusplus
}
#endif // __cplusplus

















#endif
