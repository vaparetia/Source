/*
   ToStrip.c

   共有頂点形式の中間フォーマットを、ストリップに直すためのサブルーチン

   by K.Kano , 3/1/2000

   $Id: ToStrip.c,v 1.19 2002/08/26 11:22:09 usr01363 Exp $
   
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
#include "MDU_tex.h"


#include "ToStrip.h"


#define RIGHT 1
#define LEFT -1

/* -------------------------------------------------------------------------- */

typedef struct _TMP_STRIP {
    PRIM *prim;
    int vert_num;
    int dir;
} TMP_STRIP;

/* -------------------------------------------------------------------------- */

/* メモリ管理関数 */
void Prim_Free(PRIM *p)
{
    if(p->tid!=NULL) MDU_Free(p->tid);
    if(p->p[0].uvs!=NULL) MDU_Free(p->p[0].uvs);
    if(p->p[1].uvs!=NULL) MDU_Free(p->p[1].uvs);
    if(p->p[2].uvs!=NULL) MDU_Free(p->p[2].uvs);

    p->tid=NULL;
    p->prea=NULL;
    p->next=NULL;
    p->p[0].uvs=NULL;
    p->p[1].uvs=NULL;
    p->p[2].uvs=NULL;

    MDU_Free(p);
}

PRIM *Prim_Alloc(int n_tid)
{
    PRIM *prim;

    if((prim=MDU_Alloc(sizeof(PRIM)))==NULL) return NULL;

    prim->n_tid=n_tid;
    prim->tid=NULL;
    prim->p[0].uvs=NULL;
    prim->p[1].uvs=NULL;
    prim->p[2].uvs=NULL;

    prim->prea=prim->next=NULL;

    if(prim->n_tid>0){
	if((prim->tid=(u_int *)MDU_Alloc(sizeof(u_int)*n_tid))==NULL) Prim_Free(prim);
	if((prim->p[0].uvs=(TVECTOR *)MDU_Alloc(sizeof(TVECTOR)*n_tid))==NULL){
	    Prim_Free(prim);
	}
	if((prim->p[1].uvs=(TVECTOR *)MDU_Alloc(sizeof(TVECTOR)*n_tid))==NULL){
	    Prim_Free(prim);
	}
	if((prim->p[2].uvs=(TVECTOR *)MDU_Alloc(sizeof(TVECTOR)*n_tid))==NULL){
	    Prim_Free(prim);
	}
    }

    return prim;
}

void Div_Prim_Free(DIV_PRIM *p)
{
    PRIM *q=p->prims.next;
    int j;

    for(j=0;j<p->n_prims;j++){
	PRIM *nq=q->next;

#if 0
	if(q->n_tid==0x80000000){
	    printf("Wrong Primitive\n");
	    *(long *)1=0;
	}
#endif

	Prim_Free(q);
	q=nq;
    }

    if(p->tid!=NULL) MDU_Free(p->tid);

    p->n_prims=0;
    p->n_tid=0;
    p->tid=NULL;
    p->prims.prea=&(p->prims);
    p->prims.next=&(p->prims);
}

int Div_Prim_Alloc(DIV_PRIM *p,int n_tid)
{
    if(n_tid>0){
	if((p->tid=(u_int *)MDU_Alloc(sizeof(u_int)*n_tid))==NULL) return 0;
    }

    p->n_tid=n_tid;
    p->n_prims=0;
    p->have_env=0;
    p->prims.prea=&(p->prims);
    p->prims.next=&(p->prims);
    p->prims.n_tid=0x80000000;
    p->prims.p[0].vid=0x80000000;
    p->prims.p[0].nid=0x80000000;
    p->prims.p[0].uvs=NULL;
    p->prims.p[1].vid=0x80000000;
    p->prims.p[1].nid=0x80000000;
    p->prims.p[1].uvs=NULL;
    p->prims.p[2].vid=0x80000000;
    p->prims.p[2].nid=0x80000000;
    p->prims.p[2].uvs=NULL;
    p->prims.flag=0x80000000;

    return 1;
}

void Div_Prims_Free(DIV_PRIMS *div_prims)
{
    DIV_PRIM *p;
    int i;

    p=div_prims->div_prims;
    for(i=0;i<div_prims->n_div_prims;i++,p++) Div_Prim_Free(p);
    MDU_Free(div_prims);
}

DIV_PRIMS *Div_Prims_Alloc(int n_div_prims)
{
    DIV_PRIMS *p;
    DIV_PRIM *q;
    int i;

    if((p=(DIV_PRIMS *)MDU_Alloc(sizeof(DIV_PRIMS)
				 +sizeof(DIV_PRIM)*n_div_prims))==NULL) return NULL;

    p->n_div_prims=n_div_prims;
    p->start_single=n_div_prims;

    q=p->div_prims;
    for(i=0;i<n_div_prims;i++,q++){
	q->n_prims=0;
	q->n_tid=0;
	q->tid=NULL;
	q->have_env=0;

	q->prims.prea=&(q->prims);
	q->prims.next=&(q->prims);
	q->prims.n_tid=0x80000000;
	q->prims.p[0].vid=0x80000000;
	q->prims.p[0].nid=0x80000000;
	q->prims.p[0].uvs=NULL;
	q->prims.p[1].vid=0x80000000;
	q->prims.p[1].nid=0x80000000;
	q->prims.p[1].uvs=NULL;
	q->prims.p[2].vid=0x80000000;
	q->prims.p[2].nid=0x80000000;
	q->prims.p[2].uvs=NULL;
	q->prims.flag=0x80000000;
    }

    return p;
}

