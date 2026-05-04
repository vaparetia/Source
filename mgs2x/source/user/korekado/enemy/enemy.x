/*
	enemy.x
	汎用敵兵ルーチン プロトタイプ

	1997/07/07 Y.Korekado
	$Id: enemy.x,v 1.1.1.3 2002/11/19 11:44:09 Yoshizawa1 Exp $
	
*/
//enemy.c
extern FVECTOR EneBodyTrgSize[] ;
extern PTARGET Ptarg ;

extern int ENE_FaintExit( ACTION *act ) ;
extern void ENE_AppearEffect( ENETHINK *entk, int type ) ;
extern void *ENE_BreakBody( ACTION *act, int type, void * ) ;
extern void ENE_TraceClear( ENETHINK *entk ) ;
extern int ENE_NoRadio( void ) ;
extern int ENE_CheckPlayerHidden( void ) ;
extern void ENE_SetLastRadioPos( ENETHINK *entk, FVECTOR *pos, int hzx_id ) ;
extern int ENE_PureAttacker( ACTION *act ) ;
extern int ENE_PureWatcher( ACTION *act ) ;
extern int ENE_OtherEnemyRadioAction( ENETHINK *entk ) ;
extern int ENE_InSightEnemyFromID( ENETHINK *entk,int uniq_id, long64 status, long64 not_status ) ;
extern void ENE_PlayerOnCorp( FVECTOR *pos ) ;
extern void ENE_ChangeRoute( ENETHINK *entk, int route ) ;
extern void ENE_ChangeRoute2( ENETHINK *entk, int route ) ;
extern int	ENE_InNearZone( FVECTOR	*pos, int zone, int grpid ) ;
extern void ENE_StandPosRintrpt( R_INTRPT *r_intrpt, int mot, FVECTOR *pos, int *dir ) ;
extern void	ENE_TraceEnd( ENETHINK *entk ) ;
extern int ENE_EneFindSightIn( ENETHINK	*, ENEFIND *, int *, int * ) ;
extern int ENE_EyeOnlineCheck( int, FVECTOR *, FVECTOR * ) ;
extern int ENE_BodyOnlineCheck( int, FVECTOR *, FVECTOR * ) ;
extern void ENE_ProcCall( ENE_ARGS * ) ;
extern void ENE_DeathProc( ENE_ARGS * ) ;
extern int ENE_GetSafeZone( HZX_ZONE_ADD zoneaddr, int num ) ;
extern int ENE_GetPearSafeZone( ENETHINK *, int ) ;
extern void ENE_RouteWarp( ENETHINK * ) ;
extern void ENE_CheckWarpNearPos( OBJECT *body, CONTROL *, int, int ) ;
extern void ENE_WarpNearPos( CONTROL *, int, int ) ;
extern int ENE_GetAddrToAddrDir( int from_addr, int to_addr ) ;
extern int ENE_GetZoneToZoneDir( int, int, int ) ;
extern int ENE_GetMinDirNearZone( HZX_ZONE_ADD, int ) ;
extern int ENE_GetWideNearZone( int, int, int ) ;
extern int ENE_GetSafeZoneNum( ENETHINK * ) ;
extern int ENE_RunawayZone( CONTROL *run, CONTROL *from ) ;
extern int ENE_GetTrgRoute( ENETHINK * ) ;
extern int ENE_GetNearZone( int, int, int ) ;
extern void ENE_ZonePos( FVECTOR	*, int, int ) ;
extern void ENE_Zoneadd2Pos( FVECTOR	*, int ) ;
extern int ENE_ZoneDir( FVECTOR	*, int, int ) ;
extern int ENE_GetGRot( CONTROL *, float ) ;
extern	int	_GetGRot( CONTROL *, float ) ;
extern void ENE_SetHeadMark( ACTION *, int, int ) ;
extern void ENE_Incline( CONTROL * ) ;
extern void ENE_SetIk( ENETHINK * ) ;
extern int ENE_PlayerHoldCheck( ENETHINK * ) ;
extern void ENE_SetBodyTargetSize( TARGET *, int ) ;
extern int ENE_SetOffenseTarget( ACTION	*, int ) ;
extern int ENE_SetOffenseTargetEne( ACTION	*, int ) ;
extern void ENE_RefreshParam( ENETHINK * ) ;
extern TARGET_PARTS	*KR_GetTargetPart( TARGET *trg, int	i ) ;
extern int ENE_Target2DurableArea( int p, int n ) ;
extern int ENE_DamageCheck( ENETHINK * ) ;
extern int ENE_DirectReachCheck( ENETHINK * ) ;
extern void ENE_Adjust( ENETHINK * ) ;
extern void ENE_AdjustAimPoint( ENETHINK * ) ;
extern void ENE_SetAimPlayer( ENETHINK * ) ;
extern void ENE_FamasShoot( FMATRIX	* ) ;
extern void ENE_Shoot( FMATRIX *, ACTION * ) ;
extern void ENE_ShootBullet(ACTION * ,int , FVECTOR * );
extern void ENE_SetNeedl( OBJECT *, int, FVECTOR * ) ;
extern void ENE_SetNeedlV( OBJECT *, int, FVECTOR * ) ;
extern void ENE_ClearNeedlV( OBJECT * ) ;
extern void ENE_ClearNeedl( ACTION * ) ;
extern void ENE_ClearPDamage( ACTION *act ) ;
extern void ENE_ClearPDamageMask( ACTION *act, int mask ) ;
extern void ENE_EyeInfoCheck( ENETHINK *, EYEINFO * ) ;
extern void ENE_EyeInfoCheckWhzxid( ENETHINK *entk, EYEINFO *eyei, int w_hzx_id ) ;
extern void ENE_SetAlertLevel( ENETHINK * ) ;
extern int ENE_RoutePad( ZONENAVI *, TRGPOINT *, CONTROL * ) ;
extern int ENE_SameZone( FVECTOR *, FVECTOR *, int ) ;

