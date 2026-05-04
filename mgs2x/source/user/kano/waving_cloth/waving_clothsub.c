//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	waving_clothsub.c
		布シミュレート演算サブルーチン

	1999/08/16 K.Kano
	$Id: waving_clothsub.c,v 1.2 2002/11/27 14:38:29 takaki Exp $
*/

#include "waving_cloth.h"


#if 1

#include "nest.c"

#endif


#if __SCRATCHPAD_USE_VIRTUAL_ALLOC__
#define SCRATCHPAD		0x70000000
#else
#include "libgv.h"
#define SCRATCHPAD		SCRPAD_ADDR
#endif

#define FREEAREA_ON_SCRATCHPAD	SCRATCHPAD

#define LNOW_BUF	((SVECTOR *)FREEAREA_ON_SCRATCHPAD)
#define LNORMAL_BUF	((SVECTOR *)FREEAREA_ON_SCRATCHPAD+0x380)
#define INVM_BUF	((FMATRIX *)((int)FREEAREA_ON_SCRATCHPAD+0x4000-0x20*sizeof(FMATRIX)))

#define setDmaTag(t,i,a,q)	(t)->id=(i),(t)->next=(sceDmaTag *)(a),(t)->qwc=(q)


extern FVECTOR	G_wind;
int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );


int InitClothModel(Work *work)
{
    // OBJECT *object=&(work->body);
    // DG_OBJ  *obj=object->objs->objs;
	// DG_OBJ  *obj=work->objs->objs;
    CV2_MDL *cvd;
    // int total_vertex,total_polygon;
    int i;
    // int j,k,n;

    cvd=(*(work->cvd_def+work->cvd_sel))->models;

    work->base=NULL;

    work->lbase_normal=NULL;
    work->base_normal=NULL;

    work->now=NULL;
    work->v=NULL;
    work->dv=NULL;
    work->normal=NULL;


    i=(cvd->n_verts+3)&~3;
    if((work->base=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*i*6))==NULL) return 0;

	GV_ZeroMemory( work->base, sizeof(FVECTOR)*i*6 ) ;
    work->now=work->base+i;
    work->v=work->now+i;
    work->dv=work->v+i;
    work->normal=work->dv+i;
    work->base_normal=work->normal+i;

    work->lbase_normal=cvd->norms;

    /* 正常終了 */
    return 1;
}

void ExitClothModel(Work *work)
{
    if(work->cvd_def!=NULL) GV_Free(work->cvd_def);
    if(work->base!=NULL) GV_Free(work->base);
}


