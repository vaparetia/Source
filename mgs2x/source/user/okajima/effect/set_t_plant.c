//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	set_t_plant.c
	カメラに平行にテクスチャセット（クロスフェード）
	2001/08/13 S.Okajima
	$Id: set_t_plant.c,v 1.1.1.3 2002/11/19 11:47:13 Yoshizawa1 Exp $
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

#define N_PRIMS2	(1)
#define N_VERTS2	(32)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x1000)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int		name;
	int		where;
	int		invisible_flag;
	int		verts_num;
	int		count;
	int		count_max;

	float		alpha_max;

	DG_PRIM2	*prim ;
} Work ;

enum {
	REQ_OFF=0,
	REQ_ON,
};
/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, int col, int size )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;
	int		col_r;
	int		col_g;
	int		col_b;

	col_r = ((col>>24)&255);
	col_g = ((col>>16)&255);
	col_b = ((col>> 8)&255);

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos     = SCR_POS ;
	uvrgbwh = SCR_UVS ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < work->verts_num ; k++ ){
			DG_COPY_VEC( pos, center );

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = uvrgbwh->h = size;

			uvrgbwh->r = col_r ;
			uvrgbwh->g = col_g ;
			uvrgbwh->b = col_b ;
			uvrgbwh->a = 0 ;

			center++;
			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh ++ ;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS,  sizeof(FVECTOR),          work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS,  sizeof(FVECTOR),          work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), work->verts_num * N_PRIMS2 );

	return 1;
}

/*----------------------------------------------------------------*/
static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
			work->invisible_flag = 1;
			break;
		  case REQ_ON:
			work->invisible_flag = 0;
			break;
		  default:
			break;
		}
		msg--;
	}
}


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i, clock, num_verts;
	int	col_a;
	DG_PRIM2_UVRGBWH	*uvrgbwh		;	/* スプライト用 */

	CheckMesgParam( work );

	if( work->invisible_flag ){
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim ) ;
	}

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;
	num_verts = work->verts_num * N_PRIMS2;

	work->count--;
	if( work->count < 0 ) work->count = work->count_max;

	if( work->count_max > 0 ){
		col_a = work->alpha_max * (float)work->count / (float)work->count_max;
	}else{
		col_a = work->alpha_max;
	}

	uvrgbwh      = work->prim->uvrgb[clock];
	for(i=0; i<work->verts_num; i++){
		uvrgbwh->a     = col_a;
		uvrgbwh++;
	}


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int GetResources( Work *work, int name, int where )
{
	int	i;
	int	tex_name;
	int	size=0;
	int	col=0;
	SVECTOR		rot;
	FVECTOR		pos;
	FVECTOR		center[N_PRIMS2 * N_VERTS2];
	DG_PRIM2	*prim ;
	DG_TEX		*tex=NULL ;

	work->name = name;
	work->where = where;

	work->invisible_flag = 0;

	if ( GCL_GetOption( 't' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			tex_name       = GCL_GetNextInt();
			tex = DG_GetTexture( tex_name );
		}
	}

	if ( GCL_GetOption( 's' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			size = GCL_GetNextInt() ;
		}
	}

	work->alpha_max = 255.0f;
	if ( GCL_GetOption( 'c' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			col     = ((GCL_GetNextInt())<<24) ;
			col    |= ((GCL_GetNextInt())<<16) ;
			col    |= ((GCL_GetNextInt())<< 8) ;
			work->alpha_max = (float)GCL_GetNextInt();
		}
	}

	work->verts_num=0;
	if ( GCL_GetOption( 'p' ) != NULL ) {
		for(i=0; i<N_VERTS2; i++){
			if( GCL_NextStr() != NULL ){
				center[i].vx   = (float)GCL_GetNextInt() ;
				center[i].vy   = (float)GCL_GetNextInt() ;
				center[i].vz   = (float)GCL_GetNextInt() ;
			}else{
				break;
			}
		}
		work->verts_num=i;
	}

	if ( GCL_GetOption( 'u' ) != NULL ){
		pos.vx = (float)GCL_GetNextInt();
		pos.vy = (float)GCL_GetNextInt();
		pos.vz = (float)GCL_GetNextInt();
	}else{
		pos.vx = 0.0f;
		pos.vy = 0.0f;
		pos.vz = 0.0f;
	}

	if ( GCL_GetOption( 'r' ) != NULL ){
		rot.vx = 0;
		rot.vy = GCL_GetNextInt();
		rot.vz = 0;
	}else{
		rot.vx = 0;
		rot.vy = 0;
		rot.vz = 0;
	}

	DG_SetPos2( &pos, &rot );
	DG_PutVector( center, center, work->verts_num );


	work->count = 0;
	work->count_max = 0;
	if ( GCL_GetOption( 'f' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			work->count_max = DIRECT_TICK( GCL_GetNextInt() );
//			if( work->count_max < 1 ) work->count_max = 1;
		}
	}

	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS2,
				work->verts_num );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex, center, col, size );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewPutTexturePlant( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where  ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
