//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	steam_smoke.c
	攻撃ターゲット付き水蒸気と消化器ガス
	2000/01/26 S.Okajima
	$Id: steam_smoke.c,v 1.1.1.3 2002/11/19 11:47:15 Yoshizawa1 Exp $
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
#define	SPEED			(60.0f)
#define	SPEED_RAND		(20.0f)
#define	ANGLE_RAND		(128)

#define N_PRIMS		(2)
#define N_VERTS		(16)

#define	COLOR_R		(255)
#define	COLOR_G		(255)
#define	COLOR_B		(255)
#define	MAX_ALPHA	(16)

//#define	LIFE_TIME	( 600 )
#define	LIFE_TIME	( 200 )	/* 是角さん指示 */
#define	REFLESH_INTERVAL	(N_PRIMS * N_VERTS)

#define	SIZE_SMALL		( 100.0f)
#define	SIZE_ADD_SMALL	( 10.0f )
#define	SIZE_MAX_SMALL	( 600.0f)

#define	SIZE_MIDDLE		( 100.0f )
#define	SIZE_ADD_MIDDLE	( 10.0f )
#define	SIZE_MAX_MIDDLE	( 1000.0f)

#define	SIZE_LARGE		( 100.0f )
#define	SIZE_ADD_LARGE	( 25.0f )
#define	SIZE_MAX_LARGE	( 1500.0f)


extern void *NewSteamOnCamera( FVECTOR *center, SVECTOR *rot, float size, int life );
extern int OK_StepCheckHzd( FVECTOR *now, FVECTOR *before, FVECTOR *vec, float decay, float margin, HZX_GROUP_ID map_id );

int	OK_steam_se_num_s=0;
int	OK_steam_se_num_m=0;
int	OK_steam_se_num_l=0;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	DG_PRIM2	*prim ;

	FMATRIX		world;
	FVECTOR		center;
	FVECTOR		vec[N_PRIMS*N_VERTS];
	int			prim_count[N_PRIMS*N_VERTS];
	int			count;

	int			life;
	int			white;
	int			prim_num;

	FVECTOR		size;

	TARGET   		target;
	POWER_TARGET 	power;

	int			pattern;
	int			hzd_flag;

} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;
	FVECTOR		*vec;
	FVECTOR		*pos;
	FVECTOR		*pos_back;
	int	i,j;
	int	clock;
	int	count;
	int	number;
	int	*prim_count;
	int	size;
	int	size_add;
	int	size_max;





//	DG_InvisiblePrim2( work->prim ) ;
	switch( work->pattern ){
		case 0:	/* 小 */
		default:
			size     = SIZE_SMALL;
			size_add = SIZE_ADD_SMALL;
			size_max = SIZE_MAX_SMALL;
			break;
		case 1:	/* 中 */
			size     = SIZE_MIDDLE;
			size_add = SIZE_ADD_MIDDLE;
			size_max = SIZE_MAX_MIDDLE;
			break;
		case 2:	/* 大 */
			size     = SIZE_LARGE;
			size_add = SIZE_ADD_LARGE;
			size_max = SIZE_MAX_LARGE;
			break;
		case 10:	/* 消化器 */
			size     = SIZE_LARGE;
			size_add = SIZE_ADD_LARGE;
			size_max = SIZE_MAX_LARGE;
			break;
	}

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

	clock = work->prim->buffer_clock;

	vec      = work->vec;
	pos      = work->prim->pos[clock];
	pos_back = work->prim->pos[1-clock];
	uvrgbwh      = work->prim->uvrgb[clock];
	count = 0;
	number = 0;
	prim_count = work->prim_count;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			if( vec->vw == 0.0f ){
				if( number < work->life ){
					vec->vw = size + size*rnd();
					pos->vx = work->center.vx + vec->vx;
					pos->vy = work->center.vy + vec->vy;
					pos->vz = work->center.vz + vec->vz;
					pos->vw = TPI * rnd();
					uvrgbwh->a = MAX_ALPHA;
					(*prim_count) = REFLESH_INTERVAL;
				}
			}else{
				if( (*prim_count) < 0 ){
					count++;
					vec->vw = size + size*rnd();
					pos->vx = work->center.vx + vec->vx;
					pos->vy = work->center.vy + vec->vy;
					pos->vz = work->center.vz + vec->vz;
					pos->vw = TPI * rnd();

					if( work->life < LIFE_TIME ){
						(*prim_count) = REFLESH_INTERVAL;
						uvrgbwh->a = MAX_ALPHA;
					}else{
						(*prim_count) = REFLESH_INTERVAL - ( work->life - LIFE_TIME );
						if( (*prim_count) > 0 ){
							uvrgbwh->a = MAX_ALPHA * (*prim_count) / REFLESH_INTERVAL;
						}else{
							uvrgbwh->a = 0;
						}
					}
				}else{
					vec->vw += size_add;
					if( vec->vw > size_max ) vec->vw = size_max;
					pos->vx = pos_back->vx + vec->vx;
					pos->vy = pos_back->vy + vec->vy;
					pos->vz = pos_back->vz + vec->vz;
					if(j%2){
						pos->vw = pos_back->vw + TPI * 0.1f * rnd();
					}else{
						pos->vw = pos_back->vw - TPI * 0.1f * rnd();
					}
					uvrgbwh->a = MAX_ALPHA * (*prim_count) / REFLESH_INTERVAL;
				}

			}
			uvrgbwh->w = (int)(cosf( pos->vw ) * vec->vw );
			uvrgbwh->h = (int)(sinf( pos->vw ) * vec->vw );
			(*prim_count)--;

