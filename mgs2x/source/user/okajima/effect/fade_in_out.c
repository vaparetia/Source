//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fade_in_out.c
	フェードイン・アウト
	2000/02/09 S.Okajima
	$Id: fade_in_out.c,v 1.1.1.3 2002/11/19 11:47:05 Yoshizawa1 Exp $
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
#define	SCREEN_NEAR	( 50.05001f )
//#define	SCREEN_NEAR	( 51.0f )
#define	SIZE_CENTER			(24.0f)
#define	SIZE_DROP_RND		(8.0f)
#define	SIZE_DROP_MIN		(8.0f)
#define	SIZE_PERS			(24.0f)

#define	N_VERTS			(4)
#define	N_PRIMS			(1)

#define	N_VERTS_DROP	(64)
#define	N_POLYS_DROP	(N_VERTS_DROP/4)
#define	N_PRIMS_DROP	(1)

#define	TEXTURE_CENTER		( 3594043 /*"drop01_msk"*/ )
#define	TEXTURE_DROP		( 3594043 /*"drop01_msk"*/ )

#define	ALPHA_MAX	(128)

#define	SCR_POS	(SCRPAD_ADDR)
#define	SCR_UVS	(SCRPAD_ADDR + 0x2000)


typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	DG_PRIM2	*prim_second ;

	int			add_sub;
	FVECTOR		init_color;
	int			fade_in_out;
	int			variation;
	int			count_max;
	int			count;
	int			stable;
	int			proc_id;
	FVECTOR		*pos;

	int			alpha_max;

	int			scn_prog_flag;
} Work ;

#define BOTTOM_LIMIT (-4)
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	alpha;
	int	alpha_center;
	int	over_count;
	int	i;
	FVECTOR			fvtemp ;
	FVECTOR			*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	float	screen_near_x;
	float	screen_near_y;
	float	size;

	work->count--;
	over_count = work->count;
	work->count = (work->count>=BOTTOM_LIMIT)?work->count:BOTTOM_LIMIT;
	if( over_count > 0 ) over_count = 0;

	if( work->variation == 0 ){
		if( work->proc_id != -1 ){
			if( work->count < BOTTOM_LIMIT ){
				if(work->proc_id!=-1){
					GCL_ExecProc( work->proc_id, NULL );
					work->proc_id = -1;
				}
				if( !work->stable ) GV_DestroyActor( work ) ;
			}
		}else{
			if( !work->stable ) GV_DestroyActor( work ) ;
		}
		return;
	}


	if( work->count < BOTTOM_LIMIT ){
		if( work->proc_id !=-1 ){
			GCL_ExecProc( work->proc_id, NULL );
			work->proc_id = -1;
		}
		if( !work->stable ) GV_DestroyActor( work ) ;
		return;
	}

	DG_SwitchBuffPrim2( work->prim_second );
	clock = work->prim_second->buffer_clock;


	if( !work->fade_in_out ){
		alpha = ALPHA_MAX * over_count / work->count_max ;
		alpha = ( alpha < work->alpha_max )?work->alpha_max:alpha;
	}else{
		alpha = ALPHA_MAX * (work->count_max - over_count) / work->count_max ;
		alpha = ( alpha > work->alpha_max )?work->alpha_max:alpha;
	}

	switch( work->variation ){
	  case 0:
		break;
	  case 1:	//バリエーション：センター
		alpha_center = alpha * 3 / 2;
		if( alpha_center>128 ) alpha_center=128;
		if( alpha_center<0   ) alpha_center=0;
		uvrgb = work->prim_second->uvrgb[clock];
		uvrgb->a = alpha_center;	uvrgb++;
		uvrgb->a = alpha_center;	uvrgb++;
		uvrgb->a = alpha_center;	uvrgb++;
		uvrgb->a = alpha_center;
		break;
	  case 2:	//バリエーション：ドロップ
		uvrgb = work->prim_second->uvrgb[clock];
		for( i=0; i<N_POLYS_DROP; i++ ){
			alpha_center = alpha*2*i/N_POLYS_DROP;
			if( alpha_center>128 ) alpha_center=128;
			if( alpha_center<0   ) alpha_center=0;
			uvrgb->a = alpha_center;	uvrgb++;
			uvrgb->a = alpha_center;	uvrgb++;
			uvrgb->a = alpha_center;	uvrgb++;
			uvrgb->a = alpha_center;	uvrgb++;
		}
		break;
	  case 3:	//バリエーション：パース
		alpha_center = alpha * 3 / 2;
		if( alpha_center>128 ) alpha_center=128;
		if( alpha_center<0   ) alpha_center=0;
		pos   = work->prim_second->pos[clock];
		uvrgb = work->prim_second->uvrgb[clock];
		uvrgb->a = alpha_center;	uvrgb++;
		uvrgb->a = alpha_center;	uvrgb++;
		uvrgb->a = alpha_center;	uvrgb++;
		uvrgb->a = alpha_center;

		DG_SetPos( &DG_Chanls->eye_pers );
		DG_PutVector( work->pos, &fvtemp, 1 );
		screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls->screen;
		screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;
		fvtemp.vx = screen_near_x * fvtemp.vx / fvtemp.vw;
		fvtemp.vy = screen_near_y * fvtemp.vy / fvtemp.vw;

		if( work->fade_in_out ){
			size = SIZE_PERS * (float)over_count / (float)work->count_max;
		}else{
			size = SIZE_PERS * (float)(work->count_max - over_count) / (float)work->count_max;
		}
		pos->vx = fvtemp.vx - size;
		pos->vy = fvtemp.vy - size;
		pos++;
		pos->vx = fvtemp.vx + size;
		pos->vy = fvtemp.vy - size;
		pos++;
		pos->vx = fvtemp.vx - size;
		pos->vy = fvtemp.vy + size;
		pos++;
		pos->vx = fvtemp.vx + size;
		pos->vy = fvtemp.vy + size;

		break;
	}
}

