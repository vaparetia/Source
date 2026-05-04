//BP - Stub file
#ifndef __LIBIPU_H__
#define __LIBIPU_H__

#ifndef SYS_COMMON_TYPES
typedef unsigned int   u_int;
#endif

typedef struct {
   u_int d4madr; /* 停止したときの D4_MADR レジスタの値 */
   u_int d4tadr; /* 停止したときの D4_TADR レジスタの値 */
   u_int d4qwc;  /* 停止したときの D4_QWC レジスタの値 */
   u_int d4chcr; /* 停止したときの D4_CHCR レジスタの値 */
   u_int d3madr; /* 停止したときの D3_MADR レジスタの値 */
   u_int d3qwc;  /* 停止したときの D3_QWC レジスタの値 */
   u_int d3chcr; /* 停止したときの D3_CHCR レジスタの値 */
   u_int ipubp;  /* 停止したときの IPU_BP レジスタの値 */
   u_int ipuctrl;/* 停止したときの IPU_CTRL レジスタの値 */
} sceIpuDmaEnv;

#endif//__LIBIPU_H__
