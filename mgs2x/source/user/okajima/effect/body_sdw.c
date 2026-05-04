//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	body_sdw.c
	体から出る影
	1999/10/13 S.Okajima
	$Id: body_sdw.c,v 1.1.1.3 2002/11/19 11:47:00 Yoshizawa1 Exp $
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
#include	"lit_man.h"
#include	"../etc/ok_util.h"

extern int	ok_body_shadow_off;

/*----------------------------------------------------------------*/
#define	MAX_SHADOW_NUM	(4)

#define	N_JOINTS		(8)
#define	N_POLYS			(N_JOINTS)
/* ６４を超えてはならない */
#define	N_VERTS			(N_POLYS * 4)
#define	N_PRIMS			(1)


#define	MAX_JOINTS		(21)

//#define	LIGHT_POINT_BACK	(1000.0f)
#define	LIGHT_POINT_BACK	(300.0f)

//#define	BASE_RGB	(96)
#define	BASE_RGB	(128)

#define	MAX_ALPHA		(16)

#define	EFFECT_LEN_MAX	(4000.0f)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_VEC			(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR		*obj_pos[ MAX_JOINTS ] ;

	int			clock;

	int			sw[MAX_LIGHT_NUM];
	float		len[MAX_LIGHT_NUM];
	DG_PRIM2	*prim[MAX_SHADOW_NUM];
} Work ;

#if 0
static int vert_data[ N_POLYS * 2 ]={
	 0,  1,
	 1,  2,
	 2,  3,
	 3,  4,
	 4,  5,

	 5,  6,
	 2, 11,
	11, 12,
	 2,  7,
	 7,  8,

	 8,  9,
	 9, 10,
	 0, 13,
	13, 14,
	14, 15,

	15, 16,
	 0, 17,
	17, 18,
	18, 19,
	19, 20
};
static int vert_data[ N_POLYS * 2 ]={
	 4,  5,
	 5,  6,
	 8,  9,
	 9, 10,

	 4,  8,
	 8, 17,
	17, 13,
	13,  4,

	13, 14,
	14, 16,
	17, 18,
	18, 20,
};
#else
static int vert_data[ N_POLYS * 2 ]={
	 4,  5,
	 5,  6,
	 8,  9,
	 9, 10,

	 4,  8,
	 8, 17,
	17, 13,
	13,  4,
};
#endif

/*----------------------------------------------------------------*/
static	void	MakeVertics( Work *work, u_char alpha, int shadow_num, int lit_num, float ratio )
{
	DG_PRIM2	*prim;
	FVECTOR	*pos;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR	outer[ MAX_JOINTS ];
	FVECTOR	*out;
	FVECTOR	**obj_pos;
	FVECTOR	lit;
	int	*vert1;
	int	*vert2;
	int		i ;
	static	FVECTOR	backward={ 0.0f, 0.0f, -LIGHT_POINT_BACK, 1.0f };

	/* 光源を後方にセットする */
	DG_SetPos2( ok_lit_pos_sorted[lit_num], ok_lit_rot_sorted[lit_num] );
	DG_PutVector( &backward, &lit, 1 );

	out     = outer;
	obj_pos = work->obj_pos;
	for(i=0; i<MAX_JOINTS; i++){
		out->vx = (*obj_pos)->vx + ( (*obj_pos)->vx - lit.vx ) * ratio;
		out->vy = (*obj_pos)->vy + ( (*obj_pos)->vy - lit.vy ) * ratio;
		out->vz = (*obj_pos)->vz + ( (*obj_pos)->vz - lit.vz ) * ratio;
		out++;
		obj_pos++;
	}

	prim  = work->prim[shadow_num];
	pos   = prim->pos[work->clock];
	uvrgb = prim->uvrgb[work->clock];
	vert1 = vert_data;
	vert2 = vert_data;
	vert2++;
	for(i=0; i<N_POLYS; i++){
		uvrgb->a = alpha;	uvrgb+=2;
		uvrgb->a = alpha;	uvrgb+=2;

		DG_COPY_VEC( pos++, work->obj_pos[ *vert1 ] );
		DG_COPY_VEC( pos++, &outer[ *vert1 ] );
		DG_COPY_VEC( pos++, work->obj_pos[ *vert2 ] );
		DG_COPY_VEC( pos++, &outer[ *vert2 ] );
		vert1+=2;
		vert2+=2;
	}
}

