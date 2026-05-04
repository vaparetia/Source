extern int SIG_CheckDirSub(int ,int ) ;
extern int SIG_GetFarZone3(int ,int ,int ,int) ;
extern int Sig_GetRoute(HZX_GROUP_ID ,int , int );
extern int SIG_ZoneBoundCheck(HZX_ZON *,float) ;
extern int SIG_BoundCheck(FVECTOR *, float ,float );
extern int SIG_CheckSafeType(ENETHINK *,int ) ;
extern int Sig_InsideZone( HZX_ZON *,FVECTOR *,float) ;
extern int Sig_InsideZoneWide( HZX_ZON *, FVECTOR * ,float);
extern int SIG_GetRIntrptZ2Z( HZX_GROUP_ID , int , int ) ;
extern void SIG_SetZonePos2Fvec(HZX_ZON * ,FVECTOR * ) ;
extern void SIG_SetChasePos(ENETHINK * ,FVECTOR * ) ;
extern int CheckAnesAT(ENETHINK * ) ;
extern int CheckNoActive(ENETHINK * ) ;
extern int CheckLastAT(ENETHINK * ) ;
extern int CheckRescueAll(ENETHINK * ) ;
extern int SIG_CheckRIntrpt(ENETHINK * ) ;
extern int CheckOnlyRIntrpt(ENETHINK * ) ;
extern int CheckJumpPosENE( ENETHINK * ) ;
extern void SetMoveMode(ENETHINK * ) ;
extern float CheckGunPlOnline(ENETHINK * ,FVECTOR * ) ;
extern int SIG_CheckDirSub(int ,int ) ;
extern int SIG_GetFarDirZone( HZX_GROUP_ID , int , int ) ;
extern int CheckNoSearch(int) ;
extern void SIG_CheckCorner(ENETHINK * ) ;
extern ENETHINK *GetAtRanking(ENETHINK	* ,int ) ;
extern int GetNewSerch( int ,int ) ;
extern int SIG_GetFarZone3(int ,int ,int ,int ) ;
extern void SIG_SetRandFvec(FVECTOR * ,int) ;
extern void SIG_BasicShoot( ENETHINK * ,FVECTOR * ,int ) ;
extern void SIG_RandShoot( ENETHINK * ) ;
extern int SIG_CheckTrap( CONTROL * ,u_int ) ;
extern int SIG_EyeInsideCheck( SENSEPARAM *,ENETHINK * ,FVECTOR *) ;
extern int SIG_NktCheck(ENETHINK * ) ;
extern void SIG_MultiTexOn(OBJECT * ) ;
extern void SIG_MultiTexOff(OBJECT * ) ;
extern void SIG_SetTrgpPlayerLastpos( TRGPOINT	*,COMMANDER *) ;
extern float SIG_VecLen3F2D( FVECTOR * , FVECTOR * ) ;
extern int SIG_CheckFrontSeg(CONTROL *,float) ;
extern void SIG_AT_VoiceCall(ENETHINK *,int,int ) ;
extern SNC_EXTERN_INLINE int SIG_CheckStealthStatus(ENETHINK *entk) ;
extern void SIG_AllObjectInvisible(OBJECT * ) ;
extern void SIG_AllObjectVisible(OBJECT *) ;
extern void Sig_SetTrgpInsideZone(ENETHINK *) ;
extern void SIG_SetSpeak(int ,ACTION *);
extern void SIG_RandShoot_TNG( ENETHINK	* ) ;
