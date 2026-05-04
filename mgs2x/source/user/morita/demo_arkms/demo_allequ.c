//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_arkms.c
   ARKMS専用 キャラ

   2000/12/22 T. Morita
   $Id: demo_allequ.c,v 1.1.1.3 2002/11/19 11:45:54 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"
#include "../include/util.h"


typedef struct work_t
{
    GV_ACT_EX   actor   ;

    OBJECT     *object  ;
    int         name    ;
} Work ;


enum
{

    EQUIP_MARINE = 1,
    EQUIP_NYPD,
    EQUIP_CIT_FEMALE,
    EQUIP_CIT_MALE,
    EQUIP_HOSTAGE,
    EQUIP_CIT_OLDLADY,
} ;

#if  0 /* command.defにて */
enum
{
    /* スネーク */
    胸左上のマガジン=0x00010000,
    胸右上=0x00020000,
    胸左下=0x00040000,
    胸右下=0x00080000,

    /* タンカーオルガ */
    ホルスター=0x00000001,
    スカウトナイフ=0x00000002,
    無線機=0x00010000,

    /* プラントオルガ */
    ホルスター=0x00000001,
    スカウトナイフ=0x00000002,
    ポーチ０=0x00010000,
    ポーチ１=0x00000004,

    /*セルゲイ*/
    ホルスター=0x00000001,
    スカウトナイフ=0x00000002,
    ポーチ０=0x00010000,
    ポーチ１=0x00000004,

    /* ゴルルコビッチ */
    銃のホルスター       =  0x0001,
    ナイフ               =  0x0002,
    ナイフを抜いたケース  =  0x0004,
    マガジン入れ         =  0x0008,
    バックパック         =  0x0010,
    無線機               =  0x0020,
    暗視ゴーグル         =  0x0040,
    暗視ゴーグル開き     =  0x0200,
    右ライトオン         =  0x0400,
    左ライトオン         =  0x0800,
    武器右手持ち         =  0x0080,
    武器ぶら下げる       =  0x0100,
    武器左手持ち         =  0x1000,
    プラントゴル兵       =  0x2000,
    プラント攻撃ゴル兵   =  0x4000,
    銃天狗              =  0x8000,
    刀天狗              =  0x10000,
    刀鞘                =  0x20000,
} ;
#endif

extern void *NewCreateEquipment( OBJECT *object, short ID, int Data_Num ) ;
extern void *SK_NewEvm_Skirt( OBJECT *pObj , int strcode , int name ) ;
extern void *NewScnEvm_SkirtA( OBJECT *pObj , int name ) ;// ネーちゃん
extern void *NewScnEvm_SkirtB( OBJECT *pObj , int name ) ;// おばちゃん

extern void *NewEmmaEquip( int name, OBJECT *body, int flag ) ;
extern void *NewFortEquip( int name, OBJECT *body, int flag, int wpname ) ;
extern void *NewOrgFaceEft(int name, void *cotrol, int mode ) ;
extern void *NewPutAttachments( void *parent, OBJECT *body,
				int at_id, int flag ) ;
extern void *NewHairModel_Demo( int ,int ,int ,void *,int ,
				FVECTOR *,SVECTOR *,
				float ,int ,int ,int ,int  ) ;
extern void *NewEvmHairModel_Demo( int ,int ,int ,void *,int ,
				   FVECTOR *,SVECTOR *,
				   float ,int ,int ,int ,int  ) ;
extern void *NewEvmHairModel_Demo2( int ,int ,int ,void *,
				    FVECTOR *,SVECTOR *,
				    float ,int ,int ,int  ) ;
extern void *NewEvmHairModel_called2(int name,int model_name,int sample_num,
				     OBJECT *target,
				     unsigned char *objnum,int n_objnum,
				     FVECTOR *x,SVECTOR *r,
				     float oval_param,
				     int collision_flag,
				     unsigned char *collision_objs,
				     int visible_flag,
				     int light_flag,int boundmodel_name) ;

extern void *NewRopeModel3_called( int ,int ,void *,int ,
				   FVECTOR *,SVECTOR *,
				   float ,int ,int ,int ,int  ) ;
extern void *NewEneEquip( int name, void *parent, OBJECT *body,
			  int flag, int wpname, int left_wp ) ;
extern void *NewSIG_BlurPoint(FMATRIX *world,
			      FVECTOR *shift,CVECTOR *col ,int *sw ) ;
extern void *NewCergeiEri_demo( int ,void *,int  ) ;

extern void *NewWavingClothModelW_called( int ,int *,int ,int ,void *, int ,
					  FVECTOR *,SVECTOR *,
					  int ,float ,int  ) ;
