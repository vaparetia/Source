//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   CDBIOS for XBOX
*/

#include <stdio.h>
#include "libfs.h"
#include "cdbios_in.h"

#include "BP_FileSupport.h"

#ifdef _DEV_

#define DEV_MODE 1
#define DVD_MODE 1
#define HD_MAX_FILENUM		16		// 1からはじまるので、最大数はこの数-1
#define HD_MAX_FILENAME		1024

#elif defined( _XDAT_ )

// XBOXセキュリティ対応
#define XDAT_MODE 1
#define HD_MAX_FILENUM		16		// 1からはじまるので、最大数はこの数-1
#define XDAT_MAX			(10*2)
#define DISC_0_TOP			0x00000000
#define DISC_1_TOP			0x01000000

#else

#define DVD_MODE 1
#define HD_MAX_FILENUM		16

#endif

#define DVD_MAX_HANDLE		16

#define HD_FILE_ID( _id )			( (_id) << 24 )
#define DEV_FILE_MASK				0x40000000

#define SECTOR_SIZE				(2048)
#define SECTOR( _size )		( (_size+SECTOR_SIZE-1)/SECTOR_SIZE )

#define CDBIOS_STACK_SIZE	(12*1024)

typedef struct {
	CDBIOS_TASK_STATUS common;
	int force_stop_flag;
	void *buffer_address;

	int command;
	int thread_id;
//BP_XBOX	HANDLE thread_handle;
//BP_XBOX	HANDLE sema_handle;
	void (*callback_func)( CDBIOS_READ_STATUS * );
#ifdef DVD_MODE
//BP_XBOX	HANDLE	dvd_file_handle[ DVD_MAX_HANDLE ];
#endif
#ifdef XDAT_MODE
	int datnum[ 2 ];
	int filenum;

	struct {
		HANDLE handle;
		unsigned int len;
	} dats[ 2 ][ XDAT_MAX ];

	struct {
		char name[ 16 ];
		int top;
	} file[ HD_MAX_FILENUM ];
#endif

#ifdef DEV_MODE
	char *hd_filename[ HD_MAX_FILENUM ];
	char hd_filename_buf[ HD_MAX_FILENAME ];
#endif

   void * bp_op;
   int    bp_read_size;
   int    bp_intr_mode;
} CDBIOS_WORK;

static CDBIOS_WORK work;


/* ---------------------------------------------------------------------- */

static void bp_read_sectors_async( CDBIOS_WORK *w )
{
   CDBIOS_READ_STATUS *rs = &work.common.read;
   int fileIndex = rs->pos >> 24;
   int bufOfs = 0;
   char path[255];
#if 0
   //From .DAT file
   int lsn = rs->pos & 0xffffff;
   int loc = lsn * SECTOR_SIZE;
   const char * filename = fs_file_info[fileIndex].name;
   strcpy( path, filename );
#else
   //From individual .SDT files
   int orgLsn = rs->pos & 0xffffff;
   const char * filename = fs_file_info[fileIndex].name;
   //We have split out the streams / codecs from the .DAT files for CP4.  Convert original dat / lsn
   //pair to a new sdat / lsn pair.
   int loc = BP_GetRemappedStreamNameAndLoc( fileIndex, orgLsn, rs->bp_stream_top, path );
   if( loc < 0 )
   {
      //This ugly case only comes up when we're reading from CODEC.DAT, where the read top is rounded
      //down and the read size is rounded up to the nearest sector.  We can't read past the top of
      //an individual file, so the above function returns a negative number for an offset when we read
      //an unaligned codec script (which is almost all of them).
      if( fileIndex != FS_FILE_CODEC )
         BP_BREAK;
      bufOfs = -loc;
      loc = 0;
   }

   // Ensure we're reading the whole file
   if( fileIndex == FS_FILE_CODEC)
   {
      // We're not reading enough data for this file!
      if( BP_GetFileSizeAttr(path, NULL) > w->bp_read_size )
         BP_BREAK;
   }

#endif

   w->bp_op = BP_ReadFileByFilenameAsync( path, (char*)rs->ee_buffer + bufOfs, loc, w->bp_read_size - bufOfs );
}

