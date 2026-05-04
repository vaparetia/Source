/*
	pl_work.x
	プレイヤー外部変数、関数定義ファイル

	2001/01/25		M.Sonoyama
	$Id: pl_work.x,v 1.3 2002/11/23 12:46:55 Yoshizawa1 Exp $
*/

#ifndef __pl_work_x__
#define	__pl_work_x__

extern	PlayerWork		*GM_PlayerWork ;
extern	PL_ACTION		PL_StillMode[], PL_CB_BoxStill, PL_CB_BoxStop ;
extern	PL_ACTION		PL_Dead, PL_Down ;

extern	PL_PluginSet	PL_PluginList ;
extern	PL_PollingSet	PL_PollingList ;
extern	PL_DeadMode		PL_DeadModeList ;	

extern	PL_DAMAGEFUNC	PL_DamageFunc ;
extern	PL_ACTION		PL_ReturnModes[] ;

#define	PL_ReturnMode	PL_ReturnModes[ 0 ] 

extern	PL_TARGETCALLBACKFUNC	PL_TargetCallbackFunc ;
extern	int				PL_ReturnModeMotionArc ;
extern	PL_OPERATE		PL_CheckAttackFunc ;
extern	BEYOND			*PL_Beyond ;
extern	FORCE			*PL_Force ;
extern	PL_MOTION_SET	*PL_MotionSet ;

extern	float			PL_MotionVelocity ;

extern	HZX_HZD			PL_KnockSegment ;
extern	int				PL_KnockJoint ; 

extern	u_char			PL_DamageVib1H[] ;
extern	u_char			PL_DamageVib1L[] ;
extern	u_char			PL_DamageVib2H[] ;
extern	u_char			PL_DamageVib2L[] ;

extern	PL_WeaponSet	PL_WeaponSets[] ;
extern	PL_ItemSet		PL_ItemSets[] ;

extern	void		PL_AddPlugin( PL_PluginSet *, int, PL_PLUGIN_INIT, PL_ACTION ) ;
extern	void		PL_AddPollingFunc( PL_PollingSet *, PL_POLLING_FUNC ) ;
extern	void		PL_RemovePollingFunc( PL_PollingSet * ) ;

extern	void		PL_IntoSubject( PlayerWork * ) ;
extern	void		PL_LeaveSubject( PlayerWork * ) ;
extern	void		PL_IntoIntrude( PlayerWork * ) ;
extern	void		PL_LeaveIntrude( PlayerWork * ) ;
extern	void		PL_LeaveCaution( PlayerWork * ) ;
extern	void		PL_SetInvincible( PlayerWork *, int ) ;
extern	void		PL_UnsetInvincible( PlayerWork * ) ;
extern	void		PL_SendMessage( int, int *, int ) ;
extern	void		PL_ClearCaptureTarget( PlayerWork * ) ;
extern	void		PL_GetModelLength( PlayerWork *, float *, float * ) ;
extern	void		PL_GetModelLength2( PlayerWork *work, float *front, float *back, int flag ) ;
extern	int			PL_StrongestUDLRValue( GV_PAD *pad ) ;
extern	void		PL_ExecForceActProc( PlayerWork *work, int mode, int time, int end_flag ) ;
extern	void		PL_ClearForceActProc( void ) ;
extern	void		PL_ChangeStance( PlayerWork *work, int stance ) ;
extern	void		PL_SetMotionChangeSets( int wp, short *set ) ;
extern	void		PL_SetMotionSet( int wp ) ;
extern	int			PL_MoveLevel( PlayerWork *work ) ;
extern	int			PL_ReactWall( int to ) ;
extern	int			PL_CheckMovRotLenSegment( HZX_GROUP_ID hzx_id, FVECTOR *mov, FVECTOR *shift,
	 	 									  SVECTOR *rot, float len, int chk, int flag, int fflag ) ;
extern	float		PL_NearestEnemyLen( void ) ;

extern	void		PL_SubjectTurn( PlayerWork * ) ;
extern	void		PL_SubjectPeep( PlayerWork * ) ;

extern	void		PL_LevelCheck( PlayerWork * ) ;

extern	void		PL_SeekTurn( PlayerWork * ) ;
extern	void		PL_NoWeapon( PlayerWork * ) ;
extern	void		PL_NoEquip( PlayerWork * ) ;
extern	int			PL_CheckIntrude( PlayerWork * ) ;
extern	int			PL_UnequipSpecials( void ) ;

