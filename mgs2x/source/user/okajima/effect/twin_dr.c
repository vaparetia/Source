//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	twin_dr.c
	両開き扉
	1999/01/31 M.Matsuzaki
	$Id: twin_dr.c,v 1.1.1.3 2002/11/19 11:47:15 Yoshizawa1 Exp $
*/

/* -------------------------------------------------------------- */
/* include */
#define MATSUZAKI_SOURCE

#include "../../skoba/weapon_old/matsu.h"

extern int		GM_N_WhereList;
extern CONTROL	*GM_WhereList[];

/*----------------------------------------------------------------*/
/* define */

#define M_PI2				((float)M_PI*2.0f)
#define TWDR_HIT_HOSEI		(0.7071067811865f)
#define TWDR_HIT_R			(140.0f)
#define TWDR_HIT_R_HOSEI	(TWDR_HIT_R*TWDR_HIT_HOSEI)
#define TWDR_ENEMY_HIT_R	(380.0f)
#define TWDR_ENEMY_HIT_R_HOSEI 	(TWDR_ENEMY_HIT_R*TWDR_HIT_HOSEI)

//#define TWDR_SIMPLE_HIT_OFSZ	(120.0f)
//#define BODY_FLG		(DG_FLAG_TEXT|\
//						 DG_FLAG_PAINT|DG_FLAG_FINISHCALC)

#define BODY_FLG		(DG_FLAG_GBOUND|DG_FLAG_FINISHCALC)

#define TWDR_ARC_LIMIT	((float)M_PI*0.5f)	/* 角度制限 */
#define	TWDR_WEIGHT		(2800.0f)		/* 重さ */

#define TWDR_BUND_KEISU		(-0.5f)
//#define TWDR_SPRING		(0.4f)
#define TWDR_SPRING			(0.6f)
#define	TWDR_FRICTION		(0.97f)

/*----------------------------------------------------------------*/
/* struct */

/* 左右の扉用work */
typedef struct	tw_door_w{
	float			froty;		/* 現在角度 */
	float			pow;		/* 力	 */
	float			weight;		/* 質量 */
	float			maxarc;		/* 最大角度 */
	float			minarc;		/* 最小角度 */
	int 			id;			/* 扉	0:左 1:右 */
	DG_OBJ			*pobj;		/*  */
	struct tw_work	*work;		/*  */
	TARGET   		target_d;	/*  */
	POWER_TARGET 	power_d;	/*  */
	
	int 			count;		/*  */
}DOOR_W;

/* 全体管理用work */
typedef struct	tw_work{
	GV_ACT_EX			actor ;

	int				name ;
	int				where ;

	DG_OBJS			*objs;
	DG_DEF			*def;
	
	float			weight;		/* ドア */
	float			froty;		/* 全体回転値 */
	
	struct tw_door_w	door[2];
	
	FVECTOR			pos;
	FMATRIX			lights[2];
	
}Work;

/*----------------------------------------------------------------*/
/* functions */
static float Mz_itof(int);
static float Mz_LinePointLength(FVECTOR *,FVECTOR *, FVECTOR *);
static float Mz_LinePointCheck(FVECTOR *,FVECTOR *, FVECTOR *);

static float Mz_itof(int i){
	int j;
	j = ( i & 0x0800 ) ? ( i | 0xfffff000 ) : ( i & 0xfff );
	return ((float)j * (float)M_PI / 2048.0F) ;
}

static float Mz_LinePointLength(FVECTOR *a,FVECTOR *b, FVECTOR *c){
	FVECTOR	t1,t2;
	_sceVu0SubVector(&t1,b,a);
	_sceVu0SubVector(&t2,c,a);
	_sceVu0OuterProduct(&t2,&t1,&t2);
	_sceVu0OuterProduct(&t1,&t1,&t2);
	_sceVu0Normalize(&t1,&t1);
	_sceVu0SubVector(&t2,a,c);
	return _sceVu0InnerProduct(&t1,&t2);
}

static float Mz_LinePointCheck(FVECTOR *a,FVECTOR *b, FVECTOR *c){
	return (((b->vx-a->vx)*(c->vz-a->vz))-
			((b->vz-a->vz)*(c->vx-a->vx)));
}

