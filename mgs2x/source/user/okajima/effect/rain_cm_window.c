//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	rain_cm_window.c
	窓に飛び散る雨滴
	2000/02/05 S.Okajima
	$Id: rain_cm_window.c,v 1.1.1.3 2002/11/19 11:47:09 Yoshizawa1 Exp $
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

#include "bp_vector.h"

extern int		G_wind_sw;
extern SVECTOR	G_wind_rot;
extern FVECTOR	OK_rain_fall_vec;
extern SVECTOR	OK_rain_fall_rot;

/*----------------------------------------------------------------*/
#define	SCREEN_NEAR	( 51.0f )

#define	ADD_TRANS	(0)
#define	SUB_TRANS	(1)

/* 固定 */
#define	ADD_SUB		(2)

#define	MAX_TEX_NUM	(2)

#define	SCR_POS0	(SCRPAD_ADDR)
#define	SCR_POS1	(SCRPAD_ADDR + 0x2000)

#define	N_VERTS			(64)
#define	N_POLYS			(N_VERTS / 4)
#define	N_PRIMS			(4)



#define	LARGE_NUMBER	(32000*1000)

/* 画面最大距離の１／Xから */
#define	INFLUENCE_DIVIDE	(2.0f)
#define	MAX_DISTANCE_XY	( (int)(bp_sqrtf( (float)(DRAW_WIDTH * DRAW_WIDTH) + (float)(DRAW_HEIGHT * DRAW_HEIGHT))/INFLUENCE_DIVIDE) )   //BP_MATH - emulate PS2 sqrtf

#define	LIGHT_POS_NUM_MAX	(64)

#define	SLIDE_SPEED			(0.03f)
#define	SLIDE_SPEED_SLOW	(0.02f)

#define	INTERVAL		(200)

#define	LIFE_TIME		(90)

#define	COLOR_R			(255)
#define	COLOR_G			(255)
#define	COLOR_B			(255)
//#define	COLOR_A			(12)
#define	COLOR_A			(16)


#define	SHIFT			(5.0f)
//#define	SIZE			(10.0f)
#define	SIZE			(30.0f)
#define	SIZE_RANDAM		(10.0f)

#define	TEXTURE		( 4642619 /*"drop02_msk"*/ )

#define	OK_ASPECT_RATIO			( 0.60f )

#define	PURUPURU_PAT			( 8 )


typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	FVECTOR		center;
	SVECTOR		rot;
	FMATRIX		world;
	FMATRIX		world_inv;

	DG_PRIM2	*prim[ADD_SUB] ;
	float		size[ N_PRIMS * N_POLYS];
	float		pos_x[N_PRIMS * N_POLYS];
	float		pos_y[N_PRIMS * N_POLYS];

	float		purupuru_x[PURUPURU_PAT];
	float		purupuru_y[PURUPURU_PAT];

	float		height;
	float		width;

} Work ;

/*----------------------------------------------------------------*/
static inline float OK_InnerProduct( FVECTOR *a, FVECTOR *b)
{
    float ans;
#ifdef BP_PSX2_ASM
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
#else
	ans = BP_Vec3_InnerProduct(a,b);
#endif
    return ans;
}
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		k,l;
	float	*size;
	float	*pos_x;
	float	*pos_y;

	float	shift_x;
	float	shift_y;

	SVECTOR	rot;
	FVECTOR	fvtemp1;
	FVECTOR	fvtemp2;
	int	cycle;
	int	clock;
	float	intense;
	DG_PRIM2			**prim;
	DG_PRIM2			**prim_keep;
	FVECTOR				*pos0_0;
	FVECTOR				*pos0_1;
	FVECTOR				*pos0_2;
	FVECTOR				*pos0_3;
	FVECTOR				*pos1_0;
	FVECTOR				*pos1_1;
	FVECTOR				*pos1_2;
	FVECTOR				*pos1_3;

   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }
	clock = 1 - work->prim[0]->buffer_clock;
	prim = work->prim;
	for( k=0; k<ADD_SUB; k++ ){
		DG_SwitchBuffPrim2( *prim );
		GM_GroupPrim2( *prim, GM_CurrentStageMap ) ;
		prim++;
	}

	//fvtemp1:窓面の法線
	fvtemp1.vx= 0.0f;
	fvtemp1.vy= 0.0f;
	fvtemp1.vz= 1.0f;
	DG_SetPos( &work->world );
	DG_RotVector( &fvtemp1, &fvtemp1, 1 );

