//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_wl.c
	壁に飛び散る血
	1999/09/01 S.Okajima
	$Id: blood_wl.c,v 1.1.1.3 2002/11/19 11:46:59 Yoshizawa1 Exp $
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

extern void	*OK_BLOOD_SPREAD_WORK;
extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern void *NewBloodWall( int name, int map );

int	ok_blood_wl_count;
int	OK_FloorOffFlag;

/*----------------------------------------------------------------*/
#define CHECK_TYPE (HZX_CHK_FIX | HZX_CHK_RECOIL_TYPE)				/* 園山さん指定 */
#define SEG_TYPE   (HZX_SEG_NO_BULLET)								/* 園山さん指定 */
#define FLR_TYPE   (HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_BLOOD)		/* 園山さん指定 */
#define SEG_ATTR   (HZX_SEG_NO_BULLETHOLE)							/* 園山さん指定 */
#define FLR_ATTR   (HZX_FLOOR_NO_BULLETHOLE | HZX_FLOOR_NO_OBJECT)	/* 園山さん指定 */
/*----------------------------------------------------------------*/
#define	MAX_TEX			(5)
#define	N_VERTS			(32)
#define	N_POLYS			(N_VERTS/4)
#define	N_PRIMS			(8)

/* グループ内でのパターン */
#define	N_PAT			(N_POLYS)

#define	DECAY_BEFORE_NUM	(4)

#define	DECAY_RATE		(2.0f)

#define	SEARCH_LENGTH	(2000.0f)
//#define	SEARCH_LENGTH	(1000.0f)

/* 以下の合計が最大到達点 */
#define	SCALE_MIN		(1200.0f)
#define	MAX_SCALE		(2000.0f)

#ifdef ENGLISH
#define	COLOR_A			(128)
#else
#define	COLOR_A		(BLOOD_ALPHA_1)
#endif

#define	DIVIDE			(8)
#define	CONST_MAX			(65536)
#define	DATA			(CONST_MAX*(DIVIDE-1)/DIVIDE)

#define	SPREAD_DECAY	(300)

#ifdef ENGLISH
#define	PAT_SIZE1		(512.0f / (float)N_PAT)
#define	PAT_SIZE2		(512.0f / (float)N_PAT)
#define	PAT_SIZE_MIN	(128.0f)
#else
#define	PAT_SIZE1		(256.0f / (float)N_PAT)
#define	PAT_SIZE2		(256.0f / (float)N_PAT)
#define	PAT_SIZE_MIN	(128.0f) /*128.0f*/
#endif

#define	SHIFT_CENTER	(5.0f)
//#define	RAISE			(-4000)
#define	RAISE			(-60000)

#define	BOUNDARY_CHECK	(SHIFT_CENTER   + 20.0f)
#define	BOUNDARY_OFF	(510.0f)

#define	SERACH_WIDTH	(256)

#define	TILL_DECAY_TIME		(COLOR_A/(int)DECAY_RATE)

typedef	struct	{
	DG_PRIM2		*prim ;
} Unit ;


typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	int			init_check_count;

	int			grp_count;	/* グループカウント */
	int			next_number;

	FVECTOR		d_pos[N_PAT * 4] ;	/* 更新は unit 分毎行なう */

	float		alpha[N_PRIMS*N_POLYS];
	Unit		unit[N_PAT];
} Work ;

Work	*OK_BLOOD_WALL_WORK = NULL;



/*----------------------------------------------------------------*/
// 床面を消しても血が残る問題用
static void BoundCallBack( Work *work )
{
	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		work->init_check_count = TILL_DECAY_TIME + 60;
//		work->init_check_count = 4;
		ok_blood_wl_count = 0;
	}
}

/*----------------------------------------------------------------*/
// 床面を消しても血が残る問題用
static void CheckAndClear( Work *work, int clock )
{
	int i,j ;
	int	flag;
	FVECTOR	*keep_p;
	FVECTOR	*keep_p2;
	FVECTOR	*fv_p;
	FVECTOR	*fv_p2;
	FVECTOR	fvtemp;

	work->init_check_count--;
//printf("%d\n",work->init_check_count);

	for( i=0; i<N_PAT; i++ ){
		fv_p = work->unit[i].prim->pos[clock];
		fv_p2 = work->unit[i].prim->pos[1-clock];
		for( j=0; j<N_PRIMS*N_POLYS; j++ ){
			keep_p = fv_p;
			keep_p2 = fv_p2;
			flag = 0;

			if( UTL_EFT_CheckBound( fv_p++ ) >= 0 ){	/*バウンドに入る？*/
				flag = 1;
				fv_p+=3;
			}else{
				if( UTL_EFT_CheckBound( fv_p++ ) >= 0 ){	/*バウンドに入る？*/
					flag = 1;
					fv_p+=2;
				}else{
					if( UTL_EFT_CheckBound( fv_p++ ) >= 0 ){	/*バウンドに入る？*/
						flag = 1;
						fv_p++;
					}else{
						if( UTL_EFT_CheckBound( fv_p++ ) >= 0 ){	/*バウンドに入る？*/
							flag = 1;
						}
					}
				}
			}

			if( UTL_EFT_CheckBound( fv_p2++ ) >= 0 ){	/*バウンドに入る？*/
				flag = 1;
				fv_p2+=3;
			}else{
				if( UTL_EFT_CheckBound( fv_p2++ ) >= 0 ){	/*バウンドに入る？*/
					flag = 1;
					fv_p2+=2;
				}else{
					if( UTL_EFT_CheckBound( fv_p2++ ) >= 0 ){	/*バウンドに入る？*/
						flag = 1;
						fv_p2++;
					}else{
						if( UTL_EFT_CheckBound( fv_p2++ ) >= 0 ){	/*バウンドに入る？*/
							flag = 1;
						}
					}
				}
			}

			if( flag ){
#if 1
				DG_COPY_VEC( &fvtemp, keep_p++ );
				DG_COPY_VEC( keep_p++, &fvtemp );
				DG_COPY_VEC( keep_p++, &fvtemp );
				DG_COPY_VEC( keep_p++, &fvtemp );

				DG_COPY_VEC( &fvtemp, keep_p2++ );
				DG_COPY_VEC( keep_p2++, &fvtemp );
				DG_COPY_VEC( keep_p2++, &fvtemp );
				DG_COPY_VEC( keep_p2++, &fvtemp );
#else
				DG_COPY_VEC( keep_p++, &DG_ZeroVector );
				DG_COPY_VEC( keep_p++, &DG_ZeroVector );
				DG_COPY_VEC( keep_p++, &DG_ZeroVector );
				DG_COPY_VEC( keep_p++, &DG_ZeroVector );
#endif
			}
		}
	}
/*
	fv_p = work->d_pos;
	for( i=0; i<N_PAT; i++ ){
		DG_COPY_VEC( &fvtemp, fv_p++ );
		DG_COPY_VEC( fv_p++, &fvtemp );
		DG_COPY_VEC( fv_p++, &fvtemp );
		DG_COPY_VEC( fv_p++, &fvtemp );
	}
*/
}


