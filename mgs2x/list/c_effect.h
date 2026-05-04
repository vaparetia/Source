//
//	c_effect.h
//
//	written by H.Yoshiike 1999.Oct
//       $Id: c_effect.h,v 1.89 2002/08/13 10:13:21 usr04249 Exp $
//

// 各ステージ共通エフェクト群(srcファイルの最初の方でinclude)
// エフェクト類はまとめておく

//------------------------------------------------------------
// 大まかなステージ分類によるdefine設定
//------------------------------------------------------------

#ifdef EFFECT_CUSTUM		// 個々のsrcで設定

#elif defined( EFFECT_TANKER_OUT )

#elif defined( EFFECT_TANKER_IN )

#elif defined( EFFECT_PLANT_OUT )

#elif defined( EFFECT_PLANT_IN )

#elif defined( EFFECT_PLANT_WATER )

#else
// デフォルト状態では天球以外のすべてのエフェクトが入ってしまう
#define	E_GLASS_DANKON			// グラス系弾痕
#define	E_KOGETUKI				// 壁への焦げ付き
#define	E_FOOTSTAMP				// 足跡
#define	E_ENEMY					// 敵兵系
#define	E_ENEMY_LIGHT			// 敵兵ライト、スティンガー、ニキータ、剣の火花、しょんべん兵
#define	E_SLING					// スリング
#define	E_ENEMY_N_GL_LIGHT		// 敵兵暗視ゴーグル、マンハッタンライト
#define	E_MANHATTAN_LIGHT		// マンハッタンライト
#define	E_C4BOMB				// Ｃ４爆弾、赤外線センサー
#define	E_WATER_MINE			// クロスフェード（甲板のライトで使用）＆ 水中機雷 解体Ｃ４
#define	E_CROSS_FADE_LIGHT		// クロスフェード（甲板のライトで使用）
#define	E_CHARA_DENTOU			// chara 懐中電灯、フォーチュン戦
#define	E_WATER					// 水関係
#define	E_CHARA_HAMON			// chara 波紋
#define	E_BROKEN_OBJECT			// 壊れ物
#define	E_BROKEN_PAPEROBJECT	// 壊れ物紙
#define	E_BROKEN_PLATE			// 壊れ物皿、小麦粉
#define	E_HIMATU				// 水飛沫
#define	E_CHARA_MADOAME			// chara 窓雨
#define	E_STEAM					// 粉 & 水蒸気
#define	E_POWDER				// 小麦粉
#define	E_SKY_NORMAL			// 空
#define	E_LINEAR_GUN			// リニアガン
#define	E_TENKYUU				// 天球ドーム
#define	E_SUN					// 太陽
#define	E_DOOR_LUMP				// ドアランプ
#define	E_OTHER1				// 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴
#define	E_OTHER2				// プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫
#define	E_OTHER3				// ドッグタグ、ハリアエフェクト、ソリダスエフェクト
#define	BONBORI					// ぼんぼり
#define	NODE_LAMP				// ノードディスプレイランプ
#endif



//------------------------------------------------------------
// 以下実際のpack部分
//------------------------------------------------------------

// 血
pack_trnall	effect.tri \
	effect/blood/blood_1bw_msk.bmp \
	effect/blood/blood_2_msk.bmp \
	effect/blood/blood_2bw_msk.bmp \
	effect/blood/blood_2bw_alp.bmp \
	effect/blood/blood_1bw_msk.bmp \
	effect/blood/chi01_msk.bmp \
	effect/blood/chi02_msk.bmp \
	effect/blood/chi03_msk.bmp \
	effect/blood/chi04_msk.bmp \
	effect/blood/chi05_msk.bmp \
	effect/blood/chi01_alp.bmp \
	effect/blood/chi02_alp.bmp \
	effect/blood/chi03_alp.bmp \
	effect/blood/chi04_alp.bmp \
	effect/blood/chi05_alp.bmp \
	effect/katchap/ketchap_g.bmp \
	effect/katchap/ketchap00_alp.bmp


// マズルフラッシュ
pack_trnall	effect.tri \
	effect/weapon_effect/socom_f_msk.bmp

