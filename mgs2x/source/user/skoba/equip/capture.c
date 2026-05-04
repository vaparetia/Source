//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	capture.c
		デジカメで画像を取り込み圧縮するためのモジュール
	2001/05/21	K.Uehara
	2001/07/13  K.Kano
	  デジカメのメモリ管理を無線と統一。
      無線のデータロードにも対応し、メモリーカードセーブ画面のデータを
	  データロードによって取得できるように対応

	$Id: capture.c,v 1.7 2002/12/19 14:06:02 takaki Exp $
*/

/* by yano  メモリーカードまわりがまだなので保留 */

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"

#include	"memlist.h"
#include	"jpeg.h"
#include	"capture.h"

#include	"../../kano/titlescr/portsel.h"

#include	"../../kano/mcman/mcman.h"
#ifdef KP_XBOX
// BP #include	<xgraphics.h>
#include	"../../kano/xmcman/xmcman.h"
#endif

#include "sprite_2d.h"
#include "../../kira/2D_action/layout_2d.h"
#include "../../mode/codec/codec_signal.h"
#include "../../mode/codec/codecmem.h"
#include "../../mode/codec/cdc_load.h"

#include "../../kano/titlescr/subtitle.h"


extern void BP_AquireRenderThread();
extern void BP_ReleaseRenderThread();

extern void BP_WaitForLastRenderToComplete();
extern void BP_CaptureScreenShotTanker(int photoNum);
extern void BP_CaptureScreenShot(void** pJPEG, int* pJPEGSize, void** pPNG, int* pPNGSize);

#ifdef BP_360
extern void bp_set_current_savegame( const char * const dirsuffix );
extern int gBP_CanSelectNewDevice;
#endif

/* ---------------------------------------------------------------------- */
/*
	JPEG圧縮関連関数
*/

/* キャプチャコンフィグレーション */
#define JPEG_SIZE_LIMIT		( 24*1024 )

#define CAPTURE_MEM_SIZE	(3*1024*1024)

#define CAPTURE_CUT_W		0		// 左右カットする幅
#define CAPTURE_CUT_H		0		// 上下カットする幅

#define CAPTURE_X	CAPTURE_CUT_W
#define CAPTURE_Y	CAPTURE_CUT_H
#define CAPTURE_W	( DRAW_WIDTH - (CAPTURE_CUT_W)*2 )
#define CAPTURE_H	( DRAW_HEIGHT - (CAPTURE_CUT_H)*2 )


#define USE_CODEC_MALLOC

#ifdef USE_CODEC_MALLOC
#define MALLOC(_size)		codecMalloc(_size)
#define FREE(_addr,_size)	codecFree(_addr)
#else
#define MALLOC(_size)		GV_MlMalloc(ml,_size)
#define FREE(_addr,_size)	GV_MlFree(ml,_addr,_size)
#endif


#define STR_TITLE_LEN		64

#ifdef KP_XBOX
#define CODE_SHOWSLOT			0x002cb484			/* showBtm */
#define CODE_SHOWFILE			0x002cfbe7			/* showTop */
#define CODE_HIDEALL			0x0073acc4			/* hideAll */
#endif

/*
	画面キャプチャを行なうための準備
*/

typedef struct _capture_Work {
	GV_ACT_EX actor;
	int dg_chanl_flag_save[ 2 ];

#ifndef USE_CODEC_MALLOC
	MEMLIST ml;
#endif

	void *image;
	void *thumbnail;
   int thumbnailsize;
	void *code;
	void *font_work;
	int codesize;
	int allocatesize;

	int step;
	int sub_step;
	int time;
	int port_sel;

	int busy_flag;
	int l2d_handle;
	int action_strcode;
	int other_free_time;

	SPR_OBJ *titlespr[STR_TITLE_LEN];

	MCScrWork mcscr;
#ifdef PSX2	
	MCMAN_WORK mcman;
#else
	MCX_MAN	man;
//	IDirect3DBaseTexture8*	ptexxpr;
#if 0//BP
	LPDIRECT3DSURFACE8	pDxt1GameImageSurface;
#endif
#endif
#if 1 // BP_Render
   int save_flag; // save_flag == 0 ss mode; save_flag == 1 digital camera save 
   int photoNum; // If tanker ss, this is texture rendertarget offset to use
#endif
} Work;

typedef struct {
	GV_ACT_EX actor;
	int       delay;
	char      flag;
} Parent_Work;


extern void *NewTextScreenControlForCodec( void );
extern void *NewTextScreenControlEx( int text_vram_width, int text_vram_height, int prio, int flag, int bufferedTextFlag );
extern void MENU_ClearTextTexture( void *work );

