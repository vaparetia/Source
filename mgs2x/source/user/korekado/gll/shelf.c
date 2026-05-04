//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	shelf.c
	棚

	2002/08/16 Y.Korekado
	$Id: shelf.c,v 1.3 2002/11/23 12:46:55 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"gll_def.h"

#include	"korekado/conv/korekado.x"
/*----------------------------------------------------------------*/
extern void  *NewSpreadFlour(FMATRIX *world ) ;
extern void  *NewFlour_Splay(FMATRIX  *world) ;
extern void *NewFlour_Fall( int map, int name, FVECTOR *hit, FVECTOR *force );
extern void *NewFlour_Down( int map, int name, FVECTOR *hit, FVECTOR *force );
extern void *NewGllFlourSara( int map, int name, FVECTOR *hit, int mode );
extern void *NewFlour_Gas3( FVECTOR *center );
/*----------------------------------------------------------------*/
#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT)
//#define HEART_MODEL	(919420)	/* GV_StrCode("vr2_tgt_octa_nov" ) */
#define SHELF_MODEL	(13288496)	/* GV_StrCode("vr2_tgt_straw" ) */

#define MAX_LIFE	(100)
#define MIN_SIZE	(10.0)
#define SIZE_RANGE	(100.0 - MIN_SIZE)
/*----------------------------------------------------------------*/
typedef	struct	{
    GV_ACT_EX	actor;
	OBJECT		body ;

	FVECTOR		scale ;
	FVECTOR		pos ;
	SVECTOR		rot ;
	FVECTOR		*trg_size ;

	int			*flag ;
	int			life ;
	int			mode ;

	TARGET		deftrg ;	/* 防御ターゲット */
	POWER_TARGET	power ;	/* 防御属性 */
	FVECTOR		off_center ;
}Work ;

/*----------------------------------------------------------------*/
#define TARGET_CLASS	(TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK)
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
static void	ScaleMatrix( FMATRIX *mat, FVECTOR *vec )
{
	mat->m[0][0] *= vec->vx ;
	mat->m[1][0] *= vec->vx ;
	mat->m[2][0] *= vec->vx ;

	mat->m[0][1] *= vec->vy ;
	mat->m[1][1] *= vec->vy ;
	mat->m[2][1] *= vec->vy ;

	mat->m[0][2] *= vec->vz ;
	mat->m[1][2] *= vec->vz ;
	mat->m[2][2] *= vec->vz ;

//printf("scalemat> [%f][%f][%f]\n",mat->m[0][0],mat->m[1][1],mat->m[2][2] );
}

static FVECTOR TargetSizeX = { 1000.0, 100.0, 100.0 } ;
static FVECTOR TargetSizeZ = { 100.0, 100.0, 1000.0 } ;

static void SetScale( Work *work )
{
	FVECTOR	t_size ;
	float f, k, l, m ;

	m = (float)MAX_LIFE ;
	l = (float)work->life ;
	if ( l < 0 ) l = 0.0 ;
	k = SIZE_RANGE * l / m ;
	f = (k+10.0) / 100.0f ;
	f *= 3.0 ;
#if 1
	work->scale.vx = f ;
	work->scale.vy = f ;
	work->scale.vz = f ;
#else
	d = (work->mode) ? 2.0 : 3.0 ;
	work->scale.vx = f * d ;
	work->scale.vy = f ;
 	d = (work->mode) ? 3.0 : 2.0 ;
	work->scale.vz = f * d ;
#endif

	_sceVu0ScaleVector( &t_size, work->trg_size, f );	

	GM_SetTargetSize( &work->deftrg, &t_size ) ;
}

static void SetObject( Work *work )
{
	FMATRIX	mat ;

	DG_SetPos2( &work->pos, &work->rot ) ;
	DG_GetPos( &mat ) ;

	ScaleMatrix( &mat, &work->scale ) ;
	DG_SetPos( &mat ) ;
	DG_PutObjs( work->body.objs ) ;
}

