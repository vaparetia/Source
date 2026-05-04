//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sig_grd.c
	グレネード
	$Id: sig_grd.c,v 1.1.1.3 2002/11/19 11:49:52 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"libdg.h"
#include	"libdg.cnf"
#include	"gameheader.h"
#include	"libutl.h"
#include "korekado/enemy/enemy.h"

extern float _RcosF( int);
extern float _RsinF( int);

#ifdef DEBUG_MODE
extern void PosBox(FVECTOR * ,float ,SVECTOR * );
#endif
extern	void	*NewBlast3( FVECTOR *, int, int, int, int, int, int, int ) ;
//extern	void	*NewBlastChaff( FMATRIX * ) ;
extern	void	*NewStanEffect( FVECTOR * , int ) ;

extern	void	*NewInterPoly_Gr( FVECTOR *, float , CVECTOR );

typedef	struct	{
	GV_ACT_EX			actor ;
    CONTROL_NOEVT		ctrl ;
	DG_OBJS		*objs;
	FMATRIX		light[2] ; 
	FVECTOR		end ;	/*目標位置*/
	FVECTOR		start ;	/*デバグ用 発射位置保持*/
	FVECTOR		step_buf ;	
    SVECTOR		rot ;
	int cnt ;	
	int touch ;			/*反射処理用*/
	float terminal ;	/*デバグ用 最高高さ*/
	int	endcnt ;		/*目標到達予測時間*/
	int	blast_time ;	/*発射から爆発まで*/
	int	side ;			/*攻撃サイド*/
	short	mode ;
	short	type ;

//	TARGET		trg;
//	POWER_TARGET power;
}Work ;

enum {
	GRD_FLY = 0,
	GRD_BOUND ,
	GRD_STOP
} ;

enum {
	TYPE_GRD  = 0,
	TYPE_CHF ,
	TYPE_STN ,
	TYPE_M4_GRD,
	TYPE_NO_AVOID_GRD
} ;

/*blast timeの特種値*/
enum {
	BLAST_TIME_QUICK = -1 /*着弾同時爆破*/
};
#if 0
PLAYER_SIDE, WP_Grenade, MDL_GRENADE 
#endif

#define DIE_CNT (120)
#define DEFAULT_HIGH (1000.0F)

#define GRD_GRAV	( -9.0F)

static FVECTOR	ShiftGrenade = { 29.5F, -110.5F, 20.5F } ;

static void BoundCheck(Work *work){
	int type ;
	FVECTOR		ptp ;

	if(work->mode >= GRD_STOP) return ;
	if ( work->ctrl.n_touches != 0 ) {
		type = HZX_GetOnlineHazardType() ;
		HZX_GetOnlinePoint( &ptp ) ;
		if ( type == 1 ) {
		} else {
			/* 床 */
			if(work->mode == GRD_FLY){
//				work->ctrl.step.vy *= (-0.8F) ;
				work->ctrl.step.vy = 0.1F*GV_VecLen3F( &work->step_buf );
//printf("REFRECT V %f\n",work->ctrl.step.vy);
				work->ctrl.step.vx *= 0.5F ;
				work->ctrl.step.vz *= 0.5F ;
				work->ctrl.mov.vy = ptp.vy ;
				work->ctrl.mov.vy += (50.0F) ;
				GM_SeSetMode( SD_W_BOUND02, &work->ctrl.mov, GM_SEMODE_BOMB ) ;
//printf("First Hit Cnt %d pos y %f\n",work->cnt,ptp.vy);
				GV_CallChildSignalFunc( work, GV_SIGNAL_KILL, 0 );
			}
			if(work->mode == GRD_BOUND){
				if ( ptp.vy < work->ctrl.mov.vy ) { 
					/* 床バウンド */
					work->ctrl.step = DG_ZeroVector ;
					work->ctrl.mov.vy = ptp.vy ;
					work->ctrl.mov.vy += (40.0F) ;
				GM_SeSetMode( SD_W_BOUND02, &work->ctrl.mov, GM_SEMODE_BOMB ) ;
//printf("Secound Hit Cnt %d pos y %f\n",work->cnt,ptp.vy);
				}
			}
			if(work->mode < GRD_STOP) work->mode++;
		}
	}
}
static	int	InitControl( Work *work )
{
	CONTROL	*ctrl ;

	ctrl = ( CONTROL * )&( work->ctrl ) ;
	if ( GM_InitControl( ctrl, 0, GM_PlayerMap ) < 0 ) return -1 ;
	GM_ConfigControlHazard( ctrl, 40, 80, 80 ) ;
	ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	/* onlineチェックのみ行う */
#if 0
	ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
#else
	/*最初は壁見ない*/
	ctrl->skip_flag |= (CTRL_SKIP_HZX| CTRL_SKIP_TRAP|CTRL_SKIP_MESSAGE|CTRL_SKIP_CHANGE_MAP|CTRL_SKIP_GET_ADDRESS);
#endif
	return 0 ;
}