//printf("%d:%d %d %d\n",number,uvrgbwh->w,uvrgbwh->h,uvrgbwh->a);
//printf("%f %f %f\n",pos->vx,pos->vy,pos->vz);
/*
			{
				FVECTOR fvtemp;
				fvtemp.vx = pos->vx;
				fvtemp.vy = pos->vy+500.0f;
				fvtemp.vz = pos->vz;
				AN_Test_Eye2( &fvtemp, 2 );
			}
*/

			prim_count++;
			number++;
			pos++;
			pos_back++;
			vec ++;
			uvrgbwh++;
		}
	}

#if 0
	GM_MoveTarget2Map( &work->target, &work->world, GM_CurrentStageMap );
	GM_PutTarget( &work->target );
//	NewTargetView2( &work->target,255,0,0 ) ;
#endif

	work->life++;
	if( count >= N_PRIMS*N_VERTS-1 ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
//	GM_FreeTarget( &work->target );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0;
	DG_PRIM2_UVRGBWH	*uvrgbwh1;
	int	i,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for( i=0; i<N_PRIMS; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
			uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
			uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
			uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

			uvrgbwh0->w = uvrgbwh1->w = 0 ;
			uvrgbwh0->h = uvrgbwh1->h = 0 ;
			uvrgbwh0->r = uvrgbwh1->r = COLOR_R ;
			uvrgbwh0->g = uvrgbwh1->g = COLOR_G ;
			uvrgbwh0->b = uvrgbwh1->b = COLOR_B ;
			uvrgbwh0->a = uvrgbwh1->a = 0 ;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

}

static inline float NormalizeInnerProduct( FVECTOR *a, FVECTOR *b)
{
	FVECTOR	local_a;
	FVECTOR	local_b;

	_sceVu0Normalize( &local_a, a );
	_sceVu0Normalize( &local_b, b );
    return _sceVu0InnerProduct( &local_a, &local_b );
}

/*------------------------------------------------*/
/* 攻撃 */
static int InitTarget( Work *work )
{
	int	size_max;
	long64	wp_flag=0;
	FVECTOR offset;

	size_max = SIZE_MAX_SMALL;
	switch( work->pattern ){
		case 0:	/* 小 */
		default:
			size_max = SIZE_MAX_SMALL;
			wp_flag = WP_NOBLOOD|WP_STEAM;
			break;
		case 1:	/* 中 */
			size_max = SIZE_MAX_MIDDLE;
			wp_flag = WP_NOBLOOD|WP_STEAM;
			break;
		case 2:	/* 大 */
			size_max = SIZE_MAX_LARGE;
			wp_flag = WP_NOBLOOD|WP_STEAM;
			break;
		case 10:	/* 消化器 */
			size_max = SIZE_MAX_SMALL;
			wp_flag = (WP_NOBLOOD|WP_EXTINGUISHER);
			break;
	}

	if( work->hzd_flag ){
		size_max = 0;
	}


	work->size.vx = size_max * 0.5f * 0.5f;
	work->size.vy = size_max * 0.5f * 0.5f;
	work->size.vz = REFLESH_INTERVAL * SPEED * 0.5f;
	offset.vx = 0.0f;
	offset.vy = 0.0f;
	offset.vz = -(work->size.vz * 0.5f);

	GM_SetTarget( &work->target,
	              TARGET_OFFENSE|TARGET_ROTATE,
	              GM_CurrentStageMap,
	              BOTH_SIDE,
//	              PLAYER_SIDE,
	              &work->size,
	              &offset);
	GM_SetPowerTarget( &work->target,
	                   &work->power,
	                   POWER_CONST,
	                   255,
	                   1,
	                   1,
	                   &DG_ZeroVector);
	GM_SetTargetWeaponType( &work->target, wp_flag );
	GM_SetTargetName( &work->target, 1000 );
	GM_MoveTarget2Map( &work->target, &work->world, GM_CurrentStageMap );

	GM_PutTarget( &work->target );
//	NewTargetView2( &work->target,255,0,0 ) ;

	return 0;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FMATRIX *world, FVECTOR *pole )
{
	int	i,j;
	FVECTOR		*vec;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		norm;
	FVECTOR		fvtemp;
	FVECTOR		center;
	SVECTOR		local_rot;
	SVECTOR		rot;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			*prim_count;

	_sceVu0Normalize( &norm, pole );
	_sceVu0ScaleVector( &fvtemp, &norm, 100.0f );
	_sceVu0AddVector( &center, (FVECTOR *)world->m[3], &fvtemp );

	_sceVu0ScaleVector( &fvtemp, &norm, 1000.0f );
	_sceVu0AddVector( &fvtemp, &center, &fvtemp );

	work->hzd_flag = 0;
	if( HZX_OnlineHazardCheck( /* ハザードチェック */
			HZX_CurrentGroupID,
			&center,
			&fvtemp,
			HZX_CHK_ALL | HZX_CHK_RECOIL_TYPE_ONLY, 
			HZX_SEG_RECOIL_TYPE , HZX_FLOOR_RECOIL_TYPE ) ){
		work->hzd_flag = 1;
//printf("CHECK!!!!!!!!!!!\n");
	}



	switch( work->pattern ){
		case 0:	/* 小 */
		default:
			switch( OK_steam_se_num_s ){
				case 0:
					GM_SeSetMode( SD_A_STEAM05, (FVECTOR *)world->m[3], GM_SEMODE_NORMAL ) ;
					OK_steam_se_num_s++;
					break;
				case 1:
				default:
					GM_SeSetMode( SD_A_STEAM06, (FVECTOR *)world->m[3], GM_SEMODE_NORMAL ) ;
					OK_steam_se_num_s = 0;
					break;
			}
			break;
		case 1:	/* 中 */
			switch( OK_steam_se_num_s ){
				case 0:
					GM_SeSetMode( SD_A_STEAM03, (FVECTOR *)world->m[3], GM_SEMODE_NORMAL ) ;
					OK_steam_se_num_s++;
					break;
				case 1:
				default:
					GM_SeSetMode( SD_A_STEAM04, (FVECTOR *)world->m[3], GM_SEMODE_NORMAL ) ;
					OK_steam_se_num_s = 0;
					break;
			}
			break;
		case 2:	/* 大 */
			switch( OK_steam_se_num_s ){
				case 0:
					GM_SeSetMode( SD_A_STEAM01, (FVECTOR *)world->m[3], GM_SEMODE_NORMAL ) ;
					OK_steam_se_num_s++;
					break;
				case 1:
				default:
					GM_SeSetMode( SD_A_STEAM02, (FVECTOR *)world->m[3], GM_SEMODE_NORMAL ) ;
					OK_steam_se_num_s = 0;
					break;
			}
			break;
		case 10:	/* 消化器 */
			{
				extern void *NewFlour_Gas3( FVECTOR *center );
				extern void *NewSpreadFlour( FMATRIX *world );
				NewFlour_Gas3( (FVECTOR *)world->m[3] );
				NewSpreadFlour( world );
			}
			break;
	}



	DG_COPY_VEC( &work->center, (FVECTOR *)world->m[3] );

	tex = DG_GetTexture( 9998494 /*"powder02_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	fvtemp.vx = 0.0f;
	fvtemp.vy =-1.0f;
	fvtemp.vz = 0.0f;
	DG_SetPos( world );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
	if( NormalizeInnerProduct( &fvtemp, pole ) < 0.0f ){
		pole->vx *= -1.0f;
		pole->vy *= -1.0f;
		pole->vz *= -1.0f;
	}
	OK_DirVecXY( &DG_ZeroVector, pole, &rot );

	rot.vx += irnd()%256;
	rot.vz += irnd()%4096;

	DG_SetPos2( &work->center, &rot );
	DG_GetPos( &work->world );

	vec = work->vec;
	pos0 = work->prim->pos[0];
	pos1 = work->prim->pos[1];
	prim_count = work->prim_count;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			DG_COPY_VEC( pos0, &work->center );
			DG_COPY_VEC( pos1, pos0 );

			pos0->vw = pos1->vw = TPI * rnd();
			vec->vx = 0;
			vec->vy = 0;
			vec->vz = -(SPEED + rnd() * SPEED_RAND);
			if( work->hzd_flag ){
				vec->vz*= 0.125f;
			}
			local_rot.vx = rot.vx + irnd() % ANGLE_RAND - ANGLE_RAND/2;
			local_rot.vy = rot.vy + irnd() % ANGLE_RAND - ANGLE_RAND/2;
			local_rot.vz = irnd() % 4096;
			DG_SetPos2( &DG_ZeroVector, &local_rot );
			DG_RotVector( vec, vec, 1 );
			vec->vw = 0.0f;		/* フラグ兼サイズ */
			(*prim_count) = REFLESH_INTERVAL;

			prim_count++;
			pos0++;
			pos1++;
			vec ++;
		}
	}

	work->life = 0;
	work->count=0;

	InitTarget( work );		// work->sizeも 生成

	// カメラくもり
	NewSteamOnCamera( &work->center, &rot, GV_VecLen3F( &work->size )*2.0f, LIFE_TIME );

	return 0 ;
}