/*----------------------------------------------------------------*/
static void NextGroupSet( Work *work )
{

// 最後を優先下げの為に使う（カメラから遠くにセットする）

	work->grp_count++;
	if( work->grp_count >= N_PRIMS*N_POLYS-1 ) work->grp_count=0;
	work->next_number = work->grp_count+1;
	if( work->next_number >= N_PRIMS*N_POLYS-1 ) work->next_number=0;

	work->alpha[work->grp_count] = COLOR_A;

}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j,k;
	int	clock;
	FVECTOR	*pos;
	FVECTOR	*pos2;
	FVECTOR	*d_pos;
	FVECTOR	fvtemp;
	DG_PRIM2_UVRGB		*uvrgb ;
	DG_PRIM2_UVRGB		*uvrgb2 ;
	float	*d_alpha;
	int		int_alpha;
	Unit	*unit;

   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }
	for( i=0; i<N_PAT; i++ ){
//		GM_GroupPrim2( work->unit[i].prim, GM_PlayerMap ) ;
//		GM_GroupPrim2( work->unit[i].prim, 0x7fffffff ) ;	// 常に表示
		work->unit[i].prim->group_id = 0x7fffffff;	// 常に表示
		DG_SwitchBuffPrim2( work->unit[i].prim );
	}
	clock = work->unit[0].prim->buffer_clock;
	if( work->init_check_count ){
		OK_FloorOffFlag = 1;
		CheckAndClear( work, clock );
	}else{
		OK_FloorOffFlag = 0;
	}

	// 一つ遠くに設定
	_sceVu0ScaleVector( &fvtemp, (FVECTOR *)DG_Chanls->eye.m[2], 100000.0f );
	unit  = work->unit;
	for( i=0; i<N_PAT; i++ ){
		pos = unit->prim->pos[clock];
		pos+= (N_PRIMS*N_POLYS-1)*4;
		for( j=0; j<4; j++ ){
			DG_COPY_VEC( pos++, &fvtemp );
		}
		unit++;
	}



	j = TILL_DECAY_TIME-ok_blood_wl_count;	/* 時間差 */
//printf(":::::%d\n",j);
	if( work->init_check_count <= 0 ){
		if( j!=TILL_DECAY_TIME ){
			if( j<N_PAT/2 ) j=N_PAT/2;
			if( j>N_PAT   ) j=N_PAT  ;
			d_pos    = work->d_pos;
			d_alpha  = work->alpha;
			d_alpha += work->grp_count;
			int_alpha=(int)(*d_alpha);
			for( k=0; k<j; k++ ){
				unit   = &work->unit[k];

				pos    = unit->prim->pos[clock];
				pos   += work->grp_count * 4;
				uvrgb  = unit->prim->uvrgb[clock];
				uvrgb += work->grp_count * 4;

				pos2    = unit->prim->pos[1-clock];
				pos2   += work->grp_count * 4;
				uvrgb2  = unit->prim->uvrgb[1-clock];
				uvrgb2 += work->grp_count * 4;

//printf("a:%d %d\n",k,work->grp_count);
				DG_COPY_VEC( pos++,  d_pos );
				DG_COPY_VEC( pos2++, d_pos );
				d_pos++;
				DG_COPY_VEC( pos++,  d_pos );
				DG_COPY_VEC( pos2++, d_pos );
				d_pos++;
				DG_COPY_VEC( pos++,  d_pos );
				DG_COPY_VEC( pos2++, d_pos );
				d_pos++;
				DG_COPY_VEC( pos++,  d_pos );
				DG_COPY_VEC( pos2++, d_pos );
				d_pos++;

				uvrgb->a =  int_alpha;	uvrgb++;
				uvrgb->a =  int_alpha;	uvrgb++;
				uvrgb->a =  int_alpha;	uvrgb++;
				uvrgb->a =  int_alpha;	uvrgb++;
				uvrgb2->a = int_alpha;	uvrgb2++;
				uvrgb2->a = int_alpha;	uvrgb2++;
				uvrgb2->a = int_alpha;	uvrgb2++;
				uvrgb2->a = int_alpha;	uvrgb2++;
			}
		}
	}

	//--------

	d_alpha = work->alpha;
	d_alpha += work->next_number;
	int_alpha=(int)(*d_alpha);
	for( i=0; i<N_PAT; i++ ){
		unit  = &work->unit[i];
		uvrgb = unit->prim->uvrgb[clock];
		uvrgb += work->next_number*4;

		uvrgb->a = int_alpha;	uvrgb++;
		uvrgb->a = int_alpha;	uvrgb++;
		uvrgb->a = int_alpha;	uvrgb++;
		uvrgb->a = int_alpha;	uvrgb++;
	}
	(*d_alpha) -= DECAY_RATE;
	if( (*d_alpha) < 0.0f ) (*d_alpha) = 0.0f;

	if( ok_blood_wl_count > 0 ) ok_blood_wl_count--;
}

static void Die( Work *work )
{
	int	i;

	UTL_EFT_DelCallback( work ) ;

	for( i=0; i<N_PAT; i++ ){
		work->unit[i].prim = OK_FreePrim2 ( work->unit[i].prim );
	}
}



/* seg についての rotを得る */
static	void	CalcLocalRot_Seg( HZX_SEG *seg, SVECTOR *rot, FVECTOR *norm_dir )
{
	FVECTOR fvtemp0;
	FVECTOR fvtemp1;
	float tmp ;


	rot->vx=0;
	rot->vz=0;


	fvtemp0.vx = seg->p2.z - seg->p1.z;
	fvtemp0.vy = 0.0f;
	fvtemp0.vz = seg->p2.x - seg->p1.x;
	fvtemp0.vw = 0.0f;

	_sceVu0Normalize( &fvtemp0, &fvtemp0 );
	_sceVu0Normalize( &fvtemp1, norm_dir );

	fvtemp1.vx = -fvtemp1.vx;
	tmp = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );

	if( tmp < 0.0f ){	// 壁の表裏をチェックし、表から見たポリゴンの頂点回転を合わせる
		tmp = atan2f( fvtemp0.vz, fvtemp0.vx );
	}else{
		tmp = atan2f( fvtemp0.vz, fvtemp0.vx ) + PI;
	}

	rot->vy = 4095 & ( short )( ( tmp * 2048.0f / PI ) + 0.5f ) ; /* 四捨五入 */
}

/* floor についての rotを得る */
static	void	CalcLocalRot_Floor( HZX_FLR *floor, SVECTOR *rot )
{
	FVECTOR	to;

	/* 法線収得 */
	to.vx=floor->p1.h;
	to.vy=floor->p3.h;
	to.vz=floor->p2.h;

	OK_DirVecXY( &to, &DG_ZeroVector, rot );
}


#if 0
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
#endif

