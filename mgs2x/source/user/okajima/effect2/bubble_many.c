//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bubble_many.c
	泡集合
	2000/12/25 S.Okajima
	$Id: bubble_many.c,v 1.1.1.3 2002/11/19 11:47:18 Yoshizawa1 Exp $

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

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"./bubble.h"

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
#define	SCR_UVR		(SCRPAD_ADDR + 0x3000)
//#define N_PRIMS		(16)
#define N_PRIMS		(8)	/* w11a で 厳しいので */
#define N_VERTS		(32)
#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	RAISE				(0)

#define	INIT_PART_NUM		(16)

#define	FREQUENCY_RATIO	(0.02f)

//#define	COL_A	(32)
//#define	COL_A	(16)
#define	COL_A	(BUBBLE_ALPHA)

/*----------------------------------------------------------------*/
static int OK_BubbleColor;

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
typedef	struct Work_bubble_many_t	{  //BP_GENERAL - added type so we can debug on pc/x360
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;

	FVECTOR		*center;
	FVECTOR		before_center;

	int			total_num;
	int			life;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i;
	int		clock;
	int		before_num;
	int		temp_count;
	int		i_alpha;
	float	alpha;
	float	surface;
	float	ratio;
	float	*p_randam;
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	center;
	FVECTOR	before_center;
	FVECTOR	diff_vec;
	FVECTOR	fvtemp;
	DG_PRIM2	*prim ;

//printf("%d\n",work->life);

	if( work->center==NULL ){
		GV_DestroyActor( work ) ;
		return;
	}

   bp_math_assert( BP_Vec3_Check(work->center) );  //BP_MATH - check valid

	prim = work->prim;

	DG_VisiblePrim2( prim );

	surface = GM_WaterLevel;

	OK_frnd_to_scr( SCR_RND, RANDAM_FIELD_NUM );
	p_randam = (float *)SCR_RND;

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;


	DG_COPY_VEC( &center, work->center );
	DG_COPY_VEC( &before_center, &work->before_center );

	_sceVu0SubVector( &diff_vec, &center, &before_center ) ;

	if( (center.vy        < surface)
	 && (before_center.vy < surface) ){ // 前後の開始点共に水面より下
//	if( (GV_Time&3) < 2 ){
		before_num = work->total_num;
		work->total_num += INIT_PART_NUM;
		if( work->total_num > N_PRIMS*N_VERTS ){
			temp_count = N_PRIMS*N_VERTS - before_num;
			work->total_num = N_PRIMS*N_VERTS;
		}else{
			temp_count = INIT_PART_NUM;
		}
		OK_Mem_Scr( SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), work->total_num );
		if( before_num < N_PRIMS*N_VERTS ){
			pos  = SCR_POS;
			pos += before_num;
			for( i=0; i<temp_count; i++ ){
				float	size = rnd();
				ratio = rnd();
				
				size*= size*size;
				pos->vx  = before_center.vx + diff_vec.vx*ratio;
				pos->vy  = before_center.vy + diff_vec.vy*ratio;
				pos->vz  = before_center.vz + diff_vec.vz*ratio;
				pos->vw = BUBBLE_SIZE_MIN + size*BUBBLE_SIZE_RAND;
				pos++;
			}
		}
	}else if(work->total_num!=0){
		OK_Mem_Scr( SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), work->total_num );
	}


	// update
	if( work->life < COL_A ){
		alpha = (float)work->life;
	}else{
		alpha = (float)COL_A;
	}
	if( alpha < 0.0f ) alpha = 0.0f;

	i_alpha = (int)alpha;

