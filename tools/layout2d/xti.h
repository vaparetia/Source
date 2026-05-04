#ifndef _xti_h_
#define _xti_h_

/* XTI 自動生成時のオプション */
#define XTIOPT_CM2      0x00000001   /* 生成に使用した .cm2 を消さない */

typedef struct _xti_tex {
  struct _xti_tex * prev;
  struct _xti_tex * next;

  char * tex_name;
} _xti_tex;

typedef struct _xti_info {
  struct _xti_info * prev;
  struct _xti_info * next;
  char * xti_name;

  _xti_tex * begin;
  _xti_tex * end;
} _xti_info;

int xti_Init(void);
int xti_AddTexture(char * xti_name, char * tex_name);
int xti_OutputXTI(char * sdir, char * odir, int opt);

#endif /* _xti_h_ */

