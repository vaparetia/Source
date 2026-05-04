KCEJ SIF MANAGER
	$Id: readme.txt,v 1.5 2000/06/26 08:52:26 usr01475 Exp $

SCEのsifcmd, sifrpcを使ったIOPへの通信ライブラリ。
sifcmdはユーザーバッファを1つしか持たないため、IOP側の内制モジュールは
このモジュールを共通で使用する。

sifcmd, sifrpcそのものは、SCEのライブラリ内ですでに使用されているため、
基本的な方針としては、返り値のいらないコマンドはsifcmdを使用して転送、
返り値の必要、または処理の終了待ちが必要なコマンドはsifrpcを使用する。

sifcmdのユーザーバッファを使用。システムレジスタは以下のように使用する。
	REG 8 : リクエストを受け付けるID番号に対応したビットが１になる。

このモジュールを使用する時は、sifcmdは使用禁止。
(sceSifSendCmdは使用しても問題ない)
sifrpcについては、同時に使用することができる。

パケットに関しては、ユーザーデータが80-4バイトまでのもののみ
ライブラリでサポート。それ以上のものは呼出し側で確保する必要があり。
(sif_send_directを使用する)
ただし、呼出し側で確保した場合、IOP側では全く送信したかどうかのチェックを
行なわないので、自前で行なう必要がある。

# -----------------------------
# 設定用 ( EE, IOP 共通 )

int sif_set_callback_func( int id, void *func, void *data );

呼出番号idにコールバック関数funcを登録する。
dataはfuncに渡されるポインタ引数。
funcは以下の形の関数で、割り込み禁止区間で呼ばれる。

typedef struct {
	unsigned int psize:8;
	unsigned int dsize:24;
	unsigned int daddr;
	unsigned int fcode;
	unsigned int fno;		// ここまでは、sceSifCmdHdr ( opt->fno )

	int data[ 112 / sizeof( int ) ];		// 最大数
} SIF_CALLBACK_PARAM;

void callback_func( SIF_CALLBACK_PARAM *hd, void *data );

dはsceSifCmdHdrから始まる、EEから送られてきたデータ。最大112(16*7)バイト。

dataは、sif_set_callback_funcの引数のdataがそのまま渡される。
(GPの設定ができないため、このポインタ経由でやらないと駄目のようだ）

# -----------------------------
# データ転送用（non-block, ステータスはなし) ( EE, IOP 共通 )

内部にsceSifCmdHdr構造体を持っているので、プログラム側では用意する必要はない。

# -- 割り込み発生型
返り値はDMA転送のID。sceSifDmaStatで終了したかどうかがわかる。
#define sif_check_status( id )		sceSifDmaStat( id )

int sif_send( int id, int fno );
int i_sif_send( int id, int fno );
32ビットのデータを転送して、向うに割り込みを発生させる。

int sif_send_packet( int id, int fno, void *packet, int size );
int i_sif_send_packet( int id, int fno, void *packet, int size );
fnoとsize バイト(MAX16*6 = 96Byte/16byte単位)をパラメータとして割り込みを発生させる。

int sif_send_packet_and_data( int id, int fno, void *packet, int psize
									, void *dst, void *data, int size );
int i_sif_send_packet_and_data( int id, int fno, void *packet, int psize
									, void *dst, void *data, int size );
fnoとpsizeバイトのパケットと、dataから始まるsizeバイト(MAX1024*1024-16)の
メモリ領域を相手のdstに転送する。

以上のデータはどうやらEEはライブラリ内部のバッファに送られるらしい。
IOPはライブラリ内部には転送されないため、実際の送出が完了するまで
そのアドレスは使用できない。

int sif_send_direct( int id, int fno, void *packet, int psize
									, void *dst, void *data, int size );
int i_sif_send_direct( int id, int fno, void *packet, int psize
									, void *dst, void *data, int size );
呼出し側でパケットを確保するタイプ。先頭に、SIF_PACEKT_HEADERの実体をもつ
構造体が必要。SIF_PACEKT_HEADERのなかは、この関数内で設定する。

# -- 直接転送型
int sif_send_mem( void *dst, void *src, int size );
int i_sif_send_mem( void *dst, void *src, int size );
自分のメモリ空間のsrcから相手のメモリ空間のdstへsizeバイト転送する。
むこうにも割り込みは発生しない。
返り値はDMA転送のID。sceSifDmaStatで終了したかどうかがわかる。

# -----------------------------
# 返り値マネージャ ID = 0

RPCを使わずに、EEからIOPにリクエストを送って、EEに返り値を渡すことを
想定している。EEはポーリングでとれればいいという思想。
コールバックが必要な場合は、上のデータ転送用関数群を使えばいい。

処理の流れは以下の通り。

EE : 返り値QUEUEを取得。IDを得る。IDをデータに入れて、IOPにsif_sendする。
IOP: 処理を行なう。返り値IDを使って返り値をEEの返り値マネージャに送る。
EE : 返り値QUEUEを監視して、IOPからデータが来たらもらい、返り値QUEUEを返却する。

EE 側の返り値QUEUE

typedef struct {
	int id;			// 31Bit が返ってきたかどうかのフラグ
	int data1;
	int data2;
	int data3;
} SIF_RV_QUEUE;

SIF_RV_QUEUE *sif_rv_get_queue( void );
返り値QUEUEを取得。NULLならエラー。

int sif_rv_call_func( int id, int fno, SIF_RV_QUEUE *queue );
向う側の処理を呼び出す。

int sif_rv_return_value( SIF_RV_QUEUE *queue );
int i_sif_rv_return_value( SIF_RV_QUEUE *queue );
向う側に値を返す。
queueは、callbackのheader->dataをキャストして取得できる。。

void sif_rv_release_queue( SIF_RV_QUEUE *queue );
queueを解放する。

# メモリ転送(util)
SIF_RV_QUEUE *sif_get_mem( void *dst, void *src, int size );
RVを使って、相手のメモリ空間のsrcから自分のメモリ空間のdstへsizeバイト転送する。
終了判定は、返り値のSIF_RV_QUEUE *で見る。
queue を解放する必要があることに注意。