void Div_Prims_Move(DIV_PRIMS *r,DIV_PRIMS *a)
{
    int i;
    DIV_PRIM *b;

    memcpy(r->div_prims,a->div_prims,sizeof(DIV_PRIM)*a->n_div_prims);

    r->start_single=a->start_single;
    b=a->div_prims;
    for(i=0;i<a->n_div_prims;i++){
	(b+i)->n_prims=0;
	(b+i)->n_tid=0;
	(b+i)->tid=NULL;
	(b+i)->prims.prea=&((b+i)->prims);
	(b+i)->prims.next=&((b+i)->prims);
    }

    b=r->div_prims;
    for(i=0;i<a->n_div_prims;i++){
	if((b+i)->n_prims>0){
	    (b+i)->prims.prea->next=&((b+i)->prims);
	    (b+i)->prims.next->prea=&((b+i)->prims);
	}
	else{
	    (b+i)->prims.prea=&((b+i)->prims);
	    (b+i)->prims.next=&((b+i)->prims);
	}
    }
}

/* 辺の繋がりを調べて、ストリップで繋がる可能性のあるものでグループ分けする。
   人間型の場合は、全てストリップで繋がる可能性があるので、グループ分けされることは
   ないが、ステージのような場合には、有効と思われる。*/
static DIV_PRIMS *Div_Prim_Divide_By_Line(DIV_PRIM *dprim)
{
    DIV_PRIMS *prims,*prims2;
    DIV_PRIM *d,*e;
    int i,j;
    int n_div_prims;
    int new_n_div_prims;

    // printf("Start\n");

    if((prims=Div_Prims_Alloc(1))==NULL) return NULL;
    if(!Div_Prim_Alloc(prims->div_prims,dprim->n_tid)){
	Div_Prims_Free(prims);
	return NULL;
    }

    memcpy(prims->div_prims[0].tid,dprim->tid,dprim->n_tid*sizeof(u_int));
    memcpy(&(prims->div_prims[0].prims),&(dprim->prims),sizeof(PRIM));
    dprim->prims.next->prea=dprim->prims.prea->next=&(prims->div_prims[0].prims);

    prims->div_prims[0].n_prims=dprim->n_prims;
    prims->div_prims[0].have_env=dprim->have_env;

    dprim->prims.prea=dprim->prims.next=&(dprim->prims);
    dprim->n_prims=0;


    new_n_div_prims=n_div_prims=prims->n_div_prims;

    /* ストリップで繋げられる三角形かどうかチェックしていく。 */
    for(i=0;i<n_div_prims;i++){
	PRIM *p=prims->div_prims[i].prims.next;
	int n_prims=prims->div_prims[i].n_prims;
	int n_tid=prims->div_prims[i].n_tid;


	/* グループ番号の初期化 */
	for(j=0;j<n_prims;j++){
	    p->flag=j;
	    p=p->next;
	}


	p=prims->div_prims[i].prims.next;

	/* グループ分け */
	for(j=0;j<n_prims;j++){
	    PRIM *pp;
	    int k;

back_start:
	    pp=p->next;

	    for(k=j+1;k<n_prims;k++){
		int ii,jj;
		int ii2,jj2;

		for(ii=0;ii<3;ii++){
		    for(jj=0;jj<3;jj++){
			if(Cmp_Prim_Point(&(p->p[ii]),&(pp->p[jj]),n_tid)) goto hit_point1;
		    }
		}

		pp=pp->next;
		continue;

hit_point1:
		for(ii2=ii+1;ii2<3;ii2++){
		    for(jj2=0;jj2<3;jj2++){
			if(Cmp_Prim_Point(&(p->p[ii2]),&(pp->p[jj2]),n_tid)) goto hit_point2;
		    }
		}

		pp=pp->next;
		continue;

hit_point2:
		if(pp->flag<p->flag){
		    /* これから上書きしようとするグループ番号の方が若い番号の場合、
		       若い番号を採用してやり直し */

		    p->flag=pp->flag;
		    goto back_start;
		}
		pp->flag=p->flag;
		pp=pp->next;
	    }
	    p=p->next;
	}


	p=prims->div_prims[i].prims.next;

	/* グループ番号の数を計算 */
	{
	    int now_group=0;
	    int n_addgroups=0;

	    for(j=0;j<n_prims;j++){
		if(p->flag>now_group){
		    n_addgroups++;
		    now_group=p->flag;
		}
		p=p->next;
	    }

	    new_n_div_prims+=n_addgroups;
	}
    }

#if 0
    printf("new_n_div_prims , n_div_prims = %d, %d\n",new_n_div_prims,n_div_prims);
#endif

    if(n_div_prims==new_n_div_prims) return prims;

    if((prims2=Div_Prims_Alloc(new_n_div_prims))==NULL){
	Div_Prims_Free(prims);
	return NULL;
    }


    /* グループ分け */
    d=prims->div_prims+0;
    e=prims2->div_prims+0;

    new_n_div_prims=0;

    for(i=0;i<n_div_prims;i++,d++){
	PRIM *p=d->prims.next;
	int n_prims=d->n_prims;
	int n_divs=0;

	for(j=0;j<n_prims;j++){
	    PRIM *np=p->next;
	    int k;

	    for(k=0;k<n_divs;k++){
		if(p->flag==(e+k)->prims.next->flag) break;
	    }
	    if(k==n_divs){
		int ii;

		if(!Div_Prim_Alloc(e+k,d->n_tid)){
		    Div_Prims_Free(prims);
		    Div_Prims_Free(prims2);
		    return NULL;
		}
		for(ii=0;ii<d->n_tid;ii++) *((e+k)->tid+ii)=*(d->tid+ii);
		(e+k)->have_env=d->have_env;

		n_divs++;
	    }

	    // printf("n_prims = %d ( %d )\n",(e+k)->n_prims,d->n_prims);

	    p->next->prea=p->prea;
	    p->prea->next=p->next;

	    p->next=(e+k)->prims.prea->next;
	    p->prea=(e+k)->prims.prea;
	    (e+k)->prims.prea->next=p;
	    (e+k)->prims.prea=p;

	    (e+k)->n_prims++;
	    d->n_prims--;

	    p=np;
	}

	e+=n_divs;
	new_n_div_prims+=n_divs;
    }

    // printf("new_n_div_prims = %d\n",new_n_div_prims);

    Div_Prims_Free(prims);

    // printf("End\n");

    return prims2;
}

