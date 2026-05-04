//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   usp.c
   Ｍ９２／ＵＳＰ／ＳＯＣＯＭ／ＦＡＭＡＳ／ＳＰＰ
   
   1999/10/04 M.Sonoyama
   $Id: usp.c,v 1.1.1.3 2002/11/19 11:50:32 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>


#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"

#include 	"../weapon_old/matsu.h"

extern void *NewUSPLight( FMATRIX *mat, float umbra, float penumbra, float range, int color );

/*------------------------------------------------------------------*/

extern void	MazzleUSP( FMATRIX *, int, int ) ;
extern void	MazzleSOCOM( FMATRIX *, int, int ); 
extern void	MazzleFAMAS( FMATRIX *, int, int ) ;
extern void	MazzleM4A1( FMATRIX *, int, int ) ;
extern void	MazzleAKS_RAI( FMATRIX *, int, int ) ;
extern void	AN_CartridgeUSP_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void	AN_CartridgeSOCOM_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void	AN_CartridgeFMS_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void	AN_CartridgeM4A1_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void	AN_CartridgeAKS_RAI_E( FMATRIX *, OBJECT *, CONTROL * ) ;
extern void	*NewCartridgeControl( int *, OBJECT *, OBJECT *, int ) ;
//extern  void    MazzleMeca( FMATRIX *world, FVECTOR *shift );

extern void	MazzleM92_Sea( FMATRIX *, int, int ) ;
extern void	AN_CartridgeM92_Sea_E( FMATRIX *, OBJECT *, CONTROL * ) ;

static void	*SpprprSeDelay( void *parent ) ;
/*------------------------------------------------------------------*/

//#define	DEFAULT_ROT	(-14.4898F)
#define	DEFAULT_ROT	(0.0F)
#define	DEFAULT_ROT_SUB	(-2.0F)

#define	BODY_FLAG	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC | \
					 DG_FLAG_ONEPIECE)
//#define	BODY_FLAG_SUB	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC)
#define	BODY_FLAG_SUB	(DG_FLAG_SHADE)

#define	TEXTURE		TXT_LSIGHT_LINE
#define	TEXTURE2	TXT_LSIGHT_POINT

#define	PRIM_FLAG1	(DG_PRIM2_LINE | DG_PRIM2_SHADE | DG_PRIM2_ANTIALIASING | DG_PRIM2_TEX)
#define	PRIM_FLAG2	(DG_PRIM2_SPRT | DG_PRIM2_TEX | DG_PRIM2_FOG | DG_PRIM2_ALPHA)

#define	N_PRIMS1	(1)
#define	N_VERTS1	(2)

#define	N_PRIMS2	(2)
#define	N_VERTS2	(1)

#define	LENGTH		(20000.0F)
//#define	LENGTH_SUB	(20000.0F)
static	float	LENGTH_SUB = 40000.0F ;
static  char    SK_LightFlag = 0;

#define	TARGET_CLASS	(TARGET_OFFENSE | TARGET_SEEK | TARGET_ONLINE | TARGET_ONLINE_MIN | \
						 TARGET_CHILD)
#define	TARGET_CLASS2	(TARGET_OFFENSE | TARGET_SEEK | TARGET_ONLINE | TARGET_ONLINE_MIN)

#define	BULLET_TYPE		(BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_NOISE_S)
#define	BULLET_TYPE2	(BUL_TYPE_SPARK|BUL_TYPE_NOISE_SS)
#define	BULLET_TYPE_SPP	(BUL_TYPE_SPARK|BUL_TYPE_SCAR|BUL_TYPE_VISIBLE)

#define	USP_MOTION	(123600)	/* usp */

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define MEM_ADDR1	((void*)( SCRPAD_ADDR + 0x0000 ))
#define MEM_ADDR2	((void*)( SCRPAD_ADDR + 0x2000 ))

enum {
    THIS_IS_M92 = 0,
    THIS_IS_USP,	
    THIS_IS_SOCOM,
    THIS_IS_FAMAS,
	THIS_IS_SPP,
	THIS_IS_AKS,
	THIS_IS_M4,
	THIS_IS_AKS_SP,
	THIS_IS_SOCOM_SP,
	THIS_IS_USP_SP,
} ;

enum {
	EFFECT_FLAGS_AMO    = (0x01),
	EFFECT_FLAGS_LINE   = (0x02),
	EFFECT_FLAGS_REC    = (0x04),
	EFFECT_FLAGS_ON_TMP	= (0x08),
	EFFECT_FLAGS_NO_MZL	= (0x40),
};

enum {
	EFFECT_MAIN_VIEW = 0x100,
};

static	int	BodyDatas[] = {
	MDL_M92,
	MDL_M92_SUB,
	MDL_USP,
	MDL_USP_SUB,
	MDL_SOCOM,
	MDL_SOCOM_SUB,
	MDL_FAMAS,
	MDL_FAMAS_SUB,
	MDL_SPP,
	MDL_SPP_SUB,
	MDL_AKS_RAI,
	MDL_AKS_RAI_SUB,
	MDL_M4A_RAI,
	MDL_M4A_RAI_SUB,
	MDL_AKS_RAI_SP, 
	MDL_AKS_RAI_SP_SUB,
	MDL_SOCOM_SP, 
	MDL_SOCOM_SP_SUB,
	MDL_USP_SP,
	MDL_USP_SP_SUB,
} ;

static	int	WpNum[] = {
    WP_m92,
    WP_Usp,
    WP_Socom,
    -1,	/* ファマスは没 */
	-1,	/* ＳＰＰは没 */
	WP_Aks,
	WP_m4,
	WP_Aks,
	WP_Socom,
	WP_Usp
} ;

static	int	BulletTypes[] = {
    BULLET_TYPE2,
    BULLET_TYPE,
    BULLET_TYPE,
    BULLET_TYPE,
	BULLET_TYPE_SPP,
    BULLET_TYPE,
    BULLET_TYPE,
    BULLET_TYPE,
    BULLET_TYPE,
    BULLET_TYPE,
    BULLET_TYPE
} ;

static	int	Magazines[] = {
    15,							/* m92 */
    15,							/* usp */
    12,							/* scm */
    25,							/* fms */
	4,							/* spp */
	30,							/* aks */
	30,							/* m4 */
	30,							/* aks_sp */
	12,							/* scm_sp */
	15							/* usp_sp */
} ;

static	FVECTOR	LaserPoints[] = {
    { 17.5F, -281.0F, 19.7F },	/* m92 */
    { 17.5F, -370.0F, 69.7F },	/* m92_sub */
    { 17.5F, -270.5F, -7.0F },	/* usp */
    { 17.0F, -250.0F, 67.0F },	/* usp_sub */
    { 26.5F, -260.0F, 34.7F },	/* scm */
    { 17.5F, -274.0F, 58.7F + 20.0F },	/* scm_sub */
    { 17.0F, -500.0F, 57.0F },	/* fms */    
    { 17.0F, -500.0F, 17.0F },	/* fms_sub */
	{ 17.5F, -280.0F, 44.7F },	/* spp */
	{ 17.5F, -280.0F, 70.7F },	/* spp_sub */
    { 20.0F, -538.0F, 57.0F },	/* aks_sp */    
    { 20.0F, -538.0F, 57.0F },	/* aks_sp_sub */
    { 47.0F, -538.0F, 92.0F },	/* m4 */
    { 47.0F, -538.0F, 92.0F },	/* m4_sub */
    { 20.0F, -538.0F, 57.0F },	/* aks_sp */    
    { 20.0F, -538.0F, 57.0F },	/* aks_sp_sub */
    { 26.5F, -260.0F, 34.7F },	/* scm_sp */
    { 17.5F, -481.0F, 58.7F + 20.0F },	/* scm_sp_sub */
    { 17.5F, -270.5F, -7.0F },	/* usp_sp */
    { 17.0F, -250.0F, 67.0F },	/* usp_sp_sub */
} ;

extern void	*NewBullet( FMATRIX *, u_int, u_int, u_int, u_int, u_int, u_int, int ) ;
extern void 	*NewSpotLight( FMATRIX *mat, float umbra, 
							  float penumbra, float range, int color ) ;
extern void	*NewCircleLight( FMATRIX *, int *, int, int * ) ;

/*------------------------------------------------------------------*/

static	short	LightR = 230, LightG = 190, LightB = 120 ;
static	short	Light2R = 80, Light2G = 55, Light2B = 40 ;
//static	float	Light2SP1 = 1250.0F, Light2SP2 = 2500.0F ;

static	int		LightOn = 0 ;

#define	USP_LightOn		PL_USP_LightOn

/*------------------------------------------------------------------*/