extern void ENE_InitSling( void *ptr, OBJECT *body, OBJECT *weapon, FMATRIX *lights ) ;
extern void ENE_InitSlingGBA( void *ptr, OBJECT *body, OBJECT *weapon, FMATRIX *lights ) ;
extern void ENE_InitSlingGPS( void *ptr, OBJECT *body, OBJECT *weapon, FMATRIX *lights ) ;
extern void ENE_InitSlingGPA( void *ptr, OBJECT *body, OBJECT *weapon, FMATRIX *lights ) ;
extern void *ENE_SetAttachment( void *ptr, OBJECT *body ) ;
extern void *ENE_SetAttachmentPLT( void *ptr, OBJECT *body ) ;
extern void *ENE_SetAttachmentGPS( void *ptr, OBJECT *body ) ;
extern void *ENE_SetAttachmentGBA( void *ptr, OBJECT *body ) ;
extern void *ENE_SetAttachmentGPA_SPS( void *ptr, OBJECT *body ) ;
extern void *ENE_SetAttachmentTNGGUN( void *ptr, OBJECT *body ) ;
extern void *ENE_SetAttachmentTNGKATANA( void *ptr, OBJECT *body ) ;

/*----- 補助関数 --------------------------------------------------*/
extern int ENE_InZone( FVECTOR *pos, int addr ) ;
extern int ENE_ZoneIntrptCheck( HZX_ZONE_ADD this ) ;
extern int ENE_IntrptCheck( HZX_ZONE_ADD this, HZX_ZONE_ADD next ) ;
extern int ENE_ZoneTrace( ZONENAVI *, TRGPOINT *, CONTROL *, int ) ;
extern int ENE_ZoneTraceIntrpt( ZONENAVI *, TRGPOINT *, CONTROL *, int ) ;
extern int ENE_DirectTrace( TRGPOINT *, FVECTOR *, int ) ;
extern int ENE_InRange( FVECTOR *, FVECTOR *, int ) ;
extern int ENE_InRangeXYZ( FVECTOR *, FVECTOR *, int, int, int ) ;
extern int ENE_InRangeOnline( FVECTOR *, FVECTOR *, int, int ) ;
extern int ENE_InRangeToRange( FVECTOR *pos, FVECTOR *trg, int in_range, int out_range ) ;