/* -------------------------------------------------------------------------- */

void Strip_Free(STRIP *strip)
{
    int i;
    if(strip->tid!=NULL) MDU_Free(strip->tid);
    for(i=0;i<strip->n_points;i++){
	if(strip->p[i].uvs!=NULL) MDU_Free(strip->p[i].uvs);
    }
    MDU_Free(strip);
}

int Strip_Uvs_Alloc(STRIP *s,int index)
{
    if((s->p[index].uvs=(TVECTOR *)MDU_Alloc(sizeof(TVECTOR)*s->n_tid))==NULL) return 0;
    return 1;
}

STRIP *Strip_Alloc(int n_tid,int strip_len)
{
    STRIP *strip;
    int i;

    if((strip=MDU_Alloc(sizeof(STRIP)+sizeof(PRIM_POINT)*strip_len))==NULL) return NULL;

    strip->tid=NULL;
    if(n_tid>0){
	if((strip->tid=(u_int *)MDU_Alloc(sizeof(u_int)*n_tid))==NULL){
	    Strip_Free(strip);
	    return NULL;
	}
    }
    strip->n_points=strip_len;
    strip->n_tid=n_tid;
    strip->have_env=0;
    strip->n_objids=0;
    strip->max_objids=0;

    for(i=0;i<strip_len;i++){
	strip->p[i].uvs=NULL;
    }

    return strip;
}

void Strips_Free(STRIP **strips,int n_strips)
{
    int i;
    for(i=0;i<n_strips;i++){
	if(*(strips+i)!=NULL) Strip_Free(*(strips+i));
    }
    MDU_Free(strips);
}

STRIP **Strips_Alloc(int n_strips)
{
    STRIP **strips;
    int i;
    if((strips=(STRIP **)MDU_Alloc(sizeof(STRIP *)*n_strips))==NULL) return NULL;
    for(i=0;i<n_strips;i++) *(strips+i)=NULL;
    return strips;
}

int Cmp_Prim_Point(PRIM_POINT *p1,PRIM_POINT *p2,int n_tid)
{
    int i;

    if(p1->vid!=p2->vid) return 0;

    // printf("Check 1\n");

    if(p1->nid!=p2->nid) return 0;

    // printf("Check 2\n");

    for(i=0;i<n_tid;i++){
	int u1,u2,v1,v2;

	u1=(int)((p1->uvs+i)->u*4096.0f);
	v1=(int)((p1->uvs+i)->v*4096.0f);
	u2=(int)((p2->uvs+i)->u*4096.0f);
	v2=(int)((p2->uvs+i)->v*4096.0f);

	if(u1!=u2){
#if 0
	    printf("Vid is equal , but U is deferent.(n_tid = %d)\n",n_tid);
	    printf("Addr p1->u,v p2->u,v = %4.3f,%4.3f  %4.3f,%4.3f\n",
		   (p1->uvs+i)->u,(p1->uvs+i)->v,(p2->uvs+i)->u,(p2->uvs+i)->v);
#endif
	    return 0;
	}
	if(v1!=v2){
#if 0
	    printf("Vid is equal , but V is deferent.(n_tid = %d)\n",n_tid);
	    printf("Addr p1->u,v p2->u,v = %4.3f,%4.3f  %4.3f,%4.3f\n",
		   (p1->uvs+i)->u,(p1->uvs+i)->v,(p2->uvs+i)->u,(p2->uvs+i)->v);
#endif
	    return 0;
	}
    }

    return 1;
}

int Cmp_Prim_Point_X(PRIM_POINT *p1,PRIM_POINT *p2,int n_tid)
{
    int i;

    if(p1->vid!=p2->vid) return 0;

    // printf("Check 1\n");

    if(p1->nid!=p2->nid) return 0;

    // printf("Check 2\n");

    for(i=0;i<n_tid;i++){
	int u1,u2,v1,v2;

	u1=(int)((p1->uvs+i)->u*32767.0f);
	v1=(int)((p1->uvs+i)->v*32767.0f);
	u2=(int)((p2->uvs+i)->u*32767.0f);
	v2=(int)((p2->uvs+i)->v*32767.0f);

	if(u1!=u2){
#if 0
	    printf("Vid is equal , but U is deferent.(n_tid = %d)\n",n_tid);
	    printf("Addr p1->u,v p2->u,v = %4.3f,%4.3f  %4.3f,%4.3f\n",
		   (p1->uvs+i)->u,(p1->uvs+i)->v,(p2->uvs+i)->u,(p2->uvs+i)->v);
#endif
	    return 0;
	}
	if(v1!=v2){
#if 0
	    printf("Vid is equal , but V is deferent.(n_tid = %d)\n",n_tid);
	    printf("Addr p1->u,v p2->u,v = %4.3f,%4.3f  %4.3f,%4.3f\n",
		   (p1->uvs+i)->u,(p1->uvs+i)->v,(p2->uvs+i)->u,(p2->uvs+i)->v);
#endif
	    return 0;
	}
    }

    return 1;
}



#define DRAW_FLAG_NODRAW	0x8000		/* この頂点では描画を開始しない */
#define DRAW_FLAG_DOUBLE_SIDE	0x0fff		/* 両面ポリゴンの場合 */
#define DRAW_FLAG_CLIP_RIGHT	0x0000		/* 頂点が画面上で右回りになった場合、
						   ポリゴンをクリッピングする */
#define DRAW_FLAG_CLIP_LEFT	0x0020		/* 頂点が画面上で左回りになった場合、
						   ポリゴンをクリッピングする */

