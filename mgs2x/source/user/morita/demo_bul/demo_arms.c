//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	arms_con.c
	デモ用武器エフェクトコントロール
	
	2000/01/18 T.Shibata
	
	$Id: demo_arms.c,v 1.1.1.3 2002/11/19 11:45:55 Yoshizawa1 Exp $

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

#include 	"libutl.h"
#include 	"../../mode/demo/eft_con.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include	"../include/util.h"

#include	"../../shibata/util/ts_util.h"
#include	"../../skoba/weapon/arms_data.h"

enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


extern void *NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
extern void	*NewBulletM4_demo( FMATRIX * , int ); 
extern void *NewLineSmokeMng( FMATRIX * , FVECTOR * , int );
extern void	*NewBulletStinger( FMATRIX * , CONTROL * , int , int );
extern void *NewStgSight( void );
extern void	*NewBulletRGB6( FMATRIX *, CONTROL *, int, int ) ;

extern void	AN_CartridgeUSP_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void AN_CartridgeAKS_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void AN_CartridgeFMS_E(FMATRIX *, OBJECT *, CONTROL * );
extern void AN_CartridgeMKR_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void AN_CartridgeP90_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void AN_CartridgeP90_SOL_E( FMATRIX * , OBJECT * , CONTROL * );
extern void AN_CartridgeM4_demo_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void AN_CartridgeM4_demo_gun_sel_E( FMATRIX * , OBJECT * , CONTROL * );
extern void AN_CartridgeM4_demo_gun_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void AN_CartridgeM4_demo_gun_grenade_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void AN_CartridgeM4_demo_gun_sel_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void AN_CartridgeSAA_LEFT_E( FMATRIX * , OBJECT * , CONTROL * );
extern void AN_CartridgeSAA_RIGHT_E( FMATRIX * , OBJECT * , CONTROL * );
extern void AN_CartridgeSOCOM_E( FMATRIX * , OBJECT * , CONTROL * );
extern void AN_CartridgeM92_Sea_E( FMATRIX * , OBJECT * , CONTROL * );
extern void	AN_CartridgeM4A1_SNK_E( FMATRIX * , OBJECT * , CONTROL * );
extern void AN_CartridgeM4_Snk_Grenade_E( FMATRIX * , OBJECT * , CONTROL * );
extern void	AN_CartridgeM4A1_HI_E( FMATRIX * , OBJECT * , CONTROL * );
extern void	AN_CartridgePSG_E( FMATRIX * , OBJECT * , CONTROL * );

extern void	MazzleUSP( FMATRIX *, int, int ) ;
extern void MazzleAKS( FMATRIX *, int, int ) ;
extern void	MazzleFAMAS( FMATRIX *, int, int ) ;
extern void MazzleMKR( FMATRIX *, int, int ) ;
extern void MazzleP90( FMATRIX *, int, int );
extern void MazzleP90_Solidas( FMATRIX * , int , int );
extern void MazzleM4demo( FMATRIX *, int, int );
extern void MazzleM4demo_sel( FMATRIX *, int, int );
extern void MazzleM4demo_gun( FMATRIX *, int, int );
extern void MazzleM4demo_gun_grenade( FMATRIX *, int, int );
extern void MazzleM4demo_gun_sel( FMATRIX *, int, int );
extern void MazzleSAA( FMATRIX *, int, int ); // 左
extern void MazzleSOCOM( FMATRIX *, int , int );
extern void MazzleM92_Sea( FMATRIX *, int , int );
extern void	MazzleM4A1_Snk( FMATRIX *, int , int );
extern void MazzleM4demo_Snk( FMATRIX * , int , int );
extern void MazzleM4A1_HI( FMATRIX * , int , int );
extern void MazzlePSG( FMATRIX * , int , int );
// Ray 特殊
extern void *NewRayValcanFire( FMATRIX *world, int side, int type );
extern void *NewRayValcanFireNoVRArea( FMATRIX *world, int side, int type ); // 跳弾エフェクト有り無し

extern void *NewLineSmoke( FMATRIX *world, FVECTOR *shift, int life, float pow, float max_size );

enum {
	MSG_ARMSCON_MUZZLE = 0,		//[1]薬莢有り[2]弾丸軌跡有り[3]兆弾有り[4]添付有
	MSG_ARMSCON_AFTSMOKE,		//[1]種類
};

