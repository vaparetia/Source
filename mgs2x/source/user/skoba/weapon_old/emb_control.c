//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	enb_control.c
	統合薬莢管理（コモデル）
	
	2001/07/23 T.Shibata
	
	$Id: emb_control.c,v 1.1.1.3 2002/11/19 11:50:34 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <string.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include 	"libutl.h"
#include	"rand.h"

#include	"emb_control.h"


#define CLOCK_COUNT	(BP_BASE_TICK())

#define   BOUND_DISTANCE	(6000.0f)

#define   UPPER_SHIFT	(10.0f)

#define   MIN_VEC	(20.0f)
#define   MIN_MIN_VEC	(5.0f)

// -----------------------------------------------------------
//			extern
extern int DG_QueueComdlObjs( DG_COMDL *comdl );
extern void DG_DequeueComdlObjs( DG_COMDL *comdl );

#define	PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

static FVECTOR g_EmbScaleVector[AMO_KIND_MAX] = {
	{ 1.0f, 1.0f,   1.0f, 0.0f },
	{ 1.0f, 1.0f,   1.0f, 0.0f },
	{ 1.0f, 1.0f,   1.0f, 0.0f },

	{ 1.0f, 1.0f,   1.0f, 0.0f },	
	{ 1.0f, 0.58f,  1.0f, 0.0f },
	{ 1.0f, 1.0f,   1.0f, 0.0f },
	{ 1.0f, 0.866f, 1.0f, 0.0f },
	{ 1.0f, 0.866f, 1.0f, 0.0f },
	{ 1.0f, 2.0f,   1.0f, 0.0f },
	{ 1.0f, 1.0f,   1.0f, 0.0f },
	{ 1.0f, 1.0f,   1.0f, 0.0f },
	{ 1.0f, 0.866f, 1.0f, 0.0f },
	
	{ 1.0f, 1.0f,   1.0f, 0.0f },
};

static u_char g_EmbMdlKindList[AMO_KIND_MAX] = {
	0,0,0,
	1,1,1,1,1,1,2,1,1,
	3,
};
//処理用定数 
#define		N_MDL_KIND		(4)

//モード 
#define		EMB_MODE_ACTIVE		(0x0001)
#define		EMB_MODE_CALLEFT	(0x0002)		//外の人が見ます。 
#define		EMB_MODE_MOVE		(0x0004)

//計算用定数 
#define   VEC_LIMIT			(-100.0f)
#define	  DECAY_RATIO		(0.5f)
#define	  DECAY_RATIO_W		(0.8f)


typedef	struct	{
	GV_ACT_EX	actor;
	DG_COMDL	*comdl[N_MDL_KIND];		//
	int			n_comdl[N_MDL_KIND];	//
	int			n_mdl_kind;
	int			n_max_embs;				//
	int			n_act_allembs;
	int			n_active_embs[AMO_KIND_MAX];		//
	float		emb_init_ratio[AMO_KIND_MAX];

	int			bound_count;
	EMB_DATA	*emb_data;
	EMB_DATA	*emb_list;
	EMB_DATA	*emb_list_emb[AMO_KIND_MAX];
} Work;

static Work *_EnbControlWork = NULL;

