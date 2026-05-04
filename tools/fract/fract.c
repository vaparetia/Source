/*
  UV Map Tool

  Tue Nov 16 07:45:11 JST 1999
  T.Morita

  $Id: fract.c,v 1.4 2002/04/10 00:52:38 usr04098 Exp $
  */
#ifndef _WIN32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif /*_WIN32*/
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#define _MAIN_FILE_
#include "alltypes.h"

void init_param( Xdir *p, int argc, char *argv[] )
{
    int i ;

    p->wdth = p->hght = 600 ;

    for( i=MAX_VRTX ; --i>=0 ; )
	h_side[i] = w_side[i] = -1 ;

    /* default param */
    width = height = 285 ;
    mergine = 25 ;
    thick   = 5 ;
    tri_id  = GV_StrCode( "w01a1" ) ;
    tex_id  = GV_StrCode( "g_dankon_alp_ovl" ) ;
#if 1
    tri_id = 15709653 ; /*btl_piece ovl_add */
    tex_id = 1363563  ;

    tri_id = 15709653 ;/*break7d ovl_alp*/
    tex_id = 8291175  ;

#endif

    lvl    = 12 ;

    for( ; --argc>=0 ; argv++ )
	if ( (*argv)[0] == '-' )
	    switch( (*argv)[1] )
	    {
	    case 'n':
		switch( (*argv)[2] )
		{
		case 'w':
		    flag |= FLG_NO_WINDOW ;
		    break ;
		}
		break ;
	    case 'o':
		fname = *++argv ; argc-- ;
		break ;
	    case 'l':
		lvl     = atoi( *++argv ) ; argc-- ;
		break ;
	    case 'g':
		width   = atoi( *++argv ) ; argc-- ;
		height  = atoi( *++argv ) ; argc-- ;
		mergine = atoi( *++argv ) ; argc-- ;
		thick   = atoi( *++argv ) ; argc-- ;
		break ;
	    case 't':
		tri_id = GV_StrCode( *++argv ) ; argc-- ;
		tex_id = GV_StrCode( *++argv ) ; argc-- ;
		break ;
	    case 's':
		srandom( atoi( *++argv ) ) ; argc-- ;
		break ;
	    default:
		printf( "Invalid Option<%s>\n", *argv ), exit( 1 ) ;
	    }
	else
	    fname = *argv ;
}

int main( int argc, char *argv[] )
{
    Xdir p ;
#ifndef _WIN32
    XEvent e ;

    init_param( &p, argc-1, argv+1 ) ;
    create_polygon( &p ) ;
    if ( flag & FLG_NO_WINDOW )
    {
	output( &p ) ;
	return 0 ;
    }
    p.disp = XOpenDisplay( NULL ) ;
    p.win  = XCreateSimpleWindow( p.disp, RootWindow(p.disp, 0),
				  350,50, p.wdth, p.hght, 0,
				  BlackPixel(p.disp, 0),
				  WhitePixel(p.disp, 0) ) ;
    XSelectInput( p.disp, p.win, 
		  KeyPressMask| ExposureMask| DestroyNotify| VisibilityChangeMask ) ;

    p.ers = XCreateGC( p.disp, p.win, 0, 0 ) ;
    XSetForeground( p.disp, p.ers, BlackPixel( p.disp, 0 ) ) ;
    XSetFunction( p.disp, p.ers, GXequiv ) ;
    p.ovr = XCreateGC( p.disp, p.win, 0, 0 ) ;
    XSetForeground( p.disp, p.ovr, BlackPixel( p.disp, 0 ) ) ;

    p.fnt = XCreateGC( p.disp, p.win, 0, 0 ) ;
    XSetForeground( p.disp, p.fnt, BlackPixel( p.disp, 0 ) ) ;
    XSetBackground( p.disp, p.fnt, WhitePixel( p.disp, 0 ) ) ;
    XSetFont( p.disp, p.fnt, XLoadFont( p.disp, "r14" ) ) ;

    p.dot = XCreateGC( p.disp, p.win, 0, 0 ) ;
    XSetForeground( p.disp, p.dot, BlackPixel( p.disp, 0 ) ) ;
    XSetLineAttributes( p.disp, p.dot, 1, LineOnOffDash, CapButt, JoinMiter ) ;

    XMapWindow( p.disp, p.win ) ;

    while( !(flag & FLG_FINISHED) ) 
    {
	XNextEvent( p.disp, &e ) ;
	switch( e.type )
	{
	case KeyPress :
	    flag = KeyPressEvent( &p, &e ) ;
	    break ;

	case VisibilityNotify :
	    flag = VisibilityNotifyEvent( &p, &e ) ;

	case Expose :
	    flag = ExposeEvent( &p, &e ) ;
	    break ;

	case DestroyNotify :
	    flag |= FLG_FINISHED ;
	    break ;
	}
	if ( XEventsQueued( p.disp, QueuedAfterFlush ) > 5 )
	    XSync( p.disp, True ) ;
   }
    XCloseDisplay( p.disp ) ;

#else /*WIN32*/
    init_param( &p, argc-1, argv+1 ) ;
    create_polygon( &p ) ;
    output( &p ) ;
#endif

    return 0 ;
}
