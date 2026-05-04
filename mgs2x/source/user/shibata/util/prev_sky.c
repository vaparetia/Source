//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    plant_sun.c
	ゲーム中の太陽
	2000/12/14 T.Shibata
	
	$Id: prev_sky.c,v 1.1.1.3 2002/11/19 11:48:54 Yoshizawa1 Exp $

*/
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"../util/ts_util.h"
#include	"gameheader.h"
#include	"../../kano/preview/prekey.h"
#include	"../../kano/preview/preview_def.h"
#include	"../../kano/preview/prefile.h"

#define		PRINT_PFVEC(_i,_fv) printf("[%d] vx %f:vy %f:vz %f:vw %f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)

//11*12 == 132
//44 * 3
//(N_PRIMS*N_VERTS)==(N_STRIP_VERTS*N_STRIPS)

#define		N_PRIMS			(12)
#define		N_VERTS			(8)
#define		N_ALLVERTS		(N_VERTS*N_PRIMS)
#define		N_STRIP_VERTS	(8)
#define		N_STRIPS		(12)

#ifdef BP_PS2
#define	   	MEM_SCR_UV	((void*)(SCRPAD_ADDR))
#define	   	MEM_SCR_POS	((void*)(MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS))
#define	   	MEM_SCR_TMP	((void*)(MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS))
#else
#define	   	MEM_SCR_UV	((void*)(SCRPAD_ADDR))
#define	   	MEM_SCR_POS	((void*)((char *)MEM_SCR_UV + sizeof(DG_PRIM2_UVRGB)*N_ALLVERTS))
#define	   	MEM_SCR_TMP	((void*)((char *)MEM_SCR_POS + sizeof(FVECTOR)*N_ALLVERTS))
#endif

#define				SKY_SCALE	(32.0f)
//#define			SKY_SCALE	(1.0f)

#define		SKY_COLOR_R		(130)
#define		SKY_COLOR_G		(200)
#define		SKY_COLOR_B		(250)

#define 	CLOUD_COLOR		(255)
#define 	CLOUD_ALPHA		(128)

#define		N_MAX_MODLS			(4)

typedef struct {
	GV_ACT_EX		actor;
	DG_PRIM2		*base_prim;
	DG_PRIM2		*base_cloud[4];
	float			base_cloud_angle[4];
	CVECTOR			color;
	int				flags;

	FVECTOR			center;
	FVECTOR			bound[12][2];
	FVECTOR			base_cloud_pos[4][30];
	FMATRIX			root;

	float			cloud_speed[N_MAX_MODLS+4];
	int				corsor;
	int				stage_num;
	int				tex_num;
	int				load_file_num;
	int				save_file_num;
	float			sky_bottom;
	float			w,h;
	int				mdl_num;
	float			rot_y[N_MAX_MODLS];
	DG_OBJS			*objs[N_MAX_MODLS];
	FMATRIX			obj_root[N_MAX_MODLS];
	FMATRIX			lights[N_MAX_MODLS][2];
	CVECTOR			obj_color[N_MAX_MODLS+4];


} Work;
static float RootRot[N_MAX_MODLS+4] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
#define SUN_WH
static Work *PrevSkyWork = NULL;

enum {
	PREV_SKY_SHIFT_X = 0,
	PREV_SKY_SHIFT_Y,
	PREV_SKY_SHIFT_Z,
	PREV_SKY_COL_R,
	PREV_SKY_COL_G,
	PREV_SKY_COL_B,
	PREV_SKY_COL_A,
	PREV_SKY_COL_RGB,
	PREV_SKY_BOTTOM,

#ifdef SUN_WH
	PREV_SKY_WH,
#endif
	PREV_SKY_CLOUD,
	PREV_SKY_CLOUD_SPEED,
	PREV_SKY_TEX,
	PREV_SKY_MDL_NUM,
	PREV_SKY_OBJ_COL_R,
	PREV_SKY_OBJ_COL_G,
	PREV_SKY_OBJ_COL_B,
	
	PREV_SKY_STAGE,
	PREV_SKY_SAVE,
	PREV_SKY_LOAD,

	PREV_SKY_MAX,
};

#define N_STAGE	(sizeof(StageSkyPos)/sizeof(StageSkyPos[0]))

FVECTOR StageSkyPos[] = {
	{ 0.0f, 0.0f, 0.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	
	{ 52000.0f, -40000.0f, -60000.0f, 1.0f },
	{ 52000.0f, -40000.0f, -60000.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	
	{ 52000.0f, -40000.0f, 60000.0f, 1.0f },
	{ 52000.0f, -40000.0f, 60000.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	{ 0.0f, -40000.0f, 0.0f, 1.0f },
	
	{ 0.0f, -40000.0f, -150000.0f, 1.0f },
	{ 10000.0f, -40000.0f, -180000.0f, 1.0f },
	{ 52000.0f, -40000.0f, -240000.0f, 1.0f },
	{ 52000.0f, -40000.0f, -240000.0f, 1.0f },
	{ 52000.0f, 0.0f, -210000.0f, 1.0f },

	{ 0.0f, -40000.0f, -150000.0f, 1.0f },
	{ 10000.0f, -40000.0f, -180000.0f, 1.0f },
	{ 52000.0f, -40000.0f, -240000.0f, 1.0f },
	{ 52000.0f, -40000.0f, -240000.0f, 1.0f },
	{ 52000.0f, 0.0f, -210000.0f, 1.0f },

	{ 10000.0f, -40000.0f, -180000.0f, 1.0f },
	{ 52000.0f, -40000.0f, -240000.0f, 1.0f },
	{ 52000.0f, -40000.0f, -240000.0f, 1.0f },
	{ 52000.0f, 0.0f, -210000.0f, 1.0f },
};

static char StageTypeCloud[] = {
	-1,
	0,
	1,
	0,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	2,
	0,
	2,
	1,
	1,
	1,
	2,
	2,

	1,
	1,
	1,
	2,
	2,
	
	1,
	1,
	2,
	2,
};

static CVECTOR StageSkyCol[] = {
	{ 41,32,30, 0 },
	{ 90,110,95, 0 },
	{ 150,175,175, 0 },
	{ 90,110,95, 0 },
	{ 150,175,175, 0 },
	{ 90,110,95, 0 },
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 55,40,25, 0 },
	{ 90,110,95, 0 },
	{ 55,40,25, 0 },
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 55,40,25, 0 },
	{ 55,40,25, 0 },

	
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 55,40,25, 0 },
	{ 55,40,25, 0 },
	
	{ 150,175,175, 0 },
	{ 150,175,175, 0 },
	{ 55,40,25, 0 },
	{ 55,40,25, 0 },
};

