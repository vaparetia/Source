//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	mcdata.c
		ゲームデータの取得、展開
	
	2001/06/06 K.Kano
	$Id: mcdata.c,v 1.1.1.3 2002/11/19 11:43:16 Yoshizawa1 Exp $
*/
#endif

#ifdef KP_XBOX
#include	<xtl.h>
#endif

#include "gameheader.h"
#include "libgv.cnf"
#include "game.h"
#include "mcman.h"
#include "../titlescr/titlescr.h"
#include "libfs.h"

#include <time.h>

#ifdef __GNUC__
#define GetLinkVariable( _varname, _vartop ) \
	(*(typeof(_varname) *)((int)_vartop+(((int)&_varname)-((int)linkvarbuf))))
#define SetLinkVariable( _varname, _vartop, _data ) \
	((*(typeof(_varname) *)((int)_vartop+(((int)&_varname)-((int)linkvarbuf)))) = _data )
#else
// short, int にしか対応してない
#define GetLinkVariable( _varname, _vartop ) \
(sizeof(_varname) == 2 ? \
 (*(short *)((int)(_vartop)+(((int)&(_varname))-((int)linkvarbuf)))) \
 : (*(int *)((int)(_vartop)+(((int)&(_varname))-((int)linkvarbuf)))) )

#define SetLinkVariable( _varname, _vartop, _data ) \
{	\
	if( sizeof( _varname ) == 2 ) {	\
		*(short *)((int)(_vartop)+(((int)&(_varname))-((int)linkvarbuf))) = (_data);\
	} else { \
		*(int *)((int)(_vartop)+(((int)&(_varname))-((int)linkvarbuf))) = (_data); \
	} \
}

#endif

extern int BP_FRAMES_PER_SEC(void);
#undef FRAMES_PER_SEC
#define FRAMES_PER_SEC		(BP_FRAMES_PER_SEC())

int BP_FramesSysToFrames60( int frames );
int BP_Frames60ToFramesSys( int frames60 );
extern void BP_MCManSetDateTime(unsigned char date_time[], time_t _dateTime );

#define ID1			((char)0xaa)
#define ID2			((char)0x55)


/* 適当 */
#define MAGIC_NUMBER	(0x3298)


/* 32 bit CRC 計算 */

#define CRCPOLY 0xEDB88320

unsigned int calc_crc_code( void *data, int len )
{
	unsigned int i, j;
	unsigned int r;
	char *p;

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
	return r ^ 0xFFFFFFFF;
}


/* Ｍ系列乱数 */
/* 多項式 : x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 +
            x^8 + x^7 + x^5 + x^4 + x^2 + x^1 + 1 */
static unsigned int codetable_func(unsigned int seed)
{
	int bit;

	bit=(seed>>(32-1))^(seed>>(26-1))^(seed>>(23-1))^(seed>>(22-1))^
		(seed>>(16-1))^(seed>>(12-1))^(seed>>(11-1))^(seed>>(10-1))^
		(seed>>(8-1))^(seed>>(7-1))^(seed>>(5-1))^(seed>>(4-1))^
		(seed>>(2-1))^(seed>>(1-1));
	seed=(seed<<1)|(bit & 1);

#ifdef DEBUG_MODE
	// printf("Rand = 0x%08x\n",seed);
#endif

	return seed;
}


/* 暗号化 */
void CodeData(void *data,int size,unsigned int *seed)
{
	unsigned char *s=(unsigned char *)data;
	int i;
	unsigned char pre_s;
	// unsigned char cur_s;

	pre_s=0;
	for(i=size;i>0;i--){
		*seed=codetable_func(*seed);
		// cur_s=*s;
		*s+=(pre_s+*seed);
		// pre_s=cur_s;
		pre_s=*s;
		s++;
	}
}

/* 暗号の解除 */
void DecodeData(void *data,int size,unsigned int *seed)
{
	unsigned char *s=(unsigned char *)data;
	int i;
	unsigned char pre_s,pre_s2;

	pre_s=0;
	for(i=size;i>0;i--){
		*seed=codetable_func(*seed);
		pre_s2=*s;
		*s-=(pre_s+*seed);
		// pre_s=*s;
		pre_s=pre_s2;
		s++;
	}
}

