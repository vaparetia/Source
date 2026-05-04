//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_m9.c
	Ｍ９麻酔弾が抜けると出る血
	2000/04/26 S.Okajima
	$Id: blood_m9.c,v 1.1.1.3 2002/11/19 11:46:58 Yoshizawa1 Exp $
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
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
extern int	OK_FloorOffFlag;

/*----------------------------------------------------------------*/
#define	SPEED_MAX		(10.0f)
#define	SPEED_MIN		(3.0f)
#define	ANGLE_MAX		(512)
#define	ANGLE_ADD		(64)

#define	BLOOD_M9_GRAVITY	(-0.5f)

#define	N_VERTS		(16)
#define	N_PRIMS		(4)

#define	N_VERTS_F	(64)
#define	N_POLYS_F	(N_VERTS_F/4)
#define	N_PRIMS_F	(4)


#ifdef ENGLISH
#define	MAX_ALPHA		(64)
#else
#define	MAX_ALPHA		(BLOOD_ALPHA_0)
#endif

#define	LIFE_TIME		( N_VERTS * N_PRIMS )
#define	LIFE_TIME_FLOOR	( LIFE_TIME + 600 )

#define	SIZE			( 20 )
#define	SIZE_FLOOR		( 30 )

#define	UPPER_SHIFT		( 2.0f )
#define	CHECK_SHIFT		( (float)SIZE_FLOOR )

#define	MAX_CHECK		( 4 )

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	FMATRIX		*world;
	FVECTOR		mov;
	FVECTOR		norm;

	DG_PRIM2	*prim ;
	DG_PRIM2	*prim_inter ;

	DG_PRIM2	*prim_f ;

	FVECTOR		vec[N_PRIMS*N_VERTS];

	float		speed;
	SVECTOR		rot;

	int			life;
	int			life_floor;
	int			white;

	int			floor_flag;
	float		flr_height[2];

} Work ;

/*----------------------------------------------------------------*/
/* floor についての rotを得る */
static	void	CalcLocalRot_Floor( HZX_FLR *floor, SVECTOR *rot )
{
	FVECTOR	to;

	/* 法線収得 */
	to.vx=floor->p1.h;
	to.vy=floor->p3.h;
	to.vz=floor->p2.h;

	OK_DirVecXY( &DG_ZeroVector, &to, rot );
}

