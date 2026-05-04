//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	stg_fall_parts.c
	スティンガーブースター落下
	2000/07/07 S.Okajima
	$Id: stg_fall_parts.c,v 1.1.1.3 2002/11/19 11:50:37 Yoshizawa1 Exp $

*/
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"
#include	"gameheader.h"
#include	"libmt.h"

#define		FALL_NAME	(0x2c0f86)	/* (GV_StrCode( "stg_msl2" )) */
#define		FALL_TIME	( 60*60*1 ) 
#define		LIFE		( 120 ) 



#define	RAISE					(0)
#define	RAISE_SHIFT_WORLD		(20.0f)

#define	SHIFT_BACK		(30.0f)

#define	DUMMY_SIZE		(0)
#define	DUMMY_COLOR		(32)
#define	DUMMY_ALPHA		(0)

#define	N_PRIMS			(1)
#define	N_VERTS			(32)
#define	N_VP			(N_PRIMS * N_VERTS)

static FVECTOR s_pos[N_VP];

/* 中心から円錐ライトの最遠中心まで */
#define	LIGHT_LEN		(100.0f)

#define	COL_R				(16)
#define	COL_G				(48)
#define	COL_B				(64)
#define	ALPHA_BASE			(16.0f)
#define	ALPHA_MIN			(2)
#define	EXTIN_ALPHA			(0.25f)

#define	SIZE_BASE		(30)
#define	SIZE_STEP		(3)

typedef	struct	{
	GV_ACT_EX		actor ;

	DG_OBJS		*objs ;
	FMATRIX		lights[ 2 ] ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	FVECTOR		vec ;
	int			life;
	int			count;

	DG_PRIM2	*prim;
	float		alpha;

} Work ;


static	void Act( Work *work )
{
	int	j;
	int	alpha;
	int	clock;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	if( work->life!=-1 ){
		if( --work->life <= 0 ){
			GV_DestroyActor( work ) ;
			return;
		}
	}
	GM_GroupObjs( work->objs, GM_CurrentStageMap );
	DG_GetLightMatrix( &work->pos, work->lights );

	work->rot.vx += 32;
	work->rot.vy += 16;
	work->rot.vz += 0;
	work->vec.vy += P_GRAVITY;
	_sceVu0ScaleVector( &work->vec, &work->vec, 0.99f );
	_sceVu0AddVector( &work->pos, &work->pos, &work->vec ) ;

	DG_SetPos2( &work->pos, &work->rot );
	DG_PutObjs( work->objs );


//-------------------------------
	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;
	DG_SetPos2( &work->pos, &work->rot );
	DG_PutVector( s_pos, work->prim->pos[clock], N_VP );

	uvrgbwh = work->prim->uvrgb[clock];
	work->alpha -= EXTIN_ALPHA;
	work->alpha  = (work->alpha > 0.0f)?work->alpha: 0.0f;
	alpha = (int)( (work->alpha * rnd() + work->alpha) * 0.5f );
	if( work->count < 8 ){
		for( j=0; j<N_VP; j++ ){
			uvrgbwh->w = uvrgbwh->h = (j*SIZE_STEP*(8-work->count) + SIZE_BASE);
			uvrgbwh->a = alpha * j / N_VP + ALPHA_MIN;
			uvrgbwh++;
		}
	}else{
		for( j=0; j<N_VP; j++ ){
			uvrgbwh->w = uvrgbwh->h = j*SIZE_STEP + SIZE_BASE;
			uvrgbwh->a = alpha * j / N_VP + ALPHA_MIN;
			uvrgbwh++;
		}
	}

	if( work->count == 0 ){
		extern void *NewConnectSmoke( FMATRIX *world, FVECTOR *shift, int life, float size_rnd );
		FVECTOR	shift;
		shift.vx = 0.0f;
		shift.vy = SHIFT_BACK;
		shift.vz = 0.0f;
		GV_SetActorChild( work, NewConnectSmoke( &work->objs->world, &shift, 30, 64.0f ) );

		{
			extern void *NewSTG_Spark( FMATRIX *hand, float vec_len );
			NewSTG_Spark( &work->objs->world, 10.0f );
		}
	}


	work->count++;


}

static	void Die( Work *work )
{
	if( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
	work->prim = OK_FreePrim2( work->prim );
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

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
}


static FVECTOR ShiftCenter={ 0.0f, 630.0f, 0.0f, 0.0f };
static	int GetResources( Work *work, FVECTOR *pos, SVECTOR *rot, FVECTOR *vec )
{
	int	i;
	DG_DEF		*def ;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	//-----------------------------
	work->rot.vx = rot->vx + irnd()%16;
	work->rot.vy = rot->vy + irnd()%16;
	work->rot.vz = rot->vz;
	DG_SetPos2( pos, rot );
	DG_PutVector( &ShiftCenter, &work->pos, 1 );
	_sceVu0ScaleVector( &work->vec, vec, 0.8f );
//	DG_COPY_VEC( &work->vec, vec );
	work->life = LIFE;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( FALL_NAME, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_FINISHCALC | DG_FLAG_ONEPIECE, 0 );
	if( work->objs == NULL ) return -1;
	DG_QueueObjs( work->objs );

#if 0
	/* ０番以外は不可視 */
	for(i=1; i<work->objs->n_models; i++){
		work->objs->objs[i].flag |= DG_FLAG_INVISIBLE ;
	}
#endif
	DG_GetLightMatrix( &work->pos, work->lights );
	DG_SetLightMatrix( work->objs, work->lights );

	//-----------------------------
	tex = DG_GetTexture( GV_StrCode( "chi01_msk" ) );
//	tex = DG_GetTexture( GV_StrCode( "chi01_alp" ) );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS );

	for ( i = 0 ; i < N_VP ; i++ ){
		s_pos[i].vx=0.0f;
		s_pos[i].vy=(float)i * LIGHT_LEN / (float)(N_VP-1) + SHIFT_BACK;
		s_pos[i].vz=0.0f;
	}

	work->alpha = ALPHA_BASE;
	work->count = 0;

	return 0 ;
}

void *NewStingerObjectFall( FVECTOR *pos, SVECTOR *rot, FVECTOR *vec )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, rot, vec ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