// 煙、跳弾
pack_trnall	effect.tri \
	effect/smoke/smoke_msk.bmp /* 息、タンカー煙突煙、盾壊れ、赤外センサー壊れ、跳弾で使用 */\
	effect/color/col128_add.bmp /* 盾跳弾、通常跳弾、ソリダス戦跳弾 */

// 弾痕
pack_trnall	effect.tri \
	effect/dankon/dankon2_alp.bmp /* 通常弾痕 */
#ifdef E_GLASS_DANKON
pack_trnall	effect.tri \
	effect/dankon/g_dankon_alp.bmp /* グラス用弾痕 */ \
	effect/frost/frost02_alp.bmp /* ガラスに冷却スプレーをかけたときの霜 */
#endif

// 爆発系
pack_trnall	effect.tri \
	effect/bakuha/fire1_alp.bmp /* 通常爆発で使用（Ｃ４とか） */\
	effect/bakuha/bombgas6_alp.bmp /* 通常爆発で使用（Ｃ４とか） */\
	effect/bakuha/bombpowder7_msk.bmp /* 通常爆発で使用（Ｃ４とか） */\
	effect/bakuha/bombpowder1_msk.bmp /* 銃系で使用 */
#ifdef E_KOGETUKI
pack_trnall	effect.tri \
	effect/bakuha/bombpowder2_msk.bmp /* 壁への焦げ付き（現在は起動しない） */
#endif

// スタン爆発 & 霧雨 & 蒸気
pack_trnall	effect.tri \
	effect/fog/fog02_msk.bmp /* 霧雨、水蒸気、スタン爆 */

// チャフ爆発 & 雨
pack_trnall	effect.tri \
	effect/rain/rain01_msk.bmp /* 雨全般、チャフ爆発 */

// ぼんぼり
pack_trnall	effect.tri \
	effect/bonbori/rcm_l_msk.bmp /* 無数に使用 */\
	effect/bonbori/svc_bonbori.bmp /* フォーチュン戦：監視カメラ等：ハリアー：ソリダス等々 */

// 足跡
#ifdef E_FOOTSTAMP
/* 常駐行です */
//pack_trnall	effect.tri \
//	effect/footstamp/fstamp03_alp.bmp /* 足跡 */
#endif

// 敵兵
#ifdef E_ENEMY
pack_trnall	effect.tri \
	effect/mark/at_break_alp.bmp /* !破壊 */\
	effect/mark/qt_break_alp.bmp /* ?破壊 */\
	effect/mark/at_normal_alp.bmp /* ! */\
	effect/mark/qt_normal_alp.bmp /* ? */\
	effect/mark/hosi_msk.bmp /* 気絶星 */\
	effect/mark/kirari_msk.bmp /* 発見目ピカッ */\
	effect/mark/zzz_msk.bmp /* 睡眠ZZZ */ \
	effect/drop/drop01_alp_mod1201.bmp /* びっくり壊し */ \
	effect/color/col128_alp.bmp		/* びっくり壊し */ \
	effect/powder/powder02_alp.bmp /* ＳＡＡ、水蒸気攻撃、消火器、デモたばこ スタン E_STEAMにもあります */ \
	effect/color/col256_bld100.bmp /* プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫 E_OTHER2にもあります */
#endif

// 敵兵のライト、スティンガー、ニキータ、剣の火花、しょんべん兵
#ifdef E_ENEMY_LIGHT
pack_trnall	effect.tri \
	effect/light/light05_msk.bmp /* 敵兵のライトに使用 */\
	effect/light/light10_msk.bmp /* 敵兵のライトに使用 */\
	effect/color/col128_alp.bmp /* 敵兵のライトでプレイヤーの身体から影：濁流 */
#endif

// スリング(敵兵、人質)
#ifdef E_SLING
pack_trnall	effect.tri \
	effect/aks_sling/gbs_ak_sling2.bmp
#endif

// 敵兵暗視ゴーグル、マンハッタン
#ifdef E_ENEMY_N_GL_LIGHT
pack_trnall	effect.tri \
	effect/light/light08_msk.bmp /* 「敵兵暗視ゴーグル」「w00aマンハッタン等」に使用 */\
	effect/plasma/plasma_msk.bmp /* 「敵兵暗視ゴーグル」*/ \
	effect/fog/w00_fog_fader_alp.bmp	//マンハッタン遠くを消す
