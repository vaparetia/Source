/*
	scratch.h

	1999/09/13 K.Kano
	$Id: scratch.x,v 1.1.1.3 2002/11/19 11:42:58 Yoshizawa1 Exp $
*/


#ifndef _scratch_h_
#define _scratch_h_


/* スクラッチパッド管理関数群
   スクラッチパッドは、通常のメモリと違い、とても容量が小さく、
   また、一時記憶として使われるので、通常のメモリ管理関数とは違った管理法を
   用いています。
   具体的には、一番最後にAllocされたメモリを、一番先にFreeする管理法で、
   使う時にはそのことを考慮して下さい。*/

void ScratchpadManInit(void);

/* メモリをスクラッチパッドから確保します。
   確保できなかった場合は、GV_Mallocを呼び出します。
   確保されたメモリのアライメントは16byte境界になります。*/
void *ScratchpadAlloc(int size);

/* addrで指定されたメモリを解放します。
   addrは、フラグとして用いられているだけで、実際には最後にAllocされた
   メモリを解放します。addrは、最後にAllocされたメモリを指定して下さい。*/
void ScratchpadFree(void *addr);

/* メモリをスクラッチパッドから確保します。
   確保されたメモリのアライメントは16byte境界になります。
   ScratchpadAllocとの違いは、
   1)別領域を使用し、ScratchpadAlloc、ScratchpadFreeを干渉しない。
   2)GV_Mallocを呼び出さない。*/
void *ScratchpadAlloc2(int size);

/* 最後にAllocされたメモリを解放します。
   この関数は、ScratchpadAlloc2で確保された領域を解放します。*/
void ScratchpadFree2(void);


#endif
