/*
   camera.h
   カメラシステムヘッダ

   1999/07/13 M.Sonoyama
   $Id: camera.h,v 1.1.1.3 2002/11/19 11:41:46 Yoshizawa1 Exp $
*/

#ifndef __camera_h__
#define	__camera_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "BP_Camera.h"  //BP_CAMERA


#define	MAX_CHANLS	(2)
#define	MAX_CAMERAS	(32)

/* Kameraakutareberu */
enum {
	CAMERA_PRIO = 128,
	CAMERA_ZOOM_CTRL_PRIO = 160,
	CAMERA_ZFOCUS_CTRL_PRIO = 196,
	CAMERA_DAEMON_PRIO = GV_PRIO_MAX - 8,
} ;

#define	CAMERA_DEBUG_PRIO	(CAMERA_DAEMON_PRIO - 1)

/* Camera priority */
enum {
    GM_CAMERA_CUT_IN = 0,	/* Cut-in */
    GM_CAMERA_PROG1,		/* 1 Program Camera */
    GM_CAMERA_SUBJECT,		/* Subjective Players */
    GM_CAMERA_PROG2,		/* 2 Camera Program */
    GM_CAMERA_BEHIND,		/* Bihaindokamera */
    GM_CAMERA_PROG3,		/* Camera 3 Program */
    GM_CAMERA_AREA,			/* Specify the camera area (scenario) */
    GM_CAMERA_PROG4,		/* 4 Camera Program */
    GM_CAMERA_DEFAULT,		/* Sutejideforutokamera */
    GM_CAMERA_MAX
} ;

/* Cameras */
enum {	
    /* Be one of the three */
    /* Specify the camera position and target position */
    GM_CAM_TYPE_CAMERA_AND_TARGET = 0x0001,
    /* Specify and track the target position and rotation */
    GM_CAM_TYPE_TARGET_AND_ROTATE = 0x0002,
    /* Specify and track the camera position and rotation */
    GM_CAM_TYPE_CAMERA_AND_ROTATE = 0x0004,
} ;

/* Camera system complete*/
enum {
    GM_CAM_INTERP_NO_SET = -1,
    GM_CAM_INTERP_QUICK, /* Change in an instant */
    GM_CAM_INTERP_EXP4,  /*Completion Type 1 */
    GM_CAM_INTERP_EXP8,  /* Type 2 Completion */
    GM_CAM_INTERP_EXP16,  /* Type 3 supplement */
    GM_CAM_INTERP_LINEAR, /* Velocity at the time specified completion */
    GM_CAM_INTERP_INTO_SUBJECT, /* Special mode for the subjective */
    GM_CAM_INTERP_OUT_SUBJECT, /* Special mode for the subjective*/
    GM_CAM_INTERP_INTO_BEHIND,
    GM_CAM_INTERP_BEHIND,
    GM_CAM_INTERP_INTO_BEHIND2,
    GM_CAM_INTERP_MAX
} ;

#define	TIME_INTO_SUBJECT	(18)
#define	TIME_OUT_SUBJECT	(18)

/* Priority */
#define	GM_CAMERA_PRIO_MAX	(255)

/* Flag */
enum {
    GM_FLAG_CAMERA_CHANGED = 0x0001, /* Cameras have changed */
    GM_FLAG_CAMERA_NO_USE = 0x0010,  /* Use / nonuse */
    GM_FLAG_CAMERA_INTERP = 0x0100,  /* Completion of */
    GM_FLAG_CAMERA_FIRST = 0x1000,   /* Have not set the initial value */
    GM_FLAG_CAMERA_QUICK = 0x2000,   /* If it is not complete, no camera movement completion*/
	GM_FLAG_CAMERA_NO_SWING = 0x4000,	/* No vertical vibration */
	GM_FLAG_CAMERA_SLIDECAMDIR_REVERSE = 0x8000,	/* Slide the camera reverses direction to pass*/
} ;

/* Flag */
enum {
    /* Following in the track & rotate GM_CameraTarget */
    CAM_FLAG_TRACE = 0x0001,		
    /* camera & target ) Fixed */
    CAM_FLAG_FIX   = 0x0002,		
    /* The camera tracks the GM_CameraTarget */
    CAM_FLAG_FIX_TRACE   = 0x0004,	
    /* The camera angle X only tracks the GM_CameraTarget */
    CAM_FLAG_FIX_X_TRACE   = 0x0008,
    
    CAM_FLAG_BOUND = 0x0010,			/* Bound In Ri */
    CAM_FLAG_LIMIT = 0x0020,			/* Ri Jae Limit */

