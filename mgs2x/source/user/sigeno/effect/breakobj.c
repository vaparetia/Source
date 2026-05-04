//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breakobj.c
	モデル頂点を法線方向に飛散
	2002/03/12 K.Sigeno
	$Id: breakobj.c,v 1.1.1.3 2002/11/19 11:49:44 Yoshizawa1 Exp $
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

#define	BREAK_VER (3)
//#include "enemy.h"
//#include "enemy.x"
#define MDL_NUM (22)
/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	*vanim;
	DG_OBJS		*objs ;
	FMATRIX		lights[2] ;
	CVECTOR		col ;
	SVECTOR		rot ;
	float		speed ;
	float		decay ;
	int			cnt ;
	int			n_models ;
	int			flag ;
	int			decay_time ;
} Work ;

#define GET_LIGHT	(0x01)
#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)
#if 0
extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
#endif


#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif


static void SetAmb(Work *work,u_char r,u_char g,u_char b){
	work->lights[1].m[ 3 ][ 0 ] = (float) r ;
	work->lights[1].m[ 3 ][ 1 ] = (float) g ;
	work->lights[1].m[ 3 ][ 2 ] = (float) b ;
}
#define Q_WORD (2)
static void SvectoFvel(SVECTOR *sv , FVECTOR *res,float speed){
	FVECTOR vel ;
	vel.vx = (float) sv->vx ;
	vel.vy = (float) sv->vy ;
	vel.vz = (float) sv->vz ;
	GV_LenVec3F( &vel, res, 0.0f, speed ) ;
}
/*
	obj = v_anime->obj ;
	mdl = obj->model ;
	obj->verts = (SV1ECTOR*)mdl->packs->verts ;
	obj->norms = (SVECTOR*)mdl->packs->norms ;
*/
static void SIG_ActVRTRGBreak(Work *work,DG_OBJS *objs ,float speed,int cnt) {
//xbox
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	int n_verts,i,j,k;
	int	verts_off,norm_off;
	DG_OBJ	*obj;
	FVECTOR	vel,buf;


	SVECTOR *sv;

	SVECTOR *norms ;	/* 法線配列バッファ */
	short	n_packs ;
	DG_OBJ_PACKET	*packets ;

	/*バウンディングボックスも拡大*/
	objs->bound_min.vx -= speed ;
	objs->bound_min.vy -= speed ;
	objs->bound_min.vz -= speed ;
	objs->bound_max.vx += speed ;
	objs->bound_max.vy += speed ;
	objs->bound_max.vz += speed ;


	for(j= 0 ;j<objs->n_models;j++){
		mdl = work->vanim[j].obj->model;
		mdl_pack = mdl->packs;
		n_packs = objs->objs[j].n_packs ;
		verts_off = 0 ;
		norm_off = 0 ;
		obj = &objs->objs[j] ;
		obj->bound_min.vx -= speed ;
		obj->bound_min.vy -= speed ;
		obj->bound_min.vz -= speed ;
		obj->bound_max.vx += speed ;
		obj->bound_max.vy += speed ;
		obj->bound_max.vz += speed ;
		for(k=0;k<mdl->n_packs;k++){
			sv = &objs->objs[j].verts[verts_off*Q_WORD] ;
			norms = &objs->objs[j].norms[norm_off*Q_WORD] ;
			n_verts = objs->objs[j].packets[k].n_verts ;
			SvectoFvel(norms,&buf,speed) ;
			for (i=0;i<mdl_pack->n_verts;i++){
				/*頂点キックのみが2回続くならポリゴン切断と判断*/
				if((norms->pad&0xffff)== 0x8fff){
					if(i<(n_verts-1)){
						norms++ ;
						if((norms->pad&0xffff)== 0x8fff){
							SvectoFvel(norms,&buf,speed) ;
							if(i>((cnt+1)*16)){
								break ;
							}
						}
						norms-- ;
					}
				}
				sv->vx -= (short) buf.vx ;
				sv->vy -= (short) buf.vy ;
				sv->vz -= (short) buf.vz ;
				sv->pad = 0x1000;
				sv++ ;
				norms++ ;
			}
			verts_off += objs->objs[j].packets[k].verts_offset ;
			norm_off += objs->objs[j].packets[k].norms_offset ;
			mdl_pack++ ;
		}
	}
}
#define LIFE_TIME	(60)

//#define FADE_TIME	(60)
//#define FADE_TIME	(30)
#define FADE_TIME	(30)

