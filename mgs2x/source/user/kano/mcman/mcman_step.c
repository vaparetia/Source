//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mcman_step.c
		メモリーカード制御ステップ処理部
	
	2001/04/16 K.Kano
	$Id: mcman_step.c,v 1.1.1.3 2002/11/19 11:43:17 Yoshizawa1 Exp $
*/


#include "mcman.h"
#include <time.h>

#ifdef BP_360
#include <Xtl.h>
#endif

#ifdef PSX2
#include <libcdvd.h>
#include <libscf.h>
#endif

#include "BP_BuildDefines.h"
#include "BP_Font.h"
#include "BP_Misc.h"
#include "BP_TrophyLogicMGS2.h"
#include "BP_SaveLoadMGS.h"
#include "BP_Transfarring.h"

#include "mcman_shared.h"

#if BP_VITA
#define SCE_RTC_USE_LIBC_TIME_H 1

#include "scetypes.h"
#include "rtc.h"
#endif

int BP_FramesSysToFrames60( int frames );
int BP_Frames60ToFramesSys( int frames60 );

extern void DecodeVRInfo( MCMAN_VR_INFODATA* pDst, const MCMAN_INFODATA* pSrc );
extern void EncodeVRInfo( MCMAN_INFODATA* pDst, const MCMAN_VR_INFODATA* pSrc );

//cell API header does not include cleanly from this file!
#define CELL_SAVEDATA_BINDSTAT_ERR_NOOWNER		(1<<8)	/*J 所有者が不明のセーブデータ */
#define CELL_SAVEDATA_BINDSTAT_ERR_OWNER		(1<<9)	/*J 他の所有者のセーブデータ */

static sceMcTblGetDir dirchk_table[4];
static int dirchk_table_used_size;
static char s_dirname[0x80];

#define DIRCHK_TABLE_SIZE		(sizeof(dirchk_table)/sizeof(dirchk_table[0]))

#define LINKINFO_SIZE			4


#define TABLE_SIZE		(sizeof(work->table)/sizeof(work->table[0]))
// #define TABLE_SIZE		16

//#define DEBUG_MODE 1

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */

void BP_MCManSetDateTime(unsigned char date_time[], time_t _dateTime )
{
#if defined(BP_360)
   SYSTEMTIME systemTime;
   FileTimeToSystemTime((FILETIME *)&_dateTime, &systemTime);
   date_time[DATETIME_INDEX_YEAR] = systemTime.wYear-2000;
   date_time[DATETIME_INDEX_MONTH]= systemTime.wMonth;
   date_time[DATETIME_INDEX_DAY]  = systemTime.wDay;
   date_time[DATETIME_INDEX_HOUR] = 22;    // unused;
   date_time[DATETIME_INDEX_MIN]  = 22;    // unused;
   date_time[DATETIME_INDEX_SEC]  = 22;    // unused;
#elif BP_VITA
   SceDateTime sceDateTime, sceLocalDateTime;
   SceRtcTick sceSystemTime, sceLocalTime;

   sceRtcSetTime_t(&sceDateTime, _dateTime);
   sceRtcGetTick(&sceDateTime, &sceSystemTime);
   sceRtcConvertUtcToLocalTime(&sceSystemTime, &sceLocalTime);
   sceRtcSetTick(&sceLocalDateTime, &sceLocalTime);

   date_time[DATETIME_INDEX_YEAR] = sceLocalDateTime.year - 2000;
   date_time[DATETIME_INDEX_MONTH]= sceLocalDateTime.month;
   date_time[DATETIME_INDEX_DAY]  = sceLocalDateTime.day;
   date_time[DATETIME_INDEX_HOUR] = 22;    // unused;
   date_time[DATETIME_INDEX_MIN]  = 22;    // unused;
   date_time[DATETIME_INDEX_SEC]  = 22;    // unused;

#else
   time_t _time;
   struct tm *_timeinfo;
   //   _index = (mcman_allocated_work->n_files-1)-_index;
   _time = _dateTime;
   _timeinfo = localtime(&_time);
   date_time[DATETIME_INDEX_YEAR] = _timeinfo->tm_year-100;
   date_time[DATETIME_INDEX_MONTH]= _timeinfo->tm_mon+1;
   date_time[DATETIME_INDEX_DAY]  = _timeinfo->tm_mday;
   date_time[DATETIME_INDEX_HOUR] = 22;    // unused;
   date_time[DATETIME_INDEX_MIN]  = 22;    // unused;
   date_time[DATETIME_INDEX_SEC]  = 22;    // unused;
#endif
}

/* 32 bit CRC 計算 */

#define CRCPOLY 0xEDB88320

extern char *GetOtherFilename( void );

static void calc_crc_code(void *data,int len,unsigned char *crc)
{
	unsigned int i, j;
	unsigned int r;
	unsigned char *p;

	r = 0xFFFFFFFF;
	p = data;

	for( i = len; i > 0; i-- ){
		r ^= *( p ++ );
		for( j = 8; j > 0; j-- ){
			if( r & 1 ){
				r = ( r >> 1 ) ^ CRCPOLY;
			} else {
				r >>= 1;
			}
		}
	}
	r^=0xFFFFFFFF;

	/* ビッグエンディアン方式で格納
	   意味は特に無いが、PS2はリトルエンディアンを用いているので、
	   隠す意味では違った方式を採った方が良い */
	*(crc+0)=(r>>24) & 0xff;
	*(crc+1)=(r>>16) & 0xff;
	*(crc+2)=(r>>8) & 0xff;
	*(crc+3)=(r>>0) & 0xff;
}

/* 文字列と思わせて、暗号化のための表に使う */
static const unsigned char codetable[]={
	"2"
	"Metal"
	"Of"
	"Solid"
	"Sons"
	"Liberty"
	"Gear"
};

#define CODETABLE_SIZE		(sizeof(codetable)/sizeof(codetable[0])-1)

/* 暗号化
   古典的ではあるが、コード化が速く、コード表さえ発見されなければ、
   解読しづらい。*/
static void CodeData(void *data,int size)
{
	unsigned char *s=(unsigned char *)data;
	int i,j;
	unsigned char pre_s;
	// unsigned char cur_s;

	pre_s=0;
	for(i=size,j=0;i>0;i--){
		// cur_s=*s;
		*s+=(pre_s+codetable[j]);
		// pre_s=cur_s;
		pre_s=*s;
		s++,j++;
		if(j>=CODETABLE_SIZE) j=0;
	}
}

/* 暗号の解除 */
static void DecodeData(void *data,int size)
{
	unsigned char *s=(unsigned char *)data;
	int i,j;
	unsigned char pre_s,pre_s2;

	pre_s=0;
	for(i=size,j=0;i>0;i--){
		pre_s2=*s;
		*s-=(pre_s+codetable[j]);
		// pre_s=*s;
		pre_s=pre_s2;
		s++,j++;
		if(j>=CODETABLE_SIZE) j=0;
	}
}

static void MakePhotoDirName(MCMAN_WORK *work,char *name,int id)
{
	char format[0x80];
	strcpy(format,MC_PFILE_NAME);
	strcat(format,"%03d");
	sprintf(name,format,id);
	// sprintf(name,MC_PFILE_NAME "%03d",id);
#ifdef DEBUG_MODE
	printf("MCMAN : Dirname = %s\n",name);
#endif
}

static void MakeGameDirName(MCMAN_WORK *work,char *name,int id)
{
   char format[0x80] = { 0 };
	switch ( work->file_kind ) {
	case MCMAN_FILE_KIND_GAME:
		strcpy(format,MC_GFILE_NAME);
		break;
	case MCMAN_FILE_KIND_VR:
		strcpy(format,MC_VFILE_NAME);
		break;
	case MCMAN_FILE_KIND_SNAKE_TALES:
		strcpy(format,MC_SFILE_NAME);
		break;
#ifdef PSX2
	case MCMAN_FILE_KIND_FIRSTCHECK:
		strcpy(format,MC_GFILE_NAME);	// ここではゲームファイル名に
		break;
	case MCMAN_FILE_KIND_OTHER:
		strcpy(format, GetOtherFilename() );
		break;
#endif
	}
	strcat(format,"%03d");
	sprintf(name,format,id);
	// sprintf(name,MC_GFILE_NAME "%03d",id);
#ifdef DEBUG_MODE
	printf("MCMAN : Dirname = %s\n",name);
#endif
}

/* Base64の様に8bit->6bitの可読文字に変換 */
static const unsigned char scode[]={
	"ghijklmn" "01234567" "opqrstuv" "EFGHIJKL" "89abcdef" "UVWXYZ_." "wxyzABCD" "MNOPQRST"
};

static void MakeDataName(MCMAN_WORK *work,char *name,const void *info,unsigned char *crc)
{
	unsigned char *binfo=(unsigned char *)info;
	unsigned char cinfo[INFOCODE2_SIZE+2];
	unsigned char dinfo[CHARCODE_SIZE];
	int checksum;
	int i,j,k;

	/* チェックサムの計算 */
	checksum=0;
	for(i=0;i<INFOCODE_SIZE;i++){
		checksum^=binfo[i];
		cinfo[i]=binfo[i];
	}
	for( ;i<INFOCODE2_SIZE;i++){
		checksum^=crc[i-INFOCODE_SIZE];
		cinfo[i]=crc[i-INFOCODE_SIZE];
	}
	cinfo[i]=checksum;
	cinfo[i+1]=0;

	/* 6bit毎に切り分ける */
	j=0; k=0;
	for(i=0;i<CHARCODE_SIZE;i++){
		switch(k){
		case 0:
			dinfo[i]=(cinfo[j]>>2) & 0x3f;
			break;
		case 1:
			dinfo[i]=(cinfo[j]>>1) & 0x3f;
			break;
		case 2:
			dinfo[i]=cinfo[j] & 0x3f;
			break;
		case 3:
			dinfo[i]=((cinfo[j] & 0x1f)<<1)|((cinfo[j+1]>>7) & 0x01);
			break;
		case 4:
			dinfo[i]=((cinfo[j] & 0x0f)<<2)|((cinfo[j+1]>>6) & 0x03);
			break;
		case 5:
			dinfo[i]=((cinfo[j] & 0x07)<<3)|((cinfo[j+1]>>5) & 0x07);
			break;
		case 6:
			dinfo[i]=((cinfo[j] & 0x03)<<4)|((cinfo[j+1]>>4) & 0x0f);
			break;
		case 7:
			dinfo[i]=((cinfo[j] & 0x01)<<5)|((cinfo[j+1]>>3) & 0x1f);
			break;
		}

		k+=6;
		if(k>=8){
			k-=8;
			j++;
		}
	}

	/* コード化 */
	name[0]='D';
	for(i=0;i<CHARCODE_SIZE;i++){
		int c=(dinfo[i]+i*7) & 0x3f;
		name[i+1]=scode[c];
	}
	name[i+1]='\0';
}

static void MakeDataName_Kind(MCMAN_WORK *work,char *name,const void *info,unsigned char *crc, int file_kind)
{
   //Only VR and Game saves display play time in file listings.  Convert to 60ths of a second.
   MCMAN_INFODATA * infodata = (MCMAN_INFODATA*)(info);

   switch ( file_kind )
   {
   case MCMAN_FILE_KIND_GAME:
      {
         int frames60 = BP_FramesSysToFrames60( infodata->mgs2_playtime );
         printf("MakeDataName_Kind: convert saving game time %d -> %d\n", infodata->mgs2_playtime, frames60 );
         infodata->mgs2_playtime = frames60;
      }
      break;
   case MCMAN_FILE_KIND_VR:
      {
         int frames60;
         MCMAN_VR_INFODATA vrinfo = { { 0 } };
         DecodeVRInfo( &vrinfo, infodata );
         frames60 = BP_FramesSysToFrames60( vrinfo.vr_playtime );
         printf("MakeDataName_Kind: convert saving vr time %d -> %d\n", vrinfo.vr_playtime, frames60 );
         vrinfo.vr_playtime = frames60;
         EncodeVRInfo( infodata, &vrinfo );
      }
      break;
   }

   MakeDataName( work, name, info, crc );
}

