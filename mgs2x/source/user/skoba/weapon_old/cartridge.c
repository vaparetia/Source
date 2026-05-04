//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	武器のカートリッジ
	2000/01/23   H.TANAKA
	2000/10/18 S.Okajima
	2001/03/29 S.Kobayashi
	$Id: cartridge.c,v 1.1.1.3 2002/11/19 11:50:32 Yoshizawa1 Exp $
*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"big_weapon.h"

#include "libutl.h"
#include	"def_dma.h"
#include	"../../okajima/etc/ok_util.h"



#define USP_SHIFT_X	(17.5f)
#define USP_SHIFT_Y	(-130.0f)
#define USP_SHIFT_Z	(70.0f)

#define SOCOM_SHIFT_X	(17.5f)
#define SOCOM_SHIFT_Y	(-102.0f)
#define SOCOM_SHIFT_Z	(78.7f)

#define MKR_SHIFT_X	(20.0f)
#define MKR_SHIFT_Y	(-123.0f)
#define MKR_SHIFT_Z	(54.5f)

#define FMS_SHIFT_X	(20.0f)
#define FMS_SHIFT_Y	(33.0f)
#define FMS_SHIFT_Z	(102.0f)

#define AKS_SHIFT_X	(20.0f)
#define AKS_SHIFT_Y	(-242.0f)
#define AKS_SHIFT_Z	(85.0f)

#define SPS_SHIFT_X	(20.5f)
#define SPS_SHIFT_Y	(-242.0f)
#define SPS_SHIFT_Z	(85.0f)

#define P90_SHIFT_X	(17.5f)
#define P90_SHIFT_Y	(40.f)
#define P90_SHIFT_Z	(-20.0f)

#define P90_SOL_SHIFT_X	(17.0f)
#define P90_SOL_SHIFT_Y	(20.f)
#define P90_SOL_SHIFT_Z	(-35.0f)

#define GLK_SHIFT_X	(-3.0f)
#define GLK_SHIFT_Y	(-137.f)
#define GLK_SHIFT_Z	(71.0f)

#define M4A1_SHIFT_X (4.0f)
#define M4A1_SHIFT_Y (-205.0f)
#define M4A1_SHIFT_Z (92.0f)

#define M4A1_SNK_SHIFT_X (20.0f)
#define M4A1_SNK_SHIFT_Y (-225.0f)
#define M4A1_SNK_SHIFT_Z (82.0f)

#define M4A1_HI_SHIFT_X (20.0f)
#define M4A1_HI_SHIFT_Y (-227.0f)
#define M4A1_HI_SHIFT_Z (80.0f)

#define M4_SHIFT_X	(20.0f)
#define M4_SHIFT_Y	(-204.f)
#define M4_SHIFT_Z	(44.0f)

#define M4_SEL_SHIFT_X	(5.0f)
#define M4_SEL_SHIFT_Y	(-344.f)
#define M4_SEL_SHIFT_Z	(24.0f)

#define M4_DEMO_GUN_SHIFT_X (4.0f)
#define M4_DEMO_GUN_SHIFT_Y (-205.0f)
#define M4_DEMO_GUN_SHIFT_Z (92.0f)

#define ABK_SHIFT_X	(0.0f)
#define ABK_SHIFT_Y	(-241.0f)
#define ABK_SHIFT_Z	(77.0f)

#define AKS_RAI_SHIFT_X	(5.0f)
#define AKS_RAI_SHIFT_Y	(-227.0f)
#define AKS_RAI_SHIFT_Z	(89.0f)

#define SAA_RIGHT_SHIFT_X	(22.0f)
#define SAA_RIGHT_SHIFT_Y	(-144.0f)
#define SAA_RIGHT_SHIFT_Z	(56.0f)

#define SAA_LEFT_SHIFT_X	(-22.0f)
#define SAA_LEFT_SHIFT_Y	(-144.0f)
#define SAA_LEFT_SHIFT_Z	(56.0f)

#define M92_SEA_SHIFT_X	(8.0f)
#define M92_SEA_SHIFT_Y	(-119.0f)
#define M92_SEA_SHIFT_Z	(73.0f)

