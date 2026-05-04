//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	メモリ管理ライブラリ

	1999/03/18 K.Uehara
	$Id: memory.c,v 1.6 2002/12/09 13:29:17 takaki Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

//#include	"mts.h"

#include	"libgv.h"
#include	"libgv.cnf"

#if 1 //BP_PS2
#define excep_printf printf
#else
int excep_printf( const char *format, ... );
#endif

#ifdef	PSX2
typedef char byte;
#endif

#define MINIMUM_SIZE	32	/* これ以下のサイズのメモリは前のブロックに統合 */

#ifdef DEBUG_MODE

//#define ALLOC_FILL_CHECK

//#define MEMFILL_CHECK	1

#if defined( ALLOC_FILL_CHECK ) || defined( MEMFILL_CHECK )
#define FILLDATA		I64(0xABCDEF01ABCDEF01)
#ifdef KP_XBOX
#include <XMMIntrin.h>
#endif
static void Fill_Memory( void *ptr, int size )
{
	int i;
	u_long64 *p;

	p = ptr;
	for( i = size; i > 0; i -= sizeof( u_long64 ) ){
#ifdef KP_XBOX
		// ADD M.Kobayashi 2002/07/23
		// これが無いと何故かサウンドが止まる
		// キャッシュコントローラと APU もしくは DSOUND 間の
		// 不整合バグ？ ストアバッファが埋まるとまずい？
		_mm_prefetch( p, _MM_HINT_NTA );
#endif		
		*( p++ ) = FILLDATA;
	}
}
#endif

#ifdef MEMFILL_CHECK
static void MemoryCheck( void *ptr, int size )
{
	int i;
	u_long64 *p;

	p = ptr;
	for( i = size; i > 0; i -= sizeof( u_long64 ) ){
		if( *p != FILLDATA ){
			printf( "MEMCHECK ERROR %X [ %x:%lx ]\n", ptr, p, *p );
			HANGUP();
		}
		p++;
	}
}
#endif	// MEMFILL_CHECK
#endif	// DEBUG_MODE

/*
	メモリブロックの構造

	----
	MEM_TAG tag;
	data[]
	----
	MEM_TAG tag;
	:(offset byte>sizeof(int aligndata))
	int aligndata;	//	( STAGE_ALIGN | offset )
	data[]
*/


/*
  メモリブロックタグ
	static used の場合はNULL
	dymanic の場合 ptr には参照元のポインタのアドレスが入る
	free block の場合 ptr には次の free blockのポインタが入る
	size の最上位ビットが立っていればフリー

	delayed_freeなら、そのリンクリスト
*/

typedef struct _mem_tag {
#ifdef KP_WINDOWS
	int	line;
	int	pad[3];
#endif
	struct _mem_tag *prev;
	struct _mem_tag *next;
	void *ptr;
	int size;
} MEM_TAG;

#define STATE_FREE	0x80000000	// FREE BLOCK
#define STATE_ALIGN	0x40000000	// アライメントにそろえている
#define STATE_DELAY	0x20000000	// DELAYED FREE予約
#define STATE_LOAD	0x10000000	// ロードされたデータ(デバッグ用)
#define STATE_ACTOR	0x08000000  // ACTOR用メモリ
#define STATE_MASK	0xf8000000

#define IS_FREE( _t )	( (_t)->size & STATE_FREE )

#define SET_FREE( _t )	( (_t)->size |= STATE_FREE )
#define SET_USED( _t )	( (_t)->size &= ~(STATE_FREE|STATE_DELAY) )

#define SET_DELAYED_FREE( _t )	( (_t)->size |= STATE_DELAY )
#define IS_DELAYED_FREE( _t )	( (_t)->size & STATE_DELAY )

#define SIZE( _t )		( (_t)->size & ~STATE_MASK )

/*
  メモリ構造管理構造体
  base.sizeにtype情報。
*/

typedef struct _mem_info {
	MEM_TAG base;

	MEM_TAG *delayed_free_list;
	int	size;
	int freesize;
} MEM_SYS;

