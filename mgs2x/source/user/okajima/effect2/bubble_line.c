//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bubble_line.c
	細かい泡でラインを生成

	2000/01/18 S.Okajima
	$Id: bubble_line.c,v 1.4 2002/11/23 12:16:42 Yoshizawa1 Exp $
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
extern float GM_WaterLevel;
extern int	OK_PutRipple( FVECTOR *center );

/*----------------------------------------------------------------*/
#define	RAISE			(0)

#define	SCALE_DOWN		(0.25f)
#define	SCREEN_NEAR			( 51.0f )

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
//#define N_PRIMS		(16)
#define N_PRIMS		(4)
#define N_VERTS		(32)
#define	RANDAM_FIELD_NUM	(0x1000/4)

#define SIZE_MIN	(30)
#define SIZE_RAND	(100)
#define BUBBLE_LINE_SIZE_MAX	(SIZE_MIN + SIZE_RAND)

#define N_MUL	(4)
#define ALPHA	(32)

#define RADIUS_MIN	( 50.0f)
#define RADIUS_RND	(200.0f)

#define ACTIV_LENGTH	(8000.0f)

#define CYCLE_SPAN	(60)
#define BIG_RADIUS_COUNT	(16)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int		cycle;
	int		cycle_count;
	int		cycle_head;

	int		free_flag;

	int		invisible_flag;

	int		top_num;
	int		alpha_num;

	FVECTOR dummy_center;
	FVECTOR *center;

	DG_TEX		*tex ;
	DG_PRIM2	*prim ;
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

