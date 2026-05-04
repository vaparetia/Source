#ifndef _mgs2_h_
#define _mgs2_h_

/* バージョン値計算マクロ */
#define VER_CALC(_mj, _mn, _pt)   (((long)(_mj) << 8) + ((long)(_mn) << 16) + ((long)(_pt) << 24))

/* 現在のデータ形式バージョン */
#define VERSION VER_CALC(0,1,2)

#define POS_PACK(fx, fy) ((((unsigned long)short_float(fy)) << 16) | short_float(fx))

/* 表示状態定義シーケンスコマンド */
#define SEQ_STAT_END    0x00000000
#define SEQ_STAT_DISP   0x10000000
#define SEQ_STAT_TEX    0x20000000
#define SEQ_STAT_VERTEX 0x30000000
#define SEQ_STAT_RGBA   0x40000000
#define SEQ_STAT_SIZE   0x50000000
#define SEQ_STAT_UV     0x60000000
#define SEQ_STAT_TXSIZ  0x70000000
#define SEQ_STAT_ALPHA  0x80000000
#define SEQ_STAT_PRI    0x90000000
#define SEQ_STAT_CENTER 0xa0000000
#define SEQ_STAT_ANGLE  0xb0000000
#define SEQ_STAT_MAGNI  0xc0000000
#define SEQ_STAT_PARTUV 0xd0000000
#define SEQ_STAT_PTXSIZ 0xe0000000


/* アクショントラックシーケンスコマンド */
#define SEQ_ACT_END     0x00000000
#define SEQ_ACT_WAIT    0x40000000
#define SEQ_ACT_SET     0x80000000
#define SEQ_ACT_MORF    0xc0000000
#define SEQ_ACT_SIGN    0x10000000

#define PRI_MASK        7

unsigned short short_float(float f);
int mgs2_Output2D(sprLayout * layout, FILE * wfp);
int mgs2_BuildTRI(sprLayout * layout,
		  char * search_dir, char * out_dir, int opt);
#endif /* _mgs2_h_ */
