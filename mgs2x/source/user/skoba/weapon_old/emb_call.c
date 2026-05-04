//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	emb_call.c
	統合薬莢呼び出し
	
	2001/07/23 T.Shibata
	
	$Id: emb_call.c,v 1.1.1.3 2002/11/19 11:50:34 Yoshizawa1 Exp $

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

#include 	"libutl.h"
#include	"rand.h"

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"emb_control.h"


extern void *NewConnectSmoke_SKOBA( FVECTOR *pos, SVECTOR *rot, FVECTOR *shift, u_char *mode, float size_rnd, CVECTOR col );

#define   DEFAULT_TIME		(20)

/* USPモデルの呼び出し */
void *MdlCartridg_USP(
	FVECTOR  *pos,							   
	FVECTOR  *speed,			  /* スピード */
	SVECTOR  *rot,
	OBJECT   *weapon,
	CONTROL  *control			  /* 人体コントロール */
	)
{
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%128-64;
	add_rot.vy = irnd()%128-64;
	add_rot.vz = irnd()%128-64 ;

	SetNewEmbData( AMO_KIND_USP, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );
	return 0 ;
}

/* Socomモデルの呼び出し */
void *MdlCartridg_SOCOM(
	FVECTOR  *pos,							   
	FVECTOR  *speed,			  /* スピード */
	SVECTOR  *rot,
	OBJECT   *weapon,
	CONTROL  *control			  /* 人体コントロール */
	)
{
	
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%128-64;
	add_rot.vy = irnd()%128-64;
	add_rot.vz = irnd()%128-64 ;

	SetNewEmbData( AMO_KIND_SOCOM, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );
	return 0 ;

}

/* FMSモデルの呼び出し */
void *MdlCartridg_FMS(
	FVECTOR  *pos,							   
	FVECTOR  *speed,			  /* スピード */
	SVECTOR  *rot,
	OBJECT   *weapon,
	CONTROL  *control			  /* 人体コントロール */
	)
{
	
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%128-64;
	add_rot.vy = irnd()%128-64;
	add_rot.vz = irnd()%128-64 ;

	SetNewEmbData( AMO_KIND_FMS, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );
	return 0 ;
}

/* AKSモデルの呼び出し */
void *MdlCartridg_AKS(
FVECTOR  *pos,							   
FVECTOR  *speed,			  /* スピード */
SVECTOR  *rot,
OBJECT   *weapon,
CONTROL  *control			  /* 人体コントロール */
)
{
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.90f;
	set_speed.vy = speed->vy * 1.20f;
	set_speed.vz = speed->vz * 0.90f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%128-64;
	add_rot.vy = irnd()%128-64;
	add_rot.vz = irnd()%128-64 ;

	SetNewEmbData( AMO_KIND_AKS, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );
	
	return 0 ;
}

/* SPSモデルの呼び出し */
void *MdlCartridg_SPS(
FVECTOR  *pos,							   
FVECTOR  *speed,			  /* スピード */
SVECTOR  *rot,
OBJECT   *weapon,
CONTROL  *control			  /* 人体コントロール */
)
{

	//int				count_all ;
	EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	CVECTOR			color;
	
	set_speed.vx = speed->vx * 0.90f;
	set_speed.vy = speed->vy * 1.20f;
	set_speed.vz = speed->vz * 0.90f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%128-64;
	add_rot.vy = irnd()%128-64;
	add_rot.vz = irnd()%128-64 ;

	emb_data = SetNewEmbData( AMO_KIND_SPS, pos, &set_speed, rot, &add_rot, DIRECT_TICK( irnd()%20 ) );

	color.r = 74;
	color.g = 74;
	color.b = 74;
	
	NewConnectSmoke_SKOBA( &emb_data->pos , &emb_data->rot , &DG_ZeroVector , &emb_data->mode , 120.f , color );
	
	return 0 ;
}

/* P90モデルの呼び出し 薬莢*/	
void *MdlCartridg_P90(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control )
{
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%128-64;
	add_rot.vy = irnd()%128-64;
	add_rot.vz = irnd()%128-64 ;

	SetNewEmbData( AMO_KIND_P90, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );

	return 0 ;
}

/* PSGモデルの呼び出し 薬莢*/	
void *MdlCartridg_PSG(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control )
{
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%128-64;
	add_rot.vy = irnd()%128-64;
	add_rot.vz = irnd()%128-64 ;

	SetNewEmbData( AMO_KIND_PSG, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );

	return 0 ;	
}

/* GLKモデルの呼び出し 薬莢*/	
void *MdlCartridg_GLK(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%1024-512;
	add_rot.vy = irnd()%512-256;
	add_rot.vz = irnd()%1024-512;

	SetNewEmbData( AMO_KIND_GLK, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );

	return 0 ;
}

// M4A1モデルの呼び出し 薬莢 (ライデン用)	 
void *MdlCartridg_M4A1(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%1024-512;
	add_rot.vy = irnd()%512-256;
	add_rot.vz = irnd()%1024-512;

	SetNewEmbData( AMO_KIND_M4A1, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );

	return 0 ;
}

// MECAモデルの呼び出し 薬莢 (Cayphe用)	 
void *MdlCartridg_MECA(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%1024-512;
	add_rot.vy = irnd()%512-256;
	add_rot.vz = irnd()%1024-512;

	SetNewEmbData( AMO_KIND_MECA, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );

	return 0 ;
}

// ABAKANモデルの呼び出し	 
void *MdlCartridg_ABK( FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%1024-512;
	add_rot.vy = irnd()%512-256;
	add_rot.vz = irnd()%1024-512;

	SetNewEmbData( AMO_KIND_ABK, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );

	return 0 ;

}

/* M4モデルの呼び出し 薬莢*/	
void *MdlCartridg_M4_demo(FVECTOR  *pos, FVECTOR *speed, SVECTOR *rot, OBJECT *weapon, CONTROL  *control )
{
	//int				count_all ;
	EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	CVECTOR			color;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%128-64;
	add_rot.vy = irnd()%128-64;
	add_rot.vz = irnd()%128-64;

	emb_data = SetNewEmbData( AMO_KIND_M4GRN, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );
	
	color.r = 74;
	color.g = 74;
	color.b = 74;

	NewConnectSmoke_SKOBA( &emb_data->pos , &emb_data->rot , &DG_ZeroVector , &emb_data->mode , 80.f , color );
	
	return 0 ;

}

// M4_demo_gunモデルの呼び出し 薬莢 	 
void *MdlCartridg_M4_demo_gun(FVECTOR *pos , FVECTOR *speed , SVECTOR *rot , OBJECT *weapon , CONTROL  *control )
{
	//int				count_all ;
	//EMB_DATA		*emb_data ;
	//DG_COMDL_POS	*comdl_pos ;
	SVECTOR			add_rot;
	FVECTOR			set_speed;
	
	set_speed.vx = speed->vx * 0.70f;
	set_speed.vy = speed->vy * 0.90f;
	set_speed.vz = speed->vz * 0.70f;
	set_speed.vw = 0.0f;
	add_rot.vx = irnd()%1024-512;
	add_rot.vy = irnd()%512-256;
	add_rot.vz = irnd()%1024-512;

	SetNewEmbData( AMO_KIND_M4, pos, &set_speed, rot, &add_rot, DIRECT_TICK( DEFAULT_TIME ) );
	
	return 0 ;
	
}

