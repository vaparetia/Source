//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	wpeffect.c
	武器エフェクト

	1999/12/03 M.Matsuzaki
    2001/05/01 S.Kobayashi

	$id:
*/

#define MATSUZAKI_SOURCE
#include "./matsu.h"
#include "wpeffect.h"
#include "../test/etc.h"
//#define WEAPON_EF_DEBUG_MODE		/* 武器エフェクトのデバッグモード  */ 

#define BODY_FLAG	(DG_FLAG_SHADE /*| DG_FLAG_ONEPIECE */)
#define BODY_FLAG2	(DG_FLAG_SHADE | DG_FLAG_FINISHCALC)

extern BLOWBACK_INFO BlowBack_info[];

/************************************************************
  WeaponEfInitObject()
  解説：武器オブジェクトを初期化 ＆ 武器エフェクトの使用準備
  引数：管理構造体ポインタ,オブジェクトポインタ,
        rootオブジェクトポインタ,ユニット番号,武器種類
		その武器のモデルコード
  返値：0 設定		-1 失敗
  ***********************************************************/
int WeaponEfInitObject(WEAPON_EF_CTRL *pctrl,	/* 管理構造体のポインタ   */
					   OBJECT *pobj,			/* オブジェクトのポインタ  */
					   OBJECT *root,			/* rootオブジェクト  */
					   int unit,				/* ユニット番号  */
					   int kind,				/* 武器種類  */
					   int model)				/* その武器のモデルコード  */
{
	pctrl->root = root;

#if 0
extern int Magazines[];
printf("<<<<<%d>>>>>\n",Magazines[2]);
printf("<mkr_sk  :%d>\n",GV_StrCode("mkr_sk"));
printf("<usp_sub :%d>\n",GV_StrCode("usp_sub"));
printf("<scm_sub1:%d>\n",GV_StrCode("scm_sub"));
printf("<scm_sub2:%d>\n",GV_StrCode("scm_sub"));
printf("<m92_sub :%d>\n",GV_StrCode("m92_sub"));
printf("<fms     :%d>\n",GV_StrCode("fms"));
printf("<fms_sub :%d>\n",GV_StrCode("fms_sub"));

	BodyDatas[2]=(GV_StrCode("mkr_sk"));	/* m92_snk */
	BodyDatas[3]=(GV_StrCode("mkr_sk"));	/* m92_sub  */
#endif
	
	if(!pobj || !root)return -1;
	if(!root->objs)return -1;
	
	/* WP_*** => WEF_ID_***への変換  */
	switch(kind){
    case WP_Usp:		/* ID:0 ＵＳＰ  */
		pctrl->kind = WEF_ID_USP;
		break;
	case WP_Mkr:		/* ID:1 マカロフ  */
		pctrl->kind = WEF_ID_MKR;
		break;
    case WP_Famas:		/* ID:2 ファマス  */
		pctrl->kind = WEF_ID_FAMAS;
		break;
	case WP_Socom:		/* ID:3 ソーコム  */
		pctrl->kind = WEF_ID_SOCOM;
		break;
	case WP_m92:		/* ベレッタＭ９（麻酔銃）  */
		pctrl->kind = WEF_ID_M92;
		break;
	case WP_ShotGun_Near:		/* ショットガン（至近）  */
	case WP_ShotGun_Far:		/* ショットガン（遠方）  */
		pctrl->kind = WEF_ID_SHOTGUN;
		break;
	case WP_Glk:		/* グロック（ファットマン）  */
		pctrl->kind = WEF_ID_GLK;
		break;
	case WP_Abk:		/* アバカン  */
		pctrl->kind = WEF_ID_ABK;
		break;
    case WP_Rgb6:		/* ＲＧＢ＿６（Ｇランチャー）  */
		pctrl->kind = WEF_ID_RGB6;
		break;
    case WP_M4_Grd_Htc: // GRD付きＭ4 
		pctrl->kind = WEF_ID_M4_GRD_HTC;
		break;

#if 0
	case WP_None:		/* 無し  */
    case WP_Psg1:		/* ＰＳＧ－１ （スナイパーライフル） */
    case WP_Spp1M:		/* ＳＰＰ＿１Ｍ（水中銃）  */
    case WP_Nikita:		/* ニキータ  */
    case WP_Stinger:		/* スティンガー  */
		/* 10  */
    case WP_Claymore:	/* クレイモア地雷  */
    case WP_C4Bomb:		/* Ｃ４爆弾  */
    case WP_StealthCamera:	/* ステルスカメラ  */
    case WP_Balloon:		/* 風船ライデン  */
    case WP_ChaffGrenade:	/* チャフグレネード  */
    case WP_StunGrenade:	/* スタングレネード  */
    case WP_Mic:		/* 指向性マイク  */
    case WP_Blade:		/* 高周波ブレード  */
    case WP_ColdSpray:	/* 爆弾凍結スプレー  */
    case WP_MeltSpray:	/* 死体溶解スプレー  */
    /* 20  */
    case WP_FireExt:		/* 消化器  */
    case WP_Robot:		/* ロボット  */
#endif
	default:
		pctrl->kind = WEF_ID_NONE;
		break;
    }
	GM_InitObject( pobj , model , ( ( pctrl->kind < WEF_ID_MAX && WEF_ID_NONE < pctrl->kind ) ? BODY_FLAG2 : BODY_FLAG ) );
	if( !pobj->objs ){
		return ( -1 );
	}
	GM_ConfigObjectRoot( pobj, root, unit );
	/*
	if ( joint_flag == JOINT_ON ){ // 複数の間接があるもののみworld指定(自動計算) 
		pobj->objs->rots = &tmp2;
	}
	*/
	pctrl->motion=0;//BlowBack_info[pctrl->kind].motionnum; 
	pctrl->pobjs = pobj->objs;
	pctrl->flag2 = WPEF_FLG2_SHOTFIRST;
	pctrl->flag = 0;

	return 0;
}/*** WeaponEfInitObject() fin ***/



