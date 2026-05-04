//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/********************************************************************************/
/*	har_seeffect.c								*/
/*	ハリアエフェクト							*/
/*	2001/04/19 H.Satoyoshi							*/
/*	$Id: har_demoeffect.c,v 1.1.1.3 2002/11/19 11:48:12 Yoshizawa1 Exp $		*/
/********************************************************************************/

/********************************************************************************/
/*	include files								*/
/********************************************************************************/
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <float.h>

#include "gameheader.h"
#include "libutl.h"


//********ハリアワーク
typedef struct {
    GV_ACT_EX		actor ;
    OBJECT*		body;

    // *********排気煙
    ALIGN16_PRE FMATRIX ALIGN16_POST	nozel[2] ;

    //ライト用ワーク
    ALIGN16_PRE FVECTOR ALIGN16_POST	body_light_pos1;
    ALIGN16_PRE FVECTOR ALIGN16_POST	body_light_pos2;
    //筋雲エフェクト
    ALIGN16_PRE FVECTOR ALIGN16_POST	wing_edge1;
    ALIGN16_PRE FVECTOR ALIGN16_POST	wing_edge2;

    float		wsmoke_width;
    int			wing_smoke_flg;	//雲引きフラグ
    u_short		n_msg;
    GV_MSG		*msg;
    int			name;
}Har_demo_Work;

