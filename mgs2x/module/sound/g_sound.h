/*
	g_sound.h
	サウンドコード＆常駐ＳＥ ディファイン

	$Id: g_sound.h,v 1.91 2002/08/09 04:20:13 usr03806 Exp $
*/

#ifndef	___ASSIST_____
#define	___ASSIST_____ 1
/* \\mj003\prog\mgs\source\game にコピーすること */
/*----------------------------------------------------------------*/

//extern	void	GM_SeSet( SVECTOR	*, int ) ;
//extern	void	GM_SeSet2( int, int, int ) ;
//extern	void	GM_SeSet3( SVECTOR *, int, int ) ;
//extern	void	GM_SdSet( int, int ) ;
//extern	void	GM_SeSetPan( SVECTOR *, int, int ) ;
//extern	void	GM_SeSetArea( SVECTOR *, int, int ) ;

//#include "../inc_se/se_stage.h"
//#include "../inc_se/se_defin.h"

/*----------------------------------------------------------------*/
//#define SEP_SAMPLE	0xfe00007f		/*常駐波形パック*/

/*--------------------*/
/*--- Song Control ---*/
/*--------------------*/
/*SONG PAUSE*/
#define SNG_PAUSEON		0x01FFFF01
#define SNG_PAUSEOFF	0x01FFFF02
/*INT PAUSE*/
#define INT_PAUSEON		0x01FFFF24
#define INT_PAUSEOFF	0x01FFFF25
/*SONG FADE IN*/
#define SNG_FIN_S		0x01FFFF03	/* フェードイン(1sec) */
#define SNG_FIN_M		0x01FFFF04	/* フェードイン(3sec) */
#define SNG_FIN_L		0x01FFFF05	/* フェードイン(5sec) */
/*SONG FADE OUT & PAUSE*/
#define SNG_FOUTP_SS	0x01FFFF06	/* 中断フェードアウト(0.5sec) */
#define SNG_FOUTP_S		0x01FFFF07	/* 中断フェードアウト(1sec) */
#define SNG_FOUTP_M		0x01FFFF08	/* 中断フェードアウト(3sec) */
#define SNG_FOUTP_L		0x01FFFF09	/* 中断フェードアウト(5sec) */
/*SONG FADE OUT & STOP*/
#define SNG_FOUTS_SS	0x01FFFF0A	/* 終了フェードアウト(0.5sec) */
#define SNG_FOUTS_S		0x01FFFF0B	/* 終了フェードアウト(1sec) */
#define SNG_FOUTS_M		0x01FFFF0C	/* 終了フェードアウト(3sec) */
#define SNG_FOUTS_L		0x01FFFF0D	/* 終了フェードアウト(5sec) */
/*Kaihi MODE*/
#define SNG_KAIHI_01	0x01FFFF10
#define SNG_KAIHI_RESET	0x01FFFF11
#define SNG_KAIHI_RESET2	0x01FFFF12
/*Syukan MODE*/
#define SNG_SYUKAN_ON	0x01FFFF20
#define SNG_SYUKAN_OFF	0x01FFFF21

/*Mic MODE */
#define SNG_MIC_ON	0x01FFFF22
#define SNG_MIC_OFF	0x01FFFF23

/*SONG STOP*/
#define SNG_STOP		0x01FFFFFF
/*SNG: INTRO SKIP*/
#define SD_INTRO_SKIP	0xFF0000FF
/*SNG: PLAY FX TRACK*/
#define SD_PLAY_FX		0xFF000100
/*SNG: FADER AUTOMATION */
#define SD_AUTO_FADER1	0xFF000101
#define SD_AUTO_FADER2	0xFF000102
#define SD_AUTO_FADER3	0xFF000103
#define SD_AUTO_FADER4	0xFF000104
#define SD_AUTO_FADER5	0xFF000105
#define SD_AUTO_FADER6	0xFF000106
#define SD_AUTO_FADER7	0xFF000107
#define SD_AUTO_FADER8	0xFF000108