MEM_SYS memsys[ MAX_MEMSYS ];

static void init_memtag( MEM_TAG *tag, int size )
{
	tag->prev = NULL;
	tag->next = NULL;
	tag->ptr = NULL;
	tag->size = size;
}

void GV_InitMemoryBlock( int id, int type, void *top, int size )
{
	MEM_SYS *m;
	MEM_TAG *tag, *base;
printf( "memsys %X\n", memsys );
printf( "BLOCK %d:%08X:%X\n", id, top, size );

	m = memsys + id;

	/* メモリブロック全体を1つのFreeブロックとして登録 */
	tag = ( MEM_TAG * )top;
	init_memtag( tag, size );
	SET_FREE( tag );

	base = &( m->base );
	init_memtag( base, type );
	base->next = tag;
	tag->prev = base;
	base->ptr = ( void * )tag;

	m->delayed_free_list = NULL;
	m->size = size;
	m->freesize = size;

#ifdef MEMFILL_CHECK
	Fill_Memory( tag + 1, SIZE( tag ) - sizeof( MEM_TAG ) );
#endif
}

void GV_InitMemorySystemAll( void )
{
	int i;

	/*
		すべてのメモリブロック管理構造体を
		バッファ無しの状態で仮に初期化する
	*/
	for( i = 0; i < MAX_MEMSYS; i++ ){
		init_memtag( &memsys[ i ].base, 0 );
	}

#if 0
   {
      unsigned int resident_size = RESIDENT_SIZE;
      unsigned int mem_size = MEM_SIZE;
      unsigned int pack_size = PACK_SIZE;
      unsigned int codec_size = CODEC_SIZE;
      unsigned int stream_buffer_size = STREAM_BUFFER_SIZE;
      unsigned int misc_buffer_size = MISC_BUFFER_SIZE;
      unsigned int mem_total_size = MEM_TOTAL_SIZE;
      char * resident_bottom = RESIDENT_BOTTOM;
      char * mem_addr = MEM_ADDR;
      char * mem_bottom = MEM_BOTTOM;
      char * pack_addr0 = PACK_ADDR0;
      char * pack_addr1 = PACK_ADDR1;
      char * pack_buf_top = PACK_BUF_TOP;
      char * pack_buf_bottom = PACK_BUF_BOTTOM;
      char * stream_buffer = PACK_BUF_BOTTOM;
      char * misc_buffer = MISC_BUFFER;
      printf("GV_MemTop %p -> %p\n", GV_MemTop, GV_MemTop + mem_total_size );
      printf("RESIDENT_SIZE %d\n", resident_size );
      printf("MEM_SIZE %d\n", mem_size );
      printf("PACK_SIZE %d\n", pack_size );
      printf("CODEC_SIZE %d\n", codec_size );
      printf("STREAM_BUFFER_SIZE %d\n", stream_buffer_size );
      printf("MISC_BUFFER_SIZE %d\n", misc_buffer_size );
      printf("MEM_TOTAL_SIZE %d\n", mem_total_size );
      printf("RESIDENT_BOTTOM %p\n", resident_bottom );
      printf("MEM_ADDR %p -> %p\n", mem_addr, mem_addr + mem_size );
      printf("MEM_BOTTOM %p\n", mem_bottom );
      printf("PACK_ADDR0 %p -> %p\n", pack_addr0, pack_addr0 + pack_size );
      printf("PACK_ADDR1 %p -> %p\n", pack_addr1, pack_addr1 + pack_size );
      printf("PACK_BUF_TOP %p\n", pack_buf_top );
      printf("PACK_BUF_BOTTOM %p\n", pack_buf_bottom );
      printf("STREAM_BUFFER %p -> %p\n", stream_buffer, stream_buffer + stream_buffer_size );
      printf("MISC_BUFFER %p -> %p\n", misc_buffer, misc_buffer + misc_buffer_size );
   }
#endif
}

/*
	指定されたalignにそろえる。
	alignは16の倍数である必要あり

	メモリブロックはALIGNにそろえるために大きめにとったりする。
	大きめにとった場合、その直前のintにずれ情報を記録する。
*/

