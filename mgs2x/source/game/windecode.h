//-----------------------------------------------------------------------------
// File: DShowTextures.h
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 8.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#ifndef	_WINDECODE_H_
#define	_WINDECODE_H_
//#include <streams.h>

/*	Virtual function table	*/
typedef struct {
	struct _mwply_if *vtbl;
} MW_PLY_OBJ;
typedef MW_PLY_OBJ *MWPLY;
/*	MWPLY Interface function  	*/
typedef struct _mwply_if {
	void (*QueryInterface)();		/*	for COM compatibility	*/
	void (*AddRef)();				/*	for COM compatibility	*/
	void (*Release)();				/*	for COM compatibility	*/
	/*== Functions of MWPLY	====================================*/ 
	/*	V-sync function 										*/
	/*	This function is called by user while v-sync interrupt.	*/
	void (*VsyncHndl)(MWPLY mwply);
	/*	execute decoding one frame								*/
	/*	This function is called by user from main-loop.			*/ 
	int (*ExecSvrHndl)(MWPLY mwply);
	/*	destroy MWPLY handle									*/
	void (*Destroy)(MWPLY mwply);
	/*	start playback by file name 							*/
	void (*StartFname)(MWPLY mwply, char *fname);
	/*	stop playback  											*/
	void (*Stop)(MWPLY mwply);
	/*	get status of MWPLY handle								*/
	MWE_PLY_STAT (*GetStat)(MWPLY mwply);
	/*	get playing time										*/
	/*	if playing movie is 30 fps, *tscale is 30 and 			*/
	/*		*ncount is total number of displayed frames			*/ 
	void (*GetTime)(MWPLY mwply, int *ncount, int *tscale);
	/*	Set pause switch.  sw=0(Continue), 1(Pause)				*/
	void (*Pause)(MWPLY mwply, int sw);
	/*	Set Output Volume (vol= 0 to -960)						*/
	void (*SetOutVol)(MWPLY mwply, int vol);
	/*	Get Output Volume (return value= 0 to -960)				*/
	int (*GetOutVol)(MWPLY mwply);
	/*	Set output panpot chno=0(Mono/Left),1(Right)			*/
	/*	pan = +15(Right),0(Center),-15(Left),-128(Auto)			*/
	void (*SetOutPan)(MWPLY mwply, int chno, int pan);
	/*	Get output panpot chno=0(Mono/Left),1(Right)			*/
	int (*GetOutPan)(MWPLY mwply, int chno);
	/*	start playing by stream joint							*/
	void (*StartSj)(MWPLY mwply, SJ sji);
	/*	start playing by memory									*/
	void (*StartMem)(MWPLY mwply, void *addr, int len);
} MWD_IF;

/* ハンドル生成パラメータ構造体				*/
/* Parameter structure of handle creation	*/
typedef struct {
	int	ftype;				/* 再生するストリームの種別 				*/
								/* File type								*/
	int	max_bps;			/* 最大のビットストリーム量　(bit/sec)		*/
								/* Maximum number of bit per second			*/
	int	max_width;			/* 再生画像サイズの最大幅					*/
								/* Maximum width of video stream			*/
	int	max_height;			/* 再生画像サイズの最大高さ					*/
								/* Maximum height of video stream			*/
	int	nfrm_pool_wk;		/* システム領域のフレームプール数（通常3)	*/
								/* フレーム落ちが発生した場合は、この値を	*/
								/* 増やしてください。						*/
								/* Number of frame pools in system memory.	*/
								/* Normaly this number is 3. If frame is 	*/
								/* droped,you have to increase this number.*/
	int	max_stm;			/* 同時読み込みストリーム数(ADX含む)		*/
								/* 値が0の場合はデフォルト値(1)とみなす。	*/
								/* The number of maximum streams playing 	*/
								/* at the same time. 						*/
								/* This number include ADXT/ADXF streams.	*/
								/* If the value is zero then assume 1.		*/
	char	*work;				/* ワーク領域								*/
								/* Address of working area					*/
	int	wksize;				/* ワーク領域サイズ							*/
								/* Size of working area						*/
	int  compo_mode;			/* 合成モード								*/
								/* Composition mode							*/
	int	rsv[3];				/* 予約 (全て0を設定して下さい) 			*/
								/* Reserved(Please set 0 in all of area)	*/
} MWS_PLY_CPRM_SFD;
//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

extern void mwPlyInitYcc420plnToArgb8888(void);		// Initialize convert table


extern int mwPlyCalcWorkCprmSfd(MWS_PLY_CPRM_SFD *cprm);
extern MWPLY mwPlyCreateSofdec(MWS_PLY_CPRM_SFD *cprm,int mode);
extern SJ mwPlyGetInputSj(MWPLY mwply);

//#define mwPlyStartSj(mwply, sji)     (*(mwply)->vtbl->StartSj)((mwply), (sji))
#define mwPlyStartSj(mwply, sji)     {}


extern int ADXM_ExecMain(void);
extern int mwPlyGetCurFrm(MWPLY mwply, MWS_PLY_FRM *frm,int tick,int send_size);
extern int mwPlyGetStat( MWPLY mwply );


extern void mwPlyFinishSfdFx(void);
extern void ADXM_SetCbErr(void*,void*);
extern void mwPlyRelCurFrm(MWPLY mwply);
extern void mwPlyInitSfdFx(MWS_PLY_INIT_SFD *iprm);
extern void mwPlyDestroy( MWPLY mwply);

extern 	int WinstrmSendIPic(MWPLY mwply, MWS_PLY_FRM *frm,BYTE *src,int size,int code);


extern void WindowsMpegInit(void);
extern void WindowsMpegRelease(void);
typedef	int	( *SEARCH_CALLBACK )( LPBYTE lpsrc, DWORD size ,DWORD	pos, void *work) ;
int  SearchChankCode(LPBYTE lpsrc, DWORD size ,SEARCH_CALLBACK func, void *work);


#ifdef __cplusplus
}
#endif // __cplusplus




#endif
