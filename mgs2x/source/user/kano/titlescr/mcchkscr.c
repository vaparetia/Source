//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mcchkscr.c
		起動直後のメモリーカードチェック

	2001/06/12 K.Kano
	$Id: mcchkscr.c,v 1.2 2002/12/05 18:42:00 takaki Exp $
*/


#include "mcchkscr.h"

#include "font.h"
#include "libfs.h"

#include "BP_LocalizedTextByEnum.h"

#include "BP_CommonDialog.h"
#include "BP_Network.h"
#include "BP_Transfarring.h"
#include "Transfarring_UI_Strings.h"

#include "BP_Misc.h"

#define N_PORTS		2


#define ANIM_WORK_SIZE		5

#define DISP_CHANL			4
#define MENU_PRIORITY		0


typedef struct _mcchkscr_Work {
	GV_ACT_EX actor;

	MCMAN_WORK mcman;

	int name;

	int step;
	int sub_step;
	int ans[N_PORTS];
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

	// ファイル数を数える用
	int mode;
	int	bCount;
	int nCount;

   int bCheckSyncInstead;

//	int mcwarning_resource;
} Work;


#define SUCCESS			1
#define NO_SPACE		-1
#define NO_CARD			-2
#define ERROR_CARD		-3


#if 0 //多言語 yano 2002.06.11
#ifdef NTSC

#define MCWARNING_RESOURCE		0x004098bc		/* メモリーカード警告メッセージ */

#endif

#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL

#define MCWARNING_RESOURCE			(work->mcwarning_resource)		/* メモリーカード警告メッセージ */

#define E_MCWARNING_RESOURCE		0x004098bc		/* メモリーカード警告メッセージ */
#define F_MCWARNING_RESOURCE		0x0002428c		/* メモリーカード警告メッセージフランス語 */
#define G_MCWARNING_RESOURCE		0x00574419		/* メモリーカード警告メッセージドイツ語 */
#define I_MCWARNING_RESOURCE		0x0040dc0b		/* メモリーカード警告メッセージイタリア語 */
#define S_MCWARNING_RESOURCE		0x00f726c6		/* メモリーカード警告メッセージスペイン語 */

static const int mcwarning_resource_name[]={
	E_MCWARNING_RESOURCE,
	F_MCWARNING_RESOURCE,
	G_MCWARNING_RESOURCE,
	I_MCWARNING_RESOURCE,
	S_MCWARNING_RESOURCE,
};

#endif
#else
#define MCWARNING_RESOURCE			(mcwarning_resource_name[GM_Language-GM_LANG_ENGLISH])		/* メモリーカード警告メッセージ */

#define J_MCWARNING_RESOURCE		0x004f3898		/* メモリーカード警告メッセージ日本語 */
#define E_MCWARNING_RESOURCE		0x001fe578		/* メモリーカード警告メッセージ英語 */
#define F_MCWARNING_RESOURCE		0x0002428c		/* メモリーカード警告メッセージフランス語 */
#define G_MCWARNING_RESOURCE		0x00574419		/* メモリーカード警告メッセージドイツ語 */
#define I_MCWARNING_RESOURCE		0x0040dc0b		/* メモリーカード警告メッセージイタリア語 */
#define S_MCWARNING_RESOURCE		0x00f726c6		/* メモリーカード警告メッセージスペイン語 */

static const int mcwarning_resource_name[]={
	E_MCWARNING_RESOURCE,
	F_MCWARNING_RESOURCE,
	G_MCWARNING_RESOURCE,
	I_MCWARNING_RESOURCE,
	S_MCWARNING_RESOURCE,
	0, //韓国語
	J_MCWARNING_RESOURCE,
};
#endif


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


#if 0
#define FONT_WIDTH				24
#define FONT_HEIGHT				24
#else
#define FONT_WIDTH				FONT_SIZE_W
#define FONT_HEIGHT				FONT_SIZE_H
#endif
#define FONT_DISP_WIDTH			16
#define FONT_DISP_HEIGHT		16
#define LINE_SPACE				RUBI_SIZE_H