//
static EMB_DATA *GetInitEmbData( int emb_id )
{
	Work		*work = _EnbControlWork;
	EMB_DATA	*data = NULL,*emb_list;// = work->emb_data;
	int i,del_emb_id;

	if( !work ) return NULL;

//printf("::::::::::::::::::::::::::::%d %d\n",g_EmbMdlKindList[emb_id],work->n_mdl_kind);

	if( g_EmbMdlKindList[emb_id] >= work->n_mdl_kind ){

		if( g_EmbMdlKindList[emb_id] == 3 ){
			printf("ＳＰＳ薬莢が無いのでＦＡＭＡＳに変更\n");
			g_EmbMdlKindList[emb_id] = 1;
		}else{
			printf("グレネード弾のデータないっす。\n");
			return NULL;
		}
	}
	if( ++work->n_act_allembs < work->n_max_embs ){
		//空有り 
		emb_list = work->emb_data;
		for( i = 0; i < work->n_max_embs; i++,emb_list++ ){
			//printf("[%d]mode %x\n",i,emb_list->mode);
			if( (emb_list->mode & EMB_MODE_ACTIVE) ) continue;
			data = emb_list;
			break;
		}
		//printf("まだまだいけまっせー<%p>\n",data);
	}else{
		//空無し 
//		float max_diff_ratio = -FLOAT_MAX;
		int		temp_max_time = 0x80000000;

#if 0
		work->n_act_allembs = work->n_max_embs-1;
		del_emb_id = 0;
		for( i = 0; i < AMO_KIND_MAX; i++ ){
			//今の割合 
			float ftemp = (float)work->n_active_embs[i]/(float)work->n_max_embs;
			ftemp = ftemp - work->emb_init_ratio[i];
			if( ftemp > max_diff_ratio ){
				max_diff_ratio = ftemp;
				del_emb_id = i;
			}
		}
		

		work->n_active_embs[del_emb_id]--;
		emb_list = work->emb_list_emb[del_emb_id];
		data = emb_list;
		temp_max_time = data->time;

		for( ; emb_list->next_emb; emb_list = emb_list->next_emb ){
			if( (emb_list->next->mode & EMB_MODE_CALLEFT) && temp_max_time < emb_list->next_emb->time ){
				data = emb_list;
				temp_max_time = emb_list->next_emb->time;
			}
		}

		if( data == work->emb_list_emb[del_emb_id] ){
			//先頭 
			work->emb_list_emb[del_emb_id] = data->next;
		}else{
			//途中 
			emb_list = data;
			data = emb_list->next_emb;
			emb_list->next_emb = data->next_emb;
		}

		for( emb_list = work->emb_list; emb_list->next; emb_list = emb_list->next ){
			if( emb_list->next == data ){
				emb_list->next = data->next;
				break;
			}
		}
#else
		//バグ修正 
		emb_list = work->emb_data;
		for( i = 0; i < work->n_max_embs; i++,emb_list++ ){
			if( /*(emb_list->mode & EMB_MODE_CALLEFT) && */ temp_max_time < emb_list->time ){
				data = emb_list;
				temp_max_time = emb_list->time;
				del_emb_id = data->emb_id;
				//printf(" max_time %d\n",temp_max_time);
			}
		}
#endif
		//printf("無理っぽい\n");
	}
	if( !data ){ printf("cannot get\n"); return NULL; }
	data->next_emb = work->emb_list_emb[emb_id];
	work->emb_list_emb[emb_id] = data;

	data->next = work->emb_list;
	work->emb_list = data;

	work->n_active_embs[emb_id]++;
	data->mode = (EMB_MODE_MOVE|EMB_MODE_ACTIVE);
	
	return data;
}

EMB_DATA *SetNewEmbData( int emb_id, FVECTOR *pos, FVECTOR *speed, SVECTOR *init_rot, SVECTOR *rot, short point_time )
{
	EMB_DATA	*data = GetInitEmbData( emb_id );

	if( data ){
		DG_COPY_VEC( &data->pos, pos );
		DG_COPY_VEC( &data->speed, speed );

		data->rot = *rot;
		data->const_rot = *init_rot;
		data->point_time = point_time;
		data->time = 0;
		data->emb_id = emb_id;
		data->mdl_kind = g_EmbMdlKindList[emb_id];
		
	}
	//printf("mode %x\n",data->mode);
	
	return data;
}


static inline void _MulMatrix( FMATRIX *world, int emb_id )
{
#if 0
	DG_SetPos( world );
	DG_ScalePos( &g_EmbScaleVector[emb_id] );
	DG_GetPos( world );
#else
	//今はＹスケールしかないのでこれで 
	_sceVu0ScaleVector( (FVECTOR*)world->m[1], (FVECTOR*)world->m[1], g_EmbScaleVector[emb_id].vy );
#endif
}

