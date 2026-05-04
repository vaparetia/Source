/*
	speak.h
	敵兵台詞ヘッダ

	2000/04/24 Y.Korekado
	$Id: speak.h,v 1.1.1.3 2002/11/19 11:44:10 Yoshizawa1 Exp $
	
*/
#ifndef __ENESPEAK___
#define	__ENESPEAK___ 1

#define	EV_MAN_NUM	4	/* 同じ台詞の種類 */

#define	EV_NOTICE_FOOT		SD_V_MAKI20		/* 足跡発見「ん！」 */
#define	EV_WHAT_FOOT		SD_V_MAKI21		/* 足跡調査後「うーん？」 */
 #define	EV_NOTICE_BOX		SD_V_MAKI20		/* ダンボール発見「ん！」 */
 #define	EV_WHAT_BOX			SD_V_MAKI21		/* ダンボール発見後「うーん？」 */
#define	EV_NOTICE_NOISE		SD_V_MAKI20		/* 物音聞いた「ん！」 */
#define	EV_WHAT_NOISE		SD_V_MAKI21		/* 物音聞いた後「うーん？」 */
#define	EV_NOTICE_FIND_LV2	SD_V_MAKI20		/* 不審物発見Lv2「ん！」 */
#define	EV_WHAT_FIND_LV2	SD_V_MAKI21		/* 不審物発見Lv2後「うーん？」 */
#define	EV_NOTICE_OBORO		SD_V_MAKI20		/* かすかに見えた「ん！」 */
#define	EV_WHAT_OBORO		SD_V_MAKI21		/* かすかに見えた後「うーん？」 */



#define	EV_NOTICE_PLAYER	SD_V_MAKI24		/* プレイヤー発見「あっ！」 */

#define	EV_NOTICE_ENEDAM	SD_V_MAKI27		/* 味方ダメージ「ん！」 */
// #define	EV_WHERE_ENEDAM		SD_V_MAKI28		/* 味方ダメージ後「どこだ？」 */
#define	EV_NOTICE_CORP		SD_V_MAKI27		/* 死体発見「はっ！」 */
 #define	EV_NOTICE_BLOOD		SD_V_MAKI27		/* 血跡発見「ん！」 */
// #define	EV_WHAT_BLOOD		SD_V_MAKI31		/* 血跡発見後「うーん？」 */
 #define	EV_NOTICE_FIND_LV3	SD_V_MAKI27		/* 不審物発見Lv3「ん！」 */

#define	EV_NOTICE_HOLDUP	SD_V_MAKI35		/* ホールドアップ「ひっ！」 */
#define	EV_NOTICE_ADULT		SD_V_MAKI36		/* エロ本発見「ん！」 */
#define	EV_NOTICE_HELP		SD_V_GBS_A016		/* ホールドアップ「助けてー！」 */
#define	EV_NOTICE_NORADIO	SD_V_MAKI28		/* 無線の代わり「くそ！」 */

#if 1
#define	EV_DETECT_1		(SD_V_MAKI20)	/* 誰だ */
#define	EV_DETECT_RAD3	(SD_V_ATKO05)	/* くそっ！！ */
#define	EV_DETECT_2		(SD_V_MAKI24)	/* はっ！ */
#define	EV_DETECT_3		(SD_V_MAKI22)	/* んっ？ */
#define	EV_NP_1			(SD_V_MAKI23)	/* 異常なしか */
#define	EV_FOOT_1		(SD_V_MAKI20)	/* ん、足跡 */
#define	EV_BOX_1		(SD_V_MAKI25)	/* なんだこの箱は？ */
#define	EV_BOX_2		(SD_V_MAKI20)	/* 邪魔だ！ */
#define	EV_BLOOD_1		(SD_V_MAKI30)	/* ん！血痕？ */
#define	EV_OBJ_1		(SD_V_MAKI32)	/* ん、何だ */
#define	EV_HOLD_1		(SD_V_MAKI20)	/* ひー */

/* for attacker */
#define	EV_GRENADE_1	(SD_V_C01MAKI)	/* 手榴弾 */
	#define	EV_CAMON_1		(SD_V_MAKI20)	/* こっちだ */
	#define	EV_BACK_1		(SD_V_MAKI20)	/* さがれ */
