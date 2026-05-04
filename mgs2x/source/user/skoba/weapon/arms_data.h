/*
    arms_data.h
	武器のデータヘッダー

	$Id: arms_data.h,v 1.1.1.3 2002/11/19 11:50:26 Yoshizawa1 Exp $
*/

#ifndef ARMS_DATA_H
#define ARMS_DATA_H
/*
//デモに出てくる奴(弾丸系)
　m92_snk	M9麻酔銃		マズルフラッシュ、排莢なし　煙あり

　usp		USP				ピストル弾用マズルフラッシュと排莢、煙
　scm		ソコム			ピストル弾用マズルフラッシュと排莢、煙
　scm_s		ソコム			ピストル弾用マズルフラッシュと排莢、煙
　mkr		マカロフ		ピストル弾用マズルフラッシュと排莢、煙
　m92		M9				ピストル弾用マズルフラッシュと排莢、煙	ない
　saa		ＳＡＡ			ピストル弾用マズルフラッシュと排莢、煙	

　fms		ファマス		ピストル弾用マズルフラッシュと排莢、煙
　abk		アバカンAN94	機銃用マズルフラッシュと排莢、煙
　aks		AKS_74U			機銃用マズルフラッシュと排莢、煙
　aks_s		AKS_74U			機銃用マズルフラッシュと排莢、煙	要チェック
　m4a_nm	m4a1			機銃用マズルフラッシュと排莢、煙
　p90		P９０			機銃用マズルフラッシュと排莢、煙

　stg		スティンガー	スティンガー用エフェクト（準備済み）	知らん
　m4b_gl	m4a1グレネードランチャー 					ランチャー用汎用エフェクト（未製作）
  m4b_gun   m4銃口
  m4b_gun_grenade   m4銃口
  m4b_gun_sel       m4銃口
　m4b_gl_sel	m4a1グレネードランチャー 					ランチャー用汎用エフェクト（未製作）
  m4a_gun_snk   m4a1銃口 snake用
  m4a_gl_snk   m4a1グレネードランチャー snake用
  m4a_gun_hi   m4a1銃口 ハイテク兵用

  psg PSG snake - raiden 共用  
  p90_solidas ソリダス用
  Ray_Vlcn RAY用バルカン
  lnr		リニアライフル（フォーチュン使用武器)		エフェクト未製作
*/

enum {
	// 麻酔銃
	ARMS_DATA_M92_SNA = 0,  // 0
	// 弾丸
	ARMS_DATA_USP,          // 1
	ARMS_DATA_SCM,          // 2
	ARMS_DATA_SCM_S,        // 3
	ARMS_DATA_MKR,          // 4
	ARMS_DATA_M92,          // 5
	ARMS_DATA_SAA,          // 6
	// 機銃
	ARMS_DATA_FMS,          // 7
	ARMS_DATA_ABK,          // 8
	ARMS_DATA_AKS,          // 9
	ARMS_DATA_AKS_S,        // 10
	ARMS_DATA_M4A_NM,       // 11
	ARMS_DATA_P90,          // 12
	// 特殊
	ARMS_DATA_STG,          // 13
	ARMS_DATA_M4B_GL,       // 14
	ARMS_DATA_LNR,          // 15

	// 追加
	ARMS_DATA_M4B_GUN,         // 16
	ARMS_DATA_M4B_GUN_GRENADE, // 17
	ARMS_DATA_M4B_GUN_SEL,     // 18
	ARMS_DATA_M4B_GL_SEL,      // 19
	ARMS_DATA_M4A1_GUN_SNK,     // 20
	ARMS_DATA_M4A1_GL_SNK,     // 21
	ARMS_DATA_M4A1_GUN_HI,     // 22

	ARMS_DATA_PSG,     // 23
	ARMS_DATA_P90_SOL,         // 24
	ARMS_DATA_RAY_VLCN,        // 25
	// 尻
	ARMS_DATA_BOTTOM,          // 26
};

	//	{ 17.5f,    0.0f, -23.0f, 1.0f}, p90

#endif
