//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bul_stg.c
	発射されたスティンガー

	2000/04/26 M.Sonoyama
	2000/07/05 S.Okajima
	$Id: bul_stg.c,v 1.1.1.3 2002/11/19 11:50:01 Yoshizawa1 Exp $
*/

#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
//BP_PS2 #include	"break.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <libutl.h>
#include	"../../okajima/etc/ok_util.h"

#include	"gameheader.h"
#include	"libmt.h"

#include "BP_Misc.h"

#define	FRONT_WING_INIT_ANGLE	(0.24f)
#define	FRONT_WING_ANGLE_STEP	(0.06f)
#define	TAIL_WING_INIT_ANGLE	(0.36f)
#define	TAIL_WING_ANGLE_STEP	(0.12f)

#define	INTERVAL_LIMIT	(60)

#define	BODY_NAME	(10575994)	/* stg_msl */
//#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_ONEPIECE)
#define	BODY_FLAG	(DG_FLAG_SHADE)


//#define	DECAY_RATIO	(0.95f)
#define	DECAY_RATIO	(0.996f)
// 追加コバ４ 
#define	INNER_LIMIT	(0.5f)

#define	MAX_RATIO	(0.25f)
#define	ADD_RATIO	(0.001f)
#define	MUL_RATIO	(40.0f)

#define	RAISE					(0)
#define	RAISE_SHIFT_WORLD		(20.0f)

#define	DUMMY_SIZE		(0)
#define	DUMMY_COLOR		(32)
#define	DUMMY_ALPHA		(0)

#define	N_PRIMS			(1)
#define	N_VERTS			(32)
#define	N_VP			(N_PRIMS * N_VERTS)

static FVECTOR s_pos[N_VP];
static FVECTOR s_pos_wide[N_VP];
static FVECTOR ShiftVec = {120.0f, 4000.0f, 500.0f};

/* 中心から円錐ライトの最遠中心まで */
#define	LIGHT_LEN		(800.0f)
#define	SHIFT_BACK		(-300.0f)


#define	COL_R				(8)
#define	COL_G				(24)
#define	COL_B				(32)

#define	LIT_R				(64)
#define	LIT_G				(196)
#define	LIT_B				(255)

#define	ALPHA_BASE			(96.0f)
#define	ALPHA_MIN			(16)
#define	EXTIN_ALPHA			(2.0f)

#define	SIZE_BASE		(300)
#define	SIZE_STEP		(16)

#define COUNT_LOW (30)
#define COUNT_HIGH (12)
#define SK_LIVE_LOW ( 180 / 2 )
#define SK_LIVE_HIGH ( 40 )

#define STR_LIGHT05_MSK (762348)

enum {
	PHASE_0=0,
	PHASE_1,
};

extern	TARGET	*PL_LockonTarget ;

FVECTOR	StingerHomingPosition ;

static	int	Interval_switch = 0;
static	int	Total_num = 0;
static  int SK_TurnSpeed;
static  float SK_MaxSpeed;
static  float SK_AddSpeed;
static  float SK_Draw;
static  int SK_Life;
static  FVECTOR SK_DefaultSpeed;

enum { 
	SK_HOMING_NORMAL = 0,
	SK_HOMING_LOW_SUPORT,
	SK_HOMING_OVER,
};

typedef struct SStingerBulletWorkTag
{
	GV_ACT_EX		actor ;
	CONTROL_NOEVT	control ;
	OBJECT			body ;
	FVECTOR			rots[9];
	FMATRIX			lights[ 2 ] ;
	TARGET			target ;
	GM_BOMB			list ;

	int				count ;
	int				flag ;

	FVECTOR			*homing_pos;
	int				phase ;
	int				total_count ;
	int				effect_count ;
	int				prim_count;
	float			vec;
	float			ratio;			/* 追尾率 */
	int             turn_speed;   // 旋回性能            
	SVECTOR			svtemp;
	SVECTOR			svtemp_add;

	DG_PRIM2	*prim;
	float		alpha;
	int         time;
	char        togle;  // homing limit
	FMATRIX		memorize_mat ;
	char        effect_flag;
}
SStingerBulletWork;

extern void *NewSubMazzleFlushMngNormal( int , FMATRIX * , FVECTOR * , FVECTOR * , int , float , float , int );
extern void *NewSKLineBox( CONTROL * , float );
extern void	DG_TransPersOne( FVECTOR * , FVECTOR * );
extern void *NewSTG_SmokeBlurEffect( FMATRIX * , int , int , int , int , int , int , int , int , int , int , int );
extern void MissileNum( FVECTOR * );
extern void *NewSK_Smoke2rgbaNormal( int num , FMATRIX *world, FVECTOR *shift, FVECTOR *pPower , FVECTOR *pColor , float size_rnd , 
								  FVECTOR *range , float alpha , float alpha_div , char *status );

/*--------------------------------------------------------------------*/

