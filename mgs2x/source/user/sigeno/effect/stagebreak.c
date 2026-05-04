//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	stagebreak.c
	ステージモデルを分解飛散
	2002/04/03 K.Sigeno
	$Id: stagebreak.c,v 1.1.1.3 2002/11/19 11:49:49 Yoshizawa1 Exp $
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

extern DG_OBJS *GM_GetMapObjs(int) ;

//#define	MAX_DIF		(5000)
//#define	MAX_DIF		(-10000)
//#define	MAX_DIF		(10000)

#define MAP_NUM		(0) /*変形させるマップ番号*/
enum	{
	MODE_BUILD = 0,
	MODE_ERASE,
};	
#define ROT_Y (0)
#define ROT_NORM (1)
#define ROT_FREE (2)
#define NO_MOVE	(3)

#define ROT_MODE ROT_Y //FIX
//#define ROT_MODE NO_MOVE

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	DG_VERTS_ANIME	*vanim;
	DG_OBJS		*objs ;
	FVECTOR		pos;
	FVECTOR		bound_min;
	FVECTOR		bound_max;
	FVECTOR		*objs_bound_min;
	FVECTOR		*objs_bound_max;
	SVECTOR		rot ;
	SVECTOR		dir ;
	int			time_a ;
	int			time_b ;
	float		z_min ;
	float		z_max ;
	float		rate ;
	float		len ;
	int			cnt ;
	int			mode ;
	int			map_num ;
	int			status ;
	int			name ;
	int			se_cnt ;
	int			se_cyc ;
} Work ;

enum {
	STAGE_WAIT = (0x0001),
};
#if 0
static void SetAmb(Work *work,u_char r,u_char g,u_char b){
	work->lights[1].m[ 3 ][ 0 ] = (float) r ;
	work->lights[1].m[ 3 ][ 1 ] = (float) g ;
	work->lights[1].m[ 3 ][ 2 ] = (float) b ;
}
#endif
#define NORM_LEN (4096.0f)
static inline void SetMoveVec(SVECTOR *norms,FVECTOR *buf,float len){
//		GV_LenVec3( norms, &buf, 0.0f, (int)speed ) ;
	float rate ;
	rate = len/NORM_LEN ;
#if 1
	buf->vx = (float)norms->vx*rate ;
	buf->vy = (float)norms->vy*rate ;
	buf->vz = (float)norms->vz*rate ;
#else
	buf->vx = norms->vx*len/NORM_LEN ;
	buf->vy = norms->vy*len/NORM_LEN ;
	buf->vz = norms->vz*len/NORM_LEN ;
#endif
}
#define Q_WORD (2)


static void SetBoundScale(Work *work ,float size ) {
	int j;
	DG_OBJ	*obj;

	work->bound_min = work->objs->bound_min;
	work->bound_max = work->objs->bound_max;

	/*バウンディングボックスも拡大*/
	work->objs->bound_min.vx -= size ;
	work->objs->bound_min.vy -= size ;
	work->objs->bound_min.vz -= size ;
	work->objs->bound_max.vx += size ;
	work->objs->bound_max.vy += size ;
	work->objs->bound_max.vz += size ;

	for(j= 0 ;j<work->objs->n_models;j++){
		obj = &work->objs->objs[j] ;
		work->objs_bound_min[j] = obj->bound_min ;
		work->objs_bound_max[j] = obj->bound_max ;

		obj->bound_min.vx -= size ;
		obj->bound_min.vy -= size ;
		obj->bound_min.vz -= size ;
		obj->bound_max.vx += size ;
		obj->bound_max.vy += size ;
		obj->bound_max.vz += size ;
	}
}
static void RestoreBound(Work *work) {
	int j;
	DG_OBJ	*obj;

	work->objs->bound_min = work->bound_min ;
	work->objs->bound_max = work->bound_max ;

	for(j= 0 ;j<work->objs->n_models;j++){
		obj = &work->objs->objs[j] ;
		obj->bound_min = work->objs_bound_min[j] ;
		obj->bound_max = work->objs_bound_max[j] ;
	}
}

