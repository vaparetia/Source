/*
   fort.h
   フォーチュン/SEヘッダファイル

   2000/01/12 T.Morita
   $Id: fort_se.h,v 1.1.1.3 2002/11/19 11:46:16 Yoshizawa1 Exp $			
*/

#define FRT_SE_SGR_PIN      SD_W_PIN02
#define FRT_SE_SGR_BOUND    SD_W_BOUND02

#define FRT_SE_AMO_BOUND    SD_W_MAGAZI01

#define FRT_SE_USP_FIRE     SD_W_SOCOM02
#define FRT_SE_USP_FIRE_E   SD_W_EMPTY01
#define FRT_SE_USP_RELOAD   SD_W_EQUIP03
#define FRT_SE_USP_BLOWBK   SD_W_EQUIP02

/*グレネードなげ*/
#define FRT_VO_THROUGH_SGR0  SD_V_ORGA04  //フォーチュン、手榴弾投げる「食らえ！」
#define FRT_VO_THROUGH_SGR1  SD_V_ORGA03  //フォーチュン、気合「ィヤッ！」
#define FRT_VO_THROUGH_SGR2  SD_V_SORGA19 //フォーチュン、マスト切り気合「ティッ！」

/*ダメージを食らった時*/
#define FRT_VO_DAMAGE0 SD_V_ORGA05  //フォーチュン、ダメージ１「ウッ！」
#define FRT_VO_DAMAGE1 SD_V_ORGA06  //フォーチュン、ダメージ２「アァッ！」
#define FRT_VO_PASTOUT SD_V_ORGA07  //フォーチュン、気を失うやられ「ア…ッアハゥ」

/* やられついでにいう言葉 */
#define FRT_VO_SHOTTEASE0  SD_V_SORGA09 //フォーチュン、「やるわねぇ」
#define FRT_VO_SHOTTEASE1  SD_V_SORGA10 //フォーチュン、「まだよ！」

/*スネークがどこに隠れているのか分からない時*/
#define FRT_VO_HIDETEASE0  SD_V_SORGA01 //フォーチュン、「隠れてるだけ！？」
#define FRT_VO_HIDETEASE1  SD_V_SORGA02 //フォーチュン、「たいした事無いわね！」
#define FRT_VO_HIDETEASE2  SD_V_SORGA04 //フォーチュン、「出て来なさい！」

/*フォーチュンがスネークを見つけた*/
#define FRT_VO_FINDTEASE0  SD_V_SORGA06 //フォーチュン、「丸見えよ！」
#define FRT_VO_FINDTEASE1  SD_V_SORGA03 //フォーチュン、「そこね！」
#define FRT_VO_FINDTEASE2  SD_V_SORGA22 //フォーチュン、「逃げてるつもり？」

/*外した時*/
#define FRT_VO_MISSTEASE0  SD_V_SORGA05 //フォーチュン、「その程度？」
#define FRT_VO_MISSTEASE1  SD_V_SORGA07 //フォーチュン、「どこを狙ってるの？」

/*エルード時*/
#define FRT_VO_ERUDE0      SD_V_SORGA08 //フォーチュン、「ハハハ…（嘲笑）」
#define FRT_VO_ERUDE1      SD_V_SORGA23 //フォーチュン、「バカにしてるの？」

#define FRT_VO_TALK0 SD_V_SORGA11 //フォーチュン、「私は生まれた時から…」
#define FRT_VO_TALK1 SD_V_SORGA12 //フォーチュン、「戦場の中で私は育った」
#define FRT_VO_TALK2 SD_V_SORGA13 //フォーチュン、「戦いと勝利が私を育てた」
#define FRT_VO_TALK3 SD_V_SORGA14 //フォーチュン、「部隊と共に私は戦い…」
#define FRT_VO_TALK4 SD_V_SORGA15 //フォーチュン、「生も死も全てを分かち…」
#define FRT_VO_TALK5 SD_V_SORGA16 //フォーチュン、「部隊の他には家族も…」
#define FRT_VO_TALK6 SD_V_SORGA17 //フォーチュン、「部隊は私の全てだ」
#define FRT_VO_TALK7 SD_V_SORGA18 //フォーチュン、「お前が誰であろうと…」

/*ジャンプの気合い*/
#define FRT_VO_AIRJUMP SD_V_SORGA19 //フォーチュン、マスト切り気合「ティッ！」

/* 怒りの表現 */
#define FRT_VO_UTTER0  SD_V_ORGA03  //フォーチュン、気合「ィヤッ！」
#define FRT_VO_UTTER1  SD_V_ORGA01  //フォーチュン、舌打ち「チィッ！」

/* ダメージが大きい時 */
#define FRT_VO_TIRED   SD_V_SORGA28 //フォーチュン、息切れ「ッッハァハァ」

/* スネークに見つけられた時 */
#define FRT_VO_FOUND   SD_V_ORGA02  //フォーチュン、見つかった！？「ハッ！」

#define FRT_SE_BRK_LIGHT    SD_A_LLIGHT01 //投光器破壊音
#define FRT_SE_BLW_HOLO1    SD_A_CLOTH01  //布めくれ
#define FRT_SE_BLW_HOLO2    SD_A_CLOTH02  //布バタバタ2
#define FRT_SE_BLW_HOLO3    SD_A_CLOTH03  //布バタバタ3

#define FRT_SE_WLL_SCAR1    SD_A_RICO_OM1 //フォーチュン戦用金属跳弾（鉄板、鉄柵）
#define FRT_SE_WLL_SCAR2    SD_A_RICO_OM2 //フォーチュン戦用鉄板跳弾（コンテナ）
#define FRT_SE_WLL_SCAR3    SD_A_RICO_ON1 //フォーチュン戦用通常跳弾（床、壁）
#define FRT_SE_WLL_SCAR4    SD_A_RICO_OW1 //フォーチュン戦用木製跳弾（木箱）