static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	SStingerBulletWork		*work ;

	{
		FVECTOR		dc ;

		GM_TargetGetCenter( &dc, def ) ;
		if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &off->hit, &dc, 
								    HZX_CHK_ALL,
								    HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
+
								    HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) ) {
			GM_TargetHitCancel( off, def ) ;
			return ;
		}
	}

	work = ( SStingerBulletWork * )ptr ;
	work->flag |= 1 ;
	printf( "stinger missile hit!\n" ) ;
}

static void HomingControl( CONTROL *target , SVECTOR *pos , SStingerBulletWork *pWork , int *homing_speed , FVECTOR *diff )
{
	FVECTOR fvtmp;
	SVECTOR tmp;
	float power;
	float sum;
	int stmp;

	// すいこみ 
	if ( pWork->time < 450 ){
		power = SK_Draw * pWork->time / 600.0f;
	} else {
		power = SK_Draw;
	}
	_sceVu0Normalize( &fvtmp , diff ); 
	_sceVu0ScaleVector( &fvtmp , &fvtmp , power ); 
	_sceVu0AddVector( &target->step, &target->step, &fvtmp );

   if ( BP_IsPAL()!=TRUE )
	   pWork->time+= 5;
   else
	   pWork->time+= 6;

   if( pWork->vec > SK_MaxSpeed ){
		pWork->vec = SK_MaxSpeed;
	}
	tmp.vx = pos->vx - target->turn.vx; 
	tmp.vy = pos->vy - target->turn.vy;
	tmp.vz = 0;

	if ( tmp.vx > 2048 ){// 敷居をはさむ場合  
		tmp.vx -= 4096;
	} else if ( tmp.vx < -2048 ){
		tmp.vx += 4096;
	}
	if ( tmp.vy > 2048 ){
		tmp.vy -= 4096;
	} else if ( tmp.vy < -2048 ){
		tmp.vy += 4096;
	}
	// homing の 範囲に入っているかどうか 
	switch ( pWork->togle ){
	case SK_HOMING_NORMAL :
		stmp = ( tmp.vx * tmp.vx ) + ( tmp.vy * tmp.vy );
		stmp = ( int )fpu_Sqrt( stmp );
		// 改良 2001/05/15 
		if ( stmp > SK_TurnSpeed + ( pWork->vec / 10 ) ){
			tmp.vx = tmp.vx * SK_TurnSpeed / stmp;
			tmp.vy = tmp.vy * SK_TurnSpeed / stmp;
		}
		// check
		sum = DG_FABS( diff->vx );
		sum += DG_FABS( diff->vy );
		sum += DG_FABS( diff->vz );
		if ( sum < 6000.0f ){
			pWork->togle = SK_HOMING_LOW_SUPORT;
		}
		if ( _sceVu0InnerProduct( diff , &pWork->control.step ) < 0 ){
			pWork->togle = SK_HOMING_LOW_SUPORT;
		} else {
			target->turn.vx += tmp.vx;
			target->turn.vy += tmp.vy;
			target->rot = target->turn;
			target->interp = 1;
		}
		break;
	case SK_HOMING_LOW_SUPORT :
		if ( _sceVu0InnerProduct( diff , &pWork->control.step ) > 0 ){
			stmp = ( tmp.vx * tmp.vx ) + ( tmp.vy * tmp.vy );
			stmp = ( int )fpu_Sqrt( stmp );
			// 改良 2001/05/15 
			if ( stmp > SK_TurnSpeed + ( pWork->vec / 10 ) ){
				tmp.vx = tmp.vx * SK_TurnSpeed / stmp;
				tmp.vy = tmp.vy * SK_TurnSpeed / stmp;
			}
			target->turn.vx += tmp.vx;
			target->turn.vy += tmp.vy;
			target->rot = target->turn;
			target->interp = 1;
		} else {
			pWork->togle = SK_HOMING_OVER;
			if ( pWork->count > DIRECT_TICK( SK_LIVE_LOW ) ) {
				pWork->count = DIRECT_TICK( COUNT_LOW );
			} else if ( pWork->count > DIRECT_TICK( SK_LIVE_HIGH ) ) {
				pWork->count = DIRECT_TICK( COUNT_HIGH );
			} else {
				pWork->count = 0;
			}
		}
		break;
	}
	pWork->vec += SK_AddSpeed;
#if 0
	if ( ( tmp.vx == 0 ) && ( tmp.vy == 0 ) ){
	    pWork->vec += ACCEL*3;
	}
#endif
}

static int _stinger_vec_valid(FVECTOR *v)
{
   static int const skIEEENaNMask = 0x7f800000;
   int i;
   i = *(int *)(&v->vx);
   if ((i & skIEEENaNMask) == skIEEENaNMask)
   {
      return FALSE;
   }
   i = *(int *)(&v->vy);
   if ((i & skIEEENaNMask) == skIEEENaNMask)
   {
      return FALSE;
   }
   i = *(int *)(&v->vz);
   if ((i & skIEEENaNMask) == skIEEENaNMask)
   {
      return FALSE;
   }
   return TRUE;
}

