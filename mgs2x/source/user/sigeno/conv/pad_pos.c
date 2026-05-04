//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pad_pos.c
	デバッグ用 パッドで座標セット
	2001/06/26 K.Sigeno
	$Id: pad_pos.c,v 1.1.1.3 2002/11/19 11:49:07 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "gameheader.h"
#include "libutl.h"

void SIG_PadPos(int pad,int mode,float *x,float *y,float def_sp,float rate){
	float speed ;
	if(GV_PadData[ pad ].status & PAD_R1){
		speed =def_sp*rate ;
	}else {
		speed =def_sp ;
	}
	if (mode == 0){
		/*十字キー*/
		if(GV_PadData[ pad ].status & PAD_U){
			*y -= speed ; 
		}
		if(GV_PadData[ pad ].status & PAD_D){
			*y += speed ; 
		}
		if(GV_PadData[ pad ].status & PAD_L){
			*x -= speed ; 
		}
		if(GV_PadData[ pad ].status & PAD_R){
			*x += speed ; 
		}
	}else if(mode ==1){
		if(GV_PadData[ pad ].status & PAD_B){
			*y += speed ; 
		}
		if(GV_PadData[ pad ].status & PAD_X){
			*y -= speed ; 
		}
		if(GV_PadData[ pad ].status & PAD_A){
			*x += speed ; 
		}
		if(GV_PadData[ pad ].status & PAD_Y){
			*x -= speed ; 
		}
	}
}
