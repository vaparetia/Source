/*
	partsub2.c
		パーティクルのシミュレーション

	2000/5/25 K.Kano
	$Id: partsub2.c,v 1.1.1.3 2002/11/19 11:43:26 Yoshizawa1 Exp $
*/


#include "partsub2.h"


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


extern FVECTOR	G_wind;
int OK_GetLocalWind( FVECTOR *pos, FVECTOR *output );


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


static void AddParticleSeqLink(SPR_PARTICLE *p,SPR_PARTICLES *ps)
{
    p->next=ps->seq_link;
    p->prea=NULL;
    if(p->next!=NULL) p->next->prea=p;
    ps->seq_link=p;
}

#if 0

static SPR_PARTICLE *GetParticleFromSeqLink(SPR_PARTICLES *ps)
{
    SPR_PARTICLE *p=ps->seq_link;
    if(p!=NULL){
		ps->seq_link=p->next;
		p->next->prea=NULL;
    }
    return p;
}

#endif

static void RemoveParticleSeqLink(SPR_PARTICLE *p,SPR_PARTICLES *ps)
{
    if(p->next!=NULL) p->next->prea=p->prea;
    if(p->prea!=NULL) p->prea->next=p->next;
    else ps->seq_link=p->next;
}

static void AddParticleNoseqLink(SPR_PARTICLE *p,SPR_PARTICLES *ps)
{
    p->next=ps->noseq_link;
    p->prea=NULL;
    if(p->next!=NULL) p->next->prea=p;
    ps->noseq_link=p;
}

static SPR_PARTICLE *GetParticleFromNoseqLink(SPR_PARTICLES *ps)
{
    SPR_PARTICLE *p=ps->noseq_link;
    if(p!=NULL){
		ps->noseq_link=p->next;
		if(p->next!=NULL) p->next->prea=NULL;
    }
    return p;
}

#if 0

static void RemoveParticleNoseqLink(SPR_PARTICLE *p,SPR_PARTICLES *ps)
{
    if(p->next!=NULL) p->next->prea=p->prea;
    if(p->prea!=NULL) p->prea->next=p->next;
    else ps->noseq_link=p->next;
}

#endif

void AddAllParticleNoseqLink(SPR_PARTICLES *ps)
{
    SPR_PARTICLE *p=ps->particle;
    int i;

    ps->seq_link=NULL;
    ps->noseq_link=NULL;
    for(i=0;i<ps->com.size;i++,p++){
		AddParticleNoseqLink(p,ps);
    }
}


static void CopyOld(SPR_PARTICLE *p)
{
    CopyFvector4(&(p->oldp[p->mem_frames_index]),&(p->p));
    CopyFvector4(&(p->oldsz[p->mem_frames_index]),&(p->sz));
    CopyCvector4(&(p->oldrgba[p->mem_frames_index]),&(p->rgba));
    p->mem_frames_index++;
    if(p->mem_frames_index>=MAX_MEMORY_FRAMES) p->mem_frames_index=0;
}

static void CopyOldAll(SPR_PARTICLE *p)
{
    int i=MAX_MEMORY_FRAMES;
    while(i>0){
		i--;
		CopyFvector4(&(p->oldp[i]),&(p->p));
		CopyFvector4(&(p->oldsz[i]),&(p->sz));
		CopyCvector4(&(p->oldrgba[i]),&(p->rgba));
    }
    p->mem_frames_index++;
    if(p->mem_frames_index>=MAX_MEMORY_FRAMES) p->mem_frames_index=0;
}

static SPR_PARTICLE *MakeAfterimg(SPR_PARTICLE *p,SPR_PARTICLES *particles)
{
    SPR_PARTICLE *p2=GetParticleFromNoseqLink(particles);

    if(p2!=NULL){
		memcpy(p2,p,sizeof(SPR_PARTICLE));

		AddParticleSeqLink(p2,particles);
		p2->status=PARTICLE_STATUS_AFTERIMAGE;
    }

    return p2;
}


static void RestartSimSprParticle(SPR_PARTICLES *spr)
{
    SPR_PARTICLE *p;
    int size;
    int i;

    spr->frame=0;
    spr->count=spr->data->simulation_frame[N_SIMULATION_FRAME-1];
    spr->num_particles_on_frame=0;
    spr->count_particles_on_frame=0;

    p=spr->particle;
    size=spr->com.size;

    for(i=0;i<size;i++){
		(p+i)->status=PARTICLE_STATUS_NONE;
		(p+i)->count=0;
    }

	AddAllParticleNoseqLink(spr);

	DG_VisiblePrim2(spr->prim);
}

void RestartSimParticle(COM_PARTICLES *com)
{
	if(com==NULL) return;

	switch(com->type){
	case PARTICLE_TYPE_SPRITE:
	case PARTICLE_TYPE_LINE:
	case PARTICLE_TYPE_POLY:
	case PARTICLE_TYPE_POLY_DIAMOND:
		RestartSimSprParticle((SPR_PARTICLES *)com);
		break;
	}
}



