/*
	se_set.h
	    壁床音設定など音声にかかわる設定

	2001/06/11 H.Yoshiike
	$Id: sd_set.h,v 1.30 2002/05/09 13:07:48 usr01475 Exp $
*/

// ファイルが二重呼びされたときの対処
#ifndef d:SE_SET_H
#define SE_SET_H	1

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

#define MT_SETABLE_NPC_EMMA	(32)	// NPCエマ
#define MT_SETABLE_NPC_SNA	(33)	// NPCスネーク

#define MT_SETABLE_ORGA		(34)	// オルガ
#define MT_SETABLE_FORT		(35)	// フォーチュン
#define MT_SETABLE_FAT		(36)	// ファットマン
#define MT_SETABLE_VAMP		(37)	// バンプ
#define MT_SETABLE_SOL		(38)	// ソリダス
#define MT_SETABLE_OCELOT	(39)	// オセロット
*/


#if 0
音量カーブの設定方法

/*音のモード*/
enum {
	VOLPARAM_SEMODE_NORMAL,	/* 俯瞰時はカメラターゲット、主観時はカメラ位置を中心に計算 */
	VOLPARAM_GM_SEMODE_BOMB,		/* 遠くても必ず少しは聞こえる */
	VOLPARAM_GM_SEMODE_REAL,		/* 主観やビハインドになったときだけ聞こえる */
} ;
/*画面の種類*/
enum {
	SE_VOLPARAM_NORMAL_VIEW,	/*通常画面*/
	SE_VOLPARAM_OWN_VIEW,		/*主観モード画面*/
	SE_VOLPARAM_DEMO_VIEW,		/*デモ画面*/
} ;

command セット音量パラメータ[GM_SetVolumeParam] \
	$i:モード \
	$i:画面種類 \
	$w:有効角度 \			/* 0〜4095 */
	$i:内側計算最小距離 \
	$i:内側計算最大距離 \
	$i:内側最小距離時音量 \	/* 0〜63 */
	$i:内側中心距離時音量 \	/* 0〜63 */
	$i:内側最大距離時音量 \	/* 0〜63 */
	$w:減衰最大角 \			/* 0〜4095 */
	$i:外側計算最小距離 \
	$i:外側計算最大距離 \
	$i:外側最小距離時音量 \	/* 0〜63 */
	$i:外側中心距離時音量 \	/* 0〜63 */
	$i:外側最大距離時音量 \	/* 0〜63 */
	$i:パン係数				/* 100標準 */


＊＊＊デフォルト値をセットするには＊＊＊

ノーマル系、俯瞰時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_SEMODE_NORMAL d:SE_VOLPARAM_NORMAL_VIEW \
	75, 6000, 12000, 0x3F, 0x18, 0x00, \
	90,	5000, 10000, 0x3f, 0x18, 0x00, \
	100

ノーマル系、主観時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_SEMODE_NORMAL d:SE_VOLPARAM_OWN_VIEW \
	60, 0, 10000, 0x3F, 0x3F, 0x00, \
	90, 0, 9800, 0x3f, 0x3F, 0x00, \
	100

ノーマル系、デモ時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_SEMODE_NORMAL d:SE_VOLPARAM_DEMO_VIEW \
	75, 6000, 12000, 0x3F, 0x18, 0x00, \
	90,	5000, 10000, 0x3f, 0x18, 0x00, \
	100


ボム系、俯瞰時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_GM_SEMODE_BOMB d:SE_VOLPARAM_NORMAL_VIEW \
	75, 6000, 14000, 0x3F, 0x38, 0x1f, \
	90, 5000, 12000, 0x3F, 0x38, 0x1f, \
	100

ボム系、主観時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_GM_SEMODE_BOMB d:SE_VOLPARAM_OWN_VIEW \
	60, 0, 10000, 0x3F, 0x3F, 0x1f, \
	90, 0, 9800, 0x3f, 0x3F, 0x1f, \
	100

ボム系、デモ時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_GM_SEMODE_BOMB d:SE_VOLPARAM_DEMO_VIEW \
	75, 6000, 14000, 0x3F, 0x38, 0x1f, \
	90, 5000, 12000, 0x3F, 0x38, 0x1f, \
	100


リアル系、俯瞰時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_GM_SEMODE_REAL d:SE_VOLPARAM_NORMAL_VIEW \
	75, 6000, 12000, 0x3F, 0x18, 0x00, \
	90, 5000, 10000, 0x3f, 0x18, 0x00, \
	100

