/*
	hostage.h
	船倉兵

	2001/01/10 Y.Korekado
	$Id: hostage.h,v 1.1.1.3 2002/11/19 11:44:17 Yoshizawa1 Exp $
*/

/* イベント */

/*----------------------------------------------------------------*/
#define CHILD_TARGET_NUM	(11)
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world
#define MAX_LOD	(3)
#define	BODY_FLAG	(DG_FLAG_IRREACTION|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
//#define BODY_FLAG (DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE)

#define	HSTG_STATUS_RIC			0x00000001	/* エイムズ */
#define	HSTG_STATUS_WOMEN		0x00000002	/* 女性 */
#define	HSTG_STATUS_JITABATA	0x00000004	/* 声かけられるとじたばた */
#define	HSTG_STATUS_PEE			0x00000008	/* 声かけられるとおもらし */
#define	HSTG_STATUS_SOPPO		0x00000010	/* 声かけられると反対向く */
#define	HSTG_STATUS_LOD			0x00000020	/* LOD */
#define	HSTG_STATUS_N_GREEN		0x00000040	/* 緑ネクタイ */
#define	HSTG_STATUS_N_BROWN		0x00000080	/* 茶ネクタイ */
#define	HSTG_STATUS_OL			0x00000100	/* 若い女性 */
#define	HSTG_STATUS_FOOT		0x00000200	/* 足伸ばす */
#define	HSTG_STATUS_JITABATA2	0x00000400	/* 声かけられるとじたばた音声違い */
#define	HSTG_STATUS_NO_EIMS		0x00000800	/* 私はエイムズではない */
#define	HSTG_STATUS_NO_EIMS2	0x00001000	/* 私はエイムズではない２ */
#define	HSTG_STATUS_JENIFA		0x00002000	/* 私はじぇにファー */
#define	HSTG_STATUS_DARE		0x00004000	/* 私はランダム名前 */
#define	HSTG_STATUS_NOEIMS_OL	0x00008000	/* 私はじぇにファー普通 */
#define	HSTG_STATUS_MAIL		0x00010000	/* 携帯メール中 */
#define	HSTG_STATUS_MAIL_STOP	0x00020000	/* 携帯メール中止 */
#define	HSTG_STATUS_NO_HEART	0x00040000	/* 心音なし */
#define	HSTG_STATUS_NO_TARGET	0x00080000	/* ターゲットなし */
#define	HSTG_STATUS_RINDA		0x00100000	/* 私はリンダ */
#define	HSTG_STATUS_CYNDI		0x00200000	/* 私はシンディー */
#define	HSTG_STATUS_MAIL_NOCALL	0x00400000	/* 携帯メール打つだけ */

#define	HSTG_STATUS_DIE			0x80000000	/* 死亡 */
#define	HSTG_STATUS_MUST_DIE	0x40000000	/* 死亡確定 */

/*----------------------------------------------------------------*/

#define HSTG_NOTICE_NOISE 0x00000001	/* 物音モード */


/*----------------------------------------------------------------*/
#define HSTG_MIC_QUETION_DIS 3000	/* あんたがエイムズ？の有効距離 */
#define HSTG_MIC_QUETION_DIR 512	/* あんたがエイムズ？の有効角度 */
#define HOST_SE_COS_45	(0.7071067811865f)
#define HOST_SE_COS_30	(0.8660254037844f)
#define HOST_SE_COS_20	(0.9396926207859f)
#define HOST_SE_COS_15	(0.9659258262891f)
#define HOST_SE_COS_10	(0.9848077530122f)
#define HOST_SE_COS_5	(0.9961946980917f)

