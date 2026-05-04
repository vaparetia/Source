/*
   orga_flag.h
   オルガ/ヘッダファイル

   2000/07/11 T.Morita
   $Id: orga_flag.h,v 1.1.1.3 2002/11/19 11:46:25 Yoshizawa1 Exp $			
*/

/* フラグ情報(用途複数あり) */
enum flag_t
{
    ORGA_F_NONE         = 0x00000000,

    /* work->stll_mtn用 */
    ORGA_F_IS_SQUAT    	= 0x00000001, /* 静止モーションの状態 (work->stll_mtn) */
    ORGA_F_IS_BEHIND   	= 0x00000002, /* 静止モーションの状態 (work->stll_mtn) */

    /* work->flag用 */ 
    ORGA_F_THINK_MSK   	= 0x0000000f, /* 思考（モード＋レベル）   */
    ORGA_F_MODE_MSK    	= 0x00000003, /* モード（移動0x0／攻撃0x4／警戒0x8／挑発0xc） */
    ORGA_F_OFFENCE     	= 0x00000001, /* 攻撃モード（1:攻撃モード 0:移動モード） */
    ORGA_F_MODIST      	= 0x00000002, /* 警戒モード（警戒モード以外,攻撃か移動になる） */
    ORGA_F_LEVEL_MSK   	= 0x0000000c, /* プレーヤーレベル 0:easy 1:norm 2:hard 3:mania */
    ORGA_F_LEVEL_EASY  	= 0x00000000,
    ORGA_F_LEVEL_NORM  	= 0x00000004,
    ORGA_F_LEVEL_HARD  	= 0x00000008,
    ORGA_F_LEVEL_MANIA 	= 0x0000000c,

    ORGA_F_NVR_DAMAGED 	= 0x00000010, /* 無敵                   */
    ORGA_F_NVR_RESET   	= 0x00000020, /* リセットできない       */
    ORGA_F_ENB_SAFE    	= 0x00000040, /* 安全検知（身をさらけだしているかどうか）*/
    ORGA_F_ENB_AVOID   	= 0x00000080, /* 回避検知（狙われているかどうか）        */
    ORGA_F_ENB_AIM     	= 0x00000100, /* 狙い検知（スネークを狙えるかどうか）    */
    ORGA_F_ENB_ROUTE   	= 0x00000200, /* 移動検知（障害物を避けるかどうか 移動モードのみ）  */

    ORGA_F_DANGR_STAND 	= 0x00000400, /* 立つと危険             */
    ORGA_F_DANGR_RIGHT 	= 0x00000800, /* 右側が危険             */
    ORGA_F_DANGR_LEFT  	= 0x00001000, /* 左側が危険             */
    ORGA_F_DMG_FARHIDE 	= 0x00002000, /* 奥の隠れ場所でやられた */
    ORGA_F_SRCH_HIDE   	= 0x00004000, /* 最も近い隠れ場所を検索 */
    ORGA_F_SAME_HIDE   	= 0x00008000, /* 同じ隠れ場所になった   */

    ORGA_F_STOP_STILL  	= 0x00010000, /* モーション静止状態     */
    ORGA_F_NOMISS_SHOT 	= 0x00020000, /* 必ず当てる             */
    ORGA_F_UNRECOG_POS 	= 0x00040000, /* 相手が見えないでいる   */
    ORGA_F_UNRECOG_AIM 	= 0x00080000, /* 相手がどこにいるのか分からない  */
    ORGA_F_GOT_SICK    	= 0x00100000, /* 参っている(50%)          これら2つが同時に立つと(12.5%) */
    ORGA_F_GOT_WORSE   	= 0x00200000, /* けっこう参っている(25%)  */
    ORGA_F_LIFE_MASK   	= 0x00300000, /* ライフを抽出するマスク   */
    ORGA_F_AIMED_NEAR  	= 0x00400000, /* 現在,近くを狙われている  */
    ORGA_F_DMG_HOROFIRE	= 0x00800000, /* ホロ撃ちのときやられた   */
    ORGA_F_DMG_HOROATTK	= 0x01000000, /* ホロ攻撃のときやられた   */
    ORGA_F_PLYER_SHOOT 	= 0x02000000, /* プレイヤーが銃(m9)を撃った */

    ORGA_F_DEMO_SKIP	= 0x04000000, /* スキップ出来るようになる */
    ORGA_F_DEMO_MOVIE	= 0x08000000, /* デモの相互排除(再生している時オン) */

    ORGA_F_FACETO_HEAD 	= 0x10000000, /* 首のみ回転（首から回転） */
    ORGA_F_FACETO_AIM1 	= 0x20000000, /* 両手回転（胸回転有り）   */
    ORGA_F_FACETO_AIM2 	= 0x40000000, /* 片手回転（胸回転なし）   */
    ORGA_F_FACETO_ALL  	= 0x70000000, /* ホーミングフラグすべて   */

