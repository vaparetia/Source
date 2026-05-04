/*
	clothsub.c
		布シミュレート演算サブルーチン

	1999/08/16 K.Kano
	$Id: clothsub.c,v 1.1.1.3 2002/11/19 11:43:08 Yoshizawa1 Exp $
*/

#include "cloth.h"


#define SCRATCHPAD		0x70000000
#define FREEAREA_ON_SCRATCHPAD	SCRATCHPAD

#define setDmaTag(t,i,a,q)	(t)->id=(i),(t)->next=(sceDmaTag *)(a),(t)->qwc=(q)


extern FVECTOR	G_wind;


static int InitClothModel(Work *work)
{
    OBJECT *object=&(work->body);
    int total_vertex,total_polygon;
    int i;

    work->poly_params=NULL;
    work->ver_params=NULL;
    work->line_params=NULL;
    work->array_indexes_vertex=NULL;
    work->vertex[0]=NULL;
    work->vertex[1]=NULL;
    work->normal[0]=NULL;
    work->normal[1]=NULL;
    work->flag=1;

    if((work->array_indexes_vertex
	=(int **)GV_Malloc(sizeof(int *)*object->objs->objs->n_packs))==NULL) return 0;
    GV_ZeroMemory(work->array_indexes_vertex,sizeof(int *)*object->objs->objs->n_packs);

    if((work->vertex[0]
	=(SVECTOR **)GV_Malloc(sizeof(SVECTOR *)*object->objs->objs->n_packs*4))==NULL) return 0;
    GV_ZeroMemory(work->vertex[0],sizeof(SVECTOR *)*object->objs->objs->n_packs*4);

    work->vertex[1]=work->vertex[0]+object->objs->objs->n_packs;
    work->normal[0]=work->vertex[1]+object->objs->objs->n_packs;
    work->normal[1]=work->normal[0]+object->objs->objs->n_packs;

    /* object->n_models==1に想定 */
    total_vertex=0;
    total_polygon=0;
    for(i=0;i<object->objs->objs->n_packs;i++){
	int j;

	total_vertex+=(object->objs->objs->packs+i)->n_verts;
	for(j=0;j<(object->objs->objs->packs+i)->n_verts;j++){
	    if(!(*((object->objs->objs->packs+i)->norms+(j<<2)+3) & 0x8000)) total_polygon++;
	}

	if((*(work->array_indexes_vertex+i)
	    =(int *)GV_Malloc(sizeof(int)*(object->objs->objs->packs+i)->n_verts))==NULL) return 0;

	/* これらのワークは、16Byte境界から始まる必要がある */
	{
	    int size=((sizeof(SVECTOR)*(object->objs->objs->packs+i)->n_verts)+16-1) & ~(16-1);

	    if((*(work->vertex[0]+i)
		=(SVECTOR *)GV_Malloc(size))==NULL) return 0;
	    if((*(work->vertex[1]+i)
		=(SVECTOR *)GV_Malloc(size))==NULL) return 0;
	    if((*(work->normal[0]+i)
		=(SVECTOR *)GV_Malloc(size))==NULL) return 0;
	    if((*(work->normal[1]+i)
		=(SVECTOR *)GV_Malloc(size))==NULL) return 0;
	}

	/* ワークの初期化 */
	for(j=0;j<(object->objs->objs->packs+i)->n_verts;j++){
#ifdef PSX2
	    asm("
	    ld		$8,0(%0)
	    ld		$9,0(%1)
	    sd		$8,0(%2)
	    sd		$8,0(%3)
	    sd		$9,0(%4)
	    sd		$9,0(%5)
            " : :
            "r"((object->objs->objs->packs+i)->verts+(j<<2)),
            "r"((object->objs->objs->packs+i)->norms+(j<<2)),
	    "r"(*(work->vertex[0]+i)+j), "r"(*(work->vertex[1]+i)+j),
	    "r"(*(work->normal[0]+i)+j), "r"(*(work->normal[1]+i)+j) :
	    "$8","$9","memory");
#else
	    *(*(work->vertex[0]+i)+j) =
		  *(*(work->vertex[1]+i)+j) =
			*((object->objs->objs->packs+i)->verts+(j<<2)) ;
	    *(*(work->normal[0]+i)+j) =
		  *(*(work->normal[1]+i)+j) =
			*((object->objs->objs->packs+i)->norms+(j<<2)) ;
#endif
	}

	(object->objs->objs->sub_packet[0]+i)->packet.verts_tag.addr
	    =*(work->vertex[0]+i);
	(object->objs->objs->sub_packet[1]+i)->packet.verts_tag.addr
	    =*(work->vertex[1]+i);
	(object->objs->objs->sub_packet[0]+i)->packet.norms_tag.addr
	    =*(work->normal[0]+i);
	(object->objs->objs->sub_packet[1]+i)->packet.norms_tag.addr
	    =*(work->normal[1]+i);
    }

#if 0
    printf("N Packs = %d\n",object->objs->objs->n_packs);
    printf("Total vertex = %d\n",total_vertex);
    printf("Total polygon = %d\n",total_polygon);
#endif

    if((work->ver_params
	=(VERTEX_PARAMETER *)GV_Malloc(sizeof(VERTEX_PARAMETER)
				       *((total_vertex+8-1) & ~(8-1))))==NULL) return 0;
    {
	int *ver_polyn;

	if((ver_polyn
	    =(int *)GV_Malloc(sizeof(int)*total_vertex))==NULL) return 0;

	work->num_vers=0;

	for(i=0;i<object->objs->objs->n_packs;i++){
	    int j;

	    for(j=0;j<(object->objs->objs->packs+i)->n_verts;j++){
		FVECTOR tmp;
		int k;

		vu0_LdSVv0((SVECTOR *)((object->objs->objs->packs+i)->verts+(j<<2)));
		vu0_IV0toFVv0();
		vu0_Setv0w1();
		vu0_Stv0(&tmp);

		/* 重複の検索 */
		for(k=0;k<work->num_vers;k++){
		    if((work->ver_params+k)->lbase.vx==tmp.vx &&
		       (work->ver_params+k)->lbase.vy==tmp.vy &&
		       (work->ver_params+k)->lbase.vz==tmp.vz) break;
		}
		if(k==work->num_vers){
		    /* 各要素の初期化 */

		    /* TEST */
		    (work->ver_params+k)->type=VERTEX_TYPE_FREE;
		    *(ver_polyn+k)=1;

		    fpu_CopyVector(&((work->ver_params+k)->lbase),&tmp);
		    work->num_vers++;
		}
		else{
		    (*(ver_polyn+k))++;
		}
		*(*(work->array_indexes_vertex+i)+j)=k;
	    }
	}
	for(i=0;i<work->num_vers;i++){
	    (work->ver_params+i)->inv_n=1.0f/(float)(*(ver_polyn+i));
	}
	GV_Free(ver_polyn);
    }

    if((work->poly_params
	=(POLY_PARAMETER *)GV_Malloc(sizeof(POLY_PARAMETER)*total_polygon))==NULL) return 0;
    work->num_polys=0;

    for(i=0;i<object->objs->objs->n_packs;i++){
	int j,v0,v1,v2;

	v0=*(*(work->array_indexes_vertex+i)+0);
	v1=*(*(work->array_indexes_vertex+i)+1);

	for(j=2;j<(object->objs->objs->packs+i)->n_verts;j++){
	    int k;

	    v2=*(*(work->array_indexes_vertex+i)+j);

	    if(!(*((object->objs->objs->packs+i)->norms+(j<<2)+3) & 0x8000)){
		/* 重複の検索 */
		for(k=0;k<work->num_polys;k++){
		    if((work->poly_params+k)->index[0]==v0 &&
		       (work->poly_params+k)->index[1]==v1 &&
		       (work->poly_params+k)->index[2]==v2) break;
		    if((work->poly_params+k)->index[0]==v0 &&
		       (work->poly_params+k)->index[1]==v2 &&
		       (work->poly_params+k)->index[2]==v1) break;
		    if((work->poly_params+k)->index[0]==v1 &&
		       (work->poly_params+k)->index[1]==v0 &&
		       (work->poly_params+k)->index[2]==v2) break;
		    if((work->poly_params+k)->index[0]==v1 &&
		       (work->poly_params+k)->index[1]==v2 &&
		       (work->poly_params+k)->index[2]==v0) break;
		    if((work->poly_params+k)->index[0]==v2 &&
		       (work->poly_params+k)->index[1]==v0 &&
		       (work->poly_params+k)->index[2]==v1) break;
		    if((work->poly_params+k)->index[0]==v2 &&
		       (work->poly_params+k)->index[1]==v1 &&
		       (work->poly_params+k)->index[2]==v0) break;
		}
		if(k==work->num_polys){

		    /* 各要素の初期化 */

		    vu0_Ldv0(&((work->ver_params+v0)->lbase));
		    vu0_Ldv1(&((work->ver_params+v1)->lbase));
		    vu0_Ldv2(&((work->ver_params+v2)->lbase));
		    vu0_Subv0v1v0();
		    vu0_Subv2v1();

		    vu0_LdSVv1((SVECTOR *)((object->objs->objs->packs+i)->norms+(j<<2)));

		    vu0_OuterProductv0v2();

		    (work->poly_params+k)->index[0]=v0;

		    vu0_IV12toFVv1();

		    (work->poly_params+k)->index[1]=v1;
		    (work->poly_params+k)->index[2]=v2;

		    if(vu0_InnerProductv0v1()<0){
			(work->poly_params+k)->normal_sign=-1.0f;
		    }
		    else{
			(work->poly_params+k)->normal_sign=1.0f;
		    }

		    work->num_polys++;
		}
	    }
	    v0=v1; v1=v2;
	}
    }

    if((work->line_params
	=(LINE_PARAMETER *)GV_Malloc(sizeof(LINE_PARAMETER)*work->num_polys*3))==NULL) return 0;
    work->num_lines=0;

    for(i=0;i<work->num_polys;i++){
	int v0,v1,v2;
	int vmin,vmax;
	int j;

	v0=(work->poly_params+i)->index[0];
	v1=(work->poly_params+i)->index[1];
	v2=(work->poly_params+i)->index[2];

	/* 重複の検索 */
	vmin=v0;
	vmax=v1;
	if(vmin>vmax){ int tmp; tmp=vmin; vmin=vmax; vmax=tmp; }
	for(j=0;j<work->num_lines;j++){
	    if((work->line_params+j)->index[0]==vmin &&
	       (work->line_params+j)->index[1]==vmax) break;
	}
	if(j==work->num_lines){
	    float l;

	    (work->line_params+j)->index[0]=vmin;
	    (work->line_params+j)->index[1]=vmax;

	    vu0_Ldv0(&((work->ver_params+vmin)->lbase));
	    vu0_Ldv1(&((work->ver_params+vmax)->lbase));
	    vu0_Subv0v1();
	    l=vu0_VectorLength2v0();
	    (work->line_params+j)->l0=fpu_Sqrt(l);

	    work->num_lines++;
	}

	vmin=v1;
	vmax=v2;
	if(vmin>vmax){ int tmp; tmp=vmin; vmin=vmax; vmax=tmp; }
	for(j=0;j<work->num_lines;j++){
	    if((work->line_params+j)->index[0]==vmin &&
	       (work->line_params+j)->index[1]==vmax) break;
	}
	if(j==work->num_lines){
	    float l;

	    (work->line_params+j)->index[0]=vmin;
	    (work->line_params+j)->index[1]=vmax;

	    vu0_Ldv0(&((work->ver_params+vmin)->lbase));
	    vu0_Ldv1(&((work->ver_params+vmax)->lbase));
	    vu0_Subv0v1();
	    l=vu0_VectorLength2v0();
	    (work->line_params+j)->l0=fpu_Sqrt(l);

	    work->num_lines++;
	}

	vmin=v2;
	vmax=v0;
	if(vmin>vmax){ int tmp; tmp=vmin; vmin=vmax; vmax=tmp; }
	for(j=0;j<work->num_lines;j++){
	    if((work->line_params+j)->index[0]==vmin &&
	       (work->line_params+j)->index[1]==vmax) break;
	}
	if(j==work->num_lines){
	    float l;

	    /* 各要素の初期化 */

	    (work->line_params+j)->index[0]=vmin;
	    (work->line_params+j)->index[1]=vmax;

	    vu0_Ldv0(&((work->ver_params+vmin)->lbase));
	    vu0_Ldv1(&((work->ver_params+vmax)->lbase));
	    vu0_Subv0v1();
	    l=vu0_VectorLength2v0();
	    (work->line_params+j)->l0=fpu_Sqrt(l);

	    work->num_lines++;
	}
    }

#if 0
    printf("N vers = %d\n",work->num_vers);
    printf("N polys = %d\n",work->num_polys);
    printf("N lines = %d\n",work->num_lines);
#endif

    /* TEST */
    (work->ver_params+0)->type=VERTEX_TYPE_STATIC;

    set_fnest(work);

    /* 正常終了 */
    return 1;
}

