/*
   DivPrims.c

   共有頂点形式の中間フォーマットを、ストリップに直すためのサブルーチン

   by K.Kano , 3/1/2000

   $Id: DivPrims.c,v 1.11 2002/06/04 13:45:37 usr01363 Exp $
   
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
#include <SFDLINUX.h>

#include "METALGEAR.h"
#include "fmt_km4.h"

#include "MDU_util.h"
#include "MDU_mdl.h"


#include "ToStrip.h"


/* -------------------------------------------------------------------------- */

#if 0

static void DumpDivPrims(DIV_PRIMS *div_prims)
{
    DIV_PRIM *div=div_prims->div_prims;
    int i,j;

    printf("\n");
    for(i=0;i<div_prims->n_div_prims;i++,div++){
	PRIM *p=div->prims.next;

	for(j=0;j<div->n_prims;j++,p=p->next){
	    if(div->n_tid!=p->n_tid){
		printf("DumpDivPrims : n_tid is not equal.\n");

		printf("Check Next :\n");
		printf("i,j     : %d,%d\n",i,j);
		printf("N Prims : %d\n",div->n_prims);
		printf("N Tid   : %d %d\n",div->n_tid,p->n_tid);
		printf("P       : 0x%08x 0x%08x 0x%08x\n",(int)p,(int)(p->prea),(int)(p->next));
	    }
	    else{
		int k;
		for(k=0;k<div->n_tid;k++){
		    if(div->tid[k]!=p->tid[k]){
			printf("DumpDivPrims : tid is not equal.\n");
			break;
		    }
		}
	    }
	}

	p=div->prims.prea;

	for(j=0;j<div->n_prims;j++,p=p->prea){
	    if(div->n_tid!=p->n_tid){
		printf("DumpDivPrims : n_tid is not equal.\n");

		printf("Check Prea :\n");
		printf("i,j     : %d,%d\n",i,j);
		printf("N Prims : %d\n",div->n_prims);
		printf("N Tid   : %d %d\n",div->n_tid,p->n_tid);
	    }
	    else{
		int k;
		for(k=0;k<div->n_tid;k++){
		    if(div->tid[k]!=p->tid[k]){
			printf("DumpDivPrims : tid is not equal.\n");
			break;
		    }
		}
	    }
	}
    }
}

#endif

/* -------------------------------------------------------------------------- */

/* エンベロープの有無による分割 */
static DIV_PRIMS *Kmx_Divide_By_ENV(DIV_PRIMS *prims,KMX_MDL *mdl)
{
    DIV_PRIMS *prims2;
    KMX_VERTS *verts;
    DIV_PRIM *d,*e;
    int i,j;
    int n_div_prims;
    int new_n_div_prims;

    verts=mdl->verts;

    n_div_prims=new_n_div_prims=prims->n_div_prims;

    /* エンベロープ演算を含むプリミティブがあるかどうかチェック */
    for(i=0;i<n_div_prims;i++){
	PRIM *p=prims->div_prims[i].prims.next;
	int n_prims=prims->div_prims[i].n_prims;


	for(j=0;j<n_prims;j++){
	    PRIM *np=p->next;
	    int k;

#if 0
	    printf("Tid =");
	    if(prims->div_prims[i].tid!=NULL){
		printf(" %d",*(prims->div_prims[i].tid+0));
	    }
	    else{
		printf(" None");
	    }
	    if(p->tid!=NULL){
		printf(" %d\n",*(p->tid+0));
	    }
	    else{
		printf(" None\n");
	    }
#endif

	    for(k=0;k<3;k++){
		int vid=p->p[k].vid;
		if((verts+vid)->env.parentid!=255){
		    new_n_div_prims++;
		    goto forbreak;
		}
	    }

	    p=np;
	}

forbreak:
	;

    }

    if(n_div_prims==new_n_div_prims) return prims;

    if((prims2=Div_Prims_Alloc(new_n_div_prims))==NULL){
	Div_Prims_Free(prims);
	return NULL;
    }

    d=prims2->div_prims+0;
    e=prims2->div_prims+n_div_prims;

    Div_Prims_Move(prims2,prims);
    Div_Prims_Free(prims);

    for(i=0;i<n_div_prims;i++,d++){
	PRIM *p=d->prims.next;
	int n_prims=d->n_prims;
	int eflag=0;

	d->have_env=0;

	for(j=0;j<n_prims;j++){
	    PRIM *np=p->next;
	    int k;

	    for(k=0;k<3;k++){
		int vid=p->p[k].vid;
		if((verts+vid)->env.parentid!=255) break;
	    }
	    if(k!=3){
		if(!eflag){
		    if(!Div_Prim_Alloc(e,d->n_tid)){
			Div_Prims_Free(prims2);
			return NULL;
		    }
		    for(k=0;k<d->n_tid;k++) *(e->tid+k)=*(d->tid+k);

		    e->have_env=1;
		    eflag=1;
		}

		p->next->prea=p->prea;
		p->prea->next=p->next;

		p->next=e->prims.prea->next;
		p->prea=e->prims.prea;
		e->prims.prea->next=p;
		e->prims.prea=p;

		e->n_prims++;
		d->n_prims--;
	    }

	    p=np;
	}

	if(eflag) e++;
    }

    return prims2;
}

