//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    harrier_claster_exp.c
    クラスター爆発管理エフェクト
    2001/07/07 Yuuta Kunibe	
    $Id: harrier_claster_exp.c,v 1.1.1.3 2002/11/19 11:44:40 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"



#define	UPPER_FLOOR_HEIGHT	(-1000.0f)
#define	LOWER_FLOOR_HEIGHT	(-4000.0f)

#define	STAGE_CENTER_X		(0.0f)	
#define	STAGE_WIDTH_X		(3000.0f)

#define	STAGE_MAX_Z		(-137000.0f)
#define	STAGE_MIN_Z		(-164500.0f)
#define	STAGE_WIDTH_Z		(STAGE_MIN_Z-STAGE_MAX_Z)

#define	HOLE_MAX_X		(1500.0f)
#define	HOLE_MIN_X		(-1500.0f)
#define	HOLE_MAX_Z		(-148500.0f)
#define	HOLE_MIN_Z		(-152000.0f)

#define	STAGE_PIPE_X		(5200.0f)
#define	STAGE_PIPE_Y		(-2800.0f)

#define	RANDOM_Z		(1000.0f)

#define	PANNEL1_Z1		(-152650.0f)
#define	PANNEL1_Z2		(-153850.0f)
#define	PANNEL1_Z3		(-155050.0f)

#define	PANNEL2_Z1		(-152950.0f)
#define	PANNEL2_Z2		(-154750.0f)

#define	TANK_Z			(-153800.0f)

#define	EXP_COUNT		(60)
#define	LIFE			(90)



/*--- クラスター爆発単体エフェクト ---*/
extern void *NewHarrierClasterUnitSmoke( FVECTOR *position );
extern void *NewHarrierClasterSpark( FVECTOR *pos );



enum {
    NO_HIT 		= 0x00,		// 当たっていない
    STAGE_HIT		= 0x01,		// ステージに着弾
    PLAYER_HIT_UPPER	= 0x02,		// 上フロア、階段のプレイヤーに着弾
    FLOOR_BREAK		= 0x04,		// ステージ壊れ発動
    PLAYER_HIT_LOWER	= 0x08,		// 下フロアのプレイヤーに着弾
};
    


typedef	struct	{

    GV_ACT_EX	actor;
    int			count;

    FVECTOR		hit_pos;
    int			hit_flag;

    FVECTOR		*pos;
    int			*flag;

    int			brk_proc;		//ステージ破壊プロック
	int			name;
    int 		brk_flag[6];
    
} Work;


extern void *NewHarrierStageObjs( int objcode, FMATRIX *mat, int *flag );
extern void *NewHarrierTankBreak( int *flag );


/*--- 壊れオブジェクト配置関数 ---*/
static void CallBreakObject( Work *work )
{

	FMATRIX		mat;

	/* 鉄パネル１ */
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] = -1200.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -152650.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[0] );
	
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] = -350.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -152650.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[0] );
	
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] =  450.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -152650.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[0] );


	/* 鉄パネル２ */
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] = -1200.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -153850.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[1] );
	
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] = -350.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -153850.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[1] );
	
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] =  450.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -153850.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[1] );


	/* 鉄パネル３ */
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] = -1200.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -155050.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[2] );
	
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] = -350.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -155050.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[2] );
	
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] =  450.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -155050.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk0"), &mat, &work->brk_flag[2] );


	/* 板パネル１ */
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] = -2200.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -152950.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk1"), &mat, &work->brk_flag[3] );
	
	
	/* 板パネル２ */
	DG_COPY_MAT( &mat, &DG_UnitMatrix );
	mat.m[3][0] = -2200.0f;
	mat.m[3][1] = -1000.0f;
	mat.m[3][2] = -154700.0f;
	NewHarrierStageObjs( GV_StrCode("w25a_floor_brk1"), &mat, &work->brk_flag[4] );
	

	/* タンク */
	NewHarrierTankBreak( &work->brk_flag[5] );

}


