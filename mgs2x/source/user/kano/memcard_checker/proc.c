//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	proc.c
		メモリーカードチェッカー
	
	2001/01/12 K.Kano
	$Id: proc.c,v 1.1.1.3 2002/11/19 11:43:19 Yoshizawa1 Exp $
*/


#include "memcard_checker.h"

#include "libfs.h"

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

#define sceMcFileInfoCreate	0x01
#define sceMcFileInfoModify	0x02
#define sceMcFileInfoAttr	0x04

#if 0

// These interfere with defines in the Vita headers

#  define SCE_RDONLY      0x0001
#  define SCE_WRONLY      0x0002
#  define SCE_RDWR        0x0003
#  define SCE_NBLOCK      0x0010  /* Non-Blocking I/O */
#  define SCE_APPEND      0x0100  /* append (writes guaranteed at the end) */
#  define SCE_CREAT       0x0200  /* open with file create */
#  define SCE_TRUNC       0x0400  /* open with truncation */
#  define SCE_EXCL        0x0800  /* exclusive create */
#  define SCE_NOBUF       0x4000  /* no device buffer and console interrupt */
#  define SCE_NOWAIT      0x8000  /* asyncronous i/o */

#endif

#ifndef SCE_SEEK_SET
#define SCE_SEEK_SET        (0)
#endif
#ifndef SCE_SEEK_CUR
#define SCE_SEEK_CUR        (1)
#endif
#ifndef SCE_SEEK_END
#define SCE_SEEK_END        (2)
#endif

#endif


/* ------------------------------------------------------------------------ */


void ProcStepup(Work *work)
{
	printf("Proc : %d -> %d\n",work->proc_step,work->proc_step+1);

	work->proc_step++;
	work->proc_substep=0;
}

void ProcStepset(Work *work,int s)
{
	printf("Proc : %d -> %d\n",work->proc_step,s);

	work->proc_step=s;
	work->proc_substep=0;
}

void ProcSubStepup(Work *work)
{
	work->proc_substep++;
}

void ProcSubStepset(Work *work,int s)
{
	work->proc_substep=s;
}


/* ------------------------------------------------------------------------ */


static int Proc_MCCheck(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessCheckStart(&(work->mcwork))) ProcSubStepup(work);
		break;
	case 1:
		if(!MCAccessing(&(work->mcwork))){
			return 1;
		}
		break;
	}

	return 0;
}

static int Proc_GetDir(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessGetdirStart(&(work->mcwork),work->port,work->slot[work->port],"*",
							   work->dirtable,DIRTABLE_SIZE,&(work->dirtable_used_size))){

			ProcSubStepup(work);
		}
		break;
	case 1:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			/* Filerの中でマークの記録用に使われる */
			{
				int i;
				for(i=0;i<work->dirtable_used_size;i++){
					work->dirtable[i].Reserve1=0;
				}
			}
			work->selfile=0;
			work->cur_start_file=0;
			return 1;
		case -1:
			work->dirtable_used_size=0;
			return 1;
		case -2:
			/* MCCheckへ */
			return -2;
		}
		break;
	}

	return 0;
}

static int Proc_Chdir(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(!(work->dirtable[work->selfile].AttrFile & sceMcFileAttrSubdir)) return 2;

		if(MCAccessChdirStart(&(work->mcwork),work->port,work->slot[work->port],
							  work->dirtable[work->selfile].EntryName,work->path)){

			ProcSubStepup(work);
		}
		break;
	case 1:
		return MCAccessGetResult(&(work->mcwork));
	}

	return 0;
}

static int Proc_ChdirRoot(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessChdirStart(&(work->mcwork),work->port,work->slot[work->port],
							  "/",work->path)){

			ProcSubStepup(work);
		}
		break;
	case 1:
		return MCAccessGetResult(&(work->mcwork));
	}

	return 0;
}


