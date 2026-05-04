#ifndef _view_h_
#define _view_h_

/* 表示状態定義シーケンスコマンド */
#define SEQ_STAT_MASK   0xf0000000

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
#define SEQ_ACT_MASK    0xc0000000

#define SEQ_ACT_END     0x00000000
#define SEQ_ACT_WAIT    0x40000000
#define SEQ_ACT_SET     0x80000000
#define SEQ_ACT_MORF    0xc0000000

#ifndef _view_h_
#define EXT   extern
#define INIT(_n...)
#else
#define EXT
#define INIT(_n...)   = _n
#endif

/* グローバル変数等はここで定義
 * EXT int  global_variable INIT(10);  などと書く。
 */




#undef EXT
#undef INIT(_n...)









#endif /* _view_h_ */