static void Die( Work *work )
{
	work->prim_second = OK_FreePrim2( work->prim_second );
}

/*----------------------------------------------------------------*/
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, float size )
{
	FVECTOR				*pos ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int		i;

	DG_ConfigPrim2Tex( prim, tex );

	if( work->add_sub==0 ){
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else{
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}

	for ( i = 0 ; i < 2 ; i++ ){
		pos     = prim->pos[i];
		uvrgb   = prim->uvrgb[i];

		pos->vx = -size ;
		pos->vy = -size ;
		pos->vz =  SCREEN_NEAR;
		pos++;

		pos->vx =  size ;
		pos->vy = -size ;
		pos->vz =  SCREEN_NEAR;
		pos++;

		pos->vx = -size ;
		pos->vy =  size ;
		pos->vz =  SCREEN_NEAR;
		pos++;

		pos->vx =  size ;
		pos->vy =  size ;
		pos->vz =  SCREEN_NEAR;
		pos++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = (int)work->init_color.vx ;
		uvrgb->g = (int)work->init_color.vy ;
		uvrgb->b = (int)work->init_color.vz ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = (int)work->init_color.vx ;
		uvrgb->g = (int)work->init_color.vy ;
		uvrgb->b = (int)work->init_color.vz ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = (int)work->init_color.vx ;
		uvrgb->g = (int)work->init_color.vy ;
		uvrgb->b = (int)work->init_color.vz ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = (int)work->init_color.vx ;
		uvrgb->g = (int)work->init_color.vy ;
		uvrgb->b = (int)work->init_color.vz ;
		uvrgb->a = 0 ;
		uvrgb++;
	}

	return 1;
}

/*----------------------------------------------------------------*/
static int InitPacketDrop( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int num_poly )
{
	FVECTOR				center ;
	FVECTOR				*pos ;
	DG_PRIM2_UVRGB		*uvrgb ;
	int		j;
	float	size;

	DG_ConfigPrim2Tex( prim, tex );

	if( work->add_sub==0 ){
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else{
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}

	size = rnd() * SIZE_DROP_RND + SIZE_DROP_MIN;

	pos   = SCR_POS;
	uvrgb = SCR_UVS;
	for ( j = 0 ; j < num_poly ; j++ ){
		center.vx = rnd()*SCREEN_NEAR - SCREEN_NEAR*0.5f;
		center.vy = rnd()*SCREEN_NEAR - SCREEN_NEAR*0.5f;

		pos->vx = center.vx-size ;
		pos->vy = center.vy-size ;
		pos->vz =  SCREEN_NEAR;
		pos++;

		pos->vx = center.vx+size ;
		pos->vy = center.vy-size ;
		pos->vz =  SCREEN_NEAR;
		pos++;

		pos->vx = center.vx-size ;
		pos->vy = center.vy+size ;
		pos->vz =  SCREEN_NEAR;
		pos++;

		pos->vx = center.vx+size ;
		pos->vy = center.vy+size ;
		pos->vz =  SCREEN_NEAR;
		pos++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = (int)work->init_color.vx ;
		uvrgb->g = (int)work->init_color.vy ;
		uvrgb->b = (int)work->init_color.vz ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = (int)work->init_color.vx ;
		uvrgb->g = (int)work->init_color.vy ;
		uvrgb->b = (int)work->init_color.vz ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = (int)work->init_color.vx ;
		uvrgb->g = (int)work->init_color.vy ;
		uvrgb->b = (int)work->init_color.vz ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = (int)work->init_color.vx ;
		uvrgb->g = (int)work->init_color.vy ;
		uvrgb->b = (int)work->init_color.vz ;
		uvrgb->a = 0 ;
		uvrgb++;
	}


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        num_poly * 4 ) ;
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        num_poly * 4 ) ;
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), num_poly * 4 ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), num_poly * 4 ) ;


	return 1;
}


