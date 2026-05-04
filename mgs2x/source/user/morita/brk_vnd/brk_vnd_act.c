//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_vnd_act.c 
   プット自動販売機 アクト

   1999/12/26 T.Morita
   $Id: brk_vnd_act.c,v 1.1.1.3 2002/11/19 11:45:53 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "brk_vending.h"






/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( HZD_BOX *hzd, CAN *p, int hzd_flag )
{
    static FVECTOR bounce = { BRK_VND_BOUNCE, BRK_VND_BOUNCE, BRK_VND_BOUNCE, 0.0f } ;
    FVECTOR size ;
    FMATRIX *m = &p->objs->world ;
    int flag = 0 ;

    p->rot.vx += p->rot_v.vx ;
    p->rot.vy += p->rot_v.vy ;
    p->rot.vz += p->rot_v.vz ;
    RotateMatrixYXY( m, &p->rot ) ;
    p->pos_v.vy -= BRK_VND_GRAVITY ;

    if ( p->tic > 0 )
	p->tic-- ;

    if ( hzd_flag )
    {
	BRK_UTL_SizeOfBound( p->objs, m, &size ) ;
	hzd_flag = BRK_CheckHazard( hzd, &p->pos, &p->pos_v, &bounce, &size ) ;
    }
    switch( hzd_flag )
    {
    case 3:
    case 1:
	if ( !p->work->tic && p->tic < 0 )
	    GM_SeSetMode( SD_A_ICECAN02, &p->pos, GM_SEMODE_BOMB ) ;

        _sceVu0ScaleVector( &p->pos_v, &p->pos_v, BRK_VND_VEL_R ) ;
	if ( BRK_UTL_PutCenterHazard( hzd, &p->pos, &p->pos_v, 1000.0f, 25.0f ) )
	{
	    p->pos_v.vy = 30.0f ;
	    p->rot_v.vx = irnd()&10 ? 200 : -200 ;
	    flag = 2 ;
	}
	else if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
	{
	    if ( p->tic || p->pos_v.vy<BRK_VND_GRAVITY*4 )
		flag = 1 ;
	}
	else
	    p->rot_v.vx = /* (( p->rot.vx>0 ? DEGtoANG(-90) : DEGtoANG(90)) - p->rot.vx) / 16*/ irnd() & 255 ;
	p->tic = 3 ;
	p->work->tic = 3 ; /* 3フレーム以内に音がならないようにしている */
        break ;
    case 0:
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    }

    _sceVu0CopyVectorXYZ( (FVECTOR *)m->m[W], &p->pos ) ;
    GM_MoveTarget2Map( &p->target, m, p->work->where ) ;
    DG_GetLightMatrixFix( &p->pos, p->lights ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( HZD_BOX *hzd, CAN *p )
{
    int      r, flag ;
    FVECTOR  size ;
    FMATRIX *m = &p->objs->world ;

    r = p->rot.vx + p->rot_v.vx ;
    p->rot_v.vx += (r>0 ? DEGtoANG(2) : DEGtoANG(-2))*(r/DEGtoANG(90) ? -1 : 1) ;
#if 1
    printf( "r%d rot%d+%d (%d) ==> %d %d\n", r, p->rot.vx, p->rot_v.vx, DEGtoANG(90),
	    (!(p->rot.vx/DEGtoANG(90)) &&   r/DEGtoANG(90) ),
	    (  p->rot.vx/DEGtoANG(90)  && !(r/DEGtoANG(90))) );
#endif
    flag = 1 ;
    if ( (!(p->rot.vx/DEGtoANG(90)) &&   r/DEGtoANG(90) ) ||
	 (  p->rot.vx/DEGtoANG(90)  && !(r/DEGtoANG(90))) )
    {
	r = p->rot.vx>0 ? DEGtoANG(90) : DEGtoANG(-90) ;
	p->rot_v.vx = 0 ;
	if ( !p->rot_v.vy && !p->rot_v.vz )
	    flag = 0 ;
    }

    p->rot.vx  = r ;
    p->rot.vy += p->rot_v.vy = p->rot_v.vy*15/16  ;
    p->rot.vz += p->rot_v.vz = p->rot_v.vz*15/16  ;
    RotateMatrixYXY( m, &p->rot ) ;

    p->pos_v.vy = -BRK_VND_GRAVITY*5 ;
    BRK_UTL_SizeOfBound( p->objs, m, &size ) ;
    if ( !BRK_CheckHazard( hzd, &p->pos, &p->pos_v, &BRK_HZD_NoBounce, &size ) )
	flag = 2 ;
    else
	p->pos_v.vy = 0 ;
    _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
    _sceVu0CopyVectorXYZ( (FVECTOR *)m->m[W], &p->pos ) ;
    GM_MoveTarget2Map( &p->target, m, p->work->where ) ;

    return flag ;
}


#if 0
void BRK_VND_ActCanMove( CAN *c, int move )
{
    int len ;

    switch( move )
    {
    case 1:
	c->flag = 2 ;
    default :
	if ( (c->snd_flag&0xffff) < 4 || move >= 4 )
	{
	    if((c->snd_flag>>16) > 20)
		len = GM_MAX_VOL ;
	    else
	    {
		len = (GM_MAX_VOL>>2) + ((c->snd_flag>>16) * (GM_MAX_VOL>>2) * 3 )/20 ;
		c->snd_flag++ ;
	    }
	    GM_SeSet( GM_SeGetPan( &c->pos, GM_SEMODE_BOMB ), len, SD_A_ICECAN02 ) ;
	    GM_SeSetMode( SD_A_ICECAN02, &c->pos, GM_SEMODE_BOMB ) ;
	    c->snd_flag &= 0xffff ;
	}
	break ;
    case 0:
	c->snd_flag += 0x00010000 ;
    }
}
#endif


void BRK_VND_ActCan( Work *work )
{
    int  i ;
    CAN *c ;
    int  flag ;

    if ( work->tic > 0 )
	work->tic-- ;

    flag = 0 ;
    for ( i=work->n_can, c=work->can ; --i>=0 ; c++ )
    {
	switch( c->flag )
	{
	case 4:
	    if ( (c->objs = BRK_VND_ChangeModel( work, work->can_nrm, NULL, c->lights )) )
		BRK_VND_InitTarget( &c->target, &c->power, work->where, c->objs,
				    BRK_VND_CanCallBack, c ) ;
	    else
	    {
		c->flag = 0 ;
		printf( "Can't initialize DG_OBJS for Can(Mayb no Memory) :: NewVendingMachine\n" ) ;
		break ;
	    }

	default:
	    //BRK_VND_ActCanMove( c, MoveObject( work->hzd, c, 0 ) ) ;
	    c->flag-- ;
	    MoveObject( work->hzd, c, 0 ) ;
	    break ;

	case 2:
	    //BRK_VND_ActCanMove( c, MoveObject( work->hzd, c, 1 ) ) ;
	    if ( MoveObject( work->hzd, c, 1 ) == 1 )
		c->flag = 1 ;
	    break ;

	case 1:
	    c->flag = FallDownObject( work->hzd, c ) ;
	    break ;

	case 0:
		;
	}
	flag |= c->flag ;
    }

    if ( !flag )
	work->n_can |= BRK_VND_INACTIVE ;
}
