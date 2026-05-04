//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	memcard_checker.c
		メモリーカードチェッカー
	
	2001/01/12 K.Kano
	$Id: memcard_checker.c,v 1.1.1.3 2002/11/19 11:43:18 Yoshizawa1 Exp $
*/


#include "memcard_checker.h"


/* ------------------------------------------------------------------------ */


#define KEY_REPEAT1		20
#define KEY_REPEAT2		(KEY_REPEAT1+5)

#if 1 //BP
//#ifdef KP_XBOX
#define SCE_STM_R	0x0001
#define SCE_STM_W	0x0002
#define SCE_STM_X	0x0004
#define SCE_STM_C	0x0008
#define SCE_STM_F	0x0010
#define SCE_STM_D	0x0020
#define sceMcFileAttrReadable		SCE_STM_R
#define sceMcFileAttrWriteable		SCE_STM_W
#define sceMcFileAttrExecutable		SCE_STM_X
#define sceMcFileAttrDupProhibit	SCE_STM_C
#define sceMcFileAttrSubdir		SCE_STM_D
#define sceMcFileAttrClosed		0x0080
#define sceMcFileAttrPDAExec		0x0800
#define sceMcFileAttrPS1		0x1000
#endif


static const char * const port_name[]={
	"PORT 1",
	"PORT 2",
};

static const char * const slot_name[]={
	"SLOT A",
	"SLOT B",
	"SLOT C",
	"SLOT D",
};


// Key Input

static void KeyAct(Work *work)
{
	work->pad_press=GV_PadData[0].press;

    if(GV_PadData[0].status && work->pad_on0==GV_PadData[0].status){
		if(work->pad_repeat_count>KEY_REPEAT1){
			if(work->pad_repeat_count>KEY_REPEAT2){
				work->pad_repeat_count=KEY_REPEAT2;
				work->pad_on=work->pad_on0;
			}
			else{
				work->pad_on=0;
			}
		}
		else{
			work->pad_on=0;
		}
		work->pad_repeat_count++;
	}
	else{
		work->pad_on=work->pad_on0=GV_PadData[0].status;
		work->pad_repeat_count=0;
	}
}


// Display Menu / Status

static int MoveCursor_ModeSelect(Work *work)
{
	int pad=work->pad_press;

	if(pad & PAD_SEL){
		/* Next */
		work->mode++;
		work->mode2=0;

		if(work->mode==MC_MENU_MODE_FILESEL){
			if(MCAccessGetType(&(work->mcwork),work->port,work->slot[work->port])!=2 ||
			   !MCAccessIsFormatted(&(work->mcwork),work->port,work->slot[work->port]) ||
			   work->dirtable_used_size==0){

				work->mode++;
				work->mode2=0;
			}
		}
		if(work->mode>=MC_MENU_MODE_MAX){
			work->mode=0;
			work->mode2=0;
		}
		return 1;
	}

	return 0;
}


#define N_DISP_DIRENTRYS		4

static void DispDir(sceMcTblGetDir *entry,int x,int y)
{
	char attr[0x30];
	int i;
#ifdef KP_XBOX
	SYSTEMTIME systime;
#endif	

	for(i=0;i<0x30;i++) attr[i]='-';

	i=0;
	if(entry->AttrFile & sceMcFileAttrReadable){
		attr[i]='R';
	}
	i++;
	if(entry->AttrFile & sceMcFileAttrWriteable){
		attr[i]='W';
	}
	i++;
	if(entry->AttrFile & sceMcFileAttrExecutable){
		attr[i]='X';
	}
	i++;
	if(entry->AttrFile & sceMcFileAttrDupProhibit){
		attr[i+0]='D';
		attr[i+1]='u';
	}
	i+=2;
	if(entry->AttrFile & sceMcFileAttrSubdir){
		attr[i+0]='S';
		attr[i+1]='d';
	}
	i+=2;
	if(entry->AttrFile & sceMcFileAttrClosed){
		attr[i+0]='F';
		attr[i+1]='c';
	}
	i+=2;
	if(entry->AttrFile & sceMcFileAttrPDAExec){
		attr[i+0]='P';
		attr[i+1]='d';
		attr[i+2]='a';
	}
	i+=3;
	if(entry->AttrFile & sceMcFileAttrPS1){
		attr[i+0]='P';
		attr[i+1]='s';
		attr[i+2]='1';
	}
	i+=3;
	attr[i++]='\0';

	if(entry->Reserve1){
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("*");
	}

    DEBUG_Locate(x+FONT_WIDTH,y,MENU_MODE_NORMAL);
    DEBUG_Printf(entry->EntryName);

    DEBUG_Locate(x+FONT_WIDTH+0x20*FONT_WIDTH,y,MENU_MODE_NORMAL);
    DEBUG_Printf("%s Size %d",attr,entry->FileSizeByte);

    DEBUG_Locate(x+FONT_WIDTH,y+FONT_HEIGHT,MENU_MODE_NORMAL);
#ifdef PSX2	
//	DEBUG_Printf("Create %02dy%02dm%02dd %02dh%02dm%02ds",
//				 entry->CYear,entry->CMonth,entry->CDay,
//				 entry->CHour,entry->CMin,entry->CSec);
#endif	
#ifdef	KP_XBOX
	FileTimeToSystemTime( &entry->_Create, &systime );
	DEBUG_Printf("Create %02dy%02dm%02dd %02dh%02dm%02ds",
				 systime.wYear, systime.wMonth, systime.wDay,
				 systime.wHour, systime.wMinute, systime.wSecond );
#endif
	

    DEBUG_Locate(x+FONT_WIDTH+29*FONT_WIDTH,y+FONT_HEIGHT,MENU_MODE_NORMAL);
#ifdef PSX2	
//	DEBUG_Printf("Modify %02dy%02dm%02dd %02dh%02dm%02ds",
//				 entry->MYear,entry->MMonth,entry->MDay,
//				 entry->MHour,entry->MMin,entry->MSec);
#endif	
#ifdef	KP_XBOX
	FileTimeToSystemTime( &entry->_Modify, &systime );
	DEBUG_Printf("Modify %02dy%02dm%02dd %02dh%02dm%02ds",
				 systime.wYear, systime.wMonth, systime.wDay,
				 systime.wHour, systime.wMinute, systime.wSecond );
#endif
}