#ifdef DEBUG_MODE
#ifdef KP_WINDOWS	// (#ifdef KP_XBOX)
void *GV_AllocMemoryD( int id, void **ptr, int size, int align, char *fname ,int line)
#else
void *GV_AllocMemoryD( int id, void **ptr, int size, int align, char *fname )
#endif
#else
void *GV_AllocMemory( int id, void **ptr, int size, int align )
#endif
{
	MEM_SYS *m;
	MEM_TAG *tag, *prev;
	void *res;
#ifdef ALLOC_FILL_CHECK
	int org_size = size;
#endif


	m = memsys + id;

	size = NORM16( size ) + sizeof( MEM_TAG );

	res = NULL;
	prev = &( m->base );

	/* フリーリストからサイズに合ったブロックを検索 */
	for( tag = ( MEM_TAG * )prev->ptr; tag != NULL; prev = tag, tag = ( MEM_TAG * )tag->ptr ){
		int s, malign;

		malign = 0;
		s = SIZE( tag );

		if( s < size ){
			continue;
		}
		if( align > 16 ){
			int d;
			unsigned int adr = ( unsigned int )( tag + 1 );
			adr = ( ( adr + align - 1 ) & ~( align - 1 ) );
			d = adr - ( unsigned int )( tag + 1 );
			if( s < size + d ){
				continue;
			}
			size += d;
#ifdef MEMFILL_CHECK
			MemoryCheck( tag + 1, size - sizeof( MEM_TAG ) );
#endif
			if( d > 0 ){
				/* ずれ情報を返すポインタの直前とタグの直後に設定 */
				*( int * )( adr - sizeof( int ) ) = STATE_ALIGN | d;
				*( int * )( tag + 1 ) = STATE_ALIGN | d;
				malign = STATE_ALIGN;
			}
			res = ( void * )adr;
		} else {
			res = tag + 1;
#ifdef MEMFILL_CHECK
			MemoryCheck( tag + 1, size - sizeof( MEM_TAG ) );
#endif
		}
		if( s > size + MINIMUM_SIZE ){
			/* ブロックサイズが大きいので、分割 */
			MEM_TAG *ntag;
			MEM_TAG *next;
			tag->size = size;
			next = tag->next;

			/* もともと freeブロックなので、新たに作った方をリンクに張りなおす */
			ntag = ( MEM_TAG * )( ( char * )tag + size );

			if( next != NULL && IS_FREE( next ) ){
				/* 次のブロックもFreeなので、統合する */
				if( ( ntag->next = next->next ) != NULL ){
					next->next->prev = ntag;
				}
				ntag->size = ( s - size ) + SIZE( next );
				ntag->ptr = next->ptr;
			} else {
				ntag->size = ( s - size );
				ntag->ptr = tag->ptr;
				ntag->next = tag->next;
				if( next != NULL ){
					next->prev = ntag;
				}
			}
			ntag->prev = tag;
			tag->next = ntag;
			prev->ptr = ntag;
			SET_FREE( ntag );
		} else {
			/* ブロックサイズがほぼ同じなのでそのまま使用する */
			SET_USED( tag );
			prev->ptr = tag->ptr;
		}
		m->freesize -= SIZE( tag );
#ifdef DEBUG_MODE
#ifdef KP_WINDOWS
		tag->line = line;
#endif
		tag->ptr = fname;
		tag->size |= malign;
#endif
		break;
	}
#ifdef DEBUG_MODE
	if( res == NULL ){
		// NULL CHECK
		GV_ERROR( GV_ERROR_NO_MEMORY );
		printf( "NO MEMORY!! (%d) %d\n", id, size );
//		GV_DumpMemory( id );
	}
#ifdef ALLOC_FILL_CHECK
	else {
		Fill_Memory( res, org_size );
	}
#endif
#endif
	if( ptr != NULL ){
		*ptr = res;
	}

	return res;
}

