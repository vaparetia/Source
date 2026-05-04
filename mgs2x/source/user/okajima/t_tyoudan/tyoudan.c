//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tyoudan.c
	跳弾

	1999/07/27 H.Tanaka
	version. 1999/09/07
	2000/10/18 S.Okajima
	$Id: tyoudan.c,v 1.1.1.3 2002/11/19 11:47:47 Yoshizawa1 Exp $
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


#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"libgcl.h"
#include	"gameheader.h"

#include	"def_dma.h"


#define  N_PRIMS		24
#define  N_PACKETS	   1
#define	 SPARK_PERIOD	15


extern int NewVRSpark(FMATRIX *world);
extern void *NewSpark2(int n_packets, FMATRIX *matrix, float min_speed, 
				float speed_wide, float gravity,SVECTOR *rot,SVECTOR *rot_wide,
				FVECTOR *color,float length, int count) ;

void *NewSpark( world )  
FMATRIX   *world ;
{
	SVECTOR   rot ;
	SVECTOR   rot_wide ;
	FVECTOR   color ;

	rot.vx = 768 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	rot_wide.vx = 512 ;
	rot_wide.vy = 4096 ;
	rot_wide.vz = 0 ;
	color.vx = 255.0F ;
	color.vy = 128.0F ;
	color.vz = 128.0F ;
	color.vw = 50.0F ;

	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/19 */
	{
		NewVRSpark(world);
		rot_wide.vx = 1024;
		return NewSpark2(N_PRIMS*0.5,world,20.0F,32.0F,0.0F,&rot,&rot_wide,&color,0.4F,SPARK_PERIOD+3) ;
	}
	return NewSpark2(N_PRIMS,world,1.0F,32.0F,0.0F,&rot,&rot_wide,&color,1.0F,SPARK_PERIOD) ;
}

void *NewSpark_White(world)
FMATRIX *world ;
{
	SVECTOR   rot ;
	SVECTOR   rot_wide ;
	FVECTOR   color ;

	rot.vx = 768 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	rot_wide.vx = 512 ;
	rot_wide.vy = 4096 ;
	rot_wide.vz = 0 ;
	color.vx = 255.0F ;
	color.vy = 255.0F ;
	color.vz = 255.0F ;
	color.vw = 50.0F ;

	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/19 */
	{
		NewVRSpark(world);
		rot_wide.vx = 1024;
		return NewSpark2(N_PRIMS*0.5,world,20.0F,32.0F,0.0F,&rot,&rot_wide,&color,0.4F,SPARK_PERIOD+3) ;
	}
	return NewSpark2(N_PRIMS,world,1.0F,32.0F,0.0F,&rot,&rot_wide,&color,1.0F,SPARK_PERIOD) ;
}

void *NewSpark_Demo(
	FMATRIX	  *d_world,
	FVECTOR	  *d_color
)
{
	SVECTOR   rot ;
	SVECTOR   rot_wide ;
	FVECTOR   color ;

	rot.vx = 768 ;
	rot.vy = 0 ;
	rot.vz = 0 ;
	rot_wide.vx = 4096 ;
	rot_wide.vy = 4096 ;
	rot_wide.vz = 0 ;
	color.vx = d_color->vx ;
	color.vy = d_color->vy ;
	color.vz = d_color->vz ;
	color.vw = 50.0F ;

	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/19 */
	{
		NewVRSpark(d_world);
		rot_wide.vx = 1024;
		return NewSpark2(N_PRIMS*0.5, d_world, 20.0F, 32.0F, 0.0F, &rot, &rot_wide, &color, 0.4F, SPARK_PERIOD+3) ;
	}
	return NewSpark2(N_PRIMS, d_world, 1.0F, 32.0F, 0.0F, &rot, &rot_wide, &color, 1.0F, SPARK_PERIOD ) ;
}