#define	EV_EAT_1		(SD_V_C04MAKI)	/* くらえ */
#define	EV_AID_1		(SD_V_MAKI20)	/* 応援を呼べ */
#define	EV_GO_1			(SD_V_MAKI37)		/* ごー */
#define	EV_STOP_1		(SD_V_MAKI39)		/* 停まれ */
#define	EV_CLEAR_1		(SD_V_C08MAKI)	/* クリア */

#else
#define	EV_DETECT_1		SD_V_201MAKI	/* 誰だ */
#define	EV_DETECT_RAD3	SD_V_204MAKI	/* くそっ！！ */
#define	EV_DETECT_2		SD_V_208MAKI	/* はっ！ */
#define	EV_DETECT_3		SD_V_301MAKI	/* んっ？ */
#define	EV_NP_1			SD_V_302MAKI	/* 異常なしか */
#define	EV_FOOT_1		SD_V_601MAKI	/* ん、足跡 */
#define	EV_BOX_1		SD_V_701MAKI	/* なんだこの箱は？ */
#define	EV_BOX_2		SD_V_703MAKI	/* 邪魔だ！ */
#define	EV_BLOOD_1		SD_V_901MAKI	/* ん！血痕？ */
#define	EV_OBJ_1		SD_V_A01MAKI	/* ん、何だ */
#define	EV_HOLD_1		SD_V_B01MAKI	/* ひー */

/* for attacker */
#define	EV_GRENADE_1	SD_V_C01MAKI	/* 手榴弾 */
#define	EV_CAMON_1		SD_V_C02MAKI	/* こっちだ */
#define	EV_BACK_1		SD_V_C03MAKI	/* さがれ */
#define	EV_EAT_1		SD_V_C04MAKI	/* くらえ */
#define	EV_AID_1		SD_V_C05MAKI	/* 応援を呼べ */
#define	EV_GO_1			SD_V_MAKI37		/* ごー */
#define	EV_STOP_1		SD_V_MAKI39		/* 停まれ */
#define	EV_CLEAR_1		SD_V_C08MAKI	/* クリア */
#endif

/* 各モードタイミング別の台詞 */

enum {
	EV_MODE_NONE,
	EV_MODE_DAMAGE_RAD,	/* ダメージモード無線連絡 */
	EV_MODE_HOLD_RAD,	/* ホールドアップモード無線連絡 */
	EV_MODE_ENEDAM_RAD,	/* 味方ダメージモード無線連絡 */
} ;


//#define LV2_STRM_LENGTH	COUNT_VMODE(127) //180
//#define LV1_STRM_LENGTH	COUNT_VMODE(240)
#define LV2_STRM_LENGTH	COUNT_VMODE(240) //180
#define LV1_STRM_LENGTH	COUNT_VMODE(240)
#define CAUTION_STRM_LENGTH	COUNT_VMODE(240)

#define MAX_VOICE_NUM	(256*2)

/* ストリーミングレベル */
enum {
	STR_LEVEL_0,
	STR_LEVEL_1,
	STR_LEVEL_2,
} ;

#define CHARACTER_VOICE_NUM	(0x37+1)	/* 4種類ある敵兵の台詞数 */
#define OTHER_VOICE_HEAD	CHARACTER_VOICE_NUM*4*2	/* それ以外の台詞の頭番号 */