static  void  SetSound(FVECTOR *speed,int se, FVECTOR  *pos, int mode)
{
	float	len ;
	int	  pan ;
	int	  vol ;
   float bp_angle;

	
//	GM_SeSetMode( se, pos, GM_SEMODE_NORMAL );
	len = GV_VecLen3F(speed) ;

	if(len >= 40.0F){
		vol = GM_SeGetVol( pos, mode );
		pan = GM_SeGetPan( pos, mode, &bp_angle );
		if(len <= 160.0F) vol = (int)( (float)vol * (len - 40.0F) / 120.0F) ;

		//printf(" vol %d: pan %d\n",vol,pan);
		GM_SeSet3D(pan,vol,se, bp_angle) ;
	}

}

#if 0
static void DelEmbList_emb( Work *work, EMB_DATA *data )
{
	EMB_DATA *data_list = work->emb_list_emb[data->emb_id];

	if( data_list == data ){
		work->emb_list_emb[data->emb_id] = data->next;
		return;
	}
	for( ; data_list->next; data_list = data_list->next ){
		if( data == data_list->next ){
			data_list->next = data->next;
			break;
		}
	}
}
#endif

static void BoundCallBack( Work *work )
{
	if( !GM_CheckGameStatus( STATE_DEMO ) ) {
		work->bound_count = 2;
	}
}

static void Act( Work *work )
{
	int				i,hzx_id,n_comdl[N_MDL_KIND];
//	int				flag;
	int				bound_dis;
	FVECTOR			cam_pos ;
	FVECTOR			far_ponit;
//	FVECTOR			*pos ;
	EMB_DATA		*emb_data = work->emb_list;
	EMB_DATA		*next;
	DG_COMDL		*comdl[N_MDL_KIND];// = work->comdl;
	DG_COMDL_POS	*cmdl_pos[N_MDL_KIND];

	hzx_id = GM_GetHzxGroupID( GM_CurrentStageMap );
	work->bound_count--;
	if( work->bound_count < 0 ){
		work->bound_count = 0;
	}

	//処理前初期化 
	for( i = 0; i < work->n_mdl_kind; i++ ){
		n_comdl[i] = 0;
		comdl[i] = work->comdl[i];
		cmdl_pos[i] = comdl[i]->pos;
		GM_GroupObject( comdl[i], GM_CurrentStageMap ) ;
	}

	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );
	bound_dis = BOUND_DISTANCE * DG_Chanls->screen;

#if 0
	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		emb_data = work->emb_data;
		for(i = 0; i < work->n_mdl_kind; i++,emb_data++ ){
			if( !(emb_data->mode & EMB_MODE_ACTIVE) ) continue;
			pos = &emb_data->pos;
			if( DG_FABS(pos->vx-cam_pos.vx) > bound_dis
			       || DG_FABS(pos->vy-cam_pos.vy) > bound_dis
			       || DG_FABS(pos->vz-cam_pos.vz) > bound_dis ){
				emb_data->mode &= ~EMB_MODE_ACTIVE;
			}
		}
	}
