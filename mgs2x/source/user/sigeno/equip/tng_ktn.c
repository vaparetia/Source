//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tng_ktn.c
	天狗兵刀
	$Id: tng_ktn.c,v 1.1.1.3 2002/11/19 11:49:52 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"libdg.h"
#include	"libdg.cnf"
#include	"gameheader.h"
#include	"libutl.h"
#include "korekado/enemy/enemy.h"
#include	"../tng/tng.h"
//extern	void	*NewInterPoly_Gr( FVECTOR *, float , CVECTOR );
	extern void DG_ConnectObjs( DG_OBJS *, DG_OBJS * ) ;
//extern void *NewInterPoly_Demo(FVECTOR *,FVECTOR *,int,SVECTOR * ) ;
//extern void *NewInterLineBlade( FVECTOR *,FVECTOR *,int , SVECTOR * ) ;

extern void *NewBladeEft( FVECTOR *pos0, FVECTOR *pos1, int n_disp, int init_col, int *alpha );
//NewBladeEft( &pos[0], &pos[1], 8, 0x80808000, &debug_alpha );
extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;

extern void SearchAndKillAttachment_called2(int ,OBJECT *) ;
extern void *NewSpark_White( FMATRIX * );


/*
tng_magazine.kms
tng_saya.kms 
*/

#define BLUR (6)
//#define BLUR_ALPHA	(96)
#define BLUR_ALPHA	(128)
#define BLUR_RGB		(0x08141a00)
#define BLUR_DELAY	(10)
typedef	struct	{
	GV_ACT_EX			actor ;
	DG_OBJS		*objs;
//	DG_OBJS		*saya;
	FMATRIX		light[2] ; 
	OBJECT	*body ;
	short		*flag ;
	int		flag_buf ;
	void	*blur ;
	FVECTOR	pos[2] ; /*付随エフェクト用*/
	FVECTOR	shift[2] ;
	TARGET		def;
	POWER_TARGET power;
	ENETHINK	*entk;
	float		fall ;
	float		yuka ;
	SVECTOR		rot ;
	int		fall_cnt ;
	int		alpha ;
	int		blur_cnt ;
} TngKtnWork ;


/*
HUMAN21_KOSHI
HUMAN21_ONAKA
HUMAN21_MUNE
HUMAN21_HIDARI_ASHI1
腰：位置１６４，４０，６
*/
#define SAYA_POS HUMAN21_HIDARI_KOSHI
//static FVECTOR	saya_shift = {164.0f,40.0f,6.0f,0.0f} ;

#define TRG_SHIFT (350.0f)
#define TRG_SIZE (300.0f)

