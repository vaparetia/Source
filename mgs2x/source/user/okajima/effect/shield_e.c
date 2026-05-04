//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	shield_e.c
	盾エフェクト
	1999/11/15 S.Okajima
	$Id: shield_e.c,v 1.1.1.3 2002/11/19 11:47:13 Yoshizawa1 Exp $

*/
#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"blood.h"


/* 壊れ盾ライト */
#define		FALL_NAME		(0x2b6590)	/* "shl_lit2" */

#define		SHIELD_SIZE_X	(180.0f)
#define		SHIELD_SIZE_Y	(800.0f)
#define		SHIELD_SIZE_Z	(500.0f)

/* 認識できる盾の数 */
#define		N_SHIELD	(8)
/* 全弾痕数 */
#define		N_BREAK_POINT	(64) 
/* 全血のり数 */
#define		N_BLOOD_POINT	(16) 

#define		DUNKON_MODEL1	(2528399 /*"shl_bul1"*/)
#define		DUNKON_MODEL2	(2528400 /*"shl_bul2"*/)
#define		DUNKON_MODEL3	(2528401 /*"shl_bul3"*/)
#define		DUNKON_MODEL4	(2528402 /*"shl_bul4"*/)
#define		DUNKON_MODEL5	(2528403 /*"shl_bul5"*/)
#define		DUNKON_MODEL6	(2528404 /*"shl_bul6"*/)

#define		DUNKON_WIDTH1	(45.00f)
#define		DUNKON_WIDTH2	(47.00f)
#define		DUNKON_WIDTH3	(49.00f)
#define		DUNKON_WIDTH4	(55.00f)
#define		DUNKON_WIDTH5	(67.00f)
#define		DUNKON_WIDTH6	(78.00f)

#define		MAX_MODEL_PATTERN	(6)


#define		DUNKON_MODEL_L1	(2519185 /*"shl_bll3"*/)
#define		DUNKON_MODEL_L2	(2519184 /*"shl_bll2"*/)
#define		DUNKON_MODEL_L3	(2519183 /*"shl_bll1"*/)
#define		DUNKON_MODEL_R1	(2519377 /*"shl_blr3"*/)
#define		DUNKON_MODEL_R2	(2519376 /*"shl_blr2"*/)
#define		DUNKON_MODEL_R3	(2519375 /*"shl_blr1"*/)
#define		DUNKON_MODEL_U1	(2519473 /*"shl_blu3"*/)
#define		DUNKON_MODEL_U2	(2519472 /*"shl_blu2"*/)
#define		DUNKON_MODEL_U3	(2519471 /*"shl_blu1"*/)
#define		DUNKON_MODEL_D1	(2518929 /*"shl_bld3"*/)
#define		DUNKON_MODEL_D2	(2518928 /*"shl_bld2"*/)
#define		DUNKON_MODEL_D3	(2518927 /*"shl_bld1"*/)

#define		DUNKON_WIDTH_SLICE1	(10.00f)
#define		DUNKON_WIDTH_SLICE2	(17.00f)
#define		DUNKON_WIDTH_SLICE3	(25.00f)

#define		MAX_MODEL_SLICE_PATTERN	(3)


#define		BLOOD_MODEL0	(2547759 /*"shl_chi1"*/)
#define		BLOOD_MODEL1	(2547760 /*"shl_chi2"*/)
#define		BLOOD_MODEL2	(2547761 /*"shl_chi3"*/)
#define		BLOOD_MODEL3	(2547762 /*"shl_chi4"*/)
#define		BLOOD_MODEL4	(2547763 /*"shl_chi5"*/)

#define		BLOOD_WIDTH1	(70.0f)
#define		BLOOD_WIDTH2	(60.0f)

#define		MAX_BLOOD_PATTERN	(5)

#define		R_RADIUS			(1000.5f)
#define		TARGET_SHIFT_X			(67.5f)
#define		TARGET_SHIFT_X_BL		(67.5f-30.0f)
//#define		TARGET_SHIFT_X_BL		(67.5f)
//#define		PRIORITY_SHIFT		(8.0f)
//#define		PRIORITY_SHIFT		(-12.0f)
#define		PRIORITY_SHIFT		(0.0f)

