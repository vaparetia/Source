/*
  Type definitions and Macro Definitions

  Tue Nov 16 07:45:11 JST 1999
  T.Morita

  $Id: alltypes.h,v 1.7 2002/06/13 01:35:48 usr04098 Exp $
*/

#define MAX_PANE 600
#define MAX_VRTX 512
#define MAX_POLY 512

#include <sys/types.h>
#ifdef _WIN32
#include "../libMDU/fmt_kms.h"
#else
#include "fmt_kms.h"
#endif

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

typedef struct poly_t
{
    int num ;

    int id[4] ;
    int uv[4][XY] ;

    FVECTOR center ;
} Poly ;

typedef struct xlib_t
{
#ifndef _WIN32
    Display *disp ;
    Window   win  ;

    GC  ovr, ers ;
    GC  dot, fnt ;
#endif

    int wdth, hght ;

    FVECTOR center ;
    FVECTOR size   ;
} Xdir ;


enum
{
    FLG_FINISHED  = 0x00000001,
    FLG_NO_WINDOW = 0x00000002,
    FLG_TRANS     = 0x00000004,
    FLG_NO_BEVEL  = 0x00000008,
} ;

/* Initialize Param */
extern void init_param( Xdir *p, int argc, char *argv[] ) ;

/* Create Polygons */
extern void create_polygon( Xdir *p ) ;

#ifndef _WIN32
/* Event Function (event.c) */
extern int VisibilityNotifyEvent( Xdir *p, XEvent *e ) ;
extern int KeyPressEvent( Xdir *p, XEvent *e ) ;
extern int ExposeEvent( Xdir *p, XEvent *e ) ;
extern int MotionNotifyEvent( Xdir *p, XEvent *e ) ;
#endif

/* KMS file out (output.c) */
extern int output( Xdir *p ) ;

/* StrCode function (strcode.c) */
int GV_StrCode( char *string ) ;

/* Create Bevel (bevel.c) */
int create_bevel( Poly *p, SVECTOR *v, SVECTOR *n, short *u,
#ifdef _XBOX
		  short *idx,
#endif
		  FVECTOR *cen, int depth, int axis ) ;


#ifdef _MAIN_FILE_

Poly    poly[MAX_POLY] ;
int     n_poly = 0 ;
FVECTOR vrtx[MAX_VRTX] ;
int     n_vrtx = 0 ;
int     h_side[MAX_VRTX] ;
int     w_side[MAX_VRTX] ;
int     width   ;
int     height  ;
int     mergine ;
int     thick   ;
int     lvl     ;

int     tri_id  ;
int     tex_id  ;

int     flag = 0 ;

Xdir    xdir ;
char   *fname = "fract.kms" ;

#else

extern Poly    poly[MAX_POLY] ;
extern int     n_poly ;
extern FVECTOR vrtx[MAX_VRTX] ;
extern int     n_vrtx ;
extern int     h_side[MAX_VRTX] ;
extern int     w_side[MAX_VRTX] ;
extern int     width   ;
extern int     height  ;
extern int     mergine ;
extern int     thick   ;
extern int     lvl     ;

extern int     tri_id  ;
extern int     tex_id  ;

extern int     flag    ;

extern Xdir    xdir  ;
extern char   *fname ;

#endif
