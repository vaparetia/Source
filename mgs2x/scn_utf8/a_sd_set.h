/*
	a_sd_set.h
	    壁床音設定など音声にかかわる設定(アナザー/ＶＲミッション用)

	2002/03/08 H.Yoshiike
	$Id: a_sd_set.h,v 1.12 2002/09/25 07:25:37 usr03806 Exp $
*/

// ファイルが二重呼びされたときの対処
#ifndef d:A_SE_SET_H
#define A_SE_SET_H	1

/* 以下のdefineがvardef.hでされています。
// 壁床効果音設定関係
// ------------------------------------
// 音を鳴らす対象タイプ -type
#define		SE_FLOOR		0	// 床が対象
#define		SE_WALL			1	// 壁が対象
*/

/* 以下のdefineがcommdef.hでされています。
// MT_SetMotionSeTable のキャラID
#define MT_SETABLE_PLAYER	(0)		// プレイヤー
#define MT_SETABLE_ENE_0	(1)		// 敵兵０
#define MT_SETABLE_ENE_1	(2)		// 敵兵１
#define MT_SETABLE_ENE_2	(3)		// 敵兵２
#define MT_SETABLE_ENE_3	(4)		// 敵兵３

#define MT_SETABLE_GLL_0	(5)		// 怪獣０
#define MT_SETABLE_GLL_1	(6)		// 怪獣１
#define MT_SETABLE_GLL_2	(7)		// 怪獣２
#define MT_SETABLE_GLL_3	(8)		// 怪獣３

#define MT_SETABLE_NPC_EMMA	(32)	// NPCエマ
#define MT_SETABLE_NPC_SNA	(33)	// NPCスネーク

#define MT_SETABLE_ORGA		(34)	// オルガ
#define MT_SETABLE_FORT		(35)	// フォーチュン
#define MT_SETABLE_FAT		(36)	// ファットマン
#define MT_SETABLE_VAMP		(37)	// バンプ
#define MT_SETABLE_SOL		(38)	// ソリダス
#define MT_SETABLE_OCELOT	(39)	// オセロット
*/


/////////////////////////////////////////////////////////
// ＶＲ
/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// スニーキングミッション
/////////////////////////////////////////////////////////
#ifdef d:STAGE_VS01A || d:STAGE_VS02A || d:STAGE_VS03A || d:STAGE_VS04A || d:STAGE_VS05A
/////////////////////////////////
// ＶＲスニーキングミッション１～５面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif

#ifdef d:STAGE_VS06A || d:STAGE_VS07A || d:STAGE_VS08A || d:STAGE_VS09A || d:STAGE_VS10A
/////////////////////////////////
// ＶＲスニーキングミッション６～１０面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif

/////////////////////////////////////////////////////////
// 武器訓練ミッション
/////////////////////////////////////////////////////////
#ifdef d:STAGE_WP01A || d:STAGE_WP02A || d:STAGE_WP03A || d:STAGE_WP04A || d:STAGE_WP05A
/////////////////////////////////
// ＵＳＰ／ソコム１～５面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}

#endif

#ifdef d:STAGE_WP11A || d:STAGE_WP12A || d:STAGE_WP13A || d:STAGE_WP14A || d:STAGE_WP15A
/////////////////////////////////
// Ｍ４/ＡＫＳ１～５面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif

#ifdef d:STAGE_WP21A || d:STAGE_WP22A || d:STAGE_WP23A || d:STAGE_WP24A || d:STAGE_WP25A
/////////////////////////////////
// Ｃ４/クレイモア１～５面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif

#ifdef d:STAGE_WP31A || d:STAGE_WP32A || d:STAGE_WP33A || d:STAGE_WP34A || d:STAGE_WP35A
/////////////////////////////////
// ＰＳＧ１/ＰＳＧ１-Ｔ１～５面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif

#ifdef d:STAGE_WP41A || d:STAGE_WP42A || d:STAGE_WP43A || d:STAGE_WP44A || d:STAGE_WP45A
/////////////////////////////////
// ＲＧＢ６/グレネード１～５面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO
}
#endif