#define MCWARNING_U				0
#define MCWARNING_V				0
#define MCWARNING_WIDTH			(FONT_WIDTH*30)
#if 0
#define MCWARNING_HEIGHT		(FONT_HEIGHT*10)
#else
#define MCWARNING_HEIGHT		((FONT_HEIGHT+LINE_SPACE)*10+DOWN_MARGINE)
#endif
#define YESSTR_U				0
#if 0
#define YESSTR_V				(FONT_HEIGHT*10)
#else
#define YESSTR_V				(MCWARNING_V+MCWARNING_HEIGHT)
#endif
#define YESSTR_WIDTH			(FONT_WIDTH*8)
#if 0
#define YESSTR_HEIGHT			(FONT_HEIGHT*1)
#else
#define YESSTR_HEIGHT			((FONT_HEIGHT+LINE_SPACE)*1+DOWN_MARGINE)
#endif
#define STRNO_U					(YESSTR_U+YESSTR_WIDTH)
#if 0
#define STRNO_V					(FONT_HEIGHT*11)
#else
#define STRNO_V					(MCWARNING_V+MCWARNING_HEIGHT)
#endif
#define STRNO_WIDTH				(FONT_WIDTH*8)
#if 0
#define STRNO_HEIGHT			(FONT_HEIGHT*1)
#else
#define STRNO_HEIGHT			((FONT_HEIGHT+LINE_SPACE)*1+DOWN_MARGINE)
#endif

#define MCWARNING_X				100
// #define MCWARNING_Y				100
#define MCWARNING_Y				122
#if 0
#define MCWARNING_W				(FONT_DISP_WIDTH*30)
#define MCWARNING_H				(FONT_DISP_HEIGHT*10)
#else
#define MCWARNING_W				(MCWARNING_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define MCWARNING_H				(MCWARNING_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define YESSTR_X				-40
#define YESSTR_Y				(270-18)
#if 0
#define YESSTR_W				(FONT_DISP_WIDTH*8)
#define YESSTR_H				(FONT_DISP_HEIGHT*1)
#else
#define YESSTR_W				(YESSTR_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define YESSTR_H				(YESSTR_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif
#define STRNO_X					40
#define STRNO_Y					(270-18)
#if 0
#define STRNO_W					(FONT_DISP_WIDTH*8)
#define STRNO_H					(FONT_DISP_HEIGHT*1)
#else
#define STRNO_W					(STRNO_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define STRNO_H					(STRNO_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define STRYES_X_PAL			(171+24)
#define STRNO_X_PAL				(271+24)

#define CHECKING_U				0
#define CHECKING_V				(YESSTR_V+YESSTR_HEIGHT)
#define CHECKING_WIDTH			(FONT_WIDTH*30)
#if 0
#define CHECKING_HEIGHT			(FONT_HEIGHT*1)
#else
#define CHECKING_HEIGHT			(FONT_HEIGHT*1+DOWN_MARGINE)
#endif

#define CHECKING_X				(30+16)
#define CHECKING_Y				(350-16)
#if 0
#define CHECKING_W				(FONT_DISP_WIDTH*30)
#define CHECKING_H				(FONT_DISP_HEIGHT*1)
#else
#define CHECKING_W				(CHECKING_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define CHECKING_H				(CHECKING_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif


//#define CHECKING_MCCARD			"Checking MEMORY CARD (PS2). Please wait."


#define DISP_WAIT_COUNT			DIRECT_TICK(1.0f*60.0f)


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void LayoutSignalFunc(Work *work,int sign,int value)
{
}

static void KeyAnimSet0(Work *work,int part_code,int key_code)
{
	void *part;

	part=L2D_GetParts(work->l2d_handle,part_code);
	if(part==NULL) return;

	L2D_MorfObject(part,key_code,key_code,1.0f);
}

