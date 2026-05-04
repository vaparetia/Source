#ifndef _database_h_
#define _database_h_

#include "analyze.h"

#define DB_VEC_DIM  24   /* 特徴ベクトルの次数 */

#define DEFAULT_DBASE  "voice_ref.db"  /* デフォルトのデータベース名 */

typedef struct _db_phonome {
  char   * label;
  vGroup   group;              /* その音素が含まれる、
				  もっとも高水準の音素群グループ
				  水準の高低は、以下のようになる。
				  W > V
				  D > C > V
				  F > U                            */

  double   vec[ DB_VEC_DIM ];  /* 特徴ベクトル */
 
  
} dbPhonome;




int dbLoadDatabase(char *fname);

#endif /* _database_h_ */
