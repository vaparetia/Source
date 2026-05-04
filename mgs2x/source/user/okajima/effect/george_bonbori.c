//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	george_bonbori.c
	ジョージワシントン橋点滅灯（クロスフェード）
	2000/08/09 S.Okajima
	$Id: george_bonbori.c,v 1.1.1.3 2002/11/19 11:47:06 Yoshizawa1 Exp $
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

#define	CYCLE				(120)
#define	BLINK_ALPHA_MAX		(64.0f)
#define	DECAY_BLINK_ALPHA	(0.9f)

#define	RATIO_MAX		(64.0f)
#define	RATIO_MAX_CORE	(128.0f)

#define	RGB_0			(40)
#define	RGB_1			(100)
#define	RGB_CORE		(120)

#define	DUMMY_Z			(5000.0f)
/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR		(8000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR		(-700.0f)

#define	DUMMY_Z_WIRE				(200000.0f)
/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR_WIRE			(100000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR_WIRE		(-10000.0f)



#define	R_POS		(1)
#define	R_UVS		(2)
#define	R_DAT		( R_POS +   R_UVS + R_UVS + R_UVS )

#define N_PRIMS2	(1)
#define N_VERTS2	(16)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVS0		(SCRPAD_ADDR + 0x1000)
#define	SCR_UVS1		(SCRPAD_ADDR + 0x2000)

//#define	RAISE_SHIFT		(2000.0f)
#define	RAISE_SHIFT		(500.0f)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int		name;
	int		where;

	int		wire_flag;

	int		count;
	int		blink_count;
	float	blink_alpha;
	FVECTOR		center[N_VERTS2];

	int			verts_num;
	DG_PRIM2	*prim0 ;
	DG_PRIM2	*prim1 ;
	float		zoom_max;

	float		radius0 ;
	float		radius1 ;

} Work ;
/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos     = SCR_POS ;
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < work->verts_num ; k++ ){
			DG_COPY_VEC( pos, center );

			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

			uvrgbwh1->w = uvrgbwh0->w = 0;
			uvrgbwh1->h = uvrgbwh0->h = 0;

			uvrgbwh1->r = uvrgbwh0->r = 0 ;
			uvrgbwh1->g = uvrgbwh0->g = 0 ;
			uvrgbwh1->b = uvrgbwh0->b = 0 ;
			uvrgbwh1->a = uvrgbwh0->a = 0 ;

			center++;
			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS,  sizeof(FVECTOR),          work->verts_num * N_PRIMS2 );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS,  sizeof(FVECTOR),          work->verts_num * N_PRIMS2 );

	return 1;
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i, clock, num_verts;
	FVECTOR		disp_center;
	float	physic_raise;
	float	change_start;
	float	change_end;
	float	len;
	float	zoom;
	float	radius0_x;
	float	radius0_y;
	float	radius1_x;
	float	radius1_y;
	float	cf,sf;
	float	ratio;
	float	screen_near_x;
	float	screen_near_y;
	float	ftemp0;
	float	ftemp1;
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

	DG_COPY_VEC( &disp_center, (FVECTOR *)DG_Chanls->eye.m[3] );


	if( work->wire_flag ){
//printf("work->wire_flag:\n");
		change_start = DUMMY_Z_WIRE - FADE_RANGE_NEAR_WIRE;
		if(change_start < 1.0f) change_start = 1.0f;
		change_end   = DUMMY_Z_WIRE + FADE_RANGE_FAR_WIRE;
	}else{
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
	}

	len=GV_VecLen3F( &disp_center ) / 4096.0f;	/* 対象はなんでもいい？ */
	cf = cosf( len );
	sf = sinf( len );
	radius0_x = work->radius0 * cf;
	radius0_y = work->radius0 * sf;
	radius1_x = work->radius1 * cf;
	radius1_y = work->radius1 * sf;

	DG_SwitchBuffPrim2( work->prim0 );
	DG_SwitchBuffPrim2( work->prim1 );
	clock = work->prim0->buffer_clock;
	num_verts = work->verts_num * N_PRIMS2;
	OK_Mem_Scr( SCR_POS,      work->center,                  sizeof(FVECTOR),          num_verts ) ;
//	OK_Mem_Scr( SCR_UVS0,     work->prim0->uvrgb[clock],     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;
//	OK_Mem_Scr( SCR_UVS1,     work->prim1->uvrgb[clock],     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;

	pos          = SCR_POS;
