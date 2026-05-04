/*
	memcard_access.h
		メモリーカード制御サブルーチン
	
	2001/01/15 K.Kano
	$Id: memcard_access.h,v 1.1.1.3 2002/11/19 11:43:18 Yoshizawa1 Exp $
*/


#ifndef _memcard_access_h_
#define _memcard_access_h_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#define __SCE_MC_OLD_DEFINE__	// ADD M.Kobayashi for scelib 2.4.x

//#if defined(MGS2_VRTRIAL) || defined(KP_XBOX)
#define SCENARIO_SET_SONYCODE	// シナリオで製品コードを設定するときにdefine
//#endif

#include <libmc.h>
#endif


#include	"gameheader.h"
#include	"libutl.h"

#ifdef KP_XBOX
typedef	struct sceMcTblGetDir {
	FILETIME _Create;
	FILETIME _Modify;
	unsigned FileSizeByte;
	unsigned short AttrFile;
	unsigned short Reserve1;
	unsigned Reserve2;
	unsigned PdaAplNo;
	unsigned char EntryName[32];
} sceMcTblGetDir;

#else //BP

#include "BP_SaveLoad.h"

#endif


/* ------------------------------------------------------------------------ */


#define P_FILE_VER			"000"
#define G_FILE_VER			"000"
#define V_FILE_VER			"000"
#define S_FILE_VER			"000"

#ifdef SCENARIO_SET_SONYCODE

#define _MC_PFILE_NAME		"%s" P_FILE_VER "P"
#define _MC_GFILE_NAME		"%s" G_FILE_VER "G"
#define _MC_VFILE_NAME		"%s" V_FILE_VER "V"
#define _MC_SFILE_NAME		"%s" S_FILE_VER "S"
#define _MC_AFILE_NAME		"%s" "???" "?"
#define MC_PFILE_NAME		GetPFilename()
#define MC_GFILE_NAME		GetGFilename()
#define MC_VFILE_NAME		GetVFilename()
#define MC_SFILE_NAME		GetSFilename()
#define MC_AFILE_NAME		GetAFilename()
#else	// SCENARIO_SET_SONYCODE

#include "sonycode.h"

#define PRODUCT_CODE		SONY_CODE

#ifdef JAPANESE_BP_IGNORE()

/* 日本版ファイル名 */
#define MC_PFILE_NAME		"BI" PRODUCT_CODE P_FILE_VER "P"
#define MC_GFILE_NAME		"BI" PRODUCT_CODE G_FILE_VER "G"
#define MC_VFILE_NAME		"BI" PRODUCT_CODE V_FILE_VER "V"	// VR
#define MC_SFILE_NAME		"BI" PRODUCT_CODE S_FILE_VER "S"	// Snake Tales

#endif

#if defined(ENGLISH) && defined(NTSC)

/* 米国版ファイル名 */
#define MC_PFILE_NAME		"BA" PRODUCT_CODE P_FILE_VER "P"
#define MC_GFILE_NAME		"BA" PRODUCT_CODE G_FILE_VER "G"
#define MC_VFILE_NAME		"BA" PRODUCT_CODE V_FILE_VER "V"	// VR
#define MC_SFILE_NAME		"BA" PRODUCT_CODE S_FILE_VER "S"	// Snake Tales

#endif

#ifdef EU

/* 欧州版ファイル名 */
#define _MC_PFILE_NAME		"BE" "%s" P_FILE_VER "P"
#define _MC_GFILE_NAME		"BE" "%s" G_FILE_VER "G"
#define _MC_VFILE_NAME		"BE" "%s" V_FILE_VER "V"
#define _MC_SFILE_NAME		"BE" "%s" S_FILE_VER "S"
#define MC_PFILE_NAME		GetPFilename()
#define MC_GFILE_NAME		GetGFilename()
#define MC_VFILE_NAME		GetVFilename()
#define MC_SFILE_NAME		GetSFilename()

#endif

#endif	// SCENARIO_SET_SONYCODE

#define MC_PFILE_NAME_LEN	(2+10+3+1)
#define MC_GFILE_NAME_LEN	(2+10+3+1)


