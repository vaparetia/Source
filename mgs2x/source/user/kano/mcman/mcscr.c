//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mcscr.c
		メモリーカードセーブロード画面
	
	2001/04/24 K.Kano
	$Id: mcscr.c,v 1.1.1.3 2002/11/19 11:43:17 Yoshizawa1 Exp $
*/


#include "mcman.h"

#include "../../mode/codec/codec_signal.h"
#include "../../mode/codec/codecmem.h"
#include "../../mode/codec/cdc_load.h"


#define N_FILES			MCSCR_N_FILES

#define NO_SAVEFILE		0xffff


enum {
	MCSCR_CHECK_CARD=0,
	MCSCR_GET_CARDINFO,
	MCSCR_WAIT_GET_CARDINFO,
	MCSCR_SEL_FILE,

	MCSCR_CONFIRM,
	MCSCR_START_SAVELOAD,
	MCSCR_WAIT_SAVELOAD,

	MCSCR_SUCCESS_SAVELOAD,
	MCSCR_FAILED_SAVELOAD,
};


static void SearchSaveFile(MCScrWork *work)
{
	int i=0;
	while(i<FILE_MAX){
		if(!MCManCheckFileFlag(i)){
			work->save_index=i;
			return;
		}
		i++;
	}
	work->save_index=NO_SAVEFILE;
}

static int SearchUpFile(int start)
{
	int i=start;
	while(i<FILE_MAX){
		if(MCManCheckFileFlag(i)) return i;
		i++;
	}
	return -1;
}

static int SearchDownFile(int start)
{
	int i=start;
	while(i>=0){
		if(MCManCheckFileFlag(i)) return i;
		i--;
	}
	return -1;
}

static void InitSearchFile(MCScrWork *work)
{
	int i;
	int start=0;

	for(i=0;i<N_FILES;i++){
		int j;
		j=SearchUpFile(start);
		if(j==-1){
			work->n_indexes=i;
			return;
		}
		else{
			work->indexes[i]=j;
			start=j+1;
		}
	}
	work->n_indexes=N_FILES;
}


/* --------------------------------------------------------------- */
/* 表示関係は仮                                                    */
/* 全部デバッグ表示で作ってある                                    */
/* --------------------------------------------------------------- */


static int SelFile_MoveCursor(MCScrWork *work)
{
	int pad=GV_PadDataDirect[0].press;

	switch(pad & (PAD_A|PAD_B)){
	case PAD_A:
		if(work->file_cursor<work->n_indexes ||
		   (work->file_cursor==work->n_indexes && work->save_index!=NO_SAVEFILE)){

			return 1;
		}
		break;
	case PAD_B:
		return -1;
	}

	switch(pad & (PAD_L|PAD_R)){
	case PAD_L:
	case PAD_R:
		work->port^=1;
		return -2;
	}

	switch(pad & (PAD_U|PAD_D)){
	case PAD_U:
		if(work->file_cursor==0){
			if(work->n_indexes>0){
				int tindex=SearchDownFile(work->indexes[0]-1);

				if(tindex>=0){
					int i=N_FILES-1;
					while(i>0){
						work->indexes[i]=work->indexes[i-1];
						i--;
					}
					work->indexes[0]=tindex;
					work->n_indexes++;
					if(work->n_indexes>N_FILES) work->n_indexes=N_FILES;
				}
			}
		}
		else{
			work->file_cursor--;
		}
		break;
	case PAD_D:
		if(work->file_cursor==N_FILES-1){
			int tindex=SearchUpFile(work->indexes[work->n_indexes-1]+1);

			if(tindex>=0){
				int i=0;
				while(i<N_FILES-1){
					work->indexes[i]=work->indexes[i+1];
					i++;
				}
				work->indexes[N_FILES-1]=tindex;
				work->n_indexes++;
				if(work->n_indexes>N_FILES) work->n_indexes=N_FILES;
			}
			else if(work->save_index!=NO_SAVEFILE &&
					work->n_indexes==N_FILES){

				int i=0;
				while(i<N_FILES-1){
					work->indexes[i]=work->indexes[i+1];
					i++;
				}
				work->n_indexes--;
			}
		}
		else if(work->file_cursor<work->n_indexes-1 ||
				(work->save_index!=NO_SAVEFILE && work->file_cursor<work->n_indexes)){

			work->file_cursor++;
		}
		break;
	}

	return 0;
}

