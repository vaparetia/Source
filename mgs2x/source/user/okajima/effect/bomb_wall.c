//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bomb_wall.c
	壁に爆破の焦げ跡
	2000/03/11 S.Okajima
	$Id: bomb_wall.c,v 1.1.1.3 2002/11/19 11:47:02 Yoshizawa1 Exp $

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
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
#define	MAX_TEX			(4)
#define	N_VERTS			(4)
#define	N_PRIMS			(1)

#define	SEARCH_LENGTH	(3000.0f)

#define	COLOR_R			(128)
#define	COLOR_G			(128)
#define	COLOR_B			(128)
#define	COLOR_A			(16)

#define	SIZE_MIN	(200.0f)
#define	SIZE_RND	(800.0f)

#define	SHIFT_CENTER	(5.0f)
#define	RAISE			(-4000)

#define	BOUNDARY_CHECK	(SHIFT_CENTER   + 20.0f)
#define	BOUNDARY_OFF	(SIZE_MIN + SIZE_RND)

//#define	X_ANGLE_NUM		(3)
//#define	Y_ANGLE_NUM		(8)
#define	X_ANGLE_NUM		(2)
#define	Y_ANGLE_NUM		(4)

#define	WALL_NUM		(64)

void *NewBombWall( void );

typedef	struct	{
	int		flag;
	HZX_FLR	seg;
	FVECTOR	point_pos ;
	float	scale ;
} Unit ;

typedef	struct	{
	float		u_scale;
	float		u_offset;
	float		v_scale;
	float		v_offset;
} TexDat ;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	int			wall_num;
	int			swap_flag[WALL_NUM];	/* 初期化を続けるカウント */
	DG_PRIM2	*prim[WALL_NUM] ;
	TexDat		tex_dat[WALL_NUM];

	Unit		unit[ X_ANGLE_NUM * Y_ANGLE_NUM ];	/* 計算用に確保しておく */
} Work ;

Work	*OK_SUSU_WALL_WORK = NULL;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2	**prim ;
	int		*ip;
	int		i;

	ip = work->swap_flag;
	prim = work->prim;
	for( i=0; i<WALL_NUM; i++ ){
		GM_GroupPrim2( (*prim), GM_CurrentStageMap ) ;
		if( (*ip) != 0 ){
//printf("act:%d:%d:init1\n",i);
			DG_SwitchBuffPrim2( (*prim) );
			(*ip)  = 0;
		}
		prim++;
		ip++;
	}

}

static void Die( Work *work )
{
	int		i;

	for( i=0; i<WALL_NUM; i++ ){
	 work->prim[i] = OK_FreePrim2 ( work->prim[i] );
	}
	OK_SUSU_WALL_WORK=NULL;
}