/*--------------------*/
/*---- SE CONTROL ----*/
/*--------------------*/
/*EXPANDED SE STOP*/
#define SE_EXP_STOP		0x01FFFFFD	/* 1999/06/03 拡張SE停止 */
/*ALL SE STOP*/
#define SE_ALL_STOP		0x01FFFFFE	/* 1999/06/03 全SE停止 */
#define SE_JOUCHUU_OFF	0xFF0000FE	/* 持続SE停止 */
#define SE_JOUCHUU_OFF2	0xFF0000FD	/* 持続SE停止(0xBE以外の持続効果音を消す) */

/*--------------------*/
/*---- STR CONTROL ---*/
/*--------------------*/
#define STR_STOP		0xFFFFFFFD	/*ストリーミング停止*/
/*STR FADE IN*/
//#define STR_FIN_M		0xFF0000F8
//#define STR_FIN_L		0xFF0000F9
/*STR FADE OUT & PAUSE*/
//#define STR_FOUTP_SS	0xFF0000F4
//#define STR_FOUTP_S		0xFF0000F5
//#define STR_FOUTP_M		0xFF0000F6
//#define STR_FOUTP_L		0xFF0000F7
/*STR FADE OUT & STOP*/
//#define STR_FOUTS_SS	0xFF0000FA
//#define STR_FOUTS_S		0xFF0000FB
//#define STR_FOUTS_M		0xFF0000FC
//#define STR_FOUTS_L		0xFF0000FD

#define STR1_FADE_OUT	0xFF00000D
#define STR2_FADE_OUT	0xFF000011

/*--------------------*/
/*-- REVERB CONTROL --*/
/*--------------------*/
/*SE REVERB ON/OFF*/
#define SE_REVERB_ON	0xFF000007
#define SE_REVERB_OFF	0xFF000008
#define STR_REVERB_ON	0xFF000009

/*SE REVERB DEPTH CHANGE*/
#define SE_REVERB_IN	0xF9003000	/*w04ダクト内に入る*/
#define SE_REVERB_OUT	0xF9002400	/*w04ダクトから出る*/

/*--------------------*/
/*-- SYSTEM CONTROL --*/
/*--------------------*/
#define SD_MONORAL		0xFF000005
#define SD_STEREO		0xFF000006

/*--------------------- BGM --------------------------------------*/

/*SONG PLAY CODE*/
#define SNG_PLAY_01		0x01000001	/*ACTION BGM(INTRO START)*/
#define SNG_PLAY_02		0x01000002	/*ACTION BGM(ACTION START)*/
#define SNG_PLAY_03		0x01000003	/*ACTION BGM(KAIHI START)*/
#define SNG_PLAY_04		0x01000004	/*SENNYUU BGM*/
#define SNG_PLAY_05		0x01000005
#define SNG_PLAY_06		0x01000006
#define SNG_PLAY_07		0x01000007
#define SNG_PLAY_08		0x01000008

/*SONG LOAD CODE*/
#define SNG_LOAD_00		0x01000010
#define SNG_LOAD_01		0x01000011
#define SNG_LOAD_02		0x01000012
#define SNG_LOAD_03		0x01000013
#define SNG_LOAD_04		0x01000014
#define SNG_LOAD_05		0x01000015
#define SNG_LOAD_06		0x01000016
#define SNG_LOAD_07		0x01000017

/*--------------------- SE ---------------------------------------*/

//　●区分　　　　　　　　　　　　　　　　　　　　//
//　P_プレイヤー動作、W_武器関連、I_アイテム関連　//
//　E_敵兵関連、V_音声、S_システム系　　　　　　　//

#define SD_W_SOCOM01 0x01 //ソコム・ショット
#define SD_P_FOOTL01 0x02 //通常足音左
#define SD_P_FOOTR01 0x03 //通常足音右
#define SD_W_EMPTY01 0x04 //ソコム・空撃ち
#define SD_P_HOFUKU05 0x05 //ほふく左
#define SD_P_HOFUKU06 0x06 //ほふく右
#define SD_P_SENAKA02 0x07 //壁に張付く
#define SD_P_STAND02 0x08 //しゃがみ、立ち上がり
#define SD_W_EQUIP02 0x09 //ソコム構える
#define SD_P_HEARTB01 0x0A //心臓の鼓動
#define SD_S_FULL0005 0x0B //アイテム満杯
#define SD_S_KAIHUKU1 0x0C //ライフ回復
#define SD_S_ITEM0003 0x0D //アイテム出現
#define SD_S_RADAR001 0x0E //レーダー復調
#define SD_S_OVER04 0x0F //ゲームオーバー