#define	EFFECT_FLAGS_AMO	(0x01)
#define	EFFECT_FLAGS_LINE	(0x02)
#define	EFFECT_FLAGS_REC	(0x04)
#define	EFFECT_FLAGS_ON_TMP	(0x08)
#define EFFECT_FLAGS_ON_SPARK (0x10)
#define EFFECT_FLAGS_SAA_LEFT (0x20)
#define EFFECT_FLAGS_NO_MZL (0x40)

enum {
	AFTSMOKE_KIND_LINE = 0,
	AFTSMOKE_KIND_BOTTOM,
};

typedef	struct	{
	GV_ACT_EX	actor;
	int			name;
	int			arms_id;
	FMATRIX		*world;
        EFTCONTROL       control ;
} Work;


// プロトタイプ
static void Muzzle_HandGunDefault( Work * , int, int );
static void Muzzle_LongGunDefault( Work * , int, int );
static void Muzzle_SpGunDefault( Work * , int, int );
static void Muzzle_MKR( Work * , int, int );
static void Muzzle_AKS( Work * , int, int );
static void Muzzle_M92_SNA( Work * , int, int );
static void Muzzle_P90( Work * , int, int );
static void Muzzle_P90_SOL( Work * , int, int );
static void Muzzle_STG( Work * , int, int );
static void Muzzle_M4B_GL( Work * , int , int );
static void Muzzle_M4B_GL_SEL( Work * , int , int );
static void Muzzle_M4B_GUN( Work * , int , int );
static void Muzzle_M4B_GUN_GRENADE( Work * , int , int );
static void Muzzle_M4B_GUN_SEL( Work * , int , int );
static void Muzzle_SAA( Work * , int , int );
static void Muzzle_SOCOM( Work * , int , int );
static void Muzzle_M92_Sea( Work * , int , int );
static void Muzzle_M4A1_GL_SNK( Work * , int , int );
static void Muzzle_M4A1_GUN_SNK( Work * , int , int );
static void Muzzle_M4A1_GUN_HI( Work * , int , int );
static void Muzzle_PSG( Work * , int , int );
static void Muzzle_Ray_Vlcn( Work * , int , int );


static void (*ArmaMuzzleFunc[ARMS_DATA_BOTTOM])( Work * , int, int ) = {
	Muzzle_M92_SNA,
	Muzzle_HandGunDefault,
	Muzzle_SOCOM, // socom
	Muzzle_HandGunDefault, // socom_snk なさげ
	Muzzle_MKR,
	Muzzle_M92_Sea,
	Muzzle_SAA,

	Muzzle_LongGunDefault,
	Muzzle_LongGunDefault,
	Muzzle_AKS,
	Muzzle_AKS,
	Muzzle_M4B_GUN,
	Muzzle_P90,

	Muzzle_STG,
	Muzzle_M4B_GL,
	Muzzle_SpGunDefault,
	Muzzle_M4B_GUN,
	Muzzle_M4B_GUN_GRENADE,
	Muzzle_M4B_GUN_SEL,
	Muzzle_M4B_GL_SEL,
	Muzzle_M4A1_GUN_SNK,
	Muzzle_M4A1_GL_SNK,
	Muzzle_M4A1_GUN_HI,

	Muzzle_PSG,
	Muzzle_P90_SOL,
	Muzzle_Ray_Vlcn,
};

