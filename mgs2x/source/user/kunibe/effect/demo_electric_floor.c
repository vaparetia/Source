//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  demo_electric_floor.c
  デモ用電撃床エフェクト呼び出し
  2001/06/01 Yuuta Kunibe	
  $Id: demo_electric_floor.c,v 1.1.1.3 2002/11/19 11:44:36 Yoshizawa1 Exp $
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


#define COLOR			(255)
#define ALPHA			(128)

#define N_PRIMS			(2)
#define N_VERTS 		(64)
#define CENTER_NUM 		( N_PRIMS * N_VERTS / 2 )

/* 電撃床情報 */
#define	FLOOR_MAX_X		(9375.0f)
#define	FLOOR_MIN_X		(4000.0f)
#define	FLOOR_MAX_Z		(-239500.0f)
#define	FLOOR_MIN_Z		(-242000.0f)

#define PANNEL_NUM		(12)

#define	PANNEL_WX		(750.0f)
#define	PANNEL_WZ		(350.0f)



/* パネル位置 */
static float pannel_PX[3] = { 8400.0f, 6600.0f, 4900.0f };
static float pannel_PZ[4] = {-239800.f, -240400.0f, -241050.0f, -241700.0f };

extern void *NewSpritePlasma( FVECTOR *from, FVECTOR *middle, FVECTOR *to,
							  float width, float noise_plasma, FVECTOR *color, int flag, int life );

extern void *NewSpark2( int n_packets, FMATRIX *matrix,
			float min_speed, float speed_wide, float gravity,
			SVECTOR *rot,SVECTOR *rot_wide,
			FVECTOR *color, float length, int count );


enum {
    EFLOOR_MODE_ACTIVE	= 0x00,
    EFLOOR_MODE_SLEEP	= 0x01,
    EFLOOR_MODE_DEATH	= 0x02,
};

enum {
    EFLOOR_MESG_ACTIVE	= 0x00,
    EFLOOR_MESG_SLEEP	= 0x01,
    EFLOOR_MESG_KILL	= 0x02,
};


typedef	struct	{

    GV_ACT_EX	actor;
	int			name;
	int			seed;

	int			mode;

    int			flag;
    
    int			cnt;
    int			cnt2;

    FVECTOR		light_pos;
    int			light_life;

    FVECTOR		color;
    
} Work;



// シード固定ランダム関数
static inline int EFLR_IRand( Work *work, int a ) {  
    return ( GM_IRnd( &work->seed ) % a ); 
}
static inline int EFLR_GetRandom( Work *work, int min, int max ) {  
    return ( GM_IRnd( &work->seed ) % ((max - min) + 1) + min ); 
}
static inline float EFLR_FRand( Work *work ) {  
    return ( GM_FRnd( &work->seed ) ); 
}
static inline float EFLR_Rand( Work *work ) {  
    return ( GM_Rnd( &work->seed ) ); 
}