typedef	struct Work_usp_t{   //BP_GENERAL - added name so you can debug on PC
    GV_ACT_EX	actor ;
    OBJECT		weapon ;
    OBJECT		weapon_sub ;
    
    TARGET		seek ;

    FVECTOR		step ;
    FVECTOR		hit ;
    FMATRIX		world ;

    FVECTOR		from ;

    CONTROL		*ctrl ;
    OBJECT		**body ;
    DG_PRIM2		*laser ;
    DG_PRIM2		*light ;
    DG_PRIM2		*laser_ir ;		/* 赤外線ゴーグル使用時用 */
    DG_PRIM2		*light_ir ;		/* 赤外線ゴーグル使用時用 */

    WEAPON_EF_CTRL	bctrl ;
    WEAPON_EF_CTRL	bctrl_sub ;
    
    float		len ;
    int			*unit ;/* 関節番号 */
    u_int		*trigger ;/* なんか番号 */
    int			side ;/* 不明 */

    DG_TEX		*tex ;
    float		cur_v0 ;
    int			time ;
    int			speed ;

    int			silencer ;
    void		*gunlight ;
    int			this ;
#if 0
    int			zdepth ;
#endif
	int			crtrgctrl ;
	int			crtrgctrl_sub ;
    int			motion ;
    int			circle_light ;
	int			chanl ;
	int			vflag ;
	int			connected ;
} Work ;

/*------------------------------------------------------------------*/

extern void	UTL_StartSprToMem( void *, void *, int ) ;
extern void	UTL_StartMemToSpr( void *, void *, int ) ;
extern void	UTL_EndSprToMem( void ) ;
extern void	UTL_EndMemToSpr( void ) ;
extern void		*NewBulletM4_demo( FMATRIX *world , int side ) ;
//extern void *NewNameLayout( int );
//extern void *NewQuestion( void );

static	inline	void	SprToMem( dst, src, size, num )
void		*dst, *src ;
int		size, num ;
{
    UTL_StartSprToMem( dst, src, size * num / sizeof( u_long128 ) ) ;
    UTL_EndSprToMem() ;
}

static	inline	void	MemToSpr( dst, src, size, num )
void		*dst, *src ;
int		size, num ;
{
    UTL_StartMemToSpr( dst, src, size * num / sizeof( u_long128 ) ) ;
    UTL_EndMemToSpr() ;
}

/*------------------------------------------------------------------*/

#if 0
#define	ZMANAGER	(7290139)	/* Z_manager */
#define	ZDEPTH_SET	(2)
#define	ZDEPTH_ON	(12)
#define	ZDEPTH_OFF	(22)

#define	DEPTH		(7622382)	/* depth */
#define	DEPTH_ON	(12)
#define	DEPTH_OFF	(22)
#endif

#if 0
/* Ｚデプス変更 */
static	void	ZdepthSet( work, time )
Work		*work ;
int		time ;
{
    GV_MSG	msg ;
    int		buf[ 4 ] ;

    if ( !( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE ) ) ) return ;

    msg.address = ZMANAGER ;
    msg.message_len = 4 ;
    msg.message = buf ;
    buf[ 0 ] = ZDEPTH_SET ;
    buf[ 1 ] = ( int )work->len + 500 ;
    buf[ 2 ] = 60000 ;
    buf[ 3 ] = time ;
    GV_SendMessage( &msg ) ;
}

/* Ｚデプス管理 */
static	void	ZdepthSwitch( work, mode )
Work		*work ;
int		mode ;
{
    GV_MSG	msg ;
    int		buf[ 3 ] ;

    if ( !( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE ) ) ) mode = 0 ;

    if ( work->zdepth == mode ) {
		if ( mode ) ZdepthSet( work, 1 ) ;
		return ;
    }
    work->zdepth = mode ;

    msg.address = ZMANAGER ;
    msg.message_len = 1 ;
    msg.message = buf ;
    buf[ 0 ] = ( mode ) ? ZDEPTH_ON : ZDEPTH_OFF ;
    GV_SendMessage( &msg ) ;

    msg.address = DEPTH ;
    msg.message_len = 1 ;
    msg.message = buf ;
    buf[ 0 ] = ( mode ) ? DEPTH_ON : DEPTH_OFF ;
    GV_SendMessage( &msg ) ;

    if ( mode ) ZdepthSet( work, 60 ) ;
}
#endif

/* サプレッサ付きモデルにチェンジ */
static	int		ChangeModelToWithSpprsr( Work *work, int this )
{
	OBJECT		*weapon ;

	if ( GM_PlayerBody != NULL ) {
		DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
		DG_DisconnectObjs( GM_PlayerArmBody->objs, work->weapon_sub.objs ) ;
	}
	GM_FreeObject( &( work->weapon ) ) ;
	GM_FreeObject( &( work->weapon_sub ) ) ;

	weapon = &( work->weapon ) ;
	// コバ４改良サプレッサーモデル切替え時バグ修正
	switch ( this ){
	case THIS_IS_SOCOM_SP :
	case THIS_IS_USP_SP :
		WeaponEfInitObject( &( work->bctrl ), &( work->weapon ), GM_PlayerBody, HUMAN21_MIGI_TE,
						   WpNum[ work->this ], BodyDatas[ this * 2 ] ) ;
		WeaponEfInitObject( &( work->bctrl_sub ), &( work->weapon_sub ), GM_PlayerArmBody, HUMAN21_MIGI_TE,
						   WpNum[ work->this ], BodyDatas[ this * 2 + 1 ] ) ;
		if ( work->weapon.objs == NULL || work->weapon_sub.objs == NULL ) {
			return -1 ;
		}
		SpprprSeDelay( work ) ;
		break;
	default :
		GM_InitObject( weapon, BodyDatas[ this * 2 ], BODY_FLAG ) ;
		if ( weapon->objs == NULL ) return -1 ;
		GM_ConfigObjectRoot( weapon, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
		weapon = &( work->weapon_sub ) ;
		GM_InitObject( weapon, BodyDatas[ this * 2 + 1 ], BODY_FLAG ) ;
		if ( weapon->objs == NULL ) return -1 ;
		GM_ConfigObjectRoot( weapon, GM_PlayerBody, HUMAN21_MIGI_TE ) ;
		SpprprSeDelay( work ) ;
		break;
	}

	PL_EquipWeaponCode = BodyDatas[ this * 2 + 1 ] ;

	GM_PlayerWeaponModel = BodyDatas[ this * 2 ] ;
	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon_sub ;

	if ( GM_PlayerBody != NULL ) {
		DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
		DG_ConnectObjs( GM_PlayerArmBody->objs, work->weapon_sub.objs ) ;
	}
	work->this = this ;
	work->silencer = 1 ;

	return 0 ;
}

/* レーザーサイトの長さを求める */
static	void	GetHazardLength( work, from, to )
Work		*work ;
FVECTOR		*from, *to ;
{
    FVECTOR		ptr ;
    DG_PRIM2_PACKET	*packs ;

#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW ) {
		FVECTOR		diff ;
		/* 射程150m */
		_sceVu0SubVector( &diff, to, from ) ;
		GV_LenVec3F( &diff, &diff, 0.0F, 150000.0F ) ;
		_sceVu0AddVector( to, from, &diff ) ;
	}
#endif

    packs = work->light->packet[ work->light->buffer_clock ] ;
//HZX_OnlineDebugFlag = 1 ;
#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_HAZARDCHECK_R ) {
		HZX_OnlineDebugFlag = 2 ;
	}
#endif
    if ( HZX_OnlineHazardCheck( work->ctrl->hzx_id, from, to,
							   HZX_CHK_ALL, 
							   HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
							   HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) {
		HZX_GetOnlineVector( &ptr ) ;
		HZX_GetOnlinePoint( &( work->hit ) ) ;
		packs[ 1 ].flag &= ~DG_PRIM2_INVISIBLE0 ;
#ifdef DEBUG_MODE
//		if ( GM_PlayerDebugMode == GM_PDM_RSEGMENTVIEW ) 
		if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW ) 
		{
			int		type, atr ;
			HZX_FLR	seg ;
			HZX_GetOnlineHazard( &seg, &atr ) ;
			type = HZX_GetOnlineHazardType() ;
			GM_SetCurrentMap( GM_CurrentStageMap ) ;
			if ( type == 1 ) {
				if ( GV_PadData->press & PAD_X ) {
					DumpSegment( ( HZX_SEG * )&seg ) ;
					DumpVec( &( work->hit ) ) ;
				}
				HZX_ViewSegment( ( HZX_SEG * )&seg ) ;
				HZX_ViewSeNo( &seg, &work->hit ) ;
			} else {
				if ( GV_PadData->press & PAD_X ) {
					DumpFloor( &seg ) ;
					DumpVec( &( work->hit ) ) ;
				}
				HZX_ViewFloor( &seg, HZX_GetOnlineHazardFloorType() ) ;
				HZX_ViewSeNo( &seg, &work->hit ) ;
			}
		}
#endif
	} else {
		_sceVu0SubVector( &ptr, to, from ) ;
		work->hit = *to ;
		packs[ 1 ].flag |= DG_PRIM2_INVISIBLE0 ;
	}
	work->len = GV_VecLen3F( &ptr ) ;
	if ( work->len < 610.0F ) {
		work->len = 0.0F ;
	}