/* SHIELD */
#define	SHIFT2_X			(54.0f)
#define	SHIFT2_Y			(-100.0f)
#define	SHIFT2_Z			(203.0f)

static	int	model_name[MAX_MODEL_PATTERN];
static	int	model_width[MAX_MODEL_PATTERN];

static	int	model_name_l[MAX_MODEL_SLICE_PATTERN];
static	int	model_name_r[MAX_MODEL_SLICE_PATTERN];
static	int	model_name_u[MAX_MODEL_SLICE_PATTERN];
static	int	model_name_d[MAX_MODEL_SLICE_PATTERN];
static	int	model_width_slice[MAX_MODEL_SLICE_PATTERN];

static	int	bl_model_name[MAX_BLOOD_PATTERN];
static	int	bl_model_width[2];

static	int		width_sw;
static	int		hight_sw;

typedef	struct	{
	GV_ACT_EX		actor ;

	OBJECT		*shield[N_SHIELD] ;
	FMATRIX		light[N_SHIELD][2];
	FMATRIX		bl_light[N_SHIELD][2];

	/* 弾痕 */
	DG_OBJS		*objs[N_BREAK_POINT] ;
	FVECTOR		mem_pos[N_BREAK_POINT];
	FVECTOR		pos[N_BREAK_POINT];
	short		rot_y[N_BREAK_POINT];
	int			objs_to_shield[N_BREAK_POINT];

	/* 血のり */
	DG_OBJS		*bl_objs[N_BLOOD_POINT] ;
	FVECTOR		bl_mem_pos[N_BLOOD_POINT];
	FVECTOR		bl_pos[N_BLOOD_POINT];
	short		bl_rot_y[N_BLOOD_POINT];
	int			bl_objs_to_shield[N_BLOOD_POINT];


} Work ;

Work	*OK_SHIELD_WORK = NULL;

static int MakeBloodObjs( Work *work, int p_num, int s_num, int model_num )
{
	DG_DEF		*def ;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( bl_model_name[model_num], 'k' ) ) ;

	work->bl_objs[p_num] = DG_MakeObjs( def, DG_FLAG_FINISHCALC|DG_FLAG_ONEPIECE|DG_FLAG_NOFOG, 0 );
	if( work->bl_objs[p_num]==NULL ) return 0;

	work->bl_objs[p_num]->group_id = work->shield[s_num]->objs->group_id;

	DG_QueueObjs( work->bl_objs[p_num] );
//	DG_SetLightMatrix( work->bl_objs[p_num], work->shield[s_num]->objs->light );
	DG_SetLightMatrix( work->bl_objs[p_num], work->bl_light[s_num] );
//printf("make::%d:%d\n",p_num,s_num);
	return 1;
}

