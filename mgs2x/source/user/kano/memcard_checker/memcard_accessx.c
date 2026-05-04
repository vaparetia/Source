//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	memcard_access.c
		メモリーカード制御サブルーチン
	
	2001/01/12 K.Kano
	$Id: memcard_accessx.c,v 1.1.1.3 2002/11/19 11:43:18 Yoshizawa1 Exp $
*/

#ifdef KP_XBOX //BP

#include "memcard_access.h"
#include "sceemux.h"


enum {
	MCAccess_WAIT=0,

	MCAccess_PRECHECK,
	MCAccess_CHECKSTART,
	MCAccess_CHECK,

	MCAccess_OPEN,
	MCAccess_CLOSE,
	MCAccess_SEEK,
	MCAccess_READ,
	MCAccess_WRITE,

	MCAccess_MKDIR,
	MCAccess_CHDIR,
	MCAccess_GETDIR,
	MCAccess_GETDIR_NEXT,

	MCAccess_RENAME,
	MCAccess_DELETE,
	MCAccess_CHMOD,

	MCAccess_FORMAT,
	MCAccess_UNFORMAT,

	MCAccess_Exit,
};

#define	EVENT_NAME "EVENT0"

static LONG	MCAccess_lRequest;

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


#define MCAccessStep(_work,_level)	((_work)->step[(_level)])

#define MCAccessStepClear(_work,_level) \
do{ \
	int _i; \
	for(_i=(_level);_i<STEP_LEVEL_MAX;_i++) (_work)->step[_i]=0; \
}while(0)


#ifdef DEBUG
// #define STEP_CHECK
#endif


#ifdef STEP_CHECK