#define M4_DEMO_GUN_GRENADE_SHIFT_X (5.0f)
#define M4_DEMO_GUN_GRENADE_SHIFT_Y (-86.0f)
#define M4_DEMO_GUN_GRENADE_SHIFT_Z (99.0f)

#define M4_DEMO_GUN_SEL_SHIFT_X (-10.0f)
#define M4_DEMO_GUN_SEL_SHIFT_Y (-226.0f)
#define M4_DEMO_GUN_SEL_SHIFT_Z (79.0f)

#define M4_SNK_GRENADE_SHIFT_X (20.0f)
#define M4_SNK_GRENADE_SHIFT_Y (-347.0f)
#define M4_SNK_GRENADE_SHIFT_Z (27.0f)

#define M4_HI_GRENADE_SHIFT_X (20.0f)
#define M4_HI_GRENADE_SHIFT_Y (-347.0f)
#define M4_HI_GRENADE_SHIFT_Z (24.0f)

#define PSG_SHIFT_X (17.5f)
#define PSG_SHIFT_Y (-267.0f)
#define PSG_SHIFT_Z (89.7f)

extern void *MdlCartridg_FMS(
	FVECTOR  *pos,							   
	FVECTOR  *speed,			  /* スピード  */
	SVECTOR  *rot,
	OBJECT   *weapon,
	CONTROL  *control			  /* 人体コントロール  */
);

extern void TS_VecToRot( SVECTOR *rot, FVECTOR *vec );

/* ----------------------------- */
/* M92  */
void   CartridgeM92_Sea(FMATRIX *world, OBJECT *weapon, CONTROL *control)
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR	pos;
	FVECTOR	speed;
	SVECTOR	rot;

	shift.vx = M92_SEA_SHIFT_X;
	shift.vy = M92_SEA_SHIFT_Y;
	shift.vz = M92_SEA_SHIFT_Z;

	shift.vw = 1.0F ;
	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	shift.vx = -15.0f + rnd() * 20.0f;
	shift.vy = 5.0f + rnd() * 15.0f;
	shift.vz = 20.0f + rnd() * 15.0f;
	DG_RotVector(&shift,&speed,1) ;

	TS_VecToRot( &rot, (FVECTOR*)world->m[2] );
	rot.vx += irnd()%128 - 64;
	rot.vy += irnd()%128 - 64;
	rot.vz = irnd()%4096;

	MdlCartridg_USP(&pos,&speed,&rot,weapon,control) ;

	shift.vx = 17.5F ;
	shift.vy = -130.0F ;
	shift.vz = 60.5F ;	
	DG_SetPos( world ); 
	DG_PutVector( &shift , &pos , 1 );
	DG_MovePos( &shift );
	shift.vx = -3.0f - rnd() * 3.0f;
	shift.vy = 0.0f;
	shift.vz = rnd() * 5.0f;
	DG_RotVector( &shift , &speed , 1 );
	NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
//	NewGunSmoke(&pos,&speed,10.0F,1) ; 
}

void  AN_CartridgeM92_Sea_E(FMATRIX *world,OBJECT   *weapon,CONTROL  *control)
{
	CartridgeM92_Sea(world, weapon, control) ;
}

/* ----------------------------- */
/* USP */
void   CartridgeUSP(FMATRIX *world, OBJECT *weapon, CONTROL *control)
{
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

#if 0
	shift.vx = 13.1F ;
	shift.vy = -180.0F ;
	shift.vz = 50.0F ;
#else
	shift.vx = USP_SHIFT_X;
	shift.vy = USP_SHIFT_Y;
	shift.vz = USP_SHIFT_Z;
#endif
	shift.vw = 1.0F ;
	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	pos.vw = 1.0F;
	
	shift.vx = -20.0f + rnd() * 10.0f;
	shift.vy = 30.0f + rnd() * 10.0f;
	shift.vz = 20.0f + rnd() * 10.0f;
	DG_RotVector(&shift,&speed,1) ;
/*
	rot.vx = (short)(rnd() * 200.0F) ;
	rot.vy = (short)(rnd() * 200.0F) ;
	rot.vz = 384 + (short)(rnd() * 256) ;
*/
	//rot.vx = irnd()%128 ; 
	//rot.vy = irnd()%128 ; 
	//rot.vz = 128 + irnd()%128 ; 

	TS_VecToRot( &rot, (FVECTOR*)world->m[2] );
	rot.vx += irnd()%128 - 64;
	rot.vy += irnd()%128 - 64;
	rot.vz = irnd()%4096;

	MdlCartridg_USP(&pos,&speed,&rot,weapon,control) ;

	shift.vx = 17.5F ;
	shift.vy = -130.0F ;
	shift.vz = 60.5F ;	
	DG_SetPos( world ); 
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	shift.vx = -3.0f - rnd() * 3.0f ;
	shift.vy = 0.0f;
	shift.vz = rnd() * 5.0f;
	DG_RotVector(&shift,&speed,1) ;
	NewGunSmoke(&pos,&speed,10.0F,1) ;
}

