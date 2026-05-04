//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	portsel.c
		ポートセレクト画面

	2001/06/15 K.Kano
	$Id: portsel.c,v 1.5 2002/12/05 18:41:59 takaki Exp $
*/


#include "titlescr.h"

#include "../../mode/codec/codecmem.h"

#include "font.h"

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "libfs.h"

#include "BP_CommonDialog.h"
#include "BP_Network.h"
#include "BP_Transfarring.h"
#include "Transfarring_UI_Strings.h"

#include "BP_Misc.h"
#include "BP_Renderer.h"

#define N_TITLE_ITEMS		2
#define N_PORT_ITEMS		2


#define ANIM_COUNT			DIRECT_TICK(10)

#define ANIM_WORK_SIZE		2


typedef struct _portsel_Work {
	GV_ACT_EX actor;

	int parent_name;
	int name;

	int signal_flag;
	int start_mode;

	int busy_flag;
	int l2d_handle;
	int action_strcode;
	int handle_flag;

	int step;
	int sub_step;
	int mode;
	int timer;

	int key_status;
	int key_press;
	int key_autostatus;
	int key_count;

	int ans;

	int port_cursor;
	int port_mode;

	SPR_OBJ *cursor;
	SPR_OBJ *cursor_spr;
	SPR_OBJ *title[N_TITLE_ITEMS];
	SPR_OBJ *port[N_PORT_ITEMS];

	void *strman;

	float t_ypos;

	int anim_count;
	struct {
		SPR_OBJ *target;
		unsigned char talpha;
	} key_anim[ANIM_WORK_SIZE];

	float tag_anim_p;
	int tag_anim0;
	int tag_anim1;
	int tag_count;

   MCMAN_WORK* mcman;
} Work;


#if 0
#define FONT_WIDTH				24
#define FONT_HEIGHT				24
#else
#define FONT_WIDTH				FONT_SIZE_W
#define FONT_HEIGHT				FONT_SIZE_H
#endif
#define FONT_HEIGHT2			32
#define LINE_SPACE				RUBI_SIZE_H
#define LINE_HEIGHT				(FONT_HEIGHT+LINE_SPACE)

#define FONT_DISP_WIDTH			15
#define FONT_DISP_HEIGHT		15


#define TEXT_U			0
#define TEXT_V			0
#define TEXT_WIDTH		(FONT_WIDTH*30)
#if 0
#define TEXT_HEIGHT		FONT_HEIGHT
#define TEXT_HEIGHT2	FONT_HEIGHT2
#else
#define TEXT_HEIGHT		(LINE_HEIGHT+DOWN_MARGINE)
#define TEXT_HEIGHT2	TEXT_HEIGHT
#endif

#define TEXT2_U			0
#if 0
#define TEXT2_V			(FONT_HEIGHT2*1)
#else
#define TEXT2_V			(TEXT_V+TEXT_HEIGHT)
#endif
#define TEXT2_WIDTH		(FONT_WIDTH*30)
#if 0
#define TEXT2_HEIGHT	FONT_HEIGHT
#define TEXT2_HEIGHT2	FONT_HEIGHT2
#else
#define TEXT2_HEIGHT	(LINE_HEIGHT+DOWN_MARGINE)
#define TEXT2_HEIGHT2	TEXT2_HEIGHT
#endif


#define TEXT_X			56

#if defined(ENGLISH) && defined(NTSC)
#define TEXT_Y			168
#else
#define TEXT_Y			72
#endif

#if 0
#define TEXT_W			(FONT_DISP_WIDTH*30)
#define TEXT_H			FONT_DISP_HEIGHT
#else
#define TEXT_W			(TEXT_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define TEXT_H			(TEXT_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif

#define TEXT2_X			56

#if defined(ENGLISH) && defined(NTSC)
#define TEXT2_Y			168
#else
#define TEXT2_Y			72
#endif

#if 0
#define TEXT2_W			(FONT_DISP_WIDTH*30)
#define TEXT2_H			FONT_DISP_HEIGHT
#else
#define TEXT2_W			(TEXT2_WIDTH*FONT_DISP_WIDTH/FONT_WIDTH)
#define TEXT2_H			(TEXT2_HEIGHT*FONT_DISP_HEIGHT/FONT_HEIGHT)
#endif


enum {
	RESOURCE_MCNOTFOUND=0,

	/* ロード時 : ファイルがありません */
	/* セーブ時 : セーブのための空き領域がありません */
	RESOURCE_ERROR,
};


#define TEXT_R				(160/2)
#define TEXT_G				(180/2)
#define TEXT_B				(170/2)

#define TEXT_R2				((160/2)*2/3)
#define TEXT_G2				((180/2)*2/3)
#define TEXT_B2				((170/2)*2/3)

#define TEXTCOLOR			(TEXT_R | (TEXT_G<<8) | (TEXT_B<<16) | (0x80<<24))
#define GRY_TEXTCOLOR		(TEXT_R2 | (TEXT_G2<<8) | (TEXT_B2<<16) | (0x80<<24))