static void ConvertSjis( char* sjis, char* ascii )
{	
	int i=0,j=0;

	while(*(ascii+i)!='\0'){
		int c=*(unsigned char *)(ascii+i);

		if(c<0x20 || c>=0x7f) c=0x20;
		c=(c-0x20)*2;

		sjis[j+0]=*(ASCII_CODE_SJIS_STR+c+0);
		sjis[j+1]=*(ASCII_CODE_SJIS_STR+c+1);

		i++;
		j+=2;
	}
	sjis[j]='\0';
}


/* ゲーム用アイコンデータの生成 */
void MC_InitIconEnv(int id, int file_kind )
{
	static const int bgclr_u[]={   0,  0,  0,0x80, };
	static const int bgclr_l[]={  48, 20,  0,0x80, };
	static const FVECTOR tnk_dir={ -0.695837426f,
								   -0.477145663f
								   -0.536788871f,
								   0.0f, };
	static const FVECTOR tnk_lrgb={ 112.0f/255.0f,128.0f/255.0f,128.0f/255.0f,0.0f, };
	static const FVECTOR tnk_argb={ 16.0f/255.0f,32.0f/255.0f,28.0f/255.0f,0.0f, };
	static const FVECTOR plt_dir={ -0.749811749f,
								   -0.506629560f,
								   -0.425568831f,
								   0.0f, };
	static const FVECTOR plt_lrgb={ 164.0f/255.0f,144.0f/255.0f,112.0f/255.0f,0.0f, };
	static const FVECTOR plt_argb={ 44.0f/255.0f,64.0f/255.0f,64.0f/255.0f,0.0f, };

	static const FVECTOR dmy_dir={ 1.0f,0.0f,0.0f,0.0f, };
	static const FVECTOR dmy_lrgb={ 0.0f,0.0f,0.0f,0.0f, };

	int stage_num;
	char title[0x80];
	void *linkvar;
	int playtime;
	int hnum1000,hnum100,hnum10,hnum1;
	int mnum10,mnum1,snum10,snum1;
	int idnum10,idnum1;
	char timestr[0x20],idstr[0x08+1];
	int flag=0;

	switch( file_kind ) {
	case MCMAN_FILE_KIND_GAME:
		linkvar=GCL_GetLinkvarSaveAreaTop();
		stage_num=GetLinkVariable( GM_StageNum, linkvar );

		playtime=GM_PlayTime/FRAMES_PER_SEC;

#if 0
		if(playtime>=60*60*10000){
			playtime=60*60*9999+60*59+59;
		}
#else
		if(playtime>=60*60*100){
			playtime=60*60*99+60*59+59;
		}
#endif

		snum1=playtime%60;
		playtime/=60;
		mnum1=playtime%60;
		playtime/=60;
		hnum1=playtime;
		
		snum10=snum1/10;
		snum1%=10;
		mnum10=mnum1/10;
		mnum1%=10;
		hnum10=hnum1/10;
		hnum1%=10;
		hnum100=hnum10/10;
		hnum10%=10;
		hnum1000=hnum100/10;
		hnum100%=10;

		strcpy(timestr,MC_BASE_PLAY_TIME);

#if 0
		if(hnum1000>0){
			timestr[3]+=hnum1000;
			flag=1;
		}
		else{
			timestr[2]=timestr[0];
			timestr[3]=timestr[1];
		}
		if(hnum100>0 || flag){
			timestr[5]+=hnum100;
			flag=1;
		}
		else{
			timestr[4]=timestr[0];
			timestr[5]=timestr[1];
		}
		if(hnum10>0 || flag){
			timestr[7]+=hnum10;
			flag=1;
		}
		else{
			timestr[6]=timestr[0];
			timestr[7]=timestr[1];
		}
		timestr[9]+=hnum1;

		timestr[13]+=mnum10;
		timestr[15]+=mnum1;

		timestr[19]+=snum10;
		timestr[21]+=snum1;
#else
		if(hnum10>0){
			timestr[3]+=hnum10;
			flag=1;
		}
#if 0
		else{
			timestr[2]=timestr[0];
			timestr[3]=timestr[1];
		}
#endif
		timestr[5]+=hnum1;

		timestr[9]+=mnum10;
		timestr[11]+=mnum1;
#if 0
		timestr[15]+=snum10;
		timestr[17]+=snum1;
#endif
#endif


//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
		idnum10=(id/10) % 10;
		idnum1=id%10;

		strcpy(idstr,MC_BASE_ID);
		idstr[3]+=idnum10;
		idstr[5]+=idnum1;
//#endif


		strcpy(title,MGS2_TITLE);
//#if defined(JAPANESE_BP_IGNORE()) || defined(PAL)
		strcat(title,idstr);
		strcat(title,timestr+2);
//#else
//		strcat(title,timestr);
//#endif
		strcat(title,mc_title_stage_name[stage_num]);
		SetIconSysTitle(title,MGS2_TITLE_LF_LOCATE);
		break;
	case MCMAN_FILE_KIND_VR:
		{
			char buf0[0x80];
			char buf1[0x80];
			float fAchieve;

			strcpy( title, MGS2_VR_TITLE );
			
			fAchieve = TTL_CalcVRAchievementRatio();
			
			sprintf(buf0, "%3.1f%%", fAchieve );
			ConvertSjis( buf1, buf0 );
			strcat( title, buf1 );

			SetIconSysTitle(title,MGS2_VR_LF_LOCATE);
			break;
		}	
	case MCMAN_FILE_KIND_SNAKE_TALES:
		{
			char buf0[0x80];
			char buf1[0x80];

			strcpy( title, MGS2_ST_TITLE );
			sprintf(buf0, "%02d/%c", id, 'A'+((GCL_ReadVarRef( &TTL_ReferenceVariable[0], 0 ) - 1 )% 5 ) );
			ConvertSjis( buf1, buf0 );
			
			strcat( title, buf1 );

			SetIconSysTitle(title,MGS2_ST_LF_LOCATE);

			break;
		}
	}


	if(GM_Configuration & GM_CONFIG_STORY_TANKER){
		/* 指定順序 : lu ru ld rd */
		SetIconSysBgColor(bgclr_u,bgclr_u,bgclr_l,bgclr_l);

		SetIconSysLight(&tnk_dir,&dmy_dir,&dmy_dir,&tnk_lrgb,&dmy_lrgb,&dmy_lrgb);
		SetIconSysAmbient(&tnk_argb);
	}
	else{
		/* 指定順序 : lu ru ld rd */
		SetIconSysBgColor(bgclr_u,bgclr_u,bgclr_l,bgclr_l);

		SetIconSysLight(&plt_dir,&dmy_dir,&dmy_dir,&plt_lrgb,&dmy_lrgb,&dmy_lrgb);
		SetIconSysAmbient(&plt_argb);
	}
}


