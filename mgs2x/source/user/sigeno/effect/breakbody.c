//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	breabody.c
	多間接モデル頂点を飛散
	2002/03/14 K.Sigeno
	$Id: breakbody.c,v 1.1.1.3 2002/11/19 11:49:42 Yoshizawa1 Exp $
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

//#include "../../yamashita/outline/outline.h"

//#define VR_EF 0

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif

#define BREAK_PARTS_MAX 24
#define NORAMLPARTS_MAX 21

static int BODY_PARTS[BREAK_PARTS_MAX] ={

HUMAN21_MIGI_TSUMASAKI  ,
HUMAN21_HIDARI_TSUMASAKI,
HUMAN21_MIGI_KAKATO     ,
HUMAN21_HIDARI_KAKATO   ,

HUMAN21_MIGI_ASHI2      ,
HUMAN21_HIDARI_ASHI2    ,

HUMAN21_HIDARI_ASHI1    ,
HUMAN21_MIGI_ASHI1      ,

HUMAN21_KOSHI           ,

HUMAN21_ONAKA           ,

HUMAN21_HIDARI_TE       ,
HUMAN21_MIGI_TE         ,

HUMAN21_MUNE            ,


HUMAN21_HIDARI_UDE2     ,
HUMAN21_MIGI_UDE2       ,


HUMAN21_MIGI_UDE1       ,
HUMAN21_HIDARI_UDE1     ,

HUMAN21_MIGI_KATA       ,
HUMAN21_HIDARI_KATA     ,


HUMAN21_KUBI            ,

HUMAN21_ATAMA           ,

-1,-1,-1
};


#define NORMAL_MODE			(0x00000000)
//#define AMB_MODE			(0x00000001)
#define BUILD_Y				(0x00000002)
#define BUILD_UP			(0x00000004)	/*足から変形開始*/
#define NO_CHECK_SW			(0x00000008)
#define DISAPPEAR_MODE		(0x00000010) /*実体から消えへ デフォルトは無から実体へ*/
#define BODY_MODE			(0x00000020)
#define BUILD_X				(0x00000040)
#define BUILD_ROLL			(0x00000080)
#define BUILD_REV			(0x00000100) /*Y軸下から飛んでくる*/
#define MOTION_TR			(0x00000200) /*モーション追随*/
#define BUILD_RND			(0x00000400)	/*ノイジー*/
#define NO_MOVE_VEC			(0x00000800)	/*スケール加工のみ*/
#define NO_DELAY			(0x00001000)	/*全頂点同時*/
#define NO_MAKE_OBJ			(0x00002000)	/*既存OBJを加工*/
#define NO_WEIGHT			(0x00004000)	/*既存OBJを加工*/


/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	*vanim;
	DG_OBJS		*objs ;
	DG_OBJS		*p_objs ;
	FMATRIX		lights[2] ;
//	float		speed ;
	int			cnt ;
	int			n_models ;

	float		len ;
	int			time_a ;
	int			time_b ;
	int			delay ;
	int			mode ;

	float		min ;
	float		max ;
	float		rate ;

	FVECTOR color;
	void		*oozeblood ;		/* にじみ血用 */

} Work ;

#define	OBJECT_FLAG		(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
#define	OBJECT_FLAG_PRE	(DG_FLAG_PAINT | DG_FLAG_AUTOREPAINT|DG_FLAG_FINISHCALC)
#if 0
extern void	DG_FreePreshade( DG_OBJS * ) ;
extern int	DG_MakePreshade( DG_OBJS *, LIT_DEF * ) ;
#endif



static inline int PartsRank(int j,Work *work){
	int l,check ;

#if 0
	if(work->mode &NO_DELAY){
		return 0 ;
	}
#endif

	if(j >= NORAMLPARTS_MAX){
		check = work->objs->objs[j].parent ;
	}else {
		check = j ;
	}

//DISAPPEAR_MODE

	for(l=0;l<BREAK_PARTS_MAX;l++){
		if(check==BODY_PARTS[l]){
			if(work->mode & DISAPPEAR_MODE){
				if(work->mode & BUILD_UP){
//					return ((work->objs->n_models-1) - l ) ;
					return ((NORAMLPARTS_MAX-1) - l ) ;
				}else {
					return l ;
				}
			}else {
				if(work->mode & BUILD_UP){
					return l ;
				}else {
//					return ((work->objs->n_models-1) - l ) ;
					return ((NORAMLPARTS_MAX-1) - l ) ;
				}
			}
		}
	}
	if(check==BODY_PARTS[l]){
		return BREAK_PARTS_MAX-1 ;
	}else {
		return 0 ;
	}
}

