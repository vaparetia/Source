#ifndef _objdef_h_
#define _objdef_h_

#include "token.h"
#include "label.h"

typedef struct defObject {
  struct defObject * prev;  /* 直前のオブジェクト管理構造体へのポインタ */
  struct defObject * next;  /* 直後のオブジェクト管理構造体へのポインタ */

  int   code;       /* オブジェクト名称の StrCode */
  int   id_label;
  int   id_attrib;
  int   id_vertex;
  int   id_parent;
} defObject;

typedef struct defObjLink {
  defObject * begin;
  defObject * end;
} defObjLink;

#ifndef _objdef_c_
#define EXT extern
#define INIT(args...)
#else
#define EXT
#define INIT(args...)  = args
#endif /* _objdef_c_ */

EXT  defObjLink  defLink INIT({ NULL, NULL });

#undef EXT
#undef INIT(_n...)

int defInitObjectDefine(void);
int defObjDefine(char * cmd, tknFILE * tkn, lblBlock * block, FILE * wfp);
int defGetObjVertexNum(int id);

#endif /* _objdef_h_ */