#define	BASE_ROT	(8)
#define VEL_MIN		(5.0f)
#define	F_ROT_RATE	(0.10f)
static void Act(Work *work)
{
	float rate ,speed;
	SVECTOR rot;
	CVECTOR col ;
	speed = work->speed ;

#if (BREAK_VER == 3)
	if(work->cnt > work->decay_time) {
		speed *= work->decay ;
	}
#endif

	SIG_ActVRTRGBreak(work,work->objs ,speed ,work->cnt) ;

	if(work->cnt > LIFE_TIME) {
		GV_DestroyActor(work) ;
		return ;
	}



#if 0
/*点滅*/
	if(work->flag & GET_LIGHT ){
		DG_GetLightMatrix( (FVECTOR *)&work->objs->world.m[3], work->lights );
	}
	if(work->cnt>FADE_TIME){
		if(work->cnt&1){
			DG_VisibleObjs(work->objs) ;
		}else {
			DG_InvisibleObjs(work->objs) ;
		}
	}
#else
/*フェード*/
	if(work->cnt<(LIFE_TIME-FADE_TIME)){
		rate = 1.0f ;
	}else {
		rate = (float)  (LIFE_TIME-(work->cnt))  /(float)FADE_TIME ;
	}
//	rate *= 0.5f;
//	rate *= 0.0f;

//	rate *= 0.75f;
	rate *= 0.50f;

	col.r = rate * (float)work->col.r ;
	col.g = rate * (float)work->col.g ;
	col.b = rate * (float)work->col.b ;

	SetAmb(work,(u_char)col.r ,(u_char)col.g,(u_char)col.b) ;
#endif

#if (BREAK_VER == 1)
//Ver 1.0
	if(work->speed > VEL_MIN){
		work->speed *= work->decay ;
		work->rot.vx = (float)work->rot.vx * work->decay ;
		work->rot.vy = (float)work->rot.vy * work->decay ;
		work->rot.vz = (float)work->rot.vz * work->decay ;
	}
	DG_SetPos( &work->objs->world ) ;
	DG_RotatePos(&work->rot) ;
#elif (BREAK_VER == 2)
//Ver 1.1 F.Curve
	if(work->speed > VEL_MIN){
		work->speed *= work->decay ;
		rot.vx = work->rot.vx ;
		rot.vy = work->rot.vy ;
		rot.vz = work->rot.vz ;
	}else {
		rot.vx = work->rot.vx*F_ROT_RATE ;
		rot.vy = work->rot.vy*F_ROT_RATE ;
		rot.vz = work->rot.vz*F_ROT_RATE ;
	}
	DG_SetPos( &work->objs->world ) ;
	DG_RotatePos(&rot) ;
#elif (BREAK_VER == 3)
	if(work->cnt > work->decay_time) {
		rot.vx = work->rot.vx*F_ROT_RATE ;
		rot.vy = work->rot.vy*F_ROT_RATE ;
		rot.vz = work->rot.vz*F_ROT_RATE ;
	}else {
		rot.vx = work->rot.vx ;
		rot.vy = work->rot.vy ;
		rot.vz = work->rot.vz ;
	}
	DG_SetPos( &work->objs->world ) ;
	DG_RotatePos(&rot) ;
#endif

	DG_PutObjs( work->objs );
	work->cnt++ ;
}
static void Die(Work *work)
{
	if(work->vanim != NULL){
		SIG_FreeVanim(work->vanim,work->n_models) ;
	}
	if(work->objs != NULL){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
	}
	if(work->vanim != NULL ){
		GV_Free(work->vanim) ;
	}
}

static int GetResources(Work *work, int mdl,FMATRIX *world ,float speed,float decay,
CVECTOR *col,SVECTOR *rot,int decay_time)
{
	FVECTOR tmp ;
	DG_DEF	*def ;
	int i;

	work->flag = 0 ;
	def = (DG_DEF*) GV_GetCache( GV_CacheID( mdl, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_NOFOG|DG_FLAG_ONEPIECE, DG_CHANL_MAIN ) ;
	if(work->objs == NULL) return 0 ;


	DG_QueueObjs( work->objs ) ;
	DG_SetPos( world ) ;
	DG_PutVector(&DG_ZeroVector,&tmp,1);
	DG_PutObjs( work->objs );

//	PosBox(&tmp,250.0f , NULL );
//printf("F OBJ X[%f] Y[%f] Z[%f] \n",tmp.vx,	tmp.vy,	tmp.vz	);


	if(col != NULL){
		work->col = *col ;
		SetAmb(work,(u_char)col->r ,(u_char)col->g,(u_char)col->b) ;
	}else {
		work->flag = GET_LIGHT ;
	}
//	SetAmb(work,(u_char)127 ,(u_char)127,(u_char)127) ;
	DG_SetLightMatrix( work->objs, work->lights );

	work->speed = speed ;
	work->decay = decay;
	work->decay_time = DIRECT_TICK(decay_time);


	work->n_models = work->objs->n_models ;
//	DG_VERTS_ANIME	*vanim;

	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->n_models );
	if(work->vanim == NULL ) return ;
	SIG_InitVanim(work->vanim,work->objs,work->n_models) ;

	work->cnt = 0 ;
	work->rot = *rot ;

	return 1;
}

void *NewSigBreakObj(int mdl,FMATRIX *world,float speed ,float decay,CVECTOR *col,SVECTOR	*rot,int decay_time)
{
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,mdl,world,speed,decay,col,rot,decay_time)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