static int Proc_GetDir2(Work *work)
{
	switch(work->proc_substep){
	case 0:
		work->seldir=0;
		work->cur_start_dir=0;

		work->dirtable2_used_size=0;
		if(MCAccessGetdirStart(&(work->mcwork),work->port2,work->slot2[work->port2],"*",
							   work->dirtable2+work->dirtable2_used_size,1,
							   &(work->getdir_used_size))){

			ProcSubStepup(work);
		}
		break;
	case 1:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			if(work->getdir_used_size){
				if(work->dirtable2[work->dirtable2_used_size].AttrFile &
				   sceMcFileAttrSubdir){

					work->dirtable2_used_size++;
				}
				ProcSubStepup(work);
			}
			else return 1;
			break;
		case -1:
			return 1;
		case -2:
			/* MCCheckへ */
			return -2;
		}
		break;
	case 2:
		if(MCAccessGetdirNextStart(&(work->mcwork),work->port2,work->slot2[work->port2],"*",
								   work->dirtable2+work->dirtable2_used_size,1,
								   &(work->getdir_used_size))){

			ProcSubStepup(work);
		}
		break;
	case 3:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			if(work->getdir_used_size){
				if(work->dirtable2[work->dirtable2_used_size].AttrFile &
				   sceMcFileAttrSubdir){

					work->dirtable2_used_size++;
				}
				ProcSubStepset(work,2);
			}
			else return 1;
			break;
		case -1:
			return 1;
		case -2:
			/* MCCheckへ */
			return -2;
		}
		break;
	}

	return 0;
}

static int Proc_Chdir2(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessChdirStart(&(work->mcwork),work->port2,work->slot2[work->port2],
							  work->dirtable2[work->seldir].EntryName,work->path2)){

			ProcSubStepup(work);
		}
		break;
	case 1:
		return MCAccessGetResult(&(work->mcwork));
	}

	return 0;
}

static int Proc_Chdir2Root(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessChdirStart(&(work->mcwork),work->port2,work->slot2[work->port2],
							  "/",work->path2)){

			ProcSubStepup(work);
		}
		break;
	case 1:
		return MCAccessGetResult(&(work->mcwork));
	}

	return 0;
}

static int Proc_Chdir3(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(!(work->dirtable[work->selfile].AttrFile & sceMcFileAttrSubdir)) return 2;

		if(MCAccessChdirStart(&(work->mcwork),work->port,work->slot[work->port],
							  work->path,NULL)){

			ProcSubStepup(work);
		}
		break;
	case 1:
		return MCAccessGetResult(&(work->mcwork));
	}

	return 0;
}


static int Proc_Delete(Work *work)
{
	switch(work->proc_substep){
	case 0:
		{
			int n_marks=0;
			int i;

			for(i=0;i<work->dirtable_used_size;i++){
				if(work->dirtable[i].Reserve1){
					n_marks++;
				}
			}
			if(n_marks>0){
				ProcSubStepset(work,0x10);
			}
			else{
				ProcSubStepup(work);
			}
		}
		break;
	case 1:
		if(MCAccessDeleteStart(&(work->mcwork),work->port,work->slot[work->port],
							   work->dirtable[work->selfile].EntryName)){

			ProcSubStepup(work);
		}
		break;
	case 2:
		return MCAccessGetResult(&(work->mcwork));

	default:
		{
			int index=(work->proc_substep>>4)-1;
			int step=work->proc_substep & 0x0f;

			switch(step){
			case 0:
				if(index>=work->dirtable_used_size){
					/* Getdir */
					return 1;
				}

				if(work->dirtable[index].Reserve1){
					ProcSubStepup(work);
				}
				else{
					ProcSubStepset(work,work->proc_substep+0x10);
				}
				break;
			case 1:
				if(MCAccessDeleteStart(&(work->mcwork),work->port,work->slot[work->port],
									   work->dirtable[index].EntryName)){

					ProcSubStepup(work);
				}
				break;
			case 2:
				switch(MCAccessGetResult(&(work->mcwork))){
				case 1:
				case -1:
				case -2:
					ProcSubStepset(work,(work->proc_substep & ~0x0f)+0x10);
					break;
				}
				break;
			}
		}
		break;
	}

	return 0;
}

