//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fanim.c
	顔アニメキャラ

	2000/06/28 K.Takabe
	$Id: emma_mir.c,v 1.1.1.3 2002/11/19 11:46:03 Yoshizawa1 Exp $

*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

/* ---------------------------------------------------------------- */
	/*
		ワーク構造体定義
	*/
typedef	struct	{
    DG_OBJS    *parent ;			/* ＥＶＭオブジェクト */
    DG_OBJS    *objs ;
    int         num ;
} Work ;


/*

  アクター関数

  */
void EMA_ActMirrorObj( Work *work )
{
    if ( work )
	if ( work->parent )
	{
	    _sceVu0CopyMatrix( &work->objs->world,
			       &work->parent->objs[work->num].world ) ;
	    work->objs->group_id = work->parent->group_id ;
	}
}

/*

  ワークの確保

 */
void *EMA_InitMirrorObj( DG_OBJS *objs, int model, int num )
{
    Work *work ;
    DG_DEF *def ;

    if ( (work = GV_Malloc( sizeof(Work) )) && objs )
    {
	if ( (def = GV_GetCache( GV_CacheID( model, 'k' ) )) )
	{
	    work->objs = DG_MakeObjs( def, 
				      DG_FLAG_SHADE|DG_FLAG_ONEPIECE, 0 ) ;
	    if ( work->objs )
	    {
		DG_QueueObjs( work->objs ) ;
		DG_ConnectObjs( objs, work->objs ) ;
		DG_SetLightMatrix( work->objs, objs->light ) ;
		DG_InvisibleObjs( work->objs ) ;
		work->parent = objs ;
		work->num    = num  ;

		return work ;
	    }
	}
	else
	    printf( "Emma: No Hair Model<5608863>----------\n" ) ;
	GV_Free( work ) ;
    }

    return NULL ;
}

void EMA_FreeMirrorObj( Work *work ) 
{
    if ( work )
    {
	if ( work->objs )
	{
	    DG_DisconnectObjs( work->parent, work->objs ) ;
	    DG_DequeueObjs( work->objs ) ;
	    DG_FreeObjs( work->objs ) ;
	}
	GV_Free( work ) ;
    }
}

