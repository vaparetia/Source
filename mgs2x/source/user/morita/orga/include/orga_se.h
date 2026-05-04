/*
   orga.h
   オルガ/SEヘッダファイル

   2000/01/12 T.Morita
   $Id: orga_se.h,v 1.1.1.3 2002/11/19 11:46:25 Yoshizawa1 Exp $			
*/

#define ORGA_SE_SGR_PIN      SD_W_PIN02
#define ORGA_SE_SGR_BOUND    SD_W_BOUND02

#define ORGA_SE_AMO_BOUND    SD_W_MAGAZI01

#define ORGA_SE_USP_FIRE     SD_W_SOCOM02
#define ORGA_SE_USP_FIRE_E   SD_W_EMPTY01
#define ORGA_SE_USP_RELOAD   SD_W_EQUIP03
#define ORGA_SE_USP_BLOWBK   SD_W_EQUIP02

/*グレネードなげ*/
#define ORGA_VO_THROUGH_SGR0  SD_V_ORGA04  //オルガ、手榴弾投げる「食らえ！」
#define ORGA_VO_THROUGH_SGR1  SD_V_ORGA03  //オルガ、気合「ィヤッ！」
#define ORGA_VO_THROUGH_SGR2  SD_V_SORGA19 //オルガ、マスト切り気合「ティッ！」

/*ダメージを食らった時*/
#define ORGA_VO_DAMAGE0 SD_V_ORGA05  //オルガ、ダメージ１「ウッ！」
#define ORGA_VO_DAMAGE1 SD_V_ORGA06  //オルガ、ダメージ２「アァッ！」
#define ORGA_VO_PASTOUT SD_V_ORGA07  //オルガ、気を失うやられ「ア…ッアハゥ」

/* やられついでにいう言葉 */
#define ORGA_VO_SHOTTEASE0  SD_V_SORGA09 //オルガ、「やるわねぇ」
#define ORGA_VO_SHOTTEASE1  SD_V_SORGA10 //オルガ、「まだよ！」

/*スネークがどこに隠れているのか分からない時*/
#define ORGA_VO_HIDETEASE0  SD_V_SORGA01 //オルガ、「隠れてるだけ！？」
#define ORGA_VO_HIDETEASE1  SD_V_SORGA02 //オルガ、「たいした事無いわね！」
#define ORGA_VO_HIDETEASE2  SD_V_SORGA04 //オルガ、「出て来なさい！」

/*オルガがスネークを見つけた*/
#define ORGA_VO_FINDTEASE0  SD_V_SORGA06 //オルガ、「丸見えよ！」
#define ORGA_VO_FINDTEASE1  SD_V_SORGA03 //オルガ、「そこね！」
#define ORGA_VO_FINDTEASE2  SD_V_SORGA22 //オルガ、「逃げてるつもり？」

/*外した時*/
#define ORGA_VO_MISSTEASE0  SD_V_SORGA05 //オルガ、「その程度？」
#define ORGA_VO_MISSTEASE1  SD_V_SORGA07 //オルガ、「どこを狙ってるの？」

/*エルード時*/
#define ORGA_VO_ERUDE0      SD_V_SORGA08 //オルガ、「ハハハ…（嘲笑）」
#define ORGA_VO_ERUDE1      SD_V_SORGA23 //オルガ、「バカにしてるの？」

#define ORGA_VO_TALK0 SD_V_SORGA11 //オルガ、「私は生まれた時から…」
#define ORGA_VO_TALK1 SD_V_SORGA12 //オルガ、「戦場の中で私は育った」
#define ORGA_VO_TALK2 SD_V_SORGA13 //オルガ、「戦いと勝利が私を育てた」
#define ORGA_VO_TALK3 SD_V_SORGA14 //オルガ、「部隊と共に私は戦い…」
#define ORGA_VO_TALK4 SD_V_SORGA15 //オルガ、「生も死も全てを分かち…」
#define ORGA_VO_TALK5 SD_V_SORGA16 //オルガ、「部隊の他には家族も…」
#define ORGA_VO_TALK6 SD_V_SORGA17 //オルガ、「部隊は私の全てだ」
#define ORGA_VO_TALK7 SD_V_SORGA18 //オルガ、「お前が誰であろうと…」

/*ジャンプの気合い*/
#define ORGA_VO_AIRJUMP SD_V_SORGA19 //オルガ、マスト切り気合「ティッ！」

/* 怒りの表現 */
#define ORGA_VO_UTTER0  SD_V_ORGA03  //オルガ、気合「ィヤッ！」
#define ORGA_VO_UTTER1  SD_V_ORGA01  //オルガ、舌打ち「チィッ！」

/* ダメージが大きい時 */
#define ORGA_VO_TIRED   SD_V_SORGA28 //オルガ、息切れ「ッッハァハァ」

/* スネークに見つけられた時 */
#define ORGA_VO_FOUND   SD_V_ORGA02  //オルガ、見つかった！？「ハッ！」

#define ORGA_SE_BRK_LIGHT    SD_A_LLIGHT01 //投光器破壊音
#define ORGA_SE_BLW_HOLO1    SD_A_CLOTH01  //布めくれ
#define ORGA_SE_BLW_HOLO2    SD_A_CLOTH02  //布バタバタ2
#define ORGA_SE_BLW_HOLO3    SD_A_CLOTH03  //布バタバタ3

#define ORGA_SE_WLL_SCAR1    SD_A_RICO_OM1 //オルガ戦用金属跳弾（鉄板、鉄柵）
#define ORGA_SE_WLL_SCAR2    SD_A_RICO_OM2 //オルガ戦用鉄板跳弾（コンテナ）
#define ORGA_SE_WLL_SCAR3    SD_A_RICO_ON1 //オルガ戦用通常跳弾（床、壁）
#define ORGA_SE_WLL_SCAR4    SD_A_RICO_OW1 //オルガ戦用木製跳弾（木箱）
