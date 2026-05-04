/*
	enemem.h

	2001/05/2 Y.Korekado
	$Id: enemem.h,v 1.1.1.3 2002/11/19 11:44:27 Yoshizawa1 Exp $
*/

#ifndef __enememh_____
#define __enememh_____


/*----------------------------------------------*/
#define MAX_LOAD_MEM	(4)				/* 記憶ロード数 */
#define MAX_ENEMEM	(12*2*MAX_LOAD_MEM)	/* 平均敵兵人数＊死体＊記憶ロード数 */
#define MAX_LOAD		(255)			/* ロード */

#define LOADSUB( a, b ) (((b) + 128 - (a)) & 0x7f)
#define ENEMEM_ITEMS(a,b) ((a) | ((b)<<4))
#define ENEMEM_ITEM1(a) ((a)&0xf)
#define ENEMEM_ITEM2(a) (((a)&0xf0) >> 4)

#define FAINT_EVER_1		( (COUNT_VMODE(60*30)/3)-1 )	/* 星一つ */
#define FAINT_EVER_2		( (COUNT_VMODE(60*30)*2/3)-1 )	/* 星二つ */
#define FAINT_EVER_3		( COUNT_VMODE(60*30) )			/* 星三つ */

#define SLEEP_EVER_1		( COUNT_VMODE(60*60)-1 )	/* Z１こ*/
#define SLEEP_EVER_2		( COUNT_VMODE(60*60*2)-1 )	/* Z２こ*/
#define SLEEP_EVER_3		( COUNT_VMODE(60*60*3)-1 )	/* Z３こ*/
#define SLEEP_EVER_4		( COUNT_VMODE(60*60*4) )	/* Z４こ*/

/*----------------------------------------------*/
/* status */
enum {
	ENEMEM_ST_NORMAL,
	ENEMEM_ST_SLEEP,
	ENEMEM_ST_FAINT,
	ENEMEM_ST_HELL,
	ENEMEM_ST_HOLDUP,
	ENEMEM_ST_DEAD,
} ;

/* pose */
enum {
	ENEMEM_PS_NORMAL,
	ENEMEM_PS_FRONT,
	ENEMEM_PS_BACK,
	ENEMEM_PS_WALL,
	ENEMEM_PS_LOCKER,
} ;

/*----------------------------------------------*/
typedef struct _EneMem {
	int		stage ;		/* ステージ名 */
	int		hzx_id ;	/* マップ */
	int		name ;		/* 名前 */
	int		alive_name ;	/* 死体になる前の名前 */

	int		time ;		/* 時間 */
	float	x,y,z ;		/* 座標 */
	short	dir ;		/* 方向 */
	u_char	status ;	/* 状態 */
	u_char	pose ;		/* 体勢 */

	short	life ;		/* 体力 */
	short	faint ;		/* 気絶値 */
	short	pbreak ;		/* 部位ダメージ */
	u_char	item ;		/* アイテム */
	u_char	loadnum ;	/* ロード回数 */
} ENEMEM ;	/* 44byte */

extern int ENEMEM_GclSet( void ) ;
void ENEMEM_Set( ENEMEM *enm, int stage, int hzx_id, int name, FVECTOR *pos, int dir,
				int status, int pose, int time, int item,
				int life, int faint, int pbreak, int alive_name ) ;
extern void ENEMEM_Save( ENEMEM *save_enm ) ;
extern ENEMEM *ENEMEM_Load( int stage, int name ) ;
extern ENEMEM *ENEMEM_DeadLoad( int stage, int name ) ;
extern void ENEMEM_Init( int load_num ) ;
#endif