static int DecodeDataName(MCMAN_WORK *work,const char *name,void *info)
{
	unsigned char *binfo=(unsigned char *)info;
	unsigned char cinfo[INFOCODE2_SIZE+1];
	unsigned char dinfo[CHARCODE_SIZE];
	unsigned char code[0x80];
	int checksum;
	int i,j,k;

	/* コード表作成 */
	for(i=0;i<0x80;i++) code[i]=0xff;
	for(i=0;i<0x40;i++){
		code[scode[i]]=i;
	}

	/* デコード */
	for(i=0;i<CHARCODE_SIZE;i++){
		dinfo[i]=code[(unsigned char)(name[i+1])];
		if(dinfo[i]==0xff) return 0;
		dinfo[i]=(dinfo[i]-i*7) & 0x3f;
	}

	/* 6bit->8bitコードに直す */
	j=0; k=0;
	for(i=0;i<INFOCODE2_SIZE+1;i++){
		int x;

		switch(k){
		case 0:
			cinfo[i]=dinfo[j]<<2;
			break;
		case 1:
			cinfo[i]=(dinfo[j] & 0x1f)<<3;
			break;
		case 2:
			cinfo[i]=(dinfo[j] & 0x0f)<<4;
			break;
		case 3:
			cinfo[i]=(dinfo[j] & 0x07)<<5;
			break;
		case 4:
			cinfo[i]=(dinfo[j] & 0x03)<<6;
			break;
		case 5:
			cinfo[i]=(dinfo[j] & 0x01)<<7;
			break;
		}
		x=6-k;
		k=0;
		j++;

		if(x<2){
			cinfo[i]|=dinfo[j]<<(2-x);
			x+=6;
			j++;
		}

		switch(x){
		case 2:
			cinfo[i]|=dinfo[j];
			k=0;
			j++;
			break;
		case 3:
			cinfo[i]|=(dinfo[j]>>1) & 0x1f;
			k=5;
			break;
		case 4:
			cinfo[i]|=(dinfo[j]>>2) & 0x0f;
			k=4;
			break;
		case 5:
			cinfo[i]|=(dinfo[j]>>3) & 0x07;
			k=3;
			break;
		case 6:
			cinfo[i]|=(dinfo[j]>>4) & 0x03;
			k=2;
			break;
		case 7:
			cinfo[i]|=(dinfo[j]>>5) & 0x01;
			k=1;
			break;
		}
	}

	/* チェックサムの計算と検査 */
	checksum=0;
	for(i=0;i<INFOCODE2_SIZE;i++){
		checksum^=cinfo[i];
		binfo[i]=cinfo[i];
	}

   //BP - have to disable checksum-built-into-dataname feature because there's nowhere
   //to put it on PS3.  There are only 7 DOS-style characters worth of invisible-to-user
   //space in a PARAM.SFO file and it's pointless to waste that space on this redundant
   //functionality.  The PS3 save data record has its own tamper / corruption detection.
#ifndef BP_PS3
	if(cinfo[i]!=checksum) return 0;
#endif

	return 1;
}

int DecodeDataName_Kind(MCMAN_WORK *work,const char *name,void *info, int file_kind)
{
   MCMAN_INFODATA * infodata = (MCMAN_INFODATA*)(info);

   int ret = DecodeDataName( work, name, info );

   if( ret )
   {
      //Success
      //Only VR and Game saves display play time in file listings.  Convert from 60ths of a second.
      switch ( file_kind )
      {
      case MCMAN_FILE_KIND_GAME:
         {
            int frames = BP_Frames60ToFramesSys( infodata->mgs2_playtime );
            printf("DecodeDataName_Kind: convert loaded game time %d -> %d\n", infodata->mgs2_playtime, frames );
            infodata->mgs2_playtime = frames;
         }
         break;
      case MCMAN_FILE_KIND_VR:
         {
            int frames;
            MCMAN_VR_INFODATA vrinfo = { { 0 } };
            DecodeVRInfo( &vrinfo, infodata );
            frames = BP_Frames60ToFramesSys( vrinfo.vr_playtime );
            printf("DecodeDataName_Kind: convert loaded vr time %d -> %d\n", vrinfo.vr_playtime, frames );
            vrinfo.vr_playtime = frames;
            EncodeVRInfo( infodata, &vrinfo );
         }
         break;
      }
   }

   return ret;
}

static int CheckFileFlag(MCMAN_WORK *work,int index)
{
	int i=(index>>4);
	int s=(index & 15)*2;

	if(index<0 || index>=FILE_MAX) return 0;

#ifdef DEBUG
	// printf("CheckFileFlag = %d : 0x%08x\n",index,work->file_flags[i]);
#endif

	return (work->file_flags[i]>>s) & 3;
}

static int SetFileFlag(MCMAN_WORK *work,int index)
{
	int i=(index>>4);
	int s=(index & 15)*2;

	if(index<0 || index>=FILE_MAX) return 0;

#ifdef DEBUG
	// printf("SetFileFlag = %d : 0x%08x -> ",index,work->file_flags[i]);
#endif

	work->file_flags[i]|=(3<<s);

#ifdef DEBUG
	// printf("0x%08x\n",work->file_flags[i]);
#endif

	return 1;
}

static int ResetFileFlag(MCMAN_WORK *work,int index)
{
	int i=(index>>4);
	int s=(index & 15)*2;

	if(index<0 || index>=FILE_MAX) return 0;

	work->file_flags[i]&=~(1<<s);

	return 1;
}

static int ClearFileFlag(MCMAN_WORK *work,int index)
{
   int i=(index>>4);
   int s=(index & 15)*2;

   if(index<0 || index>=FILE_MAX) return 0;

   work->file_flags[i]&=~(3<<s);

   return 1;
}

static void ParamSet(MCMAN_WORK *work,char *filename,void *data,int size)
{
	work->cur_data=data;
	work->cur_size=size;
	if(work->path!=filename) strcpy(work->path,filename);

	work->sub_step=0;
}

enum {
	BINSAVELOAD_STEP_OPEN=0,
	BINSAVELOAD_STEP_OPEN_WAIT,
	BINSAVELOAD_STEP_OPINFO,
	BINSAVELOAD_STEP_OPINFO_WAIT,
	BINSAVELOAD_STEP_OPDATA,
	BINSAVELOAD_STEP_OPDATA_WAIT,
	BINSAVELOAD_STEP_CLOSE,
	BINSAVELOAD_STEP_CLOSE_WAIT,

	BINSAVELOAD_STEP_ERROR_CLOSE=0x10,
	BINSAVELOAD_STEP_ERROR_CLOSE_WAIT,
};

static int BinSave(MCMAN_WORK *work,int flag)
{
	MCAccessWork *accw=&(work->accwork);
#ifdef BP_PS2
	extern unsigned char _program_top;
	unsigned char *lkinfo=(unsigned char *)&_program_top;
#else
   static unsigned char lkinfo[LINKINFO_SIZE] = { 0 };
#endif	

	switch(work->sub_step){
	case BINSAVELOAD_STEP_OPEN:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessOpenStart(accw,work->port,work->slot,
							 work->path,SCE_WRONLY|SCE_CREAT,&(work->fd))){

			work->sub_step++;
		}
		break;
	case BINSAVELOAD_STEP_OPEN_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(flag) work->sub_step++;
			else work->sub_step=BINSAVELOAD_STEP_OPDATA;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case BINSAVELOAD_STEP_OPINFO:
		if(_UpdateMCStatus(work)) return -1;

#ifdef PSX2		/// バージョンチェック・Ｘは他の方法で（？）
		if(MCAccessWriteStart(accw,work->fd,lkinfo,LINKINFO_SIZE,&(work->ret_size))){

#ifdef DEBUG_MODE
			printf("MCMAN : BinSave : link info = 0x%02x 0x%02x 0x%02x 0x%02x\n",
				   *(lkinfo+0),*(lkinfo+1),*(lkinfo+2),*(lkinfo+3));
#endif

			work->sub_step++;
		}
#else	// PSX2
		work->sub_step += 2;	// 次のWAIT も飛ばす
#endif	// PSX2	
		break;
	case BINSAVELOAD_STEP_OPINFO_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->ret_size==LINKINFO_SIZE) work->sub_step++;
			else work->sub_step=BINSAVELOAD_STEP_ERROR_CLOSE;
			break;
		case MCACC_ERROR:
			work->sub_step=BINSAVELOAD_STEP_ERROR_CLOSE;
			break;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case BINSAVELOAD_STEP_OPDATA:
		if(_UpdateMCStatus(work)) return -1;

		if(work->cur_size==0){
			work->sub_step=BINSAVELOAD_STEP_CLOSE;
			break;
		}
		if(MCAccessWriteStart(accw,work->fd,work->cur_data,work->cur_size,
							  &(work->ret_size))){
			work->sub_step++;
		}
		break;
	case BINSAVELOAD_STEP_OPDATA_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->ret_size==work->cur_size) work->sub_step++;
			else work->sub_step=BINSAVELOAD_STEP_ERROR_CLOSE;
			break;
		case MCACC_ERROR:
			work->sub_step=BINSAVELOAD_STEP_ERROR_CLOSE;
			break;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case BINSAVELOAD_STEP_CLOSE:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessCloseStart(accw,work->fd)){
			work->sub_step++;
		}
		break;
	case BINSAVELOAD_STEP_CLOSE_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			return 1;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case BINSAVELOAD_STEP_ERROR_CLOSE:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessCloseStart(accw,work->fd)){
			work->sub_step++;
		}
		break;
	case BINSAVELOAD_STEP_ERROR_CLOSE_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -1;
		}
		break;
	}
	return 0;
}

static int BinLoad(MCMAN_WORK *work,int flag)
{
	MCAccessWork *accw=&(work->accwork);
#ifdef PSX2	
	static unsigned char lkinfo[LINKINFO_SIZE];
#endif	

	switch(work->sub_step){
	case BINSAVELOAD_STEP_OPEN:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessOpenStart(accw,work->port,work->slot,
							 work->path,SCE_RDONLY,&(work->fd))){

			work->sub_step++;
		}
		break;
	case BINSAVELOAD_STEP_OPEN_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(flag) work->sub_step++;
			else work->sub_step=BINSAVELOAD_STEP_OPDATA;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case BINSAVELOAD_STEP_OPINFO:
		if(_UpdateMCStatus(work)) return -1;
#ifdef PSX2
		if(MCAccessReadStart(accw,work->fd,lkinfo,LINKINFO_SIZE,&(work->ret_size))){
			work->sub_step++;
		}
#else	// PSX2
		work->sub_step += 2;	// 次のWAIT も飛ばす
#endif	// PSX2		
		break;
	case BINSAVELOAD_STEP_OPINFO_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->ret_size==LINKINFO_SIZE){
#ifdef PSX2				
#ifdef DEBUG_MODE
				printf("MCMAN : BinLoad : link info = 0x%02x 0x%02x 0x%02x 0x%02x\n",
					   lkinfo[0],lkinfo[1],lkinfo[2],lkinfo[3]);
#endif
#endif				
				work->cur_size-=LINKINFO_SIZE;
				work->sub_step++;
			}
			else work->sub_step=BINSAVELOAD_STEP_ERROR_CLOSE;
			break;
		case MCACC_ERROR:
			work->sub_step=BINSAVELOAD_STEP_ERROR_CLOSE;
			break;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case BINSAVELOAD_STEP_OPDATA:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessReadStart(accw,work->fd,work->cur_data,work->cur_size,
							 &(work->ret_size))){
			work->sub_step++;
		}
		break;
	case BINSAVELOAD_STEP_OPDATA_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->ret_size==work->cur_size) work->sub_step++;
			else work->sub_step=BINSAVELOAD_STEP_ERROR_CLOSE;
			break;
		case MCACC_ERROR:
			work->sub_step=BINSAVELOAD_STEP_ERROR_CLOSE;
			break;
		case MCACC_NEED_CHECK:
#ifdef DEBUG_MODE
			printf("Read Error\n");
#endif
			return -2;
		}
		break;
	case BINSAVELOAD_STEP_CLOSE:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessCloseStart(accw,work->fd)){
			work->sub_step++;
		}
		break;
	case BINSAVELOAD_STEP_CLOSE_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			return 1;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
#ifdef DEBUG_MODE
			printf("Close Error\n");
#endif
			return -2;
		}
		break;

	case BINSAVELOAD_STEP_ERROR_CLOSE:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessCloseStart(accw,work->fd)){
			work->sub_step++;
		}
		break;
	case BINSAVELOAD_STEP_ERROR_CLOSE_WAIT:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
#ifdef DEBUG_MODE
			printf("Error Close Error\n");
#endif
			return -1;
		}
		break;
	}
	return 0;
}

static int DeleteAllTables(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);
	int index=work->sub_step>>1;

	switch(work->sub_step & 1){
	case 0:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリ内のデータ部分を無くす */
		if(MCAccessDeleteStart(accw, work->port, work->slot, work->table[index].EntryName)){
			work->sub_step++;
		}
		break;
	case 1:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->sub_step++;
			index=work->sub_step>>1;
			if(index>=work->table_used_size) return 1;
			if(index>=TABLE_SIZE) return 2;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}
	return 0;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void PauseStep(MCMAN_WORK *work)
{
	int mode,port,slot;

	work->flag|=MCMAN_FLAG_PAUSE_STEP;

	mode=work->mode;
	work->mode=work->store_mode;
	work->store_mode=mode;
	port=work->port;
	work->port=work->store_port;
	work->store_port=port;
	slot=work->slot;
	work->slot=work->store_slot;
	work->store_slot=slot;

	work->store_step=work->step;
	work->step=0;
	work->sub_step=0;
}


int CheckStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step){
	case 0:
		work->flag&=~(MCMAN_FLAG_ALREADY_RETURN_RESULT|MCMAN_FLAG_CHECK_BACKGROUND|
					  MCMAN_FLAG_CHECK_TO_BACK|MCMAN_FLAG_NEXT_REQUEST);

		if(MCAccessAutoChecking(accw)){
			work->step++;

#ifdef DEBUG_MODE
			printf("AutoCheckWait Start\n");
#endif
		}
		else if(work->flag & MCMAN_FLAG_MCCHECKED){
			return 1;
		}
		else if(MCAccessCheckStart(accw)){
			work->step++;

#ifdef DEBUG_MODE
			printf("CheckStep Start\n");
#endif
		}
		break;
	case 1:
		if(MCAccessGetResult(accw)) return 1;
		break;
	}
	return 0;
}

int FormatStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step){
	case 0:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessFormatStart(accw,work->port,work->slot)){
			work->step++;
		}
		break;
	case 1:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			return 1;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}

	return 0;
}

int SavePhotoStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	// printf("MCMAN step = %d\n",work->step);

	switch(work->step){
	case 0:
		work->path[0]='/';
		MakePhotoDirName(work,work->path+1,work->id);
		work->step++;

		/* ディレクトリがある場合は、Mkdirのステップを飛ばす
		   0 -> 21 -> 22 -> 23 -> 24 -> 25 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10 -> 11
		   通常は
		   0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10 -> 11 */
		if(CheckFileFlag(work,work->id))
      {
			work->step=21;
			break;
		}
	case 1:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの作成 */
		if(MCAccessMkdirStart(accw,work->port,work->slot,work->path)){
			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
			break;
		case MCACC_NEED_CHECK:
			return -2;
			break;
		}
		break;

	case 3:
	case 21:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの移動 */
		if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
			work->step++;
		}
		break;
	case 4:
	case 22:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 5:
		/* アイコンファイルのセーブ */
#ifdef PSX2		
#if 0
		MakePhotoDirName(work,work->path,work->id);

		strcpy(work->iconsys.FnameView,work->path);
		strcpy(work->iconsys.FnameCopy,work->path);
		strcpy(work->iconsys.FnameDel,work->path);

		ParamSet(work,work->path,work->icondata,work->iconsize);
#else
		strcpy(work->iconsys.FnameView,ICON_FILENAME);
		strcpy(work->iconsys.FnameCopy,ICON_FILENAME);
		strcpy(work->iconsys.FnameDel,ICON_FILENAME);

		ParamSet(work,ICON_FILENAME,work->icondata,work->iconsize);
#endif
#endif		
		work->step++;
		break;
	case 7:
		/* icon.sysのセーブ */
#ifdef PSX2		
		ParamSet(work,ICONSYS_FILENAME,&(work->iconsys),sizeof(work->iconsys));
#endif		
		work->step++;
		break;
	case 9:
		/* Dummyfileのセーブ */
		MakePhotoDirName(work,work->path,work->id);
		ParamSet(work,work->path,NULL,0);
		work->step++;
		break;
	case 11:
		/* データファイルのセーブ */
		{
			unsigned char crc[4];

			calc_crc_code(work->data,work->datasize,crc);
			CodeData(work->data,work->datasize);

			MakeDataName(work,work->path,work->info,crc);
			ParamSet(work,work->path,work->data,work->datasize);

			work->step++;
		}
		break;
	case 13:
		/* ワーク内の情報の更新 */
#if 0
		work->date_time[work->id][DATETIME_INDEX_YEAR]
			=(work->rtc.year>>4)*10+(work->rtc.year & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_MONTH]
			=(work->rtc.month>>4)*10+(work->rtc.month & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_DAY]
			=(work->rtc.day>>4)*10+(work->rtc.day & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_HOUR]
			=(work->rtc.hour>>4)*10+(work->rtc.hour & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_MIN]
			=(work->rtc.minute>>4)*10+(work->rtc.minute & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_SEC]
			=(work->rtc.second>>4)*10+(work->rtc.second & 0x0f);
#else
		work->date_time[work->id][DATETIME_INDEX_YEAR]
			=(G_rtc.year>>4)*10+(G_rtc.year & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_MONTH]
			=(G_rtc.month>>4)*10+(G_rtc.month & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_DAY]
			=(G_rtc.day>>4)*10+(G_rtc.day & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_HOUR]
			=(G_rtc.hour>>4)*10+(G_rtc.hour & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_MIN]
			=(G_rtc.minute>>4)*10+(G_rtc.minute & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_SEC]
			=(G_rtc.second>>4)*10+(G_rtc.second & 0x0f);
#endif
		memcpy(work->file_info[work->id],work->info,INFOCODE_SIZE);
		SetFileFlag(work,work->id);
		return 1;

	case 6:
	case 8:
	case 10:
		switch(BinSave(work,0)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case 12:
		switch(BinSave(work,(LINKINFO_SIZE>0 ? 1 : 0))){
		case MCACC_SUCCESS:
			work->step++;
			DecodeData(work->data,work->datasize);
			break;
		case MCACC_ERROR:
			DecodeData(work->data,work->datasize);
			return -1;
		case MCACC_NEED_CHECK:
			DecodeData(work->data,work->datasize);
			return -2;
		}
		break;

	case 23:
		if(_UpdateMCStatus(work)) return -1;

		/* データ部分のファイル名を取得 */
		if(MCAccessGetdirStart(accw,work->port,work->slot,"D*",
							   work->table,TABLE_SIZE,
							   &(work->table_used_size))){

			work->step++;
		}
		break;
	case 24:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->table_used_size==1){
				/* 正常にデータを取得した */
				work->step++;
				work->sub_step=0;
			}
			else if(work->table_used_size==0){
				/* データがなくなっている */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : ??? Data not Exist\n",work->id);
#endif

				work->step=5;
			}
			else{
				/* 前のデータを消さずにセーブした */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : Over Two Data Files Exist !!!\n",work->id);
#endif

				work->step++;
				work->sub_step=0;
			}
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case 25:
		/* ディレクトリ内のデータ部分を無くす */
		switch(DeleteAllTables(work)){
		case 1:
			work->step=5;
		case 2:
			work->step-=2;
			break;
		}
		break;
	}

	return 0;
}

int LoadPhotoStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step){
	case 0:
		work->path[0]='/';
		MakePhotoDirName(work,work->path+1,work->id);
		work->step++;
	case 1:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの移動 */
		if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 3:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   "D*",work->table,TABLE_SIZE,
							   &(work->table_used_size))){

			work->step++;
		}
		break;
	case 4:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->table_used_size==1){
				/* 正常にデータを取得した */
				ParamSet(work,work->table[0].EntryName,work->data,work->table[0].FileSizeByte);
				work->step++;
			}
			else{
				/* 前のデータを消さずにセーブした
				   もしくはデータがなくなっている */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : Error File !!!\n",work->id);
#endif

				return -1;
			}
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 5:
		/* データファイルのロード */
		switch(BinLoad(work,1)){
		case MCACC_SUCCESS:
			{
            int ret;
				unsigned char crc[4];
				unsigned char file_info[INFOCODE2_SIZE];

				DecodeDataName(work,work->table[0].EntryName,file_info);

				DecodeData(work->data,work->cur_size);
				calc_crc_code(work->data,work->cur_size,crc);

				if(crc[0]==file_info[INFOCODE_SIZE+0] &&
				   crc[1]==file_info[INFOCODE_SIZE+1] &&
				   crc[2]==file_info[INFOCODE_SIZE+2] &&
				   crc[3]==file_info[INFOCODE_SIZE+3]){

					ret = 1;
				}
				else{

#ifdef DEBUG_MODE
					printf( "CRC2 ERROR %02X%02X%02X%02X %02X%02X%02X%02X\n",
							crc[0],crc[1],crc[2],crc[3],
							file_info[INFOCODE_SIZE+0],
							file_info[INFOCODE_SIZE+1],
							file_info[INFOCODE_SIZE+2],
							file_info[INFOCODE_SIZE+3]);
#endif

#ifndef BP_PS3
               ret = -1;
#else
               //BP: there's no room to encode a crc in the PARAM.SFO which is where this data
               //is stored now.  Must ignore mismatching crc and count on the cellSaveData API
               //to detect tampered / corrupt data.
               ret = 1;
#endif
				}
            if( MGS_SaveStatus_IsCorrupted() )
            {
               //If PS3 or X360 API has determined that the file is corrupt, it trumps the CRC check.
               ret = -1;
            }
            return ret;
			}
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}

	return 0;
}

int LoadPhotoIconStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step){
	case 0:
		work->path[0]='/';
		MakePhotoDirName(work,work->path+1,work->id);
		work->step++;
	case 1:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの移動 */
		if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			MakePhotoDirName(work,work->path,work->id);
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 3:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   work->path,work->table,TABLE_SIZE,
							   &(work->table_used_size))){

			work->step++;
		}
		break;
	case 4:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->table_used_size==1){
				/* 正常にデータを取得した */
#ifdef PSX2				
				ParamSet(work,work->path,work->icondata,work->table[0].FileSizeByte);
#endif				
				work->step++;
			}
			else{
				/* データがなくなっている */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : Error File !!!\n",work->id);
#endif

				return -1;
			}
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 5:
		/* アイコンファイルのロード */
		switch(BinLoad(work,0)){
		case MCACC_SUCCESS:
			return 1;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}

	return 0;
}



static int GetPhotoDirSearchSub(MCMAN_WORK *work,int easy_flag)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step & 1){
	case 0:
		if(_UpdateMCStatus(work)) return -1;

		strcpy(s_dirname,"/");
		strcat(s_dirname,MC_PFILE_NAME);
		strcat(s_dirname,"*");

		if(work->step & ~1){
			if(MCAccessGetdirNextStart(accw,work->port,work->slot,s_dirname,
									   work->table,TABLE_SIZE,
									   &(work->table_used_size))){

				work->step++;
			}
		}
		else{
			if(MCAccessGetdirStart(accw,work->port,work->slot,s_dirname,
								   work->table,TABLE_SIZE,
								   &(work->table_used_size))){

				work->step++;
			}
		}
		break;
	case 1:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			{
				int i;
				for(i=0;i<work->table_used_size;i++){
					sceMcTblGetDir *t=&(work->table[i]);
					int index;
#ifdef PSX2					
					sceCdCLOCK clock;
#endif
#ifdef KP_XBOX
					SYSTEMTIME	clock;
					FILETIME	localtime;
#endif
					index=(t->EntryName[MC_PFILE_NAME_LEN+0]-'0')*100;
					index+=(t->EntryName[MC_PFILE_NAME_LEN+1]-'0')*10;
					index+=(t->EntryName[MC_PFILE_NAME_LEN+2]-'0')*1;

					if(index>=PHOTO_FILE_MAX) continue;

					if(SetFileFlag(work,index)){
						work->n_files++;

						work->easy_search_n_files[work->port]=work->n_files;
						work->readable_n_files=work->n_files;

						if(index>work->max_id){
							work->max_id=index;
							work->easy_search_max_id[work->port]=index;
						}

#if 1 //BP_PS2
                  memset( &clock, 0, sizeof( clock ) );
                  BP_MCManSetDateTime( work->date_time[index], t->_Modify );
#else

#ifdef PSX2						
						clock.stat=0;
						clock.second=(((t->_Modify.Sec/10) % 10)<<4);
						clock.second|=(t->_Modify.Sec % 10);
						clock.minute=(((t->_Modify.Min/10) % 10)<<4);
						clock.minute|=(t->_Modify.Min % 10);
						clock.hour=(((t->_Modify.Hour/10) % 10)<<4);
						clock.hour|=(t->_Modify.Hour % 10);
						clock.day=(((t->_Modify.Day/10) % 10)<<4);
						clock.day|=(t->_Modify.Day % 10);
						clock.month=(((t->_Modify.Month/10) % 10)<<4);
						clock.month|=(t->_Modify.Month % 10);
						clock.year=(((t->_Modify.Year/10) % 10)<<4);
						clock.year|=(t->_Modify.Year % 10);

						sceScfGetLocalTimefromRTC(&clock);

						work->date_time[index][DATETIME_INDEX_YEAR]
							=(clock.year>>4)*10+(clock.year & 0x0f);
						work->date_time[index][DATETIME_INDEX_MONTH]
							=(clock.month>>4)*10+(clock.month & 0x0f);
						work->date_time[index][DATETIME_INDEX_DAY]
							=(clock.day>>4)*10+(clock.day & 0x0f);
						work->date_time[index][DATETIME_INDEX_HOUR]
							=(clock.hour>>4)*10+(clock.hour & 0x0f);
						work->date_time[index][DATETIME_INDEX_MIN]
							=(clock.minute>>4)*10+(clock.minute & 0x0f);
						work->date_time[index][DATETIME_INDEX_SEC]
							=(clock.second>>4)*10+(clock.second & 0x0f);
#endif						
#ifdef KP_XBOX

						FileTimeToLocalFileTime( &t->_Modify, &localtime );
						FileTimeToSystemTime( &localtime, &clock );
						
						work->date_time[index][DATETIME_INDEX_YEAR]
								= (u_char) clock.wYear;
						work->date_time[index][DATETIME_INDEX_MONTH]
								= (u_char) clock.wMonth;
						work->date_time[index][DATETIME_INDEX_DAY]
								= (u_char) clock.wDay;
						work->date_time[index][DATETIME_INDEX_HOUR]
								= (u_char) clock.wHour;
						work->date_time[index][DATETIME_INDEX_MIN]
								= (u_char) clock.wMinute;
						work->date_time[index][DATETIME_INDEX_SEC]
								= (u_char) clock.wSecond;
						
#endif	

#endif //BP

#ifdef DEBUG
						printf("ymd hms : %d : %d/%d/%d %d:%d:%d\n",
							   index,
							   work->date_time[index][DATETIME_INDEX_YEAR],
							   work->date_time[index][DATETIME_INDEX_MONTH],
							   work->date_time[index][DATETIME_INDEX_DAY],
							   work->date_time[index][DATETIME_INDEX_HOUR],
							   work->date_time[index][DATETIME_INDEX_MIN],
							   work->date_time[index][DATETIME_INDEX_SEC]);
#endif
					}
				}
			}
#if 0 //BP_PS2
         //BP - NEVER allow to advance to the case where it calls sceMcGetDir with a mode of 1
         //("get next n entries") since we don't support it
         //(this condition should always be true now anyway since TABLE_SIZE is large enough to
         //hold all results possible for all save file types,
         //DATA_FILE_MAX+VR_FILE_MAX+SNAKE_TALES_FILE_MAX+PHOTO_FILE_MAX)
			if(work->table_used_size<TABLE_SIZE)
#endif
         {
            return 1;
         }
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}
	return 0;
}

