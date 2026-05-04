/*
   pl_inline.h
   ライデン／グローバルインライン関数
   プラグイン関数内でインクルードされる
   
   2000/03/23 M.Sonoyama
   $Id: pl_inline.h,v 1.1.1.3 2002/11/19 11:50:58 Yoshizawa1 Exp $
   */

#ifndef __pl_inline_h__
#define	__pl_inline_h__

#ifdef KP_XBOX
// XboxではSNC_EXTERN_INLINEはやめておく。
#undef SNC_EXTERN_INLINE
#define SNC_EXTERN_INLINE
#endif

extern	SNC_EXTERN_INLINE	void	PL_IVtoFV( int *, float * ) ;
extern	SNC_EXTERN_INLINE	void	PL_SetStatus( long64 ) ;
extern	SNC_EXTERN_INLINE	void	PL_UnsetStatus( long64 ) ;
extern	SNC_EXTERN_INLINE	long64	PL_Status( long64 ) ;
extern	SNC_EXTERN_INLINE	void	PL_SetFlag( long64 ) ;
extern	SNC_EXTERN_INLINE	void	PL_UnsetFlag( long64 ) ;
extern	SNC_EXTERN_INLINE	long64	PL_Flag( long64 ) ;
extern	SNC_EXTERN_INLINE	void	PL_SetFlag2( long64 ) ;
extern	SNC_EXTERN_INLINE	void	PL_UnsetFlag2( long64 ) ;
extern	SNC_EXTERN_INLINE	long64	PL_Flag2( long64 ) ;
extern	SNC_EXTERN_INLINE void	PL_SetVWait( PlayerWork *, int ) ;
extern	SNC_EXTERN_INLINE	int		PL_FrameTime( int ) ;
extern	SNC_EXTERN_INLINE	void	PL_StopTurn( PlayerWork * ) ;
extern	SNC_EXTERN_INLINE	void	PL_MatToVec( FMATRIX *, FVECTOR * ) ;
extern	SNC_EXTERN_INLINE	void	PL_InitBehindPeepCamera( PlayerWork * ) ;
extern	SNC_EXTERN_INLINE	void	PL_AddFindObj( int ) ;
extern	SNC_EXTERN_INLINE	void	PL_ResetAddress( PlayerWork * ) ;
extern	SNC_EXTERN_INLINE	float	PL_ObjHeight( PlayerWork *, int ) ;
extern	SNC_EXTERN_INLINE	int	PL_GetMessageValue( GV_MSG *, int, int ) ;
extern	SNC_EXTERN_INLINE	int	PL_WeaponType( PlayerWork * ) ;
extern	SNC_EXTERN_INLINE	int	PL_ItemType( PlayerWork * ) ;

extern	void	PL_SetAction3( PlayerWork *, int, int, int ) ;
extern	void	PL_SetAction2( PlayerWork *, int, int, int ) ;
extern	void	PL_SetAction4( PlayerWork *, int, float, int ) ;
extern	void	PL_SetAction2Ex( PlayerWork *work, int action, int time, float t, 
								 int interp, long64 mask ) ;
extern	void	PL_SetAction2Check( PlayerWork *work, int action, int interp, int mask ) ;

extern	void	PL_SetArmAction( PlayerWork *, int ) ;
//SNC_EXTERN_INLINE	void	PL_SetArmAction3( PlayerWork *work, int action, int time, int interp ) ;
//SNC_EXTERN_INLINE	void	PL_SetArmAction4( PlayerWork *work, int action, float t, int interp ) ;
extern	void	PL_SetArmAction3( PlayerWork *work, int action, int time, int interp ) ;
extern	void	PL_SetArmAction4( PlayerWork *work, int action, float t, int interp ) ;
extern	void	PL_SetMode( PlayerWork *, PL_ACTION ) ;
extern	void	PL_SetMode2( PlayerWork *, PL_ACTION ) ;

extern	SNC_EXTERN_INLINE	void	PL_SetModeName( PlayerWork *, PL_ACTION, char * ) ;
extern	SNC_EXTERN_INLINE	void	PL_ModeName( PlayerWork *work, char *name ) ;
extern	SNC_EXTERN_INLINE	int		PL_CheckMode( PlayerWork *work, char *name ) ;

extern	SNC_EXTERN_INLINE	int		PL_MotionTime( PlayerWork * ) ;
extern	SNC_EXTERN_INLINE	int		PL_MotionTime2( PlayerWork * ) ;
extern	SNC_EXTERN_INLINE	int	PL_CheckMotionTime( PlayerWork *, int time ) ;
extern	SNC_EXTERN_INLINE	int	PL_CheckMotionTime2( PlayerWork *, int time ) ;
extern	SNC_EXTERN_INLINE	float	PL_CheckMotionRate( PlayerWork *work ) ;

extern	SNC_EXTERN_INLINE	void	PL_ObjPos( PlayerWork *, int, FVECTOR * ) ;
extern	SNC_EXTERN_INLINE	void	PL_ArmObjPos( int n, FVECTOR *pos ) ;

extern	SNC_EXTERN_INLINE	FMATRIX	*PL_ObjWorld( PlayerWork *, int ) ;
extern	SNC_EXTERN_INLINE	int		PL_CheckInvisible( PlayerWork * ) ;

