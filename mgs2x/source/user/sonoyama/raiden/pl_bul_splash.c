//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   pl_bul_splash.c
   弾水飛沫がある場合に登録してもらう

   2001/06/14	M.Sonoyama
   $Id: pl_bul_splash.c,v 1.1.1.3 2002/11/19 11:50:57 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

extern	void	*NewAutoSplushVertical_Hold( FVECTOR *pos, float width, float intense, int flag ) ;
extern	void	*NewBlast3Water( FVECTOR *, int, int, int, int, int, int, int ) ;
extern	void	*SetSplushSequenceBullet( FVECTOR *pos, float width, float intense, int flag );

int		PL_COM_BulletSplashDaemon( void )
{
	PL_BulletSplashFunc = SetSplushSequenceBullet ;
	PL_BlastWaterFunc = NewBlast3Water ;
	return 0 ;
}