#define SD_P_DROP01 0x10 //エルード等のジャンプ
#define SD_I_CAMERA02 0x11 //デジカメ・シャッター
#define SD_P_FOOTL02 0x12 //弱足音（左）
#define SD_P_FOOTR02 0x13 //弱足音（右）
#define SD_S_EQUIP01 0x14 //ソコム・装備決定
#define SD_S_IDISP02 0x15 //アイテムメニュー表示
#define SD_S_IGET02 0x16 //アイテム取得
#define SD_S_ISEL02 0x17 //アイテム選択
#define SD_V_PDMG01 0x18 //スネークダメージ１
#define SD_V_PDMG02 0x19 //スネークダメージ２
#define SD_V_POUT0001 0x1A //スネークアウト
#define SD_W_RIFLE02 0x1B //ＰＳＧ１・ショット
#define SD_P_HIKIZU01 0x1C //死体引き摺り
#define SD_P_KINUZU01 0x1D //衣擦れ１
#define SD_P_KINUZU02 0x1E //衣擦れ２
//#define SD_S_CUR01 0x1F カーソル移動（タイトル、ブリーフィング）

//#define SD_S_WIN01 0x20 ウィンドウ表示（タイトル、ブリーフィング）
//#define SD_S_START01 0x21 決定音（タイトル、ブリーフィング）
#define SD_S_IDEC02 0x22 //アイテム使用
#define SD_S_BUZZER01 0x23 //カードキーＮＧブザー
#define SD_I_ZOOM01 0x24 //双眼鏡ズーム
#define SD_E_ATARU02 0x25 //銃柄殴り
#define SD_E_CAP_MO01 0x26 //注射キャップ外す
#define SD_W_RICOCH02 0x27 //跳弾（弱）
#define SD_W_RICOCH01 0x28 //跳弾（強）
#define SD_W_EXPLOS01 0x29 //爆発（普）
#define SD_S_SIREN06 0x2A //警報
#define SD_W_BOUND02 0x2B //手榴弾はずむ
#define SD_W_PIN02 0x2C //手榴弾ピン抜く
#define SD_E_FAMAS03 0x2D //敵・ファマス・ショット
#define SD_E_MORPHI01 0x2E //注射
#define SD_W_EQUIP03 0x2F //ソコム・リロード

#define SD_W_FAMAS02 0x30 //ファマス・ショット
#define SD_W_C4PUT02 0x31 //Ｃ４貼付
#define SD_W_C4SW01 0x32 //Ｃ４スイッチ
#define SD_E_DOWN03 0x33 //敵・転倒
#define SD_P_PUNCH02 0x34 //パンチ・ヒット
#define SD_P_KICK02 0x35 //キック・ヒット
#define SD_E_E_SWIN01 0x36 //敵兵キック振り音
#define SD_P_WALL02 0x37 //壁叩く
#define SD_P_P_SWIN02 0x38 //パンチ・風切り
#define SD_P_K_SWIN02 0x39 //キック・風切り
#define SD_W_CHAF0003 0x3A //チャフ散布
#define SD_P_UNTEIL01 0x3B //エルード移動（左）
#define SD_P_UNTEIR01 0x3C //エルード移動（右）
#define SD_E_EEQUIP01 0x3D //敵兵リロード
#define SD_E_S_DOWN01 0x3E //弱ダウン
#define SD_E_W_DOWN01 0x3F //壁あたり