リアル系、主観時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_GM_SEMODE_REAL d:SE_VOLPARAM_OWN_VIEW \
	60, 0, 10000, 0x2F, 0x2F, 0x00, \
	90, 0, 9800, 0x2f, 0x2F, 0x00, \
	100

リアル系、デモ時のパラメータ
command セット音量パラメータ \
	d:VOLPARAM_GM_SEMODE_REAL d:SE_VOLPARAM_DEMO_VIEW \
	75, 6000, 12000, 0x2F, 0xf, 0x00, \
	90, 5000, 10000, 0x2f, 0xf, 0x00, \
	100

#endif


#include "a_sd_set.h"		// アナザー・ＶＲミッションの効果音設定ファイル


/////////////////////////////////////////////////////////
// タンカー編
/////////////////////////////////////////////////////////

#ifdef d:STAGE_W00A
//////////////////////
// 甲板
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄筋
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４通常
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄筋
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６鉄板
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \	//２鉄筋
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \	//４通常
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \	//５鉄筋
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \	//６鉄板
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床（コンクリ）
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４水床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５水鉄階段
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６水鉄階段
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W00B
//////////////////////
// オルガ戦
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４通常
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄筋
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６鉄板
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７木箱
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \	//４通常
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \	//５鉄筋
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \	//６鉄板
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \	//７木箱
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO	//８ホロ

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４水床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５水鉄階段
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６水鉄階段
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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


// オルガ足音の音設定
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_ORGA -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
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
		-id d:MT_SETABLE_ORGA -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO
}
#endif
#ifdef d:STAGE_W00C
//////////////////////
// オルガ戦後
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４通常
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄筋
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６鉄板
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７木箱
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \	//４通常
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \	//５鉄筋
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \	//６鉄板
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \	//７木箱
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４水床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５水鉄階段
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６水鉄階段
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//７水木箱
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W01A
//////////////////////
// 居住区１Ｆ
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄筋
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４通常
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄筋
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４通常
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２ロッカー扉
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３ロッカー扉
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４水床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W01B
//////////////////////
// 居住区２Ｆ
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄筋
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６警報ベル
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄筋
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_MM \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６警報ベル
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_MM \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカー扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W01C
//////////////////////
// 居住区３Ｆ
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄筋
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６警報ベル
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄筋
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_MM \	//
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６警報ベル
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_MM \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカー扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W01D
//////////////////////
// 居住区４Ｆ
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄筋
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４通常
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７木箱
				d:se_code:SD_A_WALLM08,d:NOISE_S	//８段ボール

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄筋
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_MM \	//４通常
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７木箱
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//８段ボール

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \
				d:se_code:SD_A_RICOCH04,d:NOISE_MM \
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \
				d:se_code:SD_A_RICOCH08,d:NOISE_SS

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４階段
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W01E
//////////////////////
// 艦橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄筋
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４通常
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄筋
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４通常
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄床
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３水床
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４階段
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W01F
//////////////////////
// 居住区１Ｆリフレッシュルーム
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄筋
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４通常
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄筋
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４通常
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４階段
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO	//８ガラス
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO
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
				d:se_code:SD_P_FOOTW01L,d:NOISE_ZERO
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
				d:se_code:SD_P_FOOTW01R,d:NOISE_ZERO


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
				d:se_code:SD_E_EFOOT_1L1,d:NOISE_ZERO
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
				d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO

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
				d:se_code:SD_E_EFOOT_1L2,d:NOISE_ZERO
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
				d:se_code:SD_E_EFOOT_1R2,d:NOISE_ZERO

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
				d:se_code:SD_E_EFOOT_1L3,d:NOISE_ZERO
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
				d:se_code:SD_E_EFOOT_1R3,d:NOISE_ZERO

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
				d:se_code:SD_E_EFOOT_1L4,d:NOISE_ZERO
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
				d:se_code:SD_E_EFOOT_1R4,d:NOISE_ZERO
}
#endif
#ifdef d:STAGE_W02A
//////////////////////
// 機関室
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄筋
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \	//２鉄筋
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカー扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W03A
//////////////////////
// 左舷長廊下
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9003000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W03B
//////////////////////
// 右舷長廊下
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9003000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７木箱
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７木箱
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//７木箱
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W04A
//////////////////////
// 第一船倉
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002400

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
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
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W04B
//////////////////////
// 第二船倉
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002400

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
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
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W04C
//////////////////////
// 第三船倉
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002400

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３鉄階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
// プラント編
/////////////////////////////////////////////////////////
#ifdef d:STAGE_W11A || d:STAGE_W11B
//////////////////////
// 海底ドック
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６木箱
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６木箱
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//８水面

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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３水床
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカ扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６木床
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W11C
//////////////////////
// フォーチュン戦（海底ドック）
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６木箱
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６木箱
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３ガソリン
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５瓦礫床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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