static void DispDirs(Work *work,int x,int y)
{
	int i,j;

	j=work->cur_start_file;
	for(i=0;i<N_DISP_DIRENTRYS && j<work->dirtable_used_size;i++,j++){
		DispDir(&(work->dirtable[j]),x+2*FONT_WIDTH,y+FONT_HEIGHT+i*FONT_HEIGHT*2);
	}

	/* 上下に動ける事示すマーク */
	if(work->cur_start_file){
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("/\\\n");
	}
	if(work->cur_start_file+N_DISP_DIRENTRYS<work->dirtable_used_size){
		DEBUG_Locate(x,y+FONT_HEIGHT+N_DISP_DIRENTRYS*FONT_HEIGHT*2,MENU_MODE_NORMAL);
		DEBUG_Printf("\\/\n");
	}

	/* Cursor */
	if(work->dirtable_used_size>0){
		i=work->selfile-work->cur_start_file;
		DEBUG_Locate(x,y+FONT_HEIGHT+i*FONT_HEIGHT*2,MENU_MODE_NORMAL);
		if(work->mode==MC_MENU_MODE_FILESEL){
			DEBUG_Printf(">>\n");
			DEBUG_Printf(">>\n");
		}
		else{
			DEBUG_Printf(" >\n");
			DEBUG_Printf(" >\n");
		}
	}
}

static int MoveCursor_Dirs(Work *work)
{
	int pad=work->pad_press;
	int pad_on=work->pad_on;

	if(MoveCursor_ModeSelect(work)) return 0;

	switch(pad & (PAD_A|PAD_B|PAD_X|PAD_Y|PAD_R1|PAD_R2)){
	case PAD_A:
		/* Chdir */
		return 1;
	case PAD_B:
		/* Chdir Root */
		return 2;
	case PAD_X:
		/* Mark */
		work->dirtable[work->selfile].Reserve1^=1;
		break;
	case PAD_Y:
		break;
	case PAD_R1:
		/* Mark All */
		{
			int i;
			for(i=0;i<work->dirtable_used_size;i++){
				work->dirtable[i].Reserve1=1;
			}
		}
		break;
	case PAD_R2:
		/* Unmark All */
		{
			int i;
			for(i=0;i<work->dirtable_used_size;i++){
				work->dirtable[i].Reserve1=0;
			}
		}
		break;
	}

	/* Cursor Move */
	switch(pad_on & (PAD_U|PAD_D)){
	case PAD_U:
		work->selfile--;
		if(work->selfile<0) work->selfile=0;
		if(work->selfile<work->cur_start_file) work->cur_start_file=work->selfile;
		break;
	case PAD_D:
		work->selfile++;
		if(work->selfile>=work->dirtable_used_size) work->selfile=work->dirtable_used_size-1;
		if(work->selfile>=work->cur_start_file+N_DISP_DIRENTRYS){
			work->cur_start_file=work->selfile-N_DISP_DIRENTRYS+1;
		}
		break;
	}

	return 0;
}

