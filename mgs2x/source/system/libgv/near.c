//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   near.c
   漸近計算ルーチン

   1999/07/07 M.Sonoyama
   $Id: near.c,v 1.1.1.3 2002/11/19 11:42:44 Yoshizawa1 Exp $			

	int		GV_NearExp2( from, to )
	int		from, to ;

		ギュンと近づく

	int		GV_NearExp4( from, to )
	int		from, to ;

		シューンと近づく

	int		GV_NearExp8( from, to )
	int		from, to ;

		ウニョ～ンと近づく

	int		GV_NearPhase( from, to )
	int		from, to ;

		角位相を目標に近い方にあわせる

	int		GV_NearRange( from, to, range )
	int		from, to, range ;

		目標から range 以内に近づく

	int		GV_NearSpeed( from, to, speed )
	int		from, to, speed ;

		指定されたスピードで近づく

	int		GV_NearTime( from, to, interp )
	int		from, to, interp ;

		指定された時間で近づく

	------------------------------------------------

	int		GV_NearExp2P( from, to )
	int		from, to ;
	int		GV_NearExp4P( from, to )
	int		from, to ;
	int		GV_NearExp8P( from, to )
	int		from, to ;
	int		GV_NearTimeP( from, to, interp )
	int		from, to, interp ;

		「GV_NearPhase()＋各種関数」の複合ルーチン

	------------------------------------------------

	void		GV_NearExp2V( vfrom, vto, n )
	void		*vfrom, *vto ;
	int		n ;
	void		GV_NearExp4V( vfrom, vto, n )
	void		*vfrom, *vto ;
	int		n ;
	void		GV_NearExp8V( vfrom, vto, n )
	void		*vfrom, *vto ;
	int		n ;
	void		GV_NearPhaseV( vfrom, vto, n )
	void		*vfrom, *vto ;
	int		n ;
	void		GV_NearRangeV( vfrom, vto, range, n )
	void		*vfrom, *vto ;
	int		range, n ;
	void		GV_NearSpeedV( vfrom, vto, speed, n )
	void		*vfrom, *vto ;
	int		speed, n ;
	void		GV_NearTimeV( vfrom, vto, interp, n )
	void		*vfrom, *vto ;
	int		interp, n ;
	void		GV_NearExp2PV( vfrom, vto, n )
	void		*vfrom, *vto ;
	int		n ;
	void		GV_NearExp4PV( vfrom, vto, n )
	void		*vfrom, *vto ;
	int		n ;
	void		GV_NearExp8PV( vfrom, vto, n )
	void		*vfrom, *vto ;
	int		n ;
	void		GV_NearExp2SV( vfrom, vto, speed, n )
	void		*vfrom, *vto ;
	int		speed, n ;
	void		GV_NearExp4SV( vfrom, vto, speed, n )
	void		*vfrom, *vto ;
	int		speed, n ;
	void		GV_NearExp8SV( vfrom, vto, speed, n )
	void		*vfrom, *vto ;
	int		speed, n ;
	void		GV_NearTimePV( vfrom, vto, interp, n )
	void		*vfrom, *vto ;
	int		interp, n ;

		前述の関数のベクトル版
		vfrom / vto は、short または SVECTOR のポインタ
		n は short の要素数
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif	// PSX2

#ifdef KP_XBOX
#include <xtl.h>
#include "mgsx_type.h"
#endif // KP_XBOX

#include	"gameheader.h"

/*----------------------------------------------------------------*/

#ifndef	NO_PROTOTYPE
static	inline	int	NearExp2( int, int ) ;
static	inline	int	NearExp4( int, int ) ;
static	inline	int	NearPhase( int, int ) ;
static	inline	int	NearRange( int, int, int ) ;
static	inline	int	NearSpeed( int, int, int ) ;
static	inline	int	NearExp2P( int, int ) ;
static	inline	int	NearExp4P( int, int ) ;
static	inline	int	NearExp2S( int, int, int ) ;
static	inline	int	NearExp4S( int, int, int ) ;
static	inline	int	NearTime( int, int, int ) ;
#endif

