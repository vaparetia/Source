/*******************************************************************************
 * msn - msn.h
 * ミッションズ
 * 2002/05/08 S.Yamashita
 * $Id: msn.h,v 1.2 2002/12/10 08:18:38 takaki Exp $
 */

#ifndef __INC_MSN__
#define __INC_MSN__

/*******************************************************************************
 * include
 */

#ifndef KP_WINDOWS
#include "../../../../scn/msn_stage_id2.h"
#else
#include "msn_stage_id2.h"
#endif

/*******************************************************************************
 * definitions and typedefs and structures
 */

/* ミッションタイプ */
enum {
	MSN_MISSION_VR = 0,
	MSN_MISSION_ALTERNATIVE,
	MSN_MISSION_MAX,
};

/* モードタイプ */
enum {
	MSN_MODE_SNEAKING = 0,
	MSN_MODE_WEAPON,
	MSN_MODE_FIRST_PERSON_VIEW,
	MSN_MODE_VARIETY,
	MSN_MODE_STREAKING,
	MSN_MODE_BOMB_DISPOSAL,
	MSN_MODE_ELIMINATE,
	MSN_MODE_HOLD_UP,
	MSN_MODE_PHOTOGRAPH,
	MSN_MODE_MAX,
};

/* 武器タイプ */
enum {
	MSN_WEAPON_NULL = 0,	// weapon 選択がないモードのときの便宜的な定義
	MSN_WEAPON_SNEAKING = 0,
	MSN_WEAPON_ELIMINATE_ALL,
	MSN_WEAPON_HANDGUN,
	MSN_WEAPON_ASSAULT_RIFLE,
	MSN_WEAPON_C4_CLAYMORE,
	MSN_WEAPON_GRENADE,
	MSN_WEAPON_PSG1,
	MSN_WEAPON_STINGER,
	MSN_WEAPON_NIKITA,
	MSN_WEAPON_HF_BLADE,
	MSN_WEAPON_MAX,
};

/* ２Ｄ状態 */
#define MSN_2DSTAT_WINDOW_PAUSE (0x00000001)
#define MSN_2DSTAT_QUICK_WINDOW (0x00000002)
#define MSN_2DSTAT_DEBUGMODE_ON (0x00000004)



/* セーブデータ関連 */
/*  1 - 2002.06.28 体験版 */
/*  2 - 2002.07.11 Substance - Escape 削除 */
/*  3 - 2002.07.23 Substance - アドレス変更 */
/*  4 - 2002.07.30 Substance - 完全に新しいフォーマットに変更(１ステージに１６バイトを使用) */
/*  5 - 2002.08.15 Substance - クリアコードのデータを完全に記録。バージョン情報に変更フラグ情報を追加 */

/* 旧バージョン */
#if 0
	#define MSN_DATA_VERSION   (3)
	#define MSN_HISCORE_SIZE   (sizeof(int) * MSN_STAGE_ID_MAX * 3)
	#define MSN_CLEARFLAG_SIZE ((MSN_STAGE_ID_MAX + 7) / 8)
	#define MSN_DATA_SIZE      (sizeof(int) + sizeof(int) + MSN_HISCORE_SIZE + MSN_CLEARFLAG_SIZE)
	#define MSN_OVERWRITE      (0x80000000)

	/* 各カウンター */
	#define _MSN_RETRY_COUNT    ((unsigned int  *)(((char *)GM_TankerPicture) + GM_TANKER_PICTURE_SIZE))
	#define _MSN_CONTINUE_COUNT ((unsigned int  *)_MSN_RETRY_COUNT + 1)
	/* セーブデータの各アドレス */
	#define _MSN_SAVE_DATA      ((unsigned int  *)_MSN_CONTINUE_COUNT + 1)
	#define _MSN_DATA_VERSION   ((unsigned int  *)_MSN_SAVE_DATA)
	#define _MSN_DATA_SIZE      ((unsigned int  *)_MSN_DATA_VERSION + 1)
	#define _MSN_HISCORE        ((unsigned int  *)_MSN_DATA_SIZE + 1)
	#define _MSN_CLEARFLAG      ((unsigned char *)(_MSN_HISCORE + (MSN_STAGE_ID_MAX * 3)))
	/* その他変数 */
	#define _MSN_2DSTATUS       ((unsigned int  *)(_MSN_CLEARFLAG + (((MSN_CLEARFLAG_SIZE + 3) / 4) * 4)))
	#define _MSN_STAGE_ID       ((         int  *)_MSN_2DSTATUS + 1)
#endif