int PhotoEasySearchStep(MCMAN_WORK *work)
{
	return GetPhotoDirSearchSub(work,0);
}

int PhotoDirSearchStep(MCMAN_WORK *work)
{
	return GetPhotoDirSearchSub(work,0);
}

int GetPhotoInfo(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step & 3){
	case 3:
		work->checking_id++;
		work->step++;

		if(work->flag & MCMAN_FLAG_CHECK_BACKGROUND){
			if(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT){
				if(work->flag & MCMAN_FLAG_NEXT_REQUEST){
					work->flag&=~MCMAN_FLAG_NEXT_REQUEST;
					PauseStep(work);
					break;
				}
			}
			else if((work->step>>2)>=PHOTO_FILE_PAGE_SIZE){
				work->result=1;
				work->flag|=MCMAN_FLAG_ALREADY_RETURN_RESULT;
			}
		}
		if((work->step>>2)>=work->checked_limit) return 1;

	case 0:
		while(1){
			int index=work->checking_id;

			if(index>=FILE_MAX) return 1;

			if(CheckFileFlag(work,index)) break;
			work->checking_id++;
		}

		work->path[0]='/';
		MakePhotoDirName(work,work->path+1,work->checking_id);
		strcat(work->path,"/D*");

#ifdef DEBUG_MODE
		printf("Check Filename = %s\n",work->path);
#endif

		work->step++;

	case 1:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   work->path,dirchk_table,DIRCHK_TABLE_SIZE,
							   &(dirchk_table_used_size))){

			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(dirchk_table_used_size==1){
            // TODO: PHOTE_SAVE get rid of this fix file size logic and use varaible size files
				/* 正常にデータを取得した */
				if(dirchk_table[0].FileSizeByte!=PHOTO_FILE_MAXSIZE+LINKINFO_SIZE){
					ResetFileFlag(work,work->checking_id);
#if 0 // BP_Save/Load - We don't care if the file is corrupt, corrupt files should still be reported like everything else
					work->n_files--;
#endif
					work->easy_search_n_files[work->port]=work->n_files;
				}
				else if(!DecodeDataName(work,dirchk_table[0].EntryName,
										work->file_info[work->checking_id])){

					ResetFileFlag(work,work->checking_id);
#if 0 // BP_Save/Load - We don't care if the file is corrupt, corrupt files should still be reported like everything else
					work->n_files--;
#endif
					work->easy_search_n_files[work->port]=work->n_files;
				}
#if 0
				else{
 					work->max_id=work->checking_id;
					work->easy_search_max_id[work->port]=work->checking_id;
				}
#endif
			}
			else{
				/* 前のデータを消さずにセーブした
				   もしくはデータがなくなっている */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : Error File !!!\n",work->checking_id);
#endif

				ResetFileFlag(work,work->checking_id);
				work->n_files--;
				work->easy_search_n_files[work->port]=work->n_files;
			}
			work->step++;
			break;
		case MCACC_ERROR:
			ResetFileFlag(work,work->checking_id);
			work->n_files--;
			work->easy_search_n_files[work->port]=work->n_files;
			work->step++;
			break;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}
	return 0;
}

int GetPhotoInfoR(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step & 3){
	case 3:
		work->checking_id--;
		work->step++;

		if(work->flag & MCMAN_FLAG_CHECK_BACKGROUND){
			if(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT){
				if(work->flag & MCMAN_FLAG_NEXT_REQUEST){
					work->flag&=~MCMAN_FLAG_NEXT_REQUEST;
					PauseStep(work);
					break;
				}
			}
			else if((work->step>>2)>=PHOTO_FILE_PAGE_SIZE){
				work->result=1;
				work->flag|=MCMAN_FLAG_ALREADY_RETURN_RESULT;
			}
		}
		if((work->step>>2)>=work->checked_limit) return 1;

	case 0:
		while(1){
			int index=work->checking_id;

			if(index<0) return 1;

			if(CheckFileFlag(work,index)) break;
			work->checking_id--;
		}

		work->path[0]='/';
		MakePhotoDirName(work,work->path+1,work->checking_id);
		strcat(work->path,"/D*");

#ifdef DEBUG_MODE
		printf("Check Filename = %s\n",work->path);
#endif

		work->step++;

	case 1:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   work->path,dirchk_table,DIRCHK_TABLE_SIZE,
							   &(dirchk_table_used_size))){

			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(dirchk_table_used_size==1){
				/* 正常にデータを取得した */
				if(dirchk_table[0].FileSizeByte!=PHOTO_FILE_MAXSIZE+LINKINFO_SIZE){
					ResetFileFlag(work,work->checking_id);
					work->n_files--;
					work->easy_search_n_files[work->port]=work->n_files;
				}
				else if(!DecodeDataName(work,dirchk_table[0].EntryName,
								   work->file_info[work->checking_id])){

					ResetFileFlag(work,work->checking_id);
					work->n_files--;
					work->easy_search_n_files[work->port]=work->n_files;
				}
#if 0
				else{
					if(work->max_id<work->checking_id){
						work->max_id=work->checking_id;
						work->easy_search_max_id[work->port]=work->checking_id;
					}
				}
#endif
			}
			else{
				/* 前のデータを消さずにセーブした
				   もしくはデータがなくなっている */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : Error File !!!\n",work->checking_id);
#endif

				ResetFileFlag(work,work->checking_id);
				work->n_files--;
				work->easy_search_n_files[work->port]=work->n_files;
			}
			work->step++;
			break;
		case MCACC_ERROR:
			ResetFileFlag(work,work->checking_id);
			work->n_files--;
			work->easy_search_n_files[work->port]=work->n_files;
			work->step++;
			break;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}
	return 0;
}


int ChangePhotoInfoStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	// printf("change photo step = %d\n",work->step);

	switch(work->step){
	case 0:
		work->path[0]='/';
		MakePhotoDirName(work,work->path+1,work->id);
		work->step++;
	case 1:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの移動 */
		if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 3:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   "D*",work->table,TABLE_SIZE,
							   &(work->table_used_size))){

			work->step++;
		}
		break;
	case 4:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->table_used_size==1){
				/* 正常にデータを取得した */
				work->step++;
			}
			else{
				/* 前のデータを消さずにセーブした
				   もしくはデータがなくなっている */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : Error File !!!\n",work->id);
#endif

				return -1;
			}
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 5:
		{
			unsigned char *sinfo=MCManGetFileInfo(work->id);
			unsigned char crc[4];
			int i;

			crc[0]=sinfo[INFOCODE_SIZE+0];
			crc[1]=sinfo[INFOCODE_SIZE+1];
			crc[2]=sinfo[INFOCODE_SIZE+2];
			crc[3]=sinfo[INFOCODE_SIZE+3];

			MakeDataName(work,work->path,work->info,crc);

			for(i=0;i<INFOCODE_SIZE;i++) sinfo[i]=((unsigned char *)(work->info))[i];

			work->step++;
		}
	case 6:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessRenameStart(accw,work->port,work->slot,
							   work->table[0].EntryName,work->path)){

			work->step++;
		}
		break;
	case 7:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			// return 1;
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

		/* 写真のタイトルを変更 */
	case 8:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessOpenStart(accw,work->port,work->slot,ICONSYS_FILENAME,
							 SCE_WRONLY,&(work->fd))){

			work->step++;
		}
		break;
	case 9:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case 10:
		if(_UpdateMCStatus(work)) return -1;
#ifdef PSX2
		if(MCAccessSeekStart(accw,work->fd,
							 (int)&(work->iconsys.TitleName[0])-(int)&(work->iconsys),
							 0,NULL))
#endif			
		{
			

			work->step++;
		}
		break;
	case 11:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
		case MCACC_NEED_CHECK:
			work->step=0x20;
			break;
		}
		break;
	case 12:
		if(_UpdateMCStatus(work)) return -1;

#ifdef PSX2
		if(MCAccessWriteStart(accw,work->fd,work->iconsys.TitleName,
							  sizeof(work->iconsys.TitleName),&(work->ret_size)))
#endif			
		{

			work->step++;
		}
		break;
	case 13:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
#ifdef PSX2			
			if(work->ret_size==sizeof(work->iconsys.TitleName)) work->step++;
			else work->step=0x20;
#else
			work->step++;
#endif			
			break;
		case MCACC_ERROR:
		case MCACC_NEED_CHECK:
			work->step=0x20;
			break;
		}
		break;
	case 14:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessCloseStart(accw,work->fd)){
			work->step++;
		}
		break;
	case 15:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			return 1;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 0x20:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessCloseStart(accw,work->fd)){
			work->step++;
		}
		break;
	case 0x21:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}

	return 0;
}


/* ------------------------------------------------------------------------ */


int SaveGameStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step){
	case 0:
		work->path[0]='/';
		MakeGameDirName(work,work->path+1,work->id);
		work->step++;

		/* ディレクトリがある場合は、Mkdirのステップを飛ばす
		   0 -> 21 -> 22 -> 23 -> 24 -> 25 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10 -> 11
		   通常は
		   0 -> 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10 -> 11 */
      // BP - if directory exists, skip mkdir and follow the first sequence.
      // else follow the second sequence.
 		if(CheckFileFlag(work,work->id)){
			work->step=21;
			break;
		}
	case 1:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの作成 */
		if(MCAccessMkdirStart(accw,work->port,work->slot,work->path)){
			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
			break;
		case MCACC_NEED_CHECK:
			return -2;
			break;
		}
		break;

	case 3:
	case 21:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの移動 */
		if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
			work->step++;
		}
		break;
	case 4:
	case 22:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 5:
		/* アイコンファイルのセーブ */
#ifdef PSX2		
#if 0
		MakeGameDirName(work,work->path,work->id);

		strcpy(work->iconsys.FnameView,work->path);
		strcpy(work->iconsys.FnameCopy,work->path);
		strcpy(work->iconsys.FnameDel,work->path);

		ParamSet(work,work->path,work->icondata,work->iconsize);
#else
		strcpy(work->iconsys.FnameView,ICON_FILENAME);
		strcpy(work->iconsys.FnameCopy,ICON_FILENAME);
		strcpy(work->iconsys.FnameDel,ICON_FILENAME);

		ParamSet(work,ICON_FILENAME,work->icondata,work->iconsize);
#endif
#endif		
		work->step++;
		break;
	case 7:
		/* icon.sysのセーブ */

#if 0
		printf("%d %d %d %d\n",
			   work->iconsys.BgColor[3][0],
			   work->iconsys.BgColor[3][1],
			   work->iconsys.BgColor[3][2],
			   work->iconsys.BgColor[3][3]);
#endif
#ifdef PSX2
		ParamSet(work,ICONSYS_FILENAME,&(work->iconsys),sizeof(work->iconsys));
#endif		
		work->step++;
		break;
	case 9:
		/* Dummyfileのセーブ */
		MakeGameDirName(work,work->path,work->id);
		{
#ifdef MGS2_VRTRIAL
			ParamSet(work,work->path, NULL, 0);
#else
			if( work->file_kind == MCMAN_FILE_KIND_GAME ) {
				// ゲームの場合はタンカーとプラントでサイズをそろえる
				int size1,size2;
				size1=(ICON_FILE_MAXSIZE+CLASTER_SIZE-1) & ~(CLASTER_SIZE-1);
				size2=(work->iconsize+CLASTER_SIZE-1) & ~(CLASTER_SIZE-1);
				ParamSet(work,work->path,work /* 適当な値 */ ,size1-size2);
			} else {
				ParamSet(work,work->path, NULL, 0);
			}
#endif
		}
		work->step++;
		break;
	case 11:
		/* データファイルのセーブ */
		{
			unsigned char crc[4];

			calc_crc_code(work->data,work->datasize,crc);
			CodeData(work->data,work->datasize);

			MakeDataName_Kind(work,work->path,work->info,crc, work->file_kind);
			ParamSet(work,work->path,work->data,work->datasize);

			work->step++;
		}
		break;
	case 13:
		/* ワーク内の情報の更新 */
#if 0
		work->date_time[work->id][DATETIME_INDEX_YEAR]
			=(work->rtc.year>>4)*10+(work->rtc.year & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_MONTH]
			=(work->rtc.month>>4)*10+(work->rtc.month & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_DAY]
			=(work->rtc.day>>4)*10+(work->rtc.day & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_HOUR]
			=(work->rtc.hour>>4)*10+(work->rtc.hour & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_MIN]
			=(work->rtc.minute>>4)*10+(work->rtc.minute & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_SEC]
			=(work->rtc.second>>4)*10+(work->rtc.second & 0x0f);
