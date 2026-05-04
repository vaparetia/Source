//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ripple.c
	波紋
	2000/02/13 S.Okajima
	$Id: ripple.c,v 1.1.1.3 2002/11/19 11:47:11 Yoshizawa1 Exp $
*/

/*

加算減算でそれぞれ dg_prim2 をもつ。
別テクスチャはこれらを別個にもつ。

２つのテクスチャを使用する場合、４つの dg_prim2 となる。

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
#define	RAISE			(-10000)

#define	ADD_TRANS	(0)
#define	SUB_TRANS	(1)

/* 固定 */
#define	ADD_SUB		(2)

#define	N_VERTS			(64)
#define	N_POLYS			(N_VERTS / 4)
#define	N_PRIMS_MAX		(32)

#define	COLOR_R			(128)
#define	COLOR_G			(128)
#define	COLOR_B			(128)
//#define	COLOR_A			(48.0f)
#define	COLOR_A			(32.0f)

#define	SIZE			(120.0f)
#define	SIZE_RANDAM		(500.0f)
#define	RIPPLE_SIZE_MAX		(800.0f)
#define	SIZE_STEP		(40.0f)

#define	ALPHA_RATIO		(0.5f)

// １６色
#define	TEXTURE			( 7429426 /*"hamon04_add_msk"*/ )
#define	A_TEXTURE		( 7494962 /*"hamon05_add_msk"*/ )
// ２５６色
//#define	TEXTURE			( 12829412 /*"hamon04h_add"*/ )
//#define	A_TEXTURE		( 12829414 /*"hamon05h_add"*/ )


typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim[ADD_SUB] ;

	DG_PRIM2	*a_prim ;
	FVECTOR		prim_center[ N_PRIMS_MAX * N_POLYS];

	int			name ;
	int			map ;
	int			n_prim;

	float		radius;

	FVECTOR		boundary_data[4];	/* クリッピングのみに使う */

	float		boundary_min_x;
	float		boundary_min_z;
	float		boundary_width_x;
	float		boundary_width_z;
	float		height;

//	float		wc[ N_PRIMS_MAX * N_POLYS];
//	float		ws[ N_PRIMS_MAX * N_POLYS];
//	float		size[ N_PRIMS_MAX * N_POLYS];

	FVECTOR		fvdata[ N_PRIMS_MAX * N_POLYS];
	FVECTOR		fvdata_fix;

	int			invisible_flag;

} Work ;

enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_PARAM,
	REQ_NO
};

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
		  case REQ_KILL:
		  case GV_MESSAGE_KILL:
			GV_DestroyActor( work ) ;
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
	FVECTOR				boundary[4];
	FVECTOR				*fvdata;
	FVECTOR				*prim_center;
	int					clock;
	int					alpha0;
	int					alpha1;
	int					k,l;
	int					flag;
	DG_PRIM2			**prim;
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	DG_PRIM2_UVRGB		*uvrgb2 ;
	FVECTOR				*pos0;
	FVECTOR				*pos1;
	FVECTOR				*pos2;

	CheckMesgParam( work );

	if( work->invisible_flag ){
		for( k=0; k<ADD_SUB; k++ ){
			DG_InvisiblePrim2( work->prim[k] );
		}
		DG_InvisiblePrim2( work->a_prim );
		return;
	}else{
		for( k=0; k<ADD_SUB; k++ ){
			DG_VisiblePrim2( work->prim[k] );
		}
		DG_VisiblePrim2( work->a_prim );
	}


	flag=1;
	if( work->prim_center->vy > DG_Chanls->eye.m[3][1] ){
		flag=0;
	}else{
		DG_SetPos( &DG_Chanls->eye_pers );
		DG_PutVector( work->boundary_data, boundary, 4 );
		pos0 = pos1 = pos2 = boundary;
		pos0++;
		pos1+=2;
		pos2+=3;
		if( (boundary->vz > boundary->vw)
		 && (pos0->vz     > pos0->vw    )
		 && (pos1->vz     > pos1->vw    )
		 && (pos2->vz     > pos2->vw    ) ){	/* 端点全て画面奥にきた */
			flag=0;
		}else{
			boundary->vw = (boundary->vw > 0.0f)? boundary->vw: -boundary->vw;
			pos0->vw     = (pos0->vw     > 0.0f)? pos0->vw    : -pos0->vw    ;
			pos1->vw     = (pos1->vw     > 0.0f)? pos1->vw    : -pos1->vw    ;
			pos2->vw     = (pos2->vw     > 0.0f)? pos2->vw    : -pos2->vw    ;

			if(       (boundary->vx < -boundary->vw)
			       && (pos0->vx     < -pos0->vw    )
			       && (pos1->vx     < -pos1->vw    )
			       && (pos2->vx     < -pos2->vw    ) ){
				 flag=0;
			}else if( (boundary->vx >  boundary->vw)
			       && (pos0->vx     >  pos0->vw    )
			       && (pos1->vx     >  pos1->vw    )
			       && (pos2->vx     >  pos2->vw    ) ){
				 flag=0;
			}else if( (boundary->vy < -boundary->vw)
			       && (pos0->vy     < -pos0->vw    )
			       && (pos1->vy     < -pos1->vw    )
			       && (pos2->vy     < -pos2->vw    ) ){
				 flag=0;
			}else if( (boundary->vy >  boundary->vw)
			       && (pos0->vy     >  pos0->vw    )
			       && (pos1->vy     >  pos1->vw    )
			       && (pos2->vy     >  pos2->vw    ) ){
				 flag=0;
			}
		}
	}

	if( flag==0 ){
		DG_InvisiblePrim2( work->a_prim );
		DG_InvisiblePrim2( work->prim[0] );
		DG_InvisiblePrim2( work->prim[1] );
		return;
	}else{
		DG_VisiblePrim2( work->a_prim );
		DG_VisiblePrim2( work->prim[0] );
		DG_VisiblePrim2( work->prim[1] );
	}

   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }

	clock = 1 - work->prim[0]->buffer_clock;
	prim = work->prim;
	for( k=0; k<ADD_SUB; k++ ){
		DG_SwitchBuffPrim2( *prim );
		prim++;
	}
	DG_SwitchBuffPrim2( work->a_prim );

	/* 更新座標を計算 */
	pos0   = work->prim[0]->pos[clock];
	pos1   = work->prim[1]->pos[clock];
	pos2   = work->a_prim->pos[clock];
	uvrgb0 = work->prim[0]->uvrgb[clock];
	uvrgb1 = work->prim[1]->uvrgb[clock];
	uvrgb2 = work->a_prim->uvrgb[clock];
	prim_center = work->prim_center;
	fvdata = work->fvdata;
	for ( k=0; k<work->n_prim; k++ ){
		for ( l=0; l<N_POLYS; l++ ){
//			rsize_sin = (*size) * (*ws);
//			rsize_cos = (*size) * (*wc);
//			a_rsize_sin = rsize_sin * 2.0f;
//			a_rsize_cos = rsize_cos * 2.0f;
//			shift = (*size)*0.0625f;

#ifdef BP_PSX2_ASM //yano /*未検証*/
			asm volatile ("
				lqc2		vf11, 0(%3)			# vf11:prim_center
				lqc2		vf12, 0(%4)			# vf12:fvdata
				lqc2		vf13, 0(%5)			# vf13:&work->fvdata_fix

				#---           x           y           z           w
				#vf12         ws        size          wc         ---
				vmuly.xz	vf1,  vf12, vf12
				vadd.xz		vf2,  vf1,  vf1
				vmulw.y		vf1,  vf12, vf12
				#---           x           y           z           w
				#vf1   rsize_sin       shift   rsize_cos        ---
				#vf2 a_rsize_sin         --- a_rsize_cos        ---

				vadd.xyz	vf14, vf0,  vf11

				vadd.y		vf8,  vf0,  vf14
				vadd.y		vf9,  vf0,  vf11
				vadd.y		vf10, vf0,  vf14
				vsubz.x		vf8,  vf11, vf1  
				vsubz.x		vf10, vf14, vf2
				vaddx.z		vf8,  vf11, vf1
				vaddx.z		vf10, vf14, vf2
				vaddy.xz	vf9,  vf8,  vf1
				sqc2		vf10, 0(%2)
				sqc2		vf8,  0(%0)
				sqc2		vf9,  0(%1)

				vadd.y		vf16, vf0,  vf14
				vadd.y		vf17, vf0,  vf11
				vadd.y		vf18, vf0,  vf14
				vadd.xz		vf16, vf11, vf1
				vadd.xz		vf18, vf14, vf2
				vaddy.xz	vf17, vf16, vf1
				sqc2		vf18, 16(%2)
				sqc2		vf16, 16(%0)
				sqc2		vf17, 16(%1)

				vadd.y		vf8,  vf0,  vf14
				vadd.y		vf9,  vf0,  vf11
				vadd.y		vf10, vf0,  vf14
				vsub.xz		vf8,  vf11, vf1
				vsub.xz		vf10, vf14, vf2
				vaddy.xz	vf9,  vf8,  vf1
				sqc2		vf10, 32(%2)
				sqc2		vf8,  32(%0)
				sqc2		vf9,  32(%1)

				vadd.y		vf16, vf0,  vf14
				vadd.y		vf17, vf0,  vf11
				vadd.y		vf18, vf0,  vf14
				vaddz.x		vf16, vf11, vf1
				vaddz.x		vf18, vf14, vf2
				vsubx.z		vf16, vf11, vf1
				vsubx.z		vf18, vf14, vf2
				vaddy.xz	vf17, vf16, vf1
				sqc2		vf18, 48(%2)
				sqc2		vf16, 48(%0)
				sqc2		vf17, 48(%1)

				vsub.y		vf8,  vf13, vf12	# vf8.y = RIPPLE_SIZE_MAX - fvdata->vy
				vmuly.y		vf8,  vf8,  vf8		# vf8.y*=vf8.y
				vmulx.y		vf8,  vf8,  vf13	# vf8.y*=(COLOR_A / (RIPPLE_SIZE_MAX*RIPPLE_SIZE_MAX))
				vmulz.y		vf9,  vf8,  vf13	# vf9.y = vf8.y * ALPHA_RATIO

				vftoi0.y	vf8,  vf8
				vftoi0.y	vf9,  vf9
				vmtir		vi1,  vf8y
				vmtir		vi2,  vf9y
				cfc2		$8,   $vi1
				cfc2		$9,   $vi2
				sw			$8,   0(%6)	# alpha0
				sw			$9,   0(%7)	# alpha1
				":
				 :"r"(pos0),
				  "r"(pos1),
				  "r"(pos2),
				  "r"(prim_center),
				  "r"(fvdata),
				  "r"(&work->fvdata_fix),
				  "r"(&alpha0),
				  "r"(&alpha1)
				 :"$8","$9","memory"
			);

			pos0+=4;	pos1+=4;	pos2+=4;

#else
			{
			float rsize_sin, rsize_cos, a_rsize_sin, a_rsize_cos, shift;
			float ftemp;
			rsize_sin = fvdata->vy * fvdata->vx;
			rsize_cos = fvdata->vy * fvdata->vz;
			a_rsize_sin = rsize_sin * 2.0f;
			a_rsize_cos = rsize_cos * 2.0f;
			shift = fvdata->vy * fvdata->vw;

			pos0->vx = prim_center->vx - rsize_cos;
			pos0->vy = prim_center->vy;
			pos0->vz = prim_center->vz + rsize_sin;
			pos0->vw = 1.0f;
			pos2->vx = prim_center->vx - a_rsize_cos;
			pos2->vy = prim_center->vy;
			pos2->vz = prim_center->vz + a_rsize_sin;
			pos2->vw = 1.0f;
			pos1->vx = pos0->vx + shift;
			pos1->vy = prim_center->vy;
			pos1->vz = pos0->vz + shift;
			pos1->vw = 1.0f;
			pos0++;	pos1++;	pos2++;

			pos0->vx = prim_center->vx +  rsize_sin;
			pos0->vy = prim_center->vy;
			pos0->vz = prim_center->vz + rsize_cos;
			pos0->vw = 1.0f;
			pos2->vx = prim_center->vx + a_rsize_sin;
			pos2->vy = prim_center->vy;
			pos2->vz = prim_center->vz + a_rsize_cos;
			pos2->vw = 1.0f;
			pos1->vx = pos0->vx + shift;
			pos1->vy = prim_center->vy;
			pos1->vz = pos0->vz + shift;
			pos1->vw = 1.0f;
			pos0++;	pos1++;	pos2++;

			pos0->vx = prim_center->vx - rsize_sin;
			pos0->vy = prim_center->vy;
			pos0->vz = prim_center->vz - rsize_cos;
			pos0->vw = 1.0f;
			pos2->vx = prim_center->vx - a_rsize_sin;
			pos2->vy = prim_center->vy;
			pos2->vz = prim_center->vz - a_rsize_cos;
			pos2->vw = 1.0f;
			pos1->vx = pos0->vx + shift;
			pos1->vy = prim_center->vy;
			pos1->vz = pos0->vz + shift;
			pos1->vw = 1.0f;
			pos0++;	pos1++;	pos2++;

			pos0->vx = prim_center->vx + rsize_cos;
			pos0->vy = prim_center->vy;
			pos0->vz = prim_center->vz - rsize_sin;
			pos0->vw = 1.0f;
			pos2->vx = prim_center->vx + a_rsize_cos;
			pos2->vy = prim_center->vy;
			pos2->vz = prim_center->vz - a_rsize_sin;
			pos2->vw = 1.0f;
			pos1->vx = pos0->vx + shift;
			pos1->vy = prim_center->vy;
			pos1->vz = pos0->vz + shift;
			pos1->vw = 1.0f;
			pos0++;	pos1++;	pos2++;

			ftemp = ( RIPPLE_SIZE_MAX - fvdata->vy );
			ftemp = ftemp * ftemp;
			ftemp = ftemp * COLOR_A / ( RIPPLE_SIZE_MAX*RIPPLE_SIZE_MAX );
			alpha1 = (int)( ftemp * ALPHA_RATIO );
			alpha0 = (int)  ftemp;
			}
#endif

//printf("%d:%d:\n",alpha0,alpha1);

			uvrgb0->a = uvrgb1->a = alpha0;
			uvrgb2->a             = alpha1;
			uvrgb0++;	uvrgb1++;	uvrgb2++;
			uvrgb0->a = uvrgb1->a = alpha0;
			uvrgb2->a             = alpha1;
			uvrgb0++;	uvrgb1++;	uvrgb2++;
			uvrgb0->a = uvrgb1->a = alpha0;
			uvrgb2->a             = alpha1;
			uvrgb0++;	uvrgb1++;	uvrgb2++;
			uvrgb0->a = uvrgb1->a = alpha0;
			uvrgb2->a             = alpha1;
			uvrgb0++;	uvrgb1++;	uvrgb2++;

/*
			pos0->vx = prim_center->vx - rsize_cos;
			pos0->vz = prim_center->vz + rsize_sin;
			pos2->vx = prim_center->vx - a_rsize_cos;
			pos2->vz = prim_center->vz + a_rsize_sin;
			pos1->vx = pos0->vx + shift;
			pos1->vz = pos0->vz + shift;
			pos0++;	pos1++;	pos2++;

			pos0->vx = prim_center->vx + rsize_sin;
			pos0->vz = prim_center->vz + rsize_cos;
			pos2->vx = prim_center->vx + a_rsize_sin;
			pos2->vz = prim_center->vz + a_rsize_cos;
			pos1->vx = pos0->vx + shift;
			pos1->vz = pos0->vz + shift;
			pos0++;	pos1++;	pos2++;

			pos0->vx = prim_center->vx - rsize_sin;
			pos0->vz = prim_center->vz - rsize_cos;
			pos2->vx = prim_center->vx - a_rsize_sin;
			pos2->vz = prim_center->vz - a_rsize_cos;
			pos1->vx = pos0->vx + shift;
			pos1->vz = pos0->vz + shift;
			pos0++;	pos1++;	pos2++;

			pos0->vx = prim_center->vx + rsize_cos;
			pos0->vz = prim_center->vz - rsize_sin;
			pos2->vx = prim_center->vx + a_rsize_cos;
			pos2->vz = prim_center->vz - a_rsize_sin;
			pos1->vx = pos0->vx + shift;
			pos1->vz = pos0->vz + shift;
			pos0++;	pos1++;	pos2++;

			alpha = COLOR_A * (int)(RIPPLE_SIZE_MAX - fvdata->vy) * (int)(RIPPLE_SIZE_MAX - fvdata->vy) / (int)RIPPLE_SIZE_MAX / (int)RIPPLE_SIZE_MAX;
			uvrgb0->a = uvrgb1->a = alpha; uvrgb0++; uvrgb1++;
			uvrgb0->a = uvrgb1->a = alpha; uvrgb0++; uvrgb1++;
			uvrgb0->a = uvrgb1->a = alpha; uvrgb0++; uvrgb1++;
			uvrgb0->a = uvrgb1->a = alpha; uvrgb0++; uvrgb1++;

			alpha = alpha / 2;
			uvrgb2->a = alpha; uvrgb2++;
			uvrgb2->a = alpha; uvrgb2++;
			uvrgb2->a = alpha; uvrgb2++;
			uvrgb2->a = alpha; uvrgb2++;
*/

			fvdata->vy += SIZE_STEP;
			if( fvdata->vy > RIPPLE_SIZE_MAX ){
				prim_center->vx = work->boundary_min_x + work->boundary_width_x*rnd();
				prim_center->vz = work->boundary_min_z + work->boundary_width_z*rnd();
				fvdata->vy = SIZE_RANDAM*rnd() + SIZE ;
			}

			prim_center++;
			fvdata++;
		}
	}

}