/* ---------------------------------------------------------------------- */
/*
	画面キャプチャーを行ない,JPEGに圧縮する
*/

#define THUMB_MSIZE		(THUMB_W * THUMB_H * sizeof( short ))
#define CAPTURE_MSIZE	(CAPTURE_W * CAPTURE_H * sizeof( int ))
#define Q_RAUTE_MAX (5)

static inline int Qcheck( int size_a , int size_b ) // a b がOKなこと前提
{
	int tmp;

	tmp = ( size_b - size_a ) / 2;
	tmp = tmp < 0 ? -tmp : tmp;
	if ( tmp <= 2 ){
		tmp = size_b - tmp;
	} else {
		tmp = size_b - 1;
	}
	return ( tmp );
}

static void Capture( Work *work, int max_code )
{
	int flip;
	int size;
	int codesize;
	void *vram;
	void *thumb;
	UTL_JPEGWORK jpeg;

#ifndef USE_CODEC_MALLOC
	MEMLIST *ml = &work->ml;
#endif

	flip = ( DG_LastWhich < 0 ) ? ( 1 - DG_Clock ) : ( 1 - DG_LastWhich );

   BP_WaitForLastRenderToComplete();
   BP_AquireRenderThread();

   if( work->save_flag )
   {
      void* pJPEGBuffer;
      int JPEGSize;
      void* pPNGBuffer;
      int PNGSize;
      BP_CaptureScreenShot(&pJPEGBuffer, &JPEGSize, &pPNGBuffer, &PNGSize);

      work->code = MALLOC( JPEGSize );
      work->codesize = JPEGSize;
      memcpy(work->code, pJPEGBuffer, JPEGSize);
      free(pJPEGBuffer);

      work->thumbnail = MALLOC( PNGSize );
      work->thumbnailsize = PNGSize;
      memcpy(work->thumbnail, pPNGBuffer, PNGSize);
      free(pPNGBuffer);
   }
   else
   {
      BP_CaptureScreenShotTanker(work->photoNum);
   }

   BP_ReleaseRenderThread();
}

static void CaptureEnd( Work *work )
{
#ifndef USE_CODEC_MALLOC
	MEMLIST *ml = &work->ml;
#endif

	/* チャンネル0,1を表示に */
	int i;

	if(work->code!=NULL){
		FREE(work->code,work->allocatesize);
	}
	if(work->thumbnail!=NULL){
		FREE(work->thumbnail,THUMB_MSIZE);
	}

	for( i = 0; i < 2; i++ ){
		DG_Chanls[ i ].flag = work->dg_chanl_flag_save[ i ];
	}
	/* 画面切替を再開 */
	DG_UnDrawFrameCount = 1;

#ifndef USE_CODEC_MALLOC
	DG_ResizePacketMemory( 0 );
#else
	CDC_ReleaseLogicalDirectory();
	codecMemRecov();
#endif
	GM_SdSet( SNG_PAUSEOFF ); // 演説ならす
}


/* ---------------------------------------------------------------------- */
/*
	ACT
*/



#define DISP_CHANL			4
#define L2D_PRIORITY		0
#define L2D_STRCODE			0x0096a917		/* photo_save */

#define CODE_DEFAULTACTION		0x0008a3fb		/* DefaultAction */

#define STR_NODE_FONT		0x0034ca34		/* font */

#define SK_DESTROY (0x1)
#define SK_CHILD_DIE (0x10)
#define FREE_TIME (6)

enum {
	CAPTURE_STEP_INIT=0,
	CAPTURE_STEP_PORTSEL,

	CAPTURE_STEP_PHOTO_SAVE_MODE,

	CAPTURE_STEP_OTHER_FREE,
	CAPTURE_STEP_OTHER_FREE_TIME,

	CAPTURE_STEP_END,
};

// extern 
extern void *NewPhotoSaveMode( SPR_OBJ **kano_work , int port_sel , void *save_data , int code_size , int handle , void *font_work , MCMAN_WORK *mcman );

