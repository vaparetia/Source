//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   stealth.c
   ステルス迷彩

   2001/07/23	M.Sonoyama
   $Id: stealth.c,v 1.1.1.3 2002/11/19 11:50:16 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include "BP_Debug.h"

#ifdef DEBUG_MODE
//#define		STEALTH_HAIR
#endif

extern	void *NewOpticalCamouflage( DG_OBJS *objs, int flag ) ;

typedef	struct	{
	GV_ACT_EX		actor ;
	void			*st_rai ;
	void			*st_hair ;
	int				root_flag ;
	int				arm_flag ;
	void			*work2 ;
} Work ;

typedef	struct	{
	GV_ACT_EX		actor ;	
	int				*root_flag ;
	int				*arm_flag ;
} Work2 ;

/*----------------------------------------------------------------*/

//#ifdef STEALTH_HAIR
#if 1 // BP_KP_Cheats
static	void	EvmHairVisible( int v )
{
	GV_MSG		msg ;
	int			mesg[ 2 ] ;

	if ( GV_PauseLevel != 0 ) return ;
	mesg[ 0 ] = 0 ;
	mesg[ 1 ] = v ;
	msg.address = GV_StrCode( "ライデン髪の毛" ) ;
	msg.message = mesg ;
	msg.message_len = 2 ;
	GV_SendMessage( &msg ) ;
}

/* バンダナ消し */
static	void	VisibleBandana( int v )
{
	GV_MSG		msg ;
	int			buf[ 2 ] ;

	if ( GV_PauseLevel != 0 ) return ;
	buf[ 0 ] = 0 ;
    buf[ 1 ] = v ;
	msg.address = GV_StrCode( "バンダナ１" ) ;
	msg.message = buf ;
	msg.message_len = 2 ;
	GV_SendMessage( &msg ) ;
	msg.address = GV_StrCode( "バンダナ２" ) ;
	GV_SendMessage( &msg ) ;
}

int gNameOfMagazine = 2677990;
static void VisibleMagazine( int v )
{
   GV_MSG		msg ;
   int			buf[ 3 ] ;

   if ( GV_PauseLevel != 0 ) return ;
   buf[ 0 ] = 0 ;
   buf[ 1 ] = gNameOfMagazine;
   buf[ 2 ] = v ;
   msg.address = GV_StrCode( "装備品Ｓ" );
   msg.message = buf ;
   msg.message_len = 3 ;
   GV_SendMessage( &msg ) ;
}
#endif

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
//#ifdef DEBUG_MODE
#if 1 // BP_KP_Cheats
	int			mode = 1 ;
#endif
	if ( GM_PlayerBody == NULL ) return ;

	GM_SetPlayerStatus( PLAYER_STEALTH ) ;

	/* 本体 */
	if ( work->st_rai == NULL ) {
//#ifdef DEBUG_MODE
#if 1 // BP_KP_Cheats
		//if ( PlayerDebugMenuStatus & PDMS_DEEPSTEALTH ) {
      if( gBP_KP_Cheat_OptCmfMode != kBP_KP_Cheat_OptCmf_Normal )
      {
			work->st_rai = NewOpticalCamouflage( GM_PlayerBody->objs, 2 ) ;
			mode = 2 ;
		} else {
			work->st_rai = NewOpticalCamouflage( GM_PlayerBody->objs, 1 ) ;
		}
      //#ifdef STEALTH_HAIR
#if 1 // BP_KP_Cheats
      if( gBP_KP_Cheat_OptCmfMode != kBP_KP_Cheat_OptCmf_Normal )
      {
		   VisibleBandana( 0 ) ;
         VisibleMagazine( 0 );
      }
#endif
#else
		work->st_rai = NewOpticalCamouflage( GM_PlayerBody->objs, 1 ) ;
#endif
		if ( work->st_rai != NULL ) {
			( ( GV_ACT * )work->st_rai )->name = GV_StrCode( "rai_stealth_work" ) ;
		}
		//GV_SetActorChild( work, work->st_rai ) ;
	}		
	/* 髪の毛 */
