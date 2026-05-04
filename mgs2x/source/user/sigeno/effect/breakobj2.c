//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breakobj2.c
	モデルを分解飛散
	2002/04/05 K.Sigeno
	$Id: breakobj2.c,v 1.1.1.3 2002/11/19 11:49:45 Yoshizawa1 Exp $
*/

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

#include "../effect/sig_vanim.h"


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

#define NORMAL_MODE			(0x00000000)
#define AMB_MODE			(0x00000001)
#define BUILD_Y				(0x00000002)
#define BUILD_UP			(0x00000004)
#define NO_CHECK_SW			(0x00000008)
#define DISAPPEAR_MODE		(0x00000010)
#define BODY_MODE			(0x00000020)
#define BUILD_X				(0x00000040)
#define BUILD_ROLL			(0x00000080)
#define BUILD_REV			(0x00000100) /*Y軸下から飛んでくる*/
#define MOT_TR				(0x00000200)
#define RAND_VERTS			(0x00000400)
#define BUILD_UP_Z			(0x00000800)
#define PURE_NORM			(0x00001000)
#define NO_MAKE_OBJ			(0x00002000)
#define STOP_END			(0x00004000)

enum {
	ST_WAIT = 0x0001,
};


//#define	MAX_DIF		(5000)
#define	MAX_DIF		(-10000)
#define CYCLE		(30)
#define RATE_VY		(250.0f)
/* ワーク */
typedef	struct SBreakObj2WorkTag
{
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	*vanim;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	FMATRIX		*world ;

	FVECTOR 	shift ;
	SVECTOR 	rot ;

	FVECTOR		pos;
	float		min ;
	float		max ;
	float		len ;
	float		rate ;
	int			cnt ;
	int			time_a ;
	int			time_b ;
	int			delay ;
	int			mode ;
	int			status ;
	int			*sw ;
	int			dummy_sw ;
}
SBreakObj2Work;

#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)
#if 0
extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
#endif

static void SetAmb(SBreakObj2Work *work,u_char r,u_char g,u_char b){
	work->lights[1].m[ 3 ][ 0 ] = (float) r ;
	work->lights[1].m[ 3 ][ 1 ] = (float) g ;
	work->lights[1].m[ 3 ][ 2 ] = (float) b ;
}
#define NORM_LEN (4096.0f)
#define Q_WORD (2)


static void SetBoundScale(SBreakObj2Work *work,DG_OBJS *objs ,float size) {
	int j;
	DG_OBJ	*obj;

	if(work->mode & BUILD_UP_Z){
		work->min = objs->bound_min.vz ;
		work->max = objs->bound_max.vz ;
	}else {
		work->min = objs->bound_min.vy ;
		work->max = objs->bound_max.vy ;
	}
	if(work->mode &	NO_MAKE_OBJ){
//		return ;
	}
	/*バウンディングボックスも拡大*/
	objs->bound_min.vx -= size ;
	objs->bound_min.vy -= size ;
	objs->bound_min.vz -= size ;
	objs->bound_max.vx += size ;
	objs->bound_max.vy += size ;
	objs->bound_max.vz += size ;

	for(j= 0 ;j<objs->n_models;j++){
		obj = &objs->objs[j] ;
		obj->bound_min.vx -= size ;
		obj->bound_min.vy -= size ;
		obj->bound_min.vz -= size ;
		obj->bound_max.vx += size ;
		obj->bound_max.vy += size ;
		obj->bound_max.vz += size ;
	}
}
static inline int SetDelayCnt(int cnt ,SVECTOR *org_sv,SBreakObj2Work *work){
	int tmp_cnt ;
	tmp_cnt = cnt ;
//BUILD_Y			(0x00000002)
//BUILD_UP		(0x00000004)

	if(work->mode & BUILD_UP_Z){
		if(work->mode & BUILD_UP){
			tmp_cnt -= (org_sv->vz - work->min)/work->rate  ;
		}else {
			tmp_cnt -= (work->max - org_sv->vz )/work->rate  ;
		}
	}else {
		if(work->mode & BUILD_UP){
			tmp_cnt -= (org_sv->vy - work->min)/work->rate  ;
		}else {
			tmp_cnt -= (work->max - org_sv->vy )/work->rate  ;
		}
	}
	if(tmp_cnt<0) tmp_cnt = 0 ;
	if(tmp_cnt>work->time_a) tmp_cnt = work->time_a ;
	return tmp_cnt ;
}