/* テクスチャの種類によって分割 */
static DIV_PRIMS *Kmx_Divide_By_TEX(DIV_PRIMS *prims)
{
    DIV_PRIMS *prims2;
    PRIM *p;
    int n_div_prims;
    int n_prims;
    int i,j;

    n_prims=prims->div_prims->n_prims;
    if(n_prims==0) return prims;

    /* テクスチャのマッチングを調べる */
    p=prims->div_prims->prims.next;
    n_div_prims=0;

    // printf("Check 22222.11111\n");

    // printf("N Prims = %d\n",n_prims);

    for(i=0;i<n_prims;i++){
	PRIM *q=prims->div_prims->prims.next;
	PRIM *np=p->next;
	int flag=1;

#if 0
	/* テクスチャが貼ってない場合 */
	if(p->n_tid==0){
	    printf("Remove Primitive : Primitive has no texture.\n");
	    continue;
	}
#endif

	for(j=0;j<i;j++){
	    PRIM *nq=q->next;

	    if(p->n_tid==q->n_tid){
		int k;
		for(k=0;k<p->n_tid;k++){
		    if(*(p->tid+k)!=*(q->tid+k)) break;
		}
		if(k==p->n_tid) flag=0;
	    }
	    q=nq;
	}
	if(flag) n_div_prims++;
	p=np;
    }

    // printf("Check 22222.22222\n");

    if((prims2=Div_Prims_Alloc(n_div_prims))==NULL){
	Div_Prims_Free(prims);
	return NULL;
    }

    // printf("Check 22222.33333\n");

    p=prims->div_prims->prims.next;
    n_div_prims=0;

    for(i=0;i<n_prims;i++){
	PRIM *np=p->next;
	int tflag=0;

#if 0
	/* テクスチャが貼ってない場合 */
	if(p->n_tid==0){
	    p->next->prea=p->prea;
	    p->prea->next=p->next;
	    Prim_Free(p);
	    p=np;

	    continue;
	}
#endif

	/* 現在登録されているテクスチャパターンのチェック */
	for(j=0;j<n_div_prims;j++){
	    if(prims2->div_prims[j].n_tid==p->n_tid){
		int k;
		for(k=0;k<p->n_tid;k++){
		    if(prims2->div_prims[j].tid[k]!=p->tid[k]) break;
		}
		if(k==p->n_tid){
		    p->next->prea=p->prea;
		    p->prea->next=p->next;

		    p->next=prims2->div_prims[j].prims.prea->next;
		    p->prea=prims2->div_prims[j].prims.prea;
		    prims2->div_prims[j].prims.prea->next=p;
		    prims2->div_prims[j].prims.prea=p;

		    prims2->div_prims[j].n_prims++;
		    prims->div_prims->n_prims--;

		    tflag=1;
		    break;
		}
	    }
	}
	if(!tflag){
	    int k;

	    if(!Div_Prim_Alloc(&(prims2->div_prims[n_div_prims]),p->n_tid)){
		Div_Prims_Free(prims);
		Div_Prims_Free(prims2);
	    }
	    for(k=0;k<p->n_tid;k++){
		prims2->div_prims[n_div_prims].tid[k]=p->tid[k];
	    }

	    p->next->prea=p->prea;
	    p->prea->next=p->next;

	    p->next=prims2->div_prims[n_div_prims].prims.prea->next;
	    p->prea=prims2->div_prims[n_div_prims].prims.prea;
	    prims2->div_prims[n_div_prims].prims.prea->next=p;
	    prims2->div_prims[n_div_prims].prims.prea=p;

	    prims2->div_prims[n_div_prims].n_prims++;
	    prims->div_prims->n_prims--;

	    n_div_prims++;
	}

	p=np;
    }

    // printf("Check 22222.44444\n");

    Div_Prims_Free(prims);

    // printf("Check 22222.55555\n");

    return prims2;
}

