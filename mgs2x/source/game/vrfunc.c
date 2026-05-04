//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vrfunc.c
	VR関数アドレス

	2002/05/21 Y.Korekado
	$Id: vrfunc.c,v 1.1.1.3 2002/11/19 11:41:57 Yoshizawa1 Exp $
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
#endif

#include	"gameheader.h"
#include	"libgv.h"

/* user/korekado/enemy/campose.c ---------------------------------- */
static void ( *_CPE_SetCamPoseSquare)( DG_PRIM2 *, FMATRIX * ) ;
static void ( *_CPE_DeletCamPoseSquare)( DG_PRIM2 * ) ;

void	SET_VRFUNC_campose( void *func1, void *func2 )
{
	_CPE_SetCamPoseSquare = func1 ;
	_CPE_DeletCamPoseSquare = func2 ;
}

void	VRFUNC_CPE_SetCamPoseSquare( DG_PRIM2 *prim, FMATRIX *world )
{
	if ( _CPE_SetCamPoseSquare == NULL ) return ;
	( *_CPE_SetCamPoseSquare )( prim, world ) ;
}
void	VRFUNC_CPE_DeletCamPoseSquare( DG_PRIM2 *prim )
{
	if ( _CPE_DeletCamPoseSquare == NULL ) return ;
	( *_CPE_DeletCamPoseSquare )( prim ) ;
}
/*user/sigeno/vr*/
static void (*_SIG_EneEquipBreak )(OBJECT * ,int) ;
static void *(*_EnemyAppearEf)(OBJECT *,OBJECT *,int) ;
static void *(*_NewENE_BreakBody)(OBJECT *body,int type,void *ooze) ;

void SET_VR_FUNC_BodyBreak(void *func1,void *func2,void *func3){
	_SIG_EneEquipBreak =func1;
	_EnemyAppearEf = func2;
	_NewENE_BreakBody = func3;
}
void VRFUNC_SIG_EneEquipBreak(OBJECT	*body ,int type){
	if(_SIG_EneEquipBreak == NULL){
		printf("WARNING !!!!PLEASE SET FUNCTION SIG_EneEquipBreak\n");
		return ;
	}
	_SIG_EneEquipBreak(body , type) ;
}
void *VRFUNC_EnemyAppearEf(OBJECT *body,OBJECT *weapon,int type){
	if(_EnemyAppearEf == NULL){
		printf("WARNING !!!!PLEASE SET FUNCTION EnemyAppearEf\n");
		return NULL ;
	}
	return _EnemyAppearEf(body,weapon,type) ;
}
void *VRFUNC_NewENE_BreakBody(OBJECT *body,int type,void *ooze) {
	if(_NewENE_BreakBody == NULL){
		printf("WARNING !!!!PLEASE SET FUNCTION VRFUNC_NewENE_BreakBody\n");
		return NULL ;
	}
	return _NewENE_BreakBody(body,type,ooze) ;
}

/* Init ----------------------------------------------------------- */
void	GM_VRFUNC_Init( void )
{
	_CPE_SetCamPoseSquare = NULL ;
	_CPE_DeletCamPoseSquare = NULL ;

	/*sigeno/vr*/
	_SIG_EneEquipBreak = NULL ;
	_EnemyAppearEf = NULL ;
	_NewENE_BreakBody = NULL ;
}
/* ---------------------------------------------------------------- */