static MEM_TAG *get_tag( void *ptr )
{
	MEM_TAG *tag;
	int d;

	d = *( int * )( ( u_int )ptr - sizeof( int ) );
	if( d & STATE_ALIGN ){
		*( byte** )(&ptr) -= d & 0xffff;
	}

	/* メモリブロックタグの取得 */
	tag = ( MEM_TAG * )ptr - 1;

	return tag;
}

void GV_FreeMemory( int id, void *ptr )
{
	MEM_SYS *m;
	MEM_TAG *tag, *p;
	MEM_TAG *prev;

	m = memsys + id;

	/* メモリブロックタグの取得 */
	tag = get_tag( ptr );
#ifdef DEBUG_MODE
	ASSERT( !IS_FREE( tag ) );
	SET_FREE( tag );
#endif

#ifdef MEMFILL_CHECK
	Fill_Memory( tag + 1, SIZE( tag ) - sizeof( MEM_TAG ) );
#endif
	prev = tag->prev;
	m->freesize += SIZE( tag );
	if( IS_FREE( prev ) ){
		/* 直前のブロックもFreeなので連結する */
		if( ( prev->next = tag->next ) != NULL ){
			prev->next->prev = prev;
		}
		prev->size = SIZE( prev ) + SIZE( tag );
		SET_FREE( prev );
#ifdef MEMFILL_CHECK
		Fill_Memory( tag, sizeof( MEM_TAG ) );
#endif
		tag = prev;
	} else {
		/* このブロックをFreeとして登録 */
		/* Freeリストに登録するために検索する */
		for( p = &( m->base ); p != NULL; p = ( MEM_TAG * )p->ptr ){
			/* アドレス順に並んでいることが前提 */
			if( p->ptr > ( void * )tag || p->ptr == NULL ){
				tag->ptr = p->ptr;
				p->ptr = tag;
				break;
			} 
		}
	}
	{
		MEM_TAG *next;
		next = tag->next;
		if( next != NULL && IS_FREE( next ) ){
			/* 直後にあるブロックもFreeだったら連結する */
			if( ( tag->next = next->next ) != NULL ){
				next->next->prev = tag;
			}
			tag->size = SIZE( tag ) + SIZE( next );
			/* 直後のブロックはフリーリストでも直後のはず */
			tag->ptr = next->ptr;
#ifdef MEMFILL_CHECK
			Fill_Memory( next, sizeof( MEM_TAG ) );
#endif
		}
	}
	SET_FREE( tag );
}

/*
	メモリブロックを縮小する。
*/

void GV_ResizeMemory( int id, void *ptr, int size )
{
	MEM_SYS *m;
	MEM_TAG *tag, *new;

	m = memsys + id;

	tag = get_tag( ptr );

	size = ( ( byte* )ptr + NORM16( size ) ) - ( byte* )tag;

	ASSERT( SIZE( tag ) >= (u_int) size );
	if( SIZE( tag ) - size <= MINIMUM_SIZE ){
		return;
	}

	/* 後ろのブロックをUSEDブロックにする */
	new = ( MEM_TAG* )(( byte* )tag + size);
	new->size = SIZE( tag ) - size;
	new->ptr = NULL;
	new->prev = tag;
	if( ( new->next = tag->next ) != NULL ){
		tag->next->prev = new;
	}
	tag->next = new;
	tag->size = size;

	/* 後ろのブロックを解放する */
	GV_FreeMemory( id, new + 1 );
}

/*
	現在の最大空きブロックのサイズを得る
*/

int GV_GetMaxFreeMemory( int id )
{
	MEM_SYS *m;
	MEM_TAG *p;
	int max;

	m = memsys + id;
	max = 0;

	for( p = &( m->base ); p != NULL; p = ( MEM_TAG * )p->ptr ){
		int size;
		size = SIZE( p );
		if( max < size ){
			max = size;
		}
	}
	return max - sizeof( MEM_TAG );
}

