//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_woodbox.c
   フォーチュン戦 木箱壊れ

   2000/12/15 T.Morita
   $Id: brk_woodbox.c,v 1.1.1.3 2002/11/19 11:46:15 Yoshizawa1 Exp $
*/
#include "fort_obj.h"

#define BRK_WOOD_GRAVITY      4
int NewWoodenBoxBroken( HIDE *hide, TARGET *trgt, int flag )
{
    static int WoodSound[] = { SD_A_WOODOTI1, SD_A_WOODOTI2 } ;
    static int n_WoodSound = sizeof(WoodSound) / sizeof(int) ;
    

    /* 破壊音 */
    GM_SeSetMode( SD_A_WOODBRE1, (FVECTOR *)hide->objs->world.m[W], GM_SEMODE_BOMB ) ;

    NewAnythingDust( hide, trgt, 30, 14408557,/* w11c2_woodbox_frg3_cm*/
		     BRK_WOOD_GRAVITY, 50.0f, 60*10, WoodSound, n_WoodSound ) ;
    NewAnythingDust( hide, trgt,  6, 14408557,/* w11c2_woodbox_frg3_cm*/
		     BRK_WOOD_GRAVITY, 50.0f, -1,    NULL, 0 ) ;
    NewAnythingDust( hide, trgt, 16, 14343021,/* w11c2_woodbox_frg1_cm*/
		     BRK_WOOD_GRAVITY, 50.0f, 60*10, NULL, 0 ) ;
    NewAnythingDust( hide, trgt, 16, 14343021,/* w11c2_woodbox_frg1_cm*/
		     BRK_WOOD_GRAVITY, 50.0f, -1,    NULL, 0 ) ;

    NewFortBlast( (FVECTOR *)hide->objs->world.m[W], 
		  PLAYER_SIDE, 2000.0f, 1500.0f, 5, 10, WP_Kick, 4 ) ;

    FRT_OBJ_FreeObject( hide, 1 ) ;
    hide->type++ ;

    return 1 ;
}


int BRK_FRT_OBJ_BreakInitWoodenBox( HIDE *hide, FVECTOR *pos )
{
    NewAnythingDustScaleInit( hide, pos,
			      6 , 14408557,/* w11c2_woodbox_frg3_cm*/
			      1.0f ) ;
    NewAnythingDustScaleInit( hide, pos, 
			      16, 14343021,/* w11c2_woodbox_frg1_cm*/
			      1.0f ) ;
	return 0 ;
}
