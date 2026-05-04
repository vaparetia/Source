/*
	preparticle.c
		パーティクルのシミュレーション

	2000/5/15 K.Kano
	$Id: preparticle.c,v 1.1.1.3 2002/11/19 11:43:30 Yoshizawa1 Exp $
*/


#include "preview_def.h"


#define FVECTOR4				FVECTOR
#define ClearFvector4			fpu_ClearVector
#define CopyFvector4			fpu_CopyVector
#define AddFvector3				fpu_AddVectors
#define SubFvector3				fpu_SubVectors
#define MulFvector3				fpu_MulVectorScaler
#define OuterProductFvector3	fpu_OuterProduct
#define Length2OfFvector3		fpu_VectorLength2
#define LengthOfFvector3(a)		fpu_Sqrt(fpu_VectorLength2(a))
#define rsqrtf					fpu_Rsqrt

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )


static inline void ClearCvector4(CVECTOR *a)
{
	asm ("sw	$0,0(%0)" : : "r"(a) : "memory");
}

static inline void CopyCvector4(CVECTOR *a,CVECTOR *b)
{
	asm ("
	lw	$8,0(%1)
	sw	$8,0(%0)
	" : : "r"(a),"r"(b) : "$8","memory");
}

static inline void CopyFvector4_(FVECTOR *a,FVECTOR_ *b)
{
	a->vx=b->vx;
	a->vy=b->vy;
	a->vz=b->vz;
	a->vw=b->vw;
}


#define VECX(a)		((a).vx)
#define VECY(a)		((a).vy)
#define VECZ(a)		((a).vz)
#define VECW(a)		((a).vw)

#define VECR(a)		((a).r)
#define VECG(a)		((a).g)
#define VECB(a)		((a).b)
#define VECA(a)		((a).cd)


#define PARTICLEFILENAME	"host0:./particle.rap"


static void SprParticle_Free(SPR_PARTICLES *spr)
{
	GM_FreePrim2(spr->prim);
	GV_Free(spr->particle);
	GV_Free(spr);
}

static SPR_PARTICLES *SprParticle_Alloc(int size)
{
	DG_PRIM2 *prim;
	SPR_PARTICLE *part;
	SPR_PARTICLES *spr;

	if((spr=GV_Malloc(sizeof(SPR_PARTICLES)+sizeof(DATA_PARTICLES)))==NULL){
		return NULL;
	}

	if((part=GV_Malloc(sizeof(SPR_PARTICLE)*size))==NULL){
		GV_Free(spr);
		return NULL;
	}

	if((prim=GM_MakePrim2(DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA,size,1))==NULL){
		GV_Free(spr);
		GV_Free(part);
		return NULL;
	}

	spr->data=(DATA_PARTICLES *)(spr+1);
	spr->prim=prim;
	spr->particle=part;

	spr->com.type=PARTICLE_TYPE_SPRITE;
	spr->com.size=size;

	AddAllParticleNoseqLink(spr);

	spr->tmat=NULL;
	fpu_ClearVector(&(spr->pos));
	fpu_ClearVector(&(spr->init_v));
	spr->init_v_flag=0;

	return spr;
}

static SPR_PARTICLES *LineParticle_Alloc(int size)
{
	DG_PRIM2 *prim;
	SPR_PARTICLE *part;
	SPR_PARTICLES *spr;

	if((spr=GV_Malloc(sizeof(SPR_PARTICLES)+sizeof(DATA_PARTICLES)))==NULL){
		return NULL;
	}

	if((part=GV_Malloc(sizeof(SPR_PARTICLE)*size))==NULL){
		GV_Free(spr);
		return NULL;
	}

	if((prim=GM_MakePrim2(DG_PRIM2_LINE|DG_PRIM2_FOG|DG_PRIM2_ALPHA,size,2))==NULL){
		GV_Free(spr);
		GV_Free(part);
		return NULL;
	}

	spr->data=(DATA_PARTICLES *)(spr+1);
	spr->prim=prim;
	spr->particle=part;

	spr->com.type=PARTICLE_TYPE_LINE;
	spr->com.size=size;

	AddAllParticleNoseqLink(spr);

	spr->tmat=NULL;
	fpu_ClearVector(&(spr->pos));
	fpu_ClearVector(&(spr->init_v));
	spr->init_v_flag=0;

	return spr;
}

static SPR_PARTICLES *PolyParticle_Alloc(int size)
{
	DG_PRIM2 *prim;
	SPR_PARTICLE *part;
	SPR_PARTICLES *spr;

	if((spr=GV_Malloc(sizeof(SPR_PARTICLES)+sizeof(DATA_PARTICLES)))==NULL){
		return NULL;
	}

	if((part=GV_Malloc(sizeof(SPR_PARTICLE)*size))==NULL){
		GV_Free(spr);
		return NULL;
	}

	if((prim=GM_MakePrim2(DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA,size,4))==NULL){
		GV_Free(spr);
		GV_Free(part);
		return NULL;
	}

	spr->data=(DATA_PARTICLES *)(spr+1);
	spr->prim=prim;
	spr->particle=part;

	spr->com.type=PARTICLE_TYPE_POLY;
	spr->com.size=size;

	AddAllParticleNoseqLink(spr);

	spr->tmat=NULL;
	fpu_ClearVector(&(spr->pos));
	fpu_ClearVector(&(spr->init_v));
	spr->init_v_flag=0;

	return spr;
}

static SPR_PARTICLES *PolyDParticle_Alloc(int size)
{
	SPR_PARTICLES *poly;
	poly=PolyParticle_Alloc(size);
	if(poly!=NULL){
		poly->com.type=PARTICLE_TYPE_POLY_DIAMOND;
	}
	return poly;
}

static void CopySprParticleFromData(SPR_PARTICLES *spr,DATA_PARTICLES *buf)
{
	DG_TEX *tex;

	/* プリミティブへテクスチャをセット */
	tex=DG_GetTexture(buf->tex_strcode);
	// printf("Texture = %d\n",buf->tex_strcode);

	DG_ConfigPrim2Tex(spr->prim,tex);
	spr->tex=tex;

	memcpy(spr->data,buf,sizeof(DATA_PARTICLES));

	switch(spr->data->alpha_mode){
	case PARTICLE_ALPHAMODE_S:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(2,2,0,0,0x00);
		break;
	case PARTICLE_ALPHAMODE_D:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(2,2,0,1,0x00);
		break;

	case PARTICLE_ALPHAMODE_SAs:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(0,2,0,2,0x00);
		break;
	case PARTICLE_ALPHAMODE_S1MAs:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(2,0,0,2,0x00);
		break;
	case PARTICLE_ALPHAMODE_DAs:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(1,2,0,2,0x00);
		break;
	case PARTICLE_ALPHAMODE_D1MAs:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(2,1,0,2,0x00);
		break;

	case PARTICLE_ALPHAMODE_S_D:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(0,2,2,1,0x80);
		break;
	case PARTICLE_ALPHAMODE_SAs_D:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(0,2,0,1,0x00);
		break;
	case PARTICLE_ALPHAMODE_S1MAs_D:
		break;
	case PARTICLE_ALPHAMODE_S_DAs:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(1,2,0,0,0x00);
		break;
	case PARTICLE_ALPHAMODE_S_D1MAs:
		break;

	case PARTICLE_ALPHAMODE_SAs_DAs:
		break;
	case PARTICLE_ALPHAMODE_SAs_D1MAs:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(0,1,0,1,0x00);
		break;
	case PARTICLE_ALPHAMODE_S1MAs_DAs:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(1,0,0,0,0x00);
		break;
	case PARTICLE_ALPHAMODE_S1MAs_D1MAs:
		break;

	case PARTICLE_ALPHAMODE_MSAs_D:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(2,0,0,1,0x00);
		break;
	case PARTICLE_ALPHAMODE_S_MDAs:
		spr->prim->tex_trans.alpha.data=SCE_GS_SET_ALPHA(2,1,0,0,0x00);
		break;
	}
}


/* ツールで作ったデータのロード */
static void ParticleLoad(void)
{
	DATA_PARTICLES buf;

	if(!PreviewLoadBinFile(PARTICLEFILENAME,&buf,sizeof(buf))){
		return;
	}

#if 1
	printf("Flag = 0x%08x 0x%08x\n",buf.simulation_frame_flag,buf.flag);
#endif

	switch(buf.type){
	case PARTICLE_TYPE_SPRITE:
		if((PreviewParticle.sim_work=SprParticle_Alloc(buf.size))==NULL) break;
		CopySprParticleFromData(PreviewParticle.sim_work,&buf);
		RestartSimParticle(PreviewParticle.sim_work);
		break;
	case PARTICLE_TYPE_LINE:
		if((PreviewParticle.sim_work=LineParticle_Alloc(buf.size))==NULL) break;
		CopySprParticleFromData(PreviewParticle.sim_work,&buf);
		RestartSimParticle(PreviewParticle.sim_work);
		break;
	case PARTICLE_TYPE_POLY:
		if((PreviewParticle.sim_work=PolyParticle_Alloc(buf.size))==NULL) break;
		CopySprParticleFromData(PreviewParticle.sim_work,&buf);
		RestartSimParticle(PreviewParticle.sim_work);
		break;
	case PARTICLE_TYPE_POLY_DIAMOND:
		if((PreviewParticle.sim_work=PolyDParticle_Alloc(buf.size))==NULL) break;
		CopySprParticleFromData(PreviewParticle.sim_work,&buf);
		RestartSimParticle(PreviewParticle.sim_work);
		break;
	}
}


/* シミュレーション用ワークの破棄 */
static void ParticleFree(void)
{
	if(PreviewParticle.sim_work!=NULL){
		SprParticle_Free((SPR_PARTICLES *)(PreviewParticle.sim_work));
	}
}



typedef struct {
    GV_ACT_EX	actor;
} Work;

static void Preview_ParticleAct(void)
{
	COM_PARTICLES *com=(COM_PARTICLES *)(PreviewParticle.sim_work);

	if(com==NULL) return;

	if(SimParticle(com)){
		/* RestartSimParticle(com); */
	}
}

static void Preview_ParticleDie(void)
{
	ParticleFree();
}


void Particle_DebugPrint(void)
{
	int x,y;

    x=LOCATE_X;
    y=LOCATE_Y;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("PARTICLE MENU");

    x=LOCATE_X+0x08*4;
    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("File Load");

    y+=COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf("Simulation Restart");

    /* カーソル */
    x=LOCATE_X;
    y=LOCATE_Y+COLUMN_HEIGHT+PreviewParticle.menu_cursole_pos*COLUMN_HEIGHT;
    DEBUG_Locate(x,y,MENU_MODE_NORMAL);
    DEBUG_Printf( ">>>" );
}

static void FileLoad_DebugCursole(void)
{
	if(PreviewKey.press & PAD_A){
		ParticleFree();
		ParticleLoad();
	}
}

static void SimRestart_DebugCursole(void)
{
	if(PreviewKey.press & PAD_A){
		if(PreviewParticle.sim_work!=NULL){
			RestartSimParticle((COM_PARTICLES *)(PreviewParticle.sim_work));
		}
	}
}

void Particle_DebugCursole(void)
{
	switch(PreviewKey.auto_status & (PAD_U|PAD_D)){
	case PAD_U:
		PreviewParticle.menu_cursole_pos--;
		if(PreviewParticle.menu_cursole_pos<0){
			PreviewParticle.menu_cursole_pos=1;
		}
		break;
	case PAD_D:
		PreviewParticle.menu_cursole_pos++;
		if(PreviewParticle.menu_cursole_pos>=2){
			PreviewParticle.menu_cursole_pos=0;
		}
		break;
	}

	switch(PreviewParticle.menu_cursole_pos){
	case 0:
		/* File Load */
		FileLoad_DebugCursole();
		break;
	case 1:
		/* Simulation Restart */
		SimRestart_DebugCursole();
		break;
	}
}


static int GetResource(void)
{
	return 1;
}

/* 初期化部メイン */
void *NewPreviewParticle(void)
{
    Work *work ;

    work=(Work *)GV_NewActor(GV_ACTOR_AFTER,sizeof(Work)) ;
    if(work!=NULL) {
		GetResource();
		GV_SetActor(&(work->actor),Preview_ParticleAct,Preview_ParticleDie);
		GV_ActorEX(&(work->actor));
    }
    return (void *)work ;
}
