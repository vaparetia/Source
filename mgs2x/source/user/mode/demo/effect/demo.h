/*

  デモ ライフラリ 関数

  2000/08/23 T.Morita
  $Id: demo.h,v 1.1.1.3 2002/11/19 11:45:10 Yoshizawa1 Exp $

 */
void*    DM_GetObjectHandle( int id );
void*    DM_GetObjectControl( void *handle );/* CONTROL*を返り値としている */
void*    DM_GetObjectObject( void *handle ); /* OBJECT*を返り値としている */
void*    DM_GetObjectDgObjs( void *handle ); /* DG_OBJS*を返り値としている */
void*    DM_GetObjectDgObj( void *handle, int joint ); /* DG_OBJ*を返り値としている */
FMATRIX* DM_GetObjectMatrix( void *handle, int joint ) ;
FMATRIX* DM_GetObjectLightMtx( void *handle ) ;
void*    DM_NULL() ; /* 必ずNULLを返す関数 */

void* DM_GetEvmObjObject( void *handle ); /* DF_EVMOBJ* */
FVECTOR* DM_GetObjectPos( void *handle, int joint ); /* DG_OBJからポジションゲット*/
int      DM_GetCurrentMap(); /*カレントマップを取得する*/
void 	DM_IVecToSVector( IVECTOR *i_vec, SVECTOR *s_vec );
void 	DM_GetRotToRadVec(FVECTOR *r,float x,float y,float z,float w);
void DM_InFVector(FVECTOR *r,float x,float y,float z,float w);
void DM_GetFMatrixM( FVECTOR *out, FVECTOR *rot, int num );
void DM_GetFMatrixM_ZXY( FVECTOR *out, FVECTOR *rot, int num );
void DM_GetFMatrixM_XYZ( FVECTOR *out, FVECTOR *rot, int num );
int     DM_GetDemoActorMap() ; /* デモデーモンのマップを拾う */
