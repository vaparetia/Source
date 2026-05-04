/*
	mcchkscr.c
		起動直後のメモリーカードチェック

	2001/06/12 K.Kano
	$Id: mcchkscrx.c,v 1.6 2002/12/05 18:42:00 takaki Exp $


	2002/03/20 M.Kobayashi
	X では HDD に空きがあるかどうか調べる。
	無いと警告を出して、そのまま続けるかダッシュボードに戻るか選択させる。

	クリアフラグはタイトルデータ領域にセーブしたほうが楽？

	TODO: クリアフラグ処理
	      ダッシュボードをユーザが選んだときの処理
		  他のスレッドを全て止めて Launch しなくてはならない

	2002/05/08 M.Kobayashi
	MCX_MAN を使用
	
*/


#include "mcchkscr.h"

#include "font.h"

#include "../xmcman/xmcman.h"	

#define	USER_DEVICE_NAME	"U:\\"


#define N_PORTS		2


#define ANIM_WORK_SIZE		5

#define DISP_CHANL			4
#define MENU_PRIORITY		0


typedef struct {
	GV_ACT_EX actor;

	MCX_MAN		man;

	int name;

	int step;
	int sub_step;
	int ans;
	int proc;

	int busy_flag;
	int l2d_strcode;
	int l2d_handle;
	int action_strcode;

	void *strman;
	int strflag;

	int str_width;
	int yes_width;
	int no_width;

	int yesno_cursor;

	int wait_count;

	struct {
		int part_code;
		int key0_code;
		int key1_code;
		float p;
		int count;
	} key_anim[ANIM_WORK_SIZE];

	int mcwarning_resource;
} Work;


#define SUCCESS			1
#define NO_SPACE		-1
#define NO_CARD			-2
#define ERROR_CARD		-3


#define MCWARNING_RESOURCE			(work->mcwarning_resource)		/* メモリーカード警告メッセージ */

#define E_MCWARNING_RESOURCE		0x001fe578		/* メモリーカード警告メッセージ英語 */
#define F_MCWARNING_RESOURCE		0x0002428c		/* メモリーカード警告メッセージフランス語 */
#define G_MCWARNING_RESOURCE		0x00574419		/* メモリーカード警告メッセージドイツ語 */
#define I_MCWARNING_RESOURCE		0x0040dc0b		/* メモリーカード警告メッセージイタリア語 */
#define S_MCWARNING_RESOURCE		0x00f726c6		/* メモリーカード警告メッセージスペイン語 */
#define J_MCWARNING_RESOURCE		0x004f3898		/* メモリーカード警告メッセージ日本語 */

static const int mcwarning_resource_name[]={
	E_MCWARNING_RESOURCE,
	F_MCWARNING_RESOURCE,
	G_MCWARNING_RESOURCE,
	I_MCWARNING_RESOURCE,
	S_MCWARNING_RESOURCE,
	0, // 韓国語
	J_MCWARNING_RESOURCE,
};


enum {
	RESINDEX_NO_SPACE=0,
	RESINDEX_NO_CARD,
	RESINDEX_ERROR_CARD,
	RESINDEX_YES,
	RESINDEX_NO,
};


#define CODE_DEFAULTACTION		0x0008a3fb		/* DefaultAction */

#define CODE_OPEN_WARNING		0x0010a77f		/* openWarn */
#define CODE_CLOSE_WARNING		0x00e38f5a		/* closeWarn */

#define CODE_CURSOR				0x008adb1f		/* cursor */
#define CODE_KEY_CURSOR_YES		0x005b0ac4		/* cursYes */
#define CODE_KEY_CURSOR_NO		0x008ad6fc		/* cursNo */

#define CODE_SELECTOR			0x007b94ee		/* menuMask */
#define CODE_KEY_UNSEL_NO		0x003c422f		/* unNo */
#define CODE_KEY_UNSEL_YES		0x0088711a		/* unYes */

