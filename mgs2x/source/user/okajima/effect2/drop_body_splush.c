//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	drop_body_splush.c
	体からの水飛沫：水面付近で発生

	1999/10/23 S.Okajima
	$Id: drop_body_splush.c,v 1.1.1.3 2002/11/19 11:47:19 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"
#include	"utl_dma.h"


#define	JOINT_NUM		(22)

#define	SCR_MAX_VEC_NUM		(0x4000 / 16)
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_VEC		(SCRPAD_ADDR + 0x2000)

#define	N_MUL		(4)

#define	N_VERTS		(32)
#define	N_PRIMS		(N_MUL)
#define	N_PARTS		(N_PRIMS*N_VERTS)
#define	N_UNITS		(JOINT_NUM)


#if 1
#define	TEX_NAME		(3594043 /*"drop01_msk"*/)
#define	P_ALPHA_MAX		(64)
#define	P_RGB_MAX		(64)
#define	SIZE			(6)
#define	SIZE_MIN		(4)
#else
#define	TEX_NAME		(8781729 /*"splash07_alp"*/)
#define	P_ALPHA_MAX		(32)
#define	P_RGB_MAX		(32)
#define	SIZE			(300)
#define	SIZE_MIN		(5)
#endif


#define	WIDTH_RND		(40.0f)
#define	VEC_MAX		(80.0f)
#define	VEC_MIN		(  5.0f)
#define	VEC_SCALE	(  1.1f)
#define	VEC_RND_PARAM	(0.50f)
#define	NRM_RND_PARAM	(0.10f)
#define	RIPPLE_LIMIT	(20.0f)
#define	BUBBLE_LIMIT	(50.0f)
#define	SKIP_NUM	(4)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
extern int	OK_PutSplushSurface( FVECTOR *center, FVECTOR *force );
extern int	OK_PutRipple( FVECTOR *center );
extern void *NewBubbleMotion( FVECTOR *pos, FVECTOR *vec, int life );

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;
	int			name;
	int			map;

	CONTROL		*control;
	DG_OBJS		*objs;
	SVECTOR		before_rot;
	FVECTOR		drop_step;
	FVECTOR		vec[N_UNITS*N_PRIMS*N_VERTS];

	DG_PRIM2	*prim ;

	int			bubble_count;

	int			prim_cycle;
	int			chara_name;
	int			n_models;
	int			n_verts[JOINT_NUM];
	FMATRIX		before_world[JOINT_NUM];
	FMATRIX		*world[JOINT_NUM];
	FVECTOR		*verts[JOINT_NUM];
	FVECTOR		*norms[JOINT_NUM];
	float		parts_length[JOINT_NUM];

	int			activate_flag[N_UNITS];
} Work ;

/* ---------------------------------------------------------------- */
static int SetData( FVECTOR *pos, FVECTOR *before_pos, FVECTOR *nrm, FVECTOR *prim_pos, FVECTOR *prim_vec )
{
	FVECTOR fvtemp;
	float	ratio;
	float	fparam0;
	float	fparam1;

	_sceVu0SubVector( &fvtemp, pos, before_pos ) ;
	fvtemp.vy = ( fvtemp.vy > 0.0f )? fvtemp.vy: -fvtemp.vy;

	ratio = ( DG_FABS(fvtemp.vx) > fvtemp.vy )? DG_FABS(fvtemp.vx): fvtemp.vy;
	ratio = ( DG_FABS(fvtemp.vz) > ratio     )? DG_FABS(fvtemp.vz): ratio;

	if( ratio < VEC_MIN ) return 0;

	prim_pos->vx = pos->vx + WIDTH_RND*frnd();
	prim_pos->vy = GM_WaterLevel;
	prim_pos->vz = pos->vz + WIDTH_RND*frnd();

	fparam0 =  0.3f + 0.2f*rnd();
	fparam1 = (0.5f - fparam0) * ratio;
	prim_vec->vx = fvtemp.vx * fparam0 + nrm->vx * fparam1 ;
	prim_vec->vy = fvtemp.vy * rnd()   + VEC_MIN;
	prim_vec->vz = fvtemp.vz * fparam0 + nrm->vz * fparam1;

	prim_vec->vw = 1.0f;

	if( ratio > VEC_MAX  ){
		prim_vec->vy*= 0.5f;
	}


	return 1;
}


/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	int i, j, k, n;
	int	clock;
	int	count0;
	int	prim_num;
	DG_PRIM2 *prim;
	float	ftemp;
	float	*part_len;
	FVECTOR	fvtemp;
	FVECTOR	*pos;
	FVECTOR	*nrm;
	FVECTOR	*before_pos;
	FVECTOR	*prim_pos;
	FVECTOR	*prim_vec;
	FVECTOR	*prim_pos_before;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh_before ;	/* スプライト用 */
	int		*n_verts;
	int		*activate;
	FVECTOR	**verts;
	FVECTOR	**norms;
	FMATRIX	**world;
	FMATRIX	*before_world;


	work->bubble_count++;


