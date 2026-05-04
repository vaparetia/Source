//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	赤外線トラップ用のC4	
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
#include "libutl.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#ifdef PSX2
#include	"def_dma.h"
#endif
#ifdef PSX2
#include	"utl_dma.h"
#endif
#include	"../etc/ok_util.h"

/* ---------------------------------------------------------------- */
	/*
		補助マクロ
	*/

#define  FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define   BODY_FLAG (DG_FLAG_TEXT | DG_FLAG_FINISHCALC | DG_FLAG_SHADE | DG_FLAG_ONEPIECE)


#define   N_PRIMS   1
#define   N_VERTS   1
#define   PRIM_TYPE		 (DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA)

extern void *NewBombEffect( FVECTOR *pos, int mode );

enum {
	TRAP_C4_NONE = 0,
	TRAP_C4_EXPLOSE,
	TRAP_C4_LAMP_OFF,
} ;

typedef  struct
{
	GV_ACT_EX		  actor ;
	int			map ;
	int			name ;

	int			all_off;
	int			lamp_off;
	int			m9_active;

	int			noise;
	int			proc_id;

	int			hit_flag ;
	int			target_flag ;

	TARGET			target ;
	POWER_TARGET	power  ;

	FVECTOR		pos ;
	SVECTOR		rot ;

	DG_OBJS		*objs ;
	DG_MDL		*mdl ;
	FMATRIX		light[2] ;

	/* ボンボリのエフェクト */
	DG_PRIM2		*prim ;
	int			 count ;
} Work ;

extern void *NewBombGasEffect( FVECTOR *pos, SVECTOR *pole_rot );
extern void *NewBombKasu( FVECTOR *pos, SVECTOR *pole_rot );
extern void *NewFlyingSmoke( FVECTOR *pos, FVECTOR *vector, int life );
extern void *NewBlast( FVECTOR *vec, int side, int range1, int range2, int damage, int faint, int wp );
extern void *NewBlast3( FVECTOR *vec, int side, int range1, int range2, int damage, int faint, int wp, int flag );

/* -------------------------------------------------------------------*/
/* オブジェを開放して、爆破する */
#define	SEARCH_HZX			(2000)

static  void IRS_C4_Explosion(Work *work,TARGET *t)
{
	HZX_GROUP_ID	map_id;
	float		flr_height[2];
	float		rad;
	float		ftemp;
	FVECTOR		fvtemp;
	int			seg_num;
	int			atr[2] ;
	HZX_SEG		seg[2] ;
	SVECTOR		pole_rot;

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );
	seg_num = HZX_NearHazardCheck( map_id,
					&work->pos,
					SEARCH_HZX,
					HZX_CHK_ALL,0,
					SEARCH_HZX);

	/* 反射ベクトル */
	pole_rot.vy = 0;
	pole_rot.vz = 0;
	rad = 0.0f;	/* Ｘ回転の合計を計算する */
	if( seg_num ){
		HZX_GetNearHazard( seg, atr ) ;
		HZX_GetReactVector( &fvtemp );

		ftemp = atan2f( fvtemp.vx, fvtemp.vz ) ;
		pole_rot.vy = 4095 & ( short )( ( ftemp * 2048.0f / PI ) + 0.5f );

		rad = PI * 0.5f;
		if( HZX_LevelHazardCheck( map_id, &work->pos, HZX_CHK_ALL, 0 ) & 1 ){
			HZX_GetLevelHeight( flr_height );
			ftemp = work->pos.vy - flr_height[0];
			if( ftemp > 0 ){
				rad += asinf( ftemp );
			}
		}
	}
	pole_rot.vx = 4095 & ( short )( ( rad * 1024.0f / PI ) + 0.5f );