#define Q_WORD (2)

static void SetBoundScale(Work *work,DG_OBJS *objs ,float size) {
	int j;
	DG_OBJ	*obj;

	work->min = objs->bound_min.vy ;
	work->max = objs->bound_max.vy ;

	if(work->mode &NO_MOVE_VEC){
		return ;
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

static inline int SetDelayCnt2(int cnt ,Work *work,int parts){
	int tmp_cnt ;
	tmp_cnt = cnt ;

	if(work->mode &NO_DELAY){
		return tmp_cnt ;
	}

//	tmp_cnt -= work->time_b*PartsRank(parts,work)/work->objs->n_models ;
	tmp_cnt -= work->time_b*PartsRank(parts,work)/(NORAMLPARTS_MAX-1) ;

//	if(tmp_cnt<0) tmp_cnt = 0 ;
//	if(tmp_cnt>work->time_a) tmp_cnt = work->time_a ;

	return tmp_cnt ;
}


//#define WEIGHT_BORDER (0.80f) //これ以下はゼロにする
#define WEIGHT_BORDER (0.85f) //これ以下はゼロにする

static inline float SetWeightRate(float rate){
	if(rate < WEIGHT_BORDER) return 0.0f ;
	return (rate-WEIGHT_BORDER)/(1.0f-WEIGHT_BORDER) ;
}
#define BUILD_MODE (BUILD_NORM)
#define ROLL_X_RATE (8)

static inline void SetMoveVec(Work *work,SVECTOR *buf,int speed ,SVECTOR *norms){
	FVECTOR tmp ;

	if(work->mode &NO_MOVE_VEC){
		buf->vx = 0 ;
		buf->vy = 0 ;
		buf->vz = 0 ;
		return ;
	}

	if((work->mode & BUILD_Y)||(work->mode & BUILD_X)){
		if(work->mode & BUILD_X){
			if(work->mode & BUILD_ROLL){
				tmp.vx=(-speed/ROLL_X_RATE) ;
			}else {
				tmp.vx= -speed ;
			}
		}else {
			tmp.vx = 0.0f ;
		}
		if(work->mode & BUILD_Y){
			if(work->mode & BUILD_REV){
				tmp.vy = speed ;
			}else {
				tmp.vy =-speed ;
			}
		}else {
			tmp.vy =0.0f ;
		}
		tmp.vz=0.0f;
		DG_RotVector(&tmp,&tmp,1) ;
		buf->vx = tmp.vx ;
		buf->vy = tmp.vy ;
		buf->vz = tmp.vz ;
	}else if(work->mode & BUILD_RND){
		SVECTOR  test ;
		test.vx = ((irnd()>>8)& 255) - 127 ;
		test.vy = ((irnd()>>8)& 255) - 127 ;
		test.vz = ((irnd()>>8)& 255) - 127 ;

		GV_LenVec3( &test, buf, 1.0f, speed ) ;
	}else {
		GV_LenVec3( norms, buf, 1.0f, speed ) ;
		tmp.vx = buf->vx ;
		if(work->mode & BUILD_REV){
			tmp.vy = buf->vy + speed ;
		}else {
			tmp.vy = buf->vy - speed ;
		}
		tmp.vz = buf->vz ;
		DG_RotVector(&tmp,&tmp,1) ;
		buf->vx = tmp.vx ;
		buf->vy = tmp.vy ;
		buf->vz = tmp.vz ;
	}
}




static void SIG_ActVanimObjBuild(Work *work,int cnt) {
//xbox
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;
	FVECTOR ef_pos ;
	DG_OBJS	*objs ;
	float rate,rate_dif ;
	float	tmp_speed,weight,weight_dif;
	int n_verts,i,j,k,l,kick,tmp_cnt,rank;
	int	verts_off,norm_off, speed ;
	DG_OBJ	*obj;
	FVECTOR	tmp;
	FMATRIX	world ;
#ifdef PSX2
	SVECTOR *sv,*tmp_sv,*org_sv,*trg_sv,buf,rot;
#else
//xbox
	DG_VERTEX_KMSM *sv ;
	SVECTOR *tmp_sv,*org_sv,*trg_sv,buf,rot;
#endif
	SVECTOR *norms ;	/* 法線配列バッファ */
	short	n_packs ;
	DG_OBJ_PACKET	*packets ;
	objs = work->objs ;
//printf("cnt %d\n",cnt);
	rot.vx = 0;
	rot.vy = 0;
	rot.vz = 0;

	ef_pos.vx = objs->world.m[3][0] ;
	ef_pos.vy = objs->objs[HUMAN21_MIGI_TSUMASAKI].world.m[3][1] ;
	ef_pos.vz = objs->world.m[3][2] ;

	for(j= 0 ;j<objs->n_models;j++){
//		FVECTOR tmp ;
#ifndef PSX2
		sv = work->vanim[j].verts_top[work->vanim[j].buffer_clock];
#endif
//xbox
		mdl = work->vanim[j].obj->model;
		mdl_pack = mdl->packs;

		if(cnt < 0){
			if(work->mode &DISAPPEAR_MODE){
				tmp_cnt = work->time_a ;
			}else {
				tmp_cnt = 0 ;
			}
		}else {
			tmp_cnt = SetDelayCnt2(cnt ,work,j) ;
		}
		if((tmp_cnt>work->time_a)||(tmp_cnt<0)){
			continue ;
		}

		n_packs = objs->objs[j].n_packs ;
		verts_off = 0 ;
		norm_off = 0 ;

//	fall_line
//		ef_pos.vy = objs->objs[j].world.m[3][1] ;

//		DG_SetPos( &objs->objs[j].world ) ;
		_sceVu0InversMatrix(&world ,&objs->objs[j].world) ;
		DG_SetPos(&world) ;

//		tmp.vx = 0.0f ;
//		tmp.vy = -200.0f ;
//		tmp.vz = 0.0f ;

//		DG_PutVector(&DG_ZeroVector,&tmp,1) ;
//		PosBox(&tmp,25.0f ,NULL );


		if(work->mode & BUILD_ROLL){

			rot.vy = (tmp_cnt*256)&4095;
			DG_RotatePos(&rot) ;
		}
//xbox
		for(k=0;k<mdl->n_packs;k++){
			if( 1){
#ifdef PSX2
				sv = &objs->objs[j].verts[verts_off*Q_WORD] ;
#endif
//xbox
				org_sv = (SVECTOR*)objs->objs[j].model->packs[k].verts ;
				norms = (SVECTOR*)objs->objs[j].model->packs[k].norms ;

				n_verts = objs->objs[j].packets[k].n_verts ;

				speed = work->len*(float)((work->time_a-tmp_cnt)*
					(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;

				SetMoveVec(work,&buf,speed,norms) ;
				rate = (float)tmp_cnt/(float)work->time_a ;
				rate_dif = 1.0f - rate ;
				weight = SetWeightRate(rate) ;
				weight_dif = 1.0f - weight ;
				kick = 1 ;
				trg_sv = sv ;


//if(((work->cnt%10) ==0)&&(j==HUMAN21_ATAMA)){
//	printf("rate [%f] rate_dif[%f]\n",rate,rate_dif);
//}

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
								speed = work->len*(float)((work->time_a-tmp_cnt)*(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;

								SetMoveVec(work,&buf,speed,norms) ;
								rate = (float)tmp_cnt/(float)work->time_a ;
								rate_dif = 1.0f - rate ;
								weight = SetWeightRate(rate) ;
								weight_dif = 1.0f - weight ;
#endif
							}
							norms-- ;
						}
					}
#endif
	//printf(" norms pad[%x]\n",norms->pad) ;
//					if(work->mode &NO_MOVE_VEC){
					{
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
								sv->vx = (float) sv->vx*rate + (float)trg_sv->vx * rate_dif ;
								sv->vy = (float) sv->vy*rate + (float)trg_sv->vy * rate_dif ;
								sv->vz = (float) sv->vz*rate + (float)trg_sv->vz * rate_dif ;
#endif
							}
						}
#endif
					}

//					sv->pad = 0x1000 ;
//					sv->pad = (org_sv->pad * weight ) + (4096.0f*weight_dif);
//					sv->pad = (((float)org_sv->pad/4096.0f) * rate )*4096.0f + (4096.0f*rate_dif);
#ifdef PSX2
					if(!(work->mode & NO_WEIGHT)){
						sv->pad = (((float)org_sv->pad/4096.0f) * weight )*4096.0f + (4096.0f*weight_dif);
					}
#else
//xbox
//					sv->wt = ;
#endif
//					sv->pad = (((float)org_sv->pad/4096.0f) * 0.0f )*4096.0f + (4096.0f*1.0f);
//					if(weight>0.99f){
//						sv->pad = org_sv->pad ;
//					}else {
//						sv->pad = (((float)org_sv->pad/4096.0) * weight )*4096.0f;
//					}
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

static void Act(Work *work)
{

	float width ;
//	FVECTOR color;
	int i,n_body;
	n_body = work->p_objs->n_models ;

	GM_CurrentMap = GM_CurrentStageMap ;

#if 1
	if(work->mode & MOTION_TR){
		for(i= 0 ;i<n_body;i++){
			work->objs->objs[i].world = work->p_objs->objs[i].world ;
		}
	}
#endif

	if(!(work->mode & NO_MAKE_OBJ)){
		DG_GetLightMatrix( (FVECTOR *) &work->objs->world.m[3] , work->lights );
	}
	SIG_ActVanimObjBuild(work,work->cnt) ;

	if(work->mode &DISAPPEAR_MODE){
		if(work->delay>0){
			work->delay-- ;
		}else {
			work->cnt-- ;
		}
		if(work->cnt < 0) {
			GV_DestroyActor(work) ;
		}
	}else {
		if(work->delay>0){
			work->delay-- ;
		}else {
			work->cnt++ ;
		}
		if(work->cnt > work->time_a+work->time_b) {
			GV_DestroyActor(work) ;
		}
	}

}
static void Die(Work *work)
{
	if(work->vanim != NULL){
		SIG_FreeVanim(work->vanim,work->n_models) ;
		GV_Free(work->vanim) ;
	}
	if(!(work->mode & NO_MAKE_OBJ)){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
	}
	if ( !(GV_IsStageDestroy( work )) ) {
		if ( work->oozeblood != NULL ) GV_DestroyActorQuick( work->oozeblood ) ;
	}
}

static int GetResources(Work *work, DG_OBJS *p_objs,float len,CVECTOR *col,int time_a,int time_b)
{
	DG_DEF	*def ;
	int i;

	work->len = len ;

	work->time_a = DIRECT_TICK(time_a) ;
	work->time_b = DIRECT_TICK(time_b) ;

	work->vanim = NULL ;
	work->objs = NULL ;

	def = (DG_DEF*) p_objs->def ;
	if(work->mode & NO_MAKE_OBJ){
		work->objs = p_objs ;
		work->p_objs = p_objs ;
	}else {
		work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC, DG_CHANL_MAIN ) ;
		if(work->objs == NULL ) return 0 ;
//		ASSERT(work->objs != NULL ) ;
		DG_QueueObjs( work->objs ) ;
		work->p_objs = p_objs ;
		work->objs->group_id = work->p_objs->group_id ;
		DG_SetPos( &p_objs->world ) ;
		DG_PutObjs( work->objs );
		for(i= 0 ;i<p_objs->n_models;i++){
			work->objs->objs[i].world = p_objs->objs[i].world ;
		}
		DG_SetLightMatrix( work->objs, work->lights );
		DG_GetLightMatrix( (FVECTOR *) &work->objs->world.m[3] , work->lights );

	}

	work->n_models = work->objs->n_models ;

	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->n_models );
	if(work->vanim == NULL ) return 0 ;
	if(SIG_InitVanim(work->vanim,work->objs,work->n_models) == 0){
		return 0 ;
	}
	SetBoundScale(work,work->objs ,fabs(work->len)) ;

	if(work->mode &DISAPPEAR_MODE){
		work->cnt = (work->time_a+work->time_b) ;
	}else {
		work->cnt = 0 ;
	}

	work->rate = (work->max - work->min) / (float)work->time_b ;

	SIG_ActVanimObjBuild(work,-1) ;

#if 0
	{
//ステルス迷彩
		extern void *NewOpticalCamouflage( DG_OBJS *objs, int flag );
		GV_SetActorChild( &work->actor, (void *) NewOpticalCamouflage( work->objs, 0 ) ) ;
	}
#endif
	return 1;
}

//void *NewSigBreakBody(OBJECT *body,float len ,CVECTOR *col,int time_a,int time_b,int mode,void *ooze)
void *NewSigBreakBody(DG_OBJS *p_objs,float len ,CVECTOR *col,int time_a,int time_b,int delay,int mode,void *ooze)
{
	extern void TAKABE_OozeBloodChangeObjs( void *, DG_OBJS * ) ;
	Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		work->mode = mode;
		work->delay = delay ;
		if(!GetResources(work,p_objs,len,col,time_a,time_b)){
			printf("NewSigBreakBody init failed !!!\n");
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	if(ooze != NULL){
		work->oozeblood = ooze ;
		TAKABE_OozeBloodChangeObjs( ooze, work->objs ) ;
	}else{
		work->oozeblood = NULL ;
	}
	return (void *)work ;
}

/*
NewSigBreakBody内部でPAL補正するのでそのまま
*/
#define TIME_A	(30)
#define TIME_B	(90)

#define ENE_HOLD_LEN	(200.0f)
#define ENE_ELIM_LEN	(600.0f)
#define ENE_HOLD_LEN	(200.0f)


void *NewENE_BreakBody(OBJECT *body,int type,void *ooze){

	int mode=0 ;
	float len = ENE_HOLD_LEN ;
	int time_a,time_b ;

	time_a = TIME_A ;
	time_b = TIME_B ;
	
	switch (type){
//消え
		case 1 :
//下降
			mode = BUILD_UP|BUILD_Y|BUILD_REV|DISAPPEAR_MODE ;
			break;
		case 2 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//上昇下降 	死体消え
			mode = BUILD_UP|BUILD_Y|DISAPPEAR_MODE;
			len = ENE_ELIM_LEN ;
			GM_SeSetMode( SD_A_V_SHITAI, (FVECTOR *)& body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
			break;
		case 3 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//ホールドアップ消え
			GM_SeSetMode( SD_A_V_HOLDUP, (FVECTOR *)& body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
			mode = BUILD_UP|BUILD_REV|DISAPPEAR_MODE;
			len = ENE_HOLD_LEN ;
			break;
		case 4 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//上昇下降 	クリア時用 高速死体消え 
			mode = BUILD_UP|BUILD_Y|DISAPPEAR_MODE;
			len = ENE_ELIM_LEN ;
//			GM_SeSetMode( SD_A_V_SHITAI, (FVECTOR *)& body->objs->world.m[3], GM_SEMODE_NORMAL ) ;
			time_a = 15 ;
			time_b = 15 ;
			break;
//以下出現
		case 5 :
//下降
			mode = BUILD_UP|BUILD_Y;
			break;
		case 6 :
//上昇
			mode = BUILD_Y|BUILD_REV;
		break;
		case 7 :
//下降螺旋
			mode = BUILD_UP ;
			break;
		case 8 :
//上昇螺旋
			mode = BUILD_ROLL|BUILD_ROLL|BUILD_REV ;
			break;
		case 9 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//ＶＲ敵兵出現
			mode = NO_MOVE_VEC|NO_MAKE_OBJ|NO_WEIGHT;
			len = 0 ;
			time_a = 8 ;
			time_b = 7 ;
			break;
		case 10 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//ＶＲ敵兵消失
			mode = NO_MOVE_VEC|NO_MAKE_OBJ|NO_WEIGHT|DISAPPEAR_MODE ;
			len = 0 ;
			time_a = 8 ;
			time_b = 7 ;
			break;
		case 11 :
//FIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIXFIX
//ＶＲ敵兵消失モデル生成型
			mode = NO_MOVE_VEC|NO_WEIGHT|DISAPPEAR_MODE ;
			len = 0 ;
			time_a = 8 ;
			time_b = 7 ;
			break;
	}
	return NewSigBreakBody(body->objs,len, NULL,time_a,time_b,0,mode,ooze) ;
}