//#ifdef STEALTH_HAIR
#if 1 // BP_KP_Cheats
   if( gBP_KP_Cheat_OptCmfMode != kBP_KP_Cheat_OptCmf_Normal )
   {
      if ( PL_ShadowHairObjs != NULL ) {
         if ( work->st_hair == NULL ) {
            work->st_hair = NewOpticalCamouflage( PL_ShadowHairObjs, mode ) ;
            if ( work->st_hair != NULL ) {
               ( ( GV_ACT * )work->st_hair )->name = GV_StrCode( "rai_hair_stealth_work" ) ;
            }
            GV_SetActorChild( work, work->st_hair ) ;
            //EvmHairVisible( 0 ) ;
         }
         if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
            DG_InvisibleObjsChanl( PL_ShadowHairObjs, 0 ) ;
         } else {
            DG_VisibleObjsChanl( PL_ShadowHairObjs, 0 ) ;
            DG_VisibleObjsChanl( PL_ShadowHairObjs, 1 ) ;
         }
         EvmHairVisible( 0 ) ;
      } else if ( work->st_hair != NULL ) {
         //GV_DestroyOtherActor( work->st_hair ) ;
         GV_DestroyChild( work ) ;
         work->st_hair = NULL ;			
         EvmHairVisible( 1 ) ;
      }
   }
#endif
	/* フラグの書き換え */
	work->root_flag = GM_PlayerBody->objs->flag & DG_FLAG_SHADOWMAKE ;
	GM_PlayerBody->objs->flag &= ~DG_FLAG_SHADOWMAKE ;
	GM_PlayerArmBody->evmobj->flag |= DG_EVMOBJ_SEMITRANS ;
}

static	void	Die( Work *work )
{
	int		flag ;
	GV_ACT	*child ;

	GM_ResetPlayerStatus( PLAYER_STEALTH ) ;
//#ifdef STEALTH_HAIR
#if 1 // BP_KP_Cheats
   if( gBP_KP_Cheat_OptCmfMode != kBP_KP_Cheat_OptCmf_Normal )
   {
      if ( PL_ShadowHairObjs != NULL ) {
         DG_InvisibleObjsChanl( PL_ShadowHairObjs, 0 ) ;
         DG_InvisibleObjsChanl( PL_ShadowHairObjs, 1 ) ;
      }
   }
#endif
	if ( work->work2 != NULL ) GV_DestroyOtherActor( work->work2 ) ;
	if ( GM_PlayerBody != NULL && work->st_rai != NULL ) {
		flag = GM_PlayerBody->objs->flag ;
		child = GV_SearchActor( work->st_rai ) ;
		if ( child != NULL && child->name == GV_StrCode( "rai_stealth_work" ) ) {
			GV_DestroyOtherActorQuick( work->st_rai ) ;
		}
		GM_PlayerBody->objs->flag = flag & ~DG_FLAG_OPTCMF ;
	}
//#ifdef STEALTH_HAIR
#if 1 // BP_KP_Cheats
   if( gBP_KP_Cheat_OptCmfMode != kBP_KP_Cheat_OptCmf_Normal )
   {
      if ( PL_ShadowHairObjs != NULL && work->st_hair != NULL ) {
         flag = PL_ShadowHairObjs->flag ;
         child = GV_SearchActor( work->st_hair ) ;
         if ( child != NULL && child->name == GV_StrCode( "rai_hair_stealth_work" ) ) {
            GV_DestroyOtherActorQuick( work->st_hair ) ;
         }
         PL_ShadowHairObjs->flag = flag & ~DG_FLAG_OPTCMF ;
      }
      EvmHairVisible( 1 ) ;
      VisibleBandana( 1 ) ;
      VisibleMagazine( 1 );
   }
#endif
}

/*----------------------------------------------------------------*/

static	void	Act2( Work2 *work )
{
	if ( GM_PlayerBody == NULL ) return ;
	GM_PlayerBody->objs->flag |= *work->root_flag ;
	if ( GM_PlayerArmBody != NULL ) GM_PlayerArmBody->evmobj->flag &= ~DG_EVMOBJ_SEMITRANS ;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

void		*NewStealth( CONTROL *ctrl, OBJECT **body, int **unit, u_int *trigger )
{
	Work	*work ;
	Work2	*work2 ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER2, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_STEALTH_ACTOR_PRIO ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GV_ActorEX( &work->actor ) ;

	work2 = ( Work2 * )GV_CreateActor( GV_ACTOR_PREV2, GV_CLASS_OBJECT,
									   sizeof( Work2 ), PLAYER_STEALTH_ACTOR2_PRIO ) ;
	if ( work2 == NULL ) {
		GV_DestroyActor( work ) ;
		return NULL ;
	}
	GV_SetActor( &work2->actor, Act2, NULL ) ;

	work2->root_flag = &work->root_flag ;
	work2->arm_flag = &work->arm_flag ;
	work->work2 = work2 ;

	GM_SetPlayerStatus( PLAYER_STEALTH ) ;
	GM_ClearCodeFlag |= GM_CLEAR_SPECIAL_ITEM_USED | GM_CLEAR_STEALTH_USED ;

	return work ;
}

