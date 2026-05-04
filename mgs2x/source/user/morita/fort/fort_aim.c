//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_aim.c
   フォーチュン戦専用 爆心地の計算

   2001/01/17 T. Morita
   $Id: fort_aim.c,v 1.1.1.3 2002/11/19 11:46:06 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"
#include "include/fort.h"

#define FRT_PHASE_BIT    24
#define FRT_N_AIM        80
#define FRT_PRIOLITY_BIT 16

typedef struct aim_t
{
    FVECTOR front ; /* vwに幅が入る */
    FVECTOR rear  ; /* vwに幅が入る */
    u_int   aim_id ;/* ID(ターゲットへのアドレス) */
    u_int   nxt_id ;/* 優先先のID(ターゲットへのアドレス) */
    u_int   phase ; /* 攻撃フェーズ */
    u_int   flag  ;
} AIM ;


static AIM   FRT_AIM_AimSpot[FRT_N_AIM] ;
static int   FRT_AIM_n_AimSpot = 0 ;

static AIM *FRT_AIM_Pool[FRT_N_AIM] ;
static int  FRT_AIM_n_Pool = 0 ;

extern int FRT_OBJ_ActCheckHideFromToByID( u_int id,
					   FVECTOR *from, FVECTOR *to ) ;

static AIM *FRT_AIM_SearchAimSpot( u_int aim_id )
{
    int  i   ;
    AIM *aim ;

    aim = FRT_AIM_AimSpot ;
    for ( i=FRT_N_AIM ;--i>=0 ; aim++ )
	if ( aim_id == aim->aim_id )
	    return aim ;
    return NULL ;
}



/*

  初期化関数

*/
void FRT_AIM_InitAimSpot()
{
    int  i   ;
    AIM *aim ;

    aim = FRT_AIM_AimSpot ;
    for ( i=FRT_N_AIM ;--i>=0 ; aim++ )
	aim->flag = 0 ;
    FRT_AIM_n_AimSpot = 0 ;
    FRT_AIM_n_Pool =  0;
}

/*

  狙い元の登録

*/
int FRT_AIM_AddAimSpot( void *aim_id, void *nxt_id, int phase,
			FVECTOR *center,
			FVECTOR *front, FVECTOR *rear, u_int flag )
{
    int  i   ;
    AIM *aim ;

    ASSERT( center ) ;

    aim = FRT_AIM_AimSpot ;
    for ( i=FRT_N_AIM ;--i>=0 ; aim++ )
	if ( !aim->flag )
	{
	    aim->aim_id = (u_int)aim_id ;
	    aim->nxt_id = (u_int)nxt_id ;
	    aim->phase = phase ;
	    aim->flag  = flag | FRT_AIM_EnableSpot ;
	    _sceVu0CopyVector( &aim->rear, center ) ;
	    if ( rear )
	    {
		_sceVu0AddVector( &aim->rear , center, rear  ) ;
		aim->flag |= FRT_AIM_RearEnable ;
	    }
	    if ( front )
	    {
		_sceVu0AddVector( &aim->front, center, front ) ;
		aim->flag |= FRT_AIM_FrontEnable ;
	    }
	    else
	    {
		_sceVu0CopyVector( &aim->front, center ) ;
		aim->flag |= FRT_AIM_FrontEnable ;
	    }
printf( "added %08x nxt%08x flg%x phase %x %.0f %.0f %.0f\n",
	aim->aim_id,
	aim->nxt_id,
	aim->flag, aim->phase,
	aim->front.vx,aim->front.vy,aim->front.vz ) ;

	    return ++FRT_AIM_n_AimSpot ;
	}
    return -1 ;
}


/*

  狙い元の登録

*/
int FRT_AIM_RemoveAimSpot( void *aim_id )
{
    AIM *aim ;

    if ( (aim = FRT_AIM_SearchAimSpot( (u_int)aim_id )) )
    {
	printf( "removed %.0f %.0f %.0f\n", aim->front.vx,aim->front.vy,aim->front.vz ) ;
	aim->flag = 0 ;
	return --FRT_AIM_n_AimSpot ;
    }
    return -1 ;
}




/*

  フォーチュンが使う方

*/
int FRT_AIM_GetNumberOfPool()
{
    return FRT_AIM_n_AimSpot ;
}

/* 標的フェーズを優先順位順に pool にならべる */
int FRT_AIM_GetAimmingObjects( u_int phase, int type, FVECTOR *src )
{
    int  i, j ;
    AIM *aim = FRT_AIM_AimSpot ;

    ASSERT( src ) ;

    FRT_AIM_n_Pool = 0 ;

    /* 標的フェーズを優先順位順に pool にならべる */
    for ( i=FRT_N_AIM ; --i>=0 ; aim++ )
	if ( (aim->flag & FRT_AIM_EnableSpot) && (aim->flag & type) )
	    if ( type & FRT_AIM_NoPhase || aim->phase <= phase )
//		if ( type & FRT_AIM_NoOnlineCheck || !FRT_OBJ_ActCheckHideFromToByID( aim->id, &aim->front, src ) )
		{
		    for ( j=FRT_AIM_n_Pool ; j>0 ; j-- )
			if ( FRT_AIM_Pool[j-1]->phase < aim->phase )
			    break ;
			else
			    FRT_AIM_Pool[j] = FRT_AIM_Pool[j-1] ;
		    FRT_AIM_n_Pool++ ;
		    FRT_AIM_Pool[j] = aim ;
		}
    return FRT_AIM_n_Pool ;
}

