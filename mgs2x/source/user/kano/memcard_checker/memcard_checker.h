/*
	memcard_checker.h
		メモリーカードチェッカー
	
	2001/01/12 K.Kano
	$Id: memcard_checker.h,v 1.1.1.3 2002/11/19 11:43:19 Yoshizawa1 Exp $
*/


#ifndef _memcard_checker_h_
#define _memcard_checker_h_


#include "memcard_access.h"


#define DIRTABLE_SIZE		0x100


typedef struct {
	GV_ACT actor;

	MCAccessWork mcwork;

	sceMcTblGetDir dirtable[DIRTABLE_SIZE];
	int dirtable_used_size;

	int FrameCount;
	int proc_step;
	int proc_substep;

	int port,slot[PORT_MAX];
	int tmpport,tmpslot[PORT_MAX];

	char path[0x80];

	int mode,mode2;

	int selfile;
	int selcom;
	int selattr;
	int modify_attr[6];
	unsigned short attr_mask1,attr_mask0;

	int cur_start_file;

	int pad_press;
	int pad_on,pad_on0;
	int pad_repeat_count;

	int NameEntry_cursor;
	int Letter_cursor;
	int Confirm_cursor;
	int InputSize;

	char MCFileName[0x80];

	void *loadwork;
	int loadwork_size;
	int fd;
	int load_ans;


	sceMcTblGetDir dirtable2[DIRTABLE_SIZE];
	int dirtable2_used_size;
	int getdir_used_size;

	int seldir;
	int cur_start_dir;

	int port2,slot2[PORT_MAX];
	char path2[0x80];
} Work;


#define FONT_WIDTH		0x08
#define FONT_HEIGHT		0x12


#define FILENAME_LEN	63


// Menu Mode

enum {
	MC_MENU_MODE_PORTSEL=0,
	MC_MENU_MODE_COMSEL,
	MC_MENU_MODE_FILESEL,

	MC_MENU_MODE_MAX,

	MC_MENU_MODE_ATTRSEL=MC_MENU_MODE_MAX,
	MC_MENU_MODE_RENAME,
	MC_MENU_MODE_MKDIR,
	MC_MENU_MODE_UPLOAD,
	MC_MENU_MODE_DOWNLOAD,
	MC_MENU_MODE_COPY,
	MC_MENU_MODE_MOVE,
	MC_MENU_MODE_FORMAT,
	MC_MENU_MODE_UNFORMAT,
	MC_MENU_MODE_MKDUMMY,
};


// Processing

enum {
	PROC_WAIT=0,
	PROC_MCCHECK,
	PROC_GETDIR,
	PROC_CHDIR,
	PROC_CHDIR_ROOT,

	PROC_COPY,
	PROC_MOVE,
	PROC_DELETE,
	PROC_CHMOD,

	PROC_FORMAT,
	PROC_UNFORMAT,
	PROC_MKDIR,
	PROC_RENAME,

	PROC_UPLOAD,
	PROC_DOWNLOAD,

	PROC_GETDIR2,
	PROC_CHDIR2,
	PROC_CHDIR2_ROOT,

	PROC_CHDIR3,

	PROC_MKDUMMY,
};


void ProcStepup(Work *work);
void ProcStepset(Work *work,int s);
void ProcSubStepup(Work *work);
void ProcSubStepset(Work *work,int s);

int Proc(Work *work);


#define DUMMY_FILE_NAME			"DUMMYFILE"

#define INPUT_SIZE_L_LIMIT		1024
#define INPUT_SIZE_U_LIMIT		(1024*1024*8)
#define INPUT_SIZE_UNIT			1024


#endif