#define REPEAT_FIRST		20
#define REPEAT_NEXT			3


#define CURSOR_ADJUST		3.0f


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


enum {
	MCARD_START=0,
	MCARD_CHECK,
	MCARD_PORT_SEL,
   MCARD_CLOUD_SYNC,
	MCARD_FILE_CHECK,
	MCARD_ERROR,
};

enum {
	MCARD_MODE_LOAD=0,
	MCARD_MODE_SAVE,
	MCARD_MODE_PHOTOLOAD,
	MCARD_MODE_PHOTOSAVE,

	MCARD_MODE_DOGTAG_LOAD,
};

static void Key(Work *work)
{
	work->key_press=GV_PadDataDirect[0].press;

	if(GV_PadDataDirect[0].status && GV_PadDataDirect[0].status==work->key_status){
		if(work->key_count>REPEAT_FIRST){
			work->key_count-=REPEAT_NEXT;
			work->key_autostatus=work->key_status;
		}
		else work->key_autostatus=0;

		work->key_count++;
	}
	else{
		work->key_autostatus=work->key_status=GV_PadDataDirect[0].status;
		work->key_count=0;
	}
}

static void LocalShowCursor(Work *work)
{
	SPR_SHOW(work->cursor);
	SPR_SHOW(work->cursor_spr);
}

static void HideCursor(Work *work)
{
	SPR_HIDE(work->cursor);
	SPR_HIDE(work->cursor_spr);
}


/* ------------------------------------------------------------------------ */


static void AnimCtrlSet(Work *work,int index,SPR_OBJ *spr,int talpha)
{
	work->key_anim[index].target=spr;
	work->key_anim[index].talpha=talpha;
}

static void AnimCtrl(Work *work)
{
	int i;

	if(work->anim_count>0){
		for(i=0;i<ANIM_WORK_SIZE;i++){
			if(work->key_anim[i].target==NULL) continue;

			work->key_anim[i].target->sprite.col.a
				+=((int)(work->key_anim[i].talpha)-(int)(work->key_anim[i].target->sprite.col.a))
				/work->anim_count;
		}

		if(work->cursor!=NULL){
			work->cursor->empty.pos.y
				+=(work->t_ypos-work->cursor->empty.pos.y)/(float)(work->anim_count);
		}

		work->anim_count--;
	}
	else{
		for(i=0;i<ANIM_WORK_SIZE;i++){
			work->key_anim[i].target=NULL;
		}
	}
}


/* ------------------------------------------------------------------------ */


static const int tag_part_name[]={
	CODE_SRASH,
	CODE_DOGTAG,
	CODE_ALBUMTAG,
	CODE_DATASAVETAG,
	CODE_DATALOADTAG,
	CODE_UNDERBAR,
};

#define PART_SIZE		(sizeof(tag_part_name)/sizeof(tag_part_name[0]))


static void TagAnimSet0(Work *work,int tag_anim)
{
	int i;

	work->tag_anim0=tag_anim;
	work->tag_anim1=tag_anim;
	work->tag_count=0;
	work->tag_anim_p=1.0f;

	for(i=0;i<PART_SIZE;i++){
		void *part=L2D_GetParts(work->l2d_handle,tag_part_name[i]);
		if(part==NULL) continue;
		L2D_MorfObject(part,work->tag_anim1,work->tag_anim1,1.0f);
	}
}

static void TagAnimSet(Work *work,int tag_anim0,int tag_anim1,int count)
{
	work->tag_anim0=tag_anim0;
	work->tag_anim1=tag_anim1;
	work->tag_count=count;
	work->tag_anim_p=0.0f;
}

static void TagAnim(Work *work)
{
	if(work->tag_count>0){
		int i;

		work->tag_anim_p+=(1.0f-work->tag_anim_p)/(float)(work->tag_count);
		work->tag_count--;

		for(i=0;i<PART_SIZE;i++){
			void *part=L2D_GetParts(work->l2d_handle,tag_part_name[i]);
			if(part==NULL) continue;
			L2D_MorfObject(part,work->tag_anim0,work->tag_anim1,work->tag_anim_p);
		}
	}
}


/* ------------------------------------------------------------------------ */


static void DispNotFound(Work *work)
{
	if(work->port_mode==0){
		MENU_PutTextScreen(work->strman,TEXT_X,TEXT_Y,TEXT_X+TEXT_W,TEXT_Y+TEXT_H,
						   TEXT_U,TEXT_V,TEXT_U+TEXT_WIDTH,TEXT_V+TEXT_HEIGHT,TEXTCOLOR);
	}
}

static void DispError(Work *work)
{
	MENU_PutTextScreen(work->strman,TEXT2_X,TEXT2_Y,TEXT2_X+TEXT2_W,TEXT2_Y+TEXT2_H,
					   TEXT2_U,TEXT2_V,TEXT2_U+TEXT2_WIDTH,TEXT2_V+TEXT2_HEIGHT,TEXTCOLOR);
}


/* ------------------------------------------------------------------------ */


