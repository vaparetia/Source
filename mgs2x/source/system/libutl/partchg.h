/*
	partchg.h
		人間のモデルの一部すり替え
		＋頂点アニメーション

	2000/03/28 K.Kano
	$Id: partchg.h,v 1.1.1.3 2002/11/19 11:42:58 Yoshizawa1 Exp $
*/


#ifndef _partchg_h_
#define _partchg_h_


typedef struct {
    DG_OBJS *human;
    int index;

    DG_OBJS *part;

    VERTEX_ANIME_WORK **va;
    int n_patterns;

    int disp_enable;
} PARTCHANGE_WORK;


/* 初期化 */
PARTCHANGE_WORK *InitPartChange(DG_OBJS *human,int index,DG_DEF *def,
				CV2_DEF **cv2def,int n_patterns);

/* 終了処理 */
void ExitPartChange(PARTCHANGE_WORK *work);

/* 通常処理 */
void MovePartChange(PARTCHANGE_WORK *work);

/* アニメーションの目標パターン番号とフレームカウント数を指定する */
void SetParamForPartChange(PARTCHANGE_WORK *work,int patten_num,int count);

typedef struct {
	int		length ;
	int		n_joints ;
	FVECTOR		*move ;
	FVECTOR		*rots ;
	FVECTOR		*trans ;
} RMT_DATA ;

typedef struct {
	DG_OBJS *human;
	int index;
	DG_EVMOBJ *evmobj;

	RMT_DATA mtn;
	int frame;
	int mtn_enable;

	int disp_enable;
} EVMPARTCHANGE_WORK;


EVMPARTCHANGE_WORK *InitEvmPartChange(DG_OBJS *human,int index,EVM_DEF *evm);
void ExitEvmPartChange(EVMPARTCHANGE_WORK *work);
void MoveEvmPartChange(EVMPARTCHANGE_WORK *work);
void SetMtnForEvmPartChange(EVMPARTCHANGE_WORK *work,int name_id);


#endif