static int MakeDankonObjs( Work *work, int p_num, int s_num, int model_num )
{
	DG_DEF		*def ;

	if( model_num<100 ){
		def = (DG_DEF*)GV_GetCache( GV_CacheID( model_name[model_num], 'k' ) ) ;
	}else{
		model_num-=100;
		if(width_sw!=-1){
			if(width_sw==0){
//printf("RRR\n");
				def = (DG_DEF*)GV_GetCache( GV_CacheID( model_name_r[model_num], 'k' ) ) ;
			}else{
//printf("LLL\n");
				def = (DG_DEF*)GV_GetCache( GV_CacheID( model_name_l[model_num], 'k' ) ) ;
			}
		}else{
			if(hight_sw==0){
//printf("DDD\n");
				def = (DG_DEF*)GV_GetCache( GV_CacheID( model_name_d[model_num], 'k' ) ) ;
			}else{
//printf("UUU\n");
				def = (DG_DEF*)GV_GetCache( GV_CacheID( model_name_u[model_num], 'k' ) ) ;
			}
		}
	}


	work->objs[p_num] = DG_MakeObjs( def, DG_FLAG_FINISHCALC|DG_FLAG_ONEPIECE|DG_FLAG_NOFOG, 0 );
	if( work->objs[p_num]==NULL ) return 0;

	work->objs[p_num]->group_id = work->shield[s_num]->objs->group_id;

	DG_QueueObjs( work->objs[p_num] );
//	DG_SetLightMatrix( work->objs[p_num], work->shield[s_num]->objs->light );
	DG_SetLightMatrix( work->objs[p_num], work->light[s_num] );
//printf("make::%d:%d\n",p_num,s_num);

	return 1;
}
static void DeleteDankonObjs( Work *work, int p_num )
{
	if( work->objs[p_num]!=NULL ){
//printf("delete::%d:%d\n",p_num,work->objs_to_shield[p_num]);
		DG_DequeueObjs( work->objs[p_num] );
		DG_FreeObjs( work->objs[p_num] );
		work->objs[p_num]=NULL;
	}
}
static void DeleteDankonObjs_BL( Work *work, int p_num )
{
	if( work->bl_objs[p_num]!=NULL ){
//printf("delete::%d:%d\n",p_num,work->objs_to_shield[p_num]);
		DG_DequeueObjs( work->bl_objs[p_num] );
		DG_FreeObjs( work->bl_objs[p_num] );
		work->bl_objs[p_num]=NULL;
	}
}
static	void Act( Work *work )
{
	int	i;
	int	flag;
	int	drop_mem;
	SVECTOR	rot;
	FVECTOR	*mem_pos;
	int		*objs_to_shield;
	FMATRIX	*pmat0;
	FMATRIX	*pmat1;
	FMATRIX	*bl_pmat0;
	FMATRIX	*bl_pmat1;
	FVECTOR	fvtemp0;
	OBJECT	**shield_model ;

	shield_model=work->shield;
	if( shield_model==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}


	pmat0=&work->light[0][0];
	pmat1=pmat0;
	pmat1++;
	bl_pmat0=&work->bl_light[0][0];
	bl_pmat1=bl_pmat0;
	bl_pmat1++;
	for( i=0; i<N_SHIELD; i++ ){
		if( (*shield_model)!=NULL ){
			if( (*shield_model)->objs!=NULL ){
				DG_COPY_MAT( pmat0, &(*shield_model)->objs->light[0] );
				DG_COPY_MAT( pmat1, &(*shield_model)->objs->light[1] );

				DG_COPY_VEC( &fvtemp0, (FVECTOR *)pmat1->m[3] );
				fvtemp0.vx = (fvtemp0.vx<32.0f)?32.0f:fvtemp0.vx;
				fvtemp0.vy = (fvtemp0.vy<32.0f)?32.0f:fvtemp0.vy;
				fvtemp0.vz = (fvtemp0.vz<32.0f)?32.0f:fvtemp0.vz;
				DG_COPY_VEC( (FVECTOR *)pmat1->m[3], &fvtemp0 );

				DG_COPY_VEC( &fvtemp0, (FVECTOR *)pmat1->m[1] );
				fvtemp0.vx = (fvtemp0.vx<32.0f)?32.0f:fvtemp0.vx;
				fvtemp0.vy = (fvtemp0.vy<32.0f)?32.0f:fvtemp0.vy;
				fvtemp0.vz = (fvtemp0.vz<32.0f)?32.0f:fvtemp0.vz;
				DG_COPY_VEC( (FVECTOR *)pmat1->m[1], &fvtemp0 );

				DG_COPY_MAT( bl_pmat0, pmat0 );
				DG_COPY_MAT( bl_pmat1, pmat1 );
				bl_pmat1->m[3][0]=128.0f;
				bl_pmat1->m[3][1]=128.0f;
				bl_pmat1->m[3][2]=128.0f;
			}
		}
		pmat0 += 2;
		pmat1 += 2;
		bl_pmat0 += 2;
		bl_pmat1 += 2;
		shield_model++;
	}

	mem_pos        = work->mem_pos;
	objs_to_shield = work->objs_to_shield;
	rot.vx=0;
	rot.vz=0;
	for( i=0; i<N_BREAK_POINT; i++ ){
		if( (*objs_to_shield) == -1 ){
		}else if(work->shield[*objs_to_shield]==NULL ){	/* 念のため */
			DeleteDankonObjs( work, i );
			(*objs_to_shield) = -1;
		}else if( work->shield[*objs_to_shield]->flag & DG_FLAG_INVISIBLE ){
			work->objs[i]->flag |=  DG_FLAG_INVISIBLE ;
		}else{
			work->objs[i]->flag &= ~DG_FLAG_INVISIBLE ;

			DG_SetPos( &work->shield[*objs_to_shield]->objs->world ) ;
			DG_MovePos( mem_pos );
			rot.vy=work->rot_y[i];
			DG_RotatePos( &rot );
			DG_PutObjs( work->objs[i] );
			GM_GroupObjs( work->objs[i], GM_CurrentStageMap );
		}

		if( (*objs_to_shield) != -1 ){
			work->objs[i]->group_id = work->shield[(*objs_to_shield)]->objs->group_id;
		}

		mem_pos++;
		objs_to_shield++;
	}



	mem_pos        = work->bl_mem_pos;
	objs_to_shield = work->bl_objs_to_shield;
	rot.vx=0;
	rot.vz=0;
	drop_mem=-1;
	for( i=0; i<N_BLOOD_POINT; i++ ){
		flag=1;
		if( (*objs_to_shield) == -1 ){
			flag=0;
		}else if(work->shield[*objs_to_shield]==NULL ){	/* 念のため */
			DeleteDankonObjs_BL( work, i );
			(*objs_to_shield) = -1;
			flag=0;
		}else if( (work->shield[*objs_to_shield]->flag & DG_FLAG_INVISIBLE)
			   || (GM_Configuration & GM_CONFIG_BLOOD_OFF) ){
			work->bl_objs[i]->flag |=  DG_FLAG_INVISIBLE ;
		}else{
			work->bl_objs[i]->flag &= ~DG_FLAG_INVISIBLE ;
		}

		if( flag ){
			DG_SetPos( &work->shield[*objs_to_shield]->objs->world ) ;
			DG_MovePos( mem_pos );
			rot.vy=work->bl_rot_y[i];
			DG_RotatePos( &rot );
			DG_PutObjs( work->bl_objs[i] );
			GM_GroupObjs( work->bl_objs[i], GM_CurrentStageMap );
			drop_mem=i;
		}

		if( (*objs_to_shield) != -1 ){
			work->bl_objs[i]->group_id = work->shield[(*objs_to_shield)]->objs->group_id;
		}

		mem_pos++;
		objs_to_shield++;
	}
/*
	if(drop_mem!=-1 && GV_Time%180==0){
		extern void *NewBloodDrops( FVECTOR *pos, int decay, int white, int addr ) ;
		NewBloodDrops( (FVECTOR *)work->bl_objs[drop_mem]->world.m[3], 360, 0, -1 ) ;
	}
*/
}

