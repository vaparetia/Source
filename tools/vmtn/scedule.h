#ifndef _scedule_h_
#define _scedule_h_

/* =========================================================================
 *                                   Macro
 * ========================================================================= */

#define DEF_POWER  0.8    /* デフォルトのパワー */
#define MIN_POWER  0.3    /* パワーがこの値未満の場合は、この値として扱う */
#define CLOSE_SPEED 60

/* =========================================================================
 *                                 Type define
 * ========================================================================= */

typedef struct _scedule {
  int      t;     /* 音素切り替わりのフレーム番号(1/100[sec]単位で指定) */
  char   * label; /* 音素ラベル                                         */

  double   power; /* 音声のパワー 0～1 の値をとる。
		     (現在は常にデフォルト値が入る。将来の拡張用)       */
} scScedule;


/* =========================================================================
 *                                Prototype
 * ========================================================================= */
void         scInit(void);
scScedule ** scReadScedule(char *fname);
scScedule ** scReadSceduleByStream(FILE *stream);
void         scReleaseScedule(scScedule **sc_list);
int          scSceduleCount(scScedule **sc_list);
scScedule ** scAdjustScedule(scScedule **sc_list);

#endif /* _scedule_h_ */