static int InitPacket2( Work *work )
{
	DG_TEX		*tex ;
	DG_PRIM2	*prim ;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;
	float	size;
	int		isize;

	tex = work->tex;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if( prim==NULL ){
		printf("null prim\n");
		return -1;
	}

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos0       = prim->pos[ 0 ];
	pos1       = prim->pos[ 1 ];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		size = rnd();
		size*= size*size;
		pos1->vw = pos0->vw = SIZE_MIN + size*SIZE_RAND;
		isize  = (int)pos0->vw;

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
		uvrgbwh1->r  = uvrgbwh0->r  = BUBBLE_COL_R ;
		uvrgbwh1->g  = uvrgbwh0->g  = BUBBLE_COL_G ;
		uvrgbwh1->b  = uvrgbwh0->b  = BUBBLE_COL_B ;
		uvrgbwh1->a  = uvrgbwh0->a  = 0 ;

		pos0 ++;
		pos1 ++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	DG_InvisiblePrim2( prim );

	return 0;
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	count;
	int	clock;
	int	i;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	fvtemp;
	FVECTOR	*pos;
	float	*p_randam;
	float	len;
	float	radius;
	float	height;

	if( work->name!=-1 ){
		CheckMesgParam( work );
	}

	if( work->invisible_flag ){
		work->prim = OK_FreePrim2( work->prim );
		return;
	}

	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( work->center, &fvtemp, 1 );
	if( fvtemp.vz > fvtemp.vw ){	// カメラの後ろ側
		work->prim = OK_FreePrim2( work->prim );
		return;
	}

	_sceVu0SubVector( &fvtemp, work->center, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	len = GV_VecLen3F( &fvtemp );
	if( len > ACTIV_LENGTH  &&  work->free_flag ){
		work->prim = OK_FreePrim2( work->prim );
		return;
	}

	if( work->prim==NULL ){
		if( InitPacket2( work ) < 0 ){
			return;
		}
		work->free_flag = 0;
	}else{
		DG_VisiblePrim2( work->prim );
	}

	OK_frnd_to_scr( SCR_RND, RANDAM_FIELD_NUM );
	p_randam = (float *)SCR_RND;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	OK_Mem_Scr( SCRPAD_ADDR, work->prim->pos[1-clock], sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;




	height = GM_WaterLevel - BUBBLE_LINE_SIZE_MAX;
	count = N_PRIMS*N_VERTS;
	pos = SCRPAD_ADDR;
	uvrgbwh_before = work->prim->uvrgb[1-clock];
	uvrgbwh        = work->prim->uvrgb[  clock];
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		pos->vx += pos->vw * SCALE_DOWN * (*(p_randam++));
		pos->vy += pos->vw * BUBBLE_SPEED;
		pos->vz += pos->vw * SCALE_DOWN * (*(p_randam++));

//printf("%f %f\n",*(p_randam++),*(p_randam++));
		if( pos->vy > height ){
			pos->vy = height;
			if((count&7) == 0  &&  uvrgbwh->a != 0 )OK_PutRipple( pos );
			uvrgbwh->a = 0;
			count--;
		}else{
			if( uvrgbwh_before->a < ALPHA  ){
				if( uvrgbwh_before->a != 0 ){
					uvrgbwh->a = uvrgbwh_before->a - 1;
					uvrgbwh->a = ( uvrgbwh->a > 0 )? uvrgbwh->a: 0;
				}else{
					uvrgbwh->a = 0;
					count--;
				}
			}else{
				uvrgbwh->a = uvrgbwh_before->a;
			}
		}

		pos++;
		uvrgbwh_before++;
		uvrgbwh++;
	}

	if( len > ACTIV_LENGTH ){
		if( count <= 0 ){
			work->free_flag = 1;
		}
	}else if( work->cycle ){
		radius = RADIUS_MIN + RADIUS_RND*rnd();
		if( work->cycle_head > 0 ){
			work->cycle_head--;
			radius*= 4;
		}
		for( i=0; i<N_MUL; i++ ){
			// 初期化
			pos = SCRPAD_ADDR;
			pos += work->top_num;
			pos->vx = work->center->vx + radius*frnd();
			pos->vy = work->center->vy + radius*frnd();
			pos->vz = work->center->vz + radius*frnd();
			uvrgbwh = work->prim->uvrgb[clock];
			uvrgbwh += work->top_num;
			uvrgbwh->a = ALPHA ;

			// α減衰開始
			uvrgbwh = work->prim->uvrgb[clock];
			uvrgbwh += work->alpha_num;
			uvrgbwh->a = ALPHA -1;

			work->top_num++;
			work->top_num = (work->top_num<N_VERTS*N_PRIMS)? work->top_num: 0;

			work->alpha_num++;
			work->alpha_num = (work->alpha_num<N_VERTS*N_PRIMS)? work->alpha_num: 0;
		}
	}
	OK_Scr_Mem( work->prim->pos[  clock], SCRPAD_ADDR, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;

	work->cycle_count--;
	if( work->cycle_count <= 0 ){
		work->cycle = 1 - work->cycle;
		work->cycle_count = irnd()%CYCLE_SPAN;
		if( work->cycle ){
			work->cycle_count *= 2;
			work->cycle_head = BIG_RADIUS_COUNT;
		}
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int GetResources( Work *work )
{

	work->cycle_count = 0;
	work->cycle = 0;
	work->cycle_head = 0;

	work->free_flag = 0;

	work->alpha_num = ALPHA ;
	work->top_num = 0;
	work->tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

//  act 内で 動的確保
//	if( InitPacket2( work ) < 0 ) return -1;

	return 0 ;
}

void *NewBubbleLineScn( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map = map;

		/* 設置座標 */
		if ( GCL_GetOption( 'p' ) != NULL ) {
			work->dummy_center.vx = (float)GCL_GetNextInt() ;
			work->dummy_center.vy = (float)GCL_GetNextInt() ;
			work->dummy_center.vz = (float)GCL_GetNextInt() ;
			work->dummy_center.vw = 1.0f ;
		}else{
			ASSERT(0);
		}
		work->center = &work->dummy_center;

		work->invisible_flag = 0 ;
		if( GCL_GetOption( 'i' ) != NULL ){
			work->invisible_flag = GCL_GetNextInt() ;
		}


		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewBubbleLineProg( FVECTOR *center )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = -1;
		work->map = -1;
		work->center = center;
		work->invisible_flag = 1 ;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