/*--------------------------------------------------------------------*/
//static FVECTOR ShiftCenter={ 0.0f, 550.0f, 0.0f, 0.0f };
static FVECTOR ShiftCenter={ 0.0f, 900.0f, 0.0f, 0.0f };

static void Act( SStingerBulletWork *work )
{
	int	i,j;
	int	alpha;
	int	clock;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR		from ;
	FVECTOR		to ;
	FVECTOR		diff ;
	FVECTOR		fvtemp ;
	FVECTOR		*rots ;
	FMATRIX		mat ;
	SVECTOR		svtemp ;
	CONTROL		*ctrl ;
	// こば４追加 
	int   homing_speed;

	/* ポリデモ中はＯＫ */
	if ( GM_CheckGameStatus( STATE_SCN_DEMO | STATE_PRG_DEMO ) ||
		( work->list.flag & GM_BMB_FLAG_DESTROY ) ) {
		GV_DestroyActor( work ) ;
		return ;
	}

	GM_SetCurrentMap( GM_StageMapAll ) ;

	if ( work->list.flag & GM_BMB_FLAG_NOBLAST ) {
		if ( PL_FallBulletFunc != NULL ) {
			PL_FallBulletFunc( &work->body.objs->world, NULL, &work->control.step, 
							   GV_StrCode( "stg_msl" ), 750.0F, 500.0F, FALLBUL_TYPE_YDOWN ) ;
			GV_DestroyActor( work ) ;
			return ;
		}
	}

	if ( PL_CheckBulletSplash( &work->control.mov, &work->control.step, 150.0F, 200.0F ) ) {
		work->flag |= 1 ;
	}

	ctrl = ( CONTROL * )&( work->control ) ;

   // Arm fix:
   // fix for crash bug when firing stinger missiles a lot
   // sometimes from can be invalid (not sure why!)
   // fixes MGSTWO-3267
   if (!_stinger_vec_valid(&ctrl->mov)
      || !_stinger_vec_valid((FVECTOR *)work->body.objs->world.m[3]))
   {
      GV_DestroyActor( work ) ;
      return ;
   }

   DG_COPY_VEC( &from, &ctrl->mov ) ;

	if ( PL_LockonTarget != NULL ) {
		DG_COPY_VEC( &to, &PL_LockonTarget->center ) ;
	} else {
		DG_COPY_VEC( &to, &from ) ;
	}

	_sceVu0SubVector( &diff, &to, &from ) ;
	GM_ActControl( ctrl ) ;
	GM_ActObject( &work->body ) ;
	DG_GetLightMatrix( &from, work->lights ) ;
	if ( work->control.n_touches || -- work->count <= 0 ) {
#if 1
		if ( work->count <= 0 ) printf( "stinger missile time over!\n" ) ;
		else {
			printf( "stinger missile touch hazard!\n" ) ;
#ifdef DEBUG_MODE
			if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW ) {
				HZX_ViewHazard( ctrl->level[ 0 ] ) ;
			}
#endif
		}
#endif
		work->flag |= 1 ;
	}
	GM_MoveOnlineTargetMap( &work->target, &from, &ctrl->mov, GM_StageMapAll ) ;
	GM_PutTarget( &work->target ) ;
	if ( work->flag & 1 )
   {
      // Arm fix: MGSTWO-3267
      if (_stinger_vec_valid(&from))
      {
	      extern	void	*NewBlast( FVECTOR *, int, int, int, int, int, int ) ;
	      NewBlast( &from, BOTH_SIDE, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Stinger ) ;
      }
		GV_DestroyActor( work ) ;
		return ;
	}
	DG_SetPos( &work->body.objs->world );
	DG_MovePos( &ShiftCenter );
	DG_GetPos( &work->memorize_mat );

	switch( work->phase ){
	  case PHASE_0:
		DG_InvisiblePrim2( work->prim ) ;
		work->prim_count = -1;
		ctrl->step.vy += P_GRAVITY*0.2f;
		_sceVu0ScaleVector( &ctrl->step, &ctrl->step, DECAY_RATIO );
		if( work->effect_count > DIRECT_TICK( 15 ) ){
			GM_SeSetMode( SD_W_MISILE03, &ctrl->mov, GM_SEMODE_BOMB ) ; 
			work->effect_count = 0;
			work->phase = PHASE_1;
			work->effect_flag = 0x0; // 軌跡の描画  
#if 1
			if( GM_BlurDisableFlag == 0 ){
				extern void *NewBlurProgTimer(
                        float start,	//最初の強さ（この強さで突然始まる 0~128） 
                        int   time0,	//最初から途中までの変化時間(フレーム) 
                        float path,		//途中の強さ(0~128) 
                        int   time1,	//途中から最後までの変化時間(フレーム) 
                        float end		//最後の強さ（この強さで突然終了する 0~128） 
                      );
#ifndef KP_XBOX /* ゲーム性が崩れるまで画面が引きつるため、ＸＢＯＸ版ではカット */
				GV_SetActorChild( work, 
					NewBlurProgTimer(
                        96.0f,	//最初の強さ（この強さで突然始まる 0~128） 
                        20,		//最初から途中までの変化時間(フレーム) 
                        92.0f,	//途中の強さ(0~128) 
                        30,	//途中から最後までの変化時間(フレーム) 
                        0.0f	//最後の強さ（この強さで突然終了する 0~128） 
                      )
				);
#endif
			}
#endif
#if 1
			if( GM_BlurDisableFlag == 0 ){
#ifndef KP_XBOX /* ゲーム性が崩れるまで画面が引きつるため、ＸＢＯＸ版ではカット */
				extern void *NewSmokeBlurEffect( FMATRIX *world, int start_speed, int end_speed, int start_size, int end_size, int spot_size, int spot_angle, int n_prims, int interval, int color, int flag );
				GV_SetActorChild( work, 
					NewSTG_SmokeBlurEffect(
						&work->memorize_mat,
						100,	//ブラーパーティクル初期スピード 
						50,	//ブラーパーティクル最終スピード 
						200,	//ブラーパーティクル初期サイズ 
						400,	//ブラーパーティクル最終サイズ 
						500,	//ブラーパーティクル出現位置半径 
						256,	//最大放射角度（ＰＳ角度0~4095） 
						16,		//最大パーティクル数 
						1,		//噴射間隔（フレーム単位） 
						0x00818181,
						0x0000,
						120
					)
				 );
#endif
			}
#endif

#if 1
			{
				                          // 即時コピー型 
				extern void *NewSTG_Spark( FMATRIX *hand, float vec_len );
				NewSTG_Spark( &work->memorize_mat, 50.0f );
				NewSTG_Spark( &work->memorize_mat, 150.0f );
			}
#endif
		}
		break;
	  case PHASE_1:
		if( &to==NULL ){
			GV_DestroyActor( work ) ;
			return;
		}
		work->prim_count++;

		if( work->effect_count == DIRECT_TICK( 15 ) ){
			extern void *NewConnectSmoke( FMATRIX *world, FVECTOR *shift, int life, float size_rnd );
			GV_SetActorChild( work, NewConnectSmoke( &work->memorize_mat, &DG_ZeroVector, 60, 128.0f ) );
		}

		if( work->effect_count == DIRECT_TICK( 20 ) ){
			extern void *NewSTG_SmokeBack( FVECTOR *pos, FVECTOR *vector );
			fvtemp.vx = 0.0f;
			fvtemp.vy = 1.0f;
			fvtemp.vz = 0.0f;
			DG_SetPos( &work->memorize_mat );
			DG_RotVector( &fvtemp, &fvtemp, 1 );
			NewSTG_SmokeBack( (FVECTOR *)work->memorize_mat.m[3], &fvtemp );
		}
#if 0
		{

			NewSubMazzleFlushMngNormal( 3 , &work->memorize_mat , &DG_ZeroVector ,&power , 10 , 700.0f, 3.0f , 18 );
		}
#endif
		DG_COPY_VEC( &ctrl->step , &DG_ZeroVector );

		if ( PL_LockonTarget != NULL ) {
			GV_VecToRot( &diff, &svtemp ) ;
			svtemp.vx -= 1024 ;

			//コントロールのstepに足しこむ
			HomingControl( ctrl , &svtemp , work , &homing_speed , &diff );

         // Arm fix:
         // fix for crash bug when firing stinger missiles a lot
         // sometimes from can be invalid (not sure why!)
         // fixes MGSTWO-3267
         if (!_stinger_vec_valid(&ctrl->mov)
            || !_stinger_vec_valid((FVECTOR *)work->body.objs->world.m[3]))
         {
            GV_DestroyActor( work ) ;
            return ;
         }
      } else {
			work->vec += SK_AddSpeed; 
			if( work->vec > SK_MaxSpeed ){
				work->vec = SK_MaxSpeed;
			}
		}
		// by koba4 2001/06/17
		fvtemp.vx = SK_DefaultSpeed.vx;
		fvtemp.vy = SK_DefaultSpeed.vy - work->vec;
		fvtemp.vz = SK_DefaultSpeed.vz;
		DG_SetPos2( &DG_ZeroVector, &ctrl->rot ) ;
		DG_RotVector( &fvtemp, &fvtemp, 1 ) ;
		_sceVu0AddVector( &ctrl->step, &ctrl->step, &fvtemp ) ;

		break;
	  default:
		break;
	}

	DG_COPY_VEC( &GM_StingerPosition, &work->control.mov ) ;

