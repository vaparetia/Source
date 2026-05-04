//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	プロアクションリプレイ対策で変数領域を移動する

	2001/09/07	K.Uehara
*/

#include <stdio.h>
#include <string.h>

#ifdef PSX2	/// ないので
#include "g_define.h"
#endif

#include "libgcl.h"
#include "linkvar.h"

#define MAX_STEP 1024

extern char gcl_variable_buf[ ( MAX_LINKVARBUF + MAX_VAR_BUF ) * 2 + 1024 ];

extern char *linkvarbuf;
extern char *sv_linkvarbuf;
extern char *var_buf;
extern char *sv_var_buf;

int NewGclVariableMove( void )
{
#ifdef PSX2	/// とりあえず
	char *l, *sl, *v, *sv;
	void *buf;
	int seed;
	char *top;

printf( "-----SWAP VARIABLE !!\n" );
	buf = GV_Malloc( ( MAX_LINKVARBUF + MAX_VAR_BUF ) * 2 );
	if( buf == NULL ) return -1;

	l = buf;
	sl = l + MAX_LINKVARBUF;
	v = sl + MAX_LINKVARBUF;
	sv = v + MAX_VAR_BUF;

	// 退避する
	
	memcpy( l, linkvarbuf, MAX_LINKVARBUF );
	memcpy( sl, sv_linkvarbuf, MAX_LINKVARBUF );
	memcpy( v, var_buf, MAX_VAR_BUF );
	memcpy( sv, sv_var_buf, MAX_VAR_BUF );

	seed = GET_HSYNC_TIMER();
printf( "seed = %d\n", ( seed % ( MAX_STEP / 4 ) ) * 4 );
	top = gcl_variable_buf + ( seed % ( MAX_STEP / 4 ) ) * 4;

	if( seed & 1 ){
		linkvarbuf = top;
		sv_linkvarbuf = linkvarbuf + MAX_LINKVARBUF;
		var_buf = sv_linkvarbuf + MAX_LINKVARBUF;
		sv_var_buf = var_buf + MAX_VAR_BUF;
	} else {
		var_buf = top;
		linkvarbuf = var_buf + MAX_VAR_BUF;
		sv_var_buf = linkvarbuf + MAX_LINKVARBUF;
		sv_linkvarbuf = sv_var_buf + MAX_VAR_BUF;
	}

	// 復帰する

	memcpy( linkvarbuf, l, MAX_LINKVARBUF );
	memcpy( sv_linkvarbuf, sl, MAX_LINKVARBUF );
	memcpy( var_buf, v, MAX_VAR_BUF );
	memcpy( sv_var_buf, sv, MAX_VAR_BUF );

	GV_Free( buf );
printf( "-----SWAP VARIABLE END !!\n" );


	/* 有効アイテム配列をセット */
	if ( GM_Configuration & GM_CONFIG_STORY_TANKER ) {
		GM_SetCurrentItemSet( 0 ) ;
	} else {
		GM_SetCurrentItemSet( 1 ) ;
	}
#endif ///PSX2
	return 0;
}