static void KeyAnim(Work *work)
{
	int i;

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


static inline int DateCompare(unsigned char *a,unsigned char *b)
{
	if(a[DATETIME_INDEX_YEAR]<b[DATETIME_INDEX_YEAR]) return -1;
	else if(a[DATETIME_INDEX_YEAR]>b[DATETIME_INDEX_YEAR]) return 1;

	if(a[DATETIME_INDEX_MONTH]<b[DATETIME_INDEX_MONTH]) return -1;
	else if(a[DATETIME_INDEX_MONTH]>b[DATETIME_INDEX_MONTH]) return 1;

	if(a[DATETIME_INDEX_DAY]<b[DATETIME_INDEX_DAY]) return -1;
	else if(a[DATETIME_INDEX_DAY]>b[DATETIME_INDEX_DAY]) return 1;

	if(a[DATETIME_INDEX_HOUR]<b[DATETIME_INDEX_HOUR]) return -1;
	else if(a[DATETIME_INDEX_HOUR]>b[DATETIME_INDEX_HOUR]) return 1;

	if(a[DATETIME_INDEX_MIN]<b[DATETIME_INDEX_MIN]) return -1;
	else if(a[DATETIME_INDEX_MIN]>b[DATETIME_INDEX_MIN]) return 1;

	if(a[DATETIME_INDEX_SEC]<b[DATETIME_INDEX_SEC]) return -1;
	else if(a[DATETIME_INDEX_SEC]>b[DATETIME_INDEX_SEC]) return 1;

	return 0;
}

static int PortCheck(Work *work,int port)
{

#if 0
	{
		static int errindex=0;
		static int errcode[]={
			NO_CARD,
			NO_SPACE,
			ERROR_CARD,
		};
		int reterr=errcode[errindex];

		if(port==1){
			errindex++;
			if(errindex>=3) errindex=0;
		}

		return reterr;
	}
#endif

	switch(work->sub_step){
	case 0:
		switch(MCManGetCardType(port)){
		case MCMAN_CARDTYPE_NOCARD:
		case MCMAN_CARDTYPE_PS1:
		case MCMAN_CARDTYPE_POCKET_STATION:

		case MCMAN_CARDTYPE_MULTITAP:
			return NO_CARD;

		case MCMAN_CARDTYPE_ERRORCARD:
			return ERROR_CARD;

		case MCMAN_CARDTYPE_PS2_FORMATTED:
			work->sub_step++;
			break;

		case MCMAN_CARDTYPE_PS2_UNFORMATTED:
			return SUCCESS;
		}
		break;
	case 1:
		if(GetGameInfoStart(port)){
			work->sub_step++;
		}
		break;
	case 2:
		switch(MCManGetResult()){
		case 1:
			/* クリアフラグ取得 */
			{
				int i;
				int rid=0;
				unsigned char *rdate=NULL;

				if( work->bCount && 
					(work->mode != MCMAN_FILE_KIND_GAME) ) {	// ゲームの場合はフラグチェックに
					for(i=0;i<DATA_FILE_MAX;i++){
						if( work->mode == MCMAN_FILE_KIND_OTHER ) {
							int j=(i>>4);
							int s=(i & 15)*2;
							if( (work->mcman.file_flags[j] >> s) & 3 ) {
								++work->nCount;
							}
						} 
                  else 
                  {
#if defined(BP_360)
                     if(BPX360_MCManCheckFileFlag_DoNotCareIfCorrupt(i))
#else
							if(MCManCheckFileFlag(i))
#endif
                     {
								++work->nCount;
							}
						}
					}
				} else {
					for(i=0;i<DATA_FILE_MAX;i++){
						if(MCManCheckFileFlag(i)){
							MCMAN_INFODATA *info=(MCMAN_INFODATA *)MCManGetFileInfo(i);
							unsigned char *date=(unsigned char *)MCManGetDateTime(i);

							if( work->bCount ) ++work->nCount;
							
							GM_TitleMenuStatus|=info->clear_flag;
							
							if(rdate==NULL){
								rdate=date;
								rid=i;
							}
							else{
								if(DateCompare(date,rdate)>=0){
									rdate=date;
									rid=i;
								}
							}
						}
					}
				}
				if(rdate!=NULL){
					if(GetLoaddataID()==MC_LOADDATA_NOID) SetLoaddataID(port,rid);
				}
			}

			//  VR, Snake Tales のからみで ゲームファイルの数によらず容量不足時に警告を出す
			//  Change 2002/10/08 M.Kobayashi
#if 0
			if(MCManGetFreeSize(port)>=GAME_MC_MAXSIZE /*||
				 MCManNFiles()>0*/) return SUCCESS;
#else
			// VR, Snake Tales, Game がすべてあれば警告なし
			if(MCManGetFreeSize(port)>=GAME_MC_MAXSIZE 
			   || ((work->mcman.exist_flag & ( MCMAN_EXIST_FLAG_GAME | MCMAN_EXIST_FLAG_VR | MCMAN_EXIST_FLAG_SNAKE_TALES ))
				   == ( MCMAN_EXIST_FLAG_GAME | MCMAN_EXIST_FLAG_VR | MCMAN_EXIST_FLAG_SNAKE_TALES )) ) {
				return SUCCESS;
			}

#endif
			else return NO_SPACE;
		case -1:
			work->sub_step--;
			break;
		}
		break;
	}

	return 0;
}


static void CreateChecking(Work *work)
{
	MENU_ClearPartTextTexture(work->strman,CHECKING_V,CHECKING_HEIGHT);

	//MENU_CreateTextTexture(work->strman,CHECKING_U,CHECKING_V,
   //                       CHECKING_WIDTH,CHECKING_HEIGHT,0,0,0,
   //                       BP_GetStringForEnum(kBP_PLS_CheckingSlot12));

	work->wait_count=DISP_WAIT_COUNT;
}

static void DispChecking(Work *work)
{
#if 1
	if(work->wait_count>0){
		work->wait_count--;
		return;
	}
#endif

	// if(work->strflag) return;

	MENU_PutTextScreen(work->strman,
					   CHECKING_X,CHECKING_Y,CHECKING_X+CHECKING_W,CHECKING_Y+CHECKING_H,
					   CHECKING_U,CHECKING_V,CHECKING_U+CHECKING_WIDTH,CHECKING_V+CHECKING_HEIGHT,
					   TEXTCOLOR);
}


static void CreateErrorMessage(Work *work)
{
	char *error_message;
	char tstr[0x100];
	char *s,*t;
	char *yes_str,*no_str;
	int numflag=1;

	yes_str=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_YES);
	no_str=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_NO);

	// printf("%s %s xxx\n",yes_str,no_str);

	if(work->ans[0]==NO_SPACE || work->ans[1]==NO_SPACE){
		/* 空きがない */
		error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_NO_SPACE);
	}
	else if(work->ans[0]==ERROR_CARD || work->ans[1]==ERROR_CARD){
		/* エラーのカードのみがささっている */
		error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_ERROR_CARD);
	}
	else{

#if 0
		static int error_no=0;

		/* カードがささっていない */
		switch(error_no){
		case 0:
			error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_NO_CARD);
			break;
		case 1:
			error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_NO_SPACE);
			break;
		case 2:
			error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_ERROR_CARD);
			break;
		}

		error_no++;
		if(error_no>2) error_no=0;