static void BornSprParticle(SPR_PARTICLES *particles)
{
	DATA_PARTICLES *data=particles->data;
	FVECTOR pos,init_v;
	int i;

    if(particles->frame<data->simulation_frame[0]){
		particles->num_particles_on_frame=0;
    }
    else if(particles->frame==data->simulation_frame[0]){
		particles->num_particles_on_frame=data->nums_simulation_frame[0];
    }
    else if(particles->frame>=data->simulation_frame[5]){
		particles->num_particles_on_frame=0;
    }
    particles->count_particles_on_frame
		=particles->num_particles_on_frame*(1.0f+data->num_percent*frnd());

	// printf("%d , %d , %d\n",particles->frame,particles->count,particles->count_particles_on_frame);


	if(particles->tmat!=NULL) vu0_Ldm0(particles->tmat);

	switch(particles->init_v_flag){
	case 0:
		CopyFvector4_(&init_v,&(data->init_v));
		init_v.vw=0.0f;
		break;
	case 1:
		CopyFvector4_(&init_v,&(data->init_v));
		init_v.vw=0.0f;
		vu0_Ldv0(&init_v);
		vu0_Mulv0m0v0();
		vu0_Stv0(&init_v);
		break;
	case 2:
		CopyFvector4(&init_v,&(particles->init_v));
		break;
	case 3:
		vu0_Ldv0(&(particles->init_v));
		vu0_Setv0w0();
		vu0_Mulv0m0v0();
		vu0_Stv0(&init_v);
		break;
	}

	vu0_Ldv0(&(particles->pos));
	vu0_Setv0w1();
	if(particles->tmat!=NULL) vu0_Mulv0m0v0();
	vu0_Stv0(&pos);

#if 0
	printf("pos = %f %f %f %f\n",
		   pos.vx,pos.vy,pos.vz,pos.vw);
	printf("v   = %f %f %f %f\n",
		   init_v.vx,init_v.vy,init_v.vz,init_v.vw);
#endif


	while(particles->count_particles_on_frame>0){
		SPR_PARTICLE *p=GetParticleFromNoseqLink(particles);

		if(p==NULL) break;

		particles->count_particles_on_frame--;

		/* Start Sequence */
		AddParticleSeqLink(p,particles);
		p->status=PARTICLE_STATUS_ON_SEQUENCE;

		CopyFvector4(&(p->init_p),&pos);

		switch(data->born_type){
		case 0:
			break;
		case 1:
			{
				FVECTOR4 tmpx={ 1.0f,0.0f,0.0f,0.0f, };
				FVECTOR4 tmpy;

				/* 極座標の生成 */
				float sita=rnd()*(float)M_PI*2.0f;
				float dis=data->born_radius
					-rnd()*data->born_rnd_radius;

				OuterProductFvector3(&tmpy,&init_v,&tmpx);
				if(Length2OfFvector3(&tmpy)==0.0f){
					VECX(tmpx)=0.0f;
					VECY(tmpx)=1.0f;
					VECZ(tmpx)=0.0f;
					OuterProductFvector3(&tmpy,&init_v,&tmpx);
				}
				OuterProductFvector3(&tmpx,&tmpy,&init_v);
				MulFvector3(&tmpx,&tmpx,
							rsqrtf(Length2OfFvector3(&tmpx),cosf(sita)*dis));
				MulFvector3(&tmpy,&tmpy,
							rsqrtf(Length2OfFvector3(&tmpy),sinf(sita)*dis));

				AddFvector3(&(p->init_p),&(p->init_p),&tmpx);
				AddFvector3(&(p->init_p),&(p->init_p),&tmpy);
			}
			break;
		case 2:
			{
				/* 極座標の生成 */
				float sitax=rnd()*(float)M_PI*2.0f;
				float sitay=rnd()*(float)M_PI;
				float dis=rnd()*data->born_radius;

				VECX(p->init_p)=sinf(sitax)*sinf(sitay)*dis;
				VECY(p->init_p)=cosf(sitax)*dis;
				VECZ(p->init_p)=sinf(sitax)*cosf(sitay)*dis;
			}
			break;
		}

		CopyFvector4(&(p->init_v),&init_v);
		{
			FVECTOR4 tmpx={ 1.0f,0.0f,0.0f,0.0f, };
			FVECTOR4 tmpy;
			float vrnd;
			float vrndxy,vrndz;
			float cosz,sinz;

			OuterProductFvector3(&tmpy,&(p->init_v),&tmpx);
			if(Length2OfFvector3(&tmpy)==0.0f){
				VECX(tmpx)=0.0f;
				VECY(tmpx)=1.0f;
				VECZ(tmpx)=0.0f;
				OuterProductFvector3(&tmpy,&(p->init_v),&tmpx);
			}
			OuterProductFvector3(&tmpx,&tmpy,&(p->init_v));
			MulFvector3(&tmpx,&tmpx,
						bp_sqrtf(Length2OfFvector3(&(p->init_v))/Length2OfFvector3(&tmpx))); //BP_MATH - emulate PS2 sqrtf
			MulFvector3(&tmpy,&tmpy,
						bp_sqrtf(Length2OfFvector3(&(p->init_v))/Length2OfFvector3(&tmpy))); //BP_MATH - emulate PS2 sqrtf

			vrndxy=rnd()*(float)M_PI*2.0f;
			vrndz=rnd()*data->init_v_angle;

			cosz=cosf(vrndz);
			sinz=sinf(vrndz);
			MulFvector3(&(p->init_v),&(p->init_v),(float)cosz);
			MulFvector3(&tmpx,&tmpx,(float)(sinz*cosf(vrndxy)));
			MulFvector3(&tmpy,&tmpy,(float)(sinz*sinf(vrndxy)));
			AddFvector3(&(p->init_v),&(p->init_v),&tmpx);
			AddFvector3(&(p->init_v),&(p->init_v),&tmpy);


			vrnd=frnd()*data->init_vrnd;
			MulFvector3(&tmpx,&(p->init_v),
						rsqrtf(Length2OfFvector3(&(p->init_v)),vrnd));
			AddFvector3(&(p->init_v),&(p->init_v),&tmpx);
		}

		CopyFvector4_(&(p->init_sz),&(data->sz[0]));
		ClearFvector4(&(p->init_vsz));

		CopyCvector4(&(p->init_rgba),&(data->rgba[0]));
		ClearCvector4(&(p->init_vrgba));

		CopyFvector4(&(p->p),&(p->init_p));
		CopyFvector4(&(p->v),&(p->init_v));
		CopyFvector4(&(p->sz),&(p->init_sz));
		CopyFvector4(&(p->vsz),&(p->init_vsz));
		CopyCvector4(&(p->rgba),&(p->init_rgba));
		CopyCvector4(&(p->vrgba),&(p->init_vrgba));

		p->enable_distance=data->distance+data->rnd_distance*frnd();

		p->frame=0;
		p->mem_frames_index=0;
		p->count=data->alive_time+data->rnd_alive_time*frnd();
		if(p->count<0) p->count=0;

		CopyOldAll(p);
	}

	// printf("%d = %d\n",particles->frame,particles->num_particles_on_frame);

	for(i=1;i<N_SIMULATION_FRAME;i++){
		if(FLAG_TST(data->simulation_frame_flag,i)){
			if(particles->frame<data->simulation_frame[i]){
				int dif_count=data->simulation_frame[i]-particles->frame;
				particles->num_particles_on_frame
					+=(data->nums_simulation_frame[i]
					   -particles->num_particles_on_frame)/dif_count;
				break;
			}
		}
	}
}

