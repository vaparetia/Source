#ifndef _3d_util_h_
#define _3d_util_h_

FVECTOR * get_joints_vector(FVECTOR * vec, DG_EVMOBJ * evmobj, int num);
int       get_render_point(float *x, float *y, FVECTOR *vec, int chanl);

FMATRIX * create_rotate_matrix_YXZ(FMATRIX * mat,
				   float heading, float pitch, float roll);
FMATRIX * create_trans_matrix(FMATRIX * mat, float x, float y, float z);

#endif /* _3d_util_h_ */