#else
      error_message=(char *)GetLocalResource(MCWARNING_RESOURCE,RESINDEX_NO_CARD);
#endif

	}

	s=error_message;
	t=tstr;

	while(*s!='\0'){
		*t=*s;
		if(*t & 0x80){
			s++,t++;
			*t=*s;
		}
		else if(*t=='?' && numflag){
			int num1,num10,num100,num1000;
			int size=GAME_MC_MAXSIZE;
			int flag=0;

			size/=1024;
			num1=size % 10;
			size/=10;
			num10=size % 10;
			size/=10;
			num100=size % 10;
			size/=10;
			num1000=size % 10;

			if(num1000>0){
				*t='0'+num1000;
				t++;
				flag=1;
			}
			if(num100>0 || flag){
				*t='0'+num100;
				t++;
				flag=1;
			}
			if(num10>0 || flag){
				*t='0'+num10;
				t++;
				flag=1;
			}
			*t='0'+num1;

			numflag=0;
		}

		s++,t++;
	}
	*t='\0';

	MENU_ClearTextTexture(work->strman);

	work->str_width=MENU_CreateTextTexture(work->strman,
										   MCWARNING_U,MCWARNING_V,
										   MCWARNING_WIDTH,MCWARNING_HEIGHT,0,0,0,tstr);

	work->yes_width=MENU_CreateTextTexture(work->strman,
										   YESSTR_U,YESSTR_V,
										   YESSTR_WIDTH,YESSTR_HEIGHT,0,0,0,yes_str);

	work->no_width=MENU_CreateTextTexture(work->strman,
										  STRNO_U,STRNO_V,
										  STRNO_WIDTH,STRNO_HEIGHT,0,0,0,no_str)-STRNO_U;

	work->strflag=1;
}