/* ---------------------------------------------------------------------- */
/*
	メモリーカードへのセーブ
*/


void SaveGameData(MCMAN_GAMEDATA *data,int id, int filetype)
{
	unsigned int crc;
	unsigned int seed;
	void *linkvar;
	void *varbuf;
	int gm_titlemenustatus;

#ifdef BP_PS2	// XBOX ではコードセクションリードで例外が発生する
	extern unsigned char _program_top;
	unsigned int *bseedp=(unsigned int *)(&_program_top+MAGIC_NUMBER+(GM_PlayTime & 0xfffc));
	while(*bseedp==0) bseedp++;
#else
   unsigned int bp_bseed = 0;
   unsigned int * bseedp = &bp_bseed;
#endif	

	memset( data, 0, sizeof( MCMAN_GAMEDATA ) );	// 一応０クリア
	
	linkvar = GCL_GetLinkvarSaveAreaTop();
	varbuf = GCL_GetVarSaveAreaTop();

	memcpy( data->linkvar, linkvar, MAX_LINKVARBUF );
	memcpy( data->varbuf, varbuf, MAX_VAR_BUF );
	if( filetype == MCMAN_FILE_KIND_VR ) {
		memcpy( data->vr_score_data, _MSN_SAVE_DATA, MSN_SAVE_DATA_SIZE );
	} else {
		memcpy( data->tanker_photo_data, GM_TankerPicture, GM_TANKER_PICTURE_SIZE );
	}

	/* プレイ時間を現在時に修正 */
#if 1 //BP - convert at savedata prep time to 60ths of a second
   {
      int bp_playTime60 = BP_FramesSysToFrames60(GM_PlayTime);
      printf("BP: convert saving GM_PlayTime: %d -> %d\n", GM_PlayTime, bp_playTime60 );
      SetLinkVariable( GM_PlayTime, data->linkvar, bp_playTime60 );
   }
#else
	SetLinkVariable( GM_PlayTime, data->linkvar, GM_PlayTime );
#endif

	/* セーブしてはいけないフラグがあるので、それの削除 */
	gm_titlemenustatus
			=GetLinkVariable( GM_TitleMenuStatus, data->linkvar);
	gm_titlemenustatus&=~TITLE_MENU_OPTION_CLEAR_MASK;

	/* 2001/9/12 K.Kano
	   実際にクリアしたデータのみにフラグを立てるように修正 */
	if( GetLinkVariable( GM_TankerClearCount, data->linkvar ) == 0 ) {

		gm_titlemenustatus&=~TITLE_MENU_TANKER_CLEARD;
	}
	if( GetLinkVariable( GM_PlantClearCount, data->linkvar) ==0){

		gm_titlemenustatus&=~TITLE_MENU_PLANT_CLEARD;
	}

	SetLinkVariable( GM_TitleMenuStatus, data->linkvar, gm_titlemenustatus);

	/* セーブ回数は現在のカウントを使用 */
	SetLinkVariable( GM_SaveCount, data->linkvar, GM_SaveCount);

	crc = calc_crc_code( data->linkvar, MAX_LINKVARBUF );
	crc ^= calc_crc_code( data->varbuf, MAX_VAR_BUF );

	data->id1=ID1;
	data->id2=ID2;

	memcpy( data->variable_version, &GCL_VariableVersion, sizeof( int ) );

#ifdef DEBUG_MODE
	printf("Game Data Version = 0x%08x\n",GCL_VariableVersion);
#endif

#ifdef PSX2	
	seed = *bseedp;
#else
	seed = BP_PS2_rand();
#endif	

	memcpy( data->crc, &crc, sizeof( int ) );
	memcpy( data->seed, &seed, sizeof( int ) );

	CodeData(data->linkvar,MAX_LINKVARBUF,&seed);
	CodeData(data->varbuf,MAX_VAR_BUF,&seed);
	if( filetype == MCMAN_FILE_KIND_VR ) {
		CodeData(data->vr_score_data, MSN_SAVE_DATA_SIZE, &seed );
	} else {
		CodeData(data->tanker_photo_data, GM_TANKER_PICTURE_SIZE, &seed );
	}
#if 0
	InitIconEnv(id);
#endif	
}

