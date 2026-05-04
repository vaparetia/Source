//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	eneequip.c
	デモ用敵兵装備品

	2000/03/27 Y.Korekado
	$Id: eneequip.c,v 1.1.1.3 2002/11/19 11:44:03 Yoshizawa1 Exp $
*/
/*
メッセージ

ENE_EQUIP_SHOT[0] status ：発砲
	status :
		SHOOT_LEFT		0x0001：左手の武器から
		SHOOT_NOSPARK	0x0002：兆弾無し
		SHOOT_RANDAM	0x0004：軌道がランダム
		SHOOT_TMPLIGHT1	0x0008：点光源１
		SHOOT_TMPLIGHT2	0x0010：点光源２

ENE_EQUIP_MAGG[1] status ：マガジンチェンジ
	status :
		1：銃に装着
		2：左手に持つ
		4：銃から外す
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
#include	"libmt.h"
#include	"libutl.h"

#include	"gameheader.h"
#include	"define.h"
#include	"korekado/conv/korekado.x"
#include "../../kano/attachment/attachments.h"
#include "../../skoba/weapon_old/matsu.h"

/*----------------------------------------------------------------*/
#define	EQUIP_HLST		0x00000001
#define	EQUIP_KNIF		0x00000002
#define	EQUIP_KNIFCASE	0x00000004
#define	EQUIP_MAG		0x00000008
#define	EQUIP_BACKPACK	0x00000010
#define	EQUIP_RADIO		0x00000020
#define	EQUIP_NSIGHT	0x00000040
#define	EQUIP_WEAPON	0x00000080
#define	EQUIP_WPFREE	0x00000100
#define	EQUIP_NSIGHT_OPEN	0x00000200
#define	EQUIP_LIGHT_R	0x00000400	/* ライト右手 モデルは'aks,aks_sp,tnr_frashlight に対応 */
#define	EQUIP_LIGHT_L	0x00000800	/* ライト左手 モデルは'aks,aks_sp,tnr_frashlight に対応  */
#define	EQUIP_LEFT		0x00001000	/* 左手にモデル持つ（モデル名を指定する必要有り） */

#define	EQUIP_GPS		0x00002000	/* プラントゴル兵 */
#define	EQUIP_GPA		0x00004000	/* プラント攻撃ゴル兵 */
#define	EQUIP_TNGGUN	0x00008000	/* 銃天狗 */
#define	EQUIP_TNGKATANA	0x00010000	/* 刀天狗 */
#define	EQUIP_SAYA		0x00020000	/* 刀鞘 */

/*----------------------------------------------------------------*/
#define	AKS			(102867) /* GV_StrCode( "aks" ) */
#define	AKS_SP		(15403928)	/* GV_StrCode( "aks_sp" ) */
#define ABAKAN		MDL_ABAKAN		/*abk.kms*/
#define	FLASHLIGHT	(1730965)	/* GV_StrCode( "tnr_frashlight" ) */
//#define	M4			(7473930)	/* GV_StrCode( "m4a_nm" ) */
#define	M4			(30)	/* GV_StrCode( "m4a_nm" ) */

typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		*body ;
	OBJECT		weapon ;
	int			flag ;

	int			name ;
	int			wpname ;
	int			wpleft ;

	int	sw_radio ;
	int	sw_nsight ;
	int	sw_gun ;
	int	sw_light_r ;
	int	sw_light_l ;
	int	sw_magg ;

	void	*attachments ;
	void	*radio ;
	void	*nsight ;
	void	*slinggun ;
	void	*sling ;
	void	*left ;
	void	*right ;
	void	*light_r ;
	void	*light_l ;
	void	*magg ;
	WEAPON_EF_CTRL wctrl ;
} Work ;

static FVECTOR shift[]={
    { -185.0f,  27.5f,  61.5f,   0.0f, },
    {  -67.5f,  -2.5f, 152.5f,   0.0f, },
    {  120.0f,  22.5f, -60.0f,   0.0f, },
    {  140.0f,  37.0f, -49.0f,   0.0f, },
};
static FVECTOR shift2[]={
    {  -65.0f, -10.0f,  42.5f,   0.0f, },
    {   32.5f,-140.0f, 112.5f,   0.0f, },
    {   20.0f, -92.5f,-117.5f,   0.0f, },
};
static FVECTOR shift3[]={
    { 0.0f, -40.0f,-140.0f,   0.0f, },
    { 150.0f, 40.0f, 37.0f, 0.0f, },
    { 164.0f, 40.0f, 6.0f, 0.0f, },
    { 0.0f, 0.0f, 0.0f, 0.0f, },
};