#define SD_W_EQUIP04 0x40 //Ｍ９、弾丸装填
#define SD_W_EXPLOS02 0x41 //爆発（大）
#define SD_W_EXPLOS03 0x42 //爆発（チャフ、スタン）
#define SD_W_EQUIP05 0x43 //拳銃リロード
#define SD_A_DOORMOV1 0x44 //ドア動く
#define SD_A_DOORCLO1 0x45 //ドア閉じる
#define SD_A_DOOROPN1 0x46 //ドア開く
#define SD_V_PDMG03 0x47 //スネークダメージ３（落下ダメージ時）
#define SD_W_MAGAZI01 0x48 //空マガジン落ちる
#define SD_E_ECODBR01 0x49 //敵兵無線機破壊
#define SD_E_FMSSIR01 0x4A //敵兵・サイレンサー小銃
#define SD_W_EQUDEC01 0x4B //拳銃デコック
#define SD_W_MISSIL01 0x4C //ミサイル発射
#define SD_W_MISILE03 0x4D //ミサイル加速
#define SD_E_HIBANA02 0x4E //無線機破壊火花
#define SD_W_SIGNAL01 0x4F //ミサイル・レーダー音

#define SD_V_PDONMU01 0x50 //スネーク「動くな」
#define SD_E_HIZA01 0x51 //敵・膝を付く
#define SD_W_SOSIRE01 0x52 //ソコムサイレンサー付・ショット
#define SD_E_BIKKRI01 0x53 //びっくりマーク（！）
#define SD_V_PELUDE01 0x54 //スネークエルード堪え声
#define SD_V_PFALL01 0x55 //スネーク落下悲鳴
#define SD_V_PVOMIT01 0x56 //スネーク嘔吐
#define SD_A_RICDAN06 0x57 //被りダンボール主観着弾
#define SD_S_R_CALL01 0x58 //無線コール（相手から）
#define SD_S_R_FACE01 0x59 //人物カメラ切替
#define SD_S_R_CANCEL 0x5A //無線画面キャンセル
#define SD_S_R_CURSOR 0x5B //無線対象人物選択
#define SD_S_R_DISP01 0x5C //無線接続ノイズ
#define SD_S_R_SND01 0x5D //無線センド（自分から）
#define SD_E_CAMMOV01 0x5E //カメラ首振り１
#define SD_E_CAMFND01 0x5F //カメラフォーカス合わせる

#define SD_S_R_TUNE01 0x60 //無線周波数選択
#define SD_S_R_WINDW1 0x61 //無線人物ウィンドウ開く
#define SD_S_R_WINDW2 0x62 //無線人物ウィンドウ閉まる
#define SD_A_CODEC01 0x63 //敵兵無線切断ノイズ
#define SD_S_KAIFAL01 0x64 //階段転げ落ち
#define SD_W_CASE03 0x65 //薬莢落ちる
//#define SD_S_START001 0x66 ゲームスタート
#define SD_A_HASIGO1L 0x67 //ハシゴ足音（左） 
#define SD_A_HASIGO1R 0x68 //ハシゴ足音（右） 
#define SD_A_MAGAZI03 0x69 //マガジン敵兵に当たる 
#define SD_S_O2DAMAGE 0x6A //ライフゲージ減少
#define SD_V_KACHA01 0x6B //敵兵動作時衣擦れ１（弱）
#define SD_V_KACHA02 0x6C //敵兵動作時衣擦れ２（弱）
#define SD_V_KACHA03 0x6D //敵兵動作時衣擦れ３（強）
#define SD_V_KACHA04 0x6E //敵兵動作時衣擦れ４（強）
#define SD_A_MITCLO01 0x6F //水密扉閉める

#define SD_A_MITNOT01 0x70 //水密扉開かない
#define SD_A_MITOPN01 0x71 //水密扉開く
#define SD_I_MASK01 0x72 //マスク呼吸１、吐く（マスク主観時）
#define SD_A_MITROL01 0x73 //水密扉取っ手回す
#define SD_W_SIGNAL02 0x74 //ミサイル燃料切れ警告
#define SD_E_SHTFRE01 0x75 //敵兵ショットガン発砲
#define SD_E_SHTREL01 0x76 //敵兵ショットガンコッキング
#define SD_A_LOCKCLO1 0x77 //ロッカードア閉まる（ガチャン）
#define SD_S_RADAR003 0x78 //レーダー・ジャミング（使用不可）
#define SD_A_LOCKMOV1 0x79 //ロッカードア動作（ギギ…）
#define SD_A_LOCKOPN1 0x7A //ロッカードア開く（カチャ）
#define SD_A_LOCKHIT1 0x7B //ロッカーに主観で頭をぶつける。
#define SD_A_LOCKOPN2 0x7C //ロッカードア開かない（ガチン）
#define SD_A_LOCKDWN1 0x7D //ロッカードア外れる
//#define SD_S_KAIHUKU4 0x7E //パワーアップ（ボス後の回復）
#define SD_A_LOCKDWN2 0x7F //ロッカードア弾む