static void Act(TngKtnWork *work)
{
	FMATRIX	trg;
	FVECTOR shift = {0.0f,-100.0f,TRG_SHIFT,0.0f} ;
	ENTK_TENG_A *tng_a ;
	tng_a = (ENTK_TENG_A *) work->entk->eve_a ;

	/*BlurCheck*/
	if(!(tng_a->katana & TNG_KATANA_BLUR_ON)){
		work->blur_cnt = BLUR_DELAY ;
		if(*work->flag & TNG_KATANA_ON){
			tng_a->katana |= TNG_KATANA_BLUR_ON ;
		}
	}
	if(PL_GetPlayerWeapon() == WP_Psg1T){
		GM_SetTargetName( &work->def, WP_m92) ;
	}else {
		GM_SetTargetName( &work->def, WP_Psg1 );
	}

#if 0
	int test = 0 ;
//		if(GV_PadData[ 1 ].press & 
	if(GV_PadData[ 1 ].status & PAD_L2){
		work->shift[test].vx +=1.0F ;
		printf("NOW %d [%f] \n",test,work->shift[test].vx);
	}
	if(GV_PadData[ 1 ].status & PAD_R2){
		work->shift[test].vx -=1.0F ;
		printf("NOW %d [%f] \n",test,work->shift[test].vx);
	}
#endif
	if(*work->flag & TNG_KATANA_FALL){
		/*落下中*/
//		rot.vx = (short) work->fall ;
//		rot.vy = (short) work->fall ;
//		rot.vz = (short) work->fall ;
		if(
		((work->yuka+100.0f)> work->objs->world.m[ 3 ][ 1 ] )
		&&(work->fall <0.0f)
		){
			work->fall *= -0.6f ;
			work->rot.vx *= -3;
			work->rot.vy *= 3;
			work->rot.vz *= 3;
			work->rot.vx /= 4;
			work->rot.vy /= 4;
			work->rot.vz /= 4;
		}else {
			work->fall -= 7.0f ;
		}
		DG_SetPos(&work->objs->world) ;
		DG_RotatePos(&work->rot) ;
		DG_GetPos(&work->objs->world) ;
		work->objs->world.m[ 3 ][ 0 ] += 10.0f ;
		work->objs->world.m[ 3 ][ 1 ] += work->fall ;
		work->objs->world.m[ 3 ][ 2 ] += 10.0f ;
		work->fall_cnt++ ;
	}else {
		/*右手に装備*/
		work->fall_cnt = 0 ;
		work->fall = 120.0f ;
		work->yuka = work->entk->ctrl->levels[0];
#if 0
		work->objs->world 
			= work->body->objs->objs[HUMAN21_MIGI_TE].world ;
#endif
	}


	DG_SetPos( &work->body->objs->objs[HUMAN21_KOSHI].world );
//	DG_MovePos( &saya_shift ) ;
//	DG_GetPos( &work->saya->world );

	GM_GroupObjs( work->objs, work->body->map_name ) ;
//	GM_GroupObjs( work->saya, work->body->map_name ) ;

	if(
	(*work->flag & TNG_KATANA_ON)
	&&(
	((work->fall_cnt < DIRECT_TICK(60)))
	||(work->fall_cnt&1)
	)
	){
		work->objs->flag &= ~(DG_FLAG_INVISIBLE);
		work->objs->flag |= 
		(work->body->objs->flag & DG_FLAG_INVISIBLE);	
	}else {
		work->objs->flag |= DG_FLAG_INVISIBLE ;
	}
//	work->saya->flag = work->objs->flag ;

	DG_SetPos( &work->objs->world );
	DG_PutVector( work->shift, work->pos, 2 );

	if(work->blur_cnt ==0 ){
		if(work->blur == NULL) {
			work->blur = NewBladeEft( &work->pos[0], &work->pos[1], 
			BLUR, BLUR_RGB, &work->alpha );
			if(work->blur != NULL) {
				GV_SetActorChild( work , work->blur );
			}
		}
	}else {
		if ( work->blur != NULL ) {
			GV_DestroyOtherActor( work->blur ) ;
			work->blur = NULL ;
		}
	}

	if(
	(work->entk->act->status & ACT_STATUS_GURAD)
	&&(*work->flag & TNG_KATANA_ON)
	&&(tng_a->guard_n>0)
//	&&(GM_WeaponFire != WP_Psg1) 
	){
		work->def.class &= ~TARGET_SKIP ;
		DG_SetPos( &work->body->objs->objs[HUMAN21_ATAMA].world );
		DG_MovePos( &shift ) ;
		DG_GetPos(&trg) ;
		GM_MoveTarget2Map( &work->def,&trg,work->body->map_name);
	}else {
		work->def.class |= TARGET_SKIP ;
	}

	work->flag_buf = *work->flag ;

	if(work->blur_cnt >0 ){
		work->blur_cnt-- ;
	}
}
static void Die(TngKtnWork *work)
{
	extern void DG_DisconnectObjs( DG_OBJS *, DG_OBJS * ) ;

	DG_DisconnectObjs( work->body->objs, work->objs ) ;
    if ( work->objs != NULL ){
		DG_DequeueObjs( work->objs ) ;
		DG_FreeObjs( work->objs );
    }
//	DG_DisconnectObjs( work->body->objs, work->saya ) ;
//    if ( work->saya != NULL ){
//		DG_DequeueObjs( work->saya ) ;
//		DG_FreeObjs( work->saya );
//    }
	GM_FreeTarget( &work->def );
}
/* ターゲットコールバック関数 */
/* アタリエフェクト */
static void CallSpark( work )
TngKtnWork	*work;
{
	SVECTOR	spark_rot = { 0,0,1024};
	FMATRIX	spark_m;
	FVECTOR	*hit ;
	TARGET	*trg;
	int type ;

	trg = &work->def;
	hit = &work->def.hit;
	DG_SetPos( &work->body->objs->objs[HUMAN21_MIGI_TE].world );
	DG_RotatePos(&spark_rot);
//	DG_SetPos2( hit, &spark_rot );
	DG_GetPos( &spark_m );

	spark_m.m[ 3 ][ 0 ] = hit->vx ;
	spark_m.m[ 3 ][ 1 ] = hit->vy ;
	spark_m.m[ 3 ][ 2 ] = hit->vz ;

	NewSpark_White( &spark_m );
	type =(BUL_TYPE_VISIBLE|BUL_TYPE_SPARK|BUL_TYPE_NO_ATTACK) ;
//	NewBullet( &spark_m, type, PLAYER_SIDE, 10, 10, 5000, 1000, WP_Famas ) ;
	NewBullet( &spark_m, type, PLAYER_SIDE, 10, 10, 5000, 1000, WP_Aks ) ;

//
}

