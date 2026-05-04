/*
	KCEJ SIF MANAGER
		関数プロトタイプほか

	1999/11/17	K.Uehara
	$Id: jsifman.h,v 1.4 2000/06/26 08:52:25 usr01475 Exp $
*/

#ifndef _SIF_H_DEFS
#include "sif.h"
#endif
#ifndef _SIFCMD_H_DEFS
#include "sifcmd.h"
#endif

/* この関数を使うモジュールのID番号 */
/* お互いに相手側のIDを使って呼び出す */

// IOP側
enum {
	IOP_SIF_RVMAN,
	IOP_SIF_SD_SET,
	IOP_SIF_CDBIOS,
	IOP_SIF_USBFS,
	IOP_SIF_CALLBACK_FUNC_MAX
};

// EE側
enum {
	EE_SIF_RVMAN,
	EE_SIF_CDBIOS,
	EE_SIF_USBFS,
	EE_SIF_CALLBACK_FUNC_MAX
};

#ifdef EE
#define SIF_CALLBACK_FUNC_MAX	EE_SIF_CALLBACK_FUNC_MAX
#define SIF_RVMAN				EE_SIF_RVMAN
#endif
#ifdef IOP
#define SIF_CALLBACK_FUNC_MAX	IOP_SIF_CALLBACK_FUNC_MAX
#define SIF_RVMAN				IOP_SIF_RVMAN
#endif

/* コールバック関数のための構造体宣言 */

typedef struct {
	unsigned int psize:8;
	unsigned int dsize:24;
	unsigned int daddr;
	unsigned int fcode;
	unsigned int fno;		// ここまでは、sceSifCmdHdr ( opt->fno )

	int data[ 4 * 6 ];		// 最大数
} SIF_CALLBACK_PARAM;

typedef sceSifCmdHdr  SIF_PACEKT_HEADER;

/* 使うシステムレジスタ */

#define SIF_SYSREG_IDSET	8

/* 返り値マネージャ (RVMAN) */

typedef struct {
	int id;
	int data1;
	int data2;
	int data3;
} SIF_RV_QUEUE;

enum {
	SIF_RV_RETURN_VALUE,
	SIF_OTHER_MEM_GET,
};

#define SIF_RV_VALFLAG					(0x80000000)

extern inline int sif_rv_check_queue( SIF_RV_QUEUE *queue )
{
	return ( *( volatile int * )&queue->id & SIF_RV_VALFLAG );
}

#if 0 //BP_PS2
extern inline int sif_check_setup_module( int module_id )
{
	return ( sceSifGetSreg( SIF_SYSREG_IDSET ) & ( 1 << module_id ) );
}
#endif

#define MAX_PACKET_DATA_SIZE	( 80 - sizeof( int ) )

/* ----------------------------------------------------- */

int sif_init( void );	// EE のみ
int sif_init_rv_man( void );	// EE のみ

int sif_set_callback_func( int id, void *func, void *data );

int sif_send_packet_and_data( int id, int fno, void *packet, int psize
								, void *dest, void *src, int size );

int i_sif_send_packet_and_data( int id, int fno, void *packet, int psize
								, void *dest, void *src, int size );

#define sif_send( _id, _fno ) \
	sif_send_packet_and_data( _id, _fno, NULL, 0, NULL, NULL, 0 )
#define sif_send_packet( _id, _fno, _packet, _psize ) \
	sif_send_packet_and_data( _id, _fno, _packet, _psize, NULL, NULL, 0 )

#define i_sif_send( _id, _fno ) \
	i_sif_send_packet_and_data( _id, _fno, NULL, 0, NULL, NULL, 0 )
#define i_sif_send_packet( _id, _fno, _packet, _psize ) \
	i_sif_send_packet_and_data( _id, _fno, _packet, _psize, NULL, NULL, 0 )

int sif_send_mem( void *dst, void *src, int size );
int i_sif_send_mem( void *dst, void *src, int size );

SIF_RV_QUEUE *sif_rv_get_queue( void );
int sif_rv_call_func( int id, int fno, SIF_RV_QUEUE *queue );
int sif_rv_return_value( SIF_RV_QUEUE *queue );
int i_sif_rv_return_value( SIF_RV_QUEUE *queue );
void sif_rv_release_queue( SIF_RV_QUEUE *queue );
SIF_RV_QUEUE *sif_get_mem( void *dst, void *src, int size );
#define sif_get_mem_end( _que )	sif_rv_release_queue( _que )
int sif_check_status( int id );

int sif_send_direct( int id, int fno, void *packet, int psize
									, void *dst, void *src, int size );
int i_sif_send_direct( int id, int fno, void *packet, int psize
									, void *dst, void *src, int size );