static void bp_start_next_read( CDBIOS_WORK *w )
{
   CDBIOS_READ_STATUS *rs = &work.common.read;
   if( rs->intr_size > 0 )
   {
      work.bp_read_size = rs->intr_size;
      work.bp_intr_mode = 1;
   }
   else
   {
      work.bp_read_size = rs->total_size - rs->read_size;
   }
   bp_read_sectors_async( &work );
}

void bp_cdbios_sync()
{
   if( work.bp_op )
   {
      CDBIOS_READ_STATUS *rs;

      rs = &work.common.read;

      if( !BP_TryFinishFileOp( work.bp_op ) )
      {
         return;
      }

      //Update rs and call callback.
      work.bp_op = NULL;
      rs->read_size += work.bp_read_size;
      *( char ** )(&rs->ee_buffer) += work.bp_read_size;
      if( rs->intr_size > 0 ){
         rs->intr_size -= work.bp_read_size;
      }
      rs->pos += SECTOR( work.bp_read_size );

      if( work.bp_intr_mode || rs->read_size == rs->total_size ){
         // ユーザーコールバッグを発生

         if( work.callback_func != NULL ){
            ( *work.callback_func )( rs );
         }
         work.bp_intr_mode = 0;
      }

      //Start next read.
      if( rs->read_size < rs->total_size )
      {
         bp_start_next_read( &work );
      }
      else
      {
         //We are done.
         work.callback_func = NULL;
         work.common.status = CDBIOS_STATE_IDLE;
      }
   }
}

/* ---------------------------------------------------------------------- */
/*
	サブルーチン
*/

#if 0 //BP_XBOX
static int wait_command( CDBIOS_WORK *w )
{
	WaitForSingleObject( w->sema_handle, INFINITE );
	w->command = w->common.exec_com;

	return w->command;
}
#endif

static void set_status( CDBIOS_WORK *w, int status )
{
	w->common.status = status;
}

/*
   各状況のステータスマシン
*/

// リード関数
// posからsizeだけbufferに読み込む。

#if 0 //BP_XBOX
static int read_callback_result = 0;

static void CALLBACK read_callback(DWORD dwErrorCode, DWORD dwSize, LPOVERLAPPED p)
{
	read_callback_result -= dwSize;
}

static int do_read_file( CDBIOS_WORK *w, HANDLE handle, void *buffer
						, int size, OVERLAPPED *ov )
{
	// 無限リトライ付きリード関数

#ifdef KP_WINDOWS
	if( !X2W_OverlappedReadFileEnable() ){ ov = NULL ; }
#endif

	if( SleepEx( 0, TRUE ) == WAIT_IO_COMPLETION ){
		printf( "Flush Event\n" );
	}
	read_callback_result = size;

	while( ReadFileEx( handle, buffer, size, ov, read_callback ) == 0 ){
		printf( "ReadError %d\n", GetLastError() );
		set_status( w, CDBIOS_STATE_COMMAND_ERROR );
		Sleep( 500 );	// 0.5 Sec
	}
	while( SleepEx( 5000, TRUE ) != WAIT_IO_COMPLETION ){
		printf( "TIMEOUT\n" );
		set_status( w, CDBIOS_STATE_COMMAND_ERROR );
		// タイムアウトの発生
	}
	if( read_callback_result == size ){
		// read error
		printf( "ReadError %d\n", GetLastError() );
		return 0;
	}
	return 1;
}
#endif

#if defined( DVD_MODE ) || defined( DEV_MODE )

