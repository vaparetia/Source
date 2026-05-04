//-----------------------------------------------------------------------------
// File: 
//
// Desc: DirectShow sample code - adds support for DirectShow videos playing 
//       on a DirectX 8.0 texture surface. Turns the D3D texture tutorial into 
//       a recreation of the VideoTex sample from previous versions of DirectX.
//
// Copyright (c) 2000-2001 Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------

#include	"mgs_type.h"
#include	"libgv.h"
#include	"libgv.cnf"
#include	"libfs.h"
#include	"libdg.h"
#include	"stream.h"
#include	"g_define.h"

#include	"strctrl.h"

#include	"wincriemu.h"
#include	"mts.h"

typedef struct{
	char *data;					/*							*/
								/*	Begin address						*/
	int len;					/*								*/
								/*	Size of chunk length				*/
	int	status;						
	int	pad;						
}SJCKHEAD,*LPSJCKHEAD;

typedef struct {
	struct _sj_vtbl *vtbl;		/*							*/
								/*	Interfaces							*/
	LPSJCKHEAD	lpsjck_top;
	int			ck_ent;
	int			pad;
	int			free_cus;
	int			free_size;
	int			use_cus;
	int			use_size;
	int 		sema;
	int 		sema2;
} SJ_OBJ_JOIN;



//#define LOCK()		WaitSema( sjj->sema )
//#define UNLOCK()	SignalSema( sjj->sema )
#define LOCK()		
#define UNLOCK()	

/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			ＳＪを破棄する
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinLock(SJ sj)
{
	
	SJ_OBJ_JOIN	*sjj;
	sjj = (SJ_OBJ_JOIN*)sj;
//	WaitSema( sjj->sema2 );
}
/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			ＳＪを破棄する
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinUnlock(SJ sj)
{
	
	SJ_OBJ_JOIN	*sjj;
	sjj = (SJ_OBJ_JOIN*)sj;
//	SignalSema( sjj->sema2 );

}
/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			ＳＪを破棄する
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinDestroy(SJ sj)
{
	
	SJ_OBJ_JOIN	*sjj;
	sjj = (SJ_OBJ_JOIN*)sj;
	sjuni_JoinLock(sjj);
	LOCK();
//	DeleteSema( sjj->sema );
//	DeleteSema( sjj->sema2 );
	GV_Free(sj);
}
/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			ＳＪをリセットする
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinReset(SJ sj)
{
	SJ_OBJ_JOIN	*sjj;
	sjj = (SJ_OBJ_JOIN*)sj;
	LOCK();
	memset(sjj->lpsjck_top,0,sjj->ck_ent<< 4);
	UNLOCK();
}