static char StageName[N_STAGE][8] = {
	"w10a",
	"w12a",
	"w12b",
	"w13a",
	"w13b",
	"w15a",
	"w15b",
	"w17a",
	"w19a",
	"w20b",
	"w21a",
	"w21b",
	"w23a",
	"w23b",
	"w25a",
	"w25b",
	"w25c",
	"w25d",
	"w32a",

	"w32b",
	"w32c",
	"w60a",
	"w62a0",
	"w62a1",
	
	"tmp1",
	"tmp2",
	"tmp3",
	"tmp4",
};

static char SkyTypeName[5][5] = {
	"morn",
	"day ",
	"even",
	"nigh",
	"snpr",
};

static int SkyTypeTexCode[5][4] = {
	{
		1045148,			//GV_StrCode("sky_m1_alp")
		2093724,			//GV_StrCode("sky_m2_alp")
		3142300,			//GV_StrCode("sky_m3_alp")
		4190876,			//GV_StrCode("sky_m4_alp")
	},
	{
		1045130,			//GV_StrCode("sky_d1_alp")
		2093706,			//GV_StrCode("sky_d2_alp")
		3142282,			//GV_StrCode("sky_d3_alp")
		4190858,			//GV_StrCode("sky_d4_alp")
	},
	{
		1045132,			//GV_StrCode("sky_e1_alp")
		2093708,			//GV_StrCode("sky_e2_alp")
		3142284,			//GV_StrCode("sky_e3_alp")
		4190860,			//GV_StrCode("sky_e4_alp")
	},
	
	{
		1045150,			//GV_StrCode("sky_n1_alp")
		2093726,			//GV_StrCode("sky_n2_alp")
		3142302,			//GV_StrCode("sky_n3_alp")
		4190878,			//GV_StrCode("sky_n4_alp")
	},
	{
		1045160,			//GV_StrCode("sky_s1_alp")
		2093736,			//GV_StrCode("sky_s2_alp")
		3142312,			//GV_StrCode("sky_s3_alp")
		4190888,			//GV_StrCode("sky_s4_alp")
	},
};

static char *Modl_Name[] = {
	"sky_wall1",
	"sky_wall2",
	"sky_wall3",
	"sky_wall4",

	"cloud 1",
	"cloud 2",
	"cloud 3",
	"cloud 4",
};

static DG_PRIM2 *InitBaseCloud( Work *work, float len, float off_y, float len_y,
								short rot_vy, int num, int tex_code,
								int color,  int alpha, int pri );


void PrevSkyPrint()
{
    int x,y;

    x=LOCATE_X+9;
    y=LOCATE_Y;
    DEBUG_Locate( x, y, 0 );
    DEBUG_Printf( "--- Prev Sky Menu ---\n"     );
    DEBUG_Printf( "  SHIFT	X[%f]\n", PrevSkyWork->root.m[3][0] );
    DEBUG_Printf( "  SHIFT	Y[%f]\n", PrevSkyWork->root.m[3][1] );
    DEBUG_Printf( "  SHIFT	Z[%f]\n", PrevSkyWork->root.m[3][2] );
    DEBUG_Printf( "  COLOR	R[%3d]\n", PrevSkyWork->color.r );
    DEBUG_Printf( "  COLOR	G[%3d]\n", PrevSkyWork->color.g );
    DEBUG_Printf( "  COLOR	B[%3d]\n", PrevSkyWork->color.b );
    DEBUG_Printf( "  SUN ALP [%3d]\n", PrevSkyWork->color.cd );
    DEBUG_Printf( "  <MODE RGB>\n" );
    DEBUG_Printf( "  SKY_SIZE[%f]\n",PrevSkyWork->sky_bottom );
	
#ifdef SUN_WH
    DEBUG_Printf( "  SUN W[%0.2f] H[%0.2f]<hold R1 height>\n", PrevSkyWork->w,PrevSkyWork->h);
#endif
    DEBUG_Printf( "  CLOUD   " );
	if(PrevSkyWork->flags & 0x0002) DEBUG_Printf( "[OFF]\n" );
	else DEBUG_Printf( "[ ON]\n" );

    DEBUG_Printf( "  CLOUD_SPEED   [%0.2f][%0.5f]<press A clear>\n",
				  PrevSkyWork->cloud_speed[PrevSkyWork->mdl_num], RootRot[PrevSkyWork->mdl_num]);
    DEBUG_Printf( "  CLOUD_TYPE   [%s]\n", SkyTypeName[PrevSkyWork->tex_num]);
    DEBUG_Printf( "  OBJNUM	[%s]\n", Modl_Name[PrevSkyWork->mdl_num] );
    DEBUG_Printf( "  OBJCOL	R[%3d]\n", PrevSkyWork->obj_color[PrevSkyWork->mdl_num].r );
    DEBUG_Printf( "  OBJCOL	G[%3d]\n", PrevSkyWork->obj_color[PrevSkyWork->mdl_num].g );
    DEBUG_Printf( "  OBJCOL	B[%3d]\n", PrevSkyWork->obj_color[PrevSkyWork->mdl_num].b );

    DEBUG_Printf( "  STAGE   [%s]\n", StageName[PrevSkyWork->stage_num] );
    DEBUG_Printf( "  DATA SAVE [%s]\n", StageName[PrevSkyWork->save_file_num] );
    DEBUG_Printf( "  DATA LOAD [%s]\n", StageName[PrevSkyWork->load_file_num] );
    if(PrevSkyWork->flags & 0x0001) DEBUG_Printf( "   <Visible :L1>\n" );
    else DEBUG_Printf( "   <Invisible :L1>\n" );
	DEBUG_Printf( "   <Shift Speed Up :L2 hold>\n" );

    x=LOCATE_X;
    y=LOCATE_Y + (PrevSkyWork->corsor+1)*FONT_HEIGHT;
    DEBUG_Locate( x, y, 0 );
    DEBUG_Printf( ">>" );
}

static float ShiftControl( float fig, float add, float ex_add )
{
	float ret = fig;
	if(PreviewKey.status & PAD_L2 ){
		if( PreviewKey.auto_status & PAD_R ) ret += ex_add;
		if( PreviewKey.auto_status & PAD_L ) ret -= ex_add;
		if( PreviewKey.press & PAD_X ) ret += 10.0f;
		if( PreviewKey.press & PAD_Y ) ret -= 10.0f;
	}else{
		if( PreviewKey.auto_status & PAD_R ) ret += add;
		if( PreviewKey.auto_status & PAD_L ) ret -= add;
		if( PreviewKey.press & PAD_X ) ret += 5.0f;
		if( PreviewKey.press & PAD_Y ) ret -= 5.0f;
	}
	return (ret);
}