//-------------------------------
	DG_VisiblePrim2( work->prim ) ;
	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;
	DG_SetPos( &work->memorize_mat );
	uvrgbwh = work->prim->uvrgb[clock];
	if( work->prim_count != -1 ){
		work->alpha -= EXTIN_ALPHA;
		work->alpha  = (work->alpha > 0.0f)?work->alpha: 0.0f;
		alpha = (int)( (work->alpha * rnd() + work->alpha) * 0.5f );
		if( work->prim_count < 32 ){
			DG_PutVector( s_pos_wide, work->prim->pos[clock], N_VP );
			for( j=0; j<N_VP; j++ ){
				uvrgbwh->w = uvrgbwh->h = (N_VP-j)*SIZE_STEP*(32-work->prim_count)/4 + SIZE_BASE;
				uvrgbwh->a = alpha * (N_VP-j) / N_VP + ALPHA_MIN;
				uvrgbwh++;
			}
			DG_SetPos( &work->body.objs->world );
			DG_MovePos( &ShiftVec );
			DG_GetPos( &mat );
			DG_SetTmpLight2( (FVECTOR *)mat.m[3], 800.0f, 1600.0f, LIT_R | LIT_G<<8 | LIT_B <<16, LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
//			AN_Test_Eye2( (FVECTOR *)mat.m[3], 2 );
		}else{
			DG_PutVector( s_pos, work->prim->pos[clock], N_VP );
			for( j=0; j<N_VP; j++ ){
				uvrgbwh->w = uvrgbwh->h = (N_VP-j)*SIZE_STEP + SIZE_BASE;
				uvrgbwh->a = alpha * (N_VP-j) / N_VP + ALPHA_MIN;
				uvrgbwh++;
			}
		}
	}else{
//		alpha = (int)( (work->alpha * rnd() + work->alpha) * 0.5f * 0.25f);
		DG_PutVector( s_pos, work->prim->pos[clock], N_VP );
		for( j=0; j<N_VP; j++ ){
//			uvrgbwh->w = uvrgbwh->h = ((N_VP-j)*SIZE_STEP + SIZE_BASE)/8;
//			uvrgbwh->a = alpha * (N_VP-j) / N_VP + ALPHA_MIN;
			uvrgbwh->w = uvrgbwh->h = 0;
			uvrgbwh->a = 0;
			uvrgbwh++;
		}
	}


	if( work->total_count == 0 ){
		extern void *NewSTG_SmokeBack( FVECTOR *pos, FVECTOR *vector );
		fvtemp.vx = 0.0f;
		fvtemp.vy = 1.0f;
		fvtemp.vz = 0.0f;
		DG_SetPos( &work->memorize_mat );
		DG_RotVector( &fvtemp, &fvtemp, 1 );
		NewSTG_SmokeBack( (FVECTOR *)work->memorize_mat.m[3], &fvtemp );
	}
	if( work->total_count == DIRECT_TICK( 4 ) ){
		extern void *NewSTG_SmokeFront( FVECTOR *pos, FVECTOR *vector );
		fvtemp.vx = 0.0f;
		fvtemp.vy = -1.0f;
		fvtemp.vz = 0.0f;
		DG_SetPos( &work->memorize_mat );
		DG_RotVector( &fvtemp, &fvtemp, 1 );
		NewSTG_SmokeFront( (FVECTOR *)work->memorize_mat.m[3], &fvtemp );
	}
	if( work->total_count == DIRECT_TICK( 4 ) ){
		extern void *NewStingerObjectFall( FVECTOR *pos, SVECTOR *rot, FVECTOR *vec );
		NewStingerObjectFall( &ctrl->mov, &ctrl->rot, &ctrl->step );
	}


	if( work->total_count == DIRECT_TICK( INTERVAL_LIMIT ) ){
		Interval_switch = 0;
	}



	if( work->total_count > DIRECT_TICK( 35 ) ){
		rots = work->rots;
		rots++;
		for( i=0; i<4; i++ ){
			if( rots->vx > 0.0f ){
				rots->vx -= FRONT_WING_ANGLE_STEP;
			}else if( rots->vx < 0.0f ){
				rots->vx += FRONT_WING_ANGLE_STEP;
			}
			if( rots->vy > 0.0f ){
				rots->vy -= FRONT_WING_ANGLE_STEP;
			}else if( rots->vy < 0.0f ){
				rots->vy += FRONT_WING_ANGLE_STEP;
			}
			if( rots->vz > 0.0f ){
				rots->vz -= FRONT_WING_ANGLE_STEP;
			}else if( rots->vz < 0.0f ){
				rots->vz += FRONT_WING_ANGLE_STEP;
			}
			rots++;
		}
		for( i=0; i<4; i++ ){
			if( rots->vx > 0.0f ){
				rots->vx -= TAIL_WING_ANGLE_STEP;
			}else if( rots->vx < 0.0f ){
				rots->vx += TAIL_WING_ANGLE_STEP;
			}
			if( rots->vy > 0.0f ){
				rots->vy -= TAIL_WING_ANGLE_STEP;
			}else if( rots->vy < 0.0f ){
				rots->vy += TAIL_WING_ANGLE_STEP;
			}
			if( rots->vz > 0.0f ){
				rots->vz -= TAIL_WING_ANGLE_STEP;
			}else if( rots->vz < 0.0f ){
				rots->vz += TAIL_WING_ANGLE_STEP;
			}
			rots++;
		}
	}

	work->total_count++;
	work->effect_count++;
}