static int read_sectors( CDBIOS_WORK *w, void *buffer, unsigned int pos, int size )
{
#if 1 //BP_PS2
   int fileIndex = pos >> 24;
   int lsn = pos & 0xffffff;
   const char * filename = fs_file_info[fileIndex].name;
   char path[ 255 ];
   void* fp;

  fp = BP_OpenFile(filename, NULL);
   if( !fp )
      BP_BREAK;
   BP_SeekFile( fp, lsn * SECTOR_SIZE, SEEK_SET );
   BP_ReadFile(fp, buffer, size);
   BP_CloseFile(fp);
   return 1;
#else
	int fn;
	int ps;
	OVERLAPPED ov;

	fn = pos >> 24;
	ps = pos & 0x00FFFFFF;

	ov.Offset = ps * SECTOR_SIZE;
	ov.OffsetHigh = 0;

	size = SECTOR( size ) * SECTOR_SIZE;

	return do_read_file( w, w->dvd_file_handle[ fn ], buffer, size, &ov );
#endif
}

#endif

#ifdef XDAT_MODE

#define MIN( a, b )	((a)<(b)?(a):(b))

static int read_sectors( CDBIOS_WORK *w, void *buffer, unsigned int pos, int size )
{
	int disc_no;
	int n;
	int s;
	unsigned int sect;
	HANDLE handle = INVALID_HANDLE_VALUE;
	OVERLAPPED ov;

	// ＰＯＳからどのファイルかを検索。
	if( pos < DISC_1_TOP ){
		disc_no = 0;
	} else {
		disc_no = 1;
		pos -= DISC_1_TOP;
	}

	for( n = 0; n < w->datnum[ disc_no ]; n++ ){
		unsigned int len;
		len = w->dats[ disc_no ][ n ].len;
		if( pos < len ){
			break;
		}
		pos = pos - len;
	}
	if( n == w->datnum[ disc_no ] ){
		printf( "MAX ERROR !!\n" );
		return 0;
	}

	sect = SECTOR( size );

	s = MIN( sect, ( w->dats[ disc_no ][ n ].len - pos ) );

	// 一回目。
	ov.Offset = pos * SECTOR_SIZE;
	ov.OffsetHigh = 0;

	if( do_read_file( w, w->dats[ disc_no ][ n ].handle
					 , buffer, s * SECTOR_SIZE, &ov ) == 0 ){
		// 失敗。
		return 0;
	}

	buffer = ( char * )buffer + s * SECTOR_SIZE;
	sect -= s;
	s = sect;

	if( sect > 0 && n + 1 < w->datnum[ disc_no ] ){
		// 二回目。posは次のファイルの頭からになる。
		n = n + 1;
		ov.Offset = 0 * SECTOR_SIZE;
		ov.OffsetHigh = 0;

		printf( "2nd Read %d %d\n", n, s );

		if( do_read_file( w, w->dats[ disc_no ][ n ].handle
						 , buffer, s * SECTOR_SIZE, &ov ) == 0 ){
			// 失敗。
			return 0;
		}
	}
	return 1;
}
#endif

#ifdef DEV_MODE

static int read_sectors_dev( CDBIOS_WORK *w, void *buffer, unsigned int pos, int size )
{
	int fn;
	int ps;
	int fd;

	pos = pos & ~DEV_FILE_MASK;

	fn = pos >> 24;
	ps = pos & 0x00FFFFFF;

	fd = pcOpen( w->hd_filename[ fn ], O_RDONLY );
	pcLseek( fd, ps * SECTOR_SIZE, SEEK_SET );
	pcRead( fd, buffer, size );
	pcClose( fd );

	return 1;
}

