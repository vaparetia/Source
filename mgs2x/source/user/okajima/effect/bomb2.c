/*

	bomb2.c
	爆炎エフェクト
	2001/01/24 S.Okajima
	$Id: bomb2.c,v 1.1.1.3 2002/11/19 11:47:00 Yoshizawa1 Exp $

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
#include	"../effect/bomb_vibration.h"

#define	FLASH_LENGTH	(2)
#define	FLASH_LENGTH_D	(FLASH_LENGTH * 5)

#define	LIFE_TIME		(64)
#define	SIZE_MIN		(500.0f)
#define	SIZE_RAND		(500.0f)

#define	SEARCH_HZX			(2000)

#define	RAISE				(2800)

#define	JET_NUM			(4)
#define	MAX_VEC			(250.0f)

#define	DELAY_BOMBER_COUNT		(2)

extern float OK_PutBombVibration( FVECTOR *pos, float ratio );
extern void *NewShakeCamera( int chanl, int intense, int time );
extern void *NewShakeCamera2( int chanl, int intense, int time, FVECTOR *pos );
extern void *NewBombGasEffect( FVECTOR *pos, SVECTOR *pole_rot );
extern void *NewFlyingSmoke( FVECTOR *pos, FVECTOR *vector, int life );
extern void *NewBombKasu( FVECTOR *pos, SVECTOR *pole_rot );

int	ok_bomb_num          = 0;
int	ok_bomb_flash_count  = 0;
int	ok_flash_count_owner = 0;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;
	int			mode;
	int			life;
	int			count;
	int			total_num;
	int			viblation_count_h;
	int			viblation_count_l;
	int			my_num;
	FVECTOR		pos;
} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	if( ok_flash_count_owner == work->my_num ){
		if( ok_bomb_flash_count > 0 ) ok_bomb_flash_count--;
	}

	if ( !GM_CheckGameStatus( STATE_DEMO ) ) {
		if( work->viblation_count_h > 0 ){
			work->viblation_count_h--;
			GM_SetVibration1( 0, irnd()%2 );
		}
		if( work->viblation_count_l > 0 ){
			work->viblation_count_l--;
			GM_SetVibration2( 0, (int)( (float)(240 + irnd()%16)*OK_PutBombVibration( &work->pos, 1.0f ) ) );
		}
	}

	work->life--;
	if( work->life < -1 ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	ok_bomb_num--;
	if( ok_bomb_num <= 0 ) ok_bomb_num  = 0;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, int mode )
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

	work->total_num = 0;
	work->mode = mode;
	work->count = 0;

	DG_COPY_VEC( &work->pos, pos );

	map_id = GM_GetHzxGroupID( GM_CurrentStageMap );
	seg_num = HZX_NearHazardCheck( map_id,
					pos,
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
		if( HZX_LevelHazardCheck( map_id, pos, HZX_CHK_ALL, 0 ) & 1 ){
			HZX_GetLevelHeight( flr_height );
			ftemp = pos->vy - flr_height[0];
			if( ftemp > 0 ){
				rad += asinf( ftemp / ((SIZE_RAND + SIZE_MIN) * (float)LIFE_TIME) );
			}
		}
	}
	pole_rot.vx = 4095 & ( short )( ( rad * 1024.0f / PI ) + 0.5f );

	work->life = LIFE_TIME;

//	NewShakeCamera( 0, 256, 10 );

	if( !GM_CheckGameStatus( STATE_PLAY_DEMO ) ) NewShakeCamera2( 0, 512, 10, pos );

	NewBombKasu( pos, &pole_rot );

	/* 中規模の煙 */
	if( work->mode > 0 ) NewBombGasEffect( pos, &pole_rot );

	ok_bomb_num++;
	work->my_num = ok_bomb_num;
//	if( ok_bomb_num==1 ){
	if( ok_bomb_num < 3 ){
		work->viblation_count_h = VIBLATION_LENGTH_1_H;
		work->viblation_count_l = VIBLATION_LENGTH_1_L;
	}else{
		work->viblation_count_h = VIBLATION_LENGTH_2_H;
		work->viblation_count_l = VIBLATION_LENGTH_2_L;
	}

	return 0 ;
}

/* mode:0 中心の炎（あまり使わないでー） */
/* mode:1 中心の炎 ＋ 中規模の煙 */
/* mode:2 中心の炎 ＋ 中規模の煙 ＋ 壁を這う煙 */
void *NewBombEffect2( FVECTOR *pos, int mode )
{
	Work		*work ;

	OPERATOR() ;

	if( ok_bomb_flash_count > 0 ) return NULL ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