void SaveGameInfo(MCMAN_INFODATA *info)
{
	void *linkvar;
   time_t saveTime;
   char convertedTime[DATETIME_SIZE];

	// printf("Save : GM_TitleMenu = 0x%04x\n",GM_TitleMenuStatus);

	/* 追加情報。ここにはファイル選択画面で出す情報を入れる */
#if 1
	memset(info,0x00,sizeof(MCMAN_INFODATA));

	linkvar = GCL_GetLinkvarSaveAreaTop();

	info->mgs2_playtime=GM_PlayTime;
	info->stage_num
			= GetLinkVariable( GM_StageNum, linkvar );
	info->difficulty
			= GetLinkVariable( GM_GameLevel, linkvar );
	info->clear_count
			= GetLinkVariable( GM_GameClearCount, linkvar );
	info->clear_flag=GM_TitleMenuStatus & ~TITLE_MENU_OPTION_CLEAR_MASK;

	/* 2001/9/12 K.Kano
	   実際にクリアしたデータのみにフラグを立てるように修正 */
	if( GetLinkVariable( GM_TankerClearCount, linkvar ) ==0){
		
		info->clear_flag&=~TITLE_MENU_TANKER_CLEARD;
	}
	if( GetLinkVariable( GM_PlantClearCount, linkvar ) ==0){

		info->clear_flag&=~TITLE_MENU_PLANT_CLEARD;
	}

   saveTime = time( NULL );

   BP_MCManSetDateTime( convertedTime, saveTime );

   info->save_date = convertedTime[DATETIME_INDEX_YEAR] << 9;
   info->save_date |= convertedTime[DATETIME_INDEX_MONTH] << 5;
   info->save_date |= convertedTime[DATETIME_INDEX_DAY] & 0x1F;

#else
	memcpy(info,"XXXX" "XXXX" "XXXX" "XXXX",sizeof(MCMAN_INFODATA));
#endif

}

