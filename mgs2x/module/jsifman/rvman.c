/*
	KCEJ SIF MANAGER / 返り値マネージャ
		sifcmd, sifrpc のサービス関数群
		ソースコードそのものはEE,IOP共通

	1999/11/18	K.Uehara
	$Id: rvman.c,v 1.6 2000/06/21 12:45:59 usr01475 Exp $
*/


#include <stdio.h>
#ifdef IOP
#include <kernel.h>
#endif
#ifdef EE
#include <eekernel.h>
#endif
#include <sif.h>
#include <sifcmd.h>

#include "common.h"
#include "jsifman.h"

/* ---------------------------------------------------------------------- */
/*
	返り値マネージャ
*/

#define MAX_SIF_RV_QUEUE 8

static SIF_RV_QUEUE SIF_RV_Queue[ MAX_SIF_RV_QUEUE ] __attribute__((aligned(64)));
static int SIF_RV_Queue_id;

#define RV_GET_ID( a )			( (a) & ~SIF_RV_VALFLAG )
#define RV_SET_VALFLAG( a )		( a |= SIF_RV_VALFLAG )

static void rv_man_callback( SIF_CALLBACK_PARAM *hd, SIF_RV_QUEUE *quetop )
{
	switch( hd->fno ){
	  case SIF_RV_RETURN_VALUE:
		{
			SIF_RV_QUEUE *q, *queue;
			int id, i;

			q = ( SIF_RV_QUEUE * )hd->data;

			queue = quetop;
			id = RV_GET_ID( q->id );

			for( i = MAX_SIF_RV_QUEUE; i > 0; i-- ){
				if( id == queue->id ){
					*queue = *q;
					break;
				}
				queue ++;
			}
		}
		break;
	  case SIF_OTHER_MEM_GET:
		{
			SIF_RV_QUEUE *q;
			void *src, *dst;
			int size;

			q = ( SIF_RV_QUEUE * )hd->data;

			RV_SET_VALFLAG( q->id );

			dst = ( void * )q->data1;
			src = ( void * )q->data2;
			size = q->data3;

			i_sif_send_packet_and_data( SIF_RVMAN, SIF_RV_RETURN_VALUE
										, q, sizeof( SIF_RV_QUEUE )
										, dst, src, size );
		}
		break;
	}
#ifdef EE
	ExitHandler();
#endif
}

static int init_flag = 0;

int sif_init_rv_man( void )
{
	int i;
	FUNC_INIT();

	if( init_flag ) return 0;
	init_flag = 1;

	for( i = 0; i < MAX_SIF_RV_QUEUE; i++ ){
		SIF_RV_Queue[ i ].id = 0;
	}
	SIF_RV_Queue_id = 1;

	sif_set_callback_func( SIF_RVMAN, rv_man_callback, SIF_RV_Queue );

	FUNC_EXIT();

	return 0;
}

SIF_RV_QUEUE *sif_rv_get_queue( void )
{
	int i;
	SIF_RV_QUEUE *rp;

	FUNC_INIT();

	rp = SIF_RV_Queue;
	for( i = MAX_SIF_RV_QUEUE; i > 0; i-- ){
		if( rp->id == 0 ){
			rp->id = SIF_RV_Queue_id;
			SIF_RV_Queue_id = ( SIF_RV_Queue_id + 1 ) & ~SIF_RV_VALFLAG;
			if( SIF_RV_Queue_id == 0 ) SIF_RV_Queue_id = 1;
			FUNC_EXIT();
			return rp;
		}
		rp++;
	}
	FUNC_EXIT();
	return NULL;
}

int sif_rv_call_func( int id, int fno, SIF_RV_QUEUE *queue )
{
	return sif_send_packet( id, fno, queue, sizeof( SIF_RV_QUEUE ) );
}

int sif_rv_return_value( SIF_RV_QUEUE *queue )
{
	RV_SET_VALFLAG( queue->id );
	return sif_send_packet( SIF_RVMAN, SIF_RV_RETURN_VALUE
							, queue, sizeof( SIF_RV_QUEUE ) );
}

int i_sif_rv_return_value( SIF_RV_QUEUE *queue )
{
	RV_SET_VALFLAG( queue->id );
	return i_sif_send_packet( SIF_RVMAN, SIF_RV_RETURN_VALUE
							, queue, sizeof( SIF_RV_QUEUE ) );
}

void sif_rv_release_queue( SIF_RV_QUEUE *queue )
{
	queue->id = 0;
}

SIF_RV_QUEUE *sif_get_mem( void *dst, void *src, int size )
{
	SIF_RV_QUEUE *rv;

	if( ( rv = sif_rv_get_queue() ) != NULL ){
		rv->data1 = ( int )dst;
		rv->data2 = ( int )src;
		rv->data3 = size;
		sif_rv_call_func( SIF_RVMAN, SIF_OTHER_MEM_GET, rv );
	}
	return rv;
}