//	uvrgbwh0     = SCR_UVS0;
//	uvrgbwh1     = SCR_UVS1;
	uvrgbwh0     = work->prim0->uvrgb[clock];
	uvrgbwh1     = work->prim1->uvrgb[clock];

	screen_near_x = (ASPECT_X()                                           ) * DG_Chanls->screen;
	screen_near_y = (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) * DG_Chanls->screen;


	work->blink_alpha *= DECAY_BLINK_ALPHA;
	if(work->count-- < 0){
		work->count = work->blink_count;
		work->blink_alpha = BLINK_ALPHA_MAX;
	}


	for(i=0; i<work->verts_num; i++){
		/* 座標を透視変換する */
		DG_SetPos( &DG_Chanls->eye_pers_no_offset );//BP_CAMERA - replaced "eye_pers" with "eye_pers_no_offset" to fix projection/inverse projection
		DG_PutVector( pos, pos, 1 );

		if( pos->vz < pos->vw  &&  pos->vw > RAISE_SHIFT + 51.0f ){

			pos->vw = (pos->vw  > 0.0f)? pos->vw: -pos->vw;

			if(pos->vw > change_end){
				ratio=RATIO_MAX;
			}else if(pos->vw > change_start ){
				ratio=(pos->vw - change_start)*(float)RATIO_MAX/(change_end - change_start);
			}else{
				ratio=0.0f;		/* フラグとしても使用 */
			}

//if(GV_Time%120==0)printf("%f:%f:%f:%f:\n",pos->vx,pos->vy,pos->vz,pos->vw);

			physic_raise = pos->vw - RAISE_SHIFT;

			zoom = work->zoom_max
			              + (1.0f-work->zoom_max) * (RATIO_MAX - ratio) / RATIO_MAX;
#if 0
			pos->vz = physic_raise;
			pos->vx = physic_raise * pos->vx / screen_near_x / pos->vw;
			pos->vy = physic_raise * pos->vy / screen_near_y / pos->vw;
#else
			pos->vz = physic_raise;
			ftemp0 = pos->vx / pos->vw;
			ftemp1 = pos->vy / pos->vw;
			if( ftemp0 > 2.0f || ftemp0 < -2.0f || ftemp1 > 2.0f || ftemp1 < -2.0f ){
				zoom = 0.0f;
			}
			pos->vx = physic_raise * pos->vx / screen_near_x / pos->vw;
			pos->vy = physic_raise * pos->vy / screen_near_y / pos->vw;
#endif

			/* カメラ座標をワールド座標に変換 */
			DG_SetPos( &DG_Chanls->eye );
			DG_PutVector( pos, pos, 1 );


//if(GV_Time%120==0)printf("%f\n",zoom);

			physic_raise /= 2048.0f;
			cf = cosf( physic_raise );
			sf = sinf( physic_raise );

			if( work->blink_count <= 0 ){
				uvrgbwh0->w     = (int)(radius0_x * zoom);
				uvrgbwh0->h     = (int)(radius0_y * zoom);
				uvrgbwh0->r = uvrgbwh0->g = uvrgbwh0->b = RGB_0;
				uvrgbwh1->w     = (int)(radius1_x * zoom);
				uvrgbwh1->h     = (int)(radius1_y * zoom);
				uvrgbwh1->r = uvrgbwh1->g = uvrgbwh1->b = RGB_1;
				if( work->wire_flag ){
					uvrgbwh0->a     = (int)(ratio*0.4f);
				}else{
					uvrgbwh0->a     = (int)(ratio);
				}
				uvrgbwh1->a     = (int)(RATIO_MAX-ratio);
			}else{
				uvrgbwh0->w     = (int)(radius0_x * zoom);
				uvrgbwh0->h     = (int)(radius0_y * zoom);
				uvrgbwh0->r = uvrgbwh0->g = uvrgbwh0->b = RGB_0;
				uvrgbwh1->w     = uvrgbwh0->w*2;
				uvrgbwh1->h     = uvrgbwh0->h*2;
				uvrgbwh1->r = uvrgbwh1->g = uvrgbwh1->b = RGB_1;
				uvrgbwh0->a     = (int)(work->blink_alpha);
				uvrgbwh1->a     = uvrgbwh0->a;
			}
		}else{
			uvrgbwh0->w = 0;
			uvrgbwh0->h = 0;
			uvrgbwh1->w = 0;
			uvrgbwh1->h = 0;
		}


		pos++;
		uvrgbwh0++;
		uvrgbwh1++;
	}

	OK_Scr_Mem( work->prim0->pos[clock],       SCR_POS,      sizeof(FVECTOR),          num_verts ) ;
	OK_Scr_Mem( work->prim1->pos[clock],       SCR_POS,      sizeof(FVECTOR),          num_verts ) ;
//	OK_Scr_Mem( work->prim0->uvrgb[clock],     SCR_UVS0,     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;
//	OK_Scr_Mem( work->prim1->uvrgb[clock],     SCR_UVS1,     sizeof(DG_PRIM2_UVRGBWH), num_verts ) ;

//	uvrgbwh0     = work->prim0->uvrgb[clock];
//printf("e:%x:%d %d %d\n",uvrgbwh0,uvrgbwh0->w,uvrgbwh0->h,uvrgbwh0->a);

}

static void Die( Work *work )
{
	work->prim0 = OK_FreePrim2( work->prim0 );
	work->prim1 = OK_FreePrim2( work->prim1 );

}

static int GetResources( Work *work, int name, int where )
{
	int	i;
	int	tex_name;
	DG_PRIM2	*prim ;
	DG_TEX		*tex0 ;
	DG_TEX		*tex1 ;

	work->name = name;
	work->where = where;

	tex0=NULL ;
	tex1=NULL ;

	work->wire_flag = 0;
	if ( GCL_GetOption( 'w' ) != NULL ) {
		work->wire_flag = 1;
	}

	work->blink_count = 0;
	if ( GCL_GetOption( 'b' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			work->blink_count = GCL_GetNextInt();
		}
	}

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

	if ( GCL_GetOption( 'v' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			tex_name       = GCL_GetNextInt();
//			tex_core = DG_GetTexture( tex_name );
		}
	}


	if ( GCL_GetOption( 's' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			work->radius0     = (float)GCL_GetNextInt()*2.0f ;
			work->radius1     = (float)GCL_GetNextInt()*2.0f ;
//			work->radius_core = (float)GCL_GetNextInt() ;
		}
	}

	work->verts_num=0;
	if ( GCL_GetOption( 'p' ) != NULL ) {
		for(i=0; i<N_VERTS2; i++){
			if( GCL_NextStr() != NULL ){
				work->center[i].vx   = (float)GCL_GetNextInt() ;
				work->center[i].vy   = (float)GCL_GetNextInt() ;
				work->center[i].vz   = (float)GCL_GetNextInt() ;
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
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
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
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
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

void *NewPutTexNoTarget(  int name, int where  )
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
