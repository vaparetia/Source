//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	radsprite.c
	レーダー表示キャラ 設定
	2000/09/18 K.Sigeno
	$Id: radsprite.c,v 1.1.1.3 2002/11/19 11:49:31 Yoshizawa1 Exp $
*/
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <math.h>

#include	"def_dma.h"
#include	"gameheader.h"

#include "radsprite.h"
#define		ARROW_CNT_X	(8.0F)
static void SetRGBA_Point(SPR_OBJ *obj,char r,char g,char b,char a){
	obj->point.col.r = r ;
	obj->point.col.g = g ;
	obj->point.col.b = b ;
	obj->point.col.a = a ;
}
int SIG_InitRadarSprite(RADAR_CTRL *rctrl) {
	SPR_POS		r_pos,rot_c;
#if 0
	/*矢印廃止*/
	return 0 ;
#endif
	rctrl->face = 
		SPR_Create_2D_Object( SP_BOX, DG_CHANL_MENU, NULL) ;
//		SPR_Create_2D_Object(SP_POINT, DG_CHANL_MENU, NULL) ;
	if(rctrl->face == NULL) return 1;
	/*ブレンディングの各値を設定*/

#if 1
	rctrl->face->head.alpha = SCE_GS_SET_ALPHA(2,2,0,0,0) ;
//	rctrl->face->head.flags |= SPR_FLAG_ALPHA;
	SPR_SetPriority(rctrl->face,7);
#else
	rctrl->face->head.alpha = SCE_GS_SET_ALPHA(2,2,0,0,0) ;
	rctrl->face->head.flags |= SPR_FLAG_ALPHA;
#endif
	SPR_HIDE(rctrl->face);
	return 0 ;
}
void SIG_FreeRadarSprite(RADAR_CTRL *rctrl) {
	/*親を殺せば子も死ぬ*/
#if 0
	printf("RADAR CONTROL SPRITE FREE\n");
	SPR_Destroy_2D_Object(rctrl->sight) ;
	SPR_Destroy_2D_Object(rctrl->face) ;
#else
	SPR_Destroy_2D_Object(rctrl->face) ;
#endif
}