static void Die( Work *work )
{
	int	i;
	DG_PRIM2 **prim;

	prim=work->prim;
	for( i=0; i<ADD_SUB; i++ ){
		*prim = OK_FreePrim2( *prim );
		prim++;
	}

	work->a_prim = OK_FreePrim2( work->a_prim );
}


/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int add_sub )
{
	FVECTOR			*pos0 ;
	FVECTOR			*pos1 ;
	DG_PRIM2_UVRGB	*uvrgb0 ;
	DG_PRIM2_UVRGB	*uvrgb1 ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	if( add_sub==ADD_TRANS ){
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		prim->raise = RAISE;
	}else{
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		prim->raise = RAISE/2;
	}

	pos0   = prim->pos[0] ;
	pos1   = prim->pos[1] ;
	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i = 0 ; i < work->n_prim ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			pos0->vy = pos1->vy = work->height;	pos0++;	pos1++;
			pos0->vy = pos1->vy = work->height;	pos0++;	pos1++;
			pos0->vy = pos1->vy = work->height;	pos0++;	pos1++;
			pos0->vy = pos1->vy = work->height;	pos0++;	pos1++;


			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++; uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++; uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++; uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->r = uvrgb1->r = COLOR_R ;
			uvrgb0->g = uvrgb1->g = COLOR_G ;
			uvrgb0->b = uvrgb1->b = COLOR_B ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++; uvrgb1++;
		}
	}

	return 1;
}


