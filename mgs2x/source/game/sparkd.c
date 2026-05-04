//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   sparkd.c
   跳弾管理者・弾痕管理者
   
   2000/03/29 M.Sonoyama
   $Id: sparkd.c,v 1.1.1.3 2002/11/19 11:41:56 Yoshizawa1 Exp $
   */

#ifdef PSX2
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"


int		GM_SparkNoiseLevel ;

static	GM_SPARK_FUNC	SparkFunctions[ GM_MAX_SPARK_FUNCS ] ;

/* エフェクトを跳弾関数配列に登録する */
void	GM_EntrySparkFunction( func, no )
GM_SPARK_FUNC	func ;
int		no ;	
{
    SparkFunctions[ no ] = func ;
    GM_SparkEntryFlag |= ( 1 << no ) ;
}

/* 番号にしたがって跳弾エフェクトをコール */
void	GM_CallSpark( no, m, v )
int		no ;
FMATRIX		*m ;
FVECTOR		*v ;
{
    void	*( *func )( FMATRIX *, FVECTOR *, int ) ;

	GM_SparkNoiseLevel = 0 ;

    if ( ( GM_SparkEntryFlag & ( 1 << no ) ) == 0 ) {
		printf( "spark effect is not entried : %d\n", no ) ;
		return ;
    }

    func = SparkFunctions[ no ] ;
    ( *func )( m, v, no ) ;
}

/*-----------------------------------------------------------------*/

/* ＳＥ呼び分け */
static	void	CallSparkSE( FMATRIX *m, int seNo, int def )
{
	MT_EXCHG_SE		se_data ;
	FVECTOR			pos ;

	GM_SparkNoiseLevel = 0 ;
	MT_GetExchangeSe( &se_data, def, 0, seNo, seNo ) ;
	if ( se_data.se_code != 0xffff ) {
		GV_MatToVec( m, &pos ) ;
#ifdef KP_XBOX
		// アドレス計算を間違うことがあるので、障害エフェクトなし
		GM_SeSetModeAddr( se_data.se_code, &pos, GM_SEMODE_BOMB, GM_INVALID_ADDR ) ;
#else		
		GM_SeSetMode( se_data.se_code, &pos, GM_SEMODE_BOMB ) ;
#endif		
		GM_SparkNoiseLevel = se_data.noise_vol ;
	}
}

/*-----------------------------------------------------------------*/

static	void	*GM_NewSpark( FMATRIX *m, FVECTOR *v, int seNo )
{
    extern void	NewSpark( FMATRIX * ) ;
    FMATRIX	world = {{
		{1.0F,0.0F,0.0F,0.0F},
		{0.0F,1.0F,0.0F,0.0F},
		{0.0F,0.0F,1.0F,0.0F},
		{0.0F,0.0F,0.0F,1.0F}}};
    FVECTOR	pos = { 0.0f, 0.0f, 0.0f, 1.0f};

    GV_MatToVec( m, &pos ) ;

	//GM_SeSetMode( SD_W_RICOCH02, &pos, GM_SEMODE_BOMB ) ;

    DG_ReflectMatrix( v, m, &world ) ;
    GV_VecToMat( &pos, &world ) ;
	NewSpark( &world ) ;

	CallSparkSE( m, seNo, SD_W_RICOCH02 ) ;
    return m ;
}

void	NewEntryNewSpark( void )
{
    GCL_GetOption( 'n' ) ;
    GM_EntrySparkFunction( GM_NewSpark, GCL_GetNextInt() ) ;
}

/*-----------------------------------------------------------------*/

static	void	*GM_NewSparkOnlySE( FMATRIX *m, FVECTOR *v, int seNo )
{
	CallSparkSE( m, seNo, SD_W_RICOCH02 ) ;
    return m ;
}

void	NewEntrySparkOnlySE( void )
{
    GCL_GetOption( 'n' ) ;
    GM_EntrySparkFunction( GM_NewSparkOnlySE, GCL_GetNextInt() ) ;
}

/*-----------------------------------------------------------------*/

static	GM_SCAR_FUNC	ScarFunctions[ GM_MAX_SCAR_FUNCS ] ;

/* エフェクトを弾痕関数配列に登録する */
void	GM_EntryScarFunction( func, no )
GM_SCAR_FUNC	func ;
int		no ;	
{
    ScarFunctions[ no ] = func ;
    GM_ScarEntryFlag |= ( 1 << no ) ;
}

/* 番号にしたがって弾痕エフェクトをコール */
void	GM_CallScar( no, m, s, f )
int		no ;
FMATRIX		*m ;
HZX_SEG		*s ;
HZX_FLR		*f ;
{
    void	*( *func )( FMATRIX *, HZX_SEG *, HZX_FLR * ) ;

    if ( ( GM_ScarEntryFlag & ( 1 << no ) ) == 0 ) {
		printf( "scar effect is not entried : %d\n", no ) ;
		return ;
    }
    func = ScarFunctions[ no ] ;
    ( *func )( m, s, f ) ;
}

/*-----------------------------------------------------------------*/