static void PortModeCheck(Work *work)
{
	work->port_mode=0;

	switch(MCManGetCardType(0)){
	case MCMAN_CARDTYPE_NOCARD:
	case MCMAN_CARDTYPE_PS1:
	case MCMAN_CARDTYPE_POCKET_STATION:

	case MCMAN_CARDTYPE_MULTITAP:
		break;

	case MCMAN_CARDTYPE_ERRORCARD:
		break;

	case MCMAN_CARDTYPE_PS2_FORMATTED:
		work->port_mode|=1;
		break;

	case MCMAN_CARDTYPE_PS2_UNFORMATTED:
#if 0
		if(work->mode==MCARD_MODE_SAVE ||
		   work->mode==MCARD_MODE_PHOTOSAVE){

			work->port_mode|=1;
		}
#else
		work->port_mode|=1;
#endif
		break;
	}

	switch(MCManGetCardType(1)){
	case MCMAN_CARDTYPE_NOCARD:
	case MCMAN_CARDTYPE_PS1:
	case MCMAN_CARDTYPE_POCKET_STATION:

	case MCMAN_CARDTYPE_MULTITAP:
		break;

	case MCMAN_CARDTYPE_ERRORCARD:
		break;

	case MCMAN_CARDTYPE_PS2_FORMATTED:
		work->port_mode|=2;
		break;

	case MCMAN_CARDTYPE_PS2_UNFORMATTED:
#if 0
		if(work->mode==MCARD_MODE_SAVE ||
		   work->mode==MCARD_MODE_PHOTOSAVE){

			work->port_mode|=2;
		}
#else
		work->port_mode|=2;
#endif
		break;
	}
}

static int StartSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag){
			if(!work->start_mode){
				switch(work->mode){
				case MCARD_MODE_LOAD:
					TagAnimSet0(work,CODE_HIDE_LOAD);
					work->action_strcode=CODE_OPENLOAD;
					break;
				case MCARD_MODE_PHOTOLOAD:
					TagAnimSet0(work,CODE_HIDE_ALBUMLOAD);
					work->action_strcode=CODE_OPENLOAD;
					break;
				case MCARD_MODE_SAVE:
				case MCARD_MODE_PHOTOSAVE:
					TagAnimSet0(work,CODE_HIDE_SAVE);
					work->action_strcode=CODE_OPENSAVE;
					break;
				case MCARD_MODE_DOGTAG_LOAD:
					TagAnimSet0(work,CODE_HIDE_DOGTAGLOAD);
					work->action_strcode=CODE_OPENLOAD;
					break;
				}

				SE_EXPANDLINE();
			}
			HideCursor(work);
			work->sub_step++;
		}
		break;
	case 1:
		return 1;
	}

	return 0;
}

static int CheckSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag){
			work->action_strcode=CODE_MCCHECK;

			if(!MCManCheckingOrChecked()){
				work->sub_step=2;
				break;
			}
			if(MCManChecked()){
				work->sub_step=2;
				break;
			}

			work->sub_step++;
		}
		break;
	case 1:
		if(!MCManCheckingOrChecked()){
			work->sub_step++;
			break;
		}
		if(MCManChecked()){
			work->sub_step++;
			break;
		}
		break;
	case 2:
	case 4:
		{
			int port=(work->sub_step-2)/2;
			int type=MCManGetCardType(port);

			switch(type){
			case MCMAN_CARDTYPE_NOCARD:
			case MCMAN_CARDTYPE_PS1:
			case MCMAN_CARDTYPE_POCKET_STATION:
			case MCMAN_CARDTYPE_ERRORCARD:
			case MCMAN_CARDTYPE_MULTITAP:
			case MCMAN_CARDTYPE_PS2_UNFORMATTED:
				work->sub_step+=2;
				break;

			case MCMAN_CARDTYPE_PS2_FORMATTED:
				switch(work->mode){
				case MCARD_MODE_LOAD:
				case MCARD_MODE_SAVE:
				case MCARD_MODE_DOGTAG_LOAD:
					if(GameEasySearchStart(port)){
						work->sub_step++;       //<<-- trying to go back
					}
					else{
						work->sub_step+=2;
					}
					break;
				case MCARD_MODE_PHOTOLOAD:
				case MCARD_MODE_PHOTOSAVE:
					if(PhotoEasySearchStart(port)){
						work->sub_step++;
					}
					else{
						work->sub_step+=2;
					}
					break;
				}
				break;
			}
		}
		break;
	case 3:
	case 5:
		if(MCManCheckingOrChecked()){
			work->sub_step=1;
			break;
		}
		switch(MCManGetResult()){
		case 1:
		case -1:
			work->sub_step++;
			break;
		}
		break;
	case 6:
		work->action_strcode=CODE_MCCHECKEND;
		return 1;
	}

	return 0;
}

enum
{
   FILECHECK_BEGINCHECKSYNC=0,
   FILECHECK_DOCHECKSYNC,
   FILECHECK_BEGINSYNC,
   FILECHECK_DOSYNC,
};

