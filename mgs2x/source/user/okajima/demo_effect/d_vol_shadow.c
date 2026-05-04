//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_vol_shadow.c
	体から出る影（ライン状）
	2000/04/04 S.Okajima
	$Id: d_vol_shadow.c,v 1.1.1.3 2002/11/19 11:46:56 Yoshizawa1 Exp $
*/
//asm 複数の関数で同じレジスタの内容を参照 あとで移植
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define	RAISE				(4000)

#define	JOINT_NUM		(22)

//#define	POS_W	(1)
//#define	UVS_W	(1)
///#define	POS_VS_UVS	(POS_W + UVS_W)

//#define	DIV_VERTS		(1)
//#define	DIV_NORMS		(1)
//#define	SCR_DIVISION	( DIV_VERTS + DIV_NORMS )

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

#define	N_VERTS		(64)
#define	N_LINES		(N_VERTS/2)
//関節毎にスクラッチパット使用
#define	N_PRIMS		( JOINT_NUM )

//#define	PARAM1		( (MAX_FVECTOR - N_VERTS*POS_VS_UVS) / SCR_DIVISION )
//#define	PARAM2		(PARAM1 * 16)
//#define	SCR_VERTS	(SCR_UVS   + N_VERTS * 16 * UVS_W)
//#define	SCR_NORMS	(SCR_VERTS + PARAM2 * DIV_VERTS)
#if 0 //BP
//#ifdef PSX2
#define	SCR_VERTS	((void *)SCRPAD_ADDR)
#define	SCR_NORMS	(SCR_VERTS + 0x2000)
#else
#define	SCR_VERTS	(SCRPAD_ADDR)
#define	SCR_NORMS	(SCR_VERTS + 0x2000)
#endif



#define	SCALE		(0.90f)

#define	RAND_WIDTH	(100.0f)

#define	SHIFT_ROT		(256.0f)

#define	NORMS_LIMIT		(0.3f)

/* 消えかける */
#define	DECAY_COUNT		(60)

extern int DM_FrameSkip ;

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	int			add_sub;

	DG_OBJS		*objs;
	FVECTOR		*light_pos;
	float		reach;
	float		back_shift;

	int			n_models;

	DG_PRIM2	*prim ;
	FMATRIX		*world[JOINT_NUM];
	FVECTOR		*verts[JOINT_NUM];
	FVECTOR		*norms[JOINT_NUM];
	int			n_verts[JOINT_NUM];

	int			life;
	int			decay_max;

	CVECTOR		col;
} Work ;

