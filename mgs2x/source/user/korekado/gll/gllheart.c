//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gllheart.c
	ごるるごん心臓

	2002/07/09 Y.Korekado
	$Id: gllheart.c,v 1.1.1.3 2002/11/19 11:44:12 Yoshizawa1 Exp $

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
#include	"camera.h"
#include	"gllheart.h"

#include	"korekado/conv/korekado.x"
/*----------------------------------------------------------------*/
#define BODY_FLAG (DG_FLAG_IRREACTION|DG_FLAG_SHADE)
//#define	BODY_FLAG	(DG_FLAG_TEXT|DG_FLAG_PAINT)
//#define HEART_MODEL	(13374045)	/* GV_StrCode("vr2_tgt_octa" ) */
#define HEART_MODEL	(919420)	/* GV_StrCode("vr2_tgt_octa_nov" ) */

#define MAX_LIFE	(100)
#define MIN_SIZE	(10.0)
#define SIZE_RANGE	(100.0 - MIN_SIZE)

#define WAVE_LEN	(3000)
/*----------------------------------------------------------------*/
typedef	struct	{
    GV_ACT_EX	actor;
	OBJECT		body ;

	FMATRIX		lights[2] ;
	FVECTOR		scale ;

	OBJECT		*gllbody ;
	FVECTOR		shift ;
	int			status ;
	int			obj_n ;
	int			life ;
	int			min_life ;
	int			*flag ;
	TARGET		deftrg ;	/* 防御ターゲット */
	POWER_TARGET	power ;	/* 防御属性 */
	FVECTOR		off_center ;
	int			wave_time ;
	int			light_time ;
	int			turn_time ;
	int			mode ;
}Work ;

/*----------------------------------------------------------------*/
#define FLAG_DAMAGED	0x00000001
#define FLAG_BRAKED		0x00000002

#define TARGET_CLASS	(TARGET_POWER|TARGET_DEFENSE|TARGET_SEEK)

/*----------------------------------------------------------------*/
extern void *NewMakeObjectBoundHazard( HZX_GROUP_ID	hzx_id, DG_OBJS *objs, u_int seg_atr, u_int flr_atr ) ;
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

static FVECTOR TargetSize = { 400.0, 400.0, 400.0 } ;

static void SetScale( Work *work )
{
	FVECTOR	t_size ;
	float f, k, l, m ;

	m = (float)MAX_LIFE ;
	l = (float)work->life + work->min_life ;
	if ( l < 0.0 ) l = 0.0 ;
	k = SIZE_RANGE * l / m ;
	f = (k+10.0) / 100.0f ;
	work->scale.vx = f ;
	work->scale.vy = f ;
	work->scale.vz = f ;

	_sceVu0ScaleVector( &t_size, &TargetSize, f );	

	GM_SetTargetSize( &work->deftrg, &t_size ) ;

//	work->deftrg.size.vx = TargetSize.vx * f ;
//	work->deftrg.size.vy = TargetSize.vy * f ;
//	work->deftrg.size.vz = TargetSize.vz * f ;
}

