/*
	cdbios_in.h
		KCEJ CD/DVD MANAGER internal common header
		2000/03/23 K.Uehara
	$Id: cdbios_in.h,v 1.4 2000/12/11 06:31:46 worm Exp $
*/

#include "cdbios.h"

/*
	cdbiosステータス共通ワーク
		情報はIOP側からEE側に割り込みなしで送られる
*/

// 64バイト境界に載せる
typedef struct {
	int status;			// 状態
	int exec_com;		// 実行中のタスク
	int last_err;		// 最後に発生したエラーコード
	int seek_pos;
	CDBIOS_READ_STATUS read;
	int param[ 16 - ( 4 + 5 ) ];
} CDBIOS_TASK_STATUS __attribute__((aligned(64)));

/* param の内容 */

/* read command は、cdbios.h */

// exec_com

// EE->IOP
enum {
	CDBIOS_COMMAND_WAIT,		// 実行待ち
	CDBIOS_COMMAND_READ,		// read start
	CDBIOS_COMMAND_READSYNC,	// read callback
	CDBIOS_COMMAND_SEEK,
	CDBIOS_COMMAND_STOP,
	CDBIOS_COMMAND_GETTIME,
	CDBIOS_COMMAND_SRCHFILE,
	CDBIOS_COMMAND_CHANGEPARAM,
	CDBIOS_COMMAND_INIT,
	CDBIOS_HD_COMMAND_SET_FILE,		// for HD_EMU
};

// IOP->EE
enum {
	CDBIOS_EE_READ_CALLBACK = 1,		// READコールバック
};


#ifdef MASTER
#define printf( fmt, args... )
#define Kprintf( fmt, args... )
#endif