/************************************************************
  TwindoorCallBack()
  解説：扉ターゲットコールバック関数
  引数：オフェンス側p, ディフェンス側p,ドア用ワークp
  返値：なし
  ***********************************************************/
static void TwindoorCallBack( TARGET *off, TARGET *def, void *p ){
	FVECTOR	vec2,vec3;
	DOOR_W	*dr;
	FMATRIX	mat;
	float	f0;
	DG_MDL	*mdl;

	dr = (DOOR_W *)p;
	mdl=dr->pobj->model;

	/* 打撃系ターゲット */
	if ( def->damaged & TARGET_POWER ){
		if ( off->power == NULL ) {
			def->damaged &= ~TARGET_POWER ;
			def->weapon_type &= ~off->weapon_type ;
			return ;
		}

		/* 回転力を適当に求める */
		_sceVu0TransposeMatrix(&mat,&dr->pobj->world);
		_sceVu0ApplyMatrix(&vec2,&mat,&off->power->force);
		/* 「てこ」の原理（軸から近いと力が減る） */
		f0 = (off->hit.vx - dr->pobj->world.m[3][0]);
		f0 = (f0 > 0.0f)?f0:-f0;
		f0 = f0*vec2.vz*0.0005f;

		if ( def->weapon_type & (WP_BULLET|WP_M92) ){
			/* 火花の方向計算 */
			vec2.vz=-vec2.vz;	vec2.vw=1.0f;
			_sceVu0Normalize( &vec2, &vec2 );
			_sceVu0ApplyMatrix(&vec2,&dr->pobj->world,&vec2);
			vec3.vx=0.0f;vec3.vy=-1.0f;vec3.vz=0.0f;
			MT_QuatSetFromAx( &vec2, &vec3, &vec2 );
			MT_QuatToMat( &mat, &vec2 );
			_sceVu0TransMatrix(&mat,&mat,&off->hit);

			{
				extern void NewSpark(FMATRIX*);
				NewSpark(&mat);
			}
			GM_SeSetMode( SD_W_RICOCH02, &off->hit, GM_SEMODE_BOMB ) ;

		}else{
			f0 *= 0.005f;
		}

		/* 加算 */
		if(dr->id){
			dr->pow += f0;
		}else{
			dr->pow -= f0;
		}

		/* clear damage */
		GM_ClearTargetDamage( def ) ;
    }
	
}/*** TwindoorCallBack fin ***/

/*----------------------------------------------------------------*/
static int CollisionObjList[]={
	HUMAN21_KOSHI,
	HUMAN21_ONAKA,
	HUMAN21_MUNE,
	//HUMAN21_MIGI_KATA,
	HUMAN21_MIGI_UDE1,
	HUMAN21_MIGI_UDE2,
	HUMAN21_MIGI_TE,
	//HUMAN21_HIDARI_KATA,
	HUMAN21_HIDARI_UDE1,
	HUMAN21_HIDARI_UDE2,
	HUMAN21_HIDARI_TE,
	//HUMAN21_KUBI,
	HUMAN21_ATAMA,
	HUMAN21_MIGI_ASHI1,
	HUMAN21_MIGI_ASHI2,
	//HUMAN21_MIGI_KAKATO,
	//HUMAN21_MIGI_TSUMASAKI,
	HUMAN21_HIDARI_ASHI1,
	HUMAN21_HIDARI_ASHI2,
	//HUMAN21_HIDARI_KAKATO,
	//HUMAN21_HIDARI_TSUMASAKI,
	-1
};