extern void *NewConnectEquip( DG_OBJS *objs, int joint, int kms, int *flag, int disp_flag ) ;
extern void *NewConnectObjectEquip( OBJECT *object, int joint, int kms, int *flag, int disp_flag );


static int GetResourcesCalled( Work *work, int name, CONTROL *ctrl,
			       int type, int flag )
{
    int     model, bounding ;
    OBJECT *object = ctrl->object ;

    printf( "SetEquip type%d flag%x\n", type, flag ) ;

    /* 手に武器を持たせる */
    if ( flag & 0x20000000 )
    {
#if 0
	GV_SetActorChild( work,
			  NewCreateEquipment( object, 3, 1 ) ) ;
#endif
	flag &= ~0x20000000 ;
    }

    switch( type )
    {
	/* ライデン */
    case 12663374: /* rai_def */
    case 11804297: /* rai_def_sh_mt */
    case 5512841:  /* rai_def_mh_mt.evm */
    case 14591498: /* rai_def_addhand_mh_mt.evm */
	/*ライデン通常=0&ライデン水中=1&ライデン濡れ髪=2&ライデン少し濡れ髪=3*/
	model    = 5518691 /*rai_hair_mh_mt*/ ;
	bounding = 12256813/*rai_hair_bounding*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo( name,
						model, 0/* サンプル番号 */,
						object, HUMAN21_ATAMA,
						NULL, NULL,
						1.0f, 3/*首あたりあり*/,
						0/*可視フラグ*/,
						0/* ライトモード*/,
						bounding ) ) ;

	if ( flag & 0x001 )/* ドッグタグを装備する */
	{
	    model    = 8342217 /*rai_dogtag_naked_mh*/ ;
	    bounding = 988797  /*rai_dogtag_bounding*/ ;
	    GV_SetActorChild( work,
			      NewRopeModel3_called( model, 9,
						    object, HUMAN21_ATAMA,
						    NULL, NULL,
						    1.0f, 1, 0, 1,
						    bounding ) ) ;
	}
	break ;

	/* 忍者ライデン */
    case 12468361: /* rai_min_sh_mt.kms */
	  GV_SetActorChild( work,
					   NewConnectObjectEquip( object,
											 HUMAN21_ATAMA,
											 7862789/*rai_nin_face_open */,
											 //4024755/*rai_nin_face_close */,
											 NULL, DG_FLAG_IRREACTION ) ) ;
    case 6176905 : /* rai_min_mh_mt.kms */
	break ;

	/* メリル */
    case 11203702: /* mrl_fc_def_mh_mt */
    case 5494549:/* mrl_def_mh_mt.evm      本体   */
    {
	static char objnum[] = { 11, 12 } ;
	static char colobj[] = { 11, 12 } ;
	model    = 16682505 ;/* mrl_def_hair_mh.evm 髪の毛 */
	bounding = 15527389 ;/* mrl_dummy.kms       バウンディングボックス用 */

	GV_SetActorChild( work,
			  NewEvmHairModel_called2( name, model, 
						   28,/*メリルの髪の毛*/
						   object,
						   objnum,sizeof(objnum),
						   NULL, /* local pos*/
						   NULL, /* local rot*/
						   1.0f, /* oval_param     */
						   1,    /* collision_flag */
						   colobj,
						   0, 0, bounding ) ) ;

	  GV_SetActorChild( work,
					   NewConnectObjectEquip( object,
											 HUMAN21_KOSHI,
											 5197317/*mrl_waist_pack*/,
											 NULL, 0 ) ) ;
	  GV_SetActorChild( work,
					   NewConnectObjectEquip( object,
											 HUMAN21_KOSHI,
											 13790832/*mrl_waist_mag*/,
											 NULL, 0 ) ) ;

	break ;
    }

	/* プラント編オルガ */
    case 9746755:/*org_plant_mh_mt.evm*/
	model = 8865549 /*org_plant_hair_mh     */ ;
	bounding = 749896 /*org_plant_dummy  */ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name,
						 model,
						 15/*プラントオルガの髪の毛*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;
	if ( flag & 0x80000000 )
	    flag = 0x00030001 ;
	NewPutAttachments( work, object, 4, flag ) ;
	break ;


	/* プラント編オルガ 死におるが */
    case 789047:/* org_plant_dead_mh_mt.evm */
	model = 13309162/*org_plant_dead_hair_mh*/ ;
	bounding = 749896 /*org_plant_dummy  */ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name,
						 model,
						 15/*プラントオルガの髪の毛*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;
	if ( flag & 0x80000000 )
	    flag = 0x00030001 ;
	NewPutAttachments( work, object, 4, flag ) ;
	break ;


	/* スネーク */
    case 4274856:/* sna_def */
    case 5614245:/* sna_def_mh.evm */
    case 4105761:/* sna_def_addhand_mh_mt.evm*/
	if ( flag & 0x80000000 )
	    flag = 0x000f0000 ;
	NewPutAttachments( work, object, 1, flag ) ;
    case 6268969:  /* sna_oss_mh_mt  前作スネークはバンダナだけ */
    {
	static FVECTOR Offset = { 0.0f, 60.0f, -90.0f, 1.0f } ;

	bounding = 8601362 /*sna_shadow */ ;
	GV_SetActorChild( work,
			  NewRopeModel3_called( 2511417/*sna_bdn1*/, 2,
						object, HUMAN21_ATAMA,
						&Offset, NULL,
						1.0f, 1, 0, 1,
						bounding ) ) ;
	GV_SetActorChild( work,
			  NewRopeModel3_called( 2511418/*sna_bdn2*/, 3,
						object, HUMAN21_ATAMA,
						&Offset, NULL,
						1.0f, 1, 0, 1,
						bounding ) ) ;
	break ;
    }
    case 12560425: /* sna_oss_sh_mt  前作スネーク */ 
	break ;

	/* タキシードスネーク */
    case  6605929:/* sna_txd_mh_mt.evm*/
    case 12897385:/* sna_txd_sh_mt.kms*/
    {
		unsigned char objnum[] = { 0, 1, 2, 13, 17 } ;
		unsigned char colobj[] = { 0, 1, 2, 13, 17 } ;
	    model    = 10273604 /* sna_txd_suso_mh_mt */ ;
	    bounding = 4066937  /* sna_txd_bounding   */ ;
	    GV_SetActorChild( work,
						 NewEvmHairModel_called2( name,
												 model, 28,
												 object, objnum, 5,
												 NULL, NULL,
												 1.0f, 5, colobj,
												 0/*可視フラグ*/,
												 0/* ライトモード*/,
												 bounding ) ) ;
	}
	break ;

	/* タンカー編オルガ */
    case 5678563: /* org_def_addhand_mh_mt.evm */
    case 10566064:/* org_def.evm */
	if ( flag & 0x80000000 )
	    flag = 0x00010003 ;
	NewPutAttachments( work, object, 2, flag ) ;
	break ;


	/* セルゲイ */
    case 6699649:/*crg_wet_mh_mt.evm*/
    case 10665224:/*crg_blood_mh_mt.evm*/
    case 5453569:/*crg_def_mh_mt.evm*/
    case 5678371:/*crg_def_addhand_mh_mt.evm*/
	if ( flag & 0x80000000 )
	    flag = 0x00010007 ;

