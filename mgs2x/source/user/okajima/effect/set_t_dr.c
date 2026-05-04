//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	set_t_fc.c
	カメラに平行にテクスチャセット（クロスフェード）ドア専用
	1999/11/09 S.Okajima
	$Id: set_t_dr.c,v 1.1.1.3 2002/11/19 11:47:12 Yoshizawa1 Exp $

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
extern float ok_focus_z_far;
extern void OK_LightPos_PutCameraDrop( FVECTOR *fvec );
extern void *NewBreakLight( FVECTOR *, FVECTOR *, int  ) ;

#define	DUMMY_Z			(5000.0f)
#define	RATIO_MAX		(48.0f)

#define	RGB_0		(32)
#define	RGB_1		(32)

/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR		(8000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR		(-700.0f)

#define	R_POS		(1)
#define	R_UVS		(2)
#define	R_DAT		( R_POS +   R_UVS + R_UVS + R_UVS )

#define N_SET		(3)
#define N_PRIMS2	(1)
#define N_VERTS2	(16)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVS0		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS1		(SCRPAD_ADDR + 0x2000)

#define	RAISE_SHIFT		(150.0f)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int		name;
	int		where;

	FVECTOR		center[N_VERTS2];

	int			verts_num;
	DG_PRIM2	*prim0 ;
	DG_PRIM2	*prim1 ;
	float		zoom_max;

	float		radius0 ;
	float		radius1 ;

//    TARGET tgt[N_PACKETS];        /*  11/26 /1999  Revised by T.Morita */
//    POWER_TARGET pow[N_PACKETS];  /*  11/26 /1999  Revised by T.Morita */
    TARGET tgt[N_VERTS2];           /*  11/26 /1999  Revised by S.Okajima */
    POWER_TARGET pow[N_VERTS2];     /*  11/26 /1999  Revised by S.Okajima */
	int			flag[N_VERTS2];     /*  11/26 /1999  Revised by S.Okajima */
} Work ;
/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;

//printf("pp:%x\n",center);

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos     = SCR_POS ;
	uvrgbwh = SCR_UVS0 ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < work->verts_num ; k++ ){
			DG_COPY_VEC( pos, center );

//printf("%f %f %f\n",pos->vx,pos->vy,pos->vz);

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->w = 0;
			uvrgbwh->h = 0;

			uvrgbwh->r = 0 ;
			uvrgbwh->g = 0 ;
			uvrgbwh->b = 0 ;
			uvrgbwh->a = 0 ;

			center++;
			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh ++ ;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS,  sizeof(FVECTOR),          work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS,  sizeof(FVECTOR),          work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), work->verts_num * N_PRIMS2 );

	return 1;
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR		raise_shift;
	FVECTOR		fvcal;
	float	change_start;
	float	change_end;
	float	len;
	float	zoom;
	float	radius0_x;
	float	radius0_y;
	float	radius1_x;
	float	radius1_y;
	float	cf,sf;
	int	i, clock, num_verts;
	float	ratio;
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh0		;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1		;	/* スプライト用 */



	if( ok_flush_status!=0 ){
		DG_InvisiblePrim2( work->prim0 ) ;
		DG_InvisiblePrim2( work->prim1 ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim0 ) ;
		DG_VisiblePrim2( work->prim1 ) ;
	}


	if( ok_focus_z_far!=0.0f ){
		change_start = ok_focus_z_far - FADE_RANGE_NEAR;
		if(change_start < 1.0f) change_start = 1.0f;
		change_end   = ok_focus_z_far + FADE_RANGE_FAR;
	}else{
		/* キャラ：Ｚ制御が起動していないとき */
		change_start = DUMMY_Z - FADE_RANGE_NEAR;
		if(change_start < 1.0f) change_start = 1.0f;
		change_end   = DUMMY_Z + FADE_RANGE_FAR;
	}

	DG_SetPos( &DG_Chanls->eye );
	raise_shift.vx=0.0f;
	raise_shift.vy=0.0f;
	raise_shift.vz=-RAISE_SHIFT;
	DG_RotVector( &raise_shift, &raise_shift, 1 );




	fvcal.vx=0.0f;
	fvcal.vy=0.0f;
	fvcal.vz=1.0f;
	DG_SetPos( &DG_Chanls->eye_inv );
	DG_RotVector( &fvcal, &fvcal, 1 );

