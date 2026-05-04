/*
	prefog.h
		プレビュー用のフォグ管理機構

	1999/12/10 K.Kano
	$Id: prefog.h,v 1.1.1.3 2002/11/19 11:43:30 Yoshizawa1 Exp $
*/


#ifndef _prefog_h_
#define _prefog_h_


#define	AMB_DEFAULT_R	(64.0f)
#define	AMB_DEFAULT_G	(64.0f)
#define	AMB_DEFAULT_B	(64.0f)

#define	PLL_DEFAULT_R	(128.0f)
#define	PLL_DEFAULT_G	(128.0f)
#define	PLL_DEFAULT_B	(128.0f)
#define	PLL_DEFAULT_X	(512.0f)
#define	PLL_DEFAULT_Y	(2048.0f + 512.0f)
#define	PLL_DEFAULT_Z	(0.0f)

#define	FOG_DEFAULT_R	(128)
#define	FOG_DEFAULT_G	(128)
#define	FOG_DEFAULT_B	(128)
#define	FOG_DEFAULT_N	(0.0f)
#define	FOG_DEFAULT_F	(60000.0f)


typedef struct {
    FVECTOR	col_dir ;
    FVECTOR	col_lit ;

    FVECTOR	amb_lit ;

    FVECTOR	fog_color ;
    float	fog_near;
    float	fog_far;
    
	DG_OBJS	*yajirushi_objs ;
    FMATRIX	yajirushi_lights[ 2 ] ;

    FVECTOR	forVRSlit;

    int		parallel_cursole_pos;
    int		ambient_cursole_pos;
    int		fog_cursole_pos;
    int		file_cursole_pos;
    int		lt2_select;

    int		speed_mode;

	int		where;
} PreviewLight_Param;


typedef struct {
    FVECTOR	col_dir ;
    FVECTOR	col_lit ;

    FVECTOR	amb_lit ;

    FVECTOR	fog_color ;
    float	fog_near;
    float	fog_far;
} LightSaveFormat;


#ifdef _WORK_DECLARED_
PreviewLight_Param PreviewLight;
#else
extern PreviewLight_Param PreviewLight;
#endif


void CopyLightFromSavework(void);
void CopyLightToSavework(void);

void ForVRSlight(void);
void Parallel_DebugCursole(void);
void Parallel_DebugPrint(void);

void Ambient_DebugCursole(void);
void Ambient_DebugPrint(void);

void Fog_DebugCursole(void);
void Fog_DebugPrint(void);

void Preview_LightAct(void);

void SetStageLight(void);
void Preview_GetNowLightData(void);
void Preview_SetDefaultLightData(void);

void File_DebugPrint(void);
void File_DebugCursole(void);

void *NewPreviewLight(void);


#endif
