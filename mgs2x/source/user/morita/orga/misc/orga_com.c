//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_com.c 
   オルガ ハイド生成プットオブジェ

   2000/03/08 T.Morita
   $Id: orga_com.c,v 1.1.1.3 2002/11/19 11:46:25 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

// for DG-LIB
#include "../../include/libdg_x.h"
#include "../../include/util.h"

#include "../include/orga.h"

int NewGetOrgaPos( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( ORG_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)ORG_Work->control.mov.vx ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)ORG_Work->control.mov.vy ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)ORG_Work->control.mov.vz ) ;

    return 1 ;
}

int NewGetOrgaSrcDst( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( ORG_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)ORG_Work->hide_spot    ? ORG_Work->hide_spot->id    : -1 ) ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)ORG_Work->hide_hist[0] ? ORG_Work->hide_hist[0]->id : -1 ) ;

    return 1 ;
}

int NewGetOrgaFlag( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( ORG_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)ORG_Work->flag ) ;

    return 1 ;
}


int NewGetOrgaActFlag( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( ORG_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, (int)ORG_Work->act_flg ) ;

    return 1 ;
}


int NewGetOrgaLife( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */
	int life ;/* 小さいほう */

    if ( ORG_Work == NULL )
	return 0 ;

    if ( GCL_NextStr() == NULL )
	return -1 ;

	if (  ORG_Work->vitality > ORG_Work->vitality_m9 )
	  life = ORG_Work->vitality_m9 ;
	else 
	  life = ORG_Work->vitality ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    GCL_SetVarRef( &ref, 0, life ) ;

    return 1 ;
}


int NewGetHoloStatus( void )
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    if ( ORG_Work == NULL )
	return 0 ;
    if ( GCL_NextStr() == NULL )
	return -1 ;
    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    if ( ORG_Work->misc_holo_stat == NULL )
	GCL_SetVarRef( &ref, 0, (int)-1 ) ;
    else
	GCL_SetVarRef( &ref, 0, (int)*ORG_Work->misc_holo_stat ) ;

    return 1 ;
}