static int CheckMakeInnerShield_BL( FVECTOR *center, short *rot_y )
{
	float	ftemp1;
	float	ftemp2;
	float	range_x;
	float	range_y;
	int		i;
	int		model_num;
	int		ret;
	int		temp_width_sw;
	int		temp_hight_sw;

//printf("center:%f:%f:%f\n",center->vx,center->vy,center->vz);

	if( center->vz<0 ){
		temp_width_sw=0;
		range_x =  SHIELD_SIZE_Z*0.5f + center->vz;
	}else{
		temp_width_sw=1;
		range_x =  SHIELD_SIZE_Z*0.5f - center->vz;
	}

	if( center->vy<0 ){
		temp_hight_sw=0;
		range_y =  SHIELD_SIZE_Y*0.5f + center->vy;
	}else{
		temp_hight_sw=1;
		range_y =  SHIELD_SIZE_Y*0.5f - center->vy;
	}
	if( range_x < 70.0f && range_y < 70.0f ) return -1;	/* ４角すり抜け */

//printf("range_x:%f\n",range_x);
//printf("range_y:%f\n",range_y);

	model_num=irnd()%2;

	ret=0;
	for(i=model_num; i>=0; i--){
//printf("%d\n",i);
		if( bl_model_width[i] < range_x  &&  bl_model_width[i] < range_y ){
			ret=i;
			break;
		}
	}
//printf("---\n");
	if(i<=-1) return -1;

//	printf("center:%f:%f:%f\n",center->vx,center->vy,center->vz);
	ftemp1=asinf( center->vz / R_RADIUS );	/* 中心からの角度 */
	ftemp2= R_RADIUS * (1-cosf( ftemp1 ));	/* 垂直接線からの距離 */
	center->vx = TARGET_SHIFT_X_BL - ftemp2 + PRIORITY_SHIFT;
	*rot_y=(short)(-2048.0f * ftemp1 / PI);
	return ret+3;
}