#define MCAccessStepUp(_work,_level) \
do{ \
	printf("MCAccessStep Up   ( %d ) : %d -> %d\n", \
		   (_level),(_work)->step[(_level)],(_work)->step[(_level)]+1); \
	(_work)->step[(_level)]++; \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#define MCAccessStepDown(_work,_level) \
do{ \
	printf("MCAccessStep Down ( %d ) : %d -> %d\n", \
		   (_level),(_work)->step[(_level)],(_work)->step[(_level)]-1); \
	(_work)->step[(_level)]--; \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#define MCAccessStepSet(_work,_level,_val) \
do{ \
	printf("MCAccessStep Set  ( %d ) : %d -> %d\n", \
		   (_level),(_work)->step[(_level)],(_val)); \
	(_work)->step[(_level)]=(_val); \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#else

#define MCAccessStepUp(_work,_level) \
do{ \
	(_work)->step[(_level)]++; \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#define MCAccessStepDown(_work,_level) \
do{ \
	(_work)->step[(_level)]--; \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#define MCAccessStepSet(_work,_level,_val) \
do{ \
	(_work)->step[(_level)]=(_val); \
	MCAccessStepClear(_work,(_level)+1); \
}while(0)

#endif


#define MAIN_STEP			MCAccessStep(work,0)
#define MAIN_STEP_UP()		MCAccessStepUp(work,0)
#define MAIN_STEP_DOWN()	MCAccessStepDown(work,0)
#define MAIN_STEP_SET(val)	MCAccessStepSet(work,0,val)

#define SUB_STEP			MCAccessStep(work,1)
#define SUB_STEP_UP()		MCAccessStepUp(work,1)
#define SUB_STEP_DOWN()		MCAccessStepDown(work,1)
#define SUB_STEP_SET(val)	MCAccessStepSet(work,1,val)


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

/* 直前にロードしたゲームデータのID番号を記録する
   本来ならmcman/の方に書かれるべきであるが、常にメモリにあり続けるので、
   メモリの逼迫をさけるために低レベル関数側に書いた。*/
unsigned short mc_loadport;
unsigned short mc_loaddata_id;

/* ロードしたデータの日付 */
unsigned int mc_loaddata_time;

/* 秒単位の日付の計算用 */
const unsigned char conv_month_day[]={
	0,31,28,31,30,31,30,31,31,30,31,30,31,
};


#define DIFFERENT_CARD_0		(-0x200)
#define DIFFERENT_CARD_1		(-0x100)


/* ------------------------------------------------------------------------ */


unsigned int ComGetLoaddataTime(void)
{
#ifdef DEBUG_MODE
	printf("Get Load Data Date = %x\n",mc_loaddata_time);
#endif

	return mc_loaddata_time;
}

void	MC_SetLoaddataTime( FILETIME* pFt )
{	
	SYSTEMTIME	systime;
	FILETIME	ft2000;
	ULARGE_INTEGER ul2000;
	ULARGE_INTEGER ulcur;
	
	// UTC 2000/1/1 0:00:00 からの秒単位時間に変換する
	systime.wYear = 2000;
	systime.wMonth = 1;
	systime.wDay = 1;
	systime.wHour = 0;
	systime.wMinute = 0;
	systime.wSecond = 0;
	systime.wMilliseconds = 0;
	SystemTimeToFileTime( &systime, &ft2000);
	
	memcpy( &ul2000, &ft2000, sizeof(ul2000) );
	memcpy( &ulcur, pFt, sizeof(ulcur) );

	mc_loaddata_time = (int)( (ulcur.QuadPart - ul2000.QuadPart) / 10000000 );
	 
}

unsigned int ComGetNowTime(void)
{
	SYSTEMTIME	systime;
	FILETIME	ft2000;
	FILETIME	ftcur;	
	ULARGE_INTEGER ul2000;
	ULARGE_INTEGER ulcur;
	int dt;

	// UTC 2000/1/1 0:00:00 からの秒単位時間に変換する
	systime.wYear = 2000;
	systime.wMonth = 1;
	systime.wDay = 1;
	systime.wHour = 0;
	systime.wMinute = 0;
	systime.wSecond = 0;
	systime.wMilliseconds = 0;

	SystemTimeToFileTime( &systime, &ft2000);
	
	GetSystemTimeAsFileTime( &ftcur );
	
	memcpy( &ul2000, &ft2000, sizeof(ul2000) );
	memcpy( &ulcur, &ftcur, sizeof(ulcur) );

	dt = (int)( (ulcur.QuadPart - ul2000.QuadPart) / 10000000 );
	
	printf("current date %x\n", dt );
	
	return dt;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

#if defined( PAL ) || defined( SCENARIO_SET_SONYCODE )

/* シナリオ側でプロダクトコードを設定できるような仕掛け */
static char pfilename[0x30];
static char gfilename[0x30];
static char vfilename[0x30];
static char sfilename[0x30];

int ComSetProductCode(void)
{
	char *pcode=NULL;
	pcode=GCL_GetNextString();
	if(pcode==NULL){
		pfilename[0]='\0';
		gfilename[0]='\0';
		vfilename[0]='\0';
		sfilename[0]='\0';
		return 0;
	}
	sprintf(pfilename,_MC_PFILE_NAME,pcode);
	sprintf(gfilename,_MC_GFILE_NAME,pcode);
	sprintf(vfilename,_MC_VFILE_NAME,pcode);
	sprintf(sfilename,_MC_SFILE_NAME,pcode);

#ifdef DEBUG_MODE
	printf("pfilename = %s\n",pfilename);
	printf("gfilename = %s\n",gfilename);
	printf("vfilename = %s\n",vfilename);
	printf("sfilename = %s\n",sfilename);
#endif

	return 0;
}

char *GetPFilename(void)
{
	return pfilename;
}

char *GetGFilename(void)
{
	return gfilename;
}

char *GetVFilename(void)
{
	return vfilename;
}

char *GetSFilename(void)
{
	return sfilename;
}

#else

int ComSetProductCode(void)
{
	return 0;
}

char *GetPFilename(void)
{
	return MC_PFILE_NAME;
}

char *GetGFilename(void)
{
	return MC_GFILE_NAME;
}

char *GetVFilename(void)
{
	return MC_VFILE_NAME;
}

char *GetSFilename(void)
{
	return MC_SFILE_NAME;
}

#endif

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* ヘッダ */
typedef struct {
	int version;
} PS2ICON_FILEHEADER;

/* モデル */
typedef struct {
	int nbsp;
	int attrib;
	float bface;
	int nbvtx;
} PS2ICON_MDLHEADER;

typedef struct {
	short vtx[4];
} PS2ICON_MDLVERTEX;

typedef struct {
	short normal[4];
	short st[2];
	unsigned char color[4];
} PS2ICON_MDLVERTEXINFO;

/* アニメーション */
typedef struct {
	int nbseq;
} PS2ICON_ANIMSEQHEADER;

typedef struct {
	int nbframe;
	float speed;
	int offset;
	int nbksp;
} PS2ICON_ANIMSEQ;

typedef struct {
	int kspid;
	int nbkf;
} PS2ICON_ANIMSHAPE;

typedef struct {
	float frame;
	float weight;
} PS2ICON_ANIMFRAME;


/* PS2のアイコンファイルのサイズを計算して返す */
int MCAccessCalcIconSize(void *icon)
{
	int size=0;
	int i,j;

	PS2ICON_FILEHEADER *fileheader;
	PS2ICON_MDLHEADER *mdlheader;
	PS2ICON_MDLVERTEX *mdlvertex;
	PS2ICON_ANIMSEQHEADER *animseqheader;
	PS2ICON_ANIMSEQ *animseq;
	PS2ICON_ANIMSHAPE *animshape;
	PS2ICON_ANIMFRAME *animframe;

	fileheader=(PS2ICON_FILEHEADER *)icon;
	size+=sizeof(PS2ICON_FILEHEADER);

	mdlheader=(PS2ICON_MDLHEADER *)((int)icon+size);
	size+=sizeof(PS2ICON_MDLHEADER);

	mdlvertex=(PS2ICON_MDLVERTEX *)((int)icon+size);
	size+=(sizeof(PS2ICON_MDLVERTEX)*mdlheader->nbsp+
		   sizeof(PS2ICON_MDLVERTEXINFO))*mdlheader->nbvtx;

	animseqheader=(PS2ICON_ANIMSEQHEADER *)((int)icon+size);
	size+=sizeof(PS2ICON_ANIMSEQHEADER);

	for(i=0;i<animseqheader->nbseq;i++){
		animseq=(PS2ICON_ANIMSEQ *)((int)icon+size);
		size+=sizeof(PS2ICON_ANIMSEQ);

		for(j=0;j<animseq->nbksp;j++){
			animshape=(PS2ICON_ANIMSHAPE *)((int)icon+size);
			size+=sizeof(PS2ICON_ANIMSHAPE);

			animframe=(PS2ICON_ANIMFRAME *)((int)icon+size);
			size+=sizeof(PS2ICON_ANIMFRAME)*animshape->nbkf;
		}
	}

	if(mdlheader->attrib & 0x08){
		int *texsize=(int *)((int)icon+size);
		size+=*texsize+sizeof(int);
	}
	else{
		size+=0x8000;
	}

#ifdef DEBUG
	printf("Icon size = %d\n",size);
#endif

	return size;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void PortSlotNumUpdate(MCAccessWork *work)
{
	work->SelSlot++;
	if(work->SelSlot>=work->SlotMax[work->SelPort]){
		work->SelSlot=0;

		work->SelPort++;
		if(work->SelPort>=PORT_MAX) work->SelPort=0;
	}
}

static int RetryCountup(MCAccessWork *work)
{
	work->RetryCount++;
	if(work->RetryCount>=RETRY_COUNT_MAX) return 1;
	return 0;
}

int MCAccessPrecheckStart(MCAccessWork *work)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	work->AutoPortCheck_Counter=PORTCHECK_COUNT_MAX;
	if(sceMcGetInfo(work->SelPort,work->SelSlot,&(work->TmpType),NULL,NULL)!=0) return 0;

	MAIN_STEP_SET(MCAccess_PRECHECK);

	return 1;
}

static int MCAccessPrecheck(MCAccessWork *work)
{
	int cmd,result;

	switch(sceMcSync(1,&cmd,&result)){
	case 0:
		break;
	case 1:
		if(result==0){
			return 1;
		}
		else if(result<-10 && work->Type[work->SelPort][work->SelSlot]==work->TmpType){
			return 1;
		}
		else{
			work->Type[work->SelPort][work->SelSlot]=DIFFERENT_CARD_0;
			return -1;
		}
		break;
	case -1:
		work->Type[work->SelPort][work->SelSlot]=DIFFERENT_CARD_0;
		return -1;
	}

	return 0;
}

static int MCAccessPrecheck2Check(MCAccessWork *work)
{
	int tmp_max;
	int i,j;

	tmp_max=sceMcGetSlotMax(0);
	if(tmp_max>0) work->SlotMax[0]=tmp_max;
	else return 0;

	tmp_max=sceMcGetSlotMax(1);
	if(tmp_max>0) work->SlotMax[1]=tmp_max;
	else return 0;

	MAIN_STEP_SET(MCAccess_CHECK);
	work->RetryCount=0;
	work->Result=0;

	for(i=0;i<PORT_MAX;i++){
		for(j=0;j<SLOT_MAX;j++){
			work->OldType[i][j]=work->Type[i][j];
		}
	}

#ifdef DEBUG
	printf("Portcheck Start\n");
#endif

	return 1;
}

int MCAccessCheckStart(MCAccessWork *work)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;
	return MCAccessPrecheck2Check(work);
}

static int MCAccessCheck(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0x00: /* port=0 , slot=0 */
	case 0x10: /* port=0 , slot=1 */
	case 0x20: /* port=0 , slot=2 */
	case 0x30: /* port=0 , slot=3 */

	case 0x40: /* port=1 , slot=0 */
	case 0x50: /* port=1 , slot=1 */
	case 0x60: /* port=1 , slot=2 */
	case 0x70: /* port=1 , slot=3 */
#ifdef KP_XBOX
	case 0x80:	/* port=2 , slot=0 終了条件時に通る 2002/03/20 ADD M.Kobayashi */
#endif		
		{
			int port,slot;
			int slot_max;

			port=(SUB_STEP>>4)>>2;
			slot=(SUB_STEP>>4) & 3;
			slot_max=work->SlotMax[port];

			if(port>=PORT_MAX){
				return 1;
			}
			if(slot>=slot_max){
				if(port){
					return 1;
				}
				else{
					SUB_STEP_SET(0x40);
					break;
				}
			}

#ifdef DEBUG_MODE
			// printf("Port Slot = %d %d\n",port,slot);
#endif

			if(sceMcGetInfo(port,slot,&(work->Type[port][slot]),
							&(work->FreeSize[port][slot]),&(work->Formatted[port][slot]))==0){

				/* Success */
				SUB_STEP_UP();
			}
			else{
				/* Failed */
				if(RetryCountup(work)){
					SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
					work->Type[port][slot]=-1;
					if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
					printf("Portcheck Failed1 %d %d\n",port,slot);
#endif

				}
			}
		}
		break;

	case 0x01: /* port=0 , slot=0 */
	case 0x11: /* port=0 , slot=1 */
	case 0x21: /* port=0 , slot=2 */
	case 0x31: /* port=0 , slot=3 */

	case 0x41: /* port=1 , slot=0 */
	case 0x51: /* port=1 , slot=1 */
	case 0x61: /* port=1 , slot=2 */
	case 0x71: /* port=1 , slot=3 */
		{
			int cmd,result;
			int port,slot;

			port=(SUB_STEP>>4)>>2;
			slot=(SUB_STEP>>4) & 3;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case 0:
					/* Success  case 1*/
					SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
					work->RetryCount=0;
					if(work->OldType[port][slot]!=DIFFERENT_CARD_0){
						work->OldType[port][slot]=work->Type[port][slot];
					}
					break;
				case -1:
				case -2:
					/* Success  case 2*/
					SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
					work->RetryCount=0;
					work->OldType[port][slot]=DIFFERENT_CARD_1;
					if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
					printf("Portcheck Failed2 %d %d\n",port,slot);
#endif
					break;
				default:
					if(result<-10){
						switch(work->Type[port][slot]){
						case 2:
							/* Retry */
							if(RetryCountup(work)){
								SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
								work->Type[port][slot]=-1;
								work->RetryCount=0;
								if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
								printf("Portcheck Failed3 %d %d\n",port,slot);
#endif
							}
							else SUB_STEP_DOWN();
							break;
						case 0:
							/* Retry */
							if(RetryCountup(work)){
								SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
								work->RetryCount=0;
								if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
								printf("Portcheck Failed4 %d %d\n",port,slot);
#endif
							}
							else SUB_STEP_DOWN();
							break;
						default:
							SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
							work->RetryCount=0;
							break;
						}
					}
					else{
						/* Retry */
						if(RetryCountup(work)){
							SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
							work->Type[port][slot]=-1;
							work->RetryCount=0;
							if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
							printf("Portcheck Failed5 %d %d\n",port,slot);
#endif
						}
						else SUB_STEP_DOWN();
					}
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)){
					SUB_STEP_SET((SUB_STEP & 0xf0)+0x10);
					work->Type[port][slot]=-1;
					work->RetryCount=0;
					if(GetLoaddataPort()==port) ResetLoaddataID();

#ifdef DEBUG_MODE
					printf("Portcheck Failed6 %d %d\n",port,slot);
#endif
				}
				else SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessOpenStart(MCAccessWork *work,int port,int slot,char *name,int mode,int *fd)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_OPEN);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.open.name=name;
	work->arg.open.mode=mode;
	work->arg.open.fd=fd;

	return 1;
}

static int MCAccessOpen(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcOpen(work->SelPort,work->SelSlot,work->arg.open.name,work->arg.open.mode)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -3:
				case -4:
				case -5:
				case -7:
					return -1;
				default:
					if(result>=0){
						work->fd=*(work->arg.open.fd)=result;
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessCloseStart(MCAccessWork *work,int fd)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_CLOSE);
	work->RetryCount=0;
	work->Result=0;

	work->arg.close.fd=fd;

	return 1;
}

static int MCAccessClose(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcClose(work->arg.close.fd)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					work->fd=-1;
					return -2;
				case -4:
					work->fd=-1;
					return -1;
				default:
					if(result>=0){
						work->fd=-1;
						return 1;
					}
					else{
						work->fd=-1;
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessSeekStart(MCAccessWork *work,int fd,int offset,int mode,int *fp)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_SEEK);
	work->RetryCount=0;
	work->Result=0;

	work->arg.seek.fd=fd;
	work->arg.seek.offset=offset;
	work->arg.seek.mode=mode;
	work->arg.seek.fp=fp;

	return 1;
}

static int MCAccessSeek(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcSeek(work->arg.seek.fd,work->arg.seek.offset,work->arg.seek.mode)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					work->fd=-1;
					return -2;
				case -4:
					work->fd=-1;
					return -1;
				default:
					if(result>=0){
						if(work->arg.seek.fp!=NULL){
							*(work->arg.seek.fp)=result;
						}
						return 1;
					}
					else{
						work->fd=-1;
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessReadStart(MCAccessWork *work,int fd,void *buf,int size,int *retsize)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_READ);
	work->RetryCount=0;
	work->Result=0;

	work->arg.rw.fd=fd;
	work->arg.rw.buf=buf;
	work->arg.rw.size=size;
	work->arg.rw.retsize=retsize;

	return 1;
}

static int MCAccessRead(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcRead(work->arg.rw.fd,work->arg.rw.buf,work->arg.rw.size)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					work->fd=-1;
					return -2;
				case -4:
					work->fd=-1;
				case -3:
				case -5:
					return -1;
				default:
					if(result>=0){
						if(work->arg.rw.retsize) *(work->arg.rw.retsize)=result;
						return 1;
					}
					else{
						work->fd=0;
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessWriteStart(MCAccessWork *work,int fd,void *buf,int size,int *retsize)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_WRITE);
	work->RetryCount=0;
	work->Result=0;

	work->arg.rw.fd=fd;
	work->arg.rw.buf=buf;
	work->arg.rw.size=size;
	work->arg.rw.retsize=retsize;

	return 1;
}

static int MCAccessWrite(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcWrite(work->arg.rw.fd,work->arg.rw.buf,work->arg.rw.size)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					work->fd=-1;
					return -2;
				case -4:
					work->fd=-1;
				case -3:
				case -5:
				case -8:
					return -1;
				default:
					if(result>=0){
						if(work->arg.rw.retsize) *(work->arg.rw.retsize)=result;
						return 1;
					}
					else{
						work->fd=-1;
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessMkdirStart(MCAccessWork *work,int port,int slot,char *name)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_MKDIR);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.mkdir.name=name;

	return 1;
}

static int MCAccessMkdir(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcMkdir(work->SelPort,work->SelSlot,work->arg.mkdir.name)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -3:
				case -4:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessChdirStart(MCAccessWork *work,int port,int slot,char *name,char *pwd)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_CHDIR);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.chdir.name=name;
	work->arg.chdir.pwd=pwd;

	return 1;
}

static int MCAccessChdir(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcChdir(work->SelPort,work->SelSlot,
					  work->arg.chdir.name,work->arg.chdir.pwd)==0){

			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -3:
				case -4:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

static int MCAccessGetdir0Start(MCAccessWork *work,int port,int slot,
								char *name,sceMcTblGetDir *table,int table_size,int *ret_size)
{
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.getdir.name=name;
	work->arg.getdir.table=table;
	work->arg.getdir.table_size=table_size;
	work->arg.getdir.ret_table_size=ret_size;

	return 1;
}

int MCAccessGetdirStart(MCAccessWork *work,int port,int slot,
						char *name,sceMcTblGetDir *table,int table_size,int *ret_size)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_GETDIR);

	return MCAccessGetdir0Start(work,port,slot,name,table,table_size,ret_size);
}

int MCAccessGetdirNextStart(MCAccessWork *work,int port,int slot,
							char *name,sceMcTblGetDir *table,int table_size,int *ret_size)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_GETDIR_NEXT);

	return MCAccessGetdir0Start(work,port,slot,name,table,table_size,ret_size);
}