#ifdef d:STAGE_WP51A || d:STAGE_WP52A || d:STAGE_WP53A || d:STAGE_WP54A || d:STAGE_WP55A
/////////////////////////////////
// スティンガー１～５面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO
}
#endif

#ifdef d:STAGE_WP61A || d:STAGE_WP62A || d:STAGE_WP63A || d:STAGE_WP64A || d:STAGE_WP65A
/////////////////////////////////
// ニキータ１～５面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO
}
#endif

#ifdef d:STAGE_WP71A || d:STAGE_WP72A || d:STAGE_WP73A || d:STAGE_WP74A || d:STAGE_WP75A
/////////////////////////////////
// 刀/素手１面
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif


/////////////////////////////////////////////////////////
// スペシャル（バラエティ）
/////////////////////////////////////////////////////////
#ifdef d:STAGE_SP01A || d:STAGE_SP02A
/////////////////////////////////
// アスレチック１＆アスレチック２
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO
}
#endif

#ifdef d:STAGE_SP03A || d:STAGE_SP04A
/////////////////////////////////
// ザコサバイバル
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

	// ゴルルゴン
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTL \
		-data	d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTR \
		-data	d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO
	// 歩き衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLGACHA \
		-data	d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO
	// 上体衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLKINUZ \
		-data	d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO
	// 咆え -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLHOWL1 \
		-data	d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO
	// 唸り -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLBRESS \
		-data	d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO
	// ダウン -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_A_GLLDOWN1 \
		-data	d:se_code:SD_A_GLLDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN1,d:NOISE_ZERO

	// メカゲノラ
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTL \
		-data	d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTR \
		-data	d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO
	// 歩き衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLGACHA \
		-data	d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO
	// 上体衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLKINUZ \
		-data	d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO
	// 咆え -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLHOWL1 \
		-data	d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO
	// 唸り -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLBRESS \
		-data	d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO
	// ダウン -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_A_GLLDOWN1 \
		-data	d:se_code:SD_A_MGNDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_MGNDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_MGNDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_MGNDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_MGNDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_MGNDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_MGNDOWN1,d:NOISE_ZERO \
				d:se_code:SD_A_MGNDOWN1,d:NOISE_ZERO

	// ゲノラ
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTL \
		-data	d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTR \
		-data	d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO
	// 歩き衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLGACHA \
		-data	d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO
	// 上体衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLKINUZ \
		-data	d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO
	// 咆え -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLHOWL1 \
		-data	d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO
	// 唸り -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLBRESS \
		-data	d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO
	// ダウン -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_A_GLLDOWN1 \
		-data	d:se_code:SD_A_GLLDOWN12,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN12,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN12,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN12,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN12,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN12,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN12,d:NOISE_ZERO \
				d:se_code:SD_A_GLLDOWN12,d:NOISE_ZERO

}
#endif

#ifdef d:STAGE_SP05A
/////////////////////////////////
// 狙撃１
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif
}
#endif

#ifdef d:STAGE_SP06A
/////////////////////////////////
// 狙撃２
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif

#ifdef d:STAGE_SP07A
/////////////////////////////////
// ゴルルゴン
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

	// ゴルルゴン
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTL \
		-data	d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTR \
		-data	d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR,d:NOISE_ZERO
	// 歩き衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLGACHA \
		-data	d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA,d:NOISE_ZERO
	// 上体衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLKINUZ \
		-data	d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ,d:NOISE_ZERO
	// 咆え -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLHOWL1 \
		-data	d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL1,d:NOISE_ZERO
	// 唸り -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_0 -type d:SE_FLOOR -se d:se_code:SD_E_GLLBRESS \
		-data	d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS,d:NOISE_ZERO

	// メカゲノラ
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTL \
		-data	d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTL,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTR \
		-data	d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO \
				d:se_code:SD_E_MGNFOOTR,d:NOISE_ZERO
	// 歩き衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLGACHA \
		-data	d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO \
				d:se_code:SD_E_MGNGACHA,d:NOISE_ZERO
	// 上体衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLKINUZ \
		-data	d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO \
				d:se_code:SD_E_MGNKINUZ,d:NOISE_ZERO
	// 咆え -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLHOWL1 \
		-data	d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO \
				d:se_code:SD_E_MGNHOWL1,d:NOISE_ZERO
	// 唸り -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_1 -type d:SE_FLOOR -se d:se_code:SD_E_GLLBRESS \
		-data	d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO \
				d:se_code:SD_E_MGNBRESS,d:NOISE_ZERO

	// ゲノラ
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTL \
		-data	d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTL2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLFOOTR \
		-data	d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLFOOTR2,d:NOISE_ZERO
	// 歩き衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLGACHA \
		-data	d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLGACHA2,d:NOISE_ZERO
	// 上体衣擦れ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLKINUZ \
		-data	d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLKINUZ2,d:NOISE_ZERO
	// 咆え -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLHOWL1 \
		-data	d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO \
				d:se_code:SD_E_GLLHOWL12,d:NOISE_ZERO
	// 唸り -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_GLL_2 -type d:SE_FLOOR -se d:se_code:SD_E_GLLBRESS \
		-data	d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO \
				d:se_code:SD_E_GLLBRESS2,d:NOISE_ZERO

	command セット音量パラメータ \
		d:VOLPARAM_GM_SEMODE_BOMB d:SE_VOLPARAM_NORMAL_VIEW \
		75, 3000, 30000, 0x3F, 0x30, 0x0F, \
		90, 2500, 29400, 0x3F, 0x30, 0x0F, \
		100

	command セット音量パラメータ \
		d:VOLPARAM_GM_SEMODE_BOMB d:SE_VOLPARAM_OWN_VIEW \
		60, 0, 30000, 0x3F, 0x30, 0x0F, \
		90, 0, 29400, 0x3F, 0x30, 0x0F, \
		100

	command セット音量パラメータ \
		d:VOLPARAM_GM_SEMODE_BOMB d:SE_VOLPARAM_DEMO_VIEW \
		75, 3000, 30000, 0x3F, 0x30, 0x0F, \
		90, 2500, 29400, 0x3F, 0x30, 0x0F, \
		100

}
#endif

#ifdef d:STAGE_SP08A
/////////////////////////////////
// ダークステージ
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO
}
#endif

#ifdef d:STAGE_ST01A || d:STAGE_ST02A || d:STAGE_ST03A || d:STAGE_ST04A || d:STAGE_ST05A
/////////////////////////////////
// ストリーキング
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO


	// ハダカプレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:se_code:SD_A_FOOT_S1L \
		-data	d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_07L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// ハダカプレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:se_code:SD_A_FOOT_S1R \
		-data	d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO


// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO
}
#endif

#ifdef d:STAGE_SP10A
/////////////////////////////////
// じゃんけん
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif
}
#endif


#ifdef d:STAGE_SP21A || d:STAGE_SP22A || d:STAGE_SP23A || d:STAGE_SP24A
/////////////////////////////////
// 主観モードステージ
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif

#ifdef d:STAGE_SP25A
/////////////////////////////////
// 主観モードステージ
/////////////////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//０通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//１鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//３鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//４鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//０通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//１鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//３鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//４鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//５金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//０通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//１鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//３鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//４鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_SS \	//６水床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_SS \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
	// プレイヤー歩き左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL02 \
		-data	d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07L,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08L,d:NOISE_ZERO
	// プレイヤー歩き右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR02 \
		-data	d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOTW08R,d:NOISE_ZERO

// 敵兵関係の設定(４人分用意する)
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L1,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_0 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

	// ２番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L2,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_1 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R2,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R2,d:NOISE_ZERO

	// ３番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L3,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_2 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R3,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R3,d:NOISE_ZERO

	// ４番目に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7L4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8L4,d:NOISE_ZERO
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ENE_3 -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R4,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R4,d:NOISE_ZERO

}
#endif


#else

	#if d:DEBUG_PRINT
	print 'a_se_set.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'
	#endif

#endif

