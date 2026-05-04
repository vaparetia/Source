/*
 * PlayStation2 とのやりとりを行うサーバプログラム。
 *
 * USBを介したクライアントからの要求に答え、ファイルの内容を送受信する。
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/linQcable.h>

#include <fcntl.h>

#include <setjmp.h>

#define _linQio_c_
#include "config.h"
#include "linQio.h"

#ifndef _DEBUG_
//#define DEBUG(args...)
#else
#define DEBUG(args...)  { \
	extern int verbose_mode; \
	if( verbose_mode ) fprintf(stdout, args);\
}
#endif /* _DEBUG_ */

#define log( a... )	fprintf( stderr, a )
#define err( a... )	fprintf( stderr, a )

static jmp_buf jmp_env;

/* コマンドパケットバッファ */
static unsigned char _cmd_packet[ PACKET_SIZE ];

/* PS2 のリクエストで操作されるファイル */
static FILE * _ps2_fp[ MAX_PS2_FILES ];

/* 送受信で使用されるバッファ */
static unsigned char _trans_buf[ TRANS_BUFSIZE ];


/* Root ディレクトリとなるディレクトリを保持 */
static char * _ps2Root = NULL;

void error_return( int code )
{
	longjmp( jmp_env, code );
}

/* ------------------------------------------------------------------------- */

/* パケットの後半4 バイトを long 値とみなし、その値を取得する */
static long sv_get_long(unsigned char * packet)
{
	long ret;
	int i;

	ret = 0;
	for(i = 0; i < 4; i++)
		ret |= (long)packet[i + 4] << (8 * i);
	return ret;
}

/* long 値をパケットの後半4 バイトにセットする */
static void sv_set_long(long var, unsigned char *packet)
{
	int i;

	for(i = 0; i < 4; i++)
		packet[i + 4] = (var >> (8 * i)) & 0xff;

	return;
}


/*
 * 現在開いているファイルを全てクローズする
 */
static int sv_all_close(void)
{
	int i;

	for(i = 0; i < MAX_PS2_FILES; i++)
		if(_ps2_fp[i] != NULL)
			{
				fclose(_ps2_fp[i]);
				_ps2_fp[i] = NULL;
			}
}

/*
 * パケットの受け取り
 */
int sv_recv_packet(int fd, unsigned char *packet)
{
	ssize_t recieved_size;
	int i;

	recieved_size = read( fd, packet, PACKET_SIZE );
#if 0
	printf("Rcv:");
	for(i = 0; i < recieved_size; i++)
		printf(" %02x", packet[i]);
	printf("\n");
	/* fflush(stdout); */
#endif
	return (recieved_size == PACKET_SIZE) ? 0 : 1;
}

/*
 * パケットの送出
 */
int sv_send_packet(int fd, unsigned char *packet)
{
	ssize_t sended_siz;
	int i;

	/* 作成したパケットを送出する */
	sended_siz = write(fd, packet, PACKET_SIZE);
#if 0
	printf("Snd:");
	for(i = 0; i < sended_siz; i++)
		printf(" %02x", packet[i]);
	printf("\n");

	fprintf(stderr, "OK. packet = %d bytes\n", sended_siz);
#endif
	return (sended_siz == PACKET_SIZE) ? 0 : -1;
}


/*
 * エラー時にエラーコードを送出する
 */
static int sv_send_error(int fd, int err_code)
{
	unsigned char err_packet[ PACKET_SIZE ];
	int i;

	err_packet[0] = 0x01;
	err_packet[1] = 0x01;
	err_packet[2] = (unsigned char)(err_code & 0xff);
	for(i = 3; i< PACKET_SIZE; i++) err_packet[i] = 0x00;

	return sv_send_packet(fd, err_packet);
}

/*
 * OK のステータスを送出する
 */
static int sv_send_success(int fd)
{
	int ret;
	static unsigned char packet[ PACKET_SIZE ] = {0x01, 0x00, 0x00, 0x00,
												  0x00, 0x00, 0x00, 0x00};
	ret = sv_send_packet(fd, packet);

	return ret;
}

/*
 * 初期化
 */
