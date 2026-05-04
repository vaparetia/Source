//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	scratch.c

	1999/09/13 K.Kano
	$Id: scratch.c,v 1.1.1.3 2002/11/19 11:42:58 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <sys/types.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"


#define SCRPAD_SIZE	0x4000
#define SCRPADMAN_ARRAYSIZE	16
#define SCRPADMAN_ARRAYSIZE2	16


/* スクラッチパッド管理関数群
   スクラッチパッドは、通常のメモリと違い、とても容量が小さく、
   また、一時記憶として使われるので、通常のメモリ管理関数とは違った管理法を
   用いています。
   具体的には、一番最後にAllocされたメモリを、一番先にFreeする管理法で、
   使う時にはそのことを考慮して下さい。*/

static void *scrpad_next;
static int scrpad_top,scrpad_bottom;
static void *scrpad_limit;
static void *scrpadman_array[SCRPADMAN_ARRAYSIZE];
static void *scrpadman_array2[SCRPADMAN_ARRAYSIZE2];

void ScratchpadManInit(void)
{
    scrpad_next=SCRPAD_ADDR;
    scrpad_limit=(u_char*)SCRPAD_ADDR+SCRPAD_SIZE;
    scrpad_top=0;
    scrpad_bottom=0;
}

/* メモリをスクラッチパッドから確保します。
   確保できなかった場合は、GV_Mallocを呼び出します。
   確保されたメモリのアライメントは16byte境界になります。*/
void *ScratchpadAlloc(int size)
{
    int size128=(size+16-1) & ~(16-1);

    if((unsigned int)scrpad_next+size128>(unsigned int)scrpad_limit){

#ifdef DEBUG
	printf("Scratchpad Overflow : Size\n");
#endif

	return GV_Malloc(size);
    }
    else if(scrpad_top>=SCRPADMAN_ARRAYSIZE){

#ifdef DEBUG
	printf("Scratchpad Overflow : Array\n");
#endif

	return GV_Malloc(size);
    }
    else{
	void *ans=scrpad_next;
	scrpadman_array[scrpad_top]=scrpad_next;
	scrpad_next=(void *)((int)scrpad_next+size128);
	scrpad_top++;
	return ans;
    }
}

/* addrで指定されたメモリを解放します。
   addrは、フラグとして用いられているだけで、実際には最後にAllocされた
   メモリを解放します。addrは、最後にAllocされたメモリを指定して下さい。*/
void ScratchpadFree(void *addr)
{
    if(((int)addr & 0xf0000000)==((int)SCRPAD_ADDR & 0xf0000000)){
	if(scrpad_top>0){
	    scrpad_top--;
	    scrpad_next=scrpadman_array[scrpad_top];
	}
#ifdef DEBUG
	else{
	    printf("Scratchpad Overfree\n");
	}
#endif
    }
    else{
	GV_Free(addr);
    }
}

/* メモリをスクラッチパッドから確保します。
   確保されたメモリのアライメントは16byte境界になります。
   ScratchpadAllocとの違いは、
   1)別領域を使用し、ScratchpadAlloc、ScratchpadFreeを干渉しない。
   2)GV_Mallocを呼び出さない。*/
void *ScratchpadAlloc2(int size)
{
    int size128=(size+16-1) & ~(16-1);

    if((unsigned int)scrpad_next>(unsigned int)scrpad_limit-size128){

#ifdef DEBUG
	printf("Scratchpad Overflow : Size\n");
#endif

	return NULL;
    }
    else if(scrpad_bottom>=SCRPADMAN_ARRAYSIZE){

#ifdef DEBUG
	printf("Scratchpad Overflow : Array\n");
#endif

	return NULL;
    }
    else{
	scrpadman_array2[scrpad_bottom]=scrpad_limit;
	scrpad_limit=(void *)((int)scrpad_limit-size128);
	scrpad_bottom++;
	return scrpad_limit;
    }
}

/* 最後にAllocされたメモリを解放します。
   この関数は、ScratchpadAlloc2で確保された領域を解放します。*/
void ScratchpadFree2(void)
{
    if(scrpad_bottom>0){
	scrpad_bottom--;
	scrpad_limit=scrpadman_array2[scrpad_bottom];
    }
#ifdef DEBUG
    else{
	printf("Scratchpad Overfree\n");
    }
#endif
}