static int GetResources( Work *work, int name, int where )
{
	int			j,k,l;
	DG_PRIM2	**prim ;
	DG_TEX		*tex ;
	float		ftemp0;
	float		ftemp1;
	float		ftemp2;
	float		ftemp3;
	float		th;
	FVECTOR		*fvdata;
	FVECTOR		*prim_center;

	work->name = name;
	work->map = where;

	work->invisible_flag = 0;

	/* プリム数 */
	work->n_prim = 1;
	if ( GCL_GetOption( 'n' ) != NULL ) {
		work->n_prim = GCL_GetNextInt() ;
		if( work->n_prim > N_PRIMS_MAX ) work->n_prim = N_PRIMS_MAX;
	}

	/* 縦横幅 */
	if ( GCL_GetOption( 'h' ) != NULL ) {
		work->height = (float)GCL_GetNextInt() + 20.0f ;
	}else{
		printf("GCL:ERR!::set the ripple height\n");
		return -1;
	}

	/* 設置座標 */
	if ( GCL_GetOption( 'b' ) != NULL ) {
		ftemp0 = (float)GCL_GetNextInt() ;
		ftemp1 = (float)GCL_GetNextInt() ;
		ftemp2 = (float)GCL_GetNextInt() ;
		ftemp3 = (float)GCL_GetNextInt() ;
		if( ftemp2 > ftemp0 ){
			work->boundary_min_x   = ftemp0;
			work->boundary_width_x = ftemp2 - ftemp0;
		}else{
			work->boundary_min_x   = ftemp2;
			work->boundary_width_x = ftemp0 - ftemp2;
		}
		if( ftemp3 > ftemp1 ){
			work->boundary_min_z   = ftemp1;
			work->boundary_width_z = ftemp3 - ftemp1;
		}else{
			work->boundary_min_z   = ftemp3;
			work->boundary_width_z = ftemp1 - ftemp3;
		}

//printf("%f %f %f %f\n",work->boundary_min_x,work->boundary_min_z,work->boundary_width_x,work->boundary_width_z);

		// バンダリチェック用
		work->boundary_data[0].vx = ftemp0;
		work->boundary_data[0].vy = work->height;
		work->boundary_data[0].vz = ftemp1;

		work->boundary_data[1].vx = ftemp2;
		work->boundary_data[1].vy = work->height;
		work->boundary_data[1].vz = ftemp1;

		work->boundary_data[2].vx = ftemp0;
		work->boundary_data[2].vy = work->height;
		work->boundary_data[2].vz = ftemp3;

		work->boundary_data[3].vx = ftemp2;
		work->boundary_data[3].vy = work->height;
		work->boundary_data[3].vz = ftemp3;

	}else{
		printf("GCL:ERR!::set the ripple boundary\n");
		return -1;
	}




	tex = DG_GetTexture( TEXTURE );
	prim  = work->prim;
	for( j=0; j<ADD_SUB; j++ ){
		(*prim) = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_prim, N_VERTS );
		if( (*prim) == NULL){
			printf("null prim:ripple.c:a\n");
			return -1;
		}
		InitPacket2( work, (*prim), tex, j );
		DG_VisiblePrim2( *prim ) ;
		GM_GroupPrim2( *prim, work->map ) ;
		prim++;
	}

	tex = DG_GetTexture( A_TEXTURE );
	work->a_prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, work->n_prim, N_VERTS );
	if(work->a_prim==NULL){
		printf("null prim:ripple.c:b\n");
		return -1;
	}
	InitPacket2( work, work->a_prim, tex, ADD_TRANS );