#define SD_A_LOCKDWN3 0x80 //ロッカードア倒れる
#define SD_A_LOCKMIL1 0x81 //ロッカーポスター胸ノック
#define SD_A_MOROTI01 0x82 //Ｍ９注射器落ちる
#define SD_A_LOCKDMG1 0x83 //ロッカードア軋みズレ
#define SD_A_LOCKATK1 0x84 //ロッカードア攻撃当たり
#define SD_A_ITEMKIE1 0x85 //アイテム時間切れ消滅
#define SD_A_M_NINE01 0x86 //Ｍ９注射器刺さる
#define SD_V_PKIAI01 0x87 //スネーク動作気合
#define SD_P_GUNPNC01 0x88 //小火器パンチ・ヒット
#define SD_A_DAMBRA01 0x89 //被り段ボール壊れる
//#define SD_S_TYPING01 0x8A パスコード通常タイプ音
//#define SD_S_TYPING02 0x8B パスコードラストタイプ音
//#define SD_S_V_CANS02 0x8C SPモードキャンセル音
#define SD_S_R_SEL01 0x8D //無線短縮ウィンドウ出す
#define SD_E_KOFURI01 0x8E //敵兵ホールドアップ腰振り
#define SD_E_EHEART01 0x8F //指向性マイク用心臓音１

#define SD_E_EHEART02 0x90 //指向性マイク用心臓音２
#define SD_E_EHEART03 0x91 //指向性マイク用心臓音３
#define SD_E_EHEART04 0x92 //指向性マイク用心臓音４
#define SD_S_MICNOIZE 0x93 //マイク通奏ノイズ(14fps連)
#define SD_S_MICSWING 0x94 //マイク動作ノイズ(7fps連)
#define SD_P_STAND03 0x95 //側転着地(ライデン)
#define SD_I_SPRAY01 0x96 //凍結スプレー噴射開始
#define SD_I_SPRAY02 0x97 //凍結スプレー噴射中(01を単コール後、02を15fps連)
#define SD_A_FREEZE01 0x98 //時限Ｃ４凍結中「ピキピキ」(15fps連)
#define SD_A_FREEZE02 0x99 //時限Ｃ４凍結完了
#define SD_A_C4LEDBR1 0x9A //時限Ｃ４ＬＥＤ点滅(最も明るくなる6fps前にコール)
#define SD_S_PHONE001 0x9B //携帯電話コール「ピリリリ…」
#define SD_S_PHONE002 0x9C //携帯電話受信「ピッ（ボタンを押す音）」
#define SD_S_PHONE000 0x9D //携帯電話ミュート（無線画面移行時）
#define SD_S_CAM_OK01 0x9E //カメラ撮影ＯＫ
#define SD_S_CAM_NG01 0x9F //カメラ撮影ＮＧ

#define SD_S_PHONEMEL 0xA0 //携帯電話着メロ（メインテーマ）
#define SD_A_DOGTAG01 0xA1 //ドッグタグ落下音
#define SD_I_MASK02 0xA2 //マスク呼吸２、吸う（マスク主観時）
#define SD_E_ABAKAN01 0xA3 //敵兵アバカン発砲
#define SD_E_M4EGRE01 0xA4 //敵兵Ｍ４グレネード発射
#define SD_E_M4ENEM01 0xA5 //敵兵Ｍ４発砲
#define SD_E_MACALL01 0xA6 //敵兵マカロフ発砲
#define SD_W_AKS74U01 0xA7 //ＡＫ・ショット
#define SD_W_AKSIRE01 0xA8 //ＡＫサイレンサー・ショット
#define SD_W_M_FOUR01 0xA9 //Ｍ４・ショット
#define SD_W_SOCSIR12 0xAA //ソコムサイレンサー・ショット
#define SD_W_RIFSIR01 0xAB //ＰＳＧ１－Ｔ・ショット
#define SD_W_LANTURE2 0xAC //ＲＧＢ－６・ショット
#define SD_W_NIKFIRE1 0xAD //ニキータミサイル発射
#define SD_W_NIKSRCH1 0xAE //ニキータミサイル加速（主観画面）
#define SD_W_MINEEXP1 0xAF //クレイモア地雷爆発