#ifdef DEBUG_MODE
	if ( PlayerDebugMenuStatus & PDMS_HAZARDCHECK_R ) {
		HZX_OnlineDebugFlag = 0 ;
	}
#endif
//HZX_OnlineDebugFlag = 0 ;
}

/* レーザーのテクスチャを更新 */
static	void	UpdateLaserTex( work )
Work		*work ;
{
	DG_PRIM2_UVRGB	*uvrgb ;
	DG_TEX		*tex ;
	float		v0 ;

	uvrgb = work->laser->uvrgb[ work->laser->buffer_clock ] ;
	tex = work->tex ;
	v0 = work->cur_v0 ;
	uvrgb[ 0 ].v = FTOI12( v0 * tex->v_scale + tex->v_offset ) ;
	uvrgb[ 1 ].v = FTOI12( v0 * tex->v_scale + tex->v_offset ) ;
	work->cur_v0 += 1.0F / 256.0F * ( float )work->speed ;
	if ( work->cur_v0 >= 1.0F ) {
		work->cur_v0 = 0.0F ;
		work->speed = BP_PS2_rand() % 2 + 1 ;
	}
}

/* レーザーサイト更新 */
static	void	UpdateLaser( work, from, to ) 
Work		*work ;
FVECTOR		*from, *to ;
{
	FVECTOR		*p ;
	FVECTOR		hit2, diff ;
	float		len ;
	DG_PRIM2_UVRGBWH	*urw ;

	if ( work->len == 0.0F ) {
		DG_InvisiblePrim2( work->laser ) ;
		DG_InvisiblePrim2( work->light ) ;
		return ;
	}

	_sceVu0SubVector( &diff, &work->hit, from ) ;
#if 1
	len = GV_VecLen3F( &diff ) ;	
	if ( len > 5.0F ) {
		GV_LenVec3F( &diff, &diff, 0.0F, GV_VecLen3F( &diff ) - 5.0F ) ;
	}
#endif
	_sceVu0AddVector( &hit2, from, &diff ) ;


	p = work->laser->pos[ work->laser->buffer_clock ] ;
	p[ 0 ] = *from ;
	p[ 1 ] = hit2 ; 

	p = work->light->pos[ work->light->buffer_clock ] ;
	urw = ( DG_PRIM2_UVRGBWH * )work->light->uvrgb[ work->light->buffer_clock ] ;
	p[ 0 ] = *from ;
	p[ 1 ] = hit2 ;
	urw[ 0 ].w = urw[ 0 ].h = 4 ;
	urw[ 1 ].w = urw[ 1 ].h = ( int )( 4.0F + work->len / 400.0F ) ;
}

/* ターゲットに当たった */
static	void	HitTarget( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	FVECTOR	diff, dc ;

	work = ( Work * )ptr ;

	/* ヒット点と防御中心の間をオンラインチェック */
	GM_TargetGetCenter( &dc, def ) ;
	if ( HZX_OnlineHazardCheck( HZX_CurrentGroupID, &off->hit, &dc, HZX_CHK_ALL,
							    HZX_SEG_NO_BULLET | HZX_SEG_RECOIL_TYPE,
							    HZX_FLOOR_NO_BULLET | HZX_FLOOR_RECOIL_TYPE ) ) {
		GM_TargetHitCancel( off, def ) ;
		return ;
	}

#ifdef DEBUG_MODE
//	if ( GM_PlayerDebugMode == GM_PDM_SEGMENTVIEW ) {
	if ( PlayerDebugMenuStatus & PDMS_HAZARDVIEW ) {
		GM_SetCurrentMap( work->ctrl->map ) ;
		NewTargetView2( def, 32, 232, 32 ) ;
	}
#endif
	_sceVu0SubVector( &diff, &( off->hit ), &( off->center ) ) ;
	work->len = GV_VecLen3F( &diff ) ;
	work->hit = off->hit ;
	//printf( "hit %f %f %f\n", off->hit.vx,off->hit.vy,off->hit.vz ) ;
	if ( work->len < 610.0F ) {
		DG_InvisiblePrim2( work->laser ) ;
		DG_InvisiblePrim2( work->light ) ;
		return ;
	}


	/*レーザーポインタの可視フラグを変更する*/
	{
		DG_PRIM2_PACKET	*packs ;
		packs = work->light->packet[ work->light->buffer_clock ] ;
		packs[ 1 ].flag &= ~DG_PRIM2_INVISIBLE0 ;

		DG_VisiblePrim2Chanl( work->light, 0 ) ;
	}

	//    DG_SetPos( work->weapon.objs->root ) ;
	//    DG_PutVector( &LaserPoint, &from, 1 ) ;
	UpdateLaser( work, &work->from, &( off->offset ) ) ;
	GM_SetPlayerStatus( PLAYER_SIGHT_LOCKON ) ;


#if 0
	/* OneShotKill表示実験 */
	if ( ( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE ) ) &&
		 ( def->name == GV_StrCode( "one_shot_kill" ) ) ) {
		MENU_Locate( 432, 160, MENU_MODE_CENTER ) ;
		MENU_SetColor( 196, 196, 196 ) ;
		MENU_Printf( "One-Shot Kill!!\n" ) ;
	}
#endif

#if 0
	ZdepthSet( work, 1 ) ;
#endif
}

/* マップ更新 */
static	inline	void	UpdateMap( work )
Work		*work ;
{
	int		map ;

	map = GM_CurrentMap = work->ctrl->map ;
	work->weapon.map_name = map ;
	GM_GroupObjs( work->weapon.objs, map ) ;
	work->weapon_sub.map_name = map ;
	GM_GroupObjs( work->weapon_sub.objs, map ) ;
	GM_GroupPrim2( work->laser, map ) ;
	GM_GroupPrim2( work->light, map ) ;
}

/*------------------------------------------------------------------*/

static	void	Act( work )
Work		*work ;
{
	int		this, trgclass, vflag, chanl ;
	u_int	trg, lightcolor ;
	int     v_effect_flag;
	float	len ;
	FVECTOR	from0, from, to, diff ;
	FMATRIX	world, adj, *root ;
	FVECTOR	lp0, lp, sp, spe ;
	FVECTOR	root_pos ;

	this = work->this ;
	vflag = work->vflag ;
	chanl = work->chanl ;

	/* サプレッサーリアルタイム付け */
	//if ( this == THIS_IS_SOCOM && GM_ItemNum( IT_SocomSpprsr ) > 0 ) {
    if ( this == THIS_IS_SOCOM && PL_GetPlayerItem() == IT_SocomSpprsr ) {
		if ( ChangeModelToWithSpprsr( work, THIS_IS_SOCOM_SP ) >= 0 ) {
			GM_SetItemNum( IT_SocomSpprsr, 0 ) ;
			GM_PlayerStateFlag |= PL_SOCOM_SPPRSR_ATTACHED ;
		}
	//} else if ( this == THIS_IS_AKS && GM_ItemNum( IT_AKSpprsr ) > 0 ) {
    } else if ( this == THIS_IS_AKS && PL_GetPlayerItem() == IT_AKSpprsr ) {
		if ( ChangeModelToWithSpprsr( work, THIS_IS_AKS_SP ) >= 0 ) {
			GM_SetItemNum( IT_AKSpprsr, 0 ) ;
			GM_PlayerStateFlag |= PL_AK_SPPRSR_ATTACHED ;
		}
	//} else if ( this == THIS_IS_USP && GM_ItemNum( IT_UspSpprsr ) > 0 ) {
    } else if ( this == THIS_IS_USP && PL_GetPlayerItem() == IT_UspSpprsr ) {
		if ( ChangeModelToWithSpprsr( work, THIS_IS_USP_SP ) >= 0 ) {
			GM_SetItemNum( IT_UspSpprsr, 0 ) ;
			GM_PlayerStateFlag |= PL_USP_SPPRSR_ATTACHED ;
		}
	}

	GM_ResetPlayerStatus( PLAYER_SIGHT_LOCKON ) ;

	UpdateMap( work ) ;

	_sceVu0UnitMatrix( &adj ) ;

	if ( GM_PlayerBody->objs->flag & vflag ) { // 主観
		GM_ConfigObjectRoot( &( work->weapon_sub ), 
							*( work->body ), *( work->unit ) ) ;
		root = work->weapon_sub.objs->root ;
		DG_InvisibleObjsChanl( work->weapon.objs, chanl ) ;

		if ( ( *( work->body ) )->objs->flag & vflag ) {
			DG_InvisibleObjsChanl( work->weapon_sub.objs, chanl ) ;
		} else {
// yano
			DG_VisibleObjsChanl( work->weapon_sub.objs, chanl ) ;
//			work->weapon_sub.objs->flag &= DG_FLAG_INVISIBLE0 ;
//			work->weapon_sub.objs->flag |= DG_FLAG_INVISIBLE1 ;
//			work->weapon_sub.objs->flag &= ~DG_FLAG_INVISIBLE2 ;
		}
#if 0
		if ( this == THIS_IS_M92 ) {
			_sceVu0RotMatrixX( &adj, &adj, DEFAULT_ROT_SUB * 3.141592653589793F / 180.0F ) ;
			_sceVu0TransposeMatrix( &adj_inv, &adj ) ;
			_sceVu0ApplyMatrix( &lp, &adj_inv, &LaserPoints( this * 2 + 1 ) ) ;
		} else {
			lp = LaserPoints[ this * 2 + 1 ] ;
		}
#else
		lp = LaserPoints[ this * 2 + 1 ] ;
		v_effect_flag = EFFECT_MAIN_VIEW;
#endif
		trgclass = TARGET_CLASS ;
	} else { // 通常 
		GM_ConfigObjectRoot( &( work->weapon ), 
							*( work->body ), *( work->unit ) ) ;
		root = work->weapon.objs->root ;
		DG_VisibleObjsChanl( work->weapon.objs, chanl ) ;
		DG_InvisibleObjsChanl( work->weapon_sub.objs, chanl ) ;
#if 0
		if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
			_sceVu0RotMatrixX( &adj, &adj, DEFAULT_ROT * 3.141592653589793F / 180.0F ) ;
			_sceVu0TransposeMatrix( &adj_inv, &adj ) ;
			_sceVu0ApplyMatrix( &lp, &adj_inv, &LaserPoints[ this * 2 ] ) ;
		} else {
			lp = LaserPoints[ this * 2 ] ;
		}
#else
		lp = LaserPoints[ this * 2 ] ;
		v_effect_flag = 0;
#endif
		trgclass = TARGET_CLASS2 ;
	}

	lp0 = lp ; lp0.vy += 620.0F ; 