    CAM_FLAG_FIX_TARGET_TRACE = 0x0040, /* Fixed targets, follow- */
	CAM_FLAG_YROT_FIX_TRACE = 0x0080,	/* Positioning, Y follow a fixed direction */

    CAM_FLAG_DIR_TRACE = 0x0100,		/* Using GM_CameraDir */
    CAM_FLAG_PAD_ADJUST = 0x0200,		/* Camera angle to be reflected in the pad */
    CAM_FLAG_PAD_ADJUST_NO_RESET = 0x0400,	/* Save Adjustments pad (behind only)） */
    
    CAM_FLAG_PAD_ADJUST_BP_3RD_PERSON = 0x0800,	   /* BP_CAMERA - Use BP 3rd person camera */

    CAM_FLAG_KILL = 0x1000,				/* End */
    CAM_FLAG_SET = 0x2000,				/* The value is set at a time */
    CAM_FLAG_ON_NOW = 0x4000,			/* I was ON */
    CAM_FLAG_CHANGED = 0x8000,			/* Camera has been changed */

    CAM_FLAG_CUT_IN  = 0x00010000,		/* Attribute the Kattoinkamera */

    CAM_FLAG_ZROTATE_AVAILABLE = 0x00040000,	/* Z in the rotation */
    CAM_FLAG_NO_CUSHION_X = 0x00100000,			/* No Fun X */
    CAM_FLAG_NO_CUSHION_Y = 0x00200000,			/* Y no play */
    CAM_FLAG_NO_CUSHION_Z = 0x00400000,			/* No play Z */

	CAM_FLAG_BOUNDCHANGE_QUICK = 0x01000000,	/* Bound limit for an immediate change */
	CAM_FLAG_INTERP_CAMERA_SET	=	 0x02000000,	

	CAM_FLAG_NO_BLASTCAMERA =		0x04000000,	/* No explosions shaking the camera */
	CAM_FLAG_LIMITBOUND_EXIST =		0x08000000,	/* But there is a limit FIX */

	CAM_FLAG_PAUSE_NO_STOP = 0x10000000,		/* Also stop posing*/
	CAM_FLAG_FINISHCALC = 0x20000000,			/* Calculation of the camera is close all */
	CAM_FLAG_DELETED = 0x40000000,				/* Erased */
} ;

#define	CAM_FLAG_NO_CUSHION (CAM_FLAG_NO_CUSHION_X|CAM_FLAG_NO_CUSHION_Y| \
			     CAM_FLAG_NO_CUSHION_Z)


/* Camera configuration structure */
typedef	ALIGN16_DECL(struct) _GM_CameraSet {
    FVECTOR		position ;	/* Camera position */
    FVECTOR		target ;	/* Target position */

    float		angle ;		/* Angle */
    int			type ;		/* Cameras */
    u_int      		priority ;	/* Priority */
    u_int		on ;		/* Use / nonuse */

    struct _GM_CameraSet *next ;	/* Next Cameras */
    void	       	 *interp_func1 ; /* Completion function (input)*/
    void		 *interp_func2 ; /* Completion function (out) */
    u_short	      	interp_time1 ;	/* Completion Time */
    u_short		interp_time2 ;	/* Completion Time */

    /* Now keep together */
    /* Some cameras do not require, or even by */
    FVECTOR		bound1, bound2 ;
    FVECTOR		limit1, limit2 ;
    SVECTOR		rotate ;
    int			track ;
    u_int		name ;

    int			flag ;
    int			chanl ;
    int			level ;
    int			off_time ;

    u_char		slide_type ;
    u_char		slide_wait_count ;
    u_char		slide_wait1, slide_wait2 ;

    float		slide1, slide2 ; 
    float		slide_step ;
    float		slide_interp ;

    int			slide_step_int ;
    int			slide_track ;

	struct	_GM_CameraSet	*interp_camera_aim ;
	int				interp_camera_time ;


    SVECTOR		slide_rot ;

   //BP_CAMERA - extra BP tweakables for HD
   SBP_CameraSettings bp_settings;
   //BP_CAMERA - extra BP tweakables for HD

} GM_CameraSet  ;


typedef	void (*GM_CameraMatrixFunc)( DG_CHANL *chanl, FVECTOR *pos, FVECTOR *trg, float angle ) ;

