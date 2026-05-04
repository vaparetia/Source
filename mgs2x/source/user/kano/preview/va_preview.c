/*
	va_preview.c
		頂点アニメプレビュー

	1999/12/13 K.Kano
	$Id: va_preview.c,v 1.1.1.3 2002/11/19 11:43:31 Yoshizawa1 Exp $
*/


#include "preview_def.h"


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

#define N_PROGRAMS	20

#define COUNT		60


/* ワーク */
typedef	struct _Work {
    GV_ACT_EX	actor;		/* リンク */
    OBJECT		body;

    FMATRIX		lights[2];

    /* ワールド座標に表示する場合の位置と回転 */
    FVECTOR		pos;
    SVECTOR		rot;

    /* 人間モデルにくっつけるモードの追加。
       一部すり替えモード */
    int			target_human_index;
    int			human_object_index;

    VERTEX_ANIME_WORK	**va;

    CV2_DEF		**cv2_defs;
    char		**filename;

    int			n_models;
    int			filename_size;

    int			ptnnum0;
    int			ptnnum1;
    int			disp_enable;
    int			count;

    /* モード切替
       0 ... 通常アニメーションモード
       1 ... プログラムアニメーションモード
       2 ... プログラムアニメーション再生モード
       3 ... 移動/回転モード */
    int	mode;

    /* 現在のカーソル位置 (プログラムモード) */
    int cursole;

    /* 現在の再生位置 */
    int playpoint;

    /* 再生の限界位置 */
    int playlimit;

    struct {
		int ptn;
		int count;
    } programs[N_PROGRAMS];

    /* 移動モード時のカーソル位置 */
    int	mov_cursole;
    int speed_mode;

    /* 次のワークへのポインタ。*/
    struct _Work *next;
} Work ;


enum {
    VA_MODE_NORMAL_ANIME=0,
    VA_MODE_PROGRAM_ANIME,
    VA_MODE_PLAY_ANIME,

    VA_MODE_MOVE_ROTATE,
};


typedef struct {
    int n_frames;
    struct {
		int strcode;
		int count;
    } programs[N_PROGRAMS];
} BIN_SAVEFILE;


/* ------------------------------------------------------------------------ */
/* ファイルセーブ/ロード */


#define SAVEFILENAME	"host0:./vanime.dat"


static int VAPreview_FileSave(Work *work)
{
    BIN_SAVEFILE buf;
    int i;

    buf.n_frames=work->playlimit;

    for(i=0;i<N_PROGRAMS;i++){
		if(i<work->playlimit){
			buf.programs[i].strcode=GV_StrCode(*(work->filename+work->programs[i].ptn));
			buf.programs[i].count=work->programs[i].count;
		}
		else{
			buf.programs[i].strcode=0;
			buf.programs[i].count=0;
		}
    }


	if(!PreviewSaveBinFile(SAVEFILENAME,&buf,sizeof(buf))){
		return 0;
	}

    printf("Successed : Write %s .\n",SAVEFILENAME);
    return 1;
}

static int VAPreview_FileLoad(Work *work)
{
    BIN_SAVEFILE buf;
    int i,j;

	if(!PreviewLoadBinFile(SAVEFILENAME,&buf,sizeof(buf))){
		return 0;
	}

    if(buf.n_frames>N_PROGRAMS){
		printf("Illeagal the number of frames.\n");
		return 0;
    }

    for(i=0;i<buf.n_frames;i++){
		for(j=0;j<work->filename_size;j++){
			if(buf.programs[i].strcode==GV_StrCode(*(work->filename+j))){
				break;
			}
		}
		if(j==work->filename_size){
			printf("VAnime Pattern not found.\n");
			return 0;
		}
		buf.programs[i].strcode=j;
    }

    work->playlimit=buf.n_frames;

    for(i=0;i<N_PROGRAMS;i++){
		work->programs[i].ptn=buf.programs[i].strcode;
		work->programs[i].count=buf.programs[i].count;
		if(work->programs[i].count<1) work->programs[i].count=1;
		else if(work->programs[i].count>999) work->programs[i].count=999;
    }

    printf("Successed : Read %s .\n",SAVEFILENAME);

    return 1;
}

/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void PutWork(Work *work)
{
    Work *list=(Work *)(PreviewVA.va);

    PreviewVA.va_size++;
    work->next=NULL;

    if(list==NULL){
		PreviewVA.va=work;
    }
    else{
		while(list->next!=NULL) list=list->next;
		list->next=work;
    }
}

static Work *GetWork(Work *work,int index)
{
    while(index>0){
		if(work!=NULL) work=work->next;
		index--;
    }
    return work;
}