/*----- ターゲットセット ------------------------------------------*/
extern int ENE_SearchNearRootPoint( ROUTENAVI *, FVECTOR * ) ;
extern void ENE_SetNextnode( ROUTENAVI * ) ;
extern void ENE_SetTrgpNextnode( ROUTENAVI *, TRGPOINT * ) ;
extern void ENE_SetTrgpNode( ROUTENAVI *, TRGPOINT * ) ;
extern int ENE_SetTrgpFromRnaviNode( ROUTENAVI *, TRGPOINT *, int node ) ;
extern void ENE_SetTrgpPlayer( ZONENAVI *, TRGPOINT *, COMMANDER * ) ;
extern void ENE_SetTrgpNearRootPoint( ROUTENAVI *, ZONENAVI *, TRGPOINT *, FVECTOR * ) ;
extern void ENE_SetTrgpPoint( TRGPOINT *, FVECTOR *, int ) ;
extern void ENE_SetTrgpPosMap( TRGPOINT *, FVECTOR *, int ) ;
extern void ENE_SetTrgpWaitPoint( ENETHINK *entk, TRGPOINT * ) ;
extern void ENE_SetTrgpEnemy( ENETHINK *, TRGPOINT * ) ;
extern void ENE_SetTrgpEyei( EYEINFO *, TRGPOINT * ) ;
extern void ENE_SetTrgpDefense( TRGPOINT *, FVECTOR *, int ) ;
extern void ENE_SetTrgpCSP( TRGPOINT *, int, int ) ;
extern void ENE_SetTrgpZone( TRGPOINT *, int, int ) ;
extern void ENE_SetTrgpZoneaddr( TRGPOINT *, HZX_ZONE_ADD ) ;
extern void ENE_SetTrgpSafeArea( TRGPOINT *, int, int ) ;
extern void ENE_SetTrgp( TRGPOINT *, FVECTOR *, int ) ;
extern void ENE_SetTrgpZoneSide( TRGPOINT *, HZX_ZONE_ADD, int ) ;

/*-----  --------------------------------------------------*/
extern void ENE_GM_Act( ENETHINK * ) ;
extern void ENE_PreProcess( ENETHINK * ) ;
extern void ENE_AftProcess( ENETHINK * ) ;
extern void ENE_FreeResources( ENETHINK * ) ;
extern void ENE_ActInit ( ENETHINK * ) ;
extern void ENE_SetHeadMark2( ACTION *, int, int ) ;
extern void ENE_ActStatusCheck ( ENETHINK * ) ;
extern void ENE_DefTargCallBack( TARGET *, TARGET *, ENETHINK * ) ;
extern void	ENE_Gravitation( ENETHINK *entk ) ;