int InitClothModel_withoutDATAFILE(Work *work)
{
#ifndef KP_XBOX	// テスト用に残しておいたらしい。
    // OBJECT *object=&(work->body);
    // DG_OBJ  *obj=object->objs->objs;
	DG_OBJ  *obj=work->objs->objs;
    CV2_MDL *cvd;
    int total_vertex,total_polygon;
    int i,j,k;
    int n;

    cvd=(*(work->cvd_def+work->cvd_sel))->models;

    work->base=NULL;

    work->lbase_normal=NULL;
    work->base_normal=NULL;

    work->now=NULL;
    work->v=NULL;
    work->dv=NULL;
    work->normal=NULL;


	/* ツールで対応 */
    work->ver_params=NULL;
    work->poly_params=NULL;
    work->line_params=NULL;

    /* object->n_models==1に想定 */
    work->num_vers=total_vertex=cvd->n_verts;
    total_polygon=0;

	for(i=0;i<obj->n_packs;i++){
		for(j=0;j<(obj->model->packs+i)->n_verts;j++){
			if(!(*((short *)(obj->model->packs+i)->norms+(j<<2)+3) & 0x8000)){
				total_polygon++;
			}
		}
    }

#if 0
    printf("N Packs = %d\n",obj->n_packs);
    printf("Total vertex = %d\n",total_vertex);
    printf("Total polygon = %d\n",total_polygon);
#endif


    if((work->ver_params
		=(VERTEX_PARAMETER *)GV_Malloc(sizeof(VERTEX_PARAMETER)*total_vertex))==NULL) return 0;


    i=(total_vertex+3)&~3;
    if((work->base=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*i*7))==NULL) return 0;
    work->now=work->base+i;
    work->v=work->now+i;
    work->dv=work->v+i;
    work->normal=work->dv+i;
    work->lbase_normal=work->normal+i;
    work->base_normal=work->lbase_normal+i;


    n=0;
    for(i=0;i<obj->n_packs;i++){
		for(j=0;j<(obj->model->packs+i)->n_verts;j++,n++){
			int v0;

			v0=*(cvd->verts_index+n);
			vu0_LdSVv0((SVECTOR *)((obj->model->packs+i)->norms+(j<<2)));
			vu0_IV12toFVv0();
			vu0_Stv0(work->lbase_normal+v0);
		}
		n=(n+1)&~1;
    }


    if((work->poly_params
		=(POLY_PARAMETER *)GV_Malloc(sizeof(POLY_PARAMETER)*total_polygon))==NULL) return 0;
    work->num_polys=0;

    {
		int *ver_polyn;

		if((ver_polyn=(int *)GV_Malloc(sizeof(int)*total_vertex))==NULL) return 0;
		GV_ZeroMemory(ver_polyn,sizeof(int)*total_vertex);

		n=0;
		for(i=0;i<obj->n_packs;i++){
			int v0,v1,v2;

			v0=*(cvd->verts_index+n); n++;
			v1=*(cvd->verts_index+n); n++;

			for(j=2;j<(obj->model->packs+i)->n_verts;j++,n++){

				v2=*(cvd->verts_index+n);

				if(!(*((short *)(obj->model->packs+i)->norms+(j<<2)+3) & 0x8000)){

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

						vu0_Ldv0(cvd->verts+v0);
						vu0_Ldv1(cvd->verts+v1);
						vu0_Ldv2(cvd->verts+v2);
						vu0_Subv0v1v0();
						vu0_Subv2v1();

						vu0_LdSVv1((SVECTOR *)((obj->model->packs+i)->norms+(j<<2)));

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

						(*(ver_polyn+v0))++;
						(*(ver_polyn+v1))++;
						(*(ver_polyn+v2))++;

						work->num_polys++;
					}
				}
				v0=v1; v1=v2;
			}
			n=(n+1)&~1;
		}
		for(i=0;i<total_vertex;i++){
			(work->ver_params+i)->inv_n=1.0f/(float)(*(ver_polyn+i));
		}
		GV_Free(ver_polyn);
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

			vu0_Ldv0(cvd->verts+vmin);
			vu0_Ldv1(cvd->verts+vmax);

			(work->line_params+j)->index[0]=vmin;

			vu0_Subv0v1();

			(work->line_params+j)->index[1]=vmax;

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

			vu0_Ldv0(cvd->verts+vmin);
			vu0_Ldv1(cvd->verts+vmax);

			(work->line_params+j)->index[0]=vmin;

			vu0_Subv0v1();

			(work->line_params+j)->index[1]=vmax;

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

			vu0_Ldv0(cvd->verts+vmin);
			vu0_Ldv1(cvd->verts+vmax);

			(work->line_params+j)->index[0]=vmin;

			vu0_Subv0v1();

			(work->line_params+j)->index[1]=vmax;

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

    set_fnest(work);

#endif
    /* 正常終了 */
    return 1;
}

void ExitClothModel_withoutDATAFILE(Work *work)
{
    if(work->cvd_def!=NULL) GV_Free(work->cvd_def);
    if(work->poly_params!=NULL) GV_Free(work->poly_params);
    if(work->ver_params!=NULL) GV_Free(work->ver_params);
    if(work->line_params!=NULL) GV_Free(work->line_params);
    if(work->base!=NULL) GV_Free(work->base);
}



#if 0

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

#endif



