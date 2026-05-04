/*
	DMATAG デバッグルーチン
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include	"libgv.h"
#include	"libgv.cnf"
#include	"libdg.h"
#include	"libdg.cnf"

#include	"def_dma.h"

typedef struct {
	unsigned int atr;
	unsigned int addr;
	unsigned int pad[ 2 ];
} DMATAG;

void DBG_DumpDmaTag( void *tag )
{
	DMATAG *tp;
	int qwc_count;
	int call_addr;
	int end_flag;

	tp = tag;

	call_addr = 0;
	qwc_count = 0;
	end_flag = 0;
	while( end_flag == 0 ){
		int id;
		int qwc;
		id = ( tp->atr >> 28 ) & 0x7;
		qwc = tp->atr & 0xFFFF;

		qwc_count += qwc + 1;

		switch( id ){
		  case 1:	// cnt
			tp += 1 + qwc;
			break;	
		  case 3:	// ref
		  case 4:	// refs
			tp ++;
			break;

		  case 5:	// call
			call_addr = tp->addr;
		  case 2:	// next
			tp = ( DMATAG * )tp->addr;
			break;

		  case 6:	// ret
			if( call_addr == 0 ){
				// 終り
				end_flag = 1;
			} else {
				tp->addr = ( DMATAG * )call_addr + 1;
				call_addr = 0;
			}
			break;

		  case 0:	// refe
		  case 7:	// end
			// 終り
			end_flag = 1;
			break;
		}
	}
	printf( "DMA TOTAL %d\n", qwc_count * 16 );
}
