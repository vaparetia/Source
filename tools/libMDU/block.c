/*
   block.c

   構造体用メモリ管理関数群

   by K.Kano , 2/24/2000

   $Id: block.c,v 1.10 2002/08/26 11:22:09 usr01363 Exp $
   
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
//#include <SFDLINUX.h>

//#include "METALGEAR.h"
#include "fmt_kms.h"

#include "MDU_util.h"
#include "MDU_mdl.h"

#include "block.h"

/*-----------------------------------------------------------------*/

FVECTOR *Cv2_Verts_Alloc(int n_verts,long *size)
{
    FVECTOR *v;
    long lsize=sizeof(FVECTOR)*n_verts;

    if((v=(FVECTOR *)MDU_Alloc(sizeof(FVECTOR)*n_verts))==NULL) return NULL;

    if(size!=NULL) *size=lsize;

    return v;
}

FVECTOR *Cv2_Norms_Alloc(int n_norms,long *size)
{
    return Cv2_Verts_Alloc(n_norms,size);
}

FVECTOR *Cv2_Uvs_Alloc(int n_uvs,long *size)
{
    return Cv2_Verts_Alloc(n_uvs,size);
}

short *Cv2_VertsIndex_Alloc(int n_verts_index,long *size)
{
    short *index;
    long lsize=sizeof(short)*n_verts_index;

    if((index=(short *)MDU_Alloc(sizeof(short)*n_verts_index))==NULL) return NULL;

    if(size!=NULL) *size=lsize;

    return index;
}

short *Cv2_NormsIndex_Alloc(int n_norms_index,long *size)
{
    return Cv2_VertsIndex_Alloc(n_norms_index,size);
}

short *Cv2_UvsIndex_Alloc(int n_uvs_index,long *size)
{
    return Cv2_VertsIndex_Alloc(n_uvs_index,size);
}

VERTEX_USERDATA *Cv2_VertUsrdata_Alloc(int n_verts,long *size)
{
    VERTEX_USERDATA *v;
    long lsize=sizeof(VERTEX_USERDATA)*n_verts;

    if((v=(VERTEX_USERDATA *)MDU_Alloc(sizeof(VERTEX_USERDATA)*n_verts))
       ==NULL) return NULL;

    if(size!=NULL) *size=lsize;

    return v;
}

NORMAL_USERDATA *Cv2_NormUsrdata_Alloc(int n_norms,long *size)
{
    NORMAL_USERDATA *n;
    long lsize=sizeof(NORMAL_USERDATA)*n_norms;

    if((n=(NORMAL_USERDATA *)MDU_Alloc(sizeof(NORMAL_USERDATA)*n_norms))
       ==NULL) return NULL;

    if(size!=NULL) *size=lsize;

    return n;
}

CV2_OBJS *Cv2_Alloc(int n_models,long *size)
{
    CV2_OBJS *def;
    int i;
    long lsize=sizeof(CV2_OBJS)+sizeof(CV2_OBJ)*n_models;

    if((def=(CV2_OBJS *)MDU_Alloc(lsize))==NULL) return NULL;

    def->id=1160797; /* = StrCode("Common Vertex Data ver102") */
    def->n_objs=n_models;

    for(i=0;i<n_models;i++){
	def->objs[i].n_verts=0;
	def->objs[i].n_verts_index=0;
	def->objs[i].verts=NULL;
	def->objs[i].verts_index=NULL;

	def->objs[i].n_norms=0;
	def->objs[i].n_norms_index=0;
	def->objs[i].norms=NULL;
	def->objs[i].norms_index=NULL;

	def->objs[i].n_uvs=0;
	def->objs[i].n_uvs_index=0;
	def->objs[i].uvs=NULL;
	def->objs[i].uvs_index=NULL;

	def->objs[i].vert_usrdata=NULL;
	def->objs[i].norm_usrdata=NULL;
    }

    if(size!=NULL) *size=lsize;

    return def;
}


