//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rain_cm.c
	カメラ画面に飛び散る雨
	1999/11/06 S.Okajima
	$Id: d_rain_cm.c,v 1.1.1.3 2002/11/19 11:46:54 Yoshizawa1 Exp $
*/
//asm 複数関数でレジスタの値を参照　後で移植
/*

加算減算でそれぞれ dg_prim2 をもつ。
別テクスチャはこれらを別個にもつ。

２つのテクスチャを使用する場合、４つの dg_prim2 となる。

*/


#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"gameheader.h"
#include	"libmt.h"
#include	"libutl.h"
#include	"../etc/ok_util.h"

extern int		G_wind_sw;
extern FVECTOR	OK_rain_fall_vec;
extern SVECTOR	OK_rain_fall_rot;
extern float	OK_rain_percentage;

/*----------------------------------------------------------------*/
#define	RATIO		( 4096 )

#define	SCREEN_NEAR	( 51.0f )

#define	ADD_TRANS	(0)
#define	SUB_TRANS	(1)

/* 固定 */
#define	ADD_SUB		(2)

#define	MAX_TEX_NUM	(2)

#define	R_POS		(1)
#define	R_UVS		(2)

#define	SCR_LENGTH	( 0x4000 )
#define	MAX_FVECTOR	( SCR_LENGTH/16 )

#define	PARAM1		(MAX_FVECTOR / (R_POS + R_UVS  +  R_POS + R_UVS))
#define	PARAM2		(PARAM1 * 16)

#define	SCR_POS0	(SCRPAD_ADDR)
#define	SCR_POS1	(SCR_POS0 + PARAM2)
#define	SCR_UVS0	(SCR_POS1 + PARAM2 )
#define	SCR_UVS1	(SCR_UVS0 + PARAM2 + PARAM2 )



/* スプライト（回転無し）の頂点（中心と読み替え）は３２個まで */
#define	N_VERTS2			(32)
#define	N_PRIMS2			(1)



#define	LARGE_NUMBER	(32000*1000)

/* 画面最大距離の１／Xから */
#define	INFLUENCE_DIVIDE	(2.0f)
#define	MAX_DISTANCE_XY	( (int)(bp_sqrtf( (float)(DRAW_WIDTH * DRAW_WIDTH) + (float)(DRAW_HEIGHT * DRAW_HEIGHT))/INFLUENCE_DIVIDE) )   //BP_MATH - emulate PS2 sqrtf

#define	LIGHT_POS_NUM_MAX	(64)

#define	SLIDE_SPEED			(0.4f)
#define	SLIDE_SPEED_SLOW	(0.15f)

//#define	INTERVAL		(120)
#define	INTERVAL		(2)

#define	MAX_UNIT		(64)

#define	LIFE_TIME		(90)

#define	COLOR_R			(127)
#define	COLOR_G			(127)
#define	COLOR_B			(127)
#define	COLOR_A			(32)
#define	ALPHA_SP		(64)



#define	SHIFT			(1.0f)
#define	SIZE			(4.0f)
#define	SIZE_RANDAM		(2.0f)

#define	TEXTURE1		( 4642619 /*"drop02_msk"*/ )
#define	TEXTURE2		( 3594043 /*"drop01_msk"*/ )



#define	OK_ASPECT_RATIO			( 0.60f )

#define	PURUPURU_PAT			( 8 )

#include	"../effect/rain_cm.h"