#if 0	
	if ( flag & 0x000000100 )
	    GV_SetActorChild( work,
			      NewCergeiEri_demo( 0, object, name ) ) ;
	flag &= ~0x000000100 ;
#endif
			  
	NewPutAttachments( work, object, 3, flag ) ;
	break ;



#if 0
	/* 忍者オルガ */
    case 6569793: /* org_tng_mh_mt.evm */
	NewOrgFaceEft( name, ctrl, 0 ) ;
	break ;
#endif


	/* オセロット 手つき(コート無し) */
    case 5513405: /* rev_def_mh_mt.evm */
    case 1222157: /* rev_def_addhand_mh_mt.evm */
	model = 7584084 /*rev_hair_fix*/ ;
	bounding = 3849521/*rev_coat_bounding*/ ;
	GV_SetActorChild( work,
			  NewRopeModel3_called( model, 4,
						object, HUMAN21_MUNE,
						NULL, NULL,
						1.1f, 1, 0, 1,
						bounding ) ) ;
	break ;

	/* オセロット */
    case 2552518: /* rev_coat.evm */
	if ( flag & 1 )
	{
	    model    = 11027440 /* rev_hair_liq_mh  */ ;
	    bounding = 13109812 /* rev_liq_bounding */ ;
	    GV_SetActorChild( work,
			      NewEvmHairModel_Demo( name,
						    model, 4, /* リキッド用 */
						    object, HUMAN21_ATAMA,
						    NULL, NULL,
						    0.9f, 3/*首あたりあり*/,
						    0/*可視フラグ*/,
						    0/* ライトモード*/,
						    bounding ) ) ;
	}
	else
	{
	    model = 7584084 /*rev_hair_fix*/ ;
	    bounding = 3849521/*rev_coat_bounding*/ ;
	    GV_SetActorChild( work,
			      NewRopeModel3_called( model, 4,
						    object, HUMAN21_MUNE,
						    NULL, NULL,
						    1.1f, 1, 0, 1,
						    bounding ) ) ;
	}

	/* コート */
	model = 7504651/*rev_parts_coat*/ ;
	GV_SetActorChild( work,
			  NewWavingClothModelW_called( name, &model, 1, 3,
						       object, HUMAN21_MUNE,
						       NULL, NULL,
						       bounding,
						       1.1f, 0 ) ) ;
	break ;


    case 16497327: /*htc_def_mt*/
	/*ハイテクぼんぼり*/
	{
#if 0
	    static int swt = 1;
	    FVECTOR shift = {-111.0f, 101.0f, 52.0f, 0.0f };
	    CVECTOR color = { 80, 20, 0, 127 } ;

	    if ( !(flag & 0x80000000) )
		GV_SetActorChild( work ,
				  NewSIG_BlurPoint( &object->objs->objs[HUMAN21_ATAMA].world,
						    &shift, &color ,&swt )) ;
#endif
	}
	break ;



	/* フォーチュン コートつき */
    case 11197026: /* for_coat_mh_mt.evm */
    case 12057798: /* for_coat_addhand_mh_mt */
	if ( flag & 0x80000000 )
	    flag = 0x00000003 ;
	if ( flag & 0x00000800 )
	    model = 114226, flag &= ~0x00000800 ;
	else
	    model = 0 ;
	GV_SetActorChild( work,
			  NewFortEquip( name, object, flag, model ) ) ;
	break ;


	/* フォーチュン 裸 */
    case 12128086: /* for_last_addhand_mh_mt.evm */
    case 5465517:  /* for_def_mh_mt.evm     */
    case 11756973: /* for_def_sh_mt.kms     */
    case 173394:   /* for_def_addhand_mh_mt.evm */
	bounding = 3846528 /* for_coat_bounding.kms */ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name,
						 7767005/*for_hair_def_mh*/,
						 //9094282/*for_hair_coat_mh*/,
						 3, object,
						 NULL, NULL,
						 1.0f, 0, 0,
						 bounding ) ) ;
	break ;