static void SimSprParticle(SPR_PARTICLES *particles)
{
	DATA_PARTICLES *data=particles->data;
    SPR_PARTICLE *next;

    next=particles->seq_link;

    while(next!=NULL){
		SPR_PARTICLE *p=next;
		int i;

		next=p->next;

		switch(p->status){
		case PARTICLE_STATUS_ON_SEQUENCE:
			CopyOld(p);

			{
				FVECTOR4 a;

				ClearFvector4(&a);

				if(FLAG_TST(data->flag,PARTICLE_FLAG_GRAVITY)){
					VECY(a)+=P_GRAVITY*data->gravity_parcent;
				}
				if(FLAG_TST(data->flag,PARTICLE_FLAG_AIR)){
					extern FVECTOR G_wind;
					FVECTOR4 air;
					FVECTOR4 wind,*pwind=&wind;

					if(!OK_GetLocalWind(&(p->p),&wind)){
						pwind=&G_wind;
					}

					CopyFvector4(&air,pwind);
					MulFvector3(&air,&air,data->air_parcent);
					AddFvector3(&a,&a,&air);
				}
				if(FLAG_TST(data->flag,PARTICLE_FLAG_V_REGIST)){
					FVECTOR4 v_reg;
					CopyFvector4(&v_reg,&(p->v));
					MulFvector3(&v_reg,&v_reg,data->v_regist);
					AddFvector3(&a,&a,&v_reg);
				}

				AddFvector3(&(p->p),&(p->p),&(p->v));
				AddFvector3(&(p->v),&(p->v),&a);
			}
			{
				int i;

				for(i=1;i<5;i++){
					if(FLAG_TST(data->sz_flag,i)){
						if(p->frame<data->sz_frame[i]){
							int dif_frame=data->sz_frame[i]-p->frame;
							VECX(p->sz)=VECX(p->sz)
								+(VECX(data->sz[i])-VECX(p->sz))/dif_frame;
							VECY(p->sz)=VECY(p->sz)
								+(VECY(data->sz[i])-VECY(p->sz))/dif_frame;
							break;
						}
					}
				}

				for(i=1;i<5;i++){
					if(FLAG_TST(data->rgba_flag,i)){
						if(p->frame<data->rgba_frame[i]){
							int dif_frame=data->rgba_frame[i]-p->frame;
							VECR(p->rgba)=VECR(p->rgba)
								+(VECR(data->rgba[i])-VECR(p->rgba))/dif_frame;
							VECG(p->rgba)=VECG(p->rgba)
								+(VECG(data->rgba[i])-VECG(p->rgba))/dif_frame;
							VECB(p->rgba)=VECB(p->rgba)
								+(VECB(data->rgba[i])-VECB(p->rgba))/dif_frame;
							VECA(p->rgba)=VECA(p->rgba)
								+(VECA(data->rgba[i])-VECA(p->rgba))/dif_frame;
							break;
						}
					}
				}
			}

			if(FLAG_TST(data->flag,PARTICLE_FLAG_DISTANCE)){
				FVECTOR4 tmp;
				SubFvector3(&tmp,&(p->p),&(p->init_p));
				if(LengthOfFvector3(&tmp)>p->enable_distance){
					p->status=PARTICLE_STATUS_OFF_SEQUENCE;
					p->frame=0;

					p->count=data->ending_time+data->rnd_ending_time*frnd();

					if(p->count==0){
						p->status=PARTICLE_STATUS_NONE;
						RemoveParticleSeqLink(p,particles);
						AddParticleNoseqLink(p,particles);
					}
					break;
				}
			}

			/* 残像作成 */
			for(i=0;i<N_AFTERIMAGE;i++){
				if(FLAG_TST(data->afterimage_flag,i)){
					SPR_PARTICLE *p2=MakeAfterimg(p,particles);
					if(p2!=NULL){
						p2->afterimg_index=i;
						p2->frame=-data->afterimage_frame[i];
						p2->count=data->afterimage_endframe[i];

						VECX(p2->sz)*=VECX(data->afterimage_sz_ratio[i]);
						VECY(p2->sz)*=VECY(data->afterimage_sz_ratio[i]);

						VECR(p2->rgba)*=VECX(data->afterimage_rgba_ratio[i]);
						VECG(p2->rgba)*=VECY(data->afterimage_rgba_ratio[i]);
						VECB(p2->rgba)*=VECZ(data->afterimage_rgba_ratio[i]);
						VECA(p2->rgba)*=VECW(data->afterimage_rgba_ratio[i]);
					}
				}
			}

			if(p->count>0){
				p->frame++;
				p->count--;

				if(p->count==0){
					p->status=PARTICLE_STATUS_OFF_SEQUENCE;
					p->frame=0;

					p->count=data->ending_time+data->rnd_ending_time*frnd();

					if(p->count==0){
						p->status=PARTICLE_STATUS_NONE;
						RemoveParticleSeqLink(p,particles);
						AddParticleNoseqLink(p,particles);
					}
					break;
				}
			}
			break;

		case PARTICLE_STATUS_OFF_SEQUENCE:
			CopyOld(p);

			{
				FVECTOR4 a;

				ClearFvector4(&a);
				if(FLAG_TST(data->flag,PARTICLE_FLAG_GRAVITY)){
					VECY(a)+=P_GRAVITY*data->gravity_parcent;
				}
				if(FLAG_TST(data->flag,PARTICLE_FLAG_V_REGIST)){
					FVECTOR4 v_reg;
					CopyFvector4(&v_reg,&(p->v));
					MulFvector3(&v_reg,&v_reg,data->v_regist);
					AddFvector3(&a,&a,&v_reg);
				}

				AddFvector3(&(p->p),&(p->p),&(p->v));
				AddFvector3(&(p->v),&(p->v),&a);

				if(FLAG_TST(data->sz_flag,5)){
					int dif_frame=p->count;

					VECX(p->sz)=VECX(p->sz)
						+(VECX(data->sz[5])-VECX(p->sz))/dif_frame;
					VECY(p->sz)=VECY(p->sz)
						+(VECY(data->sz[5])-VECY(p->sz))/dif_frame;
				}

				if(FLAG_TST(data->rgba_flag,5)){
					int dif_frame=p->count;

					VECR(p->rgba)=VECR(p->rgba)
						+(VECR(data->rgba[5])-VECR(p->rgba))/dif_frame;
					VECG(p->rgba)=VECG(p->rgba)
						+(VECG(data->rgba[5])-VECG(p->rgba))/dif_frame;
					VECB(p->rgba)=VECB(p->rgba)
						+(VECB(data->rgba[5])-VECB(p->rgba))/dif_frame;
					VECA(p->rgba)=VECA(p->rgba)
						+(VECA(data->rgba[5])-VECA(p->rgba))/dif_frame;
				}
			}

			/* 残像作成 */
			for(i=0;i<N_AFTERIMAGE;i++){
				if(FLAG_TST(data->afterimage_flag,i)){
					SPR_PARTICLE *p2=MakeAfterimg(p,particles);
					if(p2!=NULL){
						p2->afterimg_index=i;
						p2->frame=-data->afterimage_frame[i];
						p2->count=data->afterimage_endframe[i];

						VECX(p2->sz)*=VECX(data->afterimage_sz_ratio[i]);
						VECY(p2->sz)*=VECY(data->afterimage_sz_ratio[i]);

						VECR(p2->rgba)*=VECX(data->afterimage_rgba_ratio[i]);
						VECG(p2->rgba)*=VECY(data->afterimage_rgba_ratio[i]);
						VECB(p2->rgba)*=VECZ(data->afterimage_rgba_ratio[i]);
						VECA(p2->rgba)*=VECW(data->afterimage_rgba_ratio[i]);
					}
				}
			}

			if(p->count>0){
				p->frame++;
				p->count--;

				if(p->count==0){
					p->status=PARTICLE_STATUS_NONE;
					RemoveParticleSeqLink(p,particles);
					AddParticleNoseqLink(p,particles);
				}
			}
			break;

		case PARTICLE_STATUS_AFTERIMAGE:
			if(p->frame>=0){
				int dif_frame=p->count;

				VECX(p->sz)=VECX(p->sz)
					+(VECX(data->afterimage_sz[p->afterimg_index])-VECX(p->sz))/dif_frame;
				VECY(p->sz)=VECY(p->sz)
					+(VECY(data->afterimage_sz[p->afterimg_index])-VECY(p->sz))/dif_frame;

				VECR(p->rgba)=VECR(p->rgba)
					+(VECR(data->afterimage_rgba[p->afterimg_index])-VECR(p->rgba))/dif_frame;
				VECG(p->rgba)=VECG(p->rgba)
					+(VECG(data->afterimage_rgba[p->afterimg_index])-VECG(p->rgba))/dif_frame;
				VECB(p->rgba)=VECB(p->rgba)
					+(VECB(data->afterimage_rgba[p->afterimg_index])-VECB(p->rgba))/dif_frame;
				VECA(p->rgba)=VECA(p->rgba)
					+(VECA(data->afterimage_rgba[p->afterimg_index])-VECA(p->rgba))/dif_frame;

				if(p->count>0){
					p->count--;
					if(p->count==0){
						p->status=PARTICLE_STATUS_NONE;
						RemoveParticleSeqLink(p,particles);
						AddParticleNoseqLink(p,particles);
					}
				}
			}
			else p->frame++;
			break;
		}
	}
}

