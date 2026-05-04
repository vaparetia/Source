//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	va_demo.c
		デモ用頂点アニメーション

	2001/06/12 K.Kano
	$Id: va_demo.c,v 1.1.1.3 2002/11/19 11:43:43 Yoshizawa1 Exp $
*/

#if defined(DEBUG) || defined(DEBUG_MODE)
//#define L_DEBUG_MODE
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include "gameheader.h"
#include "libutl.h"


/* ワーク */
typedef	struct _Work {
    GV_ACT_EX	actor;		/* リンク */
	DG_OBJS		*objs;
	int			name;
	int			n_models;
	int 		n_cv2s;
    VERTEX_ANIME_WORK *va[0];
} Work ;


#define	OBJECT_FLAG	(DG_FLAG_TEXT|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

#define N_CV2_FILES		6


enum {
	VA_MSG_ANIME=0,
};


/* ------------------------------------------------------------------------ */
/* メイン処理 */

static void Act(Work *work)
{
    GV_MSG *msg;
    int n_msg ;
	int i;

    n_msg=GV_ReceiveMessage(work->name,&msg);

    while(n_msg>0){
		int command=*(msg->message+0);
		int *arg=msg->message+1;

		switch(command){
		case VA_MSG_ANIME:
			{
				int count;
				float p[N_CV2_FILES];
				int j;

				count=*(arg+0);
				for(i=0;i<N_CV2_FILES;i++) p[i]=(float)*(arg+1+i)/100.0f;

				for(i=0;i<work->n_models;i++){
					for(j=0;j<work->n_cv2s;j++){
						work->va[i]->p[j]=p[j];
					}
					work->va[i]->count=count;
				}
			}
			break;
		}

		msg++;
		n_msg--;
    }

	for(i=0;i<work->n_models;i++){
		SimpleVertexAnimation(work->va[i]);
	}
}

static void Die(Work *work)
{
	int i;
	for(i=0;i<work->n_models;i++){
		ExitVertexAnimation(work->va[i]);
	}
}


/* ------------------------------------------------------------------------ */
/* 初期化部 */


/* 資源を獲得 */
static int GetResources(Work *work,int n_cv2_files,int *cv2_filecode)
{
	DG_OBJ *obj;
	CV2_DEF *cv2[N_CV2_FILES];
	CV2_MDL *cv2mdl;
	int i,j;

	for(i=0;i<n_cv2_files;i++){
		cv2[i]=(CV2_DEF*)GV_GetCache(GV_CacheID(cv2_filecode[i],'c'));
		if(cv2[i]==NULL) return 0;
		if(work->n_models!=cv2[i]->n_models) return 0;
	}

	obj=work->objs->objs;
	cv2mdl=cv2[0]->models;

	for(i=0;i<work->n_models;i++,obj++,cv2mdl++){
		work->va[i]=InitVertexAnimation(obj,cv2mdl,DG_VANIME_VERTS,n_cv2_files);

		if(work->va[i]==NULL){
			for(i--;i>=0;i--) ExitVertexAnimation(work->va[i]);
			return 0;
		}

		for(j=0;j<n_cv2_files;j++){
			work->va[i]->key[j]=&(cv2[j]->models[i]);
			work->va[i]->p[j]=0.0f;
		}
		work->va[i]->p[0]=1.0f;
		work->va[i]->count=0;
	}

    return 1;
}

/* 初期化部メイン */
void *NewVADemo(int name,DG_OBJS *objs,
				int cv2_file0,int cv2_file1,int cv2_file2,int cv2_file3,
				int cv2_file4,int cv2_file5)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_USER,sizeof(Work)+sizeof(VERTEX_ANIME_WORK *)*objs->n_models);

    if(work!=NULL) {
		int cv2_files=0;
		int cv2_filecode[N_CV2_FILES];

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX(&(work->actor));
		GV_SetActorMessageKill(&(work->actor),name);

		work->name=name;
		work->objs=objs;
		work->n_models=objs->n_models;

		if(cv2_file0!=0 && cv2_file0!=1){
			cv2_filecode[cv2_files]=cv2_file0;
			cv2_files++;

			if(cv2_file1!=0 && cv2_file1!=1){
				cv2_filecode[cv2_files]=cv2_file1;
				cv2_files++;

				if(cv2_file2!=0 && cv2_file2!=1){
					cv2_filecode[cv2_files]=cv2_file2;
					cv2_files++;

					if(cv2_file3!=0 && cv2_file3!=1){
						cv2_filecode[cv2_files]=cv2_file3;
						cv2_files++;

						if(cv2_file4!=0 && cv2_file4!=1){
							cv2_filecode[cv2_files]=cv2_file4;
							cv2_files++;

							if(cv2_file5!=0 && cv2_file5!=1){
								cv2_filecode[cv2_files]=cv2_file5;
								cv2_files++;
							}
						}
					}
				}
			}
		}
		else{
			GV_DestroyActor(work) ;
			return NULL ;
		}

		work->n_cv2s=cv2_files;

		if(!GetResources(work,cv2_files,cv2_filecode)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
    }
    return (void *)work ;
}
