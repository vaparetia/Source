//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	mcman.c
		メモリーカード制御アクター
	
	2001/04/16 K.Kano
	$Id: mcman.c,v 1.1.1.3 2002/11/19 11:43:16 Yoshizawa1 Exp $
*/


#include "mcman.h"
#ifdef PSX2
#include <libcdvd.h>
#include <libscf.h>
#endif

#include "BP_SaveLoadMGS.h"
#include "TransfarringVTACGlue.h"

extern void ShowCorruptSaveWarning(int *pResult);

enum {
	MCMAN_MODE_NONE=0,

	MCMAN_MODE_MCCHECK,

	MCMAN_MODE_LOAD_PHOTO,
	MCMAN_MODE_LOAD_PHOTOICON,
	MCMAN_MODE_SAVE_PHOTO,
	MCMAN_MODE_GET_PHOTO_INFO,
	MCMAN_MODE_CHANGE_PHOTO_INFO,

	MCMAN_MODE_LOAD_GAME,
	MCMAN_MODE_SAVE_GAME,
	MCMAN_MODE_GET_GAME_INFO,
	MCMAN_MODE_CHANGE_GAME_INFO,

	MCMAN_MODE_PHOTO_FORMAT,
	MCMAN_MODE_GAME_FORMAT,

	MCMAN_MODE_PHOTO_EASY_SEARCH,
	MCMAN_MODE_GAME_EASY_SEARCH,

	MCMAN_MODE_PHOTO_DIR_SEARCH,
	MCMAN_MODE_GAME_DIR_SEARCH,
   MCMAN_MODE_DELETE_GAME,
   MCMAN_MODE_DELETE_PHOTO,
};


MCMAN_WORK *mcman_allocated_work=NULL;

#define MCACC_WORK		(&(mcman_allocated_work->accwork))


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */
#ifdef PSX2

#define ICON_TEX_U		((float)ICON_TEX_WIDTH/128.0f)
#define ICON_TEX_V		((float)ICON_TEX_HEIGHT/128.0f)

#define ASPECT_RATE		((float)ICON_TEX_HEIGHT/(float)ICON_TEX_WIDTH)


/* アイコンのデータのモデル部分
   シンプルなアイコンということで、ポリゴン二枚による四角形にテクスチャを
   張った構造をデフォルトデータとして置いておく。*/