static void VAPreview_DebugTitlePrint(void)
{
    Work *work;
    int x,y;

    x=LOCATE_X;
    y=LOCATE_Y;

    work=GetWork(PreviewVA.va,PreviewVA.va_index);

    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Model List ");
    if(PreviewVA.va_size>1) DEBUG_Printf("%d",PreviewVA.va_index);

    if(work->disp_enable){
		DEBUG_Printf("  <Visible   :L1>");
    }
    else{
		DEBUG_Printf("  <Invisible :L1>");
    }
}

static void VAPreview_DebugNormalPrint(void)
{
    Work *work;
    int x,y,yy;
    int i;

    work=GetWork(PreviewVA.va,PreviewVA.va_index);

    x=LOCATE_X;
    y=LOCATE_Y+COLUMN_HEIGHT;

    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<Normal Preview Mode :L2>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Count      : %d",work->count);

    x+=FONT_WIDTH*8;
    y+=COLUMN_HEIGHT;
    yy=y;
    for(i=0;i<work->filename_size;i++){
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf(work->filename[i]);
		y+=COLUMN_HEIGHT;
    }

    y=yy;
    x-=FONT_WIDTH*8;

    DEBUG_Locate(x,y+work->ptnnum0*COLUMN_HEIGHT,MENU_MODE_NORMAL);
    DEBUG_Printf("R1>");

    x+=FONT_WIDTH*4;
    DEBUG_Locate(x,y+work->ptnnum1*COLUMN_HEIGHT,MENU_MODE_NORMAL);
    DEBUG_Printf("R2>");
}

