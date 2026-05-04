#ifndef _lang_env_h_
#define _lang_env_h_

#include "main.h"
#include "lip.h"

/*
 * 音韻記号と音素 ID の対応表用構造体
 */
typedef struct {
  char * label;
  int    phone_id;
} ConvTable;


/*
 * 言語依存環境をまとめる
 */
typedef struct _lang_env {
  ConvTable * table;              /* 音韻記号と音素 ID の対応表ポインタ */
  Seq ** (*adjust_func)(Seq **);  /* 連続音素の分割関数ポインタ         */
} LangENV;

LangENV * envGetLanguageEnvironment(LANG lang);

#endif /* _lang_env_h_ */