static int Confirm_MoveCursor(MCScrWork *work)
{
	int pad=GV_PadDataDirect[0].press;

	switch(pad & (PAD_A|PAD_B)){
	case PAD_A:
		return 1;
	case PAD_B:
		return -1;
	}

	switch(pad & (PAD_U|PAD_D)){
	case PAD_U:
	case PAD_D:
		work->confirm_cursor^=1;
		break;
	}

	return 0;
}

static void DispFrame(MCScrWork *work)
{
    DEBUG_Locate(0x40,0x30,MENU_MODE_NORMAL);
	DEBUG_Printf("Port %1d",work->port);

    DEBUG_Locate(0x80,0x30,MENU_MODE_NORMAL);
	switch(work->mode){
	case MCSCR_MODE_LOADGAME:
		DEBUG_Printf("Load Game");
		break;
	case MCSCR_MODE_SAVEGAME:
		DEBUG_Printf("Save Game");
		break;
	case MCSCR_MODE_LOADPHOTO:
		DEBUG_Printf("Load Photo");
		break;
	case MCSCR_MODE_SAVEPHOTO:
		DEBUG_Printf("Save Photo");
		break;
	}

	if(work->flags & 2){
		DEBUG_Locate(0x40,0x40,MENU_MODE_NORMAL);
		DEBUG_Printf("/\\");
	}
	if(work->flags & 1){
		DEBUG_Locate(0x40,0xb8,MENU_MODE_NORMAL);
		DEBUG_Printf("\\/");
	}
}

static void DispFiles(MCScrWork *work)
{
	int i;
	int type=MCManGetCardType(work->port);

	switch(type){
	case MCMAN_CARDTYPE_ERRORCARD:
		DEBUG_Locate(0x80,0x40+(N_FILES/2)*0x10,MENU_MODE_NORMAL);
		DEBUG_Printf("Error Card");
		return;

	case MCMAN_CARDTYPE_NOCARD:
	case MCMAN_CARDTYPE_PS1:
	case MCMAN_CARDTYPE_POCKET_STATION:
	case MCMAN_CARDTYPE_MULTITAP:
		DEBUG_Locate(0x80,0x40+(N_FILES/2)*0x10,MENU_MODE_NORMAL);
		DEBUG_Printf("No Card");
		return;

	case MCMAN_CARDTYPE_PS2_UNFORMATTED:
		if(!(work->mode & 1)){
			DEBUG_Locate(0x80,0x40+(N_FILES/2)*0x10,MENU_MODE_NORMAL);
			DEBUG_Printf("Unformatted");
			return;
		}
		break;
	}

	for(i=0;i<N_FILES;i++){
		DEBUG_Locate(0x60,0x40+i*0x10,MENU_MODE_NORMAL);

		if(i<work->n_indexes){
			DEBUG_Printf("File %3d",work->indexes[i]);
		}
		else if(i==work->n_indexes &&
				work->save_index!=NO_SAVEFILE){

			DEBUG_Printf("New File");
		}
		else break;
	}

	DEBUG_Locate(0x40,0x40+work->file_cursor*0x10,MENU_MODE_NORMAL);
	DEBUG_Printf(">>");
}

static void DispCheckCard(MCScrWork *work)
{
    DEBUG_Locate(0x80,0x40+(N_FILES/2)*0x10,MENU_MODE_NORMAL);
	DEBUG_Printf("Now Checking MemoryCard");
}