#if 0
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
#endif

static void GetTriData( FVECTOR *center, FVECTOR *vec, HZX_SEG *seg )
{
	center[0].vx = seg->p1.x;
	center[0].vy = seg->p1.y;
	center[0].vz = seg->p1.z;
	center[0].vw = 1.0f;

	center[1].vx = seg->p2.x;
	center[1].vy = seg->p2.y + seg->p2.h;
	center[1].vz = seg->p2.z;
	center[1].vw = 1.0f;
	
	vec[0].vx = 0.0f;
	vec[0].vy = seg->p1.h;
	vec[0].vz = 0.0f;
	vec[0].vw = 0.0f;
	
	vec[1].vx = seg->p2.x - seg->p1.x;
	vec[1].vy = seg->p2.y - seg->p1.y;
	vec[1].vz = seg->p2.z - seg->p1.z;
	vec[1].vw = 0.0f;

	vec[2].vx = seg->p1.x - seg->p2.x;
	vec[2].vy = (seg->p1.y + seg->p1.h) - (seg->p2.y+seg->p2.h);
	vec[2].vz = seg->p1.z - seg->p2.z;
	vec[2].vw = 0.0f;

	vec[3].vx = 0.0f;
	vec[3].vy = -seg->p2.h;
	vec[3].vz = 0.0f;
	vec[3].vw = 0.0f;
}

static void GetTriData_flr( FVECTOR *center, FVECTOR *vec, HZX_SEG *seg )
{
	center[0].vx = seg->p1.x;
	center[0].vy = seg->p1.y;
	center[0].vz = seg->p1.z;
	center[0].vw = 1.0f;

	center[1].vx = seg->p3.x;
	center[1].vy = seg->p3.y;
	center[1].vz = seg->p3.z;
	center[1].vw = 1.0f;
	
	vec[0].vx = seg->p2.x - seg->p1.x;
	vec[0].vy = seg->p2.y - seg->p1.y;
	vec[0].vz = seg->p2.z - seg->p1.z;
	vec[0].vw = 0.0f;
	
	vec[1].vx = seg->p4.x - seg->p1.x;
	vec[1].vy = seg->p4.y - seg->p1.y;
	vec[1].vz = seg->p4.z - seg->p1.z;
	vec[1].vw = 0.0f;
	
	vec[2].vx = seg->p2.x - seg->p3.x;
	vec[2].vy = seg->p2.y - seg->p3.y;
	vec[2].vz = seg->p2.z - seg->p3.z;
	vec[2].vw = 0.0f;
	
	vec[3].vx = seg->p4.x - seg->p3.x;
	vec[3].vy = seg->p4.y - seg->p3.y;
	vec[3].vz = seg->p4.z - seg->p3.z;
	vec[3].vw = 0.0f;

	
}


static inline float _Vu0VecLenXYZ( FVECTOR *vec )
{
	float	ans;
	
	
#ifdef BP_PSX2_ASM
	asm volatile ("
    lqc2		vf4, 0x00(%1)
    vmul.xyz    vf5, vf4, vf4
    vmulax.w	ACC, vf0, vf5x
    vmadday.w	ACC, vf0, vf5y
    vmaddz.w	vf5, vf0, vf5z
	vsqrt		Q, vf5w
	vwaitq
	vaddq.x		vf6, vf0, Q
    qmfc2.i		$8,vf6
    sw			$8,0(%0)
	": : "r"(&ans), "r"(vec) : "$8", "memory" );
#else
	

	ans=BP_Vec3_Length(
	    vec
	); 
/*	
	//lqc2		vf4, 0x00(%1)
	vf4 = *vec;
	
	//vmul.xyz    vf5, vf4, vf4
	vf5 = vf4 * vf4;
	
	//vmulax.w	ACC, vf0, vf5x
	ACC.w = vf0.w * vf5.x;
	
	//vmadday.w	ACC, vf0, vf5y
	ACC.w += ACC.w + vf0.w * vf5.y;
	
	//vmaddz.w	vf5, vf0, vf5z
	vf5.w += ACC.w + vf0.w * vf5.z;
	
	//vsqrt		Q, vf5w
	//vwaitq
	//vaddq.x		vf6, vf0, Q
	//qmfc2.i		$8,vf6
	//sw			$8,0(%0)
*/
	


#endif

	return (ans);
}


static inline float BackFaceCulling( FVECTOR *n, FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    FVECTOR a0, a1 ;

    _sceVu0SubVector( &a0, v2, v1 ) ;
    _sceVu0SubVector( &a1, v3, v1 ) ;
    _sceVu0OuterProduct( &a0, &a0, &a1 ) ;
	return _sceVu0InnerProduct( n, &a0 );
}
static inline void NormalVector( FVECTOR *n, FVECTOR *v1, FVECTOR *v2, FVECTOR *v3 )
{
    FVECTOR a0, a1 ;

    _sceVu0SubVector( &a0, v2, v1 ) ;
    _sceVu0SubVector( &a1, v3, v1 ) ;
    _sceVu0OuterProduct( n, &a0, &a1 ) ;
    _sceVu0Normalize( n, n ) ;
}

// 三角形二つとしてバンダリチェック
static	int	CheckPos4WithinSeg_Tri( FVECTOR *pos, FVECTOR *center, FVECTOR *vec )
{
#if 1  /* バグっていたのでこっちに変えました。床もこっちのほうが良いかも T.Morita */
	FVECTOR	 *check_pos = pos;
	FVECTOR   seg[4] ;
	float   a,b,c ;
	FVECTOR n ;
	int   i ;

	
	_sceVu0AddVector( &seg[0], &vec[0], &center[0] );
	_sceVu0CopyVector( &seg[1], &center[0] );
	_sceVu0CopyVector( &seg[2], &center[1] );
	_sceVu0AddVector( &seg[3], &vec[1], &center[0] );
	NormalVector( &n, &seg[0], &seg[1], &seg[2] ) ;

	for( i=4 ; --i>=0 ; check_pos++ ){
		/* 点が3頂点の内側にあるかどうかを判定する */
		a = BackFaceCulling( &n, &seg[0], &seg[1], check_pos ) ;
		b = BackFaceCulling( &n, &seg[1], &seg[2], check_pos ) ;
		c = BackFaceCulling( &n, &seg[2], &seg[0], check_pos ) ;
		if ( (a>0 && b>0 && c>0) || (a<0 && b<0 && c<0) ) {
			continue ;
		}

		/* 点が3頂点の内側にあるかどうかを判定する */
		a = BackFaceCulling( &n, &seg[1], &seg[2], check_pos ) ;
		b = BackFaceCulling( &n, &seg[2], &seg[3], check_pos ) ;
		c = BackFaceCulling( &n, &seg[3], &seg[1], check_pos ) ;
		if ( (a>0 && b>0 && c>0) || (a<0 && b<0 && c<0) ) {
			continue ;
		}
		return 0 ;
	}
	return 1 ;
#else
	FVECTOR		*check_pos = pos;
	int			i;
	float		s,t,vec_len[4];
	FVECTOR		fvtemp0,fvtemp1;

	vec_len[0] = _Vu0VecLenXYZ( &vec[0] );
	vec_len[1] = _Vu0VecLenXYZ( &vec[1] );
	vec_len[2] = _Vu0VecLenXYZ( &vec[2] );
	vec_len[3] = _Vu0VecLenXYZ( &vec[3] );

	for( i = 0; i < 4; i++,check_pos++ ){
		_sceVu0Normalize( &fvtemp0, &vec[0] );
		_sceVu0SubVector( &fvtemp1, check_pos, &center[0] );
		s = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );
		s /= vec_len[0];
		_sceVu0Normalize( &fvtemp0, &vec[1] );
		t = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );
		t /= vec_len[1];
		if( s > 0.0f && t > 0.0f && s+t < 1.0f ){
			//AN_Test_Eye2( check_pos, 2 );
			continue;
		}
		_sceVu0Normalize( &fvtemp0, &vec[2] );
		_sceVu0SubVector( &fvtemp1, check_pos, &center[1] );
		s = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );
		s /= vec_len[2];
		_sceVu0Normalize( &fvtemp0, &vec[3] );
		t = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );
		t /= vec_len[3];
		if( s > 0.0f && t > 0.0f && s+t < 1.0f ){
			//AN_Test_Eye2( check_pos, 2 );
			continue;
		}
		//AN_Test_Eye2( check_pos, 2 );

		return 0;
	}
	return 1;