/*----------------------------------------------------------------*/

	/*
		「4096 / t」のテーブル
	*/
static	short	TimeInv[] = { 
	0, 2048, 1365, 1024, 819, 682, 585, 512,
	455, 409, 372, 341, 315, 292, 273, 256
} ;

/*----------------------------------------------------------------*/

static	inline	int	NearExp2( from, to )
int			from, to ;
{
	int		diff ;

	diff = to - from ;
	if ( diff > -2 && diff < 2 ) return to ;
	return from + diff / 2 ;
}

static	inline	int	NearExp4( from, to )
int			from, to ;
{
	int		diff ;

	diff = to - from ;
	if ( diff == 0 ) return to ;
	if ( diff > -4 && diff < 4 ) diff = ( diff > 0 ) ? 4 : -4 ;
	return from + diff / 4 ;
}

static	inline	int	NearExp8( from, to )
int			from, to ;
{
	int		diff ;

	diff = to - from ;
	if ( diff == 0 ) return to ;
	if ( diff > -8 && diff < 8 ) diff = ( diff > 0 ) ? 8 : -8 ;
	return from + diff / 8 ;
}

//#ifdef PAL
static	inline	int	NearExp8PAL( from, to )
int			from, to ;
{
	float	diff ;

	diff = ( float )( to - from ) ;
	if ( DG_FABS( diff ) < 0.10F ) return to ;
	if ( diff > -8.0F / 1.20F && diff < 8.0F / 1.20F ) {
		diff = ( diff > 0.0F ) ? 8.0F / 1.20F : -8.0F / 1.20F ;
	}
	return from + ( int )( diff * 1.20F / 8.0F ) ;
}
//#endif

static	inline	int	NearExp16( from, to )
int			from, to ;
{
	int		diff ;

	diff = to - from ;
	if ( diff == 0 ) return to ;
	if ( diff > -16 && diff < 16 ) diff = ( diff > 0 ) ? 16 : -16 ;
	return from + diff / 16 ;
}

static	inline	int		NearExpN( int from, int to, int n )
{
	int			diff ;
	float		v ;

	diff = to - from ;
	v = ( float )diff / ( float )n ;
	if ( DG_FABS( v ) < 0.10F ) return to ;
	if ( v > 0.0F && v < 1.0F ) v = 1.0F ;	
	else if ( v < 0.0F && v > -1.0F ) v = -1.0F ;	
	return from + ( int )v ;
}

static	inline	int	NearPhase( from, to )
int			from, to ;
{
	short		diff ;

	diff = 4095 & ( from - to ) ;
	if ( diff > 2048 ) diff -= 4096 ;
	return to + diff ;
}

static	inline	int	NearRange( from, to, range )
int			from, to ;
int			range ;
{
	int		border ;

	border = to - range ;
	if ( from < border ) return border ;
	border = to + range ;
	if ( from > border ) return border ;
	return from ;
}

static	inline	int	NearSpeed( from, to, speed )
int			from, to ;
int			speed ;
{
	int		next ;

	if ( from < to ) {
		next = from + speed ;
		if ( next >= to ) return to ;
	} else {
		next = from - speed ;
		if ( next <= to ) return to ;
	}
	return next ;
}

static	inline	int	NearTime( from, to, interp )
int			from, to, interp ;
{
	if ( interp == 0 ) return to ;
	return ( to - from ) * interp / 4096 + from ;
}

/*----------------------------------------------------------------*/

static	inline	float	NearExp2F(
		float			from, float to )
{
	float		diff ;

	diff = to - from ;
	return from + diff / 2.0F ;
}

static	inline	float	NearExp4F(
		float			from, float to )
{
	float		diff ;

	diff = to - from  ;
	return from + diff / 4.0F ;
}

