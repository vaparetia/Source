//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	chair_hit.c
	イスの当たり（VTR用）
	1999/02/01 M.Matsuzaki
	$Id: chair_hit.c,v 1.1.1.3 2002/11/19 11:47:03 Yoshizawa1 Exp $
	
*/

/* -------------------------------------------------------------- */
/* include */
#define MATSUZAKI_SOURCE

#include "../../skoba/weapon_old/matsu.h"

/*----------------------------------------------------------------*/
/* define */
#define OBJECT_NUMMAX	8					/* ソファーの数最大値 */
//#define BODY_FLG		(DG_FLAG_TEXT|\
//						 DG_FLAG_PAINT|DG_FLAG_FINISHCALC)
#define BODY_FLG		(DG_FLAG_ONEPIECE)

#define HITPARTS_NUM	4

#define M_PI2			(M_PI*2.0f)

//#define TERGET_VIEW
//#define PUTCHAIROBJ

/*----------------------------------------------------------------*/
/* struct */

typedef struct	_unit_w{
	struct _work	*work;
#ifdef PUTCHAIROBJ
	DG_OBJS			*objs;
	FMATRIX			lights[2];
#endif
	short			roty;
	FVECTOR			pos;
	int 			id;
	FMATRIX			world;
	TARGET   		target[HITPARTS_NUM];
	POWER_TARGET 	power[HITPARTS_NUM];
}UNIT_W;

typedef struct	_work{
	GV_ACT_EX		actor;
	UNIT_W		*unit;
	int 		num;
	int			name;
	int		 	where;
}Work;

typedef	struct	{
	SVECTOR	rotate;
	FVECTOR	trans;
	FVECTOR	size;
} TARGET_DATA ;

static TARGET_DATA tr_data[HITPARTS_NUM]={
	/* 下（赤） */
	{{-1100,0,2048,0},							/* r */
	 {0.0f,280.0f,80.0f,0.0f},					/* t */
	 {260.0f,240.0f,100.0f,1.0f}},				/* s */
	/* 背もたれ（緑） */
	{{-100,0,0,0},								/* r */
	 {0.0f,486.0f,-275.0f,0.0f},				/* t */
	 {285.0f,320.0f,80.0f,1.0f}},				/* s */
	/* 向かって右の肘掛け（黄） */
	{{-60,-990,-950,0}, 						/* r */
	 {300.0f,375.0f,30.0f,0.0f},				/* t */
	 {235.0f,270.0f,60.0f,1.0f}},				/* s */
	/* 左の肘掛け（青） */
	{{-60,990,950,0},							/* r */
	 {-300.0f,375.0f,30.0f,0.0f},				/* t */
	 {235.0f,270.0f,60.0f,1.0f}}				/* s */
};

/*----------------------------------------------------------------*/
/* functions */
#if 0
static float Mz_itof(int);
static float Mz_itof(int i){
	int tmp;
	
	tmp = ( i & 0x0800 ) ? ( i | 0xfffff000 ) : ( i & 0xfff );
	
	return (((float)tmp * (float)M_PI / 2048.0F)) ;
}
#endif

static void GetDirection(FVECTOR 	*pvec, 	TARGET *off, TARGET *def,
						 UNIT_W	*un,	int no){
	FMATRIX	*tgmat=&un->target[no].world;
	FVECTOR	fvec,*size=&tr_data[no].size;
	
	fvec=*size;
	fvec.vx = 0.0f;
	fvec.vy *= -0.5f;
	fvec.vz *= -0.5f;
	fvec.vw=1.0f;
	
	_sceVu0ApplyMatrix(&fvec,tgmat,&fvec);
	_sceVu0SubVector(pvec,&off->hit,&def->center);
	_sceVu0SubVector(pvec,pvec,&fvec);
	
}

/************************************************************
  TargetCallBack()
  解説：ターゲットコールバック関数
  引数：オフェンス側p, ディフェンス側p,ドア用ワークp
  返値：なし
  ***********************************************************/
static void TargetCallBack( TARGET *off, TARGET *def, void *p ){
	UNIT_W	*un = (UNIT_W *)p;
	Work	*work = un->work;
	FMATRIX	mat;
	FVECTOR	fvec,ansvec;
	int 	i;
	
	if ( def->damaged & TARGET_POWER ) {
		for(i=0;i<HITPARTS_NUM;i++){
			if(def==&un->target[i]){
				GetDirection(&fvec,off,def,un,i);
				{
					extern void *NewMzCotton(int,int,FVECTOR*,FVECTOR*);
					
					_sceVu0TransposeMatrix(&mat,&un->target[i].world);
					_sceVu0ApplyMatrix(&ansvec,&mat,&fvec);
					
					if(ansvec.vz>0.0f)
						NewMzCotton(work->name,work->where,&off->hit,&fvec);
				}
				break;
			}
		}
		GM_ClearTargetDamage(def) ;
    }
}/*** TargetCallBack fin ***/


/*----------------------------------------------------------------*/

/************************************************************
  Act()
  解説：メイン
  引数：Workポインタ
  返値：無し
  ***********************************************************/