/*----------------------------------------------------------------*/
static inline void _SetMatrix( FMATRIX *m )
{
#ifdef BP_PSX2_ASM
	asm volatile("
	lqc2		vf4,0x00(%0)
	lqc2		vf5,0x10(%0)
	lqc2		vf6,0x20(%0)
	lqc2		vf7,0x30(%0)
	": : "r"(m) );
#endif
}


#ifdef BP_PSX2_ASM
static inline void _RotTrans( FVECTOR *res, FVECTOR *v )
{
	asm volatile("
	lqc2			vf8,0x00(%1)
	vmulax.xyzw		ACC, vf4,vf8
	vmadday.xyzw	ACC, vf5,vf8
	vmaddaz.xyzw	ACC, vf6,vf8
	vmaddw.xyzw		vf8, vf7,vf0
	sqc2			vf8,0x00(%0)
	":: "r"(res),"r"(v):"memory" );
}
#else
static inline void _RotTrans(FVECTOR *res ,FVECTOR *vec ,FMATRIX *world ){
	/*_SetMatrixの機能も兼ねる*/
	res->vx = (world->m[0][0]*vec->vx)+(world->m[1][0]*vec->vy)+(world->m[2][0]*vec->vz) ;
	res->vy = (world->m[0][1]*vec->vx)+(world->m[1][1]*vec->vy)+(world->m[2][1]*vec->vz) ;
	res->vz = (world->m[0][2]*vec->vx)+(world->m[1][2]*vec->vy)+(world->m[2][2]*vec->vz) ;
	res->vw = (world->m[0][3]*vec->vx)+(world->m[1][3]*vec->vy)+(world->m[2][3]*vec->vz) ;
}
#endif


#if 0 //BP_UNUSED

#ifdef BP_PSX2_ASM
static inline float OK_InnerProduct( FVECTOR *a, FVECTOR *b)
{
    float ans;
    asm volatile ("
    lwc1	$f1,0(%1)
    lwc1	$f2,0(%2)
    lwc1	$f3,4(%1)
    lwc1	$f4,4(%2)
    lwc1	$f5,8(%1)
    lwc1	$f6,8(%2)
    mula.s	$f1,$f2
    madda.s	$f3,$f4
    madd.s	%0,$f5,$f6
    " : "=f"(ans) : "r"(a), "r"(b) : "$f1","$f2","$f3","$f4","$f5","$f6");
    return ans;
}
#endif

#endif

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
		  default:
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		rnd_num;
	int		i,j,k,l;
	int		dis, dis_temp;
	int		*size;
	int		*count;
	float	*pos_x;
	float	*pos_y;

	float	shift_x;
	float	shift_y;
	float	light_x;
	float	light_y;
	float	lx;
	float	ly;

	float	screen;
	float	ftemp;

	FVECTOR	fvtemp1;
   FVECTOR	fvtemp2 /* BP */ = { 0 };
	FVECTOR	*pfvec;
	int	cycle;
	int	clock;
	float	screen_adjust;
	DG_PRIM2			**prim;
	DG_PRIM2			**prim_keep;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	FVECTOR				*pos0;
	FVECTOR				*pos1;
	int		activate_polys;
	int		dis_act_polys;



	CheckMesgParam( work );


	screen = DG_Chanls->screen;
	screen = (screen > 4.0f)? 4.0f: screen;
	work->screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / screen;
	work->screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / screen;

/* 画角が変化しても相対的な水滴の大きさはある程度維持する */
//	screen_adjust = 2.0f / ((2.0f - DG_Chanls->screen)*0.75f + DG_Chanls->screen) ;
	screen_adjust = 2.0f / DG_Chanls->screen ;
	if( screen_adjust > 2.0f ){
		screen_adjust = 2.0f;
	}else if( screen_adjust < 0.5f ){
		screen_adjust = 0.5f;
	}

   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }
	clock = 1 - work->prim[0]->buffer_clock;

	if( work->invisible_flag ){
		prim  = work->prim;
		count = work->count;
		for( i=0; i<MAX_TEX_NUM; i++ ){
			for( j=0; j<ADD_SUB; j++ ){
				DG_SwitchBuffPrim2( *prim );
				DG_InvisiblePrim2( *prim ) ;
				prim++;
			}
			for ( k=0; k<N_PRIMS2; k++ ){
				for ( l=0; l<N_VERTS2; l++ ){
					*count=0;
					count++;
				}
			}
		}
		return;
	}else{
		prim = work->prim;
		for( i=0; i<MAX_TEX_NUM; i++ ){
			for( j=0; j<ADD_SUB; j++ ){
				DG_SwitchBuffPrim2( *prim );
				DG_VisiblePrim2( *prim ) ;
				GM_GroupPrim2( *prim, GM_CurrentStageMap ) ;
				prim++;
			}
		}
	}


	/* カメラに近い光源を反映させる */
	DG_COPY_VEC( &fvtemp1, (FVECTOR *)DG_Chanls->eye.m[3] );
	dis=LARGE_NUMBER;
	pfvec=work->lit_tex_pos;
	j=-1;
	for( i=0; i<work->lit_tex_pos_num; i++ ){

#ifdef BP_PSX2_ASM
		_SetMatrix( &DG_Chanls->eye_pers );
		_RotTrans( &fvtemp2, pfvec );
#else
		_RotTrans(&fvtemp2, pfvec ,&DG_Chanls->eye_pers ) ;
#endif

//printf("::::%f %f %f %f\n",fvtemp2.vx,fvtemp2.vy,fvtemp2.vz,fvtemp2.vw);

		if( fvtemp2.vz < fvtemp2.vw ){
			fvtemp2.vw = ( fvtemp2.vw >= 0.0f ) ? fvtemp2.vw : -fvtemp2.vw ;
			if ( fvtemp2.vx < fvtemp2.vw
			  && fvtemp2.vx >-fvtemp2.vw
			  && fvtemp2.vy < fvtemp2.vw
			  && fvtemp2.vy >-fvtemp2.vw ){
				_sceVu0SubVector( &fvtemp2, pfvec, &fvtemp1 ) ;
				fvtemp2.vw = 0.0f;
				dis_temp = (int)GV_VecLen3F( &fvtemp2 ) ;
//printf("dis_temp:%d\n",dis_temp);
				if( dis_temp < dis ){
					dis = dis_temp;
					j=i;
				}
			}
		}
		pfvec++;
	}

	/* 光源ありかなしか */
	light_x=0.0f;
	light_y=0.0f;
	if(j>=0){
//AN_Test_Eye2( &work->lit_tex_pos[j], 2 );
//printf("%f %f %f\n",work->lit_tex_pos[j].vx,work->lit_tex_pos[j].vy,work->lit_tex_pos[j].vz);

#ifdef BP_PSX2_ASM
		_SetMatrix( &DG_Chanls->eye_pers );
		_RotTrans( &fvtemp1, &work->lit_tex_pos[j] );
#else
		_RotTrans(&fvtemp1, &work->lit_tex_pos[j] ,&DG_Chanls->eye_pers ) ;
#endif


		fvtemp1.vw = ( fvtemp1.vw > 0.0f )? fvtemp1.vw: -fvtemp1.vw;
		light_x = work->screen_near_x * fvtemp1.vx / fvtemp1.vw;
		light_y = work->screen_near_y * fvtemp1.vy / fvtemp1.vw;
		work->presence_light=1;
	}else{
		work->presence_light=0;
	}


	cycle = GV_Time % (PURUPURU_PAT*2)+irnd()%PURUPURU_PAT/2;
	if(cycle>=PURUPURU_PAT) cycle -= PURUPURU_PAT;
	cycle = (cycle<PURUPURU_PAT)?cycle:PURUPURU_PAT*2-1-cycle;
	shift_x = SHIFT * work->purupuru_x[cycle];
	shift_y = SHIFT * work->purupuru_y[cycle];

	work->time--;

	ftemp = (float)(N_PRIMS2*N_VERTS2) * OK_rain_percentage;
	activate_polys = (int)ftemp;
	dis_act_polys  = N_PRIMS2*N_VERTS2 - activate_polys;

	prim    = work->prim;
	pos_x   = work->pos_x;
	pos_y   = work->pos_y;
	count   = work->count;
	for( i=0; i<MAX_TEX_NUM; i++ ){
		/* 更新座標 */
		pos0     = SCR_POS0;
		for ( k=0; k<N_PRIMS2; k++ ){
			for ( l=0; l<N_VERTS2; l++ ){
				if( (*count)>0 ){
					pos0->vw=1.0f;
					if( (l<4) || (G_wind_sw && irnd()%4==0) ){
						(*pos_x) += SLIDE_SPEED      * fvtemp2.vx;
						(*pos_y) += SLIDE_SPEED      * fvtemp2.vy;
					}else if( (irnd()>>8)%16==0 ){
						(*pos_x) += SLIDE_SPEED_SLOW * fvtemp2.vx;
						(*pos_y) += SLIDE_SPEED_SLOW * fvtemp2.vy;
					}
					if( (*pos_x) < -1.1f
					 || (*pos_y) < -1.1f
					 || (*pos_x) >  1.1f
					 || (*pos_y) >  1.1f ){
						(*count) = -1;
					}else{
						(*count)--;
					}
				}else{
					rnd_num = (irnd()>>8);
					if( (G_wind_sw==1 && rnd_num%16==0 ) || (G_wind_sw==0 && rnd_num%64==0 ) ){
						pos0->vw=1.0f;
						(*count) = (int)( (float)LIFE_TIME * (rnd()*0.5f + 0.5f) );
						(*pos_x) = frnd() * 1.1f;
						(*pos_y) = frnd() * 1.1f;
					}else if( work->time < 0 ){
						pos0->vw=2.0f;
						work->time = (irnd()>>8)%256;
						(*pos_x) = frnd();
						(*pos_y) = -rnd();
					}else{
						pos0->vw=0.0f;
					}
				}
				pos0->vx = (*pos_x) * work->screen_near_x ;
				pos0->vy = (*pos_y) * work->screen_near_y ;
				pos0->vz = SCREEN_NEAR;	/* 重要 */

				pos0++;
				pos_x++;
				pos_y++;
				count++;
			}
		}
		/* スクラッチパット内の座標の更新は終わり */
		/* --------------------------------------------------------*/

		prim_keep=prim;
		prim++;
		OK_Mem_Scr( SCR_UVS0, (*prim_keep)->uvrgb[ clock ], sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
		OK_Mem_Scr( SCR_UVS1, (*prim)->uvrgb[ clock ],      sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
		OK_Mem_Scr( SCR_POS1, (*prim)->pos[ clock ],        sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
		count = work->count;
		size  = work->size;
		pos0 = SCR_POS0;	/* 上で更新した座標を参照 */
		pos1 = SCR_POS1;	/* 更に計算する */
		uvrgbwh0 = SCR_UVS0;
		uvrgbwh1 = SCR_UVS1;
		if( work->presence_light ){
//printf("exist\n");
			for ( k=0; k<activate_polys; k++ ){
				if( pos0->vw==0.0f ){
					uvrgbwh1->a = uvrgbwh0->a = 0;
					uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = 0;
				}else if( pos0->vw==1.0f  &&  (irnd()>>8)%RATIO ){
					lx = pos0->vx - light_x;
					ly = pos0->vy - light_y;
					pos1->vx  = pos0->vx - lx*0.0625f - shift_x;
					pos1->vy  = pos0->vy - ly*0.0625f - shift_y;
					uvrgbwh1->a = uvrgbwh0->a = (*count) * COLOR_A / LIFE_TIME;
					uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = (*size) * screen_adjust;
				}else{
//printf("落ち:0\n");
					lx = pos0->vx - light_x;
					pos1->vx = pos0->vx - lx*0.0625f;
					pos1->vy = pos0->vy = 0.0f;
					uvrgbwh1->a = ALPHA_SP;	/* 減算は強め */
					uvrgbwh0->a = COLOR_A;
					uvrgbwh1->w = uvrgbwh0->w = (*size) * (rnd()+2.0f);
					uvrgbwh1->h = uvrgbwh0->h = (int)(work->screen_near_y*2.0f);
				}
				size++;
				count++;
				uvrgbwh0++;
				uvrgbwh1++;
				pos0++;
				pos1++;
			}
			for ( k=dis_act_polys; k>0; k-- ){
				uvrgbwh1->a = uvrgbwh0->a = 0;
				uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = 0;
				uvrgbwh0++;
				uvrgbwh1++;
			}
		}else{
			for ( k=0; k<activate_polys; k++ ){
				if( pos0->vw!=0.0f ){
					uvrgbwh1->a = uvrgbwh0->a = (*count) * COLOR_A / LIFE_TIME;
					uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = (*size) * screen_adjust;
					pos1->vx = pos0->vx + shift_x ;
					pos1->vy = pos0->vy + shift_y ;
				}else if( (irnd()>>8)%RATIO ){
					uvrgbwh1->a = uvrgbwh0->a = 0;
					uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = 0;
				}else{
//printf("落ち:1\n");
					pos1->vx = pos0->vx + shift_x;
					pos1->vy = pos0->vy = 0.0f;
					uvrgbwh1->a = ALPHA_SP;	/* 減算は強め */
					uvrgbwh0->a = COLOR_A;
					uvrgbwh1->w = uvrgbwh0->w = (*size) * (rnd()+2.0f);
					uvrgbwh1->h = uvrgbwh0->h = (int)(work->screen_near_y*2.0f);
				}
				size++;
				count++;
				uvrgbwh0++;
				uvrgbwh1++;
				pos0++;
				pos1++;
			}
			for ( k=dis_act_polys; k>0; k-- ){
				uvrgbwh1->a = uvrgbwh0->a = 0;
				uvrgbwh1->w = uvrgbwh0->w = uvrgbwh1->h = uvrgbwh0->h = 0;
				uvrgbwh0++;
				uvrgbwh1++;
			}
		}


		OK_Scr_Mem( (*prim_keep)->pos[   clock ], SCR_POS0, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( (*prim     )->pos[   clock ], SCR_POS1, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( (*prim_keep)->uvrgb[ clock ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
		OK_Scr_Mem( (*prim     )->uvrgb[ clock ], SCR_UVS1, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
		prim++;


#if 0
		for( j=0; j<ADD_SUB; j++ ){
			OK_Scr_Mem( (*prim)->pos[ clock ],   SCR_POS, sizeof(FVECTOR), N_VERTS2 * N_PRIMS2 ) ;
			prim++;
		}
#endif
	}

	if( work->life > 0 ){
		work->life--;
		if( work->life <= 0 ){
			GV_DestroyActor( work ) ;
		}
	}

}

static void Die( Work *work )
{
	int	i,j;
	DG_PRIM2 **prim;

	prim=work->prim;
	for( i=0; i<MAX_TEX_NUM; i++ ){
		for( j=0; j<ADD_SUB; j++ ){
			*prim = OK_FreePrim2( *prim );
			prim++;
		}
	}
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int add_sub )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	if( add_sub==ADD_TRANS ){
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else{
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}

	pos     = SCR_POS0 ;
	uvrgbwh = SCR_UVS0 ;
	for ( i = 0 ; i < N_PRIMS2 ; i++ ){
		for ( k = 0 ; k < N_VERTS2 ; k++ ){
			pos->vx = 0.0f ;
			pos->vy = 0.0f ;
			pos->vz = SCREEN_NEAR;

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

//			uvrgbwh->h = uvrgbwh->w = (int)( SIZE_RANDAM*((float)(k)/(float)(N_VERTS2)) + SIZE ) ;
			uvrgbwh->w = 0 ;
			uvrgbwh->h = 0 ;

			uvrgbwh->r = COLOR_R ;
			uvrgbwh->g = COLOR_G ;
			uvrgbwh->b = COLOR_B ;
			uvrgbwh->a = COLOR_A ;

			pos ++ ;		/* 同一プリミティブではデータは連続している */
			uvrgbwh ++ ;
		}
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS0, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS0, sizeof(FVECTOR),          N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGBWH), N_VERTS2 * N_PRIMS2 ) ;

	return 1;
}


static int GetResources( Work *work, int life )
{
	int			i,j,k,l;
	int			*size;
	int			*count;
	DG_PRIM2	**prim ;
	DG_TEX		*tex[MAX_TEX_NUM] ;
	float	ftemp;

	work->invisible_flag = 0 ;

	work->time=0;
	work->life=life;

	tex[0] = DG_GetTexture( TEXTURE1 );
	tex[1] = DG_GetTexture( TEXTURE2 );

	prim  = work->prim;
	count = work->count;
	size  = work->size;
	for( i=0; i<MAX_TEX_NUM; i++ ){
		for( j=0; j<ADD_SUB; j++ ){
			*prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS2, N_VERTS2 );
//printf("XXXXXXXXX:%x\n",*prim);
			if(*prim==NULL){
				printf("null prim\n");
				return -1;
			}
			InitPacket2( work, *prim, tex[i], j );
			(*prim)->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
			DG_RaisePrim2( *prim, j*100 );

			prim++;
		}
		for ( k=0; k<N_PRIMS2; k++ ){
			for ( l=0; l<N_VERTS2; l++ ){
				(*count)=0;
				(*size) = (int)( SIZE_RANDAM*((float)(k)/(float)(N_VERTS2)) + SIZE ) ;
				count++;
				size++;
			}
		}
	}

	for( i=0; i<PURUPURU_PAT; i++ ){
		ftemp = 3.14159265f * 1.0f * (float)(i) / (float)(PURUPURU_PAT);
		work->purupuru_x[i]= -cosf(ftemp);
		work->purupuru_y[i]=  sinf(ftemp);
	}

	return 0 ;
}

/*
life : 効果継続時間（時間後自殺する）:::-1で死なない
*/
void *NewRainCamera_Demo( int life )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = 0;

		if ( GetResources( work, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
/*
life : 効果継続時間（時間後自殺する）:::-1で死なない
*/
void *NewRainCamera_Demo_AddesMessageName( int name, int life )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;

		if ( GetResources( work, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