/* 初期設定値を取得 */
static	void	GetOptionValue( Work *work )
{
/*
chara フェード [NewFadeInOut] $s:名前 \
	-m $i:加算減算 \	//0:加算 1:減算
	-c $v:ＲＧＢ \
	-f $i:フェードモード \	//0:fade_in 1:fade_out
	-v $i:バリエーション \	//0:full 1:center 2:drop
	-t $i:移行時間（フレーム） \
	-p $p:移行時間後起動proc \
	-s $i:移行時間後デストロイしない //αの最終値をセットする（そのαで変化ストップ）
*/
	if ( GCL_GetOption( 'm' ) != NULL ){
		work->add_sub = GCL_GetNextInt();
	}else{
		work->add_sub = 0;
	}

	if ( GCL_GetOption( 'c' ) != NULL ){
		work->init_color.vx = (float)GCL_GetNextInt();
		work->init_color.vy = (float)GCL_GetNextInt();
		work->init_color.vz = (float)GCL_GetNextInt();
	}else{
		work->init_color.vx = 255.0f;
		work->init_color.vy = 255.0f;
		work->init_color.vz = 255.0f;
	}

	if ( GCL_GetOption( 'f' ) != NULL ){
		work->fade_in_out = GCL_GetNextInt();
	}else{
		work->fade_in_out = 0;
	}
	if( !work->fade_in_out ){
		work->alpha_max = 0;
	}else{
		work->alpha_max = ALPHA_MAX;
	}

	if ( GCL_GetOption( 'v' ) != NULL ){
		work->variation = GCL_GetNextInt();
	}else{
		work->variation = 0;
	}

	if ( GCL_GetOption( 't' ) != NULL ){
		work->count = work->count_max = GCL_GetNextInt();
	}else{
		work->count = work->count_max = 60;
	}

	if ( GCL_GetOption( 'p' ) != NULL ){
		work->proc_id = GCL_GetNextInt();
	}else{
		work->proc_id = -1;
	}

	if ( GCL_GetOption( 's' ) != NULL ){
		work->stable = 1;
		work->alpha_max = GCL_GetNextInt();
	}else{
		work->stable = 0;
	}

	work->pos = NULL;

}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	if( work->scn_prog_flag ){	//シナリオ起動
/*
		extern void *NewFadeInOut_Demo( int col_r, int col_g, int col_b, int col_a, int time );
		NewFadeInOut_Demo( (int)work->init_color.vx, (int)work->init_color.vy, (int)work->init_color.vz, ALPHA_MAX-(int)work->alpha_max, 0 );
		NewFadeInOut_Demo( (int)work->init_color.vx, (int)work->init_color.vy, (int)work->init_color.vz, (int)work->alpha_max, work->count );
*/
		extern void *NewFadeInOutForce_Demo( int ini_r, int ini_g, int ini_b, int ini_a,
                                             int col_r, int col_g, int col_b, int col_a, int time );
		NewFadeInOutForce_Demo( (int)work->init_color.vx, (int)work->init_color.vy, (int)work->init_color.vz, ALPHA_MAX-(int)work->alpha_max,
                                (int)work->init_color.vx, (int)work->init_color.vy, (int)work->init_color.vz, (int)work->alpha_max, work->count );

	}

	switch( work->variation ){
	  case 0:	//全画面をもう一枚
		break;
	  case 1:	//バリエーション：センター
		tex = DG_GetTexture( TEXTURE_CENTER );
		prim = work->prim_second = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS, N_VERTS );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacket( work, prim, tex, SIZE_CENTER );
		prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
		break;
	  case 2:	//バリエーション：ドロップ
		tex = DG_GetTexture( TEXTURE_DROP );
		prim = work->prim_second = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS_DROP, N_VERTS_DROP );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacketDrop( work, prim, tex, N_POLYS_DROP );
		prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
		break;
	  case 3:	//バリエーション：パース
		tex = DG_GetTexture( TEXTURE_DROP );
		prim = work->prim_second = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS_DROP, N_VERTS_DROP );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacket( work, prim, tex, SIZE_PERS );
		prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
		break;
	}
	//---------------------------------

	return 0 ;
}


