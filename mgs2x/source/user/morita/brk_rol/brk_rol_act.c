/*
   brk_rol_act.c
   転がりオブジェクト アクト

   1999/12/13 T. Morita
   $Id: brk_rol_act.c,v 1.1.1.3 2002/11/19 11:45:44 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_roll.h"


static void GetCurrentSize( DG_OBJS *objs, FMATRIX *m, FVECTOR *size )
{
    int     i, j ;
    SVECTOR    *s ;
    FVECTOR f, min, max ;
    DG_MDLPACK *o = objs->objs->model->packs ;

    min.vx = min.vy = min.vz =  60000.0f ;
    max.vx = max.vy = max.vz = -60000.0f ;
    for ( i=objs->objs->model->n_packs ; --i>=0 ; o++ )
        for ( j=o->n_verts, s=(SVECTOR*)o->verts ; --j>=0 ; s++ )
        {
            SVector2FVector( &f, s ) ;
            ApplyMatrixXYZ( &f, m, &f ) ;
            MinMaxVector( &min, &max, &f ) ;
        }
    _sceVu0SubVector( size, &max, &min ) ;
    _sceVu0ScaleVector( size, size, 0.5f ) ;
}


/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( HZD_BOX *hzd, FMATRIX *m, MOVE *p, DG_OBJS *objs )
{
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    int flag = 0 ;
    FVECTOR size ;

    p->rot_x = (p->rot_x + p->rot_vx)&4095 ;
    p->rot_y = (p->rot_y + p->rot_vy)&4095 ;
    p->roll  = (p->roll  + p->roll_v)&4095 ;
    p->rot_x = p->rot_x>2048 ? p->rot_x-4096 : p->rot_x ;
    p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;
    p->roll  = p->roll >2048 ? p->roll -4096 : p->roll  ;

    RotateMatrixYXY( m, &DG_UnitMatrix, p->roll, p->rot_x, p->rot_y ) ;
    if ( objs )
        GetCurrentSize( objs, m, &size ) ;
    p->pos_v.vy -= BRK_GRAVITY ;
    switch( BRK_CheckHazard( hzd, &p->pos, &p->pos_v, &bounce, &size ) )
    {
    case 3:
    case 1:
        _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VEL_R ) ;
        flag = (int)(p->pos_v.vy/BRK_GRAVITY)+2 ;
        if ( flag == 2 )
            if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
                flag = 1 ;
        if ( flag > 4 )
            p->rot_vx = irnd()&10 ? 200 : -200 ;
        else
            p->rot_vx = ( (p->rot_x < - 2048                       ?  p->limit[2] :
			   p->rot_x < -(p->limit[2]+p->limit[1])/2 ? -p->limit[2] :
			   p->rot_x < -(p->limit[1]+p->limit[0])/2 ? -p->limit[1] :
			   p->rot_x <   0                          ? -p->limit[0] :
			   p->rot_x <  (p->limit[1]+p->limit[0])/2 ?  p->limit[0] :
			   p->rot_x <  (p->limit[2]+p->limit[1])/2 ?  p->limit[1] : 
			   p->rot_x <   2048                       ?  p->limit[2] :-p->limit[2] )
			  - p->rot_x) / 16 ;
        break ;
    case 0:
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( HZD_BOX *hzd, FMATRIX *m, MOVE *p, DG_OBJS *objs )
{
    int r, flag = 2 ;
    int i ;
    FVECTOR size ;

    /* 角度の倒れる傾向が以下の式 */
    r = p->rot_x + (p->rot_vx += (p->rot_x < - 2048                       ? -10 :
				  p->rot_x < - p->limit[2]                ?  10 :
				  p->rot_x < -(p->limit[2]+p->limit[1])/2 ? -10 :
				  p->rot_x < - p->limit[1]                ?  10 :
				  p->rot_x < -(p->limit[1]+p->limit[0])/2 ? -10 :
				  p->rot_x < - p->limit[0]                ?  10 :
				  p->rot_x <   0                          ? -10 :
				  p->rot_x <   p->limit[0]                ?  10 :
				  p->rot_x <  (p->limit[1]+p->limit[0])/2 ? -10 :
				  p->rot_x <   p->limit[1]                ?  10 :
				  p->rot_x <  (p->limit[2]+p->limit[1])/2 ? -10 : 
				  p->rot_x <   p->limit[2]                ?  10 :
				  p->rot_x <   2048                       ? -10 : 10 )) ;

    /* 安定角度に達したかどうか（安定角度を通過したか？） */
    for ( i=3 ; --i>=0 ; )
	if ( (r > p->limit[i] && p->rot_x <= p->limit[i]) || 
	     (r < p->limit[i] && p->rot_x >= p->limit[i])  )
	    r =  p->limit[i], flag = i==1 ? 3 : 0, i = 0 ;
	else if ( (r > -p->limit[i] && p->rot_x <= -p->limit[i]) || 
		  (r < -p->limit[i] && p->rot_x >= -p->limit[i])  )
	    r = -p->limit[i], flag = i==1 ? 3 : 0, i = 0 ;
    p->rot_x = r ;
    p->rot_vy = p->rot_vy * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->rot_y = (p->rot_y + p->rot_vy)&4095 ;
    p->roll  = (p->roll  + p->roll_v)&4095 ;
    p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;
    p->roll  = p->roll >2048 ? p->roll -4096 : p->roll  ;
    p->pos_v.vy = -BRK_GRAVITY ;
    RotateMatrixYXY( m, &DG_UnitMatrix, p->roll, p->rot_x, p->rot_y ) ;
    if ( objs )
        GetCurrentSize( objs, m, &size ) ;
    if ( !BRK_CheckHazard( hzd, &p->pos, &p->pos_v, &DG_ZeroVector, &size ) )
	p->pos_v.vy = 0, flag = 1 ;
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/* 転がる */
static int RollObject( HZD_BOX *hzd, FMATRIX *m, MOVE *p, DG_OBJS *objs )
{
    int    i, flag = 3 ;
    FVECTOR size ;
    float  tn = tanf( (p->limit[1] - 1024) / 2048.0f * (float)M_PI) ;

    p->rot_vy = tn * p->roll_v ;
    p->rot_y = (p->rot_y + p->rot_vy)&4095 ;
    p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;

    if ( !(p->roll_v = p->roll_v * (BRK_ROT_R-1) / BRK_ROT_R) )
	flag = 0 ;
    p->roll  = (p->roll  + p->roll_v)&4095 ;
    p->roll  = p->roll >2048 ? p->roll -4096 : p->roll  ;

    RotateMatrixYXY( m, &DG_UnitMatrix, p->roll, p->rot_x, p->rot_y ) ;
    if ( objs )
        GetCurrentSize( objs, m, &size ) ;

    p->pos_v.vy = -BRK_GRAVITY*10 ;
    p->pos_v.vx = tn * size.vy * cosf( p->rot_y / 2048.0f * (float)M_PI ) ;
    p->pos_v.vz = tn * size.vy * sinf( p->rot_y / 2048.0f * (float)M_PI ) ;

    printf( "rn %f rvy %d siz %f  p %.2f %.2f %.2f v %.2f %.2f %.2f ->", tn, p->rot_vy, size.vy,
	    p->pos.vx, p->pos.vy, p->pos.vz,    p->pos_v.vx, p->pos_v.vy, p->pos_v.vz ) ;

    switch( i=BRK_CheckHazard( hzd, &p->pos, &p->pos_v, &DG_ZeroVector, &size ) )
    {
    case 0:
	printf( "(0) p %.2f %.2f %.2f v %.2f %.2f %.2f\n", 
		p->pos.vx, p->pos.vy, p->pos.vz,  p->pos_v.vx, p->pos_v.vy, p->pos_v.vz ) ;
	p->pos_v.vy = 0 ;
	flag = 1 ;
	break ;
    case 1:
	printf( "(1) p %.2f %.2f %.2f v %.2f %.2f %.2f\n",
		p->pos.vx, p->pos.vy, p->pos.vz,  p->pos_v.vx, p->pos_v.vy, p->pos_v.vz ) ;
        p->pos_v.vy = 0 ;
	_sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
	break ;
    default:
	printf( "(%d) p %.2f %.2f %.2f v %.2f %.2f %.2f\n", i,
		p->pos.vx, p->pos.vy, p->pos.vz,  p->pos_v.vx, p->pos_v.vy, p->pos_v.vz ) ;
    }
    TransMatrix( m, &p->pos ) ;

    return flag ;
}





