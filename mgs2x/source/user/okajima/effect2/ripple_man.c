//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ripple_man.c
	波紋制御

	2001/01/05 S.Okajima
	$Id: ripple_man.c,v 1.1.1.3 2002/11/19 11:47:23 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"./bubble.h"

/*----------------------------------------------------------------*/
void *NewRippleMan( int name, int map );
extern float GM_WaterLevel;
/*----------------------------------------------------------------*/

#define	RAISE_ADD		( 1000)
#define	RAISE_SUB		(-1000)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVR		(SCRPAD_ADDR + 0x2000)

//#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
//#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(8)
#define	N_LOOPS		(4)

#define	COL_R		(64)
#define	COL_G		(64)
#define	COL_B		(64)
#define	MAX_ALPHA	(128)	/* 寿命に影響 */
#define	DEC_ALPHA	(4)		/* 寿命に影響 */



#define	ADD_SIZE		(5.0f)
#define	SHADOW_SHIFT	(ADD_SIZE)


typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			activate_num;

	DG_PRIM2	*prim_add ;
	DG_PRIM2	*prim_sub ;
} Work ;

static Work *OK_RippleManWork = NULL;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2		*prim_add ;
	DG_PRIM2		*prim_sub ;
	FVECTOR			*pos;
	FVECTOR			*dest_pos;
	FVECTOR			*dest_pos2;
	FVECTOR			*dest_pos_before;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*dest_uvrgb;
	DG_PRIM2_UVRGB	*dest_uvrgb2;
	DG_PRIM2_UVRGB	*dest_uvrgb_before;
	int		i, j ;
	int		clock;
	int		flag;

	prim_add = work->prim_add;
	prim_sub = work->prim_sub;
	GM_GroupPrim2( prim_add, GM_CurrentStageMap ) ;
	GM_GroupPrim2( prim_sub, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim_add ) )
   {
      return;
   }
	DG_SwitchBuffPrim2( prim_sub );
	clock = prim_add->buffer_clock;

	if( GV_PauseLevel != 0 ){
		dest_pos          = prim_add->pos[  clock];
		dest_pos_before   = prim_add->pos[1-clock];
		dest_uvrgb        = prim_add->uvrgb[  clock];
		dest_uvrgb_before = prim_add->uvrgb[1-clock];
		for ( i = 0 ; i < N_LOOPS ; i++ ){
			OK_Mem_Scr( SCR_POS, dest_pos_before,   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
			OK_Mem_Scr( SCR_UVR, dest_uvrgb_before, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
			OK_Scr_Mem( dest_pos,   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
			OK_Scr_Mem( dest_uvrgb, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
			dest_pos         += N_PRIMS*N_VERTS;
			dest_pos_before  += N_PRIMS*N_VERTS;
			dest_uvrgb       += N_PRIMS*N_VERTS;
			dest_uvrgb_before+= N_PRIMS*N_VERTS;
		}
		dest_pos          = prim_sub->pos[  clock];
		dest_pos_before   = prim_sub->pos[1-clock];
		dest_uvrgb        = prim_sub->uvrgb[  clock];
		dest_uvrgb_before = prim_sub->uvrgb[1-clock];
		for ( i = 0 ; i < N_LOOPS ; i++ ){
			OK_Mem_Scr( SCR_POS, dest_pos_before,   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
			OK_Mem_Scr( SCR_UVR, dest_uvrgb_before, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
			OK_Scr_Mem( dest_pos,   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
			OK_Scr_Mem( dest_uvrgb, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
			dest_pos         += N_PRIMS*N_VERTS;
			dest_pos_before  += N_PRIMS*N_VERTS;
			dest_uvrgb       += N_PRIMS*N_VERTS;
			dest_uvrgb_before+= N_PRIMS*N_VERTS;
		}
		return;
	}

	flag = 0;

	dest_pos          = prim_add->pos[  clock];
	dest_pos2         = prim_sub->pos[  clock];
	dest_pos_before   = prim_add->pos[1-clock];
	dest_uvrgb        = prim_add->uvrgb[  clock];
	dest_uvrgb2       = prim_sub->uvrgb[  clock];
	dest_uvrgb_before = prim_add->uvrgb[1-clock];
	for ( i = 0 ; i < N_LOOPS ; i++ ){
		OK_Mem_Scr( SCR_POS, dest_pos_before,   sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Mem_Scr( SCR_UVR, dest_uvrgb_before, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		pos   = SCR_POS;
		uvrgb = SCR_UVR;
		for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
			if( uvrgb->a != 0 ){

				flag = 1;

				uvrgb->a-=DEC_ALPHA;			uvrgb++;
				uvrgb->a-=DEC_ALPHA;			uvrgb++;
				uvrgb->a-=DEC_ALPHA;			uvrgb++;
				uvrgb->a-=DEC_ALPHA;			uvrgb++;

				pos->vx+= ADD_SIZE;
				pos->vy = GM_WaterLevel;
				pos++;
				pos->vz+= ADD_SIZE;
				pos->vy = GM_WaterLevel;
				pos++;
				pos->vz-= ADD_SIZE;
				pos->vy = GM_WaterLevel;
				pos++;
				pos->vx-= ADD_SIZE;
				pos->vy = GM_WaterLevel;
				pos++;
			}else{
				uvrgb+=4;
				pos+=4;
			}
		}
		OK_Scr_Mem( dest_pos,    SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb,  SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
		OK_Scr_Mem( dest_uvrgb2, SCR_UVR, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);

		pos   = SCR_POS;
		for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
			pos->vx+= SHADOW_SHIFT;
			pos++;
			pos->vx+= SHADOW_SHIFT;
			pos++;
			pos->vx+= SHADOW_SHIFT;
			pos++;
			pos->vx+= SHADOW_SHIFT;
			pos++;
		}
		OK_Scr_Mem( dest_pos2,   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);

		dest_pos         += N_PRIMS*N_VERTS;
		dest_pos2        += N_PRIMS*N_VERTS;
		dest_pos_before  += N_PRIMS*N_VERTS;
		dest_uvrgb       += N_PRIMS*N_VERTS;
		dest_uvrgb2      += N_PRIMS*N_VERTS;
		dest_uvrgb_before+= N_PRIMS*N_VERTS;
	}

	if( !flag ){
		if ( !GM_CheckGameStatus( STATE_DEMO ) ) GV_DestroyActor( work ) ;
	}


}

/*----------------------------------------------------------------*/
int	OK_PutRipple( FVECTOR *center )
{
	Work			*work;
	DG_PRIM2		*prim ;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	FVECTOR			fvtemp;
	int		clock;
	int		num;

	if( OK_RippleManWork==NULL ){
		if ( GM_CheckGameStatus( STATE_DEMO ) ){
//			printf("ERR:--------------------------------------------------\n");
//			printf("ERR:デモ中なんで先に波紋を常駐する必要があるんです。\n");
//			printf("ERR:Pls. set ripple manager at first!!\n");
//			printf("ERR:--------------------------------------------------\n");
			return 0;
		}
		NewRippleMan( 1, 1 );
		if( OK_RippleManWork==NULL ) return 0;
	}
	work = OK_RippleManWork;

	prim = work->prim_add;
	clock = prim->buffer_clock;

	pos          = prim->pos[clock];
	uvrgb        = prim->uvrgb[clock];
	num = work->activate_num*4;
	pos         += num;
	uvrgb       += num;

	(uvrgb++)->a = MAX_ALPHA;
	(uvrgb++)->a = MAX_ALPHA;
	(uvrgb++)->a = MAX_ALPHA;
	(uvrgb++)->a = MAX_ALPHA;

	fvtemp.vx = center->vx;
	fvtemp.vy = GM_WaterLevel;
	fvtemp.vz = center->vz;

	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );
	DG_COPY_VEC( pos++, &fvtemp );

//AN_Test_Eye2( center, 2 );

/*
if( DG_FABS(fvtemp.vx) < 1000.0f
 && DG_FABS(fvtemp.vy) < 1000.0f
 && DG_FABS(fvtemp.vz) < 1000.0f
  ){

printf("%d\n",work->activate_num);

}
*/

	work->activate_num++;
	work->activate_num = (work->activate_num < N_LOOPS*N_PRIMS*N_POLYS)? work->activate_num: 0;

	return 1;
}

static void Die( Work *work )
{
	work->prim_add = OK_FreePrim2( work->prim_add );
	work->prim_sub = OK_FreePrim2( work->prim_sub );
	OK_RippleManWork = NULL;
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int add_sub )
{
	FVECTOR			*pos0;
	FVECTOR			*pos1;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		i ;

	DG_ConfigPrim2Tex( prim, tex );
	if(add_sub){
		DG_RaisePrim2( prim, RAISE_SUB );
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}else{
		DG_RaisePrim2( prim, RAISE_ADD );
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}

	pos0   = prim->pos[ 0 ];
	pos1   = prim->pos[ 1 ];
	uvrgb0 = prim->uvrgb[ 0 ];
	uvrgb1 = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS*N_PRIMS*N_POLYS ; i++ ){
		DG_COPY_VEC( pos0, &DG_ZeroVector );	pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector );	pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector );	pos0++;
		DG_COPY_VEC( pos0, &DG_ZeroVector );	pos0++;
		DG_COPY_VEC( pos1, &DG_ZeroVector );	pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector );	pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector );	pos1++;
		DG_COPY_VEC( pos1, &DG_ZeroVector );	pos1++;

		uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x8fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x8fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x0fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb1->q = uvrgb0->q = 4096 ;
		uvrgb1->f = uvrgb0->f = 0x0fff ;
		uvrgb1->r = uvrgb0->r = COL_R ;
		uvrgb1->g = uvrgb0->g = COL_G ;
		uvrgb1->b = uvrgb0->b = COL_B ;
		uvrgb1->a = uvrgb0->a = 0 ;
		uvrgb0++;
		uvrgb1++;
	}

}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

//	tex = DG_GetTexture( 6684577 /*"splash05_alp"*/ );
//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	tex = DG_GetTexture( 7744599 /*"hamon09_add_alp"*/ );

	prim = work->prim_add = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS*N_LOOPS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex, 0 );

	prim = work->prim_sub = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS*N_LOOPS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex, 1 );


	work->activate_num = 0;

	return 0 ;
}

void *NewRippleMan( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

//	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
//	work = (Work *)GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	work = (Work *)GV_NewEffect( GV_ACTOR_PREV, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map  = map;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

		OK_RippleManWork = work;

	}
	return (void *)work ;
}