/* プリミティブを作成する */
static DIV_PRIMS *Kmx_Make_PRIM(KMX_MDL *mdl)
{
    DIV_PRIMS *prims;
    DIV_PRIM *div_prim;
    KMX_PRIMS *mdl_prim;
    int i,j;

    if((prims=Div_Prims_Alloc(1))==NULL) return NULL;

    div_prim=prims->div_prims;
    mdl_prim=mdl->prims;

    Div_Prim_Alloc(div_prim,0);

    for(i=0;i<mdl->n_prims;i++,mdl_prim++){
	PRIM *p;

	if((p=Prim_Alloc(mdl_prim->n_tid))==NULL){
	    Div_Prims_Free(prims);
	    return NULL;
	}

	for(j=0;j<mdl_prim->n_tid;j++) *(p->tid+j)=*(mdl_prim->tid+j);

	for(j=0;j<3;j++){
	    int k;

	    p->p[j].vid=*(mdl_prim->vid+j);
	    p->p[j].nid=*(mdl_prim->nid+j);

	    for(k=0;k<mdl_prim->n_tid;k++){
		memcpy(p->p[j].uvs+k,mdl_prim->uvs+k*mdl_prim->n_id+j,sizeof(TVECTOR));
	    }

	    p->p[j].prim=mdl_prim;
	    p->p[j].v_order=j;
	}

	p->next=div_prim->prims.prea->next;
	p->prea=div_prim->prims.prea;
	div_prim->prims.prea->next=p;
	div_prim->prims.prea=p;

	div_prim->n_prims++;
    }

    return prims;
}

/* テクスチャを条件によって分割する*/
DIV_PRIMS *Kmx2DivPrims(KMX_MDL *mdl)
{
    DIV_PRIMS *div_prims;


    printf("MakePrim");
    fflush(stdout);

    /* ストリップ作成用プリミティブを作成する */
    if((div_prims=Kmx_Make_PRIM(mdl))==NULL) return NULL;


    printf(", DivideTEX");
    fflush(stdout);

    /* テクスチャの種類によって分割 */
    if((div_prims=Kmx_Divide_By_TEX(div_prims))==NULL) return NULL;

    printf(", DivideWeight  ");
    fflush(stdout);

    /* エンベロープによる分割 */
    if((div_prims=Kmx_Divide_By_ENV(div_prims,mdl))==NULL) return NULL;

    return div_prims;
}

/* -------------------------------------------------------------------------- */