static int CheckMakeInnerShield( FVECTOR *center, short *rot_y )
{
	float	ftemp1;
	float	ftemp2;
	float	range_x;
	float	range_y;
	int		i;
	int		model_num;
	int		ret;
	int		temp_width_sw;
	int		temp_hight_sw;

//printf("center:%f:%f:%f\n",center->vx,center->vy,center->vz);

	if( center->vz<0 ){
		temp_width_sw=0;
		range_x =  SHIELD_SIZE_Z*0.5f + center->vz;
	}else{
		temp_width_sw=1;
		range_x =  SHIELD_SIZE_Z*0.5f - center->vz;
	}

	if( center->vy<0 ){
		temp_hight_sw=0;
		range_y =  SHIELD_SIZE_Y*0.5f + center->vy;
	}else{
		temp_hight_sw=1;
		range_y =  SHIELD_SIZE_Y*0.5f - center->vy;
	}
	if( range_x < 70.0f && range_y < 70.0f ) return -1;	/* ４角すり抜け */

//printf("range_x:%f\n",range_x);
//printf("range_y:%f\n",range_y);

	width_sw=-1;
	hight_sw=-1;
	model_num=irnd()%MAX_MODEL_PATTERN;
	ret=0;
	for(i=model_num; i>=0; i--){
//printf("%d\n",i);
		if( model_width[i] < range_x  &&  model_width[i] < range_y ){
			ret=i;
			break;
		}
	}
//printf("---\n");
	/* ○ではだめだったので半月 */
	if(i<=-1){
		for(i=MAX_MODEL_SLICE_PATTERN-1; i>=0; i--){
//printf("%d\n",i);
			if( range_x < range_y){
				if( model_width_slice[i] < range_x ){
					width_sw=temp_width_sw;
					if( width_sw==0 ){
						center->vz =-SHIELD_SIZE_Z*0.5f + model_width_slice[i];
						center->vz+=2.0f;	/* 左右微調整 */
					}else{
						center->vz = SHIELD_SIZE_Z*0.5f - model_width_slice[i];
						center->vz-=2.0f;	/* 左右微調整 */
					}
					ret=i;
					break;
				}
			}else{
				if( model_width_slice[i] < range_y ){
					hight_sw=temp_hight_sw;
					if( hight_sw==0 ){
						center->vy =-SHIELD_SIZE_Y*0.5f + model_width_slice[i];
					}else{
						center->vy = SHIELD_SIZE_Y*0.5f - model_width_slice[i];
					}
					ret=i;
					break;
				}
			}
		}
		ret=i+100;		/* 返り値によりモデル変える */
//printf("SLICE:%d\n",i);
		if(i<=-1){
//printf("OVER\n");
			return -1;
		}
	}

//	printf("center:%f:%f:%f\n",center->vx,center->vy,center->vz);
	ftemp1=asinf( center->vz / R_RADIUS );	/* 中心からの角度 */
	ftemp2= R_RADIUS * (1-cosf( ftemp1 ));	/* 垂直接線からの距離 */
	center->vx = TARGET_SHIFT_X - ftemp2 + PRIORITY_SHIFT;
	*rot_y=(short)(-2048.0f * ftemp1 / PI);
	return ret;
}