static int ColorControl( int col, int add, int ex_add )
{
	int ret = col;
	if(PreviewKey.status & PAD_L2 ){
		if( PreviewKey.auto_status & PAD_R ) ret += ex_add;
		if( PreviewKey.auto_status & PAD_L ) ret -= ex_add;
		if( PreviewKey.press & PAD_X ) ret += add;
		if( PreviewKey.press & PAD_Y ) ret -= add;
	}else{
		if( PreviewKey.auto_status & PAD_R ) ret += add;
		if( PreviewKey.auto_status & PAD_L ) ret -= add;
		if( PreviewKey.press & PAD_X ) ret += 1;
		if( PreviewKey.press & PAD_Y ) ret -= 1;
	}
	if(ret > 255) ret = 255;
	else if( ret < 0 ) ret = 0;

	return (ret);
}

#define SKY_DATA_FILE	"host0:./skydata.sky"

static int SkyDataSave( Work *work )
{
	char	file_name[256] = "";
	strcpy( file_name, StageName[work->save_file_num] );//, strlen(StageName[work->save_file_num]) );
	strcat( file_name, ".sky" );
	return (PreviewSaveBinFile( file_name, work, sizeof(Work) )!=0);
}

static int SkyDataLoad( Work *work )
{
	Work	data;
	char	file_name[256];
	strcpy( file_name, StageName[work->load_file_num] );//, strlen(StageName[work->load_file_num]) );
	strcat( file_name, ".sky" );
	
	if( PreviewLoadBinFile( file_name, &data, sizeof(Work) ) ){
		

		work->color = data.color;
		DG_COPY_VEC( &work->center, &data.center );
		memcpy( work->bound, data.bound, sizeof(FVECTOR)*12*2 );
		memcpy( work->base_cloud_pos, data.base_cloud_pos, sizeof(FVECTOR)*4*30 );
		DG_COPY_MAT( &work->root, &data.root );
		work->cloud_speed[0] = data.cloud_speed[0];
		work->cloud_speed[1] = data.cloud_speed[1];
		work->cloud_speed[2] = data.cloud_speed[2];
		work->cloud_speed[3] = data.cloud_speed[3];
		work->cloud_speed[4] = data.cloud_speed[4];
		work->cloud_speed[5] = data.cloud_speed[5];
		work->cloud_speed[6] = data.cloud_speed[6];
		work->cloud_speed[7] = data.cloud_speed[7];
		
		work->sky_bottom = data.sky_bottom;
		work->w = data.w;
		work->h = data.h;

		memcpy( work->obj_root, data.obj_root, sizeof(FMATRIX)*N_MAX_MODLS );
		//memcpy( work->rot, data.rot, sizeof(SVECTOR)*N_MAX_MODLS );
		memcpy( work->lights, data.lights, sizeof(FMATRIX)*N_MAX_MODLS*2 );
		memcpy( work->obj_color, data.obj_color, sizeof(CVECTOR)*(N_MAX_MODLS+4) );
		
		return 0;
	}
	return 1;
}