/*----------------------------------------------------------------*/
static	void	CalcLen( Work *work )
{
	static FVECTOR width={ EFFECT_LEN_MAX, EFFECT_LEN_MAX, EFFECT_LEN_MAX, 0.0f };
	int	i;
	int	*sw;
	FVECTOR	pos;
	float	*len;
	FVECTOR	bound1;
	FVECTOR	bound2;

	_sceVu0SubVector( &bound1, *work->obj_pos, &width );
	_sceVu0AddVector( &bound2, *work->obj_pos, &width );

	len      = work->len;
	sw       = work->sw;
	for( i=0; i<ok_lit_data_num && i<MAX_LIGHT_NUM; i++ ){
		if( ((*sw) = vu0_CheckBoundingBox( ok_lit_pos_sorted[i], &bound1, &bound2 ))==1 ){
			_sceVu0SubVector( &pos, (*work->obj_pos), ok_lit_pos_sorted[i] );
			(*len) = GV_VecLen3F( &pos );
		}
		len++;
		sw++;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	static	FVECTOR scale={ 0.0f, 0.0f, -1.0f, 0.0f };
	int	light_num;
	int	i,j;
	int	roop_count;
	int	itemp1;
	int	*sw;
	float	ratio;
	float	min;
	float	*len;
	FVECTOR	fvtemp;
	SVECTOR	rot;
	FMATRIX	mat;

	if( GM_CheckPlayerStatus(PLAYER_WATCH|PLAYER_INTRUDE)
	 || ok_body_shadow_off==1 ){
		for( i=0; i<MAX_SHADOW_NUM; i++ ){
			DG_InvisiblePrim2( work->prim[i] ) ;
		}
		return;
	}

	for( i=0; i<MAX_SHADOW_NUM; i++ ){
		GM_GroupPrim2( work->prim[i], GM_CurrentStageMap ) ;
		DG_SwitchBuffPrim2( work->prim[i] );
	}
	work->clock = work->prim[0]->buffer_clock;


	/* 個々の距離を求める（work->len[] に 入る） */
	CalcLen( work );

	light_num = ( ok_lit_data_num < MAX_SHADOW_NUM )?ok_lit_data_num:MAX_SHADOW_NUM;

//printf("ok_lit_data_num:%d\n",ok_lit_data_num);

	/* 最も近い光源から得たデータを使用しセットする */
	roop_count=0;
	for( i=0; i<light_num; i++ ){
		min = EFFECT_LEN_MAX;
		len = work->len;
		itemp1   = -1;
		sw       = work->sw;
		for( j=0; j<ok_lit_data_num; j++ ){
			if( *sw ){
				if( *len < min ){
					min    = *len;
					itemp1 = j;
				}
			}
			len++;
			sw++;
		}
		if(itemp1 != -1){
//ASSERT( ok_lit_rot_sorted[itemp1]==NULL );
			DG_SetPos2( &DG_ZeroVector, ok_lit_rot_sorted[itemp1] );
			DG_RotVector( &scale, &fvtemp, 1 );

//ASSERT( ok_lit_pos_sorted[itemp1]==NULL );
//printf("%d::%f %f %f\n",itemp1,ok_lit_pos_sorted[itemp1]->vx,ok_lit_pos_sorted[itemp1]->vy,ok_lit_pos_sorted[itemp1]->vz);
			OK_DirVecXY( *work->obj_pos, ok_lit_pos_sorted[itemp1], &rot );
			DG_SetPos2( &DG_ZeroVector, &rot );
			DG_GetPos( &mat );

//			_sceVu0InversMatrix( &mat, &mat );
			FastInverseMatrix( &mat, &mat );

			DG_SetPos( &mat );
			DG_RotVector( &fvtemp, &fvtemp, 1 );

			if( fvtemp.vz <= 0.0f ) continue;
			DG_VisiblePrim2( work->prim[i] ) ;

			ratio = (EFFECT_LEN_MAX - min) / EFFECT_LEN_MAX;
			MakeVertics( work,
				(u_char)( (float)MAX_ALPHA * fvtemp.vz * ratio ),
				i,
				itemp1,
				ratio );

			work->len[itemp1] = EFFECT_LEN_MAX;
			roop_count++;
		}
	}

	for( i=roop_count; i<MAX_SHADOW_NUM; i++ ){
		DG_InvisiblePrim2( work->prim[i] ) ;
	}

}


static void Die( Work *work )
{
	int	i;

	for( i=0; i<MAX_SHADOW_NUM; i++ ){
		work->prim[i] = OK_FreePrim2( work->prim[i] );
	}
}

/*----------------------------------------------------------------*/
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );

	pos   = SCR_POS ;
	uvrgb = SCR_UVS ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = BASE_RGB ;
			uvrgb->g = BASE_RGB ;
			uvrgb->b = BASE_RGB ;
			uvrgb->a = 0 ;
			uvrgb++;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, DG_OBJS *objs )
{
	DG_PRIM2	*prim;
	int	i;
	DG_TEX		*tex ;

	tex = DG_GetTexture( 8617636 /*"col128_alp"*/ );

	for( i=0; i<MAX_SHADOW_NUM; i++ ){
		prim = work->prim[i] = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacket( work, prim, tex );
	}

	for(i=0; i<MAX_JOINTS; i++){
		work->obj_pos[i] = (FVECTOR*)(objs->objs[i].world.m[3]);
	}

	return 0 ;
}


void *NewBodyShadow( DG_OBJS *objs )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
//printf("NewBodyShadow:%d\n",sizeof( Work ));
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, objs ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