//	(GM_WeaponFire == WP_Psg1) 

static	void	ChildTargCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	TngKtnWork	*work ;
	ENTK_TENG_A *tng_a ;

	work = ( TngKtnWork * )ptr ;
	tng_a = (ENTK_TENG_A *) work->entk->eve_a ;

#if 1
    if ( def->damaged & TARGET_POWER ) {
//printf("SHL VITAL %d\n",def->power->vital);
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			if(off->power != NULL ) {
				def->power->force = off->power->force ;
			}
		 	if( def->weapon_type & (WP_BULLET|WP_M92)){
				CallSpark(work);
				GM_SeSetMode(SD_A_SWORDRIC,&def->hit,GM_SEMODE_BOMB ) ;
				*work->flag |= TNG_KATANA_BULLET_HIT ;
				tng_a->guard_n--;
			}
		}
	}
#endif
//	if(def->weapon_type & WP_BULLET){
//		CallSpark(work);
//	}
	def->weapon_type = 0 ;
}
static void SetTarget(work)
TngKtnWork *work;
{
	int flag,map;
	FVECTOR	size = { 750.0f, 500.0f, TRG_SIZE, 0.0f } ;

	/*ターゲット設定*/
	flag =(TARGET_DEFENSE|TARGET_SEEK|TARGET_NO_LOCKON
	|TARGET_POWER|TARGET_ROTATE);
	map = 0;
	GM_SetTarget( &work->def, flag,map, ENEMY_SIDE, &size, &DG_ZeroVector ) ;

	GM_SetTargetCallBack( &work->def, ChildTargCallBack, work ) ;
	GM_SetTargetWeaponType(&work->def,0);

	GM_SetPowerTarget( &work->def, &work->power,POWER_DECREASE,0, 0, 0, &DG_ZeroVector );

	GM_SetTargetName( &work->def, WP_Psg1 );

	GM_PutTarget( &work->def );

//	NewTargetView( &work->def, 0, 0, 255 ) ;

}

/*資源確保と各種初期設定*/
static int GetResources( TngKtnWork *work ,OBJECT *body,short *flag,ENETHINK *entk){
	DG_DEF  *def ;
//	FVECTOR sub ;
//	float endtime ,len ,base_vel;

	work->body = body;
	work->flag = flag;
	work->flag_buf = 0 ;
	work->blur = NULL ;
	work->blur_cnt = 0 ;
	work->fall = 0.0f ;
	work->yuka = 0.0f ;
	work->entk = entk ;
	work->rot.vx =100 ;
	work->rot.vy =100 ;
	work->rot.vz =100 ;
	work->fall_cnt = 0 ;
	work->alpha = BLUR_ALPHA ;
//GV_StrCode( "tbl_katana");
//	high = DEFAULT_HIGH ;
	def = (DG_DEF*) GV_GetCache( GV_CacheID( MDL_TNG_KANATA_G, 'k' ) ) ;
	work->objs = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_FINISHCALC, DG_CHANL_MAIN ) ;
	ASSERT(work->objs != NULL ) ;

	/*鞘*/
