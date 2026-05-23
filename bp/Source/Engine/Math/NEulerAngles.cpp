//----------------------------------------------------------------------------
// NEulerAngles.cpp
// Bluepoint
// Copyright 2006
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "NEulerAngles.h"

//----------------------------------------------------------------------------

#include "BPEMath.h"

//----------------------------------------------------------------------------
// Euler Angle Conversion by Ken Shoemake, Gramphics Gems IV
// Source: http://vered.rose.utoronto.ca/people/david_dir/GEMS/GEMS.html
//----------------------------------------------------------------------------

#pragma warning ( disable : 4244 )
#pragma warning ( disable : 4552 )

typedef struct {float x, y, z, w;} Quat; /* Quaternion */
typedef float HMatrix[4][4]; /* Right-handed, for column vectors */
typedef Quat EulerAngles;    /* (x,y,z)=ang 1,2,3, w=order code  */

/*** Order type constants, constructors, extractors ***/

    /* There are 24 possible conventions, designated by:    */
    /*	  o EulAxI = axis used initially		    */
    /*	  o EulPar = parity of axis permutation		    */
    /*	  o EulRep = repetition of initial axis as last	    */
    /*	  o EulFrm = frame from which axes are taken	    */
    /* Axes I,J,K will be a permutation of X,Y,Z.	    */
    /* Axis H will be either I or K, depending on EulRep.   */
    /* Frame S takes axes from initial static frame.	    */
    /* If ord = (AxI=X, Par=Even, Rep=No, Frm=S), then	    */
    /* {a,b,c,ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v	    */
    /* rotates v around Z by c radians.			    */

#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
    /* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>1;s=o&1;o>>1;\
    n=o&1;o>>1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
    /* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
    /* Static axes */
#define EulOrdXYZs    EulOrd(kAX,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(kAX,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(kAX,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(kAX,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(kAY,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(kAY,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(kAY,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(kAY,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(kAZ,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(kAZ,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(kAZ,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(kAZ,EulParOdd,EulRepYes,EulFrmS)
    /* Rotating axes */
#define EulOrdZYXr    EulOrd(kAX,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(kAX,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(kAX,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(kAX,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(kAY,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(kAY,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(kAY,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(kAY,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(kAZ,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(kAZ,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(kAZ,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(kAZ,EulParOdd,EulRepYes,EulFrmR)

/* Construct matrix from Euler angles (in radians). */
void Eul_ToHMatrix(EulerAngles & ea, HMatrix & M)
{
    double ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i,j,k,h,n,s,f;
    EulGetOrd(ea.w,i,j,k,h,n,s,f);
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = -ea.y; ea.z = -ea.z;}
    ti = ea.x;	  tj = ea.y;	th = ea.z;
    ci = cos(ti); cj = cos(tj); ch = cos(th);
    si = sin(ti); sj = sin(tj); sh = sin(th);
    cc = ci*ch; cs = ci*sh; sc = si*ch; ss = si*sh;
    if (s==EulRepYes) {
	M[i][i] = cj;	  M[i][j] =  sj*si;    M[i][k] =  sj*ci;
	M[j][i] = sj*sh;  M[j][j] = -cj*ss+cc; M[j][k] = -cj*cs-sc;
	M[k][i] = -sj*ch; M[k][j] =  cj*sc+cs; M[k][k] =  cj*cc-ss;
    } else {
	M[i][i] = cj*ch; M[i][j] = sj*sc-cs; M[i][k] = sj*cc+ss;
	M[j][i] = cj*sh; M[j][j] = sj*ss+cc; M[j][k] = sj*cs-sc;
	M[k][i] = -sj;	 M[k][j] = cj*si;    M[k][k] = cj*ci;
    }
    M[kAW][kAX]=M[kAW][kAY]=M[kAW][kAZ]=M[kAX][kAW]=M[kAY][kAW]=M[kAZ][kAW]=0.0; M[kAW][kAW]=1.0;
}