static void ExitClothModel(Work *work)
{
    if(work->poly_params!=NULL) GV_Free(work->poly_params);
    if(work->ver_params!=NULL) GV_Free(work->ver_params);
    if(work->line_params!=NULL) GV_Free(work->line_params);
    if(work->array_indexes_vertex!=NULL){
	OBJECT *object=&(work->body);
	int i;
	for(i=0;i<object->objs->objs->n_packs;i++){
	    if(*(work->array_indexes_vertex+i)!=NULL){
		GV_Free(*(work->array_indexes_vertex+i));
	    }
	}
    }
    if(work->vertex[0]!=NULL){
	OBJECT *object=&(work->body);
	int i;
	for(i=0;i<object->objs->objs->n_packs;i++){
	    if(*(work->vertex[0]+i)!=NULL){
		GV_Free(*(work->vertex[0]+i));
	    }
	    if(*(work->vertex[1]+i)!=NULL){
		GV_Free(*(work->vertex[1]+i));
	    }
	    if(*(work->normal[0]+i)!=NULL){
		GV_Free(*(work->normal[0]+i));
	    }
	    if(*(work->normal[1]+i)!=NULL){
		GV_Free(*(work->normal[1]+i));
	    }
	}
    }
}



static inline void toSPRDmaSync(void)
{
    //UTL_WaitDma(SCE_DMA_toSPR);
}

