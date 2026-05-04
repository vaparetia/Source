#ifndef _linQio_h_
#define _linQio_h_

#include <linux/usb.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>

#define RETRY_TIMES    8
#define TRANS_BUFSIZE  (1024*32)
#define PACKET_SIZE 8


#define SYNC 0x01

#define CMD_INIT  0x01   /* 初期化   */
#define CMD_OPEN  0x02   /* オープン */
#define CMD_CLOSE 0x03   /* クローズ */
#define CMD_READ  0x04   /* 読み込み */
#define CMD_WRITE 0x05   /* 書き込み */
#define CMD_LSEEK 0x06   /* 移動     */

#define CMD_PUTS  0x10

/*
 * エラー値
 */
#define ERR_COMMAND      0x01
#define ERR_PROTOCOL     0x02
#define ERR_COULDNOTOPEN 0x03
#define ERR_READ         0x04
#define ERR_WRITE        0x05

/* オープンのモードを表す値 */
#define PS2_READ   1
#define PS2_WRITE  2
#define PS2_RDWR   3

/*
 * SEEK の際の値
 */
#define PSEEK_SET  SEEK_SET
#define PSEEK_CUR  SEEK_CUR
#define PSEEK_END  SEEK_END


#ifndef _linQio_c_
#define EXT extern
#define INIT(n)
#else
#define EXT
#define INIT(n)  = n
#endif /* _linQio_c_ */



#undef EXT
#undef INIT


int svMain(char *dev);
int svSetRootPath(char *path);
void svRelease(void);

#endif /* _linQio_h_ */