static int sv_init(int fd)
{
	unsigned char packet[ PACKET_SIZE ];
	int i;
	int ret;

	sv_all_close();   /* 現在開いているファイルを全てクローズする */
  
	for(i = 0; i < PACKET_SIZE; i++) packet[i] = 0x00;
    
	packet[0] = 0x01;
	packet[1] = 0x00;
	packet[2] = _cmd_packet[2];  /* 送られて来たプロセス ID を返送する */
  
	ret = sv_send_packet(fd, packet);
#if 0
	fprintf(stderr, "success sended.\n");
#endif
	log( "<PS2 connection start>\n" );

	return ret;
}

/*
 * ファイルのオープン
 */
static int sv_open(int fd)
{
	unsigned char packet[ PACKET_SIZE ];
	int mode, len, rootlen, rdlen;
	char *fname, *fullpath;
	char *modestr[] = { 0, "rb", "wb", "r+b" };
	int i;

	mode = _cmd_packet[2];

	len = sv_get_long(_cmd_packet);
	rootlen = strlen(_ps2Root);

//	DEBUG("file name len = %d\n", len);
  
	if(NULL == (fname = malloc(rootlen + len + 1))) goto err_0;
	if(sv_send_success(fd)) return -1;
  
	strcpy(fname, _ps2Root);
  
//	DEBUG("fd(%d)  len(%d)\n", fd, len);

	rdlen = read(fd, fname + rootlen, len);

//	DEBUG("fname read: %d characters.\n", rdlen);

	if(len != rdlen) goto err_1;

	len += rootlen;
	fname[len] = 0;

	DEBUG("usbOpen(\"%s\", %d)\n", fname, mode);
	log( "<SV:%s>\n", fname + rootlen );

	/* 使用されていないファイルポインタを探す */
	for(i = 0; i < MAX_PS2_FILES; i++)
		if(_ps2_fp[i] == NULL) break;

	if(i == MAX_PS2_FILES) goto err_1;
	if(NULL == (_ps2_fp[i] = fopen(fname, modestr[mode]))) goto err_1;

	free(fname);

	packet[0] = 0x01;
	packet[1] = 0x00;
	packet[2] = i;
	for(i = 3; i < PACKET_SIZE; i++) packet[i] = 0x00;
	return sv_send_packet(fd, packet);
  
err_1:
	free(fname);
err_0:
	return sv_send_error(fd, ERR_COULDNOTOPEN);
}

/*
 * ファイルのクローズ
 */
static int sv_close(int fd)
{
	int ps_fd;

	ps_fd = _cmd_packet[2];

	DEBUG("usbClose(%d)\n", ps_fd);

	if( _ps2_fp[ps_fd] == NULL ){
		error_return( 2 );
	}

	fclose(_ps2_fp[ps_fd]);
	_ps2_fp[ps_fd] = NULL;  /* 使用が終ったファイルポインタは NULL にしておく */
	return sv_send_success(fd);
}

//#define CHECK_TIME 1
#ifdef CHECK_TIME
#include <sys/time.h>
#endif

/*
 * ファイルの読み込み
 */
static int sv_read(int fd)
{
	unsigned char packet[ PACKET_SIZE ];
	int psfd;
	unsigned long size;
	size_t trans;
	int status;
	int i;
	long now, term, left;
#ifdef CHECK_TIME
	struct timeval t1, t2;
	int sendsize;
#endif

	psfd = _cmd_packet[2];

	if( _ps2_fp[psfd] == NULL ){
		error_return( 3 );
	}

	size = sv_get_long(_cmd_packet);

	DEBUG("usbRead(%d, <*buffer>, 0x%08x)\n", psfd, size);

	now = ftell(_ps2_fp[psfd]);
	fseek(_ps2_fp[psfd], 0L, SEEK_END);
	term = ftell(_ps2_fp[psfd]);
	fseek(_ps2_fp[psfd], now, SEEK_SET);
	left = term - now;

	/* "読める" サイズが "読みたい" サイズより小さい場合,
	"読める" サイズを返す。 */
	if(left < size) size = left;
	sv_set_long(size, packet);
	packet[0] = 0x01;
	packet[1] = 0x00;
	packet[2] = 0x00;
	packet[3] = 0x00;

	sv_send_packet(fd, packet);
	sv_recv_packet(fd, packet);	// OK

#ifdef CHECK_TIME
	gettimeofday( &t1, NULL );
	sendsize = size;
#endif

	/* 指定サイズ分を転送する */
	status = 0;
	while(size > 0)
		{
			trans = (size > TRANS_BUFSIZE) ? TRANS_BUFSIZE : size;
			if(trans > fread(_trans_buf, 1, trans, _ps2_fp[psfd])){
				status = -1;
			}
			if(trans > write(fd, _trans_buf, trans)){
				status = -1;
			}
			size -= trans;
			log( "%d     \r", size );
		}
	log( "  \r" );

	sv_recv_packet(fd, packet);

	if(packet[0] != 0x01 || packet[1] != 0x00 || status)
		return sv_send_error(fd, ERR_READ);

#ifdef CHECK_TIME
	gettimeofday( &t2, NULL );
	if( sendsize > TRANS_BUFSIZE ){
		int sd;

		sd = ( t2.tv_sec - t1.tv_sec ) * 1000000 + ( t2.tv_usec - t1.tv_usec );
		fprintf( stderr, "%f bytes/sec\n", sendsize * 1000000.0F / sd );
	}
#endif

DEBUG( "SEND SUCCESS\n" );

	return sv_send_success(fd);
}

