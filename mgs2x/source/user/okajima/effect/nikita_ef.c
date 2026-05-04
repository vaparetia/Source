//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	nikita_ef.c
	ニキータミサイルエフェクト：後部ガス＆前部ライトボンボリ
	2000/06/06 S.Okajima
	$Id: nikita_ef.c,v 1.1.1.3 2002/11/19 11:47:08 Yoshizawa1 Exp $
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
#define	RAISE					(0)
#define	RAISE_SHIFT_WORLD		(20.0f)

#define	SHIFT_FRONT		(140.0f)
#define	SHIFT_BACK		(300.0f)

#define	ANGLE_WIDTH_LIMIT	(64)
#define	ANGLE_ADD		(1)

#define	DUMMY_SIZE		(0)
#define	DUMMY_COLOR		(32)
#define	DUMMY_ALPHA		(0)


/* 前部カメラレンズ */
#define	N_PRIMS_FRONT	(1)
#define	N_VERTS_FRONT	(2)
#define	N_VP_FRONT		(N_PRIMS_FRONT * N_VERTS_FRONT)

/* ジェット */
#define	N_PRIMS			(1)
#define	N_VERTS			(16)
#define	N_VP			(N_PRIMS * N_VERTS)


/* 中心から円錐ライトの最遠中心まで */
#define	LIGHT_LEN		(-300.0f)

#define	LIMIT_ANGLE			(0.0f)
#define	COL_R				(32)
#define	COL_G				(96)
#define	COL_B				(128)
#define	ALPHA_BASE_FRONT	(128.0f)
#define	ALPHA_BASE			(128)
#define	ALPHA_MIN			(8)

#define	BAR_NUM			(4)

#define	SIZE_BASE		(80)
#define	SIZE_STEP		(-5)

typedef	struct	{
	GV_ACT_EX	actor ;

	CONTROL		*control;

	int			phase;

	DG_PRIM2	*prim_front ;	/* 前部ボンボリ */
	DG_PRIM2	*prim_back ;	/* 後部ジェット */

} Work ;


static FVECTOR s_pos_front[N_VP_FRONT];
static FVECTOR s_pos[N_VP];

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	j;
	FVECTOR	fvtemp0;
	FVECTOR	fvtemp1;
	SVECTOR	svtemp;
	FVECTOR	cam_pos;
	float	inner;
	int	alpha;
	int	clock;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	DG_VisiblePrim2( work->prim_front ) ;
	DG_VisiblePrim2( work->prim_back ) ;
	GM_GroupPrim2( work->prim_front, GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim_back, GM_CurrentStageMap ) ;

	DG_SwitchBuffPrim2( work->prim_back );
	DG_SwitchBuffPrim2( work->prim_front );
	clock = work->prim_back->buffer_clock;



	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );

	/* 光線の方向ベクトルを生成 */
	fvtemp0.vx = 0.0f;
	fvtemp0.vy = 0.0f;
	fvtemp0.vz =-1.0f;
	DG_SetPos2( &DG_ZeroVector, &work->control->rot );
	DG_RotVector( &fvtemp0, &fvtemp0, 1 );


	//------------------------------------------------------front
	/* カメラからライトへのベクトルと、ライトのベクトルの内積 */
	_sceVu0SubVector( &fvtemp1, &work->control->mov, &cam_pos ) ;
	_sceVu0Normalize( &fvtemp1, &fvtemp1 );
	inner = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 );
	if( inner < LIMIT_ANGLE ) inner = LIMIT_ANGLE;

	/* 懐中電灯の光線の方向ベクトルを */
	/* カメラ行列と乗算（乗算結果のＺ成分がライト方向のカメラ法線への成分となる） */
	DG_SetPos( &DG_Chanls->eye_inv );
	DG_RotVector( &fvtemp0, &fvtemp0, 1 );
	if( fvtemp0.vz < LIMIT_ANGLE ) fvtemp0.vz = LIMIT_ANGLE;

	inner *= fvtemp0.vz;
	inner = (inner - LIMIT_ANGLE) / (1.0f - LIMIT_ANGLE);	/* 比に変換 */
	alpha = (int)(ALPHA_BASE_FRONT * inner);
	if( alpha < 0 ) alpha = 0;

	/* カメラ方向にシフトさせて優先問題を回避 */
	fvtemp0.vx=0.0f;
	fvtemp0.vy=0.0f;
	fvtemp0.vz=RAISE_SHIFT_WORLD;
	OK_DirVecXY( &work->control->mov, &cam_pos, &svtemp );
	DG_SetPos2( &work->control->mov, &svtemp );
	DG_PutVector( &fvtemp0, &fvtemp0, 1 );