//printf("a0::::%x\n",work->objs);
	if( work->objs != NULL ){
		DG_DequeueObjs(work->objs) ;
		DG_FreeObjs(work->objs) ;
		work->objs = NULL ;
	}

	if( work->target_flag != 0){
		work->target_flag  = 0;
		GM_ClearTargetDamage( &work->target ) ;
		GM_FreeTarget( &work->target ) ;
	}

	work->prim = OK_FreePrim2(work->prim) ;

	if( work->proc_id > 0 ){
		GCL_ARGS	args ;
		int			buf[ 4 ] ;

		args.argc = 4 ;
		args.argv = buf ;
		buf[ 0 ] = work->name ;
		buf[ 1 ] = ( int )work->pos.vx ;
		buf[ 2 ] = ( int )work->pos.vy ;
		buf[ 3 ] = ( int )work->pos.vz ;
		if ( !GM_IsGameOver() ){ // <------------ by koba4
			GCL_ExecProc( work->proc_id, &args );
		}
		work->proc_id = -1;
	}

//	NewBlast3( &work->pos, BOTH_SIDE, 1000, 2000, 64, 10, WP_C4Bomb, work->noise) ;
	NewBlast3( &work->pos, BOTH_SIDE, 10000, 10000, 0, 0, WP_StunGrenade, work->noise|BLAST_TYPE_NO_FIRE|BLAST_TYPE_ONETARG ) ;
	NewBlast3( &work->pos, BOTH_SIDE, 20000, 20000, 0, 0, WP_StunFar,     work->noise|BLAST_TYPE_NO_FIRE|BLAST_TYPE_ONETARG ) ;
	NewBombEffect( &work->pos, 2 );

}

static  void IRS_C4_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
	Work *work = (Work *)ptr ;  
	long64 flags;

	/* キャラがマップにないときは 何も処理をしない */
	if(!(work->map & GM_CurrentStageMap)) return ;

	flags = WP_BULLET | WP_BLAST;
	if( work->m9_active ) flags|= WP_M92;

	if( (def->damaged & TARGET_POWER) && (def->weapon_type & flags) ){
		printf("hit on trap \n") ;
		if( work->hit_flag==0 ) work->hit_flag = 1;
	}
}

/* 赤外線に接触したかのフラグをチェック */
static void Check_Explosion(Work *work)
{
	GV_MSG   *msg ;
	int	  mes_num ;
	int	  num ;

	mes_num = GV_ReceiveMessage( work->name, &msg ) ;

	msg += mes_num - 1 ;
	while( --mes_num >= 0 ){
		num = msg->message[0] ;
		switch( num ){
		   case TRAP_C4_NONE :
			   break ;
		   case TRAP_C4_EXPLOSE :
				if( work->hit_flag==0 ) work->hit_flag = 1;
//printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
//			   IRS_C4_Explosion(work,&work->target) ;
			   break;
		   case TRAP_C4_LAMP_OFF :
				work->lamp_off = 1;
			   break;
		   default:
			   break ;
		}
		msg -- ;
	}
}

static  void Act(Work *work)
{

	if( work->objs != NULL ){
		DG_GetLightMatrix((FVECTOR *)work->objs->world.m[3],work->light) ;
	}

	/* 判定 */
	Check_Explosion(work) ;
	if(work->count -- < 0){
		if(work->prim != NULL){
			DG_SwitchBuffPrim2(work->prim) ;
			if(work->prim->buffer_clock == 0){
				work->count = 0 ;
			}else{
				work->count = 2;
			}
		}
	}

	if( work->hit_flag==1 ){
		IRS_C4_Explosion(work,&work->target) ;
		work->hit_flag = 2;
		GV_DestroyActor(work) ;
	}

	if( work->lamp_off  ||  work->all_off ){
		work->prim = OK_FreePrim2( work->prim ) ;
	}


}

static  void Die(Work *work)
{
	if( work->objs != NULL ){
		DG_DequeueObjs(work->objs) ;
		DG_FreeObjs(work->objs) ;
		work->objs = NULL ;
	}
	if( work->target_flag != 0){
		work->target_flag  = 0;
		GM_ClearTargetDamage( &work->target ) ;
		GM_FreeTarget( &work->target ) ;
	}

	work->prim = OK_FreePrim2( work->prim ) ;
}

