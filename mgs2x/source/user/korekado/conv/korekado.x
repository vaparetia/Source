/*
	korekado.c
	ユーティリティーのプロトタイプ

	1997/07/07 Y.Korekado
	$Id: korekado.x,v 1.1.1.3 2002/11/19 11:44:03 Yoshizawa1 Exp $
	
*/
	/* プロトタイプいるよー */
extern	void	KR_AlertCount( void ) ;
extern	void	KR_KillCount( void ) ;
extern	void	KR_ClearingCount( void ) ;
extern	int		ENE_GetGRotFromPos( FVECTOR *pos, SVECTOR *rot, float z, HZX_GROUP_ID hzx_id, int seg_type, int flr_type ) ;
extern	float	KR_GetGRotFromPos(FVECTOR *pos, HZX_GROUP_ID hzx_id, int seg_type, int flr_type ) ;
extern	float	KR_InnerProductInCamera( FVECTOR *pos ) ;
extern	float	KR_InnerProduct( FVECTOR *p, FVECTOR *v1, FVECTOR *v2 ) ;
extern	int		KR_CameraDis( FVECTOR *pos ) ;
extern	void	KR_UnsetAllObjsFlag( DG_OBJS *objs, int flag ) ;
extern	void	KR_SetAllObjsFlag( DG_OBJS *objs, int flag ) ;
extern	float	KR_MotionStepSpeed( MOTION_CONTROL *mt_ctrl ) ;
extern	int		KR_MotionTime( MT3_CONTROL *mt3_ctrl ) ;
extern	int		KR_LeftMotion( MT3_CONTROL *mt3_ctrl ) ;
extern	int 	KR_MapConnect( int map1, int map2 ) ;
extern	void	KR_FindCameraCall( OBJECT *body, FVECTOR *pos, int map ) ;
extern	int		KR_CheckTrap( CONTROL *ctrl, int trap_id ) ;
extern	int		KR_GetGRot( CONTROL *, float ) ;
extern	void	KR_DestroyActor( void ** ) ;
extern	int		KR_RandU( int ) ;
extern	int		KR_RandS( int ) ;
extern	int		KR_GetDownDir( CONTROL *, int ) ;
extern	void	KR_MemCopy( void *, void *, int , int  ) ;
extern	void	KR_GroupObject( OBJECT *, int ) ;
extern	void	_CopyObjectWorld( DG_OBJS *, DG_OBJS * ) ;
extern	int		MatToYRot( FMATRIX * ) ;
extern	int		MatToXRot( FMATRIX * ) ;
extern	void	KR_FMatToFvec( FMATRIX	*, FVECTOR * ) ;
extern	void	KR_FvecToMat( FVECTOR *, FMATRIX * ) ;
extern	void	_ScaleMatrix( FMATRIX *, FVECTOR * ) ;
extern	void	_RotMatrixZYX( FMATRIX *, SVECTOR * ) ;
extern	void	_FRotQuat( FVECTOR *, FVECTOR * ) ;
extern	void	_RotQuat( FVECTOR *, SVECTOR * ) ;
extern	int		_FVecTrgDir2( FVECTOR *, FVECTOR * ) ;
extern	int		KR_FVecTrgDis2( FVECTOR *, FVECTOR * ) ;
extern	int		_FVecTrgDis( FVECTOR *, FVECTOR * ) ;
extern	float	_FVecLen2( FVECTOR * ) ;
extern	int		_FVecLen3( FVECTOR * ) ;
extern	int		_FVecDir2( FVECTOR * ) ;
extern	int		_FVecDirX( FVECTOR * ) ;
extern	void	_FVecToRotXY( FVECTOR *, SVECTOR * ) ;
extern	void	_FVecToRotYZ( FVECTOR *, SVECTOR * ) ;
extern	float	_RsinF( int ) ;
extern	float	_RcosF( int ) ;
extern	int		_DiffDirAbs( int, int ) ;
extern	int		_DiffDisAbs( FVECTOR *, FVECTOR * ) ;

extern	int		_PosInRangeXZ( FVECTOR *, FVECTOR *, int ) ;

extern	int		TSP_map[7][7] ;
extern	int		TSP( int, int * ) ;

extern	void	ENE_ExecProc( int proc_id, GCL_ARGS *arg ) ;
extern	int		ENE_GCL_GetFV( char *, FVECTOR * ) ;
extern	int		ENE_GCL_GetFVs( FVECTOR * ) ;
extern	int		ENE_GCL_GetFVandM( FVECTOR *, int * ) ;
extern	int		ENE_PreShade( DG_OBJS *, int ) ;
extern	int		ENE_GclCallProcs( char * ) ;
extern	int		ENE_GclGetInt( char *, int * ) ;
extern	int		ENE_GclGetProc( int *, int * ) ;
extern	void	KR_ProcCallSendName( int proc, int name ) ;