int FRT_AIM_GetAimSpot( FVECTOR *aim_spot,
			u_int phase, int type,
			FVECTOR *src, FVECTOR *dst )
{
    FVECTOR v ;
    AIM    *aim ;
    float   d, min ;
    int     i ;

    ASSERT( src ) ;

    /* 誰もpoolに並べていなかったら自分で並べる */
    if ( !FRT_AIM_n_Pool )
	FRT_AIM_n_Pool = FRT_AIM_GetAimmingObjects( phase, type, src ) ;

    /* プールから標的に近いものを選ぶ */
    if ( FRT_AIM_n_Pool )
    {
	aim = NULL ;
	if ( !dst )
	    dst = src ;
	if ( type & FRT_AIM_DontDestinate )
	    min = 6000000000000.0f ;
	else
	    min = 2000.0f * 2000.0f ;

#if 0/* DEBUG_MODE*/
printf( "ARGtype %x phase %x nAimSpot%d nPool%d\n", type,  phase, FRT_AIM_n_AimSpot, FRT_AIM_n_Pool ) ;
	for ( i=0 ; i<FRT_AIM_n_Pool ; i++ )
	{
	    if ( !(type & FRT_AIM_NoPhase) )
		if ( FRT_AIM_Pool[0]->phase != FRT_AIM_Pool[i]->phase )
		    break ;
	    printf( "type %x phase %x FRONT%.0f %.0f %.0f\n",
		    FRT_AIM_Pool[i]->flag, FRT_AIM_Pool[i]->phase,
		    FRT_AIM_Pool[i]->front.vx ,
		    FRT_AIM_Pool[i]->front.vy ,
		    FRT_AIM_Pool[i]->front.vz ) ;
	}
#endif

	for ( i=0 ; i<FRT_AIM_n_Pool ; i++ )
	{
	    if ( !(type & FRT_AIM_NoPhase) )
		if ( FRT_AIM_Pool[0]->phase != FRT_AIM_Pool[i]->phase )
		    break ;

	    v.vx = dst->vx - FRT_AIM_Pool[i]->front.vx ;
	    v.vz = dst->vz - FRT_AIM_Pool[i]->front.vz ;
	    d = v.vx*v.vx + v.vz*v.vz ;
	    if ( d < min )
		min = d, aim = FRT_AIM_Pool[i] ;
	}
	FRT_AIM_n_Pool = 0 ;
	if ( aim )
	    //if ( !FRT_OBJ_ActCheckHideFromToByID( aim->id, &aim->front, src ) )
	    {
		_sceVu0CopyVector( aim_spot, &aim->front ) ;
		return 1 ;
	    }
    }

    return 0 ;
}


static int FRT_AimCheck = 0 ;

/* 当たる先をみる。もし他を当てなければならなかったら FRT_HitTargetCallBack()で変更される */
void FRT_AIM_CheckAimSpot( Work *work )
{
    extern FVECTOR FRT_LNR_NozzleOfst ;
    FVECTOR v ;

    /* 同じフレームでチェックをしない */
    if ( FRT_AimCheck != GM_StagePlayTime )
    {
	FRT_AimCheck = GM_StagePlayTime ;/* グローバル変数でここでのみ代入 */

	_sceVu0ApplyMatrix( &v, &work->body.objs->objs[HUMAN21_MIGI_TE].world, &FRT_LNR_NozzleOfst ) ;
	GM_MoveOnlineTarget( &work->seeker, &v, work->trgt_aim ) ;
	GM_PutTarget( &work->seeker ) ;
    }
}

/* 撃つ場所を変えなければならない時に1が返る */
int FRT_AIM_VerifyAimSpot( void *aim_id, FVECTOR *pos )
{
    AIM *aim, *nxt ;
    int changed = 0 ;

    aim = FRT_AIM_SearchAimSpot( (u_int)aim_id ) ;
    if ( aim )
	if ( aim->nxt_id )
	    if ( (nxt = FRT_AIM_SearchAimSpot( aim->nxt_id  )) )
		if ( nxt->flag & FRT_AIM_EnableSpot )
		{
		    printf( "(aim%08x->nxt%08x)", aim_id, aim->nxt_id ) ;
		    _sceVu0CopyVector( pos, &nxt->front ), changed = 1 ;
		}
    return changed ;
}

#if DEBUG_MODE
void FRT_AIM_DebugPrint()
{
    int i ;
    AIM *aim = FRT_AIM_AimSpot ;

    /* 標的フェーズを優先順位順に pool にならべる */
    for ( i=FRT_N_AIM ; --i>=0 ; aim++ )
	if ( (aim->flag & FRT_AIM_EnableSpot) )
	{
	    printf( "id%x flag%x phase%x pos%.0f,%.0f,%.0f\n",
		    aim->aim_id,
		    aim->flag, aim->phase,
		    aim->front.vx ,
		    aim->front.vy ,
		    aim->front.vz ) ;
	}
}
#endif