void  AN_CartridgeUSP_E(FMATRIX *world,OBJECT   *weapon,CONTROL  *control)
{
	CartridgeUSP(world, weapon, control) ;
}

void  AN_CartridgeUSP(FMATRIX *world,OBJECT   *weapon,CONTROL  *control)
{
	CartridgeUSP(world, weapon, control) ;
}

/* Socom */
void   CartridgeSOCOM(FMATRIX *world, OBJECT *weapon, CONTROL *control)
{
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

	shift.vx = SOCOM_SHIFT_X;
	shift.vy = SOCOM_SHIFT_Y;
	shift.vz = SOCOM_SHIFT_Z;

	shift.vw = 1.0F ;
	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	pos.vw = 1.0F;
	
	shift.vx = ( rnd() * -20.0f - 15.0f );
	shift.vy = 5.0f + rnd() * 15.0f;
	shift.vz = 20.0f + rnd() * 15.0f;
	DG_RotVector(&shift,&speed,1) ;

	TS_VecToRot( &rot, (FVECTOR*)world->m[2] );
	rot.vx += irnd()%128 - 64;
	rot.vy += irnd()%128 - 64;
	rot.vz = irnd()%4096;

	MdlCartridg_SOCOM(&pos,&speed,&rot,weapon,control) ;

	shift.vx = 17.5F ;
	shift.vy = -130.0F ;
	shift.vz = 60.5F ;	
	DG_SetPos( world ); 
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	shift.vx = -3.0f - rnd() * 3.0f ;
	shift.vy = 0.0f;
	shift.vz = rnd() * 5.0f;
	DG_RotVector(&shift,&speed,1) ;
	NewGunSmoke(&pos,&speed,10.0F,1) ;
}

void  AN_CartridgeSOCOM_E(FMATRIX *world,OBJECT   *weapon,CONTROL  *control)
{
	CartridgeSOCOM(world, weapon, control) ;
}

/* mkr */
void   CartridgeMKR(FMATRIX *world, OBJECT   *weapon, CONTROL  *control)
{
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;


	shift.vx = MKR_SHIFT_X ;
	shift.vy = MKR_SHIFT_Y ;
	shift.vz = MKR_SHIFT_Z ;

	shift.vw = 1.0F ;
	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	pos.vw = 1.0F;
	
	shift.vx = -20.0f + rnd() * 10.0f;
	shift.vy = 30.0f + rnd() * 10.0f;
	shift.vz = 20.0f + rnd() * 10.0f;
	DG_RotVector(&shift,&speed,1) ;
/*
	rot.vx = (short)(rnd() * 200.0F) ;
	rot.vy = (short)(rnd() * 200.0F) ;
	rot.vz = 384 + (short)(rnd() * 256) ;
*/
	rot.vx = irnd()%128 ;
	rot.vy = irnd()%128 ;
	rot.vz = 128 + irnd()%128 ;
	MdlCartridg_USP(&pos,&speed,&rot,weapon,control) ;
 
	shift.vx = 20.0F ;
	shift.vy = -123.0F ;
	shift.vz = 54.5F ;	
	DG_SetPos( world ); 
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	shift.vx = -3.0f - rnd() * 3.0F ;
	shift.vy = 0.0f;
	shift.vz = rnd() * 5.0F;
	DG_RotVector(&shift,&speed,1) ;
	NewGunSmoke(&pos,&speed,10.0F,1) ;	
}