static void DispErrorMessage(Work *work)
{
	int x;
	int yes_x,no_x;
	int yescol,nocol;

   if ( BP_Area_EU() )
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	{
		SPR_OBJ *spr;
		if((spr=L2D_GetObject(work->l2d_handle,CODE_YESNO))!=NULL) SPR_HIDE(spr);
		if((spr=L2D_GetObject(work->l2d_handle,CODE_SELECTOR))!=NULL) SPR_HIDE(spr);
		if((spr=L2D_GetObject(work->l2d_handle,CODE_CURSOR))!=NULL) SPR_HIDE(spr);
	}
//#endif

	if(!work->strflag) return;

	/* センターリング */
	x=DG_Chanl(DG_CHANL_MENU)->width/2+FONT_DISP_WIDTH/2;

#ifdef NTSC
	yes_x=x+YESSTR_X-(work->yes_width*FONT_DISP_WIDTH/FONT_WIDTH)/2;
	no_x=x+STRNO_X-(work->no_width*FONT_DISP_WIDTH/FONT_WIDTH)/2;
#endif

   if ( BP_Area_EU() )
   {
//#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
	   yes_x=STRYES_X_PAL;
	   no_x=STRNO_X_PAL;

	   {
		   SPR_OBJ *spr;
		   if((spr=L2D_GetObject(work->l2d_handle,CODE_CURSOR))!=NULL) SPR_SHOW(spr);
	   }
   }
//#endif

	x-=(work->str_width*FONT_DISP_WIDTH/FONT_WIDTH)/2;

	// printf("x yx nx = %d %d %d %d %d\n",x,yes_x,no_x,work->yes_width,work->no_width);

	if(work->yesno_cursor){
		yescol=UNSEL_TEXTCOLOR;
		nocol=SEL_TEXTCOLOR;
	}
	else{
		yescol=SEL_TEXTCOLOR;
		nocol=UNSEL_TEXTCOLOR;
	}

	MENU_PutTextScreen(work->strman,x,MCWARNING_Y,
					   x+MCWARNING_W,MCWARNING_Y+MCWARNING_H,
					   MCWARNING_U,MCWARNING_V,
					   MCWARNING_U+MCWARNING_WIDTH,MCWARNING_V+MCWARNING_HEIGHT,
					   TEXTCOLOR);

   if ( BP_Area_EU() )
   {
//#ifdef AREA_EU_BP_IGNORE()	//#ifdef PAL
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
   }
//#endif

}