/*--------------------------------------------------------------*/
static  void InitTarget(Work *work,FMATRIX *world)
{
	TARGET   *t ;
	DG_MDL   *mdl ;
	FVECTOR  t_size ;
	FVECTOR  pos ;

	t = &work->target ;
	mdl = work->mdl ;

	t_size.vx = (mdl->ux - mdl->lx) * 0.5F ;
	t_size.vy = (mdl->uy - mdl->ly) * 0.5F ;
	t_size.vz = (mdl->uz - mdl->lz) * 0.5F ;


	pos.vx = 0.0F ;
	pos.vy = t_size.vy ;
	pos.vz = 0.0F ;
   pos.vw = 1.0F ; // BP_MATH - fix unitized value

	GM_SetTarget(t,TARGET_DEFENSE | TARGET_ROTATE, work->map, ENEMY_SIDE, &t_size, &pos) ;
	GM_SetPowerTarget(t,&work->power,POWER_ONCE,1,0,0,&DG_ZeroVector) ;
	GM_SetTargetCallBack(t,IRS_C4_TargetCallBack, work) ;
	GM_PutTarget(t) ;
	GM_CurrentMap = work->map;	//act control で やるものの代用
	GM_MoveTarget2(t,world) ;
//	NewTargetView(t, 0, 255, 0) ;

	work->target_flag = 1;

}

static  void GetOptionValue(Work *work)
{
	int  buf[3] ;

	work->all_off = 0;
	if ( GCL_GetOption( 'o' ) != NULL ){
		work->all_off = BLAST_TYPE_NO_NOISE;
	}

	work->noise = 0;
	if ( GCL_GetOption( 'n' ) != NULL ){
		work->noise = BLAST_TYPE_NO_NOISE;
	}

	work->m9_active = 0;
	if ( GCL_GetOption( 'm' ) != NULL ){
		work->m9_active = 1;
	}

	work->proc_id = -1;
	if ( GCL_GetOption( 'e' ) != NULL ){
		work->proc_id = GCL_GetNextInt();
	}

	/* ポジション決定 */
	if(GCL_GetOption('p') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		GV_IVtoFV(buf,(float *)&work->pos,3) ;
	}else{
		work->pos.vx = 0.0F ;
		work->pos.vy = 0.0F ;
		work->pos.vz = 0.0F ;
	}

	/* 回転を決定 */
	if( GCL_GetOption('r') != NULL){
		GCL_GetIV(GCL_NextStr(),buf) ;
		work->rot.vx = buf[0] ;
		work->rot.vy = buf[1] ;
		work->rot.vz = buf[2] ;
	}else{
		work->rot.vx = 0 ;
		work->rot.vy = 0 ;
		work->rot.vz = 0 ;
	}
}

