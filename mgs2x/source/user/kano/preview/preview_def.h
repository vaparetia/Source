/*

	preview_def.h
	デザイナープレビュー環境用：カメラ
	1999/07/07 S.Okajima
	2000/02/08 K.Kano Modified
	$Id: preview_def.h,v 1.1.1.3 2002/11/19 11:43:31 Yoshizawa1 Exp $

*/


#ifndef _preview_def_h_
#define _preview_def_h_


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include	"libutl.h"
#include	"libfs.h"

#include	"camera.h"

#include	"prekey.h"
#include	"prefile.h"
#include	"human_preview.h"
#include	"va_preview.h"
#include	"human.h"
#include	"object.h"
#include	"premove.h"
#include	"fobj.h"
#include	"precamera.h"
#include	"prefog.h"
#include	"pretex.h"
#include	"preparticle.h"


#define	LOCATE_X	0x10
#define	LOCATE_Y	0x20

#define FONT_WIDTH		9
#define FONT_HEIGHT		17

#define COLUMN_HEIGHT	FONT_HEIGHT

#define	MAX_FILE_NUM		(1024)
#define	FILE_NAME_LEN		(32)

#define	WIDTH		(256)
#define	HEIGHT		(128)


#define	SPEEDUP_1	(40)
//#define	SPEEDUP_2	(180)
#define	SPEEDUP_2	(600)

#define	DATA_LENGTH	(0x4000)


#define	MAX_HUMAN	(16)
#define	MAX_OBJECTS			(256)


#define DUMMY_KMS_FILE		"dummy000"


/* 旧型 */
typedef	struct	{
    int		mode_0;
    int		mode_1;
    int		mode_2;
    int		mode_3;

    FVECTOR	col_dir ;
    FVECTOR	col_lit ;

    FVECTOR	amb_lit ;

    int		cam_length ;
    FVECTOR	ftrg_pos ;
    SVECTOR	cam_rot ;
    FVECTOR	fcam_pos ;
    float	zoom;

    SVECTOR	fog_color ;
    float	fog_near;
    float	fog_far;

    int		tex_list_num1;
    int		tex_list_num2;

    FMATRIX	yajirushi_lights[ 2 ] ;

    int		motion_num;
    int		motion_interp;

    int		pos_fix_flag;
    FVECTOR	pos_fix_before[MAX_HUMAN];

    int		objects_now_num;
    int		objects_parm1[MAX_OBJECTS];
    int		objects_parm2[MAX_OBJECTS];
    int		objects_parm3[MAX_OBJECTS];
    int		objects_parm4[MAX_OBJECTS];
    FVECTOR	objects_mov[MAX_OBJECTS];
    SVECTOR	objects_rot[MAX_OBJECTS];
} SAVE_AREA ;

/* 新型 */
#define HUMAN_SAVE_SIZE		32
#define OBJECT_SAVE_SIZE	128
#define FOBJ_SAVE_SIZE		32

typedef struct {
	LightSaveFormat light;
	CameraSaveFormat camera;
	HumanSaveFormat human[HUMAN_SAVE_SIZE];
	ObjectSaveFormat object[OBJECT_SAVE_SIZE];
	FobjSaveFormat fobj[FOBJ_SAVE_SIZE];
} SaveFormat;


#ifdef _WORK_DECLARED_
SaveFormat SaveData;
#else
extern SaveFormat SaveData;
#endif


typedef struct {
	int font_color;
	int mode;
	int cursole;
} PreviewMainMenu_Param;


#ifdef _WORK_DECLARED_
PreviewMainMenu_Param MainMenuParam;
#else
extern PreviewMainMenu_Param MainMenuParam;
#endif


#endif
