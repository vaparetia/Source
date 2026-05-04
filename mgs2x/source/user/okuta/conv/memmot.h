/*
    memmot.h
    モーションメモリ再生処理
    2001/05/09 Masafumi Okuta
    $Id: memmot.h,v 1.1.1.3 2002/11/19 11:47:49 Yoshizawa1 Exp $
*/

#ifndef __MEMMOT_H__
#define __MEMMOT_H__

typedef struct { // メモリモーション : データ
    int			mot_name;	// モーション名 
    int			mot_num;	// モーション数 
    int			model;		// モデル名 
    int			mot_flam;	// 総モーションフレーム 

    OBJECT*		body_mtbuff;	// モーションバッファ用 

    u_short*		m_ptr;		// モーションのポインタバッファ 
    u_short*		m_len;		// モーションの長さバッファ
    FVECTOR*		m_buff;		// モーションのバッファ
    FVECTOR*		m_step;		// モーションの移動量バッファ
    FVECTOR*		abs_rots;	// モーションの絶対回転量バッファ

    FVECTOR*		step_buff;	// 移動量
    u_short*		m_height;	// モーションの高さバッファ
} MEMMOT_DATA;

typedef struct { // メモリモーション : 制御
    int			current_mot;	// 再生しているモーション番号
    FVECTOR*		current_abs;	// 現在の絶対回転量
    int			nPlayFlag;	// 再生フラグ
    int			nLoop;		// ループ回数

    float		m_time;		// モーション再生時間
    float		m_time_base;	// モーション再生加算値

    int 		interp_count;	// 補間用カウンタ
    int 		interp_inc;	// 補間用インクリメント値
    float 		interp_time;	// 補間時間

    int			last_chk_time;	// 再生時間チェックのラスト
    
    int			reverse_flag;	// 反転フラグ

    // アジャスト
    int			adj_x;		// アジャスト用 X値
    int			adj_y;		// アジャスト用 Y値
    int			adj_turn_x;	// アジャスト用 目標 X値
    int			adj_turn_y;	// アジャスト用 目標 Y値

    // 高さ
    u_short		old_height;	// 前の高さ

    // 外部参照用
    OBJECT*		body;		// 対象オブジェクト
    MEMMOT_DATA*	pmmtData;	// メモリモーションデータ
} MEMMOT_CTRL ;

extern void KR_MemCopy( void *dst, void *src, int size, int num );

extern FVECTOR*	MEMMOT_GetAbsRot( MEMMOT_CTRL*);
extern void	MEMMOT_SetMotion( MEMMOT_CTRL*, int, int);
extern void 	MEMMOT_SetMotionSpeed( MEMMOT_CTRL*, float);
extern void 	MEMMOT_ActMotion2( MOTION_CONTROL*, DG_OBJS*, FVECTOR* );
extern int 	MEMMOT_CheckObject_IsEnd( MEMMOT_CTRL*, int);
extern void 	MEMMOT_MakeMotion( MEMMOT_CTRL*);
extern void 	MEMMOT_MakeMotionSkip( MEMMOT_CTRL*);
extern void 	MEMMOT_InitMotion( MEMMOT_CTRL*, MEMMOT_DATA*, OBJECT*, int);
extern int 	MEMMOT_MotionPreCalloc( MEMMOT_DATA*, int, int);
extern void 	MEMMOT_FreeWork(MEMMOT_DATA* );
extern int 	MEMMOT_CheckMotionTime( MEMMOT_CTRL* pmmCtrl, int time );

enum{
MEMMOT_PLAY_ACT,
MEMMOT_PLAY_END_PREV,
MEMMOT_PLAY_END,
};

#endif // end of __MEMMOT_H__