static FVECTOR	_MuzzlePos[ARMS_DATA_BOTTOM] = {
	{ 17.5f, -420.0f, 79.5f, 1.0f},
	
	{ 17.5f, -270.5f, 70.5f, 1.0f}, // usp
	{ 17.5f, -274.0f, 78.7f, 1.0f}, // socom
	{ 17.5f, -461.0f, 78.7f, 1.0f}, // socom_snk あるの？
	{ 20.5f, -233.5f, 54.5f, 1.0f}, // mkr
	{ 18.0f, -273.5f, 60.0f, 1.0f},	// m92_sea
	{ 22.5f, -322.0f, 68.0f, 1.0f}, // saa

	{ 19.5f, -527.5f, 85.5f, 1.0f}, // famas いらない
	{ 19.5f, -774.0f, 77.0f, 1.0f}, // abkan まだ
	{ 19.5f, -726.0f, 74.0f, 1.0f}, // aks
	{ 19.5f, -726.0f, 74.0f, 1.0f},	// aks_s?
	{ 20.0f, -654.0f, 92.0f, 1.0f}, // m4_nm m4gunのこと
	{ 17.5f, -395.f, 57.5f , 1.0f}, // p90

	{ 17.5f, -641.0f,112.7f, 1.0f},	// stinger 注意
	{ 20.0f, -511.0f, 44.0f, 1.0f}, // m4_gl
	{ 21.0f, -1080.0f, 99.0f, 1.0f},// linia まだ
	{ 20.0f, -654.0f, 92.0f, 1.0f}, // m4gun

	{ 20.0f, -557.0f, 99.0f, 1.0f}, // m4grenade
	{  5.0f, -690.0f, 79.0f, 1.0f}, // m4sel
	{  5.0f, -670.0f, 24.0f, 1.0f}, // m4_gl_sel
	{ 20.0f, -670.0f, 82.0f, 1.0f}, // m4_gun_snk
	{ 20.0f, -653.0f, 27.0f, 1.0f}, // m4_gun_snk_gl
	{ 20.0f, -675.0f, 80.0f, 1.0f}, // ハイテク

	{ 17.5f, -954.0f, 89.7f, 1.0f}, // psg1
	{ 17.0f, -321.0f, 60.0f, 1.0f}, // p90_Sol
	{  0.0f,    0.0f,  0.0f, 1.0f}, // Ray_Vlcn
};
  
// 	{ 17.5f, -310.0f, 57.0f, 1.0f}, p90

static FVECTOR	_BulletPos[ARMS_DATA_BOTTOM] = {
	{ 17.5f, -138.0f, 72.0f, 1.0f},

	{ 17.5f, -130.0f, 70.5f, 1.0f},
	{ 17.5f, -102.0f, 78.7f, 1.0f},
	{ 17.5f, -102.0f, 78.7f, 1.0f},
	{ 20.5f, -120.0f, 54.5f, 1.0f},
	{ 17.5f, -270.5f, 70.5f, 1.0f},	//まだ無い
	{ 22.5f, -144.0f, 68.0f, 1.0f},

	{ 20.0f,   33.0f, 102.5f, 1.0f},
	{  0.0f, -241.0f, 77.0f, 1.0f},
	{ 20.0f, -242.0f, 85.0f, 1.0f},
	{ 20.0f, -242.0f, 85.0f, 1.0f},	//要チェック
	{ 20.0f, -654.0f, 92.0f, 1.0f},
	{ 17.5f, -395.f, 57.5f , 1.0f}, // p90

	{ 17.5f, -641.0f, 112.7f, 1.0f}, //まだ無い
	{ 20.0f, -204.0f,  44.0f, 1.0f},
	{  0.0f,    0.0f,  0.0f, 1.0f},
	{  4.0f, -205.0f, 92.0f, 1.0f}, // m4gun

	{ 20.0f, -557.0f, 99.0f, 1.0f}, // m4grenade
	{  5.0f, -690.0f, 79.0f, 1.0f}, // m4sel
	{  5.0f, -670.0f, 24.0f, 1.0f}, // m4_gl_sel
	{ 20.0f, -670.0f, 82.0f, 1.0f}, // m4_gun_snk
	{ 20.0f, -653.0f, 27.0f, 1.0f}, // m4_gun_snk_gl
	{ 20.0f, -675.0f, 80.0f, 1.0f}, // ハイテク

	{ 17.5f, -954.0f, 89.7f, 1.0f}, // psg1
	{ 17.0f, -321.0f, 60.0f, 1.0f}, // p90_Sol
	{  0.0f,    0.0f,  0.0f, 1.0f}, // Ray_Vlcn
};

static void AftEftLineSmoke( FMATRIX *world, int id, int kind, int temp0, int temp1, int temp2 )
{
	FVECTOR *shift;

	if(!kind) shift = &_MuzzlePos[id];
	else shift = &_BulletPos[id];
	NewLineSmoke( world, shift, temp0, (float)temp1, (float)temp2 );
}