static void Blood( Work *work, int mode )
{
	extern void *NewSigBreakObj2(int mdlcode,FMATRIX *world,int mode ,float len,int time_a ,int time_b,int *sw ) ;
	extern void *NewBlood( FMATRIX *, FVECTOR *, FVECTOR *, int, int );
	FMATRIX	*world ;
	FVECTOR force ;
	TARGET	*dam ;

	dam = &work->deftrg ;
	world = &work->body.objs->objs[0].world ;
	force.vx = dam->power->force.vx + (float)KR_RandS( 256 ) ;
	force.vy = dam->power->force.vy + (float)KR_RandS( 256 ) ;
	force.vz = dam->power->force.vz + (float)KR_RandS( 256 ) ;
//	GV_SetActorChild( work, NewBlood( world, &dam->center, &force, mode, 0 ) ) ;

	{
		NewSigBreakObj2( SHELF_MODEL, world, 0x1014, 2000.0, 12 , 2, NULL ) ;
		GM_SeSetMode( SD_A_V_MTHIBI, (FVECTOR *)&work->body.objs->objs[0].world.m[3][0], GM_SEMODE_BOMB ) ;
	}
/*
int mdlcode モデルコード
FMATRIX *world 表示座標
int mode 壊れの時は(0x1014)
float len 分解距離
int time_a 分解時間
int time_b 分解が端から始まり全体に行き渡るまでの時間
int *sw  とりあえずNULLでok
*/

}


static void  VecToMat(FMATRIX *world, FVECTOR *force, FVECTOR *hit, int mode)
{
    FVECTOR   base = {0.0F, -1.0F, 0.0F,0.0F} ;
    FVECTOR   ftmp[3] ;
    int       tmp ;

    /* 1つめのベクトル */
    _sceVu0Normalize(&ftmp[0],force) ;
    
    /* force と base の外せきを求める */
    _sceVu0OuterProduct(&ftmp[1],&base,force) ;
    /* 2つめのベクトル */
    _sceVu0Normalize(&ftmp[1],&ftmp[1]) ;
    /* 3つめのベクトル */
    _sceVu0OuterProduct(&ftmp[2],&ftmp[1],&ftmp[0]) ;

    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[2]) = ftmp[tmp] ;
    
    /* 2ビットずらす */
    mode /= 4 ;
    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[1]) = ftmp[tmp] ;
    
    /* 2ビットずらす */
    mode /= 4 ;
    tmp = mode & 0x03 ;
    *((FVECTOR *)world->m[0]) = ftmp[tmp] ; 

    *((FVECTOR *)world->m[3]) = *hit ;
      world->m[3][3] = 1.0F ;
}

