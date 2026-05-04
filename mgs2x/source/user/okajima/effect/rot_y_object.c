//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	rot_y_object.c
	くるくるレーダー
	2000/06/01 S.Okajima
	$Id: rot_y_object.c,v 1.1.1.3 2002/11/19 11:47:11 Yoshizawa1 Exp $

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

extern int ok_flush_status;

#define	N_VERTS		(32)

#define	P_RGB_MAX		(128)
#define	SIZE			(100)

#define	DEFAULT_LENGTH			(10000)
#define	DEFAULT_STABLE_LENGTH	(3000)

/* ---------------------------------------------------------------- */
typedef	struct	{
	int			n_prim;
	float		width;
	float		height;
	int			alpha;
	FVECTOR		line_data[2];
	FVECTOR		line_data_rotate[2];
	FVECTOR		line_width;
	DG_PRIM2	*prim ;
} Unit ;

typedef	struct	{
	GV_ACT_EX		actor ;

	int			name;
	int			where;

	/* カートリッジ */
	DG_OBJS		*objs ;
	FMATRIX		lights[ 2 ] ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	int			rot_y_add;
	int			splash_flag;

	int			limit_length;
	int			fade_length;
	Unit		unit;
} Work ;

static	void SplashAct( Work *work )
{
	int		j,k;
	int		clock;
	int		flag;
	int		alpha=0;
	float	param0;
	float	param1;
	float	param2;
	FVECTOR	fvtemp0[2];
	FVECTOR	*fvtemp1;
	FVECTOR	*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	if( ok_flush_status!=0 ){
		DG_InvisiblePrim2( work->unit.prim ) ;
		return;
	}else{
		DG_VisiblePrim2( work->unit.prim ) ;
	}

   //AR_PARTICLE_FULL
   DG_SwitchBuffPrim2( work->unit.prim );
	clock = work->unit.prim->buffer_clock;

	fvtemp1 = &fvtemp0[1];
	DG_SetPos( &DG_Chanls->eye_pers );
	

	/* 画面内バンダリチェックを行なう */
	/* 実画面の２倍を基準としたラインクリップ */
	DG_PutVector( work->unit.line_data_rotate, fvtemp0, 2 );

	flag=1;

//printf("%f\n",fvtemp0->vz);

	if( (fvtemp0->vz > fvtemp0->vw)
	 && (fvtemp1->vz > fvtemp1->vw) ){	/* 両端点共に画面奥にきた */
		flag=0;
	}else{

		if(fvtemp0->vw > 0.0f){
			param0 = fvtemp0->vw;
		}else{
			param0 = 0.0f;
			fvtemp0->vw = -fvtemp0->vw;
		}
		if(fvtemp1->vw > 0.0f){
			param1 = fvtemp1->vw;
		}else{
			param1 = 0.0f;
			fvtemp1->vw = -fvtemp1->vw;
		}

		fvtemp1->vw *= 2.0f;

		alpha = (param0 < param1)? (int)param0: (int)param1;
		if( alpha > work->limit_length ){
			 flag=0;
		}else if( (fvtemp0->vx < -fvtemp0->vw)
		       && (fvtemp1->vx < -fvtemp1->vw) ){
			 flag=0;
		}else if( (fvtemp0->vx >  fvtemp0->vw)
		       && (fvtemp1->vx >  fvtemp1->vw) ){
			 flag=0;
		}else if( (fvtemp0->vy < -fvtemp0->vw)
		       && (fvtemp1->vy < -fvtemp1->vw) ){
			 flag=0;
		}else if( (fvtemp0->vy >  fvtemp0->vw)
		       && (fvtemp1->vy >  fvtemp1->vw) ){
			 flag=0;
		}
	}

	if( flag ){
		DG_VisiblePrim2( work->unit.prim );
		if( alpha < work->fade_length){
			alpha = work->unit.alpha;
		}else{
			alpha = work->unit.alpha * (work->limit_length - alpha) / (work->limit_length - work->fade_length);
		}

		param1  = work->unit.width;
		param2  = work->unit.width *0.5f;
		pos     = work->unit.prim->pos[clock] ;
		uvrgbwh = work->unit.prim->uvrgb[clock] ;
		for ( j = 0 ; j < work->unit.n_prim ; j++ ){
			for ( k = 0 ; k < N_VERTS ; k++ ){
				param0 = rnd();
				pos->vx = work->unit.line_data_rotate->vx + work->unit.line_width.vx * param0 + param1*rnd() - param2;
				pos->vy = work->unit.line_data_rotate->vy + work->unit.line_width.vy * param0 + work->unit.height*rnd();
				pos->vz = work->unit.line_data_rotate->vz + work->unit.line_width.vz * param0 + param1*rnd() - param2;
				pos++;

				uvrgbwh->a = alpha;
				uvrgbwh++;
			}
		}
	}else{
		DG_InvisiblePrim2( work->unit.prim );
	}

}