static int CloudSyncSubStep(Work *work)
{
   switch(work->sub_step)
   {
   case FILECHECK_BEGINCHECKSYNC:
      if (BP_Network_IsSignedIn())
      {
         STransfarringError error;
         
         BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 1);

         if( work->mode == MCARD_MODE_LOAD )
         {
            error = Transfarring_PostWork_CheckSyncNeeded( Transfarring_KonamiSaveTypeToBPSaveType( work->mcman->file_kind ), 0 );
         }
         else
         {
            error = Transfarring_PostWork_CheckSyncNeeded( Transfarring_KonamiSaveTypeToBPSaveType( work->mcman->file_kind ), 1 );
         }

         if( error.mHighLevelError == kTE_Success )
         {
            work->sub_step++;
         }
         else
         {
            BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);
            
            return 1;
         }
      }
      else
      {
         return 1;
      }
      break;
   case FILECHECK_DOCHECKSYNC:
      {
         STransfarringError error = Transfarring_ProcessWork();
         switch(error.mHighLevelError)
         {
         case kTE_Processing:
            break;
         case kTE_Success:
            BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);
            
            if( error.mLowLevelError ) // kludge
            {
               work->sub_step=FILECHECK_BEGINSYNC;
            }
            else
            {
               return 1;
            }
            break;
         default:
            BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);

            if(error.mLowLevelError)
            {
               BP_CommonDialog_WantsErrorDialog(error.mLowLevelError);
            }
            else
            {
               BP_CommonDialog_WantsMessageDialog(GetTransfarringStringForErrorCode(error.mHighLevelError), kMDL_OK);
            }
            work->sub_step++;
            break;
         }
      }
      break;
   case FILECHECK_BEGINSYNC:
      if (BP_Network_IsSignedIn())
      {
         BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 1);
         if( work->mode == MCARD_MODE_LOAD )
         {
            Transfarring_PostWork_SyncCloudFiles(work->mcman, 0, 0);
         }
         else
         {
            Transfarring_PostWork_SyncCloudFiles(work->mcman, 0, 1);
         }
         work->sub_step++;
      }
      else
      {
         return 1;
      }
      break;
   case FILECHECK_DOSYNC:
      {
         STransfarringError error = Transfarring_ProcessWork();
         switch(error.mHighLevelError)
         {
         case kTE_Processing:
            break;
         case kTE_Success:
            BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);
            return 1;
         default:
            if(error.mLowLevelError)
            {
               BP_CommonDialog_WantsErrorDialog(error.mLowLevelError);
            }
            else
            {
               BP_CommonDialog_WantsMessageDialog(GetTransfarringStringForErrorCode(error.mHighLevelError), kMDL_OK);
            }
            BP_SetSpinnerLoadFlag(kLoadFlag_CloudSync, 0);
            return 1;
         }
      }
      break;
   }

   return 0;
}

static int PortSelSubStep(Work *work)
{
	// printf("Step = %d %d\n",work->step,work->sub_step);

	switch(work->sub_step){
	case 0:
		if(!work->busy_flag){
			PortModeCheck(work);
			switch(work->port_mode){
			case 1:
				work->action_strcode=CODE_OPENSELSLOT1;
				work->port_cursor=0;
				break;
			case 2:
				work->action_strcode=CODE_OPENSELSLOT2;
				work->port_cursor=1;
				break;
			case 3:
				work->action_strcode=CODE_OPENSELSLOT12;
				break;
			}
			work->sub_step++;
		}
		break;
	case 1:
      // BP DAK - if we have no port by now kick the user out...
      if(work->port_mode == 0)
      {
         SE_CANCEL();
         return -1;
      }

		DispNotFound(work);

		if(!work->busy_flag){
			work->sub_step++;

			if(work->port_mode){
			//LocalShowCursor(work);         //BP JG - avoid showing this cursor for a frame.
			}
			else{
				HideCursor(work);
			}

			work->anim_count=0;
			work->t_ypos=work->cursor->empty.pos.y
				=work->port[work->port_cursor]->sprite.pos.y+CURSOR_ADJUST;
			work->port[work->port_cursor]->sprite.col.a=SEL_ALPHA;
		}
		break;
	case 2:
		// printf("port mode = %d\n",work->port_mode);
		// printf("key = %d\n",work->key_press);

		DispNotFound(work);

		if(MCManCheckingOrChecked()){
			return -2;
		}

		if(work->anim_count>0) break;


      //BP JG - skip port select UI by pressing OK or CANCEL.
      //if ( work->mode == MCARD_MODE_DOGTAG_LOAD )
      {
         if ( work->start_mode == 1 )     // coming back from screen after a port select
         {
            // automatically press cancel
            //SE_CANCEL();
            return -1;
         }
         else
         {
            // first time in, move forward without waiting for a button press
            if(work->port_mode)
            {
               // automatically press ok

               SE_OK();

               work->anim_count=ANIM_COUNT;
               AnimCtrlSet(work,0,work->port[0],0x00);
               AnimCtrlSet(work,1,work->port[1],0x00);

               work->sub_step=0x10;
               break;
            }
         }
      }

      /* BP JG - ^^^ don't need this anymore due to the code above ^^^
		if(work->key_press & PAD_CANCEL){
			SE_CANCEL();
			return -1;
		}
		
      else if(work->key_press & PAD_OK)
      {
			if(work->port_mode){
				SE_OK();

				work->anim_count=ANIM_COUNT;
				AnimCtrlSet(work,0,work->port[0],0x00);
				AnimCtrlSet(work,1,work->port[1],0x00);

				work->sub_step=0x10;
				break;
			}
		}
      */
		{
			int cur0,cur1;

			cur0=work->port_cursor;

			switch(work->port_mode){
			case 1:
				work->port_cursor=0;
				break;
			case 2:
				work->port_cursor=1;
				break;
			case 3:
				switch(work->key_press & (PAD_U|PAD_D)){
				case PAD_U:
					work->port_cursor--;
					if(work->port_cursor<0){
						work->port_cursor=N_PORT_ITEMS-1;
					}
					break;
				case PAD_D:
					work->port_cursor++;
					if(work->port_cursor>=N_PORT_ITEMS){
						work->port_cursor=0;
					}
					break;
				}
				break;
			}

			cur1=work->port_cursor;

			if(cur0!=cur1){
				work->anim_count=ANIM_COUNT;

				work->t_ypos=work->port[work->port_cursor]->sprite.pos.y+CURSOR_ADJUST;

				AnimCtrlSet(work,0,work->port[cur0],UNSEL_ALPHA);
				AnimCtrlSet(work,1,work->port[cur1],SEL_ALPHA);

				SE_SEL();
			}
		}
		break;

	case 0x10:
		if(work->anim_count>0) break;

		work->sub_step++;
		work->timer=DIRECT_TICK(2);
		break;
	case 0x11:
		if(work->timer){
			work->timer--;
			break;
		}
		return 1;
	}

	return 0;
}

