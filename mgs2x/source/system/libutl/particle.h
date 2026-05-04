/*
	particle.h
		パーティクル汎用制御ルーチン
		データ構造

	1999/08/26 K.Kano
	$Id: particle.h,v 1.1.1.3 2002/11/19 11:42:58 Yoshizawa1 Exp $
*/

#ifndef _particle_h_
#define _particle_h_


enum {
    PARTICLE_BOUNDING_TYPE_ERASE=0,
    PARTICLE_BOUNDING_TYPE_OVERLAP,
    PARTICLE_BOUNDING_TYPE_REFLECT,
};

enum {
    PARTICLE_DISP_TYPE_SPR=0,
    PARTICLE_DISP_TYPE_LINE,
    PARTICLE_DISP_TYPE_POLY,
};
    
typedef ALIGN16_DECL(struct) {
    /* 現在の物理計算情報 */
    /* 現在の表示の大きさは、posのvwに入れる */
    FVECTOR pos;
    FVECTOR old_pos;
    FVECTOR v;
    FVECTOR dv;

    /* 残り生存時間 */
    int count;

    /* ポリゴンを表示する時に使うカウンタ */
    int poly_count;

    /* ダミー */
    int pad2,pad3;
} PARTICLE_PARAMETER ;

typedef ALIGN16_DECL(struct) _CONTROL_PARTICLES {
    /* パーティクルの総数 */
    int size;

    /* 表示範囲を越えた場合の処理 */
    int bounding_type;
    /* 消去、オーバーラップ、反射 */

    /* 表示タイプ */
    int display_type;

    /* 表示の大きさの初期値 */
    float display_size;

    /* 表示範囲 */
    FVECTOR bounding_box[4];

    /* グーローシェーディングをかけるかどうかのフラグ */
    int gour_flag;

    /* パーティクルパラメータの配列 */
    PARTICLE_PARAMETER *partp;

    /* パーティクルの位置情報 */
    FVECTOR *pos;

    /* 表示プリミティブ */
    void *prim;

    /* 処理内に埋め込む関数へのアドレス */
    void (*CPU_OriginalFunc)(struct _CONTROL_PARTICLES *cparts,PARTICLE_PARAMETER *partp,
			     FVECTOR *pos,void *packet,int size);

#if 0 /* 本当はこちらが正しい */
    DG_PRIM *prim;

    void (*CPU_OriginalFunc)(struct _CONTROL_PARTICLES *cparts,PARTICLE_PARAMETER *partp,
			     FVECTOR *pos,DG_PRIM_PACKET *packet,int size);
#endif

} CONTROL_PARTICLES ;


#endif