static inline int SetDelayCnt(Work *work ,int cnt ,SVECTOR *org_sv,float z_min){
	int tmp_cnt ;
	tmp_cnt = cnt ;
	tmp_cnt -= (org_sv->vz - z_min)/work->rate  ;
	if(tmp_cnt<0) tmp_cnt = 0 ;
	if(tmp_cnt>work->time_a) tmp_cnt = work->time_a ;
	return tmp_cnt ;
}
static inline int SetDelayCntPos(Work *work,int cnt ,SVECTOR *org_sv,FVECTOR *pos){
	int tmp_cnt ;
	FVECTOR	org ;


#if 1
	tmp_cnt = cnt ;
	tmp_cnt -= GV_VecLen3F2( &org, pos )/work->rate  ;
#else
	org.vx = (float) org_sv->vx ;
	org.vy = (float) org_sv->vy ;
	org.vz = (float) org_sv->vz ;
	tmp_cnt = 0 ;
	tmp_cnt = work->time_a - (work->time_a*(GV_VecLen3F2( &org, pos )/2000.0f))  ;
#endif
	if(tmp_cnt<0) tmp_cnt = 0 ;
	if(tmp_cnt>work->time_a) tmp_cnt = work->time_a ;
	return tmp_cnt ;
}
static void SIG_ActVanimStageBuild(Work *work,DG_OBJS *objs ,int cnt,float z_min,FVECTOR *pos) {

//xbox
	DG_MDLPACK	*mdl_pack;
	DG_MDL		*mdl;

	float rate,rate_dif ;
//	float	tmp_speed;
	int n_verts,i,j,k,kick,tmp_cnt;
	int	verts_off,norm_off, speed ;
//	DG_OBJ	*obj;
//	FVECTOR	vel;

#ifdef PSX2
	SVECTOR *sv,*org_sv,*trg_sv,buf;
#else
//xbox
	DG_VERTEX_KMSM *sv ;
	SVECTOR *org_sv,*trg_sv,buf;
#endif



	SVECTOR *norms ;	/* 法線配列バッファ */
	short	n_packs ;
//	DG_OBJ_PACKET	*packets ;

//printf("cnt %d\n",cnt);
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
//xbox


				org_sv = (SVECTOR*)objs->objs[j].model->packs[k].verts ;
				norms = (SVECTOR*)objs->objs[j].model->packs[k].norms ;

				n_verts = objs->objs[j].packets[k].n_verts ;
				tmp_cnt = SetDelayCnt(work,cnt ,org_sv,z_min) ;
//				tmp_cnt = SetDelayCntPos(work,cnt ,org_sv,pos) ;
				speed = work->len*(float)((work->time_a-tmp_cnt)*(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;
//printf("speed %f\n",speed) ;
#if (ROT_MODE==ROT_NORM)
/*法線*/
				GV_LenVec3( norms, &buf, 1.0f, speed ) ;
#elif (ROT_MODE==ROT_Y)
/*Y方向*/
//				buf.vx=buf.vz = 0; 
//				buf.vy = -speed ;
#elif (ROT_MODE==ROT_FREE)
/*任意方向*/
				GV_LenVec3( &work->dir, &buf, 1.0f, speed ) ;
#elif (ROT_MODE==NO_MOVE)
/*頂点移動無し*/

#endif

#if 0
				rate = (float)tmp_cnt/(float)work->time_a ;
				rate_dif = 1.0f - rate ;
#else
				rate_dif = (float)(work->time_a-tmp_cnt)/(float)work->time_a ;
				rate_dif *= rate_dif ;
				rate = 1.0f - rate_dif ;
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
//printf("NORMS LEN [%d]\n",GV_VecLen3( norms )) ;
//4096
								kick++ ;
								trg_sv = sv ;
								tmp_cnt = SetDelayCnt(work,cnt ,org_sv,z_min) ;
//								tmp_cnt = SetDelayCntPos(work,cnt ,org_sv,pos) ;
//								speed = work->len*(work->time_a-tmp_cnt)/work->time_a ;
								speed = work->len*(float)((work->time_a-tmp_cnt)*(work->time_a-tmp_cnt))/(float)(work->time_a*work->time_a) ;
#if (ROT_MODE==ROT_NORM)
								GV_LenVec3( norms, &buf, 1.0f, speed ) ;
#elif (ROT_MODE==ROT_Y)
//								buf.vx=buf.vz = 0; 
//								buf.vy = -speed ;
#elif (ROT_MODE==ROT_FREE)
								GV_LenVec3( &work->dir, &buf, 1.0f, speed ) ;
#elif (ROT_MODE==NO_MOVE)

#endif

#if 0
								rate = (float)tmp_cnt/(float)work->time_a ;
								rate_dif = 1.0f - rate ;
#else
							rate_dif = (float)(work->time_a-tmp_cnt)/(float)work->time_a ;
							rate_dif *= rate_dif ;
							rate = 1.0f - rate_dif ;
#endif

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
#if (ROT_MODE==ROT_Y)
/*Y方向のみ*/
//							sv->vx = org_sv->vx;
//							sv->vy = org_sv->vy - buf.vy ;
//							sv->vz = org_sv->vz;
							sv->vy = org_sv->vy + speed ;
#elif (ROT_MODE==NO_MOVE)
#else 
/*任意方向*/
							sv->vx = org_sv->vx - buf.vx ;
							sv->vy = org_sv->vy - buf.vy ;
							sv->vz = org_sv->vz - buf.vz ;
#endif
						}else {
#if (ROT_MODE==ROT_Y)
/*Y方向のみ*/
//							sv->vy = org_sv->vy - buf.vy ;
//							sv->vz = org_sv->vz;
							sv->vy = org_sv->vy + speed ;

#elif (ROT_MODE==NO_MOVE)
#else
/*任意方向*/
							sv->vx = org_sv->vx - buf.vx ;
							sv->vy = org_sv->vy - buf.vy ;
							sv->vz = org_sv->vz - buf.vz ;
#endif

#if (ROT_MODE==NO_MOVE)
							sv->vx = (float) org_sv->vx*rate + (float)trg_sv->vx * rate_dif ;
							sv->vy = (float) org_sv->vy*rate + (float)trg_sv->vy * rate_dif ;
							sv->vz = (float) org_sv->vz*rate + (float)trg_sv->vz * rate_dif ;
#elif (ROT_MODE==ROT_Y)
							sv->vx = (float) org_sv->vx*rate + (float)trg_sv->vx * rate_dif ;
							sv->vy = (float) sv->vy*rate + (float)trg_sv->vy * rate_dif ;
							sv->vz = (float) org_sv->vz*rate + (float)trg_sv->vz * rate_dif ;
#else
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

enum {
	MSG_START = 1,
	MSG_ERASE,
} ;
static void CheckMesg(Work *work){
	GV_MSG *msg;
	int n;

#if 0
	if(GV_PadData[ 0 ].status & PAD_A){
					work->status &= (~STAGE_WAIT) ;
					work->mode = MODE_BUILD ;
//					work->cnt = 0 ;
					if(work->cnt > (work->time_a+work->time_b+1)) {
						work->cnt = work->time_a+work->time_b+1 ;
					}
	}
	if(GV_PadData[ 0 ].status & PAD_B){
					work->status &= (~STAGE_WAIT) ;
					work->mode = MODE_ERASE ;
//					work->cnt = (work->time_a+work->time_b) ;
					if(work->cnt < 0) {
						work->cnt = 0 ;
					}
	}
#endif
	if( ( n = GV_ReceiveMessage( work->name, &msg ) ) > 0 ){
		for( ; n > 0; n-- ){
			switch( msg->message[ 0 ] ) {
				case MSG_START :
printf("MSG_START\n ");
					work->status &= (~STAGE_WAIT) ;
					work->mode = MODE_BUILD ;
//					work->cnt = 0 ;
					if(work->cnt > (work->time_a+work->time_b+1)) {
						work->cnt = work->time_a+work->time_b+1 ;
					}
					break ;
				case MSG_ERASE :
printf("MSG_ERASE\n ");
					work->status &= (~STAGE_WAIT) ;
					work->mode = MODE_ERASE ;
//					work->cnt = (work->time_a+work->time_b) ;
					if(work->cnt < 0) {
						work->cnt = 0 ;
					}
					break ;
			}
			msg++;
		}
	}
}

static void Act(Work *work)
{
	int cnt ,se_tick;
	FVECTOR pos ,sepos;

	CheckMesg(work) ;
	if(work->status & STAGE_WAIT){
		return ;
	}
//printf("STAGE BREAK CNT [%d]\n",work->cnt);
	cnt = work->cnt ;
	pos.vx = pos.vy = pos.vz = 0;
	if(work->mode == MODE_BUILD){
		if(work->cnt > (work->time_a+work->time_b+1)) {
			work->status |= STAGE_WAIT ;
			return ;
		}
	}else {
		if(work->cnt < 0) {
			work->status |= STAGE_WAIT ;
			return ;
		}
	}
	SIG_ActVanimStageBuild(work,work->objs ,cnt,work->z_min,&GM_PlayerPosition) ;
	se_tick = DG_TickCount - work->se_cnt ;
#if 1
/*SE CALL*/
	if(work->map_num == 0){
		if(work->mode == MODE_BUILD){
//printf("TICK[%d] ACT[%d] \n",se_tick,work->cnt) ;
			if(work->cnt==0){
	//			GM_SeSetMode(SD_A_V_START1,&work->pos,GM_SEMODE_BOMB) ;
				GM_SdSet(SD_A_V_START1);

				work->se_cnt = DG_TickCount ;
//printf("STAGE BREAK START SE !!!!!!!!!!!!!!!!!!!!!\n") ;
//			}else if((work->cnt>=36)&&(work->cnt<=work->time_a+work->time_b-36)&&(work->cnt%13 ==0)){
			}else if((DIRECT_TICK(36) <=se_tick)&&(work->cnt<(work->time_a+work->time_b))){
				if( (DG_TickCount - work->se_cyc) >DIRECT_TICK(12) ){
					sepos = work->pos ;
//					work->pos.vz = work->z_min+(work->z_max-work->z_min) * work->cnt/(work->time_a+work->time_b) ;
//					GM_SeSetMode(SD_A_V_START2,&work->pos,GM_SEMODE_BOMB) ;
					GM_SdSet(SD_A_V_START2);
//					work->se_cyc = 0;
					work->se_cyc = DG_TickCount;
				}else {
//					work->se_cyc++ ;
				}
			}else if(work->cnt == (work->time_a+work->time_b)) {
					GM_SdSet(SD_A_V_START3 );
			}
		}else {
			if(work->cnt == (work->time_a+work->time_b-1)){
	//			GM_SeSetMode(SD_A_V_END001,&work->pos,GM_SEMODE_BOMB) ;
				GM_SdSet(SD_A_V_END001);
			}
		}
	}
#endif
//SD_A_V_START1,	//開始時地形創造１（36fps再生した後）//v_start1 1464
//SD_A_V_START2,	//開始時地形創造２（これを12fpsで連）//v_start2 1465

#if 0
	if(GV_PadData[ 0 ].status & PAD_A){
		work->cnt++; 
	}
	if(GV_PadData[ 0 ].status & PAD_B){
		work->cnt--; 
	}
	if(GV_PadData[ 0 ].status & PAD_U){
		work->rot.vx -= 128;
	}else if(GV_PadData[ 0 ].status & PAD_D){
		work->rot.vx += 128;
	}
	if(GV_PadData[ 0 ].status & PAD_R){
		work->rot.vy += 128;
	}else if(GV_PadData[ 0 ].status & PAD_L){
		work->rot.vy -= 128;
	}
	DG_SetPos2( &DG_ZeroVector, &work->rot ) ;
	tmp.vx = 0;
	tmp.vy = 4096.0f;
	tmp.vz = 0;
	DG_PutVector( &tmp, &tmp, 1 ) ;
	work->dir.vx = tmp.vx ;
	work->dir.vy = tmp.vy ;
	work->dir.vz = tmp.vz ;

	work->cnt %= (work->time_a+work->time_b+1) ;
#else

	if(work->mode == MODE_BUILD){
		work->cnt++ ;
	}else {
		work->cnt-- ;
	}
#endif

#if 0
	if(work->cnt>240){
		if(work->cnt&1){
//			DG_VisibleObjs(work->objs) ;
		}else {
//			DG_InvisibleObjs(work->objs) ;
		}
	}
#endif
}
static void Die(Work *work)
{
printf("STAGEBREAK END\n");
//	SIG_ActVanimStageBuild(work,work->objs ,0,work->z_min,&DG_ZeroVector) ;
	if(work->vanim != NULL ){
		SIG_FreeVanim(work->vanim,work->objs->n_models) ;
	}

	if((work->objs_bound_min != NULL )&&(work->objs_bound_max != NULL )){
		RestoreBound(work) ;
		GV_Free(work->objs_bound_min);
		GV_Free(work->objs_bound_max);
	}

	GV_Free(work->vanim) ;
}

#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_PAINT|DG_FLAG_AUTOREPAINT)
//#define MDL_FLAG (DG_FLAG_SHADE|DG_FLAG_ONEPIECE)

static int GetResources(Work *work,int name , int where )
{

	work->name = name ;

	work->rot = DG_ZeroSVector ;
	work->dir.vx = 0;
	work->dir.vy = 4096;
	work->dir.vz = 0;

	work->vanim = NULL ;
	work->objs_bound_min = NULL ;
	work->objs_bound_max = NULL ;


	if ( GCL_GetOption( 'n' ) != NULL ){
		work->map_num = GCL_GetNextInt() ;
//printf("MAPNAME [%d]\n",work->map_num);
		work->map_num = GM_GetMapID( work->map_num ) ;
//printf("MAPID [%x]\n",work->map_num);
		work->map_num = GM_GetID (work->map_num) ;
//printf("GetID [%d]\n",work->map_num);
	}else {
		if ( GCL_GetOption( 'd' ) != NULL ){
			work->map_num = GCL_GetNextInt() ;
		}else {
			work->map_num = 0 ;
		}
	}

#if 0
	if ( GCL_GetOption( 'm' ) != NULL ){
		work->mode = GCL_GetNextInt() ;
	}else {
		work->mode = MODE_BUILD;
	}
#else
	work->mode = MODE_BUILD;
#endif
	if ( GCL_GetOption( 'p' ) != NULL ){
		work->pos.vx = GCL_GetNextInt() ;
		work->pos.vy = GCL_GetNextInt() ;
		work->pos.vz = GCL_GetNextInt() ;
	}

	/*ステージオブジェを取得*/
	work->objs = GM_GetMapObjs(work->map_num) ;

	/*分解にかかる時間*/
	if ( GCL_GetOption( 't' ) != NULL ){
		work->time_a = DIRECT_TICK(GCL_GetNextInt()) ;
		work->time_b = DIRECT_TICK(GCL_GetNextInt()) ;
	}
	/*ポリゴン飛行距離*/
	if ( GCL_GetOption( 'l' ) != NULL ){
		work->len = (float) GCL_GetNextInt() ;
	}

	/*頂点バッファ生成*/
	work->vanim = GV_Malloc( sizeof( DG_VERTS_ANIME ) * work->objs->n_models );
	if(work->vanim == NULL ) return 0 ;
	SIG_InitVanim(work->vanim,work->objs,work->objs->n_models) ;

	/*バウンディング情報退避*/
	work->objs_bound_min = GV_Malloc( sizeof( FVECTOR ) * work->objs->n_models );
	if(work->objs_bound_min == NULL ) return 0 ;
	work->objs_bound_max = GV_Malloc( sizeof( FVECTOR ) * work->objs->n_models );
	if(work->objs_bound_max == NULL ) return 0 ;

	work->z_min = work->objs->bound_min.vz ;
	work->z_max = work->objs->bound_max.vz ;
	work->rate = (work->z_max - work->z_min) / (float)work->time_b ;
	/*バウンディング拡大*/
	SetBoundScale(work,fabs(work->len)) ;

	if(work->mode == MODE_BUILD ){
		work->cnt = 0 ;
	}else {
		work->cnt = (work->time_a+work->time_b) ;
	}
	work->status = STAGE_WAIT ;
	work->se_cyc = 255 ;
	/*変形状態から開始するため1回呼ぶ*/
	SIG_ActVanimStageBuild(work,work->objs ,work->cnt,work->z_min,&GM_PlayerPosition) ;
	return 1;
}

void *NewSigBreakStage( int name , int where )
{
	Work *work ;
//	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA,
/*タイムオーバー演出時に使うのでポーズ中でも動作*/
	work = (Work *)GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_CHARA,
				   sizeof( Work ), 0 ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,name,where)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