/* 位置・回転更新 */
static	int		UpdatePos( work )
Work		*work ;
{
	FVECTOR	v1, mov, shift ;
	if ( work->mode != GRD_STOP )  {
//	if ( 1 )  {
		/* PAL用速度調整 */
//		PL_PalAdjV( &work->control.step ) ;
		GM_ActControl( ( CONTROL * )&work->ctrl ) ;	
//		PL_PalAdjVR( &work->control.step ) ;
	}
	/* ワールドリミットで消す */
	if ( GM_CtrlWorldLimit( ( CONTROL * )&work->ctrl ) ) {
//printf("CONTORL OUT OF LIMIT\n");
		return -1 ;
	}
	GV_MatToVec( &work->objs->world, &v1 ) ;
	DG_SetPos2( &v1, &work->rot ) ;
	DG_GetPos( &work->objs->world ) ;
	DG_RotVector( &ShiftGrenade, &shift, 1 ) ;
	_sceVu0SubVector( &mov, &work->ctrl.mov, &shift ) ;
	GV_VecToMat( &mov, &work->objs->world ) ;
	return 0 ;
}

static void BlastEnd(Work *work){
	extern void *NewStanEffect_LifeVariance( FVECTOR *pos, int mode, int life ) ;
	int dmg ;
	HZX_ZON *pl_zone;
	pl_zone = ENE_HZX_GetZone(GM_PlayerAddress);
	if(pl_zone->flag&(HZX_ZONE_NO_AVOID|HZX_ZONE_INTRUDE)){
		dmg = ENE_GRENADE_NO_AVOID_DMG ;
	}else {
		dmg = ENE_GRENADE_DMG ;
	}
//printf("GRD TYPE [%d] dmg [%d]\n",work->type,dmg);
	switch (work->type){
		case TYPE_GRD :
			NewBlast3( &work->end, work->side, 2000, 
//			2000, DMG_BLAST, FNT_BLAST, WP_Grenade, BLAST_TYPE_NO_NOISE ) ;
			2000, dmg, FNT_BLAST, WP_Grenade, BLAST_TYPE_NO_NOISE|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER) ;
printf("GRD DMG [%d]\n",dmg);
			break;
		case TYPE_M4_GRD :
			NewBlast3( &work->end, work->side, 2000, 
			2000, ENE_M4GRD_DMG, FNT_BLAST, WP_Grenade, BLAST_TYPE_NO_NOISE|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER ) ;
printf("GRD DMG [%d]\n",ENE_M4GRD_DMG);
			break ;
		case TYPE_NO_AVOID_GRD :
			NewBlast3( &work->end, work->side, 2000, 
			2000, ENE_GRENADE_NO_AVOID_DMG, FNT_BLAST, WP_Grenade, BLAST_TYPE_NO_NOISE|BLAST_TYPE_WALLTHROUGH|BLAST_TYPE_NO_PLAYER ) ;
printf("GRD DMG [%d]\n",ENE_GRENADE_NO_AVOID_DMG);
			break ;
		case TYPE_CHF :
//			NewBlastChaff( &work->objs->world ) ;
			break ;
		case TYPE_STN :
//			NewStanEffect( &work->end , 1 ) ;
			NewStanEffect_LifeVariance( &work->end, 1, (60 * 6) ) ;
			GM_SeSetMode( SD_W_EXPLOS03, &work->ctrl.mov, GM_SEMODE_BOMB ) ;
			break ;
	}
	GV_DestroyActor( work ) ;
}
static void Act(Work *work)
{
	SVECTOR col ;

	work->step_buf = work->ctrl.step ;
	if((work->touch == 0)
	&&((work->endcnt-1) <= work->cnt)
	){
//		printf("GRD TOUCH NEAR %f\n",GV_VecLen3F2( &work->ctrl.mov, &work->end )) ;
//	work->ctrl.skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;
	work->ctrl.skip_flag = (CTRL_SKIP_NEAR_CHECK|CTRL_SKIP_FLR_CHECK|CTRL_SKIP_TRAP|CTRL_SKIP_MESSAGE|CTRL_SKIP_CHANGE_MAP|CTRL_SKIP_GET_ADDRESS) ;
		work->touch = 1 ;
	}

	switch(work->mode){
		case GRD_FLY :
		if(work->type != TYPE_M4_GRD){
//			work->step = work->control.step ;
			work->rot.vx += 128 ;
			work->rot.vy += 128 ;

		}
		break ;
		case GRD_BOUND :
		work->rot.vx += 128 ;
		work->rot.vy += 128 ;
			break ;
		case GRD_STOP :
			break ;
	}

	UpdatePos(work );
	GM_GroupObjs( work->objs, GM_CurrentStageMap  ) ;
	DG_GetLightMatrix( &work->ctrl.mov, work->light );

	work->ctrl.step.vy += GRD_GRAV;
#ifdef DEBUG_MODE
//	PosBox( &work->end ,10.0F , NULL );
#endif
	col.vz =127 ;
//	PosBox( &work->ctrl.mov ,50.0F , &col );
//	PosBox( &work->start ,200.0F , NULL );

	BoundCheck( work ) ;

#if 1
	if((work->endcnt-1) <= work->cnt){
		work->ctrl.step.vx *= 0.90F ;
		work->ctrl.step.vz *= 0.90F ;
	}
	if(work->touch == 1 ){
		work->ctrl.step.vx *= 0.50F ;
		work->ctrl.step.vz *= 0.50F ;
	}
#endif
	work->cnt++ ;
	if(work->blast_time == BLAST_TIME_QUICK){
		if(work->touch == 1){
			BlastEnd(work) ;
		}
	}else {
		if(work->cnt >= work->blast_time ) {
			BlastEnd(work) ;
		}
	}
}
static void Die(Work *work)
{
    if ( work->objs != NULL ){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
    }
	GM_FreeControl((CONTROL *)&work->ctrl);
}
/*資源確保と各種初期設定*/
static int GetResources( Work *work, FVECTOR *start ,FVECTOR *end ,float high ,int side ,int time ,short type){
extern void *NewSAA_Smoke( FMATRIX *, int ) ;
    DG_DEF  *def ;
	FVECTOR sub ;
	float endtime ,len ,base_vel;

	if(type == TYPE_M4_GRD){
		def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_M4B_GRD_BUL, 'k' ) ) ;
	}else {
		def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_GRENADE, 'k' ) ) ;
	}
	work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_FINISHCALC, DG_CHANL_MAIN ) ;
	ASSERT(work->objs != NULL ) ;
	DG_QueueObjs( work->objs ) ;
	DG_SetLightMatrix( work->objs, work->light );

	InitControl( work ) ;

	work->start = work->ctrl.mov = *start ;
	work->end = *end ;