static inline float OK_InnerProduct( FVECTOR *a, FVECTOR *b)
{
#ifdef BP_PSX2_ASM
    float ans;
    asm volatile ("
    lwc1	$f1,0(%1)
    lwc1	$f2,0(%2)
    lwc1	$f3,4(%1)
    lwc1	$f4,4(%2)
    lwc1	$f5,8(%1)
    lwc1	$f6,8(%2)
    mula.s	$f1,$f2
    madda.s	$f3,$f4
    madd.s	%0,$f5,$f6
    " : "=f"(ans) : "r"(a), "r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6");
    return ans;
#else
	return (a->vx*b->vx)+(a->vy*b->vy)+(a->vz*b->vz);
#endif
}

#ifdef BP_PSX2_ASM
static inline void OK_SetMatrix( FMATRIX *m )
{
	static	FVECTOR	scale={ SCALE, SCALE, SCALE, SCALE };
	asm volatile ("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	lqc2		vf8,0x00(%1)
	vmul.xyz	vf4, vf4,vf8
	vmul.xyz	vf5, vf5,vf8
	vmul.xyz	vf6, vf6,vf8
	": : "r"(m), "r"(&scale):"memory" );
}

#endif

#ifdef BP_PSX2_ASM
static inline void OK_RotVec( FVECTOR *res, FVECTOR *v )
{
	asm volatile ("
	lqc2			vf8, 0x00(%1)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf0
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
}
#endif

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR	*pos0;
	DG_PRIM2_UVRGB			*uvrgb  ;	/* ライン用 */
	FVECTOR	body_center;
	FVECTOR	light_back;
	FVECTOR	diff;
	FVECTOR	**verts;
	FVECTOR	**norms;
	FVECTOR	*buff;
	FVECTOR *from;
	FMATRIX	**world;
#ifndef BP_PSX2_ASM
	FVECTOR	_vf12 ,*_vf9;
#endif
	int i, n;
	int	max_num;
	int	clock;
	int	alpha;
	int		*n_verts;
	FVECTOR	direction;
	SVECTOR	rot;
	float		len;
	float		param0;

	if( work->objs==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}


	DG_COPY_VEC( &body_center, (FVECTOR *)work->objs->world.m[3] );

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;


	DG_COPY_VEC( &light_back, work->light_pos );
	diff.vx = work->light_pos->vx - body_center.vx;
	diff.vy = work->light_pos->vy - body_center.vy;
	diff.vz = work->light_pos->vz - body_center.vz;
	len = GV_VecLen3F( &diff );
	if( len!=0.0f ){
		param0 = work->back_shift / len;
		light_back.vx += diff.vx * param0;
		light_back.vy += diff.vy * param0;
		light_back.vz += diff.vz * param0;
	}
//AN_Test_Eye2( work->light_pos, 2 );
//AN_Test_Eye2( &light_back, 2 );

	direction.vx = 0.0f;
	direction.vy = 0.0f;
	direction.vz = -1.0f;
	direction.vw = 1.0f;


	OK_DirVecXY( &body_center, &light_back, &rot );
	DG_SetPos2( &DG_ZeroVector, &rot );
	DG_RotVector( &direction, &direction, 1 );


	pos0     = work->prim->pos[ clock ];
	uvrgb    = work->prim->uvrgb[ clock ];
	n_verts  = work->n_verts;
	world    = work->world;
	verts    = work->verts;
	norms    = work->norms;
	i = work->n_models;

	diff.vx = body_center.vx - light_back.vx;
	diff.vy = body_center.vy - light_back.vy;
	diff.vz = body_center.vz - light_back.vz;
	len = GV_VecLen3F( &diff );
	if( work->reach > len ){
		DG_VisiblePrim2( work->prim );

		alpha  = (int)(work->col.cd * (work->reach - len) / work->reach);

		if( work->life < work->decay_max ){
			alpha = alpha * work->life / work->decay_max;
			if( alpha < 0 ) alpha = 0;
		}

		param0 = (work->reach - len) / len;
		while( --i>=0 ){
			OK_Mem_Scr( (void *)SCR_VERTS, *verts, sizeof(FVECTOR), *n_verts) ;
			OK_Mem_Scr( (void *)SCR_NORMS, *norms, sizeof(FVECTOR), *n_verts) ;

			DG_SetPos( *world );
			DG_RotVector( (void *)SCR_NORMS, (void *)SCR_NORMS, *n_verts );


			from = (void *)SCR_VERTS;
			buff = (void *)SCR_NORMS;
			n    = *n_verts;

			max_num = N_LINES;

#ifdef BP_PSX2_ASM
			OK_SetMatrix( *world );
#else
			DG_SetPos( *world ) ;
#endif
			diff.vw = RAND_WIDTH*0.5f;
			light_back.vw = param0;
//printf("%f %f %f\n",(*world)->m[3][0],(*world)->m[3][1],(*world)->m[3][2]);

#ifdef BP_PSX2_ASM
			asm volatile ("
				lqc2		vf12,0x00(%0)
				"::"r"(&light_back):"memory"
				);
#else
				DG_COPY_VEC( &_vf12, &light_back );
#endif

			while ( -- n >= 0 ) {
				buff->vw = OK_InnerProduct( buff, &direction );
				if( buff->vw < NORMS_LIMIT  &&  buff->vw > -NORMS_LIMIT ){
#ifdef BP_PSX2_ASM
					OK_RotVec( pos0, from );
#else
					DG_PutVector( from, pos0, 1 ) ;
#endif

#if 0
					pos0->vx += rnd()*RAND_WIDTH - RAND_WIDTH*0.5f;
					pos0->vy += rnd()*RAND_WIDTH - RAND_WIDTH*0.5f;
					pos0->vz += rnd()*RAND_WIDTH - RAND_WIDTH*0.5f;
					uvrgb->a = alpha;
					uvrgb++;


					pos1->vx = pos0->vx + ( pos0->vx - light_back.vx ) * param0;
					pos1->vy = pos0->vy + ( pos0->vy - light_back.vy ) * param0;
					pos1->vz = pos0->vz + ( pos0->vz - light_back.vz ) * param0;
					uvrgb->a = 0;

					uvrgb++;

					pos0+=2;
					pos1+=2;
#else
					diff.vx = rnd()*RAND_WIDTH;
					diff.vy = rnd()*RAND_WIDTH;
					diff.vz = rnd()*RAND_WIDTH;
#ifdef BP_PSX2_ASM
					asm volatile ("
						lqc2		vf8 ,0x00(%0)
						lqc2		vf9 ,0x10(%0)
						lqc2		vf10,0x00(%1)

						vadd.xyz	vf8, vf8, vf10
						vsubw.xyz	vf8, vf8, vf10

						vadda.xyz	ACC, vf0, vf8
						vsub.xyz	vf11,vf8, vf12
						vmaddw.xyz	vf9 ,vf11,vf12

						sqc2		vf8 ,0x00(%0)
						sqc2		vf9 ,0x10(%0)
						"::"r"(pos0),"r"(&diff):"memory"
					);
#else
					_vf9 = pos0 ; _vf9++;
					pos0->vx += diff.vx ; 
					pos0->vy += diff.vy ; 
					pos0->vz += diff.vz ; 
					pos0->vx -= diff.vw ; 
					pos0->vy -= diff.vw ; 
					pos0->vz -= diff.vw ; 
					_vf9->vx = pos0->vx + (pos0->vx-_vf12.vx) * _vf12.vw ;	
					_vf9->vy = pos0->vy + (pos0->vy-_vf12.vy) * _vf12.vw ;	
					_vf9->vz = pos0->vz + (pos0->vz-_vf12.vz) * _vf12.vw ;	
#endif
					(uvrgb++)->a = alpha;
					(uvrgb++)->a = 0;
					pos0+=2;
#endif

					if( --max_num <= 0 ) break;
				}
				buff ++ ;
				from ++ ;
			}

			while ( --max_num >= 0 ) {
				pos0+=2;		/* skip させる */
				(uvrgb++)->a = 0;
				(uvrgb++)->a = 0;
			}

			n_verts++;
			world++;
			verts++;
			norms++;
		}
	}else{
		DG_InvisiblePrim2( work->prim );
	}


	/* 初期値が -1 なら 自殺しない */
	if( work->life >= 0 ){
		work->life -= DM_FrameSkip ;
		work->life--;
		if( work->life < 0 ){
			GV_DestroyActor( work ) ;
		}
	}
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos0 ;
	FVECTOR				*pos1 ;
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		j, k ;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	if( work->add_sub==0 ){
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}else{
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}

	//-------------------------------
	pos0 = (FVECTOR *)prim->pos[ 0 ] ;
	pos1 = (FVECTOR *)prim->pos[ 1 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			pos0->vx = 0.0f;
			pos0->vy = 0.0f;
			pos0->vz = 0.0f;
			pos0++ ;
			pos1->vx = 0.0f;
			pos1->vy = 0.0f;
			pos1->vz = 0.0f;
			pos1++ ;
		}
	}


	//-------------------------------
	uvrgb0 = prim->uvrgb[ 0 ] ;
	uvrgb1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			if ( k & 1 ){
				uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
				uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
				uvrgb1->f = uvrgb0->f = 0x0fff ;
				uvrgb1->q = uvrgb0->q = 4096 ;
				uvrgb1->r = uvrgb0->r = work->col.r ;
				uvrgb1->g = uvrgb0->g = work->col.g ;
				uvrgb1->b = uvrgb0->b = work->col.b ;
				uvrgb1->a = uvrgb0->a = 0 ;
			} else {
				uvrgb1->u = uvrgb0->u = FTOI12( 0.5F * tex->u_scale + tex->u_offset ) ;
				uvrgb1->v = uvrgb0->v = FTOI12( 0.5F * tex->v_scale + tex->v_offset ) ;
				uvrgb1->f = uvrgb0->f = 0x8fff ;
				uvrgb1->q = uvrgb0->q = 4096 ;
				uvrgb1->r = uvrgb0->r = work->col.r ;
				uvrgb1->g = uvrgb0->g = work->col.g ;
				uvrgb1->b = uvrgb0->b = work->col.b ;
				uvrgb1->a = uvrgb0->a = 0 ;
			}
			uvrgb0 ++ ;
			uvrgb1 ++ ;
		}
	}

}

