/*
	mcman.c
		メモリーカード制御アクター
	
	2001/01/12 K.Kano
	$Id: mcman.h,v 1.1.1.3 2002/11/19 11:43:16 Yoshizawa1 Exp $
*/


#ifndef _mcman_h_
#define _mcman_h_


#ifdef PSX2
#include <libcdvd.h>
#endif

#ifdef KP_XBOX
#include	<xtl.h>

#define		SCE_RDONLY	0x0001
#define		SCE_WRONLY	0x0002
#define		SCE_RDWR	0x0003
#define		SCE_CREAT	0x0200
#endif

#include "../memcard_checker/memcard_access.h"

#include "mcman_sjis.h"

#include "../../skoba/equip/capture.h"

#include "../../yamashita/2D/msn.h"


typedef struct {
	/* ヘッダ */
	int version;

	/* モデル */
	int nbsp;
	int attrib;
	float bface;
	int nbvtx;
	struct {
		short vtx[1][4];
		short normal[4];
		short st[2];
		unsigned char color[4];
	} poly[4][3];

	/* アニメーション */
	int nbseq;
	struct {
		int nbframe;
		float speed;
		int offset;
		int nbksp;
		struct {
			int kspid;
			int nbkf;
			struct {
				float frame;
				float weight;
			} keys[1];
		} ksp[1];
	} seq[1];
} DEFAULT_ICON_HEADER;

/* 四角形ポリゴンにテクスチャを張った構造のアイコンのモデル部分 */
extern DEFAULT_ICON_HEADER default_icon_header;

/* 写真アイコン用のテクスチャのサイズ
   <圧縮後のサイズを格納>
   +( <非圧縮データであることを示すヘッダ>+<1ライン上のpixel数>*<pixel単位の色のサイズ>
      +<圧縮データであることを示すヘッダ>+<ダミーデータ> )*<ライン数>
 */
#define ICON_TEX_WIDTH		THUMB_W
#define ICON_TEX_HEIGHT		THUMB_H
#define ICON_TEXTURE_SIZE	((2+ICON_TEX_WIDTH*2+2+2)*ICON_TEX_HEIGHT)

#define DEFAULT_ICON_SIZE	(sizeof(DEFAULT_ICON_HEADER)+4+ICON_TEXTURE_SIZE)


// #define FILE_MAX		1000
// #define FILE_MAX		500

#define DATA_FILE_MAX		50
#define VR_FILE_MAX			1
#define SNAKE_TALES_FILE_MAX	5
#define PHOTO_FILE_MAX		50
#define FILE_MAX			PHOTO_FILE_MAX

#define INFOCODE_SIZE	16
#define INFOCODE2_SIZE	(INFOCODE_SIZE+4)
#define CHARCODE_SIZE	(((INFOCODE2_SIZE+1)*8+5)/6)		/* チェックサム付き */
#define DATETIME_SIZE	6	/* yymmdd hhmmss */

#define FILE_PAGE_SIZE			50
#define PHOTO_FILE_PAGE_SIZE	10

enum {
	DATETIME_INDEX_YEAR=0,
	DATETIME_INDEX_MONTH,
	DATETIME_INDEX_DAY,
	DATETIME_INDEX_HOUR,
	DATETIME_INDEX_MIN,
	DATETIME_INDEX_SEC,
};

#define ICON_FILENAME		"mgs2.ico"
#define ICONSYS_FILENAME	"icon.sys"


#define GET_RTC_COUNT		DIRECT_TICK(30)


struct _MCMAN_WORK;

typedef struct {
	GV_ACT_EX actor;
	struct _MCMAN_WORK *mcman_work;
} MCSubActWork;

