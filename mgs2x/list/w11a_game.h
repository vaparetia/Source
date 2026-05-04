//
//	w11a_game.h	
//
//	written by H.Yoshiike 2000.Aug
//
//	$Id: w11a_game.h,v 1.17 2002/10/03 15:17:35 usr04761 Exp $

// c_effect用define
#define	EFFECT_CUSTUM			// c_effectのデフォルトdefineを使用しない
#define	E_FOOTSTAMP				// 足跡
#define	E_ENEMY					// 敵兵系
#define	E_ENEMY_LIGHT			// 敵兵ライト
#define	E_SLING					// スリング
#define	E_ENEMY_N_GL_LIGHT		// 敵兵暗視ゴーグル、マンハッタンライト
#define	E_C4BOMB				// Ｃ４爆弾、赤外線センサー
#define	E_CHARA_DENTOU			// chara 懐中電灯、フォーチュン戦
#define	E_WATER					// 水関係
#define	E_HIMATU				// 水飛沫
#define	E_DOOR_LUMP				// ドアランプ
#define	E_OTHER1				// 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴
#define	E_OTHER2				// プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫
#define	E_OTHER3				// ドッグタグ、ハリアエフェクト、ソリダスエフェクト
#define	NODE_LAMP				// ノードディスプレイランプ

#include	"w11abc_common.h"
#include	"w11ab_common.h"

// 敵兵タイプdefine
#define		ENE_P_GOL
#define		ENE_MAR		'-DDEL_SHOTGUN=DUMMY -DDEL_TNG=DUMMY -DDEL_SHIELD=DUMMY -DDEL_WALKMAN=DUMMY'
// キャラクタ 
#include	"ene_plant.h"

// 主観ゴーグル
//goods		raiden_gogle_sev/raiden_gogle_sev.kms
effect		scr_effect/raiden_gogle_lenz.kms

// 強制モーション
mtn		gps_w11a.mls			// デモ用敵兵アクション

// パッドデモ
#ifdef MGS2_VMODE_PAL
rpd		w11a_paddemo_p.rpd
#else
rpd		w11a_paddemo.rpd
#endif
// ワールド（昇降機ホール部分のみ）
// それ以外のモデル 
option makexti6 -l 1024 -f PAL8
pack_all	w11a2.tri world/w11a/w11a2 world/w11_common world/door/w02a_dr.dar
option makexti6

world		w11a/w11a2
world		w11_common
world		door/w02a_dr.kms
hazard		w11a.hzx

// オブジェ関係
// 小さいサイズのテクスチャはまとめておく

//パッドデモテクスチャ
#ifndef MGS2_XBOX
/* PS2 */
#ifdef MGS2_AREA_EU
// PAL版は各言語
pack_all	padtexture.tri \
			2D/act_telop/e/act_telop2_alp_ovl.bmp \
			2D/act_telop/e/act_telop3_alp_ovl.bmp \
			2D/act_telop/e/act_telop5_alp_ovl.bmp \
			2D/act_telop/pal/act_telop02_fr_alp_ovl.bmp \
			2D/act_telop/pal/act_telop03_fr_alp_ovl.bmp \
			2D/act_telop/pal/act_telop05_fr_alp_ovl.bmp \
			2D/act_telop/pal/act_telop02_de_alp_ovl.bmp \
			2D/act_telop/pal/act_telop03_de_alp_ovl.bmp \
			2D/act_telop/pal/act_telop05_de_alp_ovl.bmp \
			2D/act_telop/pal/act_telop02_it_alp_ovl.bmp \
			2D/act_telop/pal/act_telop03_it_alp_ovl.bmp \
			2D/act_telop/pal/act_telop05_it_alp_ovl.bmp \
			2D/act_telop/pal/act_telop02_es_alp_ovl.bmp \
			2D/act_telop/pal/act_telop03_es_alp_ovl.bmp \
			2D/act_telop/pal/act_telop05_es_alp_ovl.bmp

#elif defined( MGS2_KOREA )
// 韓国版
pack_all	padtexture.tri \
			2D/act_telop/kor/act_telop2_alp_ovl.bmp \
			2D/act_telop/kor/act_telop3_alp_ovl.bmp \
			2D/act_telop/kor/act_telop5_alp_ovl.bmp

#elif defined( MGS2_AREA_US )
// アメリカ版
pack_all	padtexture.tri 2D/act_telop/e

#else
// 日本版
pack_all	padtexture.tri \
				2D/act_telop/e/act_telop2_alp_ovl.bmp \
				2D/act_telop/e/act_telop3_alp_ovl.bmp \
				2D/act_telop/e/act_telop5_alp_ovl.bmp \
				2D/act_telop/j/act_telop02_jp_alp_ovl.bmp \
				2D/act_telop/j/act_telop03_jp_alp_ovl.bmp \
				2D/act_telop/j/act_telop05_jp_alp_ovl.bmp 

#endif
#else
/* XBOX */
#ifdef MGS2_AREA_US
// アメリカ版
pack_all	padtexture.tri 2D/act_telop/x_eng
#endif
#ifdef MGS2_AREA_EU
// ヨーロッパ版
pack_all	padtexture.tri \
			2D/act_telop/x_eng/act_telop2_alp_ovl.bmp \
			2D/act_telop/x_eng/act_telop3_alp_ovl.bmp \
			2D/act_telop/x_eng/act_telop5_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop02_fr_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop03_fr_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop05_fr_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop02_de_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop03_de_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop05_de_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop02_it_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop03_it_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop05_it_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop02_es_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop03_es_alp_ovl.bmp \
			2D/act_telop/x_pal/act_telop05_es_alp_ovl.bmp
#endif
#endif

		
// デバッグ用

// 小さいサイズのテクスチャはまとめておく

// リネーム
rename	gps_w11a.mar gbsstage.mar