static int FileCheckSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		{
			int type=MCManGetCardType(work->port_cursor);

			work->ans=0;

			switch(type){
			case MCMAN_CARDTYPE_NOCARD:
			case MCMAN_CARDTYPE_PS1:
			case MCMAN_CARDTYPE_POCKET_STATION:
			case MCMAN_CARDTYPE_ERRORCARD:
			case MCMAN_CARDTYPE_MULTITAP:
			  if( MCManCheckingOrChecked() ){
				  return -2;
			  }
				if(MCManFileFlagReset()){
					work->sub_step++;
				}
				work->ans=-1;
				break;

			case MCMAN_CARDTYPE_PS2_UNFORMATTED:
			  if( MCManCheckingOrChecked() ){
				  return -2;
			  }
				if(MCManFileFlagReset()){
					work->sub_step++;
				}
				work->ans=1;
				break;

			case MCMAN_CARDTYPE_PS2_FORMATTED:
				switch(work->mode){
				case MCARD_MODE_LOAD:
				case MCARD_MODE_SAVE:
				case MCARD_MODE_DOGTAG_LOAD:
					work->sub_step++;
#if 0
					if(GetGameInfoStart(work->port_cursor)){
						// work->sub_step++;
					}
					else{
						// work->sub_step++;
						work->ans=-2;
					}
#else
					if(GameEasySearchStart(work->port_cursor)){
						// work->sub_step++;
					}
					else{
						// work->sub_step++;
						work->ans=-2;
					}
#endif
					break;
				case MCARD_MODE_PHOTOLOAD:
					work->sub_step++;
#if 0
					if(GetPhotoInfoStart(work->port_cursor)){
						// work->sub_step++;
					}
					else{
						// work->sub_step++;
						work->ans=-2;
					}
#else
					if(GetPhotoInfoBGStart(work->port_cursor)){
						// work->sub_step++;
					}
					else{
						// work->sub_step++;
						work->ans=-2;
					}
#endif
					break;
				case MCARD_MODE_PHOTOSAVE:
					work->sub_step++;

#if 0
					if(GetPhotoInfoStart(work->port_cursor)){
						// work->sub_step++;
					}
					else{
						// work->sub_step++;
						work->ans=-2;
					}
#else
					if(GetPhotoInfoRBGStart(work->port_cursor)){
						// work->sub_step++;
					}
					else{
						// work->sub_step++;
						work->ans=-2;
					}
#endif
					break;
				}
				break;
			}
		}

		work->timer=0;

		break;
	case 1:
		if(work->ans) return work->ans;

		work->action_strcode=CODE_MCCHECK;
		work->sub_step++;
		break;
	case 2:
		if(work->ans) return work->ans;

		if(MCManCheckingOrChecked()){
			return -2;
		}
		switch(MCManGetResult()){
		case 1:
			return 1;
		case -1:
			return -1;
		}
		break;
	}

	return 0;
}