void Cv2_Free(CV2_OBJS *def)
{
    int i;
    for(i=0;i<def->n_objs;i++){
	if(def->objs[i].verts!=NULL) MDU_Free(def->objs[i].verts);
	if(def->objs[i].verts_index!=NULL) MDU_Free(def->objs[i].verts_index);

	if(def->objs[i].norms!=NULL) MDU_Free(def->objs[i].norms);
	if(def->objs[i].norms_index!=NULL) MDU_Free(def->objs[i].norms_index);

	if(def->objs[i].uvs!=NULL) MDU_Free(def->objs[i].uvs);
	if(def->objs[i].uvs_index!=NULL) MDU_Free(def->objs[i].uvs_index);

	if(def->objs[i].vert_usrdata!=NULL) MDU_Free(def->objs[i].vert_usrdata);
	if(def->objs[i].norm_usrdata!=NULL) MDU_Free(def->objs[i].norm_usrdata);
    }
}


void Cv2Obj_Move(CV2_OBJ *r,CV2_OBJ *a)
{
    memcpy(r,a,sizeof(CV2_OBJ));

    a->n_verts=0;
    a->verts=NULL;
    a->n_verts_index=0;
    a->verts_index=NULL;
    a->n_norms=0;
    a->norms=NULL;
    a->n_norms_index=0;
    a->norms_index=NULL;
    a->n_uvs=0;
    a->uvs=NULL;
    a->n_uvs_index=0;
    a->uvs_index=NULL;

    a->vert_usrdata=NULL;
    a->norm_usrdata=NULL;
}

/*-----------------------------------------------------------------*/

KMS2_DEF *Kms2_Alloc(int n_models,int n_x_models,long *size)
{
    KMS2_DEF *def;
    int i;
    long lsize=sizeof(KMS2_DEF)+sizeof(KMS2_MDL)*n_x_models;

    if((def=(KMS2_DEF *)MDU_Alloc(lsize))==NULL) return NULL;

    // printf("KMS2 Def Alloc = 0x%08x\n",(int)def);

    def->n_models=n_models;
    def->n_x_models=n_x_models;
    def->texture=0;

    for(i=0;i<n_x_models;i++){
	def->models[i].type=0;
	def->models[i].parent=-1;
	def->models[i].n_packs=0;
	def->models[i].packs=NULL;

	/* 2002/6/4  K.Kano
	   X-BOX用フォーマットのための初期化を追加 */
	def->models[i].vbuff=NULL;
	def->models[i].stride=0;
	def->models[i].n_verts=0;
    }

    if(size!=NULL) *size=lsize;

    return def;
}

KMS2_MDLPACK *Kms2_MdlPack_Alloc(int n_packs,long *size)
{
    KMS2_MDLPACK *pack;
    int i;
    long lsize=sizeof(KMS2_MDLPACK)*n_packs;

    if((pack=(KMS2_MDLPACK *)MDU_Alloc(lsize))==NULL) return NULL;

    //printf("KMS2 Packs Alloc = 0x%08x\n",(int)pack);

    for(i=0;i<n_packs;i++){
	(pack+i)->flag=0;
	(pack+i)->n_verts=0;
	(pack+i)->tex_id[0]=0;
	(pack+i)->tex_id[1]=0;
	(pack+i)->tex_id[2]=0;
	(pack+i)->verts=NULL;
	(pack+i)->norms=NULL;
	(pack+i)->uvs[0]=NULL;
	(pack+i)->uvs[1]=NULL;
	(pack+i)->uvs[2]=NULL;
	(pack+i)->rgbs=NULL;

	(pack+i)->n_indices=0;
	(pack+i)->index=NULL;
    }

    if(size!=NULL) *size=lsize;

    return pack;
}

short *Kms2_Verts_Alloc(int n_verts,long *size)
{
    long lsize=sizeof(short)*n_verts*4;
    if(size!=NULL) *size=lsize;
    return (short *)MDU_Alloc(lsize);
}

short *Kms2_Norms_Alloc(int n_verts,long *size)
{
    return Kms2_Verts_Alloc(n_verts,size);
}