/* Convert matrix to Euler angles (in radians). */
EulerAngles Eul_FromHMatrix(HMatrix & M, int order)
{
    EulerAngles ea;
    int i,j,k,h,n,s,f;
    EulGetOrd(order,i,j,k,h,n,s,f);
    if (s==EulRepYes) {
	double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);
   if (sy > 16*gkEpsilon32) {
	    ea.x = atan2(M[i][j], M[i][k]);
	    ea.y = atan2(sy, double(M[i][i]));
	    ea.z = atan2(M[j][i], -M[k][i]);
	} else {
	    ea.x = atan2(-M[j][k], M[j][j]);
	    ea.y = atan2(sy, double(M[i][i]));
	    ea.z = 0;
	}
    } else {
	double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);
   if (cy > 16*gkEpsilon32) {
	    ea.x = atan2(M[k][j], M[k][k]);
	    ea.y = atan2(double(-M[k][i]), cy);
	    ea.z = atan2(M[j][i], M[i][i]);
	} else {
	    ea.x = atan2(-M[j][k], M[j][j]);
	    ea.y = atan2(double(-M[k][i]), cy);
	    ea.z = 0;
	}
    }
    if (n==EulParOdd) {ea.x = -ea.x; ea.y = - ea.y; ea.z = -ea.z;}
    if (f==EulFrmR) {float t = ea.x; ea.x = ea.z; ea.z = t;}
    ea.w = order;
    return (ea);
}
                                                
//----------------------------------------------------------------------------

CVector3 NEulerAngles::FromMatrix3(CMatrix3 const & matrix)
{
   CMatrix4 matrix44 = CMatrix4::FromMatrix3(matrix);
   CMatrix4 transposed44 = matrix44.Transpose();

   EulerAngles angles = Eul_FromHMatrix(reinterpret_cast<HMatrix&>(transposed44), EulOrdXYZs);

   angles.x = CAngle::FromRadians(angles.x).AsDegrees();
   angles.y = CAngle::FromRadians(angles.y).AsDegrees();
   angles.z = CAngle::FromRadians(angles.z).AsDegrees();

   return CVector3(angles.x, angles.y, angles.z);
}

//----------------------------------------------------------------------------

CVector3 NEulerAngles::FromMatrix34(CMatrix34 const & matrix)
{
   CMatrix4 matrix44 = CMatrix4::FromMatrix34(matrix);
   CMatrix4 transposed44 = matrix44.Transpose();

   EulerAngles angles = Eul_FromHMatrix(reinterpret_cast<HMatrix&>(transposed44), EulOrdXYZs);

   angles.x = CAngle::FromRadians(angles.x).AsDegrees();
   angles.y = CAngle::FromRadians(angles.y).AsDegrees();
   angles.z = CAngle::FromRadians(angles.z).AsDegrees();

   return CVector3(angles.x, angles.y, angles.z);
}

//----------------------------------------------------------------------------

CVector3 NEulerAngles::FromMatrix4(CMatrix4 const & matrix)
{
   CMatrix4 transposed44 = matrix.Transpose();

   EulerAngles angles = Eul_FromHMatrix(reinterpret_cast<HMatrix&>(transposed44), EulOrdXYZs);

   angles.x = CAngle::FromRadians(angles.x).AsDegrees();
   angles.y = CAngle::FromRadians(angles.y).AsDegrees();
   angles.z = CAngle::FromRadians(angles.z).AsDegrees();

   return CVector3(angles.x, angles.y, angles.z);
}

//----------------------------------------------------------------------------

CMatrix3 NEulerAngles::ToMatrix3(CVector3 const & eulerAngles)
{
   CMatrix3 const rotationMat = CMatrix3::RotateZ(CAngle::FromDegrees(eulerAngles.GetZ())) *
                                CMatrix3::RotateY(CAngle::FromDegrees(eulerAngles.GetY())) *
                                CMatrix3::RotateX(CAngle::FromDegrees(eulerAngles.GetX()));
   return rotationMat;
}

//----------------------------------------------------------------------------

CMatrix34 NEulerAngles::ToMatrix34(CVector3 const & eulerAngles)
{
   CMatrix34 const rotationMat = CMatrix34::RotateZ(CAngle::FromDegrees(eulerAngles.GetZ())) *
                                 CMatrix34::RotateY(CAngle::FromDegrees(eulerAngles.GetY())) *
                                 CMatrix34::RotateX(CAngle::FromDegrees(eulerAngles.GetX()));
   return rotationMat;
}

//----------------------------------------------------------------------------

CMatrix4 NEulerAngles::ToMatrix4(CVector3 const & eulerAngles)
{
   CMatrix4 const rotationMat = CMatrix4::RotateZ(CAngle::FromDegrees(eulerAngles.GetZ())) *
                                CMatrix4::RotateY(CAngle::FromDegrees(eulerAngles.GetY())) *
                                CMatrix4::RotateX(CAngle::FromDegrees(eulerAngles.GetX()));
   return rotationMat;
}