#endif

	
	//メイン処理 
	emb_data = work->emb_data;
	for( i = 0; i < work->n_max_embs; i++, emb_data++ ){
	//emb_data; pre = emb_data, emb_data = next ){
		FVECTOR		*pos = &emb_data->pos;
		FVECTOR		*vec = &emb_data->speed;
		FVECTOR		fvtemp0,fvtemp1,fvtemp2;
		HZX_SEG		*seg;
		HZX_FLR		flr[2];
		int			atr[2],h_flag = 0;

		next = emb_data->next;
		if( emb_data->time != 0x7fff ) emb_data->time += 1;
		if( !(emb_data->mode & EMB_MODE_ACTIVE) ) continue;

		if( DG_FABS(pos->vx-cam_pos.vx) > bound_dis
		 || DG_FABS(pos->vy-cam_pos.vy) > bound_dis
		 || DG_FABS(pos->vz-cam_pos.vz) > bound_dis ){
			emb_data->mode &= ~EMB_MODE_ACTIVE;
//printf("BOUND_STOP:%d\n",i);
			continue;
		}


		//printf("next %p\n",emb_data->next);

#if 0
		flag = 0;
		if( vec->vy < VEC_LIMIT ){
			flag = 1;
		}else if( work->bound_count ){
			if( UTL_EFT_CheckBound( pos ) >= 0 ){
				flag = 1;
			}
		}
		if( flag ){
			//終了処理 
			emb_data->mode &= ~EMB_MODE_ACTIVE;
			//if( pre ) pre->next = next;
			//else work->emb_list = next;
			//DelEmbList_emb( work, emb_data );
			work->n_act_allembs--;
			//printf("終了\n");
			continue;
		}
#else


		if( work->bound_count ){
			if( UTL_EFT_CheckBound( pos ) >= 0 ){
				//終了処理 
				emb_data->mode &= ~EMB_MODE_ACTIVE;
				//if( pre ) pre->next = next;
				//else work->emb_list = next;
				//DelEmbList_emb( work, emb_data );
				work->n_act_allembs--;
				//printf("終了\n");
				continue;
			}
		}
		if( pos->vy > GM_PlayerPosition.vy + 3500.0f ){
			//終了処理 
			emb_data->mode &= ~EMB_MODE_ACTIVE;
			work->n_act_allembs--;
			continue;
		}

#endif

		//max check
		if ( emb_data->time > emb_data->point_time ){
			emb_data->mode |= EMB_MODE_CALLEFT;
		}

		if( emb_data->mode & EMB_MODE_MOVE ){

			vec->vy += P_GRAVITY;

			//移動処理 
			_sceVu0AddVector( &fvtemp0, pos, vec );

			if( vec->vy < 0.0f ){
				far_ponit.vx = fvtemp0.vx;
				far_ponit.vy = fvtemp0.vy - UPPER_SHIFT;
				far_ponit.vz = fvtemp0.vz;
			}else{
				far_ponit.vx = fvtemp0.vx;
				far_ponit.vy = fvtemp0.vy + UPPER_SHIFT;
				far_ponit.vz = fvtemp0.vz;
			}

			h_flag = HZX_OnlineHazardCheck( hzx_id, pos, &far_ponit,
											HZX_CHK_FIX,
											HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
											HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_BLOOD | HZX_FLOOR_RECOIL_TYPE );

			if( h_flag & 2 ){	// 床 
				HZX_GetOnlineHazard( flr, atr ) ;
//printf("%f %f %f %f\n",flr->p1.y,flr->p2.y,flr->p3.y,flr->p4.y);
				if( (flr->p1.y != flr->p2.y)
				 || (flr->p1.y != flr->p3.y)
				 || (flr->p1.y != flr->p4.y) ){
					h_flag = 4;	// 反射だけさせる 
//printf("____________________:COME\n");
					HZX_GetOnlinePoint( &fvtemp1 );

					fvtemp0.vx = flr->p2.x - flr->p1.x;
					fvtemp0.vy = flr->p2.y - flr->p1.y;
					fvtemp0.vz = flr->p2.z - flr->p1.z;
					fvtemp2.vx = flr->p3.x - flr->p1.x;
					fvtemp2.vy = flr->p3.y - flr->p1.y;
					fvtemp2.vz = flr->p3.z - flr->p1.z;
					_sceVu0OuterProduct( &fvtemp0, &fvtemp0, &fvtemp2 );
					_sceVu0Normalize( &fvtemp0, &fvtemp0 );

//printf("a:%f %f %f\n",vec->vx,vec->vy,vec->vz);
					DG_ReflectVector( &fvtemp0, vec, vec );

					_sceVu0ScaleVector( vec, vec, DECAY_RATIO );
					if( GV_VecLen3F( vec ) < MIN_VEC ){
						_sceVu0Normalize( vec, vec );
						_sceVu0ScaleVector( vec, vec, MIN_VEC );
						if( GV_VecLen3F( vec ) < MIN_VEC ){
							vec->vy = 1.0f + rnd()*MIN_VEC;
						}
						if( ((irnd()>>8)%4)==0 ){
							vec->vx+= vec->vy*frnd();
							vec->vz+= vec->vy*frnd();
						}
					}

					emb_data->rot.vx = irnd()%512-256 ;
					emb_data->rot.vy = irnd()%512-256 ;
					emb_data->rot.vz = irnd()%512-256 ;

//printf("b:%f %f %f\n",vec->vx,vec->vy,vec->vz);
//					vec->vx =-vec->vx;
//					vec->vy =-vec->vy;
//					vec->vz =-vec->vz;
//					pos->vx = fvtemp1.vx + vec->vx;
//					pos->vy = fvtemp1.vy + vec->vy;
//					pos->vz = fvtemp1.vz + vec->vz;
				}
			}

			if( h_flag & 1 ){	// 壁 
				SetSound(vec, SD_W_CASE03, pos, GM_SEMODE_NORMAL);
				HZX_GetOnlinePoint( &fvtemp1 );
//				_sceVu0SubVector( &fvtemp2, &fvtemp1, &fvtemp0 );
				HZX_GetOnlineHazard( flr, atr ) ;
				seg=(HZX_SEG *)flr;
				fvtemp0.vx = seg->p2.z - seg->p1.z ;
				fvtemp0.vy = 0.0f ;
				fvtemp0.vz = seg->p1.x - seg->p2.x ;
				DG_ReflectVector( &fvtemp0, vec, vec );
				_sceVu0ScaleVector( vec, vec, DECAY_RATIO );
				pos->vx = fvtemp1.vx + vec->vx;
				pos->vy = fvtemp1.vy;
				pos->vz = fvtemp1.vz + vec->vz;			
				emb_data->rot.vx = irnd()%512-256 ;
				emb_data->rot.vy = irnd()%512-256 ;
				emb_data->rot.vz = irnd()%512-256 ;
			}
			if( h_flag & 2 ){	// 床 
				float temp = ( vec->vy > 0.0f )?vec->vy:-vec->vy;
				HZX_GetOnlinePoint( &fvtemp1 );
				HZX_GetOnlineHazard( flr, atr ) ;
				if( temp > 10.0f ) SetSound(vec, SD_W_CASE03, pos, GM_SEMODE_NORMAL);
				if( temp < DG_FABS(P_GRAVITY)*2.0f ){
					vec->vx = 0.0f;
					vec->vy = 0.0f;
					vec->vz = 0.0f;
					pos->vy = fvtemp1.vy + UPPER_SHIFT;
					emb_data->rot.vx = 0 ;
					emb_data->rot.vy = 0 ;
					emb_data->rot.vz = 1024 ;
					emb_data->const_rot.vx = 0;
					emb_data->const_rot.vz = 0;

					emb_data->mode &= ~(EMB_MODE_MOVE);
				
					if( atr[0] & HZX_FLOOR_NO_OBJECT ){	// 停止後の可視決定 
						//終了処理 
						//DelEmbList_emb( work, emb_data );
						//if( pre ) pre->next = next;
						//else work->emb_list = next;
						work->n_act_allembs--;
						emb_data->mode &= ~EMB_MODE_ACTIVE;
						//printf("終了\n");
						continue;
					}else{
					}
				}else{
					if( (irnd()>>8)%4==0  &&  temp > 10.0f ){
						float ftemp = bp_sqrtf( vec->vx * vec->vx  +  vec->vz * vec->vz );   //BP_MATH - emulate PS2 sqrtf
						float angle = rnd()*TPI;
						vec->vx = ftemp * sinf( angle ) * DECAY_RATIO_W;
						vec->vz = ftemp * cosf( angle ) * DECAY_RATIO_W;
					}else{
						vec->vx*= DECAY_RATIO_W;
						vec->vz*= DECAY_RATIO_W;
					}
					if( (irnd()>>8)%64==0 && temp > 10.0f ){
						vec->vy  = -vec->vy;
					}else{
						vec->vy *= -DECAY_RATIO;
					}
					pos->vx = fvtemp1.vx;
					pos->vy = fvtemp1.vy + vec->vy + UPPER_SHIFT;
					pos->vz = fvtemp1.vz;
					emb_data->rot.vx = irnd()%512-256 ;
					emb_data->rot.vy = irnd()%512-256 ;
					emb_data->rot.vz = irnd()%512-256 ;
				}
			}
			if( !h_flag ){
//printf("..:\n");
				DG_COPY_VEC( pos, &fvtemp0 );
			}

			//回転処理 
			if( emb_data->rot.vx > 0 ){
				emb_data->rot.vx--;
			}else if( emb_data->rot.vx < 0 ){
				emb_data->rot.vx++;
			}
			if( emb_data->rot.vy > 0 ){
				emb_data->rot.vy--;
			}else if( emb_data->rot.vy < 0 ){
				emb_data->rot.vy++;
			}
			if( emb_data->rot.vz > 0 ){
				emb_data->rot.vz--;
			}else if( emb_data->rot.vz < 0 ){
				emb_data->rot.vz++;
			}
			emb_data->const_rot.vx += emb_data->rot.vx;
			emb_data->const_rot.vy += emb_data->rot.vy;
			emb_data->const_rot.vz += emb_data->rot.vz;
		}
		//マトリックス生成 
		DG_SetPos2( pos, &emb_data->const_rot ) ;
		DG_GetPos( (FMATRIX*)cmdl_pos[emb_data->mdl_kind] );
		_MulMatrix( (FMATRIX*)cmdl_pos[emb_data->mdl_kind], emb_data->emb_id );

		//コモデルのシステムはゼロだめだから埋め合わせ 
		if( !n_comdl[emb_data->mdl_kind] ) cmdl_pos[emb_data->mdl_kind]->color.vw = 128;
		
		n_comdl[emb_data->mdl_kind]++;
		cmdl_pos[emb_data->mdl_kind]++;


	}

	//終了処理 
	//printf("usp %d: fms %d: max_kind %d\n", n_comdl[0], n_comdl[1],work->n_mdl_kind);
	for( i = 0; i < work->n_mdl_kind; i++ ){
		comdl[i]->n_objs = n_comdl[i];

		if( !n_comdl[i] ){
			//コモデルのシステムはゼロだめだから埋め合わせ 
			cmdl_pos[i] = comdl[i]->pos;
			cmdl_pos[i]->color.vw = 0;
			comdl[i]->n_objs = 1;
		}
		
	}
	
}