int GV_GetFreeMemorySize( int id )
{
#if 0
	MEM_SYS *m;
	MEM_TAG *p;
	int total;
	int _total;

	m = memsys + id;
	total = 0;
	_total = 0;

	for( p = &( m->base ); p != NULL; p = ( MEM_TAG * )p->ptr ){
		int size;
		_total += SIZE( p );
		size = SIZE( p ) - sizeof( MEM_TAG );
		total += size;
	}
printf( "TOTAL %d FREE %d\n", _total, m->freesize );
	ASSERT( _total == m->freesize );
	return total;
#else
	MEM_SYS *m;
	m = memsys + id;
	return m->freesize;
#endif
}

#ifdef DEBUG_MODE

/*
	デバッグ用チェック関数
*/

void GV_DumpMemory( int id )
{
	MEM_SYS *m;
	MEM_TAG *p;

	m = memsys + id;
printf( "DumpMemory\n" );
	for( p = m->base.next; p != NULL; p = p->next ){
		if( p->size & STATE_FREE ){
			printf( "FREE: %08X: %8d\n", p, ( p->size & ~STATE_MASK ) );
		} else {
			printf( "USED: %08X: %8d:%s\n", p, ( p->size & ~STATE_MASK ), p->ptr );
		}
		WAIT_HSYNC( 20 );
	}
printf( "DumpFree\n" );
	for( p = ( MEM_TAG * )m->base.ptr; p != NULL; p = ( MEM_TAG * )p->ptr ){
		printf( "%s: %08X: %8d\n",  ( p->size & STATE_FREE ) ? "FREE" : "USED"
			, p, ( p->size & ~STATE_MASK ) );
		WAIT_HSYNC( 20 );
	}
printf( "END\n" );
}

void GV_DumpMemorySize( int id )
{
	MEM_SYS *m;
	MEM_TAG *p;
	int usedsize, freesize;

	m = memsys + id;
	usedsize = freesize = 0;
	for( p = m->base.next; p != NULL; p = p->next ){
		if( p->size & STATE_FREE ){
			freesize += SIZE( p );
		} else {
			usedsize += SIZE( p );
		}
	}
	printf( "MEMORY: TOTAL %d USED %d FREE %d\n", usedsize + freesize, usedsize, freesize );
}

void GV_SetLoadedMemory( void )
{
	MEM_SYS *m;
	MEM_TAG *p;

	m = memsys + GV_NORMAL_MEMORY;

	for( p = m->base.next; p != NULL; p = p->next ){
		if( !( p->size & STATE_FREE ) ){
			p->size |= STATE_LOAD;
		}
	}
}

void GV_CheckMemoryLeak( void )
{
	MEM_SYS *m;
	MEM_TAG *p;
	int exist;

	m = memsys + GV_NORMAL_MEMORY;

	exist = 0;
	for( p = m->base.next; p != NULL; p = p->next ){
		if( !( p->size & STATE_FREE ) ){
			if( !( p->size & STATE_LOAD ) ){
				exist = 1;
#ifdef KP_WINDOWS
				printf( "MEMORY LEAK : %08X: %8d:%s(%d)\n", p
						, ( p->size & ~STATE_MASK ), p->ptr, p->line );
#else
				printf( "MEMORY LEAK : %08X: %8d:%s\n", p
						, ( p->size & ~STATE_MASK ), p->ptr );
#endif
			}
		}
	}
	ASSERT( exist == 0 );
}

#endif	// DEBUG_MODE

#ifndef MGS_MASTER

/*
	MEMORY LISTの整合性チェック（デバッグ用）
*/

static int pointer_check( void *p, MEM_SYS *m )
{
	void *top;
	void *bottom;
	top = m->base.next;
	bottom = ( byte* )top + m->size;

	if( p == NULL ) return 0;
	if( ( ( ( int )p & 15 ) != 0 ) || ( p < top || p > bottom ) ){
		return -1;
	}
	return 1;
}