void WavingCloth_CalcBase(Work *work,FMATRIX *m)
{
    FVECTOR *base=work->base;
    FVECTOR *now=work->now;
    FVECTOR *lbase_normal=work->lbase_normal;
    FVECTOR *base_normal=work->base_normal;
    CV2_MDL *mdl;
    FVECTOR *lbase;
    int i;

    mdl=(*(work->cvd_def+work->cvd_sel))->models;
    lbase=mdl->verts;

    vu0_Ldm0(m);

#if 0
	printf("%d 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n",
		   i,(int)lbase,(int)base,(int)now,(int)lbase_normal,(int)base_normal);
#endif

    for(i=work->num_vers;i>0;i-=2,lbase+=2,base+=2,now+=2,lbase_normal+=2,base_normal+=2){
#ifdef BP_PSX2_ASM
		asm volatile ("
		lqc2		vf16,0x00(%0)
		lqc2		vf17,0x10(%0)
		lqc2		vf18,0x00(%3)
		lqc2		vf19,0x10(%3)
		pref		0,0x20(%0)
		vmove.w		vf16,vf0
		vmove.w		vf17,vf0
		vmulx.w		vf18,vf18,vf0
		vmulx.w		vf19,vf19,vf0
		vmulax.xyzw	ACC,vf4,vf16x
		vmadday.xyzw	ACC,vf5,vf16y
		vmaddaz.xyzw	ACC,vf6,vf16z
		vmaddw.xyzw	vf16,vf7,vf16w
		pref		0,0x20(%3)
		vmulax.xyzw	ACC,vf4,vf17x
		vmadday.xyzw	ACC,vf5,vf17y
		vmaddaz.xyzw	ACC,vf6,vf17z
		vmaddw.xyzw	vf17,vf7,vf17w
		vmulax.xyzw	ACC,vf4,vf18x
		vmadday.xyzw	ACC,vf5,vf18y
		vmaddaz.xyzw	ACC,vf6,vf18z
		vmaddw.xyzw	vf18,vf7,vf18w
		vmulax.xyzw	ACC,vf4,vf19x
		vmadday.xyzw	ACC,vf5,vf19y
		vmaddaz.xyzw	ACC,vf6,vf19z
		vmaddw.xyzw	vf19,vf7,vf19w
		sqc2		vf16,0x00(%1)
		sqc2		vf17,0x10(%1)
		sqc2		vf16,0x00(%2)
		sqc2		vf17,0x10(%2)
		sqc2		vf18,0x00(%4)
		sqc2		vf19,0x10(%4)
		" : : "r"(lbase),"r"(base),"r"(now),"r"(lbase_normal),"r"(base_normal) : "memory" );
#else
		vu0_ApplyMatrixXYZ( base  , vu0_Ptrm0(), lbase   ) ;
		vu0_ApplyMatrixXYZ( base+1, vu0_Ptrm0(), lbase+1 ) ;
		_sceVu0CopyVector( now  , base   ) ;
		_sceVu0CopyVector( now+1, base+1 ) ;
		vu0_ApplyMatrixXYZ( base_normal  , vu0_Ptrm0(), lbase_normal   ) ;
		vu0_ApplyMatrixXYZ( base_normal+1, vu0_Ptrm0(), lbase_normal+1 ) ;
#endif

    }
}


/* 頂点の基準位置の計算 */
static inline void InitBasicVertex(Work *work)
{
    FVECTOR *normal=work->normal;
    FVECTOR *dv=work->dv;
    int i;

    for(i=work->num_vers;i>0;i--,dv++,normal++){
		fpu_ClearVector((FVECTOR *)GV_UNCACHEA(dv));
		fpu_ClearVector((FVECTOR *)GV_UNCACHEA(normal));
    }
}


#if 0

/* 法線の計算 */
/* 風の計算 */
static inline void CalcPolygonNormalAndAirForce(Work *work)
{
    POLY_PARAMETER *polyp=work->poly_params;
    FVECTOR *wind=&G_wind;
    FVECTOR *now=work->now;
    FVECTOR *normal=work->normal;
    FVECTOR *dv=work->dv;
    float pa2=work->model_param.pa;
    int i;

    for(i=work->num_polys;i>0;i--,polyp++){
		float l,l2;
		int v0,v1,v2;

		v0=polyp->index[0];
		v1=polyp->index[1];
		v2=polyp->index[2];

		GV_PREFECH(polyp+1);

		vu0_Ldv1(now+v0);
		vu0_Ldv0(now+v1);
		vu0_Ldv2(now+v2);
		vu0_Subv0v1v0();
		vu0_Subv2v1();

		vu0_Ldv1(normal+v0);

		vu0_OuterProductv0v2();

		vu0_Ldv2(normal+v1);

		l=vu0_VectorLength2v0();
		l=fpu_Rsqrt(l,polyp->normal_sign);

		vu0_Mulv0a(l);

		vu0_Addv1v0();

		vu0_Stv1(normal+v0);
		vu0_Ldv1(normal+v2);

		vu0_Addv2v0();
		vu0_Addv1v0();

		vu0_Stv2(normal+v1);
		vu0_Stv1(normal+v2);


#if 0
		vu0_Ldv0(v+v0);
		vu0_Ldv1(v+v1);
		vu0_Ldv2(v+v2);
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


		l=vu0_InnerProductv0v1();
		l2=vu0_VectorLength2v1();

		l*=-pa2;
#if 0
		vu0_RsqrtQ(l2,l);
#else
		l=fpu_Rsqrt(l2,l);
#endif
		vu0_Ldv2(dv+v0);
		vu0_Mulv1a(l);
		vu0_Ldv0(dv+v1);
		vu0_Addv2v1();
		vu0_Addv0v1();
		vu0_Stv2(dv+v0);
		vu0_Ldv2(dv+v2);
		vu0_Stv0(dv+v1);
		vu0_Addv2v1();
		vu0_Stv2(dv+v2);
    }
}

#else

/* 法線の計算 */
/* 風の計算 */
static inline void CalcPolygonNormalAndAirForce(Work *work)
{
    POLY_PARAMETER *polyp=work->poly_params;
    FVECTOR *now=work->now;
    FVECTOR *normal=work->normal;
    FVECTOR *dv=work->dv;
    FVECTOR *base_normal=work->base_normal;
    VERTEX_PARAMETER *verp=work->ver_params;
    float pa2=work->model_param.pa;
    int i;
    FVECTOR lwind;
    FVECTOR *wind=&lwind;
#ifndef BP_PSX2_ASM
	FVECTOR _vf16 ;
#endif

    if(!OK_GetLocalWind((FVECTOR *)&(work->objs->world.m[3][0]),&lwind)){
		wind=&G_wind;
    }

    for(i=work->num_polys;i>0;i--,polyp++){
		VERTEX_PARAMETER *verp0,*verp1,*verp2;
		float l,l2;
		int v0,v1,v2;

		v0=polyp->index[0];
		v1=polyp->index[1];
		v2=polyp->index[2];

		GV_PREFECH(polyp+1);

		verp0=verp+v0;
		verp1=verp+v1;
		verp2=verp+v2;

		vu0_Ldv1(now+v1);
		vu0_Ldv0(now+v0);
		vu0_Ldv2(now+v2);
		vu0_Subv0v1v0();
		vu0_Subv2v1();

		vu0_Ldv1(normal+v0);

		vu0_OuterProductv0v2();

		vu0_Ldv2(normal+v1);

		l=vu0_VectorLength2v0();
		l2=fpu_Rsqrt(l,polyp->normal_sign);

#ifdef BP_PSX2_ASM
		asm volatile ("
		qmtc2.ni	%0,vf16
		vmulx.xyz	vf16,vf1,vf16x
		" : : "r"(l2) );

		asm volatile ("vadd.xyz		vf2,vf2,vf16");
#else

		{
			extern FVECTOR REG_vf2, REG_vf1;
			fpu_MulVectorScaler( &_vf16, &REG_vf1, l2 ) ;
			fpu_AddVectors( &REG_vf2, &REG_vf2, &_vf16 ) ;
		}
#endif
		vu0_Stv1(normal+v0);
		vu0_Ldv1(normal+v2);

#ifdef BP_PSX2_ASM
		asm volatile ("vadd.xyz		vf3,vf3,vf16");
		asm volatile ("vadd.xyz		vf2,vf2,vf16");
#else
		{
			extern FVECTOR REG_vf2, REG_vf3 ;
			fpu_AddVectors( &REG_vf2, &REG_vf2, &_vf16 ) ;
			fpu_AddVectors( &REG_vf3, &REG_vf3, &_vf16 ) ;
		}
#endif

		vu0_Stv2(normal+v1);
		vu0_Stv1(normal+v2);


		vu0_Ldv1(wind);


		l=vu0_InnerProductv0v1();
		l2=vu0_VectorLength2v1();

		l=fpu_Abs(l)*pa2;
		l=fpu_Rsqrt(l2,l);

		vu0_Mulv1a(l);


#ifdef BP_PSX2_ASM
		asm volatile ("
		lqc2		vf24,0(%0)
		lqc2		vf25,0(%1)
		lqc2		vf26,0(%2)
		vmul.xyz	vf20,vf2,vf24
		vmul.xyz	vf21,vf2,vf25
		vmul.xyz	vf22,vf2,vf26
		vmulax.w	ACC,vf0,vf20x
		vmadday.w	ACC,vf0,vf20y
		vmaddz.w	vf20,vf0,vf20z
		vmulax.w	ACC,vf0,vf21x
		vmadday.w	ACC,vf0,vf21y
		vmaddz.w	vf21,vf0,vf21z
		vmulax.w	ACC,vf0,vf22x
		vmadday.w	ACC,vf0,vf22y
		vmaddz.w	vf22,vf0,vf22z
		vmulw.xyz	vf20,vf24,vf20w
		vmulw.xyz	vf21,vf25,vf21w
		vmulw.xyz	vf22,vf26,vf22w
		" : : "r"(base_normal+v0),"r"(base_normal+v1),"r"(base_normal+v2) );

		asm volatile ("
		lqc2		vf28,0(%0)
		lqc2		vf29,0(%1)
		lqc2		vf30,0(%2)
		qmtc2		%3,vf24
		qmtc2		%4,vf25
		qmtc2		%5,vf26
		vmulx.xyz	vf24,vf20,vf24x
		vmulx.xyz	vf25,vf21,vf25x
		vmulx.xyz	vf26,vf22,vf26x
		vadd.xyz	vf28,vf28,vf24
		vadd.xyz	vf29,vf29,vf25
		vadd.xyz	vf30,vf30,vf26
		sqc2		vf28,0(%0)
		sqc2		vf29,0(%1)
		sqc2		vf30,0(%2)
		" : : "r"(dv+v0),"r"(dv+v1),"r"(dv+v2),
		"r"(verp0->inv_n),"r"(verp1->inv_n),"r"(verp2->inv_n) : "memory" );
#else
		{
			extern FVECTOR REG_vf2 ;
			FVECTOR _vf24,_vf25,_vf26 ;
			
			fpu_MulVectorScaler( &_vf24, (base_normal+v0),
								fpu_InnerProduct( &REG_vf2, (base_normal+v0) )) ;
			fpu_MulVectorScaler( &_vf25,	(base_normal+v1),
								fpu_InnerProduct( &REG_vf2, (base_normal+v1) )) ;
			fpu_MulVectorScaler( &_vf26,	(base_normal+v2),
								fpu_InnerProduct( &REG_vf2, (base_normal+v2) )) ;
			
			fpu_MulVectorScaler( &_vf24, &_vf24, verp0->inv_n ) ;
			fpu_MulVectorScaler( &_vf25, &_vf25, verp1->inv_n ) ;
			fpu_MulVectorScaler( &_vf26, &_vf26, verp2->inv_n ) ;
			
			fpu_AddVectors( dv+v0, dv+v0, &_vf24 ) ;
			fpu_AddVectors( dv+v1, dv+v1, &_vf25 ) ;
			fpu_AddVectors( dv+v2, dv+v2, &_vf26 ) ;
		}							
#endif
    }
}

#endif




static inline void CalcSpringForce(Work *work)
{
    LINE_PARAMETER *linep=work->line_params;
    FVECTOR *now=work->now;
    FVECTOR *dv=work->dv;
    float spring_u=work->model_param.spring_u;
    int i;

    for(i=work->num_lines;i>0;i--,linep++){
		int v0,v1;
		float l;

		v0=linep->index[0];
		v1=linep->index[1];

		GV_PREFECH(linep+1);

		vu0_Ldv1(now+v1);
		vu0_Ldv2(now+v0);

		vu0_Subv1v2();
		l=vu0_VectorLength2v1();

		/* VUのRSQRT命令が使えなかったのでこうなった */

		l=fpu_Rsqrt(l,linep->l0);
		vu0_Mulv2v1a(l);
		vu0_Subv0v1v2();

		vu0_Ldv1(dv+v0);
		vu0_Mulv0a(spring_u);
		vu0_Ldv2(dv+v1);
		vu0_Addv1v0();
		vu0_Subv2v0();
		vu0_Stv1(dv+v0);
		vu0_Stv2(dv+v1);
    }
}



static void DG_RefineStripNormal2( DG_VERTS_ANIME *v_anime, short *index )
{
#if 1
	DG_OBJPACK	*mdl_pack ;
	DG_MDL		*mdl ;
   SVECTOR		*norm, *buffer ;
	int			i, j, n ;

	mdl = v_anime->obj->model ;
	mdl_pack = mdl->packs ;

	buffer = (SVECTOR*)LNORMAL_BUF ;

	norm = v_anime->norms_top[ v_anime->buffer_clock ];
	v_anime->obj->norms = (SVECTOR *)norm ;

	for ( i = mdl->n_packs ; i > 0 ; i-- ){
		n = mdl_pack->n_verts ;

		for ( j = ( n + 1 ) / 2 ; j > 0 ; j-- ){
         norm[0].vx = buffer[index[0]].vx;
         norm[0].vy = buffer[index[0]].vy;
         norm[0].vz = buffer[index[0]].vz;

         norm[1].vx = buffer[index[1]].vx;
         norm[1].vy = buffer[index[1]].vy;
         norm[1].vz = buffer[index[1]].vz;

			norm += 2 ;
			index += 2 ;
		}
		mdl_pack++ ;
	}
#endif
}


static int _CalcObjsCollisionBWithInvM(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_OBJS *objs,
									   float paramf,FMATRIX *inv_m)
{
    // int size=objs->n_models;
	int size=HUMAN21_ATAMA+1;
    int i;
    int flag=0;

    fpu_CopyVector(r,x);

    for(i=0;i<size;i++){
		flag|=CalcObjCollisionBWithInvM(r,x,base,objs,i,paramf,inv_m+i);
    }

    return flag;
}

static int _CalcObjsCollisionBWithInvM2(FVECTOR *r,FVECTOR *x,FVECTOR *base,DG_DEF *def,DG_OBJS *objs,
										float paramf,FMATRIX *inv_m)
{
    // int size=objs->n_models;
	int size=HUMAN21_ATAMA+1;
    int i;
    int flag=0;

    fpu_CopyVector(r,x);

    for(i=0;i<size;i++){
		flag|=CalcObjCollisionBWithInvM2(r,x,base,def,objs,i,paramf,inv_m+i);
    }

    return flag;
}


/* そのフレームでの規定位置と現在位置の関係から、
   速度等に変更を加える。*/
/* 頂点データの更新 */
static inline void CalcBasedStandardPointAndUpdateVertexAndComvert2Local(Work *work)
{
    MODEL_PARAMETER *mdlp=&(work->model_param);
    VERTEX_PARAMETER *verp=work->ver_params;
    FVECTOR *base=work->base;
    FVECTOR *now=work->now;
    FVECTOR *v=work->v;
    FVECTOR *dv=work->dv;
    FVECTOR *normal=work->normal;
    FVECTOR *base_normal=work->base_normal;
    FVECTOR *lbase_normal=work->lbase_normal;
    float m_xlimit=mdlp->xlimit;
    float k=mdlp->k;
    float recover_u=mdlp->recover_u;
    // OBJECT *object=&(work->body);
    SVECTOR *lnow=LNOW_BUF;
    SVECTOR *lnormal=LNORMAL_BUF;
    FMATRIX *inv_m=INVM_BUF;
    FMATRIX m;
    FVECTOR x;
    CV2_MDL *mdl;
    FVECTOR *lbase;
    int i;

    mdl=(*(work->cvd_def+work->cvd_sel))->models;
    lbase=mdl->verts;


    /* ワールド座標系からローカル座標系へ */
#if 0
    InverseMatrix(&m,&(object->objs->world));
#elif 0
    sceVu0InversMatrix(*(sceVu0FMATRIX *)&m,*(sceVu0FMATRIX *)&(object->objs->world));
#else
    // FastInverseMatrix(&m,&(object->objs->world));
	FastInverseMatrix(&m,&(work->objs->world));
#endif


    if(work->target!=NULL){
		DG_OBJS *objs=work->target->objs;
		int size=work->target->objs->def->n_models;
		for(i=0;i<size;i++){
#if 0
			InverseMatrix(inv_m+i,&(objs->objs[i].world));
#elif 0
			sceVu0InversMatrix(*(sceVu0FMATRIX *)(inv_m+i),*(sceVu0FMATRIX *)&(objs->objs[i].world));
#else
			FastInverseMatrix(inv_m+i,&(objs->objs[i].world));
#endif
		}
    }

    for(i=work->num_vers;i>0;
		i--,verp++,base++,lbase++,now++,
			normal++,base_normal++,lbase_normal++,v++,dv++,lnow++,lnormal++){

		float l;
		float xlimit,vlimit;

		if(verp->fnest==0.0f){
			fpu_CopyVector(now,base);
			fpu_CopyVector(normal,base_normal);

			vu0_Ldv0(lbase);
			vu0_Ldv1(lbase_normal);

			fpu_ClearVector(v);

			vu0_FVtoIV0v0();
			vu0_FVtoIV12v1();

#ifdef BP_PSX2_ASM
			asm volatile ("
			qmfc2.i	$8,vf1
			qmfc2.i	$9,vf2
			ppach	$8,$0,$8
			ppach	$9,$0,$9
			sd		$8,(%0)
			sd		$9,(%1)
			" : : "r"(lnow),"r"(lnormal) : "$8","$9","memory" );
#else
			{
				extern FVECTOR REG_vf1, REG_vf2 ;
				lnow->vx = *(int*)&REG_vf1.vx ;
				lnow->vy = *(int*)&REG_vf1.vy ;
				lnow->vz = *(int*)&REG_vf1.vz ;
				lnow->pad = *(int*)&REG_vf1.vw ;
				lnormal->vx = *(int*)&REG_vf2.vx ;
				lnormal->vy = *(int*)&REG_vf2.vy ;
				lnormal->vz = *(int*)&REG_vf2.vz ;
				lnormal->pad = *(int*)&REG_vf2.vw ;
			}
#endif
		}
		else{
			xlimit=m_xlimit*verp->fnest;
			vlimit=xlimit*0.05f;

			vu0_Ldv1(v);
			vu0_Ldv2(dv);
			vu0_Mulv1a(k);

			vu0_Ldv0(now);

			vu0_Addv2v1();

			vu0_Ldv1(base);
			vu0_Subv1v0();
			vu0_Mulv1a(recover_u);
			vu0_Setv0w1();
			vu0_Addv2v1();

			vu0_Ldv1(v);
			vu0_Addv1v2();

			l=vu0_VectorLength2v1();
			l=fpu_Rsqrt(l,vlimit);

			if(l<0.95f) vu0_Mulv1a(l);

			vu0_Ldv2(base);
			vu0_Addv0v1();
			vu0_Stv1(v);

			vu0_Subv1v0v2();

			l=vu0_VectorLength2v1();
			l=fpu_Rsqrt(l,xlimit);

			if(l<0.95f){
				vu0_Mulv1a(l);
				vu0_Addv0v1v2();
				vu0_Ldv1(now);
				vu0_Subv1v0v1();
				vu0_Stv1(v);
			}

			vu0_Stv0(now);

#if 1

			if(work->target!=NULL){
				if(work->boundmodel==NULL){
					if(work->collision_flag){
						if(!_CalcObjsCollisionBWithInvM(&x,now,base,work->target->objs,
														work->param_oval,inv_m)) goto next;
					}
					else{
						if(!CalcObjsCollisionBWithInvM(&x,now,base,work->target->objs,
													   work->param_oval,inv_m)) goto next;
					}
				}
				else{
					if(work->collision_flag){
						if(!_CalcObjsCollisionBWithInvM2(&x,now,base,work->boundmodel,work->target->objs,
														 work->param_oval,inv_m)) goto next;
					}
					else{
						if(!CalcObjsCollisionBWithInvM2(&x,now,base,work->boundmodel,work->target->objs,
														work->param_oval,inv_m)) goto next;
					}
				}

				vu0_Ldv0(&x);
				vu0_Ldv1(now);
				vu0_Ldv2(v);
				vu0_Stv0(now);
				vu0_Subv1v0v1();
				vu0_Addv2v1();
				vu0_Stv2(v);

			next:
				;

			}
#endif

			/* ローカル座標系に変換 */
			vu0_Ldv1(normal);
			vu0_Ldv0(now);
			vu0_Ldm0(&m);

			/* 法線の正規化も同時に行なう */
			l=vu0_VectorLength2v1();
			vu0_Setv1w0();
			//l=fpu_Rsqrt(l,1.0f);
			vu0_RsqrtQ(l,1.0f);

			vu0_Mulv1m0v1();
			vu0_Mulv0m0v0();

			//vu0_Mulv1a(l);
			vu0_WaitQ();
			vu0_Mulv1Q();

			vu0_FVtoIV0v0();
			vu0_FVtoIV12v1();

#ifdef BP_PSX2_ASM
			asm volatile ("
			qmfc2.i	$8,vf1
			qmfc2.i	$9,vf2
			ppach	$8,$0,$8
			ppach	$9,$0,$9
			sd		$8,(%0)
			sd		$9,(%1)
			" : : "r"(lnow),"r"(lnormal) : "$8","$9","memory" );
#else
			{
				extern FVECTOR REG_vf1, REG_vf2 ;
				lnow->vx = *(int*)&REG_vf1.vx ;
				lnow->vy = *(int*)&REG_vf1.vy ;
				lnow->vz = *(int*)&REG_vf1.vz ;
				lnow->pad = *(int*)&REG_vf1.vw ;
				lnormal->vx = *(int*)&REG_vf2.vx ;
				lnormal->vy = *(int*)&REG_vf2.vy ;
				lnormal->vz = *(int*)&REG_vf2.vz ;
				lnormal->pad = *(int*)&REG_vf2.vw ;
			}
#endif
		}
    }

	DG_SwitchVAnimeBuffer(&(work->va));
    DG_RefineStripVertex2(&(work->va),(*(work->cvd_def+0))->models->verts_index);
    DG_RefineStripNormal2(&(work->va),(*(work->cvd_def+0))->models->verts_index);
}


void MoveWavingCloth(Work *work)
{
#ifdef L_DEBUG_MODE
    int cnt;
#endif

#if 0
    fpu_CopyMatrix(&(work->body.objs->world),&(work->rot_mat));
    fpu_CopyMatrix(&(work->body.objs->objs[0].world),&(work->body.objs->world));
#else
    fpu_CopyMatrix(&(work->objs->world),&(work->rot_mat));
    fpu_CopyMatrix(&(work->objs->objs[0].world),&(work->objs->world));
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




#if 0

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

#endif

}


/* OBJについてくる布の場合 */

/* 頂点の基準位置の計算 */
static void InitBasicVertexWithOBJ(Work *work)
{
    FVECTOR *base=(FVECTOR *)GV_UNCACHEA(work->base);
    FVECTOR *lbase_normal=work->lbase_normal;
    FVECTOR *base_normal=(FVECTOR *)GV_UNCACHEA(work->base_normal);
    FVECTOR *dv=(FVECTOR *)GV_UNCACHEA(work->dv);
    FVECTOR *normal=(FVECTOR *)GV_UNCACHEA(work->normal);
    CV2_MDL *mdl;
    FVECTOR *lbase;
    int i;


    mdl=(*(work->cvd_def+work->cvd_sel))->models;
    lbase=mdl->verts;


    vu0_Ldm1(work->tmat);
    vu0_Ldm2(&(work->rot_mat));

    vu0_Mulm0m1m2();

    vu0_Stm0(&(work->objs->world));
    vu0_Stm0(&(work->objs->objs[0].world));

    for(i=work->num_vers;i>0;
		i-=2,lbase+=2,base+=2,lbase_normal+=2,base_normal+=2,dv+=2,normal+=2){

#ifdef BP_PSX2_ASM
		asm volatile ("
		lqc2		vf16,0x00(%0)
		lqc2		vf17,0x10(%0)
		lqc2		vf18,0x00(%2)
		lqc2		vf19,0x10(%2)
		vmove.w		vf16,vf0
		vmove.w		vf17,vf0
		vmulx.w		vf18,vf18,vf0
		vmulx.w		vf19,vf19,vf0
		sq		$0,0x00(%4)
		sq		$0,0x10(%4)
		sq		$0,0x00(%5)
		sq		$0,0x10(%5)
		vmulax.xyzw	ACC,vf4,vf16x
		vmadday.xyzw	ACC,vf5,vf16y
		vmaddaz.xyzw	ACC,vf6,vf16z
		vmaddw.xyzw	vf16,vf7,vf16w
		vmulax.xyzw	ACC,vf4,vf17x
		vmadday.xyzw	ACC,vf5,vf17y
		vmaddaz.xyzw	ACC,vf6,vf17z
		vmaddw.xyzw	vf17,vf7,vf17w
		vmulax.xyzw	ACC,vf4,vf18x
		vmadday.xyzw	ACC,vf5,vf18y
		vmaddaz.xyzw	ACC,vf6,vf18z
		vmaddw.xyzw	vf18,vf7,vf18w
		vmulax.xyzw	ACC,vf4,vf19x
		vmadday.xyzw	ACC,vf5,vf19y
		vmaddaz.xyzw	ACC,vf6,vf19z
		vmaddw.xyzw	vf19,vf7,vf19w
		sqc2		vf16,0x00(%1)
		sqc2		vf17,0x10(%1)
		sqc2		vf18,0x00(%3)
		sqc2		vf19,0x10(%3)
		" : : "r"(lbase),"r"(base),"r"(lbase_normal),"r"(base_normal),"r"(dv),"r"(normal) : "memory" );
#else
static FVECTOR def_norm = {0.0f,1.0f,0.0f,0.0f} ;

		vu0_ApplyMatrixXYZ( base  , vu0_Ptrm0(), lbase   ) ;
		vu0_ApplyMatrixXYZ( base+1, vu0_Ptrm0(), lbase+1 ) ;
		vu0_ApplyMatrixROT( base_normal  , vu0_Ptrm0(), lbase_normal   ) ;
		vu0_ApplyMatrixROT( base_normal+1, vu0_Ptrm0(), lbase_normal+1 ) ;
		DG_COPY_VEC( dv  , &DG_ZeroVector );
		DG_COPY_VEC( dv+1, &DG_ZeroVector );
		DG_COPY_VEC( normal  , &def_norm );
		DG_COPY_VEC( normal+1, &def_norm );

#endif
    }

}


void MoveWavingClothWithOBJ(Work *work)
{
#ifdef L_DEBUG_MODE
    int cnt;
#endif

#ifdef L_DEBUG_MODE
    GV_SET_PRFC_CLOCK();
#endif

    /* 頂点のパラメータの初期化 */
    InitBasicVertexWithOBJ(work);

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



#if 0

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

#endif
}