/*
	if( GV_PadData[1].status & PAD_L1 ){
		GM_WaterLevel+= 10.0f;
	}else if( GV_PadData[1].status & PAD_L2 ){
		GM_WaterLevel-= 10.0f;
	}
*/

//	GM_WaterLevel = 1000.0f;
//if(GV_Time%30==0)printf("GM_WaterLevel:%f\n",GM_WaterLevel);

	prim = work->prim;

	if( work->objs==NULL ){
		GV_DestroyActor( work ) ;
		DG_InvisiblePrim2( prim );
		return;
	}

	//内部条件
	work->control = GM_SearchWhere(work->chara_name);
	if( work->control==NULL ){
		GV_DestroyActor( work ) ;
		DG_InvisiblePrim2( prim );
		return;
	}

	DG_VisiblePrim2( prim );

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;


	prim_vec        = work->vec;
	prim_pos        = prim->pos[  clock];
	prim_pos_before = prim->pos[1-clock];
	uvrgbwh         = prim->uvrgb[  clock ];
	uvrgbwh_before  = prim->uvrgb[1-clock ];
	activate        = work->activate_flag;

	for( i=0; i<work->n_models; i++ ){
		if( (*activate) > 0 ){
			OK_Mem_Scr( SCR_POS, prim_pos_before, sizeof(FVECTOR), N_PARTS) ;
			pos = SCR_POS;
			count0 = 0;
			for( j=0; j<N_PARTS; j++ ){
				if( prim_vec->vw != 0.0f ){
					count0++;

					pos->vx+= prim_vec->vx;
					pos->vy+= prim_vec->vy;
					pos->vz+= prim_vec->vz;

					if( pos->vy < GM_WaterLevel ){
						prim_vec->vw = 0.0f;
						pos->vy = GM_WaterLevel;
						if( prim_vec->vy < -RIPPLE_LIMIT ){
							if( (count0&1)==0 ){


								OK_PutRipple( pos );


							}
						}
					}else{
						prim_vec->vw = 1.0f;
						prim_vec->vy+= P_GRAVITY;
						uvrgbwh->w = uvrgbwh->h = (irnd()>>8)%SIZE + SIZE_MIN;
					}
				}else{
					uvrgbwh->w = uvrgbwh->h = 0;
				}

				pos++;
				prim_vec++;
				uvrgbwh++;
				uvrgbwh_before++;
			}
			OK_Scr_Mem( prim_pos       , SCR_POS, sizeof(FVECTOR), N_PARTS) ;
			if( count0==0 ){
				(*activate)--;
			}else{
				(*activate)=2;
			}
		}else{
			prim_vec       += N_PARTS;
			uvrgbwh        += N_PARTS;
			uvrgbwh_before += N_PARTS;
		}
		prim_pos       += N_PARTS;
		prim_pos_before+= N_PARTS;
		activate++;
	}



//	prim_num = N_UNITS * N_PRIMS * N_VERTS;
	prim_num = N_PARTS;
	prim_vec = work->vec;
	prim_pos = prim->pos[  clock];
	uvrgbwh  = prim->uvrgb[ clock ];
	activate = work->activate_flag;
	activate+= work->prim_cycle;

	n = work->prim_cycle * N_PARTS;
	prim_pos+=        n;
	prim_vec+=        n;
	uvrgbwh +=        n;

	work->prim_cycle++;
//	if( work->prim_cycle >= N_UNITS) work->prim_cycle = 0;
	if( work->prim_cycle >= work->n_models) work->prim_cycle = 0;

	part_len     = work->parts_length;
	n_verts      = work->n_verts;
	world        = work->world;
	before_world = work->before_world;
	verts        = work->verts;
	norms        = work->norms;
	i = work->n_models ;