static void DispSprParticle(SPR_PARTICLES *particles)
{
	SPR_PARTICLE *p;
	DG_PRIM2 *prim=particles->prim;
	DG_TEX *tex=particles->tex;
	FVECTOR *pos;
	DG_PRIM2_UVRGBWH *uvrgb;
	int size=particles->com.size;
	int i;


	DG_SwitchBuffPrim2(prim);

	p=particles->particle;
	pos=prim->pos[prim->buffer_clock];
	uvrgb=(DG_PRIM2_UVRGBWH *)(prim->uvrgb[prim->buffer_clock]);

	for(i=size;i>0;i--,p++,pos++,uvrgb++){
		switch(p->status){
		case PARTICLE_STATUS_NONE:
			uvrgb->a=0;
			break;
		case PARTICLE_STATUS_ON_SEQUENCE:
		case PARTICLE_STATUS_OFF_SEQUENCE:
		case PARTICLE_STATUS_AFTERIMAGE:
			if(p->frame<0){
				uvrgb->a=0;
				break;
			}

			fpu_CopyVector(pos,&(p->p));
			uvrgb->u0=FTOI12(0.0F*tex->u_scale+tex->u_offset);
			uvrgb->v0=FTOI12(0.0F*tex->v_scale+tex->v_offset);
			uvrgb->q0=4096;
			uvrgb->f0=0x0fff;
			uvrgb->u1=FTOI12(1.0F*tex->u_scale+tex->u_offset);
			uvrgb->v1=FTOI12(1.0F*tex->v_scale+tex->v_offset);
			uvrgb->q1=4096;
			uvrgb->f1=0x0fff;
			uvrgb->w=p->sz.vx;
			uvrgb->h=p->sz.vy;
			uvrgb->r=p->rgba.r;
			uvrgb->g=p->rgba.g;
			uvrgb->b=p->rgba.b;
			uvrgb->a=(p->rgba.cd+1)/2;
			break;
		}
	}
}