static void VAPreview_DebugProgramPrint(void)
{
    Work *work;
    int x,y;
    int i;

    work=GetWork(PreviewVA.va,PreviewVA.va_index);

    x=LOCATE_X;
    y=LOCATE_Y+COLUMN_HEIGHT;

    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<Program Record Mode :L2>");

    x+=FONT_WIDTH*0x04;
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("VAnime File Load");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("VAnime File Save");

    y+=COLUMN_HEIGHT;
    y+=COLUMN_HEIGHT/2;
    for(i=0;i<N_PROGRAMS-1 && i<work->playlimit-1;i++){
		DEBUG_Locate(x,y+i*COLUMN_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf("%3d",work->programs[i].count);
    }
    for( ;i<N_PROGRAMS-1;i++){
		DEBUG_Locate(x,y+i*COLUMN_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf("---");
    }

    x+=FONT_WIDTH*0x04;
    y-=COLUMN_HEIGHT/2;
    for(i=0;i<N_PROGRAMS && i<work->playlimit;i++){
		DEBUG_Locate(x,y+i*COLUMN_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf(work->filename[work->programs[i].ptn]);
    }
    for( ;i<N_PROGRAMS;i++){
		DEBUG_Locate(x,y+i*COLUMN_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf("--------");
    }

    x-=(FONT_WIDTH*0x04+FONT_WIDTH*0x04);
    if(work->cursole<2){
		y-=COLUMN_HEIGHT*2;
		DEBUG_Locate(x,y+work->cursole*COLUMN_HEIGHT,MENU_MODE_NORMAL);
    }
    else{
		DEBUG_Locate(x,y+((work->cursole-2)*COLUMN_HEIGHT)/2,MENU_MODE_NORMAL);
    }
    DEBUG_Printf(">>>");
}

static void VAPreview_DebugPlayPrint(void)
{
    Work *work;
    int x,y,yy;
    int i;

    work=GetWork(PreviewVA.va,PreviewVA.va_index);

    x=LOCATE_X;
    y=LOCATE_Y+COLUMN_HEIGHT;

    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<Program Play Mode   :L2>");

    y+=COLUMN_HEIGHT+COLUMN_HEIGHT*2;

    x+=FONT_WIDTH*4;
    y+=COLUMN_HEIGHT/2;
    for(i=0;i<N_PROGRAMS-1 && i<work->playlimit-1;i++){
		DEBUG_Locate(x,y+i*COLUMN_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf("%3d",work->programs[i].count);
    }
    for( ;i<N_PROGRAMS-1;i++){
		DEBUG_Locate(x,y+i*COLUMN_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf("---");
    }

    x+=FONT_WIDTH*4;
    y-=COLUMN_HEIGHT/2;
    for(i=0;i<N_PROGRAMS && i<work->playlimit;i++){
		DEBUG_Locate(x,y+i*COLUMN_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf(work->filename[work->programs[i].ptn]);
    }
    for( ;i<N_PROGRAMS;i++){
		DEBUG_Locate(x,y+i*COLUMN_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf("--------");
    }

    x-=(FONT_WIDTH*4+FONT_WIDTH*4);
    y+=COLUMN_HEIGHT/2;

    yy=work->playpoint-2;
    if(yy<0) yy=0;
    yy=y+yy*COLUMN_HEIGHT;

    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf(">>>");
}

static void VAPreview_DebugMovePrint(void)
{
    Work *work;
    int x,y,yy;

    work=GetWork(PreviewVA.va,PreviewVA.va_index);

    x=LOCATE_X;
    y=LOCATE_Y+COLUMN_HEIGHT;

    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("<Move/Rotate Mode    :L2>");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    if(work->speed_mode){
		DEBUG_Printf("<High Speed Mode     :R2>");
    }
    else{
		DEBUG_Printf("<Low Speed Mode      :R2>");
    }

    x+=FONT_WIDTH*0x04;
    y+=COLUMN_HEIGHT;
    yy=y;

    if(work->target_human_index>=0){
		static const char *objname[]={
			"Koshi           ",
			"Onaka           ",
			"Mune            ",
			"Migi   Kata     ",
			"Migi   Ude1     ",
			"Migi   Ude2     ",
			"Migi   Te       ",
			"Hidari Kata     ",
			"Hidari Ude1     ",
			"Hidari Ude2     ",
			"Hidari Te       ",
			"Kubi            ",
			"Atama           ",
			"Migi   Ashi1    ",
			"Migi   Ashi2    ",
			"Migi   Kakato   ",
			"Migi   Tsumasaki",
			"Hidari Ashi1    ",
			"Hidari Ashi2    ",
			"Hidari Kakato   ",
			"Hidari Tsumasaki",
		};
		int evm_flag=*(PreviewFile.human_format_flags+work->target_human_index);
		char *filename=*(PreviewFile.human_filenames+work->target_human_index);
		int index=work->human_object_index;

		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("Human Name  : %s",filename);
		if(evm_flag){
			DEBUG_Printf(".evm");
		}
		else{
			DEBUG_Printf(".kms");
		}

		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("Part Name   : %s (ID=%d)",objname[index],index);
    }
    else{
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("Human Name  : No Selected");

		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("Part Name   : -----------");

		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("<MODE: Move On ViewCoord>");

		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("POS X : %d",(int)(work->pos.vx));
		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("POS Y : %d",(int)(work->pos.vy));
		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("POS Z : %d",(int)(work->pos.vz));

		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("ROT X : %d",(int)(work->rot.vx));
		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("ROT Y : %d",(int)(work->rot.vy));
		yy+=COLUMN_HEIGHT;
		DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
		DEBUG_Printf("ROT Z : %d",(int)(work->rot.vz));
    }

    x-=FONT_WIDTH*0x04;
    yy=y+work->mov_cursole*COLUMN_HEIGHT;
    DEBUG_Locate(x,yy,MENU_MODE_NORMAL);
    DEBUG_Printf("R1>");
}

void VAPreview_DebugPrint(void)
{
    Work *work;

    VAPreview_DebugTitlePrint();

    work=GetWork(PreviewVA.va,PreviewVA.va_index);

    switch(work->mode){
    case VA_MODE_NORMAL_ANIME:
		VAPreview_DebugNormalPrint();
		break;
    case VA_MODE_PROGRAM_ANIME:
		VAPreview_DebugProgramPrint();
		break;
    case VA_MODE_PLAY_ANIME:
		VAPreview_DebugPlayPrint();
		break;
    case VA_MODE_MOVE_ROTATE:
		VAPreview_DebugMovePrint();
		break;
    }
}

static void VAPreview_NormalCursole(Work *work)
{
    int ptnnum0,ptnnum1,flag=0,flag2=0;

    ptnnum0=work->ptnnum0;
    ptnnum1=work->ptnnum1;

    if(PreviewKey.status & PAD_R1){
		switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
		case PAD_U:
			work->ptnnum0--;
			flag=1;
			if(work->ptnnum0<0){
				work->ptnnum0=work->filename_size-1;
			}
			break;
		case PAD_D:
			work->ptnnum0++;
			flag=1;
			if(work->ptnnum0>=work->filename_size){
				work->ptnnum0=0;
			}
			break;
		}
		flag2=1;
    }
    if(PreviewKey.status & PAD_R2){
		switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
		case PAD_U:
			work->ptnnum1--;
			flag=1;
			if(work->ptnnum1<0){
				work->ptnnum1=work->filename_size-1;
			}
			break;
		case PAD_D:
			work->ptnnum1++;
			flag=1;
			if(work->ptnnum1>=work->filename_size){
				work->ptnnum1=0;
			}
			break;
		}
		flag2=1;
    }
    if(!flag2){
		switch(PreviewKey.auto_status & (PAD_U|PAD_D|PAD_L|PAD_R)){
		case PAD_L:
			work->count--;
			if(work->count<1) work->count=1;
			flag=1;
			break;
		case PAD_R:
			work->count++;
			flag=1;
			break;
		case PAD_U:
			work->count+=10;
			flag=1;
			break;
		case PAD_D:
			work->count-=10;
			if(work->count<1) work->count=1;
			flag=1;
			break;
		}
    }
    if(flag){
		int i;
		for(i=0;i<work->n_models;i++){
			(*(work->va+i))->p[ptnnum0]=0.0f;
			(*(work->va+i))->p[ptnnum1]=0.0f;
			(*(work->va+i))->p[work->ptnnum0]=1.0f;
			if(work->ptnnum0!=work->ptnnum1) (*(work->va+i))->p[work->ptnnum1]=0.0f;
			(*(work->va+i))->count=0;
		}
    }
    if(PreviewKey.press & PAD_L2){
		work->mode=VA_MODE_PROGRAM_ANIME;
    }
}

static void VAPreview_ProgramCursole(Work *work)
{
    switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
    case PAD_U:
		work->cursole--;
		if(work->cursole<0) work->cursole=0;
		break;
    case PAD_D:
		work->cursole++;
		if(work->cursole>=work->playlimit*2-1+2){
			work->playlimit++;
			if(work->playlimit>N_PROGRAMS){
				work->playlimit=N_PROGRAMS;
				work->cursole=work->playlimit*2-2+2;
			}
			else{
				work->programs[work->playlimit-1].ptn=0;
				work->programs[work->playlimit-2].count=1;
			}
		}
		break;
    }
    switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
    case PAD_R:
	{
	    int index=(work->cursole-2)>>1;
	    if(index>=0){
			if(work->cursole & 1){
				work->programs[index].count++;
				if(work->programs[index].count>999){
					work->programs[index].count=999;
				}
			}
			else if((PreviewKey.press & (PAD_L|PAD_R))==PAD_R){
				work->programs[index].ptn++;
				if(work->programs[index].ptn>=work->filename_size){
					work->programs[index].ptn=0;
				}
			}
	    }
	}
	break;
    case PAD_L:
	{
	    int index=(work->cursole-2)>>1;
	    if(index>=0){
			if(work->cursole & 1){
				work->programs[index].count--;
				if(work->programs[index].count<1){
					work->programs[index].count=1;
				}
			}
			else if((PreviewKey.press & (PAD_L|PAD_R))==PAD_L){
				work->programs[index].ptn--;
				if(work->programs[index].ptn<0){
					work->programs[index].ptn=work->filename_size-1;
				}
			}
	    }
	}
	break;
    }

    switch(PreviewKey.press & (PAD_A|PAD_X|PAD_Y|PAD_L2)){
    case PAD_A:
		switch(work->cursole){
		case 0:
			VAPreview_FileLoad(work);
			break;
		case 1:
			VAPreview_FileSave(work);
			break;
		}
		break;
    case PAD_X:
		if(work->playlimit>1){
			work->playpoint=0;
			work->mode=VA_MODE_PLAY_ANIME;
		}
		break;
    case PAD_Y:
		work->playlimit--;
		if(work->playlimit<1) work->playlimit=1;

		if(work->cursole>=work->playlimit*2-1+2){
			work->cursole=work->playlimit*2-2+2;
		}
		break;
    case PAD_L2:
		work->mode=VA_MODE_MOVE_ROTATE;
		break;
    }
}

static void VAPreview_ProgramPlayCursole(Work *work)
{
    switch(PreviewKey.press & (PAD_X|PAD_L2)){
    case PAD_X:
		work->mode=VA_MODE_PROGRAM_ANIME;
		break;
    case PAD_L2:
		work->mode=VA_MODE_MOVE_ROTATE;
		break;
    }
}

static void VAPreview_MoveCursole(Work *work)
{
    float d;

    if(PreviewKey.status & PAD_R1){
		switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
		case PAD_U:
			work->mov_cursole--;
			if(work->mov_cursole==1 &&
			   work->target_human_index<0) work->mov_cursole--;

			if(work->target_human_index<0){
				if(work->mov_cursole<0) work->mov_cursole=8;
			}
			else{
				if(work->mov_cursole<0) work->mov_cursole=1;
			}
			break;
		case PAD_D:
			work->mov_cursole++;
			if(work->mov_cursole==1 &&
			   work->target_human_index<0) work->mov_cursole++;

			if(work->target_human_index<0){
				if(work->mov_cursole>8) work->mov_cursole=0;
			}
			else{
				if(work->mov_cursole>1) work->mov_cursole=0;
			}
			break;
		}
    }

    if(PreviewKey.status & PAD_R2){
		work->speed_mode=1;
		d=20.0f;
    }
    else{
		work->speed_mode=0;
		d=1.0f;
    }

    switch(work->mov_cursole){
    case 0:
		if(work->target_human_index>=0){
			HumanControl *human=PreviewHuman.humans+work->target_human_index;
			human->body.objs->objs[work->human_object_index].flag&=~DG_FLAG_INVISIBLE;
		}

		switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
		case PAD_L:
			work->target_human_index--;
			if(work->target_human_index<-1){
				work->target_human_index=PreviewHuman.humans_size-1;
			}
			break;
		case PAD_R:
			work->target_human_index++;
			if(work->target_human_index>=PreviewHuman.humans_size){
				work->target_human_index=-1;
			}
			break;
		}
		break;
    case 1:
		if(work->target_human_index>=0){
			HumanControl *human=PreviewHuman.humans+work->target_human_index;
			int size=21;

			human->body.objs->objs[work->human_object_index].flag&=~DG_FLAG_INVISIBLE;

			switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
			case PAD_L:
				work->human_object_index--;
				if(work->human_object_index<0) work->human_object_index=size-1;
				break;
			case PAD_R:
				work->human_object_index++;
				if(work->human_object_index>=size) work->human_object_index=0;
				break;
			}
		}
		break;

    case 2:
		if(!(PreviewKey.status & PAD_R1)){
			PreviewVMoveXYZ(&(work->pos),d);
		}
		break;
    case 3:
		PreviewMoveX(&(work->pos),d);
		break;
    case 4:
		PreviewMoveY(&(work->pos),d);
		break;
    case 5:
		PreviewMoveZ(&(work->pos),d);
		break;
    case 6:
		switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
		case PAD_L:
			work->rot.vx--;
			break;
		case PAD_R:
			work->rot.vx++;
			break;
		}
		work->rot.vx&=0x0fff;
		break;
    case 7:
		switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
		case PAD_L:
			work->rot.vy--;
			break;
		case PAD_R:
			work->rot.vy++;
			break;
		}
		work->rot.vy&=0x0fff;
		break;
    case 8:
		switch(PreviewKey.auto_status & (PAD_L|PAD_R)){
		case PAD_L:
			work->rot.vz--;
			break;
		case PAD_R:
			work->rot.vz++;
			break;
		}
		work->rot.vz&=0x0fff;
		break;
    }

    if(PreviewKey.press & PAD_L2){
		work->mov_cursole=0;
		work->mode=VA_MODE_NORMAL_ANIME;
    }
}