static int Proc_Chmod(Work *work)
{
	switch(work->proc_substep){
	case 0:
		{
			int n_marks=0;
			int i;


			work->attr_mask1=0;
			work->attr_mask0=0;

			for(i=0;i<6;i++){
				static const unsigned short flagdata[]={
					sceMcFileAttrReadable,
					sceMcFileAttrWriteable,
					sceMcFileAttrExecutable,
					sceMcFileAttrDupProhibit,
					sceMcFileAttrPDAExec,
					sceMcFileAttrPS1,
				};

				switch(work->modify_attr[i]){
				case 1:
					work->attr_mask1|=flagdata[i];
					break;
				case 2:
					work->attr_mask0|=flagdata[i];
					break;
				}
			}

			work->attr_mask0=~work->attr_mask0;


			for(i=0;i<work->dirtable_used_size;i++){
				if(work->dirtable[i].Reserve1){
					n_marks++;
				}
			}
			if(n_marks>0){
				ProcSubStepset(work,0x10);
			}
			else{
				work->dirtable[work->selfile].AttrFile|=work->attr_mask1;
				work->dirtable[work->selfile].AttrFile&=work->attr_mask0;

				ProcSubStepup(work);
			}
		}
		break;
	case 1:
		if(MCAccessChmodStart(&(work->mcwork),work->port,work->slot[work->port],
							  work->dirtable[work->selfile].EntryName,
							  &(work->dirtable[work->selfile]),
							  sceMcFileInfoAttr)){

			ProcSubStepup(work);
		}
		break;
	case 2:
		return MCAccessGetResult(&(work->mcwork));

	default:
		{
			int index=(work->proc_substep>>4)-1;
			int step=work->proc_substep & 0x0f;

			switch(step){
			case 0:
				if(index>=work->dirtable_used_size){
					/* Getdir */
					return 1;
				}

				if(work->dirtable[index].Reserve1){
					work->dirtable[index].AttrFile|=work->attr_mask1;
					work->dirtable[index].AttrFile&=work->attr_mask0;

					ProcSubStepup(work);
				}
				else{
					ProcSubStepset(work,work->proc_substep+0x10);
				}
				break;
			case 1:
				if(MCAccessChmodStart(&(work->mcwork),work->port,work->slot[work->port],
									  work->dirtable[index].EntryName,
									  &(work->dirtable[index]),
									  sceMcFileInfoAttr)){

					ProcSubStepup(work);
				}
				break;
			case 2:
				switch(MCAccessGetResult(&(work->mcwork))){
				case 1:
				case -1:
				case -2:
					ProcSubStepset(work,(work->proc_substep & ~0x0f)+0x10);
					break;
				}
				break;
			}
		}
		break;
	}

	return 0;
}

static int Proc_Rename(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessRenameStart(&(work->mcwork),work->port,work->slot[work->port],
							   work->dirtable[work->selfile].EntryName,work->MCFileName)){

			ProcSubStepup(work);
		}
		break;
	case 1:
		return MCAccessGetResult(&(work->mcwork));
	}

	return 0;
}

static int Proc_Mkdir(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessMkdirStart(&(work->mcwork),work->port,work->slot[work->port],work->MCFileName)){
			ProcSubStepup(work);
		}
		break;
	case 1:
		return MCAccessGetResult(&(work->mcwork));
	}

	return 0;
}

static int Proc_Format(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessFormatStart(&(work->mcwork),work->port,work->slot[work->port])){
			ProcSubStepup(work);
		}
		break;
	case 1:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
		case -1:
		case -2:
			/* MCCheckへ */
			return -2;
		}
		break;
	}

	return 0;
}

static int Proc_Unformat(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessUnformatStart(&(work->mcwork),work->port,work->slot[work->port])){
			ProcSubStepup(work);
		}
		break;
	case 1:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
		case -1:
		case -2:
			/* MCCheckへ */
			return -2;
		}
		break;
	}

	return 0;
}