#endif	

}


// 三角形二つとしてバンダリチェック
static	int	CheckPos4WithinSeg_TriEx( FVECTOR *a, FVECTOR *b, FVECTOR *center, FVECTOR *vec )
{
	int			i,check;
	float		k[2];
	float		len;
	FVECTOR		cross[2];
	FVECTOR		diff;
	FVECTOR		verts[4];
	FVECTOR		vects[4];
	//各係数準備

	_sceVu0SubVector( &diff, b, a );
	len = _Vu0VecLenXYZ( &diff );
	//各頂点
	DG_COPY_VEC( &verts[0], &center[0] );
	_sceVu0AddVector( &verts[1], &vec[0], &center[0] );
	DG_COPY_VEC( &verts[2], &center[1] );
	_sceVu0AddVector( &verts[3], &vec[1], &center[0] );

	//各ベクトル
	_sceVu0SubVector( &vects[0], &verts[1], &verts[0] );
	_sceVu0SubVector( &vects[1], &verts[2], &verts[1] );
	_sceVu0SubVector( &vects[2], &verts[3], &verts[2] );
	_sceVu0SubVector( &vects[3], &verts[0], &verts[3] );

	for( i = 0,check = 1; i < 4; i++ ){
		FVECTOR		c;
		FVECTOR		inv_c;
		FVECTOR		prim_v;
		FVECTOR		fvtemp0, fvtemp1;
		float		inner0, inner1, temp;

		_sceVu0SubVector( &c, &verts[i], a );
		_sceVu0SubVector( &inv_c, a, &verts[i] );
		_sceVu0Normalize( &prim_v, &vects[i] );

		inner0 = _sceVu0InnerProduct( &prim_v, &inv_c );
		inner1 = _sceVu0InnerProduct( &prim_v, &diff );

		_sceVu0ScaleVector( &fvtemp0, &prim_v, inner0 );
		_sceVu0AddVector( &fvtemp0, &c, &fvtemp0 );
		_sceVu0ScaleVector( &fvtemp1, &prim_v, inner1 );
		_sceVu0SubVector( &fvtemp1, &diff, &fvtemp1 );
#if 0
		fvtemp1.vx = 1.0f/fvtemp1.vx;
		fvtemp1.vy = 1.0f/fvtemp1.vy;
		fvtemp1.vz = 1.0f/fvtemp1.vz;
		_sceVu0MulVector( &fvtemp0, &fvtemp0, &fvtemp1 );
		temp = _Vu0VecLenXYZ( &fvtemp0 );
#else

#if 1 //yano/*0除算発生*/
  	{
		float div0_temp;
		div0_temp = _Vu0VecLenXYZ( &fvtemp1 );
		if( div0_temp == 0.0f ){
			div0_temp = 0.0000001f;
		}
		temp = _Vu0VecLenXYZ( &fvtemp0 )/div0_temp;
	}
#endif //yano

#endif
		if( 0.0f <= temp && temp < 1.0f ){
			_sceVu0ScaleVector( &cross[0], &diff, temp );
#if 0
			_sceVu0ScaleVector( &fvtemp0, &prim_v, inner0 + inner1*temp );
			_sceVu0AddVector( &fvtemp0, &fvtemp0, &c );
			_sceVu0SubVector( &fvtemp1, &fvtemp0, &cross[0] );
			if( _Vu0VecLenXYZ( &fvtemp1 ) < 100.0f ){
#endif
				_sceVu0AddVector( &cross[0], &cross[0], a );
				k[0] = temp;
				check = 0;
				break;
//			}
		}
	}

	if( check ){
		return 0;		//OK
	}

	_sceVu0SubVector( &diff, a, b );
	len = _Vu0VecLenXYZ( &diff );

	//各頂点
	DG_COPY_VEC( &verts[0], &center[2] );
	_sceVu0AddVector( &verts[1], &vec[4], &center[2] );
	DG_COPY_VEC( &verts[2], &center[3] );
	_sceVu0AddVector( &verts[3], &vec[5], &center[2] );

	//各ベクトル
	_sceVu0SubVector( &vects[0], &verts[1], &verts[0] );
	_sceVu0SubVector( &vects[1], &verts[2], &verts[1] );
	_sceVu0SubVector( &vects[2], &verts[3], &verts[2] );
	_sceVu0SubVector( &vects[3], &verts[0], &verts[3] );

	for( i = 0,check = 1; i < 4; i++ ){
		FVECTOR		c;
		FVECTOR		inv_c;
		FVECTOR		prim_v;
		FVECTOR		fvtemp0, fvtemp1;
		float		inner0, inner1, temp;

		_sceVu0SubVector( &c, &verts[i], b );
		_sceVu0SubVector( &inv_c, b, &verts[i] );
		_sceVu0Normalize( &prim_v, &vects[i] );

		inner0 = _sceVu0InnerProduct( &prim_v, &inv_c );
		inner1 = _sceVu0InnerProduct( &prim_v, &diff );

		_sceVu0ScaleVector( &fvtemp0, &prim_v, inner0 );
		_sceVu0AddVector( &fvtemp0, &c, &fvtemp0 );
		_sceVu0ScaleVector( &fvtemp1, &prim_v, inner1 );
		_sceVu0SubVector( &fvtemp1, &diff, &fvtemp1 );
#if 0
		fvtemp1.vx = 1.0f/fvtemp1.vx;
		fvtemp1.vy = 1.0f/fvtemp1.vy;
		fvtemp1.vz = 1.0f/fvtemp1.vz;
		_sceVu0MulVector( &fvtemp0, &fvtemp0, &fvtemp1 );
		temp = _Vu0VecLenXYZ( &fvtemp0 );
#else
		temp = _Vu0VecLenXYZ( &fvtemp0 )/_Vu0VecLenXYZ( &fvtemp1 );
#endif

		if( 0.0f <= temp && temp < 1.0f ){
			_sceVu0ScaleVector( &cross[1], &diff, temp );
#if 0
			_sceVu0ScaleVector( &fvtemp0, &prim_v, inner0 + inner1*temp );
			_sceVu0AddVector( &fvtemp0, &fvtemp0, &c );
			_sceVu0SubVector( &fvtemp1, &fvtemp0, &cross[1] );
			if( _Vu0VecLenXYZ( &fvtemp1 ) < 100.0f ){
#endif
				_sceVu0AddVector( &cross[1], &cross[1], b );
				k[1] = temp;
				check = 0;
				break;
//			}
		}
	
	}

	if( check ){
		return 1;		//NO
	}

	_sceVu0SubVector( &diff, &cross[0], &cross[1] );
	if( _Vu0VecLenXYZ( &diff ) < 100.0f ){
		//同じとみなす
		return 0;
	}else{
		//AN_Test_Eye2( &cross[0], 2 );
		//AN_Test_Eye2( &cross[1], 2 );
		return 1;
	}
	
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
	float	max,min;

	dif_x=in->p1.x - in->p2.x;
	dif_z=in->p1.z - in->p2.z;
	if( dif_x < 0 ){
		out->p1.x = in->p1.x + BOUNDARY_CHECK;
		out->p2.x = in->p2.x - BOUNDARY_CHECK;
		dif_x*=-1;
	}else{
		out->p2.x=in->p1.x - BOUNDARY_CHECK;
		out->p1.x=in->p2.x + BOUNDARY_CHECK;

	}
	if( dif_z < 0 ){
		out->p1.z=in->p1.z + BOUNDARY_CHECK;
		out->p2.z=in->p2.z - BOUNDARY_CHECK;
		dif_z*=-1;
	}else{
		out->p2.z=in->p1.z - BOUNDARY_CHECK;
		out->p1.z=in->p2.z + BOUNDARY_CHECK;
	}

	max=(dif_x > dif_z)?dif_x:dif_z;

	if( max < BOUNDARY_OFF ) return 0;

	max = (in->p1.h > in->p2.h)?in->p1.h:in->p2.h;
	min = (in->p1.h > in->p2.h)?in->p2.h:in->p1.h;
	if( min < BOUNDARY_OFF ) return 0;
	out->p1.h = min - BOUNDARY_CHECK*2.0f;
	out->p2.h = min - BOUNDARY_CHECK*2.0f;
	out->p1.y = in->p1.y + BOUNDARY_CHECK;
	out->p2.y = in->p2.y + BOUNDARY_CHECK;
	
	return 1;
}


static	void	ShiftCenterPosition( FVECTOR *pos, FVECTOR *vec, FVECTOR *norm_dir )
{
#if 0
	_sceVu0Normalize( norm_dir, vec );
	norm_dir->vx *= -SHIFT_CENTER;
	norm_dir->vy *= -SHIFT_CENTER;
	norm_dir->vz *= -SHIFT_CENTER;
	_sceVu0AddVector( pos, pos, norm_dir );
#else
	// カリング用に計算する
	_sceVu0Normalize( norm_dir, vec );
	norm_dir->vx *= -SHIFT_CENTER;
	norm_dir->vy *= -SHIFT_CENTER;
	norm_dir->vz *= -SHIFT_CENTER;
#endif
}
/*
//FVECTOR		DebugSegVerts[4];

extern void *NewGeneralSprite( int tex_code, FVECTOR *pos, float shift,
                               int *rgba, int w, int h, int life, int *mode );

//デバック用簡易呼び出し(万能テクスチャー使用、2147483647フレーム後死亡)
#define	NewDbugSprite(_pos,_size ) \
    NewGeneralSprite( (int)(6715088), (FVECTOR*)(_pos), (float)(0.0f), (int*)(NULL), \
					  (int)(_size), (int)(_size), (int)(0x7fffffff), (int*)(NULL) )
*/
/* seg に プリミティブを張る */
static	int	SetPrims_Seg( FVECTOR *point_pos, SVECTOR *point_rot, HZX_SEG *seg, float scale, FVECTOR *origin )
{
	int		i;
	float	size_half;
	FVECTOR	temp_pos;
	FVECTOR	radius;
	SVECTOR	temp_rot1;
	SVECTOR	temp_rot2;
	FVECTOR	vert[4];
	FVECTOR	size[4];
	FVECTOR	*pos;
	short	rot_step;
	short	rot_sign;
//	int		check;
	HZX_SEG check_seg;

	// 柴田追加
	FVECTOR tri_center[2];
	FVECTOR tri_vec[4];
	
	if( !MakeCalcSeg( &check_seg, seg ) ){
		//memset( DebugSegVerts, 0 ,sizeof(FVECTOR)*4 );
		return 0;	/* 作れなかった */
	}
#if 0
#if 1
	DebugSegVerts[0].vx = seg->p1.x;
	DebugSegVerts[0].vy = seg->p1.y;
	DebugSegVerts[0].vz = seg->p1.z;
	DebugSegVerts[1].vx = seg->p1.x;
	DebugSegVerts[1].vy = seg->p1.y+seg->p1.h;
	DebugSegVerts[1].vz = seg->p1.z;
	
	DebugSegVerts[2].vx = seg->p2.x;
	DebugSegVerts[2].vy = seg->p2.y;
	DebugSegVerts[2].vz = seg->p2.z;
	DebugSegVerts[3].vx = seg->p2.x;
	DebugSegVerts[3].vy = seg->p2.y+seg->p2.h;
	DebugSegVerts[3].vz = seg->p2.z;
#else
	DebugSegVerts[0].vx = check_seg.p1.x;
	DebugSegVerts[0].vy = check_seg.p1.y;
	DebugSegVerts[0].vz = check_seg.p1.z;
	DebugSegVerts[1].vx = check_seg.p1.x;
	DebugSegVerts[1].vy = check_seg.p1.y+check_seg.p1.h;
	DebugSegVerts[1].vz = check_seg.p1.z;
	
	DebugSegVerts[2].vx = check_seg.p2.x;
	DebugSegVerts[2].vy = check_seg.p2.y;
	DebugSegVerts[2].vz = check_seg.p2.z;
	DebugSegVerts[3].vx = check_seg.p2.x;
	DebugSegVerts[3].vy = check_seg.p2.y+check_seg.p2.h;
	DebugSegVerts[3].vz = check_seg.p2.z;
#endif
	DebugSegVerts[0].vw = 1.0f;
	DebugSegVerts[1].vw = 1.0f;
	DebugSegVerts[2].vw = 1.0f;
	DebugSegVerts[3].vw = 1.0f;
#endif
	rot_sign = (irnd()%2)?1:-1;

	/* 円座標 */
	radius.vx=0.0f;
	radius.vy=0.0f;		/* 初期半径 */
	radius.vz=0.0f;
	radius.vw=1.0f;

	temp_rot1.vx=(short)(irnd() % 4096);
	temp_rot1.vy=0;
	temp_rot1.vz=0;

	temp_rot2.vy=0;
	temp_rot2.vz=0;

	rot_step=0;
	pos = OK_BLOOD_WALL_WORK->d_pos;

#if 1
	size[0].vx = SHIFT_CENTER;
	size[1].vx = SHIFT_CENTER;
	size[2].vx = SHIFT_CENTER;
	size[3].vx = SHIFT_CENTER;
#else
	size[0].vx = 0.0f;
	size[1].vx = 0.0f;
	size[2].vx = 0.0f;
	size[3].vx = 0.0f;
#endif



	size[0].vw = 1.0f;
	size[1].vw = 1.0f;
	size[2].vw = 1.0f;
	size[3].vw = 1.0f;

	GetTriData( tri_center, tri_vec, seg );//&check_seg );

	for( i=N_PAT; i>0; i-- ){
		if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
		{
		temp_rot2.vx=0;
		}
		else
		{
		temp_rot2.vx=(short)(irnd() % 4096);
		}
		size_half    = (rnd() * PAT_SIZE1 + PAT_SIZE2 ) * scale * (float)i + PAT_SIZE_MIN;

		size[0].vy = + size_half;
		size[0].vz = - size_half;
		size[1].vy = + size_half;
		size[1].vz = + size_half;
		size[2].vy = - size_half;
		size[2].vz = - size_half;
		size[3].vy = - size_half;
		size[3].vz = + size_half;

		DG_SetPos2( &DG_ZeroVector, &temp_rot1 );

//		DG_SetPos2( point_pos, &temp_rot1 );
		DG_PutVector( &radius, &temp_pos, 1 );

		DG_SetPos2( &temp_pos, &temp_rot2 );
		DG_PutVector( size, vert, 4 );
		//DG_RotVector( size, vert, 4 );
		
		DG_SetPos2( point_pos, point_rot );
		DG_PutVector( vert, pos, 4 );

		_sceVu0AddVector( &temp_pos, &temp_pos, point_pos );
		if( CheckPos4WithinSeg_Tri( pos, tri_center, tri_vec ) ){
			// 見える位置か？
			if( HZX_OnlineHazardCheck( // ハザードチェック  個々にチェックする
					HZX_CurrentGroupID,
					origin,
					&temp_pos,
					HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
					0, 0 ) ){
				//HZX_HZD hzd;
				//HZX_GetOnlineHazard( &hzd, NULL );
				//GM_CurrentMap = GM_CurrentStageMap;
				//HZX_ViewHazard( &hzd );
				DG_COPY_VEC( &pos[0], point_pos );
				DG_COPY_VEC( &pos[1], point_pos );
				DG_COPY_VEC( &pos[2], point_pos );
				DG_COPY_VEC( &pos[3], point_pos );
				temp_rot1.vx += 1024;	// 範囲外の時
			}
		}else{
			DG_COPY_VEC( &pos[0], point_pos );
			DG_COPY_VEC( &pos[1], point_pos );
			DG_COPY_VEC( &pos[2], point_pos );
			DG_COPY_VEC( &pos[3], point_pos );
		}

//		radius.vy += (size_half + size_half * scale * 0.9f ) / 3.0f;	/* 遠くに付着する場合間隔が広がる */
		radius.vy += (size_half * scale );	/* 遠くに付着する場合間隔が広がる */
		if(i == N_PAT/2 ){
			rot_sign  *= -1;
			radius.vy *= 0.8f;
		}

		rot_step+=(short)(irnd()%64 + 32) * 8 / N_PAT * rot_sign;
		temp_rot1.vx += rot_step;
		pos+=4;
	}

	return 1;
}

/* floor に プリミティブを張る */
static	int	SetPrims_Floor( FVECTOR *point_pos, SVECTOR *point_rot, HZX_FLR *floor, float scale, FVECTOR *origin )
{
	int		i, j;
	float	size_half;
	FVECTOR	temp_pos;
	FVECTOR	radius;
	FVECTOR	flr_minmax[2];
	SVECTOR	temp_rot1;
	SVECTOR	temp_rot2;
	FVECTOR	vert[4];
	FVECTOR	size[4];
	FVECTOR	*pos;
	short	rot_step;
	short	rot_sign;
	int		check=1;
	int		hazard_flag;
	HZX_FLR		flr[2];
	float		flr_height[2];
	int			flr_atrs[2];
	// 柴田追加
	FVECTOR 	tri_center[4];
	FVECTOR 	tri_vec[8];
	HZX_FLR		check_flr[4];

	if( !MakeCalcFloor( flr_minmax, floor ) ) return 0;	/* 作れなかった */

	rot_sign = (irnd()%2)?1:-1;

	/* 円座標 */
	radius.vx=0.0f;
	radius.vy=0.0f;		/* 初期半径 */
	radius.vz=0.0f;
	radius.vz=1.0f;

	temp_rot1.vx=0;
	temp_rot1.vy=0;
	temp_rot1.vz=(short)(irnd() % 4096);

	temp_rot2.vx=0;
	temp_rot2.vy=0;

	rot_step=0;
	pos = OK_BLOOD_WALL_WORK->d_pos;
	size[0].vz = 0.0f;
	size[1].vz = 0.0f;
	size[2].vz = 0.0f;
	size[3].vz = 0.0f;
	
	size[0].vw = 1.0f;
	size[1].vw = 1.0f;
	size[2].vw = 1.0f;
	size[3].vw = 1.0f;
	
	hazard_flag = 0;
	size_half = 0;

	GetTriData_flr( tri_center, tri_vec, floor );
	if(0){
		FVECTOR tri_verts[6];
		DG_COPY_VEC( &tri_verts[0], &tri_center[0] );
		_sceVu0AddVector( &tri_verts[1], &tri_vec[0], &tri_center[0] );
		_sceVu0AddVector( &tri_verts[2], &tri_vec[1], &tri_center[0] );

		DG_COPY_VEC( &tri_verts[3], &tri_center[1] );
		_sceVu0AddVector( &tri_verts[4], &tri_vec[2], &tri_center[1] );
		_sceVu0AddVector( &tri_verts[5], &tri_vec[3], &tri_center[1] );

		NewTriangleView( tri_verts, 2, 128,128,128 );
	}

	for( i=N_PAT; i>0; i-- ){
		check = 1;

		if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
		{
		temp_rot2.vz = 0;
		}
		else
		{
		temp_rot2.vz = (short)(irnd() % 4096);
		}
		size_half    = ((rnd() * PAT_SIZE1 + PAT_SIZE2 ) * scale * (float)i + PAT_SIZE_MIN)*0.8f;
		size[0].vx = - size_half;
		size[0].vy = + size_half;
		size[1].vx = + size_half;
		size[1].vy = + size_half;
		size[2].vx = - size_half;
		size[2].vy = - size_half;
		size[3].vx = + size_half;
		size[3].vy = - size_half;

		DG_SetPos2( &DG_ZeroVector, &temp_rot1 );
		DG_PutVector( &radius, &temp_pos, 1 );

		DG_SetPos2( &temp_pos, &temp_rot2 );
		DG_PutVector( size, vert, 4 );

		DG_SetPos2( point_pos, point_rot );
		DG_PutVector( vert, pos, 4 );
#if 1
		//柴田修正
		for( j = 0; j < 4; j++ ){
			DG_COPY_VEC( &temp_pos, &pos[j] );
			// 見える位置か？
			if( HZX_OnlineHazardCheck( /* ハザードチェック */  /* 個々にチェックする */
					HZX_CurrentGroupID,
					origin,
					&temp_pos,
					HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
					HZX_SEG_RECOIL_TYPE , HZX_FLOOR_RECOIL_TYPE ) ){
				check = 0;
				goto TEST;
				break;
			}

			// 床はまだあるか？
			temp_pos.vy+= 10.0f;	// ちょい高め
			hazard_flag = HZX_LevelHazardCheck(
	                HZX_CurrentGroupID,
	                &temp_pos,
	                HZX_CHK_F_FLOOR | HZX_CHK_RECOIL_TYPE_ONLY,
	                HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_BLOOD );

			if( hazard_flag & 1 ){
				HZX_GetLevelHazard( flr, flr_atrs );
//printf("%x %x::%d\n",flr_atrs[0],(HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_NO_OBJECT),flr_atrs[0]&(HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_NO_OBJECT) );
				if (!(flr_atrs[0] & ( HZX_FLOOR_NO_BULLETHOLE | HZX_FLOOR_NO_OBJECT ) ) ) {
				// 個々の床属性で「血が着く」だった（着かないではなかった）
					HZX_GetLevelHeight( flr_height );
//printf("%f\n",flr_height[0]);
					if( (DG_FABS(flr_height[0] - temp_pos.vy) < BOUNDARY_CHECK+20.0f) ){
#if 0
						if( !CheckPos4WithinSeg_Tri( pos, tri_center, tri_vec ) ){
							check = 0;
							goto TEST;
							break;
						}
#endif
					}else{
						check = 0;
						goto TEST;
						break;
					}
				}else{
					check = 0;
					goto TEST;
					break;
				}
			}else{
				check = 0;
				goto TEST;
				break;
			}

			check_flr[j] = flr[0];
		}
TEST:
		if( check ){
			//全ＯＫの時の矩形またぎチェック
			for( j = 0; j < 4; j++ ){
				int			check_index[4] = { 1, 3, 0, 2 };

				GetTriData_flr( &tri_center[0], &tri_vec[0], &check_flr[j] );
				GetTriData_flr( &tri_center[2], &tri_vec[4], &check_flr[check_index[j]] );
				
				if( CheckPos4WithinSeg_TriEx( &pos[i], &pos[check_index[j]], tri_center, tri_vec ) ){
					check = 0;
					break;
				}
			}
			//printf("\n");
		}
#else
		for( j = 0; j < 4; j++ ){
			DG_COPY_VEC( &temp_pos, &pos[j] );
			// 見える位置か？
			if( HZX_OnlineHazardCheck( /* ハザードチェック */  /* 個々にチェックする */
					HZX_CurrentGroupID,
					origin,
					&temp_pos,
					HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
					HZX_SEG_RECOIL_TYPE , HZX_FLOOR_RECOIL_TYPE ) ){
				check = 0;
				break;
			}

			// 床はまだあるか？
			temp_pos.vy+= 10.0f;	// ちょい高め
			hazard_flag = HZX_LevelHazardCheck(
	                HZX_CurrentGroupID,
	                &temp_pos,
	                HZX_CHK_F_FLOOR | HZX_CHK_RECOIL_TYPE_ONLY,
	                HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_BLOOD );

			if( hazard_flag & 1 ){
				HZX_GetLevelHazard( flr, flr_atrs );
//printf("%x %x::%d\n",flr_atrs[0],(HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_NO_OBJECT),flr_atrs[0]&(HZX_FLOOR_NO_BULLETHOLE|HZX_FLOOR_NO_OBJECT) );
				if (!(flr_atrs[0] & ( HZX_FLOOR_NO_BULLETHOLE | HZX_FLOOR_NO_OBJECT ) ) ) {
				// 個々の床属性で「血が着く」だった（着かないではなかった）
					HZX_GetLevelHeight( flr_height );
//printf("%f\n",flr_height[0]);
					if( flr_height[0] < temp_pos.vy - BOUNDARY_CHECK - 20.0f ){
						if( !CheckPos4WithinSeg_Tri( pos, tri_center, tri_vec ) ){
							check = 0;
							break;
						}
						
					}
				}else{
					check = 0;
					break;
				}
			}else{
				check = 0;
				break;
			}
		}
#endif
		if(!check){	/* 座標を一点にまとめて非表示（暫定） */
			DG_COPY_VEC( &pos[0], point_pos );
			DG_COPY_VEC( &pos[1], point_pos );
			DG_COPY_VEC( &pos[2], point_pos );
			DG_COPY_VEC( &pos[3], point_pos );
		}

		radius.vx += (size_half * scale );	/* 遠くに付着する場合間隔が広がる */
		if(i == N_PAT/2 ){
			rot_sign  *= -1;
			radius.vx *= 0.8f;
		}

		rot_step+=(short)(irnd()%64 + 32) * 8 / N_PAT * rot_sign;
		temp_rot1.vz += rot_step;
		pos+=4;
	}

	return 1;
}


void	PutWallBlood( FVECTOR *pos, SVECTOR *rot, int white )
{
	static	FVECTOR	line={ 0.0f, 0.0f, SEARCH_LENGTH, 0.0f };
	Work	*work;
	HZX_HZD		hazard ;
	int			hazard_flag;
	FVECTOR	search_line;
	FVECTOR	point_pos;
	FVECTOR	online_vec;
	FVECTOR	norm_direction;
	SVECTOR	point_rot;
	SVECTOR	search_rot;
	float scale;
	int	i;
	short	direction_y;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return;

	if( OK_BLOOD_WALL_WORK==(void *)NULL ){
		if( NewBloodWall( 0, 0 )==NULL ){
			printf("OK_BLOOD_WALL_WORK was NULL\n");
			return;
		}
	}
	work = OK_BLOOD_WALL_WORK;

	if( work->init_check_count > 0 ) return;	// エリアデリート実行中

	if( ok_blood_wl_count>0 ) return;
	ok_blood_wl_count=TILL_DECAY_TIME;


	direction_y=(irnd()%2)?SERACH_WIDTH/4:-SERACH_WIDTH/4;
	search_rot.vx = rot->vx - SERACH_WIDTH;
	search_rot.vy = rot->vy;
	search_rot.vz = rot->vz;
	for( i=0; i<3; i++ ){
		DG_SetPos2( pos, &search_rot );
		DG_PutVector( &line, &search_line, 1 );
		search_rot.vx+=SERACH_WIDTH;
		search_rot.vy = rot->vx + (i-1)*direction_y;

		hazard_flag = HZX_OnlineHazardCheck( /* ハザードチェック */
				HZX_CurrentGroupID,
				pos,
				&search_line,
				HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
			    HZX_SEG_NO_BULLET, HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_BLOOD );
		// 「あたった」、（床の場合）「通過ではない」

		if( hazard_flag!=0 ){
			HZX_GetOnlineHazard( &hazard, NULL );

			HZX_GetOnlinePoint( &point_pos );

			GM_SeSetMode( SD_E_BLOOD_H1 , &point_pos, GM_SEMODE_NORMAL );	/* 血付着音 */

			HZX_GetOnlineVector( &online_vec );

			/* 優先間違い防止のため発生元の方へシフトさせる */
			ShiftCenterPosition( &point_pos, &online_vec, &norm_direction );

			scale = GV_VecLen3F( &online_vec ); /* 血発生位置から壁（床）までの距離 */
			if( scale < SCALE_MIN             ) scale = SCALE_MIN;
			if( scale > SCALE_MIN + MAX_SCALE ) scale = SCALE_MIN + MAX_SCALE;
			scale = 1.0f - scale / (SCALE_MIN + MAX_SCALE);

			if((hazard.type != HZX_TYPE_SEGMENT)) {
				if (!(hazard.attribute & ( HZX_FLOOR_NO_BULLETHOLE | HZX_FLOOR_NO_OBJECT ) ) ) {
					// 個々の床属性で血が着くだった（着かないではなかった）
					if( online_vec.vy > 0.0f ){
						//printf("upper vector for floor:skip%f\n",online_vec.vy);
						continue;	/* 天井には着かない */
					}
					point_pos.vy += 10.0f;

					CalcLocalRot_Floor( &hazard, &point_rot );
					if( SetPrims_Floor(
									   &point_pos,
									   &point_rot,
									   &hazard,
									   scale,
									   pos ) ){
						NextGroupSet( work );
						break;
					}
				}
			}else if( !(hazard.attribute & HZX_SEG_NO_BULLETHOLE) ){	/* 壁 */
				// 個々の壁属性で血が着くだった（着かないではなかった）
				CalcLocalRot_Seg( (HZX_SEG *)&hazard, &point_rot, &norm_direction );
				if( SetPrims_Seg(
				        &point_pos,
				        &point_rot,
				        (HZX_SEG *)&hazard,
				        scale,
				        pos ) ){
					NextGroupSet( work );
					break;
				}
			}
		}
	}
}


/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos0 ;
	FVECTOR				*pos1 ;
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	prim->raise = RAISE;

	pos0 = prim->pos[0] ;
	pos1 = prim->pos[1] ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
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
	}

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
//			uvrgb0->a = uvrgb1->a = COLOR_A ;
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
//			uvrgb0->a = uvrgb1->a = COLOR_A ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0000 ;/* 左回り表示 */
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
//			uvrgb0->a = uvrgb1->a = COLOR_A ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0020 ;/* 右回り表示 */
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
//			uvrgb0->a = uvrgb1->a = COLOR_A ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;
		}
	}

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	int	i,j;
	DG_PRIM2		*prim ;
	DG_TEX		*tex[MAX_TEX] ;
	FVECTOR		*pos;

	OK_FloorOffFlag = 0;
	work->init_check_count = 0;