void VAPreview_DebugCursole(void)
{
    Work *work;

    work=GetWork(PreviewVA.va,PreviewVA.va_index);

    switch(work->mode){
    case VA_MODE_NORMAL_ANIME:
		VAPreview_NormalCursole(work);
		break;
    case VA_MODE_PROGRAM_ANIME:
		VAPreview_ProgramCursole(work);
		break;
    case VA_MODE_PLAY_ANIME:
		VAPreview_ProgramPlayCursole(work);
		break;
    case VA_MODE_MOVE_ROTATE:
		VAPreview_MoveCursole(work);
		break;
    }
    if(PreviewKey.press & PAD_L1){
		work->disp_enable^=1;
    }
}

static void Act(Work *work)
{
    int i;
    int start;

	GM_CurrentMap=PreviewLight.where;

    if(work->disp_enable){
		DG_VisibleObjs(work->body.objs);
		if(work->target_human_index>=0){
			HumanControl *human=PreviewHuman.humans+work->target_human_index;

			human->body.objs->objs[work->human_object_index].flag|=DG_FLAG_INVISIBLE;

			work->body.objs->objs[0].flag|=DG_FLAG_INVISIBLE;
		}
		else{
			work->body.objs->objs[0].flag&=~DG_FLAG_INVISIBLE;
		}
    }
    else{
		DG_InvisibleObjs(work->body.objs);
		if(work->target_human_index>=0){
			HumanControl *human=PreviewHuman.humans+work->target_human_index;

			human->body.objs->objs[work->human_object_index].flag&=~DG_FLAG_INVISIBLE;
		}
    }

    switch(work->mode){
    case VA_MODE_NORMAL_ANIME:
		for(i=0;i<work->n_models;i++){
			SimpleVertexAnimation(*(work->va+i));

			if((*(work->va+i))->count==0){
				float p;
				p=(*(work->va+i))->p[work->ptnnum1];
				(*(work->va+i))->p[work->ptnnum1]=(*(work->va+i))->p[work->ptnnum0];
				(*(work->va+i))->p[work->ptnnum0]=p;
				(*(work->va+i))->count=work->count;
			}
		}
		break;
    case VA_MODE_PROGRAM_ANIME:
		for(i=0;i<work->n_models;i++){
			int j;

			SimpleVertexAnimation(*(work->va+i));

			(*(work->va+i))->count=0;
			for(j=0;j<work->filename_size;j++){
				(*(work->va+i))->p[j]=0.0f;
			}
			if(work->cursole<2){
				(*(work->va+i))->p[work->programs[0].ptn]=1.0f;
			}
			else{
				(*(work->va+i))->p[work->programs[(work->cursole-2)>>1].ptn]=1.0f;
			}
		}
		break;
    case VA_MODE_PLAY_ANIME:
		if(work->playpoint==0){
			for(i=0;i<work->n_models;i++){
				int j;

				SimpleVertexAnimation(*(work->va+i));

				for(j=0;j<work->filename_size;j++){
					(*(work->va+i))->p[j]=0.0f;
				}
				(*(work->va+i))->p[work->programs[0].ptn]=1.0f;
				(*(work->va+i))->count=0;
			}
			work->playpoint++;
		}
		else{
			int flag=0;
			int flag2=0;

			for(i=0;i<work->n_models;i++){
				int j;

				SimpleVertexAnimation(*(work->va+i));

				if((*(work->va+i))->count==0){
					if(work->playpoint>=work->playlimit){
						flag=1;
					}
					else{
						for(j=0;j<work->filename_size;j++){
							(*(work->va+i))->p[j]=0.0f;
						}
						(*(work->va+i))->p[work->programs[work->playpoint].ptn]=1.0f;
						(*(work->va+i))->count=work->programs[work->playpoint-1].count;

						flag2=1;
					}
				}
			}
			if(flag) work->playpoint=0;
			else if(flag2) work->playpoint++;
		}
		break;
    }

    if(work->target_human_index<0){
		DG_SetPos2(&(work->pos),&(work->rot));
		DG_GetPos(&(work->body.objs->world));
		start=0;
    }
    else{
		HumanControl *human=PreviewHuman.humans+work->target_human_index;
		int parent=human->body.objs->def->models[work->human_object_index].parent;

		human->body.objs->objs[work->human_object_index].flag|=DG_FLAG_INVISIBLE;

		fpu_CopyMatrix(&(work->body.objs->objs[1].world),
					   &(human->body.objs->objs[work->human_object_index].world));
		if(parent==-1){
			fpu_CopyMatrix(&(work->body.objs->objs[0].world),
						   &(human->body.objs->world));
		}
		else{
			fpu_CopyMatrix(&(work->body.objs->objs[0].world),
						   &(human->body.objs->objs[parent].world));
		}
		fpu_CopyMatrix(&(work->body.objs->world),
					   &(work->body.objs->objs[0].world));
		start=2;
    }

    for(i=start;i<work->body.objs->def->n_models;i++){
		FVECTOR v;
		int parent=work->body.objs->def->models[i].parent;

		v.vx=work->body.objs->def->models[i].tx;
		v.vy=work->body.objs->def->models[i].ty;
		v.vz=work->body.objs->def->models[i].tz;
		v.vw=1.0f;
		vu0_Ldv0(&v);

		if(parent==-1) vu0_Ldm0(&(work->body.objs->world));
		else vu0_Ldm0(&(work->body.objs->objs[parent].world));

		vu0_Mulv0m0v0();

		vu0_Stm0(&(work->body.objs->objs[i].world));
		vu0_Stv0((FVECTOR *)&(work->body.objs->objs[i].world.m[3][0]));
    }

    DG_GetLightMatrix((FVECTOR *)&(work->body.objs->world.m[3][0]),
					  work->lights);
}