/* The camera system management structures */
typedef struct _CameraChanl {
    GM_CameraSet	list[ GM_CAMERA_MAX ] ;
    GM_CameraSet	current ;	/* The current camera settings */
    GM_CameraSet	master ;	/* Externally adjusted camera settings (the camera actually used) */
    GM_CameraSet	*next ;		/* The following camera settings */
    void		*interp_func ;	/* Functions for completion*/
    int			time ;		/* The current completion time */
    u_int		flag ;		/* Flag */
    GM_CameraMatrixFunc set_mat ; /* Default DG_SetCamera2. People change is calculated by taking the responsibility to me! ! */
} GM_CameraChanl ;

/*---------------------------------------------------------------*/

typedef	struct {
    GM_CameraSet	camera ;
    FVECTOR		bound1, bound2 ;
    FVECTOR		limit1, limit2 ;
    SVECTOR		rotate ;
    int			track ;
    u_int		name ;

    int			flag ;
    int			chanl ;
    int			level ;
    int			reserved ;
} GM_CameraSet2 ;

/*---------------------------------------------------------------*/

typedef	struct	{
    GV_ACT		actor ;
    GM_CameraChanl	chanl[ MAX_CHANLS ] ;
} GM_CameraWork ;

/*---------------------------------------------------------------*/

/* For Direikamera */
extern	void	*GM_DelayCameraWork ;
extern	int	GM_CameraDelayTime[] ;