void PrevSkyControl()
{
    if( PreviewKey.auto_status & PAD_U ){
		PrevSkyWork->corsor = (PrevSkyWork->corsor+(PREV_SKY_MAX-1))%PREV_SKY_MAX;
    }
    if( PreviewKey.auto_status & PAD_D ){
		PrevSkyWork->corsor = (PrevSkyWork->corsor+1)%PREV_SKY_MAX;
    }

    if( PreviewKey.press & PAD_L1 ){
		PrevSkyWork->flags ^= 0x0001; 
    }

	switch(PrevSkyWork->corsor){
	  case PREV_SKY_SHIFT_X:
		PrevSkyWork->root.m[3][0] = ShiftControl( PrevSkyWork->root.m[3][0], 50.0f, 500.0f );
		break;
	  case PREV_SKY_SHIFT_Y:
		PrevSkyWork->root.m[3][1] = ShiftControl( PrevSkyWork->root.m[3][1], 50.0f, 500.0f );
		break;
	  case PREV_SKY_SHIFT_Z:
		PrevSkyWork->root.m[3][2] = ShiftControl( PrevSkyWork->root.m[3][2], 50.0f, 500.0f );
		break;
	  case PREV_SKY_COL_R:
		PrevSkyWork->color.r = ColorControl( PrevSkyWork->color.r, 1, 4 );
		break;
	  case PREV_SKY_COL_G:
		PrevSkyWork->color.g = ColorControl( PrevSkyWork->color.g, 1, 4 );
		break;
	  case PREV_SKY_COL_B:
		PrevSkyWork->color.b = ColorControl( PrevSkyWork->color.b, 1, 4 );
		break;
	  case PREV_SKY_COL_A:
		PrevSkyWork->color.cd = ColorControl( PrevSkyWork->color.cd, 1, 4 );
		break;
	  case PREV_SKY_COL_RGB:
		{
			int rgb = ColorControl( PrevSkyWork->color.r, 1, 4 );
			if(PrevSkyWork->color.r != rgb){
				PrevSkyWork->color.g = PrevSkyWork->color.b = PrevSkyWork->color.r = rgb;
			}
		}
		break;
	  case PREV_SKY_BOTTOM:
		PrevSkyWork->sky_bottom = ShiftControl( PrevSkyWork->sky_bottom, 5.0f, 50.0f );
		if( PreviewKey.auto_status & PAD_R2 ) PrevSkyWork->sky_bottom = 6000.0f;
		break;
		
#ifdef SUN_WH
	  case PREV_SKY_WH:
		if(PreviewKey.status & PAD_R1 ){
			PrevSkyWork->h = ShiftControl( PrevSkyWork->h, 0.5f, 5.0f );
		}else{
			PrevSkyWork->w = ShiftControl( PrevSkyWork->w, 0.5f, 5.0f );
		}
		break;
#endif
	  case PREV_SKY_CLOUD:
		if(PreviewKey.auto_status & (PAD_R|PAD_L)) PrevSkyWork->flags ^= 0x0002;
		break;
	  case PREV_SKY_CLOUD_SPEED:
		if( PreviewKey.press & PAD_A ){
			int i;
			for( i = 0; i < N_MAX_MODLS; i++ ){
				PrevSkyWork->base_cloud_angle[i] = 0.0f;
				PrevSkyWork->rot_y[i] = 0.0f;
			}
		}
		if( PreviewKey.status & PAD_R1 ){
			RootRot[PrevSkyWork->mdl_num] =
				ShiftControl( RootRot[PrevSkyWork->mdl_num], 0.001f, 0.01f );
		}else{
			PrevSkyWork->cloud_speed[PrevSkyWork->mdl_num] =
				ShiftControl( PrevSkyWork->cloud_speed[PrevSkyWork->mdl_num], 0.5f, 5.0f );
		}
		break;
	  case PREV_SKY_TEX:
		if(PreviewKey.auto_status & PAD_R)
			PrevSkyWork->tex_num = (PrevSkyWork->tex_num + 1 )%5;
		if(PreviewKey.auto_status & PAD_L)
			PrevSkyWork->tex_num = (PrevSkyWork->tex_num + (5-1) )%5;
		if(PreviewKey.press & PAD_A){
			if(PrevSkyWork->base_cloud[0])GM_FreePrim2(PrevSkyWork->base_cloud[0]);
			if(PrevSkyWork->base_cloud[1])GM_FreePrim2(PrevSkyWork->base_cloud[1]);
			if(PrevSkyWork->base_cloud[2])GM_FreePrim2(PrevSkyWork->base_cloud[2]);
			if(PrevSkyWork->base_cloud[3])GM_FreePrim2(PrevSkyWork->base_cloud[3]);

			PrevSkyWork->flags &= 0xffff00ff;
			PrevSkyWork->flags |= (0x0004|(PrevSkyWork->tex_num<<8));
			
			PrevSkyWork->base_cloud[0] = InitBaseCloud( PrevSkyWork, 9600.0f, 500.0f, 6000.0f, 0*1024/4,
														0, SkyTypeTexCode[PrevSkyWork->tex_num][0],
														*(int*)&PrevSkyWork->obj_color[0+4], 96, 0 );
			PrevSkyWork->base_cloud[1] = InitBaseCloud( PrevSkyWork, 8800.0f, 500.0f, 6000.0f, 1*1024/4,
														1, SkyTypeTexCode[PrevSkyWork->tex_num][1],
														*(int*)&PrevSkyWork->obj_color[1+4], 96, 100 );
			PrevSkyWork->base_cloud[2] = InitBaseCloud( PrevSkyWork, 8000.0f, 300.0f, 6000.0f, 2*1024/4,
														2, SkyTypeTexCode[PrevSkyWork->tex_num][2],
														*(int*)&PrevSkyWork->obj_color[2+4], CLOUD_ALPHA, 200 );
			PrevSkyWork->base_cloud[3] = InitBaseCloud( PrevSkyWork, 7200.0f, 300.0f, 6000.0f, 3*1024/4,
														3, SkyTypeTexCode[PrevSkyWork->tex_num][3],
														*(int*)&PrevSkyWork->obj_color[3+4], CLOUD_ALPHA, 300 );
		}

		break;
		
	  case PREV_SKY_MDL_NUM:
		if(PreviewKey.auto_status & PAD_R) PrevSkyWork->mdl_num = (PrevSkyWork->mdl_num+1)%(N_MAX_MODLS+4);
		if(PreviewKey.auto_status & PAD_L) PrevSkyWork->mdl_num = (PrevSkyWork->mdl_num+N_MAX_MODLS-1+4)%(N_MAX_MODLS+4);
		break;
		
	  case PREV_SKY_OBJ_COL_R:
		PrevSkyWork->obj_color[PrevSkyWork->mdl_num].r = ColorControl( PrevSkyWork->obj_color[PrevSkyWork->mdl_num].r, 1, 4 );
		break;
	  case PREV_SKY_OBJ_COL_G:
		PrevSkyWork->obj_color[PrevSkyWork->mdl_num].g = ColorControl( PrevSkyWork->obj_color[PrevSkyWork->mdl_num].g, 1, 4 );
		break;
	  case PREV_SKY_OBJ_COL_B:
		PrevSkyWork->obj_color[PrevSkyWork->mdl_num].b = ColorControl( PrevSkyWork->obj_color[PrevSkyWork->mdl_num].b, 1, 4 );
		break;

	  case PREV_SKY_STAGE:
		if(PreviewKey.auto_status & PAD_R)
			PrevSkyWork->stage_num = (PrevSkyWork->stage_num + 1 )%N_STAGE;
		if(PreviewKey.auto_status & PAD_L)
			PrevSkyWork->stage_num = (PrevSkyWork->stage_num + (N_STAGE-1) )%N_STAGE;
		if(PreviewKey.press & PAD_A){
			DG_COPY_VEC( (FVECTOR*)PrevSkyWork->root.m[3], &StageSkyPos[PrevSkyWork->stage_num] );
			PrevSkyWork->color = *(CVECTOR*)&StageSkyCol[PrevSkyWork->stage_num];

			if( StageTypeCloud[PrevSkyWork->stage_num] < 0 ){
				PrevSkyWork->flags |= 0x0002;
			}else{
				PrevSkyWork->tex_num = StageTypeCloud[PrevSkyWork->stage_num];
				if(PrevSkyWork->base_cloud[0])GM_FreePrim2(PrevSkyWork->base_cloud[0]);
				if(PrevSkyWork->base_cloud[1])GM_FreePrim2(PrevSkyWork->base_cloud[1]);
				if(PrevSkyWork->base_cloud[2])GM_FreePrim2(PrevSkyWork->base_cloud[2]);
				if(PrevSkyWork->base_cloud[3])GM_FreePrim2(PrevSkyWork->base_cloud[3]);

				PrevSkyWork->flags |= (0x0004|(PrevSkyWork->tex_num<<8));

				PrevSkyWork->base_cloud[0] = InitBaseCloud( PrevSkyWork, 9600.0f, 500.0f, 6000.0f, 0*1024/4,
															0, SkyTypeTexCode[PrevSkyWork->tex_num][0],
															*(int*)&PrevSkyWork->obj_color[0+4], 96, 0 );
				PrevSkyWork->base_cloud[1] = InitBaseCloud( PrevSkyWork, 8800.0f, 500.0f, 6000.0f, 1*1024/4,
															1, SkyTypeTexCode[PrevSkyWork->tex_num][1],
															*(int*)&PrevSkyWork->obj_color[1+4], 96, 100 );
				PrevSkyWork->base_cloud[2] = InitBaseCloud( PrevSkyWork, 8000.0f, 300.0f, 6000.0f, 2*1024/4,
															2, SkyTypeTexCode[PrevSkyWork->tex_num][2],
															*(int*)&PrevSkyWork->obj_color[2+4], CLOUD_ALPHA, 200 );
				PrevSkyWork->base_cloud[3] = InitBaseCloud( PrevSkyWork, 7200.0f, 300.0f, 6000.0f, 3*1024/4,
															3, SkyTypeTexCode[PrevSkyWork->tex_num][3],
															*(int*)&PrevSkyWork->obj_color[3+4], CLOUD_ALPHA, 300 );
			}
		}
		break;
	  case PREV_SKY_SAVE:
		if(PreviewKey.auto_status & PAD_R)
			PrevSkyWork->save_file_num = (PrevSkyWork->save_file_num + 1 )%N_STAGE;
		if(PreviewKey.auto_status & PAD_L)
			PrevSkyWork->save_file_num = (PrevSkyWork->save_file_num + (N_STAGE-1) )%N_STAGE;
		if(PreviewKey.press & PAD_A){
			if(!SkyDataSave( PrevSkyWork ) )
				printf("ERR!! SkyData Cannot Save!!\n");
		}
		break;
	  case PREV_SKY_LOAD:
		if(PreviewKey.auto_status & PAD_R)
			PrevSkyWork->load_file_num = (PrevSkyWork->load_file_num + 1 )%N_STAGE;
		if(PreviewKey.auto_status & PAD_L)
			PrevSkyWork->load_file_num = (PrevSkyWork->load_file_num + (N_STAGE-1) )%N_STAGE;
			
		if(PreviewKey.press & PAD_A){
			if(SkyDataLoad( PrevSkyWork )){
				printf("ERR!! SkyData Cannot Load!!\n");
				break;
			}
		}
		break;
	}
}