STRIP **EasyStrip(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points)
{
    STRIP *s;
    PRIM *prim;
    int point_p=0;
    // int dir;

    if(div_prim->n_prims==0) return strips;

#if 1
    {
	STRIP **tmp_strips;

	if((tmp_strips=Strips_Alloc(*n_strips+1))==NULL){
	    if(strips!=NULL) Strips_Free(strips,*n_strips);
	    return NULL;
	}

	if(strips!=NULL){
	    memcpy(tmp_strips,strips,sizeof(STRIP *)*(*n_strips));
	    MDU_Free(strips);
	}

	strips=tmp_strips;
    }
#endif

    if((s=Strip_Alloc(div_prim->n_tid,n_max_points))==NULL){
	Strips_Free(strips,*n_strips);
	return NULL;
    }

    if(div_prim->n_tid>0){
	memcpy(s->tid,div_prim->tid,sizeof(u_int)*div_prim->n_tid);
    }

    s->n_points=0;
    s->n_tid=div_prim->n_tid;
    s->have_env=div_prim->have_env;

    *(strips+(*n_strips))=s;
    (*n_strips)++;

    prim=div_prim->prims.next;
    // dir=0;
    while(div_prim->n_prims>0){

	/* 進行チェック用 */
	printf(".");
	fflush(stdout);

	if(point_p>n_max_points-4){

#if 1
	    STRIP **tmp_strips;

	    if((tmp_strips=Strips_Alloc(*n_strips+1))==NULL){
		Strips_Free(strips,*n_strips);
		return NULL;
	    }
	    memcpy(tmp_strips,strips,sizeof(STRIP *)*(*n_strips));
	    MDU_Free(strips);

	    strips=tmp_strips;
#endif

	    s->n_points=point_p;

	    if((s=Strip_Alloc(div_prim->n_tid,n_max_points))==NULL){
		Strips_Free(strips,*n_strips);
		return NULL;
	    }
	    if(div_prim->n_tid>0){
		memcpy(s->tid,div_prim->tid,sizeof(u_int)*div_prim->n_tid);
	    }

	    s->n_points=0;
	    point_p=0;

	    s->n_tid=div_prim->n_tid;
	    s->have_env=div_prim->have_env;

	    *(strips+(*n_strips))=s;
	    (*n_strips)++;
	}

	if(s->n_tid!=prim->n_tid){
	    printf("----------------- Deferent Texture\n");
	    *(long *)1=0;
	}
	else if(s->n_tid>0){
	    if(s->tid[0]!=prim->tid[0]){
		printf("----------------- Deferent Texture\n");
		*(long *)1=0;
	    }
	}

	if(point_p==0){
	    memcpy(&(s->p[0]),&(prim->p[0]),sizeof(PRIM_POINT));
	    memcpy(&(s->p[1]),&(prim->p[1]),sizeof(PRIM_POINT));
	    memcpy(&(s->p[2]),&(prim->p[2]),sizeof(PRIM_POINT));
	    s->p[0].draw_flag=DRAW_FLAG_NODRAW;
	    s->p[1].draw_flag=DRAW_FLAG_NODRAW;
	    s->p[2].draw_flag=DRAW_FLAG_CLIP_LEFT;
	    // dir=1;
	    point_p+=3;
	}
	else if(Cmp_Prim_Point(&(s->p[point_p-2]),&(prim->p[0]),s->n_tid) &&
		Cmp_Prim_Point(&(s->p[point_p-1]),&(prim->p[1]),s->n_tid)){

	    // printf("Check 100\n");

	    MDU_Free(prim->p[0].uvs);
	    MDU_Free(prim->p[1].uvs);
	    memcpy(&(s->p[point_p+0]),&(prim->p[2]),sizeof(PRIM_POINT));
#if 0
	    if(dir){
		s->p[point_p+0].draw_flag=DRAW_FLAG_CLIP_RIGHT;
	    }
	    else{
		s->p[point_p+0].draw_flag=DRAW_FLAG_CLIP_LEFT;
	    }
#else
	    s->p[point_p+0].draw_flag=DRAW_FLAG_CLIP_LEFT;
#endif
	    // dir^=1;
	    point_p++;
	}
	else if(Cmp_Prim_Point(&(s->p[point_p-2]),&(prim->p[1]),s->n_tid) &&
		Cmp_Prim_Point(&(s->p[point_p-1]),&(prim->p[0]),s->n_tid)){

	    // printf("Check 101\n");

	    MDU_Free(prim->p[0].uvs);
	    MDU_Free(prim->p[1].uvs);
	    memcpy(&(s->p[point_p+0]),&(prim->p[2]),sizeof(PRIM_POINT));
#if 0
	    if(dir){
		s->p[point_p+0].draw_flag=DRAW_FLAG_CLIP_LEFT;
	    }
	    else{
		s->p[point_p+0].draw_flag=DRAW_FLAG_CLIP_RIGHT;
	    }
#else
	    s->p[point_p+0].draw_flag=DRAW_FLAG_CLIP_RIGHT;
#endif
	    // dir^=1;
	    point_p++;
	}
	else if(Cmp_Prim_Point(&(s->p[point_p-1]),&(prim->p[0]),s->n_tid)){

	    // printf("Check 102\n");

	    MDU_Free(prim->p[0].uvs);
	    memcpy(&(s->p[point_p+0]),&(prim->p[1]),sizeof(PRIM_POINT));
	    memcpy(&(s->p[point_p+1]),&(prim->p[2]),sizeof(PRIM_POINT));
	    s->p[point_p+0].draw_flag=DRAW_FLAG_NODRAW;
	    s->p[point_p+1].draw_flag=DRAW_FLAG_CLIP_LEFT;
	    // dir=1;
	    point_p+=2;
	}
	else{
	    memcpy(&(s->p[point_p+0]),&(prim->p[0]),sizeof(PRIM_POINT));
	    memcpy(&(s->p[point_p+1]),&(prim->p[1]),sizeof(PRIM_POINT));
	    memcpy(&(s->p[point_p+2]),&(prim->p[2]),sizeof(PRIM_POINT));
	    s->p[point_p+0].draw_flag=DRAW_FLAG_NODRAW;
	    s->p[point_p+1].draw_flag=DRAW_FLAG_NODRAW;
	    s->p[point_p+2].draw_flag=DRAW_FLAG_CLIP_LEFT;
	    // dir=1;
	    point_p+=3;
	}
	prim->p[0].uvs=NULL;
	prim->p[1].uvs=NULL;
	prim->p[2].uvs=NULL;

	{
	    PRIM *np=prim->next;

	    prim->next->prea=prim->prea;
	    prim->prea->next=prim->next;

	    // printf("Check 103\n");

	    Prim_Free(prim);
	    prim=np;
	}
	div_prim->n_prims--;
    }

    s->n_points=point_p;

    /* 進行チェック用 */
    printf("!");
    fflush(stdout);

    return strips;
}