static void Die(Work *work)
{
    if(work->va!=NULL){
		int i;
		for(i=0;i<work->n_models;i++){
			ExitVertexAnimation(*(work->va+i));
		}
    }
    if(work->cv2_defs!=NULL){
		GV_Free(work->cv2_defs);
    }

    GM_FreeObject(&(work->body));
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


/* 資源を獲得 */
static int GetResources_called(Work *work,FVECTOR *x,SVECTOR *r)
{
    OBJECT *body;
    int n_models;
    int i,j;
    char **filename=PreviewFile.vanime_filenames;
    int fnsize=PreviewFile.vanime_filenames_size;

    PutWork(work);

    work->pos=*x;
    work->rot=*r;

    body=&(work->body);

    GM_InitObject(body,GV_StrCode(*(filename+0)),OBJECT_FLAG);
    n_models=body->objs->n_models;

    if((work->cv2_defs=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*fnsize
											 +sizeof(VERTEX_ANIME_WORK *)*n_models))==NULL){
		return 0;
    }
    GV_ZeroMemory(work->cv2_defs,
				  sizeof(CV2_DEF *)*fnsize+sizeof(VERTEX_ANIME_WORK *)*n_models);
    work->va=(VERTEX_ANIME_WORK **)(work->cv2_defs+fnsize);

    for(i=0;i<fnsize;i++){
		if((*(work->cv2_defs+i)
			=(CV2_DEF*)GV_GetCache(GV_CacheID(GV_StrCode(*(filename+i)),'c')))==NULL){

			return 0;
		}
    }

    for(j=0;j<n_models;j++){
		if((*(work->va+j)
			=InitVertexAnimation(body->objs->objs+j,&((*(work->cv2_defs+0))->models[j]),
								 DG_VANIME_VERTS|DG_VANIME_NORMS,fnsize))==NULL){
			return 0;
		}
    }
    GM_ConfigObjectLight(body,work->lights) ;

    for(j=0;j<n_models;j++){
		for(i=0;i<fnsize;i++){
			(*(work->va+j))->key[i]=&((*(work->cv2_defs+i))->models[j]);
			(*(work->va+j))->p[i]=0.0f;
		}
		(*(work->va+j))->p[0]=1.0f;
		(*(work->va+j))->count=0;
    }

    work->target_human_index=-1;
    work->human_object_index=0;

    work->ptnnum0=0;
    work->ptnnum1=0;
    work->disp_enable=0;
    work->filename=filename;
    work->filename_size=fnsize;
    work->n_models=n_models;
    work->count=COUNT;

    work->mode=0;
    work->cursole=0;
    work->playpoint=0;
    work->playlimit=1;

    work->mov_cursole=0;

    for(i=0;i<N_PROGRAMS;i++){
		work->programs[i].ptn=0;
		work->programs[i].count=1;
    }

    return 1;
}