static const SVECTOR rot1[]={
    {  -68, 0, 0, 0, },
} ;

static ATTACHMENT_ARGUMENT2 arg2[]={
    {
	0x291040 /* GV_StrCode("gbs_hlst") */ ,
	NULL,
	NULL,
	0,
	shift+0,
	13,
	shift2+0,
    },
    {
	0x2a96f2 /* GV_StrCode("gbs_knif") */ ,
	NULL,
	NULL,
	0,
	shift+1,
	13,
	shift2+1,
    },
    {
	0x615b12 /* GV_StrCode("gbs_mag") */ ,
	NULL,
	NULL,
	0,
	shift+2,
	17,
	shift2+2,
    },

	/* プラントゴル兵用*/
    {
	0x291040 /* GV_StrCode("gbs_hlst") */ ,
	NULL,
	NULL,
	0,
	shift+0,
	13,
	shift2+0,
    },
    {
	5459626 /* GV_StrCode("gps_knife") */ ,
	NULL,
	(SVECTOR *)(rot1+0),
	0,
	shift+1,
	13,
	shift2+1,
    },
    {
	0x615b12 /* GV_StrCode("gbs_mag") */ ,
	NULL,
	NULL,
	0,
	shift+3,
	17,
	shift2+2,
    },
};

static ATTACHMENT_ARGUMENT3 arg3[]={
    {
	0x5b0984 /* GV_StrCode("gbs_bp") */ ,
	NULL,
	2,
	shift3+0,
	2,
    },
    {
	9531134 /* GV_StrCode("tng_magazine") */ ,
	NULL,
	0,
	(FVECTOR *)(shift3+1),
	1,
    },
    {
	3067861 /* GV_StrCode("tng_saya") */ ,
	NULL,
	0,
	(FVECTOR *)(shift3+2),
	1,
    },
    {
	14901013 /* GV_StrCode("tng_strap") */ ,
	NULL,
	0,
	(FVECTOR *)(shift3+3),
	1,
    },
};

#define	MAX_ATARG2	6
#define	MAX_ATARG3	2 /* yano modifyed 1 -> 2 2002.03.11 */
enum {
	AT2_HLST,
	AT2_KNIF,
	AT2_MAG,

	AT2_GP_HLST,
	AT2_GP_KNIF,
	AT2_GP_MAG,

} ;
enum {
	AT3_BP,
	AT3_TNG_MAG,
	AT3_TNG_SAYA,
	AT3_TNG_SAYADOME,
} ;

/* message */
enum {
	ENE_EQUIP_SHOT = 0,
	ENE_EQUIP_MAGG,
	ENE_EQUIP_GUN_FREE,
	ENE_EQUIP_GUN_HANDIN
} ;
/*----------------------------------------------------------------*/
#define	SHOOT_LEFT		0x0001
#define	SHOOT_NOSPARK	0x0002
#define	SHOOT_RANDAM	0x0004
#define	SHOOT_TMPLIGHT1	0x0008
#define	SHOOT_TMPLIGHT2	0x0010

