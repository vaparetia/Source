/*
	mouth.h
	くちパク 制御フラグ定義
	2000/07/05 K.Sigeno
	$Id: mouth.h,v 1.1.1.3 2002/11/19 11:49:48 Yoshizawa1 Exp $
*/
/*タスク状態*/
enum{
	OFF ,		/*無登録あるいは登録失敗*/
	ACT_END,		/*次フレームでDIE実行 */
	GET_RES ,	/*登録成功*/
	ACTIVE		/*動作中*/
};
#define	FACE_ANIM_NUM	(3)
#define	FACE_INTERP		(6)
