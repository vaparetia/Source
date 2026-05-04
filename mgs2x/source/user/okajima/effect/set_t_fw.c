//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	set_tex_f.c
	ワールドにテクスチャセット（クロスフェード）
	1999/11/09 S.Okajima
	$Id: set_t_fw.c,v 1.1.1.3 2002/11/19 11:47:12 Yoshizawa1 Exp $

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

#define	DUMMY_Z			(5000.0f)
#define	RATIO_MAX		(48)
#define	RGB_0		(200)
#define	RGB_1		(210)

/* ok_focus_z_far よりどれだけ奥までに遷移するか */
#define	FADE_RANGE_FAR		(8000.0f)
/* ok_focus_z_far よりどれだけ手前から遷移するか */
#define	FADE_RANGE_NEAR		(-4000.0f)

#define	N_POLYS		(16)
#define	N_VERTS		(N_POLYS * 4)
#define	N_PRIMS		(1)

#define	SCR_POS			(SCRPAD_ADDR)
#define	SCR_UVS			(SCRPAD_ADDR + 0x2000)

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	int			prim_num;

	DG_PRIM2	*prim0 ;
	DG_PRIM2	*prim1 ;

	FVECTOR		center[N_POLYS] ;
	SVECTOR		rot[N_POLYS];

	float		width ;
	float		height ;

} Work ;


/*----------------------------------------------------------------*/
// verts は FVECTOR ４つ分作成
static void MakeRotationVertics( FVECTOR *center, SVECTOR *rot, float w, float h, FVECTOR *verts )
{
	verts[0].vx = -w;
	verts[0].vy = -h;
	verts[0].vz = 0.0f;
	verts[1].vx =  w;
	verts[1].vy = -h;
	verts[1].vz = 0.0f;
	verts[2].vx = -w;
	verts[2].vy =  h;
	verts[2].vz = 0.0f;
	verts[3].vx =  w;
	verts[3].vy =  h;
	verts[3].vz = 0.0f;

	DG_SetPos2( center, rot );
	DG_PutVector( verts, verts, 4 );
/*
printf("0:%f %f %f\n",verts[0].vx,verts[0].vy,verts[0].vz);
printf("1:%f %f %f\n",verts[1].vx,verts[1].vy,verts[1].vz);
printf("2:%f %f %f\n",verts[2].vx,verts[2].vy,verts[2].vz);
printf("3:%f %f %f\n",verts[3].vx,verts[3].vy,verts[3].vz);
*/
}
/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center, SVECTOR *rot, float width, float height, int rgb )
{
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   = SCR_POS ;
	uvrgb = SCR_UVS ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < work->prim_num ; k++ ){
			MakeRotationVertics( center, rot, width, height, pos );
//printf("%f %f %f\n",center->vx,center->vy,center->vz);
			center++;
			rot++;
			pos += 4;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = rgb ;
			uvrgb->g = rgb ;
			uvrgb->b = rgb ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = rgb ;
			uvrgb->g = rgb ;
			uvrgb->b = rgb ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = rgb ;
			uvrgb->g = rgb ;
			uvrgb->b = rgb ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = rgb ;
			uvrgb->g = rgb ;
			uvrgb->b = rgb ;
			uvrgb->a = 0 ;
			uvrgb++;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        work->prim_num * N_PRIMS * 4);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        work->prim_num * N_PRIMS * 4);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), work->prim_num * N_PRIMS * 4);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), work->prim_num * N_PRIMS * 4);

	return 1;

}
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGB	*uvrgb0 ;
	DG_PRIM2_UVRGB	*uvrgb1 ;
	FVECTOR		*fvtemp;
	FVECTOR		fvcal;
	float	change_start;
	float	change_end;
	float	len;
	int		i;
	int		ratio;
	int		clock;
	int		alpha,data_alpha;