#else
		work->date_time[work->id][DATETIME_INDEX_YEAR]
			=(G_rtc.year>>4)*10+(G_rtc.year & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_MONTH]
			=(G_rtc.month>>4)*10+(G_rtc.month & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_DAY]
			=(G_rtc.day>>4)*10+(G_rtc.day & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_HOUR]
			=(G_rtc.hour>>4)*10+(G_rtc.hour & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_MIN]
			=(G_rtc.minute>>4)*10+(G_rtc.minute & 0x0f);
		work->date_time[work->id][DATETIME_INDEX_SEC]
			=(G_rtc.second>>4)*10+(G_rtc.second & 0x0f);
#endif
		memcpy(work->file_info[work->id],work->info,INFOCODE_SIZE);
		SetFileFlag(work,work->id);
		return 1;

	case 6:
	case 8:
	case 10:
		switch(BinSave(work,0)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case 12:
		switch(BinSave(work,(LINKINFO_SIZE>0 ? 1 : 0))){
		case MCACC_SUCCESS:
			work->step++;
			DecodeData(work->data,work->datasize);
			break;
		case MCACC_ERROR:
			DecodeData(work->data,work->datasize);
			return -1;
		case MCACC_NEED_CHECK:
			DecodeData(work->data,work->datasize);
			return -2;
		}
		break;

	case 23:
		if(_UpdateMCStatus(work)) return -1;

		/* データ部分のファイル名を取得 */
		if(MCAccessGetdirStart(accw,work->port,work->slot,"D*",
							   work->table,TABLE_SIZE,
							   &(work->table_used_size))){

			work->step++;
		}
		break;
	case 24:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->table_used_size==1){
				/* 正常にデータを取得した */
				work->step++;
				work->sub_step=0;
			}
			else if(work->table_used_size==0){
				/* データがなくなっている */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : ??? Data not Exist\n",work->id);
#endif

				work->step=5;
			}
			else{
				/* 前のデータを消さずにセーブした */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : Over Two Data Files Exist !!!\n",work->id);
#endif

				work->step++;
				work->sub_step=0;
			}
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	case 25:
		/* ディレクトリ内のデータ部分を無くす */
		switch(DeleteAllTables(work)){
		case 1:
			work->step=5;
		case 2:
			work->step-=2;
			break;
		}
		break;
	}

	return 0;
}

int LoadGameStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step){
	case 0:
		work->path[0]='/';
		MakeGameDirName(work,work->path+1,work->id);
		work->step++;
	case 1:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの移動 */
		if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 3:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   "D*",work->table,TABLE_SIZE,
							   &(work->table_used_size))){

			work->step++;
		}
		break;
	case 4:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->table_used_size==1){
				/* 正常にデータを取得した */
				ParamSet(work,work->table[0].EntryName,work->data,work->table[0].FileSizeByte);
				work->step++;
			}
			else{
				/* 前のデータを消さずにセーブした
				   もしくはデータがなくなっている */

#ifdef DEBUG_MODE
				printf("%d : Error File !!!\n",work->id);
#endif

				return -1;
			}
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 5:
		/* データファイルのロード */
		switch(BinLoad(work,1)){
		case MCACC_SUCCESS:
			{
            int ret;
				unsigned char crc[4];
				unsigned char file_info[INFOCODE2_SIZE];

				DecodeDataName_Kind(work,work->table[0].EntryName,file_info, work->file_kind);

				DecodeData(work->data,work->cur_size);
				calc_crc_code(work->data,work->cur_size,crc);

				if(crc[0]==file_info[INFOCODE_SIZE+0] &&
				   crc[1]==file_info[INFOCODE_SIZE+1] &&
				   crc[2]==file_info[INFOCODE_SIZE+2] &&
				   crc[3]==file_info[INFOCODE_SIZE+3]){

					ret = 1;
				}
				else{

#ifdef DEBUG_MODE
					printf( "CRC2 ERROR %02X%02X%02X%02X %02X%02X%02X%02X\n",
							crc[0],crc[1],crc[2],crc[3],
							file_info[INFOCODE_SIZE+0],
							file_info[INFOCODE_SIZE+1],
							file_info[INFOCODE_SIZE+2],
							file_info[INFOCODE_SIZE+3]);
#endif
#ifndef BP_PS3
					ret = -1;

               Transfarring_SetLocalFileCorrupt_Platform(1, Transfarring_KonamiSaveTypeToBPSaveType( work->file_kind ), work->id );
#else
               //BP: there's no room to encode a crc in the PARAM.SFO which is where this data
               //is stored now.  Must ignore mismatching crc and count on the cellSaveData API
               //to detect tampered / corrupt data.
               ret = 1;
#endif

				}
            if( MGS_SaveStatus_IsCorrupted() )
            {
               //If PS3 or X360 API has determined that the file is corrupt, it trumps the CRC check.
               ret = -1;

               Transfarring_SetLocalFileCorrupt_Platform(1, Transfarring_KonamiSaveTypeToBPSaveType( work->file_kind ), work->id );
            }

            return ret;
			}
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}

	return 0;
}

int DeleteGameStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

   switch(work->step)
   {
   case 0:
      work->path[0]='/';
		MakeGameDirName(work,work->path+1,work->id);
      work->step++;
      break;
   case 1:
      if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
         work->step++;
      }
      break;
   case 2:
      if(_UpdateMCStatus(work)) break;

      if(MCAccessGetdirStart(accw,work->port,work->slot,"D*",
         work->table,TABLE_SIZE,
         &(work->table_used_size))){

            work->step++;
            work->sub_step = 0;
      }
      break;
   case 3:
      if(_UpdateMCStatus(work)) break;

      switch(DeleteAllTables(work))
      {
      case 0: // In progress
         break;
      case 1: // Success
         ClearFileFlag( work, work->id );
         --work->readable_n_files;
         return 1;
         break;
      default: // Failure
         return -1;
         break;
      }
      break;
   }

   return 0;
}

int DeletePhotoStep(MCMAN_WORK *work)
{
   MCAccessWork *accw=&(work->accwork);

   switch(work->step)
   {
   case 0:
      work->path[0]='/';
      MakePhotoDirName(work,work->path+1,work->id);
      work->step++;
      break;
   case 1:

      /* ディレクトリの移動 */
      if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
         work->step++;
      }
      break;
   case 2:
      if(_UpdateMCStatus(work)) break;

      /* データ部分のファイル名を取得 */
      if(MCAccessGetdirStart(accw,work->port,work->slot,"D*",
         work->table,TABLE_SIZE,
         &(work->table_used_size))){

            work->step++;
            work->sub_step = 0;
      }
      break;
   case 3:
      if(_UpdateMCStatus(work)) break;

      switch(DeleteAllTables(work))
      {
      case 0: // In progress
         break;
      case 1: // Success
         ClearFileFlag( work, work->id );
         --work->readable_n_files;
         return 1;
      default: // Failure
         return -1;
      }
      break;
   }

   return 0;
}

static int GetGameDirSearchSub(MCMAN_WORK *work,int easy_flag)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step & 1){
	case 0:
		if(_UpdateMCStatus(work)) return -1;

		strcpy(s_dirname,"/");
		switch ( work->file_kind ) {
		case MCMAN_FILE_KIND_GAME:
			strcat(s_dirname,MC_GFILE_NAME);
			break;
		case MCMAN_FILE_KIND_VR:
			strcat(s_dirname,MC_VFILE_NAME);
			break;
		case MCMAN_FILE_KIND_SNAKE_TALES:
			strcat(s_dirname,MC_SFILE_NAME);
			break;
#ifdef PSX2
		case MCMAN_FILE_KIND_FIRSTCHECK:
			strcat(s_dirname,MC_AFILE_NAME);
		case MCMAN_FILE_KIND_OTHER:
			strcat(s_dirname, GetOtherFilename() );
			break;
#endif
		}
		strcat(s_dirname,"*");

		if(work->step & ~1){
			if(MCAccessGetdirNextStart(accw,work->port,work->slot,s_dirname,
									   work->table,TABLE_SIZE,
									   &(work->table_used_size))){

				// printf("Check Filename = " "/" MC_GFILE_NAME "*" "\n");

				work->step++;
			}
		}
		else{
			if(MCAccessGetdirStart(accw,work->port,work->slot,s_dirname,
								   work->table,TABLE_SIZE,
								   &(work->table_used_size))){

				// printf("Check Filename = " "/" MC_GFILE_NAME "*" "\n");
				//		printf("Check_Filename = %s\n", s_dirname );

				work->step++;
			}
		}
		break;
	case 1:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			// printf("Used = %d\n",work->table_used_size);

			{
				int i;
				for(i=0;i<work->table_used_size;i++){
					sceMcTblGetDir *t=&(work->table[i]);
					int index;
#ifdef PSX2					
					sceCdCLOCK clock;
#endif					
#ifdef KP_XBOX
					SYSTEMTIME	clock;
					FILETIME	localtime;
#endif
#ifdef PSX2
					//					printf("MCMAN : dirname %s %x\n", t->EntryName, work->exist_flag );
					if( work->file_kind == MCMAN_FILE_KIND_FIRSTCHECK ) {
						// ゲーム以外のファイルは存在フラグだけチェックして無視	
						switch( t->EntryName[MC_GFILE_NAME_LEN - 1] ) {
						case 'G':
							work->exist_flag |= MCMAN_EXIST_FLAG_GAME;
							break;
						case 'P':
							work->exist_flag |= MCMAN_EXIST_FLAG_PHOTO;
							continue;
						case 'V':
							work->exist_flag |= MCMAN_EXIST_FLAG_VR;
							continue;
						case 'S':
							work->exist_flag |= MCMAN_EXIST_FLAG_SNAKE_TALES;
							continue;
						}
					}
#endif

					index=(t->EntryName[MC_GFILE_NAME_LEN+0]-'0')*100;
					index+=(t->EntryName[MC_GFILE_NAME_LEN+1]-'0')*10;
					index+=(t->EntryName[MC_GFILE_NAME_LEN+2]-'0')*1;

					if(index>= MCMAN_GetDataFileMax( work->file_kind) ) continue;

					if(SetFileFlag(work,index)){
						work->n_files++;

						work->easy_search_n_files[work->port]=work->n_files;
						work->readable_n_files=work->n_files;

						if(index>work->max_id){
							work->max_id=index;
							work->easy_search_max_id[work->port]=index;
						}

#if 1 //BP
                  memset( &clock, 0, sizeof( clock ) );
                  BP_MCManSetDateTime( work->date_time[index], t->_Modify );
#else

#ifdef PSX2						
						clock.stat=0;
						clock.second=(((t->_Modify.Sec/10) % 10)<<4);
						clock.second|=(t->_Modify.Sec % 10);
						clock.minute=(((t->_Modify.Min/10) % 10)<<4);
						clock.minute|=(t->_Modify.Min % 10);
						clock.hour=(((t->_Modify.Hour/10) % 10)<<4);
						clock.hour|=(t->_Modify.Hour % 10);
						clock.day=(((t->_Modify.Day/10) % 10)<<4);
						clock.day|=(t->_Modify.Day % 10);
						clock.month=(((t->_Modify.Month/10) % 10)<<4);
						clock.month|=(t->_Modify.Month % 10);
						clock.year=(((t->_Modify.Year/10) % 10)<<4);
						clock.year|=(t->_Modify.Year % 10);

						sceScfGetLocalTimefromRTC(&clock);

						work->date_time[index][DATETIME_INDEX_YEAR]
							=(clock.year>>4)*10+(clock.year & 0x0f);
						work->date_time[index][DATETIME_INDEX_MONTH]
							=(clock.month>>4)*10+(clock.month & 0x0f);
						work->date_time[index][DATETIME_INDEX_DAY]
							=(clock.day>>4)*10+(clock.day & 0x0f);
						work->date_time[index][DATETIME_INDEX_HOUR]
							=(clock.hour>>4)*10+(clock.hour & 0x0f);
						work->date_time[index][DATETIME_INDEX_MIN]
							=(clock.minute>>4)*10+(clock.minute & 0x0f);
						work->date_time[index][DATETIME_INDEX_SEC]
							=(clock.second>>4)*10+(clock.second & 0x0f);
#endif						
#ifdef KP_XBOX
						FileTimeToLocalFileTime( &t->_Modify, &localtime );
						FileTimeToSystemTime( &localtime, &clock );
						
						work->date_time[index][DATETIME_INDEX_YEAR]
								= (u_char) clock.wYear;
						work->date_time[index][DATETIME_INDEX_MONTH]
								= (u_char) clock.wMonth;
						work->date_time[index][DATETIME_INDEX_DAY]
								= (u_char) clock.wDay;
						work->date_time[index][DATETIME_INDEX_HOUR]
								= (u_char) clock.wHour;
						work->date_time[index][DATETIME_INDEX_MIN]
								= (u_char) clock.wMinute;
						work->date_time[index][DATETIME_INDEX_SEC]
								= (u_char) clock.wSecond;
						
#endif						

#endif //BP

#ifdef DEBUG
						printf("ymd hms : %d : %d/%d/%d %d:%d:%d\n",
							   index,
							   work->date_time[index][DATETIME_INDEX_YEAR],
							   work->date_time[index][DATETIME_INDEX_MONTH],
							   work->date_time[index][DATETIME_INDEX_DAY],
							   work->date_time[index][DATETIME_INDEX_HOUR],
							   work->date_time[index][DATETIME_INDEX_MIN],
							   work->date_time[index][DATETIME_INDEX_SEC]);
#endif
					}
				}
			}
#if 0 //BP_PS2
         //BP - NEVER allow to advance to the case where it calls sceMcGetDir with a mode of 1
         //("get next n entries") since we don't support it
         //(this condition should always be true now anyway since TABLE_SIZE is large enough to
         //hold all results possible for all save file types,
         //DATA_FILE_MAX+VR_FILE_MAX+SNAKE_TALES_FILE_MAX+PHOTO_FILE_MAX)
			if(work->table_used_size<TABLE_SIZE)
#endif
         {
            return 1;
         }
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}
	return 0;
}