/*
 * ファイルの書き込み
 */
static int sv_write(int fd)
{
	unsigned char packet[ PACKET_SIZE ];
	int psfd;
	unsigned long size;
	size_t trans;
	int status;
	int printf_flag = 0;

	psfd = _cmd_packet[2];
	if( ( psfd & 0xF0 ) == 0xF0 ){
		// PRINTF
		printf_flag = 1;
	} else {
		if( _ps2_fp[psfd] == NULL ){
			error_return( 3 );
		}
	}

	size = sv_get_long(_cmd_packet);

	DEBUG("usbWrite(%d, <*buffer>, 0x%08x)\n", psfd, size);

	sv_send_success(fd);

	/* 指定サイズ分を転送する */
	status = 0;
	while(size > 0)
		{
			trans = (size > TRANS_BUFSIZE) ? TRANS_BUFSIZE : size;
			if(trans > read(fd, _trans_buf, trans))
				status = -1;
			if( printf_flag == 0 ){
				if(trans > fwrite(_trans_buf, 1, trans, _ps2_fp[psfd]))
					status = -1;
			} else {
				fputs( _trans_buf, stderr );
			}
			size -= trans;
			log( "%d     \r", size );
		}
	log( "  \r" );

	if(status) return sv_send_error(fd, ERR_WRITE);
	return sv_send_success(fd);
}

/*
	PUTS
*/

static int sv_puts( int fd )
{
	int size;
	int status;
	size = sv_get_long( _cmd_packet );
	status = 0;
	if( size > read( fd, _trans_buf, size ) ){
		status = -1;
	} else {
		fputs( _trans_buf, stderr );
	}
	return status;
}

/*
 * ファイルのシーク
 */
static int sv_lseek(int fd)
{
	unsigned char packet[ PACKET_SIZE ];
	unsigned long raw_offset;
	long offset;
	int psfd;
	int where;
	int pos;
	int i;

	psfd = _cmd_packet[2];
	where = _cmd_packet[3];

	offset = sv_get_long(_cmd_packet);

	DEBUG("usbLseek(%d, %d, %d)\n", psfd, offset, where);

	if( _ps2_fp[psfd] == NULL ){
		error_return( 2 );
	}

	switch(where)
		{
		  case PSEEK_SET: pos = SEEK_SET; break;
		  case PSEEK_CUR: pos = SEEK_CUR; break;
		  case PSEEK_END: pos = SEEK_END; break;
		}
	fseek(_ps2_fp[psfd], offset, pos);

	raw_offset = ftell(_ps2_fp[psfd]);

	sv_set_long(raw_offset, packet);
	packet[0] = 0x01;
	packet[1] = 0x00;
	packet[2] = 0x00;
	packet[3] = 0x00;

	return sv_send_packet(fd, packet);
}

static int sv_wait_stat( int fd )
{
	int stat;

	for( ;; ){
		if( ioctl( fd, LNQ_IOCTL_CHECK, &stat ) < 0 ){
			printf( "<unpluged.>\n" );
			return -1;
		}
		if( stat ){
			break;
		}
		usleep( 10 );
	}
	return 0;
}

