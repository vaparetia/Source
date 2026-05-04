//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_obj_clb.c 
   フォーチュン戦 ライデン隠れ専用プットオブジェ

   2000/12/14 T.Morita
   $Id: fort_obj_clb.c,v 1.1.1.3 2002/11/19 11:46:15 Yoshizawa1 Exp $
*/
#include "fort_obj.h"


static void CallOffenceWhenThrough( TARGET *off, TARGET *def )
{
    off->hit = def->hit ;
    off->damaged |= off->class & TARGET_CHECK_CLASS ;
    if ( off->callback != NULL )
	( *off->callback )( off, def, off->work ) ;
}


static inline void Hit( HIDE *h,
					    FVECTOR *offs, FVECTOR *frc , FVECTOR *org,
					    FVECTOR **hit, FVECTOR *hit_point,
					    float *dist )
{
	float d ;
	FVECTOR sub_point ;

	/* 跳弾を出す */
	_sceVu0SubVector( &sub_point, offs, org ) ;
	d = _sceVu0InnerProduct( &sub_point, &sub_point ) ;
	if ( *dist > d ) {
		_sceVu0ApplyMatrix( hit_point, &h->objs->world, offs ) ;
		*hit = hit_point ;
		*dist = d ;
	}
	_sceVu0CopyVector( offs, org ) ;
}

static void CallSparks( TARGET *off, TARGET *def, HIDE *h )
{
    extern void *NewSpark( FMATRIX *world ) ;
    FMATRIX  mtx ;
    FVECTOR *hit ;
    FVECTOR  hit_point ;
    FVECTOR  offs, frc, org ;
    int         j, k ;
    DG_OBJ     *obj ;
    DG_MDLPACK *m ;
    float       dist ;

    if ( FRT_OBJ_GetTypeAimFlag( h ) & ( /*FRT_AIM_TypeGasCan|
					 FRT_AIM_TypeExplosiv_L|
					 FRT_AIM_TypeExplosiv_R|
					 FRT_AIM_TypeForkLift|
					 */ FRT_AIM_TypeIronBox) )
	hit = &def->hit ;
    else {

		/*ローカル座標系での着弾位置と着弾方向を求める*/
		_sceVu0CopyVector( &frc , &off->power->force ) ;
		_sceVu0CopyVector( &offs, &def->hit ) ;
		offs.vw = 1.0f ;
		frc.vw  = 0.0f ;
		_sceVu0InversMatrix( &mtx, &h->objs->world ) ;
		_sceVu0ApplyMatrix( &frc , &mtx, &frc  ) ;
		_sceVu0ApplyMatrix( &offs, &mtx, &offs ) ;
		_sceVu0CopyVector( &org, &offs ) ;
		
		hit = NULL ;
		dist = 1000000.0f ;
		
		/*全てのポリゴンに対して当たり判定*/
		for ( obj=h->objs->objs, k=h->objs->n_models ; --k>=0 ; obj++ ) {
#ifdef KP_XBOX
			DG_VERTEX_KMSS *kmss = obj->vbuff ;
#else
			int i ;
#endif

			for ( m=obj->model->packs, j=obj->model->n_packs ; --j>=0 ; m++ ) {
#ifdef PSX2
				SVECTOR    *s, *n ;

				s = (SVECTOR *)m->verts ;
				n = (SVECTOR *)m->norms ;
				for ( i=m->n_verts ; --i>=0 ; s++, n++ ) {
					if ( !(n->pad & 0x8000) ) {
						if ( BRK_UTL_CheckLineInPolygon( &offs, &frc, s-2 ) ) {
							Hit( h,
								&offs, &frc, &org,
								&hit, &hit_point,
								&dist ) ;
						}
					}
				}
#else
				int v0, v1, v2 ;
				
				v0 = v1= v2= *(short*)m->index ;
				for ( j=0 ; j<m->n_indices ; j++, kmss++ ) {
					if ( v0!=v1 && v0!=v2 && v1!=v2 ) {
						if ( BRK_UTL_CheckLineInPolygon( &offs, &frc,
														&kmss[v0],
														&kmss[v1],
														&kmss[v2] ) ) {
							Hit( h,
								&offs, &frc, &org,
								&hit, &hit_point,
								&dist ) ;
						}
					}
					v2 = v1 ;
					v1 = v0 ;
					v0 =*((short*)m->index + j) ;
				}
#endif
			}
		}
		if ( !hit )
		  return ;
    }

    printf( "Fort : Spark\n" ) ;
    GM_CurrentMap = h->work->where ;
    UTL_VectoMat( &mtx, &off->power->force, hit, Y ) ;
    NewSpark( &mtx ) ;
}