#define CODE_YESNO_NULL			0x002ef841		/* yes_no_Null */
#define CODE_YESNO				0x008b0b27		/* yes_no */


// #define TEXTCOLOR				0x80808080
#define TEXTCOLOR				0x80555a50

#define SEL_TEXTCOLOR			0x80808080
#define UNSEL_TEXTCOLOR			0x38808080


#define FONT_WIDTH				FONT_SIZE_W
#define FONT_HEIGHT				FONT_SIZE_H
#define FONT_DISP_WIDTH			16
#define FONT_DISP_HEIGHT		16
#define LINE_SPACE				RUBI_SIZE_H

#define MCWARNING_U				0
#define MCWARNING_V				0
#define MCWARNING_WIDTH			(FONT_WIDTH*30)
#define MCWARNING_HEIGHT		(FONT_HEIGHT*12)
#define YESSTR_U				0
#define YESSTR_V				(MCWARNING_V+MCWARNING_HEIGHT)
#define YESSTR_WIDTH			(FONT_WIDTH*8)
#define YESSTR_HEIGHT			((FONT_HEIGHT+LINE_SPACE)*1+DOWN_MARGINE)
#define STRNO_U					(YESSTR_U+YESSTR_WIDTH)
#define STRNO_V					(MCWARNING_V+MCWARNING_HEIGHT)
#define STRNO_WIDTH				(FONT_WIDTH*8)
#define STRNO_HEIGHT			((FONT_HEIGHT+LINE_SPACE)*1+DOWN_MARGINE)

#define MCWARNING_X				100
// #define MCWARNING_Y				100
#define MCWARNING_Y				112
#define MAX_MCWARNING_W			(522)
#define MCWARNING_W				(MCWARNING_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define MCWARNING_H				(MCWARNING_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#define YESSTR_X				-40
#define YESSTR_Y				(270-18)
#define YESSTR_W				(YESSTR_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define YESSTR_H				(YESSTR_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#define STRNO_X					40
#define STRNO_Y					(270-18)
#define STRNO_W					(STRNO_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define STRNO_H					(STRNO_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define STRYES_X_PAL			(171+24)
#define STRNO_X_PAL				(271+24)

#define CHECKING_U				0
#define CHECKING_V				(YESSTR_V+YESSTR_HEIGHT)
#define CHECKING_WIDTH			(FONT_WIDTH*30)
#define CHECKING_HEIGHT			(FONT_HEIGHT*1+DOWN_MARGINE)

#define CHECKING_X				30
#define CHECKING_Y				350
#define CHECKING_W				(CHECKING_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define CHECKING_H				(CHECKING_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)

#define DISP_WAIT_COUNT			DIRECT_TICK(1.0f*60.0f)


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void LayoutSignalFunc(Work *work,int sign,int value)
{
}

static void KeyAnimSetX(Work *work,int index,int count,int part_code,int key0_code,int key1_code)
{
	work->key_anim[index].p=0.0f;
	work->key_anim[index].count=count;
	work->key_anim[index].part_code=part_code;
	work->key_anim[index].key0_code=key0_code;
	work->key_anim[index].key1_code=key1_code;
}

static void KeyAnimSet0(Work *work,int part_code,int key_code)
{
	void *part;

	if( work->l2d_handle < 0 ) return;
	
	part=L2D_GetParts(work->l2d_handle,part_code);
	if(part==NULL) return;

	L2D_MorfObject(part,key_code,key_code,1.0f);
}

static void KeyAnim(Work *work)
{
	int i;

	if( work->l2d_handle < 0 ) return;

	for(i=0;i<ANIM_WORK_SIZE;i++){
		if(work->key_anim[i].count>0){
			void *part;

			part=L2D_GetParts(work->l2d_handle,work->key_anim[i].part_code);
			if(part==NULL) return;

			work->key_anim[i].p+=(1.0f-work->key_anim[i].p)/(float)work->key_anim[i].count;

			L2D_MorfObject(part,work->key_anim[i].key0_code,work->key_anim[i].key1_code,
						   work->key_anim[i].p);

			work->key_anim[i].count--;
		}
	}
}