static	inline	float	NearExp8F(
		float			from, float to )
{
	float		diff ;

	diff = to - from ;
	return from + diff / 8.0F ;
}

static	inline	float	NearExp16F(
		float			from, float to )
{
	float		diff ;

	diff = to  - from ;
	return from + diff / 16.0F ;
}

static	inline	float	NearExpNF( float from, float to, float n )
{
	float		diff ;

	diff = to - from ;
	return from + diff / n ;
}

static	inline	float	NearRangeF(
		float			from, float to ,
		float			range )
{
	float		border ;

	border = to - range;
	if ( from < border ) return border ;
	border = to + range;
	if ( from > border ) return border ;
	return from ;
}

static	inline	float	NearSpeedF(
		float		from, float to,
		float		speed )
{
	float	next ;

	if ( from < to ) {
		next = from + speed ;
		if ( next >= to ) return to;
	} else {
		next = from - speed ;
		if ( next <= to ) return to ;
	}
	return next ;
}

static	inline	float	NearTimeF(
		float			from, float to ,
		int			interp )
{
	if ( interp == 0 ) return to ;
	return ( to - from ) / ( float )interp + from ;
}

/*----------------------------------------------------------------*/

static	inline	int	NearExp2P( from, to )
int			from, to ;
{
	return NearExp2( NearPhase( from, to ), to ) ;
}

static	inline	int	NearExp4P( from, to )
int			from, to ;
{
	return NearExp4( NearPhase( from, to ), to ) ;
}

static	inline	int	NearExp8P( from, to )
int			from, to ;
{
	return NearExp8( NearPhase( from, to ), to ) ;
}

//#ifdef PAL
static	inline	int	NearExp8PPAL( from, to )
int			from, to ;
{
	return NearExp8PAL( NearPhase( from, to ), to ) ;
}
//#endif

static	inline	int	NearExp16P( from, to )
int			from, to ;
{
	return NearExp16( NearPhase( from, to ), to ) ;
}

static	inline	int	NearExpNP( from, to, n )
int			from, to, n ;
{
	return NearExpN( NearPhase( from, to ), to, n ) ;
}

static	inline	int	NearSpeedP( from, to, speed )
int		from, to, speed ;
{
	return NearSpeed( NearPhase( from, to ), to, speed ) ;
}

static	inline	int	NearExp2S( from, to, speed )
int			from, to ;
int			speed ;
{
	int		diff ;

	diff = ( to - from ) / 2 ;
	if ( diff > -speed && diff < speed ) return to ;
	return from + diff ;
}

static	inline	int	NearExp4S( from, to, speed )
int			from, to ;
int			speed ;
{
	int		diff ;

	diff = ( to - from ) / 4 ;
	if ( diff > -speed && diff < speed ) return to ;
	return from + diff ;
}

static	inline	int	NearExp8S( from, to, speed )
int			from, to ;
int			speed ;
{
	int		diff ;

	diff = ( to - from ) / 8 ;
	if ( diff > -speed && diff < speed ) return to ;
	return from + diff ;
}

static	inline	int	NearExp16S( from, to, speed )
int			from, to ;
int			speed ;
{
	int		diff ;

	diff = ( to - from ) / 16 ;
	if ( diff > -speed && diff < speed ) return to ;
	return from + diff ;
}

/*----------------------------------------------------------------*/

int		GV_NearExp2( from, to )
int		from, to ;
{
	return NearExp2( from, to ) ;
}

int		GV_NearExp4( from, to )
int		from, to ;
{
	return NearExp4( from, to ) ;
}

int		GV_NearExp8( from, to )
int		from, to ;
{
	return NearExp8( from, to ) ;
}

int		GV_NearExp16( from, to )
int		from, to ;
{
	return NearExp16( from, to ) ;
}

int		GV_NearPhase( from, to )
int		from, to ;
{
	return NearPhase( from, to ) ;
}

int		GV_NearRange( from, to, range )
int		from, to, range ;
{
	return NearRange( from, to, range ) ;
}

