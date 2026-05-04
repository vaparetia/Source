/*
	libfs.h
		ファイル管理モジュール定義ファイル

	1999/05/27 K.Uehara
	$Id: libfs.h,v 1.6 2002/11/26 13:43:15 takaki Exp $
*/

//#ifdef KP_XBOX
//#include "libfsx.h"
//#else
/* ---------------------------------------------------------------------- */
/*
	現在の実行環境のメディアタイプ
*/

#ifdef __cplusplus
extern "C" {
#endif

enum fs_region
{
   FS_REGION_US,
   FS_REGION_JP,
   FS_REGION_EU,

   FS_REGION_COUNT
};

extern char const *FS_RegionFolders[];
extern enum fs_region FS_CurrentRegion;

EXTERN_INLINE char const *FS_BP_GetCurrentRegionFolder()
{
   return FS_RegionFolders[ FS_CurrentRegion ];
}

EXTERN_INLINE int BP_Area_EU() { return FS_CurrentRegion == FS_REGION_EU; }
EXTERN_INLINE int BP_Area_US() { return FS_CurrentRegion == FS_REGION_US; }
EXTERN_INLINE int BP_Area_JP() { return FS_CurrentRegion == FS_REGION_JP; }

#define FS_MEDIA_DISC	0	// make cd
#define FS_MEDIA_USB	1	// make usb
#define FS_MEDIA_DEV	2	// T10000
#define FS_MEDIA_HDU	3	// make hdu

int FS_MediaType( void );

/* ---------------------------------------------------------------------- */
#ifdef KP_WINDOWS
/*
	ファイルパスのための定義
*/
enum
{
	FS_PATH_CTGR_INI,		// 設定ファイルパス
	FS_PATH_CTGR_STAGE,		// STAGEパス
	FS_PATH_CTGR_STREAM,	// ストリームデータファイル

	MAX_FS_PATH_CTGR_NUM,
} ;
#endif

/*
	ファイル読み込みのための定義
*/

enum {
	FS_FILE_STAGE = 0,	// ステージファイル
	FS_FILE_CODEC = 1,	// 無線機データファイル
	FS_FILE_FACE  = 2,	// 無線機顔データファイル

	FS_FILE_MOVIE = 3,	// ムービーファイル
	FS_FILE_VOX   = 4,	// 音声ファイル
	FS_FILE_DEMO  = 5,	// デモファイル
	FS_FILE_MOVIEVR = 6,
	FS_FILE_MAX,
};

#define FS_FILE_STREAM_TOP	FS_FILE_MOVIE

#define FS_FILE_MODE_CD		0		// CDから読み取り
#define FS_FILE_MODE_HDU	1		// HDUから読み取り
#define FS_FILE_MODE_DEV	2		// 開発環境(host0)から読み取り


typedef struct {
	char *name;
	int pos;
	unsigned char mode;
	unsigned char layer;
	unsigned char alt;
	unsigned char pad;
} FS_FILE_INFO;

extern FS_FILE_INFO fs_file_info[];
	/* ファイルテーブルの実体 : file.cnfに記述, select.cに存在 */

#define FS_GET_FILE_POS( _n )	fs_file_info[ (_n) ].pos
#define FS_GET_FILE_NAME( _n )	fs_file_info[ (_n) ].name
#define FS_GET_FILE_MODE( _n )	fs_file_info[ (_n) ].mode
#define FS_SET_FILE_MODE( _n, _m )	( fs_file_info[ (_n) ].mode = _m )

#define FS_GET_FILE_ALT_POS( _n ) fs_file_info[ fs_file_info[ (_n) ].alt ].pos
#define FS_GET_FILE_LAYER( _n )		fs_file_info[ (_n) ].layer
#define FS_GET_FILE_ALT_LAYER( _n ) fs_file_info[ fs_file_info[ (_n) ].alt ].layer


#define FS_SECTOR_SIZE	2048
#define FS_SECTOR_ALIGN( a )	( (((a)+FS_SECTOR_SIZE-1)/FS_SECTOR_SIZE)*FS_SECTOR_SIZE )

#ifndef inline
#define inline __inline
#endif

EXTERN_INLINE int FS_GetStreamTop( int file_id, int offset )
{
#if 1 //BP
   {
      //Correctly switch to the alternate layer file id if this flag is set.
      int use_file_id;
      if( offset & 0x01000000 )
      {
         use_file_id = fs_file_info[ file_id ].alt;
      }
      else
      {
         use_file_id = file_id;
      }
      //Set top byte to file index because we can't add in the lsn of the file.
      return ( (unsigned int)use_file_id << 24 ) | ( offset & 0x00FFFFFF );
   }

#else //BP

   int ofs;
#  ifdef PSX2
	extern int FS_current_layer;
	if( offset & 0x01000000 ){
		ofs = FS_GET_FILE_ALT_POS( file_id );
		FS_current_layer = FS_GET_FILE_ALT_LAYER( file_id );
	} else {
		ofs = FS_GET_FILE_POS( file_id );
		FS_current_layer = FS_GET_FILE_LAYER( file_id );
	}
#  endif
#  ifdef KP_XBOX
	ofs = FS_GET_FILE_POS( file_id );
#  endif
	
   return ofs + ( offset & 0x00FFFFFF );
#endif //BP
}

#define FS_DISC_ERROR		0x00010000

/* ---------------------------------------------------------------------- */
/*
	外部使用関数宣言
*/

/* 開始 */

void FS_ResetIOP( void );
	/*	IOPモジュールのリセット */

void FS_StartDaemon( void );
	/* FS ライブラリの初期設定 */

void FS_LoadIopModules( void *addr );
	/* IOP Module のロード */

/* ステージデータ読み込み */

void *FS_LoadStageRequest( char *dirname );
	/* データ読み込み要求 info構造体へのポインタを返す */

int  FS_LoadStageSync( void *info );
	/* データが読み込まれる間の処理を行う */

void FS_LoadStageComplete( void *info );
	/* データの読み込み終了処理 */

void FS_LoadSoundPak( int code );
	/* サウンドデータの読み込み起動 */

void FS_LoadStagePreseek( char *dirname );

/* その他データ読み込み */

void FS_LoadFileRequest( int fileno, int offset, int size, void *buffer, int bp_use_offset );
	/* ファイル読み込み要求。メディアに関わらず使用可能 */
int  FS_LoadFileSync( void );
	/* load 完了チェック */

int FS_GetStreamTop( int file_id, int offset );
	/* ストリーミングの先頭セクタ取得 */

int FS_GetDiscStatus( void );
	/* ディスクの読み取りエラー通知 */

/* ---------------------------------------------------------------------- */
/*
	内部使用関数宣言
*/

void FS_StreamSystemInit( void );

/* for HD */

void FS_HDInit( void );
void FS_USBInit( void );

void FS_ChangeDirectory( char *dir );
int  FS_LoadRequest( char *name );
#ifdef KP_WINDOWS
int  FS_LoadRequestDirectPath( char *name );
#endif
void FS_LoadSet( void *buf, int size );
int  FS_LoadSync( void );
void FS_LoadStop( void );
void FS_HdStageFileInit( void );

void FS_UsbSetStreamFile( int file_id );
void FS_SetStreamFile( int file_id );

/* for CD */

void FS_CdInit( void );
void FS_CdFileSetup( FS_FILE_INFO *finfo );
void FS_CdStageFileInit( int alt );

void FS_CdStageSet( char *stage );
void FS_CdStageProgBinFix( void );

#ifndef O_RDONLY
#define O_RDONLY	0x0000
#define O_WRONLY	0x0001
#define O_RDWR		0x0002
#define O_APPEND	0x0008
#define O_CREAT		0x0100
#define O_TRUNC		0x0200
#endif

/* 互換用ダミー */
int  pcOpen( char *filename, int flag );
int  pcClose( int fd );
int  pcRead( int fd, void *buf, int nbyte );
int  pcWrite( int fd, void *buf, int nbyte );
int  pcLseek( int fd, unsigned int offset, int whence );
//#endif

#ifdef KP_WINDOWS
char* pcGetFilePath(void) ;
char* pcGetStreamFilePath(void) ;
char* pcGetErrorLogFilePath(void) ;
char* pcGetIniFilePath(void);
#endif

//----------------------------------------------------------------------------
// SLOT SYSTEM
//----------------------------------------------------------------------------

// slot.lstから渡される値のデコード
#define FS_SLOT_CODE_TO_SIZE( _code )	( ((_code) >> 20) * 2048 )
#define FS_SLOT_CODE_TO_OFFSET( _code ) ( (_code) & ( ( 1 << 20) - 1 ) )

/*
	mkcdimgのSLOTパックデータファイルそのものを管理する構造体。
*/

typedef void *FS_SLOTPACK;

typedef struct {
	int id;
	void *data;
	int size;
} FS_SLOTPACK_TAG;

/*
	スロットを管理する構造体
*/

typedef struct {
	int id;			// 現在使用しているページのID	0 は未使用
	void *page;		// ページへのポインタ。
} FS_SLOT_PAGE;

typedef struct {
	int name;		// スロットの名前
	int size;		// スロットのトータルサイズ
	int page_num;
	FS_SLOT_PAGE pages[0];
} FS_SLOT_INFO;

/*
	スロットファイルからパックされたデータを読み込む。
*/
extern int FS_SlotPackLoadSet( void *buffer, int code );

/*
	読み込み待ち。０を返すと読み込み完了。
*/
extern int FS_SlotPackLoadSync( void );

/*
	パックファイルのデータにLoadInitをかける。
	FCacheに登録するかどうかは,mode次第。
*/

#define SLOTPACK_INIT_MGS2_FACE 0x100

void FS_SlotPackInit( FS_SLOTPACK *pack, int mode );

/*
	パックファイルのデータを終了処理する。

	FCacheに登録した場合は,FCacheからの削除処理を行なう。mode = GV_INIT_CACHEを設定すること。
*/

void FS_SlotPackClose( FS_SLOTPACK *pack , int mode );

/*
	パックファイルからデータを取り出す。
*/
void *FS_SlotPackGetData( FS_SLOTPACK *pack, int id );

/*
	ファイル数を取得。
*/
int FS_SlotPackGetDataNum( FS_SLOTPACK *pack );

/*
	DATAの目録情報を取得。
	bufferはFS_SlotPackGetDataNum()の数だけ確保されなければならない。
*/
void FS_SlotPackGetDataTable( FS_SLOTPACK *pack, FS_SLOTPACK_TAG *output );

/*
	スロット/ページ関数群。
*/

void FS_SlotSystemInit( void );
FS_SLOT_INFO *FS_SlotGetInfo( u_int id );

EXTERN_INLINE void FS_SlotPageInit( FS_SLOT_PAGE *page, int mode ) {
	FS_SlotPackInit( ( FS_SLOTPACK * )page->page, mode );
}

EXTERN_INLINE void FS_SlotPageClose( FS_SLOT_PAGE *page, int mode ) {
	FS_SlotPackClose( ( FS_SLOTPACK * )page->page, mode );
}

EXTERN_INLINE void *FS_SlotPageGetData( FS_SLOT_PAGE *page, int id ){
	return FS_SlotPackGetData( ( FS_SLOTPACK * )page->page, id );
}

EXTERN_INLINE int FS_SlotPageGetDataNum( FS_SLOT_PAGE *page ){
	return FS_SlotPackGetDataNum( ( FS_SLOTPACK * )page->page );
}

EXTERN_INLINE void FS_SlotPageGetDataTable( FS_SLOT_PAGE *page, FS_SLOTPACK_TAG *output ) {
	FS_SlotPackGetDataTable( ( FS_SLOTPACK * )page->page, output );
}

EXTERN_INLINE FS_SLOT_PAGE *FS_SlotGetPageFromNo( FS_SLOT_INFO *slot, int no ) {
	return slot->pages + no;
}

EXTERN_INLINE int FS_SlotPageLoadSet( FS_SLOT_PAGE *page, int code ){
	page->id = code;
	return FS_SlotPackLoadSet( ( void * )page->page, code );
}

EXTERN_INLINE int FS_SlotPageLoadSync( FS_SLOT_PAGE *page ) {
	return FS_SlotPackLoadSync();
}

EXTERN_INLINE FS_SLOT_PAGE *FS_SlotGetPageFromId( FS_SLOT_INFO *slot, int id )
{
	int i;
	for( i = 0; i < slot->page_num; i++ ){
		if( slot->pages[ i ].id == id ){
			return &( slot->pages[ i ] );
		}
	}
	return NULL;
}

EXTERN_INLINE int FS_SlotPageIsFree( FS_SLOT_PAGE *page )
{
	return ( page->id == 0 );
}

EXTERN_INLINE void FS_SlotPageSetUse( FS_SLOT_PAGE *page, int id )
{
	page->id = id;
}

EXTERN_INLINE void *FS_SlotPageGetPtr( FS_SLOT_PAGE *page )
{
	return page->page;
}

#ifdef DEBUG_MODE
void FS_SlotDump( FS_SLOT_INFO *info );
#endif

/// @}

extern int cdbios_get_status( void );

#ifdef __cplusplus
};
#endif