/*-----  --------------------------------------------------*/
extern int ENE_ReadClearing( ROUTENAVI *, int, int ) ;
extern int ENE_ReadNodes( ROUTENAVI *, int ) ;
extern void ENE_InitTargPoint( TRGPOINT * ) ;
extern void ENE_InitZoneNavi( ZONENAVI * ) ;
extern void ENE_InitRouteNavi( ROUTENAVI *, int, int ) ;
extern void ENE_InitRoute( ROUTENAVI * ) ;
extern void ENE_InitEneThink( ENETHINK *, CONTROL *, ROUTENAVI *, ZONENAVI *, ACTION * ) ;
extern void ENE_SetIDNumber( ENETHINK * ) ;
extern void ENE_SetSenseParam( SENSEPARAM *, short, short, int, int, int ) ;
extern void ENE_SetEyeInfo( EYEINFO *, FVECTOR *, int *, int *, int * ) ;
extern void ENE_InitControl( ENETHINK *, CONTROL *, int ) ;
extern void ENE_SetMotionName( ENETHINK *entk, int base, int stage ) ;
extern void ENE_InitObject( ENETHINK *, OBJECT *, int, CONTROL *, FMATRIX * ) ;
extern void ENE_InitSubWeapon( ENETHINK *, OBJECT *, OBJECT *, int ) ;
extern void	ENE_ResetWeaponPosition( ENETHINK *entk ) ;
extern void ENE_InitWeapon( ENETHINK *, OBJECT *, OBJECT *, int, TARGET * ) ;
extern void ENE_InitAction( ENETHINK *, ACTION *, CONTROL *, OBJECT *, int, int, int ,int *) ;
extern int ENE_InitThink( ENETHINK *, CONTROL *, ROUTENAVI *, ZONENAVI *, ACTION *, int, int ) ;
extern void ENE_LoadEneMemory( ENETHINK *entk ) ;
extern int ENE_InitTarget( ENETHINK *, CONTROL *, OBJECT *, ACTION * ) ;

/* 重野追加 */
//extern void ENE_InitViewZone( ); 
//extern int ENE_OnlineZoneCheck( int,int ); 
extern int ENE_ReadOnlinInfo(int ,int) ;
extern HZX_ZON *ENE_HZX_GetZone(int); 
extern int ENE_GetRouteDis(FVECTOR *,FVECTOR *,int,int,u_int);
extern void ENE_SetOffenseTarget2(ACTION *,long64,int ,FVECTOR *) ;
extern short ENE_GetAmmoMax(int) ;
extern void ENE_SetTargetEneDirect( ACTION *,TARGET *, int ) ;
//extern int ENE_EyeCheckSub( EYEPARAM * ,FVECTOR *) ;

extern void ENE_SetOffenseTarget3(ACTION *,long64,int,FVECTOR * ,FVECTOR *);

/**************/
//enaction.c


extern int	ENE_WalkMotion( ACTION *act ) ;
extern int	ENE_RunMotion( ACTION *act ) ;
extern int	ENE_ReadyGunMotion( ACTION *act ) ;
extern int	ENE_ReloadMotion( ACTION *act ) ;
extern int	ENE_PunchDamMotion( ACTION *act ) ;
extern int	ENE_NokezoriDamMotion( ACTION *act ) ;
extern int	ENE_DiscoveryMotion( ACTION *act ) ;
extern int	ENE_NormalStandMotion( ACTION *act ) ;
extern int	ENE_StandMotion( ACTION *act ) ;
extern int	ENE_CleStandMotion( ACTION *act ) ;
extern void ENE_SetMarActionPBreak( ACTION	*, int, int, int, u_long64, int, int, int ) ;
extern void ENE_SetActionPBreak( ACTION	*, int, int, int, u_long64, int, int ) ;
extern void ENE_ActStandStill( ACTION *, int ) ;
extern void ENE_ActStandMotion( ACTION *, int ) ;
extern void ENE_ActMoveStart( ACTION *, int ) ;
extern void ENE_ActMove( ACTION *, int ) ;
extern void ENE_ActPinpointMove( ACTION *, int ) ;
extern void ENE_ActTurn( ACTION *, int ) ;
extern void ENE_ActKeepMotion( ACTION *, int ) ;
extern void ENE_ActOneTimeMotion( ACTION *, int ) ;
extern void ENE_ActSetRadio( ACTION *, int ) ;
extern void ENE_ActCallRadio( ACTION *, int ) ;
extern void ENE_ActRadioBreak( ACTION *, int ) ;
extern void ENE_ActReturnRadio( ACTION *, int ) ;
extern void ENE_ActRadioCall( ACTION *, int ) ;
extern void ENE_ActNSight_ON( ACTION *, int ) ;
extern void ENE_ActNSight_OFF( ACTION *, int ) ;
extern void ENE_ActMedication( ACTION *, int ) ;
extern void ENE_ActReadyGun( ACTION *, int ) ;
extern void ENE_ActNearAtk( ACTION *, int ) ;
extern void ENE_ActKeri( ACTION *, int ) ;
extern void ENE_ActMgunShot( ACTION *, int ) ;
extern void ENE_ActReload( ACTION *, int ) ;
extern void ENE_ActSetGlass( ACTION *, int ) ;
extern void ENE_ActGlass( ACTION *, int ) ;
extern void ENE_ActCleSign( ACTION *, int ) ;
extern void ENE_ActLockerOpen( ACTION *, int ) ;
extern void ENE_ActUnreal( ACTION *, int ) ;
extern void ENE_ActZzz( ACTION	*act, int time ) ;