static void (*AftEftFunc[AFTSMOKE_KIND_BOTTOM])( FMATRIX *, int, int, int, int, int ) = {
	AftEftLineSmoke,
};

static void MakeMatrix( Work *work, FMATRIX *dir )
{
    FMATRIX newdir ;

    _sceVu0CopyMatrix( &newdir, &DG_UnitMatrix ) ;
    _sceVu0CopyVector( (FVECTOR*)newdir.m[W], (FVECTOR*)dir->m[W] ) ;
    _sceVu0SubVector( (FVECTOR*)newdir.m[Y], (FVECTOR*)newdir.m[W], &work->control.mov ) ;
    _sceVu0Normalize( (FVECTOR*)newdir.m[Y], (FVECTOR*)newdir.m[Y] ) ;
    _sceVu0CopyMatrix( dir, &newdir ) ;
}


static void MakeMatrix2( Work *work, FMATRIX *dir )
{
    FMATRIX newdir ;

    _sceVu0CopyMatrix( &newdir, &DG_UnitMatrix ) ;
    _sceVu0CopyVector( (FVECTOR*)newdir.m[W], (FVECTOR*)dir->m[W] ) ;
    _sceVu0SubVector( (FVECTOR*)newdir.m[X], (FVECTOR*)newdir.m[W], &work->control.mov ) ;
    _sceVu0Normalize( (FVECTOR*)newdir.m[X], (FVECTOR*)newdir.m[X] ) ;
    _sceVu0CopyMatrix( dir, &newdir ) ;
}

static void Muzzle_Ray_Vlcn( Work *pWork , int id , int flags )
{
	static int vlcn_pos;
	char light_flag;
	int  side_flag;
	FMATRIX	temp;

	light_flag = 0;
	vlcn_pos++;
	// err 防止
	if ( vlcn_pos < 0 ){
		vlcn_pos = 0;
	}
	vlcn_pos %= 3;
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	side_flag = 0; // default right
	if ( flags & EFFECT_FLAGS_SAA_LEFT ){
		side_flag = 1;
	}
	DG_COPY_MAT( &temp, pWork->world );
	MakeMatrix2( pWork, &temp ) ;
	if ( flags & EFFECT_FLAGS_ON_SPARK )
	{
	    NewRayValcanFireNoVRArea( &temp , side_flag , vlcn_pos );// 六角なし
	}
	else
	{
	    NewRayValcanFire( &temp, side_flag , vlcn_pos );
	}
}

//usp
static void Muzzle_HandGunDefault( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){		
		AN_CartridgeUSP_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_Usp );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleUSP( pWork->world, 0, light_flag );
}

// Socom
static void Muzzle_SOCOM( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if( flags & EFFECT_FLAGS_AMO ){		
		AN_CartridgeSOCOM_E( pWork->world, NULL, NULL );
	}
	if( flags & ( EFFECT_FLAGS_LINE | EFFECT_FLAGS_REC ) ){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if( flags & EFFECT_FLAGS_LINE ) bullet_flags |= BUL_TYPE_VISIBLE;
		if( flags & EFFECT_FLAGS_REC ) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_Socom );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleSOCOM( pWork->world , 0 , light_flag );
}

// M92_Sea 火花あり!!
static void Muzzle_M92_Sea( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if( flags & EFFECT_FLAGS_AMO ){		
		AN_CartridgeM92_Sea_E( pWork->world, NULL, NULL );
	}
	if( flags & ( EFFECT_FLAGS_LINE | EFFECT_FLAGS_REC ) ){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[ id ] , (FVECTOR *)temp.m[ 3 ] , 1 );
		if( flags & EFFECT_FLAGS_LINE ) bullet_flags |= BUL_TYPE_VISIBLE;
		if( flags & EFFECT_FLAGS_REC ) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m92 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	if ( flags & EFFECT_FLAGS_ON_SPARK ){
		light_flag |= EFFECT_FLAGS_ON_SPARK;
	}
	MazzleM92_Sea( pWork->world , 0 , light_flag );
}

//マカロフ
static void Muzzle_MKR( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){		
		AN_CartridgeMKR_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_Usp );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleMKR( pWork->world, 0, light_flag );
}

//ファマス
static void Muzzle_LongGunDefault( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeFMS_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleFAMAS( pWork->world, 0, light_flag );
}