static	FVECTOR	ShootPointAks[] = { { 20.0F, -712.0F, 90.0F } } ;
static	void ShootBullet( Work *work, int status )
{
	extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
	extern void AN_CartridgeAKS_E( FMATRIX *, OBJECT *, CONTROL * ) ;
	extern void AN_MazzleAKS( FMATRIX *, int, int ) ;
	FMATRIX	w, *hand ;
    FVECTOR	from ;
    SVECTOR	rot ;
    int		tmplight, type, name ;

	type = 0 ;

	/* 銃を持つ手のマトリックス */
	if ( status & SHOOT_LEFT ) {
		hand = &BODYWORLD( work->body, HUMAN21_HIDARI_TE ) ;
		name = work->wpleft ;
	} else {
		hand = &BODYWORLD( work->body, HUMAN21_MIGI_TE ) ;
		name = work->wpname ;
	}

    DG_SetPos( hand ) ;
	/* 銃口までシフト */
	DG_PutVector( &ShootPointAks[0], &from, 1 ) ;
	KR_FvecToMat( &from, &w ) ;
	
	/* 跳弾 */
	type |= ( status & SHOOT_NOSPARK ) ? 0 : BUL_TYPE_SPARK ;

	/* 点光源 */
	tmplight = 0 ;
	if ( status & SHOOT_TMPLIGHT1 ) {
		tmplight = 1 ;
	} else if ( status & SHOOT_TMPLIGHT2 ) {
		tmplight = 2 ;
	}

	if ( status & SHOOT_RANDAM ) {	/* 角度にランダム入れる */
		DG_SetPos( &w ) ;
		rot.vx = KR_RandS( 64 )  ;
		rot.vy = KR_RandS( 64 )  ;
		rot.vz = 0 ;
		DG_RotatePos( &rot ) ;
		DG_GetPos( &w ) ;
	}
	NewBullet( &w, type, PLAYER_SIDE, 25, 0, 20000, 1000, WP_Famas ) ;

	switch ( name ) {
		case AKS :
			AN_CartridgeAKS_E( hand, work->body, NULL ) ;
			AN_MazzleAKS( hand, 0, tmplight ) ;
		break ;
		case AKS_SP :
		case M4 :
			AN_CartridgeAKS_E( hand, work->body, NULL ) ;
			AN_MazzleAKS( hand, 1, tmplight ) ;
		break ;
	}
}

static void CheckMessage( work )
Work	*work ;
{
	GV_MSG *msg;
	int mes_num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		switch( msg->message[0] ){
			case ENE_EQUIP_SHOT :
				ShootBullet( work, msg->message[ 1 ] ) ;
			break ;
			case ENE_EQUIP_MAGG :
				work->sw_magg = msg->message[ 1 ] ;
			break ;
			case ENE_EQUIP_GUN_FREE :
				SET_FLAG( work->sw_gun, SW_FLAG_SWITCH2 ) ;
			break ;
			case ENE_EQUIP_GUN_HANDIN :
				UNSET_FLAG( work->sw_gun, SW_FLAG_SWITCH2 ) ;
			break ;
		}
		msg--;
	}
}

static	void	Act( work )
Work		*work ;
{
	CheckMessage( work ) ;

	if ( work->wpname == ABAKAN ) {
		WeaponEfAct ( &work->wctrl );
	}
}

static	void	Die( work )
Work		*work ;
{
	if ( !(GV_IsFollowDestroy( work )) ) {
#if 1
		if( work->attachments != NULL ) GV_DestroyOtherActor( work->attachments ) ;
		if( work->radio != NULL ) GV_DestroyOtherActor( work->radio ) ;
		if( work->nsight != NULL ) GV_DestroyOtherActor( work->nsight ) ;
		if( work->slinggun != NULL ) GV_DestroyOtherActor( work->slinggun ) ;
		if( work->sling != NULL ) GV_DestroyOtherActor( work->sling ) ;
		if( work->left != NULL ) GV_DestroyOtherActor( work->left ) ;
		if( work->right != NULL ) GV_DestroyOtherActor( work->right ) ;
		if( work->light_r != NULL ) GV_DestroyOtherActor( work->light_r ) ;
		if( work->light_l != NULL ) GV_DestroyOtherActor( work->light_l ) ;
		if( work->magg != NULL ) GV_DestroyOtherActor( work->magg ) ;
#else
		if( work->attachments != NULL ) GV_DestroyActor( work->attachments ) ;
		if( work->radio != NULL ) GV_DestroyActor( work->radio ) ;
		if( work->nsight != NULL ) GV_DestroyActor( work->nsight ) ;
		if( work->slinggun != NULL ) GV_DestroyActor( work->slinggun ) ;
		if( work->sling != NULL ) GV_DestroyActor( work->sling ) ;
		if( work->left != NULL ) GV_DestroyActor( work->left ) ;
		if( work->right != NULL ) GV_DestroyActor( work->right ) ;
		if( work->light_r != NULL ) GV_DestroyActor( work->light_r ) ;
		if( work->light_l != NULL ) GV_DestroyActor( work->light_l ) ;
		if( work->magg != NULL ) GV_DestroyActor( work->magg ) ;
#endif
printf("eneequip: own destroy \n");
#if 0
	} else {
		printf("eneequip: parent destroy \n");
		if( work->slinggun != NULL ) GV_DestroyActorQuick( work->slinggun ) ;
	}
#else
	} else printf("eneequip: parent destroy \n");