static void DispLineParticle(SPR_PARTICLES *particles)
{
	SPR_PARTICLE *p;
	DG_PRIM2 *prim=particles->prim;
	// DG_TEX *tex=particles->tex;
	FVECTOR *pos;
	DG_PRIM2_UVRGB *uvrgb;
	DATA_PARTICLES *data=particles->data;
	int size=particles->com.size;
	int i;


	DG_SwitchBuffPrim2(prim);

	p=particles->particle;
	pos=prim->pos[prim->buffer_clock];
	uvrgb=(DG_PRIM2_UVRGB *)(prim->uvrgb[prim->buffer_clock]);

	for(i=size;i>0;i--,p++,pos+=2,uvrgb+=2){
		int index;

		switch(p->status){
		case PARTICLE_STATUS_NONE:
			(uvrgb+0)->a=0;
			(uvrgb+1)->a=0;
			break;
		case PARTICLE_STATUS_ON_SEQUENCE:
		case PARTICLE_STATUS_OFF_SEQUENCE:
		case PARTICLE_STATUS_AFTERIMAGE:
			if(p->frame<0){
				(uvrgb+0)->a=0;
				(uvrgb+1)->a=0;
				break;
			}

			index=p->mem_frames_index-data->late_frames;
			if(index<0) index+=MAX_MEMORY_FRAMES;

			fpu_CopyVector(pos+0,&(p->p));
			(uvrgb+0)->q=4096;
			(uvrgb+0)->f=0x8fff;
			(uvrgb+0)->r=p->rgba.r;
			(uvrgb+0)->g=p->rgba.g;
			(uvrgb+0)->b=p->rgba.b;
			(uvrgb+0)->a=(p->rgba.cd+1)/2;

			fpu_CopyVector(pos+1,&(p->oldp[index]));
			(uvrgb+1)->q=4096;
			(uvrgb+1)->f=0x0fff;
			(uvrgb+1)->r=p->oldrgba[index].r;
			(uvrgb+1)->g=p->oldrgba[index].g;
			(uvrgb+1)->b=p->oldrgba[index].b;
			(uvrgb+1)->a=(p->oldrgba[index].cd+1)/2;
			break;
		}
	}
}