static void RecieveMessage( Work* work )
{
    
    GV_MSG	*msg;
    int 	n_msg;


    if ( work->name == 0 ) {
		return;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 


    /* メッセージ反映 */
    while ( n_msg-- > 0 ) {

		switch ( msg->message[ 0 ] ) {
		case EFLOOR_MESG_ACTIVE:		
			work->mode = EFLOOR_MODE_ACTIVE;
			break;
		case EFLOOR_MESG_SLEEP:
			work->mode = EFLOOR_MODE_SLEEP;
			break;
		case EFLOOR_MESG_KILL:
			work->mode = EFLOOR_MODE_DEATH;
			break;
		}
		msg++;

    }

}



/* アクト関数 */
static void Act( Work *work )
{

    FVECTOR	from;
    FVECTOR	middle;
    FVECTOR	to;
    float	width;
    int		pannel;
    FMATRIX	mat;
    SVECTOR	rot_vec,rot_wide;




	/* メッセージ取得 */
	RecieveMessage( work );



#if 0	/* debug用mode切替え */
    if ( GV_PadData[1].press & PAD_R1 ) {
		if ( work->mode == EFLOOR_MODE_SLEEP ) {
			work->mode = EFLOOR_MODE_ACTIVE;
		}
		else {
			work->mode = EFLOOR_MODE_SLEEP;
		}
    }
    else if ( GV_PadData[1].press & PAD_L1 ) {
		work->mode = EFLOOR_MODE_DEATH;
    }
#endif	
    



	switch ( work->mode ) {
		/* 定常状態 */		
	case EFLOOR_MODE_ACTIVE:	
	
		if ( --work->cnt < DIRECT_TICK( 30 ) ) {

			if ( --work->cnt2 == 0 ) {

			    // 開始パネル
			    pannel = EFLR_IRand( work, PANNEL_NUM );
		
			    from.vx = pannel_PX[pannel/4] + EFLR_FRand( work ) * PANNEL_WX;
			    from.vy =-100.0f;
			    from.vz = pannel_PZ[pannel%4] + EFLR_FRand( work ) * PANNEL_WZ;
			    from.vw = 1.0f;
		
				
			    // 終端パネル
			    /* pannelの値で隣のパネルの値を決めで算出 */
			    switch ( pannel ) {
			    case 0:
					switch( EFLR_IRand( work, 2 ) ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel += 4;
						break;
					}
					break;
			    case 1:
			    case 2:
					switch ( EFLR_IRand( work, 3 ) ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel -= 1;
						break;
					case 2:
						pannel += 4;
						break;
					}			
					break;
			    case 3:
					switch( EFLR_IRand( work, 2 ) ) {
					case 0:
						pannel -= 1;
						break;
					case 1:
						pannel += 4;
						break;
					}
					break;
			    case 4:
					switch( EFLR_IRand( work, 2 ) ){
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel += 4;
						break;
					case 2:
						pannel -= 4;
						break;
					}
					break;
				case 5:
				case 6:
					switch ( EFLR_IRand( work, 4 ) ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel -= 1;
						break;
					case 2:
						pannel += 4;
						break;
					case 3:
						pannel -= 4;
						break;
					}			
					break;
				case 7:
					switch ( EFLR_IRand( work, 3 ) ) {
					case 0:
						pannel -= 1;
						break;
					case 1:
						pannel += 4;
						break;
					case 2:
						pannel -= 4;
						break;
					}
					break;
				case 8:
					switch ( EFLR_IRand( work, 2 ) ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel -= 4;
						break;
					}
					break;
				case 9:
				case 10:
					switch ( EFLR_IRand( work, 2 ) ) {
					case 0:
						pannel += 1;
						break;
					case 1:
						pannel -= 1;
						break;
					case 2:
						pannel -= 4;
						break;
					}			
					break;
				case 11:
					switch ( EFLR_IRand( work, 2 ) ) {
					case 0:
						pannel -= 1;
						break;
					case 1:
						pannel -= 4;
						break;
					}
					break;

				}
			
				to.vx = pannel_PX[pannel/4] + EFLR_FRand( work ) * PANNEL_WX;
				to.vy =-100.0f;
				to.vz = pannel_PZ[pannel%4] + EFLR_FRand( work ) * PANNEL_WZ;
				to.vw = 1.0f;

				// 中継点
				_sceVu0AddVector( &middle, &from, &to );
				_sceVu0ScaleVector( &middle, &middle, 0.50f );
				middle.vx += EFLR_FRand( work ) * 200.0f;
				middle.vy += 300.f + EFLR_FRand( work ) * 150.0f;
				middle.vz += EFLR_FRand( work ) * 200.0f;

				// プラズマ太さ
				width = 15.0f + EFLR_FRand( work ) * 5.0f;


				// プラズマ呼び出し
				NewSpritePlasma( &from, &middle, &to, width, 1.0f, &work->color, 0, 4 );



				/* 火花呼び出し */
				DG_COPY_MAT( &mat, &DG_UnitMatrix );
				DG_COPY_VEC( (FVECTOR *)mat.m[3], &from );
				rot_vec.vx = 3072-512;
				rot_vec.vy = 0;
				rot_vec.vz = 0;
				rot_wide.vx = 1024;
				rot_wide.vy = 1024;
				rot_wide.vz = 1;
				NewSpark2( 16,					/* 発生火花数 */
						   &mat,				/* マトリクス */
						   1.0F,				/* 最小スピード */
						   100.0F + 25.0F * EFLR_FRand( work ),		/* 幅スピード */
						   10.0F,				/* 重力 */
						   &rot_vec, &rot_wide,			/* 回転 回転幅 */
						   &work->color,				/* 色 */
						   1.0F,				/* スピードに対する火の長さの割合 */
						   10 );				/* 生存フレーム数 */


				DG_COPY_VEC( (FVECTOR *)mat.m[3], &to );
				NewSpark2( 16,					/* 発生火花数 */
						   &mat,				/* マトリクス */
						   1.0F,				/* 最小スピード */
						   100.0F + 25.0F * EFLR_FRand( work ),		/* 幅スピード */
						   10.0F,				/* 重力 */
						   &rot_vec, &rot_wide,			/* 回転 回転幅 */
						   &work->color,				/* 色 */
						   1.0F,				/* スピードに対する火の長さの割合 */
						   10 );				/* 生存フレーム数 */


				DG_COPY_VEC( &work->light_pos, &middle );
				work->light_life = 4;

				work->cnt2 = DIRECT_TICK( 1+EFLR_IRand( work, 15 ) );

			}

		}


		/* テンプライト設置 */
		if ( work->light_life > 0 ) {
			DG_SetTmpLight2 (
							 &work->light_pos,
							 500.0f + 50.f * (float)work->light_life,
							 1000.0f + 100.f * (float)work->light_life,
							 (int)(work->color.vx) | (int)(work->color.vy)<<8 | (int)(work->color.vz)<<16,
							 LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY );
			work->light_life--;
		}

		if ( work->cnt == 0 ) {
			work->cnt = DIRECT_TICK( 40 + EFLR_IRand( work, 60 ) );
		}
		break;

		/* 待機状態 */
	case EFLOOR_MODE_SLEEP:
		work->cnt  = DIRECT_TICK( 60 );
		work->cnt2 = 1;
		break;

		/* 破棄 */
	case EFLOOR_MODE_DEATH:
		GV_DestroyActor( work );
		break;

	}
	


}


static void Die(Work *work )
{
}


static void InitWork( Work *work )
{

	work->seed = 1345625;
	
    work->cnt  = DIRECT_TICK( 30 );
    work->cnt2 = DIRECT_TICK( 15 + EFLR_IRand( work, 10 ) );

    work->color.vx = 32.0f;
    work->color.vy = 100.0f;
    work->color.vz = 190.0f;
    work->color.vw = 64.0f;

	work->mode = EFLOOR_MODE_SLEEP;
    
}


void *NewDemoElectricFloor( int name )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
	    GV_SetActor( &( work->actor ), Act, Die );
	    GV_ActorEX( &work->actor );
		work->name = name;
	    InitWork( work );
	}

	return (void *)work ;

}