    ORGA_F_OVERLAYED   	= 0x80000000, /* モーションがオーバーレイされた */


    /* その他 */ 
    ORGA_F_PLYR_SIGHT  	= 0x00008000, /* プレイヤーが主観    (work->ply_sight) */
    ORGA_F_PLYR_HIDING 	= 0x00008000, /* プレイヤーが隠れてる(work->ply_hide)  */
    ORGA_F_PLYR_HIDLNG 	= 0x00004000, /* 長い間隠れてる      (work->ply_hide)  */
    ORGA_F_PLYR_HIDEXP 	= 0x00003000, /* 隠れていない時間    (work->ply_hide)  */
    ORGA_F_PLYR_HIDTIC 	= 0x00001000, /* 隠れていない時間単位(work->ply_hide)  */
    ORGA_F_PLYR_HIDTIM 	= 0x0000003f, /* 隠れている時間      (work->ply_hide)  */
    ORGA_F_PLYR_HIDPOS 	= 0x00000fc0, /* 隠れている場所のID  (work->ply_hide)  */
    ORGA_F_PLYR_HIDCLR 	= 0x0000ffc0, /* 隠れている時間クリア(work->ply_hide)  */

    ORGA_F_DONT_SHOOT  	= 0x00008000, /* 狙っている場所に至らないので撃たない(work->aim_flg)   */

    /* work->act_flg 特別行動フラグ（ある特定の条件を満たすことでフラグが上がる） */
    ORGA_F_ERUDE_ATTK  	  = 0x00000001, /* アンチエルード時の攻撃 */
    ORGA_F_GOUND_ATTK     = 0x00000002, /* 禁止区域で匍匐している */
    ORGA_F_HOLO_ATTK      = 0x00000004, /* ホロをめくる           */
    ORGA_F_GAP_ATTK       = 0x00000008, /* すき間から撃ってくる   */
    ORGA_F_SPOTLGT_ATTK   = 0x00000200, /* 投光器まぶしい攻撃               */
    ORGA_F_CHNGLGT_ATTK   = 0x00010000, /* 投光器の向き変え(付随フラグ)     */
    ORGA_F_HOLO_BLOW_ATTK = 0x00000800, /* ホロ飛ばし（前側）               */
    ORGA_F_HOLO_RUN_ATTK  = 0x00020000, /* ホロ飛ばし走り                   */
    ORGA_F_STAND_ATTK     = 0x00001000, /* 禁止区域に入っている（匍匐以外） */
    ORGA_F_HOLO_HIDE_ATTK = 0x00002000, /* ホロをめくった所から攻撃 */
    ORGA_F_BIBLIO_ATTK    = 0x00004000, /* 自伝 語りモード攻撃      */
    ORGA_F_MAGAZIN_ATTK   = 0x00008000, /* マガジンに反応する       */
    ORGA_F_HALF_ATTK      = 0x00000010, // ライフ１／２      ライフが１／２になったときに呼ばれる
    ORGA_F_QUAD_ATTK   	  = 0x00000020, // ライフ１／４      ライフが１／４になったときに呼ばれる
    ORGA_F_EIGHT_ATTK  	  = 0x00000040, // ライフ１／８      ライフが１／８になったときに呼ばれる
    ORGA_F_THIRD_ATTK     = 0x00000100, // ライフ３／４      ライフが３／４になったときに呼ばれる
    ORGA_F_MOVE_ATTK  	  = 0x00000080, /* 強制移動 */
    ORGA_F_INVALID_ATTK   = 0x000001f0, /* 特別行動の条件を見ない 制限ビットマスク */

    ORGA_F_GAMEOVER_ATTK  = 0x00040000, /* ゲームオーバーですね     */
} ;



/* completely same as misc/orga_hol2.c */
enum holo_flag_t
{
    ORGA_HOL_COVERED=0,/* horo_def.anm */
    ORGA_HOL_ORG_OPEN, /* horo_bo.anm  */
    ORGA_HOL_ORG_FREE, /* horo_bod.anm */
    ORGA_HOL_ORG_GONE, /* horo_box.anm */
    ORGA_HOL_PLY_OPNL, /* horo_lo.anm */
    ORGA_HOL_PLY_FREL, /* horo_lod.anm */
    ORGA_HOL_PLY_GONL, /* horo_lox.anm */
    ORGA_HOL_PLY_OPNR, /* horo_ro.anm */
    ORGA_HOL_PLY_FRER, /* horo_rod.anm */
    ORGA_HOL_PLY_GONR, /* horo_rox.anm */

    ORGA_HOL_N_FLAGS,
} ;