/*----------------------------------------------------------------*/
//npc->act->status
// システムでは下位１６ビット使う、上位はそれぞれ好きなように
#define NPC_ACT_STATUS_PUNCH_DAMAGE	0x00010000	/* 打撃系ダメージ受けた */
#define NPC_ACT_STATUS_OPEN_SKIRT	0x00020000	/* スカート御開幕 */
#define NPC_ACT_STATUS_TERROR		0x00040000	/* 恐怖 */
#define NPC_ACT_STATUS_MUST_DIE		0x00080000	/* 死亡確定 */
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		body ;
	OBJECT		lod_body[ MAX_LOD-1 ] ;
	OBJECT_CHG	object_chg[MAX_LOD] ;
	CONTROL		control ;
	FMATRIX		lights[2] ;
	OBJECT		necktie ;	/* ネクタイ */

	NPCWORK		npc ;
	NAVIGATE 	navigate ;
	NAVITARGET	navitrg ;
	RADAR_CTRL	rctrl ;
	NPCADJUST	npcadjust ;

    TARGET			deftrg ;	/* 防御ターゲット */
	CAPTURE_TARGET	capture ;	/* 捕まりターゲット */
    TARGET			pushtrg ;	/* ぶつかりターゲット */

    TARGET			def_child[CHILD_TARGET_NUM] ;	/* 子ターゲット*/

    int			headmark ;
    int			model_num ;

	/* sampleキャラクター専用 */
	int			think1 ;
	int			think2 ;
	int			think3 ;
	int			count3 ;

	int			mode ;	/* シナリオ実験用 */
	int			bomb_zone ;

	int			pl_dis ;	/* プレイヤーとの距離 */
	int			pl_dir ;	/* プレイヤーへの方向 */

	int			status ;
	int			notice ;	/* notice mode */

	TARGET		attack ;
	POWER_TARGET	power ;

	int			id ;
	short		heart_count ;
	short		heart_int ;
	FVECTOR		on_corp ;

	int			strmhandler ;
	int			strm ;
	u_short		quest_time ;
	char		mail_seed ;
	char		mail_time ;

	int			se_interval ;
	int			vc_end_proc ;
} Work ;


/*----------------------------------------------------------------*/
/* vox */
enum {
	VOX_MAN_PANIC_1,		//人質男性Ａ　「（悲鳴、パニックになっている）」			
	VOX_MAN_PANIC_2,		//人質男性Ｂ　「（悲鳴、パニックになっている）」			
	VOX_MAN_NO_EIMS_1,		//人質男性Ａ　「私はエイムズではない！」			
	VOX_MAN_NO_EIMS_2,		//人質男性Ｂ　「私はエイムズではない！」			
	VOX_MAN_NO_1,			//人質男性Ａ　「違う！」			
	VOX_MAN_NO_2,			//人質男性Ｂ　「違う！」			
	VOX_MAN_SAY_NO_1,		//人質男性Ａ　「違うと言っただろう！」			
	VOX_MAN_SAY_NO_2,		//人質男性Ｂ　「違うと言っただろう！」			
	VOX_WOM_PANIC_1,		//人質女性Ａ　「（悲鳴、パニックになっている）」			
	VOX_WOM_PANIC_2,		//人質女性Ｂ　「（悲鳴、パニックになっている）」			
	VOX_WOM_NO_1,			//人質女性Ａ　「違う！」			
	VOX_WOM_NO_2,			//人質女性Ｂ　「違う！」
	VOX_WOM_YORUNA_1,		//人質女性Ａ　「近寄らないで！」
	VOX_WOM_YORUNA_2,		//人質女性Ｂ　「近寄らないで！」
	VOX_WOM_HOKANO_1,		//人質女性Ａ　「私じゃゃなくて他の人にして！」
	VOX_WOM_HOKANO_2,		//人質女性Ｂ　「私じゃなくて他の人にして！」
	VOX_WOM_SAWARUNA_1,		//人質女性Ａ　「やめて！触らないで！！」
	VOX_WOM_SAWARUNA_2,		//人質女性Ｂ　「やめて！触らないで！！」
	VOX_WOM_JENEFA,			//人質女性Ｂ　「私はジェニファーよ」
	VOX_WOM_RINDA,			//人質女性Ｂ　「私はリンダ」
	VOX_WOM_CINDY,			//人質女性Ｂ　「私はシンディーよ」
	VOX_MEC_SENDMAIL,			//機械音　「メールを送信しました」
} ;

/*----------------------------------------------------------------*/
int	HSTG_CallVox( int vox ) ;
int	HSTG_GetHostageStatus( void ) ;
int	HSTG_SetHostage( Work *work) ;
int	HSTG_NoiseCheck( void ) ;
int	HSTG_MicQuestion( Work *work ) ;
void HSTG_ProcCallReaction( int name ) ;