//motion.h
extern int EnemyMotionArray21[] ;
extern int EnemyMotionArray[] ;


//command.c
extern int		CM_TMPSNAKEAREA ;
extern int		DEF_SEARCH_LEVEL ;
extern int		ENE_AlertGameLevel ;
extern int		ENE_GameStatus ;


extern u_int COM_GetRootOfset( void ) ;
extern u_int COM_GetClearingOfset( void ) ;
extern void COM_SetAvoidTime( int time ) ;
extern int COM_SameGroupUnit( ENETHINK *entk, ENETHINK *you ) ;
extern int COM_GetGroupNum( ENETHINK *entk ) ;
extern int COM_GetInvestigatedNearID( ENETHINK *entk ) ;
extern int COM_SetInvestigated( int mode, FVECTOR *pos,HZX_GROUP_ID gid ) ;
extern void COM_SetAlertStatus( int status ) ;
extern int COM_AlertStatus( void ) ;

extern int COM_CheckBoxPos( FVECTOR *pos ) ;
extern void COM_SetBoxPos( FVECTOR *pos ) ;
extern int COM_GetUnitBuff( ENETHINK *, int ) ;
extern int	COM_SetAccident( int uniq_id, FVECTOR *pos, HZX_GROUP_ID map, int delay, int status ) ;
extern void	COM_UnsetAccident( int num ) ;
extern void	COM_FreeReservAccident( int uniq_id ) ;
extern void	COM_ClearAccident( void ) ;
extern int	COM_AccidentToID( int id ) ;
extern int COM_Accident( void ) ;
extern FVECTOR *COM_GetAccidentPos( int num ) ;
extern HZX_GROUP_ID COM_GetAccidentMap( int num ) ;
extern int COM_GetAccidentStatus( int num ) ;
extern int COM_RouteProcCall( int route, int point, ENETHINK *entk ) ;
extern ENETHINK *COM_GetEnemyFromOldUniqID( int id ) ;
extern ENETHINK *COM_GetEnemyFromUniqID( int id ) ;
extern ENETHINK *COM_GetEnemyFromName( int name ) ;
extern int COM_GetCaptureModelName( void ) ;
extern int COM_GetCaptureName( void ) ;
extern int COM_GetEnemyStatus( void ) ;
extern int COM_GetEnemyPos( void ) ;
extern int COM_GetCleRoute( int ) ;
extern int COM_GetClePoint( int ) ;
extern int COM_SetClearingData( void ) ;
extern void COM_InitTmpCleProc( void ) ;
extern void COM_CleProc( int, int, int ) ;
extern void COM_SetFlameFlag( int ) ;
extern int	COM_ResurrectionPermit( ENETHINK *entk ) ;
extern void	COM_CypherDestroy( void ) ;
extern int	COM_SetCypher( void ) ;
extern int	COM_CypherResurrectionPermit( void ) ;
extern void COM_GetPlayerLastPos( FVECTOR *, int * ) ;
extern void COM_SetPlayerLastPos( FVECTOR *, int ) ;
extern void COM_SetSecurityAlert( FVECTOR *, int ) ;
extern void COM_SetSecurityAlertLevel( FVECTOR	*pos, int map, int level ) ;
extern ENETHINK *COM_GetDamageEnemy( ENETHINK * ) ;
extern int COM_GetCorpID( ENETHINK * ) ;
extern int COM_UnitThkStatus( int g_id, int u_id ) ;
extern ENETHINK *COM_NearEnemyThkStatus( ENETHINK *fromene, long64 status, int *dis ) ;
extern ENETHINK *COM_NearEnemyNotActStatus( FVECTOR *pos, long64 status, int *dis ) ;
extern void COM_UnitMemberDie( int, int ) ;
extern int COM_GetUnitMemberDieNum( int, int ) ;	/* 死亡人数確認 */
extern ENETHINK	*COM_GetMemberFromGUI( int g_id, int u_id, int id ) ;
extern void COM_SetSearchLevel( int ) ;
extern int COM_GetSearchLevel( void ) ;
extern void COM_WaitingPosition( FVECTOR *, int * ) ;