/* ------------------------------------------------------------------------ */


#define STEP_LEVEL_MAX	2

#define PORT_MAX		2
#define SLOT_MAX		4

#define RETRY_COUNT_MAX			5

// #define PORTCHECK_COUNT_MAX		10
#define PORTCHECK_COUNT_MAX		5


#define MCACC_WAIT			0
#define MCACC_SUCCESS		1
#define MCACC_ERROR			-1
#define MCACC_NEED_CHECK	-2


#define MCACC_CARDTYPE_NOCARD				0
#define MCACC_CARDTYPE_PS1					1
#define MCACC_CARDTYPE_PS2					2
#define MCACC_CARDTYPE_POCKET_STATION		3
#define MCACC_CARDTYPE_ERRORCARD			-1


#define MC_LOADDATA_NOID		0x8000

#ifdef __cplusplus
#define delete delete_type
#endif

typedef struct {
	int step[STEP_LEVEL_MAX];

	int Type[PORT_MAX][SLOT_MAX];
	int FreeSize[PORT_MAX][SLOT_MAX];
	int Formatted[PORT_MAX][SLOT_MAX];
	int OldType[PORT_MAX][SLOT_MAX];

	int TmpType;

	int SlotMax[PORT_MAX];

	int SelPort,SelSlot;
	int RetryCount;
	int Result;

	int AutoPortCheck_Counter;
	int CheckSeqFlag;

	int fd;

	union {
		struct {
			int fd;
			void *buf;
			int size;
			int *retsize;
		} rw;

		struct {
			char *name;
			int mode;
			int *fd;
		} open;

		struct {
			int fd;
		} close;

		struct {
			int fd;
			int offset;
			int mode;
			int *fp;
		} seek;

		struct {
			char *name;
		} mkdir;

		struct {
			char *name;
			char *pwd;
		} chdir;

		struct {
			char *name;
			int table_size;
			sceMcTblGetDir *table;
			int *ret_table_size;
		} getdir;

		struct {
			char *name;
		} delete;

		struct {
			char *org;
			char *next;
		} rename;

		struct {
			char *name;
			sceMcTblGetDir *table;
			unsigned int valid;
		} chmod;
	} arg;

#ifdef KP_XBOX
	HANDLE	hThread;	// スレッドハンドル
	HANDLE	hEvent;		// イベントハンドル

#endif	

} MCAccessWork;

#ifdef __cplusplus
#undef delete
#endif

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

/* 各種ルーチンのスタート関数
   0以外が返ってくるまで呼び出すこと。*/

/* システム用
   オートチェック機構が使用する。*/
int MCAccessPrecheckStart(MCAccessWork *work);

/* メモリーカードポートの状態のチェック */
int MCAccessCheckStart(MCAccessWork *work);

/* メモリーカード内のファイルのアクセスの開始/終了を宣言 */
int MCAccessOpenStart(MCAccessWork *work,int port,int slot,char *name,int mode,int *fd);
int MCAccessCloseStart(MCAccessWork *work,int fd);

/* ファイルアクセス用関数 */
int MCAccessSeekStart(MCAccessWork *work,int fd,int offset,int mode,int *fp);
int MCAccessReadStart(MCAccessWork *work,int fd,void *buf,int size,int *retsize);
int MCAccessWriteStart(MCAccessWork *work,int fd,void *buf,int size,int *retsize);

/* メモリーカードのディレクトリ構造にアクセスする関数 */
int MCAccessMkdirStart(MCAccessWork *work,int port,int slot,char *name);
int MCAccessChdirStart(MCAccessWork *work,int port,int slot,char *name,char *pwd);
int MCAccessGetdirStart(MCAccessWork *work,int port,int slot,
						char *name,sceMcTblGetDir *table,int table_size,int *ret_size);
int MCAccessGetdirNextStart(MCAccessWork *work,int port,int slot,
							char *name,sceMcTblGetDir *table,int table_size,int *ret_size);

