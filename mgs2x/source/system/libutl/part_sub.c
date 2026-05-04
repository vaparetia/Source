/*
	part_sub.c
		パーティクル汎用制御ルーチン

	1999/08/26 K.Kano
	$Id: part_sub.c,v 1.1.1.3 2002/11/19 11:42:57 Yoshizawa1 Exp $
*/


#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#include <float.h>

#include "libgv.h"
#include "libdg.h"
#include "libutl.h"
#include "utl_dma.h"


/* 全てCPUで処理するが、メモリ転送をDMAにして、スクラッチパッドで
   全て処理するように変更した。*/


#define UNIT_OF_SCRATCH		16

#define D_CTRL_BUF	(*((int *)(SCRPAD_ADDR+0x4000-sizeof(int))))

/* ローカルのデータ型 */
struct _scratchpad {
    PARTICLE_PARAMETER particle[UNIT_OF_SCRATCH];
    sceDmaTag tag1;
    FVECTOR   pos[UNIT_OF_SCRATCH*3];
    sceDmaTag tag2;
};

#define setDmaTag(t,i,a,q)	(t)->id=(i),(t)->next=(sceDmaTag *)(a),(t)->qwc=(q)


static void CPU_ParticleSub(CONTROL_PARTICLES *cparts,PARTICLE_PARAMETER *partp,
							FVECTOR *pos,int size)
{
#if 0
    FVECTOR *polyd=(FVECTOR *)((int)SCRPAD_ADDR+sizeof(struct _scratchpad [4])
							   +sizeof(CONTROL_PARTICLES));
#else
    FVECTOR *polyd=(FVECTOR *)(cparts+1);
#endif

    int i;

    asm volatile ("
	lqc2	vf27,0(%1)
	lqc2	vf28,0(%0)
	vmulx.w	vf29,vf0,vf27
	vmuly.w	vf30,vf0,vf27
	vmulz.w	vf31,vf0,vf27
	qmtc2.ni	$0,vf26
	" : : "r"(&(cparts->bounding_box[2])),"r"(&(cparts->bounding_box[3])));

    for(i=size;i>0;i--,partp++){
		vu0_Ldv0(&(partp->pos));
		vu0_Ldv1(&(partp->v));
		vu0_Ldv2(&(partp->dv));
		vu0_Stv0(&(partp->old_pos));

		vu0_Addv0v1();
		vu0_Addv1v2();

		if(partp->count>0) partp->count--;

		vu0_Stv0(&(partp->pos));
		vu0_Stv1(&(partp->v));

		switch(cparts->bounding_type){
		case PARTICLE_BOUNDING_TYPE_ERASE:
	        {
				int ans;

				asm volatile ("
				vsub.xyz	vf16,vf1,vf28
				vclipw.xyz	vf16,vf29
				vclipw.xyz	vf16,vf30
				vclipw.xyz	vf16,vf31
				vnop
				vnop
				vnop
				vnop
				cfc2	$8,$vi18
				sw	$8,0(%0)
				" : : "r"(&ans) : "$8","memory" );

				if(ans & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4))) partp->count=0;
			}
	        break;

		case PARTICLE_BOUNDING_TYPE_OVERLAP:
	        {
				int ans;

				asm volatile ("
				vsub.xyz	vf16,vf1,vf28
				vclipw.xyz	vf16,vf29
				vclipw.xyz	vf16,vf30
				vclipw.xyz	vf16,vf31
				vnop
				vnop
				vnop
				vnop
				cfc2	$8,$vi18
				sw	$8,0(%0)
				" : : "r"(&ans) : "$8","memory" );

				if(ans & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4))){
					if(ans & (1<<13)){
						asm volatile ("vadd.x vf1,vf28,vf27");
					}
					else if(ans & (1<<12)){
						asm volatile ("vsub.x vf1,vf28,vf27");
					}
					if(ans & (1<<9)){
						asm volatile ("vadd.y vf1,vf28,vf27");
					}
					else if(ans & (1<<8)){
						asm volatile ("vsub.y vf1,vf28,vf27");
					}
					if(ans & (1<<5)){
						asm volatile ("vadd.z vf1,vf28,vf27");
					}
					else if(ans & (1<<4)){
						asm volatile ("vsub.z vf1,vf28,vf27");
					}
					vu0_Stv0(&(partp->pos));
					vu0_Stv0(&(partp->old_pos));
				}
			}
	    	break;

		case PARTICLE_BOUNDING_TYPE_REFLECT:
		    {
				int ans;

				asm volatile ("
				vsub.xyz	vf16,vf1,vf28
				vclipw.xyz	vf16,vf29
				vclipw.xyz	vf16,vf30
				vclipw.xyz	vf16,vf31
				vnop
				vnop
				vnop
				vnop
				cfc2	$8,$vi18
				sw	$8,0(%0)
				" : : "r"(&ans) : "$8","memory" );

				if(ans & ((1<<13)|(1<<12)|(1<<9)|(1<<8)|(1<<5)|(1<<4))){
					if(ans & (1<<13)){
						asm volatile ("
						vsub.x	vf1,vf28,vf27
						vsub.x	vf2,vf26,vf2
						");
					}
					else if(ans & (1<<12)){
						asm volatile ("
						vadd.x	vf1,vf28,vf27
						vsub.x	vf2,vf26,vf2
						");
					}
					if(ans & (1<<9)){
						asm volatile ("
						vsub.y	vf1,vf28,vf27
						vsub.y	vf2,vf26,vf2
						");
					}
					else if(ans & (1<<8)){
						asm volatile ("
						vadd.y	vf1,vf28,vf27
						vsub.y	vf2,vf26,vf2
						");
					}
					if(ans & (1<<5)){
						asm volatile ("
						vsub.z	vf1,vf28,vf27
						vsub.z	vf2,vf26,vf2
						");
					}
					else if(ans & (1<<4)){
						asm volatile ("
						vadd.z	vf1,vf28,vf27
						vsub.z	vf2,vf26,vf2
						");
					}

					vu0_Stv0(&(partp->pos));
					vu0_Stv0(&(partp->old_pos));
					vu0_Stv1(&(partp->v));
				}
			}
	        break;
		}

		switch(cparts->display_type){
		case PARTICLE_DISP_TYPE_SPR:
			vu0_Stv0(pos);
			pos++;
			break;

		case PARTICLE_DISP_TYPE_LINE:
			fpu_CopyVector(pos+0,&(partp->old_pos));
			vu0_Stv0(pos+1);
			pos+=2;
			break;

		case PARTICLE_DISP_TYPE_POLY:

#if 1

			asm volatile ("
			lqc2	vf24,0x00(%0)
			lqc2	vf25,0x10(%0)
			lqc2	vf26,0x20(%0)
			vmulaw.xyz	ACC,vf24,vf01
			vmaddw.xyz	vf24,vf01,vf00
			vmulaw.xyz	ACC,vf25,vf01
			vmaddw.xyz	vf25,vf01,vf00
			vmulaw.xyz	ACC,vf26,vf01
			vmaddw.xyz	vf26,vf01,vf00
			sqc2	vf24,0x00(%1)
			sqc2	vf25,0x10(%1)
			sqc2	vf26,0x20(%1)
			" : : "r"(polyd+partp->poly_count),"r"(pos) : "memory");

#else

			{
				FVECTOR *d=polyd+partp->poly_count;

				vu0_Ldv2(&(partp->pos));
				vu0_Ldv0(&(d[0]));
				vu0_Ldv1(&(d[1]));

				vu0_Mulv0a(partp->pos.vw);
				vu0_Mulv1a(partp->pos.vw);

				vu0_Addv0v2();
				vu0_Addv1v2();

				vu0_Stv0(pos+0);

				vu0_Ldv0(&(d[2]));

				vu0_Stv1(pos+1);

				vu0_Mulv0a(partp->pos.vw);

				vu0_Addv0v2();

				vu0_Stv0(pos+2);
			}

#endif

			partp->poly_count++;
			if(partp->poly_count>=15) partp->poly_count-=15;

			pos+=3;

			break;
		}
    }
}

static inline void toSPRDmaSync(void)
{
    //UTL_WaitDma(SCE_DMA_toSPR);
}

static inline void fromSPRDmaSync(void)
{
    //UTL_WaitDma(SCE_DMA_fromSPR);
}

static void DMA_mem2spr(void *adr,struct _scratchpad *spr)
{
#if 0
    /* メモリからスクラッチパッド */
    volatile sceDmaChan *chan_tospr=sceDmaGetChan(SCE_DMA_toSPR);

    toSPRDmaSync();
    chan_tospr->sadr=&(spr->particle[0]);
    sceDmaSendN(chan_tospr,adr,sizeof(spr->particle)>>4);
#else
    UTL_StartMemToSpr(&(spr->particle[0]),adr,sizeof(spr->particle)>>4);
#endif
}

static void DMA_spr2mem(struct _scratchpad *spr,int size,int disp_mode,void *mem1,void *mem2)
{
    /* スクラッチパッドからメモリ */
    // volatile sceDmaChan *chan_fromspr=sceDmaGetChan(SCE_DMA_fromSPR);

#if 0
    setDmaTag(&(spr->tag1),0x10,mem2,size*(disp_mode+1)); /* End Tag */
    setDmaTag(&(spr->tag2),0x70,0,0); /* End Tag */

    fromSPRDmaSync();

    asm volatile ("sync.l");

    /* Distination Chain mode */
    chan_fromspr->sadr=&(spr->particle[0]);
    chan_fromspr->tadr=(void *)((int)(&(spr->tag1))|0x80000000);
    chan_fromspr->madr=mem1;
    chan_fromspr->qwc=size*(sizeof(PARTICLE_PARAMETER)>>4);

    {
		tD_CHCR chcr;
		chcr=chan_fromspr->chcr;
		chcr.MOD=1,chcr.STR=1;
		chan_fromspr->chcr=chcr;
    }
#elif 0
    setDmaTag(&(spr->tag1),0x10,mem2,size*(disp_mode+1)); /* End Tag */
    setDmaTag(&(spr->tag2),0x70,0,0); /* End Tag */

    fromSPRDmaSync();

    asm volatile ("sync.l");

    /* Distination Chain mode */
    chan_fromspr->sadr=&(spr->particle[0]);
    chan_fromspr->madr=mem1;
    chan_fromspr->qwc=size*(sizeof(PARTICLE_PARAMETER)>>4);
    sceDmaRecv(chan_fromspr);
#elif 0
    fromSPRDmaSync();

    chan_fromspr->sadr=&(spr->particle[0]);
    sceDmaSendN(chan_fromspr,mem1,size*(sizeof(PARTICLE_PARAMETER)>>4));

    fromSPRDmaSync();

    chan_fromspr->sadr=&(spr->pos[0]);
    sceDmaSendN(chan_fromspr,mem2,size*(disp_mode+1));
#else
    UTL_StartSprToMem(mem1,&(spr->particle[0]),size*(sizeof(PARTICLE_PARAMETER)>>4));
    UTL_StartSprToMem(mem2,&(spr->pos[0]),size*(disp_mode+1));
#endif

}

static inline void ParticleSubInit(CONTROL_PARTICLES *cparts,CONTROL_PARTICLES *scr_part)
{

#define PARAM_SIN30	0.0f
#define PARAM_SIN31	0.866025404f
#define PARAM_SIN32	-0.866025404f
#define PARAM_COS30	1.0f
#define PARAM_COS31	-0.5f
#define PARAM_COS32	-0.5f

#define PARAM_SIN50	0.0f
#define PARAM_SIN51	0.951056516f
#define PARAM_SIN52	0.587785252f
#define PARAM_SIN53	-0.587785252f
#define PARAM_SIN54	-0.951056516f
#define PARAM_COS50	1.0f
#define PARAM_COS51	0.309016994f
#define PARAM_COS52	-0.809016994f
#define PARAM_COS53	-0.809016994f
#define PARAM_COS54	0.309016994f

    static const FVECTOR polyd[]={
		{ PARAM_COS30, PARAM_COS50*PARAM_SIN30, PARAM_SIN50*PARAM_SIN30, 0.0f, },
		{ PARAM_COS31, PARAM_COS51*PARAM_SIN31, PARAM_SIN51*PARAM_SIN31, 0.0f, },
		{ PARAM_COS32, PARAM_COS52*PARAM_SIN32, PARAM_SIN52*PARAM_SIN32, 0.0f, },

		{ PARAM_COS30, PARAM_COS53*PARAM_SIN30, PARAM_SIN53*PARAM_SIN30, 0.0f, },
		{ PARAM_COS31, PARAM_COS54*PARAM_SIN31, PARAM_SIN54*PARAM_SIN31, 0.0f, },
		{ PARAM_COS32, PARAM_COS50*PARAM_SIN32, PARAM_SIN50*PARAM_SIN32, 0.0f, },

		{ PARAM_COS30, PARAM_COS51*PARAM_SIN30, PARAM_SIN51*PARAM_SIN30, 0.0f, },
		{ PARAM_COS31, PARAM_COS52*PARAM_SIN31, PARAM_SIN52*PARAM_SIN31, 0.0f, },
		{ PARAM_COS32, PARAM_COS53*PARAM_SIN32, PARAM_SIN53*PARAM_SIN32, 0.0f, },

		{ PARAM_COS30, PARAM_COS54*PARAM_SIN30, PARAM_SIN54*PARAM_SIN30, 0.0f, },
		{ PARAM_COS31, PARAM_COS50*PARAM_SIN31, PARAM_SIN50*PARAM_SIN31, 0.0f, },
		{ PARAM_COS32, PARAM_COS51*PARAM_SIN32, PARAM_SIN51*PARAM_SIN32, 0.0f, },

		{ PARAM_COS30, PARAM_COS52*PARAM_SIN30, PARAM_SIN52*PARAM_SIN30, 0.0f, },
		{ PARAM_COS31, PARAM_COS53*PARAM_SIN31, PARAM_SIN53*PARAM_SIN31, 0.0f, },
		{ PARAM_COS32, PARAM_COS54*PARAM_SIN32, PARAM_SIN54*PARAM_SIN32, 0.0f, },

		{ PARAM_COS30, PARAM_COS50*PARAM_SIN30, PARAM_SIN50*PARAM_SIN30, 0.0f, },
		{ PARAM_COS31, PARAM_COS51*PARAM_SIN31, PARAM_SIN51*PARAM_SIN31, 0.0f, },
    };

#if 0
    volatile sceDmaChan *chan_fromspr=sceDmaGetChan(SCE_DMA_fromSPR);
    volatile sceDmaChan *chan_tospr=sceDmaGetChan(SCE_DMA_toSPR);
    int d_ctrl;

    fromSPRDmaSync();
    toSPRDmaSync();

    /* Cycle Steal off */
    d_ctrl=DGET_D_CTRL();
    D_CTRL_BUF=d_ctrl;
    d_ctrl&=~(D_CTRL_RCYC_M|D_CTRL_RELE_M);
    DPUT_D_CTRL(d_ctrl);

    chan_tospr->sadr=scr_part;
    sceDmaSendN(chan_tospr,cparts,sizeof(CONTROL_PARTICLES)>>4);

    toSPRDmaSync();
    //chan_tospr->sadr=scr_part+1;
    sceDmaSendN(chan_tospr,polyd,sizeof(polyd)>>4);

    toSPRDmaSync();

    /* Cycle Steal on (Release Cycle = 32) */
    d_ctrl=DGET_D_CTRL();
    d_ctrl&=~D_CTRL_RCYC_M;
    d_ctrl|=D_CTRL_RELE_M|(2<<D_CTRL_RCYC_O);
    DPUT_D_CTRL(d_ctrl);
#else
    UTL_StartMemToSpr(scr_part,cparts,sizeof(CONTROL_PARTICLES)>>4);
    UTL_StartMemToSpr(scr_part+1,(void *)polyd,sizeof(polyd)>>4);
    toSPRDmaSync();
#endif

}

static inline void ParticleSubEnd(void)
{
    fromSPRDmaSync();
    toSPRDmaSync();
#if 0
    DPUT_D_CTRL(D_CTRL_BUF);
#endif
}

#define PACKET_INCLEMENT(_cparts,_packet) \
do{ \
	switch((_cparts)->display_type){ \
	case PARTICLE_DISP_TYPE_SPR: \
		(_packet)=(DG_PRIM_PACKET *)((int)(_packet)+((sizeof(DG_PRIM_PACKET) \
									 +sizeof(DG_SPRT))<<4)); \
		break; \
	case PARTICLE_DISP_TYPE_LINE: \
		if((_cparts)->gour_flag){ \
			(_packet)=(DG_PRIM_PACKET *)((int)(_packet)+((sizeof(DG_PRIM_PACKET) \
										 +sizeof(DG_LINE_G2))<<4)); \
		 } \
		else{ \
			(_packet)=(DG_PRIM_PACKET *)((int)(_packet)+((sizeof(DG_PRIM_PACKET) \
										 +sizeof(DG_LINE_R2))<<4)); \
		} \
		break; \
	case PARTICLE_DISP_TYPE_POLY: \
		if((_cparts)->gour_flag){ \
			(_packet)=(DG_PRIM_PACKET *)((int)(_packet)+((sizeof(DG_PRIM_PACKET) \
										 +sizeof(DG_POLY_G3))<<4)); \
		} \
		else{ \
			(_packet)=(DG_PRIM_PACKET *)((int)(_packet)+((sizeof(DG_PRIM_PACKET) \
										 +sizeof(DG_POLY_R3))<<4)); \
		} \
		break; \
	} \
}while(0)


void ParticleSub(CONTROL_PARTICLES *cparts_master)
{
    PARTICLE_PARAMETER *from_partp;
    PARTICLE_PARAMETER *to_partp;
    DG_PRIM_PACKET *packet;
    FVECTOR *pos;
    int size;
    int cpu_size[4];

    int idx_mem2spr=0,idx_cpu=0,idx_spr2mem=0;
    struct _scratchpad (*scratchpad)[4]=(struct _scratchpad (*)[4])SCRPAD_ADDR;
    CONTROL_PARTICLES *cparts=(CONTROL_PARTICLES *)&((*scratchpad)[4]);


    //printf("Check 1\n");

    /* 初期化 */
    ParticleSubInit(cparts_master,cparts);

    from_partp=cparts->partp;
    to_partp=cparts->partp;
    packet=(DG_PRIM_PACKET *)(((DG_PRIM *)(cparts->prim))->packs[DG_Clock]);
    pos=cparts->pos;
    size=cparts->size;

    //printf("Check 2\n");


    /* PATH 1 */
    /* MEM -> SPR */
    DMA_mem2spr(from_partp,&((*scratchpad)[idx_mem2spr]));
    from_partp+=UNIT_OF_SCRATCH;
    size-=UNIT_OF_SCRATCH;

    if(size<0) cpu_size[idx_mem2spr]=size+UNIT_OF_SCRATCH;
    else cpu_size[idx_mem2spr]=UNIT_OF_SCRATCH;
    idx_mem2spr=(idx_mem2spr+1)&3;


    /* PATH 1 */
    /* MEM -> SPR */
    DMA_mem2spr(from_partp,&((*scratchpad)[idx_mem2spr]));
    from_partp+=UNIT_OF_SCRATCH;
    size-=UNIT_OF_SCRATCH;

    if(size<0) cpu_size[idx_mem2spr]=size+UNIT_OF_SCRATCH;
    else cpu_size[idx_mem2spr]=UNIT_OF_SCRATCH;
    idx_mem2spr=(idx_mem2spr+1)&3;


    /* PATH 2 */
    CPU_ParticleSub(cparts,&((*scratchpad)[idx_cpu].particle[0]),
					&((*scratchpad)[idx_cpu].pos[0]),cpu_size[idx_cpu]);

    if(cparts->CPU_OriginalFunc!=NULL){
		(*(cparts->CPU_OriginalFunc))(cparts,&((*scratchpad)[idx_cpu].particle[0]),
									  &((*scratchpad)[idx_cpu].pos[0]),packet,cpu_size[idx_cpu]);
    }

    idx_cpu=(idx_cpu+1)&3;
    PACKET_INCLEMENT(cparts,packet);


    while(cpu_size[idx_spr2mem]>0){
		/* PATH 3 */
		/* SPR -> MEM */
		DMA_spr2mem(&((*scratchpad)[idx_spr2mem]),cpu_size[idx_spr2mem],
					cparts->display_type,to_partp,pos);
		to_partp+=UNIT_OF_SCRATCH;
		pos+=(cparts->display_type+1)*UNIT_OF_SCRATCH;
		idx_spr2mem=(idx_spr2mem+1)&3;


		/* PATH 1 */
		/* MEM -> SPR */
		DMA_mem2spr(from_partp,&((*scratchpad)[idx_mem2spr]));
		from_partp+=UNIT_OF_SCRATCH;
		size-=UNIT_OF_SCRATCH;

		if(size<0) cpu_size[idx_mem2spr]=size+UNIT_OF_SCRATCH;
		else cpu_size[idx_mem2spr]=UNIT_OF_SCRATCH;
		idx_mem2spr=(idx_mem2spr+1)&3;


		/* PATH 2 */
		CPU_ParticleSub(cparts,&((*scratchpad)[idx_cpu].particle[0]),
						&((*scratchpad)[idx_cpu].pos[0]),cpu_size[idx_cpu]);

		if(cparts->CPU_OriginalFunc!=NULL){
			(*(cparts->CPU_OriginalFunc))(cparts,&((*scratchpad)[idx_cpu].particle[0]),
										  &((*scratchpad)[idx_cpu].pos[0]),packet,cpu_size[idx_cpu]);
		}

		idx_cpu=(idx_cpu+1)&3;
		PACKET_INCLEMENT(cparts,packet);
    }

    ParticleSubEnd();
}


/* ワークの初期化 */
int InitParticle(CONTROL_PARTICLES *cparts)
{
    int msize=sizeof(PARTICLE_PARAMETER)*cparts->size;

    vu0_Ldv0(&(cparts->bounding_box[0]));
    vu0_Ldv1(&(cparts->bounding_box[1]));

    cparts->partp=NULL;
    cparts->pos=NULL;
    cparts->prim=NULL;

    vu0_Addv2v0v1();
    vu0_Subv0v1();

    vu0_Mulv2a(1.0f/2.0f);
    vu0_Mulv0a(1.0f/2.0f);

    asm volatile ("vabs.xyz vf1,vf1");
    vu0_Stv2(&(cparts->bounding_box[2]));
    vu0_Stv0(&(cparts->bounding_box[3]));

    if((cparts->partp=(PARTICLE_PARAMETER *)GV_Malloc(msize))==NULL) return 0;

    GV_ZeroMemory(cparts->partp,msize);

    switch(cparts->display_type){
    case PARTICLE_DISP_TYPE_LINE:
		if(cparts->gour_flag){
			if((cparts->pos
				=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*2*cparts->size))==NULL) return 0;
			if((cparts->prim=GM_MakePrim(DG_PRIM_LINE_G2,
										 cparts->size, /* packet */
										 1, /* prim */
										 cparts->pos,
										 NULL))==NULL) return 0;
		}
		else{
			if((cparts->pos
				=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*2*cparts->size))==NULL) return 0;
			if((cparts->prim=GM_MakePrim(DG_PRIM_LINE_R2,
										 cparts->size, /* packet */
										 1, /* prim */
										 cparts->pos,
										 NULL))==NULL) return 0;
		}
		break;
    case PARTICLE_DISP_TYPE_POLY:
		if(cparts->gour_flag){
			if((cparts->pos
				=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*3*cparts->size))==NULL) return 0;
			if((cparts->prim=GM_MakePrim(DG_PRIM_POLY_G3,
										 cparts->size, /* packet */
										 1, /* prim */
										 cparts->pos,
										 NULL))==NULL) return 0;
		}
		else{
			if((cparts->pos
				=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*3*cparts->size))==NULL) return 0;
			if((cparts->prim=GM_MakePrim(DG_PRIM_POLY_R3,
										 cparts->size, /* packet */
										 1, /* prim */
										 cparts->pos,
										 NULL))==NULL) return 0;
		}
		break;
    case PARTICLE_DISP_TYPE_SPR:
		if((cparts->pos
			=(FVECTOR *)GV_Malloc(sizeof(FVECTOR)*1*cparts->size))==NULL) return 0;
		if((cparts->prim=GM_MakePrim(DG_PRIM_SPRT3D1,
									 cparts->size, /* packet */
									 1, /* prim */
									 cparts->pos,
									 NULL))==NULL) return 0;
		break;
    }

    return 1;
}