static	void	Die( SStingerBulletWork			*work )
{
	GM_FreeControl( ( CONTROL * )&work->control ) ;
	GM_FreeObject( &work->body ) ;
	GM_RemoveBombList( &work->list ) ;

	if( work->prim != NULL )  GM_FreePrim2( work->prim );

	Total_num--;

	if( Total_num <= 0  &&  Interval_switch != 0 ) Interval_switch = 0;

	GM_WeaponAlive &= ~WP_ALIVE_STINGER ;
	work->effect_flag = 1;
}

/* ---------------------------------------------------------------- */
static void InitPacket( SStingerBulletWork *work, DG_PRIM2 *prim, DG_TEX *tex, int n_prims, int n_verts )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i,j,k ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim->raise=RAISE;

	//-------------------------------
	for ( i=0; i<2; i++ ){
		uvrgbwh = prim->uvrgb[ i ] ;
		for ( j=0; j<n_prims; j++ ){
			for ( k=0; k<n_verts; k++ ){
				uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
				uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
				uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
				uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
				uvrgbwh->q0 = 4096 ;
				uvrgbwh->q1 = 4096 ;
				uvrgbwh->f0 = 0x0fff ;
				uvrgbwh->f1 = 0x0fff ;
				uvrgbwh->h = uvrgbwh->w = DUMMY_SIZE ;
				uvrgbwh->r = COL_R;
				uvrgbwh->g = COL_G;
				uvrgbwh->b = COL_B;
				uvrgbwh->a = DUMMY_ALPHA ;
				uvrgbwh ++ ;
			}
		}
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

}