//	work->end.vy += 50.0f ;
	_sceVu0SubVector( &sub, &work->end, start ) ;
	len = GV_VecLen3F( &sub ) ;
	base_vel = bp_sqrtf(  (DG_FABS(GRD_GRAV)*high)*2.0F) ;   //BP_MATH - emulate PS2 sqrtf
	endtime =  (base_vel/DG_FABS(GRD_GRAV))*2 ;
	GV_LenVec3F( &sub, &work->ctrl.step, 0.0F, len/endtime ) ;

	if(type == TYPE_M4_GRD){
		work->rot.vx = 1024;
		work->rot.vy = GV_VecDir2( &sub ) ;
	}

	work->ctrl.step.vy += base_vel ;
	work->cnt = 0 ;
	work->touch = 0 ;
	work->terminal = -30000.0F ;
	work->endcnt = (int) endtime ;
	work->blast_time = time ;
	work->side = side ;
	work->mode = GRD_FLY ;
	work->type = type ;
	{	
		CVECTOR grd_col ;
		grd_col.r = 60 ;
		grd_col.g = 60 ;
		grd_col.b = 60 ;
		grd_col.cd = 27 ;
		if(type == TYPE_M4_GRD){
			GV_SetActorChild( work , 
			NewInterPoly_Gr( &work->ctrl.mov, 5.0F , grd_col ) );
		}else if(type == TYPE_NO_AVOID_GRD){
			/*NO LINE*/
		}else {
			GV_SetActorChild( work , 
//			NewInterPoly_Gr( &work->ctrl.mov, 20.0F , grd_col ) );
			NewInterPoly_Gr( &work->ctrl.mov, 5.0F , grd_col ) );
		}
	}

	GV_SetActorChild( work , NewSAA_Smoke( &work->objs->world, time ) ) ;
	return 1;
}
void	*NewMagicGrd( FVECTOR *start ,FVECTOR *end ,float high ,int side ,int time ,short type)
{
	Work		*work ;
	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), AFTER_ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, start , end , high , side , time,type ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