//ＡＫＳ
static void Muzzle_AKS( Work *pWork , int id, int flags )
{
	char light_flag;
 
	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeAKS_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleAKS( pWork->world, ((id==ARMS_DATA_AKS)?0:1), light_flag );
}

// psg1 -snake -raiden
static void Muzzle_PSG( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgePSG_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzlePSG( pWork->world, ((id==ARMS_DATA_PSG)?0:1) , light_flag );
}

// M４ -snake
static void Muzzle_M4A1_GUN_SNK( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeM4A1_SNK_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleM4A1_Snk( pWork->world, ((id==ARMS_DATA_M4A1_GUN_SNK)?0:1) , light_flag );
}

// M４ high tech
void Muzzle_M4A1_GUN_HI( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeM4A1_HI_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleM4A1_HI( pWork->world, ((id==ARMS_DATA_M4A1_GUN_HI)?0:1) , light_flag );
}

// m4a1_gl_snk グレネードランチャー
static void Muzzle_M4A1_GL_SNK( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeM4_Snk_Grenade_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBulletRGB6( &temp, NULL , BOTH_SIDE , 0 );
//		NewBulletM4_demo( &temp , BOTH_SIDE );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}

	//	if(!(flags&EFFECT_FLAGS_NO_MZL));
	MazzleM4demo_Snk( pWork->world, ((id==ARMS_DATA_M4A1_GL_SNK)?0:1), light_flag );
}

// Ｐ９０
static void Muzzle_P90( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeP90_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleP90( pWork->world, ((id==ARMS_DATA_P90)?0:1), light_flag );
}

// P90ソリダス用
static void Muzzle_P90_SOL( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeP90_SOL_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 20000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleP90_Solidas( pWork->world, ((id==ARMS_DATA_P90)?0:1), light_flag );
}

//スティンガー
static void Muzzle_STG( Work *pWork , int id, int flags )
{
	if(flags&EFFECT_FLAGS_AMO){ // 変わりにサイトをだす
		// スティンガーに薬莢はありません
		GV_SetActorChild( pWork , NewStgSight() );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBulletStinger( &temp , NULL , BOTH_SIDE , 0 );
	}
	if(flags&EFFECT_FLAGS_NO_MZL){
		GV_CallChildSignalFunc( pWork , 0x02 , id );
	}
}

//special
static void Muzzle_SpGunDefault( Work *pWork , int id, int flags )
{
	printf("銃のエフェクトまだできてませーん。\n");
}

//グレネードランチャー
static void Muzzle_M4B_GL( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeM4_demo_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBulletM4_demo( &temp , BOTH_SIDE );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}

	//	if(!(flags&EFFECT_FLAGS_NO_MZL));
	MazzleM4demo( pWork->world, ((id==ARMS_DATA_M4B_GL)?0:1), light_flag );
}

//グレネードランチャー シールズ用
static void Muzzle_M4B_GL_SEL( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeM4_demo_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[ id ] , (FVECTOR *)temp.m[ 3 ] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBulletM4_demo( &temp , BOTH_SIDE );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}

	//	if(!(flags&EFFECT_FLAGS_NO_MZL));
	MazzleM4demo_sel( pWork->world, ((id==ARMS_DATA_M4B_GL)?0:1), light_flag );
}

// m4上 海兵隊m4a_nm, m4b_gl, m4c_sc
static void Muzzle_M4B_GUN( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeM4_demo_gun_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleM4demo_gun( pWork->world , 0 , light_flag );
}

// m4上 海兵隊demo_m4_grenade_usm 追加
static void Muzzle_M4B_GUN_GRENADE( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeM4_demo_gun_grenade_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleM4demo_gun_grenade( pWork->world , 0 , light_flag );
}

// m4上 シールズ用 demo_m4_sel 追加
void Muzzle_M4B_GUN_SEL( Work *pWork , int id, int flags )
{
	char light_flag;

	light_flag = 0;
	if(flags&EFFECT_FLAGS_AMO){
		AN_CartridgeM4_demo_gun_sel_E( pWork->world, NULL, NULL );
	}
	if(flags&(EFFECT_FLAGS_LINE|EFFECT_FLAGS_REC)){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_m4 );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	MazzleM4demo_gun_sel( pWork->world , 0 , light_flag );
}