/* ストリップを作る */
static TMP_STRIP *Set_Strip_Algo_1(PRIM *start_prim,int vert_num,DIV_PRIM *div_prim,
				   int n_max_points,int *n_strip)
{
    PRIM *p;
    TMP_STRIP *strip;
    PRIM_POINT *line[2];
    int i,j,k;
    int dir=RIGHT;

    // printf("Check 1.1\n");

    /* メモリの確保 */
    if((strip=(TMP_STRIP *)MDU_Alloc(sizeof(TMP_STRIP)*(n_max_points-2)))==NULL){
	*n_strip=0;
	return NULL;
    }

    // printf("Check 1.2\n");

    /* チェックフラグの初期化 */
    p=div_prim->prims.next;
    for(i=0;i<div_prim->n_prims;i++){

	//printf("0x%08x\n",p);

	p->flag=0;
	p=p->next;
    }

    // printf("Check 1.3\n");

    i=(vert_num+dir+3) % 3;
    line[0]=start_prim->p+i;

    i=(i+dir+3) % 3;
    line[1]=start_prim->p+i;

    //printf("Check 1.4\n");

    (strip+0)->prim=start_prim;
    (strip+0)->vert_num=i;
    (strip+0)->dir=dir;

    start_prim->flag=1;
    *n_strip=1;

    // printf("Check 1.5\n");

    while(1){
	p=div_prim->prims.next;

	for(i=0;i<div_prim->n_prims;i++,p=p->next){

	    if(p->flag==1) continue;

	    if(p->flag==0x80000000){
		printf("Failed Check\n");
	    }

	    for(dir=-1;dir<2;dir+=2){

		for(j=0;j<3;j++){

		    PRIM_POINT *p1,*p2;

		    k=(j+dir+3) % 3;

		    p1=p->p+j;
		    p2=p->p+k;

		    if(Cmp_Prim_Point(line[0],p1,div_prim->n_tid) &&
		       Cmp_Prim_Point(line[1],p2,div_prim->n_tid)){

			k=(k+dir+3) % 3;

			(strip+*n_strip)->prim=p;
			(strip+*n_strip)->vert_num=k;
			(strip+*n_strip)->dir=dir;

			(*n_strip)++;

			line[0]=line[1];
			line[1]=p->p+k;

			p->flag=1;

			goto next;
		    }
		}
	    }
	}
	break;

next:

	/* ストリップが、規定の頂点数より大きくなったら、強制終了 */
	if(*n_strip+2>=n_max_points) break;
    }

    return strip;
}    

/* テンポラリのフォーマットを本形式に変更する */
static STRIP *ConvertToStrip(TMP_STRIP *strip,int strip_len,DIV_PRIM *div_prim)
{
    STRIP *s;
    int i;

    if((s=Strip_Alloc(div_prim->n_tid,strip_len+2))==NULL) return NULL;
    if(div_prim->n_tid>0){
	memcpy(s->tid,div_prim->tid,sizeof(u_int)*div_prim->n_tid);
    }

    s->n_points=strip_len+2;
    s->n_tid=div_prim->n_tid;
    s->have_env=div_prim->have_env;


    i=(strip+0)->vert_num;

    i=(i+(strip+0)->dir+3) % 3;
    memcpy(&(s->p[0]),&((strip+0)->prim->p[i]),sizeof(PRIM_POINT));
    s->p[0].draw_flag=DRAW_FLAG_NODRAW;
    (strip+0)->prim->p[i].uvs=NULL;

    i=(i+(strip+0)->dir+3) % 3;
    memcpy(&(s->p[1]),&((strip+0)->prim->p[i]),sizeof(PRIM_POINT));
    s->p[1].draw_flag=DRAW_FLAG_NODRAW;
    (strip+0)->prim->p[i].uvs=NULL;

    for(i=0;i<strip_len;i++){
	PRIM *prim=(strip+i)->prim;
	int j=(strip+i)->vert_num;
	int dir=(strip+i)->dir;

	memcpy(&(s->p[i+2]),&(prim->p[j]),sizeof(PRIM_POINT));
	if(dir==RIGHT) s->p[i+2].draw_flag=DRAW_FLAG_CLIP_LEFT;
	else s->p[i+2].draw_flag=DRAW_FLAG_CLIP_RIGHT;
	prim->p[j].uvs=NULL;

	prim->next->prea=prim->prea;
	prim->prea->next=prim->next;
	div_prim->n_prims--;

	Prim_Free(prim);
    }

    return s;
}



STRIP **Strip_Algo_1(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points)
{
    static STRIP **_Strip_Algo_1_(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points);
    DIV_PRIMS *dprims;
    DIV_PRIM *div_prim2;
    int i;

    if((dprims=Div_Prim_Divide_By_Line(div_prim))==NULL){
	printf("Failed in Divide_By_Line\n");
	return NULL;
    }
    div_prim2=dprims->div_prims;

    // printf("Divide By Line = %d\n",dprims->n_div_prims);

    for(i=0;i<dprims->n_div_prims;i++,div_prim2++){
	if((strips=_Strip_Algo_1_(div_prim2,strips,n_strips,n_max_points))==NULL){
	    if(*n_strips!=0){
		printf("Failed\n");
		Div_Prims_Free(dprims);
		return NULL;
	    }
	}
    }

    Div_Prims_Free(dprims);
    return strips;
}