void SaveVRInfo( MCMAN_VR_INFODATA* pInfo )
{
	float fAchieve;
	
	memcpy( pInfo->name, GM_MyName, MCMAN_VR_NAME_MAX );
	pInfo->vr_playtime = GM_PlayTime;
	
	fAchieve = TTL_CalcVRAchievementRatio();
	pInfo->achieve_high = (int)fAchieve;
	pInfo->achieve_low = (int)(( fAchieve - (float)((int)fAchieve))  * 10.f );
}


#ifdef PSX2
// PS2 では 16byte にデータをおさめなくてはならないのでパッキング

static char* name_char = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789/_.&!:? ";
#include "../clearcode/clearcode.h"

#define	ENCODE_DATA_NAME_LEN	11
typedef struct ENCODE_VR_INFO {
	u_char	name[ ENCODE_DATA_NAME_LEN ];	//00-10
	u_char	achieve_high;					//11-11
	u_int	achieve_low_time;				//12-15
} ENCODE_VR_INFO;


void EncodeVRInfo( MCMAN_INFODATA* pDst, const MCMAN_VR_INFODATA* pSrc )
{
	u_char	data45[ MCMAN_VR_NAME_MAX ];
	ENCODE_VR_INFO* pDstBuf = (ENCODE_VR_INFO*)pDst;
	
	int i;
	// 名前を45進数に変換
	for( i = 0; i < MCMAN_VR_NAME_MAX; i++ ) {
		char* pFound;
		if( ( pFound = strchr( name_char, pSrc->name[ i ] )) == NULL ) {
			data45[i] = 0;
		} else {
			// pSrc->name[i] == '\0' だと name_char の終端 '\0' とマッチするらしい
			data45[i] = ((pFound - name_char) + 1) % 45;
		}
	}

	// 45進数->256進数に変換
	ClearCodeChangeRadix( pDstBuf->name, ENCODE_DATA_NAME_LEN, 256,
						  data45, MCMAN_VR_NAME_MAX, 45 );
	// 他の情報を入れる
	pDstBuf->achieve_high = pSrc->achieve_high;
	if( pSrc->vr_playtime / FRAMES_PER_SEC >= 10000 * 60 * 60 ) {
		pDstBuf->achieve_low_time = 10000 * 60 * 60 - 1;
	} else {
		pDstBuf->achieve_low_time = (pSrc->vr_playtime / FRAMES_PER_SEC) & 0xfffffff;	// 28 bit
	}
	pDstBuf->achieve_low_time |= (u_int)(pSrc->achieve_low & 0xf) << 28;
}