/*--------------------------------------------------------------------*/
static	int		InitControl( SStingerBulletWork *work, FMATRIX *world, CONTROL *chara )
{
	CONTROL		*ctrl ;
	FVECTOR		mov, step ;
	SVECTOR		rot ;

	ctrl = ( CONTROL * )&( work->control ) ;
	if ( GM_InitControl( ctrl, WP_Stinger, GM_CurrentMap ) < 0 ) return -1 ;
	DG_SetPos( world ) ;
	DG_RotVector( &SK_DefaultSpeed, &step, 1 ) ;
	GV_VecToRot( &step, &rot ) ;
	GV_MatToVec( world, &mov ) ;
	rot.vx -= 1024 ;
	GM_ConfigControlPosition( ctrl, &mov, &rot ) ;
	DG_COPY_VEC( &ctrl->step, &step ) ;
	ctrl->map = GM_StageMapAll ;
	ctrl->hzx_id = HZX_AllMapID ;
	//GM_ConfigControlMapCheck( ctrl ) ;
	GM_ConfigControlHazard( ctrl, 200, 500, 500 ) ;
	ctrl->seg_flag = HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE ;
	ctrl->flr_flag = HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE | HZX_FLOOR_IK ;
	ctrl->skip_flag |= CTRL_SKIP_NEAR_CHECK | CTRL_SKIP_FLR_CHECK ;	
	return 0 ;
}

static	int		InitObject( SStingerBulletWork *work, FMATRIX *world )
{
	FVECTOR		*rots;
	OBJECT		*body ;
	FVECTOR 	initial_rotation;

	body = &work->body ;
	GM_InitObject( body, BODY_NAME, BODY_FLAG ) ;
	if ( body->objs == NULL ) return -1 ;
	GM_ConfigObjectJoint( body, work->rots );
	GM_ConfigObjectLight( body, work->lights ) ;
	DG_COPY_MAT( &body->objs->world, world ) ;

	/* 本体 */
	rots = work->rots;
	DG_COPY_VEC( rots, &DG_ZeroVector );


	/* 前部 */
	initial_rotation.vx = -TPI * FRONT_WING_INIT_ANGLE;
	initial_rotation.vy = 0.0f;
	initial_rotation.vz = 0.0f;
	rots++;
	DG_COPY_VEC( rots, &initial_rotation );

	initial_rotation.vx = 0.0f;
	initial_rotation.vy = 0.0f;
	initial_rotation.vz =  TPI * FRONT_WING_INIT_ANGLE;
	rots++;
	DG_COPY_VEC( rots, &initial_rotation );

	initial_rotation.vx =  TPI * FRONT_WING_INIT_ANGLE;
	initial_rotation.vy = 0.0f;
	initial_rotation.vz = 0.0f;
	rots++;
	DG_COPY_VEC( rots, &initial_rotation );

	initial_rotation.vx = 0.0f;
	initial_rotation.vy = 0.0f;
	initial_rotation.vz = -TPI * FRONT_WING_INIT_ANGLE;
	rots++;
	DG_COPY_VEC( rots, &initial_rotation );


	/* 後部 */
	initial_rotation.vx = 0.0f;
	initial_rotation.vy = TPI * TAIL_WING_INIT_ANGLE;
	initial_rotation.vz = 0.0f;
	rots++;
	DG_COPY_VEC( rots, &initial_rotation );

	rots++;
	DG_COPY_VEC( rots, &initial_rotation );

	rots++;
	DG_COPY_VEC( rots, &initial_rotation );

	rots++;
	DG_COPY_VEC( rots, &initial_rotation );


/*
printf("body->objs->n_models:::::::::::::::%d\n",body->objs->n_models);
printf("body->objs->def:::::::::::::::%d:%d:%d\n",body->objs->def->tx,body->objs->def->ty,body->objs->def->tz);
for(i=0; i<body->objs->n_models; i++){
	printf("%d::::%f:%f:%f\n",i,body->objs->objs[i].model->tx,body->objs->objs[i].model->ty,body->objs->objs[i].model->tz);
}
for(i=0; i<body->objs->n_models; i++){
	printf("%d::::%f:%f:%f\n",i,body->objs->def->models[i].tx,body->objs->def->models[i].ty,body->objs->def->models[i].tz);
}
*/


	return 0 ;
}