//	if ( this == THIS_IS_FAMAS ) lp0.vy += 100.0F ;
	if ( this == THIS_IS_FAMAS || 
		 this == THIS_IS_AKS ||
		 this == THIS_IS_M4 ) lp0.vy += 100.0F ;

	sp = lp0 ; 
	spe = lp ; 
	if ( ( GM_PlayerBody->objs->flag & vflag ) || 
		 GM_CheckPlayerStatus( PLAYER_BEHIND ) ) {
		spe.vy -= LENGTH_SUB - 100.0F ;
	} else {
		spe.vy -= LENGTH - 100.0F ;
	}

	_sceVu0CopyMatrix( &world, root ) ;
	GV_MatToVec( root, &root_pos ) ;

	if ( GM_CheckPlayerStatus( PLAYER_SNAKE ) ) {
		_sceVu0MulMatrix( &world, &world, &adj ) ;
		world.m[ 3 ][ 0 ] = root->m[ 3 ][ 0 ] ;
		world.m[ 3 ][ 1 ] = root->m[ 3 ][ 1 ] ;
		world.m[ 3 ][ 2 ] = root->m[ 3 ][ 2 ] ;
	} 
	_sceVu0CopyMatrix( &work->world, &world ) ;
	DG_SetPos( &world ) ;
	DG_InvisiblePrim2( work->laser ) ;
	DG_InvisiblePrim2( work->light ) ;
	trg = *( work->trigger ) ;
	//	work->crtrgctrl = 0 ;
	switch ( trg ) {
	case TRIG_YET :
	case TRIG_FALL :
		if ( this == THIS_IS_FAMAS ||
			 this == THIS_IS_AKS ||
			 this == THIS_IS_AKS_SP ||
			 this == THIS_IS_M4 ) {
			work->crtrgctrl = 1 ;
			work->crtrgctrl_sub = 1 ;
		}
		break ;
	case TRIG_SET :
		if ( this == THIS_IS_FAMAS ||
			 this == THIS_IS_AKS ||
			 this == THIS_IS_AKS_SP ||
			 this == THIS_IS_M4 ) {
			work->crtrgctrl = 1 ;
			work->crtrgctrl_sub = 1 ;
		}
		// else { by koba4
		DG_SwitchBuffPrim2( work->laser ) ;
		DG_SwitchBuffPrim2( work->light ) ;
		DG_SwitchBuffPrim2( work->laser_ir ) ;
		DG_SwitchBuffPrim2( work->light_ir ) ;
		DG_PutVector( &lp, &from, 1 ) ;
		DG_PutVector( &lp0, &from0, 1 ) ;
		DG_PutVector( &spe, &to, 1 ) ;
		GetHazardLength( work, &from0, &to ) ;
		work->from = from ;
		DG_VisiblePrim2Chanl( work->laser, 0 ) ;
		DG_VisiblePrim2Chanl( work->light, 0 ) ;
		UpdateLaser( work, &from, &to ) ;
		UpdateLaserTex( work ) ;
		GM_MoveOnlineTarget( &( work->seek ), &from0, &work->hit ) ;
		work->seek.class = trgclass ;
		GM_PutTarget( &( work->seek ) ) ;
		world.m[ 3 ][ 0 ] = from.vx ;
		world.m[ 3 ][ 1 ] = from.vy ;
		world.m[ 3 ][ 2 ] = from.vz ;
//		}
		work->time = 16 ;
		break ;
	case TRIG_FIRE :
		if ( this == THIS_IS_FAMAS ||
			 this == THIS_IS_AKS ||
			 this == THIS_IS_AKS_SP ||
			 this == THIS_IS_M4 ) {
			work->crtrgctrl = 1 ;
			work->crtrgctrl_sub = 1 ;
			// laser
			DG_SwitchBuffPrim2( work->laser ) ;
			DG_SwitchBuffPrim2( work->light ) ;
			DG_PutVector( &lp, &from, 1 ) ;
			DG_PutVector( &lp0, &from0, 1 ) ;
			DG_PutVector( &spe, &to, 1 ) ;
			GetHazardLength( work, &from0, &to ) ;
			work->from = from ;
			DG_VisiblePrim2Chanl( work->laser, 0 ) ;
			DG_VisiblePrim2Chanl( work->light, 0 ) ;
			UpdateLaser( work, &from, &to ) ;
			UpdateLaserTex( work ) ;
			GM_MoveOnlineTarget( &( work->seek ), &from0, &work->hit ) ;
			work->seek.class = trgclass ;
			GM_PutTarget( &( work->seek ) ) ;
			world.m[ 3 ][ 0 ] = from.vx ;
			world.m[ 3 ][ 1 ] = from.vy ;
			world.m[ 3 ][ 2 ] = from.vz ;
		} // else  by koba4
		if ( work->len != 0.0F ) {
			/* レーザーサイト光線部分 */
			DG_VisiblePrim2Chanl( work->laser, 0 ) ;
		} 
		if ( work->len != 0.0F ) {
			/* レーザーサイト光点部分 */
			DG_VisiblePrim2Chanl( work->light, 0 ) ;
		}
		if ( GM_WeaponNum( WpNum[ this ] ) == 0 ) {
			/* から撃ち */
			//GM_SeSetMode( SD_W_EMPTY01, &root_pos, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_W_EMPTY01, &root_pos, GM_SEMODE_BOMB ) ;
			GM_SetNoise( NOISE_DEC, &work->ctrl->mov, work->ctrl->map ) ;
			break ;
		}
		DG_PutVector( &sp, &from, 1 ) ;
		world.m[ 3 ][ 0 ] = from.vx ;
		world.m[ 3 ][ 1 ] = from.vy ;
		world.m[ 3 ][ 2 ] = from.vz ;
		if ( ( GM_PlayerBody->objs->flag & vflag ) ||
			GM_CheckPlayerStatus( PLAYER_BEHIND ) ) {
			NewBullet( &world, BulletTypes[ this ] | BUL_TYPE_HIT_CHILD, 
					  ENEMY_SIDE, 25, 5, LENGTH_SUB, 1000, WpNum[ this ] ) ;
		} else {
		  //			NewBulletM4_demo( &world , ENEMY_SIDE );
			NewBullet( &world, BulletTypes[ this ], ENEMY_SIDE, 25, 5, 
					  LENGTH, 1000, WpNum[ this ] ) ;
		}
		if ( work->silencer ) {
			GM_SetNoise( NOISE_SS, &work->ctrl->mov, work->ctrl->map ) ;
		} else {
			GM_SetNoise( NOISE_M, &work->ctrl->mov, work->ctrl->map ) ;
		}
		switch( this ) {
		case THIS_IS_USP :
			//	    GM_ConfigObjectAction( &work->weapon_sub, 0, 1, 0, 0xfffff, 0 ) ; 
			//	    work->motion = 1 ; /* ブローバック */
			work->bctrl.flag = WPEF_FLG_START;
			work->bctrl_sub.flag = WPEF_FLG_START;
			//GM_SeSetMode( SD_W_SOCOM01, &root_pos, GM_SEMODE_BOMB ) ; 
			PL_SeSetSubject( SD_W_SOCOM01, &root_pos, GM_SEMODE_BOMB ) ;
//			MazzleMeca( root , &DG_ZeroVector );
			MazzleUSP( root, v_effect_flag, SK_LightFlag ) ;
			AN_CartridgeUSP_E( root, *( work->body ), work->ctrl ) ;
			break ;
		case THIS_IS_USP_SP :
			work->bctrl.flag = WPEF_FLG_START;
			work->bctrl_sub.flag = WPEF_FLG_START;
			//GM_SeSetMode( SD_W_SOCSIR12, &root_pos, GM_SEMODE_BOMB ) ; 
			PL_SeSetSubject( SD_W_SOCSIR12, &root_pos, GM_SEMODE_BOMB ) ; 
			MazzleUSP( root, EFFECT_FLAGS_NO_MZL, SK_LightFlag ) ;
			AN_CartridgeUSP_E( root, *( work->body ), work->ctrl ) ;
			break ;
		case THIS_IS_M92 :
			work->bctrl.flag = WPEF_FLG_START;
			work->bctrl_sub.flag = WPEF_FLG_START;
			// test
//			MazzleM92_Sea( root , 0 , 0 ) ;
//			AN_CartridgeM92_Sea_E( root, *( work->body ), work->ctrl ) ;
			//
			//GM_SeSetMode( SD_W_SOSIRE01, &root_pos, GM_SEMODE_BOMB ) ; 	    
			PL_SeSetSubject( SD_W_SOSIRE01, &root_pos, GM_SEMODE_BOMB ) ; 	    
			break ;
		case THIS_IS_SOCOM :
			work->bctrl.flag = WPEF_FLG_START;
			work->bctrl_sub.flag = WPEF_FLG_START;
			MazzleSOCOM( root , v_effect_flag , SK_LightFlag ) ;
			AN_CartridgeSOCOM_E( root, *( work->body ), work->ctrl ) ;
			//GM_SeSetMode( SD_W_SOCOM01, &root_pos, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_W_SOCOM01, &root_pos, GM_SEMODE_BOMB ) ;
			break ;
		case THIS_IS_SOCOM_SP :
			work->bctrl.flag = WPEF_FLG_START;
			work->bctrl_sub.flag = WPEF_FLG_START;
			MazzleSOCOM( root , EFFECT_FLAGS_NO_MZL , 0 ) ;
			AN_CartridgeSOCOM_E( root, *( work->body ), work->ctrl ) ;
			//GM_SeSetMode( SD_W_SOCSIR12 , &root_pos, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_W_SOCSIR12 , &root_pos, GM_SEMODE_BOMB ) ;
			break ;
		case THIS_IS_FAMAS :
			work->bctrl.flag = WPEF_FLG_START;
			work->bctrl_sub.flag = WPEF_FLG_START;
			MazzleFAMAS( root, 0, SK_LightFlag ) ;
			AN_CartridgeFMS_E( root, *( work->body ), work->ctrl ) ;
			//GM_SeSetMode( SD_W_FAMAS02, &root_pos, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_W_FAMAS02, &root_pos, GM_SEMODE_BOMB ) ;
			break ;
		case THIS_IS_AKS :
			MazzleAKS_RAI( root, v_effect_flag, SK_LightFlag ) ;
			AN_CartridgeAKS_RAI_E( root, *( work->body ), work->ctrl ) ;
			//GM_SeSetMode( SD_W_AKS74U01, &root_pos, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_W_AKS74U01, &root_pos, GM_SEMODE_BOMB ) ;
			break ;
		case THIS_IS_AKS_SP : // サプレッサー
			MazzleAKS_RAI( root , ( EFFECT_FLAGS_NO_MZL | v_effect_flag ) , 0 ) ;
			AN_CartridgeAKS_RAI_E( root, *( work->body ), work->ctrl ) ;
			//GM_SeSetMode( SD_W_AKSIRE01 , &root_pos, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_W_AKSIRE01 , &root_pos, GM_SEMODE_BOMB ) ;
			break ;
		case THIS_IS_M4 :
			MazzleM4A1( root, v_effect_flag , SK_LightFlag );
  			AN_CartridgeM4A1_E( root, *( work->body ), work->ctrl ) ;
			//MazzleM4demo( root , v_effect_flag , 0 );
			//AN_CartridgeM4_demo_E( root , *work->body , work->ctrl );
			//GM_SeSetMode( SD_W_M_FOUR01 , &root_pos, GM_SEMODE_BOMB ) ;
			PL_SeSetSubject( SD_W_M_FOUR01 , &root_pos, GM_SEMODE_BOMB ) ;
			break ;
		case THIS_IS_SPP :
			//GM_SeSetMode( SD_W_SOSIRE01, &root_pos, GM_SEMODE_BOMB ) ; 
			PL_SeSetSubject( SD_W_SOSIRE01, &root_pos, GM_SEMODE_BOMB ) ; 
			break ;
		default :
			if ( GM_CheckPlayerStatus( PLAYER_HOLD ) &&
				work->len != 0.0F ) {
				DG_VisiblePrim2Chanl( work->laser, 0 ) ;
				DG_VisiblePrim2Chanl( work->light, 0 ) ;
			}
		}
		lightcolor = Light2R | ( Light2G << 8 ) | ( Light2B << 16 ) ;
		DG_PutVector( &lp, &from, 1 ) ;
		/* パッドデモ中でもカウントしちゃう */
		GM_DecrementWeapon( WpNum[ this ], 1 ) ;
		/* 発砲数 */
		if ( ++ GM_ShootCount > 30000 ) GM_ShootCount = 30000 ;
		/* 空マガジン */
		if ( PL_DecrementMagazine() == 0 ) GM_IncrementWeapon( WP_Magazine, 1 ) ;
		break ;
	case TRIG_MAG_FALL :
		/* マガジン落下 */
		work->crtrgctrl = 4 ;
		work->crtrgctrl_sub = 4 ;
		break ;
	case TRIG_MAG_RIGHT_HAND :
		/* 右手もち */
		work->crtrgctrl = 3 ;
		work->crtrgctrl_sub = 3 ;
		break ;
	case TRIG_MAG_LEFT_HAND :
		/* 左手もち */
		work->crtrgctrl = 2 ;
		work->crtrgctrl_sub = 2 ;
		break ;
	case TRIG_MAG_LEFT_HAND2 :
		/* 左手もち２ */
		work->crtrgctrl = 6 ;
		work->crtrgctrl_sub = 6 ;
		break ;
	case TRIG_MAG_INVISIBLE :
		work->crtrgctrl = 0 ;
		work->crtrgctrl_sub = 0 ;
		break ;		
	case TRIG_MAG_LEFT_HAND_AND_FALL :
		work->crtrgctrl = 0x42 ;
		work->crtrgctrl_sub = 0x42 ;
		break ;
	default :
		printf( "????????\n" ) ;
	}

	/* マガジンの不可視 */
	if ( GM_CheckPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ) {
		work->crtrgctrl = 0 ;
		work->crtrgctrl_sub = 0 ;
	}
	/* 主観でないときは、主観カートリッジは不可視 */
	if ( !( GM_PlayerBody->objs->flag & vflag ) ) { 
		work->crtrgctrl_sub = 0 ;
	}

	//printf( "<%d> %x\n", GV_Time, work->crtrgctrl ) ;