static void SetPirmRGB( DG_PRIM2 *prim, int n_verts, CVECTOR color )
{
	int				clock,i;
	DG_PRIM2_UVRGB	*uvrgb;
	clock = prim->buffer_clock;

	uvrgb = prim->uvrgb[clock];

	for( i = 0; i < n_verts; i++ ){
		uvrgb->r = color.r;
		uvrgb->g = color.g;
		uvrgb->b = color.b;
		uvrgb++;
	}
}

static void MoveBaseCloud( Work *work,float add_angle, int num )
{
	DG_PRIM2	*prim = work->base_cloud[num];
	int			clock;
	FVECTOR		*pos;
	FMATRIX		world;
	FVECTOR		force = {0.0f,0.0f,0.0f,0.0f};
	float		angle = work->base_cloud_angle[num] + RootRot[num+4];

	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	SetPirmRGB( prim, 30, work->obj_color[num+4] );
	pos = prim->pos[clock];

	while( angle < -PI ) angle += 2.0f*PI;
	while( angle > PI ) angle -= 2.0f*PI;

	force.vx = cosf(angle);
	force.vz = sinf(angle);

	TS_MakeMatrix( &world, &force, &work->center );

	DG_SetPos(&world);
	DG_PutVector(work->base_cloud_pos[num],pos,30);
	
	work->base_cloud_angle[num]+=add_angle;
	if( work->base_cloud_angle[num] > PI )work->base_cloud_angle[num]-=2.0f*PI;
	
}

static void SkyBottomSizeChange( DG_PRIM2 *prim, FVECTOR *pbound, float size )
{
	FVECTOR			*pos,*bound = pbound;
	int				clock,i;
	
	clock = prim->buffer_clock;
	pos = prim->pos[clock];

	pos = &pos[4*N_STRIP_VERTS];
	bound += 2*4;
	for( i = 0; i < 8; i++ ){
		pos[0].vy = pos[2].vy - size*SKY_SCALE;
		pos[1].vy = pos[3].vy - size*SKY_SCALE;
		bound[0].vy = (pos[0].vy>pos[2].vy)?pos[0].vy:pos[2].vy;
		bound[1].vy = (pos[0].vy>pos[2].vy)?pos[2].vy:pos[0].vy;
		bound+=2;
		pos += N_STRIP_VERTS;
	}
}