static int read_command( CDBIOS_WORK *w )
{
	volatile CDBIOS_READ_STATUS *rs;

	int intr_mode;

	int (*read_func)( CDBIOS_WORK *w, void *buffer, unsigned int pos, int size )
		= read_sectors;

	set_status( w, CDBIOS_STATE_COMMAND_EXECUTING );

	intr_mode = 0;

	rs = &w->common.read;

#ifdef DEV_MODE
	if( rs->pos & DEV_FILE_MASK ){
		read_func = read_sectors_dev;
	}
#endif

	while( rs->read_size < rs->total_size ){
		int size;

		if( rs->intr_size > 0 ){
			size = rs->intr_size;
			intr_mode = 1;
		} else {
			size = rs->total_size - rs->read_size;
		}
		while( ( *read_func )( w, rs->ee_buffer, rs->pos, size ) == 0 ){
			// 無限リトライ
			set_status( w, CDBIOS_STATE_COMMAND_ERROR );
			Sleep( 500 );	// 0.5 Sec
		}
		rs->read_size += size;
		( char * )rs->ee_buffer += size;
		if( rs->intr_size > 0 ){
			rs->intr_size -= size;
		}
		rs->pos += SECTOR( size );

		if( intr_mode || rs->read_size == rs->total_size ){
			// ユーザーコールバッグを発生

			if( w->callback_func != NULL ){
				( *w->callback_func )( rs );
			}
			intr_mode = 0;
		}
	}
	w->callback_func = NULL;
	return CDBIOS_COMMAND_WAIT;
}

/* ---------------------------------------------------------------------- */
/*
	スレッド
*/

static int WINAPI cdbios_thread( void *param )
{
	CDBIOS_WORK *w = param;
	int status;

	status = CDBIOS_COMMAND_WAIT;
	for( ;; ){
		switch( status ){
		  case CDBIOS_COMMAND_WAIT:
			set_status( w, CDBIOS_STATE_IDLE );
			status = wait_command( w );
			set_status( w, CDBIOS_STATE_COMMAND_EXECUTING );
			break;
		  case CDBIOS_COMMAND_READ:
			status = read_command( w );
			break;
		  case CDBIOS_COMMAND_SEEK:
			// 後回し
			break;
#if 0
		  case CDBIOS_COMMAND_SRCHFILE:
			// リードスレッドではないところで実装する
#endif
		  default:
			status = CDBIOS_COMMAND_WAIT;
			break;
		}
//		w->common.exec_com = status;
	}
}

/* ---------------------------------------------------------------------- */
/*
	コマンド発生
*/

static void send_command( int command )
{
   BP_BREAK;
	work.common.exec_com = command;
	work.common.status = CDBIOS_STATE_COMMAND_SET;
	ReleaseSemaphore( work.sema_handle, 1, NULL );
}

/* ---------------------------------------------------------------------- */
/*
	外からのインターフェース
*/

/*
   ファイルを設定し、アクセス用のIDを返す。
*/

int cdbios_get_filepos( char *name )
{
	CDBIOS_WORK *w = &work;
	int res = -1;
	int i;

	// DVD_MODEはファイルをopenして、そのHandleを持つ。
	if( name[ 1 ] == ':' ){
		// XBOXのドライブレターがある。
#ifdef DVD_MODE
		for( i = 1; i < DVD_MAX_HANDLE; i++ ){
			if( w->dvd_file_handle[ i ] == NULL ){
				/* 空きがあった */
				w->dvd_file_handle[ i ]
					= CreateFile( name, GENERIC_READ, 0
								 , NULL, OPEN_EXISTING
								 , FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL );
				if( w->dvd_file_handle[ i ] == INVALID_HANDLE_VALUE ){
					printf( "CreateFile Error '%s' %X\n", name, GetLastError() );
					//				ASSERT( 0 );
					return -1;
				}
				res = HD_FILE_ID( i );
				return res;
			}
		}
		printf( "Too Many Files\n" );
#endif
#ifdef XDAT_MODE
		// すでにファイルテーブルは読み込まれている。
		for( i = 0; i < w->filenum; i++ ){
			char *p;
			if( ( p = strrchr( name + 2, '\\' ) ) != NULL ){
				p = p + 1;
			} else {
				p = name + 2;
			}
			
			if( strcmp( p, w->file[ i ].name ) == 0 ){
				return w->file[ i ].top;
			}
		}
		printf( "NO FILE %s\n", name );
		return -1;
#endif
	} else {
#ifdef DEV_MODE
	// DEV_MODEは、ファイル名を持ち、逐次OPENする
#define HD_FILENAME_OFS( _work )	( (( int *)( _work )->hd_filename)[ 0 ] )

		for( i = 1; i < HD_MAX_FILENUM; i++ ){
			if( w->hd_filename[ i ] != NULL ){
				if( strcmp( w->hd_filename[ i ], name ) == 0 ){
					// すでに設定済み
					res = HD_FILE_ID( i );
					break;
				}
			} else {
				int len;
				len = strlen( name ) + 1;
				if( HD_FILENAME_OFS( w ) + len >= HD_MAX_FILENAME ){
					printf( "FILENAME BUFFER OVER !!\n" );
				} else {
					int ofs;
					ofs = HD_FILENAME_OFS( w );
					w->hd_filename[ i ] = &( w->hd_filename_buf[ ofs ] );
					strcpy( w->hd_filename_buf + ofs, name );
					HD_FILENAME_OFS( w ) = ofs + len;
					res = HD_FILE_ID( i ) | DEV_FILE_MASK;
				}
				break;
			}
		}
#else
		printf( "cdbios:Filename Error %s\n", name );
#endif
	}
	return res;
}