//printf("%d %d %d\n",OK_rain_fall_rot.vx,OK_rain_fall_rot.vy,OK_rain_fall_rot.vz);

	//fvtemp2:風向単位ベクトル（正負逆）
	rot.vx=OK_rain_fall_rot.vx;
	rot.vy=OK_rain_fall_rot.vy;
	rot.vz=0;
	fvtemp2.vx= 0.0f;
	fvtemp2.vy= 0.0f;
	fvtemp2.vz=-1.0f;
	DG_SetPos2( &DG_ZeroVector, &rot );
	DG_RotVector( &fvtemp2, &fvtemp2, 1 );

	//intense:風向の、窓面法線への成分比
	intense = OK_InnerProduct( &fvtemp1, &fvtemp2 );
	intense = (intense>0)?intense:-intense;

	//fvtemp1:風向の、窓面法線への成分
	fvtemp1.vx *= intense;
	fvtemp1.vy *= intense;
	fvtemp1.vz *= intense;

	//fvtemp2:風向の、窓面への投射
	fvtemp2.vx = fvtemp1.vx - fvtemp2.vx;
	fvtemp2.vy = fvtemp1.vy - fvtemp2.vy;
	fvtemp2.vz = fvtemp1.vz - fvtemp2.vz;

	//fvtemp2:窓相対へ変換（Ｘ，Ｙが画面の縦横座標になる）
	DG_SetPos( &work->world_inv );
	DG_RotVector( &fvtemp2, &fvtemp2, 1 );

	//以上で、
	//intense が 強度（負だと風向き逆方向）、fvtemp2.vx,fvtemp2.vy が 画面の中心からの位相強度
	//いづれも１が最大


//printf("%f %f %f %f\n",fvtemp2.vx,fvtemp2.vy,fvtemp2.vz,intense);

	cycle = GV_Time % (PURUPURU_PAT*2)+irnd()%PURUPURU_PAT/2;
	if(cycle>=PURUPURU_PAT) cycle -= PURUPURU_PAT;
	cycle = (cycle<PURUPURU_PAT)?cycle:PURUPURU_PAT*2-1-cycle;
	shift_x = SHIFT * work->purupuru_x[cycle];
	shift_y = SHIFT * work->purupuru_y[cycle];

	pos_x   = work->pos_x;
	pos_y   = work->pos_y;
	/* 更新座標を計算 */
	pos0_0     = SCR_POS0;
	for ( k=0; k<N_PRIMS; k++ ){
		for ( l=0; l<N_POLYS; l++ ){
			if( (*pos_x) > 1.0f  ||  (*pos_x) <-1.0f  ||  (*pos_y) > 1.0f  ||  (*pos_y) <-1.0f ){
				(*pos_x) = frnd();
				(*pos_y) =  rnd();
			}else if( (l<(irnd()>>8)%8) || (G_wind_sw && (irnd()>>8)%4==0) ){
				(*pos_x) += SLIDE_SPEED      * fvtemp2.vx;
				(*pos_y) += SLIDE_SPEED      * fvtemp2.vy;
			}else{
				(*pos_x) += SLIDE_SPEED_SLOW * fvtemp2.vx;
				(*pos_y) += SLIDE_SPEED_SLOW * fvtemp2.vy;
			}
			pos0_0->vx = (*pos_x) * work->width ;
			pos0_0->vy = (*pos_y) * work->height ;
			pos0_0->vz = 0.0f;	/* 窓面にピッタシ */

			pos0_0 += 4;	/* 先頭頂点を計算している */
			pos_x++;
			pos_y++;
		}
	}

	prim = work->prim;
	prim_keep = prim;
	prim++;
	OK_Mem_Scr( SCR_POS1, (*prim)->pos[ clock ],        sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;
	size  = work->size;
	pos0_0 = SCR_POS0;	/* 上で更新した座標を参照 */
	pos0_1 = SCR_POS0;	pos0_1++;
	pos0_2 = SCR_POS0;	pos0_2+=2;
	pos0_3 = SCR_POS0;	pos0_3+=3;
	pos1_0 = SCR_POS1;
	pos1_1 = SCR_POS1;	pos1_1++;
	pos1_2 = SCR_POS1;	pos1_2+=2;
	pos1_3 = SCR_POS1;	pos1_3+=3;
	for ( k=0; k<N_PRIMS; k++ ){
		for ( l=0; l<N_POLYS; l++ ){
			pos0_1->vx = pos0_0->vx + (*size);
			pos0_1->vy = pos0_0->vy;
			pos0_1->vz = pos0_0->vz;

			pos0_2->vx = pos0_0->vx;
			pos0_2->vy = pos0_0->vy - (*size);
			pos0_2->vz = pos0_0->vz;

			pos0_3->vx = pos0_0->vx + (*size);
			pos0_3->vy = pos0_0->vy - (*size);
			pos0_3->vz = pos0_0->vz;


			pos1_0->vx = pos0_0->vx + shift_x;
			pos1_0->vy = pos0_0->vy + shift_y;
			pos1_0->vz = pos0_0->vz;

			pos1_1->vx = pos0_1->vx + shift_x;
			pos1_1->vy = pos0_1->vy + shift_y;
			pos1_1->vz = pos0_1->vz;

			pos1_2->vx = pos0_2->vx + shift_x;
			pos1_2->vy = pos0_2->vy + shift_y;
			pos1_2->vz = pos0_2->vz;

			pos1_3->vx = pos0_3->vx + shift_x;
			pos1_3->vy = pos0_3->vy + shift_y;
			pos1_3->vz = pos0_3->vz;

			size++;
			pos0_0+=4; pos0_1+=4; pos0_2+=4; pos0_3+=4;
			pos1_0+=4; pos1_1+=4; pos1_2+=4; pos1_3+=4;
		}
	}
	/* 雨粒の頂点を窓面に合わせる */
	DG_SetPos( &work->world );
	DG_PutVector( SCR_POS0, SCR_POS0, N_PRIMS * N_VERTS );
	DG_PutVector( SCR_POS1, SCR_POS1, N_PRIMS * N_VERTS );

	OK_Scr_Mem( (*prim_keep)->pos[   clock ], SCR_POS0, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( (*prim     )->pos[   clock ], SCR_POS1, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;

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
}


/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int add_sub )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	if( add_sub==ADD_TRANS ){
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else{
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}

	pos   = SCR_POS0 ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos, &work->center );	pos++;
			DG_COPY_VEC( pos, &work->center );	pos++;
			DG_COPY_VEC( pos, &work->center );	pos++;
			DG_COPY_VEC( pos, &work->center );	pos++;
		}
	}
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS0, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS0, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;


	uvrgb = SCR_POS0 ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = COLOR_A ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = COLOR_A ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = COLOR_A ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = COLOR_A ;
			uvrgb++;
		}
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_POS0, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_POS0, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS ) ;

	return 1;
}


