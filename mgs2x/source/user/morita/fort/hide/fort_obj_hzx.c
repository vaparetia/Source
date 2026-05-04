//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include "fort_obj.h"

extern void           HZX_MoveDynamicSegment( HZX_D_SEGMENT *seg, IVECTOR *p1, IVECTOR *p2 ) ;
extern HZX_D_SEGMENT *HZX_AddDynamicSegment( HZX_GROUP_ID id, IVECTOR *p1, IVECTOR *p2, u_int atr ) ;

void FRT_OBJ_FreeHzxSegAndFloor( HIDE *hide )
{
    int i ;

    for ( i=4 ; --i>=0 ; )
	if ( hide->segs[i] )
	{
	    HZX_RemoveDynamicSegment( hide->segs[i] ) ;
	    hide->segs[i] = NULL ;
	}
}

void FRT_OBJ_MakeHzxSizeAndCenter( DG_DEF *def, FVECTOR *t_size, FVECTOR *t_pos )
{
    FVECTOR uv = { def->ux, def->uy, def->uz, 0.0f } ;
    FVECTOR lv = { def->lx, def->ly, def->lz, 0.0f } ;

    _sceVu0SubVector  ( t_size, &uv, &lv ) ;
    _sceVu0ScaleVector( t_size, t_size, 0.5f ) ;
    _sceVu0AddVector  ( t_pos , &uv, &lv ) ;
    _sceVu0ScaleVector( t_pos , t_pos , 0.5f ) ;
    t_pos->vx += def->tx ;
    t_pos->vy += def->ty ;
    t_pos->vz += def->tz ;
    t_pos->vw  = 1.0f ;
}

int FRT_OBJ_MakeHzxFromOBJS( HIDE *hide, DG_OBJS *objs, int where, FVECTOR *t_size, FVECTOR *t_pos )
{
    IVECTOR p1, p2 ;
    int     i ;
    FVECTOR v ;
    HZX_GROUP_ID hzx_id ;
    int     s_flag ;

    ASSERT( where ) ;

#if 0 /* 2001.5.28 フトラスナ指令(by Yoshiike) */
    /* ＸＺ方向に 3cm 太らす */
    t_size->vx += 30.0f ;
    t_size->vz += 30.0f ;
#endif
    switch( FRT_OBJ_GetTypeMasked( hide ) )
    {
    case FRT_TYP_IRONBOX2M_A:    case FRT_TYP_IRONBOX2M_B:
    case FRT_TYP_IRONBOX2M_C:    case FRT_TYP_IRONBOX2M_D:
    case FRT_TYP_IRONBOX2M_E:    case FRT_TYP_IRONBOX2M_F:
    case FRT_TYP_IRONBOX2M_G:    case FRT_TYP_IRONBOX2M_H:
    case FRT_TYP_IRONBOX2M_I:
    case FRT_TYP_FORKLIFT_A:    case FRT_TYP_FORKLIFT_B:
    case FRT_TYP_FORKLIFT_C:    case FRT_TYP_FORKLIFT_D:
	/* フォークリフトと２Mコンテナには貼らない */
	s_flag = HZX_SEG_NO_BULLET | HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_C4 |
	    HZX_SEG_NO_SPRAY ;
	break ;
    default:
	s_flag = HZX_SEG_NO_BULLETHOLE | HZX_SEG_NO_C4 | HZX_SEG_NO_SPRAY ;
    }

    hzx_id = GM_GetHzxGroupID( where ) ;
    for ( i=4 ; --i>=0 ; )
    {
	v.vx = t_pos->vx + (i&1 ? -t_size->vx :  t_size->vx) ;
	v.vy = t_pos->vy - t_size->vy - 1200.0f ;
	v.vz = t_pos->vz + (i&2 ? -t_size->vz :  t_size->vz) ;
	v.vw = 1.0f ;
#if 0
	_sceVu0ApplyMatrix( &v, &objs->world, &v ) ;
#else
	_sceVu0AddVector( &v, &v, (FVECTOR *)objs->world.m[W] ) ;
#endif
	v.vw = t_size->vy*2 + 1200.0f ;
	_sceVu0FTOI0Vector( &p1, &v ) ;

	v.vx = t_pos->vx + (i&2 ? -t_size->vx :  t_size->vx) ;
	v.vy = t_pos->vy - t_size->vy - 1200.0f ;
	v.vz = t_pos->vz - (i&1 ? -t_size->vz :  t_size->vz) ;
	v.vw = 1.0f ;
#if 0
	_sceVu0ApplyMatrix( &v, &objs->world, &v ) ;
#else
	_sceVu0AddVector( &v, &v, (FVECTOR *)objs->world.m[W] ) ;
#endif
	v.vw = t_size->vy*2 + 1200.0f ;
	_sceVu0FTOI0Vector( &p2, &v ) ;

	hide->segs[i] = HZX_AddDynamicSegment( hzx_id, &p1, &p2, s_flag ) ;
	/*レーダーに映すため*/
	hide->segs[i]->atr &= ~HZX_SEG_NO_DISP_RADAR ;

	if ( FRT_OBJ_GetTypeMasked( hide ) == FRT_TYP_WOODBOX0 )
	    hide->segs[i]->atr |= (5<<28) ;
	else
	    hide->segs[i]->atr |= (1<<28) ;
    }

    return 0 ;
}