static void Act(Work *work)
{
	DG_PRIM2	*base_prim = work->base_prim;
	int 		clock,i,flags;
//	FMATRIX		world;

	if(work->flags & 0x0001){
		DG_InvisiblePrim2(work->base_prim);
		DG_InvisiblePrim2(work->base_cloud[0]);
		DG_InvisiblePrim2(work->base_cloud[1]);
		DG_InvisiblePrim2(work->base_cloud[2]);
		DG_InvisiblePrim2(work->base_cloud[3]);
		if(work->objs[0]) DG_InvisibleObjs( work->objs[0] );
		if(work->objs[1]) DG_InvisibleObjs( work->objs[1] );
		if(work->objs[2]) DG_InvisibleObjs( work->objs[2] );
		if(work->objs[3]) DG_InvisibleObjs( work->objs[3] );
		return;
	}else if(work->flags & 0x0002){
		DG_VisiblePrim2(work->base_prim);
		if(work->objs[0]) DG_VisibleObjs( work->objs[0] );
		if(work->objs[1]) DG_VisibleObjs( work->objs[1] );
		if(work->objs[2]) DG_VisibleObjs( work->objs[2] );
		if(work->objs[3]) DG_VisibleObjs( work->objs[3] );
		DG_InvisiblePrim2(work->base_cloud[0]);
		DG_InvisiblePrim2(work->base_cloud[1]);
		DG_InvisiblePrim2(work->base_cloud[2]);
		DG_InvisiblePrim2(work->base_cloud[3]);
	}else{
		DG_VisiblePrim2(work->base_prim);
		DG_VisiblePrim2(work->base_cloud[0]);
		DG_VisiblePrim2(work->base_cloud[1]);
		DG_VisiblePrim2(work->base_cloud[2]);
		DG_VisiblePrim2(work->base_cloud[3]);
		if(work->objs[0]) DG_VisibleObjs( work->objs[0] );
		if(work->objs[1]) DG_VisibleObjs( work->objs[1] );
		if(work->objs[2]) DG_VisibleObjs( work->objs[2] );
		if(work->objs[3]) DG_VisibleObjs( work->objs[3] );
	}
	
	work->flags = (work->flags&0xffff)|(work->color.cd<<16);
	DG_SwitchBuffPrim2( base_prim );
	clock = base_prim->buffer_clock;

	SetPirmRGB( base_prim, N_ALLVERTS, work->color );

	SkyBottomSizeChange( base_prim, work->bound[0], work->sky_bottom );

	
	
//	DG_COPY_MAT(&world,&DG_UnitMatrix);
//	DG_COPY_VEC((FVECTOR*)world.m[3],&work->center);
	
	MoveBaseCloud( work, PI/65536.0f*1.0f * work->cloud_speed[4]/100.0f, 0 );
	MoveBaseCloud( work, PI/65536.0f*2.0f * work->cloud_speed[5]/100.0f, 1 );
	MoveBaseCloud( work, PI/65536.0f*3.0f * work->cloud_speed[6]/100.0f, 2 );
	MoveBaseCloud( work, PI/65536.0f*4.0f * work->cloud_speed[7]/100.0f, 3 );

	for( i = 0; i < N_MAX_MODLS; i++ ){
		float		angle = RootRot[i] + work->rot_y[i];
		while( angle > PI ) angle -= 2.0f*PI;
		while( angle < -PI ) angle += 2.0f*PI;
		
		_sceVu0RotMatrixY( &work->obj_root[i], &work->root, angle );
		
		_sceVu0ScaleVector( work->obj_root[i].m[0], work->obj_root[i].m[0], SKY_SCALE );
		_sceVu0ScaleVector( work->obj_root[i].m[1], work->obj_root[i].m[1], SKY_SCALE );
		_sceVu0ScaleVector( work->obj_root[i].m[2], work->obj_root[i].m[2], SKY_SCALE );
		DG_COPY_VEC( work->obj_root[i].m[3], work->root.m[3] );

		work->lights[i][1].m[3][0] = work->obj_color[i].r;
		work->lights[i][1].m[3][1] = work->obj_color[i].g;
		work->lights[i][1].m[3][2] = work->obj_color[i].b;

		
		work->rot_y[i] += PI/65536.0f * work->cloud_speed[i]/100.0f;
		while( work->rot_y[i] > PI ) work->rot_y[i] -= 2.0f*PI;
		while( work->rot_y[i] < -PI ) work->rot_y[i] += 2.0f*PI;

		//DG_SetPos2( (FVECTOR*)work->root.m[3], &work->rot[i] );
		//DG_GetPos( &work->obj_root[i] );
		//DG_COPY_MAT( &work->obj_root[i], &work->root );

	}
	for( i = 0; i < N_PRIMS; i++ ){
		//NewBoundingBoxView(&work->bound[i][0],&work->bound[i][1],128,32,32);
		flags = DG_BoundCheck(&work->root,&work->bound[i][0],&work->bound[i][1]);
		//printf("invis %d[%x]\n",i,flags);
		base_prim->packet[clock][i].flag = base_prim->packet[clock][i].flag & ~(0x03<<12);
		base_prim->packet[clock][i].flag |= (flags&0x3)<<12;
	}
	

}

static void Die(Work *work)
{
	int i;
	if(work->base_prim)GM_FreePrim2(work->base_prim);
	if(work->base_cloud[0])GM_FreePrim2(work->base_cloud[0]);
	if(work->base_cloud[1])GM_FreePrim2(work->base_cloud[1]);
	if(work->base_cloud[2])GM_FreePrim2(work->base_cloud[2]);
	if(work->base_cloud[3])GM_FreePrim2(work->base_cloud[3]);
	for( i = 0; i < N_MAX_MODLS; i++ ){
		if(work->objs[i]){
			DG_DequeueObjs( work->objs[i] );
			DG_FreeObjs( work->objs[i] );
		}
	}
	PrevSkyWork = NULL;
}


static FVECTOR InitStripVertsPos[] = {
	{     0.000f, 10000.000f,    0.000f, 1.0f },
	{  7071.068f,  8828.727f,    0.000f, 1.0f },
	{ 10000.000f,  6000.000f,    0.000f, 1.0f },

	{ 10000.000f,     0.000f,    0.000f, 1.0f },	
};

static float WhichMax3(float a, float b, float c)
{
	if(a > b){
		if(a > c) return a;
		else return c;
	}else{
		if(b > c) return b;
		else return c;
	}
}

static float WhichMin3(float a, float b, float c)
{
	if(a < b){
		if(a < c) return a;
		else return c;
	}else{
		if(b < c) return b;
		else return c;
	}
}
static DG_PRIM2 *InitBaseCloud( Work *work, float len, float off_y, float len_y,
								short rot_vy, int num, int tex_code,
								int color,  int alpha, int pri )
{
	DG_TEX				*tex;
	DG_PRIM2			*prim;
	FVECTOR				*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB		*uvrgb = MEM_SCR_UV;
	SVECTOR				rot = { 0, rot_vy, 0, 0};
	int					i,j;

	FVECTOR				InitPos[2] = {
		{ 0.0f, off_y*SKY_SCALE+len_y*SKY_SCALE, len*SKY_SCALE, 1.0f },
		{ 0.0f, off_y*SKY_SCALE, len*SKY_SCALE, 1.0f },
	};

//	GV_StrCode("sky_02_add_alp")
	tex = DG_GetTexture(tex_code);
	if(!tex) return NULL;
	prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE|DG_PRIM2_TEX,
						 1, 15*2 );
	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sky.c>\n"); return (NULL); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	prim->raise = -pri * 10 * (int)SKY_SCALE;
	prim->root = &work->root;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	for(i = 0; i < 3; i++ ){
		for(j = 0; j < 5; j++ ){
			if(j==4 && i == 2) rot.vy = rot_vy;
			DG_SetPos2(&work->center,&rot);
			DG_PutVector(InitPos,pos,2);
			DG_RotVector(InitPos,&work->base_cloud_pos[num][(i*5+j)*2],2);
			
			if(j!=4) rot.vy += (short)(4096.0f/12.0f);
			
			uvrgb[0].u = FTOI12( 1.0f/4.0f * (float)j * tex->u_scale + tex->u_offset );
			uvrgb[0].v = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
			uvrgb[0].f = (j)?DRAW_KICK_CODE:VERT_KICK_CODE;
			uvrgb[0].q = 4096;
			uvrgb[0].r = color&0xff;//CLOUD_COLOR;
			uvrgb[0].g = (color>>8)&0xff;//CLOUD_COLOR;
			uvrgb[0].b = (color>>16)&0xff;//CLOUD_COLOR;
			uvrgb[0].a = alpha;//CLOUD_ALPHA;

			uvrgb[1].u = FTOI12( 1.0f/4.0f * (float)j * tex->u_scale + tex->u_offset );
			uvrgb[1].v = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
			uvrgb[1].f = (j)?DRAW_KICK_CODE:VERT_KICK_CODE;
			uvrgb[1].q = 4096;
			uvrgb[1].r = color;//CLOUD_COLOR;
			uvrgb[1].g = color;//CLOUD_COLOR;
			uvrgb[1].b = color;//CLOUD_COLOR;
			uvrgb[1].a = alpha;//CLOUD_ALPHA;
		
			uvrgb+=2;
			pos+=2;
		}
	}

			
	work->base_cloud_angle[num] = 0.0f;//PI*(float)rot_vy/2048.0f;
		