/* 新バージョン */
/*
[１ステージのデータ] - １６バイト
	A. １位スコア			０～９９９９９９	　２０ bit
	B. ２位スコア			０～９９９９９９	　２０ bit
	C. ３位スコア			０～９９９９９９	　２０ bit

	D. １位上書きフラグ							　　１ bit
	E. ２位上書きフラグ							　　１ bit
	F. ３位上書きフラグ							　　１ bit
	G. クリアフラグ								　　１ bit

	H. １位 タイムスコア	０～９９９９９		　１７ bit
	I. １位 残弾数スコア	０～９９９９９		　１７ bit
	J. １位 隠密			０～２				　　２ bit
	K. １位 不殺			０～１				　　１ bit
	L. ランダムシード		０～３１			　　５ bit
	------------------------------------------------------
												１０１ bit

	メモリ内部
			---------------------------------------
	0x00	| G--KJJHI D---AAAA AAAAAAAA AAAAAAAA |
			---------------------------------------
			---------------------------------------
	0x04	| ---LLLLL E---BBBB BBBBBBBB BBBBBBBB |
			---------------------------------------
			---------------------------------------
	0x08	| -------- F---CCCC CCCCCCCC CCCCCCCC |
			---------------------------------------
			---------------------------------------
	0x0c	| HHHHHHHH HHHHHHHH IIIIIIII IIIIIIII |
			---------------------------------------
*/
#define MSN_SAVE_DATA_VERSION (5)
#define MSN_STAGE_DATA_SIZE   ((sizeof(int) * 4) * MSN_STAGE_ID_MAX)
#define MSN_SAVE_DATA_SIZE    ((sizeof(int) * 4) + MSN_STAGE_DATA_SIZE)

#define MSN_CLEARFLAG         (0x80000000)
#define MSN_NOKILLMASK        (0x10000000)
#define MSN_SNEAKINGMASK      (0x0c000000)
#define MSN_TIMEMASK_2        (0x02000000)
#define MSN_BULLETSMASK_2     (0x01000000)
#define MSN_RNDSEEDMASK       (0x1f000000)
#define MSN_OVERWRITEFLAG     (0x00800000)
#define MSN_SCOREMASK         (0x000fffff)
#define MSN_TIMEMASK_1        (0xffff0000)
#define MSN_BULLETSMASK_1     (0x0000ffff)

#define MSN_DFLAG_MODIFIED_DATA (0x00000001)
#define MSN_DFLAG_FPV_PLAYED    (0x00000002)
#define MSN_DFLAG_ALL_OPEN      (0x00000004)

/* セーブデータの各アドレス */
#define _MSN_SAVE_DATA         ((unsigned int  *)(((char *)GM_TankerPicture) + GM_TANKER_PICTURE_SIZE))		/* セーブデータの先頭アドレス */
#define _MSN_SAVE_DATA_VERSION ((unsigned int  *)_MSN_SAVE_DATA)											/* セーブデータバージョン */
#define _MSN_SAVE_DATA_SIZE    ((unsigned int  *)_MSN_SAVE_DATA_VERSION + 1)								/* セーブデータサイズ */
#define _MSN_SAVE_DATA_FLAG    ((unsigned int  *)_MSN_SAVE_DATA_SIZE + 1)									/* セーブデータフラグ */
#define _MSN_SAVE_DATA_PAD     ((unsigned int  *)_MSN_SAVE_DATA_FLAG + 1)									/* パディング */
#define _MSN_STAGE_DATA        ((unsigned int  *)_MSN_SAVE_DATA_PAD + 1)									/* 各ステージデータ */

/* その他変数 */
#define _MSN_RETRY_COUNT    ((unsigned int  *)(((char *)_MSN_STAGE_DATA) + MSN_STAGE_DATA_SIZE))
#define _MSN_CONTINUE_COUNT ((unsigned int  *)_MSN_RETRY_COUNT + 1)
#define _MSN_2DSTATUS       ((unsigned int  *)_MSN_CONTINUE_COUNT + 1)
#define _MSN_STAGE_ID       ((         int  *)_MSN_2DSTATUS + 1)

/*******************************************************************************
 * macros
 */

/* 旧バージョン */
#if 0
	/* 指定ステージのハイスコア保管位置へアクセス（int） */
	#define MSN_HISCORE(stage, ranking) (*(_MSN_HISCORE + ((stage) * 3) + ((ranking) - 1)))
	#define MSN_HISCORE2(stage, ranking) (*(_MSN_HISCORE + ((stage) * 3) + ((ranking) - 1)) & ~MSN_OVERWRITE)

	/* 指定ステージのクリアフラグの値(0 か 1)を返す */
	#define MSN_GET_CLEARFLAG(stage) (((*(_MSN_CLEARFLAG + ((stage) / 8))) & (((unsigned char)0x01) << ((stage) % 8))) != 0)

	/* 指定ステージのクリアフラグのビットに値1を設定する */
	#define MSN_SET_CLEARFLAG(stage) ((*(_MSN_CLEARFLAG + ((stage) / 8))) |= (((unsigned char)0x01) << ((stage) % 8)))
	///* 指定ステージのクリアフラグのビットに値(0 か 1)を設定する */
	//#define MSN_SET_CLEARFLAG(stage, flag) ((*(_MSN_CLEARFLAG + ((stage) / 8))) &= ~(((~((unsigned char)(flag))) << 7) >> (7 - ((stage) % 8))))