static  void InitLampPrim2(Work *work,DG_PRIM2 *prim,DG_TEX *tex)
{
	FVECTOR			 *pos_a,*pos_b ;
	DG_PRIM2_UVRGBWH	*uvrgbwh_a, *uvrgbwh_b ;
	int				 i ;
	FVECTOR			 shift ;

	pos_a = prim->pos[0] ;
	pos_b = prim->pos[1] ;
	uvrgbwh_a = prim->uvrgb[0] ;
	uvrgbwh_b = prim->uvrgb[1] ;
	
	DG_SetPos2(&work->pos,&work->rot) ;
	shift.vx = 145.0F ;
	shift.vy = 295.0F ;
	shift.vz = -10.0F ;
	shift.vw = 0.0F ;
	for(i = 0; i < N_VERTS; i++){
		DG_PutVector(&shift,pos_a,1) ;
		_sceVu0CopyVectorXYZ(pos_b,pos_a) ;
		pos_a ++ ;
		pos_b ++ ;
	}
	
	/* 一方だけ初期化 */
	/* ランプのボンボリの大きさなどの設定 */
	uvrgbwh_a->u0 = FTOI12(tex->u_offset) ;
	uvrgbwh_a->v0 = FTOI12(tex->v_offset) ;
	uvrgbwh_a->u1 = FTOI12(1.0F * tex->u_scale + tex->u_offset) ;
	uvrgbwh_a->v1 = FTOI12(1.0F * tex->v_scale + tex->v_offset) ;
	uvrgbwh_a->q0 = 4096 ;
	uvrgbwh_a->q1 = 4096 ;
	uvrgbwh_a->f0 = 0x0fff ;
	uvrgbwh_a->f1 = 0x0fff ;
	/* WH値は整数 */
	uvrgbwh_a->w = 35 ;
	uvrgbwh_a->h = 35 ;
	uvrgbwh_a->r = 230 ;
	uvrgbwh_a->g = 115 ;
	uvrgbwh_a->b = 70  ;
	uvrgbwh_a->a = 55  ;

	/* もう一方は表示しない */
	uvrgbwh_b->u0 = uvrgbwh_a->u0 ;
	uvrgbwh_b->v0 = uvrgbwh_a->v0 ;
	uvrgbwh_b->u1 = uvrgbwh_a->u1 ;
	uvrgbwh_b->v1 = uvrgbwh_a->v1 ;
	uvrgbwh_b->q0 = uvrgbwh_a->q0 ;
	uvrgbwh_b->q1 = uvrgbwh_a->q1 ;
	uvrgbwh_b->f0 = 0x0fff ;
	uvrgbwh_b->f1 = 0x0fff ;
	uvrgbwh_b->w = 0 ;
	uvrgbwh_b->h = 0 ;
	uvrgbwh_b->r = 0 ;
	uvrgbwh_b->g = 0 ;
	uvrgbwh_b->b = 0  ;
	uvrgbwh_b->a = 0  ;
}


/* 初期化 */
static  int  GetResources(Work *work)
{
	DG_OBJS   *objs = NULL ;
	DG_DEF	*def ;
	FMATRIX   world ;
	DG_TEX	*tex ;
	DG_PRIM2  *prim ;

	work->lamp_off = 0;

	work->prim = NULL;

	work->hit_flag = 0;
	work->target_flag = 0;


	GetOptionValue(work) ;


	/* モデル情報を取得 */
	def = (DG_DEF *)GV_GetCache(GV_CacheID(5339203 /*"cfr_trp"*/,'k')) ;
	if(def == NULL){
		printf("there is not a cfr_trp.kms\n") ;
		return -1 ;
	}
	work->mdl = def->models ;

	DG_SetPos2(&work->pos,&work->rot) ;
	DG_GetPos(&world) ;

	work->objs = NULL;
	if( !work->all_off ){
		/* モデルを取得 */
		objs = work->objs = DG_MakeObjs(def,BODY_FLAG,0) ;
		if(objs == NULL) return -1 ;
		DG_QueueObjs((DG_OBJS*)work->objs) ;
		GM_GroupObjs( work->objs, work->map );
		DG_PutObjs(objs) ;
		DG_SetLightMatrix(work->objs,work->light) ;
	}

	InitTarget(work,&world) ;


	/* ボンボリを用意する */
	/* プリミティブを用意する */
	prim = work->prim = GM_MakePrim2(PRIM_TYPE,N_PRIMS,N_VERTS) ;
	if(prim == NULL){
		printf("trap_c4.c(lamp) : no prims\n") ;
		return -1 ;
	}
	/* テクスチャを用意する */
	tex = DG_GetTexture(4059425 /*"xlit01b_msk"*/) ;
	if( tex == NULL){
		printf("trap_c4.c(lamp) : no texture\n") ;
		return -1 ;
	}
	DG_ConfigPrim2Tex(prim,tex) ;
	DG_SetPrim2Alpha(prim, SCE_GS_SET_ALPHA(0,2,0,1,0)) ;
//	prim->prim_env.alpha.data = SCE_GS_SET_ALPHA(0,2,0,1,0) ;

	InitLampPrim2(work,prim,tex) ;

	work->count = 2 ;
	return 0 ;
}

void *NewTRAP_C4(int name, int map)
{
	Work  *work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if(work != NULL){
		GV_SetActor( &(work->actor), Act, Die ) ;
		GV_ActorEX( &work->actor );
		work->name = name ;
		work->map  = map ;
		if( GetResources( work ) < 0 ){
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