typedef struct _MCMAN_WORK {
	GV_ACT_EX actor;
	MCAccessWork accwork;

	int flag;

	int mode;
	int store_mode;
	int id;
	int step,sub_step;
	int store_step;
	int result;
	int fd;
	int ret_size;

	int port,slot;
	int store_port,store_slot;

	void *cur_data;
	int cur_size;
	
	void *icondata;
	int iconsize;
	void *data;
	int datasize;
	unsigned char info[INFOCODE_SIZE];

	char path[0x80];

#ifdef PSX2	
	sceMcIconSys iconsys;
	unsigned int iconwork[(DEFAULT_ICON_SIZE+sizeof(int)-1)/sizeof(int)];
#endif	

	int n_files;
	int max_id;
	int checking_id;
	int readable_n_files;
	int checked_limit;

	unsigned int file_flags[(FILE_MAX*2+31)/32];
	unsigned char file_info[FILE_MAX][INFOCODE2_SIZE];
	unsigned char date_time[FILE_MAX][DATETIME_SIZE];

   //BP - increased from 64 to 128 to prevent case where sceMcGetDir() is called with mode 1 which we didn't know we needed to support
	sceMcTblGetDir table[128];
	int table_used_size;

	// MCSubActWork mcsub_work;

	int easy_search_n_files[PORT_MAX];
	int easy_search_max_id[PORT_MAX];

	int rtc_count;

	// sceCdCLOCK rtc;

	int	file_kind;	// 扱うファイルの種類

	unsigned char	exist_flag;	// ファイル存在フラグ/初期チェックで使用
} MCMAN_WORK;

enum {
	MCMAN_FILE_KIND_GAME,
	MCMAN_FILE_KIND_PHOTO,
	MCMAN_FILE_KIND_VR,
	MCMAN_FILE_KIND_SNAKE_TALES,
#ifdef PSX2
	MCMAN_FILE_KIND_OTHER,	// 他タイトルのファイルを見る（個数チェックのみ使用）
	MCMAN_FILE_KIND_FIRSTCHECK,	// 最初のチェック
#endif
	MCMAN_FILE_KIND_MAX,
};

enum { 
	MCMAN_EXIST_FLAG_GAME			= 0x01,
	MCMAN_EXIST_FLAG_PHOTO			= 0x02,
	MCMAN_EXIST_FLAG_VR				= 0x04,
	MCMAN_EXIST_FLAG_SNAKE_TALES	= 0x08,
};

enum {
	MCMAN_FLAG_MCCHECKED=0x0001,
	MCMAN_FLAG_MCCHECKING=0x0002,
	MCMAN_FLAG_MCFORMATTING=0x0004,

	MCMAN_FLAG_NEXT_REQUEST=0x0008,

	MCMAN_FLAG_PAUSE_STEP=0x0010,
	// MCMAN_FLAG_PAUSE_GAME_INFO=0x0010,
	// MCMAN_FLAG_PAUSE_PHOTO_INFO=0x0020,

	MCMAN_FLAG_ALREADY_RETURN_RESULT=0x0040,

	MCMAN_FLAG_CHECK_BACKGROUND=0x0080,

	MCMAN_FLAG_CHECK_TO_BACK=0x0100,
};


enum {
	MCMAN_CARDTYPE_NOCARD=MCACC_CARDTYPE_NOCARD,
	MCMAN_CARDTYPE_PS1=MCACC_CARDTYPE_PS1,
	MCMAN_CARDTYPE_POCKET_STATION=MCACC_CARDTYPE_POCKET_STATION,

	MCMAN_CARDTYPE_PS2_FORMATTED=MCACC_CARDTYPE_PS2,
	MCMAN_CARDTYPE_PS2_UNFORMATTED=8,

	MCMAN_CARDTYPE_ERRORCARD=MCACC_CARDTYPE_ERRORCARD,

	MCMAN_CARDTYPE_MULTITAP=-2,
};