static	void Act( Work *work )
{

//	AN_Test_Eye2( &work->pos, 2 );

	work->rot.vy += work->rot_y_add;

	//	printf ("HIGH %f \n", work->pos.vy - GM_PlayerFindPos.vy);

	if( (work->rot.vy&2047) < work->rot_y_add ){
	    if ( ( GM_PlayerControl != NULL)&&
		 (work->pos.vy - GM_PlayerFindPos.vy < 18000.0f) ){	// H.Satoyoshi 10/5
		GM_SeSetMode( SD_A_LADERLP1, &work->pos, GM_SEMODE_NORMAL );
	    }
	}
	work->rot.vy &= 4095;

	if( (GV_Time & 7)==0 ){
	    if ( ( GM_PlayerControl != NULL)&&
		 (work->pos.vy - GM_PlayerFindPos.vy < 18000.0f) ){	// H.Satoyoshi 10/5
		GM_SeSetMode( SD_A_LADERMT1, &work->pos, GM_SEMODE_NORMAL );
	    }
	}

	DG_SetPos2( &work->pos, &work->rot );
	DG_PutObjs( work->objs );
	DG_GetLightMatrix( &work->pos, work->lights );

	if( work->splash_flag == 1 ){
//		AN_Test_Eye2( &work->unit.line_data[0], 2 );
//		AN_Test_Eye2( &work->unit.line_data[1], 2 );
		DG_PutVector( work->unit.line_data, work->unit.line_data_rotate, 2 );
//		AN_Test_Eye2( &work->unit.line_data_rotate[0], 2 );
//		AN_Test_Eye2( &work->unit.line_data_rotate[1], 2 );
		work->unit.line_width.vx   = work->unit.line_data_rotate[1].vx - work->unit.line_data_rotate[0].vx;
		work->unit.line_width.vy   = work->unit.line_data_rotate[1].vy - work->unit.line_data_rotate[0].vy;
		work->unit.line_width.vz   = work->unit.line_data_rotate[1].vz - work->unit.line_data_rotate[0].vz;
		SplashAct( work );
	}
}

static	void Die( Work *work )
{
	if( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}

	work->unit.prim = OK_FreePrim2( work->unit.prim );
}

static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		j, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( j = 0 ; j < work->unit.n_prim ; j++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
			uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
			uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
			uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

			uvrgbwh0->w = uvrgbwh1->w = SIZE ;
			uvrgbwh0->h = uvrgbwh1->h = SIZE ;

			uvrgbwh0->r = uvrgbwh1->r = P_RGB_MAX ;
			uvrgbwh0->g = uvrgbwh1->g = P_RGB_MAX ;
			uvrgbwh0->b = uvrgbwh1->b = P_RGB_MAX ;
			uvrgbwh0->a = uvrgbwh1->a = work->unit.alpha ;

			uvrgbwh0++;	uvrgbwh1++;
		}
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * work->unit.n_prim * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * work->unit.n_prim * N_VERTS );
}

static	int GetResources( Work *work ){

	DG_DEF		*def ;
	int			data;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	tex = DG_GetTexture( 7733153 /*"splash06_alp"*/ );
	if ( GCL_GetOption( 'd' ) != NULL ){
		work->unit.n_prim = GCL_GetNextInt();
		work->unit.width  = (float)GCL_GetNextInt();
		work->unit.height = (float)GCL_GetNextInt();
		work->unit.alpha  = GCL_GetNextInt() / 2;

		prim = work->unit.prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->unit.n_prim, N_VERTS );
		if(prim==NULL) return -1;
		InitPacket2( work, prim, tex );
		DG_VisiblePrim2( prim );
	}else{
		return -1;
	}

	work->fade_length = DEFAULT_STABLE_LENGTH;
	if ( GCL_GetOption( 'f' ) != NULL ){
		work->fade_length = GCL_GetNextInt();
		if( work->fade_length < DEFAULT_STABLE_LENGTH ) work->fade_length = DEFAULT_STABLE_LENGTH;
	}

	work->limit_length = DEFAULT_LENGTH;
	if ( GCL_GetOption( 'l' ) != NULL ){
		work->limit_length = GCL_GetNextInt();
		if( work->limit_length <= work->fade_length ) work->limit_length = work->fade_length + 1.0f; //avoiding zero divide
	}




	if ( GCL_GetOption( 'k' ) != NULL ){
		data = GCL_GetNextInt();

		def = (DG_DEF*)GV_GetCache( GV_CacheID( data, 'k' ) ) ;
		work->objs = DG_MakeObjs( def, DG_FLAG_ONEPIECE, 0 );
		if( work->objs == NULL ) return -1;
		DG_QueueObjs( work->objs );
		GM_GroupObjs( work->objs, work->where );
	}else{
		return -1;
	}

	if ( GCL_GetOption( 'p' ) != NULL ){
		work->pos.vx = (float)GCL_GetNextInt();
		work->pos.vy = (float)GCL_GetNextInt();
		work->pos.vz = (float)GCL_GetNextInt();
	}

	work->rot_y_add = 128;
	if ( GCL_GetOption( 'r' ) != NULL ){
		work->rot_y_add = GCL_GetNextInt();
	}
	work->rot.vx = 0;
	work->rot.vy = 0;
	work->rot.vz = 0;

	work->splash_flag = 0;
	if ( GCL_GetOption( 's' ) != NULL ){
		work->splash_flag = GCL_GetNextInt();
	}


	if( def->ux - def->lx   <   def->uz - def->lz ){
		work->unit.line_data[0].vx = (def->lx + def->ux) * 0.5f  ;
		work->unit.line_data[0].vy = def->uy;
		work->unit.line_data[0].vz = def->uz;
		work->unit.line_data[1].vx = (def->lx + def->ux) * 0.5f  ;
		work->unit.line_data[1].vy = def->uy;
		work->unit.line_data[1].vz = def->lz;
	}else{
		work->unit.line_data[0].vx = def->ux;
		work->unit.line_data[0].vy = def->uy;
		work->unit.line_data[0].vz = (def->lz + def->uz) * 0.5f  ;
		work->unit.line_data[1].vx = def->lx;
		work->unit.line_data[1].vy = def->uy;
		work->unit.line_data[1].vz = (def->lz + def->uz) * 0.5f  ;
	}

	DG_SetLightMatrix( work->objs, work->lights );

	return 0 ;
}

void *NewRotateY_Object( int name, int where )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		work->name     = name;
		work->where    = where;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
