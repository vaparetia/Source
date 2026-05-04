#ifndef _output_h_
#define _output_h_

/* バージョン値計算マクロ */
#define VER_CALC(_mj, _mn, _pt)   (((long)(_mj) << 8) + ((long)(_mn) << 16) + ((long)(_pt) << 24))

/* 現在のデータ形式バージョン */
#define VERSION VER_CALC(0,1,0)

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


/* アクショントラックシーケンスコマンド */
#define SEQ_ACT_END     0x00000000
#define SEQ_ACT_WAIT    0x40000000
#define SEQ_ACT_SET     0x80000000
#define SEQ_ACT_MORF    0xc0000000

#ifndef _output_c_
#define EXT extern
#define INIT(_n...)
#else
#define EXT
#define INIT(_n...)  = _n
#endif /* _output_c_ */




#undef EXT
#undef INIT(_n...)

int outInitOutputModule(void);
int outOutputDefineSection(FILE * wfp);
int outOutputStatusSection(FILE * wfp);
int outOutputActionSection(FILE * wfp);
int outOutputMain(FILE * wfp);

#endif /* _output_h_ */