/* Completion function */
typedef	void	( *INTERP_FUNC )( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;

extern	GM_CameraWork	*GM_Camera ;
extern	FVECTOR		GM_CameraTargetValue[] ;
extern	SVECTOR		GM_CameraDirValue[] ;
extern	SVECTOR		GM_CameraRotateValue[] ;
extern	float		GM_CameraAngleValue[] ;
extern	int		GM_CameraTrackValue[] ;
extern	int		GM_CameraModeValue[] ;
extern	FVECTOR		*GM_CameraTargetPtr[] ;

extern	float		GM_CameraAmplitudeValue[] ;
extern	float		GM_CameraCycleValue[] ;
extern	float		GM_CameraAmplitudeValueDef[] ;
extern	float		GM_CameraCycleValueDef[] ;
extern	int		GM_CameraExclamationTime ;

extern	int		GM_CameraAdjustExist[] ;
extern	FVECTOR		GM_CameraAdjust[] ;

extern	GM_CameraSet	*GM_PreviousCamera ;

#define	GM_CameraTarget	GM_CameraTargetValue[ 0 ] 
#define	GM_CameraDir	GM_CameraDirValue[ 0 ] 
#define	GM_CameraRotate	GM_CameraRotateValue[ 0 ] 
#define	GM_CameraAngle	GM_CameraAngleValue[ 0 ] 
#define	GM_CameraTrack	GM_CameraTrackValue[ 0 ] 
#define	GM_CameraMode	GM_CameraModeValue[ 0 ] 
#define	GM_CameraAmplitude	GM_CameraAmplitudeValue[ 0 ] 
#define	GM_CameraCycle	GM_CameraCycleValue[ 0 ] 

/*---------------------------------------------------------------*/

/* game/cam_util.c */
extern void	GM_EntryCamera( GM_CameraSet *, int, int, int, int ) ;
extern void	GM_DeleteCamera( GM_CameraSet * ) ;
extern void	GM_SetCameraInterpMode( GM_CameraSet *, int, int, int, int ) ;
extern void	GM_InitCameraSet( GM_CameraSet *, FVECTOR *, FVECTOR *, float, int, int ) ;
extern void	GM_CameraBoundTrace( FVECTOR *, FVECTOR *, FVECTOR * ) ;
extern void	GM_CameraCushionTrace( FVECTOR *, FVECTOR *, FVECTOR * ) ;
extern void	GM_CameraMakeRotate( FVECTOR *, FVECTOR *, SVECTOR *, int * ) ;
extern void	GM_CameraMakeRotate2( FVECTOR *, FVECTOR *, SVECTOR *, int * ) ;
extern void	GM_CameraMakeTarget( FVECTOR *, FVECTOR *, SVECTOR *, int * ) ;
extern void	GM_CameraMakeCamera( FVECTOR *, FVECTOR *, SVECTOR *, int * ) ;

extern void		GM_EntryCameraList( GM_CameraSet * ) ;
extern void		GM_DeleteCameraList( u_int ) ;
extern	GM_CameraSet	*GM_FindCameraSet( u_int, int ) ;
#define	GM_FindCamera( _n )	GM_FindCameraSet( _n, 0 )
extern	GM_CameraSet	*GM_FindCameraOld( u_int ) ;

extern void	GM_SetCameraType( GM_CameraSet *, int, int ) ;
extern void	GM_SetCameraBound( GM_CameraSet *, float * ) ;
extern void	GM_SetCameraLimit( GM_CameraSet *, float * ) ;
extern void	GM_SetCameraRotate( GM_CameraSet *, SVECTOR * ) ;
extern void	GM_SetCameraTrack( GM_CameraSet *, int ) ;
extern void	GM_SetCameraAngle( GM_CameraSet *, float ) ;

extern void	GM_ChangeCamera( int ) ;
extern void	GM_SetCameraQuick( int ) ;
extern void	GM_UseCamera( int ) ;
extern void	GM_NoUseCamera( int ) ;
extern void	GM_ReverseSlideCameraDir( int ) ;

extern void	GM_CameraInterpQuick( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
extern void	GM_CameraInterpExp4( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
extern void	GM_CameraInterpExp8( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
extern void	GM_CameraInterpExp16( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
extern void	GM_CameraInterpLinear( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
extern void	GM_CameraInterpIntoSubject( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
extern void	GM_CameraInterpOutSubject( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
//void	GM_CameraInterpIntoBehind( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
//void	GM_CameraInterpOutBehind( GM_CameraChanl *, GM_CameraSet *, GM_CameraSet * ) ;
void	GM_CameraInterpIntoBehind2( GM_CameraChanl *chanl, 
								    GM_CameraSet *cam1, GM_CameraSet *cam2 ) ;

extern 	void	GM_CopyCamera( GM_CameraSet *, GM_CameraSet * ) ;			      
extern	GM_CameraSet	*GM_GetCurrentCamera( int ) ;
extern	GM_CameraSet	*GM_GetCurrentCameraSet( int ) ;
#define	GM_GetNextCamera( cn ) GM_GetCurrentCameraSet( cn ) 

extern	void	GM_ChangeCurrentCameraInterpFunc1( int chanl, void *func, int time ) ;
extern	void	GM_ChangeCurrentCameraInterpFunc2( int chanl, void *func, int time ) ;


/* game/camera.c */
GM_CameraSet	*NewProgramCamera( int, int, int, int ) ;


/* game/cam_set.c */
extern void		GM_SetCameraAdjust( int, FVECTOR * ) ;

extern	void	GM_SetCameraOscillation( int chanl, float amplitude, float cycle ) ;
extern	void	GM_PushCameraOscillationValue( int chanl ) ;
extern	void	GM_PopCameraOscillationValue( int chanl ) ;

extern	int		GM_InitInterpCameraWork( GM_CameraSet *, int ) ;
extern	void	GM_FreeInterpCameraWork( GM_CameraSet * ) ;


/* game/hmd_util.c */
extern  void    GM_SetHMDCamera( DG_CHANL *chanl, FVECTOR *pos, FVECTOR *trg, float angle ) ;

/*----------------------------------------------------------------------*/

/* Camera-related global variables */

extern	GM_CameraSet	*GM_PlayerSubjectCamera[] ; /* Subjective Camera Players (rai_init.c) */

/*----------------------------------------------------------------------*/

/* Macros, etc. */
static  inline void GM_SetCurrentCalcMat( int chanl, GM_CameraMatrixFunc func )
{
    GM_Camera->chanl[chanl].set_mat = func ;
}
static	inline	int	GM_CheckCameraActive( int cn ) 
{ 
	return !( GM_Camera->chanl[ cn ].flag & GM_FLAG_CAMERA_NO_USE ) ;
}


/*--------------------------------------------------------------*/
//BP_CAMERA - extra camera functionality
/*--------------------------------------------------------------*/

// 1st person camera overrides
extern int      gBP_1stPersonCamera_Override;
extern int      gBP_1stPersonCamera_Move;
extern int      gBP_1stPersonCamera_Toggle;
extern int      gBP_1stPersonCamera_Active;

// 3rd person camera overrides
extern int      gBP_3rdPersonCamera_Override;
extern FVECTOR  gBP_3rdPersonCamera_Target;
extern FVECTOR  gBP_3rdPersonCamera_Eye;
extern SVECTOR  gBP_3rdPersonCamera_Rot;
extern int      gBP_3rdPersonCamera_Dist;
extern float    gBP_3rdPersonCamera_HSpeed;
extern float    gBP_3rdPersonCamera_VSpeed;

// Camera overrides
extern int      gBP_Camera_InheritRot;

// Camera functions
int BP_NormalizeIntegerRot( int r );

/*--------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif
	
#endif