/*----------------------------------------------------------------*/
/* floor に プリミティブを張る */
static	void	SetPrims( FVECTOR *prim_pos, FVECTOR *point_pos, SVECTOR *point_rot, float sp_size )
{
	SVECTOR	temp_rot;
	FVECTOR	size[4];

	temp_rot.vx=0;
	temp_rot.vy=0;
	temp_rot.vz=(short)(irnd() % 4096);

	size[0].vx = - sp_size;
	size[0].vy =   sp_size;
	size[0].vz =   0.0f;
	size[1].vx =   sp_size;
	size[1].vy =   sp_size;
	size[1].vz =   0.0f;
	size[2].vx = - sp_size;
	size[2].vy = - sp_size;
	size[2].vz =   0.0f;
	size[3].vx =   sp_size;
	size[3].vy = - sp_size;
	size[3].vz =   0.0f;

	DG_SetPos2( &DG_ZeroVector, &temp_rot );
	DG_PutVector( size, prim_pos, 4 );
	DG_SetPos2( point_pos, point_rot );
	DG_PutVector( prim_pos, prim_pos, 4 );
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j;
	float		flr_height[2];
	SVECTOR		rot;
	SVECTOR		floor_rot[MAX_CHECK];
	FVECTOR		floor_pos[MAX_CHECK];
	FVECTOR		fvtemp;
	FVECTOR		mov;
	FVECTOR		norm;
	FVECTOR		keep_pos;
	FVECTOR		*vec;
	FVECTOR		*prim_pos_inter;
	FVECTOR		*prim_pos;
	FVECTOR		*prim_pos_before;
	DG_PRIM2_UVRGBWH	*uvrgbwh_inter ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	DG_PRIM2_UVRGB		*uvrgb ;
	HZX_FLR		flr[2];
	int			flr_atrs[2];
	int			clock;
	int			flag;
	int			alpha;
	int			map_id;
	int			floor_flag;
	int			put_f_flag;

	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		if( OK_FloorOffFlag ){
			GV_DestroyActor( work ) ;
			return;
		}
	}

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );

	GM_GroupPrim2( work->prim,       GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim_inter, GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim_f,     GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	DG_SwitchBuffPrim2( work->prim_inter );
	DG_SwitchBuffPrim2( work->prim_f );
	clock = work->prim->buffer_clock;


	DG_SetPos( work->world );
	DG_PutVector( &work->mov,  &mov,  1 );
	DG_RotVector( &work->norm, &norm, 1 );
	_sceVu0Normalize( &norm, &norm );


	//---------------------------
	uvrgbwh_inter   = work->prim_inter->uvrgb[clock];
	uvrgbwh         = work->prim->uvrgb[clock];
	prim_pos_inter  = work->prim_inter->pos[clock];
	prim_pos        = work->prim->pos[  clock];
	prim_pos_before = work->prim->pos[1-clock];
	DG_COPY_VEC( &keep_pos, prim_pos );
	vec  = work->vec;
	flag = 0;
	put_f_flag = 0;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			if( vec->vw==0.0f ){
				if( flag==0 && irnd()%8!=0){
					flag = 1;
					vec->vx = norm.vx * work->speed;
					vec->vy = norm.vy * work->speed;
					vec->vz = norm.vz * work->speed;
					rot.vx = work->rot.vx + irnd()%ANGLE_ADD;
					rot.vy = work->rot.vy + irnd()%ANGLE_ADD;
					rot.vz = work->rot.vz + irnd()%ANGLE_ADD;
					DG_SetPos2( &DG_ZeroVector, &rot );
					DG_RotVector( vec, vec, 1 );
					DG_COPY_VEC( prim_pos, &mov );
					vec->vw = 1.0f;
				}
			}else if( vec->vw==1.0f ){
				prim_pos->vx = prim_pos_before->vx + vec->vx;
				prim_pos->vy = prim_pos_before->vy + vec->vy;
				prim_pos->vz = prim_pos_before->vz + vec->vz;

				prim_pos_inter->vx = (prim_pos->vx + keep_pos.vx)*0.5f;
				prim_pos_inter->vy = (prim_pos->vy + keep_pos.vy)*0.5f;
				prim_pos_inter->vz = (prim_pos->vz + keep_pos.vz)*0.5f;

				DG_COPY_VEC( &keep_pos, prim_pos );
				vec->vy += BLOOD_M9_GRAVITY;

				uvrgbwh_inter->a = MAX_ALPHA;
				uvrgbwh->a       = MAX_ALPHA;

				if( put_f_flag < MAX_CHECK   &&   prim_pos->vy < *(work->flr_height) ){
//printf("%d %d ",i,j);
					if( prim_pos->vx > mov.vx ){
						fvtemp.vx = prim_pos->vx - CHECK_SHIFT;
					}else{
						fvtemp.vx = prim_pos->vx + CHECK_SHIFT;
					}
					if( prim_pos->vz > mov.vz ){
						fvtemp.vz = prim_pos->vz - CHECK_SHIFT;
					}else{
						fvtemp.vz = prim_pos->vz + CHECK_SHIFT;
					}
					fvtemp.vy = prim_pos->vy;

					floor_flag = HZX_LevelHazardCheck( map_id, &fvtemp, HZX_CHK_F_FLOOR, HZX_FLOOR_NO_PLAYER );
//printf("floor_flag:%d\n",floor_flag);
					if( floor_flag & 1 ){
						HZX_GetLevelHazard( flr, flr_atrs );
						if( !( flr_atrs[ 0 ] & HZX_FLOOR_NO_BLOOD ) ){
							CalcLocalRot_Floor( flr, &floor_rot[put_f_flag] );
							HZX_GetLevelHeight( flr_height );
							(*flr_height) += UPPER_SHIFT;

							floor_pos[put_f_flag].vx = fvtemp.vx;
							floor_pos[put_f_flag].vy = (*flr_height);
							floor_pos[put_f_flag].vz = fvtemp.vz;
							put_f_flag++;
						}
					}

					vec->vw = 2.0f;
				}
			}else{
				uvrgbwh_inter->a = 0;
				uvrgbwh->a = 0;
			}

			uvrgbwh_inter++;
			uvrgbwh++;
			prim_pos_inter ++;
			prim_pos ++;
			prim_pos_before ++;
			vec ++;
		}
	}

	if( irnd()%4==0 || work->speed<SPEED_MIN ){
		work->rot.vx = irnd()%ANGLE_MAX - ANGLE_MAX/2;
		work->rot.vy = irnd()%ANGLE_MAX - ANGLE_MAX/2;
		work->rot.vz = irnd()%ANGLE_MAX - ANGLE_MAX/2;
		work->speed  = SPEED_MAX*(rnd()+1.0f)*0.5f;
	}else{
		work->rot.vx += irnd()%ANGLE_ADD - ANGLE_ADD/2;
		work->rot.vy += irnd()%ANGLE_ADD - ANGLE_ADD/2;
		work->rot.vz += irnd()%ANGLE_ADD - ANGLE_ADD/2;
		work->speed  *= 0.97f;
	}



	//--------------------------- 床
	if( work->life_floor < MAX_ALPHA ){
		alpha = work->life_floor ;
	}else{
		alpha = MAX_ALPHA;
	}
	if( alpha<0 ) alpha=0;

	uvrgb            = work->prim_f->uvrgb[clock];
	prim_pos         = work->prim_f->pos[clock];
	prim_pos_before  = work->prim_f->pos[1-clock];
	DG_COPY_VEC( &keep_pos, prim_pos );
	for( i=0; i<N_PRIMS_F; i++ ){
		for( j=0; j<N_POLYS_F; j++ ){
			if( prim_pos_before->vw==0.0f ){
				if( put_f_flag > 0){
					put_f_flag--;
					SetPrims( prim_pos, &floor_pos[put_f_flag], &floor_rot[put_f_flag], SIZE_FLOOR * (rnd() + 1.0f) * 0.5f );
					prim_pos->vw = 1.0f;
				}
				uvrgb+=4;
				prim_pos += 4;
				prim_pos_before +=4;
			}else if( prim_pos_before->vw==1.0f ){
				prim_pos->vw        = 2.0f;
				prim_pos_before->vw = 2.0f;
				DG_COPY_VEC( prim_pos, prim_pos_before );	prim_pos++;	prim_pos_before++;
				DG_COPY_VEC( prim_pos, prim_pos_before );	prim_pos++;	prim_pos_before++;
				DG_COPY_VEC( prim_pos, prim_pos_before );	prim_pos++;	prim_pos_before++;
				DG_COPY_VEC( prim_pos, prim_pos_before );	prim_pos++;	prim_pos_before++;
				uvrgb+=4;
			}else if( prim_pos_before->vw==2.0f ){
				uvrgb->a = alpha;	uvrgb++;
				uvrgb->a = alpha;	uvrgb++;
				uvrgb->a = alpha;	uvrgb++;
				uvrgb->a = alpha;	uvrgb++;
				prim_pos += 4;
				prim_pos_before +=4;
			}else{
				uvrgb+=4;
				prim_pos += 4;
				prim_pos_before +=4;
			}
		}
	}