void DecodeVRInfo( MCMAN_VR_INFODATA* pDst, const MCMAN_INFODATA* pSrc )
{
	u_char	data45[ MCMAN_VR_NAME_MAX + 1];
	ENCODE_VR_INFO* pSrcBuf = (ENCODE_VR_INFO*)pSrc;
	int i;

	// 256進数->45進数に変換
	ClearCodeChangeRadix( data45, MCMAN_VR_NAME_MAX + 1 , 45,
						  pSrcBuf->name, ENCODE_DATA_NAME_LEN, 256 );
	// 45進数->名前に変換
	for( i = 0 ; i < MCMAN_VR_NAME_MAX; i++ ) {
		if( data45[ i ] == 0 ) { 
			pDst->name[ i ] = '\0';
		} else {
			pDst->name[ i ] = name_char[ data45[ i ] - 1 ];
		}
	}

	// 他の情報を入れる
	pDst->achieve_high = pSrcBuf->achieve_high;
	pDst->vr_playtime = (pSrcBuf->achieve_low_time & 0xfffffff) * FRAMES_PER_SEC;
	pDst->achieve_low = ( pSrcBuf->achieve_low_time >> 28) & 0xf;
}
#endif


int LoadGameData(MCMAN_GAMEDATA *data, int filetype)
{
	unsigned int crc, _crc;
	unsigned int _seed;
	void *linkvar;
	void *varbuf;
	int version;

	memcpy( &_crc, data->crc, sizeof( int ) );
	memcpy( &_seed, data->seed, sizeof( int ) );
	memcpy( &version, data->variable_version, sizeof( int ) );

	if( version != GCL_VariableVersion ){
		// GCL変数のバージョン違い。

#ifdef DEBUG_MODE
		printf("Game Data Version is Different. 0x%08x 0x%08x\n",
			   version,GCL_VariableVersion);
		// ASSERT(0);
#endif

		return 0;
	}
	if(data->id1!=ID1 || data->id2!=ID2){

#ifdef DEBUG_MODE
		printf("Game Data ID is Different.\n");
		// ASSERT(0);
#endif

		return 0;
	}

	DecodeData(data->linkvar,MAX_LINKVARBUF,&_seed);
	DecodeData(data->varbuf,MAX_VAR_BUF,&_seed);
	if( filetype == MCMAN_FILE_KIND_VR ) {
		DecodeData(data->vr_score_data, MSN_SAVE_DATA_SIZE, &_seed );
	} else {
		DecodeData(data->tanker_photo_data, GM_TANKER_PICTURE_SIZE, &_seed );
	}

	linkvar = GCL_GetLinkvarSaveAreaTop();
	varbuf = GCL_GetVarSaveAreaTop();

	memcpy( linkvar, data->linkvar, MAX_LINKVARBUF );
	memcpy( varbuf, data->varbuf, MAX_VAR_BUF );
	if( filetype == MCMAN_FILE_KIND_VR ) {
		memcpy( _MSN_SAVE_DATA, data->vr_score_data, MSN_SAVE_DATA_SIZE );
	} else {
		memcpy( GM_TankerPicture, data->tanker_photo_data, GM_TANKER_PICTURE_SIZE );
	} 
	
	crc = calc_crc_code( linkvar, MAX_LINKVARBUF );
	crc ^= calc_crc_code( varbuf, MAX_VAR_BUF );

	if( crc != _crc ){

#ifdef DEBUG_MODE
		printf( "CRC ERROR %X %X\n", crc, _crc );
#endif

		return 0;
	}

   //BP - convert at savedata loaded time from 60ths of a second (AFTER crc check)
   {
      int * bp_playTime60 = (int *)( (char*)linkvar + 312 ); //#define GM_PlayTime		(*( int *)( linkvarbuf + 312 ))
      int bp_playTime = BP_Frames60ToFramesSys(*bp_playTime60);
      printf("BP: convert loaded GM_PlayTime: %d -> %d\n", *bp_playTime60, bp_playTime );
      *bp_playTime60 = bp_playTime;
   }

	/* これはロードしてきた瞬間に有効にする */
	GM_TitleMenuStatus
			|= GetLinkVariable( GM_TitleMenuStatus, linkvar )
			& ~TITLE_MENU_STORY_SEL_MASK;

	return 1;
}

#ifdef PSX2	///
int sd_set_cli(int);
#endif	///

