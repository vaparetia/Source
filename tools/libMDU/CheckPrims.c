/*
   CheckPrims.c

   モデルユーティリティ for MGS2 on linux

   by K.Kano 2000/4/11 ～ 

   $Id: CheckPrims.c,v 1.7 2002/08/26 11:22:09 usr01363 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "MDU_util.h"
#include "MDU_mdl.h"
#include "MDU_tex.h"

#include "fmt_kms.h"

#include "ToStrip.h"

/*----------------------------------------------------------------------*/

/* 取り出した法線とエンベロープの情報の関連をチェックする。
   CV2を使った環境マッピングの仕様で、法線にエンベロープ情報を加える必要が
   あったため。 */
void Kmx_CheckNormal(KMX_DEF *def)
{
    KMX_MDL *mdl;
    int i,j,k;

    struct _check_normal {
	int flag;
	KMX_ENV env;
    } *check_normal;
    int n_check_normal;

    /* 進行チェック用 */
    printf("Check Normal for CV2\n");

    mdl=def->models;
    for(i=0;i<def->n_x_models;i++,mdl++){
	check_normal
	    =(struct _check_normal *)MDU_Alloc(sizeof(struct _check_normal)*mdl->n_norms);
	if(check_normal==NULL) continue;

	// printf("Check 1\n");

	memset(check_normal,0x00,sizeof(struct _check_normal)*mdl->n_norms);
	n_check_normal=mdl->n_norms;

	/* 進行チェック用 */
	printf(".");
	fflush(stdout);

	for(j=0;j<mdl->n_prims;j++){
	    KMX_PRIMS *prim=mdl->prims+j;

	    for(k=0;k<prim->n_id;k++){
		int vid,nid;

		vid=*(prim->vid+k);
		nid=*(prim->nid+k);

		if(check_normal[nid].flag==0){
		    check_normal[nid].env=(mdl->verts+vid)->env;
		    check_normal[nid].flag=1;
		}
		else if(check_normal[nid].env.val[0]!=(mdl->verts+vid)->env.val[0] ||
			check_normal[nid].env.val[1]!=(mdl->verts+vid)->env.val[1] ||
			check_normal[nid].env.objid!=(mdl->verts+vid)->env.objid ||
			check_normal[nid].env.parentid!=(mdl->verts+vid)->env.parentid ||
			check_normal[nid].env.id!=(mdl->verts+vid)->env.id){

		    struct _check_normal *check_normal2;
		    KMX_NORMS *norms2;

		    /* 進行チェック用 */
		    printf("!");
		    fflush(stdout);

		    check_normal2
			=(struct _check_normal *)MDU_Alloc(sizeof(struct _check_normal)
							   *(n_check_normal+1));
		    if(check_normal2==NULL) continue;
		    memcpy(check_normal2,check_normal,
			   sizeof(struct _check_normal)*n_check_normal);
		    MDU_Free(check_normal);
		    check_normal=check_normal2;

		    if((norms2=Kmx_Norms_Alloc(n_check_normal+1))==NULL) continue;
		    memcpy(norms2,mdl->norms,sizeof(KMX_NORMS)*n_check_normal);
		    Kmx_Norms_Free(mdl->norms);
		    mdl->norms=norms2;

		    *(mdl->norms+n_check_normal)=*(mdl->norms+nid);

		    nid=*(prim->nid+k)=n_check_normal;

		    check_normal[nid].env=(mdl->verts+vid)->env;
		    check_normal[nid].flag=1;

		    n_check_normal++;
		}
	    }
	}

	mdl->n_norms=n_check_normal;
	MDU_Free(check_normal);
    }

    /* 進行チェック用 */
    printf("\n");
}

/*----------------------------------------------------------------------*/