/* ウェイトによる分割 */
static DIV_PRIMS *Evf_Divide_By_Weight(DIV_PRIMS *prims,EVF_MESH *mesh)
{
    DIV_PRIMS *prims2;
    EVF_VERTS *verts;
    DIV_PRIM *d,*e;
    int i,j;
    int n_div_prims;
    int new_n_div_prims;

    verts=mesh->verts;

    n_div_prims=new_n_div_prims
	=prims->start_single=prims->n_div_prims;

    /* シングルウェイトを含むかどうかチェック */
    for(i=0;i<n_div_prims;i++){
	PRIM *p=prims->div_prims[i].prims.next;
	int n_prims=prims->div_prims[i].n_prims;

	for(j=0;j<n_prims;j++){
	    PRIM *np=p->next;
	    int vid0=p->p[0].vid;
	    int vid1=p->p[1].vid;
	    int vid2=p->p[2].vid;

	    if((verts+vid0)->n_envs==1 &&
	       (verts+vid1)->n_envs==1 &&
	       (verts+vid2)->n_envs==1){

#if 0  /* TYPE_VER1 */
		if((verts+vid0)->envs->objid==(verts+vid1)->envs->objid &&
		   (verts+vid0)->envs->objid==(verts+vid2)->envs->objid){

		    new_n_div_prims++;
		    goto forbreak;
		}
#endif

#if 1  /* TYPE_VER2 */
		new_n_div_prims++;
		goto forbreak;
#endif

	    }

	    p=np;
	}

forbreak:
	;

    }

    if(n_div_prims==new_n_div_prims){
	prims->start_single=n_div_prims;
	return prims;
    }

    if((prims2=Div_Prims_Alloc(new_n_div_prims))==NULL){
	Div_Prims_Free(prims);
	return NULL;
    }

    prims2->start_single=n_div_prims;

    d=prims2->div_prims+0;
    e=prims2->div_prims+n_div_prims;

    Div_Prims_Move(prims2,prims);
    Div_Prims_Free(prims);

    for(i=0;i<n_div_prims;i++,d++){
	PRIM *p=d->prims.next;
	int n_prims=d->n_prims;
	int eflag=0;

	d->have_env=-1;

	// printf("( %d ) n_prim 0  = %d\n",i,d->n_prims);

	for(j=0;j<n_prims;j++){
	    PRIM *np=p->next;
	    int k;
	    int vid0=p->p[0].vid;
	    int vid1=p->p[1].vid;
	    int vid2=p->p[2].vid;

	    if((verts+vid0)->n_envs==1 &&
	       (verts+vid1)->n_envs==1 &&
	       (verts+vid2)->n_envs==1){

#if 0  /* TYPE_VER1 */
		if((verts+vid0)->envs->objid==(verts+vid1)->envs->objid &&
		   (verts+vid0)->envs->objid==(verts+vid2)->envs->objid){

		    if(!eflag){
			if(!Div_Prim_Alloc(e,d->n_tid)){
			    Div_Prims_Free(prims2);
			    return NULL;
			}
			for(k=0;k<d->n_tid;k++){
			    *(e->tid+k)=*(d->tid+k);
			}

			e->have_env=i;
			eflag=1;
		    }

		    p->next->prea=p->prea;
		    p->prea->next=p->next;

		    p->next=e->prims.prea->next;
		    p->prea=e->prims.prea;
		    e->prims.prea->next=p;
		    e->prims.prea=p;

		    e->n_prims++;
		    d->n_prims--;
		}
#endif

#if 1  /* TYPE_VER2 */
		if(!eflag){
		    if(!Div_Prim_Alloc(e,d->n_tid)){
			Div_Prims_Free(prims2);
			return NULL;
		    }
		    for(k=0;k<d->n_tid;k++){
			*(e->tid+k)=*(d->tid+k);
		    }

		    e->have_env=i;
		    eflag=1;
		}

		p->next->prea=p->prea;
		p->prea->next=p->next;

		p->next=e->prims.prea->next;
		p->prea=e->prims.prea;
		e->prims.prea->next=p;
		e->prims.prea=p;

		e->n_prims++;
		d->n_prims--;
#endif

	    }

	    p=np;
	}

#if 0
	if(d->n_prims==0){
	    printf("D : 0x%08x 0x%08x 0x%08x\n",
		   (int)&(d->prims),(int)(d->prims.prea),(int)(d->prims.next));
	}
#endif

	// printf("( %d ) n_prim 1 = %d\n",i,d->n_prims);

	if(eflag) e++;
    }

    // printf("New N Div Prims = %d\n",prims2->n_div_prims);

    return prims2;
}

