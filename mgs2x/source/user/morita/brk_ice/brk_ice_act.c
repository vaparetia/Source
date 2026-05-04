//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   brk_ice_act.c
   アイスクーラー アクト

   1999/12/13 T. Morita
   $Id: brk_ice_act.c,v 1.1.1.3 2002/11/19 11:45:32 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "brk_icebox.h"

#include "../brk_utl/brk_utl.x"
#include "../../shibata/util/ts_util.h"

//static int sound_wait = 2;

/***

  物体移動関数

  ***/
/* 当たりを見て移動 */
static int MoveObject( HZD_BOX *hzd, FMATRIX *m, MOVE *p, DG_OBJS *objs, FVECTOR *size, float reg, FVECTOR *bounce )
{
    int flag = 0 ;

	p->rot_x = (p->rot_x + p->rot_vx)&4095 ;
	p->rot_y = (p->rot_y + p->rot_vy)&4095 ;
	p->rot_x = p->rot_x>2048 ? p->rot_x-4096 : p->rot_x ;
	p->rot_y = p->rot_y>2048 ? p->rot_y-4096 : p->rot_y ;
    RotateMatrixXY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    if ( objs )
	BRK_UTL_SizeOfBound( objs, m, size ) ;

    p->pos_v.vy -= BRK_GRAVITY ;
    switch( BRK_CheckHazard( hzd, &p->pos, &p->pos_v, bounce, size ) )
    {
    case 3:
    case 1:
        _sceVu0ScaleVector( &p->pos_v, &p->pos_v, reg ) ;
        flag = (int)(p->pos_v.vy/BRK_GRAVITY)+2 ;

	if ( objs )
	{
	    int d ;
	    d = BRK_UTL_PutCenterHazard( hzd, &p->pos, &p->pos_v,
					 1500.0f, 25.0f ) ;
#if 0
	    printf( "%d Flag%x cen(%f %f %f)\n", d, BRK_HZD_Hazard->flag, 
		    BRK_HZD_Hazard->world.m[W][X],
		    BRK_HZD_Hazard->world.m[W][Y],
		    BRK_HZD_Hazard->world.m[W][Z] ) ;
#endif
	    if ( d )
	    {
		p->pos_v.vy = 30.0f ;
		p->rot_vx = irnd()&10 ? 200 : -200 ;
		flag++ ;
	    }
	}

        if ( flag == 2 )
            if ( !(int)p->pos_v.vx && !(int)p->pos_v.vz )
	    {
#if 0
		if ( BRK_UTL_PutCenterHazard( hzd, &p->pos, &p->pos_v, 1000.0f, 25.0f ) )
		    _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
		else
#endif
		    flag = 1 ;
	    }
        if ( flag > 4 )
            p->rot_vx = irnd()&10 ? 200 : -200 ;
        else
            p->rot_vx = ((!(p->rot_x/ 512) ? 0   :
			  !(p->rot_x/1536) ? 1024: 2048)*(p->rot_x>0 ? 1 : -1) - p->rot_x) / 16 ;
        break ;
    case 0:
        _sceVu0AddVector( &p->pos, &p->pos, &p->pos_v ) ;
		
    }
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/* 0°or 180°に倒れ込み */
static int FallDownObject( HZD_BOX *hzd, FMATRIX *m, MOVE *p, DG_OBJS *objs, FVECTOR *size )
{
    int r, flag ;

    r = p->rot_x + (p->rot_vx += ( (p->rot_x/512)&1 ? 10 : -10) * (p->rot_x>0 ? 1 : -1) ) ;
    if ( (r > 0 && p->rot_x <= 0) || (r < 0 && p->rot_x >= 0) )
        flag = 0, r =    0 ;
    else if ( (!(p->rot_x/1024) && r/1024) || (p->rot_x/1024 && !(r/1024)) )
        flag = 0, r = p->rot_x>0 ? 1024 : -1024 ;
    else if ( (!(p->rot_x/2048) && r/2048) || (p->rot_x/2048 && !(r/2048)) )
        flag = 0, r = 2048 ;
    else
        flag = 2 ;

    p->rot_x = r ;
    p->rot_y += p->rot_vy = p->rot_vy * (BRK_ROT_R-1) / BRK_ROT_R ;
    p->pos_v.vy = -BRK_GRAVITY ;
    RotateMatrixXY( m, &DG_UnitMatrix, p->rot_x, p->rot_y ) ;
    if ( objs )
	BRK_UTL_SizeOfBound( objs, m, size ) ;
    if ( !BRK_CheckHazard( hzd, &p->pos, &p->pos_v, &DG_ZeroVector, size ) )
	p->pos_v.vy = 0, flag = 1 ;
    TransMatrix( m, &p->pos ) ;

    return flag ;
}

/* パーティクルが当たりを見て移動し,止まるとフェードアウトを始める */
#if 0
static int MoveParticle( HZD_BOX *hzd, DG_PRIM2 *p )
{
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    static FVECTOR size   = { 1.0f, 1.0f, 1.0f, 0 } ;
    FVECTOR pos_v, *prv_p, *nxt_p ;
    DG_PRIM2_UVRGB *prv_u, *nxt_u ;
    int     i, flag = 1 ;

    prv_p = p->pos[p->buffer_clock] ;
    prv_u = p->uvrgb[p->buffer_clock] ;
    DG_SwitchBuffPrim2( p ) ;
    nxt_p = p->pos[p->buffer_clock] ;
    nxt_u = p->uvrgb[p->buffer_clock] ;

    for ( i=BRK_N_DUST ; --i>=0 ; prv_p+=2, nxt_p+=2, prv_u+=2, nxt_u+=2 )
	if ( prv_u->a || nxt_u->a )
	{
	    if ( prv_u->a == nxt_u->a ) /* アルファーが同じ場合は動いている */
	    {
		_sceVu0SubVector( &pos_v, prv_p, (prv_p+1) ) ;
		_sceVu0CopyVector( nxt_p+1, prv_p ) ;
		_sceVu0CopyVector( nxt_p  , prv_p ) ;

		pos_v.vy -= BRK_GRAVITY ;

		switch( BRK_CheckHazard( hzd, nxt_p, &pos_v, &bounce, &size ) )
		{
		case 3:
		case 1:
		    _sceVu0ScaleVector( &pos_v, &pos_v, BRK_VEL_R ) ;
		    if ( !(int)(pos_v.vy/BRK_GRAVITY) && !(int)pos_v.vx && !(int)pos_v.vz )
		    {
			_sceVu0CopyVector( nxt_p+1, prv_p+1 ) ;/* 点を固定 */
			_sceVu0CopyVector( nxt_p  , prv_p   ) ;
			nxt_u->a = prv_u->a - 4 ;/* フェードアウト開始 */
			break ;
		    }
		case 2:
		    _sceVu0CopyVector( nxt_p+1, nxt_p ) ;/* ぶつかった位置の更新 */
		case 0:
		    _sceVu0AddVector( nxt_p, nxt_p, &pos_v ) ;
		}
	    }
	    else /* アルファーが異なる場合はフェードアウトしている */
		nxt_u->a = (prv_u->a > 4) ? (prv_u->a - 4) : 0 ;
	    flag = 0 ;
	}

    return flag ;
}
#endif

//add shibata
static int MoveComdl( HZD_BOX *hzd, ICEBOX *ice )
{
    int i,flag = 1,hzd_flag;
    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;
    static FVECTOR size   = { 1.0f, 1.0f, 1.0f, 0 } ;
	
    FVECTOR *dvec = ice->dvec;
    FVECTOR *center;
    DG_COMDL_POS 	*comdl_pos;
    FVECTOR fvtemp;//,scale = { 0.25f, 0.25f, 0.25f, 0.0f };
    SVECTOR *rot = ice->rot;

	if(!ice->frg_objs){ printf("NULL comdl\n"); return 1; }
	comdl_pos = ice->frg_objs->pos;
	ice->frg_objs->flag &= ~(DG_COMDL_INVISIBLE0);

//	printf("center vx %f: vy %f: vz %f\n",center->vx,center->vy,center->vz);
	for( i = 0; i < BRK_N_DUST; i++ ){
		if(comdl_pos->color.vw){
			center = (FVECTOR*)comdl_pos->world.m[3];
//			printf("dvec vx %f: vy %f: vz %f\n",dvec->vx,dvec->vy,dvec->vz);
//			printf("pre center vx %f: vy %f: vz %f\n",center->vx,center->vy,center->vz);

			dvec->vy -= BRK_GRAVITY;
			hzd_flag = BRK_CheckHazard( hzd, center, dvec, &bounce, &size );
			
			if(hzd_flag&1){//床
				dvec->vx *= 0.95f;
				dvec->vy *= -0.95f;
				dvec->vz *= 0.95f;
				comdl_pos->color.vw = (comdl_pos->color.vw > 2)?comdl_pos->color.vw-2:0;
			}
			if(hzd_flag&2){//壁
				dvec->vx *= -0.95f;
				dvec->vy *= 0.95f;
				dvec->vz *= -0.95f;
				comdl_pos->color.vw = (comdl_pos->color.vw > 2)?comdl_pos->color.vw-2:0;
			}
			_sceVu0AddVector( &fvtemp, dvec, center );
			
			rot->vx += irnd()%8;
			rot->vy += irnd()%8;
			rot->vz += irnd()%8;

			DG_SetPos2( &fvtemp, rot ) ;
			DG_GetPos( &comdl_pos->world );
			TS_ScaleMatrix(&comdl_pos->world,&comdl_pos->world,&ice->ice_scl);
//			printf("after center vx %f: vy %f: vz %f\n",center->vx,center->vy,center->vz);

			if(comdl_pos->color.vw!=0x40)
				comdl_pos->color.vw = (comdl_pos->color.vw > 1)?comdl_pos->color.vw-1:0;
			
			flag = 0;
		}
		comdl_pos++;
		dvec++; rot++;
	}
//	printf("comdl\n");

	return flag;
}

/***

  氷のアクト関数

  ***/
#define SUB_Y_SCALE (0.9995f)//(1.0f/3000.0f)	//１.0f/解ける時間
#define SUB_X_SCALE (0.9995f)//(1.0f/6000.0f)	//１.0f/解ける時間
#define SUB_Z_SCALE (0.9995f)//(1.0f/6000.0f)	//１.0f/解ける時間

void BRK_ICE_ActIce( Work *work )
{
    int    i, flag = 0 ;
    static FVECTOR bounce = { BRK_BOUNCE_WALL, 1.6f, BRK_BOUNCE_WALL, 0 } ;
    FVECTOR scale = {SUB_Z_SCALE,SUB_Y_SCALE,SUB_Z_SCALE,0.0f};
    ICEBOX *p ;

    for ( i=work->n_ice, p=work->ice ; --i>=0 ; p++ )
    {
        switch ( p->flag )
	{
	case  0:/* 全く処理しない(床に落ち着いている) */
#if 1
	    TS_ScaleMatrix(&p->objs->world,&p->objs->world,&scale);	
	    if(fpu_VectorLength2((FVECTOR*)p->objs->world.m[0]) < 0.02f){
		p->flag = -1 ;
		DG_InvisibleObjs( p->objs );
		GM_FreeTarget( &p->target ) ;
		//printf("melted\n");
	    }
	    flag = 1 ;
	    break;
#endif
	case -1:/* 全く処理しない(起動してないないか,割れてなくなった) */
	    continue ;
	case -2:/* 氷を割った処理 */
#if 0
            if ( MoveParticle( work->hzd, p->prim ) )
	    {
		GM_FreePrim2( p->prim ) ;
		p->prim = NULL ;
		p->flag = -1 ;
	    }
#else
	    /*add shibata*/
		  if( MoveComdl( work->hzd, p ) ){
		      DG_DequeueComdlObjs( p->frg_objs );
		      DG_FreeComdl( p->frg_objs );
		      p->frg_objs = NULL;
		      p->flag = -1 ;
		      //printf("kill\n");
		  }
#endif
	    flag = 1 ;
	    break ;

	case 1:/* 氷が落ちている処理 */
            switch ( MoveObject( work->hzd, &p->objs->world, &p->mov, NULL,
				 &p->target.size, 0.97f, &bounce ) )
	    {
	    case 0:
		p->sound_flag += 0x00010000 ;
		break ;
	    case 1:
                p->flag = 2 ;
	    default:
		if ( ((p->sound_flag)&0xffff) < 8 )
		{
		    static int Ice_Sound[] = { SD_A_ICEDRP01,SD_A_ICEDRP02,SD_A_ICEDRP03,SD_A_ICEDRP04 };
		    if(((p->sound_flag)>>16) > 8 )
			GM_SeSetMode( Ice_Sound[irnd()%4], &p->mov.pos, GM_SEMODE_BOMB ) ;
		    p->sound_flag &= 0xffff ;
		}
		break ;
	    }
	    DG_GetLightMatrixFix( &p->mov.pos, p->lights ) ;
	    GM_MoveTargetMap( &p->target, &p->mov.pos, work->where ) ;
	    flag = 1 ;
            break ;

	case 2:/* 床に落ち着く処理 */
            p->flag = FallDownObject( work->hzd, &p->objs->world, &p->mov, NULL, &p->target.size ) ;
	    GM_MoveTargetMap( &p->target, &p->mov.pos, work->where ) ;
	    flag = 1 ;
        }
    }
    if ( !flag )
	work->n_ice |= BRK_ICE_INACTIVE ;
}

/*氷の起動関数*/
void BRK_ICE_StartActIce( Work *work, int i, BOX *b )
{
    ICEBOX *p ;

    work->n_ice &= ~BRK_ICE_INACTIVE ; /* 氷のアクトを有効にする */
    for ( p=work->ice + b->ice_fount ; --i>=0 ; p++ )
    {
	if ( work->n_ice <= ++b->ice_fount )
	{
	    b->flag = 1 ;
	    break ;
	}

	/* 氷の初速を設定 */
	_sceVu0ScaleVector( &p->mov.pos_v, (FVECTOR *)&b->objs->world.m[Y], 50.0f ) ;
	p->mov.pos_v.vx += frnd()*10.0f;
	p->mov.pos_v.vy += frnd()*10.0f;
	p->mov.pos_v.vz += frnd()*10.0f;

	/* 氷の位置を設定 */
	RotateMatrixXY( &p->objs->world, &DG_UnitMatrix, p->mov.rot_x, p->mov.rot_y ) ;
	TransMatrix( &p->objs->world, &p->mov.pos ) ;
	_sceVu0MulMatrix( &p->objs->world, &b->objs->world, &p->objs->world ) ;
	_sceVu0CopyVector( &p->mov.pos, (FVECTOR *)&p->objs->world.m[W] ) ;
	DG_VisibleObjs( p->objs ) ;

	/* ターゲット設定 */
	BRK_ICE_InitTarget( &p->target, &p->power, work->where, 0,
			    &work->i_def->ux, &work->i_def->lx, &p->mov.pos,
			    BRK_ICE_TargetCallBack, p ) ;
	p->flag = 1 ;/*氷が動く*/

#if DEBUG_MODE
#if 0
	NewNameNum( p->objs, i ) ;
#endif
#endif

    }
}


#define AXIS(_mtx,_axis) (FVECTOR *)((_mtx).m[_axis])

/***

  箱のアクト関数

  ***/
void BRK_ICE_ActBox( Work *work )
{
    BOX    *b = &work->box ;
    FVECTOR size ;
    int     move ;
    int pan;
    float bp_angle;

    static FVECTOR bounce = { BRK_BOUNCE_WALL, BRK_BOUNCE, BRK_BOUNCE_WALL, 0 } ;

    switch ( b->flag )
    {
    case 3:
	//b->flag = 1 ; break ;/*テストのため氷を出してません*/

	/* 氷を出す */
	BRK_ICE_StartActIce( work, 3, b ) ;
	/* 水を出す */
	BRK_ICE_SprayPrimSet( work, AXIS(b->objs->world,W), AXIS(b->objs->world,Y) ) ;

    case 1:
	move = MoveObject( b->hzd, &b->objs->world, &b->mov, b->objs, &size, BRK_VEL_R, &bounce ) ;
	if ( move )
	{
	    if ( move == 1 )
		b->flag = 2 ;
	    if ( ((b->snd_flag)&0xffff) < 4 || move >= 4 )
	    {
		int len ;

		if((b->snd_flag>>16) > 20){
		    len = GM_MAX_VOL ;
		}else{
		    len = (GM_MAX_VOL>>2) + ((b->snd_flag>>16) * (GM_MAX_VOL>>2) * 3 )/20 ;
		    b->snd_flag++ ;
		}
      pan = GM_SeGetPan( &b->mov.pos, GM_SEMODE_BOMB, &bp_angle );
		GM_SeSet3D( pan, len, SD_A_ICECAN02, bp_angle ) ;
		GM_SeSetMode( SD_A_ICECAN02, &b->mov.pos, GM_SEMODE_BOMB ) ;
		b->snd_flag &= 0xffff ;
	    }
	}
	else
	    b->snd_flag += 0x00010000;

	DG_GetLightMatrixFix( &b->mov.pos, b->lights ) ;
	break ;

    case 2:
	b->flag = FallDownObject( b->hzd, &b->objs->world, &b->mov, b->objs, &size ) ;
	break ;
    }

    if ( --b->non_dmg<=0 )
    {
	b->non_dmg = 0 ; 
	b->flag |= !b->flag ? BRK_ICE_INACTIVE : 0 ;
    }
    GM_MoveTarget2Map( &b->target, &b->objs->world, work->where ) ;
}

int BRK_ICE_ActSpray( Work *work )
{
	int				i,j,clock,flag = 1;
	DG_PRIM2		*prim = work->spray;
	FVECTOR			*dvec = work->sp_dvec;
//	FVECTOR			*mpos = work->sp_mpos;
	FVECTOR			*lpos = work->sp_lpos;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
//	FMATRIX			world;
	
	DG_SwitchBuffPrim2( prim ) ;
	clock = prim->buffer_clock;
	pos = prim->pos[clock];
	uvrgb = prim->uvrgb[clock];
	
	for( i = 0; i < BRK_N_SPRAY; i++ ){
//		_sceVu0AddVector( mpos, mpos, &dvec[3] );
//		TS_MakeMatrix(&world,&dvec[3],mpos);
		//AN_Test_Eye2(mpos,2);
//		_sceVu0ScaleVector( &dvec[3],&dvec[3],0.95f );
		
		dvec[3].vy -= 4.0f;
		for( j = 0; j < 4; j++ ){
			_sceVu0AddVector( lpos, lpos, dvec );
			_sceVu0ScaleVector( dvec,dvec,0.95f );
			dvec->vy -= 3.0f;
			DG_COPY_VEC( pos, lpos );
			
//			AN_Test_Eye2(pos,2);
			
			if(uvrgb->a){
				uvrgb->a-=2;
				flag = 0;
			}
			uvrgb++;lpos++;
			dvec++; pos++;
		}
//			AN_Test_Eye2(&pos[j],2);
	}
//	printf("spray act\n");
	return flag;
}

