//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   utl_vib.c
   
   2000/01/21 T.Morita
   $Id: utl_vib.c,v 1.1.1.3 2002/11/19 11:43:00 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifndef KP_XBOX
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

/*

  ブルブルセンサー検知

  生体センサーをプレーヤが装備した時に距離に合わせて振動させます。単体
  専用なのでボス戦やイベントのみの利用になります。

  ワークにvib_timeのint変数を用意して頂き,コントロールと変数のポインタ
  を渡し,毎フレーム呼び出して頂ければうまく動作します。

*/
void GM_VibrateSensor( CONTROL *ctrl, int *vib_time )
{
    int   i   ;
    float dis ;
    static u_char  DamageVib1L[] = { 176, 4, 0, 0 } ; /* by nakamura */
    extern void *NewPadVibration( char *scr, int type ) ;

    ASSERT( vib_time && ctrl ) ;

    if( GM_Item == IT_VibSensor )
    {
	dis = GV_VecLen3F2( &GM_PlayerControl->mov, &ctrl->mov ) ;

	if ( dis < 10000 )
	{
	    i = 16 + (dis > 3600 ? (dis - 3600)/200 : 0) ;

	    if ( fpu_Abs(GM_PlayTime - *vib_time) > i )
	    {
		NewPadVibration( DamageVib1L, 2|VAR_FLAG_FORCE ) ;
		*vib_time = GM_PlayTime ;
	    }
	}
    }
}
