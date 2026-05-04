//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	namepri.c
	ドッグタグ名表示

	2001/07/05 Y.Korekado
	$Id: namepri.c,v 1.1.1.3 2002/11/19 11:44:28 Yoshizawa1 Exp $
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include "libutl.h"
#endif

#include	"gameheader.h"
#include	"def_dma.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/conv/define.h"
#include	"camera.h"

UCVECTOR  PrintNameColor[4] = {
    { 192,0,0,255},{ 64,64,255,255},{64,128,255,255},{255,255,255,255}
} ;

typedef struct {
    GV_ACT_EX	actor ;
 	OBJECT		*body ;
    FVECTOR		ret ;
	char		*str ;
} Work ;

static  void  PosToRet(Work *work)
{
	static FVECTOR	shift = { 200.0f, 300.0f, 0.0f } ;
    DG_CHANL  *cp ;
    FVECTOR   pos ;   
	GM_CameraSet	*cam ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/

	KR_FMatToFvec( &BODYWORLD( work->body, HUMAN21_ATAMA), &pos ) ;
	DG_SetPos2( &pos, &cam->rotate ) ;
	DG_PutVector( &shift, &pos, 1 ) ;	/* 絶対座標へ変換 */

    /* 位置決定 */
    cp = DG_Chanl(0) ;
	pos.vw = 1.0F ;
    _sceVu0ApplyMatrix(&work->ret, &cp->eye_pers, &pos) ;
}

static  int  InScreen( FVECTOR *ret )
{
    /* カメラの範囲内に入っているかどうか */
    if( ret->vz > ret->vw )	return 0 ;

    if( ret->vw < 0 )	ret->vw = - ret->vw ;

    if( (ret->vx > ret->vw) || (ret->vx < (-ret->vw)) )	return 0 ;

    if( (ret->vy > ret->vw) || (ret->vy < (-ret->vw)) )	return 0 ;

	return 1 ;
}

static	int	GetAlpha( )
{
	int value ;
	static int power=0 ;

#if 0
	if( ((PL_GetPlayerWeapon( ) == IT_Scope)||(PL_GetPlayerWeapon( ) == IT_TnkCamera))
		&&
		(GV_PadData[ 0 ].pressure[ PAD_PRESS_R1 ] > 10) ){
#else
	if ((GV_PadData[ 0 ].pressure[ PAD_PRESS_R1 ] > 10)){
#endif
		power ++ ;
	} else {
		power -= 8 ;
	}

	value = 1 ;
	if ( power > 60 ) {
		if ( power > 128*2 + 60  ) power = 128*2 + 60  ;
		value = ((power-60) / 2)  + 1;
	} else {
		if ( power < 0 ) power = 0 ;
		value = 1 ;
	}

	return value ;
}

static  void  PrintScreen(Work *work)
{
    int  x, y, col, alp ;
	FVECTOR		*ret ;

	ret = &work->ret ;

    x = (int)((ret->vx / ret->vw) * (float)(DRAW_WIDTH>>1)) + (DRAW_WIDTH>>1) ;
    y = (int)((ret->vy / ret->vw) * (float)(DRAW_HEIGHT>>1)) + (DRAW_HEIGHT>>1) ;

	col = 2 ;
	alp = GetAlpha( ) ;
    DEBUG_Locate(x,y,0) ;
    DEBUG_Color( PrintNameColor[col].r, PrintNameColor[col].g, 
    		PrintNameColor[col].b, alp ) ;
    DEBUG_Printf("%s\n",work->str) ;
	DEBUG_ResetColor();
}

static  void  Act(Work *work)
{
	PosToRet( work ) ;

	if ( InScreen( &work->ret ) ) {
		PrintScreen( work ) ;
	}
}

static  void  Die(Work *work)
{
    
}

static  int  GetResources(Work  *work, OBJECT *body, char *str )
{
	work->body = body ;
	work->str = str ;

    return  0 ;
}

void *NewNamePri( OBJECT *body, char *str )
{
    Work *work ;

    work = (Work *)GV_NewEffect(GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if( work != NULL){
		GV_SetActor( &(work->actor), Act,Die) ;
		GV_ActorEX( &work->actor )
		if( GetResources( work, body, str ) < 0){
		    GV_DestroyActor( work ) ;
		    return NULL ;
		}
	}
	return (void *)work ;
}