static int CheckError(Work *work)
{
	// return 0;

	switch(work->mode){
	case MCARD_MODE_LOAD:
	case MCARD_MODE_DOGTAG_LOAD:
		if(MCManGetCardType(work->port_cursor)==MCMAN_CARDTYPE_PS2_FORMATTED){
			if(MCManNFiles()>0) return 1;
		}
		break;
	case MCARD_MODE_SAVE:
		if(MCManGetCardType(work->port_cursor)==MCMAN_CARDTYPE_PS2_UNFORMATTED){
			return 1;
		}
		else if(MCManNFiles()>0 ||
				MCManGetFreeSize(work->port_cursor)>=GAME_MC_MAXSIZE){

			return 1;
		}
		break;
	case MCARD_MODE_PHOTOLOAD:
		if(MCManGetCardType(work->port_cursor)==MCMAN_CARDTYPE_PS2_FORMATTED){
			if(MCManNFiles()>0) return 1;
		}
		break;
	case MCARD_MODE_PHOTOSAVE:
		if(MCManGetCardType(work->port_cursor)==MCMAN_CARDTYPE_PS2_UNFORMATTED){
			return 1;
		}
		else if(MCManNFiles()>0 ||
				MCManGetFreeSize(work->port_cursor)>=PHOTO_MC_MAXSIZE){

			return 1;
		}
		break;
	}
	return 0;
}

static int ErrorSubStep(Work *work)
{
	switch(work->sub_step){
	case 0:
		if(!work->busy_flag){
			work->sub_step++;
			HideCursor(work);

         if ( BP_Area_EU() || BP_Area_JP() )
//#if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
   			work->action_strcode=CODE_MCCHECKEND;
//#endif

		}
		break;
#if 0
   case 1:


#  if defined(AREA_EU_BP_IGNORE()) || defined(JAPANESE_BP_IGNORE()) || defined(PAL)	// #if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
		if(work->busy_flag) break;
		work->sub_step++;

	case 2:
#  endif
#else
#endif
   case 1:
      if ( BP_Area_EU() || BP_Area_JP() )
      {
         if(work->busy_flag) break;
         work->sub_step++;
      }
      // BP - JM - note that the fallthrough in both
      // cases is intentional.  If we're not in EU or JP, then we
      // wait on work->busy_flag
      // if we're in US, then case 2 never hits
   case 2:

		DispError(work);

		if(MCManCheckingOrChecked()){
			// work->action_strcode=CODE_CLOSELIST;

#if defined(ENGLISH) && defined(NTSC)
			work->action_strcode=CODE_MCCHECKEND;
#endif


			return -2;
		}

		if(work->key_press & (PAD_CANCEL|PAD_OK)){
			// work->action_strcode=CODE_CLOSELIST;

#if defined(ENGLISH) && defined(NTSC)
			work->action_strcode=CODE_MCCHECKEND;
#endif

         work->start_mode = 1;      //BP JG - this sets that this screen has already ran, so we can auto exit out.
			SE_OK();
			return -1;
		}
		break;
	}

	return 0;
}

static void Step(Work *work)
{
	switch(work->step){
	case MCARD_START:
		if(StartSubStep(work)){
			work->step=MCARD_CHECK;
			work->sub_step=0;
		}
		break;
	case MCARD_CHECK:
		if(CheckSubStep(work)){
			work->step=MCARD_PORT_SEL;
			work->sub_step=0;
		}
		break;
	case MCARD_PORT_SEL:
		switch(PortSelSubStep(work)){
		case 1:
         if( work->mode == MCARD_MODE_LOAD || work->mode == MCARD_MODE_SAVE )
         {
            work->step=MCARD_CLOUD_SYNC;
         }
         else
         {
			   work->step=MCARD_FILE_CHECK;
         }
			work->sub_step=0;
			HideCursor(work);
			break;
      case -1:
			work->ans=TITLE_MSG_MODECANCEL;
			work->step=0x10;
			work->action_strcode=CODE_CLOSEALL;
			HideCursor(work);

			SE_WINCLOSE();
			break;
		case -2:
			work->step=MCARD_CHECK;
			work->sub_step=0;
			HideCursor(work);
			break;
      }
		break;
   case MCARD_CLOUD_SYNC:
      switch(CloudSyncSubStep(work)){
      case 1:
         work->step=MCARD_FILE_CHECK;
         work->sub_step=0;
         break;
      }
      break;
	case MCARD_FILE_CHECK:
		switch(FileCheckSubStep(work)){
		case 1:
			if(work->mode==MCARD_MODE_PHOTOLOAD){
				work->action_strcode=CODE_TOALBUM;
			}
#if 0
			else if(work->port_cursor){
				work->action_strcode=CODE_OPENSELFILE2;
			}
			else{
				work->action_strcode=CODE_OPENSELFILE1;
			}
#endif
			if(CheckError(work)){
				work->ans=TITLE_MSG_MODEEND;
				work->step=0x10;
			}
			else{
				work->step=MCARD_ERROR;
				work->sub_step=0;

#if 0
				if(work->port_cursor){
					work->action_strcode=CODE_OPENSELFILE2;
				}
				else{
					work->action_strcode=CODE_OPENSELFILE1;
				}
#else
				work->action_strcode=0;
#endif
			}
			break;
		case -1:
		case -2:
			work->step=MCARD_CHECK;
			work->sub_step=0;
			break;
		}
		break;
	case MCARD_ERROR:
		switch(ErrorSubStep(work)){
		case 1:
		case -1:
			work->step=MCARD_PORT_SEL;
			work->sub_step=0;
			break;
		case -2:
			work->step=MCARD_CHECK;
			work->sub_step=0;
			break;
		}
		break;

	case 0x10:
		if(!work->busy_flag){
			if(work->signal_flag){
				switch(work->ans){
				case TITLE_MSG_MODEEND:
					GV_CallParentSignalFunc(work,SIGNAL_PORTSEL_OK,work->port_cursor);
					break;
				case TITLE_MSG_MODECANCEL:
					GV_CallParentSignalFunc(work,SIGNAL_PORTSEL_CANCEL,0);
					break;
				}
			}
			GV_DestroyActor(work);
		}
		break;
	}

	AnimCtrl(work);
}

