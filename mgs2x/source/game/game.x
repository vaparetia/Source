/*
   game.x
   ゲーム関数宣言

   1999/07/07 M.Sonoyama
   $Id: game.x,v 1.5 2002/11/23 11:01:04 Yoshizawa1 Exp $			

   *** PROJECT OF METAL GEAR SOLID2 ***
*/

#ifndef _game_x_
#define _game_x_

#ifdef __cplusplus
extern "C" {
#endif

/* gamed.c */
extern	void 	GM_StartDaemon( void ) ;
extern	void	GM_GameOver( void * ) ;
extern	int		GM_IsGameOver( void ) ;
extern	void	GM_GameOverProcStart( void * ) ;
extern	void	GM_GameOverProcEnd( void * ) ;
extern	void	GM_GameOverClear( void ) ;
extern	void	GM_GameOverRestart( int ) ;
extern	int		GM_ForceExecBlock( char *, GCL_ARGS * ) ;
extern	int		GM_ExecBlock( char *, GCL_ARGS * ) ;
extern	int		GM_ForceExecProc( int, GCL_ARGS * ) ;
extern	int		GM_ExecProc( int, GCL_ARGS * ) ;
extern	int		GM_IsGameOver( void );

extern	void	GM_SetVibration1( int which, int value ) ;
extern	void	GM_SetVibration2( int which, int value ) ;
extern	void	GM_SetForceVibration1( int which, int value ) ;
extern	void	GM_SetForceVibration2( int which, int value ) ;
#ifdef _WINDOWS
extern	void	GM_SetDispVibration1( int value ) ;
extern	void	GM_SetDispVibration2( int value ) ;
extern	void	GM_SetForceDispVibration1( int value ) ;
extern	void	GM_SetForceDispVibration2( int value ) ;
#endif

extern	void GM_LoadRestart( int force_load_resident );
void GM_Reboot( char *boot );

/*	control.c	*/
extern	void	GM_InitWhereSystem( void ) ;
extern	int		GM_InitControlEx( CONTROL *, u_int, int, int ) ;
#define	GM_InitControl( _c, _n, _m )	GM_InitControlEx( _c, _n, _m, 0 )
extern	void	GM_ActControl( CONTROL * ) ;
extern	void	GM_FreeControl( CONTROL * ) ;
extern	void	GM_ConfigControlObject( CONTROL *, OBJECT * ) ;
extern	void	GM_ConfigControlHazard( CONTROL *, int, int, int ) ;
extern	void	GM_ConfigControlHzxHeight( CONTROL *, float, float ) ;
extern	void	GM_ConfigControlMapCheck( CONTROL * ) ;
extern	void	GM_ConfigControlAddressCheck( CONTROL * ) ;
extern	void	GM_ConfigControlTrapCheck( CONTROL * ) ;
extern	void	GM_ConfigControlMessageCheck( CONTROL * ) ;
extern	void	GM_ConfigControlSetCollide( CONTROL *, float, float ) ;
extern	void	GM_ConfigControlResetCollide( CONTROL * ) ;
extern	void	GM_ConfigControlNearCheck2( CONTROL *ctrl, float height ) ;
extern	void	GM_ConfigControlCameraTrapCheck( CONTROL * ) ;
extern	void	GM_ConfigControlAttribute( CONTROL *, int ) ;
extern	void	GM_ConfigControlNoSegmentArea( CONTROL * ) ;	
extern	void	GM_ConfigControlMapID( CONTROL * ) ;
extern	void	GM_ConfigControlPosition( CONTROL *, FVECTOR *, SVECTOR * ) ;
extern	void	GM_ConfigControlHzxCheckFlag( CONTROL *ctrl, int segflag, int flrflag ) ;
extern	void	GM_ResetControlPosition( CONTROL *, FVECTOR * ) ;
extern	void	GM_ResetControlPositionAndGroup( CONTROL *, FVECTOR *, int ) ;
extern 	int 	GM_CheckChangeGroup( CONTROL *ctrl, FVECTOR *pmov ) ;
extern	int		GM_CheckInsideTrap( CONTROL *, u_int *, int, int ) ;
extern	int		GM_CtrlWorldLimit( CONTROL *ctrl ) ;
extern	void	GM_ControlResetHeightMotion( CONTROL *ctrl ) ;

/* 	target.c	*/
extern	void	*GM_StartTargetSystem( void ) ;
extern	void	GM_InitTargetSystem( void ) ;
extern	void	GM_FreeTarget( TARGET * ) ;
extern	void	GM_SetTarget( TARGET *, u_int, u_int, u_int, FVECTOR *, FVECTOR * ) ;
extern	void	GM_SetTargetName( TARGET *, int ) ;
extern	void	GM_PutTarget( TARGET * ) ;
extern	void	GM_SetTargetSize( TARGET *, FVECTOR * ) ;
extern	void	GM_MoveTargetMap( TARGET *, FVECTOR *, int ) ;
extern	void	GM_MoveTarget2Map( TARGET *, FMATRIX *, int ) ;
extern	void	GM_MoveTarget3Map( TARGET *, FMATRIX *, int ) ;
extern	void	GM_MoveOnlineTargetMap( TARGET *, FVECTOR *, FVECTOR *, int ) ;
extern	void	GM_TargetConnectChild( TARGET *, TARGET *, int, int ) ;
extern	void	GM_TargetDisconnectChild( TARGET *, TARGET * ) ;
extern	void	GM_SetTargetWeaponType( TARGET *, long64 ) ;
extern	void	GM_SetTargetCallBack( TARGET *, TARGET_CALLBACK, void * ) ;
extern	void	GM_SetPowerTarget( TARGET *, POWER_TARGET *, u_int, u_short, u_short, u_short,
	                           FVECTOR * ) ;
extern	void	GM_SetCaptureTarget( TARGET *, CAPTURE_TARGET *, CONTROL *, OBJECT * ) ;
extern	void	GM_SetPushTarget( TARGET *, PUSH_TARGET *, float ) ;
extern	void	GM_SetStickTarget( TARGET *, STICK_TARGET *, FMATRIX * ) ;
extern	void	GM_MakeTargetForce( TARGET *, TARGET * ) ;
extern	int	GM_DamageTarget( TARGET *, TARGET * ) ;
extern	int	GM_PushTarget( TARGET *, TARGET * ) ;
extern	void	GM_CaptureTarget( TARGET *, TARGET * ) ;
extern	void	GM_ClearTargetDamage( TARGET * ) ;
extern	void	GM_ClearTargetPush( TARGET * ) ;
extern	void	GM_TargetSetSkip( TARGET * ) ;
extern	void	GM_TargetResetSkip( TARGET * ) ;
extern	void	GM_TargetGetCenter( FVECTOR *center, TARGET *t ) ;
extern	void	GM_TargetHitCancel( TARGET *off, TARGET *def ) ;
extern	int		GM_TargetCheckCenter2Center( TARGET *off, TARGET *def, 
			 	 							 int chk_type, int seg_flag, int flr_flag ) ;
extern	TARGET	*GM_TrgDefListGetTop( void ) ;
extern	void	GM_TargetSetDirectAttack( TARGET *, TARGET * ) ;
extern	void	GM_SetTargettoDynamicHazard( void *seg, TARGET *target, int type ) ;
extern	void	GM_TargetHitDynamicHazard( void	*ptr, TARGET *off, int type ) ;

#define	GM_SetTargetParts( _p, _c, _n, _l ) GM_TargetConnectChild( _p, _c, _n, _l )

#define	GM_MoveTarget( _t, _f )		GM_MoveTargetMap( _t, _f, GM_CurrentMap ) 
#define	GM_MoveTarget2( _t, _m )	GM_MoveTarget2Map( _t, _m, GM_CurrentMap ) 
#define	GM_MoveTarget3( _t, _m )	GM_MoveTarget3Map( _t, _m, GM_CurrentMap ) 
#define	GM_MoveOnlineTarget( _t, _f, _to ) GM_MoveOnlineTargetMap( _t, _f, _to, GM_CurrentStageMap )

/* map.c */
extern	int	GM_GetBit( int ) ;
extern	int	GM_GetID( int ) ;
extern	void	*GM_InitMapSystem( void ) ;
extern	int	GM_GetMapIDfromHzxGroup( int ) ;
extern	int	GM_GetMapIDfromChanlDisp( int chanl ) ;
extern	int	GM_GetHzxGroupID( int ) ;
extern	int	GM_GetDGGroupID( int ) ;
extern	MAP	*GM_GetMap( int ) ;
extern	MAP	*GM_GetMap2( int ) ;
extern	int	GM_GetMapID( int ) ;
extern	void GM_ShowMap( int map );
extern	void GM_ChangeMapConnection( int name, int command );
extern void GM_ChangeControlMapFromMapId( CONTROL *ctrl, int map );
extern void GM_ChangeControlMapFromHzxId( CONTROL *ctrl, HZX_GROUP_ID hzx_id );

void GM_CalcChanlMap( void );
void GM_ChanlControl( int chanl, int flag );

int		GM_CheckMapChangeTrap( CONTROL *ctrl, FVECTOR *p ) ;
int		GM_GetHzxGroupID( int bit );

/* mapcnct.c */

int GM_GetConnectMapID( int map );
void GM_ChangeMapConnection( int name, int command );

void	*NewMapConnectProg( int name, int map1, int map2 ) ;
void	GM_ChangeMapConnection2( void *ptr, int command ) ;
void	GM_ChangeMapConnectID( void *ptr, int map1, int map2 ) ;
void	GM_ForbidMapConnectOff( void ) ;
void	GM_EnableMapConnectOff( void ) ;
void	GM_InitMapConnect( void ) ;

/* object.c */
extern void GM_InitObject( OBJECT *object, int model, int flag );
extern void GM_ActMotion( OBJECT *object );
extern void GM_ActObject( OBJECT *object );
extern void GM_ActObject2( OBJECT *object );
extern void GM_FreeObject( OBJECT *object );
extern void GM_ConfigObjectFlags( OBJECT *object, int flag );
extern void GM_ConfigObjectLight( OBJECT *object, FMATRIX *light );
extern void GM_ConfigObjectStep( OBJECT *object, FVECTOR *step );
extern void GM_ConfigObjectRoot( OBJECT *object, OBJECT *org, int unit );
extern void GM_ConfigObjectJoint( OBJECT *object, FVECTOR *rots );
extern void GM_ConfigObjectSlide( OBJECT *object );
extern void GM_ConfigObjectMotion( OBJECT *object, int n_layer, int motion, int flag );
extern void GM_ConfigObjectAction( OBJECT *object, int n_layer, int action, int time, u_long64 mask, int interp );
extern void GM_ConfigObjectEvm( OBJECT *, int, int ) ;

/* homing.c */
extern	void	GM_InitHomingSystem( void ) ;
extern	void	GM_SetHomingTrg( HOMING_TRG *, FMATRIX *, OBJECT *, int *, CONTROL *, int ) ;
extern	void	GM_SetHomingTrgTarget( HOMING_TRG	*hom, TARGET *trg ) ;
extern	void	GM_PutHomingTrg( HOMING_TRG * ) ;
extern	HOMING_TRG *GM_GetHoming( void ) ;
extern	void	GM_FreeHomingTrg( HOMING_TRG * ) ;
extern	HOMING_TRG	*GM_GetHomingNearTrg( FVECTOR *, int, int, int, HOMING_TRG *, int, HZX_GROUP_ID ) ;
extern	HOMING_TRG	*GM_GetHomingTrg( FVECTOR *, int, int, int, HZX_GROUP_ID ) ;

extern	HOMING_TRG	*GM_GetHomingTrgInSight( FVECTOR *pos, SVECTOR *dir, SVECTOR *limit,
											 HOMING_TRG *prev, int homing_dir, HZX_GROUP_ID hzx_id ) ;
extern	int			GM_CheckHomingEnable( HOMING_TRG *hom, FVECTOR *pos, SVECTOR *dir, SVECTOR *limit,
						 				  HZX_GROUP_ID hzx_id ) ;

extern	void GM_RotToQuat( SVECTOR *, FVECTOR * ) ;
extern	void GM_RotToQuatXAfterY( SVECTOR *, FVECTOR * ) ;
extern	void GM_TrgToAdjRot( FVECTOR *, int, FVECTOR *, SVECTOR * ) ;
extern	void GM_TrgToNearRot( FVECTOR *, int, FVECTOR *, SVECTOR * ) ;
extern	void GM_TrgToNearRotLimit( FVECTOR *, int, FVECTOR *, SVECTOR * ) ;
extern	void GM_TrgToNearRotLimit2( FVECTOR *, int, FVECTOR *, SVECTOR * ) ;
extern	void GM_TrgToNearRotLimitX( FVECTOR *, int, FVECTOR *, SVECTOR * ) ;
extern	void GM_TrgToNearRotAimLimit( FVECTOR *, int, int, FVECTOR *, SVECTOR *, int, int, ROTLIMIT * ) ;
extern	void GM_DirToNearRotAimLimit( FVECTOR *, int, int, int, SVECTOR *, int, int, ROTLIMIT * ) ;
extern	void GM_MotionAdjust16( OBJECT *, SVECTOR *, int, int ) ;
extern	void GM_MotionAdjust21( OBJECT *, SVECTOR *, int, int ) ;
extern	void GM_MotionAdjustWest( OBJECT *, SVECTOR *, int, int ) ;
extern	void GM_MotionAdjustAim( OBJECT *, SVECTOR *, int, int, int ) ;
extern	void GM_AdjustQuatX( FVECTOR *, FVECTOR *, FVECTOR * ) ;
extern	void GM_AdjustQuatY( FVECTOR *, FVECTOR *, int, FVECTOR * ) ;
extern	void GM_AdjustQuatXY( FVECTOR *, FVECTOR *, int, FVECTOR * ) ;
extern	void GM_AdjustRotBody( OBJECT *, SVECTOR *, int ) ;

/* g_sound.c */
//BP extern int	GM_GetSngCode( void ) ;
extern void GM_SetVolCurveInDisFromPos( FVECTOR *pos, int *from_indis, int *to_indis, int dev ) ;
extern void	GM_SdSet( int ) ;
extern void	GM_SeSet( int, int, int ) ;
extern void	GM_SeSet3D( int, int, int, float ) ;
extern int	GM_SeGetPan( FVECTOR *, int, float * ) ;
extern int	GM_SeGetVol( FVECTOR *, int ) ;
extern void	GM_SeGetVolPan( FVECTOR *, int, int *, int *, float * ) ;
extern int	GM_GetPanDir( int ) ;
extern void GM_SeGetVolPanFromVolCurves( FVECTOR *pos, int *vol, int *pan, VOLCURVES *volcurves, float * bp_angle ) ;
extern void	GM_SeSetMode( int, FVECTOR *, int ) ;
extern void	GM_SeSetEx( int se, FVECTOR *pos, int max_dis, int min_dis ) ;
extern void GM_SeSetFromVolCurve( int se, FVECTOR *pos, VOLCURVES *volcurves ) ;
extern void GM_InitMixConvFader( void ) ;
extern void GM_MixConvFader( int, int, int ) ;
extern void GM_MixConvFrequencyFader( int track, int freq, int pan, int vol ) ;
extern void	GM_BgmFaderMode( int, FVECTOR *, int ) ;
extern void	GM_BgmFaderEx( int, FVECTOR	*, int, int ) ;
extern int	GM_SoundDistance( FVECTOR *pos ) ;
extern void	GM_SdLoadWait( void ) ;
extern int	GM_SdStatus( void ) ;
extern void	GM_InitVolumParam( void );

/* sparkd.c */
extern	void	GM_EntrySparkFunction( GM_SPARK_FUNC, int ) ;
extern	void	GM_CallSpark( int, FMATRIX *, FVECTOR * ) ;
extern	void	GM_EntryScarFunction( GM_SCAR_FUNC, int ) ;
extern	void	GM_CallScar( int, FMATRIX *, HZX_SEG *, HZX_FLR * ) ;

/* navi.c */
extern	int		GM_Navi( NAVIGATE *navi, NAVITARGET *n_trg, int range ) ;
extern	int		GM_NaviNear( NAVIGATE *navi, NAVITARGET *n_trg, int range ) ;
extern	int		GM_NaviNearExtend( NAVIGATE *navi, NAVITARGET *n_trg, int range ) ;
extern	int		GM_InZoneNavi( NAVIGATE *navi, NAVITARGET *n_trg, int range ) ;
extern	void	GM_ZoneNavi( NAVIGATE *navi, NAVITARGET *n_trg ) ;
extern	void	GM_ZoneNaviExtend( NAVIGATE *navi, NAVITARGET *n_trg ) ;
extern	void 	GM_SetNaviTargetFromPos( NAVITARGET *n_trg, FVECTOR *pos, int hzx_id ) ;
extern	void 	GM_SetNaviTargetFromCtrl( NAVITARGET *n_trg, CONTROL *ctrl ) ;
extern	void	GM_SetNaviTargetFromZoneAddr( NAVITARGET *n_trg, HZX_ZONE_ADD zoneaddr ) ;
extern	void 	GM_ReSetNavi( NAVIGATE *navi ) ;
extern	void	GM_SetNavi( NAVIGATE *navi, CONTROL *ctrl ) ;

// in interupt.c
extern	void	GM_InitInterruptSystem( void ) ;
extern	R_INTRPT	*GM_GetRIntrptCheckStatus( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr, int status ) ;
extern	R_INTRPT	*GM_GetRIntrptZ2Z( HZX_ZONE_ADD from, HZX_ZONE_ADD to ) ;
extern	R_INTRPT	*GM_GetRIntrptOne( HZX_ZONE_ADD from ) ;
extern	R_INTRPT	*GM_GetRIntrpt( HZX_ZONE_ADD from, HZX_ZONE_ADD to ) ;
extern	void	GM_SetRouteIntrpt( R_INTRPT *r_intrpt, HZX_ZONE_ADD zone1, HZX_ZONE_ADD zone2,
							FVECTOR *pos, int dir, int status, int kind ) ;
extern	void	GM_SetRIntrptCallBack( void *ptr, R_INTRPT *r_intrpt, RINTRPT_CALLBACK func ) ;
extern	void	GM_PutRoteIntrpt( R_INTRPT *r_intrpt ) ;
extern	void	GM_FreeRouteIntrpt( R_INTRPT *r_intrpt ) ;
extern	int		GM_RIntrptCallBack( R_INTRPT *r_intrpt, int mot_name, int mot_num, int order ) ;
extern	void	GM_SetZoneIntrpt( Z_INTRPT *z_intrpt, CONTROL *,HZX_ZONE_ADD zone, int status, int kind ) ;
extern	void	GM_PutZoneIntrpt( Z_INTRPT *z_intrpt ) ;
extern	void	GM_FreeZoneIntrpt( Z_INTRPT *z_intrpt ) ;
extern	Z_INTRPT *GM_GetZoneIntrptList( void ) ;
extern	Z_INTRPT *GM_SearchZoneIntrpt( Z_INTRPT *z_intrpt, HZX_ZONE_ADD zone, int status, int kind ) ;
extern	Z_INTRPT *GM_ZoneIntrpt( HZX_ZONE_ADD zone, int status, int kind ) ;
extern	int	GM_ZoneIntrptCheck( HZX_ZONE_ADD this_zone ) ;
extern	int	GM_GetZIntrptZ2Z( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr ) ;
extern	int	GM_GetZIntrptZ2Z2( HZX_ZONE_ADD from_addr, HZX_ZONE_ADD to_addr ) ;

extern	Z_INTRPT	*GM_SearchZIntrptExceptThis( HZX_ZONE_ADD zone, int status, int kind, CONTROL *ctrl ) ;

/* enefind.c */
extern	void	GM_InitEneFindSystem( void ) ;
extern	ENEFINDLIST	*GM_GetEneFindList( int id ) ;
extern	void	GM_InitEneFindList( ENEFINDLIST *efl, int type ) ;
extern	void	GM_PutEneFindList( ENEFINDLIST *efl ) ;
extern	void	GM_FreeEneFindList( ENEFINDLIST *efl ) ;
extern	ENEFIND	*GM_GetEneFindFromList( ENEFINDLIST	*efl, int id ) ;
extern	void	GM_PutEneFindToList( ENEFINDLIST *efl, ENEFIND *ef ) ;
extern	void	GM_FreeEneFindFromList( ENEFINDLIST *efl, ENEFIND *ef ) ;
extern	int 	GM_EneFindNum( void ) ;
extern	void	GM_SetEneFind( ENEFIND *ef, FVECTOR *pos, int addr, int type ) ;
extern	ENEFIND	*GM_GetEneFind( int id ) ;
extern	void	GM_PutEneFind( ENEFIND *ef ) ;
extern	void	GM_FreeEneFind( ENEFIND *ef ) ;

/* getitem.c */
extern	int			GM_IncrementWeapon( int id, int n ) ;
extern	int			GM_IncrementItem( int id, int n ) ;
extern	int			GM_DecrementWeapon( int id, int n ) ;
extern	int			GM_DecrementItem( int id, int n ) ;
extern	int			GM_WeaponNum( int id ) ;
extern	int			GM_ItemNum( int id ) ;
extern	int			GM_GetItem( int spec, int id, int n ) ;

/* varsys.c */
extern	u_char	*GM_FindVibData( int name ) ;
extern	VAR_DEF	*GM_GetVibArc( int file ) ;
extern	int		GM_InitVarControl( GV_ACT_EX *actor, VAR_CONTROL *vctrl, int file ) ;
extern	void	GM_ConfigVctrlMctrl( VAR_CONTROL *vctrl, MOTION_CONTROL *m_ctrl ) ;
extern	void	GM_ActVctrl( VAR_CONTROL *vctrl ) ;
extern	void	GM_VctrlSetVibration( VAR_CONTROL *vctrl, int vib, int flag ) ;
extern	void	GM_VarSetVibration( VAR_DEF *def, int, int, int * ) ;
extern	void	GM_VctrlStopVibration( VAR_CONTROL *vctrl ) ;
extern	void	GM_VctrlSetFlag( VAR_CONTROL *vctrl, int flag ) ;
extern	void	GM_VctrlResetFlag( VAR_CONTROL *vctrl, int flag ) ;
extern	void	GM_VctrlSetVibrationScale( VAR_CONTROL *vctrl, float scale ) ;
//extern	void	GM_VctrlSetLoop( VAR_CONTROL *vctrl ) ;
//extern	void	GM_VctrlSetForce( VAR_CONTROL *vctrl ) ;
#define	GM_VctrlSetLoop( _v ) 	GM_VctrlSetFlag( _v, VAR_FLAG_LOOP )
#define	GM_VctrlSetForce( _v ) 	GM_VctrlSetFlag( _v, VAR_FLAG_FORCE )
#define	GM_VctrlSetSkip( _v ) 	GM_VctrlSetFlag( _v, VAR_FLAG_PLAY_SKIP )
#define	GM_VctrlResetSkip( _v )	GM_VctrlResetFlag( _v, VAR_FLAG_PLAY_SKIP )

/* vibrate.c */
extern	void	GM_StartVibrationSystem( void ) ;
extern	void	GM_SetVibrationScale( void *actor, float scale ) ;

/* strctrl.c */
void GM_StreamReset( void );

int GM_VoxStream( int pos, int flag );

int GM_StreamStatus( int handler );
void GM_StreamStart( int handler );
void GM_StreamStop( int handler );
void GM_StreamStopAll( void );
int GM_StreamIsPlay( void );
void GM_VoxStreamSetPan( int handler, int vol, int pan, float bp_angle );
void GM_StreamSetActionProc( int handler, int proc );

#ifdef PSX2
#define GM_VoxStreamSetParam( handler, pos, addr, vol, pan, bp_angle )	\
	GM_VoxStreamSetPan( handler, vol, pan, bp_angle );
#endif

#ifdef KP_XBOX
void GM_VoxStreamSetParam( int handler, FVECTOR* pv, HZX_ZONE_ADD addr, int vol, int pan );
#endif


void *GM_GetStreamActor( int handler );
int GM_StreamGetChannelHandler( int channel );

/* resource.c */

void GM_InitResource( void );
void GM_SetResourceInfo( GCL_STRING_RESOURCE *res );
void GM_SetResourceId( int ref_id, int res_id );
void *GM_GetResource( int ref_id, int offset );

/* hazard.c */
extern	int	GM_HzxCheckConflict( int n_touches, FVECTOR *mov, HZX_HZD *segs, 
				 				 FVECTOR *near_vecs, signed char *is_edges ) ;
extern	void	GM_GetHazardReaction( FVECTOR *react, int n_vecs, FVECTOR *vecs, int r_sphere ) ;
#ifdef DEBUG_MODE
extern	void	*GM_ViewNarrowInfo( int n_touches, FVECTOR *mov, HZX_HZD *segs ) ;
#endif

/* sejimaku.c */
extern	void	GM_JimakuSeSet( int p, int l, int n ) ;
extern	void	GM_JimakuSeSetMode( int se, FVECTOR *pos, int mode ) ;
extern	void	GM_JimakuSeSetEx( int se, FVECTOR *pos, int max_dis, int min_dis ) ;

/* デバッグ用 */
#ifdef DEBUG_MODE
extern	void	*NewTargetView( TARGET *, u_char, u_char, u_char ) ;
extern	void	*NewTargetView2( TARGET *, u_char, u_char, u_char ) ;
#endif

/* extern.c */
extern void	GM_ResetVariable( void );

/* script.c */
extern int GM_InitScript( void );

/* camerad.c */
extern void	*GM_StartCamera( void );

#ifdef KP_XBOX
/* menupri2.c */
extern void GM_ResetMenuPrimManager( void );

/* g_soundx.c */
#ifdef BPE_TEMP_UNUSED
extern void	GM_SetListenerEnv( DS3DLISTENER* pEnv );
extern void	GM_SetSourceEnv( DSFXI3DL2Reverb* pEnv );
#endif
extern void	GM_SetObstructionMaterial( LONG lHFLevel, float flLFRatio );
extern void	GM_SetOcclusionMaterial( LONG lHFLevel, float flLFRatio );

extern void	GM_SeSetModeAddr( int, FVECTOR *, int, HZX_ZONE_ADD ) ;
extern void	GM_SeSetExAddr( int se, FVECTOR *pos, int max_dis, int min_dis, HZX_ZONE_ADD addr ) ;
extern void GM_SeSetFromVolCurveAddr( int se, FVECTOR *pos, VOLCURVES *volcurves, HZX_ZONE_ADD addr ) ;

extern int	GM_ConvertVol( int vol );
extern int	GM_ConvertFreq( int freq );

extern void GM_SdUpdate( void );

/* discon.c */

void* GM_CreatePadDisconnect( void );
void GM_ReleasePadDisconnect( void );

#endif

#ifdef __cplusplus
}
#endif

#endif