/* ------------------------------------------------------------------------ */


static void CreateErrorMessage(Work *work)
{
	char *error_message;
	char tstr[0x300];
	char *yes_str,*no_str;
	int numflag=1;

	yes_str=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_YES);
	no_str=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_NO);

	// printf("%s %s xxx\n",yes_str,no_str);

	if(work->ans == NO_SPACE ) {
		/* 空きがない */
		error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_NO_SPACE);
	}
	else if(work->ans ==ERROR_CARD ) {
		/* エラーのカードのみがささっている */
		error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_ERROR_CARD);
	}
	else{
		error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_NO_CARD);
	}
	
	{
		int freeblock = work->man.free_block;
		int needblock = work->man.require_block - freeblock;

		switch( GM_Language ) {
		case GM_LANG_ENGLISH:
			sprintf(tstr, error_message, freeblock,
					(freeblock == 1) ? "block" : "blocks",
					needblock, (needblock == 1) ? "block" : "blocks" );
			break;
		case GM_LANG_FRENCH:
			sprintf(tstr, error_message, freeblock,
					(freeblock == 1) ? "" : "s", (freeblock == 1) ? "" : "s",
					needblock, (needblock == 1) ? "" : "s", (needblock == 1) ? "" : "s");
			break;
		case GM_LANG_GERMANY:
		{
			char block1[] = {'n',' ','B','l',0x1f,0x1b,'c','k','e','\0'};
			char *block2 = &block1[2];
			sprintf(tstr, error_message, freeblock,
					(freeblock == 1) ? " Block" : block1, 
					needblock, (needblock == 1) ? "Block" : block2 );
		}
			break;
		case GM_LANG_ITALY:
		{
			char str1[] = {0x1f,0x6a,'\0'};
			sprintf(tstr, error_message, 
					(freeblock == 1) ? str1 : "Sono",
					(freeblock == 1) ? "e" : "i",
					freeblock,
					(freeblock == 1) ? "o" : "hi",
					(freeblock == 1) ? "o" : "i",
					needblock,
					(needblock == 1) ? "o" : "hi" );
			break;
		}
		case GM_LANG_SPANISH:
			sprintf(tstr, error_message, freeblock,
					(freeblock == 1) ? "" : "s", (freeblock == 1) ? "" : "s",
					needblock, (needblock == 1) ? "" : "s");
			break;
		}
	}				

	MENU_ClearTextTexture(work->strman);

	work->str_width=MENU_CreateTextTexture(work->strman,
										   MCWARNING_U,MCWARNING_V,
										   MCWARNING_WIDTH,MCWARNING_HEIGHT,0,0,0,tstr);
#if 0
	work->yes_width=MENU_CreateTextTexture(work->strman,
										   YESSTR_U,YESSTR_V,
										   YESSTR_WIDTH,YESSTR_HEIGHT,0,0,0,yes_str);

	work->no_width=MENU_CreateTextTexture(work->strman,
										  STRNO_U,STRNO_V,
										  STRNO_WIDTH,STRNO_HEIGHT,0,0,0,no_str)-STRNO_U;
#endif	

	work->strflag=1;
}