int GameEasySearchStep(MCMAN_WORK *work)
{
	return GetGameDirSearchSub(work,0);
}

int GameDirSearchStep(MCMAN_WORK *work)
{
	return GetGameDirSearchSub(work,0);
}

int GetGameInfo(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step & 3){
	case 3:
		work->checking_id++;
		work->step++;

		if(work->flag & MCMAN_FLAG_CHECK_BACKGROUND){
			if(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT){
				if(work->flag & MCMAN_FLAG_NEXT_REQUEST){
					work->flag&=~MCMAN_FLAG_NEXT_REQUEST;
					PauseStep(work);
					break;
				}
			}
			else if((work->step>>2)>=FILE_PAGE_SIZE){
				work->result=1;
				work->flag|=MCMAN_FLAG_ALREADY_RETURN_RESULT;
			}
		}
		if((work->step>>2)>=work->checked_limit) return 1;

	case 0:
		while(1){
			int index=work->checking_id;

			if(index>= MCMAN_GetDataFileMax( work->file_kind ) ) return 1;

			if(CheckFileFlag(work,index)) break;
			work->checking_id++;
		}

		work->path[0]='/';
		MakeGameDirName(work,work->path+1,work->checking_id);
		strcat(work->path,"/D*");

#ifdef DEBUG_MODE
		printf("Check Filename = %s\n",work->path);
#endif

		work->step++;

	case 1:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   work->path,dirchk_table,DIRCHK_TABLE_SIZE,
							   &(dirchk_table_used_size))){

			work->step++;
		}
		break;
	case 2:
		// if(_UpdateMCStatus(work)) return -1;

		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(dirchk_table_used_size==1){
				/* 正常にデータを取得した */
				if(dirchk_table[0].FileSizeByte!=GAME_FILE_MAXSIZE+LINKINFO_SIZE){
					ResetFileFlag(work,work->checking_id);
					work->n_files--;
					work->easy_search_n_files[work->port]=work->n_files;

               Transfarring_SetLocalFileCorrupt_Platform(1, Transfarring_KonamiSaveTypeToBPSaveType( work->file_kind ), work->checking_id );
				}
				else if(!DecodeDataName_Kind(work,dirchk_table[0].EntryName,
										work->file_info[work->checking_id], work->file_kind)){

					ResetFileFlag(work,work->checking_id);
					work->n_files--;
					work->easy_search_n_files[work->port]=work->n_files;

               Transfarring_SetLocalFileCorrupt_Platform(1, Transfarring_KonamiSaveTypeToBPSaveType( work->file_kind ), work->checking_id );
				}
#if 0
				else{
 					work->max_id=work->checking_id;
					work->easy_search_max_id[work->port]=work->checking_id;
				}
#endif
			}
			else{
				/* 前のデータを消さずにセーブした
				   もしくはデータがなくなっている */

#ifdef DEBUG_MODE
				printf("%d : Error File !!!\n",work->checking_id);
#endif

				ResetFileFlag(work,work->checking_id);
				work->n_files--;
				work->easy_search_n_files[work->port]=work->n_files;

            Transfarring_SetLocalFileCorrupt_Platform(1, Transfarring_KonamiSaveTypeToBPSaveType( work->file_kind ), work->checking_id );
			}
			work->step++;
			break;
		case MCACC_ERROR:
			ResetFileFlag(work,work->checking_id);
			work->n_files--;
			work->easy_search_n_files[work->port]=work->n_files;
         Transfarring_SetLocalFileCorrupt_Platform(1, Transfarring_KonamiSaveTypeToBPSaveType( work->file_kind ), work->checking_id );
			work->step++;
			break;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}
	return 0;
}

int GetGameInfoR(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step & 3){
	case 3:
		work->checking_id--;
		work->step++;

		if(work->flag & MCMAN_FLAG_CHECK_BACKGROUND){
			if(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT){
				if(work->flag & MCMAN_FLAG_NEXT_REQUEST){
					work->flag&=~MCMAN_FLAG_NEXT_REQUEST;
					PauseStep(work);
					break;
				}
			}
			else if((work->step>>2)>=FILE_PAGE_SIZE){
				work->result=1;
				work->flag|=MCMAN_FLAG_ALREADY_RETURN_RESULT;
			}
		}
		if((work->step>>2)>=work->checked_limit) return 1;

	case 0:
		while(1){
			int index=work->checking_id;

			if(index<0) return 1;

			if(CheckFileFlag(work,index)) break;
			work->checking_id--;
		}

		work->path[0]='/';
		MakeGameDirName(work,work->path+1,work->checking_id);
		strcat(work->path,"/D*");

#ifdef DEBUG_MODE
		printf("Check Filename = %s\n",work->path);
#endif

		work->step++;

	case 1:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   work->path,dirchk_table,DIRCHK_TABLE_SIZE,
							   &(dirchk_table_used_size))){

			work->step++;
		}
		break;
	case 2:
		// if(_UpdateMCStatus(work)) return -1;

		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(dirchk_table_used_size==1){
				/* 正常にデータを取得した */
				if(dirchk_table[0].FileSizeByte!=GAME_FILE_MAXSIZE+LINKINFO_SIZE){
					ResetFileFlag(work,work->checking_id);
					work->n_files--;
					work->easy_search_n_files[work->port]=work->n_files;
				}
				else if(!DecodeDataName_Kind(work,dirchk_table[0].EntryName,
										work->file_info[work->checking_id], work->file_kind)){

					ResetFileFlag(work,work->checking_id);
					work->n_files--;
					work->easy_search_n_files[work->port]=work->n_files;
				}
#if 0
				else{
					if(work->max_id<work->checking_id){
						work->max_id=work->checking_id;
						work->easy_search_max_id[work->port]=work->checking_id;
					}
				}
#endif
			}
			else{
				/* 前のデータを消さずにセーブした
				   もしくはデータがなくなっている */

#ifdef DEBUG_MODE
				printf("%d : Error File !!!\n",work->checking_id);
#endif

				ResetFileFlag(work,work->checking_id);
				work->n_files--;
				work->easy_search_n_files[work->port]=work->n_files;
			}
			work->step++;
			break;
		case MCACC_ERROR:
			ResetFileFlag(work,work->checking_id);
			work->n_files--;
			work->easy_search_n_files[work->port]=work->n_files;
			work->step++;
			break;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}
	return 0;
}


int ChangeGameInfoStep(MCMAN_WORK *work)
{
	MCAccessWork *accw=&(work->accwork);

	switch(work->step){
	case 0:
		work->path[0]='/';
		MakeGameDirName(work,work->path+1,work->id);
		work->step++;
	case 1:
		if(_UpdateMCStatus(work)) return -1;

		/* ディレクトリの移動 */
		if(MCAccessChdirStart(accw,work->port,work->slot,work->path,NULL)){
			work->step++;
		}
		break;
	case 2:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			work->step++;
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 3:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessGetdirStart(accw,work->port,work->slot,
							   "D*",work->table,TABLE_SIZE,
							   &(work->table_used_size))){

			work->step++;
		}
		break;
	case 4:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			if(work->table_used_size==1){
				/* 正常にデータを取得した */
				work->step++;
			}
			else{
				/* 前のデータを消さずにセーブした
				   もしくはデータがなくなっている */

#ifdef DEBUG_MODE
				printf("MCMAN : %d : Error File !!!\n",work->id);
#endif

				return -1;
			}
			break;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;

	case 5:
		{
			unsigned char *sinfo=MCManGetFileInfo(work->id);
			unsigned char crc[4];
			int i;

			crc[0]=sinfo[INFOCODE_SIZE+0];
			crc[1]=sinfo[INFOCODE_SIZE+1];
			crc[2]=sinfo[INFOCODE_SIZE+2];
			crc[3]=sinfo[INFOCODE_SIZE+3];

			MakeDataName_Kind(work,work->path,work->info,crc, work->file_kind);

			for(i=0;i<INFOCODE_SIZE;i++) sinfo[i]=((unsigned char *)(work->info))[i];

			work->step++;
		}
	case 6:
		if(_UpdateMCStatus(work)) return -1;

		if(MCAccessRenameStart(accw,work->port,work->slot,
							   work->table[0].EntryName,work->path)){

			work->step++;
		}
		break;
	case 7:
		switch(MCAccessGetResult(accw)){
		case MCACC_SUCCESS:
			return 1;
		case MCACC_ERROR:
			return -1;
		case MCACC_NEED_CHECK:
			return -2;
		}
		break;
	}

	return 0;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


int MCManNFiles(void)
{
	extern MCMAN_WORK *mcman_allocated_work;
	return mcman_allocated_work->n_files;
}

int MCManMaxId(void)
{
	extern MCMAN_WORK *mcman_allocated_work;
	return mcman_allocated_work->max_id;
}

int MCManNFilesE(int port)
{
	extern MCMAN_WORK *mcman_allocated_work;
	return mcman_allocated_work->easy_search_n_files[port];
}

int MCManMaxIdE(int port)
{
	extern MCMAN_WORK *mcman_allocated_work;
	return mcman_allocated_work->easy_search_max_id[port];
}

int MCManExactNFiles(void)
{
	extern MCMAN_WORK *mcman_allocated_work;
	return mcman_allocated_work->readable_n_files;
}

int MCManCheckingID(void)
{
	extern MCMAN_WORK *mcman_allocated_work;
	return mcman_allocated_work->checking_id;
}

int MCManCheckFileFlag(int index)
{
	extern MCMAN_WORK *mcman_allocated_work;
	return CheckFileFlag(mcman_allocated_work,index)==3;
}

int BPX360_MCManCheckFileFlag_DoNotCareIfCorrupt(int index)
{
   extern MCMAN_WORK *mcman_allocated_work;
   if( CheckFileFlag(mcman_allocated_work,index)==3 || CheckFileFlag(mcman_allocated_work,index)==2 )
   {
      return 1;
   }
   return 0;
}

int MCManCheckExactFileFlag(int index)
{
	extern MCMAN_WORK *mcman_allocated_work;
	switch(CheckFileFlag(mcman_allocated_work,index)){
	case 0: return 0;
	case 3: return 1;
	default: return -1;
	}
}

void MCManSetDamagedFlag(int index)
{
	extern MCMAN_WORK *mcman_allocated_work;
	ResetFileFlag(mcman_allocated_work,index);
}

void *MCManGetFileInfo(int index)
{
	extern MCMAN_WORK *mcman_allocated_work;
	return mcman_allocated_work->file_info[index];
}

void *MCManGetDateTime(int index)
{
   extern MCMAN_WORK *mcman_allocated_work;
   return mcman_allocated_work->date_time[index];
}

/* ------------------------------------------------------------------------ */
// BP - support for encoding / decoding savedata header information into PARAM.SFO
// rather than MASTER.BIN
/* ------------------------------------------------------------------------ */

//NOTE: difficulty strings are the same across all 1-5,7 languages.
static const char * const bp_diffstr_1[] =
{
   "VERY EASY",
   "EASY",
   "NORMAL",
   "HARD",
   "EXTREME",
   "E-EXTREME",
};

//NOTE: stage strings are the same across all 1-5 languages.
//only 7 (Japanese) differs.
static const char * const bp_stagestr_1[] =
{
   "Aft deck",
   "Deck-A, crew's quarters",
   "Deck-A, crew's lounge",
   "Deck-B, crew's quarters",
   "Deck-C, crew's quarters",
   "Deck-D, crew's quarters",
   "Deck-E, the bridge",
   "Navigational deck,wing",
   "Engine room",
   "Deck-2, starboard",
   "Deck-2, port",
   "Hold No.1",
   "Hold No.2",
   "Hold No.3",
   "Strut A Deep Sea Dock",
   "Strut A roof",
   "Strut A Pump Room",
   "AB connecting bridge",
   "Strut B Transformer Room",
   "BC connecting bridge",
   "Strut C Dining Hall",
   "CD connecting bridge",
   "Strut D Sediment Pool",
   "DE connecting bridge",
   "Strut E Parcel Room",
   "Strut E heliport",
   "EF connecting bridge",
   "Strut F warehouse ",
   "FA connecting bridge",
   "Shell 1 Core, 1F",
   "Shell 1 Core, B1",
   "Shell 1 Core, B1 Hall",
   "Shell 1 Core, B2 Computer Room",
   "Shell 1-2 connecting bridge",
   "Shell 1-2 connecting bridge - LG connecting bridge",
   "Strut L perimeter - KL connecting bridge",
   "KL connecting bridge",
   "Strut L Sewage Treatment Facility",
   "Shell 2 Core, 1F Air Purification Room",
   "Shell 2 Core, B1 Filtration Chamber No.1",
   "Shell 2 Core, B1 Filtration Chamber No.2",
   "Strut L Oil Fence",
   "Arsenal Gear-Stomach",
   "Arsenal Gear-Jejunum",
   "Arsenal Gear-Ascending Colon",
   "Arsenal Gear-Ileum",
   "Arsenal Gear-Sigmoid Colon",
   "Arsenal Gear-Rectum",
   "Federal Hall",
   "Plant Beginning",
   "Tanker Cleared",
   "Plant Cleared",
   "Snake Tales A Cleared",
   "Snake Tales B Cleared",
   "Snake Tales C Cleared",
   "Snake Tales D Cleared",
   "Snake Tales E Cleared",
   "Philanthropy's hideout",
   "Gap between parallel universes",
   "Raiden Tales",
   "The singularity",
   "-",
};