static int pointer_check2( void *p, MEM_SYS *m )
{
#ifdef PSX2	/// いらない
	void *top;
	void *bottom;
	extern int _program_top[];
	top = m->base.next;
	bottom = ( byte* )top + m->size;

	if( p == NULL ) return 0;
	if( ( ( ( int )p & 3 ) != 0 ) || p < ( void * )_program_top || p > MEM_BOTTOM ){
		return -1;
	}
#endif ///	
	return 1;
}

#define BROKEN_PREV		0x01
#define BROKEN_NEXT		0x02
#define BROKEN_PTR		0x04
#define BROKEN_SIZE		0x08

int GV_CheckMemory( void )
{
	int id;
	MEM_SYS *m;
	MEM_TAG *p, *prev;
	int flag;

	id = GV_NORMAL_MEMORY;

	m = memsys + id;

	prev = NULL;
	for( p = m->base.next; p != NULL; p = p->next ){
		int size;
		flag = 0;
		if( pointer_check2( p, m ) <= 0 ) goto broken2;
		if( pointer_check2( p->ptr, m ) < 0 ){
			flag |= BROKEN_PTR;
		}
		if( p->prev == NULL ){
			if( p != m->base.next ){
				flag |= BROKEN_PREV;
			}
		} else {
			if( pointer_check2( p->prev, m ) <= 0 ){
				flag |= BROKEN_PREV;
			}
		}
		if( pointer_check( p->next, m ) < 0 ){
			flag |= BROKEN_NEXT;
		}

		size = p->size & ~STATE_MASK;
		if( p->next != NULL && ( byte* )p + size != ( byte* )p->next ){
			flag |= BROKEN_SIZE;
		} else if( p->next == NULL &&
				   ( size + ( byte* )p != ( byte* )m->base.next + m->size ) ){
			flag |= BROKEN_SIZE;
		}

		if( flag != 0 ){
			/* 何がしかが壊れている */
			excep_printf( "mem broken %X %X %X %X %X\n"
						  , p, p->prev, p->next, p->ptr, p->size );
			if( prev != NULL ){
				// 正しかった最後のメモリブロック
				excep_printf( "prev=%X %X", prev, prev->size );
#ifdef DEBUG_MODE
				if( prev->size & STATE_ACTOR ){
					GV_ACT *act = ( GV_ACT * )( prev + 1 );
					excep_printf( " %s", act->filename );
				}
#endif
				excep_printf( "\n" );
				return 0;
			}
		}
		prev = p;
	}
	for( p = ( MEM_TAG * )m->base.ptr; p != NULL; p = ( MEM_TAG * )p->ptr ){
		if( pointer_check( p->ptr, m ) < 0 ){
			excep_printf( "mem broken free %X\n", p );
			return 0;
		}
	}
	return 1;
broken2:
	excep_printf( "mem broken ALL %X\n", p );
	return 0;
}

#endif	// MGS_MASTER

#ifndef KP_WINDOWS		// OSによって不安定なのでZeroMemoryに任せる
void		GV_ZeroMemory( void *to, int size )
{
   // BP - This function was being very clever by clearing out to a 16-byte aligned
   // space, then clearing one long128 at a time.
   // It's memset 0, so let's just memset 0.

   memset( to, 0, size );

#if 0 //BP_PS2
	typedef long128	Unit;
	Unit		*u ;
	char		*c ;
	int		i ;

//	c = (char *)GV_UNCACHE( to );
	c = ( char * )to ;
	i = ( 15 & (int)to );
	size -= i ;
	for ( ; i > 0 ; -- i ) {
		*( c ++ ) = 0 ;
	}
	u = (Unit *)c ;
	for ( i = size / sizeof( Unit ) ; i > 0 ; -- i ) {
#if 0 //BP_PS2
//#ifdef PSX2		
		*u = 0;
#else
		//BP_XBOX ZeroMemory( u, sizeof(*u) );
      memset(u, 0, sizeof(*u));
#endif		
		u ++ ;
	}
	c = (char *)u ;
	for ( i = ( sizeof( Unit ) - 1 ) & size ; i > 0 ; -- i ) {
		*( c ++ ) = 0 ;
	}
#endif
}
#endif	// KP_WINDOWS