static void SetObject( Work *work )
{
	FMATRIX	mat ;
	SVECTOR turn ;

	mat = work->gllbody->objs->objs[work->obj_n].world ;
	DG_SetPos( &mat ) ;
	DG_MovePos( &work->shift ) ;
	DG_GetPos( &mat ) ;
	ScaleMatrix( &mat, &work->scale ) ;
	DG_SetPos( &mat ) ;

	if ( work->mode == 2 ) {
		FVECTOR wave_shift ;
		int	d ;
		float l ;
		
		d = work->wave_time%128 ;
		d *= 4096/128 ;
		
		l = ((float)MAX_LIFE - (float)work->life) / (float)MAX_LIFE ;
		l *= (float)WAVE_LEN ;
		l = l*_RsinF( d ) ;
		wave_shift.vx = 0.0 ;
		wave_shift.vy = l ;
		wave_shift.vz = 0.0 ;
		wave_shift.vw = 0.0 ;
//printf("head[%f]\n",l) ;
		_sceVu0ApplyMatrix( &wave_shift, &work->gllbody->objs->objs[work->obj_n].world, &wave_shift ) ;
		DG_MovePos( &wave_shift ) ;
	}

	turn.vx = 0 ;
	turn.vy = work->turn_time * 32 ;
	turn.vz = 0 ;
	DG_RotatePos( &turn ) ;

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
		NewSigBreakObj2( HEART_MODEL, world, 0x1014, 2000.0, 12 , 2, NULL ) ;
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

static void CheckDamage( Work *work )
{
	TARGET		*trg  ;
	
	trg = &work->deftrg ;

	*work->flag &= ~GLL_HEART_DAMAGE ;
	if ( TARGET_POWER & trg->damaged ) {
		if( *work->flag & GLL_BODY_DAMAGE ) {
			if ( trg->weapon_type  & (WP_BLAST) ) {
				Blood( work, 1 ) ;
				work->life -= 10 ;
				work->light_time = 16 ;/*明るく*/
			}
			if ( trg->weapon_type  & (WP_BULLET) ) {
				Blood( work, 1 ) ;
//				work->life -= 1 ;
//				work->life -= 2 ;
				work->life -= 4 ;
				work->light_time = 16 ;/*明るく*/
			}
			if ( work->life <= 0 ) {
				*work->flag |= GLL_HEART_DESTROY ;
				GM_SeSetMode( SD_A_V_CLASHA, (FVECTOR *)&work->body.objs->objs[0].world.m[3][0], GM_SEMODE_BOMB ) ;
			}
			*work->flag |= GLL_HEART_DAMAGE ;
		}
		trg->weapon_type = 0 ;
		trg->damaged = 0 ;
	}
}

static void CheckStatus( Work *work )
{
	if( !(*work->flag & GLL_BODY_DAMAGE) ) {
		work->deftrg.class |= TARGET_SKIP ;
	} else {
		work->deftrg.class &= ~TARGET_SKIP ;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	CheckDamage( work ) ;
	CheckStatus( work ) ;
	if ( !(*work->flag & GLL_HEART_DESTROY) ) {
		SetScale( work ) ;
		SetObject( work ) ;
		GM_MoveTarget( &work->deftrg, (FVECTOR *)&work->body.objs->objs[0].world.m[3][0] ) ;
		if ( *work->flag & GLL_BODY_SPCLEAR ) {
			work->deftrg.class |= TARGET_SKIP ;
			work->body.objs->flag |= DG_FLAG_INVISIBLE ;
		}
	} else {
		work->deftrg.class |= TARGET_SKIP ;
		work->body.objs->flag |= DG_FLAG_INVISIBLE ;
	}
	
	{
		int d, l ;

		d = work->light_time%64 ;
		d *= 4096/64 ;
		
		l = (int)(64.0*_RsinF( d ));
		work->lights[1].m[3][0] = 128 + l ;/*常に明るく*/
		work->lights[1].m[3][1] = 128 + l ;
		work->lights[1].m[3][2] = 128 + l ;
	}
	
	work->wave_time ++ ;
	work->light_time ++ ;
//	work->turn_time ++ ;
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

	GM_SetTarget( trg, TARGET_CLASS, 1, ENEMY_SIDE,  &TargetSize, &shift ) ;
	GM_SetTargetWeaponType( trg, 0 ) ;
    GM_SetPowerTarget( trg, &work->power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_PutTarget( trg ) ;
	GM_SetTargetCallBack( trg, TargetCallBack, work ) ;

    GM_MoveTarget( trg, (FVECTOR *)&work->body.objs->objs[0].world.m[3][0] ) ;
//NewTargetView( trg, 200, 34, 184 ) ;
}

static	int	GetResources( Work *work, OBJECT *body, int obj_n, FVECTOR *shift, int life, int *flag, int mode )
{
	work->gllbody = body ;
	work->obj_n = obj_n ;
	work->shift = *shift ;
	work->life = life ;
	work->flag = flag ;
	work->mode = mode ;
	work->wave_time = 0 ;
	work->light_time = 0 ;
	work->turn_time = 0 ;

	if ( work->mode == 1 ) {
		work->min_life = 20 ;
	} else {
		work->min_life = 0 ;
	}

	GM_InitObject( &work->body, HEART_MODEL, BODY_FLAG );
	GM_ConfigObjectLight( &work->body, work->lights );
	work->lights[1].m[3][0] = 128 ;/*常に明るく*/
	work->lights[1].m[3][1] = 128 ;
	work->lights[1].m[3][2] = 128 ;


	work->scale.vx = 1.0F ;
	work->scale.vy = 1.0F ;
	work->scale.vz = 1.0F ;

	InitTarget( work ) ;

	SetObject( work ) ;

	if ( 0 ){
		LIT_DEF	*def ;
		int	grp_id ;
			
		grp_id = GM_GetDGGroupID( GM_CurrentMap ) ;
		def = DG_GetFixedLight( grp_id ) ;
		DG_MakePreshade( work->body.objs, def );
	}

	return 0 ;
}

void	*NewGllHeart( OBJECT *body, int obj_n, FVECTOR *shift, int life, int *flag, int mode )
{
	Work		*work ;

	OPERATOR() ;
//	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0X40 ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
	    GV_ActorEX(&work->actor) ;
		if ( GetResources( work, body, obj_n, shift, life, flag, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