#if 0
	/* エマ */
    case 5461097: /* ema_def_mh_mt.evm        */
    case 12494144:/* ema_def_addhand_mh_mt.evm*/
    case 15364231:/* ema_back_blood_mh.evm    */
    case 8222402: /* ema_houtai_addhand_mh_mt.evm */
	if ( flag & 0x80000000 )
	    flag = 0x00000007 ;
	GV_SetActorChild( work,
			  NewEmmaEquip( name, object, flag ) ) ;
	break ;
#endif

#if 0
	/* ゴルルコ兵士(プラント攻撃) */
    case 13216431: /* gba_def_mt.kms */
	if ( flag & 0x80000000 )
	    flag = 0x00004000 ;
	NewEneEquip( name, work, object, flag, 0, 0 ) ;
	break ;

	/* ゴルルコ兵士(プラント) */
    case 14170799: /* gps_def_mt.kms */
	if ( flag & 0x80000000 )
	    flag = 0x00002000 ;
	NewEneEquip( name, work, object, flag, 0, 0 ) ;
	break ;

	/* ゴルルコ兵士(タンカー) */
    case 6371217:  /* gbs_def.kms */
	if ( flag & 0x80000000 )
	    flag = 0x0000003f ;
	NewEneEquip( name, work, object, flag, 0, 0 ) ;
	break ;

	/* 天狗兵士 */
    case 10566376: /* tng_def.kms */
	if ( flag & 0x80000000 )
	    flag = 0x00030000 ;
	NewEneEquip( name, work, object, flag, 0, 0 ) ;
	break ;
#endif

#if 0
	/* オタコン 血糊 */
    case 6355049:/* otc_blood_addhand_mh_mt.evm*/
    case 10910987:/*otc_blood_mh_mt.evm*/
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name,
						 11574939/*otc_hair_mh*/,
						 23/*オタコンの前髪*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 15661981 /*otc_dummy*/ ) ) ;
	model = 7242065/*otc_blood_jacket_suso_mh*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 17/*オタコンの裾*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 15661981 /*otc_dummy*/ ) ) ;
	break ;
