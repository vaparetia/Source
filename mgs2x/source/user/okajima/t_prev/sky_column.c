//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sky_column.c
	(空の側面)

	2000/01/20 H.Tanaka
	2000/10/18 S.Okajima
	$Id: sky_column.c,v 1.1.1.3 2002/11/19 11:47:46 Yoshizawa1 Exp $
*/

/************************************************************************/
/* 空のエフェクト 


	｜   ｜   ｜   ｜   ｜   ｜   ｜   ｜   ｜ 
	｜ a ｜ a ｜ b ｜ b ｜ a ｜ a ｜ b ｜ b ｜
	｜   ｜   ｜   ｜   ｜   ｜   ｜   ｜   ｜

*/



/***********************************************************************/
#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <math.h>
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
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#ifdef PSX2
#include	"def_dma.h"
#endif
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"../etc/ok_util.h"

extern int ok_flush_status;

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define  RGB_R	   (128)
#define  RGB_G	   (128)
#define  RGB_B	   (128)


#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define  N_PRIMSA	   8
#define  N_PRIMSB	   8
#define  N_VERTS		4
#define  PRIMTYPE	   (DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define  STEP_RAD	   (10)
#define  HANKEI		 (350000.0F)
#define  HEIGHT		 (100000.0F)

#define  MEM_POSA	   ((void *)(SCRPAD_ADDR + 0x0000))	 /* ～ 0x0320 */
#define  MEM_POSB	   ((void *)(SCRPAD_ADDR + 0x0800))	 /* ～ 0x0820 */
#define  MEM_UVRGBA   ((void *)(SCRPAD_ADDR + 0x1000))	 /* ～ 0x1320 */
#define  MEM_UVRGBB   ((void *)(SCRPAD_ADDR + 0x2000))	 /* ～ 0x2320 */
#define  MEM_BASE	   ((void *)(SCRPAD_ADDR + 0x3000))	 /* ～ 0x3360 */

/* ---------------------------------------------------- */
typedef struct{
	GV_ACT_EX   actor		 ;
	int			name;


	DG_PRIM2  *prim_a,*prim_b	 ;
	DG_TEX   *tex_a, *tex_b	   ;
	int	  first_patern  ;
	float	first_anime   ;
	float	height ;
	float	bottom ;

	int		invisible_flag;
	FMATRIX	world;

} Work;

enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_ROTATION,
	REQ_KILL,
	REQ_NO
};

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;
	int itemp;

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
		  case REQ_ROTATION:
			DG_COPY_MAT( &work->world, &DG_UnitMatrix );
			itemp = msg->message[1];
		    _sceVu0RotMatrixY( &work->world, &work->world, (float)itemp*(float)M_PI/2048.0F ) ;
			break;
		  default:
			break;
		}
		msg--;
	}
}

static void Act(Work *work)
{

	CheckMesgParam( work );

	if( ok_flush_status!=0  ||  work->invisible_flag ){
		DG_InvisiblePrim2( work->prim_a ) ;
		DG_InvisiblePrim2( work->prim_b ) ;
		return;
	}else{
		DG_VisiblePrim2( work->prim_a ) ;
		DG_VisiblePrim2( work->prim_b ) ;
	}
}

static void Die(Work *work)
{
	work->prim_a = OK_FreePrim2( work->prim_a ) ;
	work->prim_b = OK_FreePrim2( work->prim_b ) ;
}

extern  void  _BigMemCopy( void *dst, void *src, int size, int num ) ;
/* ------------------------------------------------------ */
static void GetOptionValue(Work *work)
{
	if(GCL_GetOption('h') != NULL){
		work->height = (float)GCL_GetNextInt();
	}else{
		work->height = HEIGHT ;
	}
	if(GCL_GetOption('b') != NULL){
		work->bottom = (float)GCL_GetNextInt();
	}else{
		work->bottom = 0.0F;
	}
}