short *Kms2_Uvs_Alloc(int n_verts,long *size)
{
    long lsize=sizeof(short)*n_verts*2;
    if(size!=NULL) *size=lsize;
    return (short *)MDU_Alloc(lsize);
}

unsigned short *Kms2_Index_Alloc(int n_verts,long *size)
{
    long lsize=sizeof(short)*n_verts;
    if(size!=NULL) *size=lsize;
    return (short *)MDU_Alloc(lsize);
}

void *Kms2_VBuff_Alloc(int n_xverts,int multi_flag,long *size)
{
    long lsize;
    if(multi_flag) lsize=n_xverts*sizeof(KMSM_VERTEX);
    else lsize=n_xverts*sizeof(KMSS_VERTEX);
    if(size!=NULL) *size=lsize;
    return (short *)MDU_Alloc(lsize);
}

void Kms2_Verts_Free(short *verts)
{
    MDU_Free(verts);
}

void Kms2_Norms_Free(short *norms)
{
    MDU_Free(norms);
}

void Kms2_Uvs_Free(short *uvs)
{
    MDU_Free(uvs);
}

void Kms2_Index_Free( unsigned short* p )
{	
	MDU_Free( p );
}

void Kms2_VBuff_Free(void *vbuff)
{
    MDU_Free(vbuff);
}

void Kms2_MdlPack_Free(KMS2_MDLPACK *pack,int n_packs)
{
    int i;

    // printf("KMS2 Packs Free = 0x%08x\n",(int)pack);

    for(i=0;i<n_packs;i++){
	if((pack+i)->verts!=NULL) Kms2_Verts_Free((pack+i)->verts);
	if((pack+i)->norms!=NULL) Kms2_Norms_Free((pack+i)->norms);
	if((pack+i)->uvs[0]!=NULL) Kms2_Uvs_Free((pack+i)->uvs[0]);
	if((pack+i)->uvs[1]!=NULL) Kms2_Uvs_Free((pack+i)->uvs[1]);
	if((pack+i)->uvs[2]!=NULL) Kms2_Uvs_Free((pack+i)->uvs[2]);

	if((pack+i)->index != NULL) Kms2_Index_Free( (pack+i)->index );

#if 0
	if((pack+i)->rgbs!=NULL) Kms2_Rgbs_Free((pack+i)->rgbs);
#endif

    }
    MDU_Free(pack);
}

void Kms2_Free(KMS2_DEF *def)
{
    KMS2_MDL *mdl;
    int i;

    //printf("KMS2 Def Free = 0x%08x\n",(int)def);

    mdl=def->models;
    for(i=0;i<def->n_x_models;i++,mdl++){
	if(mdl->packs!=NULL) Kms2_MdlPack_Free(mdl->packs,mdl->n_packs);
	if(mdl->vbuff!=NULL) Kms2_VBuff_Free(mdl->vbuff);
    }
    MDU_Free(def);
}


void Kms2_Mdl_Move(KMS2_MDL *r,KMS2_MDL *a,int n_models)
{
    int i;

    memcpy(r,a,sizeof(KMS2_MDL)*n_models);

    for(i=0;i<n_models;i++){
	(a+i)->packs=NULL;

	/* 2002/6/4  K.Kano
	   X-BOX用フォーマットのための初期化を追加 */
	(a+i)->vbuff=NULL;
    }
}

void Kms2_MdlPack_Move(KMS2_MDLPACK *r,KMS2_MDLPACK *a,int n_packs)
{
    int i;

    memcpy(r,a,sizeof(KMS2_MDLPACK)*n_packs);

    for(i=0;i<n_packs;i++){
	(a+i)->verts=NULL;
	(a+i)->norms=NULL;
	(a+i)->uvs[0]=NULL;
	(a+i)->uvs[1]=NULL;
	(a+i)->uvs[2]=NULL;

#if 0
	(a+i)->rgbs=NULL;
#endif
	(a+i)->index=NULL;
    }
}

/*-----------------------------------------------------------------*/