#define SD_W_MINEPUT1 0xB0 //クレイモア地雷セット
#define SD_W_BOOKPUT1 0xB1 //エロ本セット
#define SD_I_SENS_B01 0xB2 //センサーＢ発信音（近付くにつれ早くなる）
//#define SD_S_GO_TYPE1 0xB3 ゲームオーバー文字出力音
#define SD_E_CAMMOV02 0xB4 //カメラ首振り２
#define SD_E_CAMMOV03 0xB5 //カメラ首振り３
#define SD_E_CAMMOV04 0xB6 //カメラ首振り４
#define SD_E_CAMCHA01 0xB7 //カメラチャフ首振り１
#define SD_E_CAMCHA02 0xB8 //カメラチャフ首振り２
#define SD_E_CAMCHA03 0xB9 //カメラチャフ首振り３
#define SD_E_CAMCHA04 0xBA //カメラチャフ首振り４
#define SD_A_RICCOMP1 0xBB //コンソール着弾
#define SD_A_SPARK01 0xBC //青ヒモ状放電
#define SD_P_ELEBTN01 0xBD //中央棟エレベーターボタン押し
#define SD_P_ELEMOV01 0xBE //中央棟エレベーター駆動中（持続SE）SE_JOUCHUU_OFF
#define SD_P_ELEOPN01 0xBF //中央棟エレベータードア動作

#define SD_P_ELECLS01 0xC0 //中央棟エレベータードア閉まる
#define SD_P_NKTPNC01 0xC1 //ニキータ・ヒット
#define SD_P_NKTSWI01 0xC2 //ニキータ振り
#define SD_P_DCTDANSA 0xC3 //ダクト段差越え
#define SD_A_ELECHM01 0xC4 //中央棟エレベーター到着チャイム
#define SD_A_RICCOMP2 0xC5 //コンソール破損
#define SD_S_R_VTR_FF 0xC6 //無線早送り(3fps連)
#define SD_A_ELESTP01 0xC7 //中央棟エレベーター待ち到着（125msec75fpsで到着）
#define SD_A_NORD_ON1 0xC8 //ノード端末近接ＯＮ
//#define SD_S_SAVEOK01 0xC9 セーブ完了ジングル
#define SD_S_R_FACE02 0xCA //人物カメラ切替(プレイヤー側)
#define SD_W_EQUIP06 0xCB //ＲＧＢ６リロード１弾込め
#define SD_W_EQUIP07 0xCC //ＲＧＢ６リロード２シリンダー収納
#define SD_I_FNAEAT01 0xCD //アイテム船虫レーション消費
#define SD_I_FNAESC01 0xCE //アイテム船虫逃げる
#define SD_S_COUNTDW1 0xCF //タイマーカウントダウン警告

#define SD_S_COUNTRV1 0xD0 //タイマーカウントアップ(数字に合わせて連)
#define SD_A_FIRERED1 0xD1 //炎燃える１（6～16fpsランダム連）
#define SD_A_FIRERED2 0xD2 //炎燃える２(以下、複数個ある場合に使う)
#define SD_A_FIRERED3 0xD3 //炎燃える３
#define SD_A_FIRERED4 0xD4 //炎燃える４
#define SD_A_FIRESMK1 0xD5 //炎消え煙立ち昇る１（6～16fpsランダム連）
#define SD_A_FIRESMK2 0xD6 //炎消え煙立ち昇る２(以下、複数個ある場合に使う)
#define SD_A_FIRESMK3 0xD7 //炎消え煙立ち昇る３
#define SD_A_FIRESMK4 0xD8 //炎消え煙立ち昇る４
#define SD_A_FIREBOK1 0xD9 //雑誌、人燃える１（6～16fpsランダム連）
#define SD_A_FIREBOK2 0xDA //雑誌、人燃える２(複数個ある場合に使う)
#define SD_A_FLYFLY01 0xDB //蝿が飛ぶ
#define SD_I_SIREEQU1 0xDC //ソコムＡＫ、サイレンサー装着
#define SD_P_ELEOPED1 0xDD //中央棟エレベータードア開き切る
#define SD_A_EXPLOSW1 0xDE //水中爆発音
#define SD_I_EROBFA01 0xDF //エロ本落下（開いている）

