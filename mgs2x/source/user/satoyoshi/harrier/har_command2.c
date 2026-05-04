//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	har_command.c								*/
/*	ハリア  コマンド/プロック						*/
/*	2001/07/09 H.Satoyoshi							*/
/*	$Id: har_command2.c,v 1.1.1.3 2002/11/19 11:48:20 Yoshizawa1 Exp $		*/
/********************************************************************************/


/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#include "harrier.h"

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	NewHar_CommandGetClaster()					*/
/*	引数:	Work	*work							*/
/*	説明:	ミサイル位置取得						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int NewHar_CommandGetWhoAtackPlayer()
{
    GCL_VAR_REF ref ; /* 配列への参照データ */

    GCL_GetNextVarRef( &ref ) ; /* 参照データの取得 */
    if (GM_PlayerDamagedWeaponType&WP_NOPLAYER){
	GCL_SetVarRef( &ref, 0, (int)1 );
    }
    else {
	GCL_SetVarRef( &ref, 0, (int)0 );
    }

    printf ("WT: %x\n",GM_PlayerDamagedWeaponType);

    return 1;
}