//AN_Test_Eye2( &fvtemp0, 2 );
//printf("alpha:%d\n",alpha);

	DG_SetPos2( &fvtemp0, &work->control->rot );
	DG_PutVector( s_pos_front, work->prim_front->pos[clock], N_VP_FRONT );
	uvrgbwh = work->prim_front->uvrgb[clock];
	for( j=0; j<N_VP_FRONT; j++ ){
		uvrgbwh->w = uvrgbwh->h = j*20 + 20;
		uvrgbwh->a = alpha * (N_VP_FRONT-j) / N_VP_FRONT ;
		uvrgbwh++;
	}


	//------------------------------------------------------back
	DG_SetPos2( &work->control->mov, &work->control->rot );
	DG_PutVector( s_pos, work->prim_back->pos[clock], N_VP );
	uvrgbwh = work->prim_back->uvrgb[clock];
	for( j=0; j<N_VP; j++ ){
		uvrgbwh->w = uvrgbwh->h = (N_VP-j)*SIZE_STEP + SIZE_BASE;
		uvrgbwh->a = ALPHA_BASE * j / N_VP + ALPHA_MIN;
		uvrgbwh++;
	}



	DG_SetTmpLight2( &work->control->mov, 500.0f, 1000.0f, COL_R | COL_G<<8 | COL_B<<16, LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;


}

static void Die( Work *work )
{
	work->prim_front = OK_FreePrim2( work->prim_front );
	work->prim_back = OK_FreePrim2( work->prim_back );
}

/* ---------------------------------------------------------------- */
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int n_prims, int n_verts )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise=RAISE;

	//-------------------------------
	for ( i=0; i<2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		for ( j=0; j<n_prims; j++ ){
			for ( k=0; k<n_verts; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h = uvrgbwh->w = DUMMY_SIZE ;
				uvrgbwh->r = COL_R;
				uvrgbwh->g = COL_G;
				uvrgbwh->b = COL_B;
				uvrgbwh->a = DUMMY_ALPHA ;
				uvrgbwh ++ ;
			}
		}
	}
}

static int GetResources( Work *work )
{
	int	i;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	//-----------------------------
//	tex = DG_GetTexture( 12296685 /*"light10_msk"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim_front = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_FRONT, N_VERTS_FRONT );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, N_PRIMS_FRONT, N_VERTS_FRONT );

	for ( i = 0 ; i < N_VP_FRONT ; i++ ){
		s_pos_front[i].vx=0.0f;
		s_pos_front[i].vy=0.0f;
		s_pos_front[i].vz=SHIFT_FRONT;
	}


	//-----------------------------
//	tex = DG_GetTexture( 12296685 /*"light10_msk"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim_back = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS );

	for ( i = 0 ; i < N_VP ; i++ ){
		s_pos[i].vx=0.0f;
		s_pos[i].vy=0.0f;
		s_pos[i].vz=(float)(N_VP-i) * LIGHT_LEN / (float)(N_VP-1) - SHIFT_BACK;
//printf("%f\n",s_pos[i].vz);
	}

	return 0 ;
}

/*
制御はアクター親子関係のシグナル発信で
*/
void *NewNikitaEffect( CONTROL *control )
{
	Work		*work ;

	if( control == NULL ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;

		work->control = control;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