#endif
}
/*----------------------------------------------------------------*/
static	void	InitSling( work, parent, body, weapon )
Work		*work ;
void		*parent ;
OBJECT		*body ;
OBJECT		*weapon ;
{
	extern void *NewSling( OBJECT *, OBJECT *, FVECTOR *, FVECTOR *, FMATRIX *, int ) ;

	FVECTOR	sft1[8], sft2[4] ;
	
	/* スリング */
	/* 右肩 */
	sft1[0].vx = -89.28 ; sft1[0].vy = 308.22 ; sft1[0].vz = 11.59 ;
	sft1[1].vx = -112.58 ; sft1[1].vy = 282.1 ; sft1[1].vz = 11.59 ;
	/* 左横腹 */
	sft1[2].vx = 205.86 ; sft1[2].vy = 44.79 ; sft1[2].vz = 59.88 ;
	sft1[3].vx = 182.56 ; sft1[3].vy = 18.67 ; sft1[3].vz = 59.88 ;
	/* 右肩奥 */
	sft1[4].vx = -89.0 ; sft1[4].vy = 308.0 ; sft1[4].vz = -47.0 ;
	sft1[5].vx = -112.0 ; sft1[5].vy = 282.0 ; sft1[5].vz = -47.0 ;
	/* 左横腹奥 */
	sft1[6].vx = 205.0 ; sft1[6].vy = 44.0 ; sft1[6].vz = 0.0 ;
	sft1[7].vx = 182.0 ; sft1[7].vy = 18.0 ; sft1[7].vz = 0.0 ;


	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 20.8 ; sft2[2].vy = -452.0 ; sft2[2].vz = 121.4 ;
	sft2[3].vx = 51.0 ; sft2[3].vy = -452.0 ; sft2[3].vz = 103.9 ;

	GV_SetActorChild( parent, 
		(work->sling = NewSling( body, weapon, sft1, sft2, body->objs->light, GV_StrCode("gbs_sling") ) ) ) ;
}

static	void	InitSlingGPS( work, parent, body, weapon )
Work		*work ;
void		*parent ;
OBJECT		*body ;
OBJECT		*weapon ;
{
	extern void *NewSling( OBJECT *, OBJECT *, FVECTOR *, FVECTOR *, FMATRIX *, int ) ;

	FVECTOR	sft1[8], sft2[4] ;
	
	/* スリング */
	/* 右肩 */
	sft1[0].vx = -89.28 ; sft1[0].vy = 308.22 ; sft1[0].vz = 11.59 ;
	sft1[1].vx = -112.58 ; sft1[1].vy = 282.1 ; sft1[1].vz = 11.59 ;
	/* 左横腹 */
	sft1[2].vx = 205.86 ; sft1[2].vy = 44.79 ; sft1[2].vz = 59.88 ;
	sft1[3].vx = 182.56 ; sft1[3].vy = 18.67 ; sft1[3].vz = 59.88 ;
	/* 右肩奥 */
	sft1[4].vx = -89.0 ; sft1[4].vy = 308.0 ; sft1[4].vz = -47.0 ;
	sft1[5].vx = -112.0 ; sft1[5].vy = 282.0 ; sft1[5].vz = -47.0 ;
	/* 左横腹奥 */
	sft1[6].vx = 205.0 ; sft1[6].vy = 44.0 ; sft1[6].vz = 0.0 ;
	sft1[7].vx = 182.0 ; sft1[7].vy = 18.0 ; sft1[7].vz = 0.0 ;


/*
アバカンのスリング位置です。
A(後上):39.5, -55.5, 72.6
B(後下):39.5, -55.5, 37.6
C(前上):41.5, -492, 95.5
D(前下):41.5, -492, 57.5
*/
	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 41.5 ; sft2[2].vy = -492.0 ; sft2[2].vz = 95.4 ;
	sft2[3].vx = 41.5 ; sft2[3].vy = -492.0 ; sft2[3].vz = 57.9 ;

	GV_SetActorChild( parent, 
		(work->sling = NewSling( body, weapon, sft1, sft2, body->objs->light, GV_StrCode("gps_sling") ) ) ) ;
}