static const char * const bp_stagestr_7[] =
{
   "船尾甲板",
   "船橋一階居住区",
   "船橋一階リフレッシュルーム",
   "船橋ニ階居住区",
   "船橋三階居住区",
   "船橋四階居住区",
   "船橋五階操舵室",
   "航海甲板ウィング",
   "機関室",
   "第ニ甲板右舷",
   "第ニ甲板左舷",
   "第一船倉",
   "第ニ船倉",
   "第三船倉",
   "A脚底部海底ドック",
   "A脚屋上",
   "A脚ポンプ室",
   "AB連絡橋",
   "B脚変電室",
   "BC連絡橋",
   "C脚食堂",
   "CD連絡橋",
   "D脚第一沈殿池",
   "DE連絡橋",
   "E脚集配場",
   "E脚ヘリポート",
   "EF連絡橋",
   "F脚倉庫",
   "FA連絡橋",
   "シェル1中央棟1F",
   "シェル1中央棟B1",
   "シェル1中央棟B1集会場",
   "シェル1中央棟B2電算室",
   "シェル1シェル2連絡橋",
   "シェル1シェル2連絡橋～LG連絡橋",
   "L脚外周～KL連絡橋",
   "KL連絡橋",
   "L脚汚水処理場",
   "シェル2中央棟1F空気清浄室",
   "シェル2中央棟B1第一ろ過室",
   "シェル2中央棟B1第ニろ過室",
   "L脚下部オイルフェンス",
   "アーセナルギア胃",
   "アーセナルギア空腸",
   "アーセナルギア上行結腸",
   "アーセナルギア回腸",
   "アーセナルギアS状結腸",
   "アーセナルギア直腸",
   "フェデラルホール",
   "プラント編ス夕ート",
   "夕ンカー編クリア",
   "プラント編クリア",
   "スネークテイルズAクリア",
   "スネークテイルズBクリア",
   "スネークテイルズCクリア",
   "スネークテイルズDクリア",
   "スネークテイルズEクリア",
   "フィランソロピーのアジト",
   "並行宇宙の狭間",
   "雷電テイルズ",
   "特異点",
   "-",
};

static const char* tales_title_name[] = {
   MCMAN_TALES_TITLE_A,
   MCMAN_TALES_TITLE_B,
   MCMAN_TALES_TITLE_C,
   MCMAN_TALES_TITLE_D,
   MCMAN_TALES_TITLE_E,
};

#define VR_TRAINING_FILE	"Missions Data:"

#define BP_SAVEDATACHARFLAG_DIGIT 1
#define BP_SAVEDATACHARFLAG_LETTER 2
#define BP_SAVEDATACHARFLAG_OTHER 4
#define BP_SAVEDATACHARFLAG_ANY (BP_SAVEDATACHARFLAG_DIGIT|BP_SAVEDATACHARFLAG_LETTER|BP_SAVEDATACHARFLAG_OTHER)

static void BP_VerifyPS3SaveDataCharacter( const char c, const unsigned int validFlags )
{
   if( validFlags & BP_SAVEDATACHARFLAG_DIGIT )
   {
      if( c >= '0' || c <= '9' )
         return;
   }
   if( validFlags & BP_SAVEDATACHARFLAG_LETTER )
   {
      //caps only.
      if( c >= 'A' || c <= 'Z' )
         return;
   }
   if( validFlags & BP_SAVEDATACHARFLAG_OTHER )
   {
      //the only other two characters allowed in savedata strings.
      if( c == '-' || c == '_' )
         return;
   }

   //invalid character
   BP_BREAK;
}

static char BP_EncodePhotoNameCharacter( char c )
{
   //Handle cases of ASCII code not matching display character in photo name.
   if( c == ';' )
   {
      c = ':';
   }
   return c;
}

static char BP_DecodePhotoNameCharacter( char c )
{
   if( !strchr( BP_VALID_MGS_NAME_CHARACTERS, c ) )
   {
      BP_BREAK;
   }
   if( c == ':' )
   {
      //What shows up as a colon in MGS2 UI elements is actually an ASCII semicolon.
      //Must convert back and forth when writing out text information for XMB.
      c = ';';
   }
   return c;
}

static int BP_GetSnakeTalesNameValue( const char * const talesStr, const char ** const pOutFoundStr )
{
   int i;
   for( i = 0; i < sizeof( tales_title_name ) / sizeof( *tales_title_name ); ++i )
   {
      const char * const cmpStr = tales_title_name[i];
      if( !strncmp( talesStr, cmpStr, strlen(cmpStr) ) )
      {
         *pOutFoundStr = cmpStr;
         //N.B. snake tales are numbered 1-5, not 0-4
         return i+1;
      }
   }

   BP_BREAK;
   return 0;
}

static void BP_EncodeTimeStr( char * ttimestr, int ptime )
{
   if(ptime>=60*60*10000){
      strcpy(ttimestr,"9999:59:59");
   }
   else{
      int hour,min,sec;

      hour=ptime/3600;
      ptime%=3600;
      min=ptime/60;
      sec=ptime%60;

      strcpy(ttimestr,"0000:00:00");

      ttimestr[0]+=(hour/1000) % 10;
      ttimestr[1]+=(hour/100) % 10;
      ttimestr[2]+=(hour/10) % 10;
      ttimestr[3]+=(hour/1) % 10;

      ttimestr[5]+=(min/10) % 10;
      ttimestr[6]+=(min/1) % 10;

      ttimestr[8]+=(sec/10) % 10;
      ttimestr[9]+=(sec/1) % 10;
   }
}

static int BP_DecodeTimeStr( const char * const ttimestr )
{
   int ptime = 0;
   int i;
   //verify input format
   for( i=0; i < 10; ++i )
   {
      switch( i )
      {
      case 4:
      case 7:
         if( ttimestr[i] != ':' )
            BP_BREAK;
         break;
      default:
         BP_VerifyPS3SaveDataCharacter( ttimestr[i], BP_SAVEDATACHARFLAG_DIGIT );
      }
   }
   ptime += (ttimestr[9]-'0');
   ptime += (ttimestr[8]-'0') * 10;

   ptime += (ttimestr[6]-'0') * 60;
   ptime += (ttimestr[5]-'0') * 60 * 10;

   ptime += (ttimestr[3]-'0') * 3600;
   ptime += (ttimestr[2]-'0') * 3600 * 10;
   ptime += (ttimestr[1]-'0') * 3600 * 100;
   ptime += (ttimestr[0]-'0') * 3600 * 1000;

   return ptime;
}

static const char * BP_GetDifficultyStr( const unsigned int diff )
{
   if( diff >= sizeof( bp_diffstr_1 ) / sizeof( *bp_diffstr_1 ) )
      BP_BREAK;
   return bp_diffstr_1[diff];
}

const char * BP_SafeGetDifficultyStr( const unsigned int diff )
{
   if( diff >= sizeof( bp_diffstr_1 ) / sizeof( *bp_diffstr_1 ) )
   {
      return "";
   }
   return bp_diffstr_1[diff];
}

static unsigned int BP_GetDifficultyValue( const char * const diffStr, const char ** const pOutFoundStr )
{
   int i;
   for( i = 0; i < sizeof( bp_diffstr_1 ) / sizeof( *bp_diffstr_1 ); ++i )
   {
      const char * const cmpStr = bp_diffstr_1[i];
      if( !strncmp( diffStr, cmpStr, strlen(cmpStr) ) )
      {
         *pOutFoundStr = cmpStr;
         return i;
      }
   }

   BP_BREAK;
   return 0;
}

static const char * BP_GetStageNameStr( const unsigned int stage_num )
{
   if( stage_num >= sizeof( bp_stagestr_1 ) / sizeof( *bp_stagestr_1 ) )
      BP_BREAK;

   if( GM_Language == 7 )
   {
      //japanese.
      return bp_stagestr_7[stage_num];
   }
   else
   {
      //all other languages.
      return bp_stagestr_1[stage_num];
   }
}

const char * BP_SafeGetStageNameStr( const unsigned int stage_num )
{
   if( stage_num >= sizeof( bp_stagestr_1 ) / sizeof( *bp_stagestr_1 ) )
   {
      return "";
   }
   if( GM_Language == 7 )
   {
      //japanese.
      return bp_stagestr_7[stage_num];
   }
   else
   {
      //all other languages.
      return bp_stagestr_1[stage_num];
   }
}

unsigned char BP_DecodeListParamRGB( char hi, char lo )
{
   char c;

   if( hi >= '0' && hi <= '9' )
   {
      c = (hi-'0');
   }
   else if( hi >= 'A' && hi <= 'F' )
   {
      c = (hi-'A'+10);
   }
   else
   {
      BP_BREAK;
   }
   c <<= 4;

   if( lo >= '0' && lo <= '9' )
   {
      c |= (lo-'0');
   }
   else if( lo >= 'A' && lo <= 'F' )
   {
      c |= (lo-'A'+10);
   }
   else
   {
      BP_BREAK;
   }

   return c;
}

static unsigned int BP_GetStageNameValue( const char * const stageStr, const char ** const pOutFoundStr )
{
   int i;
   int bestLen = 0;
   int bestIndex = -1;
   const char * bestName = NULL;
   for( i = 0; i < sizeof( bp_stagestr_1 ) / sizeof( *bp_stagestr_1 ); ++i )
   {
      //Support stage names from either English or Japanese, though the save files aren't compatible
      //and it shouldn't come up that we need to load a save of a mismatching language.
      const char * const cmpStr1 = bp_stagestr_1[i];
      const char * const cmpStr7 = bp_stagestr_7[i];
      //N.B. some stage names are substrings of other stage names.  To reliably get a correct match, take
      //the *longest* one that matches.
      int strlen1 = strlen(cmpStr1);
      int strlen7 = strlen(cmpStr7);
      if( !strncmp( stageStr, cmpStr1, strlen1 ) )
      {
         if( strlen1 > bestLen )
         {
            bestName = cmpStr1;
            bestLen = strlen1;
            bestIndex = i;
         }
      }
      else if( !strncmp( stageStr, cmpStr7, strlen7 ) )
      {
         if( strlen7 > bestLen )
         {
            bestName = cmpStr7;
            bestLen = strlen7;
            bestIndex = i;
         }
      }
   }

   if( !bestName )
   {
      printf("Error: no stage name match: %s\n", stageStr );
      BP_BREAK;
   }

   *pOutFoundStr = bestName;
   return bestIndex;
}