int		GV_NearRangeP( from, to, range )
int		from, to, range ;
{
	return NearRange( NearPhase( from, to ), to, range ) ;
}

int		GV_NearSpeed( from, to, speed )
int		from, to, speed ;
{
	return NearSpeed( from, to, speed ) ;
}

int		GV_NearTime( from, to, interp )
int		from, to, interp ;
{
	if ( interp > 15 ) interp = 15 ;
	interp = TimeInv[ interp ] ;
	return NearTime( from, to, interp ) ;
}

/*----------------------------------------------------------------*/

float		GV_NearExp2F(
		float		from, float to )
{
	return NearExp2F( from, to ) ;
}

float		GV_NearExp4F(
		float		from, float to )
{
	return NearExp4F( from, to ) ;
}

float		GV_NearExp8F(
		float		from, float to )
{
	return NearExp8F( from, to ) ;
}

float		GV_NearExp16F(
		float		from, float to )
{
	return NearExp16F( from, to ) ;
}

float		GV_NearExpNF( float from, float to, float n )
{
	return NearExpNF( from, to, n ) ;
}

float		GV_NearRangeF(
		float		from, float to, float range )
{
	return NearRangeF( from, to, range ) ;
}

float	GV_NearSpeedF(
		float	from, float to, float speed )
{
	return NearSpeedF( from, to, speed ) ;
}

float		GV_NearTimeF(
		float		from, float to ,
		int		interp )
{
	return NearTimeF( from, to, interp ) ;
}

/*----------------------------------------------------------------*/

int		GV_NearExp2P( from, to )
int		from, to ;
{
	return NearExp2P( from, to ) ;
}

int		GV_NearExp4P( from, to )
int		from, to ;
{
	return NearExp4P( from, to ) ;
}

int		GV_NearExp8P( from, to )
int		from, to ;
{
	return NearExp8P( from, to ) ;
}

#ifdef PAL
int		GV_NearExp8PPAL( from, to )
int		from, to ;
{
	return NearExp8PAL( from, to ) ;
}
#endif

int		GV_NearExp16P( from, to )
int		from, to ;
{
	return NearExp16P( from, to ) ;
}

int		GV_NearExpNP( from, to, n )
int		from, to, n ;
{
	return NearExpNP( from, to, n ) ;
}

int		GV_NearSpeedP( from, to, speed )
int		from, to, speed ;
{
	return NearSpeedP( from, to, speed ) ;
}

int		GV_NearTimeP( from, to, interp )
int		from, to, interp ;
{
	if ( interp > 15 ) interp = 15 ;
	interp = TimeInv[ interp ] ;
	return NearTime( NearPhase( from, to ), to, interp ) ;
}

/*----------------------------------------------------------------*/