// フォーチュンの音設定
	// 最初に登録された敵兵
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_FORT -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
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
		-id d:MT_SETABLE_FORT -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO

}
#endif
#ifdef d:STAGE_W12A || d:STAGE_W12C
//////////////////////
// Ａ脚屋上
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５金網
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_S \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_S \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_S \	//５金網
				d:se_code:SD_A_RICOCH06,d:NOISE_S \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_S \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_S	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W12B
//////////////////////
// Ａ脚ポンプ室
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカー扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W13A || d:STAGE_W13B
//////////////////////
// ＡＢ連絡橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
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
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W14A
//////////////////////
// Ｂ脚変電室
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２通常
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６金網
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２通常
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６金網
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３ロッカー扉
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//８ガラス
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W15A || d:STAGE_W15B
//////////////////////
// ＢＣ連絡橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄床
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W16A || d:STAGE_W16B
//////////////////////
// Ｃ脚食堂
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_S \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_S \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_S \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_S \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_RICOCH08,d:NOISE_S	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄床
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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


// トイレ扉の設定(ロッカー音からの呼び換え)
	// ドア閉まる -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKCLO1 \
		-data	d:SD_A_LOCKCLO1,d:NOISE_ZERO \
				d:se_code:SD_A_TOILCLO1,d:NOISE_ZERO \
				d:SD_A_LOCKCLO1,d:NOISE_ZERO \
				d:SD_A_LOCKCLO1,d:NOISE_ZERO \
				d:SD_A_LOCKCLO1,d:NOISE_ZERO \
				d:SD_A_LOCKCLO1,d:NOISE_ZERO \
				d:SD_A_LOCKCLO1,d:NOISE_ZERO \
				d:SD_A_LOCKCLO1,d:NOISE_ZERO
	// ドア動作 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKMOV1 \
		-data	d:SD_A_LOCKMOV1,d:NOISE_ZERO \
				d:se_code:SD_A_TOILMOV1,d:NOISE_ZERO \
				d:SD_A_LOCKMOV1,d:NOISE_ZERO \
				d:SD_A_LOCKMOV1,d:NOISE_ZERO \
				d:SD_A_LOCKMOV1,d:NOISE_ZERO \
				d:SD_A_LOCKMOV1,d:NOISE_ZERO \
				d:SD_A_LOCKMOV1,d:NOISE_ZERO \
				d:SD_A_LOCKMOV1,d:NOISE_ZERO
	// ドア開く -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKOPN1 \
		-data	d:SD_A_LOCKOPN1,d:NOISE_ZERO \
				d:se_code:SD_A_TOILOPN1,d:NOISE_ZERO \
				d:SD_A_LOCKOPN1,d:NOISE_ZERO \
				d:SD_A_LOCKOPN1,d:NOISE_ZERO \
				d:SD_A_LOCKOPN1,d:NOISE_ZERO \
				d:SD_A_LOCKOPN1,d:NOISE_ZERO \
				d:SD_A_LOCKOPN1,d:NOISE_ZERO \
				d:SD_A_LOCKOPN1,d:NOISE_ZERO
	// ドア主観で頭 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKHIT1 \
		-data	d:SD_A_LOCKHIT1,d:NOISE_ZERO \
				d:se_code:SD_A_TOILHIT1,d:NOISE_ZERO \
				d:SD_A_LOCKHIT1,d:NOISE_ZERO \
				d:SD_A_LOCKHIT1,d:NOISE_ZERO \
				d:SD_A_LOCKHIT1,d:NOISE_ZERO \
				d:SD_A_LOCKHIT1,d:NOISE_ZERO \
				d:SD_A_LOCKHIT1,d:NOISE_ZERO \
				d:SD_A_LOCKHIT1,d:NOISE_ZERO
	// ドア開かない -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKOPN2 \
		-data	d:SD_A_LOCKOPN2,d:NOISE_ZERO \
				d:se_code:SD_A_TOILOPN2,d:NOISE_ZERO \
				d:SD_A_LOCKOPN2,d:NOISE_ZERO \
				d:SD_A_LOCKOPN2,d:NOISE_ZERO \
				d:SD_A_LOCKOPN2,d:NOISE_ZERO \
				d:SD_A_LOCKOPN2,d:NOISE_ZERO \
				d:SD_A_LOCKOPN2,d:NOISE_ZERO \
				d:SD_A_LOCKOPN2,d:NOISE_ZERO
	// ドア外れる -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKDWN1 \
		-data	d:SD_A_LOCKDWN1,d:NOISE_ZERO \
				d:se_code:SD_A_TOILDWN1,d:NOISE_ZERO \
				d:SD_A_LOCKDWN1,d:NOISE_ZERO \
				d:SD_A_LOCKDWN1,d:NOISE_ZERO \
				d:SD_A_LOCKDWN1,d:NOISE_ZERO \
				d:SD_A_LOCKDWN1,d:NOISE_ZERO \
				d:SD_A_LOCKDWN1,d:NOISE_ZERO \
				d:SD_A_LOCKDWN1,d:NOISE_ZERO
	// ドア弾む -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKDWN2 \
		-data	d:SD_A_LOCKDWN2,d:NOISE_ZERO \
				d:se_code:SD_A_TOILDWN2,d:NOISE_ZERO \
				d:SD_A_LOCKDWN2,d:NOISE_ZERO \
				d:SD_A_LOCKDWN2,d:NOISE_ZERO \
				d:SD_A_LOCKDWN2,d:NOISE_ZERO \
				d:SD_A_LOCKDWN2,d:NOISE_ZERO \
				d:SD_A_LOCKDWN2,d:NOISE_ZERO \
				d:SD_A_LOCKDWN2,d:NOISE_ZERO
	// ドア倒れる -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKDWN3 \
		-data	d:SD_A_LOCKDWN3,d:NOISE_ZERO \
				d:se_code:SD_A_TOILDWN3,d:NOISE_ZERO \
				d:SD_A_LOCKDWN3,d:NOISE_ZERO \
				d:SD_A_LOCKDWN3,d:NOISE_ZERO \
				d:SD_A_LOCKDWN3,d:NOISE_ZERO \
				d:SD_A_LOCKDWN3,d:NOISE_ZERO \
				d:SD_A_LOCKDWN3,d:NOISE_ZERO \
				d:SD_A_LOCKDWN3,d:NOISE_ZERO
	// ドア軋みズレ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKDMG1 \
		-data	d:SD_A_LOCKDMG1,d:NOISE_ZERO \
				d:se_code:SD_A_TOILDMG1,d:NOISE_ZERO \
				d:SD_A_LOCKDMG1,d:NOISE_ZERO \
				d:SD_A_LOCKDMG1,d:NOISE_ZERO \
				d:SD_A_LOCKDMG1,d:NOISE_ZERO \
				d:SD_A_LOCKDMG1,d:NOISE_ZERO \
				d:SD_A_LOCKDMG1,d:NOISE_ZERO \
				d:SD_A_LOCKDMG1,d:NOISE_ZERO
	// ドア攻撃当たり -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_A_LOCKATK1 \
		-data	d:SD_A_LOCKATK1,d:NOISE_ZERO \
				d:se_code:SD_A_TOILATK1,d:NOISE_ZERO \
				d:SD_A_LOCKATK1,d:NOISE_ZERO \
				d:SD_A_LOCKATK1,d:NOISE_ZERO \
				d:SD_A_LOCKATK1,d:NOISE_ZERO \
				d:SD_A_LOCKATK1,d:NOISE_ZERO \
				d:SD_A_LOCKATK1,d:NOISE_ZERO \
				d:SD_A_LOCKATK1,d:NOISE_ZERO
}
#endif
#ifdef d:STAGE_W17A
//////////////////////
// ＣＤ連絡橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６壊れ板
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６壊れ板
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_S \	//６壊れ床
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W18A
//////////////////////
// 第一沈殿池
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W19A
//////////////////////
// ＤＥ連絡橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
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
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W20A
//////////////////////
// Ｅ脚集配場
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001c00

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２通常
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６木箱
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２通常
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６木箱
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W20B || d:STAGE_W20C || d:STAGE_W20D
//////////////////////
// Ｅ脚ヘリポート
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000


// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６木箱
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//８ホロ

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６木箱
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
				d:se_code:SD_A_RICOCH08,d:NOISE_SS	//８ホロ

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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常（コンクリ）床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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


// ファットマン足音関連の音設定
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_FAT -type d:SE_FLOOR -se d:se_code:SD_E_ROLLFTL0 \
		-data	d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \	//１通常床（コンクリ）
				d:se_code:SD_E_ROLLFEL3,d:NOISE_ZERO \	//２鉄階段（板状）
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \	//３階段
				d:se_code:SD_E_ROLLFEL2,d:NOISE_ZERO \	//４鉄床（かたまり）
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \	//
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \	//
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \	//
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO	//
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_FAT -type d:SE_FLOOR -se d:se_code:SD_E_ROLLFTR0 \
		-data	d:se_code:SD_E_ROLLFTR0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFER3,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTR0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFER2,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO
	// ブレーキ -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_FAT -type d:SE_FLOOR -se d:se_code:SD_E_ROLLST00 \
		-data	d:se_code:SD_E_ROLLST00,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLSTF1,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLST00,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLSTF0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO \
				d:se_code:SD_E_ROLLFTL0,d:NOISE_ZERO

}
#endif
#ifdef d:STAGE_W21A || d:STAGE_W21B
//////////////////////
// ＥＦ連絡橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \	//
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO	//

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W22A
//////////////////////
// Ｆ脚倉庫
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001c00

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６サッシ
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//８ホロ

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_S \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_S \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_S \	//６サッシ
				d:se_code:SD_A_RICOCH07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_RICOCH08,d:NOISE_S	//８ホロ

	// 壁跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH02,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH03,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH04,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH05,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH06,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH07,d:NOISE_ZERO \
				d:se_code:SD_A_RICOCH08,d:NOISE_ZERO

	// プレイヤー左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカー扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W23A || d:STAGE_W23B