DEFAULT_ICON_HEADER default_icon_header={
	/* ヘッダ */
	0x00010000,

	/* モデル */
	1,
	(1 | 2 | 4 | 8 ),
	1.0f,
	4*3,
	{
		/* 表面 */
		{
			{
				{
					{ -10.0f*0x0400,  0.0f*0x0400,  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, -1.0f*0x1000, 1.0f*0x1000, },
				{   0.0f*0x1000,  ICON_TEX_V*0x1000, },
				{ 255,255,255,128, },
			},
			{
				{
					{  10.0f*0x0400,  0.0f*0x0400,  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, -1.0f*0x1000, 1.0f*0x1000, },
				{   ICON_TEX_U*0x1000,  ICON_TEX_V*0x1000, },
				{ 255,255,255,128, },
			},
			{
				{
					{ -10.0f*0x0400,-20.0f*ASPECT_RATE*0x0400,
					  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, -1.0f*0x1000, 1.0f*0x1000, },
				{   0.0f*0x1000,  0.0f*0x1000, },
				{ 255,255,255,128, },
			},
		},
		{
			{
				{
					{ -10.0f*0x0400,-20.0f*ASPECT_RATE*0x0400,
					  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, -1.0f*0x1000, 1.0f*0x1000, },
				{   0.0f*0x1000,  0.0f*0x1000, },
				{ 255,255,255,128, },
			},
			{
				{
					{  10.0f*0x0400,  0.0f*0x0400,  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, -1.0f*0x1000, 1.0f*0x1000, },
				{   ICON_TEX_U*0x1000,  ICON_TEX_V*0x1000, },
				{ 255,255,255,128, },
			},
			{
				{
					{  10.0f*0x0400,-20.0f*ASPECT_RATE*0x0400,
					   0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, -1.0f*0x1000, 1.0f*0x1000, },
				{   ICON_TEX_U*0x1000,  0.0f*0x1000, },
				{ 255,255,255,128, },
			},
		},

		/* 裏面 */
		{
			{
				{
					{  10.0f*0x0400,  0.0f*0x0400,  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, 1.0f*0x1000, 1.0f*0x1000, },
				{   0.0f*0x1000,  ICON_TEX_V*0x1000, },
				{ 255,255,255,128, },
			},
			{
				{
					{ -10.0f*0x0400,  0.0f*0x0400,  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, 1.0f*0x1000, 1.0f*0x1000, },
				{   ICON_TEX_U*0x1000,  ICON_TEX_V*0x1000, },
				{ 255,255,255,128, },
			},
			{
				{
					{ -10.0f*0x0400,-20.0f*ASPECT_RATE*0x0400,
					  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, 1.0f*0x1000, 1.0f*0x1000, },
				{   ICON_TEX_U*0x1000,  0.0f*0x1000, },
				{ 255,255,255,128, },
			},
		},
		{
			{
				{
					{  10.0f*0x0400,  0.0f*0x0400,  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, 1.0f*0x1000, 1.0f*0x1000, },
				{   0.0f*0x1000,  ICON_TEX_V*0x1000, },
				{ 255,255,255,128, },
			},
			{
				{
					{ -10.0f*0x0400,-20.0f*ASPECT_RATE*0x0400,
					  0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, 1.0f*0x1000, 1.0f*0x1000, },
				{   ICON_TEX_U*0x1000,  0.0f*0x1000, },
				{ 255,255,255,128, },
			},
			{
				{
					{  10.0f*0x0400,-20.0f*ASPECT_RATE*0x0400,
					   0.0f*0x0400, 1.0f*0x0400, },
				},
				{   0.0f*0x1000,  0.0f*0x1000, 1.0f*0x1000, 1.0f*0x1000, },
				{   0.0f*0x1000,  0.0f*0x1000, },
				{ 255,255,255,128, },
			},
		},
	},

	/* アニメーション */
	1,
	{
		{
			100,
			1.0f,
			0,
			1,
			{
				{
					0,
					1,
					{
						{
							0.0f,
							1.0f,
						},
					},
				},
			},
		},
	},
};


#endif
/* アイコンテクスチャのランレングス圧縮部
   SONYのサンプルをそのまま持ってきた。*/

/* 4/25/2001
   bufferの使い方を配列からポインタに変更
   これでいいはず。*/

int MCIcon_RleEncode(unsigned char *in_data, unsigned int size, 
					 unsigned char *out_data, unsigned int *out_size)
{
    unsigned short s1, prev_s1 = 0, *pInput, *pOutput, cnt = 0;
    unsigned int index, index_buff, flag;
    unsigned short *buffer;
	unsigned int *arc_size;

    // --- set start ---
    pInput = (unsigned short *)in_data;
	arc_size=(unsigned int *)out_data;
    pOutput = (unsigned short *)(arc_size+1);

    index_buff = 0;
    flag = 3;
    index = 0;
    size = size >> 1;

	buffer=pInput;

    while(index < size) {
        // --- read char ---        
        s1 = *pInput++;
        index++;
            
        if(flag == 0) { // same chars
			if(prev_s1 == s1) {
                cnt++;
                if(cnt == 0x7fff) {
                    *pOutput++ = 0x7fff;
                    *pOutput++ = s1;
                    cnt = 0;
                    flag = 3;
                    prev_s1 = s1;
                    index_buff = 0;
                    continue;
                }
				else {
                    continue;
                }
			}
			else {
                *pOutput++ = cnt;
                *pOutput++ = prev_s1;
                cnt = 1;
                flag = 2;
                prev_s1 = s1;

				buffer=pInput-1;

                continue;
            }
        }
		else if(flag == 1) { // different chars
            if(prev_s1 != s1) {
                cnt++;            
                if(cnt == 0x7fff) {
                    *pOutput++ = - (cnt-1);
                    memcpy(pOutput, buffer, sizeof(short)*(cnt-1));
                    cnt = 1;
                    flag = 2;
                    prev_s1 = s1;
                    index_buff = 0;
                    pOutput += (cnt-1);

					buffer=pInput-1;

                    continue;
                }
				else {
                    // buffer[index_buff++] = s1;

                    prev_s1 = s1;
                    continue;
                }
			}
			else if(prev_s1 == s1) {
                *pOutput++ = -(cnt-1);
                memcpy(pOutput, buffer, sizeof(unsigned short)*(cnt-1));         
                pOutput += (cnt-1);
                index_buff = 0;
                cnt = 2;
                flag = 0;

				buffer=pInput-2;

                continue;
            }
        }
		else if(flag == 2) { 
            if(prev_s1 == s1) {
                cnt++;
                flag = 0;
                continue;
			}
			else {
                cnt++;
                flag = 1;

                // buffer[0] = prev_s1;
                // buffer[1] = s1;

                index_buff = 2;
                prev_s1 = s1;
            }
        }
		else {  // init state.
            prev_s1 = s1;
            flag = 2;
            cnt = 1;

			buffer=pInput-1;

            continue;            
        }
    }
        
    if(flag == 0){
        *pOutput++ = cnt;
        *pOutput++ = prev_s1;
    }
	else if(flag == 1) {
        *pOutput++ = -cnt;
        memcpy(pOutput, buffer, cnt * sizeof(unsigned short));
        pOutput += cnt;
    }
	else if(flag == 2) {
        *pOutput++ = cnt;
        *pOutput++ = prev_s1;
    } 

	*out_size = (unsigned int)pOutput - (unsigned int)out_data;
	*arc_size = *out_size-sizeof(unsigned int);

    return 0;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


void MCMan_PhotoIconEncode(void *icon)
{
#ifdef PSX2	
	MCMAN_WORK *work=mcman_allocated_work;
	unsigned short *src,*dis;
	unsigned int *size;
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	memcpy(work->iconwork,&default_icon_header,sizeof(default_icon_header));

	size=(unsigned int *)((int)(work->iconwork)+sizeof(default_icon_header));
	dis=(unsigned short *)(size+1);

	src=(unsigned short *)icon;

	*size=ICON_TEXTURE_SIZE;

	for(i=0;i<ICON_TEX_HEIGHT;i++){
		*dis=(unsigned short)(-ICON_TEX_WIDTH);
		dis++;

		memcpy(dis,src,ICON_TEX_WIDTH*sizeof(short));
		src+=ICON_TEX_WIDTH;
		dis+=ICON_TEX_WIDTH;

		*(dis+0)=128-ICON_TEX_WIDTH;
		*(dis+1)=0;
		dis+=2;
	}

	*(dis-2)+=128*(128-ICON_TEX_HEIGHT);
#endif	
}

void MCMan_PhotoIconDecode(void *icon)
{
#ifdef PSX2	
	MCMAN_WORK *work=mcman_allocated_work;
	unsigned short *src,*dis;
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	src=(unsigned short *)((int)(work->iconwork)+sizeof(default_icon_header));
	src+=2;
	dis=(unsigned short *)icon;

	for(i=0;i<ICON_TEX_HEIGHT;i++){
		src++;

		memcpy(dis,src,ICON_TEX_WIDTH*sizeof(short));
		src+=ICON_TEX_WIDTH;
		dis+=ICON_TEX_WIDTH;

		src+=2;
	}
#endif	
}

void MCMan_SetPhotoTitle(char *photoname)
{
	char title[0x80];
	char sjis_photoname[0x80];
	int i=0,j=0;

	while(*(photoname+i)!='\0'){
		int c=*(unsigned char *)(photoname+i);

		if(c<0x20 || c>=0x7f) c=0x20;
		c=(c-0x20)*2;

		sjis_photoname[j+0]=*(ASCII_CODE_SJIS_STR+c+0);
		sjis_photoname[j+1]=*(ASCII_CODE_SJIS_STR+c+1);

		i++;
		j+=2;
	}

	sjis_photoname[j]='\0';

	strcpy(title,MGS2_PHOTO_TITLE);
	strcat(title,sjis_photoname);

#ifdef DEBUG_MODE
	printf("Photo title : %s\n",title);
#endif

	SetIconSysTitle(title,MGS2_PHOTO_TITLE_LF_LOCATE);
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


/* メモリーカードのチェックシーケンスを通ると、一回だけ１が返ってくる */
int MCManChecked(void)
{
	int flag;
	ASSERT(mcman_allocated_work!=NULL);
	flag=((mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED)!=0);
	mcman_allocated_work->flag&=~MCMAN_FLAG_MCCHECKED;
#ifdef DEBUG_MODE
	// if(flag) printf("CheckON\n");
	// else printf("CheckOFF\n");
#endif
	return flag;
}

/* メモリカードがチェックシーケンスを通っている、通ったを判定する */
int MCManCheckingOrChecked(void)
{
	ASSERT(mcman_allocated_work!=NULL);
	return (mcman_allocated_work->flag & (MCMAN_FLAG_MCCHECKING|MCMAN_FLAG_MCCHECKED))!=0;
}

/* 現在フォーマット中の判定 */
int MCManFormatting(void)
{
	ASSERT(mcman_allocated_work!=NULL);
	return (mcman_allocated_work->flag & MCMAN_FLAG_MCFORMATTING)!=0;
}

/* メモリーカードの種類を返す
   マルチタップには対応していないので、マルチタップが差さっていることのみを返す。
   MCMAN_CARDTYPE_NOCARD          差さっていない
   MCMAN_CARDTYPE_PS1             PS1
   MCMAN_CARDTYPE_PS2_FORMATTED   PS2 Formatted
   MCMAN_CARDTYPE_POCKET_STATION  Pocket Station

   MCMAN_CARDTYPE_PS2_UNFORMATTED PS2 Unformatted

   MCMAN_CARDTYPE_ERRORCARD       Error
   MCMAN_CARDTYPE_MULTITAP        Multi-tap
   */

// Xbox TODO: port のみでなくスロットに対応しなくてはならない
// port max = 8 として slot を中に含めるのが楽か？

#ifdef DEBUG_MODE
int debug_MCManGetCardType(int port)
#else
int MCManGetCardType(int port)
#endif
{
	ASSERT(mcman_allocated_work!=NULL);
#ifdef PSX2
	if(MCAccessGetSlotMax(MCACC_WORK,port)!=1) return MCMAN_CARDTYPE_MULTITAP;
	else
#endif		
		if(MCAccessGetType(MCACC_WORK,port,0)==MCACC_CARDTYPE_PS2){
		if(MCAccessIsFormatted(MCACC_WORK,port,0)) return MCMAN_CARDTYPE_PS2_FORMATTED;
		else return MCMAN_CARDTYPE_PS2_UNFORMATTED;
	}
	return MCAccessGetType(MCACC_WORK,port,0);
}

#ifdef DEBUG_MODE

int MCManGetCardType(int port)
{
	int ans=debug_MCManGetCardType(port);
	printf("Card Type ( %d ) = %d\n",port,ans);
	return ans;
}

#endif

int MCManIsMCChanged(int port)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(MCAccessGetSlotMax(MCACC_WORK,port)!=1) return 1;
	else return MCAccessIsMCChanged(MCACC_WORK,port,0);
}

/* メモリーカードの残り空き容量を返す */
int MCManGetFreeSize(int port)
{
	return MCAccessGetFreeSize(MCACC_WORK,port,0);
}

/* 各種ステップの結果取得 */
int MCManGetResult(void)
{
	ASSERT(mcman_allocated_work!=NULL);
	// printf("GetResult = %d\n",mcman_allocated_work->result);
	return mcman_allocated_work->result;
}

int MCManAccessing(void)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->mode!=MCMAN_MODE_NONE) return 1;
	if(MCAccessing(MCACC_WORK)) return 1;

	return 0;
}

/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


int MCManFileFlagReset(void)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;
	if(mcman_allocated_work->mode!=MCMAN_MODE_NONE) return 0;
	if(MCAccessing(MCACC_WORK)) return 0;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}

int SavePhotoStart(int port,int id,void *photodata,int photosize,
				   void *icondata,int iconsize,void *info)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;

		if(MCAccessIsFormatted(MCACC_WORK,port,0)){
			mcman_allocated_work->mode=MCMAN_MODE_SAVE_PHOTO;
		}
		else{
			mcman_allocated_work->mode=MCMAN_MODE_PHOTO_FORMAT;
			mcman_allocated_work->flag|=MCMAN_FLAG_MCFORMATTING;
		}

		mcman_allocated_work->step=0;
		mcman_allocated_work->sub_step=0;

		mcman_allocated_work->port=port;
		mcman_allocated_work->slot=0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(mcman_allocated_work->flag & MCMAN_FLAG_NEXT_REQUEST) return 0;
		if(!(mcman_allocated_work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;

		mcman_allocated_work->flag|=MCMAN_FLAG_NEXT_REQUEST;

		if(MCAccessIsFormatted(MCACC_WORK,port,0)){
			mcman_allocated_work->store_mode=MCMAN_MODE_SAVE_PHOTO;
		}
		else{
			mcman_allocated_work->store_mode=MCMAN_MODE_PHOTO_FORMAT;
			mcman_allocated_work->flag|=MCMAN_FLAG_MCFORMATTING;
		}

		mcman_allocated_work->store_port=port;
		mcman_allocated_work->store_slot=0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->id=id;

	mcman_allocated_work->result=0;

#ifdef PSX2	
	if(icondata==NULL){
		mcman_allocated_work->icondata=mcman_allocated_work->iconwork;
		mcman_allocated_work->iconsize=DEFAULT_ICON_SIZE;
	}
	else{
		mcman_allocated_work->icondata=icondata;
		mcman_allocated_work->iconsize=iconsize;
	}
#endif	
	mcman_allocated_work->data=photodata;
	mcman_allocated_work->datasize=photosize;
	memcpy(mcman_allocated_work->info,info,INFOCODE_SIZE);

	return 1;
}

int LoadPhotoStart(int port,int id,void *photodata)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	// printf("Mode = %d\n",mcman_allocated_work->mode);

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		mcman_allocated_work->mode=MCMAN_MODE_LOAD_PHOTO;

		mcman_allocated_work->step=0;
		mcman_allocated_work->sub_step=0;

		mcman_allocated_work->port=port;
		mcman_allocated_work->slot=0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(mcman_allocated_work->flag & MCMAN_FLAG_NEXT_REQUEST) return 0;
		if(!(mcman_allocated_work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;

		mcman_allocated_work->store_mode=MCMAN_MODE_LOAD_PHOTO;
		mcman_allocated_work->flag|=MCMAN_FLAG_NEXT_REQUEST;

		mcman_allocated_work->store_port=port;
		mcman_allocated_work->store_slot=0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->id=id;

	mcman_allocated_work->result=0;

	mcman_allocated_work->data=photodata;

	return 1;
}

int LoadPhotoIconStart(int port,int id,void *icondata)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		mcman_allocated_work->mode=MCMAN_MODE_LOAD_PHOTOICON;

		mcman_allocated_work->step=0;
		mcman_allocated_work->sub_step=0;

		mcman_allocated_work->port=port;
		mcman_allocated_work->slot=0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(mcman_allocated_work->flag & MCMAN_FLAG_NEXT_REQUEST) return 0;
		if(!(mcman_allocated_work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;

		mcman_allocated_work->store_mode=MCMAN_MODE_LOAD_PHOTOICON;
		mcman_allocated_work->flag|=MCMAN_FLAG_NEXT_REQUEST;

		mcman_allocated_work->store_port=port;
		mcman_allocated_work->store_slot=0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->id=id;

	mcman_allocated_work->result=0;

#ifdef PSX2	
	if(icondata==NULL){
		mcman_allocated_work->icondata=mcman_allocated_work->iconwork;
	}
	else{
		mcman_allocated_work->icondata=icondata;
	}
#endif	

	return 1;
}

int GetPhotoInfoStart(int port)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	if(mcman_allocated_work->mode!=MCMAN_MODE_NONE) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_PHOTO_DIR_SEARCH;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->n_files=0;
	mcman_allocated_work->max_id=-1;
	mcman_allocated_work->checking_id=0;
	mcman_allocated_work->readable_n_files=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}

int GetPhotoInfoBGStart(int port)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_PHOTO_DIR_SEARCH;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->n_files=0;
	mcman_allocated_work->max_id=-1;
	mcman_allocated_work->checking_id=0;
	mcman_allocated_work->readable_n_files=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}

int GetPhotoInfoRBGStart(int port)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_PHOTO_DIR_SEARCH;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->n_files=0;
	mcman_allocated_work->max_id=-1;
	mcman_allocated_work->checking_id=PHOTO_FILE_MAX-1;
	mcman_allocated_work->readable_n_files=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}

int GetPhotoPageInfoStart(int port,int id,int size)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GET_PHOTO_INFO;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->checking_id=id;
	mcman_allocated_work->checked_limit=size;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	return 1;
}

int GetPhotoInfoOnlyBGStart(int port)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GET_PHOTO_INFO;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->checking_id=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	return 1;
}

int GetPhotoInfoOnlyRBGStart(int port)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GET_PHOTO_INFO;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->checking_id=PHOTO_FILE_MAX-1;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	return 1;
}

int ChangePhotoInfoStart(int port,int id,void *newinfo)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	{
		/* Infoの内容が以前と同じかどうかチェック */
		unsigned char *s=(char *)newinfo;
		unsigned char *d=mcman_allocated_work->file_info[id];
		int i;

		for(i=0;i<INFOCODE_SIZE;i++){
			if(*s!=*d) goto next;
			s++; d++;
		}

		mcman_allocated_work->result=1;

		return 1;
	}

next:
	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		mcman_allocated_work->mode=MCMAN_MODE_CHANGE_PHOTO_INFO;

		mcman_allocated_work->step=0;
		mcman_allocated_work->sub_step=0;

		mcman_allocated_work->port=port;
		mcman_allocated_work->slot=0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(mcman_allocated_work->flag & MCMAN_FLAG_NEXT_REQUEST) return 0;
		if(!(mcman_allocated_work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;

		mcman_allocated_work->store_mode=MCMAN_MODE_CHANGE_PHOTO_INFO;
		mcman_allocated_work->flag|=MCMAN_FLAG_NEXT_REQUEST;

		mcman_allocated_work->store_port=port;
		mcman_allocated_work->store_slot=0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->id=id;

	mcman_allocated_work->result=0;

	memcpy(mcman_allocated_work->info,newinfo,INFOCODE_SIZE);

	return 1;
}

int PhotoEasySearchStart(int port)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_PHOTO_EASY_SEARCH;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->n_files=0;
	mcman_allocated_work->max_id=-1;
	mcman_allocated_work->easy_search_n_files[port]=0;
	mcman_allocated_work->easy_search_max_id[port]=-1;

	mcman_allocated_work->checking_id=0;
	mcman_allocated_work->readable_n_files=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_BACKGROUND;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}

int SaveGameStart(int port,int id,void *gamedata,int gamesize,
				  void *icondata,int iconsize,void *info)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;

		if(MCAccessIsFormatted(MCACC_WORK,port,0)){
			mcman_allocated_work->mode=MCMAN_MODE_SAVE_GAME;
		}
		else{
			mcman_allocated_work->mode=MCMAN_MODE_GAME_FORMAT;
			mcman_allocated_work->flag|=MCMAN_FLAG_MCFORMATTING;
		}

		mcman_allocated_work->step=0;
		mcman_allocated_work->sub_step=0;

		mcman_allocated_work->port=port;
		mcman_allocated_work->slot=0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(mcman_allocated_work->flag & MCMAN_FLAG_NEXT_REQUEST) return 0;
		if(!(mcman_allocated_work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;

		mcman_allocated_work->flag|=MCMAN_FLAG_NEXT_REQUEST;

		if(MCAccessIsFormatted(MCACC_WORK,port,0)){
			mcman_allocated_work->store_mode=MCMAN_MODE_SAVE_GAME;
		}
		else{
			mcman_allocated_work->store_mode=MCMAN_MODE_GAME_FORMAT;
			mcman_allocated_work->flag|=MCMAN_FLAG_MCFORMATTING;
		}

		mcman_allocated_work->store_port=port;
		mcman_allocated_work->store_slot=0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->id=id;

	mcman_allocated_work->result=0;

#ifdef PSX2	
	if(icondata==NULL){
		mcman_allocated_work->icondata=mcman_allocated_work->iconwork;
		mcman_allocated_work->iconsize=DEFAULT_ICON_SIZE;
	}
	else{
		mcman_allocated_work->icondata=icondata;
		mcman_allocated_work->iconsize=iconsize;
	}
#endif	
	mcman_allocated_work->data=gamedata;
	mcman_allocated_work->datasize=gamesize;
	memcpy(mcman_allocated_work->info,info,INFOCODE_SIZE);

	return 1;
}

int LoadGameStart(int port,int id,void *gamedata)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		mcman_allocated_work->mode=MCMAN_MODE_LOAD_GAME;

		mcman_allocated_work->step=0;
		mcman_allocated_work->sub_step=0;

		mcman_allocated_work->port=port;
		mcman_allocated_work->slot=0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(mcman_allocated_work->flag & MCMAN_FLAG_NEXT_REQUEST) return 0;
		if(!(mcman_allocated_work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;

		mcman_allocated_work->store_mode=MCMAN_MODE_LOAD_GAME;
		mcman_allocated_work->flag|=MCMAN_FLAG_NEXT_REQUEST;

		mcman_allocated_work->store_port=port;
		mcman_allocated_work->store_slot=0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->id=id;

	mcman_allocated_work->result=0;

	mcman_allocated_work->data=gamedata;

	return 1;
}

int DeleteGameStart(int id)
{
   ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		mcman_allocated_work->mode=MCMAN_MODE_DELETE_GAME;

		mcman_allocated_work->step=0;
		mcman_allocated_work->sub_step=0;

		mcman_allocated_work->port=0;
		mcman_allocated_work->slot=0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->id=id;

	mcman_allocated_work->result=0;

	mcman_allocated_work->data=0;

	return 1;
}

int DeletePhotoStart(int id)
{
   ASSERT(mcman_allocated_work!=NULL);

   if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

   switch(mcman_allocated_work->mode){
   case MCMAN_MODE_NONE:
      if(MCAccessing(MCACC_WORK)) return 0;
      mcman_allocated_work->mode=MCMAN_MODE_DELETE_PHOTO;

      mcman_allocated_work->step=0;
      mcman_allocated_work->sub_step=0;

      mcman_allocated_work->port=0;
      mcman_allocated_work->slot=0;
      break;
   default:
      return 0;
   }

   mcman_allocated_work->id=id;

   mcman_allocated_work->result=0;

   mcman_allocated_work->data=0;

   return 1;
}

int GetGameInfoStart(int port)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GAME_DIR_SEARCH;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->n_files=0;
	mcman_allocated_work->max_id=-1;
	mcman_allocated_work->checking_id=0;
	mcman_allocated_work->readable_n_files=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}

int GetGameInfoBGStart(int port)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GAME_DIR_SEARCH;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->n_files=0;
	mcman_allocated_work->max_id=-1;
	mcman_allocated_work->checking_id=0;
	mcman_allocated_work->readable_n_files=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}

int GetGameInfoRBGStart(int port)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GAME_DIR_SEARCH;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->n_files=0;
	mcman_allocated_work->max_id=-1;
	mcman_allocated_work->checking_id= MCMAN_GetDataFileMax( mcman_allocated_work->file_kind ) -1;
	mcman_allocated_work->readable_n_files=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}

int GetGamePageInfoStart(int port,int id,int size)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GET_GAME_INFO;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->checking_id=id;
	mcman_allocated_work->checked_limit=size;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	return 1;
}

int GetGameInfoOnlyBGStart(int port)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GET_GAME_INFO;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->checking_id=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	return 1;
}

int GetGameInfoOnlyRBGStart(int port)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GET_GAME_INFO;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->checking_id= MCMAN_GetDataFileMax( mcman_allocated_work->file_kind ) -1;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_BACKGROUND;
	mcman_allocated_work->flag|=MCMAN_FLAG_CHECK_TO_BACK;
	mcman_allocated_work->flag&=~MCMAN_FLAG_NEXT_REQUEST;

	return 1;
}

int ChangeGameInfoStart(int port,int id,void *newinfo)
{
	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	{
		/* Infoの内容が以前と同じかどうかチェック */
		unsigned char *s=(char *)newinfo;
		unsigned char *d=mcman_allocated_work->file_info[id];
		int i;

		for(i=0;i<INFOCODE_SIZE;i++){
			if(*s!=*d) goto next;
			s++; d++;
		}

		mcman_allocated_work->result=1;

		return 1;
	}

next:
	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		mcman_allocated_work->mode=MCMAN_MODE_CHANGE_GAME_INFO;

		mcman_allocated_work->step=0;
		mcman_allocated_work->sub_step=0;

		mcman_allocated_work->port=port;
		mcman_allocated_work->slot=0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(mcman_allocated_work->flag & MCMAN_FLAG_NEXT_REQUEST) return 0;
		if(!(mcman_allocated_work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;

		mcman_allocated_work->store_mode=MCMAN_MODE_CHANGE_GAME_INFO;
		mcman_allocated_work->flag|=MCMAN_FLAG_NEXT_REQUEST;

		mcman_allocated_work->store_port=port;
		mcman_allocated_work->store_slot=0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->id=id;

	mcman_allocated_work->result=0;

	memcpy(mcman_allocated_work->info,newinfo,INFOCODE_SIZE);

	return 1;
}

int GameEasySearchStart(int port)
{
	int i;

	ASSERT(mcman_allocated_work!=NULL);

	if(mcman_allocated_work->flag & MCMAN_FLAG_MCCHECKED) return 0;

	switch(mcman_allocated_work->mode){
	case MCMAN_MODE_NONE:
		if(MCAccessing(MCACC_WORK)) return 0;
		break;
	case MCMAN_MODE_GET_GAME_INFO:
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(!(mcman_allocated_work->flag &
			 MCMAN_FLAG_ALREADY_RETURN_RESULT)) return 0;
		break;
	default:
		return 0;
	}

	mcman_allocated_work->mode=MCMAN_MODE_GAME_EASY_SEARCH;

	mcman_allocated_work->step=0;
	mcman_allocated_work->sub_step=0;
	mcman_allocated_work->result=0;

	mcman_allocated_work->port=port;
	mcman_allocated_work->slot=0;

	mcman_allocated_work->n_files=0;
	mcman_allocated_work->max_id=-1;
	mcman_allocated_work->easy_search_n_files[port]=0;
	mcman_allocated_work->easy_search_max_id[port]=-1;

	mcman_allocated_work->checking_id=0;
	mcman_allocated_work->readable_n_files=0;
	mcman_allocated_work->checked_limit=FILE_MAX;

	mcman_allocated_work->flag&=~MCMAN_FLAG_ALREADY_RETURN_RESULT;
	mcman_allocated_work->flag&=~MCMAN_FLAG_CHECK_BACKGROUND;

	for(i=0;i<sizeof(mcman_allocated_work->file_flags)
			/sizeof(mcman_allocated_work->file_flags[0]);i++){

		mcman_allocated_work->file_flags[i]=0;
	}

	return 1;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


void SetIconSysTitle(const char * const title,const int lf)
{
	ASSERT(mcman_allocated_work!=NULL);
#ifdef PSX2	
	memset(mcman_allocated_work->iconsys.TitleName,0x00,
		   sizeof(mcman_allocated_work->iconsys.TitleName));
	strcpy(mcman_allocated_work->iconsys.TitleName,title);
	mcman_allocated_work->iconsys.OffsLF=lf;
#endif	
}

void SetIconSysTransrate(const int tr)
{
#ifdef PSX2
	ASSERT(mcman_allocated_work!=NULL);
	mcman_allocated_work->iconsys.TransRate=tr;
#endif	
}

void SetIconSysBgColor(const int * const lu,const int * const ru,
					   const int * const ld,const int * const rd)
{
#ifdef PSX2	
	ASSERT(mcman_allocated_work!=NULL);
	memcpy(&(mcman_allocated_work->iconsys.BgColor[0]),lu,sizeof(int)*4);
	memcpy(&(mcman_allocated_work->iconsys.BgColor[1]),ru,sizeof(int)*4);
	memcpy(&(mcman_allocated_work->iconsys.BgColor[2]),ld,sizeof(int)*4);
	memcpy(&(mcman_allocated_work->iconsys.BgColor[3]),rd,sizeof(int)*4);
#endif	
}

void SetIconSysLight(const FVECTOR * const l1,const FVECTOR * const l2,const FVECTOR * const l3,
					 const FVECTOR * const c1,const FVECTOR * const c2,const FVECTOR * const c3)
{
#ifdef PSX2	
	ASSERT(mcman_allocated_work!=NULL);
	memcpy(&(mcman_allocated_work->iconsys.LightDir[0]),l1,sizeof(FVECTOR));
	memcpy(&(mcman_allocated_work->iconsys.LightDir[1]),l2,sizeof(FVECTOR));
	memcpy(&(mcman_allocated_work->iconsys.LightDir[2]),l3,sizeof(FVECTOR));
	memcpy(&(mcman_allocated_work->iconsys.LightColor[0]),c1,sizeof(FVECTOR));
	memcpy(&(mcman_allocated_work->iconsys.LightColor[1]),c2,sizeof(FVECTOR));
	memcpy(&(mcman_allocated_work->iconsys.LightColor[2]),c3,sizeof(FVECTOR));
#endif	
}

void SetIconSysLightDir(int index,const FVECTOR * const l)
{
#ifdef PSX2	
	ASSERT(mcman_allocated_work!=NULL);
	memcpy(&(mcman_allocated_work->iconsys.LightDir[index]),l,sizeof(FVECTOR));
#endif	
}

void SetIconSysLightColor(int index,const FVECTOR * const c)
{
#ifdef PSX2	
	ASSERT(mcman_allocated_work!=NULL);
	memcpy(&(mcman_allocated_work->iconsys.LightColor[index]),c,sizeof(FVECTOR));
#endif	
}

void SetIconSysAmbient(const FVECTOR * const c)
{
#ifdef PSX2	
	ASSERT(mcman_allocated_work!=NULL);
	memcpy(&(mcman_allocated_work->iconsys.Ambient),c,sizeof(FVECTOR));
#endif	
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static MCSubActWork subwork;

static void SubAct(MCSubActWork *work)
{
	if(work->mcman_work==NULL) return;
	MCAccessPostAct(&(work->mcman_work->accwork));
}

static void SubDie(MCSubActWork *work)
{
}

static void NewMCManSub(MCMAN_WORK *mcman_work)
{
    // MCSubActWork *work=&(mcman_work->mcsub_work);
	MCSubActWork *work=&subwork;

	GV_SetActorClass(work,GV_CLASS_CHARA);
	GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority(GV_ACTOR_DAEMON2,work,0x1FF);

	GV_SetActor(&(work->actor),SubAct,SubDie);
	GV_ActorEX(&(work->actor));

	work->mcman_work=mcman_work;
}


/* ------------------------------------------------------------------------ */
/* ------------------------------------------------------------------------ */


static void BackInfoMode(MCMAN_WORK *work)
{
	if(work->flag & MCMAN_FLAG_PAUSE_STEP){
		work->flag&=~MCMAN_FLAG_PAUSE_STEP;

		work->mode=work->store_mode;
		work->port=work->store_port;
		work->slot=work->store_slot;

		work->step=work->store_step;
		work->sub_step=0;
		return;
	}

	work->mode=MCMAN_MODE_NONE;
	return;
}

int _UpdateMCStatus(MCMAN_WORK *work)
{
	if(MCAccessAutoChecking(&(work->accwork))){
		work->flag|=MCMAN_FLAG_MCCHECKING;
	}
	else{
		work->flag&=~MCMAN_FLAG_MCCHECKING;
	}
	if(MCAccessCheckSeqFlag(&(work->accwork))){
		work->flag|=MCMAN_FLAG_MCCHECKED;
	}
	if(work->flag & (MCMAN_FLAG_MCCHECKED|MCMAN_FLAG_MCCHECKING)){
		work->flag&=~MCMAN_FLAG_PAUSE_STEP;
	}
	return ((work->flag & (MCMAN_FLAG_MCCHECKED|MCMAN_FLAG_MCCHECKING))!=0);
}

static void Act(MCMAN_WORK *work)
{
   Transfarring_LockSaveLoadMutex();
	if(work->rtc_count>=GET_RTC_COUNT){
		GM_GetRTCTime() ;
		work->rtc_count=0;
	}
	work->rtc_count++;

	MCAccessAct(&(work->accwork));

	switch(work->mode){
	case MCMAN_MODE_NONE:
#if 0
		if(MCAccessAutoChecking(&(work->accwork))){
			work->flag|=MCMAN_FLAG_MCCHECKING;
		}
		else{
			work->flag&=~MCMAN_FLAG_MCCHECKING;
		}
		if(MCAccessCheckSeqFlag(&(work->accwork))){
			work->flag|=MCMAN_FLAG_MCCHECKED;
		}
#else
		_UpdateMCStatus(work);
#endif
		break;

	case MCMAN_MODE_MCCHECK:
		work->flag|=MCMAN_FLAG_MCCHECKING;

		if(CheckStep(work)){
			work->mode=MCMAN_MODE_NONE;
			work->flag|=MCMAN_FLAG_MCCHECKED;
			work->flag&=~MCMAN_FLAG_MCCHECKING;

#ifdef DEBUG_MODE
			printf("CheckStep End\n");
#endif
		}
		break;

	case MCMAN_MODE_LOAD_PHOTO:
		switch(LoadPhotoStep(work)){
		case 1:
#ifdef DEBUG_MODE
			printf("Photo Load Finished\n");
#endif
			BackInfoMode(work);
			work->result=1;
			break;
		case -1:
#ifdef DEBUG_MODE
			printf("Photo Load Error\n");
#endif
			BackInfoMode(work);
			work->result=-1;
			break;
		case -2:
#ifdef DEBUG_MODE
			printf("Photo Load Error -> MCCheck\n");
#endif
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;
	case MCMAN_MODE_LOAD_PHOTOICON:
		switch(LoadPhotoIconStep(work)){
		case 1:
			BackInfoMode(work);
			work->result=1;
			break;
		case -1:
			BackInfoMode(work);
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;
	case MCMAN_MODE_PHOTO_FORMAT:
		switch(FormatStep(work)){
		case 1:
			work->mode=MCMAN_MODE_SAVE_PHOTO;
			work->step=0;
			work->flag&=~MCMAN_FLAG_MCFORMATTING;
			break;
		case -1:
			work->mode=MCMAN_MODE_NONE;
			work->result=-1;
			work->flag&=~MCMAN_FLAG_MCFORMATTING;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			work->flag&=~MCMAN_FLAG_MCFORMATTING;
			break;
		}
		break;
	case MCMAN_MODE_SAVE_PHOTO:
		switch(SavePhotoStep(work)){
		case 1:
			BackInfoMode(work);
			work->result=1;
			break;
		case -1:
			BackInfoMode(work);
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;
	case MCMAN_MODE_GET_PHOTO_INFO:
		if(work->flag & MCMAN_FLAG_CHECK_TO_BACK){
			switch(GetPhotoInfoR(work)){
			case 1:
				work->mode=MCMAN_MODE_NONE;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=1;
				break;
			case -1:
				work->mode=MCMAN_MODE_NONE;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=-1;
				break;
			case -2:
				work->mode=MCMAN_MODE_MCCHECK;
				work->step=0;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=-1;
				work->flag|=MCMAN_FLAG_MCCHECKING;
				break;
			}
		}
		else{
			switch(GetPhotoInfo(work)){
			case 1:
				work->mode=MCMAN_MODE_NONE;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=1;
				break;
			case -1:
				work->mode=MCMAN_MODE_NONE;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=-1;
				break;
			case -2:
				work->mode=MCMAN_MODE_MCCHECK;
				work->step=0;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=-1;
				work->flag|=MCMAN_FLAG_MCCHECKING;
				break;
			}
		}
		break;
	case MCMAN_MODE_CHANGE_PHOTO_INFO:
		switch(ChangePhotoInfoStep(work)){
		case 1:
			BackInfoMode(work);
			work->result=1;
			break;
		case -1:
			BackInfoMode(work);
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;

	case MCMAN_MODE_LOAD_GAME:
		switch(LoadGameStep(work)){
		case 1:
			BackInfoMode(work);
			work->result=1;
			break;
		case -1:
#ifdef BP_PS3
         {
            //BP - Assume corruption and put up a system UI to allow user to retry.
            //(though they could just re-enter the screen)
            int result = 0;
            ShowCorruptSaveWarning(&result);
            if (result)
            {
               //Retry!
               work->step = 0;
               break;
            }
         }
#endif
			BackInfoMode(work);
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;
	case MCMAN_MODE_GAME_FORMAT:
		switch(FormatStep(work)){
		case 1:
			work->mode=MCMAN_MODE_SAVE_GAME;
			work->step=0;
			work->flag&=~MCMAN_FLAG_MCFORMATTING;
			break;
		case -1:
			work->mode=MCMAN_MODE_NONE;
			work->result=-1;
			work->flag&=~MCMAN_FLAG_MCFORMATTING;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			work->flag&=~MCMAN_FLAG_MCFORMATTING;
			break;
		}
		break;
	case MCMAN_MODE_SAVE_GAME:
		switch(SaveGameStep(work)){
		case 1:
			BackInfoMode(work);
			work->result=1;

			/* 2001/8/18 K.Kano */
			GM_LastSave=GM_PlayTime;
			break;
		case -1:
			BackInfoMode(work);
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;
	case MCMAN_MODE_GET_GAME_INFO:
		if(work->flag & MCMAN_FLAG_CHECK_TO_BACK){
			switch(GetGameInfoR(work)){
			case 1:
				work->mode=MCMAN_MODE_NONE;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=1;
				break;
			case -1:
				work->mode=MCMAN_MODE_NONE;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=-1;
				break;
			case -2:
				work->mode=MCMAN_MODE_MCCHECK;
				work->step=0;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=-1;
				work->flag|=MCMAN_FLAG_MCCHECKING;
				break;
			}
		}
		else{
			switch(GetGameInfo(work)){
			case 1:
				work->mode=MCMAN_MODE_NONE;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=1;
				break;
			case -1:
				work->mode=MCMAN_MODE_NONE;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=-1;
				break;
			case -2:
				work->mode=MCMAN_MODE_MCCHECK;
				work->step=0;
				if(!(work->flag & MCMAN_FLAG_ALREADY_RETURN_RESULT)) work->result=-1;
				work->flag|=MCMAN_FLAG_MCCHECKING;
				break;
			}
		}
		break;
	case MCMAN_MODE_CHANGE_GAME_INFO:
		switch(ChangeGameInfoStep(work)){
		case 1:
			BackInfoMode(work);
			work->result=1;
			break;
		case -1:
			BackInfoMode(work);
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;

	case MCMAN_MODE_PHOTO_EASY_SEARCH:
		switch(PhotoEasySearchStep(work)){
		case 1:
			work->mode=MCMAN_MODE_NONE;
			work->result=1;
			break;
		case -1:
			work->mode=MCMAN_MODE_NONE;
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;
	case MCMAN_MODE_GAME_EASY_SEARCH:
		switch(GameEasySearchStep(work)){
		case 1:
			work->mode=MCMAN_MODE_NONE;
			work->result=1;
			break;
		case -1:
			work->mode=MCMAN_MODE_NONE;
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;

	case MCMAN_MODE_PHOTO_DIR_SEARCH:
		switch(PhotoDirSearchStep(work)){
		case 1:
			work->mode=MCMAN_MODE_GET_PHOTO_INFO;
			work->step=0;
			break;
		case -1:
			work->mode=MCMAN_MODE_NONE;
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;
	case MCMAN_MODE_GAME_DIR_SEARCH:
		switch(GameDirSearchStep(work)){
		case 1:
			work->mode=MCMAN_MODE_GET_GAME_INFO;
			work->step=0;
			break;
		case -1:
			work->mode=MCMAN_MODE_NONE;
			work->result=-1;
			break;
		case -2:
			work->mode=MCMAN_MODE_MCCHECK;
			work->step=0;
			work->result=-1;
			work->flag|=MCMAN_FLAG_MCCHECKING;
			break;
		}
		break;
   case MCMAN_MODE_DELETE_GAME:
		switch(DeleteGameStep(work)){
      case 0: // In progress
         break;
		case 1: // Success
         work->mode=MCMAN_MODE_NONE;
			work->result=1;
			break;
      default: // Failure
         work->mode=MCMAN_MODE_NONE;
			work->result=-1;
         break;
		}
		break;
   case MCMAN_MODE_DELETE_PHOTO:
      switch(DeletePhotoStep(work)){
      case 0: // In progress
         break;
		case 1: // Success
         work->mode=MCMAN_MODE_NONE;
			work->result=1;
			break;
      default: // Failure
         work->mode=MCMAN_MODE_NONE;
			work->result=-1;
         break;
		}
		break;
	}

   Transfarring_UnlockSaveLoadMutex();
}

static void Die(MCMAN_WORK *work)
{
	MCAccessEnd(&(work->accwork));
	// work->mcsub_work.mcman_work=NULL;
	subwork.mcman_work=NULL;
	mcman_allocated_work=NULL;

	GM_PadResetDisable=0;
}

static void InitMCMan(MCMAN_WORK *work)
{
	static const sceVu0IVECTOR clr={ 0x00,0x00,0x00,0x80, };
	static const sceVu0FVECTOR dir={ 1.0f,0.0f,0.0f,0.0f, };
	static const sceVu0FVECTOR rgb={ 128.0f,128.0f,128.0f,0.0f, };

	MCAccessInit(&(work->accwork),0);
	MCAccessAutoCheck(&(work->accwork),1);

	work->mode=MCMAN_MODE_MCCHECK;
	work->step=0;
	work->flag|=MCMAN_FLAG_MCCHECKING;

	work->file_kind = MCMAN_FILE_KIND_GAME;	// デフォルトはゲーム

#ifdef PSX2	
	/* icon.sysの標準値をセット */
	memset(&(work->iconsys),0x00,sizeof(work->iconsys));

	work->iconsys.Head[0]='P';
	work->iconsys.Head[1]='S';
	work->iconsys.Head[2]='2';
	work->iconsys.Head[3]='D';

	work->iconsys.OffsLF=32;

	work->iconsys.TransRate=0x80;

	/* 本当はFVECTORではないが、サイズが同じなのでFVECTORのCOPYを使った */
	memcpy(&(work->iconsys.BgColor[0]),&clr,sizeof(FVECTOR));
	memcpy(&(work->iconsys.BgColor[1]),&clr,sizeof(FVECTOR));
	memcpy(&(work->iconsys.BgColor[2]),&clr,sizeof(FVECTOR));
	memcpy(&(work->iconsys.BgColor[3]),&clr,sizeof(FVECTOR));

	/* 同じ型宣言、サイズであるが、SONYのものとMGS2用のものと型名が違う */
	memcpy(&(work->iconsys.LightDir[0]),&dir,sizeof(FVECTOR));
	memcpy(&(work->iconsys.LightDir[1]),&dir,sizeof(FVECTOR));
	memcpy(&(work->iconsys.LightDir[2]),&dir,sizeof(FVECTOR));

	memcpy(&(work->iconsys.LightColor[0]),&rgb,sizeof(FVECTOR));
	memcpy(&(work->iconsys.LightColor[1]),&rgb,sizeof(FVECTOR));
	memcpy(&(work->iconsys.LightColor[2]),&rgb,sizeof(FVECTOR));

	memcpy(&(work->iconsys.Ambient),&rgb,sizeof(FVECTOR));

	strcpy(work->iconsys.TitleName,DEFAULT_MC_TITLE_NAME);
	strcpy(work->iconsys.FnameView,ICON_FILENAME);
	strcpy(work->iconsys.FnameCopy,ICON_FILENAME);
	strcpy(work->iconsys.FnameDel,ICON_FILENAME);
#endif	

	work->rtc_count=0;

#if 0
	work->rtc.stat=0;
	work->rtc.second=0;
	work->rtc.minute=0;
	work->rtc.hour=0;
	work->rtc.day=0x01;
	work->rtc.month=0x01;
	work->rtc.year=0x00;
#endif

	work->exist_flag = 0;

}

void MCInitCommand(void)
{
	MCAccessInit(NULL,1);
	// mcman_allocated_work=NULL;
}

void NewMCMan(MCMAN_WORK *work)
{
	if(mcman_allocated_work!=NULL){
#ifdef DEBUG_MODE
		printf("MCMan : Already Started\n");
#endif
		return;
	}

	GV_SetActorClass(work,GV_CLASS_CHARA);
	GV_SetActorKillLevel(work,GV_KILL_LEVEL_NORMAL);
	GV_InsertActorPriority(GV_ACTOR_DAEMON,work,0x1FF);

	GV_SetActor(&(work->actor),Act,Die) ;
	GV_ActorEX(&(work->actor));

	NewMCManSub(work);

	// GV_SetActorChild(work,&(work->mcsub_work));
	GV_SetActorChild(work,&subwork);

	GM_PadResetDisable=1;

	InitMCMan(work);

	mcman_allocated_work=work;
}

///////////////
// 最大ファイル数を得る
int MCMAN_GetDataFileMax( int file_kind ) 
{	
	switch ( file_kind ) {
	case MCMAN_FILE_KIND_GAME:
		return DATA_FILE_MAX;
	case MCMAN_FILE_KIND_VR:
		return VR_FILE_MAX;
	case MCMAN_FILE_KIND_SNAKE_TALES:
		return SNAKE_TALES_FILE_MAX;
#ifdef PSX2		
	case MCMAN_FILE_KIND_FIRSTCHECK:
		return DATA_FILE_MAX;
	case MCMAN_FILE_KIND_OTHER:
		return 1;
#endif		
	}
	return 0;
}
