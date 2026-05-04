/*
  Event Routines

  Tue Nov 16 07:45:11 JST 1999
  T.Morita

  $Id: event.c,v 1.4 2002/04/10 00:52:38 usr04098 Exp $
  */
#ifndef _WIN32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>
#include <time.h>

#include "alltypes.h"


#ifndef _WIN32
void DrawLine( Xdir *p, GC gc, float x1, float y1, float x2, float y2 )
{
    float s = p->size.vx>p->size.vy ? p->size.vx : p->size.vy ;

    XDrawLine( p->disp, p->win, gc,
	       (x1 - p->center.vx)/s*0.45f *p->wdth + p->wdth/2,
	       (y1 - p->center.vy)/s*0.45f *p->hght + p->hght/2,
	       (x2 - p->center.vx)/s*0.45f *p->wdth + p->wdth/2,
	       (y2 - p->center.vy)/s*0.45f *p->hght + p->hght/2 ) ;
}

void DrawImageString( Xdir *p, Poly *ply )
{
    static char id_str[3] ;
    float s = p->size.vx>p->size.vy ? p->size.vx : p->size.vy ;

    sprintf( id_str, "%2d", ply->num ) ;
    XDrawImageString( p->disp, p->win, p->fnt,
		      (ply->center.vx - p->center.vx)/s*0.45f *p->wdth + p->wdth/2,
		      (ply->center.vy - p->center.vy)/s*0.45f *p->hght + p->hght/2,
		      id_str, strlen(id_str) ) ;

}

void DrawPoly( Xdir *p, GC gc, FVECTOR *v, Poly *ply )
{
    DrawLine( p, gc,
	      v[ply->id[0]].vx, v[ply->id[0]].vy,
	      v[ply->id[1]].vx, v[ply->id[1]].vy ) ;
    if ( ply->id[3] == -1 )
	DrawLine( p, gc,
		  v[ply->id[1]].vx, v[ply->id[1]].vy,
		  v[ply->id[2]].vx, v[ply->id[2]].vy ) ;
    else
    {
	DrawLine( p, gc,
		  v[ply->id[1]].vx, v[ply->id[1]].vy,
		  v[ply->id[3]].vx, v[ply->id[3]].vy ) ;
	DrawLine( p, gc,
		  v[ply->id[3]].vx, v[ply->id[3]].vy,
		  v[ply->id[2]].vx, v[ply->id[2]].vy ) ;
    }
    DrawLine( p, gc,
	      v[ply->id[2]].vx, v[ply->id[2]].vy,
	      v[ply->id[0]].vx, v[ply->id[0]].vy ) ;
}

void RefreshWindow( Xdir *p )
{
    int      i   = n_poly ;
    Poly    *ply = poly ;

    for ( ; --i>=0 ; ply++ )
    {
	DrawPoly( p, p->ovr, vrtx, ply ) ;
	DrawImageString( p, ply ) ;
    }
}


void DrawOnePoly( Xdir *p, int add )
{
    static int id = 0 ;
    static char id_str[4] ;

    XClearWindow( p->disp, p->win ) ;
    id = (id + add + n_poly) % n_poly ;
    DrawPoly( p, p->ovr, vrtx, &poly[id] ) ;
    sprintf( id_str, "%3d", id ) ;
    XDrawImageString( p->disp, p->win, p->fnt, p->wdth-14*10, 14, id_str, strlen(id_str) ) ;
}


int VisibilityNotifyEvent( Xdir *p, XEvent *e )
{
    int i, wth, hgt ;

    XGetGeometry( p->disp, p->win, (Window*)&i, &i, &i, &wth, &hgt, &i, &i ) ;
    if ( wth == p->wdth && hgt == p->hght )
	return 0;
    p->wdth = wth ;
    p->hght = hgt ;
    XClearWindow( p->disp, p->win ) ;

    return 0 ;
}

int KeyPressEvent( Xdir *p, XEvent *e )
{
    if ( e->xkey.keycode == 24 ) /*Quit*/
	return 1 ;
    else if ( e->xkey.keycode == 33 ) /*Print out*/
    {
	output( p ) ;
    }
    else if ( e->xkey.keycode == 54 ) /*Clear*/
    {
	u_int t ;

	time( (time_t*)&t ) ;
	srandom( t = t*random()/4096 ) ;
	printf( "srandom= %d\n", (int)t ) ;
	create_polygon( p ) ;
	XClearWindow( p->disp, p->win ) ;
	RefreshWindow( p ) ;
    }
    else if ( e->xkey.keycode == 43 ) /*Help*/
	DrawOnePoly( p, 1 ) ;
    return 0 ;
}

int ExposeEvent( Xdir *p, XEvent *e )
{
    RefreshWindow( p ) ;
    return  0 ;
}
#endif /*_WIN32*/