static	void	SetTarget( SStingerBulletWork	*work )
{
	TARGET		*t ;

	t = &work->target ;
	GM_SetTarget( t, TARGET_OFFENSE | TARGET_ONLINE | TARGET_ONLINE_MIN | 
				  TARGET_POWER | TARGET_THROUGH, GM_CurrentStageMap, 
				  ENEMY_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_WEAPONCORE ) ;
	GM_SetTargetName( t, WP_Stinger ) ;
	GM_SetTargetCallBack( t, Hit, work ) ;
}

static int GetResources( SStingerBulletWork *work, FMATRIX *world, CONTROL *ctrl, int side, int chanl )
{
	int			i;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		from, to, hit ;

	GV_MatToVec( world, &to ) ;
	if ( ctrl != NULL ){
		DG_COPY_VEC( &from, &ctrl->mov ) ; from.vy = to.vy ;
		if ( HZX_OnlineHazardCheck( ctrl->hzx_id, &from, &to,
								   HZX_CHK_ALL, HZX_SEG_NO_MISSILE | HZX_SEG_RECOIL_TYPE,
								   HZX_FLOOR_NO_MISSILE | HZX_FLOOR_RECOIL_TYPE ) > 0 ) {
			extern	void	*NewBlast( FVECTOR *, int, int, int, int, int, int ) ;
			float			len ;

			HZX_GetOnlineVector( &hit ) ;
			len = GV_VecLen3F( &hit ) ;
			if ( len > 5.0F ) {
				GV_LenVec3F( &hit, &hit, 0.0F, len - 5.0F ) ;
			}
			_sceVu0AddVector( &hit, &from, &hit ) ;
			NewBlast( &hit, BOTH_SIDE, 1000, 2000, DMG_BLAST, FNT_BLAST, WP_Stinger ) ;

			/* 発砲数 */
			if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;
			GM_DecrementWeapon( WP_Stinger, 1 ) ;
			NewPadVibration2( GV_StrCode( "rai_stg_fire" ), 0 ) ;
			GM_SetWeaponFire( WP_Stinger ) ;

			return -1 ;
		}
	}
	if ( InitControl( work, world, ctrl ) < 0 ) return -1 ;
	if ( InitObject( work, world ) < 0 ) return -1 ;
	SetTarget( work ) ;

	ASSERT( SK_Life != 0 )

	if ( PL_LockonTarget != NULL ) {
		work->count = DIRECT_TICK( SK_Life ); // Normal LockOn
	} else {
		work->count = DIRECT_TICK( ( SK_Life ) ) / 2; // 非ロックオン時は生存時間が少ない 
	}
	work->total_count = 0;
	work->effect_count = 0;
	work->phase = PHASE_0;
	work->vec   = 0.0f;
	work->ratio = 0.0f;
	work->turn_speed = SK_TurnSpeed;
	work->svtemp.vx = irnd()&4096;
	work->svtemp.vy = irnd()&4096;
	work->svtemp.vz = 0;
	work->svtemp_add.vx = irnd()%16-8;
	work->svtemp_add.vy = irnd()%16-8;;
	work->svtemp_add.vz = 0;
	work->togle = SK_HOMING_NORMAL;
	//-----------------------------
//	tex = DG_GetTexture( GV_StrCode( "chi01_msk" ) );
//	tex = DG_GetTexture( GV_StrCode( "rcm_l_msk" ) );
	tex = DG_GetTexture( STR_LIGHT05_MSK );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, N_PRIMS, N_VERTS );

	for ( i = 0 ; i < N_VP ; i++ ){
		s_pos[i].vx=0.0f;
		s_pos[i].vy=(float)(N_VP-i) * LIGHT_LEN / (float)(N_VP-1) + SHIFT_BACK;
		s_pos[i].vz=0.0f;

		s_pos_wide[i].vx=0.0f;
		s_pos_wide[i].vy=(float)(N_VP-i) * LIGHT_LEN * 2.0f / (float)(N_VP-1) + SHIFT_BACK;
		s_pos_wide[i].vz=0.0f;
	}

	work->alpha = ALPHA_BASE;
		work->prim_count = -1;

	GM_SeSetMode( SD_W_MISSIL01, &work->control.mov, GM_SEMODE_BOMB ) ; 
	GM_SetNoise( NOISE_M, &work->control.mov, work->control.map ) ;

	GM_InitBombList( &work->list, WP_Stinger, &work->control.mov, &work->control.step ) ;
	GM_AddBombList( &work->list ) ;

	GM_WeaponAlive |= WP_ALIVE_STINGER ;
	DG_COPY_VEC( &GM_StingerPosition, &work->control.mov ) ;
	work->effect_flag = 0x2;
	// 付随煙 by koba4 
	{
		FVECTOR power;
		FVECTOR color;
		FVECTOR  ftmp = { 75.f , 100.f , 75.f , 0.f };

		power.vx = 0;
		power.vy = 100.0f;
		power.vz = 460.0f;
		power.vw = 1.0f;
		color.vx = 16.0f;
		color.vy = 16.0f;
		color.vz = 32.0f;
		color.vw = 0.0f;
		GV_SetActorChild( work , NewSK_Smoke2rgbaNormal( 60 , &work->memorize_mat , &DG_ZeroVector, &power ,
														 &color , 256.0f , &ftmp  , 64.0f , 2.5f , &work->effect_flag ) );
	}
	return 0 ;
}

