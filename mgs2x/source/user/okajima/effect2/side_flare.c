//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	side_flare.c
	横に長いレンズハレーション
	2001/02/15 S.Okajima
	$Id: side_flare.c,v 1.1.1.3 2002/11/19 11:47:24 Yoshizawa1 Exp $
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

/*----------------------------------------------------------------*/
#define	SCREEN_NEAR	( 51.0f )

#define	N_VERTS			(16)
#define	N_PRIMS			(1)
#define	N_POLYS			(N_PRIMS*N_VERTS/4)
#define	N_POLYS2		(N_POLYS*N_POLYS)
#define	N_POLYS3		(N_POLYS*N_POLYS*N_POLYS)

#define	COL_R	(8)
#define	COL_G	(16)
#define	COL_B	(32)

#define	COL_MAX	(32)

//#define	ALPHA_MAX	(32.0f)
#define	ALPHA_MAX	(255.0f)

#define	SCR_POS	(SCRPAD_ADDR)
#define	SCR_UVS	(SCRPAD_ADDR + 0x2000)
#define	SCR_TMP	(SCRPAD_ADDR + 0x3ff0)


#define	LIMIT	(50000.0f)

#define	STOCK_NUM	(4)
#define	DIVIDE_NUM	(8)

extern void *NewOnlineCheck_Static( int map_id,
							 FVECTOR *from, FVECTOR *to,
							 int chk_flag, int seg_flag, int flr_flag,
							 int *result,
							 int turn, int phase, float max_len  );

/*
	NewOnlineCheck_Static(	//HZX_OnlineHazardCheckの引数flagが呼出の時から変化しない
	int				map_id;			//まっぷID
	FVECTOR			*from;			//HZX_OnlineHazardCheckの引数
	FVECTOR			*to;			//同上
	int				chk_flag;		//同上
	int				seg_flag;		//同上
	int				flr_flag;		//同上
	
	int				*result;		//答え(-1:見えない 0:半分くらい 1:見える)

	int 			turn;			//
	int				phase;			//ライン分割数
	float 			max_len;		//チェックする最大の長さ
*/

typedef	struct	{
	GV_ACT_EX	actor ;
	int			map ;

	int			result;
	int			dir_flag;
	int			dir_count;

	FVECTOR		*ex_pos_screen0;
	FVECTOR		*ex_pos_screen1;
	FVECTOR		*ex_pos_world;
	u_char		*ex_alpha;
	int			ex_alpha_max;

	float		now_alpha;
	int			before_status;
	int			pat;

	DG_PRIM2	*prim ;

} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	j;
	int	i;
	FVECTOR				fvtemp;
	FVECTOR				*fv_sc;
	FVECTOR				*pos;
	DG_PRIM2_UVRGB		*uvrgb ;
	DG_PRIM2 *prim;
	float	alpha;
	float	angle;
	float	screen_near_x;
	float	screen_near_y;
	float	width_x;
	float	width_y;
	float	core_x;
	float	core_y;
	float	temp_x;
	float	temp_y;
	float	len;
	float	intense;
	CVECTOR	col;







//	work->pat = 0;











	core_x = 0.0f;
	core_y = 0.0f;

	if( work->ex_alpha != NULL ){
		alpha = (float)(*work->ex_alpha);
	}else{
		alpha = (float)(work->ex_alpha_max);
	}


	if( work->result > 0){
		work->dir_flag = 1;
		work->dir_count = STOCK_NUM;
	}else if( work->result < 0){
		work->dir_flag =-1;
		work->dir_count = 0;
	}else{
		if( work->dir_flag > 0 ){	//前は見えていた
			if( work->dir_count>0          ){
				work->dir_count--;
			}else{
				work->dir_count=0;
			}
		}else{
			if( work->dir_count<STOCK_NUM ){
				work->dir_count++;
			}else{
				work->dir_count=STOCK_NUM;
			}
		}
	}
	alpha = alpha * (float)work->dir_count / (float)STOCK_NUM;
	alpha = (alpha > 0.0f)? alpha: 0.0f;

/////////////


#if 0
	j = (DG_FogColor.r > DG_FogColor.g)? DG_FogColor.r: DG_FogColor.g;
	j = (DG_FogColor.b > j)?             DG_FogColor.b:             j;

	if( j==0 ){
		col.r = 0;
		col.g = 0;
		col.b = 0;
	}else{
		col.r = COL_MAX * (int)DG_FogColor.r / j;
		col.g = COL_MAX * (int)DG_FogColor.g / j;
		col.b = COL_MAX * (int)DG_FogColor.b / j;
	}
#else
	col.r = COL_R;
	col.g = COL_G;
	col.b = COL_B;
