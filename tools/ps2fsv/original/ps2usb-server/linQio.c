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

#include <fcntl.h>

#define LNQ_IOCTL_CLEANUP _IO('U', 0x10)
#define LNQ_IOCTL_GETPORT _IO('U', 0x11)

#define _linQio_c_
#include "config.h"
#include "linQio.h"

#ifndef _DEBUG_
#define DEBUG(args...)
#else
#define DEBUG(args...)  fprintf(stderr, args);
#endif /* _DEBUG_ */


#define LNQ_IOCTL_RESET   _IO('U', 0x10)
#define LNQ_IOCTL_GETPORT _IO('U', 0x11)


/* コマンドパケットバッファ */
static unsigned char _cmd_packet[ PACKET_SIZE ];

/* PS2 のリクエストで操作されるファイル */
static FILE * _ps2_fp[ MAX_PS2_FILES ];

/* 送受信で使用されるバッファ */
static unsigned char _trans_buf[ TRANS_BUFSIZE ];


/* Root ディレクトリとなるディレクトリを保持 */
static char * _ps2Root = NULL;

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

  recieved_size = read(fd, packet, PACKET_SIZE);

  printf("Rcv:");
  for(i = 0; i < recieved_size; i++)
    printf(" %02x", packet[i]);
  printf("\n");
  /* fflush(stdout); */

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
  printf("Snd:");
  for(i = 0; i < sended_siz; i++)
    printf(" %02x", packet[i]);
  printf("\n");

  fprintf(stderr, "OK. packet = %d bytes\n", sended_siz);

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
  fprintf(stderr, "success sended.\n");
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
  char *modestr[] = { 0, "rb", "wb", "rwb" };
  int i;

  mode = _cmd_packet[2];

  len = sv_get_long(_cmd_packet);
  rootlen = strlen(_ps2Root);

  DEBUG("file name len = %d\n", len);
  
  if(NULL == (fname = malloc(rootlen + len + 1))) goto err_0;
  if(sv_send_success(fd)) return -1;
  
  strcpy(fname, _ps2Root);
  
  DEBUG("fd(%d)  len(%d)\n", fd, len);

  rdlen = read(fd, fname + rootlen, len);

  DEBUG("fname read: %d characters.\n", rdlen);

  if(len != rdlen) goto err_1;

  len += rootlen;
  fname[len] = 0;

  DEBUG("usbOpen(\"%s\", %d)\n", fname, mode);

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

  fclose(_ps2_fp[ps_fd]);
  _ps2_fp[ps_fd] = NULL;  /* 使用が終ったファイルポインタは NULL にしておく */
  return sv_send_success(fd);
}

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

  psfd = _cmd_packet[2];

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

  sv_recv_packet(fd, packet);

  /* 指定サイズ分を転送する */
  status = 0;
  while(size > 0)
    {
      trans = (size > TRANS_BUFSIZE) ? TRANS_BUFSIZE : size;
      if(trans > fread(_trans_buf, 1, trans, _ps2_fp[psfd]))
	status = -1;
      if(trans > write(fd, _trans_buf, trans)) status = -1;
      size -= trans;
    }

  sv_recv_packet(fd, packet);

  if(packet[0] != 0x01 || packet[1] != 0x00 || status)
    return sv_send_error(fd, ERR_READ);

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

  psfd = _cmd_packet[2];

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
      if(trans > fwrite(_trans_buf, 1, trans, _ps2_fp[psfd]))
	status = -1;
      size -= trans;
    }

  if(status) return sv_send_error(fd, ERR_WRITE);

  return sv_send_success(fd);
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

int svMain(char *dev)
{
  int fd;
  int doing;
  int status;
  int i, stat;

  /* ファイルエリアの初期化 */
  for(i = 0; i < MAX_PS2_FILES; i++) _ps2_fp[i] = NULL;

  /* デバイスファイルのオープン */
  if(-1 == (fd = open(dev, O_RDWR)))
    {
      fprintf(stderr, "Could not open device '%s'.\n", dev);
      return -1;
    }

  doing = 0;  /* この変数の値が 0 であるかぎり、リクエストを受け取り続ける */

  while(!doing)
    {
      /* クライアントからのリクエストパケットを受け取る */
      do {
	stat = sv_recv_packet(fd, _cmd_packet);
      } while(stat); /* タイムアウトなどのエラーでリクエストを
			受け損ねたときは、拾い直す */

      /* 最初の 1 バイトの値が 1 で無い場合、エラーを返す */
      if(_cmd_packet[0] != SYNC) sv_send_error(fd, ERR_COMMAND);

      switch(_cmd_packet[1])
	{
	case CMD_INIT:
	  fprintf(stderr, "CMD_INIT\n"); fflush(stderr);
	  status = sv_init(fd);
	  fprintf(stderr, "finish(INIT)\n"); fflush(stderr);
	  break;
	case CMD_OPEN:
	  fprintf(stderr, "CMD_OPEN\n"); fflush(stderr);
	  status = sv_open(fd);
	  break;
	case CMD_CLOSE:
	  fprintf(stderr, "CMD_CLOSE\n"); fflush(stderr);
	  status = sv_close(fd);
	  break;
	case CMD_READ:
	  fprintf(stderr, "CMD_READ\n"); fflush(stderr);
	  status = sv_read(fd);
	  break;
	case CMD_WRITE:
	  fprintf(stderr, "CMD_WRITE\n"); fflush(stderr);
	  status = sv_write(fd);
	  break;
	case CMD_LSEEK:
	  fprintf(stderr, "CMD_LSEEK\n"); fflush(stderr);
	  status = sv_lseek(fd);
	  break;
	}
      
      if(!status)
	fprintf(stderr, "command successfull.\n");
      
    }

  sv_all_close();   /* まだクローズされていないものは全てクローズ */

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
  printf("RootPath: %s\n", _ps2Root);
  return 0;
}

/* 使用メモリ資源全開放 */

void svRelease(void)
{
  if(NULL != _ps2Root) free(_ps2Root);
  sv_all_close();
}