/* その他 */
int MCAccessDeleteStart(MCAccessWork *work,int port,int slot,char *name);
int MCAccessRenameStart(MCAccessWork *work,int port,int slot,char *org,char *next);
int MCAccessChmodStart(MCAccessWork *work,int port,int slot,
					   char *name,sceMcTblGetDir *table,unsigned int valid);

/* メモリーカードをフォーマット/アンフォーマットする関数 */
int MCAccessFormatStart(MCAccessWork *work,int port,int slot);
int MCAccessUnformatStart(MCAccessWork *work,int port,int slot);



/* メモリーカードの制御状態を読み出す関数 */

/* メモリーカードアクセス関数のアクセス状況を知る */
int MCAccessing(MCAccessWork *work);

/* メモリーカードアクセス関数の動作結果を取得 */
int MCAccessGetResult(MCAccessWork *work);

/* オートチェック機構を稼働させるかどうかを選択 */
void MCAccessAutoCheck(MCAccessWork *work,int flag);

/* 現在差さっているメモリーカードの種類を取得 */
int MCAccessGetType(MCAccessWork *work,int port,int slot);

/* 以前差さっていたメモリーカードと違っているかどうかを取得 */
int MCAccessIsMCChanged(MCAccessWork *work,int port,int slot);

/* メモリーカードの使用可能領域を取得 */
int MCAccessGetFreeSize(MCAccessWork *work,int port,int slot);

/* メモリーカードがフォーマット済みかどうかを取得 */
int MCAccessIsFormatted(MCAccessWork *work,int port,int slot);

/* メモリーカードポートに差せるメモリーカードの数を取得
   マルチタップ対応用 */
int MCAccessGetSlotMax(MCAccessWork *work,int port);

/* オートチェックが行われたかどうかを取得 */
int MCAccessCheckSeqFlag(MCAccessWork *work);

/* オートチェック中かどうかを取得 */
int MCAccessAutoChecking(MCAccessWork *work);



/* メモリーカードアクセスのメイン関数 */
void MCAccessInit(MCAccessWork *work,int flag);
void MCAccessEnd(MCAccessWork *work);
void MCAccessAct(MCAccessWork *work);
void MCAccessPostAct(MCAccessWork *work);


/* アイコンサイズを計算する関数 */
int MCAccessCalcIconSize(void *icon);


/* 直前にアクセスしたデータ箇所の記録 */
extern unsigned short mc_loadport;
extern unsigned short mc_loaddata_id;
extern unsigned int mc_loaddata_time;

static inline void SetLoaddataID(int port,int id)
{
	mc_loadport=port;
	mc_loaddata_id=id;
}

static inline void ResetLoaddataID(void)
{
	mc_loadport=MC_LOADDATA_NOID;
	mc_loaddata_id=MC_LOADDATA_NOID;
}

static inline int GetLoaddataPort(void)
{
	return mc_loadport;
}

static inline int GetLoaddataID(void)
{
	return mc_loaddata_id;
}

static inline void SetLoaddataTime(unsigned char *date)
{
	unsigned int dt=0;
	int i;
	unsigned int days;
	extern const unsigned char conv_month_day[];
	int year,month,day,hour,min,sec;

	/* BCDではない */
	year=*(date+0);
	month=*(date+1);
	day=*(date+2);
	hour=*(date+3);
	min=*(date+4);
	sec=*(date+5);


	/* year */
	dt=year;
	dt*=365;

	/* month,day */
	days=0;
	for(i=1;i<month;i++){
		days+=conv_month_day[i];
	}
	days+=day;

	/* 閏年 */
	days+=(year+3)/4;
	if((year % 4)==0){
		if(month>2) days++;
	}
	dt+=days;
	dt*=24;

	/* hour */
	dt+=hour;
	dt*=60;

	/* min */
	dt+=min;
	dt*=60;

	/* sec */
	dt+=sec;

#ifdef DEBUG_MODE
	printf("Set Load Data Date = %d\n",dt);
#endif

	mc_loaddata_time=dt;
}


/* ファイル名の取得 */
char *GetPFilename(void);
char *GetGFilename(void);
char *GetVFilename(void);
char *GetSFilename(void);
char *GetAFilename(void);

#endif