//	TS_Scr_Mem( work->base_cloud_pos[num], MEM_SCR_POS, sizeof(FVECTOR), 30 );
	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), 30 );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), 30 );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), 30 );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), 30 );
	
//	prim->pos[0][ 30 - 2 ] = prim->pos[0][0];
//	prim->pos[0][ 30 - 1 ] = prim->pos[0][1];
//	prim->pos[1][ 30 - 2 ] = prim->pos[1][0];
//	prim->pos[1][ 30 - 1 ] = prim->pos[1][1];

	return prim;
}

static int InitBasePrim( Work *work )
{
	DG_TEX				*tex;
	DG_PRIM2			*prim;
	FVECTOR				*pos = MEM_SCR_POS;
	DG_PRIM2_UVRGB		*uvrgb = MEM_SCR_UV;
	int					i;//,j;
	FMATRIX				world;
	FVECTOR				*fvtemp = MEM_SCR_TMP;
	SVECTOR				rot = { 0, 0, 0, 0};
	
	tex = DG_GetTexture(0);
	prim = work->base_prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_ALPHA|DG_PRIM2_SHADE,
										   N_PRIMS, N_VERTS );

	if(!prim){ printf("ERR!! MAKE PRIM!! <plant_sky.c>\n"); return (-1); }
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );
	prim->root = &work->root;
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	prim->raise = -1000000;
	//天井のストリップ
	for( i = 0; i < N_STRIP_VERTS; i++ ){
		uvrgb->u = 0;
		uvrgb->v = 0;
		uvrgb->f = (i<2 || i==5 || i==6)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb->q = 4096;

		uvrgb->r = work->color.r;//SKY_COLOR_R;
		uvrgb->g = work->color.g;//SKY_COLOR_G;
		uvrgb->b = work->color.b;//SKY_COLOR_B;

//		uvrgb->a = 130 - 26*((i+1)>>1);
		uvrgb->a = 128;
//printf("%d\n",uvrgb->a);
		uvrgb++;
	}
	uvrgb = MEM_SCR_UV;
	for( i = 0; i < 4; i++ ){
		//*uvrgb = *(DG_PRIM2_UVRGB*)MEM_SCR_UV;
		memcpy(uvrgb,MEM_SCR_UV,sizeof(DG_PRIM2_UVRGB)*N_STRIP_VERTS);

		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp={SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f} ;
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[0], &fvtemp[0], 3 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp={SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f} ;
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[1], &fvtemp[3], 2 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp={SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f} ;
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[1], &fvtemp[5], 2 );
		
		DG_COPY_VEC(&pos[0],&fvtemp[0]);
		DG_COPY_VEC(&pos[1],&fvtemp[5]);
		DG_COPY_VEC(&pos[2],&fvtemp[1]);
		DG_COPY_VEC(&pos[3],&fvtemp[4]);
		DG_COPY_VEC(&pos[4],&fvtemp[2]);
		DG_COPY_VEC(&pos[5],&fvtemp[5]);
		DG_COPY_VEC(&pos[6],&fvtemp[4]);
		DG_COPY_VEC(&pos[7],&fvtemp[6]);

		//bound max
		work->bound[i][0].vx = WhichMax3(fvtemp[2].vx,fvtemp[6].vx,fvtemp[0].vx);
		work->bound[i][0].vy = fvtemp[0].vy;
		work->bound[i][0].vz = WhichMax3(fvtemp[2].vz,fvtemp[6].vz,fvtemp[0].vz);
		work->bound[i][0].vw = 1.0f;

		work->bound[i][1].vx = WhichMin3(fvtemp[2].vx,fvtemp[6].vx,fvtemp[0].vx);
		work->bound[i][1].vy = fvtemp[4].vy;
		work->bound[i][1].vz = WhichMin3(fvtemp[2].vz,fvtemp[6].vz,fvtemp[0].vz);
		work->bound[i][1].vw = 1.0f;


		uvrgb += N_STRIP_VERTS;
		pos += N_STRIP_VERTS;
	}
	
	//壁ストリップ
	for( i = 0; i < N_STRIP_VERTS; i++ ){
		uvrgb->u = 0;
		uvrgb->v = 0;
		uvrgb->f = (i<2)?VERT_KICK_CODE:DRAW_KICK_CODE;
		uvrgb->q = 4096;

		uvrgb->r = work->color.r;//SKY_COLOR_R;
		uvrgb->g = work->color.g;//SKY_COLOR_G;
		uvrgb->b = work->color.b;//SKY_COLOR_B;

		uvrgb->a = (i>=2)?128:0;//(short)(128.0f - 128.0f*(float)(i>>1)/3.0f);
		//uvrgb->a = 32;
		//printf("%d\n",uvrgb->a);
		uvrgb++;
	}
	
	uvrgb -= N_STRIP_VERTS;
	
	rot.vy = 0;
	
	for( i = 0; i < 8; i++ ){
		//*uvrgb = *(DG_PRIM2_UVRGB*)MEM_SCR_UV;
		memcpy(uvrgb,&((DG_PRIM2_UVRGB*)MEM_SCR_UV)[4*N_STRIP_VERTS],sizeof(DG_PRIM2_UVRGB)*N_STRIP_VERTS);

		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp={SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f} ;
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[2], &fvtemp[0], 2 );
		rot.vy += 4096/8;
		DG_SetPos2(&work->center,&rot);
		DG_GetPos(&world);
#ifdef BP_PS2
		TS_ScaleMatrix( &world, &world, &(FVECTOR){SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f});
#else
		{
			FVECTOR tmp={SKY_SCALE,SKY_SCALE,SKY_SCALE,0.0f} ;
			TS_ScaleMatrix( &world, &world, &tmp );
		}
