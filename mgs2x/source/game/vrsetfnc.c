//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	vrsetfnc.c
	セットVR関数

	2002/05/21 Y.Korekado
	$Id: vrsetfnc.c,v 1.1.1.3 2002/11/19 11:41:57 Yoshizawa1 Exp $
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
extern void	SET_VRFUNC_campose( void *func1, void *func2 ) ;
extern void	CPE_DeletCamPoseSquare( DG_PRIM2 *prim ) ;
extern void	CPE_SetCamPoseSquare( DG_PRIM2 *prim, FMATRIX *world ) ;

/* user/sigeno/vr/---------------------------------- */
extern void SET_VR_FUNC_BodyBreak(void *func1,void *func2,void *func3) ;
extern void SIG_EneEquipBreak(OBJECT	*body ,int type) ;
extern void *EnemyAppearEf(OBJECT *body,OBJECT *weapon,int type) ;
extern void *NewENE_BreakBody(OBJECT *body,int type,void *ooze) ;

/*************************************************************/

void	VRFUNC_Set( void )
{
	SET_VRFUNC_campose( CPE_SetCamPoseSquare, CPE_DeletCamPoseSquare ) ;
	SET_VR_FUNC_BodyBreak(SIG_EneEquipBreak,EnemyAppearEf,NewENE_BreakBody) ;
}