static void DispConfirm(MCScrWork *work)
{
	switch(work->mode){
	case MCSCR_MODE_LOADGAME:
		DEBUG_Locate(0x80,0x50+N_FILES*0x10,MENU_MODE_NORMAL);
		DEBUG_Printf("Would you like to Load Game data ?");
		break;
	case MCSCR_MODE_SAVEGAME:
		DEBUG_Locate(0x80,0x50+N_FILES*0x10,MENU_MODE_NORMAL);
		DEBUG_Printf("Would you like to Save Game data ?");
		break;
	case MCSCR_MODE_LOADPHOTO:
		DEBUG_Locate(0x80,0x50+N_FILES*0x10,MENU_MODE_NORMAL);
		DEBUG_Printf("Would you like to Load Photo data ?");
		break;
	case MCSCR_MODE_SAVEPHOTO:
		DEBUG_Locate(0x80,0x50+N_FILES*0x10,MENU_MODE_NORMAL);
		DEBUG_Printf("Would you like to Save Photo data ?");
		break;
	}

	DEBUG_Locate(0xa0,0x60+N_FILES*0x10,MENU_MODE_NORMAL);
	DEBUG_Printf("Yes");

	DEBUG_Locate(0xa0,0x70+N_FILES*0x10,MENU_MODE_NORMAL);
	DEBUG_Printf("No");

	DEBUG_Locate(0x80,0x60+N_FILES*0x10+work->confirm_cursor*0x10,MENU_MODE_NORMAL);
	DEBUG_Printf(">>");
}

static void DispIng(MCScrWork *work)
{
    DEBUG_Locate(0x80,0x40+(N_FILES/2)*0x10,MENU_MODE_NORMAL);

	switch(work->mode){
	case MCSCR_MODE_LOADGAME:
		DEBUG_Printf("Now Loading Game data");
		break;
	case MCSCR_MODE_SAVEGAME:
		if(MCManFormatting()){
			DEBUG_Printf("Now Formatting");
		}
		else{
			DEBUG_Printf("Now Saving Game data");
		}
		break;
	case MCSCR_MODE_LOADPHOTO:
		DEBUG_Printf("Now Loading Photo data");
		break;
	case MCSCR_MODE_SAVEPHOTO:
		if(MCManFormatting()){
			DEBUG_Printf("Now Formatting");
		}
		else{
			DEBUG_Printf("Now Saving Photo data");
		}
		break;
	}
}

static void DispSuccess(MCScrWork *work)
{
    DEBUG_Locate(0x80,0x40+(N_FILES/2)*0x10,MENU_MODE_NORMAL);

	switch(work->mode){
	case MCSCR_MODE_LOADGAME:
		DEBUG_Printf("Completed to Load Game data");
		break;
	case MCSCR_MODE_SAVEGAME:
		DEBUG_Printf("Completed to Save Game data");
		break;
	case MCSCR_MODE_LOADPHOTO:
		DEBUG_Printf("Completed to Load Photo data");
		break;
	case MCSCR_MODE_SAVEPHOTO:
		DEBUG_Printf("Completed to Save Photo data");
		break;
	}
}

static void DispFailed(MCScrWork *work)
{
    DEBUG_Locate(0x80,0x40+(N_FILES/2)*0x10,MENU_MODE_NORMAL);

	switch(work->mode){
	case MCSCR_MODE_LOADGAME:
		DEBUG_Printf("Failed to Load Game data");
		break;
	case MCSCR_MODE_SAVEGAME:
		DEBUG_Printf("Failed to Save Game data");
		break;
	case MCSCR_MODE_LOADPHOTO:
		DEBUG_Printf("Failed to Load Photo data");
		break;
	case MCSCR_MODE_SAVEPHOTO:
		DEBUG_Printf("Failed to Save Photo data");
		break;
	}
}


/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */


static void MCScrBackCheckCard(MCScrWork *work)
{
	work->step=MCSCR_CHECK_CARD;
	work->flags=0;
	work->n_indexes=0;
	work->save_index=NO_SAVEFILE;

#ifdef DEBUG_MODE
	printf("Next is MCSCR_CHECK_CARD\n");
#endif
}

static void MCScrBackGetCardInfo(MCScrWork *work)
{
	work->step=MCSCR_GET_CARDINFO;
	work->flags=0;
	work->n_indexes=0;
	work->save_index=NO_SAVEFILE;

#ifdef DEBUG_MODE
	printf("Next is MCSCR_GET_CARDINFO\n");
#endif
}

