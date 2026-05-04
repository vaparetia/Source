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

#include	"gameheader.h"
#include	"libutl.h"

#include "hit.h"



static int HZX_DetSegCollide3D( CONTROL *c, HZX_SEG *s, struct seg_ext_t *e, float wdth, float leng )
{
    float ax, az, d, dd, x, z ;
    float sn, cs, asn, acs ;
    float det0, det1, det2=0.0f ;
    int   ang0, ang1, cnt = -1 ;

    if ( !s || (u_int)s > 0x4000000 )
	return 0 ;
    e->seg = s ;
    x  = c->step.vx + (e->cntr_x = c->mov.vx - (s->p1.x + s->p2.x)/2.0f) ;
    z  = c->step.vz + (e->cntr_x = c->mov.vz - (s->p1.z + s->p2.z)/2.0f) ;
    ax = s->p2.z - s->p1.z ;
    az = s->p2.x - s->p1.x ;
    d  = fpu_Sqrt( ax*ax+az*az ) ;
    e->asn = asn = ax/d ;
    e->acs = acs = az/d ;
    az = -x*asn + z*acs ;
    e->ang = ang1 = 2048.0f*asinf(asn)/M_PI ;
    d /= 2.0f ;
    e->turn  = c->turn.vy ;
    do
    {
	cnt++ ;
	if ( cnt == 2 )
	{
	    dd = (c->step.vx*acs + c->step.vz*asn) ;
	    if ( (int)(e->step.vx / (c->step.vx + dd*asn + (az<0.0f ?  det2:-det2) * asn)) )
		e->step.vx = (c->step.vx + dd*asn + (az<0.0f ?  det2:-det2) * asn) ;
	    if ( (int)(e->step.vz / (c->step.vz + dd*acs + (az<0.0f ? -det2: det2) * acs)) )
		e->step.vz = (c->step.vz + dd*acs + (az<0.0f ? -det2: det2) * acs) ;
//printf( "stp(%.2f,%.2f)->(%.2f,%.2f)d(%.2f) det2(%.2f)\n", c->step.vx, c->step.vz,e->step.vx, e->step.vz, d, det2 ) ;

	    break ;
	}
	else if ( cnt == 1 )
	{
	    dd = (c->step.vx*acs + c->step.vz*asn) ;
	    e->step.vx = c->step.vx + dd*asn + (az<0.0f ?  det2:-det2) * asn ;
	    e->step.vz = c->step.vz + dd*acs + (az<0.0f ? -det2: det2) * acs ;

asm volatile ( "NOP #######################################" );
	    ang0 =  2048.0f*asinf( abs(az)/LENGTH_OF_CUBE )/M_PI - ANGLE_OF_CUBE ;
	    if      ( e->turn+ang1 <      ANGLE_OF_CUBE )
		e->turn = 0 ;
	    else if ( e->turn+ang1 < 2048-ANGLE_OF_CUBE )
		e->turn = 1024 + (e->turn+ang1>1024 ? ang0 :-ang0 ) ;
	    else if ( e->turn+ang1 < 2048+ANGLE_OF_CUBE )
		e->turn = 2048 ;
	    else if ( e->turn+ang1 < 4096-ANGLE_OF_CUBE )
		e->turn = 3072 + (e->turn+ang1>3072 ? ang0 :-ang0 ) ;
	    else
		e->turn = 0 ;
	    e->turn -= ang1 ;
	}


	sn = fpu_Abs( vu0_SinS( -e->turn ) ) ;
	cs = fpu_Abs( vu0_CosS( -e->turn ) ) ;
	det0 = x*cs - z*sn ;
	det1 = x*sn + z*cs ;
	sn = fpu_Abs( vu0_SinS(-e->turn-ang1 ) ) ;
	cs = fpu_Abs( vu0_CosS(-e->turn-ang1 ) ) ;
	det0 /= ( d*cs + wdth  ) ;
	det1 /= ( d*sn + leng ) ;
	det2 = sn*wdth + cs*leng - fpu_Abs(az) ;
//printf( "d0(%d)%.2f<%.2f d1(%d)%.2f<%.2f d2(%d)%.2f<%.2f\n", det0,tx, d*cs + wdth , det1, tz, d*sn + leng , !(det2 > 0.0f), sn*wdth + cs*LENGTH, fpu_Abs(az)  ) ;
    }
    while( !(int)det0 && !(int)det1 && (det2 > 0.0f) ) ;

    return cnt ;
}

#define MAX_SEGS 2
int HZX_DetCollide( CONTROL *c, float wdth, float leng  )
{
    struct seg_ext_t segs[MAX_SEGS] ;
    static FVECTOR mov ;
    static int turn ;
    int det, i ;

    for ( i=MAX_SEGS, det=0 ; --i>=0 ;  )
	switch( HZX_DetSegCollide3D( c, c->segs[i], &segs[i], wdth, leng ) )
	{
	case 2:
	    c->step.vx = segs[i].step.vx ;
	    c->step.vz = segs[i].step.vz ;
	case 1:
	    c->turn.vy = segs[i].turn    ;
	    if ( ++det > 1 )
	    {
		c->mov     = mov  ;
		c->step.vx = c->step.vz = 0 ;
		c->turn.vy = turn ;
		return 0 ;
	    }
	}
    mov  = c->mov     ;
    turn = c->turn.vy ;
    return 1 ;
}