EVM_DEF *Evm_Alloc(int n_models,int n_x_models,long *size)
{
    EVM_DEF *def;
    long lsize=sizeof(EVM_DEF)+sizeof(EVM_SKEL)*n_x_models;

    if((def=(EVM_DEF *)MDU_Alloc(lsize))==NULL) return NULL;

    def->n_models=n_models;
    def->n_x_models=n_x_models;
    def->n_packs=0;
    def->packet=NULL;
    def->type=0;

    if(size!=NULL) *size=lsize;

    return def;
}

EVM_PACK *Evm_Pack_Alloc(int n_packs,long *size)
{
    EVM_PACK *pack;
    long lsize=sizeof(EVM_PACK)*n_packs;
    int i;

    if((pack=(EVM_PACK *)MDU_Alloc(lsize))==NULL) return NULL;

    for(i=0;i<n_packs;i++){
	(pack+i)->n_verts=0;
	(pack+i)->n_mats=0;
	(pack+i)->verts=NULL;
	(pack+i)->norms=NULL;
	(pack+i)->uvs[0]=NULL;
	(pack+i)->uvs[1]=NULL;
	(pack+i)->uvs[2]=NULL;
	(pack+i)->weight=NULL;
	(pack+i)->rgbs=NULL;

	(pack+i)->index=NULL;
	(pack+i)->n_indices=0;
    }

    if(size!=NULL) *size=lsize;

    return pack;
}

void *Evm_Verts_Alloc(int n_verts,long *size)
{
    long lsize=sizeof(short)*n_verts*4;
    if(size!=NULL) *size=lsize;
    return MDU_Alloc(lsize);
}

void *Evm_Norms_Alloc(int n_verts,long *size)
{
    return Evm_Verts_Alloc(n_verts,size);
}

/* 追加   NISINO */
void *Evm_Indicess_Alloc(int n_indices,long *size)
{
    long lsize=sizeof(short)*n_indices;
    if(size!=NULL) *size=lsize;
    return MDU_Alloc(lsize);
}


void *Evm_Uvs_Alloc(int n_verts,long *size)
{
    return Evm_Verts_Alloc(n_verts,size);
}

void *Evm_Weight_Alloc(int n_verts,long *size)
{
    long lsize=sizeof(char)*n_verts*8;
    if(size!=NULL) *size=lsize;
    return MDU_Alloc(lsize);
}


void Evm_Free(EVM_DEF *def)
{
    if(def->packet!=NULL) Evm_Pack_Free(def->packet,def->n_packs);
    MDU_Free(def);
}

void Evm_Index_Free( unsigned short* p )
{
	MDU_Free( p );	
}

void Evm_Pack_Free(EVM_PACK *packs,int n_packs)
{
    int i;

    for(i=0;i<n_packs;i++){
	EVM_PACK *pack=packs+i;
	if(pack->verts!=NULL) Evm_Verts_Free(pack->verts);
	if(pack->norms!=NULL) Evm_Norms_Free(pack->norms);
	if(pack->uvs[0]!=NULL) Evm_Uvs_Free(pack->uvs[0]);
	if(pack->uvs[1]!=NULL) Evm_Uvs_Free(pack->uvs[1]);
	if(pack->uvs[2]!=NULL) Evm_Uvs_Free(pack->uvs[2]);
	if(pack->weight!=NULL) Evm_Weight_Free(pack->weight);
	if(pack->index !=NULL) Evm_Index_Free( pack->index );
    }
    MDU_Free(packs);
}

void Evm_Verts_Free(void *verts)
{
    MDU_Free(verts);
}

void Evm_Norms_Free(void *norms)
{
    Evm_Verts_Free(norms);
}

void Evm_Uvs_Free(void *uvs)
{
    Evm_Verts_Free(uvs);
}

void Evm_Weight_Free(void *weight)
{
    MDU_Free(weight);
}

/*-----------------------------------------------------------------*/

KMX_VERTS *Kmx_Verts_Alloc(int n_verts)
{
    return (KMX_VERTS *)MDU_Alloc(sizeof(KMX_VERTS)*n_verts);
}

KMX_NORMS *Kmx_Norms_Alloc(int n_norms)
{
    return (KMX_NORMS *)MDU_Alloc(sizeof(KMX_NORMS)*n_norms);
}