#if 0
	tex[0] = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	tex[1] = DG_GetTexture( 12033390 /*"chi02_msk"*/ );
	tex[2] = DG_GetTexture( 13081966 /*"chi03_msk"*/ );
	tex[3] = DG_GetTexture( 14130542 /*"chi04_msk"*/ );
	tex[4] = DG_GetTexture( 15179118 /*"chi05_msk"*/ );
#else
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	tex[0] = DG_GetTexture(  9373287 /*"vr2_chi01_alp"*/ );
	tex[1] = DG_GetTexture( 10421863 /*"vr2_chi02_alp"*/ );
	tex[2] = DG_GetTexture( 11470439 /*"vr2_chi03_alp"*/ );
	tex[3] = DG_GetTexture( 12519015 /*"vr2_chi04_alp"*/ );
	tex[4] = DG_GetTexture( 13567591 /*"vr2_chi05_alp"*/ );
	}
	else
	{
	tex[0] = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	tex[1] = DG_GetTexture( 12020883 /*"chi02_alp"*/ );
	tex[2] = DG_GetTexture( 13069459 /*"chi03_alp"*/ );
	tex[3] = DG_GetTexture( 14118035 /*"chi04_alp"*/ );
	tex[4] = DG_GetTexture( 15166611 /*"chi05_alp"*/ );
	}