/* アルゴリズム１ */
STRIP **_Strip_Algo_1_(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points)
{
    while(div_prim->n_prims>0){
	PRIM *p;
	TMP_STRIP *strip,*tmp_strip;
	int strip_len,tmp_strip_len;
	int i,j;

	/* 進行チェック用 */
	printf(".");
	fflush(stdout);

	p=div_prim->prims.next;

	strip=NULL;
	strip_len=0;

	// printf("  Check 1\n");

	/* 開始プリミティブを色々ずらし、一番長いストリップの出来る
	   開始位置を探しだす。*/
	for(i=0;i<div_prim->n_prims;i++,p=p->next){
	    for(j=0;j<3;j++){
		tmp_strip=Set_Strip_Algo_1(p,j,div_prim,n_max_points,&tmp_strip_len);
		if(tmp_strip_len>strip_len){
		    if(strip!=NULL) MDU_Free(strip);
		    strip=tmp_strip;
		    strip_len=tmp_strip_len;
		}
		else{
		    if(tmp_strip!=NULL) MDU_Free(tmp_strip);
		}
	    }
	}

	// printf("  Check 2\n");

	/* 領域の確保 */
	{
	    STRIP **strips2;

	    if((strips2=Strips_Alloc(*n_strips+1))==NULL){
		Strips_Free(strips,*n_strips);
		MDU_Free(strip);
		return NULL;
	    }
	    if(strips!=NULL){
		memcpy(strips2,strips,sizeof(STRIP *)*(*n_strips));
		MDU_Free(strips);
	    }
	    strips=strips2;
	}

	// printf("  Check 3\n");

	/* テンポラリのワーク形式を本形式に変更する。*/
	if((*(strips+*n_strips)=ConvertToStrip(strip,strip_len,div_prim))==NULL){
	    Strips_Free(strips,*n_strips);
	    MDU_Free(strip);
	    return NULL;
	}
	MDU_Free(strip);

	(*n_strips)++;
    }

    return strips;
}

/* ストリップをできるだけまとめる */
int Union_STRIP(STRIP **strips,int n_strips,int n_max_points)
{
    int i;

    for(i=0;i<n_strips; ){
	int index=-1;
	int len=(*(strips+i))->n_points;
	int max_len=len;
	int j;

	/* 進行チェック用 */
	printf("#");
	fflush(stdout);

	for(j=i+1;j<n_strips;j++){
	    if(len+(*(strips+j))->n_points<=n_max_points &&
	       len+(*(strips+j))->n_points>max_len){

		max_len=len+(*(strips+j))->n_points;
		index=j;
	    }
	}
	if(index!=-1){
	    STRIP *tmp_strip;

	    if((tmp_strip=Strip_Alloc(0,max_len))==NULL){
		return n_strips;
	    }

	    memcpy(tmp_strip,*(strips+i),
		   sizeof(STRIP)+sizeof(PRIM_POINT)*len);
	    memcpy(&(tmp_strip->p[len]),&((*(strips+index))->p[0]),
		   sizeof(PRIM_POINT)*(*(strips+index))->n_points);
	    tmp_strip->n_points=max_len;

	    if((*(strips+index))->tid!=NULL) MDU_Free((*(strips+index))->tid);
	    MDU_Free(*(strips+index));
	    MDU_Free(*(strips+i));

	    *(strips+i)=tmp_strip;
	    n_strips--;
	    for(j=index;j<n_strips;j++) *(strips+j)=*(strips+j+1);

	    continue;
	}
	i++;
    }

    return n_strips;
}


/* -------------------------------------------------------------------------- */
/* X-Box用アルゴリズム。
   右回転のみをチェックする、Unionで作られるデータが少し変わる。*/

/* ストリップを作る */
static TMP_STRIP *Set_Strip_Algo_1_X(PRIM *start_prim,int vert_num,DIV_PRIM *div_prim,
				     int n_max_points,int *n_strip)
{
    PRIM *p;
    TMP_STRIP *strip;
    PRIM_POINT *line[2];
    int i,j,k;
    int dir=RIGHT;

    // printf("Check 1.1\n");

    /* メモリの確保 */
    if((strip=(TMP_STRIP *)MDU_Alloc(sizeof(TMP_STRIP)*(n_max_points-2)))==NULL){
	*n_strip=0;
	return NULL;
    }

    // printf("Check 1.2\n");

    /* チェックフラグの初期化 */
    p=div_prim->prims.next;
    for(i=0;i<div_prim->n_prims;i++){

	//printf("0x%08x\n",p);

	p->flag=0;
	p=p->next;
    }

    // printf("Check 1.3\n");

    i=(vert_num+dir+3) % 3;
    line[1]=start_prim->p+i;

    i=(i+dir+3) % 3;
    line[0]=start_prim->p+i;

    //printf("Check 1.4\n");

    (strip+0)->prim=start_prim;
    (strip+0)->vert_num=i;
    (strip+0)->dir=dir;

    start_prim->flag=1;
    *n_strip=1;

    // printf("Check 1.5\n");

    while(1){
	p=div_prim->prims.next;

	for(i=0;i<div_prim->n_prims;i++,p=p->next){

	    if(p->flag==1) continue;

	    if(p->flag==0x80000000){
		printf("Failed Check\n");
	    }

	    dir=RIGHT;

	    for(j=0;j<3;j++){

		PRIM_POINT *p1,*p2;

		k=(j+dir+3) % 3;

		p1=p->p+j;
		p2=p->p+k;

		if(Cmp_Prim_Point_X(line[0],p1,div_prim->n_tid) &&
		   Cmp_Prim_Point_X(line[1],p2,div_prim->n_tid)){

		    k=(k+dir+3) % 3;

		    (strip+*n_strip)->prim=p;
		    (strip+*n_strip)->vert_num=k;
		    (strip+*n_strip)->dir=dir;

		    if((*n_strip) & 1){
			/* 奇数番目であればline[1]を更新 */
			line[1]=p->p+k;
		    }
		    else{
			/* 偶数番目であればline[0]を更新 */
			line[0]=p->p+k;
		    }

		    (*n_strip)++;

		    p->flag=1;

		    goto next;
		}
	    }
	}
	break;

next:

	/* ストリップが、規定の頂点数より大きくなったら、強制終了 */
	if(*n_strip+2>=n_max_points) break;
    }

    return strip;
}    

