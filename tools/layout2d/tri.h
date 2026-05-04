#ifndef _tri_h_
#define _tri_h_

/* TRI 自動生成時のオプション */
#define TRIOPT_CM2      0x00000001   /* 生成に使用した .cm2 を消さない */

typedef struct _tri_tex {
  struct _tri_tex * prev;
  struct _tri_tex * next;

  char * tex_name;
} _tri_tex;

typedef struct _tri_info {
  struct _tri_info * prev;
  struct _tri_info * next;
  char * tri_name;

  _tri_tex * begin;
  _tri_tex * end;
} _tri_info;

int tri_Init(void);
int tri_AddTexture(char * tri_name, char * tex_name);
int tri_OutputTRI(char * sdir, char * odir, int opt);

#endif /* _tri_h_ */