static  void  InitScrPad(Work *work,DG_TEX *tex_a,DG_TEX *tex_b)
{

	float			rad ;
	float			step ;
	FVECTOR			*base ;
	FVECTOR			*pos_a,*pos_b ;
	DG_PRIM2_UVRGB	*uvrgb_a,*uvrgb_b ;
	int				i,j,k ;
	float			patern ;

	rad = 0.0F + 0.5F * (float)M_PI ;
	step = (float)STEP_RAD / 180.0F * (float)M_PI ;
	base = MEM_BASE ;

	/* ---------------------------------------*/
	/* 50°のところから始まる*/
	rad += step * 5 ;

	/* 座標 */
	for(i = 0; i < 9; i++){
		base[0].vx = (float)HANKEI * cosf(rad) ;
		base[0].vy = work->bottom + work->height ;
		base[0].vz = (float)HANKEI * sinf(rad) ;
		if((i == 0) || (i == 8)){
			base[0].vw = 0.0F ;
		}else{
			base[0].vw = 1.0F ;
		}
		base[1].vx = base[0].vx ;
		base[1].vy = work->bottom ;
		base[1].vz = base[0].vz ;
		if(i < 5){
			base[1].vw = 24.0F * i ;
		}else{
			base[1].vw = 24.0F * ( 8 - i ) ;
		}
		rad += step ;
		base += 2 ;
	}

	/* 次は230°のところから始まる */
	rad = 0.0F + 0.5F * (float)M_PI ;
	rad += step * 23 ;

	/* 座標 */
	for(i = 0; i < 9; i++){
		base[0].vx = (float)HANKEI * cosf(rad) ;
		base[0].vy = work->bottom + work->height ;
		base[0].vz = (float)HANKEI * sinf(rad) ;
		if((i ==0) || (i == 8)){
			base[0].vw = 0.0F ;
		}else{
			base[0].vw = 1.0F ;
		}
		base[1].vx = base[0].vx ;
		base[1].vy = work->bottom ;
		base[1].vz = base[0].vz ;
		if(i < 5){
			base[1].vw = 24.0F * i ;
		}else{
			base[1].vw = 24.0F * ( 8 - i ) ;
		}
		rad += step ;
		base += 2 ;
	}

	/* ------------------------------------------- */
	pos_a = MEM_POSA ;
	pos_b = MEM_POSB ;
	uvrgb_a = MEM_UVRGBA ;
	uvrgb_b = MEM_UVRGBB ;
	base = MEM_BASE ;
	/* 位置を決定 */
	/* 50°～ 130°*/
	for(i = 0; i < 2 ; i++){
		/* テクスチャa側 */
		patern = 0.0F ;
		for(j = 0; j < 2; j++){
			uvrgb_a[0].u = FTOI12( patern * tex_a->u_scale + tex_a->u_offset ) ;
			uvrgb_a[0].v = FTOI12( 0.0F * tex_a->v_scale + tex_a->v_offset ) ;
			uvrgb_a[0].q = 4096 ;
			uvrgb_a[0].f = 0x8fff ;
			uvrgb_a[1].u = FTOI12( patern * tex_a->u_scale + tex_a->u_offset ) ;
			uvrgb_a[1].v = FTOI12( 1.0F * tex_a->v_scale + tex_a->v_offset ) ;
			uvrgb_a[1].q = 4096 ;
			uvrgb_a[1].f = 0x8fff ;
			uvrgb_a[2].u = FTOI12( (patern + 0.5F) * tex_a->u_scale + tex_a->u_offset ) ;
			uvrgb_a[2].v = FTOI12( 0.0F * tex_a->v_scale + tex_a->v_offset ) ;
			uvrgb_a[2].q = 4096 ;
			uvrgb_a[2].f = 0x0fff ;
			uvrgb_a[3].u = FTOI12( (patern + 0.5F) * tex_a->u_scale + tex_a->u_offset ) ;
			uvrgb_a[3].v = FTOI12( 1.0F * tex_a->v_scale + tex_a->v_offset ) ;
			uvrgb_a[3].q = 4096 ;
			uvrgb_a[3].f = 0x0fff ;
			for(k = 0; k < N_VERTS; k++){
				pos_a->vx = base->vx ;
				pos_a->vy = base->vy ;
				pos_a->vz = base->vz ;
				pos_a->vw = 1.0F ;
				uvrgb_a->r  = RGB_R;
				uvrgb_a->g  = RGB_G;
				uvrgb_a->b  = RGB_B;
				uvrgb_a->a  = (u_short)base->vw ;
				uvrgb_a ++ ;
				pos_a ++ ;
				base ++ ;
			}
			base -= 2 ;
			patern += 0.5F ;
		}
		//base += 2 ;

		patern = 0.0F ;
		/* テクスチャb側 */
		for(j = 0; j < 2; j++){
			uvrgb_b[0].u = FTOI12( patern * tex_b->u_scale + tex_b->u_offset ) ;
			uvrgb_b[0].v = FTOI12( 0.0F * tex_b->v_scale + tex_b->v_offset ) ;
			uvrgb_b[0].q = 4096 ;
			uvrgb_b[0].f = 0x8fff ;
			uvrgb_b[1].u = FTOI12( patern * tex_b->u_scale + tex_b->u_offset ) ;
			uvrgb_b[1].v = FTOI12( 1.0F * tex_b->v_scale + tex_b->v_offset ) ;
			uvrgb_b[1].q = 4096 ;
			uvrgb_b[1].f = 0x8fff ;
			uvrgb_b[2].u = FTOI12( (patern + 0.5F) * tex_b->u_scale + tex_b->u_offset ) ;
			uvrgb_b[2].v = FTOI12( 0.0F * tex_b->v_scale + tex_b->v_offset ) ;
			uvrgb_b[2].q = 4096 ;
			uvrgb_b[2].f = 0x0fff ;
			uvrgb_b[3].u = FTOI12( (patern + 0.5F) * tex_b->u_scale + tex_b->u_offset ) ;
			uvrgb_b[3].v = FTOI12( 1.0F * tex_b->v_scale + tex_b->v_offset ) ;
			uvrgb_b[3].q = 4096 ;
			uvrgb_b[3].f = 0x0fff ;
			for(k = 0; k < N_VERTS; k++){
				pos_b->vx = base->vx ;
				pos_b->vy = base->vy ;
				pos_b->vz = base->vz ;
				pos_b->vw = 1.0F ;
				uvrgb_b->r  = RGB_R;
				uvrgb_b->g  = RGB_G;
				uvrgb_b->b  = RGB_B;
				uvrgb_b->a  = (u_short)base->vw ;
				uvrgb_b ++ ;
				pos_b ++ ;
				base ++ ;
			}
			base -= 2 ;
			patern += 0.5F ;
		}
		//base += 2 ;
	}

	base += 2 ;
	/* 230°～ 310°*/
	for(i = 0; i < 2; i++){
		/* テクスチャa側 */
		patern = 0.0F ;
		for(j = 0; j < 2; j++){
			uvrgb_a[0].u = FTOI12( patern * tex_a->u_scale + tex_a->u_offset ) ;
			uvrgb_a[0].v = FTOI12( 0.0F * tex_a->v_scale + tex_a->v_offset ) ;
			uvrgb_a[0].q = 4096 ;
			uvrgb_a[0].f = 0x8fff ;
			uvrgb_a[1].u = FTOI12( patern * tex_a->u_scale + tex_a->u_offset ) ;
			uvrgb_a[1].v = FTOI12( 1.0F * tex_a->v_scale + tex_a->v_offset ) ;
			uvrgb_a[1].q = 4096 ;
			uvrgb_a[1].f = 0x8fff ;
			uvrgb_a[2].u = FTOI12( (patern + 0.5F) * tex_a->u_scale + tex_a->u_offset ) ;
			uvrgb_a[2].v = FTOI12( 0.0F * tex_a->v_scale + tex_a->v_offset ) ;
			uvrgb_a[2].q = 4096 ;
			uvrgb_a[2].f = 0x0fff ;
			uvrgb_a[3].u = FTOI12( (patern + 0.5F) * tex_a->u_scale + tex_a->u_offset ) ;
			uvrgb_a[3].v = FTOI12( 1.0F * tex_a->v_scale + tex_a->v_offset ) ;
			uvrgb_a[3].q = 4096 ;
			uvrgb_a[3].f = 0x0fff ;
			for(k = 0; k < N_VERTS; k++){
				pos_a->vx = base->vx ;
				pos_a->vy = base->vy ;
				pos_a->vz = base->vz ;
				pos_a->vw = 1.0F ;
				uvrgb_a->r  = RGB_R;
				uvrgb_a->g  = RGB_G;
				uvrgb_a->b  = RGB_B;
				uvrgb_a->a  = (u_short)base->vw ;
				uvrgb_a ++ ;
				pos_a ++ ;
				base ++ ;
			}
			base -= 2 ;
			patern += 0.5F ;
		}
		//base += 2 ;

		/* テクスチャb側 */
		patern = 0.0F ;
		for(j = 0; j < 2; j ++){
			uvrgb_b[0].u = FTOI12( patern * tex_b->u_scale + tex_b->u_offset ) ;
			uvrgb_b[0].v = FTOI12( 0.0F * tex_b->v_scale + tex_b->v_offset ) ;
			uvrgb_b[0].q = 4096 ;
			uvrgb_b[0].f = 0x8fff ;
			uvrgb_b[1].u = FTOI12( patern * tex_b->u_scale + tex_b->u_offset ) ;
			uvrgb_b[1].v = FTOI12( 1.0F * tex_b->v_scale + tex_b->v_offset ) ;
			uvrgb_b[1].q = 4096 ;
			uvrgb_b[1].f = 0x8fff ;
			uvrgb_b[2].u = FTOI12( (patern + 0.5F) * tex_b->u_scale + tex_b->u_offset ) ;
			uvrgb_b[2].v = FTOI12( 0.0F * tex_b->v_scale + tex_b->v_offset ) ;
			uvrgb_b[2].q = 4096 ;
			uvrgb_b[2].f = 0x0fff ;
			uvrgb_b[3].u = FTOI12( (patern + 0.5F) * tex_b->u_scale + tex_b->u_offset ) ;
			uvrgb_b[3].v = FTOI12( 1.0F * tex_b->v_scale + tex_b->v_offset ) ;
			uvrgb_b[3].q = 4096 ;
			uvrgb_b[3].f = 0x0fff ;
			for(k = 0; k < N_VERTS; k++){
				pos_b->vx = base->vx ;
				pos_b->vy = base->vy ;
				pos_b->vz = base->vz ;
				pos_b->vw = 1.0F ;
				uvrgb_b->r  = RGB_R;
				uvrgb_b->g  = RGB_G;
				uvrgb_b->b  = RGB_B;
				uvrgb_b->a  = (u_short)base->vw ;
				uvrgb_b ++;
				pos_b ++ ;
				base ++ ;
			}
			base -= 2 ;
			patern += 0.5F ;
		}
		//base += 2 ;
	}	
}