/* 音声フラグ */
#define	EV_RAD_LEVEL_1				0x10000000	// Lv1
#define	EV_RAD_LEVEL_2				0x20000000	// Lv2
#define	STRM_NO_ATTACKER			0x00000100	// 調査にいけない 
#define	STRM_CLEARING				0x00001000	// クリアリング音声 
#define	STRM_NO_COMM_USE			0x00002000	// コマンダー無しステージで流れるノイズ
#define	STRM_COMMANDER				0x00004000	// 指揮官
#define	STRM_CANCEL_THROUGH			0x00008000	// キャンセルスルー 
#define	STRM_NO_CHARA				0x00010000	// 個性無し台詞 
#define	STRM_ROUTE					0x00020000	// ルート音声 
#define	STRM_ONSELF					0x00040000	// 独り言 
#define	STRM_GHQ_CONFIRM			0x00100000	// 確認要請GHQConfirmVoiceの司令官の台詞。 
#define	STRM_NO_CANCEL				0x00200000	// 呼んだキャラが死んでもキャンセルされない。 
#define	STRM_GHQ_MECA_CONFIRM		0x00400000	// メカ系確認要請の司令官の台詞。 
#define	STRM_RAD_NOISE				0x00800000	// 無線終了ノイズ 
#define	STRM_GHQ_ALERT				0x01000000	// 危険時GHQAreaVoiceの司令官の台詞。 
#define	STRM_GHQ_CAUTION			0x02000000	// 警戒時GHQCautionVoiceの司令官の台詞。 
#define	STRM_FADER_ON				0x04000000	// 発生場所からパンとヴォリュームを計算。 
#define	STRM_TIME_TABLE				0x08000000	// 再生時間はテーブル。 

/* 音声 */

/* 攻撃兵 */
#define EV_RAD_ATK_RETURN		0x00014000	//Lv0 「配置に戻れ、警備を強化しろ」

/* 司令室 */
#define EV_RAD_GHQ_REINFORCE1	0x10816001	//Lv1 「了解、応援を回す」
#define EV_RAD_GHQ_REINFORCE2	0x10816002	//Lv1 「了解、増援を送る」
#define EV_RAD_GHQ_REINFORCE3	0x10816003	//Lv1 「わかった。至急応援部隊を派遣する」
#define EV_RAD_GHQ_ORDER1		0x10816004	//Lv1 「Aチーム現場へ向え、Bチーム各・・・」
#define EV_RAD_GHQ_ORDER2		0x00816005	//Lv1 「了解、エンジンルーム、2名増員、1名ずつ・・・」

#define EV_RAD_GHQ_QUESTION1	0x10814006	//Lv1 「定時連絡はどうした？」
#define EV_RAD_GHQ_RETURN_SNEAK	0x10814007	//Lv1 「攻撃チーム配置に戻れ、・・・」
#define EV_RAD_GHQ_QUESTION3	0x10814008	//Lv1 「状況を報告せよ」
#define EV_RAD_GHQ_CONTINUE		0x10814009	//Lv1 「引き続き警戒せよ」
#define EV_RAD_GHQ_QUESTION5	0x1081400a	//Lv1 「どうした、応答しろ！」
#define EV_RAD_GHQ_QUESTION6	0x1081400b	//Lv1 「どうしたんだ」
#define EV_RAD_GHQ_QUESTION7	0x1081400c	//Lv1 「何があった」
#define EV_RAD_GHQ_SLEEPCALL	0x1081400d	//Lv1 「返事をしろ、寝ているのか！？起きろ！！」
#define EV_RAD_GHQ_INVETIGATED	0x1081400e	//Lv1 「至急調査に向え」
#define EV_RAD_GHQ_NO_BRINGIN	0x1081400f	//Lv1 「増援は出せない。」

#define EV_RAD_GHQ_CONFIRM		0x00914000	//Lv0 「確認要請」
#define EV_RAD_GHQ_CONFIRM2		0x00914100	//Lv0 「確認要請却下」
#define EV_RAD_GHQ_MECA_CONFIRM	0x00c14000	//Lv0 「メカ系確認要請」


#define EV_RAD_ROUTE			0x04020000	//Lv0 「ルート音声無線」
#define EV_TALK_ONESELF			0x04060000	//Lv0 「ルート音声独り言」

#define EV_CLEARING_ONESELF		0x04041000	//Lv0 「クリアリング音声独り言」

/* 警備兵 */
#define EV_RAD_FIND_PL_FIRST	0x2c000000	//Lv2 「初めてプレイヤーを発見」
#define EV_RAD_FIND_PL			0x2c000002	//Lv2 「プレイヤーを発見」
#define EV_RAD_FIND_PL_AT		0x20008003	//Lv2 「攻撃兵がプレイヤーを発見」
#define EV_RAD_FIND_PL_UNEXP	0x2c000004	//Lv2 「不意にプレイヤーを発見」

