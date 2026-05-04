//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_snipe.c
  エマ 狙撃シナリオ用コマンド

  2000/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_snipe.c,v 1.1.1.3 2002/11/19 11:46:02 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../include/emma.h"

static Work *EMA_Work = NULL ;

void EMA_SnipeDie()
{
    EMA_Work = NULL ;
}

void EMA_SnipeBirth( void *ptr )
{
    Work *work = ptr ;

    EMA_Work = work ;
}


/*

  狙撃ヴァンプが使う コマンド


*/
int EMA_CommandGiveDamage( int damage )
{
    Work *work ;
    int   id   ;

    if ( GM_IsGameOver() )
	return -1 ;
    if ( (work = EMA_Work) )
	if ( !EMA_Flag( EMA_F_GAMEOVER ) )
	{
	    EMA_ResetFlag(EMA_F_HURT_BY_PLY) ;

	    /* 狙撃ヴァンプのみこっち */
	    if ( EMA_Flag(EMA_F_HURT_BY_VMP) )
	    {
		if ( !EMA_DamageVitality( work, damage, 0 ) )
		    GM_SeSetMode( SD_V_EMAOUH02, &work->control.mov,
				  GM_SEMODE_BOMB ) ;
		else
		{
		    GM_SeSetMode( SD_V_EMAHNG01, &work->control.mov,
				  GM_SEMODE_BOMB ) ;
		    //work->voice_id = SD_V_EMAHNG01 ;
		    //work->voice_tim = 1 ;

		    if ( work->voice_cnt < 3*4 && !(work->voice_cnt&3) )
		    {
			id = (work->voice_cnt<4 ? EMA_VC711 :
			      work->voice_cnt<8 ? EMA_VC721 : EMA_VC731) ;

			EMA_UtilStartStreamIdx( work, id ) ;
		    }
		    else
			work->voice_cnt++ ;
		}
	    }

	    /* それ以外の場合 */
	    else
	    {
		if ( !EMA_DamageVitality( work, damage, 0 ) )
		{
		    GM_SeSetMode( SD_V_EMAOUT01, &work->control.mov,
				  GM_SEMODE_BOMB ) ;
printf( "HONOO\n" ) ;
		    //work->voice_tim = 1 ;
		    //work->voice_id  = SD_V_EMAOUT01 ;
		}
	    }

	    /* こっちのダメージは無敵なし */
	    EMA_ResetFlag(EMA_F_NON_DAMAGE) ;
	    work->non_damage = 1 ;

	    return EMA_Work->npc.action.life ;
	}
    return -1 ;
}

CAPTURE_TARGET *EMA_CommandGetCaptureTarget()
{
    Work *work = EMA_Work ;

    if ( work )
    {
	NPCWORK *npc = &EMA_Work->npc ;

	/* 首締め状態にする */
	EMA_SetActMotion( npc, npc->capture_mar, NPC_MOT_HANG_TIE ) ;
	NPC_SetModeFromPad( npc, NPC_ActHang, npc->capture_mar, NPC_MOT_HANG_TIE, npc->action.pad ) ;

	EMA_Work->control.skip_flag &= ~CTRL_SKIP_FLR_CHECK ;

	/* ヴァンプに首締められている */
	EMA_SetFlag( EMA_F_HURT_BY_VMP ) ;

	return &work->capture ;
    }
    return NULL ;
}



/*

  シナリオ班が使うストリームコマンド

 */
int NewEMA_CommandStreamPlayEma()
{
    int id ;
    Work *work ;

    if ( (work = EMA_Work) == NULL )
        return 0 ;

    if ( work->npc.action.faint_count > 0 )
        return 0 ;

    if ( EMA_Flag(EMA_F_IS_ATTACKED) )
        return 0 ;

    id = GCL_GetNextInt() ;
    if ( EMA_UtilStartStream( EMA_Work, id ) >= 0 )
    {
	EMA_Work->voice_proc = GCL_GetOptionValue( 'p', 0 ) ;
	EMA_Work->voice_flg = EMA_VCF_SNIPER_EMMA_WISP ;

printf( "NewEMA_CommandStreamPlayEma %x %x\n", id, EMA_Work->voice_proc ) ;
	return id ;
    }
    return 0 ;
}

int NewEMA_CommandStreamPlaySna()
{
    int id ;

    if ( EMA_Work == NULL )
        return 0 ;

    switch( GetNPCSnakeStatus() )
    {
    case NPCSNAKE_SCNSTATUS_DAMAGE_BY_RAIDEN:
    case NPCSNAKE_SCNSTATUS_SLEEP:
    case NPCSNAKE_SCNSTATUS_COUNTERATTACK:
    case NPCSNAKE_SCNSTATUS_FAINT:
	return 0 ;

    default:
	id = GCL_GetNextInt() ;
	if ( EMA_UtilStartStream( EMA_Work, id ) >= 0 )
	{
	    EMA_Work->voice_proc = GCL_GetOptionValue( 'p', 0 ) ;
	    EMA_Work->voice_flg = EMA_VCF_SNIPER_SNAKE_WISP ;
	    return id ;
	}
    }
    return 0 ;
}

int NewEMA_CommandStreamStop()
{
    if ( EMA_Work == NULL )
        return 0 ;
    if ( EMA_Work->voice_code == GCL_GetNextInt() )
	EMA_UtilStopStream( EMA_Work ) ;
    return 1 ;
}

int NewEMA_CommandGetMiceVolPan()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
    IVECTOR buf ;
    FVECTOR pos ;
    int     vol, pan ;
    float bp_angle;

    if ( !GCL_NextStr() )
        return -1 ;
    GCL_GetNextIV( (int *)&buf ) ;
    vu0_IV0toFV( &buf, &pos ) ;
    GM_SeGetVolPan( &pos, GM_SEMODE_BOMB, &vol, &pan, &bp_angle ) ;

    if ( GCL_NextStr() )
    {
	GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
	GCL_SetVarRef( &ref, 0, vol ) ;
    }
    if ( GCL_NextStr() )
    {
	GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
	GCL_SetVarRef( &ref, 0, pan ) ;
    }

    return 1 ;
}