/*---------------------------------------------------------------------------
|
|		ブランクサーチ
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static int sjuni_JoinBrankSeach(SJ_OBJ_JOIN *sjj,int start)
{
	int		i,j;
	for(i = 0 ,j = start ; i < sjj->ck_ent ; i++){
		if(j >= sjj->ck_ent){
			j = 0;
		}
		if(!sjj->lpsjck_top[j].data){
			return(j);
		}
		
		j ++;
	}
	while(1){
		Sleep(10000);
		printf("------------------------------------------------------\n");
		};
	return(-1);
}
/*---------------------------------------------------------------------------
|
|		ブランクサーチ
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static int sjuni_JoinStatusSeach(SJ_OBJ_JOIN *sjj,int start,int status)
{
	int		i,j;
	for(i = 0 ,j = start ; i < sjj->ck_ent ; i++){
		if(j >= sjj->ck_ent){
			j = 0;
		}
		if(sjj->lpsjck_top[j].status == status
		 &&sjj->lpsjck_top[j].data){
			return(j);
		}
		
		j ++;
	}
	return(-1);
}
/*---------------------------------------------------------------------------
|
|		ブランクサーチ
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinStatDisp(SJ_OBJ_JOIN *sjj)
{
	int		i;
	for(i = 0  ; i < sjj->ck_ent ; i++){
		if(sjj->lpsjck_top[i].data){
			printf("%d tbl(%d) : 0x%08x , 0x%08x(%d)\n",i,sjj->lpsjck_top[i].status,sjj->lpsjck_top[i].data,sjj->lpsjck_top[i].len,sjj->lpsjck_top[i].len);
		}else{
			printf("%d tbl( ) :\n",i,sjj->lpsjck_top[i].status);
		}
	}
}

/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			チャンクを取得する
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinGetChunkSub(SJ sj, int id, int nbyte, SJCK *ck)
{
	
	SJ_OBJ_JOIN	*sjj;
	int			j,inx;
	int			current_cus;
	sjj = (SJ_OBJ_JOIN*)sj;
	
	if(id == SJ_LIN_FREE){
		current_cus = sjj->free_cus;
	}else{
		current_cus = sjj->use_cus;
	}
#ifdef DEBUG_MODE
//@		printf("SJ_Get mode %d cursor %x CK 0x%x \n",id,current_cus ,nbyte);
//@		sjuni_JoinStatDisp(sjj);
#endif

	j = current_cus;
	if(sjj->lpsjck_top[j].status == id
	 &&sjj->lpsjck_top[j].data){
		if(sjj->lpsjck_top[j].len <= nbyte){
			ck->data 	= sjj->lpsjck_top[j].data;
			ck->len 	= sjj->lpsjck_top[j].len;
			sjj->lpsjck_top[j].data = NULL;
		}else{
			ck->data 	= sjj->lpsjck_top[j].data;
			ck->len 	= nbyte;
			sjj->lpsjck_top[j].data += nbyte;
			sjj->lpsjck_top[j].len 	-= nbyte;
		}
		
		inx = sjuni_JoinStatusSeach(sjj,current_cus,id);
		if(inx <0){
			current_cus = 0;
		}else{
			current_cus = inx;
		}
		if(id == SJ_LIN_FREE){
			sjj->free_cus = current_cus ;
			sjj->free_size -= ck->len;
		}else{
			sjj->use_cus = current_cus ;
			sjj->use_size -= ck->len;
		}
	}else{
		ck->data 	= NULL;
		ck->len 	= 0;
	}

}
/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			チャンクを取得する
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinGetChunk(SJ sj, int id, int nbyte, SJCK *ck)
{
	SJ_OBJ_JOIN	*sjj;
	sjj = (SJ_OBJ_JOIN*)sj;
	LOCK();
	sjuni_JoinGetChunkSub(sj, id, nbyte, ck);
	UNLOCK();
}

/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			チャンクを戻す
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinUngetChunkSub(SJ sj, int id, SJCK *ck)
{
	SJ_OBJ_JOIN	*sjj;
	int		i,j,inx;
	int		current_cus;
	int		flag;
	SJCK 	ck2;
	sjj = (SJ_OBJ_JOIN*)sj;
	
	if(id == SJ_LIN_FREE){
		current_cus = sjj->free_cus;
		sjj->free_size += ck->len;
#ifdef DEBUG_MODE
//@		printf("SJ_Unget mode %d cursor %x CK 0x%x 0x%x \n",id,current_cus ,ck->data,ck->len);
//@		sjuni_JoinStatDisp(sjj);
#endif
		for(i = 0 ,j = current_cus ; i < sjj->ck_ent ; i++){
			if(j >= sjj->ck_ent){
				j = 0;
			}
			if(sjj->lpsjck_top[j].status == id
			 &&sjj->lpsjck_top[j].data){
				if((sjj->lpsjck_top[j].data) == ck->data + ck->len){
					sjj->lpsjck_top[j].data = ck->data;
					sjj->lpsjck_top[j].len += ck->len;
					sjuni_JoinGetChunkSub(sj, id,0x7fffffff ,&ck2);
					sjuni_JoinUngetChunkSub(sj, id,&ck2);
					
#ifdef DEBUG_MODE
//@					printf("join\n");
#endif
					return;
				}
				if((sjj->lpsjck_top[j].data + sjj->lpsjck_top[j].len) == ck->data){
					sjj->lpsjck_top[j].len += ck->len;
					
					sjuni_JoinGetChunkSub(sj, id,0x7fffffff ,&ck2);
					sjuni_JoinUngetChunkSub(sj, id,&ck2);
#ifdef DEBUG_MODE
//@					printf("join\n");
#endif
					return;
				}
			}
			
			j ++;
		}
	}else{
		current_cus = sjj->use_cus;
		sjj->use_size += ck->len;
#ifdef DEBUG_MODE
//@		printf("SJ_Unget mode %d cursor %x CK 0x%x 0x%x \n",id,current_cus ,ck->data,ck->len);
//@		sjuni_JoinStatDisp(sjj);
#endif
		for(i = 0 ,j = current_cus ; i < sjj->ck_ent ; i++){
			if(j >= sjj->ck_ent){
				j = 0;
			}
			if(sjj->lpsjck_top[j].status == id
			 &&sjj->lpsjck_top[j].data){
				if((sjj->lpsjck_top[j].data) == ck->data + ck->len){
					sjj->lpsjck_top[j].data = ck->data;
					sjj->lpsjck_top[j].len += ck->len;
#ifdef DEBUG_MODE
//@					printf("join\n");
#endif
					return;
				}
			}
			
			j ++;
		}
	}



	inx = sjuni_JoinBrankSeach(sjj,current_cus);
	sjj->lpsjck_top[inx].data = ck->data;
	sjj->lpsjck_top[inx].len = ck->len;
	sjj->lpsjck_top[inx].status = id;
	if(id == SJ_LIN_FREE){
		sjj->free_cus = inx;
	}else{
		sjj->use_cus = inx;
	}
#ifdef DEBUG_MODE
//@					printf("insert\n");
#endif
}
/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			チャンクを戻す
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinUngetChunk(SJ sj, int id, SJCK *ck)
{
	SJ_OBJ_JOIN	*sjj;
	sjj = (SJ_OBJ_JOIN*)sj;
	LOCK();
	sjuni_JoinUngetChunkSub(sj, id, ck);
	UNLOCK();
}
/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			チャンクを挿入
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinPutChunkSub(SJ sj, int id, SJCK *ck)
{
 	SJ_OBJ_JOIN	*sjj;
 	int		i,j,inx;
 	int		current_cus;
 	int		flag;
 	SJCK	ck2;

	
	sjj = (SJ_OBJ_JOIN*)sj;
	
	if(id == SJ_LIN_FREE){
		current_cus = sjj->free_cus;
		sjj->free_size += ck->len;
#ifdef DEBUG_MODE
//@		printf("SJ_Put mode %d cursor %x CK 0x%x 0x%x \n",id,current_cus ,ck->data,ck->len);
//@		sjuni_JoinStatDisp(sjj);
#endif

		flag = FALSE;
		for(i = 0 ,j = current_cus ; i < sjj->ck_ent ; i++){
			if(j >= sjj->ck_ent){
				j = 0;
			}
			if(sjj->lpsjck_top[j].status == id
			 &&sjj->lpsjck_top[j].data){
				if((sjj->lpsjck_top[j].data + sjj->lpsjck_top[j].len) == ck->data){
					sjj->lpsjck_top[j].len += ck->len;
					
					sjuni_JoinGetChunkSub(sj, id,0x7fffffff ,&ck2);
					sjuni_JoinUngetChunkSub(sj, id,&ck2);
#ifdef DEBUG_MODE
//@					printf("join\n");
#endif
					return;
				}
				if((sjj->lpsjck_top[j].data) == ck->data + ck->len){
					sjj->lpsjck_top[j].data = ck->data;
					sjj->lpsjck_top[j].len += ck->len;
					sjuni_JoinGetChunkSub(sj, id,0x7fffffff ,&ck2);
					sjuni_JoinUngetChunkSub(sj, id,&ck2);
#ifdef DEBUG_MODE
//@					printf("join\n");
#endif
					return;
				}
				flag = TRUE;
			}
			
			j ++;
		}
	}else{
		current_cus = sjj->use_cus;
		sjj->use_size += ck->len;
#ifdef DEBUG_MODE
//@		printf("SJ_Put mode %d cursor %x CK 0x%x 0x%x \n",id,current_cus ,ck->data,ck->len);
//@		sjuni_JoinStatDisp(sjj);
#endif

		flag = FALSE;
		for(i = 0 ,j = current_cus ; i < sjj->ck_ent ; i++){
			if(j >= sjj->ck_ent){
				j = 0;
			}
			if(sjj->lpsjck_top[j].status == id
			 &&sjj->lpsjck_top[j].data){
				if((sjj->lpsjck_top[j].data + sjj->lpsjck_top[j].len) == ck->data){
					sjj->lpsjck_top[j].len += ck->len;
#ifdef DEBUG_MODE
//@					printf("join\n");
#endif
					return;
				}
				flag = TRUE;
			}
			
			j ++;
		}
	}

	inx = sjuni_JoinBrankSeach(sjj,current_cus);
	sjj->lpsjck_top[inx].data = ck->data;
	sjj->lpsjck_top[inx].len = ck->len;
	sjj->lpsjck_top[inx].status = id;
	if(!flag){
		if(id == SJ_LIN_FREE){
			sjj->free_cus = inx;
		}else{
			sjj->use_cus = inx;
		}
	}
	
#ifdef DEBUG_MODE
//@					printf("insert\n");
#endif
}
/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			チャンクを挿入
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static void sjuni_JoinPutChunk(SJ sj, int id, SJCK *ck)
{
	SJ_OBJ_JOIN	*sjj;
	sjj = (SJ_OBJ_JOIN*)sj;
	LOCK();
	sjuni_JoinPutChunkSub(sj, id, ck);
	UNLOCK();
	
}
/*---------------------------------------------------------------------------
|
|		ＳＪのメソッド
|
|			取得できるデータのバイト数の取得
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
static int sjuni_JoinGetNumData(SJ sj, int id)
{
	SJ_OBJ_JOIN	*sjj;
	sjj = (SJ_OBJ_JOIN*)sj;
	if(id == SJ_LIN_FREE){
		return(sjj->free_size);
	}else{
		return(sjj->use_size);
	}
}
static SJ_IF sj_if = {
	 NULL
	,NULL
	,NULL
	,sjuni_JoinDestroy
	,NULL
	,sjuni_JoinReset
	,sjuni_JoinLock
	,sjuni_JoinUnlock
	,sjuni_JoinGetChunk
	,sjuni_JoinUngetChunk
	,sjuni_JoinPutChunk
	,sjuni_JoinGetNumData
	,NULL
	,NULL
};
/*---------------------------------------------------------------------------
|
|		ＳＪＵＮＩ作成？
|
|	Parameters
|
|	Returns
|
*----------------------------------------------------------------------------*/
SJ 		SJUNI_Create( int mode, char *work, int size )
{
	SJ_OBJ_JOIN		*sj;
	struct SemaParam sema;

	sj = (SJ_OBJ_JOIN*)GV_Malloc( sizeof(SJ_OBJ_JOIN));
	memset(sj,0,sizeof(SJ_OBJ_JOIN));
	sj->vtbl = (struct _sj_vtbl *)(&sj_if);
	
	sj->lpsjck_top = (LPSJCKHEAD)work;
	sj->ck_ent = size>> 4;
	memset(work,0,size);

	ZeroMemory( &sema, sizeof(sema) );
	sema.initCount = 1;
	sema.maxCount = 1;

//	sj->sema = CreateSema( &sema );
//	ASSERT( sj->sema >= 0 );
//	sj->sema2 = CreateSema( &sema );
//	ASSERT( sj->sema2 >= 0 );
	return((SJ)(sj));
};






/**--------------------------------------------------------------------------------------------------------------*/
/**--------------------------------------------------------------------------------------------------------------*/











int 	SFVSJD_CalcWork( LPVOID a){return(0);};
SFVSJD 	SFVSJD_Create( SJ sji , SJ sjo, LPVOID a, char *pSjd_work, int sjd_worksize ){return(NULL);};
void 	SFVSJD_Start( SFVSJD sjd ){};
void	SFVSJD_ExecHndl( SFVSJD sjd ){};
int 	SFVSJD_GetStat( SFVSJD sjd ){return(0);};
void 	SFVSJD_Finish(void){};
void 	SFVSJD_Init(void){};
void 	SFVSJD_Destroy(SFVSJD 	sj){};

void 	ADXM_SetCbErr(void *a,void *b){};