static  int  GetResources(Work *work)
{
	DG_PRIM2		*prim_a,*prim_b ;
	DG_TEX		  *tex_a, *tex_b ;
	int			 num ;

	DG_COPY_MAT( &work->world, &DG_UnitMatrix );

	GetOptionValue(work) ;

	/* 初期化 */
	prim_a = work->prim_a = GM_MakePrim2( PRIMTYPE, N_PRIMSA, N_VERTS );
	if(prim_a == NULL){
		printf("null prim\n");
		return -1;
	}

	prim_b = work->prim_b = GM_MakePrim2( PRIMTYPE, N_PRIMSB, N_VERTS ) ;
	if(prim_b == NULL){
		printf("null prim\n");
		return -1;
	}

	/* テクスチャ取得 */
	tex_a = work->tex_a = DG_GetTexture(5439296 /*"env01a_msk"*/);  
	if(tex_a == NULL){
		printf("null texture\n") ;
		return -1 ;
	}

	tex_b = work->tex_b = DG_GetTexture(6487872 /*"env01b_msk"*/);
	if(tex_b == NULL){
		printf("null texture\n") ;
		return -1 ;
	}

	DG_ConfigPrim2Tex(prim_a, tex_a) ;
	DG_SetPrim2Alpha(prim_a, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;
	DG_ConfigPrim2Tex(prim_b, tex_b) ;
	DG_SetPrim2Alpha(prim_b, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 )) ;

	/* 初期化 */
	InitScrPad(work,tex_a,tex_b) ;
	num = N_PRIMSA * N_VERTS ;
	_BigMemCopy(prim_a->pos[0],MEM_POSA,sizeof(FVECTOR),num) ;
	_BigMemCopy(prim_a->uvrgb[0],MEM_UVRGBA,sizeof(DG_PRIM2_UVRGB),num) ;
	_BigMemCopy(prim_a->pos[1],MEM_POSA,sizeof(FVECTOR),num) ;
	_BigMemCopy(prim_a->uvrgb[1],MEM_UVRGBA,sizeof(DG_PRIM2_UVRGB),num) ;
	num = N_PRIMSB * N_VERTS ;
	_BigMemCopy(prim_b->pos[0],MEM_POSB,sizeof(FVECTOR),num) ;
	_BigMemCopy(prim_b->uvrgb[0],MEM_UVRGBB,sizeof(DG_PRIM2_UVRGB),num) ;
	_BigMemCopy(prim_b->pos[1],MEM_POSB,sizeof(FVECTOR),num) ;
	_BigMemCopy(prim_b->uvrgb[1],MEM_UVRGBB,sizeof(DG_PRIM2_UVRGB),num) ;

	prim_a->root = &work->world;
	prim_b->root = &work->world;

	work->invisible_flag = 0;

	return 0;
}

void *NewSkyColumn( int name, int where )
{
	Work	   *work;

	work = (Work *)GV_NewEffect(GV_ACTOR_USER,sizeof(Work));
	if(work != NULL){
		GV_SetActor(&(work->actor),Act,Die);
		GV_ActorEX( &work->actor );

		work->name = name;

		if(GetResources(work) < 0){
			GV_DestroyActor(work);
			return NULL;
		}
	}
	return (void *)work;
}
 