//printf("i_alpha:%d\n",i_alpha);

	uvrgbwh        = prim->uvrgb[clock];
	pos                = SCR_POS;
	for( i=0; i<work->total_num; i++ ){
		if( pos->vw > 0.0f ){
			pos->vx += pos->vw * (*(p_randam++));
			pos->vy += pos->vw * BUBBLE_SPEED;
			pos->vz += pos->vw * (*(p_randam++));

			uvrgbwh->a = i_alpha;

			if( (pos->vy + pos->vw) > surface ){
				uvrgbwh->a = 0;
				uvrgbwh->w = uvrgbwh->h = 0;
				pos->vw = -10.0f;
				if( rnd() < FREQUENCY_RATIO ){
					if( pos->vy - (BUBBLE_SIZE_MIN + BUBBLE_SIZE_RAND) < surface ){
						extern int OK_PutRipple( FVECTOR *center );
						DG_COPY_VEC( &fvtemp, pos );
						fvtemp.vy = surface;
						OK_PutRipple( &fvtemp );
					}
				}
			}

		}else{
			uvrgbwh->a = 0;
			uvrgbwh->w = uvrgbwh->h = 0;
		}

		pos++;
		uvrgbwh++;
	}

	if( work->total_num!=0 ){
		OK_Scr_Mem( prim->pos[  clock], SCR_POS, sizeof(FVECTOR), work->total_num );
	}


	if( work->life-- < 0) GV_DestroyActor( work ) ;

	DG_COPY_VEC( &work->before_center, &center );

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int col )
{
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i;
	int		isize;
	int		col_r;
	int		col_g;
	int		col_b;
	float	size;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );

	pos       = SCR_POS;
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		size = rnd();
		size*= size*size;
		pos->vw = BUBBLE_SIZE_MIN + size*BUBBLE_SIZE_RAND;
		pos ++;
	}
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS*N_VERTS);


	col_r = (col>>24)&0xff;
	col_g = (col>>16)&0xff;
	col_b = (col>> 8)&0xff;

//printf("%d %d %d\n",col_r,col_g,col_b);

	pos       = prim->pos[ 0 ];
	uvrgbwh   = SCR_POS;
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		isize  = (int)pos->vw;
		pos->vw = -10.0f;
//printf("isize:%d\n",isize);
		uvrgbwh->w  = isize;
		uvrgbwh->h  = isize;
		uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		uvrgbwh->r  = col_r ;
		uvrgbwh->g  = col_g ;
		uvrgbwh->b  = col_b ;
		uvrgbwh->a  = 0 ;

		pos ++;
		uvrgbwh ++ ;
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_POS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);

	return 1;
}
/*----------------------------------------------------------------*/
static int GetResources( Work *work, int col )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			color = col;

	if( OK_BubbleColor!=0 ){
		color = OK_BubbleColor;
	}else if( !color ){
		color = ((BUBBLE_COL_R<<24)|(BUBBLE_COL_G<<16)|(BUBBLE_COL_B<<8));
	}

	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	InitPacket( work, prim, tex, color );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	work->total_num = 0;

	return 0 ;
}

/*
座標追従型
*/
void *NewBubbleMany( FVECTOR *center, int life, int col )
{
	Work		*work ;

	OPERATOR() ;

   bp_math_assert( BP_Vec3_Check(center) );  //BP_MATH - check valid

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->center = center;
		DG_COPY_VEC( &work->before_center, work->center );
		work->life = DIRECT_TICK( life + COL_A );

//AN_Test_Eye2( center, 2 );

		if ( GetResources( work, col ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void NewBubbleManyColorSet( void )
{
	if ( GCL_NextStr() == NULL ) return ;
	OK_BubbleColor = GCL_GetNextInt() ;
}






//デモ用
/*----------------------------------------------------------------*/
static int GetResources_Demo( Work *work, int col )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			color = col;
	
	if( !color ){
		color = ((BUBBLE_COL_R<<24)|(BUBBLE_COL_G<<16)|(BUBBLE_COL_B<<8));
	}
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	InitPacket( work, prim, tex, color );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	work->total_num = 0;

	return 0 ;
}

//デモ用
/*
座標追従型
*/
void *NewBubbleMany_Demo( FVECTOR *center, int life, int col )
{
	Work		*work ;
/*
printf("NewBubbleMany_Demo::::::::::::::::::::::::::::::::::::::\n");
printf("%f %f %f\n",center->vx,center->vy,center->vz);
printf("%d\n",life);
printf("%x\n",col);
*/

	OPERATOR() ;

   bp_math_assert( BP_Vec3_Check(center) );  //BP_MATH - check valid

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->center = center;
		DG_COPY_VEC( &work->before_center, work->center );
		work->life = DIRECT_TICK( life + COL_A );

		if ( GetResources_Demo( work, col ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

