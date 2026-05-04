//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scnskirt.c
	スカートEVMをシナリオ呼び
	2002/08/27 K.Sigeno
	$Id: scnskirt.c,v 1.2 2002/11/25 01:54:45 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"


extern void *NewEvm_SkirtB( OBJECT * ); // おばはん

static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}

	return ( NULL );
}
void ScnSkirtCall( void ){
	CONTROL *ctrl ;
	OBJECT *body ;
	if ( GCL_GetOption( 'n' ) != NULL ){
		ctrl = SearchControl( GCL_GetNextInt() ) ;
		if(ctrl == NULL ){
			return ;
		}
		body = ctrl->object ;
		if(body == NULL ){
			return ;
		}
	}else {
		return ;
	}
	NewEvm_SkirtB( body ); 
}