#endif
#ifdef E_MANHATTAN_LIGHT
pack_trnall	effect.tri \
	effect/light/light13_msk.bmp /* 「w00aマンハッタン等」に使用 */
#endif

// 赤外線センサー、トラップＣ４で使用
#ifdef E_C4BOMB
pack_trnall	effect.tri \
	effect/light/xlit01b_msk.bmp /* 赤外線センサー、トラップＣ４で使用 */\
	effect/laser_sight/lsight_msk.bmp /* 赤外線センサーで使用(常駐にも通常存在) */
#endif

// クロスフェード（甲板のライトで使用）＆ 水中機雷
#ifdef E_WATER_MINE
pack_trnall	effect.tri \
	effect/light/light12_msk.bmp /* 甲板のライトで使用 */\
	effect/light/xlit04a_alp.bmp /* 甲板のライト & 水中機雷で使用 */\
	effect/light/xlit04b_alp.bmp /* 甲板のライト & 水中機雷で使用 */\
	effect/light/xlit05a_alp.bmp /* 甲板のライトで使用 */\
	effect/light/xlit05b_alp.bmp /* 甲板のライトで使用 */

#endif

// クロスフェード（甲板のライトで使用）
#ifdef E_CROSS_FADE_LIGHT
pack_trnall	effect.tri \
	effect/light/light12_msk.bmp /* 甲板のライトで使用 */
#endif

// chara 懐中電灯
#ifdef E_CHARA_DENTOU
pack_trnall	effect.tri \
	effect/light/light03_msk.bmp /* 「chara 懐中電灯」のみ使用 */\
	effect/drop/drop01_alp2_mod1021.bmp /* フォーチュン戦、懐中電灯 */
#endif

// 水関係
#ifdef E_WATER
pack_trnall	effect.tri \
	effect/ripple/ripple02_alp.bmp /* 波紋 水面があるところすべて必要 */\
	effect/hamon/hamon09_add_alp.bmp /* 水面が見えるところ全部 */ \
	effect/wave/wave12_alp_ovl.bmp	/* 弾水飛沫処理用 */
#endif

#ifdef E_CHARA_HAMON
pack_trnall	effect.tri \
	effect/hamon/hamon04_add_msk.bmp /* chara 波紋のみ使用 */\
	effect/hamon/hamon05_add_msk.bmp /* chara 波紋のみ使用 */
#endif

// 壊れ物
#ifdef E_BROKEN_OBJECT
pack_trnall	effect.tri \
	effect/splash/splash03_alp.bmp /* フォーチュン戦：ワイン等の壊れ物 */\
	effect/splash/splash05_alp.bmp
#endif
#ifdef E_BROKEN_PAPEROBJECT
pack_trnall	effect.tri \
	effect/splash/splash04_alp.bmp /* 紙（への攻撃）*/
#endif
#ifdef E_BROKEN_PLATE
pack_trnall	effect.tri \
	effect/powder/powder03_alp.bmp /* 皿割れ 小麦粉 */
#endif

// 水飛沫
#ifdef E_HIMATU
pack_trnall	effect.tri \
	effect/splash/splash06_alp.bmp /* 身体、ダンボール、アイテムＢＯＸ、タンカーレーダー等 への跳ね返り水飛沫 */\
	effect/splash/splash06_msk.bmp /* 主観武器用水飛沫：オルガ戦水飛沫 */\
	effect/splash/splash07_alp.bmp /* バンププール：水飛沫系：メタル汚水発射 */
#endif

// 窓雨
#ifdef E_CHARA_MADOAME
pack_trnall	effect.tri \
	effect/rain/wrain01_add_alp_ovl.bmp /*「chara 窓雨」にのみ使用 */\
	effect/rain/wrain02_add_alp_ovl.bmp /*「chara 窓雨」にのみ使用 */\
	effect/rain/wrain03_add_alp_ovl.bmp /*「chara 窓雨」にのみ使用 */\
	effect/rain/wrain04_add_alp_ovl.bmp /*「chara 窓雨」にのみ使用 */\
	effect/rain/wrain05_add_alp_ovl.bmp /*「chara 窓雨」にのみ使用 */\
	effect/rain/wrain06_add_alp_ovl.bmp /*「chara 窓雨」にのみ使用 */