void AN_CartridgeMKR_E(FMATRIX *world,OBJECT   *weapon,CONTROL  *control)
{
	CartridgeMKR(world, weapon, control) ;
}

/* fms */
void CartridgeFMS(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

#if 0
	shift.vx = 13.1F ;
	shift.vy = -180.0F ;
	shift.vz = 50.0F ;
#else
	shift.vx = FMS_SHIFT_X ;
	shift.vy = FMS_SHIFT_Y ;
	shift.vz = FMS_SHIFT_Z ;
#endif
	shift.vw = 1.0F ;
	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	pos.vw = 1.0F;
	
	shift.vx = -20.0f + rnd() * 10.0f;
	shift.vy = 30.0f + rnd() * 10.0f;
	shift.vz = 20.0f + rnd() * 10.0f;
	DG_RotVector(&shift,&speed,1) ;
/*
	rot.vx = (short)(rnd() * 200.0F) ;
	rot.vy = (short)(rnd() * 200.0F) ;
	rot.vz = 384 + (short)(rnd() * 256) ;
*/
	TS_VecToRot( &rot, (FVECTOR*)world->m[2] );
	rot.vx += irnd()%128 - 64;
	rot.vy += irnd()%128 - 64;
	rot.vz = irnd()%4096;
//	MdlCartridg_AKS(&pos,&speed,&rot,weapon,control) ; 
//	MdlCartridg_USP(&pos,&speed,&rot,weapon,control) ; 
	MdlCartridg_FMS(&pos,&speed,&rot,weapon,control) ;
	shift.vx = 17.5F ;
	shift.vy = -130.0F ;
	shift.vz = 60.5F ;
	DG_SetPos( world ); 
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	shift.vx = -3.0f - rnd() * 3.0f ;
	shift.vy = 0.0f;
	shift.vz = rnd() * 5.0f;
	DG_RotVector(&shift,&speed,1) ;
	NewGunSmoke(&pos,&speed,10.0F,1) ;

}

void AN_CartridgeFMS_E(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	CartridgeFMS(world, weapon, control) ;
}

/* aks */
void	CartridgeAKS(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	FMATRIX fmtmp;
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;


	shift.vx = AKS_SHIFT_X ;
	shift.vy = AKS_SHIFT_Y ;
	shift.vz = AKS_SHIFT_Z ;
	shift.vw = 1.0F ;
	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	shift.vx = -40.0f + rnd() * 8.0f;
	shift.vy = 20.0f + rnd() * 10.0f;
	shift.vz = 15.0f + rnd() * 8.0f;
	DG_RotVector(&shift,&speed,1) ;
/*
	rot.vx = (short)(rnd() * 200.0F) ;
	rot.vy = (short)(rnd() * 200.0F) ;
	rot.vz = 384 + (short)(rnd() * 256) ;
*/
//	rot.vx = irnd()%128 ; 
//	rot.vy = irnd()%128 ; 
//	rot.vz = 128 + irnd()%128 ; 
	
	TS_VecToRot( &rot, (FVECTOR*)world->m[2] );
	rot.vx += irnd()%128 - 64;
	rot.vy += irnd()%128 - 64;
	rot.vz = irnd()%4096;
	MdlCartridg_AKS(&pos,&speed,&rot,weapon,control) ;
 
	shift.vx = 20.0F ;
	shift.vy = -242.0F ;
	shift.vz = 85.0F ;	
	DG_SetPos( world ); 
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	shift.vx = -3.0f - rnd() * 3.0F ;
	shift.vy = 0.0f;
	shift.vz = rnd() * 5.0F;
	DG_RotVector(&shift,&speed,1) ;
	if ( irnd() % 10 < 4 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}

void	 AN_CartridgeAKS_E(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	CartridgeAKS(world, weapon, control) ;
}



/* sps  */
void	CartridgeSPS(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	FMATRIX fmtmp;
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;


	shift.vx = 20.0F ;
	shift.vy = -242.0F ;
	shift.vz = 85.0F ;
	shift.vw = 1.0F ;
	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	pos.vw = 1.0F;

	shift.vx = -40.0f + rnd() * 8.0f;
	shift.vy = 20.0f  + rnd() * 10.0f;
	shift.vz = 15.0f  + rnd() * 8.0f;
	DG_RotVector(&shift,&speed,1) ;
/*
	rot.vx = (short)(rnd() * 200.0F) ;
	rot.vy = (short)(rnd() * 200.0F) ;
	rot.vz = 384 + (short)(rnd() * 256) ; 
*/
	rot.vx = irnd()%128 ;
	rot.vy = irnd()%128 ;
	rot.vz = 128 + irnd()%128 ;
	MdlCartridg_SPS(&pos,&speed,&rot,weapon,control) ;

	shift.vx = 20.0F ;
	shift.vy = -242.0F ;
	shift.vz = 85.0F ;	
	DG_SetPos( world ); 
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	shift.vx = -3.0f - rnd() * 3.0F ;
	shift.vy = 0.0f;
	shift.vz = rnd() * 5.0F;
	DG_RotVector(&shift,&speed,1) ;

	NewGunSmoke2( &fmtmp , &speed , 20.0F , 20 , 20.f);

//	NewGunSmoke(&pos,&speed,10.0F,1) ; 
}

/* p90  */
void CartridgeP90(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

	shift.vx = P90_SHIFT_X ;
	shift.vy = P90_SHIFT_Y ;
	shift.vz = P90_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	pos.vw = 1.0F;
	
	speed.vx = frnd() * 10;
	speed.vy = frnd() * 10;
	speed.vz = -rnd() * 5;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_P90( &pos , &speed , &rot , weapon , control) ;
}

void	 AN_CartridgeP90_E(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	CartridgeP90(world, weapon, control) ;
}

// ソリダス 
void CartridgeP90_SOL(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

	shift.vx = P90_SOL_SHIFT_X ;
	shift.vy = P90_SOL_SHIFT_Y ;
	shift.vz = P90_SOL_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift ) ;
	pos.vw = 1.0F;
	
	speed.vx = frnd() * 10;
	speed.vy = frnd() * 10;
	speed.vz = -rnd() * 5;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_P90( &pos , &speed , &rot , weapon , control) ;
}