/***

  筒のアクト関数

  ***/
void BRK_ROL_ActRoll( Work *work )
{
    int    i, flag = 0 ;
    ROLL *p ;

    for ( i=work->n_roll, p=work->roll ; --i>=0 ; p++ )
    {
        switch ( p->flag )
        {
	case -1:/* 全く処理しない(起動してないないか,割れてなくなった) */
	case  0:/* 全く処理しない(床に落ち着いている) */
	    continue ;
        case 1:/* 落ちている処理 */
            if ( MoveObject( work->hzd, &p->objs->world, &p->mov, p->objs ) == 1 )
                p->flag = 2 ;
	    DG_GetLightMatrixFix( &p->mov.pos, p->lights ) ;
            break ;
        case 2:/* 床に落ち着く処理 */
            p->flag = FallDownObject( work->hzd, &p->objs->world, &p->mov, p->objs ) ;
	    break ;
        case 3:/* 床を転がる処理 */
            p->flag = RollObject( work->hzd, &p->objs->world, &p->mov, p->objs ) ;
        }
	GM_MoveTarget2( &p->target, &p->objs->world ) ;
        flag = 1 ;
	printf( "[%d] rx %d ry %d rol %d v%d\n", p->flag, p->mov.rot_x, p->mov.rot_y, p->mov.roll,p->mov.roll_v ) ;
    }
    if ( !flag )
        work->n_roll |= BRK_ROL_INACTIVE ;
}