static int SignalFunc(void *pWork,int sign,int value)
{
	Work *work;
	void *l2d_data;

   work = ( Work * )pWork;
   l2d_data = NULL;

   // BP JG - a new 'sign' that will cause the photo-save to happen again. We do this when the save fails for any reason.
#if defined(BP_PS3)
   if ( sign == SIGNAL_PORTSEL_AGAIN)
   {
      sign = SIGNAL_PORTSEL_OK;
      work->step=CAPTURE_STEP_PORTSEL;
   }
#endif

	switch(sign)
   {
	case SIGNAL_PORTSEL_OK:
		if(work->step==CAPTURE_STEP_PORTSEL){
			if ( MCManCheckingOrChecked() != 0 ){
				work->sub_step = 0;
				work->step = CAPTURE_STEP_PORTSEL;
			} else {
				work->step++;
				work->port_sel=value;

				printf("Port = %d\n",work->port_sel);
				GV_SetActorChild( work , NewPhotoSaveMode( work->titlespr , work->port_sel , work->code ,
														   work->codesize , work->l2d_handle , work->font_work , &work->mcman ) );
			}
		}
		break;
	case SIGNAL_PORTSEL_CANCEL:
		work->step = CAPTURE_STEP_OTHER_FREE;
		break;
	case SK_DESTROY :
		if ( value & 0x1 ){
			work->sub_step = 0;
			work->step = CAPTURE_STEP_PORTSEL;
		} else {
			work->step++;
		}
		break;
	default:
		return GV_DefaultSignalFunc(work,sign,value);
	}
	return 0;
}

static int Parent_SignalFunc(void *pWork,int sign,int value)
{
	Parent_Work *work;

	work = ( Parent_Work * )pWork;
	switch( sign ){
	case SK_CHILD_DIE :
		work->delay = 10;
		work->flag = 0x1;
		break;
	default:
		return GV_DefaultSignalFunc(work,sign,value);
	}
	return 0;
}

static int InitStep(Work *work)
{
	int i;
	SPR_OBJ *spr;

	spr=L2D_GetObject(work->l2d_handle,STR_NODE_FONT);
	if(spr==NULL) ASSERT(0);

	for(i=0;i<STR_TITLE_LEN;i++){
		work->titlespr[i]=SPR_DuplicateTree(spr);
	}

	return 1;
}

#ifndef PSX2
static BOOL Save( Work* pw );
static BOOL CreateImage( Work* pw );
static void ExitFunc( Work* pw );
#endif

static int PortSelSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
#ifdef PSX2		
		GV_SetActorChild( work,NewPortselForPhotosave(work->l2d_handle, work->titlespr,STR_TITLE_LEN , work->font_work, work->mcman.mode ));
#else
		MCX_InitManager( &work->man, work->l2d_handle, work->font_work,
						 MCX_MAN_FLAG_SAVE | MCX_MAN_FLAG_PHOTO | MCX_MAN_FLAG_CODECMEM );
		GV_SetActorChild( work, &work->man );
		MCX_SetSaveLoadFunc( Save, work );
		work->man.CreateTex = CreateImage;
		MCX_SetExitFunc( ExitFunc, work );
		// ｌ２ｄカスタマイズ
		work->man.l2d_tab.a_check_show = CODE_SHOWSLOT;
		work->man.l2d_tab.a_file_show = CODE_SHOWFILE;
		work->man.l2d_tab.a_hide = CODE_HIDEALL;
#endif
		work->sub_step++;
		break;

	case 1:
		
		break;
	}
	return 0;
}

static void Step(Work *work)
{
	switch(work->step){
	case CAPTURE_STEP_INIT:
		InitStep(work);
		work->step++;		
	case CAPTURE_STEP_PORTSEL:
		PortSelSubStep(work);
		break;

	case CAPTURE_STEP_OTHER_FREE :
		GV_DestroyOtherActor( work->font_work );
		work->other_free_time = FREE_TIME;
		work->step++;
		break;

	case CAPTURE_STEP_OTHER_FREE_TIME :
		if ( work->other_free_time <= 0 ){
			work->step++;
		} else {
			work->other_free_time--;
		}	
		break;
		
	case CAPTURE_STEP_END:
		GV_CallParentSignalFunc( work , SK_CHILD_DIE , 0 );
		GV_DestroyActor(work);
		break;
	}
}

static void Act( Work *work )
{
	// メモリーカードへのセーブをここで行なう。
	// 無線と同様,メモリに制限をつける。

#if 0
  if ( GV_PadDataDirect[ 0 ].status & PAD_SEL ){
//	printf("memsize select = %ld\n" , codecMemGetSize() );
//	codecMemList();
  }
#endif
	if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK){
		work->busy_flag=0;
		if(work->action_strcode!=0){
			int stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);

#ifdef DEBUG_MODE
			printf("L2D Stat = %d\n",stat);
#endif

			work->action_strcode=0;
			work->busy_flag=1;
		}
	}
	else{
		work->busy_flag=1;
	}
	Step(work);
}