void AN_CartridgeP90_SOL_E(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	CartridgeP90_SOL(world, weapon, control) ;
}

// Glock 
void CartridgeGLK( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = GLK_SHIFT_X ;
	shift.vy = GLK_SHIFT_Y ;
	shift.vz = GLK_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 50.0f;
	speed.vy = rnd() * 70.0f;
	speed.vz = rnd() * 20.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_GLK( &pos , &speed , &rot , weapon , control) ;
}

void	 AN_CartridgeGLK_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeGLK(world, weapon, control) ;
}

// M4A1(ライデン用) 
void CartridgeM4A1( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = M4A1_SHIFT_X ;
	shift.vy = M4A1_SHIFT_Y ;
	shift.vz = M4A1_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 25.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_M4A1( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	if ( irnd() % 10 < 5 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}

void	 AN_CartridgeM4A1_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeM4A1(world, weapon, control) ;
}

// M4A1(snake用) 
void CartridgeM4A1_SNK( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = M4A1_SNK_SHIFT_X ;
	shift.vy = M4A1_SNK_SHIFT_Y ;
	shift.vz = M4A1_SNK_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 25.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_M4A1( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	if ( irnd() % 10 < 5 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}

void	 AN_CartridgeM4A1_SNK_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeM4A1_SNK(world, weapon, control) ;
}

// M4A1_HI(ハイテク兵用) 
void CartridgeM4A1_HI( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = M4A1_HI_SHIFT_X ;
	shift.vy = M4A1_HI_SHIFT_Y ;
	shift.vz = M4A1_HI_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 25.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_M4A1( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	if ( irnd() % 10 < 5 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}

void	 AN_CartridgeM4A1_HI_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeM4A1_HI(world, weapon, control) ;
}

// アバカン 
void CartridgeABK( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = ABK_SHIFT_X ;
	shift.vy = ABK_SHIFT_Y ;
	shift.vz = ABK_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 10.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_ABK( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
}

void	 AN_CartridgeABK_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeABK(world, weapon, control) ;
}

// AKS(ライデン用) 
void CartridgeAKS_RAI( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = AKS_RAI_SHIFT_X ;
	shift.vy = AKS_RAI_SHIFT_Y ;
	shift.vz = AKS_RAI_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 20.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_AKS( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	if ( irnd() % 10 < 3 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}

void	 AN_CartridgeAKS_RAI_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeAKS_RAI(world, weapon, control) ;
}

// m4demo 
void CartridgeM4_demo(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	FMATRIX fmtmp;
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

	shift.vx = M4_SHIFT_X ;
	shift.vy = M4_SHIFT_Y ;
	shift.vz = M4_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;

	//薬莢のスピード	 
	speed.vx = frnd() * 10;
	speed.vy = -rnd() * 10.0f;
	speed.vz = 0;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;

	MdlCartridg_M4_demo( &pos , &speed , &rot , weapon , control) ;
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 3.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 50.0F , 30 , 70.f );
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 2.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 20.0F , 20 , 50.f);
}

void	 AN_CartridgeM4_demo_E(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	CartridgeM4_demo(world, weapon, control) ;
}

// m4demo シールズ用grenade 
void CartridgeM4_demo_sel(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	FMATRIX fmtmp;
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

	shift.vx = M4_SEL_SHIFT_X ;
	shift.vy = M4_SEL_SHIFT_Y ;
	shift.vz = M4_SEL_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;

	//薬莢のスピード	 
	speed.vx = frnd() * 10;
	speed.vy = -rnd() * 10.0f;
	speed.vz = 0;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;

	MdlCartridg_M4_demo( &pos , &speed , &rot , weapon , control) ;
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 3.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 50.0F , 30 , 70.f );
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 2.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 20.0F , 20 , 50.f);
}