void MCScrInit(MCScrWork *work,int mode,void *data,int size,void *icondata,int iconsize,void *info)
{
	work->port=0;
	work->step=0;
	work->mode=mode;

	work->flags=0;
	work->n_indexes=0;
	work->save_index=NO_SAVEFILE;

	work->data=data;
	work->size=size;
	work->icondata=icondata;
	work->iconsize=iconsize;
	work->info=info;
}

/* 画面まわりはどうしよう？ */
int MCScrAct(MCScrWork *work)
{
	switch(work->step){
	case MCSCR_CHECK_CARD:
		if(MCManChecked()){
			work->step++;

#ifdef DEBUG_MODE
			printf("Next is MCSCR_GET_CARDINFO\n");
#endif

		}

		DispCheckCard(work);
		break;

	case MCSCR_GET_CARDINFO:
		{
			int type=MCManGetCardType(work->port);

			switch(type){
			case MCMAN_CARDTYPE_NOCARD:
			case MCMAN_CARDTYPE_PS1:
			case MCMAN_CARDTYPE_POCKET_STATION:
			case MCMAN_CARDTYPE_ERRORCARD:
			case MCMAN_CARDTYPE_MULTITAP:
				work->step=MCSCR_SEL_FILE;
				work->n_indexes=0;
				work->save_index=NO_SAVEFILE;

#ifdef DEBUG_MODE
				printf("Next is MCSCR_SEL_FILE\n");
#endif
				break;

			case MCMAN_CARDTYPE_PS2_UNFORMATTED:
				work->step=MCSCR_SEL_FILE;
				work->n_indexes=0;
				work->save_index=0;

#ifdef DEBUG_MODE
				printf("Next is MCSCR_SEL_FILE\n");
#endif
				break;

			case MCMAN_CARDTYPE_PS2_FORMATTED:
				if(work->mode & 2){
					if(GetPhotoInfoStart(work->port)){
						work->step++;

#ifdef DEBUG_MODE
						printf("Next is MCSCR_WAIT_GET_CARDINFO\n");
#endif
					}
					else{
						MCScrBackCheckCard(work);
					}
				}
				else{
					if(GetGameInfoStart(work->port)){
						work->step++;

#ifdef DEBUG_MODE
						printf("Next is MCSCR_WAIT_GET_CARDINFO\n");
#endif
					}
					else{
						MCScrBackCheckCard(work);
					}
				}
				break;
			}
		}

		DispCheckCard(work);
		break;

	case MCSCR_WAIT_GET_CARDINFO:
		if(MCManCheckingOrChecked()){
			MCScrBackCheckCard(work);
			break;
		}
		switch(MCManGetResult()){
		case 1:
			work->step++;

#ifdef DEBUG_MODE
			printf("Next is MCSCR_SEL_FILE\n");
#endif

			InitSearchFile(work);
			if(work->mode & 1) SearchSaveFile(work);
			else work->save_index=NO_SAVEFILE;
			break;
		case -1:
			work->step++;

#ifdef DEBUG_MODE
			printf("Next is MCSCR_SEL_FILE\n");
#endif

			work->n_indexes=0;
			work->save_index=NO_SAVEFILE;
			break;
		}

		DispCheckCard(work);
		break;

	case MCSCR_SEL_FILE:
		if(MCManCheckingOrChecked()){
			MCScrBackCheckCard(work);
			break;
		}
		switch(SelFile_MoveCursor(work)){
		case 1:
			work->step++;
			work->confirm_cursor=1; /* "No" */
			break;
		case -1:
			return -1;
		case -2:
			MCScrBackGetCardInfo(work);
			break;
		}
		DispFiles(work);
		break;

	case MCSCR_CONFIRM:
		if(MCManCheckingOrChecked()){
			MCScrBackCheckCard(work);
			break;
		}
		switch(Confirm_MoveCursor(work)){
		case 1:
			if(work->confirm_cursor) work->step--;
			else work->step++;
			break;
		case -1:
			work->step--;
			break;
		}
		DispConfirm(work);
		DispFiles(work);
		break;
	case MCSCR_START_SAVELOAD:
		work->id=work->indexes[work->file_cursor];
		if(work->file_cursor==work->n_indexes){
			work->id=work->save_index;
		}

		switch(work->mode){
		case MCSCR_MODE_LOADGAME:
			if(LoadGameStart(work->port,work->id,work->data)){
				work->step++;
			}
			else{
				MCScrBackCheckCard(work);
			}
			break;
		case MCSCR_MODE_SAVEGAME:
			/* アイコン関連の設定を入れる予定 */

			/* シナリオからセーブするデータを取得 */
			SaveGameData(work->data,work->id, work->mcman.file_kind);
			MC_InitIconEnv( work->id, MCMAN_FILE_KIND_GAME );
			SaveGameInfo(work->info);

			if(SaveGameStart(work->port,work->id,work->data,work->size,
							 work->icondata,work->iconsize,work->info)){

				work->step++;
			}
			else{
				MCScrBackCheckCard(work);
			}
			break;
		case MCSCR_MODE_LOADPHOTO:
			if(LoadPhotoStart(work->port,work->id,work->data)){
				work->step++;
			}
			else{
				MCScrBackCheckCard(work);
			}
			break;
		case MCSCR_MODE_SAVEPHOTO:
			/* アイコン関連の設定を入れる予定 */
			MCMan_PhotoIconEncode(work->icondata);

			if(SavePhotoStart(work->port,work->id,work->data,work->size,
							  NULL,0,work->info)){

				work->step++;
			}
			else{
				MCScrBackCheckCard(work);
			}
			break;
		}
		break;
	case MCSCR_WAIT_SAVELOAD:
		if(MCManCheckingOrChecked()){
			MCScrBackCheckCard(work);
			break;
		}
		switch(MCManGetResult()){
		case 1:
			work->step=MCSCR_SUCCESS_SAVELOAD;
			work->counter=0;
			break;
		case -1:
			work->step=MCSCR_FAILED_SAVELOAD;
			work->counter=0;
			break;
		}

		DispIng(work);
		break;

	case MCSCR_SUCCESS_SAVELOAD:
		DispSuccess(work);

		work->counter++;
		if(work->counter>3*60){
			switch(work->mode){
			case MCSCR_MODE_LOADGAME: /* Load Game */
				LoadGameData(work->data, work->mcman.file_kind);
				break;
			case MCSCR_MODE_SAVEGAME: /* Save Game */
				break;
			case MCSCR_MODE_LOADPHOTO: /* Load Photo */
			case MCSCR_MODE_SAVEPHOTO: /* Save Photo */
				break;
			}
			return 1;
		}
		break;
	case MCSCR_FAILED_SAVELOAD:
		DispFailed(work);

		work->counter++;
		if(work->counter>3*60){
			MCScrBackGetCardInfo(work);
		}
		break;
	}

	DispFrame(work);

	return 0;
}

