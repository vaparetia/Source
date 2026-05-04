#ifndef _my_malloc_h_
#define _my_malloc_h_

/*
 * 環境によっては malloc() 系関数にバグがあるものがあるため、
 * ここでマクロを使用して置換する。
 */
#define FREE(_pt)         free((_pt))
#define MALLOC(_sz)       malloc((_sz))
#define CALLOC(_nm, _sz)  calloc((_nm), (_sz))
#define REALLOC(_pt, _sz) realloc((_pt), (_sz))

#endif /* _my_malloc_h_ */