void	 AN_CartridgeM4_demo_sel_E(FMATRIX *world,OBJECT *weapon, CONTROL *control)
{
	CartridgeM4_demo_sel(world, weapon, control) ;
}

// M4A1(海兵隊用) m4a_nm, m4b_gl, m4c_sc 
void CartridgeM4_demo_gun( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = M4_DEMO_GUN_SHIFT_X ;
	shift.vy = M4_DEMO_GUN_SHIFT_Y ;
	shift.vz = M4_DEMO_GUN_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 10.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_M4_demo_gun( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	if ( irnd() % 10 < 4 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}

void	 AN_CartridgeM4_demo_gun_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeM4_demo_gun(world, weapon, control) ;
}

// M4A1(海兵隊用) demo_m4_grenade_usm 
void CartridgeM4_demo_gun_grenade( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = M4_DEMO_GUN_GRENADE_SHIFT_X ;
	shift.vy = M4_DEMO_GUN_GRENADE_SHIFT_Y ;
	shift.vz = M4_DEMO_GUN_GRENADE_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 10.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[ 2 ] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_M4_demo_gun( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	if ( irnd() % 10 < 4 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}

void AN_CartridgeM4_demo_gun_grenade_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeM4_demo_gun_grenade(world, weapon, control) ;
}

// M4A1(シールズ用) demo_m4_sel 
void CartridgeM4_demo_gun_sel( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = M4_DEMO_GUN_SEL_SHIFT_X ;
	shift.vy = M4_DEMO_GUN_SEL_SHIFT_Y ;
	shift.vz = M4_DEMO_GUN_SEL_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 10.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[ 2 ] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_M4_demo_gun( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	if ( irnd() % 10 < 4 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}


void AN_CartridgeM4_demo_gun_sel_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeM4_demo_gun_sel(world, weapon, control) ;
}


// M4A1(snake用) demo_m4_grn_sna.mdl 
void CartridgeM4_Snk_Grenade( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

	shift.vx = M4_SNK_GRENADE_SHIFT_X ;
	shift.vy = M4_SNK_GRENADE_SHIFT_Y ;
	shift.vz = M4_SNK_GRENADE_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;

	//薬莢のスピード	 
	speed.vx = frnd() * 10;
	speed.vy = -rnd() * 10.0f;
	speed.vz = 0;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;

	MdlCartridg_M4_demo( &pos , &speed , &rot , weapon , control) ;
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 3.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 50.0F , 30 , 70.f );
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 2.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 20.0F , 20 , 50.f);
}

void AN_CartridgeM4_Snk_Grenade_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeM4_Snk_Grenade(world, weapon, control) ;
}