static void DispDir2(sceMcTblGetDir *entry,int x,int y)
{
	char attr[0x30];
	int i;

	for(i=0;i<0x30;i++) attr[i]='-';

	i=0;
	if(entry->AttrFile & sceMcFileAttrReadable){
		attr[i]='R';
	}
	i++;
	if(entry->AttrFile & sceMcFileAttrWriteable){
		attr[i]='W';
	}
	i++;
	if(entry->AttrFile & sceMcFileAttrExecutable){
		attr[i]='X';
	}
	i++;
	if(entry->AttrFile & sceMcFileAttrDupProhibit){
		attr[i+0]='D';
		attr[i+1]='u';
	}
	i+=2;
	if(entry->AttrFile & sceMcFileAttrSubdir){
		attr[i+0]='S';
		attr[i+1]='d';
	}
	i+=2;
	if(entry->AttrFile & sceMcFileAttrClosed){
		attr[i+0]='F';
		attr[i+1]='c';
	}
	i+=2;
	if(entry->AttrFile & sceMcFileAttrPDAExec){
		attr[i+0]='P';
		attr[i+1]='d';
		attr[i+2]='a';
	}
	i+=3;
	if(entry->AttrFile & sceMcFileAttrPS1){
		attr[i+0]='P';
		attr[i+1]='s';
		attr[i+2]='1';
	}
	i+=3;
	attr[i++]='\0';

    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf(entry->EntryName);

    DEBUG_Locate(x+0x20*FONT_WIDTH,y,MENU_MODE_NORMAL);
    DEBUG_Printf("%s",attr);
}