//printf("%f\n",fvcal.vz);

	cf = cosf( fvcal.vz );
	sf = sinf( fvcal.vz );
	radius0_x = work->radius0 * cf;
	radius0_y = work->radius0 * sf;
	radius1_x = work->radius1 * cf;
	radius1_y = work->radius1 * sf;
/*
	radius0_x = 1000.0f;
	radius0_y = 1000.0f;
	radius1_x = 1000.0f;
	radius1_y = 1000.0f;

printf("%f %f %f %f\n",radius0_x,radius0_y,radius1_x,radius1_y);
*/

	DG_SwitchBuffPrim2( work->prim0 );
	DG_SwitchBuffPrim2( work->prim1 );
	clock = work->prim0->buffer_clock;
	num_verts = work->verts_num * N_PRIMS2;
	OK_Mem_Scr( SCR_POS,      work->center,                  sizeof(FVECTOR),          num_verts ) ;
	OK_Mem_Scr( SCR_UVS0,     work->prim0->uvrgb[clock],     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;
	OK_Mem_Scr( SCR_UVS1,     work->prim1->uvrgb[clock],     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;

#if 1
	pos          = SCR_POS;
	uvrgbwh0     = SCR_UVS0;
	uvrgbwh1     = SCR_UVS1;
	for(i=0; i<work->verts_num; i++){
//printf("%f %f %f\n",pos->vx,pos->vy,pos->vz);
		_sceVu0SubVector( &fvcal, pos, (FVECTOR *)DG_Chanls->eye.m[3] );
		_sceVu0AddVector( pos, pos, &raise_shift );
		len=GV_VecLen3F( &fvcal );

		if(len > change_end){
			ratio=RATIO_MAX;
		}else if(len > change_start ){
			ratio=(len - change_start)*(float)RATIO_MAX/(change_end - change_start);
		}else{
			ratio=0.0f;		/* フラグとしても使用 */
		}
		zoom = work->zoom_max
		              + (1.0f-work->zoom_max) * (RATIO_MAX - ratio) / RATIO_MAX;
/*
printf("zoom:%f\n",zoom);
printf("zoom_max:%f\n",work->zoom_max);
printf("ratio:%f\n",ratio);
printf("radius0_x * zoom:%f\n",radius0_x * zoom);
*/


//printf("p1:%x:%d %d %d\n",uvrgbwh0,uvrgbwh0->w,uvrgbwh0->h,uvrgbwh0->a);
		uvrgbwh0->w = uvrgbwh0->h = (int)(work->radius0 * zoom);
		uvrgbwh0->r = uvrgbwh0->g = uvrgbwh0->b = RGB_0;
		uvrgbwh0->a     = (int)(ratio);
//printf("0:%x:%d %d %d\n",uvrgbwh0,uvrgbwh0->w,uvrgbwh0->h,uvrgbwh0->a);

		uvrgbwh1->w = uvrgbwh1->h = (int)(work->radius0 * zoom);
		uvrgbwh1->r = uvrgbwh1->g = uvrgbwh1->b = RGB_1;
		uvrgbwh1->a     = DG_FTOI(RATIO_MAX-ratio);
//printf("1:%x:%d %d %d\n",uvrgbwh1,uvrgbwh1->w,uvrgbwh1->h,uvrgbwh1->a);

		pos++;
		uvrgbwh0++;
		uvrgbwh1++;
	}

#endif
	OK_Scr_Mem( work->prim0->pos[clock],       SCR_POS,      sizeof(FVECTOR),          num_verts ) ;
	OK_Scr_Mem( work->prim1->pos[clock],       SCR_POS,      sizeof(FVECTOR),          num_verts ) ;
	OK_Scr_Mem( work->prim0->uvrgb[clock],     SCR_UVS0,     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;
	OK_Scr_Mem( work->prim1->uvrgb[clock],     SCR_UVS1,     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;

//	uvrgbwh0     = work->prim0->uvrgb[clock];
//printf("e:%x:%d %d %d\n",uvrgbwh0,uvrgbwh0->w,uvrgbwh0->h,uvrgbwh0->a);

}

static void Die( Work *work )
{
        int i ;

	work->prim0 = OK_FreePrim2( work->prim0 );
	work->prim1 = OK_FreePrim2( work->prim1 );

	/*  11/26 /1999  Revised by T.Morita */
	for ( i=work->verts_num ; --i>=0 ; )
	    GM_FreeTarget( &work->tgt[i] ) ;
}

static void TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
	Work        *work ;
	int         i ;

	work = (Work *)ptr ;
	if( def->damaged & TARGET_POWER ){
		for( i=work->verts_num ; --i>=0 ; ){
			if ( &work->tgt[i] == def ){
				work->flag[i]=0;	/* 01/22 2000 S.Okajima added. */
//				if( off->power!=NULL ){
//					NewBreakLight( &def->offset, &off->power->force, work->where ) ;
//				}else{
//					NewBreakLight( &def->offset, &DG_ZeroVector, work->where ) ;
//				}
				GM_ClearTargetDamage( def ) ;
			}
		}
	}
}


static int GetResources( Work *work, int name, int where )
{
	int	i;
	int	tex_name;
	FVECTOR		fvtemp[2];
	DG_PRIM2	*prim ;
	DG_TEX		*tex0 ;
	DG_TEX		*tex1 ;

	work->name = name;
	work->where = where;

	tex0=NULL ;
	tex1=NULL ;

	if ( GCL_GetOption( 't' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			tex_name       = GCL_GetNextInt();
			tex0 = DG_GetTexture( tex_name );
		}
	}

	if ( GCL_GetOption( 'u' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			tex_name       = GCL_GetNextInt();
			tex1 = DG_GetTexture( tex_name );
		}
	}

	if ( GCL_GetOption( 's' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			work->radius0 = (float)GCL_GetNextInt() * 2.0f ;
		}
	}

	work->verts_num=0;
	if ( GCL_GetOption( 'p' ) != NULL ) {
		for(i=0; i<N_VERTS2; i++){
			if( GCL_NextStr() != NULL ){
				work->center[i].vx   = (float)GCL_GetNextInt() ;
				work->center[i].vy   = (float)GCL_GetNextInt() ;
				work->center[i].vz   = (float)GCL_GetNextInt() ;

				OK_LightPos_PutCameraDrop( &work->center[i] );

				/*  11/26 /1999  Revised by T.Morita */
				fvtemp[0].vx = fvtemp[0].vz = fvtemp[0].vy = 90.0F ;
				GM_SetTarget( &work->tgt[i], TARGET_DEFENSE, where, ENEMY_SIDE, &fvtemp[0], &work->center[i] ) ;
//				GM_SetPowerTarget( &work->tgt[i], &work->pow[i], POWER_DECREASE, GM_Vitality, 0, 0, &work->center[i] ) ;
				GM_SetPowerTarget( &work->tgt[i], &work->pow[i], POWER_ONCE, GM_Vitality, 0, 0, &work->center[i] ) ;
				GM_SetTargetCallBack( &work->tgt[i], TargetCallBack, work ) ;
				GM_PutTarget( &work->tgt[i] ) ;
				/*  01/22 /2000  Revised by S.Okajima */

//				NewTargetView( &work->tgt[i], 255, 0, 0 ) ;

				work->flag[i]=1;
			}else{
				break;
			}
		}
		work->verts_num=i;
	}

	work->zoom_max=1.0f;
	if ( GCL_GetOption( 'z' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			work->zoom_max = (float)GCL_GetNextInt() / 4096.0f;
		}
	}

	/* 回転スプライト */
	prim = work->prim0 = GM_MakePrim2(
				DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
//				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA,
//				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS2,
				work->verts_num );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex0, work->center );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	/* 回転スプライト */
	prim = work->prim1 = GM_MakePrim2(
				DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
//				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_FOG|DG_PRIM2_ALPHA,
//				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS2,
				work->verts_num );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex1, work->center );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewPutWorldTexFadeCameraForDoor(  int name, int where  )
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