KMX_PRIMS *Kmx_Prims_Alloc(int n_prims)
{
    KMX_PRIMS *prims;
    int i;

    if((prims=(KMX_PRIMS *)MDU_Alloc(sizeof(KMX_PRIMS)*n_prims))==NULL) return NULL;

    for(i=0;i<n_prims;i++){
	KMX_PRIMS *prim=prims+i;

	prim->n_id=0;
	prim->vid=NULL;
	prim->nid=NULL;

	prim->n_tid=0;
	prim->tid=NULL;

	prim->uvs=NULL;
    }

    return prims;
}

int Kmx_PrimData_Alloc(KMX_PRIMS *prim,int n_id,int n_tid)
{
    prim->n_id=n_id;
    prim->n_tid=n_tid;

    if((prim->vid=(u_int *)MDU_Alloc(sizeof(u_int)*n_id))==NULL) return 0;
    if((prim->nid=(u_int *)MDU_Alloc(sizeof(u_int)*n_id))==NULL){
	MDU_Free(prim->vid);
	prim->vid=NULL;
	return 0;
    }
    if(n_tid>0){
	if((prim->tid=(u_int *)MDU_Alloc(sizeof(u_int)*n_tid))==NULL){
	    MDU_Free(prim->vid);
	    MDU_Free(prim->nid);
	    prim->vid=NULL;
	    prim->nid=NULL;
	    return 0;
	}
	if((prim->uvs=(TVECTOR *)MDU_Alloc(sizeof(TVECTOR)*n_id*n_tid))==NULL){
	    MDU_Free(prim->vid);
	    MDU_Free(prim->nid);
	    MDU_Free(prim->tid);
	    prim->vid=NULL;
	    prim->nid=NULL;
	    prim->tid=NULL;
	    return 0;
	}
    }
    else{
	prim->tid=NULL;
	prim->uvs=NULL;
    }
    return 1;
}

KMX_XVERTS *Kmx_XVerts_Alloc(int n_verts)
{
    return (KMX_XVERTS *)MDU_Alloc(sizeof(KMX_XVERTS)*n_verts);
}

TEXINFO *Kmx_Texs_Alloc(int n_texs)
{
    TEXINFO *info=(TEXINFO *)MDU_Alloc(sizeof(TEXINFO)*n_texs);
    if(info==NULL) return NULL;
    memset(info,0x00,sizeof(TEXINFO)*n_texs);
    return info;
}

char *Kmx_Tex_Set(char *s)
{
    char *d;
    int len;

    len=strlen(s);
    if(len<=0) return NULL;
    if((d=(char *)MDU_Alloc(sizeof(char)*(len+1)))==NULL) return NULL;
    strcpy(d,s);

    return d;
}

int Kmx_MdlData_Alloc(KMX_MDL *mdl,int n_verts,int n_norms,int n_prims)
{
    if((mdl->verts=Kmx_Verts_Alloc(n_verts))==NULL){
	return 0;
    }
    mdl->n_verts=n_verts;

    if((mdl->norms=Kmx_Norms_Alloc(n_norms))==NULL){
	Kmx_Verts_Free(mdl->verts);
	return 0;
    }
    mdl->n_norms=n_norms;

    if((mdl->prims=Kmx_Prims_Alloc(n_prims))==NULL){
	Kmx_Verts_Free(mdl->verts);
	Kmx_Norms_Free(mdl->norms);
	return 0;
    }
    mdl->n_prims=n_prims;

    mdl->n_xverts=0;
    mdl->xverts=NULL;

    return 1;
}

KMX_DEF *Kmx_Alloc(int n_models,int n_x_models)
{
    KMX_DEF *def;
    KMX_MDL *mdl;
    int i;

    if((def=(KMX_DEF *)MDU_Alloc(sizeof(KMX_DEF)+sizeof(KMX_MDL)*n_x_models))==NULL) return NULL;

    def->n_models=n_models;
    def->n_x_models=n_x_models;

    def->n_texs=0;
    def->texs=NULL;

    mdl=def->models;
    for(i=0;i<n_x_models;i++,mdl++){
	mdl->type=0;
	mdl->parent=-1;
	mdl->extend=-1;

	mdl->n_verts=0;
	mdl->verts=NULL;
	mdl->n_norms=0;
	mdl->norms=NULL;
	mdl->n_prims=0;
	mdl->prims=NULL;
	mdl->n_xverts=0;
	mdl->xverts=NULL;
    }

    return def;
}