/* シングルウェイトを、影響するスケルトンでさらに分割 */
static DIV_PRIMS *Evf_Divide_SingleWeight(DIV_PRIMS *prims,EVF_MESH *mesh,int n_x_models)
{
    EVF_VERTS *verts;
    int n_div_prims;
    int i,j,k;

    // printf("n_div_prims , n_x_models = %d , %d\n",prims->n_div_prims,n_x_models);

    {
	DIV_PRIMS *tmp;

	// printf("Check 1\n");

	if((tmp=Div_Prims_Alloc(prims->n_div_prims*n_x_models))==NULL){
	    Div_Prims_Free(prims);
	    return NULL;
	}

	// printf("Check 2\n");

	Div_Prims_Move(tmp,prims);
	tmp->n_div_prims=prims->n_div_prims;

	// printf("Check 3\n");

	Div_Prims_Free(prims);
	prims=tmp;

	// printf("Check 4\n");
    }


    n_div_prims=prims->n_div_prims-prims->start_single;

    verts=mesh->verts;

    /* 分割！！ */
    for(i=0;i<n_div_prims;i++){
	DIV_PRIM *single=prims->div_prims+prims->start_single+i;
	PRIM *p=single->prims.next;
	int n_prims=single->n_prims;
	int objid0;

	DIV_PRIM *ext=prims->div_prims+prims->n_div_prims;
	int ext_cnt=0;

	// printf("%d\n",i);

	if(n_prims==0) continue;

	objid0=(verts+p->p[0].vid)->envs->objid;

	for(j=0;j<n_prims;j++){
	    PRIM *np=p->next;
	    int objid=(verts+p->p[0].vid)->envs->objid;

	    // printf("\t%d\n",j);

#if 0
	    if(p->p[0].vid==0x80000000){
		printf("Wrong Primitive\n");
	    }
#endif

	    if(objid0!=objid){
		DIV_PRIM *adddiv=ext;

		// printf("\t\tcheck\n");

		for(k=0;k<ext_cnt;k++,adddiv++){
		    PRIM *eprim=adddiv->prims.next;
		    int vid2=eprim->p[0].vid;
		    int objid2=(verts+vid2)->envs->objid;

		    if(objid==objid2) break;
		}

		// printf("Check 1\n");
		if(k==ext_cnt){
		    if(!Div_Prim_Alloc(adddiv,single->n_tid)){
			Div_Prims_Free(prims);
			return NULL;
		    }

		    // printf("Check 2\n");

		    for(k=0;k<single->n_tid;k++){
			adddiv->tid[k]=single->tid[k];
		    }
		    adddiv->have_env=single->have_env;

		    prims->n_div_prims++;
		    ext_cnt++;
		}

		p->next->prea=p->prea;
		p->prea->next=p->next;

		p->next=adddiv->prims.prea->next;
		p->prea=adddiv->prims.prea;
		adddiv->prims.prea->next=p;
		adddiv->prims.prea=p;

		adddiv->n_prims++;
		single->n_prims--;
	    }

	    p=np;
	}
    }

    n_div_prims=prims->n_div_prims-prims->start_single;

    // printf("N DivPrims = %d\n",n_div_prims);

    /* 分割はしたが、少数ポリゴンしかなかった場合は、マルチウェイトの方へ戻す。*/
    for(i=0;i<n_div_prims;i++){
	DIV_PRIM *single=prims->div_prims+prims->start_single+i;

	// printf("Single N Prims = %d\n",single->n_prims);

	if(single->n_prims<3){
	    int multi_index=single->have_env;
	    DIV_PRIM *multi=prims->div_prims+multi_index;
	    PRIM *p=single->prims.next;

	    // printf("Multi Index = %d\n",multi_index);

#if 0
	    if(p->p[0].vid==0x80000000){
		printf("Wrong Primitive\n");
	    }
#endif

#if 0
	    if(multi->n_tid!=single->n_tid){
		printf("check : N tid\n");
	    }
	    else{
		for(j=0;j<multi->n_tid;j++){
		    if(multi->tid[j]!=single->tid[j]){
			printf("check : Tid\n");
		    }
		}
	    }
#endif


#if 0
	    printf("multi index   = %d\n",multi_index);
	    printf("multi n_prims = %d\n",multi->n_prims);
#endif

	    for(j=0;j<single->n_prims;j++){
		PRIM *np=p->next;

#if 0
		if(p->p[0].vid==0x80000000){
		    printf("Wrong Primitive : %d %d\n",i,j);
		}
#endif

		// printf("0x%08x 0x%08x 0x%08x\n",(int)p,(int)(p->prea),(int)(p->next));

		p->next->prea=p->prea;
		p->prea->next=p->next;

		p->next=multi->prims.prea->next;
		p->prea=multi->prims.prea;
		multi->prims.prea->next=p;
		multi->prims.prea=p;

		multi->n_prims++;

		// printf("0x%08x 0x%08x 0x%08x\n",(int)p,(int)(p->prea),(int)(p->next));

		p=np;
	    }

	    single->n_prims=0;

#if 0
	    single->prims.prea=&(single->prims);
	    single->prims.next=&(single->prims);
#endif

	}
    }

    return prims;
}
    