static int ProcSub_Copy(Work *work,int step,int index)
{
	// printf("SubStep = %d\n");

	switch(step){
	case 0:
		work->loadwork_size=work->dirtable[index].FileSizeByte;
		if((work->loadwork=GV_Malloc(work->loadwork_size))==NULL) return -1;
		ProcSubStepup(work);
		break;
	case 1:
		if(MCAccessChdirStart(&(work->mcwork),work->port,work->slot[work->port],
							  work->path,NULL)){

			ProcSubStepup(work);
		}
		break;
	case 2:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	case 3:
		if(MCAccessOpenStart(&(work->mcwork),work->port,work->slot[work->port],
							 work->dirtable[index].EntryName,SCE_RDONLY,&(work->fd))){

			ProcSubStepup(work);
		}
		break;
	case 4:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	case 5:
		if(MCAccessReadStart(&(work->mcwork),work->fd,
							 work->loadwork,work->loadwork_size,NULL)){

			ProcSubStepup(work);
		}
		break;
	case 6:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			work->load_ans=1;
			break;
		case -1:
			/* Getdirへ */
			ProcSubStepup(work);
			work->load_ans=-1;
			break;
		case -2:
			/* MCCheckへ */
			ProcSubStepup(work);
			work->load_ans=-2;
			break;
		}
		break;
	case 7:
		if(MCAccessCloseStart(&(work->mcwork),work->fd)){
			ProcSubStepup(work);
		}
		break;
	case 8:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			/* Getdirへ */
			if(work->load_ans<0){
				GV_Free(work->loadwork);
				return work->load_ans;
			}
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	case 9:
		if(MCAccessChdirStart(&(work->mcwork),work->port2,work->slot2[work->port2],
							  work->path2,NULL)){

			ProcSubStepup(work);
		}
		break;
	case 10:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	case 11:
		if(MCAccessOpenStart(&(work->mcwork),work->port2,work->slot2[work->port2],
							 work->dirtable[index].EntryName,
							 SCE_WRONLY|SCE_CREAT,&(work->fd))){

			ProcSubStepup(work);
		}
		break;
	case 12:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	case 13:
		if(MCAccessWriteStart(&(work->mcwork),work->fd,
							  work->loadwork,work->loadwork_size,NULL)){

			ProcSubStepup(work);
		}
		break;
	case 14:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			work->load_ans=1;
			break;
		case -1:
			/* Getdirへ */
			ProcSubStepup(work);
			work->load_ans=-1;
			break;
		case -2:
			/* MCCheckへ */
			ProcSubStepup(work);
			work->load_ans=-2;
			break;
		}
		break;
	case 15:
		if(MCAccessCloseStart(&(work->mcwork),work->fd)){
			ProcSubStepup(work);
		}
		break;
	case 16:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return work->load_ans;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	}

	return 0;
}

static int Proc_Copy(Work *work)
{
	if(work->proc_substep==0){
		int i,cnt=0;
		for(i=0;i<work->dirtable_used_size;i++){
			if(work->dirtable[i].Reserve1) cnt++;
		}
		if(cnt){
			ProcSubStepset(work,0x100);
		}
	}
	if(work->proc_substep>=0x100){
		int index;
		int step;

		step=work->proc_substep & 0xff;
		index=(work->proc_substep>>8)-1;

		if(index>=work->dirtable_used_size) return 1;

		if(work->dirtable[index].Reserve1){
			if(ProcSub_Copy(work,step,index)){
				ProcSubStepset(work,(work->proc_substep & ~0xff)+0x100);
			}
		}
		else{
			ProcSubStepset(work,(work->proc_substep & ~0xff)+0x100);
		}
	}
	else{
		return ProcSub_Copy(work,work->proc_substep,work->selfile);
	}

	return 0;
}


/* ---------------------------------------------------------------- */

#define sceOpen( a, b )		pcOpen( a, b )
#define sceRead( a, b, c )	pcRead( a, b, c )
#define sceWrite( a, b, c ) pcWrite( a, b, c )
#define sceClose( a )		pcClose( a )
#define sceLseek( a, b, c )	pcLseek( a, b, c )

static int SaveBinFile(char *filename,void *buf,int size)
{
    int wfd;
	int ans=1;

    /* ファイルの書き込みオープン */
    wfd = sceOpen(filename,SCE_WRONLY|SCE_TRUNC|SCE_CREAT);
    if (wfd < 0 ) {
		/* エラー処理 */
		printf("Can't open %s\n",filename);
		return 0;
    }

    if(sceWrite(wfd,(unsigned char *)buf,size)<0){
		printf("Can't write %s\n",filename);
		ans=0;
	}
    sceClose(wfd);

	return ans;
}

static int GetSize(char *filename)
{
    int rfd;
	int len;

    /* ファイルの読み込みオープン */
    rfd = sceOpen(filename,SCE_RDONLY);
	if(rfd<0) return -1;

    /* ファイルサイズの確認 */
    len = sceLseek(rfd, 0, SCE_SEEK_END);

    sceClose(rfd);

	return len;
}