/* アクト関数 */
static void Act( Work *work )
{

    float	rate;
    float	pre_rate;
    FVECTOR	pos;

    float	pre_z;
    float	now_z;
    
    *work->flag = NO_HIT;
    

    /* EXP_COUNT(60フレーム)で爆発が進行 ->> 実際はrateをもらう？ */
    if ( work->count < EXP_COUNT ) {

	/* カウンターからパラメータ算出 */
	rate  = (float)work->count / (float)EXP_COUNT;
	now_z = STAGE_MAX_Z + STAGE_WIDTH_Z * rate;

	/* パラメータから爆発位置を算出し爆発呼び出し */
	if ( irnd()%3 ) {

	    pos.vx = STAGE_CENTER_X + STAGE_WIDTH_X * frnd();
	    pos.vz = now_z + frnd()*RANDOM_Z;
	    pos.vw = 1.0f;

	    /* ステージ両端の踊り場で爆発 */
	    if ( pos.vz > -137500.0f || pos.vz < -162000.0f ) {
		pos.vy = 500.0f;
	    }	    
	    /* ステージ中心の穴に着弾したら下の階で爆発 */
	    else if ( pos.vz < HOLE_MAX_Z && pos.vz > HOLE_MIN_Z && pos.vx < HOLE_MAX_X && pos.vx > HOLE_MIN_X ) {
		pos.vy = LOWER_FLOOR_HEIGHT;
	    }
	    /* 通常ステージ面で爆発 */
	    else {
		pos.vy = UPPER_FLOOR_HEIGHT;
	    }

	    NewHarrierClasterUnitSmoke( &pos );
	    DG_COPY_VEC( work->pos, &pos );
	    *work->flag = STAGE_HIT;

	}

	/* 隣のパイプ着弾 */
	if ( !(irnd()%5) ) {

	    pos.vx = STAGE_PIPE_X;
	    pos.vy = STAGE_PIPE_Y;
	    pos.vz = now_z + frnd()*RANDOM_Z;
	    pos.vw = 1.0f;

	    NewHarrierClasterUnitSmoke( &pos );

	}

	/* 階段着弾 */
	if ( !( irnd()%3 ) ) {

	    pos.vz = now_z;

	    if ( pos.vz < -144000.0f && pos.vz > -150000.0f ) {
		pos.vx = -3500.0f;
		pos.vw = 1.0f;

		if ( pos.vz > -146000.0f ) {
		    pos.vy = LOWER_FLOOR_HEIGHT;
		}
		else if ( pos.vz < -149000.0f ) {
		    pos.vy = UPPER_FLOOR_HEIGHT;
		}
		else {
		    pos.vy = LOWER_FLOOR_HEIGHT + ( -146000.0f - pos.vz );
		}

		NewHarrierClasterUnitSmoke( &pos );

	    }

	}



	if ( work->count >= 2 ) {

	    /* プレイヤー位置から当たり判定 */
	    if ( !work->hit_flag ) {

		if ( GM_PlayerPosition.vx > -3000.0f && GM_PlayerPosition.vx < 3000.0f ) {
	    
		    /* 通常ステージ平面 */
		    if ( GM_PlayerPosition.vy > UPPER_FLOOR_HEIGHT ) {

			if ( GM_PlayerPosition.vz > now_z ) {
			    pre_rate = (float)( work->count - 2 ) / (float)EXP_COUNT;
			    if ( GM_PlayerPosition.vz < STAGE_MAX_Z + STAGE_WIDTH_Z * pre_rate ) {
				//printf("claster_hit! floor1 ---------------------\n");

				/* 一応当たり地点にエフェクト単体呼び出し */
				DG_COPY_VEC( &pos, &GM_PlayerPosition );
				pos.vy = UPPER_FLOOR_HEIGHT;
				NewHarrierClasterUnitSmoke( &pos );

				DG_COPY_VEC( work->pos, &GM_PlayerPosition );
				work->hit_flag = 1;
				*work->flag    = PLAYER_HIT_UPPER;
				
			    }

			}		

		    }
		    /* 下フロア平面 */
		    else {

			if ( GM_PlayerPosition.vz < HOLE_MAX_Z && GM_PlayerPosition.vz > HOLE_MIN_Z
			     && GM_PlayerPosition.vx < HOLE_MAX_X && GM_PlayerPosition.vx > HOLE_MIN_X ) {

			    if ( GM_PlayerPosition.vz > now_z ) {

				pre_rate = (float)( work->count - 2 ) / (float)EXP_COUNT;
				if ( GM_PlayerPosition.vz < STAGE_MAX_Z + STAGE_WIDTH_Z * pre_rate ) {
				    //printf("claster_hit! floor2 ---------------------\n");

				    /* 一応当たり地点にエフェクト単体呼び出し */
				    DG_COPY_VEC( &pos, &GM_PlayerPosition );
				    pos.vy = LOWER_FLOOR_HEIGHT;
				    NewHarrierClasterUnitSmoke( &pos );

				    DG_COPY_VEC( work->pos, &GM_PlayerPosition );
				    work->hit_flag = 1;
				    *work->flag    = PLAYER_HIT_LOWER;
				}

			    }

			}    

		    }

		}
		else {

		    /* 階段 */
		    if ( GM_PlayerPosition.vz > now_z ) {
			pre_rate = (float)( work->count - 2 ) / (float)EXP_COUNT;
			if ( GM_PlayerPosition.vz < STAGE_MAX_Z + STAGE_WIDTH_Z * pre_rate ) {
			    //printf("claster_hit! stair -------------------------\n");

			    /* 一応当たり地点にエフェクト単体呼び出し */
			    DG_COPY_VEC( &pos, &GM_PlayerPosition );
			    if ( pos.vz > -146000.0f ) {
				pos.vy = LOWER_FLOOR_HEIGHT;
			    }
			    else if ( pos.vz < -149000.0f ) {
				pos.vy = UPPER_FLOOR_HEIGHT;
			    }
			    else {
				pos.vy = LOWER_FLOOR_HEIGHT + ( -146000.0f - pos.vz );
			    }
			    NewHarrierClasterUnitSmoke( &pos );


			    DG_COPY_VEC( work->pos, &GM_PlayerPosition );
			    work->hit_flag = 1;
			    *work->flag    = PLAYER_HIT_UPPER;
			}
		    }

		}		

	    }

	}



	/* ステージ壊れ呼び出し */
	rate = (float)( work->count - 1 ) / (float)EXP_COUNT;
	now_z = STAGE_MAX_Z + STAGE_WIDTH_Z * rate;
	pre_rate = (float)( work->count - 2 ) / (float)EXP_COUNT;
	pre_z = STAGE_MAX_Z + STAGE_WIDTH_Z * pre_rate;



	if ( pre_z > PANNEL1_Z1 && now_z <= PANNEL1_Z1 ) {

		/* 壊れステージ呼び出し */
		GCL_ARGS	args;
		args.argc = 1;
		args.argv = &work->name;
		GCL_ExecProc( work->brk_proc, &args );

		/* 壊れオブジェクト配置 */
	    CallBreakObject( work );
	    *work->flag |= FLOOR_BREAK;
		
		work->brk_flag[0] = 1;
	}

	if ( pre_z > PANNEL1_Z2 && now_z <= PANNEL1_Z2 ) {
		work->brk_flag[1] = 1;
	}

	if ( pre_z > PANNEL1_Z3 && now_z <= PANNEL1_Z3 ) {
		work->brk_flag[2] = 1;
	}


	/* 板パネル */
	if ( pre_z > PANNEL2_Z1 && now_z <= PANNEL2_Z1 ) {
		work->brk_flag[3] = 1;
	}

	if ( pre_z > PANNEL2_Z2 && now_z <= PANNEL2_Z2 ) {
		work->brk_flag[4] = 1;
	}


	/* タンク */
	if ( pre_z > TANK_Z && now_z <= TANK_Z ) {
		work->brk_flag[5] = 1;
	}


    }
    else {	/* 光追加呼び出し */

		if ( !( irnd()%3 ) ) {

			pos.vx = STAGE_CENTER_X + STAGE_WIDTH_X * frnd();
			pos.vz = STAGE_MAX_Z + STAGE_WIDTH_Z * ( 0.50f + frnd()*0.50f ) + frnd() * RANDOM_Z;
			pos.vw = 1.0f;

			/* ステージ中心の穴に着弾したら下の階で爆発 */
			if ( pos.vz < HOLE_MAX_Z && pos.vz > HOLE_MIN_Z && pos.vx < HOLE_MAX_X && pos.vx > HOLE_MIN_X ) {
				pos.vy = LOWER_FLOOR_HEIGHT;
			}
			else {
				pos.vy = UPPER_FLOOR_HEIGHT;
			}

			NewHarrierClasterSpark( &pos );
			NewHarrierClasterSpark( &pos );
	    
		}

    }




    
    if ( ++work->count >= LIFE ) {
		GV_DestroyActor( work );
    }   
    
}


static void Die( Work *work )
{
}



static int GetResources( Work *work, FVECTOR *pos, int *flag, int brk_proc )
{

    int		i;
    
    work->pos = pos;
    work->flag = flag;
    
    work->hit_flag = 0;
    work->count = 0;

	work->brk_proc = brk_proc;
	work->name = 0;
    for ( i = 0 ; i < 6 ; i++ ) {
		work->brk_flag[i] = 0;
    }
    
    return 0;

}



/*-------- void *NewHarrierClasterExplosion( void ) ----------
              クラスター爆発エフェクト管理キャラ
------------------------------------------------------------*/
void *NewHarrierClasterExplosion( FVECTOR *pos, int *flag, int brk_proc )
{

	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, flag, brk_proc ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work;

}


