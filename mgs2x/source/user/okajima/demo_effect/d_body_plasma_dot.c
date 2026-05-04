//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	d_body_plasma_dot.c
	体を這うプラズマ（ドット）
	2000/03/23 S.Okajima
	$Id: d_body_plasma_dot.c,v 1.1.1.3 2002/11/19 11:46:51 Yoshizawa1 Exp $
*/
//asm 複数関数でレジスタに保持した値を使ってる　移植後回し
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

#include	"gameheader.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"../../system/libutl/rand.h"
#include	"../etc/ok_util.h"

#define	JOINT_NUM		(22)

#define	POS_VS_UVS	(1 + 2)

#define	DIV_VERTS		(1)
#define	DIV_NORMS		(1)
#define	SCR_DIVISION	( DIV_VERTS + DIV_NORMS )

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
//#define	N_VERTS2		(16)
#define	N_VERTS2		(32)
//関節毎にスクラッチパット使用
#define	N_PRIMS2		( JOINT_NUM )

#define	PARAM1		( (MAX_FVECTOR - N_VERTS2*POS_VS_UVS) / SCR_DIVISION )
#define	PARAM2		(PARAM1 * 16)

#if 0 //BP
//#ifdef PSX2
#define	SCR_POS		((void *)SCRPAD_ADDR)
#define	SCR_UVS		(SCR_POS   + N_VERTS2 * 16)
#define	SCR_VERTS	(SCR_UVS   + N_VERTS2 * 16 * 2)
#define	SCR_NORMS	(SCR_VERTS + PARAM2 * DIV_VERTS)
#else
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCR_POS   + N_VERTS2 * 16)
#define	SCR_VERTS	(SCR_UVS   + N_VERTS2 * 16 * 2)
#define	SCR_NORMS	(SCR_VERTS + PARAM2 * DIV_VERTS)
#endif


#define	SCALE		(1.10f)

#define	P_RGB_MAX		(255)
#define	P_ALPHA_MAX		(64)

#define	SIZE		(30.0f)
#define	RAND_WIDTH	(40.0f)

#define	SHIFT_ROT		(256.0f)

#define	NORMS_LIMIT_UPPER		(0.2f)
#define	NORMS_LIMIT_UNDER		(0.1f)

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_OBJS		*objs;
	SVECTOR		rot;

	int			n_models;

	DG_PRIM2	*prim ;
	FMATRIX		*world[JOINT_NUM];
	FVECTOR		*verts[JOINT_NUM];
	FVECTOR		*norms[JOINT_NUM];
	int			n_verts[JOINT_NUM];

	float		rad;

	int			life;

} Work ;