int svMain(char *dev)
{
	int fd;
	int doing;
	int status;
	int i, stat;
	int unplug_flag;
	struct lnQ_port_info info;

	printf("Device  : %s\n", dev );
	printf("RootPath: %s\n", _ps2Root);

	/* ファイルエリアの初期化 */
	for(i = 0; i < MAX_PS2_FILES; i++) _ps2_fp[i] = NULL;
	unplug_flag = 0;

OPEN_RETRY:
	/* デバイスファイルのオープン */
	if(-1 == (fd = open(dev, O_RDWR))){
		extern int errno;
		if( errno == ENODEV ){
			if( unplug_flag == 0 ){
				log( "<Waiting for plugging USB cable..>\n" );
			}
			unplug_flag = 1;
			sleep( 1 );
			goto OPEN_RETRY;
		}
		err( "Could not open device '%s'.(%d)\n", dev, errno);
		return -1;
	}

	if( setjmp( jmp_env ) != 0 ){
		int stat;

		log( "<ps2fsv: Fatal Error.>\n" );
		log( "<Please Power Off DebuggingStation and power on again.>\n" );

		while( ioctl( fd, LNQ_IOCTL_CHECK, &stat ) >= 0 ){
			usleep( 10 );
		}
		unplug_flag = 0;
		goto OPEN_RETRY;
	}

	log( "<device open.>\n" );
	unplug_flag = 0;

#if 0
	{
		ioctl( fd, LNQ_IOCTL_GETPORT, &info );

		printf( "busnum %d\n", info.busnum );
		printf( "level  %d\n", info.level );
		printf( "parent %d\n", info.parent );
		printf( "port   %d\n", info.port );
		printf( "devnum %d\n", info.devnum );
	}
#endif
	doing = 0;  /* この変数の値が 0 であるかぎり、リクエストを受け取り続ける */

	while(!doing)
		{
			ioctl( fd, LNQ_IOCTL_SYNCSTAT, &stat );	// 一発ダミー呼び
			/* クライアントからのリクエストパケットを受け取る */
			do {
				if( sv_wait_stat( fd ) < 0 ){
					unplug_flag = 1;
					goto ERROR_END;
				}
				stat = sv_recv_packet(fd, _cmd_packet);
				if( stat != 0 ){
					extern int errno;
					printf( "ERROR %s\n", strerror( errno ) );
					error_return( 1 );
				}
			} while(stat); /* タイムアウトなどのエラーでリクエストを
						   受け損ねたときは、拾い直す */

			/* 最初の 1 バイトの値が 1 で無い場合、エラーを返す */
			if(_cmd_packet[0] != SYNC) sv_send_error(fd, ERR_COMMAND);

			switch(_cmd_packet[1])
				{
				  case CMD_INIT:
//					fprintf(stderr, "CMD_INIT\n"); fflush(stderr);
					status = sv_init(fd);
//					fprintf(stderr, "finish(INIT)\n"); fflush(stderr);
					break;
				  case CMD_OPEN:
//					fprintf(stderr, "CMD_OPEN\n"); fflush(stderr);
					status = sv_open(fd);
					break;
				  case CMD_CLOSE:
//					fprintf(stderr, "CMD_CLOSE\n"); fflush(stderr);
					status = sv_close(fd);
					break;
				  case CMD_READ:
//					fprintf(stderr, "CMD_READ\n"); fflush(stderr);
					status = sv_read(fd);
					break;
				  case CMD_WRITE:
//					fprintf(stderr, "CMD_WRITE\n"); fflush(stderr);
					status = sv_write(fd);
					break;
				  case CMD_LSEEK:
//					fprintf(stderr, "CMD_LSEEK\n"); fflush(stderr);
					status = sv_lseek(fd);
					break;
				  case CMD_PUTS:
					status = sv_puts( fd );
					break;
				}
#if 0 
			if(!status){
				fprintf(stderr, "command successfull.\n");
			}
#endif
		}
ERROR_END:
	sv_all_close();   /* まだクローズされていないものは全てクローズ */
	close( fd );

	goto OPEN_RETRY;

	return 0;
}

/*
 * PlayStation2 から参照できるディレクトリを指定
 */
int svSetRootPath(char *path)
{
	size_t len;
	int add = 0;

	if(_ps2Root != NULL) free(_ps2Root);

	len = strlen(path);

	/* 最後が / で終っていなければ一文字分バッファ長を追加 */
	if(*(path + len - 1) != '/')
		{
			add = 1;
			len++;
		}

	if(NULL == (_ps2Root = malloc(len + 1))) return -1;

	strcpy(_ps2Root, path);
	if(add) strcat(_ps2Root, "/");
	return 0;
}

/* 使用メモリ資源全開放 */

void svRelease(void)
{
	if(NULL != _ps2Root) free(_ps2Root);
	sv_all_close();
}