static void DispErrorMessage(Work *work)
{
	int x;
	int w;
	int yes_x,no_x;
	int yescol,nocol;

	if( work->l2d_handle < 0 ) return;
	
	{
		SPR_OBJ *spr;
		if((spr=L2D_GetObject(work->l2d_handle,CODE_YESNO))!=NULL) SPR_HIDE(spr);
		if((spr=L2D_GetObject(work->l2d_handle,CODE_SELECTOR))!=NULL) SPR_HIDE(spr);
		if((spr=L2D_GetObject(work->l2d_handle,CODE_CURSOR))!=NULL) SPR_HIDE(spr);
	}

	if(!work->strflag) return;

	/* センターリング */
//	x=DG_Chanl(DG_CHANL_MENU)->width/2+FONT_DISP_WIDTH/2;
	x=512/2+FONT_DISP_WIDTH/2;

#ifdef NTSC
	yes_x=x+YESSTR_X-(work->yes_width*FONT_DISP_WIDTH/FONT_WIDTH)/2;
	no_x=x+STRNO_X-(work->no_width*FONT_DISP_WIDTH/FONT_WIDTH)/2;
#endif

#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	yes_x=STRYES_X_PAL;
	no_x=STRNO_X_PAL;

#if 0	
	{
		SPR_OBJ *spr;
		if((spr=L2D_GetObject(work->l2d_handle,CODE_CURSOR))!=NULL) SPR_SHOW(spr);
	}
#endif	//0	
#endif	//PAL

	
	if( work->str_width > MAX_MCWARNING_W ) {
		x -=( MAX_MCWARNING_W * FONT_DISP_WIDTH/FONT_WIDTH)/2;
		w = MCWARNING_W * MAX_MCWARNING_W / work->str_width;
	} else {
		x-=(work->str_width*FONT_DISP_WIDTH/FONT_WIDTH)/2;
		w = MCWARNING_W;
	}
		

	// printf("x yx nx = %d %d %d %d %d\n",x,yes_x,no_x,work->yes_width,work->no_width);
	//printf("%d\n", work->str_width );

	if(work->yesno_cursor){
		yescol=UNSEL_TEXTCOLOR;
		nocol=SEL_TEXTCOLOR;
	}
	else{
		yescol=SEL_TEXTCOLOR;
		nocol=UNSEL_TEXTCOLOR;
	}

	MENU_PutTextScreen(work->strman,x,MCWARNING_Y,
					   x+w, MCWARNING_Y+MCWARNING_H,
					   MCWARNING_U,MCWARNING_V,
					   MCWARNING_U+MCWARNING_WIDTH,MCWARNING_V+MCWARNING_HEIGHT,
					   TEXTCOLOR);

#if 0
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	MENU_PutTextScreen(work->strman,yes_x,YESSTR_Y,
					   yes_x+YESSTR_W,YESSTR_Y+YESSTR_H,
					   YESSTR_U,YESSTR_V,
					   YESSTR_U+YESSTR_WIDTH,YESSTR_V+YESSTR_HEIGHT,
					   yescol);

	MENU_PutTextScreen(work->strman,no_x,STRNO_Y,
					   no_x+STRNO_W,STRNO_Y+STRNO_H,
					   STRNO_U,STRNO_V,
					   STRNO_U+STRNO_WIDTH,STRNO_V+STRNO_HEIGHT,
					   nocol);
#endif
#endif

}

static void LocalSetCursor(Work *work)
{
	if(work->yesno_cursor){
		KeyAnimSet0(work,CODE_CURSOR,CODE_KEY_CURSOR_NO);
		KeyAnimSet0(work,CODE_SELECTOR,CODE_KEY_UNSEL_YES);
	}
	else{
		KeyAnimSet0(work,CODE_CURSOR,CODE_KEY_CURSOR_YES);
		KeyAnimSet0(work,CODE_SELECTOR,CODE_KEY_UNSEL_NO);
	}
}

static void CallLocalProc(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;

		arg.argc=0;
		arg.argv=NULL;

		GCL_ExecProc(work->proc,&arg);
	}
}

static void Kill(Work *work)
{
	CallLocalProc(work);

	if(work->strman!=NULL){
		GV_DestroyOtherActor(work->strman);
		work->strman=NULL;
	}

	if(work->l2d_handle>=0){
		L2D_ReleaseLayout(work->l2d_handle);
		work->l2d_handle=-1;
	}

	GV_DestroyActor(work);
}