static int Collision(DOOR_W *dr,DG_MDL *mdl,int id){
	FMATRIX *pmat0,*pmat1,*pmat2;
	float	f0,f1,f2,fr,fw,max;
	FVECTOR	fvec0,fvec1,mov;
	CONTROL	*ctrl;
	//DG_OBJ	*pobj;
	int	 	j,flg,hit_flg=0;
	
	pmat0 = &dr->pobj->world;
	pmat1 = &dr->target_d.world;
#if 1
	if(id)
		MENU_Locate( 320, 32, 0 ) ;
	else
		MENU_Locate( 32, 32, 0 ) ;
#endif
	_sceVu0CopyVector(&fvec0,&pmat0->m[3][0]);
	_sceVu0CopyVector(&fvec1,&dr->target_d.center);
	fvec0.vw = fvec1.vw = 1.0f;
#if 1

//	MENU_Printf("(%d)\n",flg);
	
	if(GM_PlayerBody){
		if(GM_CheckPlayerStatus(PLAYER_GROUND)){
			pmat2=&GM_PlayerBody->objs->objs[HUMAN21_ATAMA].world;
		}else{
			pmat2=&GM_PlayerBody->objs->objs[HUMAN21_KOSHI].world;
		}
		flg=(Mz_LinePointCheck(&fvec0,&fvec1,
							   (FVECTOR*)&pmat2->m[3][0])>0.0f)?1:0;
		
		if(flg)
			max=-(M_PI2*2.0f);
		else
			max=(M_PI2*2.0f);
		
		//pobj=GM_PlayerBody->objs->objs;
		for(j=0;j<=HUMAN21_HIDARI_TSUMASAKI;j++/*,pobj++*/){
         // BP_WARNING - Even though this loop is going to 20, it will punt out 
         // beforehand because CollisionObjList ends with a -1
			if((CollisionObjList[j])<0)break;

			pmat2=&GM_PlayerBody->objs->objs[CollisionObjList[j]].world;
			_sceVu0CopyVector(&mov,&pmat2->m[3][0]);
			mov.vw=1.0f;
			f1=mov.vy-pmat0->m[3][1];
			f2=0.0f;
			if(f1<mdl->ly){
				f2=mdl->ly-f1;	/* f2は半径計算用 */
				f1=mdl->ly;		/* f1は直線の高さ */
			}else if(mdl->uy<f1){
				f2=f1-mdl->uy;
				f1=mdl->uy;
			}
			fvec0.vy=fvec1.vy=f1+pmat0->m[3][1];
			f1=Mz_LinePointLength(&fvec0,&fvec1,&mov);
			if(f1<TWDR_HIT_R){
				_sceVu0SubVector(&mov,&mov,&fvec1);
				if(bp_sqrtf((mov.vx*mov.vx+mov.vy*mov.vy+ //BP_MATH - emulate PS2 sqrtf
						  mov.vz*mov.vz-f1*f1))<=((mdl->ux-mdl->lx)*0.5f
				   +TWDR_HIT_R_HOSEI)){
					if(f2!=0.0f)
						fr=bp_sqrtf((TWDR_HIT_R*TWDR_HIT_R)-f2*f2);  //BP_MATH - emulate PS2 sqrtf
					else
						fr=TWDR_HIT_R;
					_sceVu0SubVector(&mov,&pmat2->m[3][0],&pmat0->m[3][0]);
					fw=bp_sqrtf(mov.vx*mov.vx+mov.vz*mov.vz); //BP_MATH - emulate PS2 sqrtf
					f0=atan2f(fr,fw);
					f1=Mz_LinePointLength(&fvec0,&fvec1,
										  (FVECTOR*)&pmat2->m[3][0]);
					f1=atan2f(f1,fw);
					//MENU_Printf("(%f)\n",fw);
					if(Mz_LinePointCheck(&fvec0,&fvec1,
										 (FVECTOR*)&pmat2->m[3][0])>0.0f)
						fw=-f1;
					else
						fw=+f1;
					if(flg){
						fw+=f0;
						if(fw>max)max=fw;
					}else {
						fw-=f0;
						if(fw<max)max=fw;
					}
					hit_flg=1;
				}
			}
		}
		
		if(hit_flg){
#if 0
			f0=fmod((max+((id)?(-M_PI/2):(M_PI/2))-dr->work->froty+M_PI2),
					M_PI2);
			dr->froty=f0;
			//MENU_Printf("(%f)\n",f0);
#else
			dr->froty+=max;
#endif
			dr->pow*=TWDR_BUND_KEISU;
			return 1;
		}
	}
#endif

#if 1
	/* 敵兵用 */
	for(j=1;j<GM_N_WhereList;j++){
		ctrl=GM_WhereList[j];
		fvec0.vy=fvec1.vy=ctrl->mov.vy;
		f1=Mz_LinePointLength(&fvec0,&fvec1,&ctrl->mov);
		if(f1<TWDR_ENEMY_HIT_R){
			//_sceVu0CopyVector(&mov,&ctrl->mov);
			_sceVu0SubVector(&mov,&ctrl->mov,&fvec1);
			if(bp_sqrtf((mov.vx*mov.vx+mov.vz*mov.vz-f1*f1))   //BP_MATH - emulate PS2 sqrtf
			   <=((mdl->ux-mdl->lx)*0.5f+TWDR_ENEMY_HIT_R_HOSEI)){
				_sceVu0SubVector(&mov,&ctrl->mov,&pmat0->m[3][0]);
				fw=bp_sqrtf(mov.vx*mov.vx+mov.vz*mov.vz); //BP_MATH - emulate PS2 sqrtf
				f0=atan2f(TWDR_ENEMY_HIT_R,fw);
				f1=Mz_LinePointLength(&fvec0,&fvec1,&ctrl->mov);
				f1=atan2f(f1,fw);
				if(Mz_LinePointCheck(&fvec0,&fvec1,&ctrl->mov)>0.0f)
					dr->froty+=(f0-f1);
				else
					dr->froty+=(f1-f0);
				dr->pow*=TWDR_BUND_KEISU;
				return 1;
			}
		}
	}
#endif
	
	return 0;
}/***  ***/