void		GV_NearExp2V( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp2( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp4V( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp4( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp8V( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp8( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp16V( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp16( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearPhaseV( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearPhase( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearRangeV( vfrom, vto, vrange, n )
void		*vfrom, *vto, *vrange ;
int			n ;
{
	short		*from, *to, *range ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	range = ( short * )vto ;
	while ( -- n >= 0 ) {
		*from = NearRange( *from, *to, *range ) ;
		from ++ ;
		to ++ ;
		range ++ ;
	}
}

void		GV_NearSpeedV( vfrom, vto, speed, n )
void		*vfrom, *vto ;
int		speed, n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearSpeed( *from, *to, speed ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearTimeV( vfrom, vto, interp, n )
void		*vfrom, *vto ;
int		interp, n ;
{
	short		*from, *to ;

	if ( interp > 15 ) interp = 15 ;
	interp = TimeInv[ interp ] ;
	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearTime( *from, *to, interp ) ;
		from ++ ;
		to ++ ;
	}
}

/*----------------------------------------------------------------*/

void		GV_NearExp2VF( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	float		*from, *to ;

	from = ( float * )vfrom ;
	to = ( float * )vto ;
	while ( -- n >= 0 ) {
		*from = NearExp2F( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp4VF( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	float		*from, *to ;

	from = ( float * )vfrom ;
	to = ( float * )vto ;
	while ( -- n >= 0 ) {
		*from = NearExp4F( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp8VF( vfrom, vto, n )
void		*vfrom, *vto ;
int			n ;
{
	float		*from, *to ;

	from = ( float * )vfrom ;
	to = ( float * )vto ;
	while ( -- n >= 0 ) {
		*from = NearExp8F( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp16VF( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	float		*from, *to ;

	from = ( float * )vfrom ;
	to = ( float * )vto ;
	while ( -- n >= 0 ) {
		*from = NearExp16F( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExpNVF( void *vfrom, void *vto, float vn, int n )
{
	float		*from, *to ;

	from = ( float * )vfrom ;
	to = ( float * )vto ;
	while ( -- n >= 0 ) {
		*from = NearExpNF( *from, *to, vn ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearRangeVF( vfrom, vto, vrange, n )
void		*vfrom, *vto, *vrange ;
int			n ;
{
	float		*from, *to, *range ;

	from = (float *)vfrom ;
	to = (float *)vto ;
	range = ( float * )vrange ;
	while ( -- n >= 0 ) {
		*from = NearRangeF( *from, *to, *range ) ;
		from ++ ;
		to ++ ;
		range ++ ;
	}
}

void		GV_NearTimeVF( vfrom, vto, interp, n )
void		*vfrom, *vto ;
int		interp, n ;
{
	float		*from, *to ;

	from = ( float * )vfrom ;
	to = ( float * )vto ;
	while ( -- n >= 0 ) {
		*from = NearTimeF( *from, *to, interp ) ;
		from ++ ;
		to ++ ;
	}
}

/*----------------------------------------------------------------*/

void		GV_NearExp2PV( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp2P( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp4PV( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp4P( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp8PV( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp8P( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

//#ifdef PAL
void		GV_NearExp8PVPAL( vfrom, vto, n )
void		*vfrom, *vto ;
int			n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp8PPAL( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}
//#endif

void		GV_NearExp16PV( vfrom, vto, n )
void		*vfrom, *vto ;
int		n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp16P( *from, *to ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExpNPV( vfrom, vto, n, nv )
void		*vfrom, *vto ;
int			n, nv ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExpNP( *from, *to, nv ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearSpeedPV( vfrom, vto, n, speed )
void		*vfrom, *vto ;
int			n, speed ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearSpeedP( *from, *to, speed ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp2SV( vfrom, vto, speed, n )
void		*vfrom, *vto ;
int		speed, n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp2S( *from, *to, speed ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp4SV( vfrom, vto, speed, n )
void		*vfrom, *vto ;
int		speed, n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp4S( *from, *to, speed ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp8SV( vfrom, vto, speed, n )
void		*vfrom, *vto ;
int		speed, n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp8S( *from, *to, speed ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearExp16SV( vfrom, vto, speed, n )
void		*vfrom, *vto ;
int		speed, n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearExp16S( *from, *to, speed ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearTimePV( vfrom, vto, interp, n )
void		*vfrom, *vto ;
int			interp, n ;
{
	short		*from, *to ;

	if ( interp > 15 ) interp = 15 ;
	interp = TimeInv[ interp ] ;
	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = NearTime( NearPhase( *from, *to ), *to, interp ) ;
		from ++ ;
		to ++ ;
	}
}

void		GV_NearTimePV2( vfrom, vto, interp, n )
void		*vfrom, *vto ;
int			interp, n ;
{
	short		*from, *to ;

	from = (short *)vfrom ;
	to = (short *)vto ;
	while ( -- n >= 0 ) {
		*from = ( int )NearTimeF( ( float )NearPhase( *from, *to ), ( float )*to, interp ) ;
		from ++ ;
		to ++ ;
	}
}