#define EV_RAD_FIND_RAD			0x18a00005	//Lv1 「無線エフェクトつき」
#define EV_RAD_DAMAGE			0x1c000006	//Lv1 「ダメージを受けた」
#define EV_RAD_HOLDUP			0x1c000007	//Lv1 「ホールドアップさせられた」
 #define EV_RAD_DAMAGE_GUN		0x1c000008	//Lv1 「銃でダメージを受けた」
#define EV_RAD_CORP				0x1c000009	//Lv1 「仲間の死体を発見」
#define EV_RAD_CORP_AT			0x1c00000a	//Lv1 「仲間の死体を攻撃兵が発見」
#define EV_RAD_ENE_DAMAGE		0x1c00000b	//Lv1 「仲間が攻撃を受けた」
#define EV_RAD_FIND				0x1c00000d	//Lv1 「侵入の痕跡を発見」
#define EV_RAD_ONEMORE_PL		0x0c00000e	//Lv1 「警備の補充を頼む」
#define EV_RAD_ROGER			0x0000000f	//Lv0 「了解」
#define EV_RAD_NO_ACCIDENT		0x0c000010	//Lv0 「異常は無かった」
#define EV_RAD_END_AVOID		0x0c000011	//Lv0 「配置に戻れ、警戒を・・・」
#define EV_RAD_MUDAASI			0x0c000012	//Lv0 「異常無し」

#define EV_NOISE_SNEAK			0x0c040013	//Lv0 「物音モード：潜入モード中」
#define EV_NOISE_AVOID			0x0c040014	//Lv0 「物音モード：回避モード中」
#define EV_FOOT_SNEAK			0x0c040015	//Lv0 「物音モード(足音)：潜入モード中」
#define EV_FOOT_AVOID			0x0c040016	//Lv0 「物音モード(足音)：回避モード中」
#define EV_BREAK_SNEAK			0x0c040017	//Lv0 「壊れ物発見」
#define EV_BOOK_SNEAK			0x0c040018  //Lv0 「エロ本発見」
#define EV_FOOTPRI_SNEAK		0x0c040019	//Lv0 「足跡発見：潜入モード中」
#define EV_FOOTPRI_AVOID		0x0c04001a	//Lv0 「足跡発見：回避モード中」
#define EV_BLOODPRI_SNEAK		0x0c04001b	//Lv0 「血跡発見：潜入モード中」
#define EV_BLOODPRI_AVOID		0x0c04001c	//Lv0 「血跡発見：回避モード中」
#define EV_OBORO_SNEAK			0x0c04001d	//Lv0 「朧モード：潜入モード中」
#define EV_OBORO_AVOID			0x0c04001e	//Lv0 「朧モード：回避モード中」
#define EV_DOWNEME_SNEAK		0x0c04001f	//Lv0 「倒れている仲間発見：潜入モード中」
#define EV_DOWNEME_AVOID		0x0c040020	//Lv0 「倒れている仲間発見：回避モード中」
#define EV_DAMENE_SNEAK			0x0c040021	//Lv0 「ダメージを受けた仲間発見：潜入モード中」
#define EV_DAMENE_AVOID			0x0c040022	//Lv0 「ダメージを受けた仲間発見：回避モード中」

#define EV_BOX_SNEAK			0x0c040023	//Lv0 「段ボール発見：潜入モード中」
#define EV_BOX_AVOID			0x0c040024	//Lv0 「段ボール発見：回避モード中」
#define EV_BOX_SNEAK2			0x0c040025	//Lv0 「段ボール発見２：潜入モード中」
#define EV_BOX_AVOID2			0x0c040026	//Lv0 「段ボール発見２：回避モード中」
#define EV_UNIFORM1_SNEAK		0x0c040027	//Lv0 「変装１：潜入モード中」
#define EV_UNIFORM1_AVOID		0x0c040028	//Lv0 「変装１：回避モード中」
#define EV_UNIFORM2_SNEAK		0x0c040029	//Lv0 「変装２：潜入モード中」
#define EV_UNIFORM2_AVOID		0x0c04002a	//Lv0 「変装２：回避モード中」
#define EV_UNIFORM3_SNEAK		0x0c04002b	//Lv0 「変装３（違う武器）：潜入モード中」
#define EV_UNIFORM3_AVOID		0x0c04002c	//Lv0 「変装３（違う武器）：回避モード中」
#define EV_UNIFORM4_SNEAK		0x0c04002d	//Lv0 「変装4（出血）：潜入モード中」
#define EV_UNIFORM4_AVOID		0x0c04002e	//Lv0 「変装4（出血）：回避モード中」
#define EV_UNIFORM5_SNEAK		0x0c04002f	//Lv0 「変装5（行動）：潜入モード中」
#define EV_UNIFORM5_AVOID		0x0c040030	//Lv0 「変装5（行動）：回避モード中」
#define EV_SLEEP_RAD_MUDAASI	0x0c000031	//Lv0 「異常無し（寝言）」
#define EV_ACCIDENT_DIRECT		0x1c000032	//Lv1 「敵がどこかにいる。探し出せ（様子がおかしい」