static int GetResources( Work *work, DG_OBJS *objs, FVECTOR *light_pos, float reach, float back_shift, int model_id, int add_sub, int life, int col )
{
	CV2_DEF		*cvd_def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int		i;
//	int		j ;
//	FVECTOR		*fvtemp0;
//	FVECTOR		*fvtemp1;

	life = DIRECT_TICK( life ) ;
	if( life <= 0 ) life = 1;

	work->col.r  = ((col>> 0)&255);
	work->col.g  = ((col>> 8)&255);
	work->col.b  = ((col>>16)&255);
	work->col.cd = ((col>>24)&255);

	work->objs=objs;
	work->light_pos = light_pos;
	work->back_shift = back_shift;
	work->reach = reach + back_shift;
	work->add_sub = add_sub;

	work->life = life;
	if( work->life < DECAY_COUNT ){
		if( work->life < 1 ){
			work->decay_max = 1;
		}else{
			work->decay_max = work->life;
		}
	}else{
		work->decay_max = DECAY_COUNT;
	}



	work->n_models = work->objs->n_models < JOINT_NUM ? work->objs->n_models : JOINT_NUM ; 

	prim = work->prim = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

//	tex = DG_GetTexture( 762348 /*"light05_msk"*/ );
//	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );


	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );








	InitPacket2( work, prim, tex );
	DG_VisiblePrim2( work->prim );

	/* モデルの共有頂点データ取得 */
	cvd_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );
	if( cvd_def==NULL ) return -1;

