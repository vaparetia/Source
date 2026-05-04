/*

  使ってるvu0_関数をエミュレートする
  足りなくなったらその都度追加しよう

  2001/04/26 M.Kobayashi
  $Id: utlx.cpp,v 1.1.1.3 2002/11/19 11:43:00 Yoshizawa1 Exp $

 */
#include "stdio.h"
#include	"mgs_type.h"
#include 	"libdg.h"
#include	"libutl.h"

#if 0 //BP_TODO

#if 0

/* バウンディングボックス内に、ある点が存在するかどうかのチェック
   バウンディングボックス内に存在すれば１、しなければ０を返す。*/
int	vu0_CheckBoundingBox(const VECTOR * const vertex,
						 const VECTOR * const v0,const VECTOR * const v1)
{
	VECTOR	vecDiff0;
	VECTOR	vecDiff1;

	D3DXVec3Subtract(&vecDiff0, v0, vertex);
	D3DXVec3Subtract(&vecDiff1, v0, vertex);

	if( vecDiff0.x * vecDiff1.x < 0.f
		&& vecDiff0.y * vecDiff1.y < 0.f
		&& vecDiff0.z * vecDiff1.z < 0.f ) return 1;
	else return 0;
}

/* PS系の角度を、ラジアンに変換 */
void vu0_PSDegV2RadV(const SVECTOR * const s,FVECTOR *f)
{
	f->x = (float)s->vx * ( 2.f * M_PI / 4096.f);
	f->y = (float)s->vy * ( 2.f * M_PI / 4096.f);
	f->z = (float)s->vz * ( 2.f * M_PI / 4096.f);
}

#endif

#endif

void sceVu0DropShadowMatrix(sceVu0FMATRIX m, sceVu0FVECTOR lp, float a, float b, float c, int mode)
{	//libvu0.c そのまま
    if (mode) {	// spot light 
		float x = lp[0], y = lp[1], z = lp[2];
		float d = (float)1-(a*x+b*y+c*z);
		
		m[0][0] = a*x+d, m[1][0] = b*x,   m[2][0] = c*x,   m[3][0] = -x;
		m[0][1] = a*y,   m[1][1] = b*y+d, m[2][1] = c*y,   m[3][1] = -y;
		m[0][2] = a*z,   m[1][2] = b*z,   m[2][2] = c*z+d, m[3][2] = -z;
		m[0][3] = a,     m[1][3] = b,     m[2][3] = c,     m[3][3] = d-(float)1;
    }
    else {		// parallel light
		float p  = lp[0], q = lp[1], r = lp[2];
		float n  = a*p+b*q+c*r;
		/*0DIV yano 2002.03.14*/
		if( n == 0.0f ) n = 0.0000001f;
		float nr = -(float)1.0/n;

		m[0][0] = nr*(a*p-n), m[1][0] = nr*(b*p),   m[2][0] = nr*(c*p),   m[3][0] = nr*(-p);
		m[0][1] = nr*(a*q),   m[1][1] = nr*(b*q-n), m[2][1] = nr*(c*q),   m[3][1] = nr*(-q);
		m[0][2] = nr*(a*r),   m[1][2] = nr*(b*r),   m[2][2] = nr*(c*r-n), m[3][2] = nr*(-r);
		m[0][3] = (float)0,          m[1][3] = (float)0,          m[2][3] =(float) 0,          m[3][3] = nr*(-n);
	}
}

#if 0 //BP_TODO

static void	XSystimeToSceClock( sceCdCLOCK* pRtc, SYSTEMTIME* pSystime )
{	// Ｘ時刻をソニー時刻表現に変換
	pRtc->stat = 0;
	pRtc->second = (pSystime->wSecond % 10) | (( pSystime->wSecond / 10 % 10 ) << 4);
	pRtc->minute = (pSystime->wMinute % 10) | (( pSystime->wMinute / 10 % 10 ) << 4);
	pRtc->hour   = (pSystime->wHour   % 10) | (( pSystime->wHour   / 10 % 10 ) << 4);
	pRtc->day    = (pSystime->wDay    % 10) | (( pSystime->wDay    / 10 % 10 ) << 4);
	pRtc->month  = (pSystime->wMonth  % 10) | (( pSystime->wMonth  / 10 % 10 ) << 4);
	pRtc->year   = (pSystime->wYear   % 10) | (( pSystime->wYear   / 10 % 10 ) << 4);

	pRtc->pad = 0;
}  

static void	SceClockToXSystime( SYSTEMTIME* pSystime, sceCdCLOCK* pRtc )
{	// ソニー時刻表現をＸ時刻に変換
	pSystime->wSecond = (pRtc->second & 0xf) + (pRtc->second >> 4) * 10;
	pSystime->wMinute = (pRtc->minute & 0xf) + (pRtc->minute >> 4) * 10;
	pSystime->wHour   = (pRtc->hour   & 0xf) + (pRtc->hour   >> 4) * 10;
	pSystime->wDay    = (pRtc->day    & 0xf) + (pRtc->day    >> 4) * 10;
	pSystime->wMonth  = (pRtc->month  & 0xf) + (pRtc->month  >> 4) * 10;
	pSystime->wYear   = (pRtc->year   & 0xf) + (pRtc->year   >> 4) * 10 + 2000;	// 2000年代固定

	pSystime->wDayOfWeek = 0;
	pSystime->wMilliseconds = 0;

	// 2000年代固定だがsceCdCLOCK にメンバを増やして情報を書き込んだほうがいいかもしれない
	// TRC はどうか？
}  


void sceScfGetLocalTimefromRTC( sceCdCLOCK* pRtc )
{	// JST->LocalTime の変換
	SYSTEMTIME	jst, localsys;
	ULARGE_INTEGER	tmp;
	FILETIME		local;

	// sce->X
	SceClockToXSystime( &jst, pRtc );
	// SYSTIME->FILETIME
	SystemTimeToFileTime( &jst, (FILETIME*)&tmp );
	// JST->UTC
	tmp.QuadPart -= 9 * 60 * 60 * 10000000i64;	// -9 hour
	// UTC->Local
	FileTimeToLocalFileTime( (FILETIME*)&tmp, &local);
	// FILETIME->SYSTIME
	FileTimeToSystemTime( &local, &localsys );
	// X->sce
	XSystimeToSceClock( pRtc, &localsys );
}

int sceCdReadClock( sceCdCLOCK* pRtc )
{	// JST を返す。UTC + 9 hour
	FILETIME utc;
	ULARGE_INTEGER	tmp;
	SYSTEMTIME	jst;

	GetSystemTimeAsFileTime( &utc );
	memcpy( &tmp, &utc, sizeof( utc ) );
	tmp.QuadPart += 9 * 60 * 60 * 10000000i64;	// +9 hour

	FileTimeToSystemTime( (FILETIME*)&tmp, &jst );

	XSystimeToSceClock( pRtc, &jst );

	return 1;
}

///////////////
// VECTOR ユーティリティ

float	FVECTOR::Dist2(FVECTOR* pvecTo)
{
	float dx,dy,dz;

	dx = pvecTo->x - x;
	dy = pvecTo->y - y;
	dz = pvecTo->z - z;

	return dx * dx + dy * dy + dz * dz;
}
FVECTOR operator*(FMATRIX& mat, FVECTOR& vec)
{
	FVECTOR vecAns;
	D3DXVec4Transform(&vecAns, &vec, &mat);	// たいていの場合 w = 1 にしなくてはならない
	return vecAns;
}

#endif
