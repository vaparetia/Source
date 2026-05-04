//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	Har_global.c								*/
/*	^$B%O%j%"%a%$%s^(B *NewHarrier						*/
/*	2002/07/08 T.Morita   							*/
/*	$Id: har_global.c,v 1.1.1.3 2002/11/19 11:48:21 Yoshizawa1 Exp $		*/
/********************************************************************************/
#include "harrier.h"

void	*GET_HAR_WORK = NULL ; /* ^$B%O%j%"!<%o!<%/JQ?t^(B */
void	*GET_KAK_WORK = NULL ; /* KASACKA^$B%o!<%/JQ?t^(B */

void	*GET_MPOD_WORK_R = NULL ;
void	*GET_MPOD_WORK_L = NULL ;
void	*GET_MISS_WORK_R = NULL ;
void	*GET_MISS_WORK_L = NULL ;

FVECTOR	KAS_ITEM_DROP_POS;
short	KAS_ITEM_FLAG;
short	KAS_ITEM_NUM;