//	def = (DG_DEF*) GV_GetCache( GV_CacheID( GV_StrCode( "tng_saya"), 'k' ) ) ;
//	work->saya = DG_MakeObjs( def, DG_FLAG_SHADE|DG_FLAG_ONEPIECE|DG_FLAG_FINISHCALC, DG_CHANL_MAIN ) ;

	DG_QueueObjs( work->objs ) ;
//	DG_QueueObjs( work->saya ) ;

//	DG_SetLightMatrix( work->objs, work->light );
	DG_SetLightMatrix( work->objs, work->body->objs->light );
//	DG_SetLightMatrix( work->saya, work->body->objs->light );

	DG_ConnectObjs( work->body->objs, work->objs ) ;
//	DG_ConnectObjs( work->body->objs, work->saya ) ;

#if 0
	work->objs->world 
		= work->body->objs->objs[HUMAN21_MIGI_TE].world ;
#else
	work->objs->root 
		= &work->body->objs->objs[HUMAN21_MIGI_TE].world ;
#endif

	DG_SetPos( &work->body->objs->objs[HUMAN21_KOSHI].world );
//	DG_MovePos( &saya_shift ) ;
//	DG_GetPos( &work->saya->world );

	work->shift[0] = work->shift[1] = DG_ZeroVector ;
	work->shift[0].vz = 80.0F ;
	work->shift[1].vz = 690.0F ;

	work->shift[0].vy = -100.0F ;
	work->shift[1].vy = -278.0F ;

	work->shift[0].vx = +12.0F ;
	work->shift[1].vx = -38.0F ;


	SetTarget(work) ;
	return 1;
}



/******
ENE_SetAttachmentTNGGUN( (void *)entk->w, body ) ;
ENE_SetAttachmentTNGKATANA( (void *)entk->w, body ) ;
*******/

void TngKatanaOn(ENETHINK *entk){
	ENTK_TENG_A *tng_a ;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	/*装備変更時のみ*/
	if(!(entk->act->bodyp.type & ENE_TYPE_KATANA)){
//		SearchAndKillAttachment_called2(3,entk->act->body) ;
//		SearchAndKillAttachment_called(entk->act->body) ;
		if ( entk->attachment != NULL ) {
			GV_DestroyOtherActor( entk->attachment ) ;
			entk->attachment = NULL ;
		}
		entk->attachment = ENE_SetAttachmentTNGKATANA( (void *)entk->w, entk->act->body ) ;
		entk->act->bodyp.type |= ENE_TYPE_KATANA ;
		entk->sw_gun &= ~SW_FLAG_VISIBLE ;
		tng_a->katana = (TNG_KATANA_ON|TNG_KATANA_BLUR_ON);
	}
	tng_a->katana |= TNG_KATANA_BLUR_ON  ;
}

void TngKatanaOff(ENETHINK *entk){
	ENTK_TENG_A *tng_a ;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	if(entk->act->bodyp.type & ENE_TYPE_KATANA){
//		SearchAndKillAttachment_called2(3,entk->act->body) ;
//		SearchAndKillAttachment_called(entk->act->body) ;
		if ( entk->attachment != NULL ) {
			GV_DestroyOtherActor( entk->attachment ) ;
			entk->attachment = NULL ;
		}
		entk->attachment = ENE_SetAttachmentTNGGUN( (void *)entk->w, entk->act->body ) ;
		entk->act->bodyp.type &= ~ENE_TYPE_KATANA ;
		entk->sw_gun |= SW_FLAG_VISIBLE ;
		tng_a->katana &= ~(TNG_KATANA_ON|TNG_KATANA_BLUR_ON) ;
	}
}
void TngKatanaBlurOff(ENETHINK *entk){
	ENTK_TENG_A *tng_a ;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_a->katana &= ~(TNG_KATANA_BLUR_ON) ;
}
void TngKatanaFall(ENETHINK *entk){
	ENTK_TENG_A *tng_a ;
	tng_a = (ENTK_TENG_A *) entk->eve_a ;
	tng_a->katana |= TNG_KATANA_FALL ;
}



void	*NewTngKatana( OBJECT *body,short *flag ,ENETHINK *entk )
{
	TngKtnWork		*work ;
	OPERATOR() ;
    work = (TngKtnWork *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
			   sizeof( TngKtnWork ), AFTER_ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work ,body , flag,entk ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