static inline void MakeVerts(SBreakObj2Work *work , SVECTOR *buf,SVECTOR *norms,int speed ){
	FVECTOR tmp ;
	if(work->mode & BUILD_Y){
		tmp.vx=tmp.vz = 0; 
		if(work->mode & BUILD_REV){
			tmp.vy = speed ;
		}else {
			tmp.vy = -speed ;
		}
		tmp.vw = 1.0f;
		DG_RotVector(&tmp,&tmp,1) ;
		buf->vx = tmp.vx ;
		buf->vy = tmp.vy ;
		buf->vz = tmp.vz ;

	}else if(work->mode & RAND_VERTS){
		SVECTOR  test ;
		test.vx = ((irnd()>>8)& 1023) - 512 ;
		test.vy = ((irnd()>>8)& 1023) - 512;
		test.vz = ((irnd()>>8)& 1023) - 512;
		GV_LenVec3( &test, buf, 1.0f, speed ) ;
	}else {
		//法線
		GV_LenVec3( norms, buf, 1.0f, speed ) ;
		tmp.vx = buf->vx ;
		if(work->mode & PURE_NORM){
			tmp.vy = buf->vy ;
		}else {
			if(work->mode & BUILD_REV){
				tmp.vy = buf->vy + speed ;
			}else {
				tmp.vy = buf->vy - speed ;
			}
		}
		tmp.vz = buf->vz ;
//		DG_RotVector(&tmp,&tmp,1) ;
		buf->vx = tmp.vx ;
		buf->vy = tmp.vy ;
		buf->vz = tmp.vz ;

	}
}

static inline void MakePolyScale(SBreakObj2Work *work ,float *rate ,float *rate_dif,int tmp_cnt){
	*rate = (float)tmp_cnt/(float)work->time_a ;
	*rate_dif = 1.0f - *rate ;
}

