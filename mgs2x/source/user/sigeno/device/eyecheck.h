/*
	eyecheck.h
	2000/07/06 K.Sigeno
	$Id: eyecheck.h,v 1.1.1.3 2002/11/19 11:49:29 Yoshizawa1 Exp $
*/
typedef	struct{
	SVECTOR		turn;
	SVECTOR		rot;
	int			interp;
} DEV_CTRL ;

#ifdef DEBUG_MODE
#define DEV_SIGHT_OFF	(0x01) /*視界OFF*/
#endif

extern int SIG_EyeCheck( EYEPARAM * ) ;
extern void SIG_VecDir(FVECTOR *,FVECTOR *,SVECTOR *) ;
extern void DEV_DirLimitX(SVECTOR *  ,int,int ) ;
extern void DEV_DirLimitY(SVECTOR *  ,int,int ) ;
extern void DEV_DirLimitY2(SVECTOR *  ,int,int,short ) ;

extern void DEV_No_Sight(EYEPARAM * ) ;
extern void DEV_In_Sight(EYEPARAM * ) ;
extern void DEV_In_SightSub(EYEPARAM *, FVECTOR *, int );
extern void DEV_Security(EYEPARAM * ) ;
extern void DEV_HeadMarkCheck(EYEPARAM *, FMATRIX *,int *) ;
extern int DEV_EyeInfoCheck(EYEPARAM * , HZX_GROUP_ID ) ;
extern void DEV_Bullet(EYEPARAM *eye,FMATRIX * ) ;
extern void DEV_InitDevCtrl(DEV_CTRL *ctrl,int interp) ;

extern void DEV_InterpRot(DEV_CTRL * ) ;
extern void DEV_CopySvec2Fvec(SVECTOR * ,FVECTOR * ) ;

extern int DEV_EnemyStatus_Id( EYEPARAM * ) ;

extern void DEV_InitEyeParam(EYEPARAM *,SVECTOR * ,float) ;
extern void DEV_CameraSe(EYEPARAM * ) ;

#ifdef DEBUG_MODE
extern void DEV_DebugMode(EYEPARAM *);
#endif

extern int DEV_CheckDirSub(int ,int );

#define	DISCV_DELAY		(90)	/*捉えてから通報までの時間差*/
#define	HEADMARK_DELAY	(60) /*捉えてからビックリまで*/

#define	HEADMARK_SET	(DISCV_DELAY - HEADMARK_DELAY)
#define	SIGHT_ALERT	120	 /*補足し続ける時間*/
#define	SIGHT_ALERT2	360	 /*白びっくり補足時間補足し続ける時間*/
#define	RADAR_RADIO_TIME (SIGHT_ALERT2 - HEADMARK_DELAY)
#define	DEV_ACCIDENT_DELAY_TIME	(600)
#define	CAM_SE_INT	(4)	/*カメラ首振りＳＥ間隔*/
#define	CAM_SE_ROT	(2)	/*SEを鳴らす角速度*/
//GM_NikitaPosition[ 0 ]
