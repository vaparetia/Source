//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bul_fatman_c4_mng.c
   ファットマンＣ４管理者

   2001/04/20	M.Sonoyama
   $Id: bul_fatman_c4_mng.c,v 1.1.1.3 2002/11/19 11:50:00 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"bul_c4.h"

typedef	struct	{
	GV_ACT_EX		actor ;
} FATMAN_C4_MNG_WORK ;

static	void	Act( FATMAN_C4_MNG_WORK *work )
{
	Work		*list, *this ;
	Work		*near[ 3 ] ;
	float		near_len[ 3 ], len ;
	int			i ;
	C4_MNG_WORK	*mng_work ;

	for ( i = 0; i < 3; i ++ ) {
		near[ i ] = NULL ;
		near_len[ i ] = 1000000.0F ;
	}

	mng_work = C4MngWork ;
	list = mng_work->list.next ;
	while( list != NULL ) {
		/* 先頭から検索 */
		this = list ;
		list = list->next ;
		if ( !( this->flag & FLAG_FATMAN ) ) continue ;
		/* ランプ明るさ用パラメータセット */
		if ( this->flag & FLAG_COUNTDOWN ) {
			this->lamp_count = this->blast_count % LAMP_BLINK_COUNT ;
		} else {
			this->lamp_count = LAMP_COLOR_MAX ;
		}
		/* 距離の近い奴３つに音ＯＫフラグセット */
		/* カウントダウンが始まっている奴のみ */
		if ( !( this->flag & FLAG_COUNTDOWN ) ) continue ;
		len = GV_VecLen3F2( &GM_PlayerPosition, ( FVECTOR * )this->world.m[ 3 ] ) ;
		this->lamp_status &= ~LAMP_STATE_SE_ENABLE ;
		for ( i = 0; i < 3; i ++ ) {
			if ( near_len[ i ] > len ) {
				near[ i ] = this ;
				near_len[ i ] = len ;
				break ;
			}
		}
	}
	/* 距離の近い奴３つに音ＯＫフラグセット */
	for ( i = 0; i < 3; i ++ ) {
		if ( near[ i ] != NULL ) {
			near[ i ]->lamp_status |= LAMP_STATE_SE_ENABLE ;	
			if ( i != 0 ) near[ i ]->lamp_status |= ( LAMP_SE_TYPE1 << ( i - 1 ) ) ;
		} else {
			return ;
		}
	}
}

static	void	Die( FATMAN_C4_MNG_WORK *work )
{

}

/* ファットマンＣ４マネージャー起動 */
void		*PL_FatmanC4Manager( void )
{
	FATMAN_C4_MNG_WORK		*work ;

	work = ( FATMAN_C4_MNG_WORK * )GV_NewActor( GV_ACTOR_USER, sizeof( FATMAN_C4_MNG_WORK ) ) ;
	ASSERT( work != NULL ) ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	{
		extern int PL_ProgKaitaiC4BlastProc ;
		PL_ProgKaitaiC4BlastProc = 0 ;
	}

	return work ;
}

/* プログラム起動したＣ４が爆発したときに呼ぶプロックを設定 */
void		PL_SetKaitaiC4BlastProc( int proc )
{
	extern int PL_ProgKaitaiC4BlastProc ;
	PL_ProgKaitaiC4BlastProc = proc ;	
}