/* flag==0 で 破壊 */
/* flag!=0 で 消す */
void PutShieldEffectBreak( OBJECT *shield, int flag )
{
	int	i;
	int	mem;
	int	bl_mem;
	int	*pitemp;

	if( OK_SHIELD_WORK==NULL ) return;

	mem=-1;
	pitemp=OK_SHIELD_WORK->objs_to_shield;
	for( i=0; i<N_BREAK_POINT; i++ ){
//printf("*pitemp:%d\n",*pitemp);
		if( *pitemp != -1 ){
			if(OK_SHIELD_WORK->shield[*pitemp]==shield ){
				mem=*pitemp;
				break;
			}
		}
		pitemp++;
	}
	if( mem!=-1 ){
		pitemp=OK_SHIELD_WORK->objs_to_shield;
		for( i=0; i<N_BREAK_POINT; i++ ){
//printf("*pitemp:%d\n",*pitemp);
			if( *pitemp == mem ){
				DeleteDankonObjs( OK_SHIELD_WORK, i );
			}
			pitemp++;
		}
	}

	bl_mem=-1;
	pitemp=OK_SHIELD_WORK->bl_objs_to_shield;
	for( i=0; i<N_BLOOD_POINT; i++ ){
//printf("*pitemp:%d\n",*pitemp);
		if( *pitemp != -1 ){
			if(OK_SHIELD_WORK->shield[*pitemp]==shield ){
				bl_mem=*pitemp;
				break;
			}
		}
		pitemp++;
	}
	if( bl_mem!=-1 ){
		pitemp=OK_SHIELD_WORK->bl_objs_to_shield;
		for( i=0; i<N_BLOOD_POINT; i++ ){
//printf("*pitemp:%d\n",*pitemp);
			if( *pitemp == bl_mem ){
				DeleteDankonObjs_BL( OK_SHIELD_WORK, i );
			}
			pitemp++;
		}
	}



	if(flag==0){
		extern void *NewCrushShield( OBJECT *shield );
		extern void *NewObjectFall( FVECTOR *pos, SVECTOR *rot, FVECTOR *vec, int model_name, float bound_ratio, int life );

		/* 盾こなごな破壊 */
		NewCrushShield( shield );

#if 0
		/* 盾ライト落下 */
		fvtemp.vx=SHIFT2_X;
		fvtemp.vy=SHIFT2_Y;
		fvtemp.vz=SHIFT2_Z;
		DG_SetPos( &shield->objs->world );
		DG_MovePos( &fvtemp );
		DG_GetPos( &mat );
		DG_COPY_VEC( &fvtemp, (FVECTOR *)mat.m[3] );
		svtemp.vx = irnd()%4096;
		svtemp.vy = irnd()%4096;
		svtemp.vz = irnd()%4096;
		NewObjectFall( &fvtemp, &svtemp, &DG_ZeroVector, FALL_NAME, 0.6f, 180 );
#endif
	}
	/* この上下は順番が関係する */
	if(mem!=-1){
//printf("pb1:%d:%d\n",mem,OK_SHIELD_WORK->shield[mem]);
		OK_SHIELD_WORK->shield[mem]=NULL;
	}
	/* この上下は順番が関係する */
	if(bl_mem!=-1){
//printf("pb2:%d\n",bl_mem,OK_SHIELD_WORK->shield[bl_mem]);
		OK_SHIELD_WORK->shield[bl_mem]=NULL;
	}

}