#define SD_I_EROBBR01 0xE0 //エロ本への着弾
#define SD_I_FNAPET01 0xE1 //アイテム船虫レーションに取付く


#define SD_S_N_COMP01 0xE7 //ノード端末読みこみ完了
#define SD_S_N_LOAD01 0xE8 //ノード端末読み込み中(6fps連)

//#define SD_S_TWINKY01 0xE9 //「ファイト！」文字点滅
#define SD_S_TYPING01 0xEA //パスコード通常タイプ音
#define SD_S_TYPING02 0xEB //パスコードラストタイプ音
#define SD_S_WINOPNL1 0xEC //ウィンドウ枠開く(左から右へ)
#define SD_S_WINOPNR1 0xED //ウィンドウ枠開く(右から左へ)
#define SD_S_WINCLSL1 0xEE //ウィンドウ枠閉まる(左から右へ)
#define SD_S_WINCLSR1 0xEF //ウィンドウ枠閉まる(右から左へ)

#define SD_S_B_READY1 0xF0 //ボスサバ「レディー！」
#define SD_S_B_FIGHT1 0xF1 //ボスサバ「ゴー！」
#define SD_S_SAVEOK01 0xF2 //セーブ完了ジングル
#define SD_S_N_START1 0xF3 //ノード端末開始＆終了
#define SD_S_TYPING03 0xF4 //切り替えカーソル音
#define SD_S_LINEMOV1 0xF5 //ライン動作エフェクト音
#define SD_S_TWINKY02 0xF6 //「ユーウィン」文字点滅
#define SD_S_V_CANS02 0xF7 //SPモードキャンセル音
#define SD_S_CUR01 0xF8 //カーソル移動（タイトル、ブリーフィング）
#define SD_S_WIN01 0xF9 //ウィンドウ表示（タイトル、ブリーフィング）
#define SD_S_START01 0xFA //決定音（タイトル、ブリーフィング）
#define SD_S_START001 0xFB //ゲームスタート
//#define SD_S_HEX_IN01 0xFC 六角フェード開始時
#define SD_S_HEXOUT01 0xFD //六角フェード終了時
#define SD_S_WINOPN01 0xFE //ウィンドウ枠開く(真ん中)
#define SD_S_WINCLS01 0xFF //ウィンドウ枠閉まる(真ん中)

#define SD_S_HEX_IN01 0xB3 //六角フェード開始時
#define SD_S_GO_TYPE1 0xFC //ゲームオーバー文字出力音

#define SD_E_BROODSW1 0x1F //天狗兵、血プシー！
#define SD_A_SWIGUARD 0x20 //刀ガード構える
#define SD_A_SWINGBAK 0x21 //刀振り返し
#define SD_A_SWINGBIT 0x66 //刀刺し風切り
#define SD_A_SWINGCUT 0x7E //刀振り風切り
#define SD_A_SWINGROL 0x8A //刀回転振り風切り
#define SD_A_SWORDBIT 0x8B //刀刺しヒット
#define SD_A_SWORDCLO 0x8C //刀収める
#define SD_A_SWORDCNG 0xC9 //刀みね打ち切り替え
#define SD_A_SWORDCUT 0xE2 //刀振りヒット
#define SD_A_SWORDHAD 0xE3 //刀と刀の衝突
#define SD_A_SWORDHIT 0xE4 //刀みね打ちヒット
#define SD_A_SWORDRIC 0xE5 //刀銃弾はじき
#define SD_A_SWORDOPN 0xE6 //刀装備開始
#define SD_A_SWORDSET 0xE9 //刀かまえる

#define SD_S_TWINKY01 0x5BE //「ファイト！」文字点滅


/*----------------------------------------------------------------*/

#endif
