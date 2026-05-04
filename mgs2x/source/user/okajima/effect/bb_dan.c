//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bb_dan.c
	バルカンレイブン用ＢＢ弾
	2000/02/28 S.Okajima
	$Id: bb_dan.c,v 1.1.1.3 2002/11/19 11:46:57 Yoshizawa1 Exp $
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
#define	SPEED			(80.0f)
#define	SPEED_RAND		(SPEED * 0.5f)
#define	ANGLE_RAND		(128)

#define	N_VERTS		(32)
#define	N_PRIMS		(8)
//#define	N_PRIMS		(1)

#define	SIZE		( 15 )
#define	SIZE_A		( (float)SIZE*2.0f )

#define	SHIFT_X		(    0.0f )
#define	SHIFT_Y		(  -80.0f )
#define	SHIFT_Z		(  580.0f )

#define	ROT_X		( 64 )
#define	ROT_Y		( 0 )
#define	ROT_Z		( 0 )

#define	COLOR_R		( 128 )
#define	COLOR_G		( 128 )
#define	COLOR_B		( 128 )
#define	MAX_ALPHA	( 128 )


#define	DECAY_RATIO		( 0.6f )
#define	DECAY_RATIO_W	( 0.9f )

extern	void	MazzleUSP( FMATRIX *, int, int ) ;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	DG_PRIM2	*prim ;

	FMATRIX		*world;		/* 被付随行列 */
	FMATRIX      nozzle ;   /* MazzleUsp用マトリックス T.Morita Added　2002.04.19 */
	FVECTOR		pos[N_PRIMS*N_VERTS];
	FVECTOR		vec[N_PRIMS*N_VERTS];
	int			flag[N_PRIMS*N_VERTS];
	int			now_num;

	int			bb_put;

} Work ;

Work	*BB_WORK=NULL;

static	FVECTOR	bb_shift={ SHIFT_X, SHIFT_Y, SHIFT_Z, 0.0f };

/*----------------------------------------------------------------*/
static	void	MakeVec( Work *work, FMATRIX *world )
{
	SVECTOR	rot;
	FVECTOR	vec;
	int		now_num;

	now_num = work->now_num;

	rot.vx = ROT_X + irnd() % ANGLE_RAND - ANGLE_RAND/2;
	rot.vy = ROT_Y + irnd() % ANGLE_RAND - ANGLE_RAND/2;
	rot.vz = ROT_Z + irnd() % 4096;

	vec.vx = 0.0f;
	vec.vy = 0.0f;
	vec.vz = (SPEED - rnd() * SPEED_RAND);

	DG_SetPos( world );
	DG_MovePos( &bb_shift );
	DG_RotatePos( &rot );
	DG_GetPos( &work->nozzle );

	DG_RotVector( &vec, &work->vec[ now_num ], 1 );
	DG_COPY_VEC( &work->pos[ now_num ], (FVECTOR *)work->nozzle.m[3] );

	{
		FVECTOR	shift;
		rot.vx =-1024 + 64;
		rot.vy =    0;
		rot.vz =    0;
		shift.vx =   0.0f;
		shift.vy = -100.0f;
		shift.vz = 350.0f;
		DG_SetPos( world );
		DG_MovePos( &shift );
		DG_RotatePos( &rot );
		DG_GetPos( &work->nozzle );
		MazzleUSP( &work->nozzle, 0, 0 ) ; /*ここのマトリックスは、MazzleUSPでポインタ参照している*/
//AN_Test_Eye2( &work->pos[ now_num ], 2 );
	}

}