#ifdef __cplusplus
extern "C" {
#endif

/* メモリーカードアイコンテクスチャの圧縮ルーチン */
int MCIcon_RleEncode(unsigned char *in_data, unsigned int size, 
					 unsigned char *out_data, unsigned int *out_size);


/* 48x42のアイコンテクスチャを、アイコンデータに変換して規定のメモリに展開する。
   この関数を使った場合、SavePhotoStart/SaveGameStartではicondataにNULLを指定する */
void MCMan_PhotoIconEncode(void *icon);

/* 規定のメモリにあるアイコンデータを、48x42のアイコンテクスチャに変換する。
   LoadPhotoIconStartでicondataにNULLを指定した場合は、規定のメモリにアイコンデータが
   Loadされるので、この関数を用いてテクスチャを取得する。*/
void MCMan_PhotoIconDecode(void *icon);



/* icon.sysファイルの各種項目の設定
   Saveの際に必ずicon.sysは更新されるので、ここの値をセットしてSaveを行うこと */
void SetIconSysTitle(const char * const title,const int lf);
void SetIconSysTransrate(const int tr);
void SetIconSysBgColor(const int * const lu,const int * const ru,
					   const int * const ld,const int * const rd);
void SetIconSysLight(const FVECTOR * const l1,const FVECTOR * const l2,const FVECTOR * const l3,
					 const FVECTOR * const c1,const FVECTOR * const c2,const FVECTOR * const c3);
void SetIconSysLightDir(int index,const FVECTOR * const l);
void SetIconSysLightColor(int index,const FVECTOR * const c);
void SetIconSysAmbient(const FVECTOR * const c);


/* メモリーカードのチェックシーケンスを通ると、一回だけ１が返ってくる */
int MCManChecked(void);

/* 現在メモリーカードのチェック中もしくはチェック終了
   メモリーカードのチェックシーケンスフラグをクリアしないので、
   チェックシーケンスを通過後は、毎回１が返ってくる
   フラグのクリアには、MCManChecked()を使う */
int MCManCheckingOrChecked(void);

/* 現在メモリーカードフォーマット中 */
int MCManFormatting(void);

/* メモリーカードの種類を返す
   マルチタップには対応していないので、マルチタップが差さっていることのみを返す。
   MCMAN_CARDTYPE_NOCARD          差さっていない
   MCMAN_CARDTYPE_PS1             PS1
   MCMAN_CARDTYPE_PS2_FORMATTED   PS2 Formatted
   MCMAN_CARDTYPE_POCKET_STATION  Pocket Station

   MCMAN_CARDTYPE_PS2_UNFORMATTED PS2 Unformatted

   MCMAN_CARDTYPE_ERRORCARD       Error
   MCMAN_CARDTYPE_MULTITAP        Multi-tap
   */
int MCManGetCardType(int port);

/* 前回チェックしたメモリーカードと同じものが差さっているかどうか確認する。
   変更されていれば、0以外の値を返す。*/
int MCManIsMCChanged(int port);

/* メモリーカードの残り空き容量を返す */
int MCManGetFreeSize(int port);

/* 各種シーケンスの結果取得 */
int MCManGetResult(void);

int MCManAccessing(void);

/* GetPhotoInfo、GetFileInfoで取得したメモリーカードの情報を取り出す。*/
int MCManNFiles(void);
int MCManMaxId(void);
int MCManCheckFileFlag(int index);
int BPX360_MCManCheckFileFlag_DoNotCareIfCorrupt(int index);
int MCManCheckExactFileFlag(int index);
void MCManSetDamagedFlag(int index);
void *MCManGetFileInfo(int index);
void *MCManGetDateTime(int index);

int MCManNFilesE(int port);
int MCManMaxIdE(int port);

int MCManExactNFiles(void);
int MCManCheckingID(void);

int MCManFileFlagReset(void);


/* 写真ファイルのセーブシーケンスの開始 */
int SavePhotoStart(int port,int id,void *photodata,int photosize,
				   void *icondata,int iconsize,void *info);

/* 写真ファイルのロードシーケンスの開始 */
int LoadPhotoStart(int port,int id,void *photodata);

/* 写真アイコンファイルのロード */
int LoadPhotoIconStart(int port,int id,void *icondata);

/* 写真ファイルの情報取得の開始 */
int GetPhotoInfoStart(int port);
int GetPhotoInfoBGStart(int port);
int GetPhotoInfoRBGStart(int port);
int GetPhotoPageInfoStart(int port,int id,int size);
int GetPhotoInfoOnlyBGStart(int port);
int GetPhotoInfoOnlyRBGStart(int port);

/* info部分のみの変更を開始 */
int ChangePhotoInfoStart(int port,int id,void *newinfo);

/* ゲームファイルのセーブシーケンスの開始 */
int SaveGameStart(int port,int id,void *data,int size,
				  void *icondata,int iconsize,void *info);

/* ゲームファイルのロードシーケンスの開始 */
int LoadGameStart(int port,int id,void *photodata);

// DKR: Used for deleting save files after transfarring
int DeleteGameStart(int id);
int DeletePhotoStart(int id);

/* ゲームファイルの情報取得の開始 */
int GetGameInfoStart(int port);
int GetGameInfoBGStart(int port);
int GetGameInfoRBGStart(int port);
int GetGamePageInfoStart(int port,int id,int size);
int GetGameInfoOnlyBGStart(int port);
int GetGameInfoOnlyRBGStart(int port);

/* info部分のみの変更を開始 */
int ChangeGameInfoStart(int port,int id,void *newinfo);

/* メモリーカードシーケンスマネージャーの立ち上げ */
void MCInitCommand(void);
void NewMCMan(MCMAN_WORK *work);

int PhotoEasySearchStart(int port);
int GameEasySearchStart(int port);

int _UpdateMCStatus(MCMAN_WORK *work);

/* 写真のタイトルを決定 */
void MCMan_SetPhotoTitle(char *photoname);


/* mcman_step.c */
int CheckStep(MCMAN_WORK *work);
int FormatStep(MCMAN_WORK *work);
int SavePhotoStep(MCMAN_WORK *work);
int LoadPhotoStep(MCMAN_WORK *work);
int LoadPhotoIconStep(MCMAN_WORK *work);
int GetPhotoInfo(MCMAN_WORK *work);
int GetPhotoInfoR(MCMAN_WORK *work);
int ChangePhotoInfoStep(MCMAN_WORK *work);
int SaveGameStep(MCMAN_WORK *work);
int LoadGameStep(MCMAN_WORK *work);
int DeleteGameStep(MCMAN_WORK *work);
int DeletePhotoStep(MCMAN_WORK *work);
int GetGameInfo(MCMAN_WORK *work);
int GetGameInfoR(MCMAN_WORK *work);
int ChangeGameInfoStep(MCMAN_WORK *work);

int GameEasySearchStep(MCMAN_WORK *work);
int PhotoEasySearchStep(MCMAN_WORK *work);
int GameDirSearchStep(MCMAN_WORK *work);
int PhotoDirSearchStep(MCMAN_WORK *work);


/* mcscr.c */

#define MCSCR_N_FILES			16


typedef struct {
	GV_ACT_EX actor;

	MCMAN_WORK mcman;

	int mode;
	int step;
	int port;

	int flags;
	int n_indexes;
	unsigned short save_index;
	unsigned short indexes[MCSCR_N_FILES];

	int file_cursor;
	int confirm_cursor;

	int counter;

	int id;
	void *icondata;
	int iconsize;
	void *data;
	int size;
	void *info;
} MCScrWork;


enum {
	MCSCR_MODE_LOADGAME=0,
	MCSCR_MODE_SAVEGAME,
	MCSCR_MODE_LOADPHOTO,
	MCSCR_MODE_SAVEPHOTO,
};

void MCScrInit(MCScrWork *work,int mode,void *data,int size,void *icondata,int iconsize,void *info);
int MCScrAct(MCScrWork *work);
void MCScrDie(MCScrWork *work);

void NewMCSaveLoad_called(MCScrWork *work,int mode,void *data,int size,
						  void *icondata,int iconsize,void *info);
void *CODEC_NewMCSave(int iconname,int l2d_strcode);


/* mcdata.c */

#if 1

#ifndef __LIBGCL__H__
#inculde "libgcl.h"
#endif

typedef struct {
	char linkvar[ MAX_LINKVARBUF ];
	char id1;						// きまり。
	char variable_version[ 4 ];		// GCLのvariable.symからくるバージョン
	char id2;						// きまり。
	char seed[ 4 ];					// 暗号化用乱数シード
	char crc[ 4 ];					// CRC
	char varbuf[ MAX_VAR_BUF ];
	union {
		char tanker_photo_data[ GM_TANKER_PICTURE_SIZE ];
		char vr_score_data[ MSN_SAVE_DATA_SIZE ];
	};
} MCMAN_GAMEDATA;

#else
typedef unsigned int MCMAN_GAMEDATA[0x1000/sizeof(int)];
#endif

#if 0

typedef unsigned int MCMAN_INFODATA[16/sizeof(int)];

#else

typedef struct {
	// 秒単位の時間
	unsigned int mgs2_playtime;

	// セーブしたステージの番号
	unsigned char stage_num;

	// 難易度
	unsigned char difficulty;

	// ゲームクリア回数
	unsigned short clear_count;

	// ゲームクリア情報
	unsigned short clear_flag;

	// 16Byteに揃える
	//	unsigned char dummy[6];
	union {
		struct {
			u_char snake_tales_no;
			u_char st_clear_flag;	// クリアフラグ
		};
		struct {
			u_char achieve_high;
			u_char achieve_low;
		};
	};
   // DKR Armature: Store the save date
   // Top 7 bits: Year since 2000
   // Next 4 bits: Month (0-11)
   // Last 5 bits: Day (1-31)
   unsigned short save_date;
	unsigned char dummy[2];
	
} MCMAN_INFODATA;

#define MCMAN_VR_NAME_MAX	16

typedef struct MCMAN_VR_INFODATA {
	char	name[ MCMAN_VR_NAME_MAX ];	// 名前・NULL Terminate じゃないので注意
	u_int	vr_playtime;					// プレイ時間
	u_char	achieve_high;				// 達成率上位
	u_char	achieve_low;				// 達成率下位
} MCMAN_VR_INFODATA;

#endif


#define CLASTER_SIZE				1024
#define N_FILES_IN_DIR				4

#if 0
/* 2001/7/27 */
#define ICON_FILE_MAXSIZE			70390
#elif 0
/* 2001/8/6 */
#define ICON_FILE_MAXSIZE			44358
#else
/* 2001/9/11 */
#define TNKICON_FILE_MAXSIZE		47814
#define PLTICON_FILE_MAXSIZE		42626
#define ICON_FILE_MAXSIZE			TNKICON_FILE_MAXSIZE
#endif

#ifdef PSX2
#define ICONSYS_MAXSIZE				sizeof(sceMcIconSys)
#else
#define ICONSYS_MAXSIZE				0
#endif
#define GAME_FILE_MAXSIZE			sizeof(MCMAN_GAMEDATA)

#define PICON_FILE_MAXSIZE			3610
#define PHOTO_FILE_MAXSIZE			( 720*1280 )

#define VICON_FILE_MAXSIZE			20914


/* 本来必要なサイズを10KByte単位にしてある */
#ifndef MGS2_VRTRIAL
#define GAME_MC_MAXSIZE \
	(((((GAME_FILE_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((ICON_FILE_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((ICONSYS_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((N_FILES_IN_DIR+1)/2)+2+10-1)/10)*10*CLASTER_SIZE)
#else
#define GAME_MC_MAXSIZE VR_MC_MAXSIZE
#endif

#define PHOTO_MC_MAXSIZE \
	(((((PHOTO_FILE_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((PICON_FILE_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((ICONSYS_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((N_FILES_IN_DIR+1)/2)+2+10-1)/10)*10*CLASTER_SIZE)

#define VR_MC_MAXSIZE \
	(((((GAME_FILE_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((VICON_FILE_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((ICONSYS_MAXSIZE+CLASTER_SIZE-1)/CLASTER_SIZE)+ \
	   ((N_FILES_IN_DIR+1)/2)+2+10-1)/10)*10*CLASTER_SIZE)
		


void SaveGameData(MCMAN_GAMEDATA *data,int id, int filetype);
void SaveGameInfo(MCMAN_INFODATA *info);
void SaveVRInfo(MCMAN_VR_INFODATA* pinfo);
int LoadGameData(MCMAN_GAMEDATA *data, int filetype);

extern int MCMAN_GetDataFileMax( int file_kind );
extern void MC_InitIconEnv(int id, int file_kind);

///////////////////////////////////
// スネークテイルズ用define

// サブタイトル(リソースになるかも？)
#define	MCMAN_TALES_TITLE_A	"A Wrongdoing"
#define	MCMAN_TALES_TITLE_B "Big Shell Evil"
#define	MCMAN_TALES_TITLE_C "Confidential Legacy"
#define	MCMAN_TALES_TITLE_D "Dead Man Whispers"
#define	MCMAN_TALES_TITLE_E "External Gazer"

// tales の数
#define MCMAN_MAX_TALES		5

#ifdef __cplusplus
};
#endif

#endif
