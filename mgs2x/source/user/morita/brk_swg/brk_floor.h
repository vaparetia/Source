/*
   brk_floor.h
   揺れ落下

   1999/12/13 T. Morita
   $Id: brk_floor.h,v 1.1.1.3 2002/11/19 11:45:47 Yoshizawa1 Exp $
*/

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;
#include "../include/util.h"

#include "brk_flr_def.h"

#define DEGtoANG(_a) ((_a)*8192/45)
#define ANGtoDEG(_a) ((int)(_a)*180/32768)
#define ANGtoRAD(_a) ((_a)*(float)M_PI/32768.0f)

#define BRK_MAX_SPEED 300.0f
#define BRK_GRAVITY 4
#define BRK_VEL_R 16
#define BRK_ROT_R 16

typedef struct work_t
{
    GV_ACT_EX    actor   ;

    //TARGET       target ;
    //POWER_TARGET power  ;

    FVECTOR   center ; /* 回転の中心位置     */
    FVECTOR   offset ; /* 回転後のオフセット */
    FVECTOR   pos    ; /* 位置     */
    FVECTOR   pos_d  ; /* 元の位置 */
    float     pos_vy ; /* 落下速度 */

    short     rot_x,  rot_y,  rot_z  ; /* 現在の角度 */
    short     rot_vx, rot_vy, rot_vz ; /* 角速度     */
    short     rot_dx, rot_dy, rot_dz ; /* 元の角度   */

    DG_OBJS  *objs    ;
    DG_OBJS  *thermal ;

    float     level ;
    float     length ; /* 床との距離 */
    int       where ;
    int       value ; /* シグナルの引数 各アクションによって用途が変わる */
    int       falltype ;
    short     mode  ;
    short     se_mode ;
} Work ;


extern void BRK_FLR_Act( Work *work ) ;
extern void BRK_FLR_MakeObjWorld( Work *work ) ;