static	void	Act( Work *work )
{
	int	i,j;
	float	f_abs;
	int		*flag;
	FVECTOR		*vec;
	FVECTOR		*pos;
	FVECTOR		*prim_pos;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	int			h_flag;
	HZX_SEG		*seg ;
	HZX_FLR		flr[2] ;
	int			atr[2] ;
	int			map_id;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	if( (work->map & GM_CurrentStageMap) != 0 ){
		DG_VisiblePrim2( work->prim ) ;
	}else{
		DG_InvisiblePrim2( work->prim ) ;
		return;
	}

	map_id=GM_GetHzxGroupID( GM_CurrentStageMap );

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );

	if( work->bb_put != 0 ){
		MakeVec( work, work->world );
		work->bb_put = 0;
		work->flag[work->now_num]=1;
		work->now_num++;
		if( work->now_num >= N_PRIMS*N_VERTS ) work->now_num=0;
	}

	flag = work->flag;
	vec  = work->vec;
	pos  = work->pos;
	prim_pos = work->prim->pos[work->prim->buffer_clock];
	uvrgbwh  = work->prim->uvrgb[work->prim->buffer_clock];
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			if( (*flag)!=0 && !(vec->vx==0.0f && vec->vy==0.0f && vec->vz==0.0f) ){
//printf("o");
				uvrgbwh->a = MAX_ALPHA ;
				vec->vy += P_GRAVITY;
				_sceVu0AddVector( &fvtemp0, pos, vec );

				h_flag = HZX_OnlineHazardCheck( /* ハザードチェック：床も見る */
				map_id,
				pos,
				&fvtemp0,
				HZX_CHK_ALL,
				HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
				HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE );
//				HZX_SEG_NO_PLAYER,
//				HZX_FLOOR_NO_PLAYER );

				if( h_flag & 1 ){	/* 壁 */
					/* ＢＢ弾跳弾音 */
					GM_SeSetMode( SD_A_BBHIT01 , pos, GM_SEMODE_NORMAL ) ;
					HZX_GetOnlinePoint( &fvtemp1 );
//					_sceVu0SubVector( &fvtemp2, &fvtemp1, &fvtemp0 ) ;
					HZX_GetOnlineHazard( flr, atr ) ;
					seg=(HZX_SEG *)flr;
					fvtemp0.vx = seg->p2.z - seg->p1.z ;
					fvtemp0.vy = 0.0f ;
					fvtemp0.vz = seg->p1.x - seg->p2.x ;
					DG_ReflectVector( &fvtemp0, vec, vec );
					vec->vx *= DECAY_RATIO;
					vec->vy *= DECAY_RATIO;
					vec->vz *= DECAY_RATIO;
//					_sceVu0AddVector( pos, &fvtemp1, vec ) ;
					pos->vx = fvtemp1.vx + vec->vx;
					pos->vy = fvtemp1.vy;
					pos->vz = fvtemp1.vz + vec->vz;
				}
				if( h_flag & 2 ){	/* 床 */
					/* ＢＢ弾跳弾音 */
					HZX_GetOnlinePoint( &fvtemp1 );
					f_abs=( vec->vy > 0 )?vec->vy:-vec->vy;
					if( f_abs > 10.0f ) GM_SeSetMode( SD_A_BBHIT01 , pos, GM_SEMODE_NORMAL ) ;
					if( DG_MAX( DG_FABS(vec->vx), DG_FABS(vec->vz) ) < 1.0f ){
						vec->vx = 0.0f;
						vec->vy = 0.0f;
						vec->vz = 0.0f;
						pos->vy = fvtemp1.vy;
						(*flag) = 0;
//						AN_Test_Eye2( pos, 2 );
					}else{
						vec->vx  =  vec->vx* DECAY_RATIO_W + rnd()*vec->vx*0.1f;
						vec->vy *= -DECAY_RATIO;
						vec->vz  =  vec->vz* DECAY_RATIO_W + rnd()*vec->vz*0.1f;
//						_sceVu0AddVector( pos, &fvtemp1, vec ) ;
//						DG_COPY_VEC( pos, &fvtemp1 );
						pos->vx = fvtemp1.vx;
						pos->vy = fvtemp1.vy + 1.0f;
						pos->vz = fvtemp1.vz;
					}
				}
				if( !(h_flag & 3) ){	/* 当たらなかった */
					DG_COPY_VEC( pos, &fvtemp0 );
				}
			}else{
//printf("x");
			}
			DG_COPY_VEC( prim_pos, pos );
			prim_pos->vy += SIZE_A;

			uvrgbwh++;
			prim_pos++;
			pos ++;
			vec ++;
			flag++;
		}
	}
//printf("\n");

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	BB_WORK = NULL;
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	for( j=0; j<2; j++ ){
		uvrgbwh = prim->uvrgb[ j ] ;
		for( i=0; i<N_PRIMS; i++ ){
			for ( k = 0 ; k < N_VERTS ; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;

				uvrgbwh->w = SIZE ;
				uvrgbwh->h = SIZE ;

				uvrgbwh->r = COLOR_R ;
				uvrgbwh->g = COLOR_G ;
				uvrgbwh->b = COLOR_B ;
//				uvrgbwh->a = MAX_ALPHA ;
				uvrgbwh->a = 0 ;
				uvrgbwh ++ ;
			}
		}
	}
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FMATRIX *world )
{
	int	i,j;
	FVECTOR		*vec;
	FVECTOR		*pos;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			*flag;

	work->now_num=0;

	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	pos  = work->pos;
	vec  = work->vec;
	pos0 = work->prim->pos[0];
	pos1 = work->prim->pos[1];
	flag = work->flag;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			DG_COPY_VEC( pos,  &DG_ZeroVector );
			DG_COPY_VEC( pos0, &DG_ZeroVector );
			DG_COPY_VEC( pos1, &DG_ZeroVector );
			DG_COPY_VEC( vec,  &DG_ZeroVector );
			(*flag)=0;
			flag++;
			pos ++;
			vec ++;
			pos0++;
			pos1++;
		}
	}

	/* ＢＢ弾発射音 */
	GM_SeSetMode( SD_A_BBSHOT00 , (FVECTOR *)work->world->m[3], GM_SEMODE_NORMAL ) ;

	return 0 ;
}

/* ワーク１個型 */
void *New_BB_Dan( FMATRIX *world, int map )
{
	Work		*work ;

	work = BB_WORK;
	if( BB_WORK==NULL ){
		work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
		if ( work != NULL ) {
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			work->map = map;

			BB_WORK = work;
			work->bb_put=1;
			work->world  = world;

			if ( GetResources( work, world ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
	}else{
		work->bb_put = 1;
		work->world  = world;
	}
	return (void *)work ;

}