/*--------------------------------------------------------------------*/
/* スティンガー */
void *NewBulletStinger( FMATRIX *world, CONTROL *ctrl, int side, int chanl )
{
	SStingerBulletWork		*work ;

	if( Interval_switch != 0 ) return NULL ;

	work = ( SStingerBulletWork * )GV_NewActor( GV_ACTOR_AFTER, sizeof( SStingerBulletWork ) ) ;
	if ( work != NULL ) {
		Interval_switch = 1;
		Total_num++;

#if 1
		/* 暫定ホーミング位置初期化 */
		{
			FVECTOR	fvtemp;
			fvtemp.vx = 0.0f;
			fvtemp.vy = -100000.0f;
			fvtemp.vz = 0.0f;
			DG_SetPos( world );
			DG_PutVector( &fvtemp, &StingerHomingPosition, 1 );
		}
#endif
		/* 暫定ホーミング位置 */
		work->homing_pos = &StingerHomingPosition;

		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world, ctrl, side, chanl ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		/* 発砲数 */
		if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;
		GM_DecrementWeapon( WP_Stinger, 1 ) ;
		NewPadVibration2( GV_StrCode( "rai_stg_fire" ), 0 ) ;
		GM_SetWeaponFire( WP_Stinger ) ;
	}
	return work ;
}

#define TURN_SPEED  (17) // 32
#define	MAX_SPEED	(1000.0f)
#define	ACCEL		(1.4f)
#define DRAW        (70.0f) // バキューム 
#define	LIFE		(180)

//static	FVECTOR	Speed = { 0.0F, -250.0F, 0.0F } ;

void NewStingerFlag( void ) 
{
	SK_TurnSpeed    = GCL_GetOptionValue( 't' , 32 ); // speed
	SK_MaxSpeed     = 1.0f * GCL_GetOptionValue( 'm' , 400 ); // speed
	SK_AddSpeed     = GCL_GetOptionValue( 'p' , 14 ) / 10.0f; // speed
	SK_Draw         = 1.0f * GCL_GetOptionValue( 'd' , 70 ); // speed
	SK_Life         = GCL_GetOptionValue( 'l' , 180 ); // speed
	GCL_GetOption( 's' );
	// 位置情報の引きだし（格納）
	SK_DefaultSpeed.vx = ( float )GCL_GetNextInt();	
	SK_DefaultSpeed.vy = ( float )GCL_GetNextInt();	
	SK_DefaultSpeed.vz = ( float )GCL_GetNextInt();	

   if ( BP_IsPAL()==TRUE )
   {
	   SK_TurnSpeed *= 1.2f;
	   SK_MaxSpeed *= 1.2f;
	   SK_Draw     *= 1.44f;
	   SK_AddSpeed *= 1.44f;
	   SK_DefaultSpeed.vx *= 1.44f;
	   SK_DefaultSpeed.vy *= 1.44f;
	   SK_DefaultSpeed.vz *= 1.44f;
   }
}