//////////////////////
// ＦＡ連絡橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W24A
//////////////////////
// シェル１中央棟１Ｆ
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカー扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W24B
//////////////////////
// シェル１中央棟Ｂ１
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカー扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W24C || d:STAGE_W24E
//////////////////////
// シェル１中央棟Ｂ１集会場
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６木箱
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６木箱
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６木床
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W24D
//////////////////////
// シェル１中央棟Ｂ２電算室
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001800

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３絨毯
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ロッカー扉
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W25A || d:STAGE_W25B
//////////////////////
// ＤＧ連絡橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６壊れ床
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６壊れ床
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_S \	//６壊れ床
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_S \
				d:se_code:SD_P_FOOT_06R,d:NOISE_S \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W25C
//////////////////////
// Ｌ脚外周
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W25D
//////////////////////
// ＫＬ連絡橋
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５鉄板
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５鉄板
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_S \	//５鳴り鉄床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
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
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W28A
//////////////////////
// Ｌ脚汚水処理場
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W31A || d:STAGE_W31D
//////////////////////
// シェル２中央棟１Ｆ
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001c00

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//７段ボール
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH07,d:NOISE_SS \	//７段ボール
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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

// 特殊設定戻し（W31B対策）
	// プレイヤー水密ドア回し -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_A_MITROL01 \
		-data	d:SD_A_MITROL01,d:NOISE_ZERO \	//１通常水密ドア
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//６水中水密ドア
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO	//
	// プレイヤー水密ドア開け -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_A_MITOPN01 \
		-data	d:SD_A_MITOPN01,d:NOISE_ZERO \	//１通常水密ドア
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//６水中水密ドア
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO	//

}
#endif
#ifdef d:STAGE_W31B || d:STAGE_W31F
//////////////////////
// シェル２中央棟Ｂ１第一濾過層
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_ON
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9001c00

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//５通常
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６水面
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//５通常
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６水面
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//３階段
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５水床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６腰まで水
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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