/* テンポラリのフォーマットを本形式に変更する */
static STRIP *ConvertToStripX(TMP_STRIP *strip,int strip_len,DIV_PRIM *div_prim)
{
    STRIP *s;
    int i;

    if((s=Strip_Alloc(div_prim->n_tid,strip_len+2))==NULL) return NULL;
    if(div_prim->n_tid>0){
	memcpy(s->tid,div_prim->tid,sizeof(u_int)*div_prim->n_tid);
    }

    s->n_points=strip_len+2;
    s->n_tid=div_prim->n_tid;
    s->have_env=div_prim->have_env;


    i=(strip+0)->vert_num;

    i=(i+(strip+0)->dir+3) % 3;
    memcpy(&(s->p[0]),&((strip+0)->prim->p[i]),sizeof(PRIM_POINT));
    s->p[0].draw_flag=DRAW_FLAG_NODRAW;
    (strip+0)->prim->p[i].uvs=NULL;

    i=(i+(strip+0)->dir+3) % 3;
    memcpy(&(s->p[1]),&((strip+0)->prim->p[i]),sizeof(PRIM_POINT));
    s->p[1].draw_flag=DRAW_FLAG_NODRAW;
    (strip+0)->prim->p[i].uvs=NULL;

    for(i=0;i<strip_len;i++){
	PRIM *prim=(strip+i)->prim;
	int j=(strip+i)->vert_num;
	int dir=(strip+i)->dir;

	memcpy(&(s->p[i+2]),&(prim->p[j]),sizeof(PRIM_POINT));
	if(dir==RIGHT) s->p[i+2].draw_flag=DRAW_FLAG_CLIP_LEFT;
	else s->p[i+2].draw_flag=DRAW_FLAG_CLIP_RIGHT;
	prim->p[j].uvs=NULL;

	prim->next->prea=prim->prea;
	prim->prea->next=prim->next;
	div_prim->n_prims--;

	Prim_Free(prim);
    }

    return s;
}



STRIP **Strip_Algo_1_X(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points)
{
    static STRIP **_Strip_Algo_1_X(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points);
    DIV_PRIMS *dprims;
    DIV_PRIM *div_prim2;
    int i;

    if((dprims=Div_Prim_Divide_By_Line(div_prim))==NULL){
	printf("Failed in Divide_By_Line\n");
	return NULL;
    }
    div_prim2=dprims->div_prims;

    // printf("Divide By Line = %d\n",dprims->n_div_prims);

    for(i=0;i<dprims->n_div_prims;i++,div_prim2++){
	if((strips=_Strip_Algo_1_X(div_prim2,strips,n_strips,n_max_points))==NULL){
	    if(*n_strips!=0){
		printf("Failed\n");
		Div_Prims_Free(dprims);
		return NULL;
	    }
	}
    }

    Div_Prims_Free(dprims);
    return strips;
}


/* アルゴリズム１ */
STRIP **_Strip_Algo_1_X(DIV_PRIM *div_prim,STRIP **strips,int *n_strips,int n_max_points)
{
    while(div_prim->n_prims>0){
	PRIM *p;
	TMP_STRIP *strip,*tmp_strip;
	int strip_len,tmp_strip_len;
	int i,j;

	/* 進行チェック用 */
	printf(".");
	fflush(stdout);

	p=div_prim->prims.next;

	strip=NULL;
	strip_len=0;

	// printf("  Check 1\n");

	/* 開始プリミティブを色々ずらし、一番長いストリップの出来る
	   開始位置を探しだす。*/
	for(i=0;i<div_prim->n_prims;i++,p=p->next){
	    for(j=0;j<3;j++){
		tmp_strip=Set_Strip_Algo_1_X(p,j,div_prim,n_max_points,&tmp_strip_len);
		if(tmp_strip_len>strip_len){
		    if(strip!=NULL) MDU_Free(strip);
		    strip=tmp_strip;
		    strip_len=tmp_strip_len;
		}
		else{
		    if(tmp_strip!=NULL) MDU_Free(tmp_strip);
		}
	    }
	}

	// printf("  Check 2\n");

	/* 領域の確保 */
	{
	    STRIP **strips2;

	    if((strips2=Strips_Alloc(*n_strips+1))==NULL){
		Strips_Free(strips,*n_strips);
		MDU_Free(strip);
		return NULL;
	    }
	    if(strips!=NULL){
		memcpy(strips2,strips,sizeof(STRIP *)*(*n_strips));
		MDU_Free(strips);
	    }
	    strips=strips2;
	}

	// printf("  Check 3\n");

	/* テンポラリのワーク形式を本形式に変更する。*/
	if((*(strips+*n_strips)=ConvertToStripX(strip,strip_len,div_prim))==NULL){
	    Strips_Free(strips,*n_strips);
	    MDU_Free(strip);
	    return NULL;
	}
	MDU_Free(strip);

	(*n_strips)++;
    }

    return strips;
}