#ifdef DEBUG_MODE
#ifdef KP_WINDOWS	// (#ifdef KP_XBOX)
void *GV_MallocD( int size, char *fname, int line )
{
	return GV_AllocMemoryD( GV_NORMAL_MEMORY, NULL, size, 0, fname , line) ;
}
#else
void *GV_MallocD( int size, char *fname )
{
	return GV_AllocMemoryD( GV_NORMAL_MEMORY, NULL, size, 0, fname ) ;
}
#endif
#else
void *GV_Malloc( int size )
{
	return GV_AllocMemory( GV_NORMAL_MEMORY, NULL, size, 0 ) ;
}
#endif

#ifdef DEBUG_MODE
#ifdef KP_WINDOWS	// (#ifdef KP_XBOX)
void *GV_MallocLoadD( int size, char *fname  ,int line)
{
	void *ptr;
	MEM_TAG *tag;
	ptr = GV_AllocMemoryD( GV_NORMAL_MEMORY, NULL, size, 0, fname ,line) ;
	if( ptr == NULL ) return NULL;
	tag = get_tag( ptr );
	tag->size |= STATE_LOAD;
	return ptr;
}

void *GV_MallocActD( int size, char *fname ,int line )
{
	void *ptr;
	MEM_TAG *tag;
	ptr = GV_AllocMemoryD( GV_NORMAL_MEMORY, NULL, size, 0, fname , line) ;
	if( ptr == NULL ) return NULL;
	tag = get_tag( ptr );
	tag->size |= STATE_ACTOR;
	return ptr;
}
#else
void *GV_MallocLoadD( int size, char *fname )
{
	void *ptr;
	MEM_TAG *tag;
	ptr = GV_AllocMemoryD( GV_NORMAL_MEMORY, NULL, size, 0, fname) ;
	if( ptr == NULL ) return NULL;
	tag = get_tag( ptr );
	tag->size |= STATE_LOAD;
	return ptr;
}

void *GV_MallocActD( int size, char *fname )
{
	void *ptr;
	MEM_TAG *tag;
	ptr = GV_AllocMemoryD( GV_NORMAL_MEMORY, NULL, size, 0, fname) ;
	if( ptr == NULL ) return NULL;
	tag = get_tag( ptr );
	tag->size |= STATE_ACTOR;
	return ptr;
}
#endif
#endif

void		GV_Free( void *addr )
{
	GV_FreeMemory( GV_NORMAL_MEMORY, addr ) ;
}

void GV_Resize( void *addr, int size )
{
	GV_ResizeMemory( GV_NORMAL_MEMORY, addr, size );
}

/* ------------------------------------------------------------------ */
/*
	delayed free 関連
	(次のVSYNCの頭でfree)
*/

void GV_CleanMemorySystem( void )
{
	int i;

	for( i = 0; i < MAX_MEMSYS; i++ ){
		MEM_SYS *sys;
		sys = &( memsys[ i ] );

		if( sys->delayed_free_list != NULL ){
			MEM_TAG *tag;

			tag = sys->delayed_free_list;

			while( tag != NULL ){
				MEM_TAG *next;
				next = tag->ptr;
				GV_FreeMemory( i, tag + 1 );
				tag = next;
			}
			sys->delayed_free_list = NULL;
		}
	}
}

void GV_FreeMemory2( int which, void *addr )
{
	MEM_SYS *sys;
	MEM_TAG *tag;

	ASSERT( which >= 0 && which < MAX_MEMSYS );
	sys = &( memsys[ which ] );

	tag = ( MEM_TAG * )addr - 1;

	/* 解放待ち状態にする */
#ifdef DEBUG_MODE
	ASSERT( !IS_FREE( tag ) );
	ASSERT( !IS_DELAYED_FREE( tag ) );
	SET_DELAYED_FREE( tag );
#endif
	tag->ptr = sys->delayed_free_list;
	sys->delayed_free_list = tag;
}

void GV_DelayedFree( void *addr )
{
	GV_FreeMemory2( GV_NORMAL_MEMORY, addr );
}