static void Act(Work *work)
{
	if(L2D_ActionStatus(work->l2d_handle)==L2D_STAT_ACK)
   {
		work->busy_flag=0;
		if(work->action_strcode!=0)
      {
         // BP JG - bypass fading up/down of UI messages for port selection.
         if ( (work->action_strcode == 14170859) ||
              (work->action_strcode == 3487334) ||
              (work->action_strcode == 7462188) ||
              (work->action_strcode == 6956960) )
         {
		   	work->action_strcode=0;
         } 
         else
         {
   			int stat=L2D_EvokeAction(work->l2d_handle,work->action_strcode);
#ifdef DEBUG_MODE
	   		printf("L2D Stat = %d\n",stat);
#endif
		   	work->action_strcode=0;
			   work->busy_flag=1;
         }
      }

	}
	else
   {
		work->busy_flag=1;
	}
	TagAnim(work);

	Key(work);
	Step(work);
}

static void Die(Work *work)
{
	if(!work->signal_flag && work->parent_name!=0 && work->parent_name!=1){
		GV_MSG msg;
		int message[4];

		msg.address=work->parent_name;
		msg.message=message;
		msg.message_len=sizeof(message)/sizeof(message[0]);

		message[0]=work->name;
		message[1]=work->ans;
		message[2]=work->port_cursor;

		GV_SendMessage(&msg);
	}

	if(work->handle_flag){
		L2D_SetSignalHandle(work->l2d_handle,NULL,NULL);
		work->handle_flag=0;
	}

#ifdef DEBUG_MODE
	printf("PortSel Finished\n");
#endif

}


/* ------------------------------------------------------------------------ */


static void LayoutSignalFunc(Work *work,int sign,int value)
{
	switch(sign){
	case CODE_HIDE_MODE:
		switch(work->mode){
		case MCARD_MODE_SAVE:
		case MCARD_MODE_PHOTOSAVE:
			TagAnimSet(work,CODE_SHOW_SAVE,CODE_HIDE_SAVE,value/DIV_TICK_VALUE);
			break;
		case MCARD_MODE_LOAD:
			TagAnimSet(work,CODE_SHOW_LOAD,CODE_HIDE_LOAD,value/DIV_TICK_VALUE);
			break;
		case MCARD_MODE_PHOTOLOAD:
			TagAnimSet(work,CODE_SHOW_ALBUMLOAD,CODE_HIDE_ALBUMLOAD,value/DIV_TICK_VALUE);
			break;
		case MCARD_MODE_DOGTAG_LOAD:
			TagAnimSet(work,CODE_SHOW_DOGTAGLOAD,CODE_HIDE_DOGTAGLOAD,value/DIV_TICK_VALUE);
			break;
		}
		break;
	case CODE_SHOW_MODE:
		switch(work->mode){
		case MCARD_MODE_SAVE:
		case MCARD_MODE_PHOTOSAVE:
			TagAnimSet(work,CODE_HIDE_SAVE,CODE_SHOW_SAVE,value/DIV_TICK_VALUE);
			break;
		case MCARD_MODE_LOAD:
			TagAnimSet(work,CODE_HIDE_LOAD,CODE_SHOW_LOAD,value/DIV_TICK_VALUE);
			break;
		case MCARD_MODE_PHOTOLOAD:
			TagAnimSet(work,CODE_HIDE_ALBUMLOAD,CODE_SHOW_ALBUMLOAD,value/DIV_TICK_VALUE);
			break;
		case MCARD_MODE_DOGTAG_LOAD:
			TagAnimSet(work,CODE_HIDE_DOGTAGLOAD,CODE_SHOW_DOGTAGLOAD,value/DIV_TICK_VALUE);
			break;
		}
		break;
	case CODE_SOUND:
		if(work->step==0x10) SE_EXPANDLINE();
		else SE_WINOPEN();
		break;
	}
}