extern void *NewHarrierWingSmoke( FVECTOR *pos, float *p_width, int *flg );

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void Die							*/
/*	引数:	Work	*work							*/
/*	説明:			      						*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void Die(Har_demo_Work *work){

}

/*+++++++++++++++++++++++++++++++++<local function>+++++++++++++++++++++++++++++*/
/*	名前:	void	CheckMessage						*/
/*	引数:	Work *work   						       	*/
/*	説明:	メッセージを受信する			       			*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static	void	CheckMessage( Har_demo_Work *work )
{
    GV_MSG	*msg ;
    int n_msg, code  ;
    n_msg = work->n_msg ;
    msg = work->msg ;
    
    while ( n_msg-- > 0 ) {
	code = msg->message[ 0 ] ;
	switch( code ) {
	    
	case 1:
	    work->wing_smoke_flg = 1;
	    break ;
	    
	case 2:
	    work->wing_smoke_flg = 0;
	    break ;
	    
	}
	
	msg++ ;
    }
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	void move_effect						*/
/*	引数:	Work	*work							*/
/*	説明:	エフェクトの位置を動かす					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static void act_effect(Har_demo_Work *work){

    work->n_msg = GV_ReceiveMessage( work->name, &( work->msg ) );
    CheckMessage( work );

    {
	SVECTOR	rot	= {2048, -70, 0, 0};
	FVECTOR	shift	= {800.0f, 0.0f, 320.0f, 0.0f};
	
	//後ノズル左
	DG_SetPos( &work->body->objs->objs[1].world ) ;
	DG_RotatePos( &rot ) ;
	DG_MovePos( &shift );
	DG_GetPos( &work->nozel[0]  ) ;
	
	//後ノズル右
	rot.vy = rot.vy*(-1);
	shift.vx = shift.vx * (-1.0f); 
	DG_SetPos( &work->body->objs->objs[1].world ) ;
	DG_RotatePos( &rot ) ;
	DG_MovePos( &shift );
	DG_GetPos( &work->nozel[1]  ) ;
    }

    //筋雲
    {
	FVECTOR	shift1	= {4477.0f, -157.70f, -1811.76f, 0.0f};
	FVECTOR	shift2	= {-4477.0f, -157.70f, -1811.76f, 0.0f};
	FMATRIX	tmpmat;
	
	DG_SetPos( &work->body->objs->objs[0].world ) ;
	DG_MovePos( &shift1 );
	DG_GetPos( &tmpmat ) ;
	work->wing_edge1 =  *((FVECTOR*)tmpmat.m[3]);
	
	DG_SetPos( &work->body->objs->objs[0].world ) ;
	DG_MovePos( &shift2 );
	DG_GetPos( &tmpmat ) ;
	work->wing_edge2 =  *((FVECTOR*)tmpmat.m[3]);

	work->wsmoke_width = 1000.0f;	// MAX 2000位
    }
    
    //ライト
    {
	FVECTOR	shift1	= {170.0f, 1130.0f, -90.0f, 0.0f};
	FVECTOR	shift2	= {0.0f, -550.0f, -1700.0f, 0.0f};
	FMATRIX	tmpmat;
	
	DG_SetPos( &work->body->objs->objs[0].world ) ;
	DG_MovePos( &shift1 );
	DG_GetPos( &tmpmat ) ;
	work->body_light_pos1 =  *((FVECTOR*)tmpmat.m[3]);
	
	DG_SetPos( &work->body->objs->objs[0].world ) ;
	DG_MovePos( &shift2 );
	DG_GetPos( &tmpmat ) ;
	work->body_light_pos2 =  *((FVECTOR*)tmpmat.m[3]);
    }
}


/*++++++++++++++++++++++++++++++<local function>++++++++++++++++++++++++++++++++*/
/*	名前:	int GetResources						*/
/*	引数:	Work	*work							*/
/*	    :	int	name							*/
/*	    :	int	where							*/
/*	説明:	ワークの確保＆イニシャライズ					*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static int GetResources(Har_demo_Work *work, OBJECT *objs, int name){

    work->body = objs;
    work->name = name;

    {	//	*****排気の熱でゆらゆら*****
	extern void *NewSmokeBlurEffect( FMATRIX *world, 
					 int start_speed, int end_speed,  int start_size, int end_size,
					 int spot_size,   int spot_angle, int n_prims,    int interval,
					 int color, int flag );
	//後ノズル左
	GV_SetActorChild( work,
			  NewSmokeBlurEffect( &work->nozel[0],
					      230, 200, 800, 1000,
					      200, 110, 30, 1,
					      0x80848280, 0 ));
	//後ノズル右
	GV_SetActorChild( work,
			  NewSmokeBlurEffect( &work->nozel[1],
					      230, 200, 800, 1000,
					      200, 110, 30, 1,
					      0x80848280, 0 ));
    }
    
    {	//	******動作ライト*****
	// 2001.7/29 S.Okajima

	extern void *NewHarrierLight( FVECTOR *pos, int rgba );
	GV_SetActorChild( work, NewHarrierLight( &work->body_light_pos1, 0xd456409c )) ;
	GV_SetActorChild( work, NewHarrierLight( &work->body_light_pos2, 0xd456409c )) ;

    }
    
    
    {	//	******筋雲*****
	GV_SetActorChild( work,
			  NewHarrierWingSmoke( &work->wing_edge1, &work->wsmoke_width, &work->wing_smoke_flg) );
	
	GV_SetActorChild( work,
			  NewHarrierWingSmoke( &work->wing_edge2, &work->wsmoke_width, &work->wing_smoke_flg) );
    }
    return 1;
}


/*******************************<Global function>********************************/
/*	名前:	void *NewHarrier						*/
/*	引数:	int	name							*/
/*	    :	int	where							*/
/*	説明:	初期化部メイン	      						*/
/********************************************************************************/
void *NewDemoHarEffect( int name, OBJECT *objs)
{
    Har_demo_Work *work ;
    work = (Har_demo_Work *)GV_CreateActor( GV_ACTOR_EFFECT, GV_CLASS_CHARA, 
				   sizeof( Har_demo_Work ), 0x20 ) ;
    if(work!=NULL) {
	GV_SetActor(&(work->actor),act_effect,Die) ;
	GV_ActorEX (&work->actor ) ;
	if(!GetResources( work, objs, name)){
	    GV_DestroyActor(work) ;
	    return NULL ;
	}
    }
    return (void *)work ;
}