static int GetResources( Work *work, int name, int where )
{
	int			i,j,k,l;
	DG_PRIM2	**prim ;
	DG_TEX		*tex ;
	float		ftemp;
	float		*size;
	float	*pos_x;
	float	*pos_y;

	/* 設置座標 */
	if ( GCL_GetOption( 'c' ) != NULL ) {
		work->center.vx = (float)GCL_GetNextInt() ;
		work->center.vy = (float)GCL_GetNextInt() ;
		work->center.vz = (float)GCL_GetNextInt() ;
		work->center.vw = 1.0f ;
	}else{
		printf("GCL:ERR!::c_light_spot::set option C\n");
		return -1;
	}

	/* 縦横幅 */
	if ( GCL_GetOption( 's' ) != NULL ) {
		work->height = (float)GCL_GetNextInt() ;
		work->width = (float)GCL_GetNextInt() ;
	}else{
		printf("GCL:ERR!::c_light_spot::set option S\n");
		return -1;
	}

	/* 回転 */
	if ( GCL_GetOption( 'r' ) != NULL ) {
		work->rot.vx = GCL_GetNextInt() ;
		work->rot.vy = GCL_GetNextInt() ;
		work->rot.vz = GCL_GetNextInt() ;
	}else{
		printf("GCL:ERR!::c_light_spot::set option R\n");
		return -1;
	}

	DG_SetPos2( &work->center, &work->rot );
	DG_GetPos( &work->world );
	FastInverseMatrix( &work->world_inv, &work->world );

	work->name = name;
	work->map = where;

	tex = DG_GetTexture( TEXTURE );

	prim  = work->prim;
	for( j=0; j<ADD_SUB; j++ ){
		*prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
//printf("XXXXXXXXX:%x\n",*prim);
		if(*prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacket2( work, *prim, tex, j );
		DG_RaisePrim2( *prim, -(j*10000+10000) );
		DG_VisiblePrim2( *prim ) ;
		prim++;
	}

	size  = work->size;
	pos_x   = work->pos_x;
	pos_y   = work->pos_y;
	for ( k=0; k<N_PRIMS; k++ ){
		for ( l=0; l<N_POLYS; l++ ){
			(*size) = SIZE_RANDAM*((float)(l)/(float)(N_POLYS)) + SIZE ;
			(*pos_x) = rnd() * 2.0f - 1.0f;
			(*pos_y) = rnd() * 2.0f - 1.0f;
			size++;
			pos_x++;
			pos_y++;
		}
	}

	for( i=0; i<PURUPURU_PAT; i++ ){
		ftemp = 3.14159265f * 1.0f * (float)(i) / (float)(PURUPURU_PAT);
		work->purupuru_x[i]= -cosf(ftemp);
		work->purupuru_y[i]=  sinf(ftemp);
	}

	return 0 ;
}



void *NewRainWindow( int name, int where )
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