extern	void			PL_ChangeMotionArc( PlayerWork *, int ) ;
extern	SNC_EXTERN_INLINE	void	PL_ReturnMotionArc( PlayerWork *work ) ;
extern	SNC_EXTERN_INLINE	void	PL_MotionSleep( PlayerWork *, int ) ;
extern	SNC_EXTERN_INLINE	void	PL_MotionActive( PlayerWork *, int ) ;

extern	SNC_EXTERN_INLINE	void	PL_SetDeadFlag( PlayerWork * ) ;

extern	SNC_EXTERN_INLINE	int	PL_UseStickL( PlayerWork *work ) ;
extern	SNC_EXTERN_INLINE	int	PL_UseStickR( PlayerWork *work ) ;

extern	SNC_EXTERN_INLINE	void	PL_EndForceAct( PlayerWork *work, int flag, int dir ) ;

#define	IVtoFV( _i, _f ) 	PL_IVtoFV( _i, _f )
#define	SetStatus( _i ) 	PL_SetStatus( _i ) 
#define	UnsetStatus( _i )  	PL_UnsetStatus( _i ) 
#define	Status( _i ) 		PL_Status( _i ) 
#define	SetFlag( _i ) 		PL_SetFlag( _i ) 
#define	UnsetFlag( _i ) 	PL_UnsetFlag( _i ) 
#define	Flag( _i ) 		PL_Flag( _i ) 
#define	SetFlag2( _i ) 		PL_SetFlag2( _i ) 
#define	UnsetFlag2( _i ) 	PL_UnsetFlag2( _i ) 
#define	Flag2( _i ) 		PL_Flag2( _i ) 
#define	SetVWait( _w, _i ) 	PL_SetVWait( _w, _i ) 
#define	FrameTime( _i ) 	PL_FrameTime( _i ) 
#define	StopTurn( _w ) 		PL_StopTurn( _w ) 
#define	MatToVec( _m, _v )	PL_MatToVec( _m, _v )
#define	InitBehindPeepCamera( _w ) 	PL_InitBehindPeepCamera( _w ) 
#define	AddFindObj( _i ) 	PL_AddFindObj( _i ) 
#define	ResetAddress( _w ) 	PL_ResetAddress( _w ) 
#define	ObjHeight( _w, _i ) 	PL_ObjHeight( _w, _i ) 
#define	GetMessageValue( _m, _i, _j ) 	PL_GetMessageValue( _m, _i, _j ) 
#define	WeaponType( _w ) 	PL_WeaponType( _w ) 
#define	ItemType( _w ) 		PL_ItemType( _w ) 

#define	SetAction( _w, _i, _j ) 	PL_SetAction3( _w, _i, 0, _j ) 
#define	PL_SetAction( _w, _i, _j ) 	PL_SetAction3( _w, _i, 0, _j ) 
#define	SetAction2( _w, _i, _j, _k ) 	PL_SetAction2( _w, _i, _j, _k ) 
#define	SetAction2Check( _w, _i, _j, _k ) 	PL_SetAction2Check( _w, _i, _j, _k ) 
#define	SetAction3( _w, _i, _j, _k ) 	PL_SetAction3( _w, _i, _j, _k ) 
#define	SetAction4( _w, _i, _j, _k ) 	PL_SetAction4( _w, _i, _j, _k ) 
#define	SetAction2Ex( _w, _a, _ti, _t, _i, _m )	PL_SetAction2Ex( _w, _a, _ti, _t, _i, _m )

#define	SetArmAction( _w, _i ) 	PL_SetArmAction( _w, _i ) 
#define	SetArmAction3( _w, _a, _t, _i ) 	PL_SetArmAction3( _w, _a, _t, _i ) 
#define	SetArmAction4( _w, _a, _t, _i ) 	PL_SetArmAction4( _w, _a, _t, _i ) 
#define	SetMode( _w, _i ) 	PL_SetMode( _w, _i ) 
#define	SetMode2( _w, _i ) 	PL_SetMode2( _w, _i ) 
#define	SetModeName( _w, _i, _n )	PL_SetModeName( _w, _i, _n )

#define	MotionTime( _w )	PL_MotionTime( _w )
#define	MotionTime2( _w )	PL_MotionTime2( _w )

#define	ObjPos( _w, _n, _p )	PL_ObjPos( _w, _n, _p )	
#define	ObjWorld( _w, _n )		PL_ObjWorld( _w, _n )
#define	CheckInvisible( _w ) 	PL_CheckInvisible( _w ) 
#define	ChangeMotionArc( _w, _m )	PL_ChangeMotionArc( _w, _m )

/* モーション終了検知 */
//#define	EndMotion( work )	GM_CheckObject_IsEnd( &( work->body ), 0 )
//#define	EndMotion2( work )	GM_CheckObject_IsEnd( &( work->body ), 1 )
//#define	EndMotionAll( work )	GM_CheckObject_PlayEnd( &( work->body ), 0 ) 

#define	EndMotion( work )		MT_CHECK_LAST1( work->body.m_ctrl, 0 ) 
#define	EndMotion2( work )		MT_CHECK_LAST1( work->body.m_ctrl, 1 ) 
#define	EndMotionAll( work )	MT_CHECK_END( work->body.m_ctrl, 0 ) 

#endif