void PutShieldEffectPos_BL( OBJECT *shield ,int joint )
{
	int	i;
	int	p_num;
	int	s_num;
	int	model_num;
	FVECTOR	*pfvtemp;
	OBJECT	**shield_model ;

	if( OK_SHIELD_WORK==NULL ) return;

	for( i=0; i<N_BLOOD_POINT; i++ ){
		if( OK_SHIELD_WORK->bl_objs_to_shield[i]==-1 ) break;
	}
	if( i==N_BLOOD_POINT ) return;
	p_num=i;	/* 空きがあった */

	s_num=0;
	shield_model=OK_SHIELD_WORK->shield;
	for( i=0; i<N_SHIELD; i++ ){
		/* 同じ盾を発見した*/
		if( *shield_model==shield){
			s_num=i;
			break;
		}
		shield_model++;
	}
	if( i==N_SHIELD ){
		shield_model=OK_SHIELD_WORK->shield;
		for( i=0; i<N_SHIELD; i++ ){
			/* 新規に盾を登録 */
			if( *shield_model==NULL ){
				s_num=i;
				break;
			}
			shield_model++;
		}
		if( i==N_SHIELD ) return;
	}
	/* p_num 番目のモデルは、s_num 番目の盾に引っ付く */

	pfvtemp=&OK_SHIELD_WORK->bl_mem_pos[p_num];
	pfvtemp->vx = 0.0f;
	pfvtemp->vy = 0.0f;
	pfvtemp->vz = 0.0f;
	switch( joint ){
		/* 即死 */
	  case -1:
		model_num=0;
		break;
		/* 右足 */
	  case 17:
	  case 18:
	  case 19:
	  case 20:
		model_num=1;
		break;
		/* 左足 */
	  case 13:
	  case 14:
	  case 15:
	  case 16:
		model_num=2;
		break;
	  default:
		pfvtemp->vx = SHIELD_SIZE_X * rnd() - SHIELD_SIZE_X*0.5f;
		pfvtemp->vy = SHIELD_SIZE_Y * rnd() - SHIELD_SIZE_Y*0.5f;
		pfvtemp->vz = SHIELD_SIZE_Z * rnd() - SHIELD_SIZE_Z*0.5f;
		model_num=CheckMakeInnerShield_BL( pfvtemp, &OK_SHIELD_WORK->bl_rot_y[p_num] );
		if( model_num==-1 ) return;
		break;
	}
	/* 登録作業 */
	*shield_model=shield ;	/* 一部、二度手間 */

//printf("model_num::::%d\n",model_num);

	if( MakeBloodObjs( OK_SHIELD_WORK, p_num, s_num, model_num ) ){
		OK_SHIELD_WORK->bl_objs_to_shield[p_num]=s_num;
	}

}

void PutShieldBloodFromPlayer( OBJECT *shield )
{
	int i;
	for( i=0; i<8; i++ ){
		PutShieldEffectPos_BL( shield ,0 );
	}
}


/* 弾痕登録 */
void PutShieldEffectPos( OBJECT *shield ,FVECTOR *pos )
{
	int	i;
	int	p_num;
	int	s_num;
	int	model_num;
	FVECTOR	*pfvtemp;
	FMATRIX	mat;
	OBJECT	**shield_model ;

	if( OK_SHIELD_WORK==NULL ) return;

	for( i=0; i<N_BREAK_POINT; i++ ){
		if( OK_SHIELD_WORK->objs_to_shield[i]==-1 ) break;
	}
	if( i==N_BREAK_POINT ) return;
	p_num=i;	/* 空きがあった */

	s_num=0;
	shield_model=OK_SHIELD_WORK->shield;
	for( i=0; i<N_SHIELD; i++ ){
		/* 同じ盾を発見した*/
		if( *shield_model==shield){
			s_num=i;
			break;
		}
		shield_model++;
	}
	if( i==N_SHIELD ){
		shield_model=OK_SHIELD_WORK->shield;
		for( i=0; i<N_SHIELD; i++ ){
			/* 新規に盾を登録 */
			if( *shield_model==NULL ){
				s_num=i;
				break;
			}
			shield_model++;
		}
		if( i==N_SHIELD ) return;
	}
	/* p_num 番目のモデルは、s_num 番目の盾に引っ付く */

	FastInverseMatrix( &mat, &shield->objs->world );

	DG_SetPos( &mat );
	pfvtemp=&OK_SHIELD_WORK->mem_pos[p_num];
	DG_PutVector( pos, pfvtemp, 1 );
	model_num=CheckMakeInnerShield( pfvtemp, &OK_SHIELD_WORK->rot_y[p_num] );
	if( model_num==-1 ) return;
	/* 以上までで登録の可否が決定 */

	/* 登録作業 */
	*shield_model=shield ;	/* 一部、二度手間 */
	if( MakeDankonObjs( OK_SHIELD_WORK, p_num, s_num, model_num ) ){
		OK_SHIELD_WORK->objs_to_shield[p_num]=s_num;
		GM_SeSetMode( SD_E_SHIRIC01 , pos, GM_SEMODE_NORMAL );	/* 弾着音 */
	}

}