//printf("::\n");
	count0 = 0;
	ftemp = 0.0f;
	while( --i>=0 ){
		pos        = (FVECTOR *)(*world)->m[3];
		before_pos = (FVECTOR *)before_world->m[3];
//printf("%f:%f:::%f:%f\n",before_pos->vy,pos->vy,(*part_len),GM_WaterLevel);
		if( ( (before_pos->vy + (*part_len) > GM_WaterLevel) && ( pos->vy - (*part_len) < GM_WaterLevel) )
		 || ( (before_pos->vy - (*part_len) < GM_WaterLevel) && ( pos->vy + (*part_len) > GM_WaterLevel) )
		  ){
//printf("%d\n",i);
			OK_Mem_Scr( SCR_POS, *verts,  sizeof(FVECTOR), *n_verts) ;
			DG_SetPos( before_world );
			DG_PutVector( SCR_POS, SCR_VEC, *n_verts );
			DG_SetPos( *world );
			DG_PutVector( SCR_POS, SCR_POS, *n_verts );

//			DG_COPY_VEC( &master_pos, (FVECTOR *)(*world)->m[3] );

			pos        = SCR_POS;
			before_pos = SCR_VEC;
			nrm        =(*norms);
			n    = *n_verts;
			while ( -- n >= 0 ) {
				if( (pos->vy <= GM_WaterLevel  &&  before_pos->vy > GM_WaterLevel)
				 || (pos->vy  > GM_WaterLevel  &&  before_pos->vy<= GM_WaterLevel) ){
					if( prim_num >= N_MUL ){
						prim_num -= N_MUL;
						for( k=0; k<N_MUL; k++ ){
							if( SetData( pos, before_pos, nrm, prim_pos, prim_vec ) ){
								if(k==0){
									OK_PutSplushSurface( prim_pos, prim_vec );

									if( pos->vy <= GM_WaterLevel ){
										if( (n + work->bubble_count)%SKIP_NUM == 0 ){
											_sceVu0SubVector( &fvtemp, pos, before_pos );
											if( GV_VecLen3F( &fvtemp ) > BUBBLE_LIMIT ){
												NewBubbleMotion( pos, &fvtemp, 64 );
											}
										}
									}

								}
								(*activate)=2;
								count0++;
								prim_pos++;
								prim_vec++;
								uvrgbwh++;
							}
						}
					}
				}
				pos++ ;
				before_pos++ ;
				nrm++;
			}
		}

		DG_COPY_MAT( before_world, (*world) );

		part_len++;
		n_verts++;
		world++;
		before_world++;
		verts++;
		norms++;
	}

/*
	for( k=0; k<prim_num; k++ ){
		prim_vec->vw = 0.0f;
		uvrgbwh->w = uvrgbwh->h = 0;
		prim_vec++;
		uvrgbwh++;
	}
*/

}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < work->n_models * N_PRIMS ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

			uvrgbwh1->w = uvrgbwh0->w = 0;
			uvrgbwh1->h = uvrgbwh0->h = 0;

			uvrgbwh1->r = uvrgbwh0->r = P_RGB_MAX ;
			uvrgbwh1->g = uvrgbwh0->g = P_RGB_MAX ;
			uvrgbwh1->b = uvrgbwh0->b = P_RGB_MAX ;
			uvrgbwh1->a = uvrgbwh0->a = P_ALPHA_MAX ;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * work->n_models * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * work->n_models * N_PRIMS * N_VERTS );

}


static int GetResources( Work *work, DG_OBJS *org_objs, CONTROL *control, int model_id )
{
	CV2_DEF		*cvd_def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int		i,j ;
	FVECTOR		*fvtemp0;
	FVECTOR		*fvtemp1;

	int			itemp ;
	float		ftemp ;
	FVECTOR		*scr_pos;

	work->bubble_count = 0;
	work->prim_cycle = 0;
	work->chara_name = control->name;
	work->objs=org_objs;
	work->control = control;
	work->n_models = (work->objs->n_models < JOINT_NUM)? work->objs->n_models : JOINT_NUM ; 

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_models * N_PRIMS, N_VERTS );
//	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_UNITS * N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( TEX_NAME );

	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( work->prim );

	/* モデルの共有頂点データ取得 */
	cvd_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );

	i = work->n_models ; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
		if( work->n_verts[i] > SCR_MAX_VEC_NUM ) work->n_verts[i] = SCR_MAX_VEC_NUM;

		work->world[i]   = &org_objs->objs[i].world;
		fvtemp0 = work->verts[i]   = cvd_def->models[i].verts;
		fvtemp1 = work->norms[i]   = cvd_def->models[i].norms;
		for( j=0; j<work->n_verts[i]; j++ ){
			if( _sceVu0InnerProduct( fvtemp0, fvtemp1 ) < 0.0f ){
				_sceVu0ScaleVector( fvtemp1, fvtemp1, -1.0f );
			}
			fvtemp0++;
			fvtemp1++;
		}

		OK_Mem_Scr( SCR_POS, work->verts[i],  sizeof(FVECTOR), work->n_verts[i]) ;
		itemp = work->n_verts[i];
		scr_pos = SCR_POS;
		ftemp = 0.0f;
		while( --itemp>=0 ){
			scr_pos->vx = DG_FABS( scr_pos->vx );
			scr_pos->vy = DG_FABS( scr_pos->vy );
			scr_pos->vz = DG_FABS( scr_pos->vz );
			scr_pos->vw = ( scr_pos->vx > scr_pos->vy )? scr_pos->vx: scr_pos->vy;
			scr_pos->vw = ( scr_pos->vw > scr_pos->vz )? scr_pos->vw: scr_pos->vz;
			ftemp       = ( ftemp       > scr_pos->vw )? ftemp:       scr_pos->vw;
			scr_pos++;
		}
		work->parts_length[i] = ftemp;
	}


	for ( i=0; i<N_UNITS; i++ ){
		work->activate_flag[i] = 0;
	}

	return (0);
}


/* ---------------------------------------------------------------- */
void *NewDropBodySplush( DG_OBJS *objs, CONTROL *control, int model_id )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
//printf("drop_bdy_splush::::::::::::::::::::::::::::::work_size:%x\n",sizeof( Work ));
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, objs, control, model_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