static void Act(Work *work)
{
	u_int press;
	press = GV_PadDataDirect[ 0 ].press |
			GV_PadDataDirect[ 1 ].press | 
			GV_PadDataDirect[ 2 ].press |
			GV_PadDataDirect[ 3 ].press ;
	if(work->l2d_handle>=0){
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

		KeyAnim(work);
	}
	else{
		work->busy_flag=0;
	}

	DispErrorMessage(work);

	switch(work->step){
	case 0:
	{
		// マネージャ終了待ち
		break;
	}
	case 3:
		if( work->ans ==SUCCESS ) {
			if(work->strflag){
				work->action_strcode=CODE_CLOSE_WARNING;
			}
			work->step=0x10;
			work->strflag=0;
		}
		else{
			if(!work->strflag){
				work->action_strcode=CODE_OPEN_WARNING;
			}
			work->strflag=0;
			work->step++;
			
			GM_TitleMenuStatus|=TITLE_MENU_SHORT_STORAGE;
			
			work->yesno_cursor=1;
			LocalSetCursor(work);
//			SetCursor(work);
		}
		if( work->l2d_strcode == 0 ) {
			work->step=0x10;
			work->strflag=0;
			break;
		}
		break;

	case 4:
		work->step++;
		break;

	case 5:
		if(work->busy_flag) break;

		/* 画面の表示の初期化 */
		CreateErrorMessage(work);
		work->step++;

	case 6:
		if( press & PAD_OK ){
			if(work->strflag){
				work->action_strcode=CODE_CLOSE_WARNING;
			}
			work->step=0x10;
			work->strflag=0;
			SE_OK();
			break;
		} else if( press & PAD_CANCEL ) {
			if(work->strflag){
				work->action_strcode=CODE_CLOSE_WARNING;
			}
			work->step=0x11;
			work->strflag=0;
			SE_OK();
			break;
		}
#if 0
		switch( press & (PAD_L|PAD_R)){
		case PAD_L:
		case PAD_R:
			work->yesno_cursor^=1;
			LocalSetCursor(work);
			SE_SEL();
			break;
		}
#endif		
		break;

	case 7:
		work->step=0;
		work->sub_step=0;
		break;

	case 0x10:
		if(work->busy_flag) break;
		Kill(work);
		break;
	case 0x11:	// ダッシュボードに飛ぶ
		if(work->busy_flag) break;
		Kill(work);
      BP_TODO_BREAK;
#if 0 //BP
//#ifndef KP_WINDOWS
		{	// TODO:他のスレッドを止めるべき？
			// (XLaunchNewImage がやってくれているといいのだが。。。)
			LD_LAUNCH_DASHBOARD	launch_info;
			ZeroMemory( &launch_info, sizeof( launch_info ) );
			launch_info.dwReason = XLD_LAUNCH_DASHBOARD_MEMORY;
			launch_info.dwParameter1 = 'U';
			launch_info.dwParameter2 = work->man.require_block ;
			XLaunchNewImage( NULL, (PLAUNCH_DATA)&launch_info );
		}
#endif			
		break;
	}
}


#if 1

static void NullAct(Work *work)
{
	Kill(work);
}

#endif


static void Die(Work *work)
{
	if( work->l2d_strcode != 0 ) {
		GM_TitleMenuStatus|=TITLE_MENU_DISPED_MCWARNING;
	}
}


/* ------------------------------------------------------------------------ */


/* オプションの評価 */
static void GetOptionValue(Work *work)
{
	work->proc=0;

	work->l2d_strcode=0;
	work->l2d_handle=-1;

    if(GCL_GetOption('d')!=NULL){
		work->l2d_strcode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
}

/* 資源を獲得 */
static void ExitFunc( Work* pw );

static int GetResources(Work *work)
{
	GetOptionValue(work);

	if( work->l2d_strcode != 0 ) {
#if 1	// AREA_EU_BP_IGNORE()
		if( !(GM_TitleMenuStatus & TITLE_MENU_SHORT_STORAGE) ) {
			GV_SetActor(&(work->actor),NullAct,Die) ;
			return 1;
		}
#endif		
		
		work->l2d_handle=L2D_LoadLayout(work->l2d_strcode,DISP_CHANL,MENU_PRIORITY,0);

		if(work->l2d_handle<0){
#ifdef DEBUG_MODE
			printf("Select Scr : L2D Init Error = %d\n",work->l2d_handle);
#endif

			ASSERT(0);
		}
		else{
			/* シグナルハンドラの設定 */
			L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);

			/* layoutの初期化 */
			L2D_EvokeAction(work->l2d_handle,CODE_DEFAULTACTION);
		}
	}

	{
		int i;
		for(i=0;i<ANIM_WORK_SIZE;i++){
			work->key_anim[i].count=0;
		}
	}

	// マネージャイニシャライズ
	MCX_InitManager( &work->man, -1, NULL,
					 MCX_MAN_FLAG_FIRSTCHECK |
					 MCX_MAN_FLAG_GAME );
	MCX_SetExitFunc( ExitFunc, work );
    return 1;
}