#endif


	/* オタコン */
    case 4629988:/* otc_def_addhand_mh_mt.evm*/
    case 5502961:/* otc_def_mh_mt.evm */
	/* 眼鏡 */
	GV_SetActorChild( work,
					 NewConnectObjectEquip( object,
										   HUMAN21_ATAMA,
										   12559050/* otc_glasses_mt */,
										   NULL, DG_FLAG_SEMITRANS | DG_FLAG_FORCEMSAA ) ) ;

	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name,
						 11574939/*otc_hair_mh*/,
						 23/*オタコンの前髪*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
+						 15661981 /*otc_dummy*/ ) ) ;
	model = 353452/*otc_jaket_suso_mh*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 17/*オタコンの裾*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 15661981 /*otc_dummy*/ ) ) ;
	break ;

	/* ローズ */
    case 5514673:/*ros_def_mh_mt.evm*/
	model = 2948001 /*ros_hair_fd_mh_mt*/ ;
	bounding = 14031266/*ros_bounding     */ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 11/*ローズの髪の毛*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;

	model = 4637847 /*ros_hair_b_mh_mt*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 11/*ローズの髪の毛*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;
	break ;

	/* ヴァンプ 裸 */
    case 16055611: /* vmp_naked_mh_mt */
	model    = 7307187  /*vmp_hair_naked_mh_mt*/ ;
	bounding = 10966673 /*vmp_naked_bounding*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 24 /*裸ヴァンプ髪の毛*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;

	model = 15584320/*vmp_parts_vkpa_mh*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 5 /*ヴァンプナイフ=*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;
	break ;


	/* ヴァンプ コート */
    case 13285730: /* vmp_coat_mh_mt */
	model    = 477064   /*vmp_parts_coat_mh*/ ;
	bounding = 12239215 /*vmp_coat_bounding*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 6/*ヴァンプコート*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;

	model = 8818038/*vmp_hair_coat_mh_mt*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 4 /*ヴァンプ髪の毛*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;

	model = 15584320/*vmp_parts_vkpa_mh*/ ;
	GV_SetActorChild( work,
			  NewEvmHairModel_Demo2( name, model,
						 5 /*ヴァンプナイフ=*/,
						 object,
						 NULL, NULL, 1.0f, 0, 0,
						 bounding ) ) ;
	break ;

	/* ＮＹ市民男子 */
    case 9716518:/*cit_maley_ct_sh.kms*/
    case 6169447:/*maley_ctg_sh.kms*/
	break ;

	/* ＮＹ市民ＯＬ */
    case 1749525:/*cit_femaled_def.kms*/
	break ;

	/*ＮＹ	おばちゃん*/
    case 2798101: /* cit_femalee_def.kms*/
    case 700949:  /* cit_femalec_def.kms*/
	GV_SetActorChild( work,
			  SK_NewEvm_Skirt( object,
					   272209/*cit_female_skirt2_mh*/,
					   name ) ) ;
	break ;

	/* 海兵隊 */
    case EQUIP_MARINE:
	/* 装備品 */
	if ( !(flag & 0x80000000) )
	{
	    GV_SetActorChild( work,
			      NewCreateEquipment( object, 0, flag ) ) ;
	}
	break ;

    case EQUIP_CIT_MALE:
	/* 装備品 */
	GV_SetActorChild( work,
			  NewCreateEquipment( object, 1, flag ) ) ;
	break ;

    case EQUIP_CIT_OLDLADY:
    case EQUIP_CIT_FEMALE:
	/* 装備品 */
	if ( flag & 0x10000 )
	    GV_SetActorChild( work,
			      SK_NewEvm_Skirt( object,
					       239441/*cit_female_skirt1*/,
					       name ) ) ;
	if ( flag & 0x20000 )
	    GV_SetActorChild( work,
			      SK_NewEvm_Skirt( object,
					       272209/*cit_female_skirt2*/,
					       name ) ) ;
	flag &= ~0x30000 ;

	GV_SetActorChild( work,
			  NewCreateEquipment( object, 2, flag ) ) ;
	break ;

    case 828773:/*hos_femalea_def_mt*/
	GV_SetActorChild( work,
			  NewScnEvm_SkirtA( object, name ) ) ;
	break ;

    case 830821:/*hos_femaleb_def_mt*/
	GV_SetActorChild( work,
			  NewScnEvm_SkirtB( object, name ) ) ;
	break ;
    }

    return 0 ;
}


void *NewDemoAllEquip( int name, CONTROL *ctrl, int type, int flag )
{
    Work *work ;

    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof(Work) ) ;
    if ( work != NULL )
    {
        GV_SetActor( &work->actor, NULL, NULL ) ;
	GV_ActorEX( &work->actor ) ;
        if ( GetResourcesCalled( work, name, ctrl, type, flag ) < 0 )
        {
            GV_DestroyActor( work ) ;
            return NULL ;
        }
    }
    return work ;
}