//	InitPacket2( work, work->a_prim, tex, SUB_TRANS );



	DG_VisiblePrim2( work->a_prim ) ;
//	DG_InvisiblePrim2( work->a_prim ) ;


	GM_GroupPrim2( work->a_prim, work->map ) ;


	fvdata = work->fvdata;
	prim_center = work->prim_center;
	for ( k=0; k<work->n_prim; k++ ){
		for ( l=0; l<N_POLYS; l++ ){
			prim_center->vx = work->boundary_min_x + work->boundary_width_x*rnd();
			prim_center->vy = work->height;
			prim_center->vz = work->boundary_min_z + work->boundary_width_z*rnd();

			th         = rnd() * 3.14159265f * 2.0f;
			fvdata->vx = sinf(th);
//			fvdata->vy = SIZE_RANDAM*((float)(l)/(float)(N_POLYS)) + SIZE ;
			fvdata->vy = RIPPLE_SIZE_MAX * rnd() ;
			fvdata->vz = cosf(th);
//			fvdata->vw = 0.30f;	/* シフト幅率（対サイズ） */
			fvdata->vw = 0.030f;	/* シフト幅率（対サイズ） */

			prim_center++;
			fvdata++;
		}
	}

	work->fvdata_fix.vx = COLOR_A / (RIPPLE_SIZE_MAX*RIPPLE_SIZE_MAX);	/* ２乗でインパクト */
	work->fvdata_fix.vy = RIPPLE_SIZE_MAX;
	work->fvdata_fix.vz = ALPHA_RATIO;	/* ３枚目のαは、１・２枚目のαの何倍か */

	return 0 ;
}


void *NewRipple( int name, int where )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