static void InitMenu(Work *work,char *mcstr,char *mcstr2)
{
	work->cursor=L2D_GetObject(work->l2d_handle,CODE_SELCURSNULL);
	work->cursor_spr=L2D_GetObject(work->l2d_handle,CODE_SELCURS);

	work->title[0]=L2D_GetObject(work->l2d_handle,CODE_DATALOADTAG);
	work->title[1]=L2D_GetObject(work->l2d_handle,CODE_DATASAVETAG);

	work->port[0]=L2D_GetObject(work->l2d_handle,CODE_SLOT1);
	work->port[1]=L2D_GetObject(work->l2d_handle,CODE_SLOT2);

	work->port_cursor=0;
	work->port_mode=0;

   DG_WaitForThreadedRenderComplete();
   BP_WaitForLastRenderThreadToComplete();

	MENU_ClearTextTexture(work->strman);

	/* 「メモリーカードが見つかりません」のテクスチャ作成 */
	MENU_CreateTextTexture(work->strman,TEXT_U,TEXT_V,
						   TEXT_WIDTH,TEXT_HEIGHT2,0,0,0,mcstr);

	/* 「ファイルがありません」もしくは「空き領域がありません」のテクスチャ作成 */
	MENU_CreateTextTexture(work->strman,TEXT2_U,TEXT2_V,
						   TEXT2_WIDTH,TEXT2_HEIGHT2,0,0,0,mcstr2);

	work->handle_flag=0;

	if(L2D_GetSignalHandle(work->l2d_handle)==NULL){
		/* シグナルハンドラの設定 */
		L2D_SetSignalHandle(work->l2d_handle,work,(void (*)(void *,int,int))LayoutSignalFunc);
		work->handle_flag=1;
	}
}


/* ------------------------------------------------------------------------ */


/* 初期化部メイン */
void *NewPortsel(int name,int parent_name,int l2d_handle,
				 int title_mode,int start_mode,void *strman,int resname,void* mcman)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		/* ロード用、リソースをローカルから取る */
      char *mcstr=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(resname,RESOURCE_MCNOTFOUND));
		char *mcstr2=BP_GCL_LOOKUP_NEW_FONT_STRING((char *)GetLocalResource(resname,RESOURCE_ERROR));

		work->parent_name=parent_name;
		work->name=name;
		work->l2d_handle=l2d_handle;
		work->mode=title_mode;
		work->signal_flag=0;

		work->step=0;
		work->sub_step=0;
		work->ans=0;

		work->strman=strman;
      work->mcman=mcman;

		work->start_mode=start_mode;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		InitMenu(work,mcstr,mcstr2);
    }
    return (void *)work ;
}


void *NewPortsel2(int name,int parent_name,int l2d_handle,
				  int title_mode,int start_mode,void *strman,int resname,void* mcman)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work));

    if(work!=NULL) {
		/* セーブ用、リソースを常駐から取る */
		//char *mcstr=(char *)GM_GetResource(resname,RESOURCE_MCNOTFOUND);
		//char *mcstr2=(char *)GM_GetResource(resname,RESOURCE_ERROR);

		work->parent_name=parent_name;
		work->name=name;
		work->l2d_handle=l2d_handle;
		work->mode=title_mode;
		work->signal_flag=0;

		work->step=0;
		work->sub_step=0;
		work->ans=0;

		work->strman=strman;

		work->start_mode=start_mode;

      work->mcman=mcman;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

      //MGSTWO-3419 - Was displaying PS2 strings at the exit of the savegame menu.
		InitMenu(work,"","");
    }
    return (void *)work ;
}


/* 初期化部メイン */
void *NewPortselForCodec(int name,int parent_name,int l2d_handle,
						 int title_mode,int start_mode,void *strman,int resname,void* mcman)
{
    Work *work ;
	//char *mcstr=(char *)GM_GetResource(resname,RESOURCE_MCNOTFOUND);
	//char *mcstr2=(char *)GM_GetResource(resname,RESOURCE_ERROR);

	if((work=(Work *)codecMalloc(sizeof(Work)))==NULL){
		return NULL;
	}

	/* アクター起動 */
	GV_ZeroMemory(work,sizeof(Work));
	GV_SetActorFreeFunc(work,codecFree);
	GV_SetActorClass(work,GV_CLASS_CHARA);
	GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority(ACT_MODE,work,0x1FF); /* ACT_MODE <--- user/mode/codec/codecmem.h */

	GV_SetActor(&(work->actor),Act,Die);
	GV_ActorEX(&(work->actor));

	work->parent_name=parent_name;
	work->name=name;
	work->l2d_handle=l2d_handle;
	work->mode=title_mode;
	work->signal_flag=1;

	work->start_mode=start_mode;

	work->step=0;
	work->sub_step=0;
	work->ans=0;

	work->strman=strman;
   work->mcman=mcman;

	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX(&(work->actor));
	GV_SetActorMessageKill(&(work->actor),name);

   //MGSTWO-3419 - Was displaying PS2 strings at the exit of the savegame menu.
	InitMenu(work,"","");

    return (void *)work ;
}