static int MCAccessGetdir0(MCAccessWork *work,int flag)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcGetDir(work->SelPort,work->SelSlot,work->arg.getdir.name,flag,
					   work->arg.getdir.table_size,work->arg.getdir.table)==0){

			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -4:
					return -1;
				default:
					if(result>=0){
						*(work->arg.getdir.ret_table_size)=result;
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessDeleteStart(MCAccessWork *work,int port,int slot,char *name)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_DELETE);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.delete.name=name;

	return 1;
}

static int MCAccessDelete(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcDelete(work->SelPort,work->SelSlot,work->arg.delete.name)==0){
			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -4:
				case -5:
				case -6:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessRenameStart(MCAccessWork *work,int port,int slot,char *org,char *next)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_RENAME);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.rename.org=org;
	work->arg.rename.next=next;

	return 1;
}

static int MCAccessRename(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcRename(work->SelPort,work->SelSlot,
					   work->arg.rename.org,work->arg.rename.next)==0){

			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -4:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessChmodStart(MCAccessWork *work,int port,int slot,
					   char *name,sceMcTblGetDir *table,unsigned int valid)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_CHMOD);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	work->arg.chmod.name=name;
	work->arg.chmod.table=table;
	work->arg.chmod.valid=valid;

	return 1;
}

static int MCAccessChmod(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
        if(sceMcSetFileInfo(work->SelPort,work->SelSlot,
							work->arg.chmod.name,
							(unsigned char *)(work->arg.chmod.table),
							work->arg.chmod.valid)==0){

			SUB_STEP_UP();
		}
		else{
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				switch(result){
				case -2:
					/* Unformat */
					return -2;
				case -4:
					return -1;
				default:
					if(result>=0){
						return 1;
					}
					else{
						return -2;
					}
					break;
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessFormatStart(MCAccessWork *work,int port,int slot)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_FORMAT);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	return 1;
}

static int MCAccessFormat(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
		if(sceMcFormat(work->SelPort,work->SelSlot)==0){
			/* Success */
			SUB_STEP_UP();
		}
		else{
			/* Failed */
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				if(result==0){
					/* Success */
					return 1;
				}
				else{
					/* Retry */
					if(RetryCountup(work)) return -1;
					SUB_STEP_DOWN();
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}

int MCAccessUnformatStart(MCAccessWork *work,int port,int slot)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;

	MAIN_STEP_SET(MCAccess_UNFORMAT);
	work->RetryCount=0;
	work->Result=0;

	work->SelPort=port;
	work->SelSlot=slot;

	return 1;
}

static int MCAccessUnformat(MCAccessWork *work)
{
	switch(SUB_STEP){
	case 0:
		if(sceMcUnformat(work->SelPort,work->SelSlot)==0){
			/* Success */
			SUB_STEP_UP();
		}
		else{
			/* Failed */
			if(RetryCountup(work)) return -1;
		}
		break;
	case 1:
		{
			int cmd,result;

			switch(sceMcSync(1,&cmd,&result)){
			case 0:
				break;
			case 1:
				if(result==0){
					/* Success */
					return 1;
				}
				else{
					/* Retry */
					if(RetryCountup(work)) return -1;
					SUB_STEP_DOWN();
				}
				break;
			case -1:
				/* Retry */
				if(RetryCountup(work)) return -1;
				SUB_STEP_DOWN();
				break;
			}
		}
		break;
	}
	return 0;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


int MCAccessing(MCAccessWork *work)
{
	return MAIN_STEP!=MCAccess_WAIT && MAIN_STEP!=MCAccess_PRECHECK;
}

int MCAccessGetResult(MCAccessWork *work)
{
	if(MAIN_STEP!=MCAccess_WAIT) return 0;
	return work->Result;
}

int MCAccessGetType(MCAccessWork *work,int port,int slot)
{
	return work->Type[port][slot];
}

int MCAccessIsMCChanged(MCAccessWork *work,int port,int slot)
{
	return work->Type[port][slot]!=work->OldType[port][slot];
}

int MCAccessGetFreeSize(MCAccessWork *work,int port,int slot)
{
	return work->FreeSize[port][slot]*1024;
}

int MCAccessIsFormatted(MCAccessWork *work,int port,int slot)
{
	return work->Formatted[port][slot];
}

int MCAccessGetSlotMax(MCAccessWork *work,int port)
{
	return work->SlotMax[port];
}

void MCAccessAutoCheck(MCAccessWork *work,int flag)
{
	if(flag){
		if(work->AutoPortCheck_Counter==0){
			work->AutoPortCheck_Counter=PORTCHECK_COUNT_MAX;
		}
	}
	else{
		work->AutoPortCheck_Counter=0;
	}
}

int MCAccessCheckSeqFlag(MCAccessWork *work)
{
	if(work->CheckSeqFlag==2){
		work->CheckSeqFlag=0;
		return 1;
	}
	return 0;
}

int MCAccessAutoChecking(MCAccessWork *work)
{
	if(work->CheckSeqFlag==1) return 1;
	return 0;
}

#if 0

static int Dummy(void)
{
	return 1;
}

#endif

void MCAccessInit(MCAccessWork *work,int flag)
{
	int i;

	if(flag){
		while(sceMcInit()!=0);
		// GV_SetLoader('i',Dummy);
		ResetLoaddataID();
	}

	if(work!=NULL){
		for(i=0;i<STEP_LEVEL_MAX;i++) work->step[i]=0;
		work->SelPort=work->SelSlot=0;
		work->AutoPortCheck_Counter=0;
		work->CheckSeqFlag=0;
		work->fd=-1;

		for(i=0;i<PORT_MAX;i++){
			int j;
			// for(j=0;j<SLOT_MAX;j++) work->Type[i][j]=work->OldType[i][j]=DIFFERENT_CARD_1;
			for(j=0;j<SLOT_MAX;j++) work->Type[i][j]=work->OldType[i][j]=0;
		}

		// MAIN_STEP_SET(MCAccess_CHECKSTART);

		{
			static DWORD WINAPI MCAccessThread(void *param);
			HANDLE	h;

			// イベント準備
#if 0			
			h = CreateEvent( NULL, FALSE, FALSE, EVENT_NAME );
			ASSERT( h != NULL );
			DuplicateHandle( NULL, h, NULL, &work->hEvent, 0, FALSE, DUPLICATE_CLOSE_SOURCE );

			// XBOX スレッド準備
			h = BP_CreateThread( NULL,	0, MCAccessThread, (void*)work, 0, NULL );
			ASSERT( h != NULL );
			DuplicateHandle( NULL, h, NULL, &work->hThread, 0, FALSE, DUPLICATE_CLOSE_SOURCE );
#endif			
			work->hEvent = CreateEvent( NULL, FALSE, FALSE, EVENT_NAME );
			ASSERT( work->hEvent != NULL );

			// XBOX スレッド準備
			work->hThread = BP_CreateThread( NULL,	0, MCAccessThread, (void*)work, 0, NULL );
			ASSERT( work->hThread != NULL );
		}
	}

}

void MCAccessEnd(MCAccessWork *work)
{
	DWORD dwExitCode;
	sceMcSync(0,NULL,NULL);
	if(work->fd>=0){
		while(sceMcClose(work->fd)==0);
		sceMcSync(0,NULL,NULL);
	}
	// 終了メッセージを送る
	InterlockedExchange( &MCAccess_lRequest, MCAccess_Exit );
	if( !SetEvent( work->hEvent ) ) {
		printf("MCAccessEnd: err %d\n", GetLastError() );
	}

	while( !GetExitCodeThread( work->hThread, &dwExitCode ) ){
		printf("MCAccessEnd: err %d\n", GetLastError() );
	}
}

void MCAccessAct(MCAccessWork *work)
{
	// printf("%d\n",MAIN_STEP);
	if( GV_PadData[0].press & PAD_L1 ) {
		InterlockedExchange( &MCAccess_lRequest, MCAccess_PRECHECK );
		SetEvent( work->hEvent );
	}

	switch(MAIN_STEP){
	case MCAccess_WAIT:
		/* No Operation */
		break;

	case MCAccess_PRECHECK:
		switch(MCAccessPrecheck(work)){
		case 1:
			MAIN_STEP_SET(MCAccess_WAIT);
			PortSlotNumUpdate(work);
			break;
		case -1:
			MAIN_STEP_SET(MCAccess_CHECKSTART);
			break;
		}
		break;
	case MCAccess_CHECKSTART:
		MCAccessPrecheck2Check(work);
		work->CheckSeqFlag=1;
		break;
	case MCAccess_CHECK:
		if(MCAccessCheck(work)!=0){
			MAIN_STEP_SET(MCAccess_WAIT);
			work->SelPort=work->SelSlot=0;
			if(work->AutoPortCheck_Counter){
				work->AutoPortCheck_Counter=PORTCHECK_COUNT_MAX;
			}
			work->Result=1;
			if(work->CheckSeqFlag) work->CheckSeqFlag=2;
		}
		break;

	case MCAccess_OPEN:
		{
			int ans=MCAccessOpen(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_CLOSE:
		{
			int ans=MCAccessClose(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;

	case MCAccess_SEEK:
		{
			int ans=MCAccessSeek(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_READ:
		{
			int ans=MCAccessRead(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_WRITE:
		{
			int ans=MCAccessWrite(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;

	case MCAccess_MKDIR:
		{
			int ans=MCAccessMkdir(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_CHDIR:
		{
			int ans=MCAccessChdir(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_GETDIR:
		{
			int ans=MCAccessGetdir0(work,0);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_GETDIR_NEXT:
		{
			int ans=MCAccessGetdir0(work,1);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;

	case MCAccess_RENAME:
		{
			int ans=MCAccessRename(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_DELETE:
		{
			int ans=MCAccessDelete(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_CHMOD:
		{
			int ans=MCAccessChmod(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;

	case MCAccess_FORMAT:
		{
			int ans=MCAccessFormat(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	case MCAccess_UNFORMAT:
		{
			int ans=MCAccessUnformat(work);

			if(ans!=0){
				MAIN_STEP_SET(MCAccess_WAIT);
				work->Result=ans;
			}
		}
		break;
	}
}

void MCAccessPostAct(MCAccessWork *work)
{
	if(MAIN_STEP==MCAccess_WAIT){
		if(work->AutoPortCheck_Counter){
			work->AutoPortCheck_Counter--;
			if(work->AutoPortCheck_Counter==0) MCAccessPrecheckStart(work);
		}
	}
}


////////////////////////////////////////
// メモリカード用スレッド
static DWORD WINAPI MCAccessThread(MCAccessWork* pw)
{
//	HANDLE hEvent = OpenEvent( 0, FALSE, EVENT_NAME );
	
	while( 1 ) {
		LONG	lRequest;
		DWORD	dwResult;
		dwResult = WaitForSingleObject( pw->hEvent, INFINITE );
		if( dwResult != WAIT_OBJECT_0 ) {
			printf("MCAccessThread: err %d\n", dwResult = GetLastError() );
		}
		lRequest = MCAccess_lRequest;
		InterlockedExchange( &MCAccess_lRequest, -1 );

		switch( lRequest ) {
		case MCAccess_Exit:
			printf("exit thread\n");
			BP_ExitThread( 0 );
			break;
		default:
			printf("thread\n");
			break;
		}
	}

	return 0;
}

#endif