/* ストリップをできるだけまとめる */
int Union_STRIP_X(STRIP **strips,int n_strips,int n_max_points)
{
    int i;

    for(i=0;i<n_strips; ){
	int index=-1;
	int len=(*(strips+i))->n_points;
	int max_len=len;
	int j;
	int ofspoint;

	/* 進行チェック用 */
	printf("#");
	fflush(stdout);

	/* 増やす必要のある頂点数 */
	if((len-2) & 1){
	    /* 三角形が奇数枚 */
	    ofspoint=3;
	}
	else{
	    /* 三角形が偶数枚 */
	    ofspoint=2;
	}

	for(j=i+1;j<n_strips;j++){
	    if(len+ofspoint+(*(strips+j))->n_points<=n_max_points &&
	       len+ofspoint+(*(strips+j))->n_points>max_len){

		max_len=len+ofspoint+(*(strips+j))->n_points;
		index=j;
	    }
	}
	if(index!=-1){
	    STRIP *tmp_strip;

	    if((tmp_strip=Strip_Alloc(0,max_len))==NULL){
		return n_strips;
	    }

	    memcpy(tmp_strip,*(strips+i),
		   sizeof(STRIP)+sizeof(PRIM_POINT)*len);
	    memcpy(&(tmp_strip->p[len+ofspoint]),&((*(strips+index))->p[0]),
		   sizeof(PRIM_POINT)*(*(strips+index))->n_points);

	    if(ofspoint==2){
		memcpy(&(tmp_strip->p[len]),&(tmp_strip->p[len-1]),sizeof(PRIM_POINT));
		memcpy(&(tmp_strip->p[len+1]),&(tmp_strip->p[len+2]),sizeof(PRIM_POINT));

		tmp_strip->p[len].uvs=NULL;
		tmp_strip->p[len+1].uvs=NULL;

		if(!Strip_Uvs_Alloc(tmp_strip,len)){
		    Strip_Free(tmp_strip);
		    return n_strips;
		}
		if(!Strip_Uvs_Alloc(tmp_strip,len+1)){
		    Strip_Free(tmp_strip);
		    return n_strips;
		}

		memcpy(tmp_strip->p[len].uvs,tmp_strip->p[len-1].uvs,
		       sizeof(TVECTOR)*tmp_strip->n_tid);
		memcpy(tmp_strip->p[len+1].uvs,tmp_strip->p[len+2].uvs,
		       sizeof(TVECTOR)*tmp_strip->n_tid);
	    }
	    else{
		memcpy(&(tmp_strip->p[len]),&(tmp_strip->p[len-2]),sizeof(PRIM_POINT));
		memcpy(&(tmp_strip->p[len+1]),&(tmp_strip->p[len-2]),sizeof(PRIM_POINT));
		memcpy(&(tmp_strip->p[len+2]),&(tmp_strip->p[len+3]),sizeof(PRIM_POINT));

		tmp_strip->p[len].uvs=NULL;
		tmp_strip->p[len+1].uvs=NULL;
		tmp_strip->p[len+2].uvs=NULL;

		if(!Strip_Uvs_Alloc(tmp_strip,len)){
		    Strip_Free(tmp_strip);
		    return n_strips;
		}
		if(!Strip_Uvs_Alloc(tmp_strip,len+1)){
		    Strip_Free(tmp_strip);
		    return n_strips;
		}
		if(!Strip_Uvs_Alloc(tmp_strip,len+2)){
		    Strip_Free(tmp_strip);
		    return n_strips;
		}

		memcpy(tmp_strip->p[len].uvs,tmp_strip->p[len-2].uvs,
		       sizeof(TVECTOR)*tmp_strip->n_tid);
		memcpy(tmp_strip->p[len+1].uvs,tmp_strip->p[len-2].uvs,
		       sizeof(TVECTOR)*tmp_strip->n_tid);
		memcpy(tmp_strip->p[len+2].uvs,tmp_strip->p[len+3].uvs,
		       sizeof(TVECTOR)*tmp_strip->n_tid);
	    }

	    tmp_strip->n_points=max_len;

	    if((*(strips+index))->tid!=NULL) MDU_Free((*(strips+index))->tid);
	    MDU_Free(*(strips+index));
	    MDU_Free(*(strips+i));

	    *(strips+i)=tmp_strip;
	    n_strips--;
	    for(j=index;j<n_strips;j++) *(strips+j)=*(strips+j+1);

	    continue;
	}
	i++;
    }

    return n_strips;
}

/* ストリップの順序を変更 */
void SortStrip(TEXINFO *texs,STRIP **strips,int n_strips)
{
    int i,j;

    /* α抜きがある一枚張りのテクスチャを後半に移す */
    i=0;
    j=n_strips-1;

    while(i<j){
	for( ;i<j;i++){
	    STRIP *s;
	    s=*(strips+i);
	    if(s->n_tid!=1) continue;
	    if((texs+*(s->tid+0))->alpha>254.0f) continue;

	    // printf("Alpha S = %f %d\n",(texs+*(s->tid+0))->alpha,*(s->tid+0));

	    break;
	}
	for( ;i<j;j--){
	    STRIP *s;
	    s=*(strips+j);
	    if(s->n_tid!=1) break;
	    if((texs+*(s->tid+0))->alpha>254.0f){
		// printf("Alpha T = %f\n",(texs+*(s->tid+0))->alpha);
		break;
	    }
	}
	if(i>=j) break;

	{
	    STRIP *s;
	    s=*(strips+i);
	    *(strips+i)=*(strips+j);
	    *(strips+j)=s;
	}

	printf("s");
	fflush(stdout);

	// printf("%d <---> %d\n",i,j);
    }

    if(i<n_strips-2){
	int k;

	/* 半透明同士でソート(挿入ソート) */
	for(j=i+1;j<n_strips;j++){
	    STRIP *s=*(strips+j);

	    for(k=i;k<j;k++){
		STRIP *ts;

		ts=*(strips+k);
		if((texs+*(s->tid+0))->alpha>(texs+*(ts->tid+0))->alpha){
		    int l;

		    for(l=j;l>k;l--){
			*(strips+l)=*(strips+l-1);
		    }
		    // memmove(strips+k+1,strips+k,sizeof(STRIP *)*(j-k));

		    *(strips+k)=s;

		    printf("i");
		    fflush(stdout);

		    break;
		}
	    }
	}
    }
}

/* -------------------------------------------------------------------------- */