#if 0
	if( --work->life < 0){
		DG_InvisiblePrim2( work->prim ) ;
		DG_InvisiblePrim2( work->prim_inter ) ;
	}
#else
	work->life--;
#endif
	if( --work->life_floor < 0) GV_DestroyActor( work ) ;


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	work->prim_inter = OK_FreePrim2( work->prim_inter );
	work->prim_f = OK_FreePrim2( work->prim_f );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i,k ;
	float	angle;
	float	size_rnd;

	DG_ConfigPrim2Tex( prim, tex );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for( i=0; i<N_PRIMS; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

			angle = rnd();
			size_rnd = SIZE * (rnd() + 1.0f) * 0.5f;
			uvrgbwh1->w = uvrgbwh0->w = (int)(cosf( angle*TPI ) * size_rnd);
			uvrgbwh1->h = uvrgbwh0->h = (int)(sinf( angle*TPI ) * size_rnd);

			uvrgbwh1->r = uvrgbwh0->r = COLOR_R ;
			uvrgbwh1->g = uvrgbwh0->g = COLOR_G ;
			uvrgbwh1->b = uvrgbwh0->b = COLOR_B ;
			uvrgbwh1->a = uvrgbwh0->a = 0 ;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

}

/* ---------------------------------------------------------------- */
static void InitPacketF( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i,k ;

	DG_ConfigPrim2Tex( prim, tex );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	//-------------------------------
	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i = 0 ; i < N_PRIMS_F ; i++ ){
		for ( k = 0 ; k < N_POLYS_F ; k++ ){
			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;
		}
	}

}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	int	i,j;
	int			floor_flag;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*vec;
	FVECTOR		center;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	tex = DG_GetTexture( 9373287 /*"vr2_chi01_alp"*/ );
	}
	else
	{
	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	}


	/* 落ちる血 */
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/06/25 */
	{
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	}
	else
	{
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	}
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_VisiblePrim2( prim ) ;

	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/06/25 */
	{
	prim = work->prim_inter = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	}
	else
	{
	prim = work->prim_inter = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	}
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_VisiblePrim2( prim ) ;

	/* 床血 */
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/06/25 */
	{
	prim = work->prim_f = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_F, N_VERTS_F );
	}
	else
	{
	prim = work->prim_f = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS_F, N_VERTS_F );
	}
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacketF( work, prim, tex );
	DG_VisiblePrim2( prim ) ;


	DG_COPY_VEC( &center, (FVECTOR *)work->world->m[3] );
	pos0 = work->prim->pos[0];
	pos1 = work->prim->pos[1];
	vec  = work->vec;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			DG_COPY_VEC( pos0, &center );
			DG_COPY_VEC( pos1, &center );
			vec->vw = 0.0f;	/* フラグ */
			pos0++;
			pos1++;
			vec++;
		}
	}
	pos0 = work->prim_inter->pos[0];
	pos1 = work->prim_inter->pos[1];
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			DG_COPY_VEC( pos0, &center );
			DG_COPY_VEC( pos1, &center );
			pos0++;
			pos1++;
		}
	}
	pos0 = work->prim_f->pos[0];
	pos1 = work->prim_f->pos[1];
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS_F; j++ ){
			DG_COPY_VEC( pos0, &center );
			DG_COPY_VEC( pos1, &center );
			pos0->vw = 0.0f;
			pos1->vw = 0.0f;
			pos0++;
			pos1++;
		}
	}

	work->life = LIFE_TIME;
	work->life_floor = LIFE_TIME_FLOOR;
	work->rot.vx = irnd()%ANGLE_MAX;
	work->rot.vy = irnd()%ANGLE_MAX;
	work->rot.vz = irnd()%ANGLE_MAX;
	work->speed  = SPEED_MAX*rnd();

	floor_flag = HZX_LevelHazardCheck( GM_GetHzxGroupID( GM_CurrentStageMap ), &center, HZX_CHK_F_FLOOR, HZX_FLOOR_NO_PLAYER );
	if( floor_flag & 1 ){
		HZX_GetLevelHeight( work->flr_height );
		work->flr_height[0] += CHECK_SHIFT;
	}else{
		work->flr_height[0]  = center.vy + CHECK_SHIFT;
	}

	return 0 ;
}

/*
[world]:[常に参照する]発生位置の座標系
[mov  ]:world からの相対発生位置
[norm ]:発生法線（後に角度のみ抽出）
*/
void *NewBlood_M9( FMATRIX *world, FVECTOR *mov, FVECTOR *norm )
{
	Work		*work ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->world = world;
		DG_COPY_VEC( &work->mov, mov );
		DG_COPY_VEC( &work->norm, norm );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


/*
[world]     :[常に参照する]発生位置の座標系
[mov  ]     :world からの相対発生位置
[mdl_world ]:発生法線（後に角度のみ抽出）
*/
void *NewBlood_M9_Model( FMATRIX *world, FVECTOR *mov, FMATRIX *mdl_world )
{
	Work		*work ;
	FVECTOR		fvtemp;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->world = world;
		DG_COPY_VEC( &work->mov, mov );

		fvtemp.vx = 0.0f;
		fvtemp.vy = 1.0f;
		fvtemp.vz = 0.0f;
		DG_SetPos( mdl_world );
		DG_RotVector( &fvtemp, &work->norm, 1 );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