//return;

	if( ok_flush_status!=0 ){
		DG_InvisiblePrim2( work->prim0 ) ;
		DG_InvisiblePrim2( work->prim1 ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim0 ) ;
		DG_VisiblePrim2( work->prim1 ) ;
	}


	DG_SwitchBuffPrim2( work->prim0 );
	DG_SwitchBuffPrim2( work->prim1 );
	clock = work->prim0->buffer_clock;

	if( ok_focus_z_far!=0.0f ){
		change_start = ok_focus_z_far - FADE_RANGE_NEAR;
		if(change_start < DRAW_Z_MIN) change_start = DRAW_Z_MIN;
		change_end   = ok_focus_z_far + FADE_RANGE_FAR;
		if(change_end   > DRAW_Z_MAX) change_end   = DRAW_Z_MAX;
	}else{
		/* キャラ：Ｚ制御が起動していないとき */
		change_start = DUMMY_Z - FADE_RANGE_NEAR;
		if(change_start < DRAW_Z_MIN) change_start = DRAW_Z_MIN;
		change_end   = DUMMY_Z + FADE_RANGE_FAR;
		if(change_end   > DRAW_Z_MAX) change_end   = DRAW_Z_MAX;
	}

	fvtemp = work->center;
	uvrgb0  = work->prim0->uvrgb[clock];
	uvrgb1  = work->prim1->uvrgb[clock];
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for(i=0; i<work->prim_num; i++){
			_sceVu0SubVector( &fvcal, fvtemp, (FVECTOR *)DG_Chanls->eye.m[3] );
			len=GV_VecLen3F( &fvcal );

			if(len > change_end){
				ratio=RATIO_MAX;
			}else if(len > change_start ){
				ratio=(int)((len - change_start)*(float)RATIO_MAX/(float)(change_end - change_start));
			}else{
				ratio=0;		/* フラグとしても使用 */
			}

			data_alpha = ratio/2;
			alpha=(data_alpha>0)?data_alpha:0;
			alpha=(data_alpha<255)?data_alpha:255;
			uvrgb0->a = alpha; uvrgb0++;
			uvrgb0->a = alpha; uvrgb0++;
			uvrgb0->a = alpha; uvrgb0++;
			uvrgb0->a = alpha; uvrgb0++;

			data_alpha = RATIO_MAX-ratio;
			alpha=(data_alpha>0)?data_alpha:0;
			alpha=(data_alpha<255)?data_alpha:255;
			uvrgb1->a = alpha; uvrgb1++;
			uvrgb1->a = alpha; uvrgb1++;
			uvrgb1->a = alpha; uvrgb1++;
			uvrgb1->a = alpha; uvrgb1++;

//AN_Test_Eye2( fvtemp, 2 );

			fvtemp++;
		}
	}


}

static void Die( Work *work )
{
	work->prim0 = OK_FreePrim2( work->prim0 );
	work->prim1 = OK_FreePrim2( work->prim1 );
}

static int GetResources( Work *work )
{
	int	i;
	int	tex_name;
	FVECTOR		*center;
	DG_PRIM2		*prim ;
	DG_TEX		*tex0=NULL ;
	DG_TEX		*tex1=NULL ;

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





	work->prim_num=0;
	if ( GCL_GetOption( 'r' ) != NULL ) {
		for(i=0; i<N_POLYS; i++){
			if( GCL_NextStr() != NULL ){
				work->rot[i].vx = (float)GCL_GetNextInt() ;
				work->rot[i].vy = (float)GCL_GetNextInt() ;
				work->rot[i].vy = 0.0f;
			}else{
				break;
			}
		}
		work->prim_num=i;
	}

	if ( GCL_GetOption( 's' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			work->width  = (float)GCL_GetNextInt() ;
			work->height = (float)GCL_GetNextInt() ;
		}
	}

	if ( GCL_GetOption( 'p' ) != NULL ) {
		center=work->center;
		for(i=0; i<work->prim_num; i++){
			if( GCL_NextStr() != NULL ){
				center->vx   = (float)GCL_GetNextInt() ;
				center->vy   = (float)GCL_GetNextInt() ;
				center->vz   = (float)GCL_GetNextInt() ;
				center++;
			}
		}
	}


//	prim = work->prim0 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, work->prim_num * 4 );
	prim = work->prim0 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, work->prim_num * 4 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex0, work->center, work->rot, work->width, work->height, RGB_0 );


//	prim = work->prim1 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, work->prim_num * 4 );
	prim = work->prim1 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, work->prim_num * 4 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex1, work->center, work->rot, work->width, work->height, RGB_1 );


	return 0 ;
}

void *NewPutWorldTexFadeWorld( void )
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
	}
	return (void *)work ;
}