/*

  荷物を壊す

*/
static int BreakObject( HIDE *h, int type,
					    TARGET *off, TARGET *def,
					    int destroy/* 必ず壊れるフラグ */ )
{
    TYPE *t ;

    /* グレネードの場合は すぐに壊れにいく */
    if ( off->weapon_type & WP_GRENADE || destroy )
	  h->type += FRT_OBJ_GetType( h )->break_id ;

    t = FRT_OBJ_GetType( h ) ;
    if ( t->func )
    {
	/* 着弾画面系エフェクト */
	FRT_LGT_SetMagnitude( (FVECTOR *)h->objs->world.m[W],
			      t->magnitude ) ;
	NewShakeCamera( 0,
			(int)(256.0f*t->magnitude/8000.0f),
			(int)( 20.0f*t->magnitude/8000.0f) ) ;
	_sceVu0CopyVector( &off->hit, &def->hit ) ;

	/* 弾が生きている(基本的に弾は,貫通しない) */
	if ( FRT_BulletEnable == 0 && destroy == 0 )
	    return 0 ;
	FRT_BulletEnable = 0 ;

	/* 壊れ関数を呼び出す fort_obj.h に登録してある */
	if ( (* t->func)( h, off, t->flag ) )
	{
	    /* 壊れシグナルを出す */
	    if ( h->work )
		GV_CallChildSignalFunc( h->work,
					FRT_OBJ_Destroyed,
					(int)h ) ;

	    /* 当たり関係を解放 */
	    FRT_OBJ_FreeHzxHzd( h, 1 ) ;

	    /* 壊れプロックを呼ぶ */
	    if ( h->procs[FRT_OBJ_P_DESTROY] )
		GCL_ExecProc( h->procs[FRT_OBJ_P_DESTROY], NULL ) ;
	    return 1 ;
	}
	else
	{
	    /* ２Mコンテナの時だけ呼び出す */
	    if ( (type >= FRT_TYP_IRONBOX2M_A &&
		  type <= FRT_TYP_IRONBOX2M_I ) ||
		 (type >= FRT_TYP_EXPLCAN1M_A &&
		  type <= FRT_TYP_EXPLCAN1M_C ) )
		if ( h->work )
		    GV_CallChildSignalFunc( h->work,
					    FRT_OBJ_Destroyed,
					    (int)h ) ;

	    /* 入れ替わりプロックを呼ぶ */
	    if ( h->procs[FRT_OBJ_P_CHANGED] )
		GCL_ExecProc( h->procs[FRT_OBJ_P_CHANGED], NULL ) ;
	}
    }
    return 0 ;
}



/*

  コールバック

*/
void FRT_OBJ_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    HIDE *h = (HIDE *)ptr ;
    int type ;

    if ( def->class & TARGET_THROUGH && off->class & TARGET_SEEK )
	CallOffenceWhenThrough( off, def ) ;

    if ( def->damaged & TARGET_POWER )
    {
	GM_ClearTargetDamage( def ) ;

	type = h->type & FRT_TYP_TYPEMASK ;
	if ( off->weapon_type & WP_BULLET )
	{
	    /* 跳弾エフェクトを呼ぶ */
	    if ( !((type>=FRT_TYP_FORKLIFT_A  && type<=FRT_TYP_FORKLIFT_D ) ||
		   (type>=FRT_TYP_IRONBOX2M_A && type<=FRT_TYP_IRONBOX2M_I)) )
		CallSparks( off, def, h ) ;
	}
	else if ( off->weapon_type & (WP_GRENADE|WP_NIKITA) )
	{
	    if ( BreakObject( h, type, off, def, 0 ) && h->move )
	    {
		h += h->move ;
		type = h->type & FRT_TYP_TYPEMASK ;

		if ( h->type & FRT_TYP_FLAG_UPPER_DECK )
		    if ( type != FRT_TYP_WOODBOX0_BRK &&
			 type != FRT_TYP_IRONBOX1M2_C )
		    BreakObject( h, type, off, &h->target, 1 ) ;
	    }
	}
    }
}