// 特殊設定
	// プレイヤー水密ドア回し -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_A_MITROL01 \
		-data	d:se_code:SD_A_INWDOOR2,d:NOISE_ZERO \	//１通常水密ドア
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:se_code:SD_A_INWDOOR2,d:NOISE_ZERO \	//６水中水密ドア
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO	//
	// プレイヤー水密ドア開け -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_A_MITOPN01 \
		-data	d:se_code:SD_A_INWDOOR1,d:NOISE_ZERO \	//１通常水密ドア
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:se_code:SD_A_INWDOOR1,d:NOISE_ZERO \	//６水中水密ドア
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO	//


}
#endif
#ifdef d:STAGE_W31C
//////////////////////
// シェル２中央棟Ｂ１第二濾過層
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//３通常
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６水面
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//３通常
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６水面
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５水床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６腰まで水
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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


// ヴァンプ関連の音設定
	// 左足通常 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_VAMP -type d:SE_FLOOR -se d:se_code:SD_E_V_WALKL2 \
		-data	d:se_code:SD_E_V_WALKL2,d:NOISE_ZERO \
				d:se_code:SD_E_V_WALKL2,d:NOISE_ZERO \
				d:se_code:SD_E_V_WALKL2,d:NOISE_ZERO \
				d:se_code:SD_E_V_WALKL2,d:NOISE_ZERO \
				d:se_code:SD_A_WPN_WTR2,d:NOISE_ZERO \
				d:se_code:SD_A_WPN_WTR2,d:NOISE_ZERO \	//腰水
				d:se_code:SD_E_V_WALKL2,d:NOISE_ZERO \
				d:se_code:SD_E_V_WALKL2,d:NOISE_ZERO
	// 左足叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_VAMP -type d:SE_FLOOR -se d:se_code:SD_E_V_FOOTL3 \
		-data	d:se_code:SD_E_V_FOOTL3,d:NOISE_ZERO \
				d:se_code:SD_E_V_FOOTL3,d:NOISE_ZERO \
				d:se_code:SD_E_V_FOOTL3,d:NOISE_ZERO \
				d:se_code:SD_E_V_FOOTL3,d:NOISE_ZERO \
				d:se_code:SD_A_WPN_WTR1,d:NOISE_ZERO \
				d:se_code:SD_A_WPN_WTR1,d:NOISE_ZERO \	//腰水
				d:se_code:SD_E_V_FOOTL3,d:NOISE_ZERO \
				d:se_code:SD_E_V_FOOTL3,d:NOISE_ZERO

// 特殊設定戻し（W31B対策）
	// プレイヤー水密ドア回し -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_A_MITROL01 \
		-data	d:SD_A_MITROL01,d:NOISE_ZERO \	//１通常水密ドア
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO \	//６水中水密ドア
				d:SD_A_MITROL01,d:NOISE_ZERO \	//
				d:SD_A_MITROL01,d:NOISE_ZERO	//
	// プレイヤー水密ドア開け -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_A_MITOPN01 \
		-data	d:SD_A_MITOPN01,d:NOISE_ZERO \	//１通常水密ドア
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//６水中水密ドア
				d:SD_A_MITOPN01,d:NOISE_ZERO \	//
				d:SD_A_MITOPN01,d:NOISE_ZERO	//

command セット音量パラメータ \
	d:VOLPARAM_SEMODE_NORMAL d:SE_VOLPARAM_NORMAL_VIEW \
	75, 6000, 12000, 0x3F, 0x3e, 0x3c, \
	90, 5000, 10000, 0x3f, 0x3e, 0x3c, \
	100

command セット音量パラメータ \
	d:VOLPARAM_SEMODE_NORMAL d:SE_VOLPARAM_OWN_VIEW \
	60, 1000, 20000, 0x3F, 0x3e, 0x3c, \
	90, 1000, 19800, 0x3f, 0x3e, 0x3c, \
	100

}
#endif
#ifdef d:STAGE_W32A || d:STAGE_W32B
//////////////////////
// Ｌ脚下部オイルフェンス
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//６水面
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//６水面
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ブイ
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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


// エマ関連の音設定
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_NPC_EMMA -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５ブイ
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// 右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_NPC_EMMA -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO
}
#endif
#ifdef d:STAGE_W41A
//////////////////////
// アーセナルギア胃(拷問部屋)
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
		-data	d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５裸足通常床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO \	//８裸足鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// ハダカプレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:se_code:SD_A_FOOT_S1R \
		-data	d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W42A