/*-------------------------------------------------*/
void *NewSteamAttackTargetSmall( FMATRIX *world, FVECTOR *pole, int seNo )
{
	Work		*work ;
//printf("NewSteamAttackTarget:Small\n");
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->pattern = 0;

		if ( GetResources( work, world, pole ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void	*NewEntrySteamAttackTargetSmall( void )
{
	GCL_GetOption( 'n' ) ;
	GM_EntrySparkFunction( NewSteamAttackTargetSmall, GCL_GetNextInt() ) ;
	return NewEntrySteamAttackTargetSmall ;
}
/*-------------------------------------------------*/
void *NewSteamAttackTargetMiddle( FMATRIX *world, FVECTOR *pole, int seNo )
{
	Work		*work ;
//printf("NewSteamAttackTarget:Middle\n");
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->pattern = 1;

		if ( GetResources( work, world, pole ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void	*NewEntrySteamAttackTargetMiddle( void )
{
	GCL_GetOption( 'n' ) ;
	GM_EntrySparkFunction( NewSteamAttackTargetMiddle, GCL_GetNextInt() ) ;
	return NewEntrySteamAttackTargetMiddle ;
}
/*-------------------------------------------------*/
void *NewSteamAttackTargetLarge( FMATRIX *world, FVECTOR *pole, int seNo )
{
	Work		*work ;
//printf("NewSteamAttackTarget:Large\n");
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->pattern = 2;

		if ( GetResources( work, world, pole ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void	*NewEntrySteamAttackTargetLarge( void )
{
	GCL_GetOption( 'n' ) ;
	GM_EntrySparkFunction( NewSteamAttackTargetLarge, GCL_GetNextInt() ) ;
	return NewEntrySteamAttackTargetLarge ;
}




/*-------------------------------------------------*/
void *NewExtinguisherGas( FMATRIX *world, FVECTOR *pole, int seNo )
{
	Work		*work ;
//printf("NewExtinguisherGas::\n");
	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->pattern = 10;

		if ( GetResources( work, world, pole ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
void	*NewEntryExtinguisherGas( void )
{
	GCL_GetOption( 'n' ) ;
	GM_EntrySparkFunction( NewExtinguisherGas, GCL_GetNextInt() ) ;
	return NewEntryExtinguisherGas ;
}




