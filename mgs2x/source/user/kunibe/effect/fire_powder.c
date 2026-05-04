//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	dust_cm.c
	カメラ前の空間にゴミ

	2000/12/28 S.Okajima
	$Id: fire_powder.c,v 1.1.1.3 2002/11/19 11:44:39 Yoshizawa1 Exp $
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
#include	"camera.h"
#include	"libmt.h"
#include	"../../okajima/etc/ok_util.h"

#define	RAISE			(0)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)


#define N_VERTS		(32)
#define N_PRIMS		(0x2000 / 16 / N_VERTS)
#define N_LOOPS		(8)

#define	RANDAM_FIELD_NUM	(0x2000 / 4)

#define DUST_SIZE	(2.5f)

#define SPEED_RND	(30.0f)

/* 画面前バンダリ */
#define	BOUND_WIDTH	(1500.0f)
#define	BOUND_WIDTH_DOUBLE	(BOUND_WIDTH*2.0f)
/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE	(2000.0f)

#define COL_R 	(255)
#define COL_G	(48)
#define COL_B	(8)
#define	ALPHA	(128)


typedef	struct	{
	GV_ACT_EX		actor ;

	int			col;

	DG_PRIM2	*prim ;
	FVECTOR	vec[N_LOOPS*N_PRIMS*N_VERTS];

	FVECTOR bound_0;
	FVECTOR bound_1;

	FVECTOR before_cam;

    int count;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2 *prim;
	int	clock;
	int	i, j;
	FVECTOR	*before_pos;
	FVECTOR	*pos;
	FVECTOR	*sc_pos;
	FVECTOR	*vec;
	FVECTOR	*sc_vec;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	b_dis;
	FVECTOR diff;
	FVECTOR cam;
	float	len;
	float	*p_randam;
	int		near_flag;

	prim = work->prim;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );


	// 前のカメラ座標との差
	_sceVu0SubVector( &diff, &cam, &work->before_cam ) ;
	len = GV_VecLen3F( &diff );
	near_flag = 0;
	if( len < BOUND_WIDTH ){
		near_flag = 1;
	}


	/* カメラのまえのバンダリをつくる */
	b_dis.vx=0.0f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );
	_sceVu0AddVector( &bound_0, &b_dis, &work->bound_0 ) ;
	_sceVu0AddVector( &bound_1, &b_dis, &work->bound_1 ) ;



//	// 再設置点基準をづらす
	clock = prim->buffer_clock;

	before_pos = prim->pos[1-clock];
	pos        = prim->pos[  clock];
	vec = work->vec;
	if( near_flag ){
		for ( j=0 ; j < N_LOOPS ; j++ ){
			OK_Mem_Scr( SCR_POS, before_pos, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			OK_Mem_Scr( SCR_TMP, vec,        sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			sc_pos = SCR_POS;
			sc_vec = SCR_TMP;
			for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
				if(       sc_pos->vx > bound_0.vx ){
					sc_pos->vx-= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vy > bound_0.vy ){
					sc_pos->vy-= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vz > bound_0.vz ){
					sc_pos->vz-= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vx < bound_1.vx ){
					sc_pos->vx+= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vy < bound_1.vy ){
					sc_pos->vy+= BOUND_WIDTH_DOUBLE;
				}else if( sc_pos->vz < bound_1.vz ){
					sc_pos->vz+= BOUND_WIDTH_DOUBLE;
				}else{
					sc_pos->vx += sc_vec->vx;
					sc_pos->vy += sc_vec->vy;
					sc_pos->vz += sc_vec->vz;
				}
				sc_vec++;
				sc_pos++;
			}
			OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			before_pos += N_VERTS*N_PRIMS;
			pos        += N_VERTS*N_PRIMS;
			vec        += N_VERTS*N_PRIMS;
		}
	}else{
		for ( j=0 ; j < N_LOOPS ; j++ ){
			OK_Mem_Scr( SCR_POS, before_pos, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			sc_pos = SCR_POS;
			// ランダムテーブルシャッフル
			OK_rnd_to_scr( SCR_TMP, RANDAM_FIELD_NUM );
			p_randam = (float *)SCR_TMP;
			for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
				sc_pos->vx = bound_1.vx + BOUND_WIDTH_DOUBLE * (*(p_randam++));
				sc_pos->vy = bound_1.vy + BOUND_WIDTH_DOUBLE * (*(p_randam++));
				sc_pos->vz = bound_1.vz + BOUND_WIDTH_DOUBLE * (*(p_randam++));
				sc_pos++;
			}
			OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
			before_pos += N_VERTS*N_PRIMS;
			pos        += N_VERTS*N_PRIMS;
		}
	}

	DG_COPY_VEC( &work->before_cam, &cam );

	if (++work->count >= 900) {
	    GV_DestroyActor( work );
	}
	
	
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;
	int		isize;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	isize  = (int)DUST_SIZE;
	vec        = work->vec;
	pos0       = prim->pos[ 0 ];
	pos1       = prim->pos[ 1 ];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS*N_PRIMS*N_VERTS ; i++ ){

		vec->vx = SPEED_RND*0.50f*frnd()* 0.25f;
		vec->vy = SPEED_RND*0.50f*(frnd()*1.0f - 0.1f);
		vec->vz = SPEED_RND*0.50f*frnd()* 0.25f;

		DG_COPY_VEC( pos1, (FVECTOR *)DG_Chanls->eye.m[3] );
		pos1->vx += BOUND_WIDTH * frnd();
		pos1->vy += BOUND_WIDTH * frnd();
		pos1->vz += BOUND_WIDTH * frnd();
		DG_COPY_VEC( pos0, pos1 );
		//pos1->vw = pos0->vw = DUST_SIZE;

		uvrgbwh1->w  = uvrgbwh0->w  = isize;
		uvrgbwh1->h  = uvrgbwh0->h  = isize;
		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

		//柴田改造 2001/4/20
		uvrgbwh1->r  = uvrgbwh0->r  = (work->col>>24);
		uvrgbwh1->g  = uvrgbwh0->g  = ((work->col>>16)&0xff) + irnd()%12;
		uvrgbwh1->b  = uvrgbwh0->b  = ((work->col>>8)&0xff) + irnd()%8;
		uvrgbwh1->a  = uvrgbwh0->a  = ALPHA ;

		vec++;
		pos0 ++;
		pos1 ++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;


	DG_COPY_VEC( &work->before_cam, (FVECTOR *)DG_Chanls->eye.m[3] );

	work->col = ((COL_R<<24)|(COL_G<<16)|(COL_B<<8)|(ALPHA));

	work->count = 0;


	/* 表示範囲 */
	work->bound_0.vx =  BOUND_WIDTH ;
	work->bound_0.vy =  BOUND_WIDTH ;
	work->bound_0.vz =  BOUND_WIDTH ;
	work->bound_1.vx = -BOUND_WIDTH ;
	work->bound_1.vy = -BOUND_WIDTH ;
	work->bound_1.vz = -BOUND_WIDTH ;


	//tex = DG_GetTexture( GV_StrCode( "bombpowder7_msk" ) );
	//tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );
	tex = DG_GetTexture( GV_StrCode( "chi01_msk" ) );


	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	OK_frnd_to_scr( SCR_TMP, RANDAM_FIELD_NUM );

	return 0 ;

}

void *NewFirePowderOnCamera(void)
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
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