extern	void		PL_GroundIK2( PlayerWork *, float ) ;
extern	int			PL_AvoidSink( PlayerWork *work, float height, float len ) ;
extern	int			PL_CheckGroundEnableDir( PlayerWork *work ) ;

extern	void		PL_SubjectTurnCB( PlayerWork *work ) ;

extern	void		PL_LoadStatus( void ) ;
extern	void		PL_SaveStatus( void ) ;

extern	void		*PL_DamageCamera( int time, long64 status ) ;


extern	void		*PL_FootPrintManager( PlayerWork *pwork ) ;
extern	void		PL_FootPrintAct( void *ptr, int init, int left, int right ) ;
extern	void		PL_FootPrintWakeUp( void *ptr ) ;
extern	void		PL_FootPrintForce( void *ptr, int which ) ;

extern	void		PL_ClearSpecialCallback( void ) ;

extern	void		PL_AdjustXZ( PlayerWork *work, FVECTOR *shift ) ;
#define	AdjustXZ( _w, _s )	PL_AdjustXZ( _w, _s ) 
extern	void		PL_AdjustXZFromTo( PlayerWork *work, FVECTOR *prev, FVECTOR *next ) ;

extern	void		*PL_RaidenEquipmentManager( OBJECT *body, int name ) ;
extern	void		*PL_RaidenShadowHair( OBJECT *body ) ;

/* pl_arm.c */
extern	void	PL_SetArmAttack( long64 weapon_type, FVECTOR *force, int from, int to, 
								 int faint, int damage, float add_len ) ;


/* pl_pad.c */
extern void PL_PadSetPatternSubjectMove( void ) ;
extern void PL_PadSetPatternA( void ) ;
extern void PL_PadSetPatternB( void ) ;
extern void PL_PadSetPatternC( void ) ;
extern void PL_SetPadType( int set ) ; /* set = 0 or 1*/

extern int	PL_PAD_ACTION	    ;/*pl_pad.c*/
extern int	PL_PAD_SUBJECT	    ;
#ifdef _WINDOWS
extern int	PL_PAD_SUBJECT_TOGGLE;
#endif
extern int	PL_PAD_STOP_AIM     ;
extern int	PL_PAD_SQUAT	    ;
extern int      PL_PAD_PRESS_SQUAT  ;
extern int	PL_PAD_WEAPON	    ;
extern int	PL_PAD_PRESS_WEAPON ;
extern int	PL_PAD_PUNCH        ;
extern int      PL_PAD_PRESS_PUNCH  ;
extern int	PL_PAD_CHANGE_WEAPON;
extern int	PL_PAD_PEEP         ;
extern int	PL_PAD_PEEP_R       ;
extern int	PL_PAD_PEEP_L       ;
extern int	PL_PAD_PRESS_PEEP_R ;
extern int	PL_PAD_PRESS_PEEP_L ; 
extern int	PL_PAD_CAPUTRE      ;
extern int	PL_PAD_LOCKON       ;
extern int	PL_PAD_KNOCK        ;
extern int	PL_PAD_HANG         ;
extern int	PL_PAD_BEYOND       ;
extern int	PL_PAD_READY_RUN    ;
extern int	PL_PAD_BEYOND_DOWN  ;
extern int	PL_PAD_LOCKER_ZOOM  ;
extern int	PL_PAD_PRESS_LOCKER ;




extern int   PL_SubjectMove   ; /* 主観移動モード     */
extern int   PL_SubjectToggle ; /* 主観移動モード     */
extern float PL_SubjectAngle  ; /* 主観移動モード角度 */
extern int   PL_ObjectFlag    ; /* オブジェクトフラグ */
extern int   PL_PadType       ; /* パッドタイプ       */



/*----------------------------------------------------------------*/

/* グローバルモード */

extern	void		PL_StandCautionStill( PlayerWork *work, int time ) ;
extern	void		PL_Blow( PlayerWork *work, int time ) ;
extern	void		PL_RunToSquat( PlayerWork *work, int time ) ;

extern	void		PL_CheckAttack( PlayerWork *work ) ;

#define	CheckAttack				PL_CheckAttack
#define	StandCautionStill		PL_StandCautionStill

extern int PL_SubjectMove ;
extern int PL_ObjectFlag  ; /* オブジェクトフラグ */

#endif