static inline void toSPRDmaSendN(void *adr,int qwc,void *spr)
{
#if 0
    sceDmaChan *d=sceDmaGetChan(SCE_DMA_toSPR);
    toSPRDmaSync();
    d->sadr=spr;
    sceDmaSendN(d,adr,qwc);
#else
    UTL_StartMemToSpr(spr,adr,qwc);
#endif
}


static inline void fromSPRDmaSync(void)
{
    //UTL_WaitDma(SCE_DMA_fromSPR);
}

static inline void fromSPRDmaSendN(void *spr,int qwc,void *adr)
{
#if 0
    sceDmaChan *d=sceDmaGetChan(SCE_DMA_fromSPR);
    fromSPRDmaSync();
    d->sadr=spr;
    sceDmaSendN(d,adr,qwc);
#else
    UTL_StartSprToMem(adr,spr,qwc);
#endif
}


#if 0

/* 頂点の基準位置の計算 */
static inline void InitBasicVertex(Work *work)
{
    VERTEX_PARAMETER *verp=work->ver_params;
    int i;

    vu0_Ldm0(&(work->body.objs->world));

    for(i=work->num_vers;i>0;i--,verp++){
	vu0_Ldv0(&(verp->lbase));
	vu0_Mulv0m0v0();
	fpu_ClearVector(&(verp->dv));
	fpu_ClearVector(&(verp->normal));
	vu0_Stv0(&(verp->base));

	if(work->flag){
	    fpu_CopyVector(&(verp->now),&(verp->base));
	}
    }

    work->flag=0;
}

#else