//////////////////////
// アーセナルギア空腸(第一メタルハンガー)
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_S \	//３鳴り鉄床
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_S \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
		-data	d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５裸足通常床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６裸足鉄階段
				d:se_code:SD_P_FOOT_07L,d:NOISE_S \	//７裸足鳴り鉄床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO \	//８裸足鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// ハダカプレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:se_code:SD_A_FOOT_S1R \
		-data	d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_S \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W43A
//////////////////////
// アーセナルギア上行結腸(丸通路)
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_S \	//３鳴り鉄床
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_S \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
		-data	d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//５裸足通常床
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//６裸足鉄階段
				d:se_code:SD_P_FOOT_07L,d:NOISE_S \	//７裸足鳴り鉄床
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO \	//８裸足鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// ハダカプレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:se_code:SD_A_FOOT_S1R \
		-data	d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_S \
				d:se_code:SD_P_FOOT_08R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W44A
//////////////////////
// アーセナルギア回腸(第二メタルハンガー)
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_S \	//３鳴り鉄床
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_S \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W45A
//////////////////////
// アーセナルギアＳ状結腸(円形エレベーター)
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//２鉄板
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//３鉄板
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//４鉄筋
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//２鉄板
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//３鉄板
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//４鉄筋
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//２鉄階段
				d:se_code:SD_P_FOOT_03L,d:NOISE_S \	//３鳴り鉄床
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//４鉄床
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_S \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W46A
//////////////////////
// アーセナルギア直腸(メタル戦)
//////////////////////
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
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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


/*
// ＲＡＹの音設定
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
*/
}
#endif
#ifdef d:STAGE_W51A
//////////////////////
// アーセナルギア上
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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
#ifdef d:STAGE_W61A
//////////////////////
// フェデラルホール
//////////////////////
proc サウンド効果音設定 {
	#if d:DEBUG_PRINT
		print 'se_set'
	#endif

// ステージのリバーブ量設定
	mesg  サウンドマネージャー ＳＤマネ SD_CODE d:SE_REVERB_OFF
	mesg  サウンドマネージャー ＳＤマネ SD_CODE 0xf9002000

// プレイヤー関係の音設定
	// 壁叩き -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_WALL -se d:SD_P_WALL02 \
		-data	d:se_code:SD_A_WALLM01,d:NOISE_S \	//１通常
				d:se_code:SD_A_WALLM02,d:NOISE_S \	//
				d:se_code:SD_A_WALLM03,d:NOISE_S \	//
				d:se_code:SD_A_WALLM04,d:NOISE_S \	//
				d:se_code:SD_A_WALLM05,d:NOISE_S \	//
				d:se_code:SD_A_WALLM06,d:NOISE_S \	//
				d:se_code:SD_A_WALLM07,d:NOISE_S \	//
				d:se_code:SD_A_WALLM08,d:NOISE_S	//

	// 床跳弾音
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_W_RICOCH02 \
		-data	d:se_code:SD_A_RICOCH01,d:NOISE_SS \	//１通常
				d:se_code:SD_A_RICOCH02,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH03,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH04,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH05,d:NOISE_SS \	//
				d:se_code:SD_A_RICOCH06,d:NOISE_SS \	//
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
		-data	d:se_code:SD_P_FOOT_01L,d:NOISE_ZERO \	//１通常床
				d:se_code:SD_P_FOOT_02L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_03L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_04L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_05L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_06L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_07L,d:NOISE_ZERO \	//
				d:se_code:SD_P_FOOT_08L,d:NOISE_ZERO	//
	// プレイヤー右足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_PLAYER -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_P_FOOT_01R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_02R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_03R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_04R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_05R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_06R,d:NOISE_ZERO \
				d:se_code:SD_P_FOOT_07R,d:NOISE_ZERO \
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


// ソリダスの音設定
	// 左足 -dataオプションは必ず８セット書くこと
	command 壁床効果音設定 \
		-id d:MT_SETABLE_SOL -type d:SE_FLOOR -se d:SD_P_FOOTL01 \
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
		-id d:MT_SETABLE_SOL -type d:SE_FLOOR -se d:SD_P_FOOTR01 \
		-data	d:se_code:SD_E_EFOOT_1R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_2R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_3R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_4R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_5R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_6R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_7R1,d:NOISE_ZERO \
				d:se_code:SD_E_EFOOT_8R1,d:NOISE_ZERO
}
#endif



#else

print 'se_set.h double include!!!!!!!!!!!!!!!!!!!!!!!!!!!!!'

#endif