int ComStoreLoadedVariable(void)
{
	/* オプション画面で変更されていた場合は、そちらを優先する */
	int gm_configuration=GM_Configuration;
	int gm_configuration2=GM_Configuration2;
	int adjx=GM_ScrAdjX;
	int adjy=GM_ScrAdjY;
	int gm_titlemenustatus=GM_TitleMenuStatus;

//#ifdef PAL	// 多ヶ国語対応
	int gm_language=GM_Language;
//#endif

	GCL_RestoreVar();

	printf("Restart : GM_TitleMenu = 0x%04x 0x%04x\n",gm_titlemenustatus,GM_TitleMenuStatus);

#ifdef DEBUG_MODE
	// printf("GM_Configration = 0x%04x 0x%04x\n",gm_configuration,GM_Configuration);
#endif

	if(gm_titlemenustatus & TITLE_MENU_OPTION_CHANGED){
		GM_Configuration
				=(gm_configuration & ~(GM_CONFIG_STORY_TANKER|
									   GM_CONFIG_TANKER_CLEARED|GM_CONFIG_END_IF_FOUND
#if 0	// 変えられたほうが親切なので元に戻しました	2002/08/19 M.Kobayashi
									   /* 2001/11/8 add by Ken Kano */
									   |GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE
#endif				  
									   ))|

				(GM_Configuration & (GM_CONFIG_STORY_TANKER|
									 GM_CONFIG_TANKER_CLEARED|GM_CONFIG_END_IF_FOUND
#if 0	// 変えられたほうが親切なので元に戻しました	2002/08/19 M.Kobayashi
									 /* 2001/11/8 add by Ken Kano */
									 |GM_CONFIG_RADAR_OFF|GM_CONFIG_RADAR_OFF_INTRUDE
#endif				  
									 ));
		GM_Configuration2 = ( gm_configuration2 & ~GM_CONFIG_DOGTAGS_2002 )
			| (GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 );	// ドッグタグモードは変わらない
//#ifdef PAL	// 多ヶ国語対応
		GM_Language=gm_language;
//#endif

	}
	
#ifdef KP_XBOX	// XBOXでは言語設定をセーブできてはいけない
	GM_Language=gm_language;
#endif
	
	if(gm_titlemenustatus & TITLE_MENU_SCRADJ_CHANGED){
		GM_ScrAdjX=adjx;
		GM_ScrAdjY=adjy;
	}

	GM_TitleMenuStatus|=(gm_titlemenustatus & ~TITLE_MENU_STORY_SEL_MASK);

	/* 表示アジャスト値をセーブしていたものに変更 */
	DG_SetDisplayOffset(GM_ScrAdjX,GM_ScrAdjY,0);

#if 0 // BP FIX divorcing options
	/* サウンドの設定 */
	if(GM_Configuration & GM_CONFIG_CUTSCENES_LETTERBOXED){
		/* モノラル */
#ifdef PSX2		
		sd_set_cli(SD_MONORAL);
#else
		GM_SdSet( SD_MONORAL );
#endif		
	}
	else{
		/* ステレオ */
#ifdef PSX2		
		sd_set_cli(SD_STEREO);
#else		
		GM_SdSet( SD_STEREO );
#endif
	}
#endif // BP FIX divorcing options

#ifdef KP_XBOX
	// Xbox のみ音量バランス設定
	GM_SdSet( 0xf7000000 | ( 0xf - ( ( GM_Configuration2 & GM_CONFIG_MUSIC_VOLUME ) >> 4 ) ) );
	GM_SdSet( 0xf7000010 | ( 0xf - ( GM_Configuration2 & GM_CONFIG_SE_VOLUME ) ) );
#endif	
	

//#ifdef PAL	//多ヶ国語対応
	{
		int NewGclLangUpdate(void);
		NewGclLangUpdate();
	}
//#endif

   //BP - this ensures that we know the most recently loaded resident pakfile.
   //Necessary for the proper overridden JP Snake / Raiden voice SE to be played back at all times.
   BP_CheckResidentAreaHook();

	return 0;
}

int NewLoadRestart( void )
{
	ComStoreLoadedVariable();

	GM_LoadRestart( 1 );

#ifdef DEBUG_MODE
	// printf("GM_Configration = 0x%04x\n",GM_Configuration);
#endif

	return 0;
}
