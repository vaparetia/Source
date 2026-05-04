//
//	w31ad_common.h	
//
//	written by H.Yoshiike 2000.Nov
//
//	$Id: w31ad_common.h,v 1.16 2002/09/11 12:13:37 usr03682 Exp $


// c_effect用define
#define	EFFECT_CUSTUM			// c_effectのデフォルトdefineを使用しない
#define	E_GLASS_DANKON			// グラス系弾痕
#define	E_KOGETUKI				// 壁への焦げ付き
#define	E_FOOTSTAMP				// 足跡
#define	E_ENEMY					// 敵兵系
#define	E_ENEMY_LIGHT			// 敵兵ライト
#define	E_SLING					// スリング
#define	E_ENEMY_N_GL_LIGHT		// 敵兵暗視ゴーグル、マンハッタンライト
#define	E_DOOR_LUMP				// ドアランプ
#define	E_OTHER1				// 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴
#define	E_OTHER2				// プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫
#define	E_OTHER3				// ドッグタグ、ハリアエフェクト、ソリダスエフェクト
#define	NODE_LAMP				// ノードディスプレイランプ

// 各ステージ共通要素
#include	"p_common.h"
#include	"common.h"
#include	"c_effect.h"

// 強制モーション
mtn		rai_w31a.mls

// ワールド
// マルチテクスチャモデル
// それ以外のモデル 
option makexti6 -l 1024 -f PAL8
pack_all	w31a0.tri world/w31a/w31a0 world/w31a/obj
pack_all	w31a_other.tri world/elevator/elv_2 world/door/dr_free.dar world/door/w31a_dr00.dar meca/gun_camera
option makexti6

world		w31a/w31a0

option makexti6 -l 1024 -f PAL8
world		w31a/w31a1
option makexti6

world		w31a/obj
world		elevator/elv_2
world		door/dr_free.kms
world		door/w31a_dr00.kms
lt2			w31a.lt2
hazard		w31a.hzx

// オブジェクト
pack_all	gun_camera.tri \
			meca/gun_camera \
			effect/ray_effect/ray_eye_bonbori_alp.bmp

meca		gun_camera

// アイテム
itembox

// 振動データ
vib       etc/core_ele_01.vib
vib       etc/core_ele_02.vib
vib       etc/core_ele_03.vib
		
// デバッグ用
// 小さいサイズのテクスチャはまとめておく


// リネーム