/*座標の変更は enemy.c もすること！！！*/
static	void	InitSlingGPA( work, parent, body, weapon )
Work		*work ;
void		*parent ;
OBJECT		*body ;
OBJECT		*weapon ;
{
	extern void *NewSling( OBJECT *, OBJECT *, FVECTOR *, FVECTOR *, FMATRIX *, int ) ;

	FVECTOR	sft1[8], sft2[4] ;
/*
頂点座標  上首側(-86 312 59)
          上肩側(-115 293 59)
          下腕側(221 27 49)
          下胴側(188 0 49)
*/
	
	/* スリング */
	/* 右肩 */
	sft1[0].vx = -86.0 ; sft1[0].vy = 312.0 ; sft1[0].vz = 59.0 ;
	sft1[1].vx = -115.0 ; sft1[1].vy = 293.0 ; sft1[1].vz = 59.0 ;
	/* 左横腹 */
	sft1[2].vx = 211.0 ; sft1[2].vy = 27.0 ; sft1[2].vz = 49.0 ;
	sft1[3].vx = 188.0 ; sft1[3].vy = 0.0 ; sft1[3].vz = 49.0 ;
	/* 右肩奥 */
	sft1[4].vx = -97.0 ; sft1[4].vy = 327.0 ; sft1[4].vz = -15.0 ;
	sft1[5].vx = -127.0 ; sft1[5].vy = 309.0 ; sft1[5].vz = -15.0 ;
	/* 左横腹奥 */
	sft1[6].vx = 219.0 ; sft1[6].vy = 27.0 ; sft1[6].vz = -10.0 ;
	sft1[7].vx = 194.0 ; sft1[7].vy = 0.0 ; sft1[7].vz = -10.0 ;

	/* 銃後ろ */
	sft2[0].vx = 39.5 ; sft2[0].vy = -55.5 ; sft2[0].vz = 72.6 ;
	sft2[1].vx = 39.5 ; sft2[1].vy = -55.5 ; sft2[1].vz = 37.6 ;
	/* 銃前 */
	sft2[2].vx = 41.5 ; sft2[2].vy = -492.0 ; sft2[2].vz = 95.4 ;
	sft2[3].vx = 41.5 ; sft2[3].vy = -492.0 ; sft2[3].vz = 57.9 ;

	GV_SetActorChild( parent, 
		(work->sling = NewSling( body, weapon, sft1, sft2, body->objs->light, GV_StrCode("gpa_sling") ) ) ) ;
}

static	void	InitM4Sling( work, parent, body, weapon )
Work		*work ;
void		*parent ;
OBJECT		*body ;
OBJECT		*weapon ;
{
	extern void *NewBodyChalkerSling( OBJECT *body, OBJECT *weapon, FMATRIX *light ) ;

	GV_SetActorChild( parent, 
		(work->sling = NewBodyChalkerSling( body, weapon, body->objs->light ))) ;
}