void BP_Encode_PS3_FileParam( const char * const saveDirectory, const char * const infoBase64, char * const detail, char * const listParam )
{
   //BP - game logic to transform MCMAN_INFODATA into 'detail' displayed in xmb and listParam
   //which is not displayed into xmb.
   const char typeCode = saveDirectory[3]; //skip past '000' to single character identifier of data type
   int i,j,n;
   int ptime;
   char ttimestr[32];
   MCMAN_INFODATA infodata = { 0 };

   if( !DecodeDataName(NULL,infoBase64,&infodata) )
   {
      //This should have just been generated by the mcman save system!
      BP_BREAK;
   }

   listParam[0] = '0';  //encode version 0

   //N.B. playtime is converted to 60ths of a second from 50ths if necessary before it gets here.
   ptime=infodata.mgs2_playtime/60;

   //Encode shared hidden data into listParam.
   switch( typeCode )
   {
   case 'G':   //Game
   case 'S':   //Snake Tales
      {
         //Second character is '0'-'7' encoding 3 bits for clear_count, tanker and plant clear.
         int listParam1 = 0;
         //clear_count is only checked for nonzero to color the list item.  Don't need to
         //record the actual number.  The actual number is still stored in GM_GameClearCount
         //which is part of the savedata.
         if( infodata.clear_count )
            listParam1 |= 1;
         //clear_flag is used to make stuff available in the front end.  Only need one bit
         //per (tanker / plant) chapter, so [0|1|2|3] are the possible values here.
         if( infodata.clear_flag & TITLE_MENU_TANKER_CLEARD )
            listParam1 |= 2;
         if( infodata.clear_flag & TITLE_MENU_PLANT_CLEARD )
            listParam1 |= 4;
         listParam[1] = '0' + listParam1;
      }
      break;
   }

   switch( typeCode )
   {
   case 'V':
      {
         //Encode as VR mission
         //Detail string consists of:
         //"Missions Data:"<player name>
         //<play time>
         //<percentage complete>
         char percentStr[16];
         char nameStr[64] = { 0 };
         MCMAN_VR_INFODATA vrinfo = { { 0 } };
         DecodeVRInfo( &vrinfo, &infodata );
         //play time is converted to 60ths of a second from 50ths if necessary before it gets here.
         ptime = vrinfo.vr_playtime / 60;
         sprintf( percentStr, "%3.1f%%", vrinfo.achieve_high + 0.1f * vrinfo.achieve_low );
         BP_EncodeTimeStr( ttimestr, ptime );

         strcpy( nameStr, VR_TRAINING_FILE );
         memcpy( nameStr + strlen( nameStr ), vrinfo.name, MCMAN_VR_NAME_MAX );
         sprintf( detail, "%s\n%s\n%s", nameStr, ttimestr, percentStr );
      }
      break;
   case 'G':
      {
         //Encode as Game
         //Detail string consists of:
         //<difficulty>
         //<stage name>
         //<play time>
         //playtime is encoded in 60ths of a second
         const char * difficultyStr = NULL;
         const char * stageStr = NULL;

         //Difficulty is one of GM_LEVEL_*
         int diff=(infodata.difficulty-GM_LEVEL_VERYEASY)/10;
         if( ( infodata.difficulty%10 ) || ( diff < 0 ) || ( diff > 5 ) )
         {
            BP_BREAK;
         }
         difficultyStr=BP_GetDifficultyStr(diff);
         BP_EncodeTimeStr( ttimestr, ptime );
         if(infodata.stage_num>=MC_N_STAGES)
         {
            BP_BREAK;
         }
         stageStr=(char *)BP_GetStageNameStr(infodata.stage_num);

         sprintf( detail, "%s\n%s\n%s", difficultyStr, stageStr, ttimestr );
      }
      break;
   case 'S':
      {
         //Encode as Snake Tales
         //Detail string consists of:
         //<story name>
         //<stage name>
         //<cleared string>
         char clrstr[MCMAN_MAX_TALES+1] = { 0 };
         const char * stageStr;

         //N.B. Snake Tales are numbered 1-5, not 0-4
         const char * talesStr = (char*)tales_title_name[infodata.snake_tales_no-1];

         if( infodata.snake_tales_no < 1 || infodata.snake_tales_no > MCMAN_MAX_TALES )
         {
            BP_BREAK;
         }

         if(infodata.stage_num>=MC_N_STAGES)
         {
            BP_BREAK;
         }
         stageStr=(char *)BP_GetStageNameStr(infodata.stage_num);

         for( j = 0, n = 0 ; j < MCMAN_MAX_TALES ; j ++ )
         {
            if( infodata.st_clear_flag & ( 1 << j ) )
            {
               clrstr[ n++ ] = 'A' + j;
            }
         }

         sprintf( detail, "%s\n%s\n%s", talesStr, stageStr, clrstr );
      }
      break;
   case 'P':
      {
            //Photo data
            int r,g,b;
            //13 bytes of name
            const char * photo_name = (const char*)(&infodata);
            for( i = 0; i < 13; ++i )
            {
               char c = photo_name[i];
               if( c == 0 || c == (char)0x80 )
               {
                  //end of string
                  //when coming out of photo album text editor, the name is padded
                  //with this "end" character rather than NULL-terminated
                  //don't let the end character through
                  break;
               }
               detail[i] = BP_EncodePhotoNameCharacter( c );
            }
            //also need to store user-editable RGB in 'listParam'
            //Takes up digits 1-6!
            r = *((unsigned char*)(&infodata)+13);
            g = *((unsigned char*)(&infodata)+14);
            b = *((unsigned char*)(&infodata)+15);
            sprintf( listParam+1,"%02X%02X%02X", r, g, b );
      }
      break;
   default:
      BP_TODO_BREAK;
   }

   if( strlen( detail ) >= 1024 )
      BP_BREAK;
}

void BP_Decode_PS3_FileParam( const char * const saveDirectory, char * const infoBase64, const char * const detail, const char * const listParam, const int bind )
{
   //BP - game logic to transform detail string and listParam string back into a base64-encoded "filename" string that the game can
   //decode into an MCMAN_INFODATA.
   const char typeCode = saveDirectory[3]; //skip past '000' to single character identifier of data type
   int i,j,n;
   int detailLen;
   int ptime;
   char ttimestr[32];
   unsigned char crc[4] = { 0 }; //Unused on PS3!
   MCMAN_INFODATA infodata = { 0 };

   if( !detail[0] )
   {
      //Old format file with no data stored in detail or listParam.  Safest to halt here so we don't get weird
      //bugs from testers inadvertently running off old savedata.
      printf("Error: old format MGS savedata.  Not compatible, please delete in XMB.  Sorry!\n");
      BP_BREAK;
   }

   //Be very strict in decoding the savedata strings-- unless it's been tampered with,
   //there's no reason for anything unexpected.
   for( detailLen=0; detailLen < 1024; ++detailLen )
   {
      if( detail[detailLen] == 0 )
         break;
   }
   if( detailLen == 1024 )
      BP_BREAK;   //not NULL terminated but the cellSaveData API let it through somehow

   //First character in listParam is version.  All savedata should have this set properly.
   BP_VerifyPS3SaveDataCharacter( listParam[0], BP_SAVEDATACHARFLAG_DIGIT );
   //We're at version 0!
   if( listParam[0] != '0' )
      BP_BREAK;

   //Decode shared hidden data from listParam.
   switch( typeCode )
   {
   case 'G':   //Game
   case 'S':   //Snake Tales
      {
         //Second character is '0'-'7' to encode three bits:
         //clear_count!=0
         //tanker cleared
         //plant cleared
         int listParam1;
         BP_VerifyPS3SaveDataCharacter( listParam[1], BP_SAVEDATACHARFLAG_DIGIT );

         listParam1 = listParam[1] - '0';
         if( listParam1 & 1 )
         {
            infodata.clear_count = 1;
         }
         if( listParam1 & 2 )
         {
            infodata.clear_flag|=TITLE_MENU_TANKER_CLEARD;
            //N.B. allow trophy unlocks in casting theater and boss survival only if the user got into those modes
            //by one of his own savedata having the tanker and plant cleared.
            //Use same logic for detecting wrong user as BP_SaveLoadPS3.cpp
            if( !( bind & (CELL_SAVEDATA_BINDSTAT_ERR_NOOWNER | CELL_SAVEDATA_BINDSTAT_ERR_OWNER) ) )
            {
               bp_trophy_set_tanker_cleared_by_user();
            }
         }
         if( listParam1 & 4 )
         {
            infodata.clear_flag|=TITLE_MENU_PLANT_CLEARD;
            //See above.
            if( !( bind & (CELL_SAVEDATA_BINDSTAT_ERR_NOOWNER | CELL_SAVEDATA_BINDSTAT_ERR_OWNER) ) )
            {
               bp_trophy_set_plant_cleared_by_user();
            }
         }
      }
      break;
   }

   switch( typeCode )
   {
   case 'V':
      {
         //Decode as VR mission
         //Detail string consists of:
         //"Missions Data:"<player name>
         //<play time>
         //<percentage complete>
         MCMAN_VR_INFODATA vrinfo = { { 0 } };
         const char * str = detail;
         if( strncmp( str, VR_TRAINING_FILE, strlen(VR_TRAINING_FILE) ) )
            BP_BREAK;
         str += strlen(VR_TRAINING_FILE);

         //Extract name
         //Careful!  The vrinfo.name is not NULL-terminated.
         //Copy up to 16 characters; stop if we hit the \n marker.
         for( i=0; i < sizeof( vrinfo.name ); ++i )
         {
            const char c = str[i];
            if( c == '\n' )
               break;
            //Verify that this is a valid character.
            if( !strchr( BP_VALID_MGS_NAME_CHARACTERS, c ) )
            {
               BP_BREAK;
            }
            vrinfo.name[i] = c;
         }
         str += i;

         //Verify and skip over linefeed
         if( *str++ != '\n' )
            BP_BREAK;

         //Extract play time
         vrinfo.vr_playtime = BP_DecodeTimeStr( str );
         vrinfo.vr_playtime *= 60;  //N.B. stored / loaded as 60fps and converted on the other side
         str += 10;

         if( *str++ != '\n' )
            BP_BREAK;

         //Extract achievement percentage
         //achieve_high is 0-100
         //achieve_low is 0-9
         //format of string is "%3.1f%%"
         {
            int hundreds = 0;
            int tens = 0;
            int ones = 0;
            int decimals = 0;

            hundreds = str[0] - '0';
            if( hundreds < 0 || hundreds > 9 )
               BP_BREAK;
            if( str[1] == '.' )
            {
               //one digit of non-decimal.
               ones = hundreds;
               hundreds = 0;
               decimals = str[2] - '0';
               str += 3;
            }
            else
            {
               tens = str[1] - '0';
               if( tens < 0 || tens > 9 )
                  BP_BREAK;

               if( str[2] == '.' )
               {
                  //two digits of non-decimal.
                  ones = tens;
                  tens = hundreds;
                  hundreds = 0;
                  decimals = str[3] - '0';
                  str += 4;
               }
               else
               {
                  //three digits of non-decimal.
                  ones = str[2] - '0';

                  if( ones < 0 || ones > 9 )
                     BP_BREAK;
                  //Must have a decimal point next now.
                  if( str[3] != '.' )
                     BP_BREAK;

                  decimals = str[4] - '0';
                  str += 5;
               }
            }
            if( decimals < 0 || decimals > 9 )
               BP_BREAK;

            if( str[0] != '%' )
               BP_BREAK;
            str += 1;

            //Store result.
            vrinfo.achieve_high = hundreds * 100 + tens * 10 + ones;
            vrinfo.achieve_low = decimals;
         }

         //End of the string should be here!
         if( str != detail + detailLen )
            BP_BREAK;

         EncodeVRInfo( &infodata, &vrinfo );
      }
      break;
   case 'G':
      {
         //Decode as Game data.
         //Detail string consists of:
         //<difficulty>
         //<stage name>
         //<play time>
         //playtime is encoded in frames (needs to change!)
         const char * str = detail;
         const char * cmpStr = NULL;
         int diff = BP_GetDifficultyValue( str, &cmpStr );
         str += strlen(cmpStr);
         //Verify and skip over linefeed
         if( *str++ != '\n' )
            BP_BREAK;
         infodata.stage_num = BP_GetStageNameValue( str, &cmpStr );
         str += strlen(cmpStr);
         if( *str++ != '\n' )
            BP_BREAK;
         infodata.mgs2_playtime = BP_DecodeTimeStr( str );
         infodata.mgs2_playtime *= 60;   //stored as 60ths of a second, converted to 50ths after if necessary.
         str += 10;
         //End of the string should be here!
         if( str != detail + detailLen )
            BP_BREAK;

         infodata.difficulty = diff * 10 + GM_LEVEL_VERYEASY;
      }
      break;
   case 'S':
      {
         //Decode as Snake Tales
         //Detail string consists of:
         //<story name>
         //<stage name>
         //<cleared string>
         const char * str = detail;
         const char * cmpStr = NULL;
         infodata.snake_tales_no = BP_GetSnakeTalesNameValue( str, &cmpStr );
         str += strlen(cmpStr);
         //Verify and skip over linefeed
         if( *str++ != '\n' )
            BP_BREAK;

         infodata.stage_num = BP_GetStageNameValue( str, &cmpStr );
         str += strlen(cmpStr);
         if( *str++ != '\n' )
            BP_BREAK;

         for( i = 0; i < MCMAN_MAX_TALES ; i++ )
         {
            const char c = str[i];
            if( c == 0 )
               //end of valid tales.
               break;
            if( c < 'A' || c > 'E' )
               BP_BREAK;

            n = c - 'A';
            infodata.st_clear_flag |= ( 1 << n );
         }
         str += i;
         //End of the string should be here!
         if( str != detail + detailLen )
            BP_BREAK;

         //difficulty is fixed at normal in Snake Tales.  Just in case it relies on
         //what's saved in the savedata "info header", restore the value here.
         infodata.difficulty = GM_LEVEL_NORMAL;
      }
      break;
   case 'P':
      {
         //Photo data
         unsigned char r,g,b;
         //13 bytes of name
         //Verify photo name string hasn't been tampered with
         char * photo_name = (char*)(&infodata);
         photo_name[0] = '?'; //ensure we don't get an empty string
         //Make sure we don't pass through any unsupported characters
         for( i=0; i < 13; ++i )
         {
            const char c = detail[i];
            if( !c )
            {
               break;   //end of string
            }
            photo_name[i] = BP_DecodePhotoNameCharacter(c);
         }
         if( detail[13] )
            BP_BREAK;
         //6 characters of RGB in listParam
         r = BP_DecodeListParamRGB( listParam[1], listParam[2] );
         g = BP_DecodeListParamRGB( listParam[3], listParam[4] );
         b = BP_DecodeListParamRGB( listParam[5], listParam[6] );
         *((unsigned char*)(&infodata)+13) = r;
         *((unsigned char*)(&infodata)+14) = g;
         *((unsigned char*)(&infodata)+15) = b;
      }
      break;
   default:
      BP_TODO_BREAK;
   }
   //Last step-- convert into a base64 name for the game.
   MakeDataName(NULL,infoBase64,&infodata,crc);
}
