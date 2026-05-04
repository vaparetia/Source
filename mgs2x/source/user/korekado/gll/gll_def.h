/*
	gll_def.h
	ゴルルゴン

	2002/04/04 Y.Korekado
	$Id: gll_def.h,v 1.3 2002/11/23 12:42:27 Yoshizawa1 Exp $
*/

/* GLL_GAME_STATUS */
extern int GLL_GAME_STATUS ;
#define GLL_GS_SEARCH_LIGHT_IN	0x00000001	/*メカゲノラサーチライトで発見*/
#define GLL_GS_HEAD_GROUNDED	0x00000002	/*頭が地面に落ちた*/
#define GLL_GS_HEAD_REMOVE		0x00000004	/*頭が落ちて離れていく*/
#define GLL_GS_HEAD_BREAK		0x00000008	/*頭が壊れた*/
#define GLL_GS_SPOT_BREAK		0x00000010	/*スポットライト破壊*/
#define GLL_GS_TUB_HEAD			0x00000020	/*たらいが頭に落ちた*/
#define GLL_GS_ACHOO			0x00000040	/*くしゃみ発射*/
#define GLL_GS_TUB_KILL			0x00000080	/*たらい消える*/
#define GLL_GS_TUB_EFE_KILL		0x00000100	/*たらいエフェクト発動*/
#define GLL_GS_CHAFF_CLEAR		0x00000200	/*メカゲノラ、チャフでクリア*/
#define GLL_GS_TUB_DORP			0x00000400	/*たらい落ちる*/
#define GLL_GS_ACHOO_TIME		0x00000800	/*くしゃみ発射中*/


/* GLL_COM_STATUS */
extern	int	GLL_COM_STATUS ;
#define	GLL_COM_DETECT	0x00000001
extern	int	GLL_ACHOO_TIME ;

/* gllcom */
extern void	GLLCOM_Detect( CONTROL	*ctrl ) ;

/*missile.c*/
extern void GllMissileInitID(void ) ;
extern void	*NewGllMissile( FVECTOR	*pos, int route ) ;


/*shelf.c tub.c*/
#define GLL_SHELF_BREAK		0x00000001
#define GLL_SHELF_DAMAGE	0x00000002
#define GLL_SHELF_BOMB		0x00000004
#define GLL_SHELF_DESTROY	0x00000008
#define GLL_SHELF_TUB_KILL	0x00000010
#define GLL_SHELF_KILL_EFE	0x00000020


/*以下はゴルルゴン専用、ビームが常に１つしか存在しないことが前提 */
extern	FMATRIX	GllEyebeemWorld ;
extern	int		GllEyebeemFlag ;
#define GLL_EYEBEEM_MOVE			(0x00000001)
#define GLL_EYEBEEM_STOP			(0x00000002)
#define GLL_EYEBEEM_ORDER_STOP		(0x00010000)
#define GLL_EYEBEEM_ORDER_BOOK_STOP	(0x00020000)

enum {
	RED_AT_BIG = 0x20,
	RED_QE_BIG,
	YLW_AT_BIG,
	YLW_QE_BIG,
	WHT_AT_BIG,
	WHT_QE_BIG,
	PNK_AT_BIG,
	RED_AT_GENORA,
	RED_QE_GENORA,
	WHT_AT_GENORA,
	WHT_QE_GENORA,
	HEART_GENORA,
} ;