static void Die( Work *work )
{
	// 終了処理
	if ( work->l2d_handle >= 0 ){
		L2D_ReleaseLayout( work->l2d_handle );
	}
	SPR_ResetMemoryManager();
	CaptureEnd( work );

#ifdef KP_XBOX
#ifndef KP_WINDOWS

#if 0 //BP
	IDirect3DSurface8_Release( work->pDxt1GameImageSurface );
#endif

#else
	if( work->pDxt1GameImageSurface )
	{
		IDirect3DSurface8_Release(work->pDxt1GameImageSurface) ;
	}
#endif
#endif
}

static void ParentAct( Parent_Work *pWork )
{
	if( ( pWork->flag & 0x1 ) ){
		if ( pWork->delay <= 0 ){
			if ( codecMemRecov() == 0 ){
				GV_DestroyActor( pWork );
			}
		} else {
			pWork->delay--;
		}
	}
}

static void ParentDie( Parent_Work *pWork )
{
	GM_SdSet( SNG_PAUSEOFF ); // 演説ならす
	GV_PauseOffActorSystem( GV_PAUSE_STOP );
}

static int InitToPhotoSave(Work *work)
{
//	static u_char info[ sizeof(MCMAN_INFODATA) ];
   float safeZoneOffsetY = 0;
	Parent_Work *parent_work ;
	void *l2d_data;


	/* 画面切替を開始 */
	DG_UnDrawFrameCount = 1;

	/* ワークの初期化 */
	work->step=0;
	work->sub_step=0;
	work->time=0;

	work->busy_flag=0;
	work->l2d_handle=-1;
	work->action_strcode=0;

	// font initialize
	work->font_work = NewTextScreenControlForCodec();
	if ( work->font_work == NULL ){
		return ( -1 );
	}
	MENU_ClearTextTexture( work->font_work );

	GV_SetActorClass( work, GV_CLASS_CHARA );
	GV_SetActorKillLevel( work, GV_KILL_LEVEL_NORMAL );
	GV_InsertActorPriority( GV_ACTOR_MANAGER, work, 0xFF );

#ifdef USE_CODEC_MALLOC
	GV_SetActorFreeFunc(work,GV_Free);
	SPR_SetMemoryManager( codecMalloc , codecFree );
#endif

	GV_SetActor( work, Act, Die );
	GV_ActorEX(&(work->actor));
	GV_SetActorSignalFunc(work,SignalFunc);

	GV_SetActorChild( work , work->font_work );

	/* メモリーカードマネージャーの起動 */
#ifdef PSX2	
#ifdef BP_360
   bp_set_current_savegame( "P" );
   gBP_CanSelectNewDevice = 1;
#endif
	NewMCMan(&(work->mcman));
	work->mcman.file_kind = MCMAN_FILE_KIND_PHOTO;
	GV_SetActorChild(work,&(work->mcman));
#endif	
	GV_PauseOnActorSystem( GV_PAUSE_STOP );

	/* parentアクター起動 */
	if(( parent_work = ( Parent_Work * )GV_Malloc( sizeof( Parent_Work ) ) ) == NULL ){
		return ( -1 );
	}
	GV_ZeroMemory( parent_work,sizeof( Parent_Work ));
	GV_SetActorFreeFunc( parent_work , GV_Free );
	GV_SetActorClass( parent_work , GV_CLASS_CHARA );
	GV_SetActorKillLevel( parent_work , GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority( GV_ACTOR_MANAGER , parent_work , 0x1FF ); /* ACT_MODE <--- user/mode/codec/codecmem.h */

	GV_SetActor( parent_work, ParentAct, ParentDie );
	GV_ActorEX(&(parent_work->actor));

	GV_SetActorChild( parent_work , work );
	parent_work->flag = 0x0;
	GV_SetActorSignalFunc( parent_work, Parent_SignalFunc);
#if 0
	/* 無線のロードを使った場合 */
	if((l2d_data=CDC_GetFileEntry(L2D_STRCODE,'o'))==NULL){
		ASSERT(0);
	}
#else
	/* 通常のロードを使った場合 */
	if((l2d_data=GV_GetCache(GV_CacheID(L2D_STRCODE,'o')))==NULL){
		ASSERT(0);
	}
#endif

	work->l2d_handle=L2D_SetupLayout(l2d_data,DISP_CHANL,L2D_PRIORITY,SPR_FLAG_PRIV, safeZoneOffsetY);
	if(work->l2d_handle<0){
		ASSERT(0);
	}

	/* layoutの初期化 */
	L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
	return ( 0 );
}

/* ---------------------------------------------------------------------- */

/*
	外とのインターフェース
*/

void *NewCaptureStart( int save_flag, void *thumbnail, void *code, int max_code, int photoNum )
{
//	void *addr;
	int i;
	int worksize; 
	Work *work;

#ifndef USE_CODEC_MALLOC
	/* パケットメモリのコンフィグレーションを変更 */
	addr = DG_ResizePacketMemory( CAPTURE_MEM_SIZE );

	/* 確保したメモリの先頭にワークを確保 */
	work = addr;
	worksize = ( ( sizeof( Work ) + 15 ) & ~15 );
	GV_ZeroMemory( work, worksize );
	addr = addr + worksize;

	/* 残りのメモリをMemListによる管理に切替える */
	GV_MlBufferInit( &work->ml, "jpeg", addr, addr + CAPTURE_MEM_SIZE - worksize, 16 );
#else
	/* メモリ管理、ファイル管理を無線と共通化 */
	codecMemInit();
	CDC_InitLoadingModule();

	worksize = ( ( sizeof( Work ) + 15 ) & ~15 );
	if((work=GV_Malloc(worksize))==NULL){
		ASSERT(0);
	}
	GV_ZeroMemory( work, worksize );

#endif

	/* チャンネル0,1を非表示 */
	for( i = 0; i < 2; i++ ){
		work->dg_chanl_flag_save[ i ] = DG_Chanls[ i ].flag;
		DG_Chanls[ i ].flag = 0;
	}
	/* 画面切替を停止 */
	DG_UnDrawFrameCount = DG_UNDRAW_MAX;

	if( max_code == 0 ){
		max_code = JPEG_SIZE_LIMIT;
	}

	if( save_flag ){
		/* データを先行してロード */
		// CDC_LoadLogicalDirectory(0);
	}

   work->photoNum = photoNum;
   work->save_flag = save_flag;

	/* JPEGへのキャプチャ本体 */
	Capture( work, max_code ) ;

#if 0
	// いついかなる状況でもでるため
	DG_Chanl( 4 )->flag = 1;
#endif
	GM_SdSet( SNG_PAUSEON ); // 演説とめる
	/* 領域が指定されていれば転送 */
	if( thumbnail != NULL ){
		memcpy( thumbnail, work->thumbnail, THUMB_MSIZE );
	}
	if( code != NULL ){
		memcpy( code, work->code, work->codesize );
	}

	/* メモリーカードへのセーブのために ACTを起動 */
	if( save_flag ){
		if ( InitToPhotoSave( work ) < 0 ){
			return ( NULL );
		}
		if ( work->thumbnail != NULL ){
			MCMan_PhotoIconEncode( work->thumbnail );
		}
	} else {
printf( "JPEG NO SAVE\n" );
		CaptureEnd( work );
//		GV_PauseOffActorSystem( GV_PAUSE_STOP );
#ifdef USE_CODEC_MALLOC
		GV_Free(work);
#endif

	}
	return work;
}


#ifndef PSX2 /* Only for XBOX T.Morita 2002.05.07 */
//// ADD M.Kobayashi 2002/04/19
static BOOL Save( Work* pw )
{	// セーブ時に呼ばれる関数
	// 別スレッドから呼ばれるので注意！
	// 成功したときのみ TRUE を返す
	HANDLE hFile;

	if( (hFile = MCX_Open( GENERIC_WRITE )) == INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	if( MCX_Write( hFile, pw->code, JPEG_SIZE_LIMIT ) != JPEG_SIZE_LIMIT ) {
		return FALSE;
	}
	if( MCX_Close( hFile ) != 0 ) {
		return FALSE;
	}
	return TRUE;
}

static BOOL CreateImage( Work* pw )
{	// ダッシュボード画像を作成
	char fullpath[ MAX_PATH ];
	sprintf( fullpath, "%s\\saveimage.xbx", pw->man.pCurfile->szDir );
#ifndef KP_WINDOWS

#if 0 //BP
	XGWriteSurfaceOrTextureToXPR( (D3DResource*)pw->pDxt1GameImageSurface, fullpath, TRUE );
#endif

#endif
	return TRUE;
}

static void ExitFunc( Work* pw )
{
	GV_CallParentSignalFunc( &pw->man.actor, SIGNAL_PORTSEL_CANCEL, 0 );
}
#ifndef _DEBUG
// dummy
// D3DXLoadSurfaceFromSurface(); を呼ぶと必要
//HRESULT WINAPI D3DDevice_SetRenderState_ParameterCheck(D3DRENDERSTATETYPE State, DWORD Value){ return S_OK; }
#endif
#endif
