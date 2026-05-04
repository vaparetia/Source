//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_bio.c
	死亡したときに広がる血
	2000/10/04 S.Okajima
	$Id: blood_bio.c,v 1.1.1.3 2002/11/19 11:46:58 Yoshizawa1 Exp $
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
#include	"blood.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
extern void *NewVapor( FVECTOR *center, float radius, int col, int life );
extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern int	OK_FloorOffFlag;

/*----------------------------------------------------------------*/
#define	RAISE			(-60000)

/* 固定 */
#define	N_VERTS			(4)
#define	N_PRIMS			(1)
#define	N_POLYS			(N_VERTS*N_PRIMS/4)

#define	COLOR_NYOU_R	(40)
#define	COLOR_NYOU_G	(40)
#define	COLOR_NYOU_B	(10)

#define	SHIFT_CENTER	(1.0f)
#define	SCR_UVS0	(SCRPAD_ADDR )

#define	SIZE_MIN_RATIO	(0.25f)

#ifdef ENGLISH
#define	COLOR_A	(255)
#else
#define	COLOR_A	(BLOOD_ALPHA_1)
#endif

#define	SIZE_COUNT	(180)

typedef	struct	{
	GV_ACT_EX	actor ;
	int			map ;

	DG_PRIM2	*prim ;
	SVECTOR		floor_rot;
	SVECTOR		local_rot;
	FVECTOR		pos;
	int			size_count;
	int			size_count_max;
	int			life;
	int			life_max;
	int			col;
	int			yuge;
	int			check_flag;
	float		size_max;
} Work ;


/*----------------------------------------------------------------*/
/* floor に プリミティブを張る */
static	int	SetPrims( FVECTOR *prim_pos, FVECTOR *point_pos, SVECTOR *local_rot, SVECTOR *floor_rot, float sp_size, int check_flag )
{
	float	shift;
	FVECTOR	*fvp;

	if( floor_rot->vx > 2048 ){
		shift =  SHIFT_CENTER;
	}else{
		shift = -SHIFT_CENTER;
	}

	prim_pos[0].vx = - sp_size;
	prim_pos[0].vy =   sp_size;
	prim_pos[0].vz =   shift;
	prim_pos[1].vx =   sp_size;
	prim_pos[1].vy =   sp_size;
	prim_pos[1].vz =   shift;
	prim_pos[2].vx = - sp_size;
	prim_pos[2].vy = - sp_size;
	prim_pos[2].vz =   shift;
	prim_pos[3].vx =   sp_size;
	prim_pos[3].vy = - sp_size;
	prim_pos[3].vz =   shift;

	DG_SetPos2( &DG_ZeroVector, local_rot );
	DG_PutVector( prim_pos, prim_pos, 4 );
	DG_SetPos2( point_pos, floor_rot );
	DG_PutVector( prim_pos, prim_pos, 4 );

	if( check_flag ){
		fvp = prim_pos;
		if( UTL_EFT_CheckBound( fvp++ ) >= 0 ) return 0;
		if( UTL_EFT_CheckBound( fvp++ ) >= 0 ) return 0;
		if( UTL_EFT_CheckBound( fvp++ ) >= 0 ) return 0;
		if( UTL_EFT_CheckBound( fvp++ ) >= 0 ) return 0;
	}

	return 1;
}


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		alpha=0;
	DG_PRIM2_UVRGB		*uvrgb ;
	int		clock;
	int		life_calc;
	float		size;
	float		ratio;


	if( !GM_CheckGameStatus( STATE_DEMO ) ){
		if( OK_FloorOffFlag ){
			work->check_flag = 1;
		}
	}

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;
	if( work->map > 0 ){
		GM_GroupPrim2( work->prim, work->map ) ;
	}else{
		GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	}

	DG_VisiblePrim2( work->prim );


	ratio = (float)(work->size_count) / (float)work->size_count_max;
	ratio*= ratio;
	ratio = 1.0f - ratio;
	size = work->size_max * (1.0f - SIZE_MIN_RATIO) * ratio + work->size_max*SIZE_MIN_RATIO;

	if( !SetPrims( work->prim->pos[clock], &work->pos, &work->local_rot, &work->floor_rot, size, work->check_flag ) ){
		DG_InvisiblePrim2( work->prim );
		GV_DestroyActor( work ) ;
		return;
	}

	alpha = (work->col&255);
	if( work->life_max > 0 ){
		life_calc = work->life_max*3/4;
		if( work->life < life_calc ){
			alpha = alpha * work->life / life_calc;
		}
	}

	uvrgb = work->prim->uvrgb[clock];
	uvrgb[0].a = alpha;
	uvrgb[1].a = alpha;
	uvrgb[2].a = alpha;
	uvrgb[3].a = alpha;



	if( work->life > 0 ){
		work->life--;
		if( work->life <= 0 ){
			GV_DestroyActor( work ) ;
		}
	}

	work->size_count--;
	if( work->size_count <= 0 ){
		work->size_count = 0;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb ;
	int		i, k ;
	CVECTOR	col;

	DG_ConfigPrim2Tex( prim, tex );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}


	col.r = (work->col>>24)&255;
	col.g = (work->col>>16)&255;
	col.b = (work->col>> 8)&255;

	prim->raise = RAISE;

	uvrgb   = SCR_UVS0 ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
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
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS0, sizeof(DG_PRIM2_UVRGB), N_VERTS * N_PRIMS ) ;

	return 1;
}

