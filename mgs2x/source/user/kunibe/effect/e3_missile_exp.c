//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    e3_missile_exp.c
    E3用アムラーム着弾
    2001/04/21 Yuuta Kunibe	
    $Id: e3_missile_exp.c,v 1.1.1.3 2002/11/19 11:44:38 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"


extern void *NewRunningSmoke( FVECTOR*, FVECTOR*, FVECTOR* );
extern void *NewExplosionSmoke( FVECTOR* );
extern void *NewDebris_Cm( FVECTOR *bound, FVECTOR *force,
					int num, int objcode,
					int color, float scale, int flags );

extern void *NewDebris_Cm_Demo( FVECTOR *bound0, FVECTOR *bound1, FVECTOR *force,
						 float pow, int num, int objcode,
						 int color, float scale, int flags);


void NewHarrierMisileExplosionE3( FVECTOR *point, FVECTOR *normal, FVECTOR *direction ) {

    FVECTOR 	bound[2];
    FVECTOR 	force;
    int		num;
    int 	objcode;
    int 	color;
    float	scale;
    int 	flags;
    FVECTOR 	vectmp;

    /* 爆発 */
    DG_COPY_VEC( &vectmp, point );
    vectmp.vx += frnd()*2000.f;
    vectmp.vz += frnd()*2000.f;
    NewExplosionSmoke( &vectmp );
    NewRunningSmoke( &vectmp, normal, direction );
    
    DG_COPY_VEC( &vectmp, point );
    vectmp.vx += frnd()*2000.f;
    vectmp.vy += 1000.f + frnd()*500.f;
    vectmp.vz += frnd()*2000.f;
    NewExplosionSmoke( &vectmp );
    
    DG_COPY_VEC( &vectmp, point );
    vectmp.vx += frnd()*2000.f;
    vectmp.vy += 2000.f + frnd()*500.f;
    vectmp.vz += frnd()*2000.f;
    NewExplosionSmoke( &vectmp );
    
    DG_COPY_VEC( &vectmp, point );
    vectmp.vx += frnd()*2000.f;
    vectmp.vy += 3000.f + frnd()*500.f;
    vectmp.vz += frnd()*2000.f;
    NewRunningSmoke( &vectmp, normal, direction );
    NewExplosionSmoke( &vectmp );


    /* 破片 */
    DG_COPY_VEC( &bound[0], point );
    bound[0].vx -= 5000.f;
    bound[0].vz -= 5000.f;
    
    DG_COPY_VEC( &bound[1], point );
    bound[1].vx += 5000.f;
    bound[1].vy += 6000.f;
    bound[1].vz += 5000.f;
    
    force.vx = 0.f;
    force.vy = 1.f;
    force.vz = 0.f;
    force.vw = 250.f;
    
    num = 100;
    
    objcode = GV_StrCode("cgr_frg1_cm");
    
    color = ( (128<<24)&(128<<16)&(128<<8) );
    
    scale = 3.0f;
    
    flags = 0;
    
    NewDebris_Cm( bound, &force, num, objcode, color, scale, flags );
    
}