//  DG_InvisibleObjs( work->weapon.objs ) ;
//	DG_InvisibleObjs( work->weapon_sub.objs ) ;
#if 0
	if ( GM_CheckPlayerStatus( PLAYER_HOLD ) ) ZdepthSwitch( work, 1 ) ;
	else				 ZdepthSwitch( work, 0 ) ;
#endif
	/* 銃ライト */
	work->circle_light = 0 ;
#if 0
	if ( ( this == THIS_IS_USP || this == THIS_IS_SOCOM ) &&
		( ( PL_GetPlayerItem() == IT_GunLight ) ||
		 ( PL_GetPlayerItem() == IT_GunLight2 ) ) &&
		( GM_CheckPlayerStatus( PLAYER_HOLD ) ) ) 
#else
    if ( ( this == THIS_IS_USP    ||
		   this == THIS_IS_USP_SP ||
		   this == THIS_IS_SOCOM  ||
		   this == THIS_IS_SOCOM_SP  ) && ( USP_LightOn == 1 ) &&
		 ( GM_CheckPlayerStatus( PLAYER_HOLD ) ) ) 
#endif
	{
		if ( trg != TRIG_SET ) {
			DG_PutVector( &lp, &from, 1 ) ;
			DG_PutVector( &lp0, &from0, 1 ) ;
			DG_PutVector( &spe, &to, 1 ) ;
			GetHazardLength( work, &from0, &to ) ;
		} else {
			from = work->from ;
		}
		_sceVu0SubVector( &diff, &( work->hit ), &from ) ;
		len = GV_VecLen3F( &diff ) ;
		lightcolor = LightR | ( LightG << 8 ) | ( LightB << 16 ) | ( 0x80 << 24 ) ;
#if 0
		if ( PL_GetPlayerItem() == IT_GunLight ) {
			DG_SetTmpSpotLight( &from, &diff, len + 100.0F, cosf( 10.0F / 180.0F * 3.14159 ),
							   cosf( 20.0F / 180.0F * 3.14159 ), lightcolor, 
							   LIT_FLAG_CHARAONLY | LIT_FLAG_BGONLY ) ;
		} else {
			if ( work->gunlight == NULL ) {
				work->gunlight = NewSpotLight( &( work->world ),
											  cosf( 9.0F / 180.0F * 3.14159 ),
											  cosf( 18.0F / 180.0F * 3.14159 ), 
											  8000.0F, lightcolor ) ;
			}
			/* 主観でないときだけ */
			if ( !( GM_PlayerBody->objs->flag & vflag ) ) {
				work->circle_light = 1 ;
			}
		}
#else

		if ( work->gunlight == NULL ) {
			if ( !(GM_GameStatus & STATE_VR_ONLY) ) {
				work->gunlight = NewSpotLight( &( work->world ),
											   cosf( 9.0F / 180.0F * 3.14159 ),
											   cosf( 18.0F / 180.0F * 3.14159 ), 
											   8000.0F, lightcolor ) ;
			} else {
				/* 全てシナリオ指定 */
				work->gunlight = NewUSPLight( &( work->world ), /* THESE ARE IGNORED --> */ 0, 0, 0, 0 /* <-- WERE IGNORED */ ) ;
			}
		}
		/* 主観でないときだけ */
		if ( !( GM_PlayerBody->objs->flag & vflag ) ) {
			work->circle_light = 1 ;
		} 
		if ( work->connected == 1 ) {
			DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
			work->weapon.objs->next_dgobjs = NULL ;
			work->connected = 0 ;
		}
		LightOn = 1 ;
#endif
	} else {
		if ( work->gunlight != NULL ) {
			GV_DestroyOtherActorQuick( work->gunlight ) ;
			work->gunlight = NULL ;
			if ( work->connected == 0 ) {
				DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
				work->connected = 1 ;
			}
		}
		LightOn = 0 ;
	}

	if ( this == THIS_IS_USP   || 
		 this == THIS_IS_USP_SP|| 
		 this == THIS_IS_FAMAS ||
		 this == THIS_IS_M92   ||
		 this == THIS_IS_SOCOM ||
		 this == THIS_IS_SOCOM_SP ) {
		WeaponEfAct( &( work->bctrl ) ) ;
		WeaponEfAct( &( work->bctrl_sub ) ) ;
#if 0
		GM_ActMotion( &work->weapon_sub ) ;
		MT_ActMotion2( work->weapon_sub.m_ctrl, work->weapon_sub.objs ) ;
		if ( work->motion != 0 && GM_CheckObject_IsEnd( &work->weapon_sub, 0 ) ) {
			GM_ConfigObjectAction( &( work->weapon_sub ), 0, 0, 0, 0xfffff, 0 ) ;
			work->motion = 0 ;
		}
#endif
	}

	/* 主観時はレーザー発射点不可視 */
	if ( GM_PlayerBody->objs->flag & vflag ) {
		work->light->packet[ work->light->buffer_clock ][ 0 ].flag |= DG_PRIM2_INVISIBLE0 ;		
	} else {
		work->light->packet[ work->light->buffer_clock ][ 0 ].flag &= ~DG_PRIM2_INVISIBLE0 ;
	}
	if ( PL_GetPlayerItem() == IT_Thermal ){
		int		visible_flag ;
		visible_flag = work->light->flag & DG_PRIM2_INVISIBLE0 ;
		work->light_ir->flag &= ~DG_PRIM2_INVISIBLE0 ;
		work->light_ir->flag |= visible_flag ;
		visible_flag = work->light->packet[ work->light->buffer_clock ][ 0 ].flag & DG_PRIM2_INVISIBLE0 ;
		work->light_ir->packet[ work->light_ir->buffer_clock ][ 0 ].flag &= ~DG_PRIM2_INVISIBLE0 ;
		work->light_ir->packet[ work->light_ir->buffer_clock ][ 0 ].flag |= visible_flag ;
		visible_flag = work->light->packet[ work->light->buffer_clock ][ 1 ].flag & DG_PRIM2_INVISIBLE0 ;
		work->light_ir->packet[ work->light_ir->buffer_clock ][ 1 ].flag &= ~DG_PRIM2_INVISIBLE0 ;
		work->light_ir->packet[ work->light_ir->buffer_clock ][ 1 ].flag |= visible_flag ;
		visible_flag = work->laser->flag & DG_PRIM2_INVISIBLE0 ;
		work->laser_ir->flag &= ~DG_PRIM2_INVISIBLE0 ;
		work->laser_ir->flag |= visible_flag ;
	} else {
		work->light_ir->flag |= DG_PRIM2_INVISIBLE0 ;
		work->laser_ir->flag |= DG_PRIM2_INVISIBLE0 ;
	}