static void DispDirs2(Work *work,int x,int y)
{
	int i,j;

	j=work->cur_start_dir;
	for(i=0;i<N_DISP_DIRENTRYS && j<work->dirtable2_used_size;i++,j++){
		DispDir2(&(work->dirtable2[j]),x+2*FONT_WIDTH,y+FONT_HEIGHT+i*FONT_HEIGHT);
	}

	/* Port , Slot */
	DEBUG_Locate(x+4*FONT_WIDTH,y,MENU_MODE_NORMAL);
	DEBUG_Printf("%s : %s",port_name[work->port2],slot_name[work->slot2[work->port2]]);

	/* 上下に動ける事示すマーク */
	if(work->cur_start_dir){
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("/\\\n");
	}
	if(work->cur_start_dir+N_DISP_DIRENTRYS<work->dirtable2_used_size){
		DEBUG_Locate(x,y+FONT_HEIGHT+N_DISP_DIRENTRYS*FONT_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf("\\/\n");
	}

	/* Cursor */
	if(work->dirtable2_used_size>0){
		i=work->seldir-work->cur_start_dir;
		DEBUG_Locate(x,y+FONT_HEIGHT+i*FONT_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf(">>\n");
	}
}

static int MoveCursor_Dirs2(Work *work)
{
	int pad=work->pad_press;

	if(MoveCursor_ModeSelect(work)){
		ProcStepset(work,PROC_CHDIR3);
		return 0;
	}

	switch(pad & (PAD_A|PAD_B|PAD_X|PAD_Y|PAD_L1|PAD_L2)){
	case PAD_A:
		/* Chdir */
		return 1;
	case PAD_B:
		/* Chdir Root */
		return 2;
	case PAD_X:
		/* Execute */
		return 3;
	case PAD_Y:
		break;
	case PAD_L1:
		/* port select */
		work->port2++;
		if(work->port2>=PORT_MAX) work->port2=0;
		return 2;
	case PAD_L2:
		/* slot select */
		{
			int slot_max=MCAccessGetSlotMax(&(work->mcwork),work->port2);

			if(slot_max<=1) break;

			work->slot2[work->port2]++;
			if(work->slot2[work->port2]>=slot_max){
				work->slot2[work->port2]=0;
			}
		}
		return 2;
	}

	/* Cursor Move */
	switch(pad & (PAD_U|PAD_D)){
	case PAD_U:
		work->seldir--;
		if(work->seldir<0) work->seldir=0;
		if(work->seldir<work->cur_start_dir) work->cur_start_dir=work->seldir;
		break;
	case PAD_D:
		work->seldir++;
		if(work->seldir>=work->dirtable2_used_size) work->seldir=work->dirtable2_used_size-1;
		if(work->seldir>=work->cur_start_dir+N_DISP_DIRENTRYS){
			work->cur_start_dir=work->seldir-N_DISP_DIRENTRYS+1;
		}
		break;
	}

	return 0;
}

static void DispMenu(Work *work,int x,int y)
{
	int i,j;

	DEBUG_Locate(x+2*FONT_WIDTH,y,MENU_MODE_NORMAL);
	DEBUG_Printf("%s :\n",port_name[0]);
	DEBUG_Printf("%s :\n",port_name[1]);

	DEBUG_Locate(x+11*FONT_WIDTH,y,MENU_MODE_NORMAL);
	for(i=0;i<PORT_MAX;i++){
		switch(MCAccessGetSlotMax(&(work->mcwork),i)){
		case 0:
		case 1:
			DEBUG_Printf("   SLOT A\n");
			work->tmpslot[i]=0;
			break;
		default:
			DEBUG_Printf("<- %s ->\n",slot_name[work->tmpslot[i]]);
			break;
		}
	}

	/* Cursor */
	if(work->mode==MC_MENU_MODE_PORTSEL){
		DEBUG_Locate(x,y+work->tmpport*FONT_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf(">>");
	}
	else{
		DEBUG_Locate(x,y+work->port*FONT_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf(" *");
	}

	{
		static const char *command_names[][4]={
			{
				"COPY",
				"", /* MOVE */
				"DELETE",
				"CHMOD",
			},
			{
				"FORMAT",
				"UNFORMAT",
				"MKDIR",
				"RENAME",
			},
			{
				"UPLOAD",
				"DOWNLOAD",
				"",
				"MKDUMMY",
			},
		};

		for(i=0;i<3;i++){
			for(j=0;j<4;j++){
				DEBUG_Locate(x+(25+2+(8+2)*i)*FONT_WIDTH,y+j*FONT_HEIGHT,MENU_MODE_NORMAL);
				DEBUG_Printf((char *)(command_names[i][j]));
			}
		}
	}


	/* Cursor */
	if(work->mode==MC_MENU_MODE_COMSEL){
		int i,j;

		i=(work->selcom>>4) & 0x0f;
		j=work->selcom & 0x0f;

		DEBUG_Locate(x+(25+i*(2+8))*FONT_WIDTH,y+j*FONT_HEIGHT,MENU_MODE_NORMAL);
		DEBUG_Printf(">>");
	}
}

static int MoveCursor_PortSelect(Work *work)
{
	int pad=work->pad_press;

	if(MoveCursor_ModeSelect(work)){
		if(work->port==work->tmpport &&
		   work->slot[work->port]==work->tmpslot[work->tmpport]){

			return 0;
		}

		work->port=work->tmpport;
		work->slot[work->port]=work->tmpslot[work->tmpport];

		/* Chdir Root + Getdir */
		return 1;
	}

	switch(pad & (PAD_U|PAD_D|PAD_L|PAD_R)){
	case PAD_U:
		work->tmpport--;
		if(work->tmpport<0) work->tmpport=0;
		break;
	case PAD_D:
		work->tmpport++;
		if(work->tmpport>=PORT_MAX) work->tmpport=PORT_MAX-1;
		break;
	case PAD_L:
		work->tmpslot[work->tmpport]--;
		if(work->tmpslot[work->tmpport]<0) work->tmpslot[work->tmpport]=0;
		break;
	case PAD_R:
		{
			int max=MCAccessGetSlotMax(&(work->mcwork),work->tmpport);

			work->tmpslot[work->tmpport]++;
			if(work->tmpslot[work->tmpport]<max){
				work->tmpslot[work->tmpport]=max-1;
			}
		}
		break;
	}

	return 0;
}

static int MoveCursor_CommandSelect(Work *work)
{
	int pad=work->pad_press;

	if(MoveCursor_ModeSelect(work)) return 0;

	switch(pad & (PAD_A|PAD_B)){
	case PAD_A:
		switch(work->selcom){
		case 0x00: /* copy */
			return 1;
		case 0x01: /* move */
			return 2;
		case 0x02: /* delete */
			return 3;
		case 0x03: /* chmod */
			return 4;

		case 0x10: /* format */
			return 5;
		case 0x11: /* unformat */
			return 6;
		case 0x12: /* mkdir */
			return 7;
		case 0x13: /* rename */
			return 8;

		case 0x20: /* upload */
			return 9;
		case 0x21: /* download */
			return 10;
		case 0x22: /* ----- */
			return 11;
		case 0x23: /* mkdummy */
			return 12;
		}
		break;
	case PAD_B:
		break;
	}

	switch(pad & (PAD_U|PAD_D|PAD_L|PAD_R)){
	case PAD_U:
		work->selcom--;
		if((work->selcom & 0x0f)==0x0f){
			work->selcom&=~0x0f;
			work->selcom+=0x13;
		}
		break;
	case PAD_D:
		work->selcom++;
		if((work->selcom & 0x0f)>=0x04){
			work->selcom&=~0x0f;
		}
		break;
	case PAD_L:
		work->selcom-=0x10;
		if((work->selcom & 0xf0)==0xf0){
			work->selcom&=0x0f;
			work->selcom+=0x20;
		}
		break;
	case PAD_R:
		work->selcom+=0x10;
		if((work->selcom & 0xf0)>=0x30){
			work->selcom&=0x0f;
		}
		break;
	}

	return 0;
}


static int DispCardInfo(Work *work,int x,int y)
{
	int type=MCAccessGetType(&(work->mcwork),work->port,work->slot[work->port]);
	int size,formatted;

	switch(type){
	case -1: /* Error */
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Error Card");
		break;
	case 0: /* No Connected */
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("No Connected");
		break;
	case 1: /* PS1 */
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("MC for Play Station");
		break;
	case 2: /* PS2 */
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("MC for Play Station 2");

		size=MCAccessGetFreeSize(&(work->mcwork),work->port,work->slot[work->port]);
		formatted=MCAccessIsFormatted(&(work->mcwork),work->port,work->slot[work->port]);

		if(formatted){
			DEBUG_Locate(x+24*FONT_WIDTH,y,MENU_MODE_NORMAL);
			DEBUG_Printf("/ Size = %d",size);

			DEBUG_Locate(x+43*FONT_WIDTH,y,MENU_MODE_NORMAL);
			DEBUG_Printf("/ Formatted");

			DEBUG_Locate(x,y+FONT_HEIGHT,MENU_MODE_NORMAL);
			DEBUG_Printf("Current Path : %s",work->path);
			return 1;
		}
		else{
			DEBUG_Locate(x+24*FONT_WIDTH,y,MENU_MODE_NORMAL);
			DEBUG_Printf("/ Unformat");
		}
		break;
	case 3: /* Pocket Station */
		DEBUG_Locate(x,y,MENU_MODE_NORMAL);
		DEBUG_Printf("Pocket Station");
		break;
	}

	return 0;
}


static void DispAttr(Work *work,int x,int y)
{
	static const unsigned char flag_mark[]={
		'-','1','0',
	};
	int i,j;
	int *attr=work->modify_attr;


	DEBUG_Locate(x+2*FONT_WIDTH,y,MENU_MODE_NORMAL);
	DEBUG_Printf("Modify Attribute ( For Directory )");


	DEBUG_Locate(x+2*FONT_WIDTH,y+1*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf("R   %c",flag_mark[attr[0]]);

	DEBUG_Locate(x+2*FONT_WIDTH+(2+5+2)*FONT_WIDTH,y+1*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf("W   %c",flag_mark[attr[1]]);

	DEBUG_Locate(x+2*FONT_WIDTH+2*(2+5+2)*FONT_WIDTH,y+1*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf("X   %c",flag_mark[attr[2]]);


	DEBUG_Locate(x+2*FONT_WIDTH,y+2*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf("Du  %c",flag_mark[attr[3]]);

	DEBUG_Locate(x+2*FONT_WIDTH+(2+5+2)*FONT_WIDTH,y+2*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf("Pda %c",flag_mark[attr[4]]);

	DEBUG_Locate(x+2*FONT_WIDTH+2*(2+5+2)*FONT_WIDTH,y+2*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf("Ps1 %c",flag_mark[attr[5]]);

	if(work->selattr<3){
		i=work->selattr;
		j=0;
	}
	else{
		i=work->selattr-3;
		j=1;
	}

	DEBUG_Locate(x+i*(5+2+2)*FONT_WIDTH,y+(j+1)*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf(">>");
}

static int MoveCursor_AttrSelect(Work *work)
{
	int pad=work->pad_press;

	switch(pad & (PAD_A|PAD_B)){
	case PAD_A:
		return 1;
	case PAD_B:
		return -1;
	}

	switch(pad & (PAD_L|PAD_R|PAD_U|PAD_D)){
	case PAD_L:
		work->selattr--;
		if(work->selattr<0) work->selattr=6-1;
		break;
	case PAD_R:
		work->selattr++;
		if(work->selattr>=6) work->selattr=0;
		break;
	case PAD_U:
		work->modify_attr[work->selattr]--;
		if(work->modify_attr[work->selattr]<0){
			work->modify_attr[work->selattr]=3-1;
		}
		break;
	case PAD_D:
		work->modify_attr[work->selattr]++;
		if(work->modify_attr[work->selattr]>=3){
			work->modify_attr[work->selattr]=0;
		}
		break;
	}

	return 0;
}

#ifdef PSX2

#if 1 //BP_PS2
#define INPUT_STR \
   "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.,\"\'`~!@#$%%^&-_+=<>()[]{}|\\"

#else

#define INPUT_STR \
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.,\"\'`~!@#$\%^&-_+=<>()[]{}|\\"
#endif

#endif

#ifdef KP_XBOX
#define INPUT_STR \
	"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz.,\"\'`~!@#$%%^&-_+=<>()[]{}|\\"
#endif
#define INPUT_STRLEN		sizeof(INPUT_STR)

#define DISP_NAMEENTRY_STRLEN	11

static const char InputStr[]={
	INPUT_STR
	INPUT_STR
};

static void DispNameEntry(Work *work,int x,int y)
{
	char part[0x10];
	int i;

	for(i=0;i<DISP_NAMEENTRY_STRLEN;i++){
		int index=work->Letter_cursor+i-DISP_NAMEENTRY_STRLEN/2;
		if(index<0) index+=INPUT_STRLEN-1;
		part[i]=InputStr[index];
	}
	part[i]='\0';

	DEBUG_Locate(x+(DISP_NAMEENTRY_STRLEN/2)*FONT_WIDTH,y,MENU_MODE_NORMAL);
	DEBUG_Printf("V");

	DEBUG_Locate(x,y+FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf(part);
}

static void DispNameEntryCursor(Work *work,char *str)
{
	if(work->FrameCount & 0x20){
		if(str[work->NameEntry_cursor]=='\0'){
			str[work->NameEntry_cursor+1]='\0';
		}
		str[work->NameEntry_cursor]='_';
	}
}

static void DispRename(Work *work,int x,int y)
{
	char str[0x80];

	strcpy(str,work->MCFileName);

	DEBUG_Locate(x,y+FONT_HEIGHT*0,MENU_MODE_NORMAL);
	DEBUG_Printf("Rename from : %s",work->dirtable[work->selfile].EntryName);

	DispNameEntryCursor(work,str);

	DEBUG_Locate(x,y+FONT_HEIGHT*1,MENU_MODE_NORMAL);
	DEBUG_Printf("Rename to   : %s",str);

	DispNameEntry(work,x,y+FONT_HEIGHT*2);
}

static void DispMkdir(Work *work,int x,int y)
{
	char str[0x80];

	strcpy(str,work->MCFileName);

	DEBUG_Locate(x,y+FONT_HEIGHT*0,MENU_MODE_NORMAL);
	DEBUG_Printf("Mkdir Path : %s",work->path);

	DispNameEntryCursor(work,str);

	DEBUG_Locate(x,y+FONT_HEIGHT*1,MENU_MODE_NORMAL);
	DEBUG_Printf("Dir Name   : %s",str);

	DispNameEntry(work,x,y+FONT_HEIGHT*2);
}

static void DispUpload(Work *work,int x,int y)
{
	char str[0x80];

	strcpy(str,work->MCFileName);

	DEBUG_Locate(x,y+FONT_HEIGHT*0,MENU_MODE_NORMAL);
	DEBUG_Printf("Upload Path : %s",work->path);

	DispNameEntryCursor(work,str);

	DEBUG_Locate(x,y+FONT_HEIGHT*1,MENU_MODE_NORMAL);
	DEBUG_Printf("File Name   : %s",str);

	DispNameEntry(work,x,y+FONT_HEIGHT*2);
}

static int MoveCursor_NameEntry(Work *work,char *str)
{
	int press=work->pad_press;
	int on=work->pad_on;
	int len=strlen(str);

	switch(on & (PAD_X|PAD_Y)){
	case PAD_X:
		/* 決定終了 */
		return 1;
	case PAD_Y:
		/* キャンセル終了 */
		return -1;
	}

	switch(press & (PAD_A|PAD_B)){
	case PAD_A:
		/* 文字決定、Insert mode */
		{
			int i=strlen(str)+1;

			if(i>=FILENAME_LEN) i=FILENAME_LEN-2;

			while(i>=work->NameEntry_cursor){
				str[i+1]=str[i];
				i--;
			}
		}

		str[work->NameEntry_cursor]=InputStr[work->Letter_cursor];
		work->NameEntry_cursor++;
		if(work->NameEntry_cursor>=FILENAME_LEN) work->NameEntry_cursor=FILENAME_LEN;
		return 0;
	case PAD_B:
		/* BackSpace */
		{
			int i;

			work->NameEntry_cursor--;
			if(work->NameEntry_cursor<0) work->NameEntry_cursor=0;
			i=work->NameEntry_cursor;

			while(str[i]!='\0'){
				str[i]=str[i+1];
				i++;
			}
		}
		return 0;
	}

	/* 入力文字選択 */
	switch(on & (PAD_L|PAD_R|PAD_U|PAD_D)){
	case PAD_L:
		work->Letter_cursor--;
		if(work->Letter_cursor<0) work->Letter_cursor+=INPUT_STRLEN-1;
		break;
	case PAD_R:
		work->Letter_cursor++;
		if(work->Letter_cursor>=INPUT_STRLEN-1) work->Letter_cursor-=(INPUT_STRLEN-1);
		break;
	case PAD_U:
		work->Letter_cursor-=8;
		if(work->Letter_cursor<0) work->Letter_cursor+=INPUT_STRLEN-1;
		break;
	case PAD_D:
		work->Letter_cursor+=8;
		if(work->Letter_cursor>=INPUT_STRLEN-1) work->Letter_cursor-=(INPUT_STRLEN-1);
		break;
	}

	/* 入力位置移動 */
	switch(on & (PAD_L1|PAD_R1)){
	case PAD_L1:
		work->NameEntry_cursor--;
		if(work->NameEntry_cursor<0) work->NameEntry_cursor=0;
		break;
	case PAD_R1:
		work->NameEntry_cursor++;
		if(work->NameEntry_cursor>=len) work->NameEntry_cursor=len;
		break;
	}

	return 0;
}

static void CopyTo2(Work *work)
{
	int i;

	work->port2=work->port;
	for(i=0;i<PORT_MAX;i++) work->slot2[i]=work->slot[i];
	strcpy(work->path2,work->path);
}

static void DispConfirm(Work *work,int x,int y)
{
	DEBUG_Locate(x,y,MENU_MODE_NORMAL);
	DEBUG_Printf(" YES   NO");

	DEBUG_Locate(x+work->Confirm_cursor*FONT_WIDTH*6,y,MENU_MODE_NORMAL);
	DEBUG_Printf(">");
}

static void DispFormat(Work *work,int x,int y)
{
	DEBUG_Locate(x,y+FONT_HEIGHT*0,MENU_MODE_NORMAL);
	DEBUG_Printf("Format. OK?");

	DispConfirm(work,x,y+FONT_HEIGHT*2);
}

static void DispUnformat(Work *work,int x,int y)
{
	DEBUG_Locate(x,y+FONT_HEIGHT*0,MENU_MODE_NORMAL);
	DEBUG_Printf("Unformat. OK?");

	DispConfirm(work,x,y+FONT_HEIGHT*2);
}

static int MoveCursor_Confirm(Work *work)
{
	int press=work->pad_press;

	switch(press & (PAD_A|PAD_B)){
	case PAD_A:
		if(work->Confirm_cursor) return -1;
		else return 1;
	case PAD_B:
		return -1;
	}
	switch(press & (PAD_L|PAD_R)){
	case PAD_L:
		work->Confirm_cursor=0;
		break;
	case PAD_R:
		work->Confirm_cursor=1;
		break;
	}
	return 0;
}

static void DispSize(Work *work,int x,int y)
{
	DEBUG_Locate(x,y,MENU_MODE_NORMAL);
	DEBUG_Printf("< %4d KByte >",work->InputSize/1024);
}

static void DispDummyFile(Work *work,int x,int y)
{
	DEBUG_Locate(x,y+FONT_HEIGHT*0,MENU_MODE_NORMAL);
	DEBUG_Printf("Input Dummy File Size");

	DispSize(work,x,y+FONT_HEIGHT*2);
}

static int MoveCursor_InputSize(Work *work)
{
	int press=work->pad_press;
	int on=work->pad_on;

	switch(press & (PAD_A|PAD_B)){
	case PAD_A:
		return 1;
	case PAD_B:
		return -1;
	}

	switch(on & (PAD_L|PAD_R|PAD_U|PAD_D)){
	case PAD_L:
		work->InputSize+=INPUT_SIZE_UNIT;
		if(work->InputSize>INPUT_SIZE_U_LIMIT) work->InputSize=INPUT_SIZE_U_LIMIT;
		break;
	case PAD_R:
		work->InputSize-=INPUT_SIZE_UNIT;
		if(work->InputSize<INPUT_SIZE_L_LIMIT) work->InputSize=INPUT_SIZE_L_LIMIT;
		break;
	case PAD_U:
		work->InputSize+=INPUT_SIZE_UNIT*100;
		if(work->InputSize>INPUT_SIZE_U_LIMIT) work->InputSize=INPUT_SIZE_U_LIMIT;
		break;
	case PAD_D:
		work->InputSize-=INPUT_SIZE_UNIT*100;
		if(work->InputSize<INPUT_SIZE_L_LIMIT) work->InputSize=INPUT_SIZE_L_LIMIT;
		break;
	}

	return 0;
}

static void Disp(Work *work)
{
	DispMenu(work,0x10,0x10+3*FONT_HEIGHT);

	if(DispCardInfo(work,0x10,0x10+0*FONT_HEIGHT)){
		DispDirs(work,0x10,0x10+13*FONT_HEIGHT);
	}

	switch(work->mode){
	case MC_MENU_MODE_PORTSEL:
		if(MoveCursor_PortSelect(work)){
			ProcStepset(work,PROC_CHDIR_ROOT);
		}
		break;
	case MC_MENU_MODE_COMSEL:
		switch(MoveCursor_CommandSelect(work)){
		case 1: /* copy */
			work->mode=MC_MENU_MODE_COPY;
			work->mode2=0;
			CopyTo2(work);
			break;
		case 2: /* move */
#if 0
			work->mode=MC_MENU_MODE_MOVE;
			work->mode2=0;
			CopyTo2(work);
#endif
			break;
		case 3: /* delete */
			ProcStepset(work,PROC_DELETE);
			break;
		case 4: /* chmod */
			work->mode=MC_MENU_MODE_ATTRSEL;
			work->selattr=0;
			{
				int i;
				for(i=0;i<6;i++) work->modify_attr[i]=0;
			}
			break;

		case 5: /* format */
			work->mode=MC_MENU_MODE_FORMAT;
			work->Confirm_cursor=1;
			break;
		case 6: /* unformat */
			work->mode=MC_MENU_MODE_UNFORMAT;
			work->Confirm_cursor=1;
			break;
		case 7: /* mkdir */
			work->mode=MC_MENU_MODE_MKDIR;
			work->NameEntry_cursor=strlen(work->MCFileName);
			break;
		case 8: /* rename */
			work->mode=MC_MENU_MODE_RENAME;
			work->NameEntry_cursor=strlen(work->MCFileName);
			break;

		case 9: /* upload */
			work->mode=MC_MENU_MODE_UPLOAD;
			work->NameEntry_cursor=strlen(work->MCFileName);
			break;
		case 10: /* download */
			ProcStepset(work,PROC_DOWNLOAD);
			break;
		case 11:
			break;
		case 12: /* mkdummy */
			work->mode=MC_MENU_MODE_MKDUMMY;
			break;
		}
		break;
	case MC_MENU_MODE_FILESEL:
		switch(MoveCursor_Dirs(work)){
		case 1:
			ProcStepset(work,PROC_CHDIR);
			break;
		case 2:
			ProcStepset(work,PROC_CHDIR_ROOT);
			break;
		}
		break;

	case MC_MENU_MODE_ATTRSEL:
		DispAttr(work,0x10,0x10+8*FONT_HEIGHT);
		switch(MoveCursor_AttrSelect(work)){
		case 1:
			ProcStepset(work,PROC_CHMOD);
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		case -1:
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		}
		break;

	case MC_MENU_MODE_RENAME:
		DispRename(work,0x10,0x10+8*FONT_HEIGHT);
		switch(MoveCursor_NameEntry(work,work->MCFileName)){
		case 1:
			ProcStepset(work,PROC_RENAME);
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		case -1:
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		}
		break;

	case MC_MENU_MODE_MKDIR:
		DispMkdir(work,0x10,0x10+8*FONT_HEIGHT);
		switch(MoveCursor_NameEntry(work,work->MCFileName)){
		case 1:
			ProcStepset(work,PROC_MKDIR);
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		case -1:
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		}
		break;

	case MC_MENU_MODE_UPLOAD:
		DispUpload(work,0x10,0x10+8*FONT_HEIGHT);
		switch(MoveCursor_NameEntry(work,work->MCFileName)){
		case 1:
			ProcStepset(work,PROC_UPLOAD);
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		case -1:
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		}
		break;

	case MC_MENU_MODE_COPY:
	case MC_MENU_MODE_MOVE:
		switch(work->mode2){
		case 0:
			ProcStepset(work,PROC_GETDIR2);
			work->mode2++;
			break;
		case 1:
			DispDirs2(work,0x10,0x10+7*FONT_HEIGHT);
			switch(MoveCursor_Dirs2(work)){
			case 1:
				ProcStepset(work,PROC_CHDIR2);
				work->mode2++;
				break;
			case 2:
				ProcStepset(work,PROC_CHDIR2_ROOT);
				work->mode2++;
				work->dirtable2_used_size=0;
				work->cur_start_dir=0;
				work->seldir=0;
				break;
			case 3:
				if(work->mode==MC_MENU_MODE_COPY) ProcStepset(work,PROC_COPY);
				else ProcStepset(work,PROC_MOVE);
				work->mode2=3;
				break;
			}
			break;
		case 2:
			work->mode2=1;
			break;
		case 3:
			work->mode=MC_MENU_MODE_COMSEL;
			work->mode2=0;
			break;
		}
		break;
	case MC_MENU_MODE_FORMAT:
		DispFormat(work,0x10,0x10+8*FONT_HEIGHT);
		switch(MoveCursor_Confirm(work)){
		case 1:
			ProcStepset(work,PROC_FORMAT);
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		case -1:
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		}
		break;
	case MC_MENU_MODE_UNFORMAT:
		DispUnformat(work,0x10,0x10+8*FONT_HEIGHT);
		switch(MoveCursor_Confirm(work)){
		case 1:
			ProcStepset(work,PROC_UNFORMAT);
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		case -1:
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		}
		break;
	case MC_MENU_MODE_MKDUMMY:
		DispDummyFile(work,0x10,0x10+8*FONT_HEIGHT);
		switch(MoveCursor_InputSize(work)){
		case 1:
			ProcStepset(work,PROC_MKDUMMY);
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		case -1:
			work->mode=MC_MENU_MODE_COMSEL;
			break;
		}
	}
}

static void DispWaiting(Work *work)
{
	DEBUG_Locate(0x10,0x10+0*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf("Now Accessing Memory card.\n");

	DEBUG_Locate(0x10,0x10+2*FONT_HEIGHT,MENU_MODE_NORMAL);
	DEBUG_Printf("Please Wait.\n");
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void Act(Work *work)
{
	MCAccessAct(&(work->mcwork));

	KeyAct(work);

	if(Proc(work)){
		DispWaiting(work);
	}
	else{
		Disp(work);
	}

	MCAccessPostAct(&(work->mcwork));

	work->FrameCount++;
}

static void Die(Work *work)
{
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* 初期設定値を取得 */
static void GetOptionValue(Work *work)
{
}

/* 資源を獲得 */
static int GetResources(Work *work)
{
	int i;

    GetOptionValue(work);

	MCAccessInit(&(work->mcwork),0);
	MCAccessAutoCheck(&(work->mcwork),1);

	work->port=work->tmpport=0;
	for(i=0;i<PORT_MAX;i++) work->slot[i]=work->tmpslot[i]=0;

	strcpy(work->path,"/");

	work->pad_press=0;
	work->pad_on=0;
	work->pad_on0=0;
	work->pad_repeat_count=0;

	work->mode=MC_MENU_MODE_PORTSEL;
	work->mode2=0;

	work->selfile=0;
	work->selcom=0;
	work->cur_start_file=0;

	work->MCFileName[0]='\0';

	ProcStepset(work,PROC_MCCHECK);
	work->FrameCount=0;

	work->InputSize=1024*1024;

    return 1;
}

/* 初期化部メイン */
void *NewMemcardChecker(int name,int where)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof( Work )) ;
    if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;

		if(!GetResources(work)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