extern int COM_GetNoisePower( void ) ;
extern int COM_GetNoiseNearID( ENETHINK * ) ;
extern COMMANDER *COM_GetCommander( void ) ;
extern E_GROUP *COM_GetGroup( int ) ;
extern E_UNIT *COM_GetUnit( int, int ) ;
extern int COM_SetGroup( ENEMYS *, E_GROUP * ) ;
extern int COM_SetUnit( E_GROUP *, E_UNIT * ) ;
extern void COM_UnsetEnethinkWork( ENETHINK * ) ;
extern int COM_SetEnethinkWork( ENETHINK *, int, int ) ;
extern int COM_GetUniqID( void ) ;
extern int COM_StageKind( void ) ;
extern void	COM_SetIDNumber( ENETHINK * ) ;
extern void COM_ResurrectionPosition( FVECTOR *, int * ) ;
extern ENETHINK *COM_SetSupportEnemy( int ) ;
extern void COM_DetectVibration( void ) ;
extern void COM_HangVibration( void ) ;
extern int COM_AnyoneStatus_G_U( COMMANDER *com, int g_id, int u_id, long64 status ) ;
extern int COM_AnyoneStatus_G( COMMANDER *com, int g_id, long64 status ) ;

extern ENETHINK *ENE_NearEnemy_OtherUnit( ENETHINK * ) ;
extern int COM_EnemyActStatusNum( long64 status ) ;
extern ENETHINK *COM_EnemyActStatus( long64 status ) ;
extern int COM_EnemyActNotStatusNum( long64 status ) ;


//endamact.c
extern int ENE_EnemyDamagePad( ACTION * ) ;
extern void ENE_EnemyDown( ACTION * ) ;
extern void ENE_SetEnemyDown( ACTION *act, int faint, int time, int pose ) ;


//corpcom.c
extern void CP_InitCorpSystem( void ) ;
extern int GetUniqName( void ) ;
extern void FreeUniqName( u_int ) ;
extern int CP_NewCorpNum( void ) ;
extern int CP_OldCorpNum( void ) ;
extern void CP_SetCorp( NEWCORP	*, CONTROL *, int *, int, ENETHINK * ) ;
extern void CP_PutCorp( NEWCORP * ) ;
extern void CP_FreeCorp( NEWCORP * ) ;
extern int CP_GetCorpID( ENETHINK * ) ;
extern NEWCORP *CP_GetCorp( int ) ;
extern int CP_CaptureCorpKill( void ) ;
extern ENETHINK *CP_CaptureCorpModelName( void ) ;
extern void CP_SetOldCorp( OLDCORP *, FVECTOR * ) ;
extern void CP_PutOldCorp( OLDCORP * ) ;
extern void CP_FreeOldCorp( OLDCORP * ) ;