#if 0
	if ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE &&
		GM_PlayerDebugMode == GM_PDM_GUN_LIGHT_COLOR &&
		work->this == THIS_IS_USP ) {
		int	status ;

		status = GV_PadData[ 1 ].status ;
		if ( status & PAD_U ) LightR ++ ;
		if ( status & PAD_D ) LightR -- ;
		if ( status & PAD_X ) LightG ++ ;
		if ( status & PAD_Y ) LightG -- ;
		if ( status & PAD_A ) LightB ++ ;
		if ( status & PAD_B ) LightB -- ;
		if ( LightR > 255 ) LightR = 0 ;
		if ( LightR < 0 ) LightR = 255 ;
		if ( LightG > 255 ) LightG = 0 ;
		if ( LightG < 0 ) LightG = 255 ;
		if ( LightB > 255 ) LightB = 0 ;
		if ( LightB < 0 ) LightB = 255 ;

		MENU_Locate( 260, 148, 0 ) ;
		MENU_Printf( "R, G, B = %d %d %d\n", LightR, LightG, LightB ) ;
	}
	if ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE &&
		GM_PlayerDebugMode == GM_PDM_GUN_FAIRY_COLOR ) {
		int	status ;

		status = GV_PadData[ 1 ].status ;
		if ( status & PAD_U ) Light2R ++ ;
		if ( status & PAD_D ) Light2R -- ;
		if ( status & PAD_X ) Light2G ++ ;
		if ( status & PAD_Y ) Light2G -- ;
		if ( status & PAD_A ) Light2B ++ ;
		if ( status & PAD_B ) Light2B -- ;
		if ( Light2R > 255 ) Light2R = 0 ;
		if ( Light2R < 0 ) Light2R = 255 ;
		if ( Light2G > 255 ) Light2G = 0 ;
		if ( Light2G < 0 ) Light2G = 255 ;
		if ( Light2B > 255 ) Light2B = 0 ;
		if ( Light2B < 0 ) Light2B = 255 ;
		MENU_Locate( 260, 148, 0 ) ;
		MENU_Printf( "R, G, B = %d %d %d\n", Light2R, Light2G, Light2B ) ;    
	}
	if ( GM_Debug2PMode == GM_DEBUG_MODE_PLAYER_STATE &&
		GM_PlayerDebugMode == GM_PDM_GUN_FAIRY_RANGE ) {
		int	status ;

		status = GV_PadData[ 1 ].status ;
		if ( status & PAD_U ) Light2SP1 += 10.0F ;
		if ( status & PAD_D ) Light2SP1 -= 10.0F ;
		if ( status & PAD_A ) Light2SP2 += 10.0F ;
		if ( status & PAD_B ) Light2SP2 -= 10.0F ;
		MENU_Locate( 260, 148, 0 ) ;
		MENU_Printf( "Range1 = %f\n", Light2SP1 ) ;
		MENU_Printf( "Range2 = %f\n", Light2SP2 ) ;
	}
#endif
}

static	void	Die( work )
Work			*work ;
{
	if ( GM_PlayerBody != NULL ) {
		if ( work->connected == 1 ) {
			DG_DisconnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
		}
		DG_DisconnectObjs( GM_PlayerArmBody->objs, work->weapon_sub.objs ) ;
	}
#if 0
	ZdepthSwitch( work, 0 ) ;
#endif
	GM_FreePrim2( work->laser ) ;
	GM_FreePrim2( work->light ) ;
	GM_FreePrim2( work->laser_ir ) ;
	GM_FreePrim2( work->light_ir ) ;
	GM_FreeObject( &( work->weapon ) ) ;
	GM_FreeObject( &( work->weapon_sub ) ) ;
	if ( GM_Weapon != WP_None &&
		 GM_Weapon != WpNum[ work->this ] ) GM_Magazine = 0 ;
	if ( work->gunlight != NULL ) {
		GV_DestroyOtherActorQuick( work->gunlight ) ;
		work->gunlight = NULL ;
	}
	GM_PlayerWeaponBody = NULL ;
	GM_PlayerSubWeaponBody = NULL ;
	GM_ResetPlayerStatus( PLAYER_SIGHT_LOCKON ) ;
	work->circle_light = -1 ;
	LightOn = 0 ;
	PL_EquipWeaponCode = 1 ;
//	work->crtrgctrl = 5 ;
}