#if 1
	i = work->n_models ; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
		if( work->n_verts[i] > MAX_FVECTOR/2 ){
//			printf("%d::%d\n",work->n_verts[i],MAX_FVECTOR/2 );
			work->n_verts[i] = MAX_FVECTOR/2;
		}
		work->verts[i]   = cvd_def->models[i].verts;
		work->norms[i]   = cvd_def->models[i].norms;
		work->world[i]   = &objs->objs[i].world;
	}
#else
	i = work->n_models ; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
		if( work->n_verts[i] > MAX_FVECTOR/2 ){
//			printf("%d::%d\n",work->n_verts[i],MAX_FVECTOR/2 );
			work->n_verts[i] = MAX_FVECTOR/2;
		}
		work->world[i]   = &work->objs->objs[i].world;
		fvtemp0 = work->verts[i]   = cvd_def->models[i].verts;
		fvtemp1 = work->norms[i]   = cvd_def->models[i].norms;
		for( j=0; j<work->n_verts[i]; j++ ){
			if( _sceVu0InnerProduct( fvtemp0, fvtemp1 ) < 0.0f ){
				_sceVu0ScaleVector( fvtemp1, fvtemp1, -1.0f );
			}
			fvtemp0++;
			fvtemp1++;
		}
	}
#endif

	return (0);
}
/* ---------------------------------------------------------------- */
void *NewBodyShadowVolume_Demo(
		DG_OBJS *objs,		/* [ポインタ保存参照]対象オブジェ */
		FVECTOR *light_pos,	/* [ポインタ保存参照]影をもたらす光源点座標 */
		float reach,		/* 光源から光が（影が）到達する距離 */
		float back_shift,	/* 角度調節のために光源位置を仮想的に後ろへシフトさせる距離 */
		int model_id,		/* CV2モデル（現在はsna_skl3.cv2:3075579） */
		int add_sub,		/* 加算減算フラグ：０）減算 １）加算 */
		int life,			/* 寿命。フレーム指定 */
		int col
		 )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, objs, light_pos, reach, back_shift, model_id, add_sub, life, col ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