/* seg についての rotを得る */
static	void	CalcLocalRot_Seg( HZX_SEG *seg, SVECTOR *rot )
{
	float x,z ;
	float tmp ;

	rot->vx=0;
	rot->vz=0;

	x=seg->p2.x - seg->p1.x;
	z=seg->p2.z - seg->p1.z;

	tmp = atan2f( x, z ) ;
	rot->vy = 4095 & ( short )( ( tmp * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */

//printf( "rot_seg %d %d %d\n",(int)rot->vx,(int)rot->vy,(int)rot->vz) ;

}

/////////////////////////////////////////////////////
#if 0

/* floor についての rotを得る */
static	void	CalcLocalRot_Floor( HZX_FLR *floor, SVECTOR *rot )
{
	FVECTOR	to;

	/* 法線収得 */
	to.vx=floor->p1.h;
	to.vy=floor->p3.h;
	to.vz=floor->p2.h;

	OK_DirVecXY( &DG_ZeroVector, &to, rot );

//printf( "rot_floor %d %d %d\n",(int)rot->vx,(int)rot->vy,(int)rot->vz) ;

//printf( "          %d %d %d\n",(int)to.vx,(int)to.vy,(int)to.vz) ;

}


/* 入力フロアはそのままを利用 */
/* 0:入力点がフロア平面にない */
/* 1:入力点がフロア平面にある */
static	int	CheckPos4WithinFloor( FVECTOR *pos, FVECTOR *minmax )
{
	FVECTOR	*check_pos;
	FVECTOR	*flr_max;
	FVECTOR	*flr_min;
	int	i;

	flr_max=minmax;
	flr_min=minmax;
	flr_min++;

	/* Ｘ・Ｚ・Ｙ絶対座標同士の比較 */
	check_pos=pos;
	for( i=4; i>0; i-- ){
		if( check_pos->vx > flr_max->vx ) return 0;
		if( check_pos->vy > flr_max->vy ) return 0;
		if( check_pos->vz > flr_max->vz ) return 0;
		if( check_pos->vx < flr_min->vx ) return 0;
		if( check_pos->vy < flr_min->vy ) return 0;
		if( check_pos->vz < flr_min->vz ) return 0;
		check_pos++;
	}

	return 1;

}

/* 入力セグメントは加工されているものを使用 */
/* 0:入力点がセグメント平面にない */
/* 1:入力点がセグメント平面にある */
static	int	CheckPos4WithinSeg( FVECTOR *pos, HZX_SEG *seg1, HZX_SEG *seg2 )
{
	FVECTOR	*check_pos;
	float	top;
	float	bottom;
	float	ftemp1;
	float	ftemp2;
	float	ftemp3;
	float	ftemp4;
	float	ftemp5;
	float	ftemp6;
	int	i;

	/* Ｘ・Ｚ座標同士の比較 */
	ftemp1 = seg1->p2.x - seg1->p1.x;
	ftemp2 = seg1->p2.z - seg1->p1.z;
	ftemp3 = seg1->p2.y - seg1->p1.y;
	ftemp4 = ftemp3 + seg1->p2.h - seg1->p1.h;

	check_pos=pos;
	if( ftemp1 > ftemp2 ){
		for( i=4; i>0; i-- ){
			if( check_pos->vx < seg2->p1.x ) return 0;
			if( check_pos->vx > seg2->p2.x ) return 0;
			check_pos++;
		}
	}else{
		for( i=4; i>0; i-- ){
			if( check_pos->vz < seg2->p1.z ) return 0;
			if( check_pos->vz > seg2->p2.z ) return 0;
			check_pos++;
		}
	}

	check_pos=pos;
	if( ftemp1 > ftemp2 ){
		for( i=4; i>0; i-- ){
			ftemp5 = check_pos->vx - seg1->p1.x;
			top    = ftemp5 * ftemp4 / ftemp1 + seg1->p1.y + seg1->p1.h ;
			bottom = ftemp5 * ftemp3 / ftemp1 + seg1->p1.y;
			if( check_pos->vy > top )    return 0;
			if( check_pos->vy < bottom ) return 0;
			check_pos++;
		}
	}else{
		for( i=4; i>0; i-- ){
			ftemp6 = check_pos->vz - seg1->p1.z;
			top    = ftemp6 * ftemp4 / ftemp2 + seg1->p1.y + seg1->p1.h ;
			bottom = ftemp6 * ftemp3 / ftemp2 + seg1->p1.y;
			if( check_pos->vy > top )    return 0;
			if( check_pos->vy < bottom ) return 0;
			check_pos++;
		}
	}

	return 1;

}


static	void	CalcFloorSub1( FVECTOR *fv1, FVECTOR *fv2 )
{
	float	ftemp;
	if( fv1->vx < fv2->vx ){
		ftemp=fv1->vx;
		fv1->vx=fv2->vx;
		fv2->vx=ftemp;
	}
	if( fv1->vy < fv2->vy ){
		ftemp=fv1->vy;
		fv1->vy=fv2->vy;
		fv2->vy=ftemp;
	}
	if( fv1->vz < fv2->vz ){
		ftemp=fv1->vz;
		fv1->vz=fv2->vz;
		fv2->vz=ftemp;
	}
}

static	int	MakeCalcFloor( FVECTOR *out, HZX_FLR *in )
{
	FVECTOR	fvflr[4];
	float	dif_x;
	float	dif_z;
	float	min;
	int	i;

	fvflr[0].vx=in->p1.x;
	fvflr[0].vy=in->p1.y;
	fvflr[0].vz=in->p1.z;

	fvflr[1].vx=in->p2.x;
	fvflr[1].vy=in->p2.y;
	fvflr[1].vz=in->p2.z;

	fvflr[2].vx=in->p3.x;
	fvflr[2].vy=in->p3.y;
	fvflr[2].vz=in->p3.z;

	fvflr[3].vx=in->p4.x;
	fvflr[3].vy=in->p4.y;
	fvflr[3].vz=in->p4.z;

	for( i=0; i<3; i++ ) CalcFloorSub1( &fvflr[i], &fvflr[i+1] );
	for( i=0; i<2; i++ ) CalcFloorSub1( &fvflr[i], &fvflr[i+1] );
	for( i=0; i<1; i++ ) CalcFloorSub1( &fvflr[i], &fvflr[i+1] );

	dif_x = fvflr[1].vx - fvflr[2].vx;
	dif_z = fvflr[1].vz - fvflr[2].vz;

	min=(dif_x<dif_z)?dif_x:dif_z;

	if( min < BOUNDARY_OFF ) return 0;

	out[0].vx=fvflr[1].vx + BOUNDARY_CHECK;
	out[0].vy=fvflr[1].vy + BOUNDARY_CHECK;
	out[0].vz=fvflr[1].vz + BOUNDARY_CHECK;

	out[1].vx=fvflr[2].vx - BOUNDARY_CHECK;
	out[1].vy=fvflr[2].vy - BOUNDARY_CHECK;
	out[1].vz=fvflr[2].vz - BOUNDARY_CHECK;

	return 1;

}

static	int	MakeCalcSeg( HZX_SEG *out, HZX_SEG *in )
{
	float	dif_x;
	float	dif_z;
	float	max;

	dif_x=in->p1.x - in->p2.x;
	dif_z=in->p1.z - in->p2.z;
	if( dif_x < 0 ){
		out->p1.x=in->p1.x - BOUNDARY_CHECK;
		out->p2.x=in->p2.x + BOUNDARY_CHECK;
		dif_x*=-1;
	}else{
		out->p1.x=in->p2.x - BOUNDARY_CHECK;
		out->p2.x=in->p1.x + BOUNDARY_CHECK;
	}
	if( dif_z < 0 ){
		out->p1.z=in->p1.z - BOUNDARY_CHECK;
		out->p2.z=in->p2.z + BOUNDARY_CHECK;
		dif_z*=-1;
	}else{
		out->p1.z=in->p2.z - BOUNDARY_CHECK;
		out->p2.z=in->p1.z + BOUNDARY_CHECK;
	}

	max=(dif_x > dif_z)?dif_x:dif_z;

	if( max < BOUNDARY_OFF ) return 0;

	return 1;
}


/* floor に プリミティブを張る */
static	void	SetPrims_Floor( Work *work, Unit *unit, SVECTOR *point_rot )
{
	int		i;
	int		check;
	FVECTOR	*pos;
	SVECTOR	temp_rot2;
	DG_PRIM2_UVRGB		*uvrgb ;
	float	size;

//	if( !MakeCalcFloor( flr_minmax, &unit->seg ) ) return 0;	/* 作れなかった */

	size = SIZE_MIN + SIZE_RND*rnd();
	size/= 2.0f;

	check  = work->wall_num;
	work->swap_flag[check]=1;

	i      = work->prim[check]->buffer_clock;
	pos    = work->prim[check]->pos[ 1 - i ] ;
	uvrgb  = work->prim[check]->uvrgb[ 1 - i ] ;
	uvrgb->a = COLOR_A ;	uvrgb++;
	uvrgb->a = COLOR_A ;	uvrgb++;
	uvrgb->a = COLOR_A ;	uvrgb++;
	uvrgb->a = COLOR_A ;	uvrgb++;


	pos[0].vx = -size;
	pos[0].vy =  size;
	pos[0].vz =  0;

	pos[1].vx =  size;
	pos[1].vy =  size;
	pos[1].vz =  0;

	pos[2].vx = -size;
	pos[2].vy = -size;
	pos[2].vz =  0;

	pos[3].vx =  size;
	pos[3].vy = -size;
	pos[3].vz =  0;

	temp_rot2.vx=0;
	temp_rot2.vy=0;
	temp_rot2.vz = (short)(irnd() % 4096);

	DG_SetPos2( &DG_ZeroVector, &temp_rot2 );
	DG_PutVector( pos, pos, 4 );

	DG_SetPos2( &unit->point_pos, point_rot );
	DG_PutVector( pos, pos, 4 );


	work->wall_num++;
	if( work->wall_num >= WALL_NUM ) work->wall_num = 0;
}

#endif
///////////////////////////////////////////////


/* seg に プリミティブを張る */
static	void	SetPrims_Seg( Work *work, Unit *unit, SVECTOR *point_rot )
{
	FMATRIX	mat;
	FVECTOR	fvtemp[2];
	float	tex_u0 = 1.0f;
	float	tex_u1 = 1.0f;
	float	tex_v0 = 1.0f;
	float	tex_v1 = 1.0f;
	float	tex_v2 = 1.0f;
	float	tex_v3 = 1.0f;
	float	u_scale;
	float	u_offset;
	float	v_scale;
	float	v_offset;
	float	a_under;
	float	a_upper;
	float	ftemp0;
	float	size;
	HZX_SEG	*seg;
	int		check;
	int		i;
	FVECTOR	*pos;
	DG_PRIM2_UVRGB		*uvrgb ;

	size = SIZE_MIN + SIZE_RND*rnd();

	check  = work->wall_num;
	work->swap_flag[ check ] = 1;
	i      = work->prim[check]->buffer_clock;
	pos    = work->prim[check]->pos[ 1 - i ] ;
	uvrgb  = work->prim[check]->uvrgb[ 1 - i ] ;

	seg = (HZX_SEG *)&unit->seg;
	fvtemp[0].vx = seg->p1.x;
	fvtemp[0].vy = seg->p1.y;
	fvtemp[0].vz = seg->p1.z;
	fvtemp[1].vx = seg->p2.x;
	fvtemp[1].vy = seg->p2.y;
	fvtemp[1].vz = seg->p2.z;

	/* 発生予定ポリゴンの中心を原点にした、壁の逆変換 */
	DG_SetPos2( &unit->point_pos, point_rot );
	DG_GetPos( &mat );

//	_sceVu0InversMatrix( &mat, &mat );
	FastInverseMatrix( &mat, &mat );

	DG_SetPos( &mat );
	DG_PutVector( fvtemp, fvtemp, 2 );

//printf("inv_seg:%f %f %f\n",fvtemp[0].vx,fvtemp[0].vy,fvtemp[0].vz);
//printf("inv_seg:%f %f %f\n",fvtemp[1].vx,fvtemp[1].vy,fvtemp[1].vz);

	fvtemp[0].vw = fvtemp[0].vy + seg->p1.h;
	fvtemp[1].vw = fvtemp[1].vy + seg->p2.h;
	a_under = ( fvtemp[1].vy - fvtemp[0].vy ) / ( fvtemp[1].vz - fvtemp[0].vz );
	a_upper = ( fvtemp[1].vw - fvtemp[0].vw ) / ( fvtemp[1].vz - fvtemp[0].vz );

//printf("uu:%f %f\n",a_under,a_upper);

	if( fvtemp[0].vz > -size ){				/* 左側：切断が起こった */
		tex_u0 = -fvtemp[0].vz / size;
		if( fvtemp[0].vw <  size ){			/* 左側：上側：交点をポリゴンの頂点 */
			tex_v0 = fvtemp[0].vw / size;
		}
		if( fvtemp[0].vy > -size ){			/* 左側：下側：交点をポリゴンの頂点 */
			tex_v2 = -fvtemp[0].vy / size;
		}
	}else{
		ftemp0 = fvtemp[0].vw + ( -size - fvtemp[0].vz ) *  a_upper;	/* 壁の上辺とポリゴンの縦線の交点 */
		if( ftemp0 < size ){				/* 左側：上側：交点をポリゴンの頂点 */
			tex_v0 = ftemp0 / size;
		}
		ftemp0 = fvtemp[0].vy + ( -size - fvtemp[0].vz ) *  a_under;	/* 壁の底辺とポリゴンの縦線の交点 */
		if( ftemp0 > -size ){				/* 左側：下側：交点をポリゴンの頂点 */
			tex_v2 = -ftemp0 / size;
		}
	}

	if( fvtemp[1].vz <  size ){				/* 右側：切断が起こった */
		tex_u1 = fvtemp[1].vz / size;
		if( fvtemp[1].vw <  size ){			/* 右側：上側：交点をポリゴンの頂点 */
			tex_v1 = fvtemp[1].vw / size;
		}
		if( fvtemp[1].vy > -size ){			/* 右側：下側：交点をポリゴンの頂点 */
			tex_v3 = -fvtemp[1].vy / size;
		}
	}else{
		ftemp0 = fvtemp[1].vw + ( -size - fvtemp[1].vz ) *  a_upper;	/* 壁の上辺とポリゴンの縦線の交点 */
		if( ftemp0 < size ){				/* 右側：上側：交点をポリゴンの頂点 */
			tex_v1 = ftemp0 / size;
		}
		ftemp0 = fvtemp[1].vy + ( -size - fvtemp[1].vz ) *  a_under;	/* 壁の底辺とポリゴンの縦線の交点 */
		if( ftemp0 > -size ){				/* 右側：下側：交点をポリゴンの頂点 */
			tex_v3 = -ftemp0 / size;
		}
	}


//printf("tex %f %f %f %f %f %f\n",tex_u0,tex_u1,tex_v0,tex_v1,tex_v2,tex_v3);


	pos[0].vx =  0;
	pos[0].vy =  size * tex_v0;
	pos[0].vz = -size * tex_u0;

	pos[1].vx =  0;
	pos[1].vy =  size * tex_v1;
	pos[1].vz =  size * tex_u1;

	pos[2].vx =  0;
	pos[2].vy = -size * tex_v2;
	pos[2].vz = -size * tex_u0;

	pos[3].vx =  0;
	pos[3].vy = -size * tex_v3;
	pos[3].vz =  size * tex_u1;

	DG_SetPos2( &unit->point_pos, point_rot );
	DG_PutVector( pos, pos, 4 );

	check  = work->wall_num;
	u_scale  = work->tex_dat[check].u_scale ;
	u_offset = work->tex_dat[check].u_offset;
	v_scale  = work->tex_dat[check].v_scale ;
	v_offset = work->tex_dat[check].v_offset;

	tex_u0 = (1.0f - tex_u0) * 0.5f;
	tex_u1 = (1.0f + tex_u1) * 0.5f;

	tex_v0 = (1.0f - tex_v0) * 0.5f;
	tex_v1 = (1.0f - tex_v1) * 0.5f;
	tex_v2 = (1.0f + tex_v2) * 0.5f;
	tex_v3 = (1.0f + tex_v3) * 0.5f;

	uvrgb->a = COLOR_A ;
	uvrgb->u = FTOI12( tex_u0 * u_scale + u_offset ) ;
	uvrgb->v = FTOI12( tex_v0 * v_scale + v_offset ) ;
	uvrgb++;

	uvrgb->a = COLOR_A ;
	uvrgb->u = FTOI12( tex_u1 * u_scale + u_offset ) ;
	uvrgb->v = FTOI12( tex_v1 * v_scale + v_offset ) ;
	uvrgb++;

	uvrgb->a = COLOR_A ;
	uvrgb->u = FTOI12( tex_u0 * u_scale + u_offset ) ;
	uvrgb->v = FTOI12( tex_v2 * v_scale + v_offset ) ;
	uvrgb++;

	uvrgb->a = COLOR_A ;
	uvrgb->u = FTOI12( tex_u1 * u_scale + u_offset ) ;
	uvrgb->v = FTOI12( tex_v3 * v_scale + v_offset ) ;


	work->wall_num++;
	if( work->wall_num >= WALL_NUM ) work->wall_num = 0;

}

static	void	ShiftCenterPosition( FVECTOR *pos, FVECTOR *vec )
{
	FVECTOR	fv_shift;

	_sceVu0Normalize( &fv_shift, vec );
	fv_shift.vx *= -SHIFT_CENTER;
	fv_shift.vy *= -SHIFT_CENTER;
	fv_shift.vz *= -SHIFT_CENTER;
	_sceVu0AddVector( pos, pos, &fv_shift );
}

static	int	SegSameCheck( Unit *unit, Unit *unit_other )
{
	HZX_SEG	*seg;
	HZX_VEC	*hzx_vec0;
	HZX_VEC	*hzx_vec1;

	seg = (HZX_SEG *)&unit->seg;
	hzx_vec0 = &seg->p1;
	seg = (HZX_SEG *)&unit_other->seg;
	hzx_vec1 = &seg->p1;
	if(  hzx_vec0->x == hzx_vec1->x
	  && hzx_vec0->z == hzx_vec1->z
	  && hzx_vec0->y == hzx_vec1->y
	  && hzx_vec0->h == hzx_vec1->h ){
		seg = (HZX_SEG *)&unit->seg;
		hzx_vec0 = &seg->p1;
		seg = (HZX_SEG *)&unit_other->seg;
		hzx_vec1 = &seg->p1;
		if(  hzx_vec0->x == hzx_vec1->x
		  && hzx_vec0->z == hzx_vec1->z
		  && hzx_vec0->y == hzx_vec1->y
		  && hzx_vec0->h == hzx_vec1->h ){
		return 1;
		}
	}
	return 0;
}

static	int	FlrSameCheck( Unit *unit, Unit *unit_other )
{
	if(
	     unit->seg.p1.x == unit_other->seg.p1.x
	  && unit->seg.p1.z == unit_other->seg.p1.z
	  && unit->seg.p1.y == unit_other->seg.p1.y
	  && unit->seg.p1.h == unit_other->seg.p1.h

	  && unit->seg.p2.x == unit_other->seg.p2.x
	  && unit->seg.p2.z == unit_other->seg.p2.z
	  && unit->seg.p2.y == unit_other->seg.p2.y
	  && unit->seg.p2.h == unit_other->seg.p2.h

	  && unit->seg.p3.x == unit_other->seg.p3.x
	  && unit->seg.p3.z == unit_other->seg.p3.z
	  && unit->seg.p3.y == unit_other->seg.p3.y
	  && unit->seg.p3.h == unit_other->seg.p3.h

	  && unit->seg.p4.x == unit_other->seg.p4.x
	  && unit->seg.p4.z == unit_other->seg.p4.z
	  && unit->seg.p4.y == unit_other->seg.p4.y
	  && unit->seg.p4.h == unit_other->seg.p4.h
	 ){		/* 床と床 */
		return 1;
	}
	return 0;
}

void	PutWallSusu( FVECTOR *pos )
{
	static	FVECTOR	line={ 0.0f, SEARCH_LENGTH, 0.0f, 0.0f };
	Work	*work;
	u_int	atr ;
	FVECTOR	search_line;
	FVECTOR	online_vec;
	SVECTOR	point_rot;
	SVECTOR	search_rot;
	int		hzx_group;
	int		i,j;
	Unit	*unit;
	Unit	*unit_other;

	if( OK_SUSU_WALL_WORK==NULL ){
		if( NewBombWall() == NULL ){
			printf("OK_SUSU_WALL_WORK was NULL\n");
			return;
		}
	}
	work = OK_SUSU_WALL_WORK;

	/* 周辺ハザード検索 */
	hzx_group = GM_GetHzxGroupID( GM_CurrentStageMap );
	search_rot.vz = 0;
	unit = work->unit;
	for( j=0; j<X_ANGLE_NUM; j++ ){
		search_rot.vx = 512 + j*(1024-512);
//		search_rot.vx = 256 + j*(1024-256);
		for( i=0; i<Y_ANGLE_NUM; i++ ){
			search_rot.vy = i*(4096/Y_ANGLE_NUM);
			DG_SetPos2( pos, &search_rot );
			DG_PutVector( &line, &search_line, 1 );
			unit->flag = HZX_OnlineHazardCheck( /* ハザードチェック */
					hzx_group,
					pos,
					&search_line,
					HZX_CHK_FIX,
					HZX_SEG_NO_RECOIL | HZX_SEG_RECOIL_TYPE,
					HZX_FLOOR_NO_RECOIL | HZX_FLOOR_RECOIL_TYPE );

			if( unit->flag != 0 ){
//printf("o");
				HZX_GetOnlinePoint( &unit->point_pos );
				HZX_GetOnlineVector( &online_vec );
				/* 優先間違い防止のため発生元の方へシフトさせる */
				ShiftCenterPosition( &unit->point_pos, &online_vec );
//AN_Test_Eye3( &unit->point_pos );
				unit->scale = GV_VecLen3F( &online_vec ); /* 血発生位置から壁（床）までの距離 */
				HZX_GetOnlineHazard( &unit->seg, &atr ) ;
			}else{
//printf("x");
			}
			unit++;
		}
	}
//printf("\n");

/*
	unit       = work->unit;
	for( j=0; j<X_ANGLE_NUM * Y_ANGLE_NUM; j++ ){
		printf("%d",unit->flag);
		unit++;
	}
	printf("\n");
*/

	/* 同一チェック */
	unit       = work->unit;
	for( j=0; j<X_ANGLE_NUM * Y_ANGLE_NUM-1; j++ ){
		if(unit->flag!=0){
			unit_other = &work->unit[j+1];
			if(unit->flag==1){			/* 壁 */
				for( i=j+1; i<X_ANGLE_NUM * Y_ANGLE_NUM; i++ ){
					if(unit_other->flag==1){	/* 壁と壁 */
						if( SegSameCheck( unit, unit_other ) ){
							unit_other->flag = 0;	/* 同じ壁が存在した */
							if( unit->scale > unit_other->scale ){
								unit->scale = unit_other->scale;
								DG_COPY_VEC( &unit->point_pos, &unit_other->point_pos );
							}
						}
					}
					unit_other++;
				}
			}else if(unit->flag==2){	/* 床 */
				for( i=j+1; i<X_ANGLE_NUM * Y_ANGLE_NUM; i++ ){
					if(unit_other->flag==2){	/* 床と床 */
						if( FlrSameCheck( unit, unit_other ) ){
							unit_other->flag = 0;	/* 同じ床が存在した */
							if( unit->scale > unit_other->scale ){
								unit->scale = unit_other->scale;
								DG_COPY_VEC( &unit->point_pos, &unit_other->point_pos );
							}
						}
					}
					unit_other++;
				}
			}
		}
		unit++;
	}

	unit       = work->unit;
	for( j=0; j<X_ANGLE_NUM * Y_ANGLE_NUM; j++ ){
//if( unit->flag ) AN_Test_Eye3( &unit->point_pos );
//		printf("%d",unit->flag);
		if(unit->flag==1){			/* 壁 */
			CalcLocalRot_Seg( (HZX_SEG *)&unit->seg, &point_rot );
			SetPrims_Seg( work, unit, &point_rot );
//E3暫定ＣＵＴ
//		}else if(unit->flag==2){	/* 床 */
//			CalcLocalRot_Floor( &unit->seg, &point_rot );
//			SetPrims_Floor( work, unit, &point_rot );
		}
		unit++;
	}
//	printf("\n");


}


/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos0 ;
	FVECTOR				*pos1 ;
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	pos0 = prim->pos[0] ;
	pos1 = prim->pos[1] ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		pos0->vx = 0.0f;
		pos0->vy = 0.0f;
		pos0->vz = 0.0f;
		DG_COPY_VEC( pos1, pos0 );	pos0++;	pos1++;

		pos0->vx = 0.0f;
		pos0->vy = 0.0f;
		pos0->vz = 0.0f;
		DG_COPY_VEC( pos1, pos0 );	pos0++;	pos1++;

		pos0->vx = 0.0f;
		pos0->vy = 0.0f;
		pos0->vz = 0.0f;
		DG_COPY_VEC( pos1, pos0 );	pos0++;	pos1++;

		pos0->vx = 0.0f;
		pos0->vy = 0.0f;
		pos0->vz = 0.0f;
		DG_COPY_VEC( pos1, pos0 );	pos0++;	pos1++;
	}

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
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

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	int	i,j;
	DG_PRIM2		*prim ;
	DG_TEX		*tex[MAX_TEX] ;

	tex[0] = DG_GetTexture( 1081306 /*"bombpowder2_msk"*/ );
	tex[1] = DG_GetTexture( 1081306 /*"bombpowder2_msk"*/ );
	tex[2] = DG_GetTexture( 1081306 /*"bombpowder2_msk"*/ );
	tex[3] = DG_GetTexture( 1081306 /*"bombpowder2_msk"*/ );

	for( i=0; i<WALL_NUM; i++ ){
		prim = work->prim[i] = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
		if(prim==NULL){
			printf("ERR!!:BombWall\n");
			return -1;
		}
		j = i%MAX_TEX;
		InitPacket2( work, prim, tex[j] );
		work->tex_dat[i].u_scale  = tex[j]->u_scale;
		work->tex_dat[i].u_offset = tex[j]->u_offset;
		work->tex_dat[i].v_scale  = tex[j]->v_scale;
		work->tex_dat[i].v_offset = tex[j]->v_offset;

		work->swap_flag[i]=0;
	}


	return 0 ;
}

void *NewBombWall( void )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		OK_SUSU_WALL_WORK=work;
		OK_SUSU_WALL_WORK->wall_num=0;
	}
	return (void *)work ;
}