static	void	Act( Work *work ){

#ifdef PUTCHAIROBJ
	int 	i;
	UNIT_W	*un=work->unit;
	
	for(i=0;i<work->num;i++,un++){
		DG_GetLightMatrix( &un->pos, un->lights );
	}
#endif
	
}/*** Act() fin ***/


/************************************************************
  Die()
  解説：資源の解放
  引数：Workポインタ
  返値：無し
  ***********************************************************/
static void Die( Work *work ){

#ifdef PUTCHAIROBJ
	int 	i,j;
	UNIT_W	*un=work->unit;
	for(i=0;i<work->num;i++,un++){
		DG_FreeObjs( un->objs );
		for(j=0;j<HITPARTS_NUM;j++){
			GM_FreeTarget( &un->target[j] );
		}
	}
#endif
	
	if(work->unit)
		GV_Free(work->unit);
}/*** Die() fin ***/


/************************************************************
  GetResources()
  解説：資源の確保
  引数：Workポインタ
  返値：無し
  ***********************************************************/
static void ObjectInit(UNIT_W *un){
	int	i;
	Work			*work=un->work;
	FMATRIX			*pwmat,tmpmat;
	TARGET			*tr=un->target;
	POWER_TARGET 	*pw=un->power;
	TARGET_DATA 	*dt=tr_data;
	SVECTOR			svec=DG_ZeroSVector;
	
#ifdef PUTCHAIROBJ
	DG_DEF 			*def;
	{
		def = (DG_DEF*)GV_GetCache(GV_CacheID(4614086 /*"chair00"*/,'k'));
		un->objs = DG_MakeObjs( def, BODY_FLG, 0 );
		DG_QueueObjs(un->objs);
		DG_SetLightMatrix( un->objs, un->lights );
		DG_VisibleObjs(un->objs);
	}
#endif
	
	pwmat=&un->world;
	svec.vy=un->roty;
	DG_SetPos2( &un->pos, &svec );
	DG_GetPos( pwmat );
	
#ifdef PUTCHAIROBJ
	DG_GetPos( &un->objs->world );
	//_sceVu0CopyMatrix(&un->objs->world,pwmat);
#endif
	
	/* ターゲットのセット */
	for(i=0;i<HITPARTS_NUM;i++,dt++,tr++,pw++){
		GM_SetTarget(tr,TARGET_DEFENSE|TARGET_ROTATE,
					 work->where,BOTH_SIDE,&dt->size,&DG_ZeroVector);
		GM_SetPowerTarget(tr,pw,POWER_CONST,
						  1,0,0,&DG_ZeroVector);
		GM_SetTargetCallBack(tr,(void*)TargetCallBack,(void*)un);
		GM_PutTarget(tr);
#if 0
		_sceVu0RotMatrix(&tmpmat,&DG_UnitMatrix,&dt->rotate);
		_sceVu0TransMatrix(&tmpmat,&tmpmat,&dt->trans);
#else
		DG_SetPos2( &dt->trans , &dt->rotate );
		DG_GetPos( &tmpmat );
#endif
		_sceVu0MulMatrix(&tmpmat,pwmat,&tmpmat);
		GM_MoveTarget2(tr,&tmpmat);
		
#ifdef TERGET_VIEW
		{
			u_char	r,g,b,c=(u_char)(i+1);
			extern void	*NewTargetView(TARGET *targ,
									   u_char r,u_char g,u_char b);
			r=(c)%2;
			g=(c>>1)%2;
			b=(c>>2)%2;
			NewTargetView(tr,r*0xff,g*0xff,b*0xff);
		}
#endif
	}
	
}

static int GetResources( Work *work , int name, int where ){
	FVECTOR			pos[OBJECT_NUMMAX],*p;
	int				i;
	UNIT_W			*un;
	
	work->where = where;
	work->name	= name;
	
	if ( GCL_GetOption( 'p' ) != NULL ) {
		p=pos;
		for(i=0;i<OBJECT_NUMMAX;i++,p++){
			if( GCL_NextStr() != NULL ){
				p->vx   = (float)GCL_GetNextInt() ;
				p->vy   = (float)GCL_GetNextInt() ;
				p->vz   = (float)GCL_GetNextInt() ;
				p->vw	= 1.0f;
			}else
				break;
		}
	}else
		return -1;
	if(i<=0)return -1;
	
	work->num=i;
	work->unit=(UNIT_W*)GV_Malloc(sizeof(UNIT_W)*i);
	
	if ( GCL_GetOption( 'r' ) != NULL ) {
		un=work->unit;
		for(i=0;i<work->num;i++,un++){
			if( GCL_NextStr() != NULL )
				un->roty = GCL_GetNextInt();
			else
				un->roty = 0;
		}
	}
	
	un = work->unit;
	for(i = 0; i<work->num ; i++, un++){
		un->id = i;
		un->pos = pos[i];
		un->work = work;
		ObjectInit(un);
	}
	
	return 0 ;
}/*** GetResources() fin ***/


/************************************************************
  NewMzChair()
  解説：
  引数：無し
  返値：無し
  ***********************************************************/
void *NewMzChair(  int name, int where  )
{
	Work		*work ;
	
	OPERATOR() ;
	
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work , name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	
	return (void *)work ;
}/*** NewMzChair fin ***/


/************************************************************
  
  解説：
  引数：
  返値：
  ***********************************************************/
/***  fin ***/