#endif
		DG_SetPos(&world);
		DG_PutVector( &InitStripVertsPos[2], &fvtemp[2], 2 );

		DG_COPY_VEC(&pos[0],&fvtemp[1]);	//bottom
		DG_COPY_VEC(&pos[1],&fvtemp[3]);	//bottom

		DG_COPY_VEC(&pos[2],&fvtemp[0]);
		DG_COPY_VEC(&pos[3],&fvtemp[2]);
		DG_COPY_VEC(&pos[4],&fvtemp[0]);
		DG_COPY_VEC(&pos[5],&fvtemp[2]);
		DG_COPY_VEC(&pos[6],&fvtemp[0]);
		DG_COPY_VEC(&pos[7],&fvtemp[2]);

		//bound max
		work->bound[i+4][0].vx = (fvtemp[0].vx>fvtemp[2].vx)?fvtemp[0].vx:fvtemp[2].vx;
		work->bound[i+4][0].vy = fvtemp[1].vy;
		work->bound[i+4][0].vz = (fvtemp[0].vz>fvtemp[2].vz)?fvtemp[0].vz:fvtemp[2].vz;
		work->bound[i+4][0].vw = 1.0f;
		
		work->bound[i+4][1].vx = (fvtemp[0].vx>fvtemp[2].vx)?fvtemp[2].vx:fvtemp[0].vx;
		work->bound[i+4][1].vy = fvtemp[0].vy;
		work->bound[i+4][1].vz = (fvtemp[0].vz>fvtemp[2].vz)?fvtemp[2].vz:fvtemp[0].vz;
		work->bound[i+4][1].vw = 1.0f;
		
		
		uvrgb += N_STRIP_VERTS;
		pos += N_STRIP_VERTS;
	}
	
	TS_Scr_Mem( prim->pos[0], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( prim->pos[1], MEM_SCR_POS, sizeof(FVECTOR), N_ALLVERTS );
	TS_Scr_Mem( prim->uvrgb[0], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );
	TS_Scr_Mem( prim->uvrgb[1], MEM_SCR_UV, sizeof(DG_PRIM2_UVRGB), N_ALLVERTS );

	return 0;
}

static void GetOptions( Work *work )
{
	work->color.r = SKY_COLOR_R;
	work->color.g = SKY_COLOR_G;
	work->color.b = SKY_COLOR_B;
	work->color.cd = 18;

	work->center.vx = 0.0f;
	work->center.vy = 0.0f;
	work->center.vz = 0.0f;
	work->center.vw = 1.0f;
	work->sky_bottom = 6000.0f;
}

static DG_OBJS *InitItemObjs( int code, FMATRIX *root, FMATRIX *light )
{
	DG_DEF		*def;
	DG_OBJS		*objs;

	def = (DG_DEF*)GV_GetCache( GV_CacheID( code, 'k' ) ) ;
	if(!def){ return NULL; }
	objs = DG_MakeObjs( def, DG_FLAG_NOFOG, 0 );
	if(!objs) return NULL;
	if(DG_QueueObjs( objs )<0) return NULL;
	if(root) objs->root = root;
	
	if(light) DG_SetLightMatrix( objs, light );
	return objs;
}

static int GetResources( Work *work )
{
	int		i;

	GetOptions( work );

	work->w = 240.0f;
	work->h = 240.0f;

	work->cloud_speed[0] = 100.0f;
	work->cloud_speed[1] = 100.0f;
	work->cloud_speed[2] = 100.0f;
	work->cloud_speed[3] = 100.0f;
	work->cloud_speed[4] = 100.0f;
	work->cloud_speed[5] = 100.0f;
	work->cloud_speed[6] = 100.0f;
	work->cloud_speed[7] = 100.0f;

	if(InitBasePrim( work )) return -1;
	work->flags = 0x00120001;
	DG_COPY_MAT( &work->root, &DG_UnitMatrix );

	work->obj_color[0+4].r = 192;
	work->obj_color[0+4].g = 192;
	work->obj_color[0+4].b = 192;
	work->obj_color[1+4].r = 224;
	work->obj_color[1+4].g = 224;
	work->obj_color[1+4].b = 224;
	work->obj_color[2+4].r = CLOUD_COLOR;
	work->obj_color[2+4].g = CLOUD_COLOR;
	work->obj_color[2+4].b = CLOUD_COLOR;
	work->obj_color[3+4].r = CLOUD_COLOR;
	work->obj_color[3+4].g = CLOUD_COLOR;
	work->obj_color[3+4].b = CLOUD_COLOR;

	work->base_cloud[0] = InitBaseCloud( work, 9600.0f, 500.0f, 6000.0f, 0*1024/4,
										 0, GV_StrCode("sky_1_alp"),
										 *(int*)&work->obj_color[0+4], 96, 0 );
	work->base_cloud[1] = InitBaseCloud( work, 8800.0f, 500.0f, 6000.0f, 1*1024/4,
										 1, GV_StrCode("sky_2_alp"),
										 *(int*)&work->obj_color[1+4], 96, 100 );
	work->base_cloud[2] = InitBaseCloud( work, 8000.0f, 300.0f, 6000.0f, 2*1024/4,
										 2, GV_StrCode("sky_3_alp"),
										 *(int*)&work->obj_color[2+4], CLOUD_ALPHA, 200 );
	work->base_cloud[3] = InitBaseCloud( work, 7200.0f, 300.0f, 6000.0f, 3*1024/4,
										 3, GV_StrCode("sky_4_alp"),
										 *(int*)&work->obj_color[3+4], CLOUD_ALPHA, 300 );

	work->tex_num = 1;
	{
		extern void *NewPrevSun( FMATRIX* root, int *flags, float*, float* );
		void *temp = NewPrevSun( &work->root, &work->flags, &work->w, &work->h );
		if(temp) GV_SetActorChild(work,temp);
	}
	work->mdl_num = 0;
	for( i = 0; i < N_MAX_MODLS; i++ ){
		work->objs[i] = InitItemObjs( GV_StrCode(Modl_Name[i]), &work->obj_root[i], work->lights[i] );
		if( !work->objs[i] )printf("%s が無い\n",Modl_Name[i] );
		memset( work->lights[i], 0, sizeof(FMATRIX)*2 );
		
		work->lights[i][1].m[3][0] = 128.0f;
		work->lights[i][1].m[3][1] = 128.0f;
		work->lights[i][1].m[3][2] = 128.0f;

		work->obj_color[i].r = 128;
		work->obj_color[i].g = 128;
		work->obj_color[i].b = 128;
	}

	
	return 0;
}

void *NewPrevSky()
{
	Work *work = NULL;
	if(PrevSkyWork) return NULL;
	work = (Work*)GV_NewEffect(GV_ACTOR_EFFECT,sizeof(Work));
	if(work){
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor );
		
		if(GetResources( work ) < 0){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		printf("new plant_sky.c\n");
		PrevSkyWork = work;
	}

	return (void *)work ;
}