/*----------------------------------------------------------------*/
static void Mz_SeCalc(float f,int *pan,int *vol,DOOR_W *dr, float * pOutBPAngle){
	int i=(int)(f/0.05f);
	if(i>=0x3f)i=0x3f;
	*vol = i*GM_SeGetVol((FVECTOR*)dr->pobj->world.m[3],GM_SEMODE_NORMAL)/0x3f;
	*pan = GM_SeGetPan((FVECTOR*)dr->pobj->world.m[3],GM_SEMODE_NORMAL, pOutBPAngle) ;
}

/************************************************************
  Act()
  解説：メイン
  引数：Workポインタ
  返値：無し
  ***********************************************************/
static	void	Act( Work *work ){
	FMATRIX fmat,*pmat0,*pwmat;
	FVECTOR	fvec0;
	DG_MDL	*mdl=work->def->models;
	DOOR_W	*dr;
	int	 	i,pan,vol,num;
   float bp_angle;
	float	f0,befy;

	mdl=work->objs->def->models;
	pwmat=&work->objs->world;


	num = work->objs->n_models;
	if( num > 2 ) num = 2;

	dr=work->door;
	for(i=0;i<num;i++,mdl++,dr++){
		Collision(dr,mdl,i);
		dr->pow *= TWDR_FRICTION;
		befy=fpu_Abs(dr->froty);
		f0=fpu_Abs(dr->pow);
		if(!(f0<0.3f && befy<((float)M_PI/90.0f))){
			f0 = dr->froty * dr->froty * TWDR_SPRING;
			if(dr->froty>0.0f){
				dr->pow += -f0;
			}else{
				dr->pow += f0;
			}
			dr->count=0;
		}else{
			dr->count++;
//MENU_Printf("** %d **",dr->count);
		}

		befy=dr->froty;
		if(dr->pow>0.0f){
			dr->froty+=fpu_Sqrt((2.0f*dr->pow/dr->weight));
		}else if(dr->pow<0.0f){
			dr->froty-=fpu_Sqrt(-(2.0f*dr->pow/dr->weight));
		}

		/* 跳ね返り */
		if(dr->froty > dr->maxarc){
			dr->froty = dr->maxarc;
			dr->pow*=TWDR_BUND_KEISU;
		}else if(dr->froty < dr->minarc){
			dr->froty = dr->minarc;
			dr->pow*=TWDR_BUND_KEISU;
		}

//printf("work->objs->n_models:%d\n",work->objs->n_models);

		/* 左右ドアのマトリクス計算 */
		pmat0=&work->objs->objs[i].world;
		_sceVu0UnitMatrix(&fmat);
		fmat.m[3][0]=mdl->tx;
		fmat.m[3][1]=mdl->ty;
		fmat.m[3][2]=mdl->tz;
		_sceVu0MulMatrix(pmat0,pwmat,&fmat);
		_sceVu0RotMatrixY(&fmat,&DG_UnitMatrix,dr->froty);
		_sceVu0MulMatrix(pmat0,pmat0,&fmat);
		_sceVu0CopyMatrix(&fmat,pmat0);
		//fvec0.vx=-mdl->tx*0.5f;
		fvec0.vx=(mdl->ux+mdl->lx)*0.5f;
		fvec0.vy=(mdl->uy+mdl->ly)*0.5f;
		fvec0.vz=0.0f;
		fvec0.vw=1.0f;
		_sceVu0ApplyMatrix(&fvec0,pmat0,&fvec0);
		fmat.m[3][0]=fvec0.vx;
		fmat.m[3][1]=fvec0.vy;
		fmat.m[3][2]=fvec0.vz;
		GM_MoveTarget2(&dr->target_d,&fmat) ;

		f0=fpu_Abs(dr->pow);
		if(befy>0.0f && dr->froty<0.0f){
			if(dr->count>30){
				dr->froty=0.0f;
				dr->pow=0.0f;
			}
			Mz_SeCalc(f0,&pan,&vol,dr, &bp_angle);
			GM_SeSet3D(pan,vol,(dr->id)?SD_A_WSDOOR01:SD_A_WSDOOR02, bp_angle);
		}else if(befy<0.0f && dr->froty>0.0f){
			if(dr->count>30){
				dr->froty=0.0f;
				dr->pow=0.0f;
			}
			Mz_SeCalc(f0,&pan,&vol,dr, &bp_angle);
			GM_SeSet3D(pan,vol,(dr->id)?SD_A_WSDOOR02:SD_A_WSDOOR01, bp_angle);
		}
	}
	DG_GetLightMatrix( &work->pos, work->lights );

}/*** Act() fin ***/

