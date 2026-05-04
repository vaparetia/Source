#ifndef _objact_h_
#define _objact_h_

typedef enum {
  STEP_WAIT,
  STEP_SET,
  STEP_MORF
} actMode;

typedef struct actStep {
  struct actStep * prev;
  struct actStep * next;

  actMode mode;

  int    st_time;  /* 時間(wait, morf で使用) */
  char * st_name;  /* 状態(set, morf で使用)  */
} actStep;


typedef struct actTrack {
  struct actTrack * prev;
  struct actTrack * next;

  char            * trk_name;  /* トラック名ラベル   */

  actStep         * begin;
  actStep         * end;
  
} actTrack;

typedef struct actAction {
  struct actAction * prev;
  struct actAction * next;

  char            * act_label; /* アクション名ラベル */

  actTrack * begin;
  actTrack * end;
  
} actAction;


typedef struct actObjLink {
  actAction * begin;
  actAction * end;
  actAction * now;
} actObjLink;

#ifndef _objact_c_
#define EXT extern
#define INIT(_n...)
#else
#define EXT
#define INIT(_n...)  = _n
#endif /* _objact_c_ */

EXT actObjLink actLink INIT({NULL, NULL, NULL});

#undef EXT
#undef INIT(_n...)

int  actInitObjectAction(void);
int  actDefAction(char * cmd, tknFILE * tkn, lblBlock * block, FILE * wfp);

#endif /* _objact_h_ */