static inline float OK_InnerProduct( FVECTOR *a, FVECTOR *b)
{
#ifdef BP_PSX2_ASM
    float ans;
    asm volatile ("
    lwc1	$f1,0(%1)	#a x
    lwc1	$f2,0(%2)	#b x
    lwc1	$f3,4(%1)	#a y
    lwc1	$f4,4(%2)	#b y
    lwc1	$f5,8(%1)	#a z
    lwc1	$f6,8(%2)	#b z
    mula.s	$f1,$f2		#acc = a x * b x
    madda.s	$f3,$f4		#acc += a y * b y
    madd.s	%0,$f5,$f6	#ans = acc + az *bz 
    " : "=f"(ans) : "r"(a), "r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6");
    return ans;
#else
    return (a->vx*b->vx)+(a->vy*b->vy)+(a->vz*b->vz);
#endif
}

#if 0 //BP_PS2

#ifdef PSX2
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
static inline void OK_RotVec( FVECTOR *res, FVECTOR *v )
{
#ifdef PSX2
	asm volatile ("
	lqc2			vf8, 0x00(%1)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf0
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
#endif
}

#endif

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR	*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	float	norm_limit;
	FVECTOR	**verts;
	FVECTOR	**norms;
	FVECTOR	*buff;
	FVECTOR *from;
	FMATRIX	**world;
#ifndef BP_PSX2_ASM
	FVECTOR	scale={ SCALE, SCALE, SCALE, SCALE };
	FMATRIX world2 ; 
#endif

	int i, n;
	int	max_num;
	int	clock;
	int		*n_verts;
	FVECTOR	direction;

	if( work->objs==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	direction.vx = 0.0f;
	direction.vy = 0.0f;
	direction.vz = -1.0f;
	direction.vw = 1.0f;

	work->rot.vx += 30;
	work->rot.vy += 42;
	work->rot.vz += 55;

	work->rad += TPI*0.005f;
	norm_limit = sinf( work->rad );

	if( norm_limit > NORMS_LIMIT_UPPER ) norm_limit = NORMS_LIMIT_UPPER;
	if( norm_limit < NORMS_LIMIT_UNDER ) norm_limit = NORMS_LIMIT_UNDER;

	DG_SetPos2( &DG_ZeroVector, &work->rot );
	DG_RotVector( &direction, &direction, 1 );


	pos      = work->prim->pos[ clock ];
	uvrgbwh  = work->prim->uvrgb[ clock ];
	n_verts  = work->n_verts;
	world    = work->world;
	verts    = work->verts;
	norms    = work->norms;
	i = work->n_models;

	while( --i>=0 ){
		OK_Mem_Scr((void *) SCR_VERTS, *verts,  sizeof(FVECTOR), *n_verts) ;
		OK_Mem_Scr((void *) SCR_NORMS, *norms,  sizeof(FVECTOR), *n_verts) ;

		DG_SetPos( *world );
		DG_RotVector((void *) SCR_NORMS, (void *)SCR_NORMS, *n_verts );

		from = (void *)SCR_VERTS;
		buff =(void *) SCR_NORMS;
		n    = *n_verts;

		max_num = N_VERTS2;

#ifdef BP_PSX2_ASM
		OK_SetMatrix( *world );
#else
//	lqc2		vf4,0x00(%0) world2
//	lqc2		vf5,0x10(%0) world2
//	lqc2		vf6,0x20(%0) world2
//	lqc2		vf7,0x30(%0) world2
//	lqc2		vf8,0x00(%1) scale
//	vmul.xyz	vf4, vf4,vf8
//	vmul.xyz	vf5, vf5,vf8
//	vmul.xyz	vf6, vf6,vf8
//	": : "r"(m), "r"(&scale):"memory" );
	world2.m[0][0] = ((*world)->m[0][0])*scale.vx ;
	world2.m[0][1] = ((*world)->m[0][1])*scale.vy ;
	world2.m[0][2] = ((*world)->m[0][2])*scale.vz ;
	world2.m[0][3] = ((*world)->m[0][3]);

	world2.m[1][0] = ((*world)->m[1][0])*scale.vx ;
	world2.m[1][1] = ((*world)->m[1][1])*scale.vy ;
	world2.m[1][2] = ((*world)->m[1][2])*scale.vz ;
	world2.m[1][3] = ((*world)->m[1][3]);

	world2.m[2][0] = ((*world)->m[2][0])*scale.vx ;
	world2.m[2][1] = ((*world)->m[2][1])*scale.vy ;
	world2.m[2][2] = ((*world)->m[2][2])*scale.vz ;
	world2.m[2][3] = ((*world)->m[2][3]);

	world2.m[3][0] = ((*world)->m[3][0]);
	world2.m[3][1] = ((*world)->m[3][1]);
	world2.m[3][2] = ((*world)->m[3][2]);
	world2.m[3][3] = ((*world)->m[3][3]);

#endif
		while ( -- n >= 0 ) {
			buff->vw = OK_InnerProduct( buff, &direction );
			if( buff->vw < norm_limit  &&  buff->vw > -norm_limit ){
				/* スプライト */
#ifdef BP_PSX2_ASM
				OK_RotVec( pos, from );
#else
//	asm volatile ("
//	lqc2			vf8, 0x00(%1) v
//	vmulax.xyzw		ACC, vf4,vf8 //acc=
//	vmadday.xyzw	ACC, vf5,vf8 //acc+=
//	vmaddaz.xyzw	ACC, vf6,vf8 //acc+=
//	vmaddw.xyzw		vf8, vf7,vf0 //res = 7*0+acc
//	sqc2			vf8,0x00(%0)
//	":: "r"(res),"r"(v):"memory" );
	
	pos->vx = (world2.m[0][0] * pos->vx)+(world2.m[1][0]*pos->vy) + (world2.m[2][0]*pos->vz) ;
	pos->vy = (world2.m[0][1] * pos->vx)+(world2.m[1][1]*pos->vy) + (world2.m[2][1]*pos->vz) ;
	pos->vz = (world2.m[0][2] * pos->vx)+(world2.m[1][2]*pos->vy) + (world2.m[2][2]*pos->vz) ;
	pos->vw = (world2.m[0][3] * pos->vx)+(world2.m[1][3]*pos->vy) + (world2.m[2][3]*pos->vz) ;

#endif
				pos->vx += rnd()*RAND_WIDTH - RAND_WIDTH*0.5f;
				pos->vy += rnd()*RAND_WIDTH - RAND_WIDTH*0.5f;
				pos->vz += rnd()*RAND_WIDTH - RAND_WIDTH*0.5f;
				pos->vw  = 1.0f;	/* 有効座標 */
				uvrgbwh->a = P_ALPHA_MAX;
				uvrgbwh++;
				pos++;
				if( --max_num <= 0 ) break;
			}
			buff ++ ;
			from ++ ;
		}

		while ( --max_num >= 0 ) {
			uvrgbwh->a = 0;
			uvrgbwh++;
			pos->vw  = 0.0f;		/* 無効座標 */
			pos++;		/* skip させる */
		}

		n_verts++;
		world++;
		verts++;
		norms++;
	}

	if( work->life > 0 ){
		work->life--;
	}else{
		GV_DestroyActor( work ) ;
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
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	pos  = (FVECTOR *)SCRPAD_ADDR ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			pos->vx = 0.0f;
			pos->vy = 0.0f;
			pos->vz = 0.0f;
			pos ++ ;		/* 同一プリミティブではデータは連続している */
		}
	}
	OK_Scr_Mem( prim->pos[ 0 ], SCRPAD_ADDR, sizeof(FVECTOR), N_VERTS2 * work->n_models ) ;
	OK_Scr_Mem( prim->pos[ 1 ], SCRPAD_ADDR, sizeof(FVECTOR), N_VERTS2 * work->n_models ) ;


	//-------------------------------
	uvrgbwh = prim->uvrgb[ 0 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;
			uvrgbwh->w = (int)( SIZE );
			uvrgbwh->h = (int)( SIZE );

			uvrgbwh->r = P_RGB_MAX ;
			uvrgbwh->g = P_RGB_MAX ;
			uvrgbwh->b = P_RGB_MAX ;
			uvrgbwh->a = P_ALPHA_MAX ;

			uvrgbwh ++ ;
		}
	}

	uvrgbwh = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < work->n_models ; j++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = (int)( SIZE );
			uvrgbwh->h = (int)( SIZE );

			uvrgbwh->r = P_RGB_MAX ;
			uvrgbwh->g = P_RGB_MAX ;
			uvrgbwh->b = P_RGB_MAX ;
			uvrgbwh->a = P_ALPHA_MAX ;

			uvrgbwh ++ ;
		}
	}
}