//clearing.c
extern int ENE_CLBoundCheck( FVECTOR * ) ;
extern HZX_CLE_AREA *ENE_GetClearArea( int ) ;
extern HZX_CLE_ROOT *ENE_GetClearRoot( HZX_CLE_AREA *, int ) ;
extern HZX_CLE_PTP *ENE_GetClearPoint( HZX_CLE_ROOT *, int ) ;
extern HZX_CLE_PTP *ENE_GetClearPoint2( int, int, int ) ;
extern void	MakeClearing( ) ;

//atavoid.c
extern int	COM_ClearingCancel( void ) ;
extern int	COM_ClearingGoNormal( void ) ;

//speak.c
extern	void	COM_SetSpeak( int, ENETHINK * ) ;
extern	int 	COM_CallRouteVoice( int route, int point, int v_flag, ENETHINK *entk ) ;
extern	int	COM_CallClearingVoice( int area, int route, int point, int v_flag, ENETHINK *entk ) ;
extern	int		COM_SetRadio( int, ENETHINK * ) ;
extern	void	COM_StopRadio( ENETHINK *entk ) ;
extern	void	COM_StopRadioNoAccident( ENETHINK *entk ) ;
extern	int		COM_GetStrmCount( void ) ;
extern	int		COM_GetStrmLv( void ) ;
extern	void	COM_InitSpeak( void ) ;

//notice.c
extern void ENE_NoticeCheck( ENETHINK *entk ) ;

//**think.c
extern void THK_TouchMode( ENETHINK * ) ;
extern void THK_TouchModeStart( ENETHINK * ) ;

extern void THK_NoiseMode( ENETHINK * ) ;
extern void THK_NoiseModeStart( ENETHINK * ) ;
extern void THK_NoiseModeStartClearing( ENETHINK * ) ;

extern void THK_FoundMode( ENETHINK * ) ;
extern void THK_FoundModeStart( ENETHINK * ) ;
extern void THK_FoundModeFastStart( ENETHINK * ) ;

extern void THK_TraceMode( ENETHINK * ) ;
extern void THK_TraceModeStart( ENETHINK * ) ;
extern void THK_TraceModeStartCle( ENETHINK * ) ;
extern void THK_TraceModeStartCleFast( ENETHINK * ) ;

extern void THK_UniformMode( ENETHINK * ) ;
extern void THK_UniformModeStart( ENETHINK * ) ;

extern void THK_BoxMode( ENETHINK * ) ;
extern void THK_BoxModeStart( ENETHINK * ) ;
extern void THK_BoxModeClearingStart( ENETHINK * ) ;
extern void THK_BoxModeNearStart( ENETHINK * ) ;

extern void THK_InvestigatedMode( ENETHINK * ) ;
extern void THK_InvestigatedModeStart( ENETHINK * ) ;

extern void THK_AccidentReportMode( ENETHINK * ) ;
extern void THK_AccidentReportModeStart( ENETHINK * ) ;

extern void THK_Discovery( ENETHINK * ) ;
extern void THK_DiscoveryModeStart( ENETHINK * ) ;
extern void THK_DiscoveryNoiseModeStart( ENETHINK * ) ;
extern void THK_DiscoveryNpcModeStart( ENETHINK * ) ;
extern void THK_DiscoveryModeStartAvoid( ENETHINK * ) ;
extern void THK_DiscoveryModeStartAvoidDelay( ENETHINK * ) ;

extern void THK_Damage( ENETHINK * ) ;
extern void THK_DamageMedicalModeStart( ENETHINK * ) ;
extern void THK_DamageModeStart( ENETHINK * ) ;

extern void	THK_HoldUpMode( ENETHINK	*entk ) ;
extern void	THK_HoldUpModeStart( ENETHINK	*entk ) ;
extern void	THK_HoldUpModeStartDamage( ENETHINK	*entk ) ;

