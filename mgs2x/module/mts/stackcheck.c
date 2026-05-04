/*
	KCEJ DEBUG UTILITY for EE
		STACK CHECK MANAGER
	2000/05/18	K.Uehara
	$Id: stackcheck.c,v 1.5 2002/06/17 12:32:27 usr01475 Exp $
*/

#include <eekernel.h>
#include <eeregs.h>
#include <eestruct.h>
#include <libdma.h>
#include <stdio.h>
#include <stdarg.h>

#include "debugutil.h"

#ifdef MASTER
#define HANGUP()
#else
#define HANGUP()	( *( int * )1 = 0 )
#endif

static DEBUG_STACKCHECK_INFO *stackinfo_top = NULL;

void DEBUG_SetStackCheck( DEBUG_STACKCHECK_INFO *info, char *name, void *top, int size )
{
	DI();
	info->name = name;
	info->top = top;
	info->size = size;
	info->next = stackinfo_top;
	stackinfo_top = info;
	EI();
	{
		void *sp;
		unsigned int *p;

		/* 現在のスタックを取得 */
		asm( "move %0, $sp" : "=r"(sp) );
		if( top < sp && top + size > sp ){
			size = sp - top;
		}
		// 指定された領域を既定値で埋めつくす
		for( p = top; size > 0; size -= sizeof( int ) ){
			*( p ++ ) = DEBUG_STACK_INVALID_VALUE;
		}
	}
}

int DEBUG_get_stack_size( DEBUG_STACKCHECK_INFO *info )
{
	unsigned int *p;
	unsigned int *top, *bottom;

	top = ( unsigned int * )info->top;
	bottom = ( unsigned int * )( info->top + info->size );

	for( p = top; p < bottom; p++ ){
		if( *p != DEBUG_STACK_INVALID_VALUE ){
			return info->size - ( ( char * )p - ( char * )info->top );
		}
	}
	return -1;	// ありえないはず
}

DEBUG_STACKCHECK_INFO *DEBUG_ReleaseStack( void )
{
	DEBUG_STACKCHECK_INFO *p, *prev;
	char *sp;

	/* 現在のスタックを取得 */
	asm( "move %0, $sp" : "=r"(sp) );

	prev = ( DEBUG_STACKCHECK_INFO * )&stackinfo_top;

	DI();
	for( p = stackinfo_top; p != NULL; p = p->next ){
		if( p->top < sp && p->top + p->size > sp ){
			// ヒット
			if( *( int * )p->top != DEBUG_STACK_INVALID_VALUE ){
				// スタックオーバー
				printf( "%s: STACK OVER !!\n", p->name );
				HANGUP();
			}
			printf( "%s:stack used %d\n", p->name, DEBUG_get_stack_size( p ) );
			prev->next = p->next;
			EI();
			return p;
		}
		prev = p;
	}
	// どれにもヒットしなかった = STACKの値が異常
	printf( "%s: Wrong STACK !!\n", p->name );
	HANGUP();
	EI();
	return NULL;
}

static DEBUG_STACKCHECK_INFO *DEBUG_get_current_stack_info( void )
{
	DEBUG_STACKCHECK_INFO *p;
	char *sp;

	/* 現在のスタックを取得 */
	asm( "move %0, $sp" : "=r"(sp) );

	for( p = stackinfo_top; p != NULL; p = p->next ){
		if( p->top < sp && p->top + p->size > sp ){
			// HIT
			if( *( int * )p->top != DEBUG_STACK_INVALID_VALUE ){
				// スタックオーバー
				printf( "%s: STACK OVER !!\n", p->name );
				HANGUP();
			}
			return ( p );
		}
	}
	return NULL;
}

int  DEBUG_GetStackUseSize( void )
{
	DEBUG_STACKCHECK_INFO *p;
	char *sp;

	/* 現在のスタックを取得 */
	asm( "move %0, $sp" : "=r"(sp) );

	for( p = stackinfo_top; p != NULL; p = p->next ){
		if( p->top < sp && p->top + p->size > sp ){
			// HIT
			if( *( int * )p->top != DEBUG_STACK_INVALID_VALUE ){
				// スタックオーバー
				printf( "%s: STACK OVER !!\n", p->name );
				HANGUP();
			}
			return DEBUG_get_stack_size( p );
		}
	}

	if( ( p = DEBUG_get_current_stack_info() ) != NULL ){
		return DEBUG_get_stack_size( p );
	}
	// どれにもヒットしなかった = STACKの値が異常
	printf( "%s: Wrong STACK !!\n", p->name );
	HANGUP();
	return 0;
}