// M4A1(Hitec用) demo_m4_grn_sna.mdl 
void CartridgeM4_Hi_Grenade( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift ;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

	shift.vx = M4_HI_GRENADE_SHIFT_X ;
	shift.vy = M4_HI_GRENADE_SHIFT_Y ;
	shift.vz = M4_HI_GRENADE_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector(&shift,&pos,1) ;
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;

	//薬莢のスピード	 
	speed.vx = frnd() * 10;
	speed.vy = -rnd() * 10.0f;
	speed.vz = 0;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;

	MdlCartridg_M4_demo( &pos , &speed , &rot , weapon , control) ;
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 3.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 50.0F , 30 , 40.f );
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 2.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 20.0F , 20 , 20.f);
}

void AN_CartridgeM4_Hi_Grenade_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeM4_Hi_Grenade(world, weapon, control) ;
}

// Single Action Army 
void CartridgeSAA_RIGHT( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;

	shift.vx = SAA_RIGHT_SHIFT_X ;
	shift.vy = SAA_RIGHT_SHIFT_Y ;
	shift.vz = SAA_RIGHT_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
}

void AN_CartridgeSAA_RIGHT_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeSAA_RIGHT(world, weapon, control) ;
}

void CartridgeSAA_LEFT( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;

	shift.vx = SAA_LEFT_SHIFT_X ;
	shift.vy = SAA_LEFT_SHIFT_Y ;
	shift.vz = SAA_LEFT_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
}

void AN_CartridgeSAA_LEFT_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgeSAA_LEFT(world, weapon, control) ;
}

// PSG(共用) 
void CartridgePSG( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	FMATRIX fmtmp;
	FVECTOR	shift;
	FVECTOR   pos;
	FVECTOR	speed;
	SVECTOR   rot;

	shift.vx = PSG_SHIFT_X ;
	shift.vy = PSG_SHIFT_Y ;
	shift.vz = PSG_SHIFT_Z ;
	shift.vw = 1.0F ;

	DG_SetPos( world );
	DG_PutVector( &shift , &pos , 1);
	DG_MovePos( &shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;
	
	speed.vx = -rnd() * 20.0f - 20.0f;
	speed.vy = rnd() * 50.0f;
	speed.vz = rnd() * 30.0f + 10.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;
	MdlCartridg_PSG( &pos , &speed , &rot , weapon , control);

	speed.vx = 0.0f;
	speed.vy = rnd() * 2.0f;
	speed.vz = rnd() * 5.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	if ( irnd() % 10 < 3 ){
		NewGunSmoke2( &fmtmp , &speed , 26.0F , 22 , 18.f );
	}
}

void	 AN_CartridgePSG_E( FMATRIX *world , OBJECT *weapon , CONTROL *control )
{
	CartridgePSG(world, weapon, control) ;
}

// Cayphe Meca 
void CartridgeMeca( FMATRIX *world , FVECTOR *shift )
{
	FMATRIX fmtmp;
	FVECTOR	pos ;
	FVECTOR	speed ;
	SVECTOR	rot ;

	DG_SetPos( world );
	DG_PutVector( shift,&pos,1) ;
	DG_MovePos( shift );
	DG_GetPos( &fmtmp );
	pos.vw = 1.0F;

	//薬莢のスピード	 
	speed.vx = -40.0f + rnd() * 8.0f;
	speed.vy = 20.0f + rnd() * 10.0f;
	speed.vz = 15.0f + rnd() * 8.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);

	TS_VecToRot( &rot, ( FVECTOR * )world->m[2] );
	rot.vx += irnd() % 128 - 64;
	rot.vy += irnd() % 128 - 64;
	rot.vz  = irnd() % 4096;

	MdlCartridg_MECA( &pos , &speed , &rot , NULL ,NULL );
	speed.vx = 0.0f;
	speed.vy = 0.0f;
	speed.vz = rnd() * 3.0f;
	speed.vw = 1.f;
	DG_RotVector( &speed , &speed , 1);
	NewGunSmoke2( &fmtmp , &speed , 100.0F , 20 , 6.f );
}

void AN_CartridgeMeca_E( FMATRIX *world , FVECTOR *shift )
{
	CartridgeMeca(world, shift) ;
}