static void DispPolyParticle(SPR_PARTICLES *particles)
{
	SPR_PARTICLE *p;
	DG_PRIM2 *prim=particles->prim;
	DG_TEX *tex=particles->tex;
	FVECTOR *pos;
	DG_PRIM2_UVRGB *uvrgb;
	DATA_PARTICLES *data=particles->data;
	int size=particles->com.size;
	int i;


	DG_SwitchBuffPrim2(prim);

	p=particles->particle;
	pos=prim->pos[prim->buffer_clock];
	uvrgb=(DG_PRIM2_UVRGB *)(prim->uvrgb[prim->buffer_clock]);

	for(i=size;i>0;i--,p++,pos+=4,uvrgb+=4){
		float l;
		int index;

		switch(p->status){
		case PARTICLE_STATUS_NONE:
			(uvrgb+0)->a=0;
			(uvrgb+1)->a=0;
			(uvrgb+2)->a=0;
			(uvrgb+3)->a=0;
			break;
		case PARTICLE_STATUS_ON_SEQUENCE:
		case PARTICLE_STATUS_OFF_SEQUENCE:
		case PARTICLE_STATUS_AFTERIMAGE:
			// printf("111\n");

			if(p->frame<0){
				(uvrgb+0)->a=0;
				(uvrgb+1)->a=0;
				(uvrgb+2)->a=0;
				(uvrgb+3)->a=0;
				break;
			}

			// printf("Display\n");

			index=p->mem_frames_index-data->late_frames;
			if(index<0) index+=MAX_MEMORY_FRAMES;

			// printf("index = %d %d %d\n",p->mem_frames_index,data->late_frames,index);

			vu0_Ldv0(&(p->v));
			vu0_Ldv1((FVECTOR *)&(DG_Chanl(DG_CHANL_MAIN)->eye.m[2][0]));
			vu0_Ldv2(&(p->p));

			vu0_OuterProductv0v1();
			l=vu0_VectorLength2v0();
			l=fpu_Rsqrt(l,p->sz.vx);
			vu0_Mulv0a(l);
			vu0_Subv1v2v0();
			vu0_Stv1(pos+0);
			vu0_Addv1v2v0();
			vu0_Stv1(pos+1);

			vu0_Mulv0a(p->oldsz[index].vx/p->sz.vx);
			vu0_Ldv2(&(p->oldp[index]));
			vu0_Subv1v2v0();
			vu0_Stv1(pos+2);
			vu0_Addv1v2v0();
			vu0_Stv1(pos+3);

			(uvrgb+0)->u=FTOI12(0.0F*tex->u_scale+tex->u_offset);
			(uvrgb+0)->v=FTOI12(0.0F*tex->v_scale+tex->v_offset);
			(uvrgb+0)->q=4096;
			(uvrgb+0)->f=0x8fff;
			(uvrgb+0)->r=p->rgba.r;
			(uvrgb+0)->g=p->rgba.g;
			(uvrgb+0)->b=p->rgba.b;
			(uvrgb+0)->a=(p->rgba.cd+1)/2;

			(uvrgb+1)->u=FTOI12(1.0F*tex->u_scale+tex->u_offset);
			(uvrgb+1)->v=FTOI12(0.0F*tex->v_scale+tex->v_offset);
			(uvrgb+1)->q=4096;
			(uvrgb+1)->f=0x8fff;
			(uvrgb+1)->r=p->rgba.r;
			(uvrgb+1)->g=p->rgba.g;
			(uvrgb+1)->b=p->rgba.b;
			(uvrgb+1)->a=(p->rgba.cd+1)/2;

			(uvrgb+2)->u=FTOI12(0.0F*tex->u_scale+tex->u_offset);
			(uvrgb+2)->v=FTOI12(1.0F*tex->v_scale+tex->v_offset);
			(uvrgb+2)->q=4096;
			(uvrgb+2)->f=0x0fff;
			(uvrgb+2)->r=p->oldrgba[index].r;
			(uvrgb+2)->g=p->oldrgba[index].g;
			(uvrgb+2)->b=p->oldrgba[index].b;
			(uvrgb+2)->a=(p->oldrgba[index].cd+1)/2;

			(uvrgb+3)->u=FTOI12(1.0F*tex->u_scale+tex->u_offset);
			(uvrgb+3)->v=FTOI12(1.0F*tex->v_scale+tex->v_offset);
			(uvrgb+3)->q=4096;
			(uvrgb+3)->f=0x0fff;
			(uvrgb+3)->r=p->oldrgba[index].r;
			(uvrgb+3)->g=p->oldrgba[index].g;
			(uvrgb+3)->b=p->oldrgba[index].b;
			(uvrgb+3)->a=(p->oldrgba[index].cd+1)/2;
			break;
		}
	}
}

static void DispPolyDParticle(SPR_PARTICLES *particles)
{
	SPR_PARTICLE *p;
	DG_PRIM2 *prim=particles->prim;
	DG_TEX *tex=particles->tex;
	FVECTOR *pos;
	DG_PRIM2_UVRGB *uvrgb;
	DATA_PARTICLES *data=particles->data;
	int size=particles->com.size;
	int i;


	DG_SwitchBuffPrim2(prim);

	p=particles->particle;
	pos=prim->pos[prim->buffer_clock];
	uvrgb=(DG_PRIM2_UVRGB *)(prim->uvrgb[prim->buffer_clock]);

	for(i=size;i>0;i--,p++,pos+=4,uvrgb+=4){
		float l;
		int index;

		switch(p->status){
		case PARTICLE_STATUS_NONE:
			(uvrgb+0)->a=0;
			(uvrgb+1)->a=0;
			(uvrgb+2)->a=0;
			(uvrgb+3)->a=0;
			break;
		case PARTICLE_STATUS_ON_SEQUENCE:
		case PARTICLE_STATUS_OFF_SEQUENCE:
		case PARTICLE_STATUS_AFTERIMAGE:
			if(p->frame<0){
				(uvrgb+0)->a=0;
				(uvrgb+1)->a=0;
				(uvrgb+2)->a=0;
				(uvrgb+3)->a=0;
				break;
			}

			index=p->mem_frames_index-data->late_frames;
			if(index<0) index+=MAX_MEMORY_FRAMES;

			vu0_Ldv0(&(p->v));
			vu0_Ldv1((FVECTOR *)&(DG_Chanl(DG_CHANL_MAIN)->eye.m[2][0]));
			vu0_Ldv2(&(p->p));

			vu0_OuterProductv0v1();
			l=vu0_VectorLength2v0();

			vu0_Ldv1(&(p->oldp[index]));
			vu0_Stv2(pos+0);
			vu0_Addv2v1();
			vu0_Stv1(pos+3);
			vu0_Mulv2a(0.5f);

			l=fpu_Rsqrt(l,p->sz.vx);
			vu0_Mulv0a(l);

			vu0_Addv1v2v0();
			vu0_Stv1(pos+1);
			vu0_Subv1v2v0();
			vu0_Stv1(pos+2);

			(uvrgb+0)->u=FTOI12(0.0F*tex->u_scale+tex->u_offset);
			(uvrgb+0)->v=FTOI12(0.0F*tex->v_scale+tex->v_offset);
			(uvrgb+0)->q=4096;
			(uvrgb+0)->f=0x8fff;
			(uvrgb+0)->r=p->rgba.r;
			(uvrgb+0)->g=p->rgba.g;
			(uvrgb+0)->b=p->rgba.b;
			(uvrgb+0)->a=(p->rgba.cd+1)/2;

			(uvrgb+1)->u=FTOI12(1.0F*tex->u_scale+tex->u_offset);
			(uvrgb+1)->v=FTOI12(0.0F*tex->v_scale+tex->v_offset);
			(uvrgb+1)->q=4096;
			(uvrgb+1)->f=0x8fff;
			(uvrgb+1)->r=p->rgba.r;
			(uvrgb+1)->g=p->rgba.g;
			(uvrgb+1)->b=p->rgba.b;
			(uvrgb+1)->a=(p->rgba.cd+1)/2;

			(uvrgb+2)->u=FTOI12(0.0F*tex->u_scale+tex->u_offset);
			(uvrgb+2)->v=FTOI12(1.0F*tex->v_scale+tex->v_offset);
			(uvrgb+2)->q=4096;
			(uvrgb+2)->f=0x0fff;
			(uvrgb+2)->r=p->rgba.r;
			(uvrgb+2)->g=p->rgba.g;
			(uvrgb+2)->b=p->rgba.b;
			(uvrgb+2)->a=(p->rgba.cd+1)/2;

			(uvrgb+3)->u=FTOI12(1.0F*tex->u_scale+tex->u_offset);
			(uvrgb+3)->v=FTOI12(1.0F*tex->v_scale+tex->v_offset);
			(uvrgb+3)->q=4096;
			(uvrgb+3)->f=0x0fff;
			(uvrgb+3)->r=p->oldrgba[index].r;
			(uvrgb+3)->g=p->oldrgba[index].g;
			(uvrgb+3)->b=p->oldrgba[index].b;
			(uvrgb+3)->a=(p->oldrgba[index].cd+1)/2;
			break;
		}
	}
}