void MCScrDie(MCScrWork *work)
{
}


/* --------------------------------------------------------------- */
/* --------------------------------------------------------------- */

typedef struct {
	MCScrWork mcscr;
	MCMAN_GAMEDATA savedata;
	MCMAN_INFODATA info;

	int proc;
	int ans;
} Work;

static void Act(Work *work)
{
	switch(MCScrAct(&(work->mcscr))){
	case 1:
		work->ans=1;
		GV_DestroyActor(work);
		break;
	case -1:
		work->ans=0;
		GV_DestroyActor(work);
		break;
	}
}

static void Die(Work *work)
{
	if(work->proc!=0 && work->proc!=1){
		GCL_ARGS arg;
		int argv[1];

		arg.argc=sizeof(argv)/sizeof(argv[0]);
		arg.argv=argv;

		argv[0]=work->ans;

		GCL_ExecProc(work->proc,&arg);
	}

	MCScrDie(&(work->mcscr));
}


/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
	void *icondata=NULL;
	int mode=0;
	int iconsize=0;

	work->proc=0;

	/* ゲームロード ... 0, ゲームセーブ ... 1
	   写真ロード   ... 2, 写真セーブ   ... 3 */
    if(GCL_GetOption('m')!=NULL){
		mode=GCL_GetNextInt();
    }
    if(GCL_GetOption('p')!=NULL){
		work->proc=GCL_GetNextInt();
    }
	if(GCL_GetOption('i')!=NULL){
		int code=GCL_GetNextInt();
		if((icondata=GV_GetCache(GV_CacheID(code,'r')))!=NULL){
			iconsize=MCAccessCalcIconSize(icondata);
		}
	}

	switch(mode){
	case 0:
		MCScrInit(&(work->mcscr),mode,&(work->savedata),sizeof(work->savedata),
				  icondata,iconsize,&(work->info));
		break;
	case 1:
		MCScrInit(&(work->mcscr),mode,&(work->savedata),sizeof(work->savedata),
				  icondata,iconsize,&(work->info));
		break;
	default:
		MCScrInit(&(work->mcscr),mode,NULL,0,NULL,0,NULL);
		break;
	}
}

