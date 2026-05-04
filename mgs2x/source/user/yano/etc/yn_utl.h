/*
        yn_utl.h

        いろいろひとまとめヘッダ
        
        2001/07/03 Y.YANO
        $Id: yn_utl.h,v 1.1.1.3 2002/11/19 11:52:00 Yoshizawa1 Exp $

*/

extern void YN_MatrixDump( FMATRIX * matrix );
extern void YN_VectorDump( FVECTOR * vector );

extern float YN_VectorLength( FVECTOR *in );
extern void YN_RotVector( FVECTOR *out , FVECTOR *in  , float angle );
extern void YN_GetHousen( FVECTOR *housen , HZX_VEC p1 , HZX_VEC p2 , HZX_VEC p3 );
extern void YN_OuterOuter( FVECTOR *out , FVECTOR *in , FVECTOR *in2 );

/*  ～ひとさまのプログラムから～  */
/*任意の軸で右回りに回転させるマトリクス取得   from Takabe */
extern void GTE_MakeRotateAxis( FMATRIX *res, FVECTOR *axis, float angle );
extern void GTE_MakeRotate( FMATRIX *res, float x, float y, float z, float angle );

/*nまでの乱数を出す*/
#define RND(_n) ( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/*-2047～2048の値に直す*/
#define REGULAR_ANGLE( _a )     ( (signed short)( ((unsigned short)(_a)) << 4 ) >> 4 )
/*FVECTOR をダンプ*/
#define PRINT_PFVEC(_i,_fv) printf("[%d] vx %8.8f:vy %8.8f:vz %8.8f:vw %8.8f:\n",_i,(_fv)->vx,(_fv)->vy,(_fv)->vz,(_fv)->vw)
/* float を 1.3.12ビット int に直す */
#define FTOI12(_f)      ( DG_FTOI( ( (float)(_f)*4096.0f) ) )