static int CountSprParticle(SPR_PARTICLES *particles)
{
    if(particles->count>0){
		particles->frame++;
		particles->count--;

		if(particles->count==0){
			DATA_PARTICLES *data=particles->data;

			// printf("Loop Check !\n");

			if(FLAG_TST(data->flag,PARTICLE_FLAG_LOOP_TERM)){
				RestartSimSprParticle(particles);

				// printf("Loop !!!\n");
			}
			else{
				RestartSimSprParticle(particles);
				particles->count=0;
				return 1;
			}
		}
    }
	return 0;
}

int SimParticle(COM_PARTICLES *com)
{
	int ans=-1;

	if(com==NULL) return -1;

	if(((SPR_PARTICLES *)com)->count==0) return 1;

	switch(com->type){
	case PARTICLE_TYPE_SPRITE:
		SimSprParticle((SPR_PARTICLES *)com);
		BornSprParticle((SPR_PARTICLES *)com);
		DispSprParticle((SPR_PARTICLES *)com);
		CountSprParticle((SPR_PARTICLES *)com);
		break;
	case PARTICLE_TYPE_LINE:
		SimSprParticle((SPR_PARTICLES *)com);
		BornSprParticle((SPR_PARTICLES *)com);
		DispLineParticle((SPR_PARTICLES *)com);
		CountSprParticle((SPR_PARTICLES *)com);
		break;
	case PARTICLE_TYPE_POLY:
		SimSprParticle((SPR_PARTICLES *)com);
		BornSprParticle((SPR_PARTICLES *)com);
		DispPolyParticle((SPR_PARTICLES *)com);
		CountSprParticle((SPR_PARTICLES *)com);
		break;
	case PARTICLE_TYPE_POLY_DIAMOND:
		SimSprParticle((SPR_PARTICLES *)com);
		BornSprParticle((SPR_PARTICLES *)com);
		DispPolyDParticle((SPR_PARTICLES *)com);
		CountSprParticle((SPR_PARTICLES *)com);
		break;
	}

	return ans;
}


/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */

typedef struct {
	GV_ACT_EX			actor;
	COM_PARTICLES	*particle;
} Work;

static void SprParticle_Free(SPR_PARTICLES *spr)
{
	if(spr==NULL) return;
	if(spr->prim!=NULL) GM_FreePrim2(spr->prim);
	if(spr->particle!=NULL) GV_Free(spr->particle);
	GV_Free(spr);
}