/*------------------------------------------------------------------*/

/* レーザーサイト光線部分初期化 */
static	int	InitLaser( work )
Work		*work ;
{
	DG_PRIM2		*prim ;
	DG_TEX		*tex ;
	DG_PRIM2_UVRGB	*uvrgb ;
	int			j ;

	prim = work->laser = GM_MakePrim2( PRIM_FLAG1, N_PRIMS1, N_VERTS1 ) ;
	if ( prim == NULL ) return -1 ;
	tex = work->tex ;
	DG_ConfigPrim2Tex( prim, tex ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;
	uvrgb = MEM_ADDR2 ;
	/* N_PRIMS1 == 1 のはず */
	for ( j = 0; j < N_VERTS1; j ++ ) {
		if ( j == 0 ) {
			uvrgb[ j ].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb[ j ].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		} else {
			uvrgb[ j ].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb[ j ].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		}
		uvrgb[ j ].r = 32 ;
		uvrgb[ j ].g = 32 ;
		uvrgb[ j ].b = 32 ;
		uvrgb[ j ].a = 64 ;
		uvrgb[ j ].q = 4096 ;
		if ( j == 0 ) uvrgb[ j ].f = 0x8fff ;
		else	  uvrgb[ j ].f = 0x0fff ;
	}
	SprToMem( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof( DG_PRIM2_UVRGB ), N_VERTS1 ) ;
	SprToMem( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof( DG_PRIM2_UVRGB ), N_VERTS1 ) ;
	prim->as_world = DG_UnitMatrix ;
	DG_InvisiblePrim2( prim ) ;

	/* 新赤外線対策用 */
	prim = work->laser_ir = GM_MakePrim2( ( PRIM_FLAG1 & ~DG_PRIM2_TEX ) | DG_PRIM2_NOBUFFER, N_PRIMS1, N_VERTS1 ) ;
	if ( prim == NULL ) return -1 ;
	DG_SetPrim2Buffer( prim, work->laser->pos[0], work->laser->pos[1], work->laser->uvrgb[0], work->laser->uvrgb[1] );
	DG_ConfigPrim2Tex( prim, tex ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 2, 1, 255 ) ) ;
	prim->as_world = DG_UnitMatrix ;
	DG_InvisiblePrim2( prim ) ;

	work->cur_v0 = 0.0F ;
	work->speed = 1 ;
	return 0 ;
}

/* レーザーサイト光点部分初期化 */
static	int	InitLight( work )
Work		*work ;
{
	DG_PRIM2		*prim ;
	DG_PRIM2_UVRGBWH	*urw ;
	DG_TEX		*tex ;
	int			i, j ;
    
	tex = DG_GetTexture( TEXTURE2 ) ;
	prim = work->light = GM_MakePrim2( PRIM_FLAG2, N_PRIMS2, N_VERTS2 ) ;
	if ( prim == NULL ) return -1 ;
	DG_ConfigPrim2Tex( prim, tex ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;
	urw = MEM_ADDR2 ;
	for ( i = 0; i < N_PRIMS2; i ++ ) {
		for ( j = 0; j < N_VERTS2; j ++ ) {
			urw[ j ].u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			urw[ j ].v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			urw[ j ].u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			urw[ j ].v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			urw[ j ].q0 = urw[ j ].q1 = 4096 ;
			urw[ j ].f0 = urw[ j ].f1 = 0x0fff ;
			urw[ j ].r = 128 ;
			urw[ j ].g = 64 ;
			urw[ j ].b = 64 ;
			urw[ j ].a = 128 ;
		}
		urw += N_VERTS2 ;
	}
	SprToMem( prim->uvrgb[ 0 ], MEM_ADDR2, sizeof( DG_PRIM2_UVRGBWH ), N_VERTS2 * N_PRIMS2 ) ;
	SprToMem( prim->uvrgb[ 1 ], MEM_ADDR2, sizeof( DG_PRIM2_UVRGBWH ), N_VERTS2 * N_PRIMS2 ) ;    
	prim->as_world = DG_UnitMatrix ;
	DG_InvisiblePrim2( prim ) ;

	/* 新赤外線対策用 */
	prim = work->light_ir = GM_MakePrim2( ( PRIM_FLAG2 & ~DG_PRIM2_TEX ) | DG_PRIM2_NOBUFFER, N_PRIMS2, N_VERTS2 ) ;
	if ( prim == NULL ) return -1 ;
	DG_SetPrim2Buffer( prim, work->light->pos[0], work->light->pos[1], work->light->uvrgb[0], work->light->uvrgb[1] );
	DG_ConfigPrim2Tex( prim, tex ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 2, 1, 255 ) ) ;
	prim->as_world = DG_UnitMatrix ;
	DG_InvisiblePrim2( prim ) ;
    
	return 0 ;
}

/* シークターゲット初期化 */
static	void	InitTarget( work )
Work		*work ;
{
	TARGET	*t ;

	t = &( work->seek ) ;
	GM_SetTarget( t, TARGET_CLASS, 0, ENEMY_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_LASERSIGHT ) ;
	GM_SetTargetCallBack( t, HitTarget, work ) ;
}

static	int	GetResources( work, body, unit, this )
Work		*work ;
OBJECT		**body ;
int		*unit ;
int		this ;
{
	OBJECT	*weapon ;
	int      this_tmp;

	PL_EquipWeaponCode = BodyDatas[ this * 2 + 1 ] ;

	/* ブローバック */
	if ( this == THIS_IS_USP || 
		 this == THIS_IS_USP_SP || 
		this == THIS_IS_FAMAS ||
		 this == THIS_IS_M92 ||
		 this == THIS_IS_SOCOM || 
		 this == THIS_IS_SOCOM_SP ){
#if 1
		this_tmp = (this == THIS_IS_SOCOM_SP ? THIS_IS_SOCOM : this); // ブローバックを可能にするため 
		this_tmp = (this == THIS_IS_USP_SP ? THIS_IS_USP : this); // ブローバックを可能にするため 
		WeaponEfInitObject( &( work->bctrl ), &( work->weapon ), *body, *unit, 
						   WpNum[ this_tmp ], BodyDatas[ this * 2 ] ) ;
		WeaponEfInitObject( &( work->bctrl_sub ), &( work->weapon_sub ), *body, *unit, 
						   WpNum[ this_tmp ], BodyDatas[ this * 2 + 1 ] ) ;
#else
		weapon = &( work->weapon ) ;
		GM_InitObject( weapon, BodyDatas[ this * 2 ], BODY_FLAG ) ;
		if ( weapon->objs == NULL ) return -1 ;
		GM_ConfigObjectRoot( weapon, *body, *unit ) ;
		weapon = &( work->weapon_sub ) ;
		GM_InitObject( weapon, BodyDatas[ this * 2 + 1 ], BODY_FLAG ) ;
		if ( weapon->objs == NULL ) return -1 ;
		GM_ConfigObjectRoot( weapon, *body, *unit ) ;
#endif
		if ( work->weapon.objs == NULL || work->weapon_sub.objs == NULL ) return -1 ;
	} else {
		weapon = &( work->weapon ) ;
		GM_InitObject( weapon, BodyDatas[ this * 2 ], BODY_FLAG ) ;
		if ( weapon->objs == NULL ) return -1 ;
		GM_ConfigObjectRoot( weapon, *body, *unit ) ;
		weapon = &( work->weapon_sub ) ;
		GM_InitObject( weapon, BodyDatas[ this * 2 + 1 ], BODY_FLAG_SUB ) ;
		if ( weapon->objs == NULL ) return -1 ;
		GM_ConfigObjectRoot( weapon, *body, *unit ) ;
	}

	GM_PlayerWeaponModel = BodyDatas[ this * 2 ] ;
	GM_PlayerWeaponBody = &work->weapon ;
	GM_PlayerSubWeaponBody = &work->weapon_sub ;

	DG_ConnectObjs( GM_PlayerBody->objs, work->weapon.objs ) ;
	DG_ConnectObjs( GM_PlayerArmBody->objs, work->weapon_sub.objs ) ;
	work->connected = 1 ;

	if ( GM_PlayerBody->objs->flag & DG_FLAG_INVISIBLE0 ) {
		GM_SetPlayerStatus( PLAYER_WEAPON_INVISIBLE ) ;
	}

	DG_InvisibleObjs( work->weapon_sub.objs ) ;

	work->tex = DG_GetTexture( TEXTURE ) ;
	if ( InitLaser( work ) < 0 ) return -1 ;
	if ( InitLight( work ) < 0 ) return -1 ;
	InitTarget( work ) ;	
	/* ライト */
	work->gunlight = NULL ;
	work->circle_light = 0 ;
	if ( ( this == THIS_IS_USP     ||
		   this == THIS_IS_USP_SP  ||
		   this == THIS_IS_SOCOM   ||
		   this == THIS_IS_SOCOM_SP ) && USP_LightOn == 1 ) {
		GV_SetActorChild( work, NewCircleLight( &work->weapon.objs->world,
											    &work->circle_light, 3, &GM_PlayerControl->map ) ) ;
	}
#if 0
	/* モーション */
	if ( this == THIS_IS_USP ) {
		GM_ConfigObjectMotion( &work->weapon_sub, 1, USP_MOTION, 0 ) ;
		GM_ConfigObjectStep( &work->weapon_sub, &work->step ) ; /* ダミー */
		GM_ConfigObjectAction( &work->weapon_sub, 0, 0, 0, 0xfffff, 0 ) ; 
		work->motion = 0 ;
	}
#endif
#if 1
	/* カートリッジ管理 */
	work->crtrgctrl = 1 ;
	work->crtrgctrl_sub = 1 ;
	switch ( this ){
	case THIS_IS_USP :
	case THIS_IS_USP_SP :
	case THIS_IS_M92 :
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl, 
													 GM_PlayerBody, &work->weapon, 2 ) ) ;
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl_sub, 
													 GM_PlayerArmBody, &work->weapon_sub, 2 ) ) ;
		work->crtrgctrl = 0 ;
		work->crtrgctrl_sub = 0 ;
		break;
	case THIS_IS_SOCOM :
	case THIS_IS_SOCOM_SP :
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl, 
													 GM_PlayerBody, &work->weapon, 8 ) ) ;
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl_sub, 
													 GM_PlayerArmBody, &work->weapon_sub, 8 ) ) ;
		work->crtrgctrl = 0 ;
		work->crtrgctrl_sub = 0 ;
		break;
	case THIS_IS_M4 :
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl, 
													 GM_PlayerBody, &work->weapon, 4 ) ) ;
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl_sub, 
													 GM_PlayerArmBody, &work->weapon_sub, 4 ) ) ;
		break;
	case THIS_IS_AKS :
	case THIS_IS_AKS_SP :
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl, 
													 GM_PlayerBody, &work->weapon, 7 ) ) ;
		GV_SetActorChild( work, NewCartridgeControl( &work->crtrgctrl_sub, 
													 GM_PlayerArmBody, &work->weapon_sub, 7 ) ) ;
		break;
	}