int Kmx_CheckTexture(KMX_DEF *def,int check_flag)
{
    KMX_MDL *mdl;
    int i,j,k;
    int ans=0;
    int *flags;

    if((flags=(int *)MDU_Alloc(sizeof(int)*def->n_texs))==NULL){
	printf("Kmx_CheckTexture : Memory Overflow !!\n");
	return 1;
    }
    for(i=0;i<def->n_texs;i++) *(flags+i)=0;

    mdl=def->models;
    for(i=0;i<def->n_x_models;i++,mdl++){
	u_int type=mdl->type;
	KMX_PRIMS *prim=mdl->prims;
	int flag=0;
	int flag2=0;

	for(j=0;j<mdl->n_prims;j++,prim++){
	    if(prim->n_tid>0){
		for(k=0;k<prim->n_tid;k++){
		    int tid=prim->tid[k];

		    if(type & DG_TYPE_TRANS){
			if(!IsTransTexture(def->texs[tid].name) ||
			   IsOverlayTexture(def->texs[tid].name)){

			    flag=1;

			    if(!*(flags+tid)){
				printf("Non-Trans Texture : %s\n",def->texs[tid].name);
				*(flags+tid)=1;
			    }
			}
		    }
		    else{
			if(IsTransTexture(def->texs[tid].name) &&
			   !IsOverlayTexture(def->texs[tid].name)){

			    flag=1;

			    if(!*(flags+tid)){
				printf("Trans Texture : %s\n",def->texs[tid].name);
				*(flags+tid)=1;
			    }
			}
		    }
		}
	    }

	    if(prim->n_tid>N_KMS_MULTI_TEXTURES){
		int tid=prim->tid[N_KMS_MULTI_TEXTURES];

		flag2=1;

		if(!*(flags+tid)){
		    printf("Over Textures : %s\n",def->texs[tid].name);
		    *(flags+tid)=1;
		}
	    }
	}

	if(flag){
	    if(type & DG_TYPE_TRANS){
		printf("Object %d : Trans Obj has non-trans texture.\n",i);
	    }
	    else{
		printf("Object %d : Non-trans Obj has trans texture.\n",i);
	    }
	    ans=1;
	}
	if(flag2){
	    printf("Object %d : Over textures on one primitive.\n",i);
	    if(check_flag) ans=1;
	}
    }

    MDU_Free(flags);

    return ans;
}

/*----------------------------------------------------------------------*/

int Evf_CheckTexture(EVF_DEF *def,int check_flag)
{
    EVF_MESH *mesh;
    EVF_PRIMS *prim;
    int j,k;
    int ans=0;
    int *flags;

    if((flags=(int *)MDU_Alloc(sizeof(int)*def->n_texs))==NULL){
	printf("Evm_CheckTexture : Memory Overflow !!\n");
	return 1;
    }
    for(j=0;j<def->n_texs;j++) *(flags+j)=0;

    mesh=&(def->mesh);
    prim=mesh->prims;

    for(j=0;j<mesh->n_prims;j++,prim++){
	if(prim->n_tid>0){
	    int flag=(IsTransTexture(def->texs[0].name) && !IsOverlayTexture(def->texs[0].name));

	    for(k=1;k<prim->n_tid;k++){
		int tid=prim->tid[k];

		if(flag!=(IsTransTexture(def->texs[tid].name) &&
			  !IsOverlayTexture(def->texs[tid].name))){

		    printf("Primitive has trans texture and non-trans texture.\n");

		    if(!*(flags+tid)){
			printf("Trans Texture : %s\n",def->texs[tid].name);
			*(flags+tid)=1;
		    }

		    ans=1;
		}
	    }
	    if(prim->n_tid>1){
		if(flag){
		    printf("Multi texture is not permitted trans-texture.\n");
		    ans=1;
		}
	    }
	}
	if(prim->n_tid>N_EVM_MULTI_TEXTURES){
	    int tid=prim->tid[N_EVM_MULTI_TEXTURES];

	    printf("Over textures on one primitive.\n");

	    if(!*(flags+tid)){
		printf("Over Textures : %s\n",def->texs[tid].name);
		*(flags+tid)=1;
	    }

	    if(check_flag) ans=1;
	}
    }

    MDU_Free(flags);

    return ans;
}

/*----------------------------------------------------------------------*/