#endif //BP_XBOX

/* ---------------------------------------------------------------------- */
/*
   そととのインターフェース関数
*/

int cdbios_get_status( void )
{
   bp_cdbios_sync();
//printf( "STAT=%X\n", work.common.status );
	return work.common.status;
}

int cdbios_get_read_size( void )
{
	return work.common.read.read_size;
}

void cdbios_callback_read( void *buffer, int pos, int size
						   , void (*callback)(CDBIOS_READ_STATUS *), int intr_size, int bp_stream_top )
{
	CDBIOS_READ_STATUS *rs;

   int intr_mode;

	rs = &work.common.read;
	rs->pos = pos;
	rs->ee_buffer = buffer;
	rs->total_size = size;
	rs->read_size = 0;
	rs->intr_size = intr_size;
   rs->bp_stream_top = bp_stream_top;
	work.callback_func = callback;

#if 1 //BP_PS2

   work.common.status = CDBIOS_COMMAND_READ;
   bp_start_next_read( &work );

#else
	send_command( CDBIOS_COMMAND_READ );
#endif
}

#if 0 //BP_UNUSED
void cdbios_callback_read_stm( void *buffer, int pos, int size
						   , void (*callback)(CDBIOS_READ_STATUS *), int intr_size )
{
	cdbios_callback_read( buffer, pos, size, callback, intr_size, 0 );
}
#endif

void cdbios_read( void *buffer, int pos, int size, int bp_stream_top )
{
	cdbios_callback_read( buffer, pos, size, NULL, -1, bp_stream_top );
}

void cdbios_stop( void )
{
}

void cdbios_preseek( int pos )
{
}

/* ---------------------------------------------------------------------- */
/*
	cdReadインターフェース
*/

int cdRead( int pos, void *buffer, int size )
{
	CDBIOS_WORK *w = &work;
   BP_TODO_BREAK;
	// 完全にメイン側の読み込みが終わってからということが保証されている
	while( read_sectors( w, buffer, pos, size ) == 0 ){
      BP_BREAK;
//		printf( "Error %x\n", GetLastError() );
	}
	return size;
}

/* ---------------------------------------------------------------------- */
/*
   初期化
*/