#define EV_HOLDUP_BLUFF			0x0c040033	//Lv0 「強気ホールドアップ」
#define EV_HOLDUP_BLUFF2		0x0c040034	//Lv0 「強気ホールドアップ2」
#define EV_HOLDUP_BLUFF3		0x0c040035//Lv0 「強気ホールドアップ3」

#define EV_UNIFORM6_SNEAK		0x0c040036	//Lv0 「変装３（違うステージ）：潜入モード中」
#define EV_UNIFORM6_AVOID		0x0c040037	//Lv0 「変装３（違うステージ）：回避モード中」

#define VOICE_SET_NUM	CHARACTER_VOICE_NUM	//(56)

#define EV_RAD_FIND_RAD_LV2		0x28a00005	//Lv1 「無線エフェクトつき」



#define	EV_SEARCH_1		0x10000001	//LV1 「やられた！気を付けろ！（無線）」
#define	EV_SEARCH_2		0x10000002	//Lv1 「敵がこのエリアにいるぞ、気を付けろ（無線）」
#define	EV_SEARCH_3		0x10000003	//Lv1 「攻撃を受けた。敵の位置は不明。警戒を怠るな！（無線）」

#define	EV_ALERT_0		0x20000004	//Lv2 「侵入者がいる！応援を頼む（無線）」
#define	EV_ALERT_1		0x20000005	//Lv2 「侵入者を発見！バックアップを送ってくれ（無線）」
#define	EV_ALERT_2		0x20000006	//Lv2 「敵はこっちだ！来てくれー！（無線）」
#define	EV_ALERT_3		0x20000007	//Lv2 「敵を発見した！応援を頼む（無線）」
#define	EV_ALERT_4		0x20000008	//Lv2 「敵はエリア１！急げ！（無線）」
#define	EV_ALERT_5		0x20000009	//Lv2 「敵はエリア２！急げ！（無線）」
#define	EV_ALERT_6		0x2000000a	//Lv2 「敵はエリア３！急げ！（無線）」
#define	EV_ALERT_7		0x2000000b	//Lv2 「いたぞー（無線）」

#define	EV_SEARCH_4		0x1000000c	//LV1 「一人やられた、気を付けろ！（無線）」
#define	EV_SEARCH_5		0x1000000d	//LV1 「また一人やられた、警戒を怠るな！（無線）」
#define	EV_SEARCH_6		0x1000000e	//LV1 「攻撃を受けた。敵の位置は不明。警備を強化しろ！（無線）」
#define	EV_SEARCH_7		0x1000000f	//LV1 「配置に戻れ！警備を強化しろ！（無線）」
#define	EV_SEARCH_8		0x10000010	//LV1 「一人やられた、気を付けろ！（無線）」
#define	EV_SEARCH_9		0x10000011	//LV1 「また一人やられた、気を付けろ！（無線）」
#define	EV_SEARCH_10	0x10000012	//LV1 「敵が近くにいる形跡がある、警備を強化しろ（無線）」
#define	EV_SEARCH_11	0x10000013	//LV1 「敵が潜入した痕跡を発見した。警戒を怠るな（無線）」
#define	EV_SEARCH_12	0x10000014	//LV1 「敵はこのエリアに侵入した。警戒を怠るな（無線）」
#define	EV_SEARCH_13	0x10000015	//LV1 「敵がこのエリアにいるぞ、気を付けろ（無線）」

#endif