#endif



/* 新バージョン */
/* 指定ステージのステージデータへのアクセス（int） */
#define MSN_STAGE_DATA(stage) ((unsigned int  *)(_MSN_STAGE_DATA + (4 * stage)))

/* 指定ステージのハイスコアの設定・取得 */
#define MSN_SET_HISCORE(stage, ranking, hiscore)  { (*(MSN_STAGE_DATA(stage) + ((ranking) - 1))) &= ~(MSN_SCOREMASK|MSN_OVERWRITEFLAG) ; (*(MSN_STAGE_DATA(stage) + ((ranking) - 1))) |= ((hiscore) & (MSN_SCOREMASK|MSN_OVERWRITEFLAG)) ; }
#define MSN_SET_HISCORE2(stage, ranking, hiscore) { (*(MSN_STAGE_DATA(stage) + ((ranking) - 1))) &= ~(MSN_SCOREMASK                  ) ; (*(MSN_STAGE_DATA(stage) + ((ranking) - 1))) |= ((hiscore) & (MSN_SCOREMASK                  )) ; }
#define MSN_GET_HISCORE(stage, ranking)           ( (*(MSN_STAGE_DATA(stage) + ((ranking) - 1))) &   (MSN_SCOREMASK|MSN_OVERWRITEFLAG) )
#define MSN_GET_HISCORE2(stage, ranking)          ( (*(MSN_STAGE_DATA(stage) + ((ranking) - 1))) &   (MSN_SCOREMASK                  ) )

/* 指定ステージのクリアフラグの値(0 か 1)を返す */
#define MSN_SET_CLEARFLAG(stage)  ((*MSN_STAGE_DATA(stage)) |= (MSN_CLEARFLAG))
#define MSN_GET_CLEARFLAG(stage) (((*MSN_STAGE_DATA(stage)) &  (MSN_CLEARFLAG)) >> 31)

// Transfarring defines
#define TRANSFARRING_MSN_STAGE_DATA(stage, vrScoreDataBuffer) (((unsigned int  *)((vrScoreDataBuffer)) + 4 + (4 * stage))) // Skips past the first 4 variables
#define TRANSFARRING_MSN_GET_HISCORE2(stage, ranking, vrScoreDataBuffer)          ( (*(TRANSFARRING_MSN_STAGE_DATA(stage, vrScoreDataBuffer) + ((ranking) - 1))) &   (MSN_SCOREMASK                  ) )
#define TRANSFARRING_MSN_GET_CLEARFLAG(stage, vrScoreDataBuffer) (((*TRANSFARRING_MSN_STAGE_DATA(stage, vrScoreDataBuffer)) &  (MSN_CLEARFLAG)) >> 31)

#define MSN_RETRY_COUNT    (*_MSN_RETRY_COUNT)
#define MSN_CONTINUE_COUNT (*_MSN_CONTINUE_COUNT)
#define MSN_2DSTATUS       (*_MSN_2DSTATUS)
#define MSN_STAGE_ID       (*_MSN_STAGE_ID)

/*******************************************************************************
 * functions
 */

void Msn_GetCurrentStageInfo(char *mission, char *mode, char *weapon, char *level, char *player);
void Msn_GetStageInfo(int stage, char *mission, char *mode, char *weapon, char *level, char *player);

void Msn_SetDefaultData(void);

int  Msn_SetNewScore(int stage, int score);
int  Msn_GetClearLevel(int stage);
int  Msn_GetStageCount(char mission, char mode, char weapon, int player, int *cleared);
int  Transfarring_Msn_GetStageCount(const char* vrScoreDataBuffer, char mission, char mode, char weapon, int player, int *cleared);

int  Msn_GetPlayer(int mode_player, int count);
int  Msn_GetPlayerNum(int mode_player, int player);

void Msn_Set1stClearData(int stage, int time_score, int bullet_score, int sneaking_score, int no_kill_score);
void Msn_Get1stClearData(int stage, int *time_score, int *bullet_score, int *sneaking_score, int *no_kill_score, int *seed);
int  Msn_GetClearCode(int stage, unsigned char *clear_code, int clear_code_length);

int  Msn_GetRetryCount(void);
int  Msn_GetContinueCount(void);
void Msn_ResetQuickWindow(void);
void SetVrStageId(void);
int  Msn_GetClearLevel2(void);
int  Msn_GetHiScore_Scn(void);
int  Msn_GetOverwriteFlag_Scn(void);

/*******************************************************************************
 */

#endif	/* __INC_MSN__ */
