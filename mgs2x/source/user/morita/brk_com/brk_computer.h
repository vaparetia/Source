/*
   wall_scar.c
   弾痕

   1999/11/09 T. Morita
   $Id: brk_computer.h,v 1.1.1.3 2002/11/19 11:45:26 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;

// for DG-LIB
#include "../include/libdg_x.h"

#include "../include/util.h"
#include "../brk_utl/brk_utl.x"

typedef struct conf_t 
{
    FVECTOR v[4]  ;
    short   flag  ;
    short   group ;
    int     name  ;
    int   life  ;
} BRK_CONF ;

typedef struct comp_t COMP ;
typedef struct work_t Work ;
struct work_t
{
    GV_ACT    actor  ;

    int       name   ;
    int       flag   ; /* 壊れ物フラグ */
    int       proc   ;

    DG_PRIM2 *prim   ;
    short     n_comp ;

    short     non_damage ;

    struct comp_t
    {
	TARGET          target ;
	POWER_TARGET    power  ;

	BRK_CONF       *conf ;

	FVECTOR        *pos   ;
	DG_PRIM2_UVRGB *uvrgb ;
	int             life  ;
	int             flag  ;
	void           *ext   ;
	Work           *work  ;
    } *comp ;
} ;

extern void *NewBreakComputer( int name, int where,
			       BRK_CONF *comp, int n_comp,
			       int flag, int proc ) ;


#define POSW(_x,_y,_z,_w) {(_x),(_y),(_z),(_w)}
#define POS(_x,_y,_z) {(_x),(_y),(_z), 1.0f}
#define COL(_x,_y,_z) {(_x),(_y),(_z),128.0f}