/************************************************************
  Die()
  解説：資源の解放
  引数：Workポインタ
  返値：無し
  ***********************************************************/
static void Die( Work *work ){
	if( work->objs != NULL ){
		DG_DequeueObjs( work->objs );
		DG_FreeObjs( work->objs );
	}
}/*** Die() fin ***/


/************************************************************
  GetResources()
  解説：資源の確保
  引数：Workポインタ
  返値：無し
  ***********************************************************/
static int GetResources( Work *work , int name, int where ){
	FMATRIX			fmat;
	FVECTOR			fvec,fvec2;
	int				i;
	float			f;
	DG_DEF			*def;
	DG_MDL			*mdl;
	
	TARGET			*tr;
	POWER_TARGET 	*pw;
	DOOR_W			*dr;


/* シナリオオプション */
	if( (i = GCL_GetOptionValue( 'm', 0 )) ){
		work->def = def = (DG_DEF*)GV_GetCache( GV_CacheID( i, 'k' ) ) ;
	}else{
		return -1 ;
	}

	work->objs = DG_MakeObjs( def, BODY_FLG, 0 );
	if( work->objs==NULL ) return -1;

	work->objs->bound_min.vx = -3000.0f;
	work->objs->bound_min.vy = -3000.0f;
	work->objs->bound_min.vz = -3000.0f;
	work->objs->bound_max.vx =  3000.0f;
	work->objs->bound_max.vy =  3000.0f;
	work->objs->bound_max.vz =  3000.0f;


	if( work->objs->n_models < 2 ){
		printf("WARNING:twin_dr.c:::-----------------------------------------------\n");
		printf("WARNING:twin_dr.c:::work->objs->n_models:%d\n",work->objs->n_models);
		printf("WARNING:twin_dr.c:::指定されたモデルには関節が１つしかありません！\n");
		printf("WARNING:twin_dr.c:::プログラム内で対処します。\n");
		printf("WARNING:twin_dr.c:::-----------------------------------------------\n");
//		return -1;
	}


	DG_QueueObjs( work->objs );
	GM_GroupObjs( work->objs, work->where );

	if ( GCL_GetOption( 'p' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			fvec.vx   = (float)GCL_GetNextInt() ;
			fvec.vy   = (float)GCL_GetNextInt() ;
			fvec.vz   = (float)GCL_GetNextInt() ;
		}
	}
	fvec.vw=1.0f;

	work->froty = 0.0f;
	if ( GCL_GetOption( 'r' ) != NULL ) {
		if( GCL_NextStr() != NULL ){
			i = (int)GCL_GetNextInt() ;
			work->froty = Mz_itof(i);
		}
	}
	
	_sceVu0UnitMatrix( &fmat );
	_sceVu0RotMatrixY( &fmat, &fmat, work->froty );
	fmat.m[ 3 ][ 0 ] = fvec.vx ;
	fmat.m[ 3 ][ 1 ] = fvec.vy ;
	fmat.m[ 3 ][ 2 ] = fvec.vz ;
	work->objs->world=fmat;
	work->pos=fvec;
	work->pos.vy+=(def->uy-def->ly)*0.5f;

	DG_SetLightMatrix( work->objs, work->lights );

	/* 角度範囲 */
	if ( GCL_GetOption( 'a' ) != NULL ) {	/* mAx */
		dr=work->door;
		if( GCL_NextStr() != NULL ){
			dr->minarc = -Mz_itof(GCL_GetNextInt());
			dr->maxarc = Mz_itof(GCL_GetNextInt());
			//printf("<<%f %f ",dr->minarc,dr->maxarc);
			dr++;
			dr->minarc = -Mz_itof(GCL_GetNextInt());
			dr->maxarc = Mz_itof(GCL_GetNextInt());
			//printf("%f %f >>\n",dr->minarc,dr->maxarc);
		}else{
			dr->minarc = -TWDR_ARC_LIMIT;
			dr->maxarc = TWDR_ARC_LIMIT;
			dr++;
			dr->minarc = -TWDR_ARC_LIMIT;
			dr->maxarc = TWDR_ARC_LIMIT;
		}
	}
	
	mdl=def->models;
	f=fpu_Abs(mdl->tx)*2.5f;

	/* ドアの初期化 */
	dr=work->door;
	for ( i=0 ; i<2 ; i++,mdl++,dr++){
		tr=&dr->target_d;	pw=&dr->power_d;
		
		/* ターゲット */
		fvec2.vx = (mdl->ux - mdl->lx)*0.5f;
		fvec2.vy = (mdl->uy - mdl->ly)*0.5f;
		fvec2.vz = (mdl->uz - mdl->lz)*0.5f;
		GM_SetTarget(tr,TARGET_DEFENSE|TARGET_ROTATE,
					 work->where,BOTH_SIDE,&fvec2,&DG_ZeroVector) ;
		GM_SetPowerTarget(tr,pw,POWER_CONST,
						  1,0,0,&DG_ZeroVector) ;
		GM_SetTargetCallBack( tr,(void*)TwindoorCallBack,(void*)dr) ;
		GM_PutTarget(tr);

		//NewTargetView(tr,200*i,50,100*(1-i) ) ;

		dr->froty=0.0f;
		dr->pow=0.0f;
		dr->weight=TWDR_WEIGHT;
		dr->id=i;
		dr->pobj=&work->objs->objs[i];
		dr->work=work;
	}

	DG_VisibleObjs(work->objs);

	return 0 ;
}/*** GetResources() fin ***/


/************************************************************
  NewMzTwindoor
  解説：
  引数：
  返値：無し
  ***********************************************************/
void *NewMzTwindoor(  int name, int where  )
{
	Work		*work ;
	
	OPERATOR() ;
	
	work = (Work *)GV_NewEffect( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {

		work->name = name;
		work->where = where;

		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work , name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	return (void *)work ;
}/*** NewMzTwindoor fin ***/


/************************************************************
 
  解説：
  引数：
  返値：
  ***********************************************************/
/***  fin ***/