void *NewMCSaveLoad(void)
{
	Work *work;

    work=(Work *)GV_NewActor(GV_ACTOR_MANAGER,sizeof(*work));

    if(work!=NULL) {
		GV_SetActor(&(work->mcscr.actor),Act,Die) ;
		GV_ActorEX(&(work->mcscr.actor));

		NewMCMan(&(work->mcscr.mcman));

		GV_SetActorChild(work,&(work->mcscr.mcman));

		GetOptionValue(work);
    }

    return (void *)work ;
}

void NewMCSaveLoad_called(MCScrWork *work,int mode,void *data,int size,
						  void *icondata,int iconsize,void *info)
{
	GV_SetActorClass(work,GV_CLASS_CHARA);
	GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority(GV_ACTOR_MANAGER,work,0x1FF);

	GV_SetActor(&(work->actor),Act,Die);
	GV_ActorEX(&(work->actor));

	MCScrInit(work,mode,data,size,icondata,iconsize,info);

	NewMCMan(&(work->mcman));

	GV_SetActorChild(work,&(work->mcman));
}


#if 0


typedef struct {
	MCScrWork mcscr;
	MCMAN_GAMEDATA savedata;
	MCMAN_INFODATA info;
} CODEC_SaveWork;


static void CODEC_Act(CODEC_SaveWork *work)
{
	switch(MCScrAct(&(work->mcscr))){
	case 1:
	case -1:
		GV_CallParentSignalFunc(work,CDC_SIGNAL_SAVE_DIE,0);
		GV_DestroyActor(work);
		break;
	}
}

static void CODEC_Die(CODEC_SaveWork *work)
{
	MCScrDie(&(work->mcscr));
}

void *CODEC_NewMCSave(int iconname)
{
	CODEC_SaveWork *work;
	void *icon;
	int iconsize;

	if((icon=CDC_GetFileEntry(iconname,'r'))==NULL){
#ifdef DEBUG
		printf("CODEC_NewMCSave : icon not found\n");
#endif
		return NULL;
	}
	iconsize=MCAccessCalcIconSize(icon);

	if((work=(CODEC_SaveWork *)codecMalloc(sizeof(CODEC_SaveWork)))==NULL){
#ifdef DEBUG
		printf("CODEC_NewMCSave : memory not enough\n");
#endif
		return NULL;
	}


	/* アクター起動 */
	GV_ZeroMemory(work,sizeof(CODEC_SaveWork));
	GV_SetActorFreeFunc(work,codecFree);
	GV_SetActorClass(work,GV_CLASS_CHARA);
	GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority(ACT_MODE,work,0x1FF); /* ACT_MODE <--- user/mode/codec/codecmem.h */

	GV_SetActor(&(work->mcscr.actor),CODEC_Act,CODEC_Die);
	GV_ActorEX(&(work->mcscr.actor));


	/* シナリオからセーブするデータを取得 */
	SaveGameData(&(work->savedata));
	SaveGameInfo(&(work->info));


	MCScrInit(&(work->mcscr),
			  MCSCR_MODE_SAVEGAME,&(work->savedata),sizeof(work->savedata),
			  icon,iconsize,&(work->info));

	NewMCMan(&(work->mcscr.mcman));


	GV_SetActorChild(work,&(work->mcscr.mcman));

	return work;
}


#endif