void Kmx_Verts_Free(KMX_VERTS *verts)
{
    MDU_Free(verts);
}

void Kmx_Norms_Free(KMX_NORMS *norms)
{
    MDU_Free(norms);
}

void Kmx_Prims_Free(KMX_PRIMS *prims,int n_prims)
{
    int i;

    for(i=0;i<n_prims;i++){
	KMX_PRIMS *prim=prims+i;

	if(prim->vid!=NULL) MDU_Free(prim->vid);
	if(prim->nid!=NULL) MDU_Free(prim->nid);
	if(prim->tid!=NULL) MDU_Free(prim->tid);
	if(prim->uvs!=NULL) MDU_Free(prim->uvs);
    }
    MDU_Free(prims);
}

void Kmx_XVerts_Free(KMX_XVERTS *xverts)
{
    MDU_Free(xverts);
}

void Kmx_MdlData_Free(KMX_MDL *mdl)
{
    if(mdl->verts!=NULL) Kmx_Verts_Free(mdl->verts);
    if(mdl->norms!=NULL) Kmx_Norms_Free(mdl->norms);
    if(mdl->prims!=NULL) Kmx_Prims_Free(mdl->prims,mdl->n_prims);
    if(mdl->xverts!=NULL) Kmx_XVerts_Free(mdl->xverts);
    mdl->n_verts=0;
    mdl->verts=NULL;
    mdl->n_norms=0;
    mdl->norms=NULL;
    mdl->n_prims=0;
    mdl->prims=NULL;
    mdl->n_xverts=0;
    mdl->xverts=NULL;
}

void Kmx_Free(KMX_DEF *def)
{
    int i;

    if(def->texs!=NULL){
	for(i=0;i<def->n_texs;i++){
	    if((def->texs+i)->name!=NULL) MDU_Free((def->texs+i)->name);
	}
	MDU_Free(def->texs);
    }
    for(i=0;i<def->n_x_models;i++){
	KMX_MDL *mdl=def->models+i;
	Kmx_MdlData_Free(mdl);
    }
    MDU_Free(def);
}


void Kmx_MdlData_Move(KMX_MDL *r,KMX_MDL *a,int n_models)
{
    int i;

    memcpy(r,a,sizeof(KMX_MDL)*n_models);

    for(i=0;i<n_models;i++){
	(a+i)->n_verts=0;
	(a+i)->verts=NULL;
	(a+i)->n_norms=0;
	(a+i)->norms=NULL;
	(a+i)->n_prims=0;
	(a+i)->prims=NULL;
    }
}

void Kmx_Prims_Move(KMX_PRIMS *r,KMX_PRIMS *a,int n_prims)
{
    int i;

    memcpy(r,a,sizeof(KMX_PRIMS)*n_prims);

    for(i=0;i<n_prims;i++){
	(a+i)->n_id=0;
	(a+i)->vid=NULL;
	(a+i)->nid=NULL;
	(a+i)->n_tid=0;
	(a+i)->tid=NULL;
	(a+i)->uvs=NULL;
    }
}

/*-----------------------------------------------------------------*/

EVF_VERTS *Evf_Verts_Alloc(int n_verts)
{
    EVF_VERTS *verts;
    int i;

    if((verts=(EVF_VERTS *)MDU_Alloc(sizeof(EVF_VERTS)*n_verts))==NULL) return NULL;

    for(i=0;i<n_verts;i++){
	(verts+i)->n_envs=0;
	(verts+i)->envs=NULL;
    }

    return verts;
}