static void CheckDamage( Work *work )
{
	FMATRIX	   world ; 
	FVECTOR	   hit ;
	float		 tmp ;
	TARGET		*trg  ;
	FVECTOR	   t_force ;
	int		   mode ;
	
	trg = &work->deftrg ;

	*work->flag &= ~GLL_SHELF_DAMAGE ;
	*work->flag &= ~GLL_SHELF_BREAK ;
	*work->flag &= ~GLL_SHELF_BOMB ;
	if ( TARGET_POWER & trg->damaged ) {
		t_force = trg->power->force ;
		if ( trg->weapon_type  & (WP_BLAST) ) {
			Blood( work, 1 ) ;
			if ( !(GLL_GAME_STATUS & GLL_GS_ACHOO_TIME ) ) {
				/*シナリオデモのためくしゃみ発射中はこわれない */
				work->life -= 10 ;
			}
			*work->flag |= GLL_SHELF_BOMB ;
		}
		if ( trg->weapon_type  & (WP_BULLET) ) {
			Blood( work, 1 ) ;

			if ( !(GLL_GAME_STATUS & GLL_GS_ACHOO_TIME ) ) {
				/*シナリオデモのためくしゃみ発射中はこわれない */
				work->life -= 1 ;
			}
			*work->flag |= GLL_SHELF_DAMAGE ;
		}
printf("life [%d] \n",work->life ) ;
		if ( work->life <= 65 ) {
			GM_SeSetMode( SD_A_V_CLASHA, (FVECTOR *)&work->pos, GM_SEMODE_BOMB ) ;
			*work->flag |= GLL_SHELF_DESTROY ;
			*work->flag |= GLL_SHELF_BREAK ;
		}

		if( trg->weapon_type & WP_BLOW ){
			_sceVu0AddVector( &hit, &trg->center, &trg->offset );
		}else{
			DG_COPY_VEC( &hit, &trg->hit );
		}
		GM_SeSetMode( SD_A_RICKOM01, &hit, GM_SEMODE_BOMB ) ;
		world = DG_UnitMatrix ;
		DG_COPY_VEC( (FVECTOR *)world.m[3], &trg->hit );
		NewSpreadFlour(&world) ;

		/* 飛び散りの向きを測定 */
		if(t_force.vy < 0.0F)
		t_force.vy = - t_force.vy ;
		tmp = bp_sqrtf(t_force.vx * t_force.vx + t_force.vz * t_force.vz) ;  //BP_MATH - emulate PS2 sqrtf
		if(t_force.vy < tmp)
		t_force.vy = tmp ;
		mode = 0x0012 ;
		VecToMat(&world,&t_force,&hit,mode) ;

		NewFlour_Gas3(&hit);
		/* 飛び散りの小麦粉 */
		NewFlour_Splay( &world ) ;

		trg->weapon_type = 0 ;
		trg->damaged = 0 ;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
#if 1
	if ( GLL_GAME_STATUS & GLL_GS_TUB_KILL ) {
		if ( !(*work->flag & GLL_SHELF_KILL_EFE) ) {
			extern void *NewSigBreakObj6(DG_OBJS *,float ,int ,int ) ;

			GV_SetActorChild( work, NewSigBreakObj6((work->body.objs),5000.0f ,30 ,15) ) ; 
			*work->flag |= GLL_SHELF_KILL_EFE ;
			work->deftrg.class |= TARGET_SKIP ;
//			work->body.objs->flag |= DG_FLAG_INVISIBLE ;
		}
		return ;
	}
#else
	if ( *work->flag & GLL_SHELF_TUB_KILL ) {
		work->deftrg.class |= TARGET_SKIP ;
		work->body.objs->flag |= DG_FLAG_INVISIBLE ;
		return ;
	}
#endif

	CheckDamage( work ) ;
	SetScale( work ) ;
	SetObject( work ) ;
	if ( *work->flag & GLL_SHELF_BREAK ) {
		work->deftrg.class |= TARGET_SKIP ;
		work->body.objs->flag |= DG_FLAG_INVISIBLE ;
	}

}

static	void	Die( Work *work )
{
	DG_FreePreshade( work->body.objs ) ;
    GM_FreeTarget( &work->deftrg ) ;
	GM_FreeObject( &work->body ) ;
}

/*----------------------------------------------------------------*/
static	void	TargetCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	
	work = (Work *)ptr ;

	if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = DG_ZeroVector ;
			GM_DamageTarget( off, def ) ;
			work->off_center = off->center ;
//printf("koreeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee [%lx] \n",off->weapon_type ) ;
		}
	}
}

static void	InitTarget( Work *work )
{
	TARGET		*trg  ;
	FVECTOR 	shift ;

	trg = &work->deftrg ;

	shift.vx = 0.0 ;	shift.vy = 0.0 ;	shift.vz = 0.0 ;

	work->trg_size = (work->mode) ? &TargetSizeZ : &TargetSizeX ;
	GM_SetTarget( trg, TARGET_CLASS, 1, ENEMY_SIDE,  work->trg_size, &shift ) ;
	GM_SetTargetWeaponType( trg, 0 ) ;
    GM_SetPowerTarget( trg, &work->power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, TargetCallBack, work ) ;

    GM_MoveTarget( trg, &work->pos ) ;
//NewTargetView( trg, 200, 34, 184 ) ;
}

static	int	GetResources( Work *work, int *flag, FVECTOR *pos, SVECTOR *rot, int life )
{
	work->life = life ;

	GM_InitObject( &work->body, SHELF_MODEL, BODY_FLAG );

	work->pos = *pos ;
	work->rot = *rot ;
	work->flag = flag ;

	work->scale.vx = 1.0F ;
	work->scale.vy = 2.0F ;
	work->scale.vz = 1.0F ;


printf(" aaaaaaaaaaaaaaaaaaaaaaa rot vy[%d]\n",work->rot.vy ) ;
	if ( (work->rot.vy == 0) || work->rot.vy == 2048 ) {
		work->mode = 0 ;
	} else {
		work->mode = 1 ;
	}
	InitTarget( work ) ;
	SetObject( work ) ;

	{
		LIT_DEF	*def ;
		int	grp_id ;
			
		grp_id = GM_GetDGGroupID( GM_CurrentMap ) ;
		def = DG_GetFixedLight( grp_id ) ;
		DG_MakePreshade( work->body.objs, def );
	}

	return 0 ;
}

void	*NewGllShelf( int *flag, FVECTOR *pos, SVECTOR *rot, int life )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX(&work->actor) ;
		if ( GetResources( work, flag, pos, rot, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
