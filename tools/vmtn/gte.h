/*
	ジオメトリ関連ヘッダファイル
*/


#ifndef __GTE__
#define __GTE__

#ifdef __cplusplus
extern "C" {
#endif

/* ベクトル型定義 */
typedef struct _fvector {
	float		vx, vy, vz,vw ;
} FVECTOR ;
/* マトリクス型定義 */
typedef struct _fmatrix {
	float		m[4][4] ;
} FMATRIX ;
/* その他の定義 */
typedef struct {
	char r, g, b, cd ;
} CVECTOR ;
typedef struct {
	short vx, vy, vz, pad ;
} SVECTOR ;
typedef struct {
	int vx, vy, vz, vw ;
} IVECTOR ;
typedef struct _fvector2d {
	float	vx, vy ;
} FVECTOR2D ;



/* マトリクス回転部掛け算 (res)=(mat0)*(mat1) */
void GTE_MulMatrix3D( FMATRIX *res, FMATRIX *mat0, FMATRIX *mat1 );
/* マトリクスの合成 (res)=(mat0)*(mat1) */
void GTE_MulMatrix( FMATRIX *res, FMATRIX *mat0, FMATRIX *mat1 );
/* マトリクスによる回転を行う (res)=(mat)*(vec1) */
void GTE_ApplyMatrix3D( FVECTOR *res, FMATRIX *mat, FVECTOR *vec1 );
/* マトリクスによる回転・移動を行う (res)=(mat)*(vec1) */
void GTE_ApplyMatrix( FVECTOR *res, FMATRIX *mat, FVECTOR *vec1 );
/* マトリクスにＸ軸回転をかける */
void GTE_RotMatrixX( FMATRIX *mat, float rot_x );
/* マトリクスにＹ軸回転をかける */
void GTE_RotMatrixY( FMATRIX *mat, float rot_y );
/* マトリクスにＺ軸回転をかける */
void GTE_RotMatrixZ( FMATRIX *mat, float rot_z );
/* 回転ベクトルに応じたマトリクスを生成（Ｍ）=（Ｘ）*（Ｙ）*（Ｚ） */
void GTE_RotMatrixXYZ( FMATRIX *mat, FVECTOR *rot );
/* 回転ベクトルに応じたマトリクスを生成（Ｍ）=（Ｚ）*（Ｙ）*（Ｘ） */
void GTE_RotMatrixZYX( FMATRIX *mat, FVECTOR *rot );
/* ベクトルの外積を求める（ res = v1 * v2 ） */
void GTE_OuterProduct( FVECTOR *res, FVECTOR *v1, FVECTOR *v2 );
/* ベクトルの正規化 */
void GTE_VectorNormal( FVECTOR *res, FVECTOR *v1 );
/* マトリクスの回転成分に対する逆マトリクスを求める */
void GTE_TransposeMatrix( FMATRIX *mat2, FMATRIX *mat1 );
/* マトリクスを単位マトリクスで初期化 */
void GTE_UnitMatrix( FMATRIX *mat );
/* 逆変換マトリクスの作成 */
void GTE_InversMatrix( FMATRIX *inv, FMATRIX *mat );
/* ２点の中間ベクトルを生成（ v1(t=0) ~ v2(t=1) ） */
int GTE_InterpolateVector( FVECTOR *res, FVECTOR *v1, FVECTOR *v2, float t );

/* ポリゴンと線分の当たり判定チェック */
int GTE_RayCheck( FVECTOR *res, FVECTOR *from, FVECTOR *to, FVECTOR *poly, int n_point );



#ifdef __cplusplus
};
#endif

#endif