extern void	THK_IndistinctMode( ENETHINK	*entk ) ;
extern void	THK_IndistinctModeStart( ENETHINK	*entk ) ;

extern void	THK_CorpsMode( ENETHINK	*entk ) ;
extern void	THK_CorpsModeStart( ENETHINK	*entk ) ;

extern void	THK_EneDamage( ENETHINK	*entk ) ;
extern void	THK_EneDamageModeStart( ENETHINK	*entk ) ;

extern void	THK_AccidentMode( ENETHINK	*entk ) ;
extern void	THK_AccidentModeStart( ENETHINK	*entk ) ;

extern void	THK_RadioMode( ENETHINK	*entk ) ;
extern void	THK_RadioModeStart( ENETHINK	*entk ) ;


//enedamag.c
extern void	ENE_EnemyStartModeDamage( ENETHINK	*entk ) ;
extern void	ENE_Enemy_Think1_Damage( ENETHINK	*entk ) ;
extern void	ENE_EnemyStartModeDamageDeath( ENETHINK	*entk ) ;

//wcsneak.c
extern void	ENE_Watcher_Think1_Sneak( ENETHINK	*) ;
extern void	ENE_WatcherStartModeSneak( ENETHINK	*) ;
extern void	ENE_WatcherStartModeSneakDamage( ENETHINK	*) ;

//wcavoid.c
extern void ENE_Watcher_Think1_Avoid( ENETHINK * ) ;
extern void ENE_WatcherStartModeAvoid( ENETHINK * ) ;
extern void ENE_WatcherStartModeAvoidWarp( ENETHINK * ) ;
extern void ENE_WatcherReturnModeAvoid( ENETHINK * ) ;
extern void ENE_WatcherStartModeAvoidDamage( ENETHINK * ) ;

//wcalert.c
extern void ENE_Watcher_Think1_Alert( ENETHINK * ) ;
extern void ENE_WatcherStartModeAlert( ENETHINK * ) ;
extern void ENE_WatcherStartModeAlertWarp( ENETHINK * ) ;

//wcress.c
extern void ENE_WatcherResurrect( ENETHINK * ) ;
extern void ENE_WatcherResurrectionMode( ENETHINK * ) ;

//atavoid.c
extern void	ENE_Attacker_Think1_Avoid( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeAvoid( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeAvoidWarp( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeAvoidEndClearing( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeAvoidReturnClearing( ENETHINK	* ) ;
extern void	ENE_AttackerClearingModeDamage( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeEndClearingToNormal( ENETHINK	* ) ;

//atsneak.c
extern void	ENE_Attacker_Think1_Normal( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeSneak( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeSneakDamage( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeSearchToSneak( ENETHINK	* ) ;

//atsearch.c
extern void	ENE_Attacker_Think1_Search( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeSearch( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeSearchDamage( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeAvoidToSearch( ENETHINK	* ) ;
extern void	ENE_AttackerStartModeSearchWarp( ENETHINK	* ) ;

//spalert.c
extern void	ENE_Support_Think1_Alert( ENETHINK	* ) ;
extern void	ENE_SupportStartModeAlert( ENETHINK	* ) ;

//spavoid.c
extern void	ENE_Support_Think1_Avoid( ENETHINK	* ) ;
extern void	ENE_SupportStartModeAvoid( ENETHINK	* ) ;
extern void	ENE_SupportReturnModeAvoid( ENETHINK	* ) ;
extern void	ENE_SupportStartModeAvoidDamage( ENETHINK	* ) ;

//spcautio.c
extern void	ENE_Support_Think1_Caution( ENETHINK	* ) ;
extern void	ENE_SupportStartModeCaution( ENETHINK	* ) ;
extern void	ENE_SupportStartModeCautionDamage( ENETHINK	* ) ;

//user/sigeno/attaker/sigavoid.c
extern void SIG_AttackerStartModeAvoidPosSet( ENETHINK *entk, FVECTOR *pos, int map ) ;

