/*
   ストリームデータ作成ルーチンヘッダ
*/

#define TRUE	1
#define FALSE	0

#define SECTOR_SIZE 2048

enum {
	CHANK_TYPE_FREE = 0,		/* 使用終了 */

	CHANK_TYPE_SYSTEM = 0x10,	/* システムデータ */
	CHANK_TYPE_END = 0xf0,		/* このストリームデータの終端 */
	CHANK_TYPE_BUFEND = 0xff	/* バッファ終端 */
};

