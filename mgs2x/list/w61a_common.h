// w61a_common.h
// ソリダス戦
// 2001/03/29 H.Yoshiike
// $Id: w61a_common.h,v 1.5 2002/07/05 05:22:59 usr03379 Exp $


// c_effect用define
#define	EFFECT_CUSTUM			// c_effectのデフォルトdefineを使用しない
#define	E_KOGETUKI				// 壁への焦げ付き
#define	E_FOOTSTAMP				// 足跡
#define	E_ENEMY_LIGHT			// 敵兵ライト、スティンガー、ニキータ、剣の火花、しょんべん兵
#define	E_ENEMY_N_GL_LIGHT		// 敵兵暗視ゴーグル、マンハッタンライト
#define	E_MANHATTAN_LIGHT		// マンハッタンライト
#define	E_WATER_MINE			// クロスフェード（甲板のライトで使用）＆ 水中機雷
#define	E_CROSS_FADE_LIGHT		// クロスフェード（甲板のライトで使用）
#define	E_SKY_NORMAL			// 空
#define	E_TENKYUU				// 天球ドーム
#define	E_SUN					// 太陽
#define	E_OTHER1				// 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴
#define	E_OTHER2				// プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫
#define	E_OTHER3				// ドッグタグ、ハリアエフェクト、ソリダスエフェクト


// 各ステージ共通要素
#include	"p_common.h"
#include	"common.h"
#include	"c_effect.h"

// キャラクタ


// ワールド
// マルチテクスチャモデル
// それ以外のモデル
pack_all	w61a_other.tri \
			world/w61a/w61a_other1.dar \
			world/w61a/w61a_gw_statue.dar

world		w61a
lt2			w61a.lt2
hazard		w61a.hzx

// 旗用頂点データ
cvd world/w61a/w61a_flag.cv2 vn

// オブジェクト
// 小さいサイズのテクスチャはまとめておく


// デバッグ用
//カタナ
pack_trnall	katana.tri \
			debug/shibata/staffroll/alpha03_alp_ovl.bmp

pack_all	hfb_mt.tri \
            weapon/hfb/hfb_mt.dar \
            weapon/hfb/hfb_mineuchi_mt.dar \
			weapon/hfb/hfb_sub_mt.dar \
			weapon/hfb/hfb_mineuchi_sub_mt.dar

weapon		hfb/hfb_mt.kms
weapon		hfb/hfb_mineuchi_mt.kms
weapon		hfb/hfb_sub_mt.kms
weapon		hfb/hfb_mineuchi_sub_mt.kms

mtn			rai_blade.mls

//ライデン手錠
pack_all	tejyou.tri \
            goods/demo_tejyou/demo_tjo_rai_broken_r.dar \
            goods/demo_tejyou/demo_tjo_rai_broken_l.dar
goods		demo_tejyou/demo_tjo_rai_broken_r.kms 
goods		demo_tejyou/demo_tjo_rai_broken_l.kms 

cvd human/sna_skl/sna_skl2.cv2 vn

debug	kimura/raiden_gogle/raiden_gogle_sev.kms

// リネーム