#endif

// 粉 & 水蒸気
#ifdef E_STEAM
pack_trnall	effect.tri \
	effect/powder/powder02_alp.bmp /* ＳＡＡ、水蒸気攻撃、消火器、デモたばこ */
#endif

// 小麦粉
#ifdef E_POWDER
pack_trnall	effect.tri \
	effect/powder/powder01_alp.bmp /* 粉テクスチャ 消火器でも使用 */ \
	effect/powder/powfog03_add.bmp /* 小麦粉 */ \
	effect/powder/powder04_alp.bmp /* 小麦粉 */
#endif

// 空
#ifdef E_SKY_NORMAL
pack_trnall	effect.tri \
	effect/sky/w00_sky_add_alp.bmp /* 「chara 新空」でのみ使用 */
#ifdef MGS2_XBOX
pack_trnall	effect.tri \
	effect/cloud/w00_sky_add.bmp /* XBOXの「thunder_flash.c」でのみ使用 */
#else
world   w00/w00_sky.kms
#endif
#endif

// リニアガン
#ifdef E_LINEAR_GUN
pack_trnall	effect.tri \
	effect/sonic/sonic02_alp.bmp /* リニアガン */
#endif

// 太陽
#ifdef E_SUN
pack_trnall	effect.tri \
	debug/shibata/sky/sun_alp.bmp /* 2000.12.15 柴田実験/空テクスチャ */

pack_trnall	flare.tri \
effect/flare/flare_b1_msk.bmp /* 2001.05.31 向手追加/レンズフレア */ \
effect/flare/flare_b2_msk.bmp \
effect/flare/flare_g1_msk.bmp \
effect/flare/flare_g2_msk.bmp \
effect/flare/flare_r1_msk.bmp \
effect/flare/flare_r2_msk.bmp
#endif

// ドアランプ
#ifdef E_DOOR_LUMP
pack_trnall	effect.tri world/door/door_lamp_alp.bmp
#endif

// 天球ドーム
#ifdef E_TENKYUU
pack_trnall	effect.tri \
	debug/shibata/sky/sky_1_alp.bmp /* 「chara 天球ドーム」 */\
	debug/shibata/sky/sky_2_alp.bmp /* 「chara 天球ドーム」 */\
	debug/shibata/sky/sky_3_alp.bmp /* 「chara 天球ドーム」 */\
	debug/shibata/sky/sky_4_alp.bmp /* 「chara 天球ドーム」 */
#endif

//	朝空
#ifdef MORNING_SKY
pack_trnall	effect.tri \
	effect/plant_sky/sky_m1_alp.bmp \
	effect/plant_sky/sky_m2_alp.bmp \
	effect/plant_sky/sky_m4_alp.bmp \
	effect/plant_sky/sky_sun_m_alp.bmp \

world	common_sky/m_sky_wall1.kms
#endif

//	昼空
#ifdef DAY_SKY
pack_trnall	effect.tri \
	effect/plant_sky/sky_d1_alp.bmp \
	effect/plant_sky/sky_d2_alp.bmp \
	effect/plant_sky/sky_d4_alp.bmp \
	effect/plant_sky/sky_sun_d_alp.bmp \

world	common_sky/d_sky_wall1.kms
#endif

//	夕空
	//	旧
#ifdef EVENING_SKY
pack_trnall	effect.tri \
	effect/plant_sky/sky_e1_alp.bmp \
	effect/plant_sky/sky_e2_alp.bmp \
	effect/plant_sky/sky_e4_alp.bmp \
	effect/plant_sky/sky_sun_e_alp.bmp \

world	common_sky/e_sky_wall1.kms
#endif

	//	w21b専用
#ifdef W21B_EVENING_SKY
pack_trnall	effect.tri \
	effect/plant_sky/sky_e1_alp.bmp \
	effect/plant_sky/sky_e2_alp.bmp \
	effect/plant_sky/w21b_sky_e3_alp.bmp \
	effect/plant_sky/sky_e4_alp.bmp \
	effect/plant_sky/sky_sun_e_alp.bmp \