//---------------------------------
void *NewFadeInOutScn( void )
{
	Work		*work ;

	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->scn_prog_flag = 1;

		/* シナリオ設定値をワークにセット */
		GetOptionValue( work );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
//---------------------------------
void *NewFadeInOut(
        int add_sub,	/* 0:加算 1:減算 */
        int color_r,	/* 0 - 255 */
        int color_g,	/* 0 - 255 */
        int color_b,	/* 0 - 255 */
        int fade_in_out,/* 0:fade_in 1:fade_out */
        int variation,	/* 0:full 1:center 2:drop */
        int count,		/* 0以上:変化フレーム（継続フレーム） */
        int stable,		/*      -1:変化フレーム後デストロイする（通常）、
                           0 - 128:途中で止めるα（且つ自分でデストロイしない） */
        FVECTOR *pos	/* 透視変換し、これを中心にして飽和させる */
){
	Work		*work ;

//AN_Test_Eye2( pos, 2 );


	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor )

		work->scn_prog_flag = 0;

		work->pos           = pos;
		work->add_sub       = add_sub;
		work->init_color.vx = (float)color_r;
		work->init_color.vy = (float)color_g;
		work->init_color.vz = (float)color_b;
		work->fade_in_out   = fade_in_out;
		work->variation     = variation;
		work->count = work->count_max = count;
		if( stable!=-1 ){
			work->stable    = 1;
			work->alpha_max = stable;
		}else{
			work->stable    = 0;
			if( !work->fade_in_out ){
				work->alpha_max = 0;
			}else{
				work->alpha_max = ALPHA_MAX;
			}
		}
		work->proc_id = -1;	/* 関数呼び出しではプロック呼び出しを行なわない */

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