#endif

#define PRIM_FLAG (DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_CULLPOLY|DG_PRIM2_FOG|DG_PRIM2_CCW)
	for( i=0; i<N_PAT; i++ ){
		if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/06/25 */
		{
		prim = work->unit[i].prim = GM_MakePrim2( PRIM_FLAG & (~DG_PRIM2_FOG), N_PRIMS, N_VERTS );
		}
		else
		{
		prim = work->unit[i].prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
		}
		if(prim==NULL){
			printf("ERR!!::::::::::::::::::::::::::::::::::::::\n");
			return -1;
		}
		InitPacket2( work, prim, tex[i%MAX_TEX] );
		DG_VisiblePrim2( prim );
		pos = work->d_pos;
		for(j=0; j<N_POLYS; j++){
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
		}
	}

	for( i=0; i<N_PRIMS*N_POLYS; i++){
		work->alpha[i]  = 0.0f;
	}

	work->grp_count=0;
	work->next_number=work->grp_count+1;


	UTL_EFT_AddCallback( BoundCallBack, work ) ;

	//NewDbugSprite( &DebugSegVerts[0], 100.0f );
	//NewDbugSprite( &DebugSegVerts[1], 100.0f );
	//NewDbugSprite( &DebugSegVerts[2], 100.0f );
	//NewDbugSprite( &DebugSegVerts[3], 100.0f );
	return 0 ;
}

void *NewBloodWall( int name, int map )
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
		OK_BLOOD_WALL_WORK=work;
		OK_BLOOD_WALL_WORK->grp_count=0;
//printf("w:%d %d \n",(int)OK_BLOOD_WALL_WORK, (int)work);
	}
	return (void *)work ;
}