static int LoadBinFile(char *filename,void *buf,int size)
{
    int rfd;
	int ans=1;

    /* ファイルの読み込みオープン */
    rfd = sceOpen(filename,SCE_RDONLY);
    if (rfd < 0 ) {
		/* エラー処理 */
		printf("Can't open  %s\n",filename);
		return 0;
    }

    if(sceRead(rfd,(unsigned char *)buf,size)<0){
		printf("Can't read %s\n",filename);
		ans=0;
	}
    sceClose(rfd);

	return ans;
}

/* ---------------------------------------------------------------- */


static int Proc_Upload(Work *work)
{
	// printf("SubProc = %d\n",work->proc_substep);

	switch(work->proc_substep){
	case 0:
		{
			char tmpfilename[0x100];

			strcpy(tmpfilename,"host:./");
			strcat(tmpfilename,work->MCFileName);

			if((work->loadwork_size=GetSize(tmpfilename))<0) return -1;

			if((work->loadwork=GV_Malloc(work->loadwork_size))==NULL){
				return -1;
			}

			if(!LoadBinFile(tmpfilename,work->loadwork,work->loadwork_size)){
				GV_Free(work->loadwork);
				return -1;
			}
		}
		ProcSubStepup(work);
		break;
	case 1:
		if(MCAccessOpenStart(&(work->mcwork),work->port,work->slot[work->port],
							 work->MCFileName,SCE_WRONLY|SCE_CREAT,&(work->fd))){

			ProcSubStepup(work);
		}
		break;
	case 2:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	case 3:
		if(MCAccessWriteStart(&(work->mcwork),work->fd,
							  work->loadwork,work->loadwork_size,NULL)){

			ProcSubStepup(work);
		}
		break;
	case 4:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			work->load_ans=1;
			break;
		case -1:
			/* Getdirへ */
			ProcSubStepup(work);
			work->load_ans=-1;
			break;
		case -2:
			/* MCCheckへ */
			ProcSubStepup(work);
			work->load_ans=-2;
			break;
		}
		break;
	case 5:
		if(MCAccessCloseStart(&(work->mcwork),work->fd)){
			ProcSubStepup(work);
		}
		break;
	case 6:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return work->load_ans;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	}

	return 0;
}