/* テクスチャの種類によって分割 */
static DIV_PRIMS *Evf_Divide_By_TEX(DIV_PRIMS *prims)
{
    DIV_PRIMS *prims2;
    PRIM *p;
    int n_div_prims;
    int n_prims;
    int i,j;

    if(prims->div_prims->n_prims==0) return prims;
    n_prims=prims->div_prims->n_prims;

    /* テクスチャのマッチングを調べる */
    p=prims->div_prims->prims.next;
    n_div_prims=0;

    for(i=0;i<n_prims;i++){
	PRIM *q=prims->div_prims->prims.next;
	PRIM *np=p->next;
	int flag=1;

#if 0
	/* テクスチャが貼ってない場合 */
	if(p->n_tid==0){
	    printf("Remove Primitive : Primitive has no texture.\n");
	    continue;
	}
#endif

	for(j=0;j<i;j++){
	    PRIM *nq=q->next;

	    if(p->n_tid==q->n_tid){
		int k;
		for(k=0;k<p->n_tid;k++){
		    if(*(p->tid+k)!=*(q->tid+k)) break;
		}
		if(k==p->n_tid) flag=0;
	    }
	    q=nq;
	}
	if(flag) n_div_prims++;

	p=np;
    }

    if((prims2=Div_Prims_Alloc(n_div_prims))==NULL){
	Div_Prims_Free(prims);
	return NULL;
    }

    p=prims->div_prims->prims.next;
    n_div_prims=0;

    for(i=0;i<n_prims;i++){
	PRIM *np=p->next;
	int tflag=0;

#if 0
	/* テクスチャが貼ってない場合 */
	if(p->n_tid==0){
	    p->next->prea=p->prea;
	    p->prea->next=p->next;
	    Prim_Free(p);
	    p=np;

	    continue;
	}
#endif

	/* 現在登録されているテクスチャパターンのチェック */
	for(j=0;j<n_div_prims;j++){
	    if(prims2->div_prims[j].n_tid==p->n_tid){
		int k;
		for(k=0;k<p->n_tid;k++){
		    if(prims2->div_prims[j].tid[k]!=p->tid[k]) break;
		}
		if(k==p->n_tid){
		    p->next->prea=p->prea;
		    p->prea->next=p->next;

		    p->next=prims2->div_prims[j].prims.prea->next;
		    p->prea=prims2->div_prims[j].prims.prea;
		    prims2->div_prims[j].prims.prea->next=p;
		    prims2->div_prims[j].prims.prea=p;

		    prims2->div_prims[j].n_prims++;
		    prims->div_prims->n_prims--;

		    tflag=1;
		    break;
		}
	    }
	}
	if(!tflag){
	    int k;

	    if(!Div_Prim_Alloc(&(prims2->div_prims[n_div_prims]),p->n_tid)){
		Div_Prims_Free(prims);
		Div_Prims_Free(prims2);
		return NULL;
	    }
	    for(k=0;k<p->n_tid;k++){
		prims2->div_prims[n_div_prims].tid[k]=p->tid[k];
	    }

	    p->next->prea=p->prea;
	    p->prea->next=p->next;

	    p->next=prims2->div_prims[n_div_prims].prims.prea->next;
	    p->prea=prims2->div_prims[n_div_prims].prims.prea;
	    prims2->div_prims[n_div_prims].prims.prea->next=p;
	    prims2->div_prims[n_div_prims].prims.prea=p;

	    prims2->div_prims[n_div_prims].n_prims++;
	    prims->div_prims->n_prims--;

	    n_div_prims++;
	}

	p=np;
    }

    Div_Prims_Free(prims);

    return prims2;
}