#if defined( DEV_MODE )
void cdbios_dev_init( void )
#elif defined( XDAT_MODE )
void cdbios_xdat_init( void )
#else
void cdbios_init( void )
#endif
{
	// ワーク初期化
	memset( &work, 0, sizeof( CDBIOS_WORK ) );

#if 0 //BP_XBOX
	// コマンド取得セマフォ作成
	work.sema_handle = CreateSemaphore( NULL, 0, 1, NULL );

	// ファイル読み込みスレッド起動

	work.thread_handle = BP_CreateThread( NULL, CDBIOS_STACK_SIZE
									, ( LPTHREAD_START_ROUTINE )cdbios_thread
									, &work
									, CREATE_SUSPENDED
									, &work.thread_id );
#ifndef KP_WINDOWS
//	SetThreadPriority( work.thread_handle, THREAD_PRIORITY_NORMAL );	// ?
	SetThreadPriority( work.thread_handle, 1 );	// ?
#else
//@	SetThreadPriority( work.thread_handle, THREAD_PRIORITY_ABOVE_NORMAL ) ;
	SetThreadPriority( work.thread_handle, THREAD_PRIORITY_HIGHEST ) ;
//@	SetThreadPriority( work.thread_handle, THREAD_PRIORITY_IDLE ) ;
#endif
	BP_ResumeThread( work.thread_handle );

#endif

#ifdef XDAT_MODE
	{
		// 初期化シーケンス
		int disc, i;
		int f = 0;
		char buffer[ SECTOR_SIZE ];

		// DISC1, DISC2に焼いてあるすべてのＤＡＴをオープンする
		for( disc = 0; disc < 2; disc ++ ){
			for( i = 0; ; i++ ){
				HANDLE handle;
				char name[ 64 ];
				sprintf( name, "D:\\DISC%d_%d.DAT", disc + 1, i );
				handle = CreateFile( name, GENERIC_READ, 0
								 , NULL, OPEN_EXISTING
								 , FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, NULL );
				if( handle == INVALID_HANDLE_VALUE ){
					printf( "%s Failed\n", name );
					break;
				}
				work.dats[ disc ][ i ].handle = handle;
				work.dats[ disc ][ i ].len = GetFileSize( handle, NULL ) / SECTOR_SIZE;
printf( "DAT %s LEN %X\n", name, work.dats[ disc ][ i ].len );
			}
			work.datnum[ disc ] = i;
			// 各ディスク先頭のセクタを読み取り、FILETABLEに登録
			{
				OVERLAPPED ov;
				DWORD res;
#ifdef KP_WINDOWS
				OVERLAPPED *ovp;
#endif

				ov.Offset = 0;
				ov.OffsetHigh = 0;
#ifndef KP_WINDOWS
				while( ReadFileEx( work.dats[ disc ][ 0 ].handle
							 , buffer, SECTOR_SIZE, &ov, read_callback ) == 0 ){
					printf( "Read ErrorD %d\n", GetLastError() );
					Sleep( 500 );	// 0.5 Sec
				}
#else
				if( X2W_OverlappedReadFileEnable() ){ ovp = NULL ; }
				else{ ovp = &ov ; }
				while( ReadFileEx( work.dats[ disc ][ 0 ].handle
							 , buffer, SECTOR_SIZE, ovp, read_callback ) == 0 ){
					printf( "Read ErrorD %d\n", GetLastError() );
					Sleep( 500 );	// 0.5 Sec
				}
#endif
				SleepEx( INFINITE, TRUE );
			}
			{
				typedef struct {
					char name[ 16 ];
					int top;
					int len;
				} FILETABLE;
				typedef struct {
					int filenum;
					FILETABLE files[ 1 ];	// 本当は可変長。
				} HEADER;

				HEADER *head = ( HEADER * )buffer;
				int n;
				FILETABLE *ft;
				ft = head->files;
				for( n = head->filenum; n > 0; n-- ){
					strcpy( work.file[ f ].name, ft->name );
					work.file[ f ].top = ft->top
						| ( ( disc == 0 ) ? DISC_0_TOP : DISC_1_TOP );
printf( "FILE %s TOP %08X\n", work.file[ f ].name, work.file[ f ].top );
					f++;
					ft ++;
				}
			}
		}
		work.filenum = f;
	}
#endif
}