static void SetCursor(Work *work)
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
		int argv[1];

		if( work->bCount ) {
			extern short GM_SaveDataNum[ 4 ];

			ASSERT( work->mode >= 0 && work->mode <= MCMAN_FILE_KIND_MAX );
			GM_SaveDataNum[ work->mode ] = work->nCount;
			
			arg.argc = sizeof(argv)/sizeof(argv[0]);
			arg.argv = argv;
			argv[0] = work->nCount;
		} else {
			arg.argc=0;
			arg.argv=NULL;
		}

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

   if( work->bCheckSyncInstead )
   {
      switch( work->step )
      {
      case 0:
         if (BP_Network_IsSignedIn())
         {
            STransfarringError error = Transfarring_PostWork_CheckSyncNeeded( Transfarring_KonamiSaveTypeToBPSaveType( work->mcman.file_kind ), 0 );
            if( error.mHighLevelError == kTE_Success )
            {
               BP_SetSpinnerLoadFlag(kLoadFlag_CloudSyncMgs2Mission, 1);
               work->step++;
            }
            else
            {
               work->step=2;
            }
         }
         else
         {
            work->step=2;
         }
         break;
      case 1:
         {
            STransfarringError error = Transfarring_ProcessWork();
            switch(error.mHighLevelError)
            {
            case kTE_Processing:
               break;
            case kTE_Success:
               if( error.mLowLevelError ) // kludge
               {
                  work->nCount=1;
                  work->step++;
               }
               else
               {
                  work->step++;
               }
               
               BP_SetSpinnerLoadFlag(kLoadFlag_CloudSyncMgs2Mission, 0);
               break;
            default:
               if(error.mLowLevelError)
               {
                  BP_CommonDialog_WantsErrorDialog(error.mLowLevelError);
               }
               else
               {
                  BP_CommonDialog_WantsMessageDialog(GetTransfarringStringForErrorCode(error.mHighLevelError), kMDL_OK);
               }
               work->step=2;
               
               BP_SetSpinnerLoadFlag(kLoadFlag_CloudSyncMgs2Mission, 0);
               break;
            }
         }
         break;
      case 2:
      default:
         // If we don't need to sync, go ahead and check the local memory card. Otherwise, continue to the load screen
         if( work->nCount == 1)
         {
            Kill(work);
         }
         else
         {
            work->bCheckSyncInstead = 0;
            work->step = 0;
         }
         break;
      }

      return;
   }

	switch(work->step){
	case 0:
		work->mcman.exist_flag = 0;

		if(!MCManCheckingOrChecked()){
			CreateChecking(work);
			work->step++;
			work->sub_step=0;
			break;
		}
		if(MCManChecked()){
			CreateChecking(work);
			work->step++;
			work->sub_step=0;
			break;
		}
		break;
	case 1:
		DispChecking(work);

		if(MCManCheckingOrChecked()){
			work->step=0;
			work->sub_step=0;
			break;
		}

		work->ans[0]=PortCheck(work,0);
		if(work->ans[0]){
			work->step++;
			work->sub_step=0;
		}
		break;
	case 2:
		DispChecking(work);

		if(MCManCheckingOrChecked()){
			work->step=0;
			work->sub_step=0;
			break;
		}

		work->ans[1]=PortCheck(work,1);
		if(work->ans[1]){
			work->step++;
			work->sub_step=0;
		}
		break;
	case 3:
		if(MCManCheckingOrChecked()){
			work->step=0;
			work->sub_step=0;
			break;
		}

		if( work->bCount ) {
			work->step=0x10;
			work->strflag=0;
			break;
		}

		if(work->ans[0]==SUCCESS || work->ans[1]==SUCCESS){
			if(work->strflag){
				work->action_strcode=CODE_CLOSE_WARNING;
			}
			work->step=0x10;
			work->strflag=0;
		}
		else
      {
         // BP_SAVE/Load - The only way this can happen is if there is no save device, since this is impossible on PS3,
         // and X360 handles this error with its own dialog we simply set our state in this case to done
#if 1
         work->step = 0x10;
#else
			if(!work->strflag){
				work->action_strcode=CODE_OPEN_WARNING;
			}
			work->strflag=0;
			work->step++;

			work->yesno_cursor=1;
			SetCursor(work);
#endif
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
		if(MCManCheckingOrChecked()){
			work->step++;
			// work->strflag=0;
			break;
		}

		if(GV_PadDataDirect[0].press & (PAD_OK|PAD_STA)){
			if(!work->yesno_cursor){
				if(work->strflag){
					work->action_strcode=CODE_CLOSE_WARNING;
				}
				work->step=0x10;
				work->strflag=0;
				SE_OK();
				break;
			}

#if 0

#ifdef DEBUG_MODE
			else{
				work->step++;
				break;
			}
#endif

#endif

		}

		switch(GV_PadDataDirect[0].press & (PAD_L|PAD_R)){
		case PAD_L:
		case PAD_R:
			work->yesno_cursor^=1;
			SetCursor(work);
			SE_SEL();
			break;
		}
		break;

	case 7:
		work->step=0;
		work->sub_step=0;
		break;

	case 0x10:
		if(work->busy_flag) break;
		Kill(work);
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
	BP_SetSpinnerLoadFlag(kLoadFlag_CloudSyncMgs2Mission, 0);
   
   if( !work->bCount || (work->mode == MCMAN_FILE_KIND_GAME )) {
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
static int GetResources(Work *work)
{
	GetOptionValue(work);

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

	{
		int i;
		for(i=0;i<ANIM_WORK_SIZE;i++){
			work->key_anim[i].count=0;
		}
	}

	work->bCount = FALSE;
	work->mode = MCMAN_FILE_KIND_FIRSTCHECK;

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

#if 0 //yano
#ifdef AREA_EU_BP_IGNORE()	// #ifdef PAL
		work->mcwarning_resource=mcwarning_resource_name[GM_Language-GM_LANG_ENGLISH];
#endif
#endif
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

		NewMCMan(&(work->mcman));

		GV_SetActorChild(work,&(work->mcman));

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		work->mcman.file_kind = work->mode;

		if((work->strman=NewTextScreenControl())==NULL){
			GV_DestroyActor(work);
			return NULL;
		}

		ResetLoaddataID();

		GM_SetGameStatus(STATE_PAUSE_DISABLE);
    }
    return (void *)work ;
}


static int GetResourcesExistCheck(Work *work)
{
	{
		int i;
		for(i=0;i<ANIM_WORK_SIZE;i++){
			work->key_anim[i].count=0;
		}
	}

	work->bCount = TRUE;
	work->nCount = 0;

	work->mode = GCL_GetOptionValue( 'm', MCMAN_FILE_KIND_GAME );
	work->proc = GCL_GetOptionValue( 'p', 0 );

	if( GCL_GetOption('s') != NULL ) {
		// 他の製品のデータ個数をチェックする
		extern void SetOtherFilename( char* str );
		char* pcode = NULL;
		pcode = GCL_GetNextString();
		SetOtherFilename( pcode );
		work->mode = MCMAN_FILE_KIND_OTHER;
	}

	work->l2d_handle = -1;
    return 1;
}

// ファイルの存在個数チェック
void *NewFileExistCheckScn(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		work->name=name;

		work->strman=NULL;
		work->strflag=0;

		work->step=0;
		work->sub_step=0;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		NewMCMan(&(work->mcman));

		GV_SetActorChild(work,&(work->mcman));

		if(!GetResourcesExistCheck(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		work->mcman.file_kind = work->mode;

      // Even if a local file doesn't exist, if a sync is needed, send them to the load screen so it can sync
      if( work->mcman.file_kind == MCMAN_FILE_KIND_VR || work->mcman.file_kind == MCMAN_FILE_KIND_SNAKE_TALES )
      {
         work->bCheckSyncInstead = 1;
      }
      else
      {
         work->bCheckSyncInstead = 0;
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