// Single Action Army
static void Muzzle_SAA( Work *pWork , int id, int flags )
{
	char light_flag;
	char silence_flag;

	light_flag = silence_flag = 0;
	if(flags & EFFECT_FLAGS_AMO){
		if ( flags & EFFECT_FLAGS_SAA_LEFT ){
			AN_CartridgeSAA_LEFT_E( pWork->world, NULL, NULL );
		} else {
			AN_CartridgeSAA_RIGHT_E( pWork->world, NULL, NULL );
		}
	}
	if(flags & ( EFFECT_FLAGS_LINE | EFFECT_FLAGS_REC ) ){
		FMATRIX	temp;
		u_int 	bullet_flags = BUL_TYPE_SCAR;

		DG_COPY_MAT( &temp, pWork->world );
		DG_SetPos( pWork->world );
		DG_PutVector( &_MuzzlePos[id] , (FVECTOR *)temp.m[3] , 1 );
		if(flags&EFFECT_FLAGS_LINE) bullet_flags |= BUL_TYPE_VISIBLE;
		if(flags&EFFECT_FLAGS_REC) bullet_flags |= BUL_TYPE_SPARK;

		MakeMatrix( pWork, &temp ) ;
		NewBullet( &temp, bullet_flags, BOTH_SIDE, 25, 0, 30000, 1000, WP_Aks );
	}
	if ( flags & EFFECT_FLAGS_ON_TMP ){
		light_flag |= EFFECT_FLAGS_ON_TMP;
	}
	if ( flags & EFFECT_FLAGS_ON_SPARK ){
		light_flag |= EFFECT_FLAGS_ON_SPARK;
	}
	if ( flags & EFFECT_FLAGS_SAA_LEFT ){
		silence_flag |= EFFECT_FLAGS_SAA_LEFT;
	}
	MazzleSAA( pWork->world , silence_flag , light_flag );
}
// mgdemo P010_01_p01 P010_01_p01.w14a loop

static void Muzzle_M92_SNA( Work *pWork , int id, int flags )
{
	printf("麻酔銃のエフェクト無いです。\n");
}

static void Act( Work *work )
{
    GV_MSG *msg;
    int mes_num;
    int flags = 0;
	
    mes_num = GV_ReceiveMessage( work->name, &msg );

    msg += mes_num-1;
    while( --mes_num >= 0 )
    {
	switch( msg->message[0] )
	{
	case MSG_ARMSCON_MUZZLE:
	    if(msg->message[1]) flags |= EFFECT_FLAGS_AMO;
	    if(msg->message[2]) flags |= EFFECT_FLAGS_LINE;
	    if(msg->message[3]) flags |= EFFECT_FLAGS_REC;
	    if(msg->message[4]) flags |= EFFECT_FLAGS_ON_TMP;
	    if(msg->message[5]) flags |= EFFECT_FLAGS_ON_SPARK;
	    if(msg->message[6]) flags |= EFFECT_FLAGS_SAA_LEFT;
	    ArmaMuzzleFunc[work->arms_id]( work , work->arms_id, flags );
	    break;
	case MSG_ARMSCON_AFTSMOKE:
	    AftEftFunc[msg->message[1]]( work->world,
					 work->arms_id,
					 msg->message[2],
					 msg->message[3],
					 msg->message[4],
					 msg->message[5] );
	    break;
	default:
	    printf("Msg Err!! : NewDemoArmControl\n");
	    break;
	}
	msg--;
    }

    GV_WaitMessage( work, work->name );
}

static void Die( Work *work  )
{
    DM_EftControlDelList( &work->control ) ;
}

void *NewDemoArmControl( int name, int arms_id, FMATRIX *world, int conname )
{
	Work *work ;

	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof(Work) ) ;
	if ( work != NULL )
	{
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		
		work->name    = name    ;
		work->arms_id = arms_id ;
		work->world   = world   ;
		DM_EftControlAddList( conname, &work->control ) ;

		if( arms_id >= ARMS_DATA_BOTTOM )
		{
		    printf("ID too big %d : NewDemoArmControl\n", arms_id ) ;
		    return NULL ;
		}
	}

	return (void *)work ;
}