/* 初期化部メイン */
void *NewVAPreview_called(FVECTOR *x,SVECTOR *r)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
	
		if(!GetResources_called(work,x,r)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}


/* 資源を獲得 */
static int GetResources(Work *work)
{
    OBJECT *body;
    int n_models;
    int i,j;
    char **filename=PreviewFile.vanime_filenames;
    int fnsize=PreviewFile.vanime_filenames_size;
    int bfilename;
    CV2_DEF *cv2;

    if(fnsize<=0) return 0;

    PutWork(work);

    work->pos=DG_ZeroVector;
    work->rot=DG_ZeroSVector;
    work->disp_enable=0;

    if(GCL_GetOption( 'x' )!=NULL){
		int buf[ 3 ];

		GCL_GetIV(GCL_NextStr(),buf);
		vu0_IV0toFV((IVECTOR *)buf,&(work->pos));
    } 
    if( GCL_GetOption( 'r' ) != NULL ){
		int buf[ 3 ];

		GCL_GetIV(GCL_NextStr(),buf);
		work->rot.vx=buf[0];
		work->rot.vy=buf[1];
		work->rot.vz=buf[2];
    }
    if( GCL_GetOption( 'd' ) != NULL ){
		work->disp_enable=GCL_GetNextInt();
    }

    bfilename=GV_StrCode("vanim0");
    if(GV_GetCache(GV_CacheID(bfilename,'k'))==NULL ||
       GV_GetCache(GV_CacheID(bfilename,'c'))==NULL){

		bfilename=GV_StrCode(*(filename+0));
    }
    if( GCL_GetOption( 'b' ) != NULL ){
		int name;
		name=GCL_GetNextInt();
		if(GV_GetCache(GV_CacheID(name,'k'))!=NULL &&
		   GV_GetCache(GV_CacheID(name,'c'))!=NULL){

			bfilename=name;
		}
    }

    body=&(work->body);

    GM_InitObject(body,bfilename,OBJECT_FLAG);
    n_models=body->objs->n_models;

    if((work->cv2_defs=(CV2_DEF **)GV_Malloc(sizeof(CV2_DEF *)*fnsize
											 +sizeof(VERTEX_ANIME_WORK *)*n_models))==NULL){
		return 0;
    }
    GV_ZeroMemory(work->cv2_defs,
				  sizeof(CV2_DEF *)*fnsize+sizeof(VERTEX_ANIME_WORK *)*n_models);
    work->va=(VERTEX_ANIME_WORK **)(work->cv2_defs+fnsize);

    for(i=0;i<fnsize;i++){
		int strcode=GV_StrCode(*(filename+i));
		if((*(work->cv2_defs+i)
			=(CV2_DEF*)GV_GetCache(GV_CacheID(strcode,'c')))==NULL){

			return 0;
		}
    }

    cv2=(CV2_DEF*)GV_GetCache(GV_CacheID(bfilename,'c'));
    for(j=0;j<n_models;j++){
		if((*(work->va+j)
			=InitVertexAnimation(body->objs->objs+j,&(cv2->models[j]),
								 DG_VANIME_VERTS|DG_VANIME_NORMS,fnsize))==NULL){
			return 0;
		}
    }

    GM_ConfigObjectLight(body,work->lights);

    for(j=0;j<n_models;j++){
		int chk_vsizem,chk_nsizem;
		int chk_vidx_sizem,chk_nidx_sizem;
		int k;

		chk_vsizem=cv2->models[j].n_verts;
		chk_nsizem=cv2->models[j].n_norms;
		chk_vidx_sizem=cv2->models[j].n_verts_index;
		chk_nidx_sizem=cv2->models[j].n_norms_index;

		printf("Object No. %d -------------------------------\n",j);
		printf("master Vertex size = %d\n",chk_vsizem);
		printf("master Normal size = %d\n",chk_nsizem);
		printf("master Vertex index size = %d\n",chk_vidx_sizem);
		printf("master Normal index size = %d\n",chk_nidx_sizem);

		for(i=0,k=1;i<fnsize;i++){
			int chk_vsize,chk_nsize;
			int chk_vidx_size,chk_nidx_size;
			int strcode=GV_StrCode(*(filename+i));

			chk_vsize=(*(work->cv2_defs+i))->models[j].n_verts;
			chk_nsize=(*(work->cv2_defs+i))->models[j].n_norms;
			chk_vidx_size=(*(work->cv2_defs+i))->models[j].n_verts_index;
			chk_nidx_size=(*(work->cv2_defs+i))->models[j].n_norms_index;

			if(strcode==bfilename){
				(*(work->va+j))->key[0]=&((*(work->cv2_defs+i))->models[j]);
				(*(work->va+j))->p[0]=0.0f;
			}
			else{
				(*(work->va+j))->key[k]=&((*(work->cv2_defs+i))->models[j]);
				(*(work->va+j))->p[k]=0.0f;
				k++;
			}

			if(chk_vsize!=chk_vsizem){
				printf("[ %s ] diffrent model ?  ( Vertex size = %d )\n",*(filename+i),chk_vsize);
			}
			if(chk_vsize>2048){
				printf("[ %s ] Over 2048 Vertexes !! ( Vertex size = %d )\n",*(filename+i),chk_vsize);
			}
			if(chk_nsize!=chk_nsizem){
				printf("[ %s ] diffrent model ?  ( Normal size = %d )\n",*(filename+i),chk_nsize);
			}
			if(chk_nsize>2048){
				printf("[ %s ] Over 2048 Normals !! ( Normal size = %d )\n",*(filename+i),chk_nsize);
			}
			if(chk_vidx_size!=chk_vidx_sizem){
				printf("[ %s ] diffrent model ?  ( Vertex index size = %d )\n",
					   *(filename+i),chk_vidx_size);
			}
			if(chk_nidx_size!=chk_nidx_sizem){
				printf("[ %s ] diffrent model ?  ( Normal index size = %d )\n",
					   *(filename+i),chk_nidx_size);
			}
		}
		(*(work->va+j))->p[0]=1.0f;
		(*(work->va+j))->count=0;
    }

    printf("--------------------------------------------\n");

    work->target_human_index=-1;
    work->human_object_index=0;

    work->ptnnum0=0;
    work->ptnnum1=0;
    work->filename=filename;
    work->filename_size=fnsize;
    work->n_models=n_models;
    work->count=COUNT;

    work->mode=0;
    work->cursole=0;
    work->playpoint=0;
    work->playlimit=1;

    work->mov_cursole=0;

    for(i=0;i<N_PROGRAMS;i++){
		work->programs[i].ptn=0;
		work->programs[i].count=1;
    }

    return 1;
}

static void NullAct(Work *work)
{
    printf("Abort Vertex Animation\n");
    GV_DestroyActor(work);
}

static void NullDie(Work *work)
{
}

/* 初期化部メイン */
void *NewVAPreview(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work)) ;
    if(work!=NULL) {
		if(!GetResources(work)) GV_SetActor(&(work->actor),NullAct,NullDie);
		else GV_SetActor(&(work->actor),Act,Die);
		GV_ActorEX(&(work->actor));
    }
    return (void *)work ;
}