world	common_sky/w21b_sky_wall1.kms
world	common_sky/w21b_sky_wall2.kms
world	common_sky/w21b_sky_wall3.kms
world	common_sky/w21b_sky_wall4.kms
#endif
	//	w32a専用
#ifdef W32A_EVENING_SKY
pack_trnall	effect.tri \
	effect/plant_sky/sky_e1_alp.bmp \
	effect/plant_sky/sky_e2_alp.bmp \
	effect/plant_sky/sky_e4_alp.bmp \
	effect/plant_sky/sky_sun_e_alp.bmp \

world	common_sky/w32a_sky_wall1.kms
world	common_sky/w32a_sky_wall2.kms
world	common_sky/w32a_sky_wall3.kms
#endif

	//	w25d専用
#ifdef W25D_EVENING_SKY
pack_trnall	effect.tri \
	effect/plant_sky/sky_e1_alp.bmp \
	effect/plant_sky/sky_e2_alp.bmp \
	effect/plant_sky/w25d_sky_e3_alp.bmp \
	effect/plant_sky/sky_e4_alp.bmp \
	effect/plant_sky/sky_sun_e_alp.bmp \

world	common_sky/w25d_sky_wall1.kms
world	common_sky/w25d_sky_wall2.kms
world	common_sky/w25d_sky_wall3.kms
#endif

//	夜空（太陽沈むデモ限定）
#ifdef NIGHT_SKY_DEMO
pack_trnall	effect.tri \
	world/w32c/w32c_sky/sky_n1_alp.bmp \
	world/w32c/w32c_sky/sky_n2_alp.bmp \
	world/w32c/w32c_sky/sky_n4_alp.bmp \

world	common_sky/e_sky_wall1.kms
#endif

// そのほか色々使いまわし系（chara特定不能）
#ifdef E_OTHER1
pack_trnall	effect.tri \
	effect/drop/drop01_msk.bmp /* 水面、水中、リニアガン、雨、フェード、バルカン人形、ダンボール主観、濁流、ソリダス剣（予定）、カメラ前の水滴 */\
	effect/drop/drop02_msk.bmp /* カメラ前の水滴 */
#endif
#ifdef E_OTHER2
pack_trnall	effect.tri \
	effect/color/col256_bld100.bmp /* プラズマ、剣残像、カミナリ、衝撃波、落下物水飛沫 */
#endif
#ifdef E_OTHER3
pack_trnall	effect.tri \
	effect/flare/flare0_msk.bmp /* ドッグタグ、ハリアエフェクト、ソリダスエフェクト */
#endif

#ifdef NODE_LAMP
pack_trnall	effect.tri \
	effect/node_effect/node_scr_awake_alp.bmp
#endif


// 以下、デフォルトでは入らないもの
//-----------------------------------------------------

// 巨大なためデフォルトでは中に入れない
// 天球
#ifdef ENV
pack_trnall	effect.tri \
	effect/env/env01a_msk.bmp \
	effect/env/env01b_msk.bmp
#endif


// タンカーでのぼんぼり
#ifdef BONBORI
pack_trnall	effect.tri \
	effect/light/light07_msk.bmp
#endif

//XBOX用の足影(すべてのステージに必要)
#ifdef MGS2_XBOX_1
pack_trnall	effect.tri \
	effect/shdw/foot_shadow_alp.bmp
#endif


//	effect/light \
//	effect/oldies \

// 0502時点でコメントアウトされていたもの
//	effect/blood/chi01msu.bmp \
//	effect/hamon/hamon04h_add.bmp \
//	effect/hamon/hamon05h_add.bmp \
//	effect/bubble \
//	effect/bakuha/bomb1_fl.bmp \
//	effect/bakuha/baku_g07_msk.bmp \
//	effect/light/xlit05a_alp.bmp \
//	effect/bakuha/fire3_alp.bmp \
//	effect/splash/splash03_msk.bmp \
//	effect/splash/splash04_msk.bmp \
//	effect/splash/splash05_msk.bmp \
// 0530時点でコメントアウトしたもの
//	effect/mark/zzz.bmp \
//	effect/splash/splash03.bmp \