static void Die( Work *work )
{
	int i;
	for( i = 0; i < N_MDL_KIND; i++ ){
		if(work->comdl[i]){
			DG_DequeueComdlObjs( work->comdl[i] );
			DG_FreeComdl( work->comdl[i] );
		}
	}
	if( work->emb_data ) GV_DelayedFree( work->emb_data );
	_EnbControlWork = NULL;
}



static void InitComdlPos( DG_COMDL	*comdl, int n_comdl, int col )
{
	DG_COMDL_POS 	*comdl_pos;
	int				i;

	comdl_pos = comdl->pos;
	
	for( i = 0; i < n_comdl; i++ ){
		comdl_pos->color.vx = col&0xff;
		comdl_pos->color.vy = (col>>8)&0xff;
		comdl_pos->color.vz = (col>>16)&0xff;
		comdl_pos->color.vw = (col>>24)&0xff;;
		comdl_pos++;
	}
}


static int GetResources( Work *work )
{
	int				i;
	int				mdl_code[N_MDL_KIND] = {
		GV_StrCode( "usp_emb" ),
		GV_StrCode( "fms_emb" ),
		GV_StrCode("sps_emb"),
		GV_StrCode("demo_m4_grn_emb"),
	};
	work->n_mdl_kind = N_MDL_KIND;
	work->n_max_embs = GCL_GetOptionValue( 'n', 1 );
	if( !work->n_max_embs ){ printf("ERR!! 薬莢の数が０なら起動の必要ない<emb_control>\n" ); return -1; }

	if( GCL_GetNextOption() != 'r' ) return -1;
	for( i = 0; i < AMO_KIND_MAX; i++ ){
		int temp = GCL_GetNextInt();

		work->n_active_embs[i] = 0;		//
		work->emb_init_ratio[i] = (float)temp/(float)work->n_max_embs;
		work->emb_list_emb[i] = NULL;
		//if( i == AMO_KIND_M4GRN && !temp ) work->n_mdl_kind = 2;
	}
	
	UTL_EFT_AddCallback( BoundCallBack, work );

	for( i = 0; i < work->n_mdl_kind; i++ ){
		DG_COMDL  	*comdl;
		DG_DEF		*def;
printf("ooooooooooooooooo:%d\n",i);
		def = (DG_DEF *)GV_GetCache(GV_CacheID( mdl_code[i], 'k' )) ;

#if 0
		if( i < 2 && !def){ printf("ERR!! NO MODEL<emb_control>[%d]\n", mdl_code[i] ); return -1; }
		else if( !def){
			printf("ERR!! NO MODEL<emb_control>[%d]\n", mdl_code[i] );
			work->n_mdl_kind = 2;
			break;
		}
#else
		if( i < 2 && !def){
printf("ooooooooooooooooo:a\n");
			printf("ERR!! NO MODEL<emb_control>[%d]\n", mdl_code[i] );
			return -1;
		}else if( i==2 && !def ){
printf("ooooooooooooooooo:b\n");
			// sps_emb -> fms_emb
			def = (DG_DEF *)GV_GetCache(GV_CacheID( mdl_code[1], 'k' )) ;
			if( !def){
printf("ooooooooooooooooo:b0\n");
				printf("ERR!! NO MODEL<emb_control>[%d]\n", mdl_code[1] );
				return -1;
			}
		}else if( i==3 && !def ){
printf("ooooooooooooooooo:c\n");
			printf("ERR!! NO MODEL<emb_control>[%d]\n", mdl_code[i] );
			work->n_mdl_kind = 3;
			break;
		}else if( !def ){
printf("ooooooooooooooooo:d\n");
			printf("ERR!! NO MODEL<emb_control>[%d]\n", mdl_code[i] );
			return -1;
		}
#endif

		comdl = work->comdl[i] = DG_MakeComdl( def->models[0].packs, DG_COMDL_SEMITRANS, work->n_max_embs, 0 );
		if( !comdl ) return -1;

		InitComdlPos( comdl, work->n_max_embs, 0x80808080 );

		comdl->n_objs = 1;
		DG_COPY_MAT( (FMATRIX*)comdl->pos, &DG_UnitMatrix );
		comdl->pos->color.vw = 0;

		work->n_comdl[i] = 0;

		DG_QueueComdlObjs( comdl );
printf("ooooooooooooooooo:queque\n");
	}

	
	work->emb_data = (EMB_DATA*)GV_Malloc( sizeof(EMB_DATA)*work->n_max_embs );
	memset( work->emb_data, 0, sizeof(EMB_DATA)*work->n_max_embs );
	return (0);
}

void *NewEnbControl( int name, int map )
{
	Work		*work ;
	if( _EnbControlWork ){ printf("一個でいい！<emb_control>\n"); return (void*)1; }

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
		_EnbControlWork = work;
	}

	return (void *)work ;
}