static SPR_PARTICLES *SprParticle_Alloc(int size)
{
	DG_PRIM2 *prim;
	SPR_PARTICLE *part;
	SPR_PARTICLES *spr;

	if((spr=GV_Malloc(sizeof(SPR_PARTICLES)))==NULL){
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

	spr->data=NULL;
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

	if((spr=GV_Malloc(sizeof(SPR_PARTICLES)))==NULL){
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

	spr->data=NULL;
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

	if((spr=GV_Malloc(sizeof(SPR_PARTICLES)))==NULL){
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

	spr->data=NULL;
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


#if 0

static SPR_PARTICLES *PolyDParticle_Alloc(int size)
{
	SPR_PARTICLES *poly;
	poly=PolyParticle_Alloc(size);
	if(poly!=NULL){
		poly->com.type=PARTICLE_TYPE_POLY_DIAMOND;
	}
	return poly;
}

#endif


/* ----------------------------------------------------------------- */
/* ----------------------------------------------------------------- */

static void Act(Work *work)
{
	if(SimParticle(work->particle)){
#if 0
		/* Loop */
		RestartSimParticle(work->particle);
#endif

#if 1
		/* Finish */
		GV_DestroyActor(work);
#endif

	}
}

static void Die(Work *work)
{
	switch(work->particle->type){
	case PARTICLE_TYPE_SPRITE:
	case PARTICLE_TYPE_LINE:
	case PARTICLE_TYPE_POLY:
	case PARTICLE_TYPE_POLY_DIAMOND:
		SprParticle_Free((SPR_PARTICLES *)(work->particle));
		break;
	}
}

static int GetResource_called(Work *work,int particle_name,
							  FVECTOR *pos,FVECTOR *v,int v_flag,FMATRIX *tmat)
{
	DATA_PARTICLES *data;

#if 0
	data=(DATA_PARTICLES *)GV_GetCache(GV_CacheID(particle_name,'p'));
#else
	data=(DATA_PARTICLES *)GV_GetCache(GV_CacheID(particle_name,'r'));
#endif

	if(data==NULL){ printf("ERR! Cannot Find File\n!!");return 0; }

	switch(data->type){
	case PARTICLE_TYPE_SPRITE:
		{
			SPR_PARTICLES *spr;
			DG_TEX *tex;

			work->particle=(COM_PARTICLES *)(spr=SprParticle_Alloc(data->size));
			if(spr==NULL) return 0;

			/* プリミティブへテクスチャをセット */
			tex=DG_GetTexture(data->tex_strcode);
			if(tex==NULL) return 0;

			DG_ConfigPrim2Tex(spr->prim,tex);
			spr->tex=tex;

			spr->data=data;
			spr->tmat=tmat;
			if(pos==NULL) fpu_ClearVector(&(spr->pos));
			else fpu_CopyVector(&(spr->pos),pos);
			if(v==NULL) fpu_ClearVector(&(spr->init_v));
			else fpu_CopyVector(&(spr->init_v),v);
			spr->init_v_flag=v_flag;

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
		return 1;

	case PARTICLE_TYPE_LINE:
		{
			SPR_PARTICLES *spr;

			work->particle=(COM_PARTICLES *)(spr=LineParticle_Alloc(data->size));
			if(spr==NULL) return 0;

			spr->data=data;
			spr->tmat=tmat;
			if(pos==NULL) fpu_ClearVector(&(spr->pos));
			else fpu_CopyVector(&(spr->pos),pos);
			if(v==NULL) fpu_ClearVector(&(spr->init_v));
			else fpu_CopyVector(&(spr->init_v),v);
			spr->init_v_flag=v_flag;

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
		return 1;

	case PARTICLE_TYPE_POLY:
		{
			SPR_PARTICLES *spr;
			DG_TEX *tex;

			work->particle=(COM_PARTICLES *)(spr=PolyParticle_Alloc(data->size));
			if(spr==NULL) return 0;

			/* プリミティブへテクスチャをセット */
			tex=DG_GetTexture(data->tex_strcode);
			if(tex==NULL) return 0;

			DG_ConfigPrim2Tex(spr->prim,tex);
			spr->tex=tex;

			spr->data=data;
			spr->tmat=tmat;
			if(pos==NULL) fpu_ClearVector(&(spr->pos));
			else fpu_CopyVector(&(spr->pos),pos);
			if(v==NULL) fpu_ClearVector(&(spr->init_v));
			else fpu_CopyVector(&(spr->init_v),v);
			spr->init_v_flag=v_flag;

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
		return 1;

	case PARTICLE_TYPE_POLY_DIAMOND:
		{
			SPR_PARTICLES *spr;
			DG_TEX *tex;

			work->particle=(COM_PARTICLES *)(spr=PolyParticle_Alloc(data->size));
			if(spr==NULL) return 0;

			/* プリミティブへテクスチャをセット */
			tex=DG_GetTexture(data->tex_strcode);
			if(tex==NULL) return 0;

			DG_ConfigPrim2Tex(spr->prim,tex);
			spr->tex=tex;

			spr->data=data;
			spr->tmat=tmat;
			if(pos==NULL) fpu_ClearVector(&(spr->pos));
			else fpu_CopyVector(&(spr->pos),pos);
			if(v==NULL) fpu_ClearVector(&(spr->init_v));
			else fpu_CopyVector(&(spr->init_v),v);
			spr->init_v_flag=v_flag;

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
		return 1;

	default:
		break;
	}

	return 0;
}

void *NewToolParticle_called(int exec_level,int particle_name,FVECTOR *pos,FVECTOR *v,int v_flag,
							 FMATRIX *tmat)
{
	Work *work;

    work=(Work *)GV_NewActor(exec_level,sizeof(Work));
    if(work!=NULL){
		GV_SetActor(&(work->actor),Act,Die);
		if(!GetResource_called(work,particle_name,pos,v,v_flag,tmat)){
			GV_DestroyActor(work);
			return NULL;
		}

		RestartSimParticle(work->particle);
    }
	return (void *)work;
}

void *NewToolParticle_DemoLink(int particle_name,FVECTOR *pos, FMATRIX *tmat)
{
	Work *work;

    work=(Work *)GV_NewActor(GV_ACTOR_EFFECT,sizeof(Work));
    if(work!=NULL){
		GV_SetActor(&(work->actor),Act,Die);
		GV_ActorEX( &work->actor );
		if(!GetResource_called(work,particle_name,pos,NULL,3,tmat)){
			GV_DestroyActor(work);
			return NULL;
		}

		RestartSimParticle(work->particle);
    }
	return (void *)work;
}

void *NewToolParticle_DemoFix(int particle_name,FVECTOR *pos )
{
	Work *work;

    work=(Work *)GV_NewActor(GV_ACTOR_EFFECT,sizeof(Work));
    if(work!=NULL){
		GV_SetActor(&(work->actor),Act,Die);
		GV_ActorEX( &work->actor );
		if(!GetResource_called(work,particle_name,pos,NULL,0,NULL)){
			GV_DestroyActor(work);
			return NULL;
		}

		RestartSimParticle(work->particle);
    }
	return (void *)work;
}
