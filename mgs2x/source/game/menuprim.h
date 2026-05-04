/*
	menuprim.h
		メニュー用2Dプリミティブ管理ルーチン

	1999/08/26 K.Uehara
	$Id: menuprim.h,v 1.1.1.3 2002/11/19 11:41:53 Yoshizawa1 Exp $
*/

#ifndef __MENUPRIM_H__
#define __MENUPRIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
	メニュープリミティブのサイズ
		( ダブルバッファの片方のサイズ )
*/

#define MAX_MENU_PRIM_SIZE		( 32 * 1024 )

/*
	メニュープリミティブ管理構造体
*/

#if 0 //BP_PS2 def PSX2
/* PS2 */
typedef struct {
	void *ptr;
	void *top;
	void *bottom;
} MENU_PRIM;

MENU_PRIM *MENU_OpenPrim( void );
void MENU_ClosePrim( void );

static inline void *MENU_NEXT_PRIM( MENU_PRIM *prim, int size )
{
	void *ptr;

	ptr = prim->ptr;
#ifdef DEBUG
	if( (int)prim->ptr + size > (int)prim->bottom ){
//BP		printf( "!! PRIM OVER !!\n" );
		HANGUP();
	}
#endif
	prim->ptr = (char*)ptr + size;
	return ptr;
}

static inline void *MENU_NEXT_TAG( MENU_PRIM *prim, int size )
{
	void *ptr;

	if( ( u_int )prim->ptr & 15 ){
		prim->ptr = ( void * )NORM16( prim->ptr );
	}
	ptr = prim->ptr;
#ifdef DEBUG
	if( (int)prim->ptr + size > (int)prim->bottom ){
//BP		printf( "!! PRIM OVER !!\n" );
		HANGUP();
	}
#endif
	prim->ptr = (char*)ptr + size;
	return ptr;
}

static inline int MENU_GET_REST( MENU_PRIM *prim )
{
	return (int)prim->bottom - (int)prim->ptr;
}

#define MENU_NEW_PRIM( a, _p )	( a = MENU_NEXT_PRIM( (_p), sizeof( *(a) ) ) )
#define MENU_NEW_TAG( a, _p )	( a = MENU_NEXT_TAG( (_p), sizeof( *(a) ) ) )


#else
/* XBOX */
void *MENU_OpenPrim( void );
void MENU_ClosePrim( void* );

#endif


#ifdef __cplusplus
}
#endif

#endif