static int GetResources( Work *work, DG_OBJS *org_objs, int model_id, int life )
{
	CV2_DEF		*cvd_def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int		i;
//	int		j;
//	FVECTOR		*fvtemp0;
//	FVECTOR		*fvtemp1;

	work->objs=org_objs;
	work->life = life;

	work->n_models = work->objs->n_models < JOINT_NUM ? work->objs->n_models : JOINT_NUM ; 

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_models, N_VERTS2 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	tex = DG_GetTexture( 762348 /*"light05_msk"*/ );
	InitPacket2( work, prim, tex );
	DG_VisiblePrim2( work->prim );

	/* モデルの共有頂点データ取得 */
	cvd_def = GV_GetCache( GV_CacheID( model_id, 'c' ) );


//printf("org_objs->n_models:%d\n",org_objs->n_models);

#if 1
	i = work->n_models; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
//printf("work->n_verts:%d\n",work->n_verts[i]);
		if( work->n_verts[i] > PARAM1 ){
//			printf("%d::%d\n",work->n_verts[i],PARAM1 );
			work->n_verts[i] = PARAM1;
		}
		work->verts[i]   = cvd_def->models[i].verts;
		work->norms[i]   = cvd_def->models[i].norms;
		work->world[i]   = &work->objs->objs[i].world;
	}
#else
	i = work->n_models ; 
	while( --i>=0 ){
		work->n_verts[i] = cvd_def->models[i].n_verts;
		if( work->n_verts[i] > PARAM1 ){
			work->n_verts[i] = PARAM1;
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

	work->rot.vx = irnd()%4096;
	work->rot.vy = irnd()%4096;
	work->rot.vz = irnd()%4096;

	work->rad = TPI*rnd();

	return (0);
}
/* ---------------------------------------------------------------- */
/*
*objs   :[ポインタ参照]対象オブジェ
model_id:CV2モデル（現在はsna_skl3.cv2:3075579）
life    :寿命。フレーム指定
*/
void *NewBodyPlasmaDot_Demo( DG_OBJS *objs, int model_id, int life )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, objs, model_id, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