#endif


	fv_sc = SCR_TMP;

	angle = DG_Chanls->screen;
	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / angle;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / angle;

	prim = work->prim;

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;


	if( work->ex_pos_screen0 != NULL ){
		if( work->ex_pos_screen1 != NULL ){
			fv_sc->vx = (work->ex_pos_screen0->vx + work->ex_pos_screen1->vx)*0.5f;
			fv_sc->vy = (work->ex_pos_screen0->vy + work->ex_pos_screen1->vy)*0.5f;
			fv_sc->vz = (work->ex_pos_screen0->vz + work->ex_pos_screen1->vz)*0.5f;
			fv_sc->vw = (work->ex_pos_screen0->vw + work->ex_pos_screen1->vw)*0.5f;

			core_x = DG_FABS(work->ex_pos_screen0->vx - work->ex_pos_screen1->vx)*0.5f;
			core_y = DG_FABS(work->ex_pos_screen0->vy - work->ex_pos_screen1->vy)*1.0f;

		}else{
			DG_COPY_VEC( fv_sc, work->ex_pos_screen0 );
		}
		len = DG_FABS(fv_sc->vw);
		if( len < LIMIT ){
			alpha = alpha * (LIMIT - len) / LIMIT;
		}else{
			alpha = 0.0f;
		}
//printf("b:%f:%f\n",len,alpha);
	}else{
		DG_COPY_VEC( fv_sc, work->ex_pos_world );

		DG_SetPos( &DG_Chanls->eye_pers );
		DG_PutVector( fv_sc, fv_sc, 1 );
		if( fv_sc->vz < fv_sc->vw ){
			intense = DG_FABS( fv_sc->vw );
			if( DG_FABS( fv_sc->vx ) < intense
			 && DG_FABS( fv_sc->vy ) < intense
			  ){
				fv_sc->vz = SCREEN_NEAR;
				fv_sc->vx = screen_near_x * fv_sc->vx / fv_sc->vw;
				fv_sc->vy = screen_near_y * fv_sc->vy / fv_sc->vw;

				_sceVu0SubVector( &fvtemp, work->ex_pos_world, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
				len = GV_VecLen3F( &fvtemp );
				if( len < LIMIT ){
					alpha = alpha * (LIMIT - len) / LIMIT;
				}else{
					alpha = 0.0f;
				}
			}else{
				alpha = 0.0f;
			}
		}else{
			alpha = 0.0f;
		}
	}


	if( work->pat == 0 ){
		screen_near_x*= 16.0f;
		alpha*= 2.0f;
		if( alpha > 250.0f ) alpha = 250.0f;
	}else{
		screen_near_x*= 4.0f;
	}
	screen_near_y*= 1.0f;


/*
//	alpha = (alpha * ALPHA_MAX)/256.0f;

	if( work->before_status > 4  &&  alpha > 1.0f ){
		work->now_alpha = alpha;
		work->before_status = 0;
	}
	if( alpha < 1.0f ){
		work->before_status++;
	}
	work->now_alpha*= 0.995f;
	alpha = work->now_alpha;
*/


	if( alpha <= 1.0f ){
		DG_InvisiblePrim2( work->prim );
		return;
	}else{
		DG_VisiblePrim2( work->prim );
	}

#if 0
	width_x = (screen_near_x * alpha)/work->ex_alpha_max;
	width_y = width_x * 0.025f;
#else
	width_x = (screen_near_x * alpha * angle/2.0f)/work->ex_alpha_max;
	width_y = (screen_near_y * alpha * angle/2.0f)/work->ex_alpha_max;
#endif

	if( work->pat == 0 ){
		width_x = DG_FABS( width_x ) / (float)N_POLYS;
	}else{
		width_x = DG_FABS( width_x );
	}
	width_y = DG_FABS( width_y );


//	intense = rnd();

	uvrgb = prim->uvrgb[clock];
	pos   = prim->pos[clock];
	for ( j = 0 ; j < N_POLYS ; j++ ){
		i = j+1;
		if( work->pat != 0 ){
			i = i*i;
			temp_x = core_x + width_x * (float)(N_POLYS2 - i) / (float)(N_POLYS2);
			temp_y = core_y + width_y * (float)(           i) / (float)(N_POLYS2);
//			temp_y *=intense;
			temp_y *=rnd();
		}else{
			core_x += width_x;
			temp_x  = core_x;
			temp_y  = core_y;
//			temp_y *=intense;
			temp_y *=rnd();
		}
//if(GV_Time%120==0 && j==0)printf("%f %f %f\n",alpha,width_x,width_y);
		pos->vx = fv_sc->vx - temp_x;
		pos->vy = fv_sc->vy - temp_y;
		pos->vz = SCREEN_NEAR;
		pos++;
		pos->vx = fv_sc->vx + temp_x;
		pos->vy = fv_sc->vy - temp_y;
		pos->vz = SCREEN_NEAR;
		pos++;
		pos->vx = fv_sc->vx - temp_x;
		pos->vy = fv_sc->vy + temp_y;
		pos->vz = SCREEN_NEAR;
		pos++;
		pos->vx = fv_sc->vx + temp_x;
		pos->vy = fv_sc->vy + temp_y;
		pos->vz = SCREEN_NEAR;
		pos++;

		uvrgb->r = col.r;
		uvrgb->g = col.g;
		uvrgb->b = col.b;
		uvrgb->a = (int)alpha;
		uvrgb++;
		uvrgb->r = col.r;
		uvrgb->g = col.g;
		uvrgb->b = col.b;
		uvrgb->a = (int)alpha;
		uvrgb++;
		uvrgb->r = col.r;
		uvrgb->g = col.g;
		uvrgb->b = col.b;
		uvrgb->a = (int)alpha;
		uvrgb++;
		uvrgb->r = col.r;
		uvrgb->g = col.g;
		uvrgb->b = col.b;
		uvrgb->a = (int)alpha;
		uvrgb++;
	}
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb ;
	int		j;
	CVECTOR	col;

#if 0
	j = (DG_FogColor.r > DG_FogColor.g)? DG_FogColor.r: DG_FogColor.g;
	j = (DG_FogColor.b > j)?             DG_FogColor.b:             j;

	if( j==0 ){
		col.r = 0;
		col.g = 0;
		col.b = 0;
	}else{
		col.r = COL_MAX * (int)DG_FogColor.r / j;
		col.g = COL_MAX * (int)DG_FogColor.g / j;
		col.b = COL_MAX * (int)DG_FogColor.b / j;
	}
#else
	col.r = COL_R;
	col.g = COL_G;
	col.b = COL_B;
#endif


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	uvrgb = SCR_UVS;
	for ( j = 0 ; j < N_POLYS ; j++ ){
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = col.r ;
		uvrgb->g = col.g ;
		uvrgb->b = col.b ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = col.r ;
		uvrgb->g = col.g ;
		uvrgb->b = col.b ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = col.r ;
		uvrgb->g = col.g ;
		uvrgb->b = col.b ;
		uvrgb->a = 0 ;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = col.r ;
		uvrgb->g = col.g ;
		uvrgb->b = col.b ;
		uvrgb->a = 0 ;
		uvrgb++;
	}

	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_VERTS ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_VERTS ) ;


	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->result = 0;
	work->dir_flag  = -1;
	work->dir_count =  0;

	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	if( work->ex_pos_world!=NULL ){
		void *tmp = NewOnlineCheck_Static(
			GM_GetHzxGroupID( GM_CurrentStageMap ),
			work->ex_pos_world,
			(FVECTOR*)DG_Chanls->eye.m[3],
			HZX_CHK_ALL, 0, 0,
			&work->result,
			STOCK_NUM, DIVIDE_NUM, 1.0f  );
//printf("%x\n",tmp);
		if( tmp == NULL ) return -1;
		GV_SetActorChild(work,tmp);
	}

	return 0 ;
}