static	int	GetResources( work, name, parent, body, flag, wpname, left )
Work	*work ;
int		name ;
void	*parent ;
OBJECT	*body ;
int		flag ;
int		wpname ;
int		left ;
{
	extern void *NewConnectObject( OBJECT *cnct_obj, int cnct_num, FVECTOR *shift, SVECTOR *rot, int model ) ;
	extern void	*NewRadio( OBJECT *, int, int *, TARGET * ) ;
	extern void	*NewNightSight( OBJECT *, int, int, int * ) ;
	extern void *NewSlingGun2( OBJECT *, OBJECT *, int, int *,WEAPON_EF_CTRL * ) ;
	extern void *NewCartridgeControl(int *,	OBJECT *, OBJECT *, int ) ;
	extern void *NewCircleLight( FMATRIX *, int *, int, int * ) ;
	ATTACHMENT_ARGUMENT2	at_arg2[MAX_ATARG2] ;
	ATTACHMENT_ARGUMENT3	at_arg3[MAX_ATARG3] ;
	int at2_num, at3_num ;

	if ( !parent )/*T.Morita Added デモエフェクト動作させるため 2000/09/07 */
	    parent =  work ;

	work->name = name ;
	work->body = body ;
	work->flag = flag ;

	work->wpname = wpname ;
	work->wpleft = left ;

	work->attachments = NULL ;
	work->radio = NULL ;
	work->nsight = NULL ;
	work->slinggun = NULL ;
	work->sling = NULL ;
	work->left = NULL ;
	work->right = NULL ;
	work->light_r = NULL ;
	work->light_l = NULL ;
	work->magg = NULL ;

	at2_num = at3_num = 0 ;


	if ( flag & EQUIP_GPS ) {
		if ( flag & EQUIP_HLST ) at_arg2[ at2_num++ ] = arg2[ AT2_GP_HLST ] ;
		if ( flag & EQUIP_KNIF ) at_arg2[ at2_num++ ] = arg2[ AT2_GP_KNIF ] ;
		if ( flag & EQUIP_MAG ) at_arg2[ at2_num++ ] = arg2[ AT2_GP_MAG ] ;
	} else if ( flag & EQUIP_TNGGUN ) {
		if ( flag & EQUIP_MAG ) at_arg3[ at3_num++ ] = arg3[ AT3_TNG_MAG ] ;
	} else if ( flag & EQUIP_TNGKATANA ) {
		if ( flag & EQUIP_SAYA ) at_arg3[ at3_num++ ] = arg3[ AT3_TNG_SAYA ] ;
		if ( flag & EQUIP_SAYA ) at_arg3[ at3_num++ ] = arg3[ AT3_TNG_SAYADOME ] ;
	} else {
		if ( flag & EQUIP_HLST ) at_arg2[ at2_num++ ] = arg2[ AT2_HLST ] ;
		if ( flag & EQUIP_KNIF ) at_arg2[ at2_num++ ] = arg2[ AT2_KNIF ] ;
		if ( flag & EQUIP_MAG ) at_arg2[ at2_num++ ] = arg2[ AT2_MAG ] ;
		if ( flag & EQUIP_BACKPACK ) at_arg3[ at3_num++ ] = arg3[ AT3_BP ] ;
	}

	if ( at3_num+at2_num > 0 ) {
		GV_SetActorChild( parent, 
			(work->attachments = 
				NewAttachments_called(body,NULL,0,at_arg2,at2_num,at_arg3,at3_num)) ) ;
	}

	/* 無線機 */
	if ( flag & EQUIP_RADIO ) {
		GV_SetActorChild( parent, 
			(work->radio = NewRadio( body, GV_StrCode("rad"), &work->sw_radio, NULL ) ) ) ;
		work->sw_radio = SW_FLAG_VISIBLE ;
	}

	/* 暗視ゴーグル */
	if ( flag & EQUIP_NSIGHT ) {
		GV_SetActorChild( parent,
			(work->nsight = NewNightSight(body,GV_StrCode("gbs_nvg2"),GV_StrCode("gbs_nvg1"), &work->sw_nsight ) ) ) ;
		work->sw_nsight = SW_FLAG_VISIBLE ;
		work->sw_nsight = 2 ;
	} else 	if ( flag & EQUIP_NSIGHT_OPEN ) {
		GV_SetActorChild( parent,
			(work->nsight = NewNightSight(body,GV_StrCode("gbs_nvg2"),GV_StrCode("gbs_nvg1"), &work->sw_nsight ) ) ) ;
		work->sw_nsight = SW_FLAG_VISIBLE ;
		work->sw_nsight = 1 ;
	}

	if ( flag & (EQUIP_WEAPON|EQUIP_WPFREE) ) {
		/* 武器 */
//		if ( wpname == 0 ) {
		if ( wpname == 1 ) {/*何も指定しない場合は１が入る*/
			if ( flag & (EQUIP_GPS|EQUIP_GPA) ) {
				wpname = ABAKAN ;
			} else {
				wpname = AKS_SP ;
			}
			work->wpname = wpname ;
		}
		if ( wpname == ABAKAN || wpname == AKS || wpname == AKS_SP || wpname == M4 ) {
			if ( flag & EQUIP_WPFREE ) {
				SET_FLAG( work->sw_gun, SW_FLAG_SWITCH2 ) ;
			} else {
				SET_FLAG( work->sw_gun, SW_FLAG_SWITCH1 ) ;
			}
/*****************************************************/
/*重野追加 可動部ありの武器も単一モデルとして表示する*/
			SET_FLAG( work->sw_gun, SW_FLAG_SWITCH6 ) ;
/*****************************************************/

			GV_SetActorChild( parent, 
				(work->slinggun = 
					NewSlingGun2( body, &work->weapon, wpname, &work->sw_gun, &work->wctrl ) ) ) ;

			work->sw_gun = SW_FLAG_VISIBLE ;
			if ( flag & EQUIP_WPFREE ) work->sw_gun |= SW_FLAG_SWITCH2 ;
			if ( wpname == M4 ) {
				InitM4Sling( work, parent, body, &work->weapon ) ;
				work->sw_magg = 1 ;
				GV_SetActorChild( parent,(work->magg = 
					NewCartridgeControl(&work->sw_magg, body, &work->weapon, 0 ) ) ) ;
			} else if ( wpname == ABAKAN ) {
				if ( flag & (EQUIP_GPS) ) {
					InitSlingGPS( work, parent, body, &work->weapon ) ;
				} else {
					InitSlingGPA( work, parent, body, &work->weapon ) ;
				}
				work->sw_magg = 1 ;
				GV_SetActorChild( parent,(work->magg = 
					NewCartridgeControl(&work->sw_magg, body, &work->weapon, 6 ) ) ) ;
			} else {
				InitSling( work, parent, body, &work->weapon ) ;
				work->sw_magg = 1 ;
				GV_SetActorChild( parent,(work->magg = 
					NewCartridgeControl(&work->sw_magg, body, &work->weapon, 0 ) ) ) ;
			}
		} else {
			GV_SetActorChild( parent,( work->slinggun =
				NewConnectObject( body, HUMAN21_MIGI_TE, NULL, NULL, wpname ))) ;
		}

		if ( flag & EQUIP_LIGHT_R ) {
			int n ;

			n = 0 ;
			work->sw_light_r = 1 ;
			switch( wpname ) {
				case AKS :
					n = 1 ;
				break ;
				case AKS_SP :
				case M4 :
					n = 0 ;
				break ;
				case FLASHLIGHT :
					n = 4 ;
				break ;
			}
//			GV_SetActorChild( parent,( work->light_r =
//				NewCircleLight(&work->weapon.objs->world,&work->sw_light_r, n,&GM_CurrentMap ))) ;
			GV_SetActorChild( parent,( work->light_r =
				NewCircleLight(&BODYWORLD( body, HUMAN21_MIGI_TE),&work->sw_light_r, n,&GM_CurrentMap ))) ;
		}
	}
	if ( flag & EQUIP_LEFT ) {
		if ( left == 0 ) left = AKS_SP ;
		GV_SetActorChild( parent,( work->left =
			NewConnectObject( body, HUMAN21_HIDARI_TE, NULL, NULL, left ))) ;
		if ( flag & EQUIP_LIGHT_L ) {
			int n ;

			n = 0 ;
			work->sw_light_l = 1 ;
			switch( left ) {
				case AKS :
					n = 1 ;
				break ;
				case AKS_SP :
					n = 0 ;
				break ;
				case FLASHLIGHT :
					n = 4 ;
				break ;
			}
			GV_SetActorChild( parent,( work->light_l =
				NewCircleLight(&BODYWORLD( body, HUMAN21_HIDARI_TE) ,&work->sw_light_l, n,&GM_CurrentMap ))) ;
		}
	}
printf("eneequip: set flag [%x] \n", flag);
	
	return 0 ;
}
/*----------------------------------------------------------------*/
void	*NewEneEquip( int name, void *parent, OBJECT *body, int flag, int wpname, int left_wp )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		GV_SetActorFreeFunc( &( work->actor ), GV_DelayedFree ) ;
		if ( GetResources( work, name, parent, body, flag, wpname, left_wp ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}


/*

警備兵セット
	GV_SetActorChild( work, NewEneEquip( 0, work, body, 0x13b, GV_StrCode( "aks_sp" ), 0 ) ) ;

攻撃兵セット
	GV_SetActorChild( work, NewEneEquip( 0, work, body, 0x13b, GV_StrCode( "aks_sp" ), 0 ) ) ;

*/