#define BUILD_MODE (BUILD_NORM)
static void SIG_ActVanimObjBuild(SBreakObj2Work *work,int cnt) {
//xbox
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;

	FMATRIX	world ;
	DG_OBJS	*objs ;
	float rate,rate_dif ;
	float	tmp_speed;
	int n_verts,i,j,k,l,kick,tmp_cnt;
	int	verts_off,norm_off, speed ;
	DG_OBJ	*obj;
	FVECTOR	vel;


#ifdef PSX2
	SVECTOR *sv,*org_sv,*trg_sv,buf;
#else
//xbox
	DG_VERTEX_KMSM *sv ;
	SVECTOR *org_sv,*trg_sv,buf;
#endif



	SVECTOR *norms ;	/* 法線配列バッファ */
	short	n_packs ;
	DG_OBJ_PACKET	*packets ;
	objs = work->objs ;
//printf("cnt %d\n",cnt);
	if(work->mode & BUILD_Y){
		_sceVu0InversMatrix(&world ,&objs->objs[0].world) ;
		DG_SetPos(&world) ;
	}
	for(j= 0 ;j<objs->n_models;j++){

#ifndef PSX2
		sv = work->vanim[j].verts_top[work->vanim[j].buffer_clock];
#endif
//xbox
		mdl = work->vanim[j].obj->model;
		mdl_pack = mdl->packs;


		n_packs = objs->objs[j].n_packs ;
		verts_off = 0 ;
		norm_off = 0 ;


//xbox
		for(k=0;k<mdl->n_packs;k++){
			if( 1){
#ifdef PSX2
				sv = &objs->objs[j].verts[verts_off*Q_WORD] ;
#endif

				org_sv = (SVECTOR*)objs->objs[j].model->packs[k].verts ;
				norms = (SVECTOR*)objs->objs[j].model->packs[k].norms ;

				n_verts = objs->objs[j].packets[k].n_verts ;
//				tmp_cnt = SetDelayCnt(cnt ,org_sv,z_min) ;
				tmp_cnt = SetDelayCnt(cnt ,org_sv,work) ;

				speed = work->len*(float)((work->time_a-tmp_cnt)*
					(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;

#if 1
				MakeVerts(work,&buf,norms,speed)  ;
#else
				if(work->mode & BUILD_Y){
					buf.vx=buf.vz = 0; buf.vy = -speed ;
				}else if(work->mode & RAND_VERTS){
					SVECTOR  test ;
					test.vx = ((irnd()>>8)& 1023) - 512 ;
					test.vy = ((irnd()>>8)& 1023) - 512;
					test.vz = ((irnd()>>8)& 1023) - 512;

					GV_LenVec3( &test, &buf, 1.0f, speed ) ;
				}else {
					GV_LenVec3( norms, &buf, 1.0f, speed ) ;
				}
#endif

#if 1
				MakePolyScale(work ,&rate ,&rate_dif,tmp_cnt) ;
#else
				rate = (float)tmp_cnt/(float)work->time_a ;
				if(rate > 0.8f) rate = 1.0f ;
				rate_dif = 1.0f - rate ;
#endif
				kick = 1 ;
				trg_sv = sv ;

//xbox
				for (i=0;i<mdl_pack->n_verts;i++){
					/*頂点キックのみが2回続くならポリゴン切断と判断*/
#if 1
					if((norms->pad&0xffff)== 0x8fff){
						if(i<(n_verts-1)){
							norms++ ;
							if((norms->pad&0xffff)== 0x8fff){
#if 1
								kick++ ;
								trg_sv = sv ;
								tmp_cnt = SetDelayCnt(cnt ,org_sv,work) ;
								speed = work->len*(float)((work->time_a-tmp_cnt)*(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;

								MakeVerts(work,&buf,norms,speed)  ;

								MakePolyScale(work ,&rate ,&rate_dif,tmp_cnt) ;

#endif
							}
							norms-- ;
						}
					}
#endif
	//printf(" norms pad[%x]\n",norms->pad) ;
#if 1
					if(tmp_cnt == work->time_a){
						sv->vx = org_sv->vx;
						sv->vy = org_sv->vy;
						sv->vz = org_sv->vz;
					}else {
						if(kick){
							sv->vx = org_sv->vx - buf.vx ;
							sv->vy = org_sv->vy - buf.vy ;
							sv->vz = org_sv->vz - buf.vz ;
						}else {
							sv->vx = org_sv->vx - buf.vx ;
							sv->vy = org_sv->vy - buf.vy ;
							sv->vz = org_sv->vz - buf.vz ;
#if 1
//縮め
							sv->vx = (float) sv->vx*rate + (float)trg_sv->vx * rate_dif ;
							sv->vy = (float) sv->vy*rate + (float)trg_sv->vy * rate_dif ;
							sv->vz = (float) sv->vz*rate + (float)trg_sv->vz * rate_dif ;
#endif
						}
					}
#endif
//					sv->pad = 0x1000 ;
	//	short	w ; ウェイト値（固定小数点(1.3.12)）
	//				ウェイト値は1.0でその関節に100%、0.0で親オブジェクトに100%になる
	//				printf("wait [%x]\n",sv->pad ) ;
					org_sv++ ;
#ifdef PSX2
					sv++ ;
#else
//xbox
					sv = (void*)( (char*)sv + mdl->stride );
#endif
					norms++ ;
					kick = 0 ;
				}
			}
			verts_off += objs->objs[j].packets[k].verts_offset ;
			norm_off += objs->objs[j].packets[k].norms_offset ;
//xbox
			mdl_pack++ ;

		}
	}
}
static void Act(SBreakObj2Work *work)
{
	int cnt ;
	FVECTOR scale ;
	cnt = work->cnt ;
	scale.vx = scale.vy = scale.vz = 0.90f;
	if(!(work->status & ST_WAIT)){
		SIG_ActVanimObjBuild(work,cnt) ;
	}
	if(!(work->mode &NO_MAKE_OBJ)){
		if(work->mode & AMB_MODE){
			u_char col;
			col = 255*((work->time_a+work->time_b)-work->cnt)/(work->time_a+work->time_b) ;
			SetAmb(work,col,col,col) ;
		}else {
			DG_GetLightMatrix( (FVECTOR *)&work->world->m[3], work->lights );
		}
	}
	if(!(work->mode &NO_MAKE_OBJ)){
		if((work->mode & MOT_TR)){
			DG_SetPos( work->world ) ;
			DG_MovePos(&work->shift) ;
			DG_RotatePos(&work->rot) ;
			if(work->mode & AMB_MODE){
				DG_ScalePos(&scale) ;
			}
			DG_PutObjs( work->objs );
		}
	}

#if 0
	if(GV_PadData[ 0 ].status & PAD_U){
		work->cnt++; 
	}
	if(GV_PadData[ 0 ].status & PAD_D){
		work->cnt--; 
	}
	if(work->cnt<0) work->cnt=0 ;
#else

	if(work->delay >0){
		work->delay--;
	}else {
		if(work->mode &DISAPPEAR_MODE){
			if(work->cnt >= 0){
				work->cnt-- ;
			}
		}else {
			if(work->cnt <= (work->time_a+work->time_b)){
				work->cnt++ ;
			}
		}
	}

#endif


#if 1
//NO_CHECK_SW

	if(
	((!(work->mode & DISAPPEAR_MODE))&&(work->cnt > (work->time_a+work->time_b)))
//	||(((work->mode & DISAPPEAR_MODE))&&(work->cnt <= 0))
	||(((work->mode & DISAPPEAR_MODE))&&(work->cnt < 0))
	||((*work->sw <0)&&(!(work->mode & NO_CHECK_SW)))
	){
//		DG_InvisibleObjs(work->objs) ;
		work->status |= ST_WAIT ;

      // Armature fix:
      // allow the box to be deleted when it finishes animating
      // fixes MGSTWO-2397
      if (!strcmp(GM_GetArea(), "tsp03a"))
      {
         work->vanim->obj->flag |= DG_FLAG_INVISIBLE;
      }
      
      if (!(work->mode & STOP_END))
      {
         GV_DestroyActor(work) ;
		}
	}
#endif
}
static void Die(SBreakObj2Work *work)
{

	if(work->vanim != NULL){
		SIG_FreeVanim(work->vanim,work->objs->n_models) ;
	}
	if(!(work->mode &	NO_MAKE_OBJ)){
		if(work->objs != NULL){
			DG_DequeueObjs( work->objs ) ;
			DG_FreeObjs( work->objs );
		}
	}
	GV_Free(work->vanim) ;
}

//#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT)
#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

static int GetResources(SBreakObj2Work *work,DG_DEF *model,FMATRIX *world,int mode ,float len,int time_a,int time_b,int *sw)
{
	DG_DEF	*def ;
	int i ;
	FVECTOR		pos;

	work->world = world ;
	work->mode = mode ;
	work->dummy_sw = 0 ;
	work->status = 0 ;
	def = model ;
	if(!(work->mode &	NO_MAKE_OBJ)){
		work->objs = DG_MakeObjs( def, MDL_FLAG, DG_CHANL_MAIN ) ;
		if(work->objs == NULL) return 0 ;
		DG_QueueObjs( work->objs ) ;
		DG_SetPos( world ) ;
		DG_MovePos(&work->shift) ;
		DG_RotatePos(&work->rot) ;
		DG_PutObjs( work->objs );
		DG_SetLightMatrix( work->objs, work->lights );
	    /* プリシェード */
		if(work->mode & AMB_MODE){
			SetAmb(work,255,255,255) ;
		}else {
			DG_GetLightMatrix( (FVECTOR *)&work->world->m[3], work->lights );
		}
	}

	if((work->mode & NO_CHECK_SW)||(sw==NULL)){
		work->sw = &work->dummy_sw ;
	}else {
		work->sw = sw ;
	}
	work->len = len ;
	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->objs->n_models );
	if(work->vanim == NULL ) return 0;
	SIG_InitVanim(work->vanim,work->objs,work->objs->n_models) ;

	SetBoundScale(work,work->objs ,fabs(len)) ;
	work->time_a = DIRECT_TICK(time_a) ;
	work->time_b = DIRECT_TICK(time_b) ;

	if(work->mode &DISAPPEAR_MODE){
		work->cnt = (work->time_a+work->time_b) ;
	}else {
		work->cnt = 0 ;
	}
	work->rate = (work->max - work->min) / (float)work->time_b ;


//	void *NewObjectOutline(
//	int     cache,		/* キャッシュＩＤ */
//	DG_OBJS *dg_objs)	/* ＤＧオブジェクト */
	SIG_ActVanimObjBuild(work,work->cnt) ;

	return 1;
}

void *NewSigBreakObj2(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,int time_b,int *sw ){
	SBreakObj2Work *work ;
	DG_DEF *def ;
	work = (SBreakObj2Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( SBreakObj2Work ), 0 ) ;
	if(work!=NULL) {
		work->shift = DG_ZeroVector ;
		work->rot = DG_ZeroSVector ;
		work->delay = 0;

		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		def = (DG_DEF*) GV_GetCache( GV_CacheID( mdlcode, 'k' ) ) ;
		if(!GetResources(work,def,world,mode ,len,time_a,time_b,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewSigBreakObj3(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,
	int delay,int *sw,FVECTOR *shift,SVECTOR *rot
){
	SBreakObj2Work *work ;
	DG_DEF *def ;

	work = (SBreakObj2Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( SBreakObj2Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = *shift ;
		work->rot = *rot ;
		work->delay = DIRECT_TICK(delay);
		def = (DG_DEF*) GV_GetCache( GV_CacheID( mdlcode, 'k' ) ) ;

		if(!GetResources(work,def,world,mode ,len,time_a,0,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewSigBreakObj4(DG_OBJS *objs,FMATRIX *world,int mode ,float len,int time_a ,
	int delay,int *sw,FVECTOR *shift,SVECTOR *rot)
{
	SBreakObj2Work *work ;
	DG_DEF *def ;
	work = (SBreakObj2Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( SBreakObj2Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = *shift ;
		work->rot = *rot ;
		work->delay = DIRECT_TICK(delay);
		def = objs->def ;
		if(!GetResources(work,def,world,mode ,len,time_a,0,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/*NO_MAKE_OBJ*/
void *NewSigBreakObj5(DG_OBJS *objs,int mode ,float len,int time_a ,int delay,int *sw)
{
	SBreakObj2Work *work ;
	DG_DEF *def ;
	work = (SBreakObj2Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( SBreakObj2Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = DG_ZeroVector ;
		work->rot = DG_ZeroSVector ;
		work->delay = DIRECT_TICK(delay);
		work->objs = objs;
		def = objs->def ;
		if(!GetResources(work,def,NULL,mode|NO_MAKE_OBJ|NO_CHECK_SW ,len,time_a,0,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void *NewSigBreakObj6(DG_OBJS *objs,float len,int time_a ,int time_b)
{
	SBreakObj2Work *work ;
	DG_DEF *def ;
	int mode = (BUILD_UP|BUILD_Y|DISAPPEAR_MODE|STOP_END);
	work = (SBreakObj2Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( SBreakObj2Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = DG_ZeroVector ;
		work->rot = DG_ZeroSVector ;
		work->delay = 0;
		work->objs = objs;
		def = objs->def ;
		if(!GetResources(work,def,NULL,mode|NO_MAKE_OBJ|NO_CHECK_SW ,len,time_a,time_b,NULL)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewSigBreakObj7(DG_OBJS *objs,float len,int time_a ,int time_b)
{
	SBreakObj2Work *work ;
	DG_DEF *def ;
	int mode = (BUILD_UP_Z|DISAPPEAR_MODE|STOP_END|BUILD_REV);
	work = (SBreakObj2Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( SBreakObj2Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->shift = DG_ZeroVector ;
		work->rot = DG_ZeroSVector ;
		work->delay = 0;
		work->objs = objs;
		def = objs->def ;
		if(!GetResources(work,def,NULL,mode|NO_MAKE_OBJ|NO_CHECK_SW ,len,time_a,time_b,NULL)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