//---------------------------------二点指定で幅を表現
void *NewSideFlareWidth(
        FVECTOR *pos_scr0,	// 死ぬまで参照し続ける
        FVECTOR *pos_scr1,	// 死ぬまで参照し続ける
        u_char	*alpha		// 死ぬまで参照し続ける
){
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor )

		work->pat = 0;
		work->ex_pos_screen0 = pos_scr0;
		work->ex_pos_screen1 = pos_scr1;
		work->ex_pos_world   = NULL;
		work->ex_alpha       = alpha;
		work->ex_alpha_max   = 255;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}

//---------------------------------
void *NewSideFlare(
        FVECTOR *pos_scr,	// 死ぬまで参照し続ける
        u_char	*alpha		// 死ぬまで参照し続ける
){
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor )

		work->pat = 1;
		work->ex_pos_screen0  = pos_scr;
		work->ex_pos_screen1  = NULL;
		work->ex_pos_world   = NULL;
		work->ex_alpha       = alpha;
		work->ex_alpha_max   = 255;


		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}

//透視変換する：α指定
//---------------------------------
void *NewSideFlare_World(
        FVECTOR *pos_world,	// 死ぬまで参照し続ける
        u_char	*alpha		// 死ぬまで参照し続ける
){
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor )

		work->pat = 2;
		work->ex_pos_screen0  = NULL;
		work->ex_pos_screen1  = NULL;
		work->ex_pos_world   = pos_world;
		work->ex_alpha       = alpha;
		work->ex_alpha_max   = 255;


		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}

//透視変換する：αの最大値のみ指定（強度はカメラからの距離）
//---------------------------------
void *NewSideFlare_WorldAuto(
        FVECTOR *pos_world,	// 死ぬまで参照し続ける
        u_char	 max_alpha
){
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor )

		work->pat = 3;
		work->ex_pos_screen0  = NULL;
		work->ex_pos_screen1  = NULL;
		work->ex_pos_world   = pos_world;
		work->ex_alpha       = NULL;
		work->ex_alpha_max   = max_alpha;


		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}