/* プリミティブを作成する */
static DIV_PRIMS *Evf_Make_PRIM(EVF_MESH *mesh)
{
    DIV_PRIMS *prims;
    DIV_PRIM *div_prim;
    EVF_PRIMS *mdl_prim;
    int i,j;

    if((prims=Div_Prims_Alloc(1))==NULL) return NULL;

    div_prim=prims->div_prims;
    mdl_prim=mesh->prims;

    Div_Prim_Alloc(div_prim,0);

    for(i=0;i<mesh->n_prims;i++,mdl_prim++){
	PRIM *p;

	if((p=Prim_Alloc(mdl_prim->n_tid))==NULL){
	    Div_Prims_Free(prims);
	    return NULL;
	}

	for(j=0;j<mdl_prim->n_tid;j++) *(p->tid+j)=*(mdl_prim->tid+j);

	for(j=0;j<3;j++){
	    int k;

	    p->p[j].vid=*(mdl_prim->vid+j);
	    p->p[j].nid=*(mdl_prim->nid+j);

	    for(k=0;k<mdl_prim->n_tid;k++){
		memcpy(p->p[j].uvs+k,mdl_prim->uvs+k*mdl_prim->n_id+j,sizeof(TVECTOR));
	    }

	    p->p[j].prim=mdl_prim;
	    p->p[j].v_order=j;
	}

	p->next=div_prim->prims.prea->next;
	p->prea=div_prim->prims.prea;
	div_prim->prims.prea->next=p;
	div_prim->prims.prea=p;

	div_prim->n_prims++;
    }

    return prims;
}

/* テクスチャを条件によって分割する*/
DIV_PRIMS *Evf2DivPrims(EVF_MESH *mesh,int n_x_models)
{
    DIV_PRIMS *div_prims;

    printf("Prestep to make strips : MakePrim");
    fflush(stdout);

    /* ストリップ作成用プリミティブを作成する */
    if((div_prims=Evf_Make_PRIM(mesh))==NULL) return NULL;

    printf(", DivideTEX");
    fflush(stdout);

    /* テクスチャの種類によって分割 */
    if((div_prims=Evf_Divide_By_TEX(div_prims))==NULL) return NULL;

    // DumpDivPrims(div_prims);

    printf(", DivideWeight");
    fflush(stdout);

    /* ウェイトによる分割 */
    if((div_prims=Evf_Divide_By_Weight(div_prims,mesh))==NULL) return NULL;

    // DumpDivPrims(div_prims);

#if 0  /* TYPE_VER1 */
    printf(", DivideSingleWeight");
    fflush(stdout);

    /* シングルウェイトの部分をさらに分割 */
    if((div_prims=Evf_Divide_SingleWeight(div_prims,mesh,n_x_models))==NULL) return NULL;
#endif

    // DumpDivPrims(div_prims);

    printf(", Finished.\n");

    return div_prims;
}

/* -------------------------------------------------------------------------- */
