//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_ironbox.c
   フォーチュン戦 鉄箱壊れ

   2000/12/15 T.Morita
   $Id: brk_container.c,v 1.1.1.3 2002/11/19 11:46:13 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

#define BRK_WOOD_GRAVITY 4
#define BRK_IRON_GRAVITY 6

int NewContainerBroken( HIDE *hide, TARGET *trgt, int flag )
{
    hide->type++ ;
    NewIronBoxDust( hide, trgt, 8 ) ;
    if ( FRT_OBJ_InitModel( hide, &hide->objs->world ) < 0 )
	PERROR( "Couldn't initialize New Model(%d) : NewIronBoxBroken\n", --hide->type ) ;

    switch( FRT_OBJ_GetTypeMasked( hide ) )
    {
    case FRT_TYP_IRONBOX2M_G:
    case FRT_TYP_IRONBOX2M_H:
    case FRT_TYP_IRONBOX2M_I:
	NewAnythingDust( hide, trgt,  8, 14343021,/* w11c2_woodbox_frg1_cm*/
			 BRK_WOOD_GRAVITY, 50.0f, -1,    NULL, 0 ) ;

	NewFortBlast( (FVECTOR *)hide->objs->world.m[W], 
		      PLAYER_SIDE, 2500.0f, 2000.0f, 25, 10, WP_Kick, 2 ) ;

    case FRT_TYP_IRONBOX2M_D:
    case FRT_TYP_IRONBOX2M_E:
    case FRT_TYP_IRONBOX2M_F:
	NewAnythingDust( hide, trgt, 12, 14408557,/* w11c2_woodbox_frg3_cm*/
			 BRK_WOOD_GRAVITY, 50.0f, -1,    NULL, 0 ) ;
	NewAnythingDustScale( hide, trgt, 8,
			      6348906, /* w11c2_ironbox_frg1_cm*/
			      BRK_IRON_GRAVITY,
			      100.0f, 3.0f,
			      NULL, 0 ) ;

    case FRT_TYP_IRONBOX2M_A:
    case FRT_TYP_IRONBOX2M_B:
    case FRT_TYP_IRONBOX2M_C:
	GM_SeSetMode( SD_A_METLANA1, (FVECTOR *)hide->objs->world.m[W],
		      GM_SEMODE_BOMB ) ;
    }

    return 0 ;
}