static	void Die( Work *work )
{
	int	i;

	for( i=0; i<N_BREAK_POINT; i++ ) DeleteDankonObjs( work, i );
	for( i=0; i<N_BLOOD_POINT; i++ ) DeleteDankonObjs_BL( work, i );
	OK_SHIELD_WORK = NULL;
}

static	int GetResources( Work *work)
{
	int	i;

	for( i=0; i<N_BREAK_POINT; i++ ){
		work->objs_to_shield[i]=-1;
	}
	for( i=0; i<N_BLOOD_POINT; i++ ){
		work->bl_objs_to_shield[i]=-1;
	}

	/* ポインタ初期化 */
	for( i=0; i<N_SHIELD; i++ ){
		work->shield[i]=NULL;
	}

	model_name[0]=DUNKON_MODEL1;
	model_name[1]=DUNKON_MODEL2;
	model_name[2]=DUNKON_MODEL3;
	model_name[3]=DUNKON_MODEL4;
	model_name[4]=DUNKON_MODEL5;
	model_name[5]=DUNKON_MODEL6;

	model_width[0]=DUNKON_WIDTH1;
	model_width[1]=DUNKON_WIDTH2;
	model_width[2]=DUNKON_WIDTH3;
	model_width[3]=DUNKON_WIDTH4;
	model_width[4]=DUNKON_WIDTH5;
	model_width[5]=DUNKON_WIDTH6;


	model_name_l[0]=DUNKON_MODEL_L1;
	model_name_l[1]=DUNKON_MODEL_L2;
	model_name_l[2]=DUNKON_MODEL_L3;
	model_name_r[0]=DUNKON_MODEL_R1;
	model_name_r[1]=DUNKON_MODEL_R2;
	model_name_r[2]=DUNKON_MODEL_R3;
	model_name_u[0]=DUNKON_MODEL_U1;
	model_name_u[1]=DUNKON_MODEL_U2;
	model_name_u[2]=DUNKON_MODEL_U3;
	model_name_d[0]=DUNKON_MODEL_D1;
	model_name_d[1]=DUNKON_MODEL_D2;
	model_name_d[2]=DUNKON_MODEL_D3;

	model_width_slice[0]=DUNKON_WIDTH_SLICE1;
	model_width_slice[1]=DUNKON_WIDTH_SLICE2;
	model_width_slice[2]=DUNKON_WIDTH_SLICE3;



	bl_model_name[0]=BLOOD_MODEL0;
	bl_model_name[1]=BLOOD_MODEL1;
	bl_model_name[2]=BLOOD_MODEL2;
	bl_model_name[3]=BLOOD_MODEL3;
	bl_model_name[4]=BLOOD_MODEL4;

	bl_model_width[0]=BLOOD_WIDTH1;
	bl_model_width[1]=BLOOD_WIDTH2;

	return 0 ;
}

void *NewShieldEffectControl( void )
{
	Work		*work ;

	/* ただ一つのみ起動できる */
	if( OK_SHIELD_WORK!=NULL ) return NULL ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	OK_SHIELD_WORK=work;
	return (void *)work ;
}