#endif 
#if 0
	GV_SetActorChild( work , NewQuestion() );
#endif

	return 0 ;
}

/*------------------------------------------------------------------*/

static	void	*New( ctrl, body, unit, trigger, side, this, silence )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side, this, silence ;
{
	Work	*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT,
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->ctrl = ctrl ;
		work->body = body ;
		work->unit = unit ;
		work->trigger = trigger ;
		work->side = side ;
		work->this = this ;
		work->chanl = GM_CurrentCameraChanl ;
		work->vflag = ( DG_FLAG_INVISIBLE0 << work->chanl ) ;
		if ( GetResources( work, body, unit, this ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
		PL_SetMagazine( WpNum[ this ], Magazines[ this ] ) ;
		work->silencer = silence ;
		LightOn = 0 ;
	}
	return work ;
}

/*------------------------------------------------------------------*/

/* Ｍ９２ */
void	*NewM92( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
	return New( ctrl, body, unit, trigger, side, THIS_IS_M92, 1 ) ;
}

/* ＵＳＰ */
void	*NewUsp( CONTROL *ctrl, OBJECT **body, int *unit, u_int *trigger, int side )
{
	int		this, silence, spprsr_se ;
	void	*work ;

	this = THIS_IS_USP ;
	silence = 0 ;
	spprsr_se = 0 ;
	//if ( GM_ItemNum( IT_UspSpprsr ) > 0 ) {
    if ( PL_GetPlayerItem() == IT_UspSpprsr ) {	
		GM_SetItemNum( IT_UspSpprsr, 0 ) ;
		GM_PlayerStateFlag |= PL_USP_SPPRSR_ATTACHED ;
		this = THIS_IS_USP_SP ;
		silence = 1 ;
		spprsr_se = 1 ;
	} else if ( GM_PlayerStateFlag & PL_USP_SPPRSR_ATTACHED ) {
		this = THIS_IS_USP_SP ;
		silence = 1 ;
	}
	work = New( ctrl, body, unit, trigger, side, this, silence ) ;
	if ( work != NULL && spprsr_se == 1 ) SpprprSeDelay( work ) ;
	return work ;
}

/* ＳＯＣＯＭ */
void	*NewSocom( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
	int			this, silence ;
	int			spprsr_se = 0 ;
	void		*work ;

	this = THIS_IS_SOCOM ;
	silence = 0 ;
	//if ( GM_ItemNum( IT_SocomSpprsr ) > 0 ) {
    if ( PL_GetPlayerItem() == IT_SocomSpprsr ) {	
		GM_SetItemNum( IT_SocomSpprsr, 0 ) ;
		GM_PlayerStateFlag |= PL_SOCOM_SPPRSR_ATTACHED ;
		this = THIS_IS_SOCOM_SP ;
		silence = 1 ;
		spprsr_se = 1 ;
	} else if ( GM_PlayerStateFlag & PL_SOCOM_SPPRSR_ATTACHED ) {
		this = THIS_IS_SOCOM_SP ;
		silence = 1 ;
	}
	work = New( ctrl, body, unit, trigger, side, this, silence ) ;
	if ( work != NULL && spprsr_se == 1 ) SpprprSeDelay( work ) ;
	return work ;
}

/* ＦＡＭＡＳ */
void	*NewFamas( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
	return New( ctrl, body, unit, trigger, side, THIS_IS_FAMAS, 0 ) ;
}

/* ＡＫＳ */
void	*NewAks( CONTROL *ctrl, OBJECT **body, int *unit, u_int *trigger, int side )
{
	int			this, silence ;
	int			spprsr_se = 0 ;
	void		*work ;

	this = THIS_IS_AKS ;
	silence = 0 ;
	//if ( GM_ItemNum( IT_AKSpprsr ) > 0 ) {
    if ( PL_GetPlayerItem() == IT_AKSpprsr ) {	
		GM_SetItemNum( IT_AKSpprsr, 0 ) ;
		GM_PlayerStateFlag |= PL_AK_SPPRSR_ATTACHED ;
		this = THIS_IS_AKS_SP ;
		silence = 1 ;
		spprsr_se = 1 ;
	} else if ( GM_PlayerStateFlag & PL_AK_SPPRSR_ATTACHED ) {
		this = THIS_IS_AKS_SP ;
		silence = 1 ;
	}
	work = New( ctrl, body, unit, trigger, side, this, silence ) ;
	if ( work != NULL && spprsr_se == 1 ) SpprprSeDelay( work ) ;
	return work ;
}

/* Ｍ４ */
void	*NewM4( CONTROL *ctrl, OBJECT **body, int *unit, u_int *trigger, int side )
{
	return New( ctrl, body, unit, trigger, side, THIS_IS_M4, 0 ) ;
}

/* ＳＰＰ１Ｍ */
void	*NewSpp1M( ctrl, body, unit, trigger, side )
CONTROL		*ctrl ;
OBJECT		**body ;
int		*unit ;
u_int		*trigger ;
int		side ;
{
	return New( ctrl, body, unit, trigger, side, THIS_IS_SPP, 0 ) ;
}

/*----------------------------------------------------------------*/

/* ＵＳＰを構えたら自動でライトをＯＮにする */
int		NewUSPLightOn( void )
{
	USP_LightOn = 1 ;
	return 1 ;
}

int	   PL_GetUSPLightOn( void )
{
	return LightOn ;
}

/* 主観時射程距離設定 */
int		NewSetSubjectShotLen( void )
{
	LENGTH_SUB = ( float )GCL_GetOptionValue( 'l', 40000 ) ;
	return 0 ;
}

// こば４追加
void *NewSKLightEff( void )
{
	int tmp;

	SK_LightFlag = 0;
	tmp = GCL_GetOptionValue( 't', 0 ) ;
	if ( tmp != 0 ){
		SK_LightFlag |= EFFECT_FLAGS_ON_TMP;
	}

	return 0 ;
}

/*----------------------------------------------------------------*/

/* ＳＥを鳴らすＡＣＴ */
typedef	struct	{
	GV_ACT_EX		actor ;
	int				count ;
} Work2 ;

static	void	Act2( Work2 *work )
{
	if ( -- work->count <= 0 ) {
		printf( "[%d] spprsrs attached!\n", GV_Time ) ;

		GM_SeSet( GM_PAN_CENTER, GM_MAX_VOL, SD_I_SIREEQU1 ) ;

		GV_DestroyActor( work ) ;
	}
}

static	void	*SpprprSeDelay( void *parent )
{
	Work2		*work ;

	work = ( Work2 * )GV_NewActor( GV_ACTOR_AFTER, sizeof( Work2 ) ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act2, NULL ) ;
	GV_ActorEX( &work->actor ) ;
	GV_SetActorChild( parent, work ) ;
	work->count = DIRECT_TICK( 15 ) ;
	printf( "[%d] spprsrs attached se act start\n", GV_Time ) ;
	return work ;
}