/*----------------------------------------------------------------*/
/* floor についての rotを得る */
static	void	CalcLocalRot_Floor( HZX_FLR *floor, SVECTOR *rot )
{
	FVECTOR	to;

	/* 法線収得 */
	to.vx=floor->p1.h;
	to.vy=floor->p3.h;
	to.vz=floor->p2.h;

	OK_DirVecXY( &DG_ZeroVector, &to, rot );
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	HZX_FLR		flr[2];
	FVECTOR		fvtemp;
	float		flr_height[2];
	int			flr_atrs[2];
	int			floor_flag;

	work->check_flag = 0;

	work->size_count = work->size_count_max = SIZE_COUNT;


	if( work->map > 0 ){
		floor_flag = HZX_LevelHazardCheck(
		                GM_GetHzxGroupID( work->map ),
		                &work->pos,
		                HZX_CHK_F_FLOOR,
		                HZX_FLOOR_NO_PLAYER );
//		                HZX_CHK_ALL, 
//		                HZX_FLOOR_NO_BULLET | HZX_FLOOR_NO_BLOOD );
		if( floor_flag & 1 ){
			HZX_GetLevelHazard( flr, flr_atrs );
//			if( 1 ){
			if( !( flr_atrs[ 0 ] & (HZX_FLOOR_NO_BLOOD|HZX_FLOOR_STEP) ) ){
//			if( !(flr[ 0 ].attribute & ( HZX_FLOOR_NO_BULLETHOLE | HZX_FLOOR_NO_OBJECT) )  ){
				if( !OK_CheckFloorEdge( &fvtemp, &work->pos, work->size_max, &flr[0] ) ){	/* エッジでない */
					CalcLocalRot_Floor( flr, &work->floor_rot );
					HZX_GetLevelHeight( flr_height );
					work->pos.vy = flr_height[0] + 5.0f;
				}else{
					return -1;
				}
			}else{
				return -1;
			}
		}else{
			return -1;
		}
	}else{
		work->floor_rot.vx = 1024;
		work->floor_rot.vy = 0;
		work->floor_rot.vz = 0;
	}



	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	tex = DG_GetTexture( 4297805 /*"vr2_ketchap00_alp"*/ );
	}
	else
	{
//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 2134548 /*"ketchap_g"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	tex = DG_GetTexture( 203440 /*"ketchap00_alp"*/ );
//	tex = DG_GetTexture( 1252016 /*"ketchap01_alp"*/ );
//	tex = DG_GetTexture( 2300592 /*"ketchap02_alp"*/ );
	}


	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	work->local_rot.vx=0;
	work->local_rot.vy=0;
	work->local_rot.vz=(short)(irnd() % 4096);

	SetPrims( work->prim->pos[0], &work->pos, &work->local_rot, &work->floor_rot, work->size_max*SIZE_MIN_RATIO, 0 );
	SetPrims( work->prim->pos[1], &work->pos, &work->local_rot, &work->floor_rot, work->size_max*SIZE_MIN_RATIO, 0 );


	if( work->yuge != 0 ){
		void *p1;
		p1 = NewVapor( &work->pos, work->size_max, 0x30384080, work->life_max/2 );
		if(p1) GV_SetActorChild(work,p1);
	}

	return 0 ;
}

void *NewBloodBioDead( FVECTOR *pos, int life, float size, int mode, int map )
{
	Work		*work ;

	OPERATOR() ;

	if( mode==0 && GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;


	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->pos, pos );
		work->life_max = life;
		work->life     = life;
		work->size_max = size;
		work->map = map;
		work->yuge = mode;

//printf("mode:%d\n",mode);

		if( mode != 0 ){
			work->col = (COLOR_NYOU_R<<24)|(COLOR_NYOU_G<<16)|(COLOR_NYOU_B<< 8)|(128);
		}else{
			work->col = (COLOR_R<<24)|(COLOR_G<<16)|(COLOR_B<< 8)|(COLOR_A&255);
		}

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


void *NewNyou( FVECTOR *pos, int life, float size, int col, int yuge )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->pos, pos );
		work->life_max = life;
		work->life     = life;
		work->size_max = size;
		work->map = -1;
		work->col = col;
		work->yuge = yuge;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

