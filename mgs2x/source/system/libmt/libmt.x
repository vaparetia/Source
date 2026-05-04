/*
	libmt.x
	モーション再生ライブラリプロトタイプ宣言

	1999/07/07 K.Takabe
	$Id: libmt.x,v 1.1.1.3 2002/11/19 11:42:51 Yoshizawa1 Exp $

*/

/* quat.c */
extern void MT_QuatToMat( FMATRIX *mat, FVECTOR *quat );
extern void MT_EulerToQuatXYZ( FVECTOR *quat, FVECTOR *rot);
extern void MT_MatToQuat( FVECTOR *quat, FMATRIX *mat );
extern void MT_QuatSlerp( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t );
extern void MT_QuatNormalize( FVECTOR *res, FVECTOR *quat );
extern void MT_QuatMul( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );

/* quat2.c */
extern void MT_QuatGetValue( FVECTOR *res, FVECTOR *quat );
extern void MT_QuatSetValue( FVECTOR *quat, FVECTOR *param );
extern void MT_QuatScaleAngle(FVECTOR *res, FVECTOR * quat, float scale );
extern void MT_QuatInverse( FVECTOR *res, FVECTOR *quat );
extern void MT_QuatSetFromAx( FVECTOR *quat, FVECTOR *from, FVECTOR *to );
extern void MT_QuatAdd( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatSub( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatDiv( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatSquare( FVECTOR *res, FVECTOR *q1 );
extern void MT_QuatSqrt( FVECTOR *res, FVECTOR *q1 );
extern float MT_QuatDot( FVECTOR *q1, FVECTOR *q2 );
extern float MT_QuatLength( FVECTOR *q1 );
extern void MT_QuatNegate( FVECTOR *res, FVECTOR *quat );
extern void MT_QuatExp( FVECTOR *res, FVECTOR *q1 );
extern void MT_QuatLog( FVECTOR *res, FVECTOR *q1 );
extern void MT_QuatLnDif( FVECTOR *res, FVECTOR *q1, FVECTOR *q2 );
extern void MT_QuatSquad( FVECTOR *res, FVECTOR *p, FVECTOR *a, FVECTOR *b, FVECTOR *q, float t );
extern void MT_QuatQCompA( FVECTOR *res, FVECTOR *q_prev, FVECTOR *q, FVECTOR *q_next );
extern void MT_QuatSeparate(FVECTOR *s,FVECTOR *r,FVECTOR *n);
extern void MT_QuatSeparate2(FVECTOR *s,FVECTOR *n,FVECTOR *r);

/* quat3.c */
extern void MT_EulerToQuatZYX( FVECTOR *quat, FVECTOR *euler );
extern void MT_QuatToEulerZYX( FVECTOR *euler, FVECTOR *quat );
extern void MT_EulerToQuatYZX( FVECTOR *quat, FVECTOR *euler );
extern void MT_QuatToEulerXZY( FVECTOR *euler, FVECTOR *quat );
extern void MT_EulerToQuatXZY( FVECTOR *quat, FVECTOR *euler );

/* math_utl.c */
extern void MT_SinX4( FVECTOR *res, FVECTOR *x );
extern void MT_CosX4( FVECTOR *res, FVECTOR *x );
extern void MT_Atan2X4( FVECTOR *res, FVECTOR *y, FVECTOR *x );

/* motion.c */
extern void *MT_InitMotion( DG_OBJS *objs, int n_layer, int motion, int flag );
extern void MT_ActMotion( MOTION_CONTROL *m_ctrl, DG_OBJS *objs, DG_EVMOBJ *evmobj );
extern void MT_ActMotion2( MOTION_CONTROL *m_ctrl, DG_OBJS *objs );
extern void MT_FreeMotion( MOTION_CONTROL *m_ctrl );
extern void MT_SetMotionData( MOTION_CONTROL *m_ctrl, int layer, int motion, int time, u_long64 mask );
extern void MT_ResetMotionData( MOTION_CONTROL *m_ctrl, int layer );
extern void MT_SetMotionInterp( MOTION_CONTROL *m_ctrl, int time, u_long64 flag );
extern void MT_SetMotionSeTable( MOTION_CONTROL *m_ctrl, int map_name, int table_id, int hazard_type, int segment_type );
extern void MT_SetMotionSpeed( MOTION_CONTROL *m_ctrl, float time );
extern void MT_EvmActMotion( MOTION_CONTROL *m_ctrl, DG_EVMOBJ *evmobj );
extern float MT_GetMotionStartHeight( MOTION_CONTROL *m_ctrl, int motion );
extern void MT_StartMotionDecodeSupport( void );
extern void MT_WaitMotionDecodeSupport( void );

/* actseq.c */
extern void* MT_InitSequence( int n_layer, int id, int flag );
extern void MT_FreeSequence( SAR_CONTROL *sar_ctrl );
extern void MT_ActSequence( SAR_CONTROL *sar_ctrl );
extern void MT_SetSequenceSpeed( SAR_CONTROL *sar_ctrl, float time );
extern void MT_PlaySequence( SAR_CONTROL *sar_ctrl, int layer, int num, int flag, int loop_time );
extern void MT_StopSequence( SAR_CONTROL *sar_ctrl, int layer );
//extern void MT_ActSequenceSEV( SAR_CONTROL *sar_ctrl, MOTION_CONTROL *m_ctrl, DG_OBJS *objs );
extern void MT_ActSequenceSEV( SAR_CONTROL *sar_ctrl, MOTION_CONTROL *m_ctrl, DG_OBJS *objs, DG_EVMOBJ *evmobj );

/* se_exchg.c */
extern void MT_InitExchangeSeList( void );
extern void MT_DeleteExchangeSeList( int table_id );
extern MT_EXCHG_SE_LIST* MT_GetExchangeSeList( void );
extern int MT_IsSeListRegist( void ) ;
extern void MT_GetExchangeSe( MT_EXCHG_SE *se_data, int se_code, int table_id, int hazard_type, int segment_type );

/* hermite.c */
extern float MT_HermiteLerp( float q0, float q1, float d0, float d1, float t );
extern void MT_HermiteLerpVec( FVECTOR *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1, float t );
extern void MT_HermiteLerpVecScale( FVECTOR *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1, float t, float scale );
extern void MT_DHermiteLerpVec( FVECTOR *res, FVECTOR *q0, FVECTOR *q1, FVECTOR *d0, FVECTOR *d1, float t );

/* mar_load.c */
extern MAR_HEADER	*MT_GetMotionArchives( int motion_id );
extern int MT_LoadInitMar( void *buf, int id );
extern int MT_LoadInitSar( void *buf, int id );
extern void MT_Initialize( void );

/* mt_effct.c */
extern void MT_ReversalMotion( FVECTOR *rots, int flag );

/* faceanim.c */
extern void MT_InitFaceControl( MT_FACE_CONTROL *f_ctrl, int name_id, int flag );
extern void MT_SetFaceMotion( MT_FACE_CONTROL *f_ctrl, int motion_num );
extern void MT_ActFaceMotion( MT_FACE_CONTROL *f_ctrl );
extern void MT_PutFaceMotion( DG_EVMOBJ *evmobj, MT_FACE_CONTROL *f_ctrl );

/* facepack.c */
extern void MT_DecodeFpkMotion( FVECTOR *qrots, FVECTOR *trans, void *pack_data, int id );

/* ik_sys.c */
extern MT_IK_CONTROL* MT_MakeIKControl( int flag, int n_joints, DG_OBJS *objs );
extern void MT_FreeIKControl( MT_IK_CONTROL *ik_ctrl );
extern void MT_ConfigIKJointObj( MT_IK_CONTROL *ik_ctrl, int n_joint, int obj_num );
extern void MT_ConfigIKJointParam( MT_IK_CONTROL *ik_ctrl, int n_joint, FVECTOR *max, FVECTOR *min );
extern void MT_ConfigIKJointParamX( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
extern void MT_ConfigIKJointParamY( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
extern void MT_ConfigIKJointParamZ( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
extern void MT_CalcInversKinematic( MT_IK_CONTROL *ik_ctrl );