/* 頂点の基準位置の計算 */
static inline void InitBasicVertex(Work *work)
{
    VERTEX_PARAMETER *from_verp=work->ver_params;
    VERTEX_PARAMETER *to_verp=work->ver_params;
    VERTEX_PARAMETER *tverp;
    int i,j;

    sceDmaTag (* const scratchpad)[4][(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4]
	=(sceDmaTag (*)[4][(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4])FREEAREA_ON_SCRATCHPAD;
    int from_which=0,to_which=0;
    int cnt=0;


    SyncDCache(from_verp,from_verp+work->num_vers);

    toSPRDmaSendN(from_verp,(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4,
		  &((*scratchpad)[from_which][0]));

    vu0_Ldm0(&(work->body.objs->world));

    for(i=work->num_vers,j=work->num_vers-16;i>0;i--,tverp++,cnt=(cnt+1)&(16-1)){

	if(cnt==0){
	    tverp=(VERTEX_PARAMETER *)&((*scratchpad)[from_which][0]);

	    if(j>0){
		from_which=(from_which+1)&3;
		from_verp+=16;
		j-=16;

		toSPRDmaSendN(from_verp,(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4,
			      &((*scratchpad)[from_which][0]));
	    }
	}

	vu0_Ldv0(&(tverp->lbase));
	vu0_Mulv0m0v0();
	fpu_ClearVector(&(tverp->dv));
	fpu_ClearVector(&(tverp->normal));
	vu0_Stv0(&(tverp->base));

	if(work->flag){
	    fpu_CopyVector(&(tverp->now),&(tverp->base));
	}

	if(cnt==(16-1)){
	    fromSPRDmaSendN(&((*scratchpad)[to_which][0]),
			    (sizeof(VERTEX_PARAMETER)*16+(16-1))>>4,to_verp);

	    to_which=(to_which+1)&3;
	    to_verp+=16;
	}
    }

    if(cnt!=0){
	fromSPRDmaSendN(&((*scratchpad)[to_which][0]),
			(sizeof(VERTEX_PARAMETER)*cnt+(16-1))>>4,to_verp);

	//fromSPRDmaSync();
    }

    work->flag=0;
}

#endif




#ifdef _XBOX

/* 法線の計算 */
/* 風の計算 */
static inline void CalcPolygonNormalAndAirForce(Work *work)
{
    VERTEX_PARAMETER *verp=work->ver_params;
    POLY_PARAMETER *polyp=work->poly_params;
    FVECTOR *wind=&G_wind;
    float pa2=work->model_param.pa;
    int i;

    for(i=work->num_polys;i>0;i--,polyp++){
	float l,l2;
	VERTEX_PARAMETER *verp0=verp+polyp->index[0];
	VERTEX_PARAMETER *verp1=verp+polyp->index[1];
	VERTEX_PARAMETER *verp2=verp+polyp->index[2];

	vu0_Ldv1(&(verp1->now));
	vu0_Ldv0(&(verp0->now));
	vu0_Ldv2(&(verp2->now));
	vu0_Subv0v1v0();
	vu0_Subv2v1();

	vu0_Ldv1(&(verp0->normal));

	vu0_OuterProductv0v2();

	vu0_Ldv2(&(verp1->normal));

	l=vu0_VectorLength2v0();
	l=fpu_Rsqrt(l,polyp->normal_sign);

	vu0_Mulv0a(l);

	vu0_Addv1v0();

	//vu0_Stv0(&(polyp->normal));

	vu0_Stv1(&(verp0->normal));
	vu0_Ldv1(&(verp2->normal));

	vu0_Addv2v0();
	vu0_Addv1v0();

	vu0_Stv2(&(verp1->normal));
	vu0_Stv1(&(verp2->normal));


#if 0
	vu0_Ldv0(&(verp0->v));
	vu0_Ldv1(&(verp1->v));
	vu0_Ldv2(&(verp2->v));
	vu0_Addv0v1();
	vu0_Ldv1(wind);
	vu0_Addv0v2();
	vu0_Mulv0a(1.0f/3.0f);
	vu0_Subv0v1();
#else
	vu0_Ldv2(wind);
	vu0_Clrv1();
	vu0_Subv1v2();
#endif
	//vu0_Ldv0(&(polyp->normal));

	l=vu0_InnerProductv0v1();
	l2=vu0_VectorLength2v1();

	l*=-pa2;
#if 0
	vu0_RsqrtQ(l2,l);
#else
	l=fpu_Rsqrt(l2,l);
#endif
	vu0_Ldv2(&(verp0->dv));
	vu0_Mulv1a(l);
	vu0_Ldv0(&(verp1->dv));
	vu0_Addv2v1();
	vu0_Addv0v1();
	vu0_Stv2(&(verp0->dv));
	vu0_Ldv2(&(verp2->dv));
	vu0_Stv0(&(verp1->dv));
	vu0_Addv2v1();
	vu0_Stv2(&(verp2->dv));
    }
}

#else

/* 法線の計算 */
/* 風の計算 */
static inline void CalcPolygonNormalAndAirForce(Work *work)
{
    VERTEX_PARAMETER *verp=work->ver_params;
    POLY_PARAMETER *polyp=work->poly_params;
    POLY_PARAMETER *tpolyp;
    FVECTOR *wind=&G_wind;
    float pa2=work->model_param.pa;
    int i;

    sceDmaTag (* const scratchpad)[2][(sizeof(POLY_PARAMETER)*16+(16-1))>>4]
	=(sceDmaTag (*)[2][(sizeof(POLY_PARAMETER)*16+(16-1))>>4])FREEAREA_ON_SCRATCHPAD;
    int to_which=0;
    int cnt=0;


    toSPRDmaSendN(polyp,(sizeof(POLY_PARAMETER)*16+(16-1))>>4,&((*scratchpad)[to_which][0]));

    for(i=work->num_polys;i>0;i--,tpolyp++,cnt=(cnt+1)&(16-1)){
	float l,l2;
	VERTEX_PARAMETER *verp0;
	VERTEX_PARAMETER *verp1;
	VERTEX_PARAMETER *verp2;

	if(cnt==0){
	    tpolyp=(POLY_PARAMETER *)&((*scratchpad)[to_which][0]);

	    to_which^=1;
	    polyp+=16;

	    toSPRDmaSendN(polyp,(sizeof(POLY_PARAMETER)*16+(16-1))>>4,&((*scratchpad)[to_which][0]));
	}

	verp0=verp+tpolyp->index[0];
	verp1=verp+tpolyp->index[1];
	verp2=verp+tpolyp->index[2];

	vu0_Ldv1(&(verp1->now));
	vu0_Ldv0(&(verp0->now));
	vu0_Ldv2(&(verp2->now));
	vu0_Subv0v1v0();
	vu0_Subv2v1();

	vu0_Ldv1(&(verp0->normal));

	vu0_OuterProductv0v2();

	vu0_Ldv2(&(verp1->normal));

	l=vu0_VectorLength2v0();
	l=fpu_Rsqrt(l,tpolyp->normal_sign);

	vu0_Mulv0a(l);

	vu0_Addv1v0();

	//vu0_Stv0(&(polyp->normal));

	vu0_Stv1(&(verp0->normal));
	vu0_Ldv1(&(verp2->normal));

	vu0_Addv2v0();
	vu0_Addv1v0();

	vu0_Stv2(&(verp1->normal));
	vu0_Stv1(&(verp2->normal));


#if 0
	vu0_Ldv0(&(verp0->v));
	vu0_Ldv1(&(verp1->v));
	vu0_Ldv2(&(verp2->v));
	vu0_Addv0v1();
	vu0_Ldv1(wind);
	vu0_Addv0v2();
	vu0_Mulv0a(1.0f/3.0f);
	vu0_Subv0v1();
#else
	vu0_Ldv2(wind);
	vu0_Clrv1();
	vu0_Subv1v2();
#endif
	//vu0_Ldv0(&(polyp->normal));

	l=vu0_InnerProductv0v1();
	l2=vu0_VectorLength2v1();

	l*=-pa2;
#if 0
	vu0_RsqrtQ(l2,l);
#else
	l=fpu_Rsqrt(l2,l);
#endif

	vu0_Mulv1a(l);

	asm volatile ("
	lqc2		vf28,0(%0)
	lqc2		vf29,0(%1)
	lqc2		vf30,0(%2)
	qmtc2		%3,vf24
	qmtc2		%4,vf25
	qmtc2		%5,vf26
	vmulx.xyz	vf24,vf2,vf24x
	vmulx.xyz	vf25,vf2,vf25x
	vmulx.xyz	vf26,vf2,vf26x
	vadd.xyz	vf28,vf28,vf24
	vadd.xyz	vf29,vf29,vf25
	vadd.xyz	vf30,vf30,vf26
	sqc2		vf28,0(%0)
	sqc2		vf29,0(%1)
	sqc2		vf30,0(%2)
	" : : "r"(&(verp0->dv)),"r"(&(verp1->dv)),"r"(&(verp2->dv)),
	"r"(verp0->inv_n),"r"(verp1->inv_n),"r"(verp2->inv_n) : "memory" );
    }
}

#endif




#if 0

static inline void CalcSpringForce(Work *work)
{
    LINE_PARAMETER *linep=work->line_params;
    VERTEX_PARAMETER *verp=work->ver_params;
    float spring_u=work->model_param.spring_u;
    int i;

    for(i=work->num_lines;i>0;i--,linep++){
	VERTEX_PARAMETER *verp0=verp+linep->index[0];
	VERTEX_PARAMETER *verp1=verp+linep->index[1];
	float l;

	vu0_Ldv1(&(verp1->now));
	vu0_Ldv2(&(verp0->now));

	vu0_Subv1v2();
	l=vu0_VectorLength2v1();

#if 0
	vu0_RsqrtQ(l,linep->l0);
	vu0_Mulv2v1Q();
	vu0_Subv0v1v2();

	vu0_Ldv1(&(verp0->dv));
	vu0_Mulv0a(spring_u);
	vu0_Ldv2(&(verp1->dv));
	vu0_Addv1v0();
	vu0_Subv2v0();
	vu0_Stv1(&(verp0->dv));
	vu0_Stv2(&(verp1->dv));
#else
	/* VUのRSQRT命令が使えなかったのでこうなった */

	l=fpu_Rsqrt(l,linep->l0);
	vu0_Mulv2v1a(l);
	vu0_Subv0v1v2();

	vu0_Ldv1(&(verp0->dv));
	vu0_Mulv0a(spring_u);
	vu0_Ldv2(&(verp1->dv));
	vu0_Addv1v0();
	vu0_Subv2v0();
	vu0_Stv1(&(verp0->dv));
	vu0_Stv2(&(verp1->dv));
#endif
    }
}

#else

static inline void CalcSpringForce(Work *work)
{
    LINE_PARAMETER *linep=work->line_params;
    LINE_PARAMETER *tlinep;
    VERTEX_PARAMETER *verp=work->ver_params;
    float spring_u=work->model_param.spring_u;
    int i;

    sceDmaTag (* const scratchpad)[2][(sizeof(LINE_PARAMETER)*16+(16-1))>>4]
	=(sceDmaTag (*)[2][(sizeof(LINE_PARAMETER)*16+(16-1))>>4])FREEAREA_ON_SCRATCHPAD;
    int to_which=0;
    int cnt=0;


    toSPRDmaSendN(linep,(sizeof(LINE_PARAMETER)*16+(16-1))>>4,&((*scratchpad)[to_which][0]));

    for(i=work->num_lines;i>0;i--,tlinep++,cnt=(cnt+1)&(16-1)){
	VERTEX_PARAMETER *verp0;
	VERTEX_PARAMETER *verp1;
	float l;

	if(cnt==0){
	    tlinep=(LINE_PARAMETER *)&((*scratchpad)[to_which][0]);

	    to_which^=1;
	    linep+=16;

	    toSPRDmaSendN(linep,(sizeof(LINE_PARAMETER)*16+(16-1))>>4,&((*scratchpad)[to_which][0]));
	}

	verp0=verp+tlinep->index[0];
	verp1=verp+tlinep->index[1];

	vu0_Ldv1(&(verp1->now));
	vu0_Ldv2(&(verp0->now));

	vu0_Subv1v2();
	l=vu0_VectorLength2v1();

#if 0
	vu0_RsqrtQ(l,tlinep->l0);
	vu0_Mulv2v1Q();
	vu0_Subv0v1v2();

	vu0_Ldv1(&(verp0->dv));
	vu0_Mulv0a(spring_u);
	vu0_Ldv2(&(verp1->dv));
	vu0_Addv1v0();
	vu0_Subv2v0();
	vu0_Stv1(&(verp0->dv));
	vu0_Stv2(&(verp1->dv));
#else
	/* VUのRSQRT命令が使えなかったのでこうなった */

	l=fpu_Rsqrt(l,tlinep->l0);
	vu0_Mulv2v1a(l);
	vu0_Subv0v1v2();

	vu0_Ldv1(&(verp0->dv));
	vu0_Mulv0a(spring_u);
	vu0_Ldv2(&(verp1->dv));
	vu0_Addv1v0();
	vu0_Subv2v0();
	vu0_Stv1(&(verp0->dv));
	vu0_Stv2(&(verp1->dv));
#endif
    }
}

#endif


#if 0

static inline float calc_parameter(float s,float min,float max,float pmin,float pmax)
{
    if(s<min) return pmin;
    else if(s<max) return pmin+(pmax-pmin)*(s-min)/(max-min);
    else return pmax;
}

/* そのフレームでの規定位置と現在位置の関係から、
   速度等に変更を加える。*/
/* 頂点データの更新 */
static inline void CalcBasedStandardPointAndUpdateVertexAndComvert2Local(Work *work)
{
    MODEL_PARAMETER *mdlp=&(work->model_param);
    VERTEX_PARAMETER *verp=work->ver_params;
    KV_PARAMETER *kv=mdlp->kv;
    float m_xlimit=mdlp->xlimit;
    OBJECT *object=&(work->body);
    FMATRIX m;
    int i;

    /* ワールド座標系からローカル座標系へ */
#if 1
    InverseMatrix(&m,&(object->objs->world));
#else
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&(object->objs->world));
#endif

#if 0
    printf("%f %f %f %f     %f %f %f %f\n",
	   object->objs->world.m[0][0],
	   object->objs->world.m[1][0],
	   object->objs->world.m[2][0],
	   object->objs->world.m[3][0],
	   m.m[0][0],
	   m.m[1][0],
	   m.m[2][0],
	   m.m[3][0]);
    printf("%f %f %f %f --> %f %f %f %f\n",
	   object->objs->world.m[0][1],
	   object->objs->world.m[1][1],
	   object->objs->world.m[2][1],
	   object->objs->world.m[3][1],
	   m.m[0][1],
	   m.m[1][1],
	   m.m[2][1],
	   m.m[3][1]);
    printf("%f %f %f %f --> %f %f %f %f\n",
	   object->objs->world.m[0][2],
	   object->objs->world.m[1][2],
	   object->objs->world.m[2][2],
	   object->objs->world.m[3][2],
	   m.m[0][2],
	   m.m[1][2],
	   m.m[2][2],
	   m.m[3][2]);
    printf("%f %f %f %f     %f %f %f %f\n",
	   object->objs->world.m[0][3],
	   object->objs->world.m[1][3],
	   object->objs->world.m[2][3],
	   object->objs->world.m[3][3],
	   m.m[0][3],
	   m.m[1][3],
	   m.m[2][3],
	   m.m[3][3]);
#endif

    vu0_Ldm0(&m);


    for(i=work->num_vers;i>0;i--,verp++){
	FVECTOR *sp=&(verp->base);
	FVECTOR *now=&(verp->now);
	FVECTOR *v=&(verp->v);
	FVECTOR *dv=&(verp->dv);
	int type=verp->type & 0xff;
	float l;
	float xlimit=m_xlimit*verp->fnest;

	vu0_Ldv0(now);
	vu0_Ldv1(sp);
	vu0_Subv2v0v1();
	l=vu0_VectorLength2v2();
	l=fpu_Sqrt(l);

	if(l>xlimit){
	    vu0_Mulv2a(xlimit/l);
	    vu0_Addv0v1v2();
	    l=xlimit;
	    vu0_Stv0(now);
	}

	vu0_Mulv2a(kv[type].recover_u/xlimit);
	vu0_Ldv1(dv);
	vu0_Subv1v2();

	vu0_Ldv0(v);

	l=calc_parameter(l,0.0f,xlimit,kv[type].min,kv[type].max);

	vu0_Mulv2v0a(l);

	l=vu0_VectorLength2v0();

	vu0_Subv1v2();

	l=fpu_Rsqrt(l,xlimit);
	if(l<1.0f){
	    vu0_Mulv0a(l);
	}

	l=vu0_VectorLength2v1();

	vu0_Stv0(v);

	l=fpu_Rsqrt(l,xlimit);
	if(l<1.0f){
	    vu0_Mulv1a(l);
	}

	vu0_Stv1(dv);

	switch(verp->type){
	case VERTEX_TYPE_STATIC:
	    vu0_Ldv0(sp);
	    fpu_ClearVector(v);
	    vu0_Stv0(now);
	    break;
	default:
	    vu0_Ldv0(now);
	    vu0_Ldv1(v);
	    vu0_Setv0w1();
	    vu0_Ldv2(dv);
	    vu0_Addv0v1();
	    vu0_Addv1v2();
	    vu0_Stv0(now);
	    vu0_Stv1(v);
	    break;
	}


	/* ローカル座標系に変換 */
	vu0_Ldv1(&(verp->normal));
	//vu0_Ldv0(now);

	/* 法線の正規化も同時に行なう */
	l=vu0_VectorLength2v1();
	vu0_Setv1w0();
	l=fpu_Rsqrt(l,1.0f);
	vu0_Mulv1a(l);

	vu0_Mulv0m0v0();
	vu0_Mulv1m0v1();

	vu0_Stv0(&(verp->lnow));
	vu0_Stv1(&(verp->lnormal));
    }
}

#else

/* そのフレームでの規定位置と現在位置の関係から、
   速度等に変更を加える。*/
/* 頂点データの更新 */
static inline void CalcBasedStandardPointAndUpdateVertexAndComvert2Local(Work *work)
{
    MODEL_PARAMETER *mdlp=&(work->model_param);
    VERTEX_PARAMETER *from_verp=work->ver_params;
    VERTEX_PARAMETER *to_verp=work->ver_params;
    VERTEX_PARAMETER *tverp;
    float m_xlimit=mdlp->xlimit;
    OBJECT *object=&(work->body);
    FMATRIX m;
    int i,j;

    sceDmaTag (* const scratchpad)[4][(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4]
	=(sceDmaTag (*)[4][(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4])FREEAREA_ON_SCRATCHPAD;
    int from_which=0,to_which=0;
    int cnt=0;


    SyncDCache(from_verp,from_verp+work->num_vers);

    toSPRDmaSendN(from_verp,(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4,&((*scratchpad)[from_which][0]));

    /* ワールド座標系からローカル座標系へ */
#if 0
    InverseMatrix(&m,&(object->objs->world));
#else
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&(object->objs->world));
#endif

#if 0
    printf("%f %f %f %f     %f %f %f %f\n",
	   object->objs->world.m[0][0],
	   object->objs->world.m[1][0],
	   object->objs->world.m[2][0],
	   object->objs->world.m[3][0],
	   m.m[0][0],
	   m.m[1][0],
	   m.m[2][0],
	   m.m[3][0]);
    printf("%f %f %f %f --> %f %f %f %f\n",
	   object->objs->world.m[0][1],
	   object->objs->world.m[1][1],
	   object->objs->world.m[2][1],
	   object->objs->world.m[3][1],
	   m.m[0][1],
	   m.m[1][1],
	   m.m[2][1],
	   m.m[3][1]);
    printf("%f %f %f %f --> %f %f %f %f\n",
	   object->objs->world.m[0][2],
	   object->objs->world.m[1][2],
	   object->objs->world.m[2][2],
	   object->objs->world.m[3][2],
	   m.m[0][2],
	   m.m[1][2],
	   m.m[2][2],
	   m.m[3][2]);
    printf("%f %f %f %f     %f %f %f %f\n\n",
	   object->objs->world.m[0][3],
	   object->objs->world.m[1][3],
	   object->objs->world.m[2][3],
	   object->objs->world.m[3][3],
	   m.m[0][3],
	   m.m[1][3],
	   m.m[2][3],
	   m.m[3][3]);
#endif

    vu0_Ldm0(&m);

    for(i=work->num_vers,j=work->num_vers-16;i>0;i--,tverp++,cnt=(cnt+1)&(16-1)){
	float l;
	float xlimit;

	if(cnt==0){
	    tverp=(VERTEX_PARAMETER *)&((*scratchpad)[from_which][0]);

	    if(j>0){
		from_which=(from_which+1)&3;
		from_verp+=16;
		j-=16;

		toSPRDmaSendN(from_verp,(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4,
			      &((*scratchpad)[from_which][0]));
	    }
	}

	xlimit=m_xlimit*tverp->fnest;

	vu0_Ldv1(&(tverp->v));
	vu0_Ldv2(&(tverp->dv));
	vu0_Mulv1a(mdlp->k);

	vu0_Ldv0(&(tverp->now));

	vu0_Addv2v1();

	vu0_Ldv1(&(tverp->base));
	vu0_Subv1v0();
	vu0_Mulv1a(mdlp->recover_u);
	vu0_Setv0w1();
	vu0_Addv2v1();
	vu0_Ldv1(&(tverp->v));

	switch(tverp->type){
	case VERTEX_TYPE_STATIC:
	    vu0_Ldv2(&(tverp->base));
	    fpu_ClearVector(&(tverp->v));
	    vu0_Cpv2v0();
	    break;
	default:
	    vu0_Addv1v2();
	    vu0_Ldv2(&(tverp->base));
	    vu0_Addv0v1();
	    vu0_Stv1(&(tverp->v));
	    break;
	}

	vu0_Subv1v0v2();

	l=vu0_VectorLength2v1();
	l=fpu_Rsqrt(l,xlimit);

	if(l<0.95f){
	    vu0_Mulv1a(l);
	    vu0_Addv0v1v2();
	    vu0_Ldv1(&(tverp->now));
	    vu0_Subv1v0v1();
	    vu0_Stv1(&(tverp->v));
	}

#if 0
	vu0_Ldv1(&(tverp->v));

	l=vu0_VectorLength2v1();
	l=fpu_Rsqrt(l,xlimit/5);

	if(l<0.95f){
	    vu0_Mulv1a(l);
	    vu0_Stv1(&(tverp->v));
	}

#endif

	/* 当たり判定を入れる予定 */


	/* ローカル座標系に変換 */
	vu0_Ldv1(&(tverp->normal));
	vu0_Stv0(&(tverp->now));

	/* 法線の正規化も同時に行なう */
	l=vu0_VectorLength2v1();
	vu0_Setv1w0();
	l=fpu_Rsqrt(l,1.0f);
	vu0_Mulv1a(l);

	vu0_Mulv0m0v0();
	vu0_Mulv1m0v1();

	vu0_Stv0(&(tverp->lnow));
	vu0_Stv1(&(tverp->lnormal));

	if(cnt==(16-1)){
	    fromSPRDmaSendN(&((*scratchpad)[to_which][0]),(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4,
			    to_verp);

	    to_which=(to_which+1)&3;
	    to_verp+=16;
	}
    }

    if(cnt!=0){
	fromSPRDmaSendN(&((*scratchpad)[to_which][0]),(sizeof(VERTEX_PARAMETER)*16+(16-1))>>4,
			to_verp);

	fromSPRDmaSync();
    }
}

#endif



#if 1

/* 頂点データを表示用バッファにコピー */
static inline void CopyVertecies2DBuff(Work *work)
{
    VERTEX_PARAMETER *verp=work->ver_params;
    OBJECT *object=&(work->body);
    int i;

    SyncDCache(verp,verp+work->num_vers);

    for(i=0;i<object->objs->objs->n_packs;i++){
	SVECTOR *vertexm=(SVECTOR *)((object->objs->objs->packs+i)->verts);
	SVECTOR *normalm=(SVECTOR *)((object->objs->objs->packs+i)->norms);
	SVECTOR *vertex=*(work->vertex[DG_Clock]+i);
	SVECTOR *normal=*(work->normal[DG_Clock]+i);
	int *index=*(work->array_indexes_vertex+i);
	int j;

	for(j=(object->objs->objs->packs+i)->n_verts;j>0;
	    j--,vertex++,normal++,vertexm++,normalm++,index++){

	    vu0_Ldv0(&((verp+*index)->lnow));
	    vu0_Ldv1(&((verp+*index)->lnormal));
	    vu0_FVtoIV0v0();
	    vu0_FVtoIV12v1();

#if 1
	    vu0_StSVv0(vertex);
	    vu0_StSVv1(normal);
#else
	    asm volatile ("
	    qmfc2.i	$8,vf1
	    qmfc2.i	$9,vf2
	    ppach	$8,$0,$8
	    ppach	$9,$0,$9
            sd		$8,(%0)
            sd		$9,(%1)
            " : : "r"(vertex),"r"(normal) : "$8","$9","memory" );

	    vertex->pad=vertexm->pad;
	    normal->pad=normalm->pad;
#endif
	}
    }
}

#else

/* 頂点データを表示用バッファにコピー */
static void CopyVertecies2DBuff(Work *work)
{
    VERTEX_PARAMETER *verp=work->ver_params;
    OBJECT *object=&(work->body);
    int i;

    sceDmaTag (* const scratchpad)[2][256]=(sceDmaTag (*)[2][256])FREEAREA_ON_SCRATCHPAD;
    int to_which=0;

    for(i=0;i<object->objs->objs->n_packs;i++){
	int size=((object->objs->objs->packs+i)->n_verts+1)>>1;
	SVECTOR *vertexm=(SVECTOR *)((object->objs->objs->packs+i)->verts);
	SVECTOR *normalm=(SVECTOR *)((object->objs->objs->packs+i)->norms);
	SVECTOR *vertex=(SVECTOR *)&((*scratchpad)[to_which][0]);
	SVECTOR *normal=(SVECTOR *)&((*scratchpad)[to_which][1+size]);
	int *index=*(work->array_indexes_vertex+i);
	int j;

	for(j=(object->objs->objs->packs+i)->n_verts;j>0;
	    j--,vertex++,normal++,vertexm++,normalm++,index++){

	    vu0_Ldv0(&((verp+*index)->lnow));
	    vu0_Ldv1(&((verp+*index)->lnormal));
	    vu0_FVtoIV0v0();
	    vu0_FVtoIV12v1();
#if 1
	    vu0_StSVv0(vertex);
	    vu0_StSVv1(normal);
#else
	    asm volatile ("
	    qmfc2.i	$8,vf1
	    qmfc2.i	$9,vf2
	    ppach	$8,$0,$8
	    ppach	$9,$0,$9
            sd		$8,(%0)
            sd		$9,(%1)
            " : : "r"(vertex),"r"(normal) :
	    "$8","$9","memory" );
#endif
	    vertex->pad=vertexm->pad;
	    normal->pad=normalm->pad;
	}

	setDmaTag(&((*scratchpad)[to_which][size]),0x70,
		  *(work->normal[DG_Clock]+i),size);

	{
	    volatile sceDmaChan *d=sceDmaGetChan(SCE_DMA_fromSPR);
	    tD_CHCR chcr;

	    while(sceDmaSync(d,1,0));

	    d->sadr=&((*scratchpad)[to_which][0]);
	    d->madr=*(work->vertex[DG_Clock]+i);
	    d->qwc=size;

	    chcr=d->chcr;
	    chcr.MOD=1,chcr.STR=1;
	    d->chcr=chcr;
	}

	to_which^=1;
    }
}

#endif


static void MoveCloth(Work *work)
{
#ifdef L_DEBUG_MODE
    int cnt;
#endif


#ifdef L_DEBUG_MODE
    GV_SET_PRFC_CLOCK();
#endif

    /* 頂点のパラメータの初期化 */
    InitBasicVertex(work);

#ifdef L_DEBUG_MODE
    cnt=GV_GET_PRFC_CLOCK();
    if(GV_PadData[0].press & PAD_SEL){
	printf("InitBasicVertex            : %d\n",cnt);
    }
#endif



#ifdef L_DEBUG_MODE
    GV_SET_PRFC_CLOCK();
#endif

    /* 法線の計算 */
    CalcPolygonNormalAndAirForce(work);

#ifdef L_DEBUG_MODE
    cnt=GV_GET_PRFC_CLOCK();
    if(GV_PadData[0].press & PAD_SEL){
	printf("CalcPolygonNormalAndAirForce : %d\n",cnt);
    }
#endif



#ifdef L_DEBUG_MODE
    GV_SET_PRFC_CLOCK();
#endif

    /* ばねによる内力計算 */
    CalcSpringForce(work);

#ifdef L_DEBUG_MODE
    cnt=GV_GET_PRFC_CLOCK();
    if(GV_PadData[0].press & PAD_SEL){
	printf("CalcSpringForce            : %d\n",cnt);
    }
#endif



#ifdef L_DEBUG_MODE
    GV_SET_PRFC_CLOCK();
#endif

    /* 基準位置を使った補正 */
    CalcBasedStandardPointAndUpdateVertexAndComvert2Local(work);

#ifdef L_DEBUG_MODE
    cnt=GV_GET_PRFC_CLOCK();
    if(GV_PadData[0].press & PAD_SEL){
	printf("CalcBasedStandardPointAndUpdateVertexAndComvert2Local : %d\n",cnt);
    }
#endif


#ifdef L_DEBUG_MODE
    GV_SET_PRFC_CLOCK();
#endif

    /* 頂点データを表示用バッファにコピー */
    CopyVertecies2DBuff(work);

#ifdef L_DEBUG_MODE
    cnt=GV_GET_PRFC_CLOCK();
    if(GV_PadData[0].press & PAD_SEL){
	printf("CopyVertecies2DBuff        : %d\n",cnt);
    }
#endif

}


#if 0

static void CalcCollision(Work *work)
{
    VERTEX_PARAMETER *verp=work->ver_params;
    int i,j;

    for(i=0;i<work->num_vers;i++,verp++){
	COLLISION_OVAL *oval=coli->ovals;

	for(j=0;j<coli->num_coli;j++,oval++){
	    VECTOR v;
	    float abc;

	    vu0_Ldv0(&(verp->now));
	    vu0_Ldv1(&(oval->center));
	    vu0_Ldv2(&(oval->param));
	    vu0_Subv0v1();
	    vu0_VectorPow2v2();
	    vu0_VectorPow2v0();
	    vu0_MakeOvalParamsv2();
	    abc=vu0_InnerProductv0v2();
	    vu0_Stv2(&v);

	    if(abc<v.w){
		vu0_SqrtQ(v.w/abc);
		vu0_Ldv0(&(verp->now));
		vu0_Subv2v0v1();
		vu0_Mulv2Q();
		vu0_Addv1v2();
		vu0_Subv2v1v0();
		vu0_Ldv0(&(verp->v));
		vu0_Addv2v0();
		vu0_Stv1(&(verp->now));
		vu0_Stv2(&(verp->v));
		break;
	    }
	}
    }
}

#endif