static int Proc_Download(Work *work)
{
	switch(work->proc_substep){
	case 0:
		{
			int i;
			int cnt=0;

			for(i=0;i<work->dirtable_used_size;i++){
				if(work->dirtable[i].Reserve1) cnt++;
			}
			if(cnt>0){
				ProcSubStepset(work,0x10);
				break;
			}
		}

		work->loadwork_size=work->dirtable[work->selfile].FileSizeByte;
		if((work->loadwork=GV_Malloc(work->loadwork_size))==NULL){
			return -1;
		}
		ProcSubStepup(work);
		break;
	case 1:
		if(MCAccessOpenStart(&(work->mcwork),work->port,work->slot[work->port],
							 work->dirtable[work->selfile].EntryName,
							 SCE_RDONLY,&(work->fd))){

			ProcSubStepup(work);
		}
		break;
	case 2:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	case 3:
		if(MCAccessReadStart(&(work->mcwork),work->fd,
							 work->loadwork,work->loadwork_size,NULL)){

			ProcSubStepup(work);
		}
		break;
	case 4:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			work->load_ans=1;
			break;
		case -1:
			/* Getdirへ */
			ProcSubStepup(work);
			work->load_ans=-1;
			break;
		case -2:
			/* MCCheckへ */
			ProcSubStepup(work);
			work->load_ans=-2;
			break;
		}
		break;
	case 5:
		if(MCAccessCloseStart(&(work->mcwork),work->fd)){
			ProcSubStepup(work);
		}
		break;
	case 6:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			if(work->load_ans<0) return work->load_ans;
			break;
		case -1:
			/* Getdirへ */
			GV_Free(work->loadwork);
			return -1;
		case -2:
			/* MCCheckへ */
			GV_Free(work->loadwork);
			return -2;
		}
		break;
	case 7:
		{
			char tmpfilename[0x100];

			strcpy(tmpfilename,"host:./");
			strcat(tmpfilename,work->dirtable[work->selfile].EntryName);

			SaveBinFile(tmpfilename,work->loadwork,work->loadwork_size);

			GV_Free(work->loadwork);
		}
		return 1;

	default:
		{
			int step=work->proc_substep & 0x0f;
			int index=(work->proc_substep>>4)-1;

			switch(step){
			case 0:
				if(index>=work->dirtable_used_size) return 1;

				if(!work->dirtable[index].Reserve1){
					ProcSubStepset(work,0x10);
					break;
				}

				work->loadwork_size=work->dirtable[index].FileSizeByte;
				if((work->loadwork=GV_Malloc(work->loadwork_size))==NULL){
					return -1;
				}
				ProcSubStepup(work);
				break;
			case 1:
				if(MCAccessOpenStart(&(work->mcwork),work->port,work->slot[work->port],
									 work->dirtable[index].EntryName,
									 SCE_RDONLY,&(work->fd))){

					ProcSubStepup(work);
				}
				break;
			case 2:
				switch(MCAccessGetResult(&(work->mcwork))){
				case 1:
					ProcSubStepup(work);
					break;
				case -1:
					/* Getdirへ */
					GV_Free(work->loadwork);
					return -1;
				case -2:
					/* MCCheckへ */
					GV_Free(work->loadwork);
					return -2;
				}
				break;
			case 3:
				if(MCAccessReadStart(&(work->mcwork),work->fd,
									 work->loadwork,work->loadwork_size,NULL)){

					ProcSubStepup(work);
				}
				break;
			case 4:
				switch(MCAccessGetResult(&(work->mcwork))){
				case 1:
					ProcSubStepup(work);
					work->load_ans=1;
					break;
				case -1:
					/* Getdirへ */
					ProcSubStepup(work);
					work->load_ans=-1;
					break;
				case -2:
					/* MCCheckへ */
					ProcSubStepup(work);
					work->load_ans=-2;
					break;
				}
				break;
			case 5:
				if(MCAccessCloseStart(&(work->mcwork),work->fd)){
					ProcSubStepup(work);
				}
				break;
			case 6:
				switch(MCAccessGetResult(&(work->mcwork))){
				case 1:
					ProcSubStepup(work);
					if(work->load_ans<0) return work->load_ans;
					break;
				case -1:
					/* Getdirへ */
					GV_Free(work->loadwork);
					return -1;
				case -2:
					/* MCCheckへ */
					GV_Free(work->loadwork);
					return -2;
				}
				break;
			case 7:
				{
					char tmpfilename[0x100];

					strcpy(tmpfilename,"host:./");
					strcat(tmpfilename,work->dirtable[index].EntryName);

					SaveBinFile(tmpfilename,work->loadwork,work->loadwork_size);

					GV_Free(work->loadwork);
				}
				ProcSubStepset(work,(work->proc_substep & ~0x0f)+0x10);
				break;
			}
		}
		break;
	}

	return 0;
}


static int Proc_MakeDummyFile(Work *work)
{
	switch(work->proc_substep){
	case 0:
		if(MCAccessChdirStart(&(work->mcwork),work->port,work->slot[work->port],
							  work->path,NULL)){

			ProcSubStepup(work);
		}
		break;
	case 1:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			return -1;
		case -2:
			/* MCCheckへ */
			return -2;
		}
		break;
	case 2:
		if(MCAccessOpenStart(&(work->mcwork),work->port,work->slot[work->port],
							 DUMMY_FILE_NAME,SCE_WRONLY|SCE_CREAT,&(work->fd))){

			ProcSubStepup(work);
		}
		break;
	case 3:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			break;
		case -1:
			/* Getdirへ */
			return -1;
		case -2:
			/* MCCheckへ */
			return -2;
		}
		break;
	case 4:
		if(MCAccessWriteStart(&(work->mcwork),work->fd,work,work->InputSize,NULL)){
			ProcSubStepup(work);
		}
		break;
	case 5:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			ProcSubStepup(work);
			work->load_ans=1;
			break;
		case -1:
			/* Getdirへ */
			ProcSubStepup(work);
			work->load_ans=-1;
			break;
		case -2:
			/* MCCheckへ */
			ProcSubStepup(work);
			work->load_ans=-2;
			break;
		}
		break;
	case 6:
		if(MCAccessCloseStart(&(work->mcwork),work->fd)){
			ProcSubStepup(work);
		}
		break;
	case 7:
		switch(MCAccessGetResult(&(work->mcwork))){
		case 1:
			/* Getdirへ */
			return work->load_ans;
		case -1:
			/* Getdirへ */
			return -1;
		case -2:
			/* MCCheckへ */
			return -2;
		}
		break;
	}

	return 0;
}

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */


int Proc(Work *work)
{
	switch(work->proc_step){
	case PROC_WAIT:
		if(MCAccessing(&(work->mcwork))) return 1;

		if(MCAccessCheckSeqFlag(&(work->mcwork))){
			if(MCAccessGetType(&(work->mcwork),work->port,work->slot[work->port])==2 &&
			   MCAccessIsFormatted(&(work->mcwork),work->port,work->slot[work->port])){

				ProcStepset(work,PROC_CHDIR_ROOT);
				work->mode=MC_MENU_MODE_COMSEL;
				work->mode2=0;

				return 1;
			}
		}
		return 0;
	case PROC_MCCHECK:
		if(Proc_MCCheck(work)){
			if(MCAccessGetType(&(work->mcwork),work->port,work->slot[work->port])==2 &&
			   MCAccessIsFormatted(&(work->mcwork),work->port,work->slot[work->port])){

				ProcStepset(work,PROC_CHDIR_ROOT);
			}
			else{
				ProcStepset(work,PROC_WAIT);
			}
		}
		break;
	case PROC_GETDIR:
		switch(Proc_GetDir(work)){
		case 1:
			ProcStepset(work,PROC_WAIT);
			break;
		case -1:
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_CHDIR:
		switch(Proc_Chdir(work)){
		case 2:
			ProcStepset(work,PROC_WAIT);
			break;
		case 1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -1:
			ProcStepset(work,PROC_CHDIR_ROOT);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_CHDIR_ROOT:
		switch(Proc_ChdirRoot(work)){
		case 1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -1:
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;

	case PROC_COPY:
		switch(Proc_Copy(work)){
		case 1:
		case -1:
			ProcStepset(work,PROC_CHDIR3);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_MOVE:
		ProcStepset(work,PROC_WAIT);
		break;
	case PROC_DELETE:
		switch(Proc_Delete(work)){
		case 1:
		case -1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_CHMOD:
		switch(Proc_Chmod(work)){
		case 1:
		case -1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;

	case PROC_FORMAT:
		switch(Proc_Format(work)){
		case 1:
		case -1:
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_UNFORMAT:
		switch(Proc_Unformat(work)){
		case 1:
		case -1:
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_MKDIR:
		switch(Proc_Mkdir(work)){
		case 1:
		case -1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_RENAME:
		switch(Proc_Rename(work)){
		case 1:
		case -1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;

	case PROC_UPLOAD:
		switch(Proc_Upload(work)){
		case 1:
		case -1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_DOWNLOAD:
		switch(Proc_Download(work)){
		case 1:
			ProcStepset(work,PROC_WAIT);
			break;
		case -1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;

	case PROC_GETDIR2:
		switch(Proc_GetDir2(work)){
		case 1:
			ProcStepset(work,PROC_WAIT);
			break;
		case -1:
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_CHDIR2:
		switch(Proc_Chdir2(work)){
		case 2:
			ProcStepset(work,PROC_WAIT);
			break;
		case 1:
			ProcStepset(work,PROC_GETDIR2);
			break;
		case -1:
			ProcStepset(work,PROC_CHDIR2_ROOT);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_CHDIR2_ROOT:
		switch(Proc_Chdir2Root(work)){
		case 1:
			ProcStepset(work,PROC_GETDIR2);
			break;
		case -1:
			ProcStepset(work,PROC_WAIT);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_CHDIR3:
		switch(Proc_Chdir3(work)){
		case 2:
			ProcStepset(work,PROC_WAIT);
			break;
		case 1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -1:
			ProcStepset(work,PROC_CHDIR_ROOT);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
		break;
	case PROC_MKDUMMY:
		switch(Proc_MakeDummyFile(work)){
		case 1:
			ProcStepset(work,PROC_GETDIR);
			break;
		case -1:
			ProcStepset(work,PROC_CHDIR_ROOT);
			break;
		case -2:
			ProcStepset(work,PROC_MCCHECK);
			break;
		}
	}

	return 1;
}