/* 初期化部メイン */
void *NewMCCheckScr(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->strman=NULL;
		work->strflag=0;

		work->step=0;
		work->sub_step=0;

		work->mcwarning_resource=mcwarning_resource_name[GM_Language-GM_LANG_ENGLISH];

		if(GM_TitleMenuStatus & TITLE_MENU_DISPED_MCWARNING){
			GV_SetActor(&(work->actor),NullAct,Die) ;
			GV_ActorEX(&(work->actor));
			GV_SetActorMessageKill(&(work->actor),name);

			if(!GetResources(work)){
				GV_DestroyActor(work) ;
				return NULL ;
			}

			GM_SetGameStatus(STATE_PAUSE_DISABLE);

			return (void *)work;
		}

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);


		// 言語設定をここでやってしまう
		// ダッシュボード設定をデフォルトにする
#if 0
		switch( XGetLanguage() ) {
		case XC_LANGUAGE_JAPANESE:
			GM_Language = GM_LANG_JAPANESE;
			break;
		case XC_LANGUAGE_GERMAN:
			GM_Language = GM_LANG_GERMANY;
			break;
		case XC_LANGUAGE_FRENCH:
			GM_Language = GM_LANG_FRENCH;
			break;
		case XC_LANGUAGE_SPANISH:
			GM_Language = GM_LANG_SPANISH;
			break;
		case XC_LANGUAGE_ITALIAN:
			GM_Language = GM_LANG_ITALY;
			break;
		default:
			GM_Language = GM_LANG_ENGLISH;
			break;
		}
#endif
		work->mcwarning_resource=mcwarning_resource_name[GM_Language-GM_LANG_ENGLISH];
	
		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		if((work->strman=NewTextScreenControl())==NULL){
			GV_DestroyActor(work);
			return NULL;
		}

		ResetLoaddataID();

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}

static void ExitFunc( Work* pw )
{
	// メモリカードマネージャ終了
	int i;
	extern short GM_SaveDataNum[ 4 ];
	// クリアフラグ取得
	MCX_FILEPROP* pData;
	for( i = 0 ; i < MCMAN_FILE_KIND_MAX ; i ++ ) {
		GM_SaveDataNum[ i ] = 0;
	}
	for( i = pw->man.nData, pData = pw->man.pFiles ; i > 0 ; i--, pData++ ) {
		if( pData->type != -1 ) {
			ASSERT( pData->type < MCMAN_FILE_KIND_MAX && pData->type >= 0 );
			GM_SaveDataNum[ pData->type ] ++;
			if( pData->type == MCMAN_FILE_KIND_GAME ) {
				GM_TitleMenuStatus |= pData->info.gameinfo.clear_flag;
			}
		}
	}
	for( i = 0 ; i < MCMAN_FILE_KIND_MAX ; i ++ ) {
		printf( "type %d = %d\n", i, GM_SaveDataNum[ i ] );
	}

	// 空き領域の確認
	if( pw->man.flag & MCX_MAN_FLAG_NOSPACE ) {
		pw->ans = NO_SPACE;
	} else {
		pw->ans = SUCCESS;
	}

	ASSERT( pw->step == 0 );
	pw->step = 3;
}