/************************************************************
  WeaponEfAct()
  解説：オブジェクトのエフェクトアクションを更新
  引数：武器エフェクト構造体のポインタ
  返値：なし
  ***********************************************************/
void WeaponEfAct(WEAPON_EF_CTRL *pctrl){
	BLOWBACK_INFO	*pinfo;

	if(!pctrl->kind)return;		/* アクションが無い武器は素通り  */
	
#ifdef WEAPON_EF_DEBUG_MODE
MENU_Locate( 32, 32, 0 ) ;
#endif

//printf("武器:1\n"); 
	/* 呼び出しフラグチェック  */
	if((pctrl->flag & WPEF_FLG_START)){
		pctrl->flag &= (~WPEF_FLG_START);
		pctrl->motion = BlowBack_info[pctrl->kind].motionnum;
		switch(pctrl->kind){
		case WEF_ID_USP:
		case WEF_ID_SOCOM:
			/* プレイヤーUSPのリロード判定  */
			if(GM_Magazine==0)pctrl->reload_wait = USP_WAIT_COUNT;
			break;
		case WEF_ID_M92:
			if(GM_Magazine==0)pctrl->reload_wait = M92_WAIT_COUNT;
			break;
		case WEF_ID_SHOTGUN:
			/* ショットガンリロード  */
			pctrl->flag3 = 1;
			pctrl->count = 0;
			break;
		case WEF_ID_GLK:
		case WEF_ID_ABK:
		case WEF_ID_RGB6:
			pctrl->reload_wait = 0;
			break;
		default:
			pctrl->flag2 &= (~WPEF_FLG2_SHOTFIRST);
			break;
		}
//		MENU_Printf("(%x:%x)\n",pctrl->flag,pctrl->flag2); 
	}

	pinfo = &BlowBack_info[pctrl->kind];
	if( ( pinfo->func ) && !( pctrl->flag & WPEF_ENEMY_UNREAL ) ){
		(*pinfo->func)( (void*)pctrl );			/* 武器別関数実行  */
	}
	// map by koba4 
	if ( ( pctrl->root != NULL ) && ( pctrl->pobjs != NULL ) ){
		GM_GroupObjs( pctrl->pobjs , pctrl->root->map_name );
	}
	if( pctrl->motion > 0 ){  // > を修正 
		if( pctrl->reload_wait ){
			if(pctrl->motion == pinfo->reload_waittime){
				if( GM_Magazine > 0 ){
					pctrl->reload_wait--;
				}
			}else{
				pctrl->motion--;
			}
		}else{
			pctrl->motion--;
		}
	}
#ifdef WEAPON_EF_DEBUG_MODE
MENU_Printf( "motion %d\n",(pctrl->motion) ) ;
MENU_Printf( "reload_wait %d\n",(pctrl->reload_wait) ) ;
MENU_Printf( "GM_Magazine %d\n", GM_Magazine) ;
#endif
}/*** WeaponEfAct() fin ***/