EVF_ENV *Evf_Envs_Alloc(int n_envs)
{
    EVF_ENV *envs;

    if((envs=(EVF_ENV *)MDU_Alloc(sizeof(EVF_ENV)*n_envs))==NULL){
	return NULL;
    }
    return envs;
}

int Evf_MeshData_Alloc(EVF_MESH *mdl,int n_verts,int n_norms,int n_prims)
{
    if((mdl->verts=Evf_Verts_Alloc(n_verts))==NULL){
	return 0;
    }
    mdl->n_verts=n_verts;

    if((mdl->norms=Evf_Norms_Alloc(n_norms))==NULL){
	Evf_Verts_Free(mdl->verts,mdl->n_verts);
	return 0;
    }
    mdl->n_norms=n_norms;

    if((mdl->prims=Evf_Prims_Alloc(n_prims))==NULL){
	Evf_Verts_Free(mdl->verts,n_verts);
	Evf_Norms_Free(mdl->norms);
	return 0;
    }
    mdl->n_prims=n_prims;

    return 1;
}

EVF_DEF *Evf_Alloc(int n_models,int n_x_models)
{
    EVF_DEF *def;

    if((def=(EVF_DEF *)MDU_Alloc(sizeof(EVF_DEF)+sizeof(EVF_SKEL)*n_x_models))==NULL){
	return NULL;
    }

    def->n_models=n_models;
    def->n_x_models=n_x_models;

    def->n_texs=0;
    def->texs=NULL;
    def->type=0;

    def->mesh.n_verts=0;
    def->mesh.verts=NULL;
    def->mesh.n_norms=0;
    def->mesh.norms=NULL;
    def->mesh.n_prims=0;
    def->mesh.prims=NULL;

    return def;
}


void Evf_Verts_Free(EVF_VERTS *verts,int n_verts)
{
    int i;

    for(i=0;i<n_verts;i++){
	if((verts+i)->envs!=NULL) MDU_Free((verts+i)->envs);
    }
    MDU_Free(verts);
}

void Evf_MeshData_Free(EVF_MESH *mdl)
{
    if(mdl->verts!=NULL) Evf_Verts_Free(mdl->verts,mdl->n_verts);
    if(mdl->norms!=NULL) Evf_Norms_Free(mdl->norms);
    if(mdl->prims!=NULL) Evf_Prims_Free(mdl->prims,mdl->n_prims);

    mdl->n_verts=0;
    mdl->verts=NULL;
    mdl->n_norms=0;
    mdl->norms=NULL;
    mdl->n_prims=0;
    mdl->prims=NULL;
}

void Evf_Free(EVF_DEF *def)
{
    int i;

    if(def->texs!=NULL){
	for(i=0;i<def->n_texs;i++){
	    if((def->texs+i)->name!=NULL) MDU_Free((def->texs+i)->name);
	}
	MDU_Free(def->texs);
    }

    if(def->mesh.verts!=NULL){
	for(i=0;i<def->mesh.n_verts;i++){
	    EVF_VERTS *vert=def->mesh.verts+i;
	    if(vert->envs!=NULL) MDU_Free(vert->envs);
	}
	MDU_Free(def->mesh.verts);
    }

    if(def->mesh.norms!=NULL) MDU_Free(def->mesh.norms);

    if(def->mesh.prims!=NULL){
	for(i=0;i<def->mesh.n_prims;i++){
	    EVF_PRIMS *prim=def->mesh.prims+i;
	    if(prim->vid!=NULL) MDU_Free(prim->vid);
	    if(prim->nid!=NULL) MDU_Free(prim->nid);
	    if(prim->tid!=NULL) MDU_Free(prim->tid);
	    if(prim->uvs!=NULL) MDU_Free(prim->uvs);
	}
	MDU_Free(def->mesh.prims);
    }

    MDU_Free(def);
}

void Evf_MeshData_Move(EVF_MESH *r,EVF_MESH *a)
{
    memcpy(r,a,sizeof(EVF_MESH));

    a->n_verts=0;
    a->verts=NULL;
    a->n_norms=0;
    a->norms=NULL;
    a->n_prims=0;
    a->prims=NULL;
}

/*-----------------------------------------------------------------*/
