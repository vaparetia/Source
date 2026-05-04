/*
	puppetik.h
	死体兵用ＩＫルーチン用ヘッダ

*/
/*
	＜使い方説明＞
	●条件
	　・モーション再生を行っている人型キャラにたいしてのみ

	●初期化
	void *TAKABE_MakePuppetIK( CONTROL *ctrl, OBJECT *body );
	CONTROL	*ctrl ;		ＩＫを行うキャラのコントロール	
	OBJECT	*body ;		ＩＫを行うキャラのオブジェクト

	初期化に成功するとワークへのポインタが取得でき、以後このワークへのポインタを使用して
	動作させる

	●開放
	void TAKABE_FreePuppetIK( void *work );
	void	*work ;		確保したＩＫ用ワーク

	●実行方法
	1.	GM_ActMotion()にてモーション再生を行う
	2.	GM_ActControl()にてコントロール処理
	4.	GM_ActObject2()にて一度モーションをモデルに適用する
	3.	TAKABE_ActPuppetIK()関数を実行

	int TAKABE_ActPuppetIK( void *work );
	void	*work ;		確保したＩＫ用ワーク

	ＩＫの処理を行う
	ＩＫ処理が静止状態になると１を返す（通常は０を返す）


	//具体的なＩＫのコントロールは以下の関数で制御する
	//void TAKABE_CmdPuppetIK( void *work, int command, int param );
	//void	*work ;		確保したＩＫ用ワーク
	//int		command ;	コマンド
	//int		param ;		コマンドに必要なパラメータ
	//ＩＫへのコマンド実行はワーク内のステータスを変更するだけで、
	//TAKABE_ActPuppetIK()を呼ばない限り反映されないので注意。

	●ユーティリティ関数によるステータスのオート管理について
	void TAKABE_UtilPuppetIK_AutoConfigOfEnemey( void *work, int mode );
	void	*work ;
	int		mode ;		敵兵状態（０：倒れ、１：足引きずり、２：腕引きずり、－１：それ以外）
	敵兵専用関数（倒れ、足引きずり、腕引きずりに対応）

	●その他
	void TAKABE_UtilPuppetIK_CopyStatus( void *dst_work, void *src_work );
	void	*dst_work ;		確保済みの新しいＩＫワーク
	void	*src_work ;		オリジナルのＩＫワーク
	ＩＫのステータスを移す

	void TAKABE_UtilPuppetIK_Piku( void *work, int joint_num );
	void	*work ;
	int		joint_num ;		ピクつかせる関節番号（-1で全体がピクつく）
	体（全５箇所）の各部分をピクつかせる

	

*/

#ifndef __PUPPET_IK_H__
#define __PUPPET_IK_H__

/* ＩＫ制御コマンド */
enum {
	TAKABE_PUPPETIK_START			= 1,	/* ＩＫ動作開始（これとは別に各個所のＩＫをＯＮにしなければならない） */
											/* ＩＫ動作ＯＦＦの直後のみＩＫ基準位置と基準ポーズの再更新を行う */
	TAKABE_PUPPETIK_IDLE			= 2,	/* モーション設定のみ */
	TAKABE_PUPPETIK_END				= 3,	/* ＩＫ動作終了（すべての関節のＩＫはＯＦＦにされる） */
	TAKABE_PUPPETIK_INIT_ROT		= 4,	/* 基準ポーズ初期化 */
	TAKABE_PUPPETIK_INIT_TRG		= 5,	/* 基準関節位置初期化 */
	TAKABE_PUPPETIK_ENABLE_ARM		= 6,	/* 両腕ＩＫのＯＮ・ＯＦＦ（１でＯＮ、０でＯＦＦ） */
	TAKABE_PUPPETIK_ENABLE_LEG		= 7,	/* 両足ＩＫのＯＮ・ＯＦＦ（１でＯＮ、０でＯＦＦ） */
	TAKABE_PUPPETIK_ENABLE_BODY		= 8,	/* 体ＩＫのＯＮ・ＯＦＦ（１でＯＮ、０でＯＦＦ） */
	TAKABE_PUPPETIK_DEBUG_MODE		= 100,	/* デバッグモードＯＮ・ＯＦＦ */
};

#ifndef PUPPETIK_SOURCE
void *TAKABE_MakePuppetIK( CONTROL *ctrl, OBJECT *body );
int TAKABE_ActPuppetIK( void *work );
void TAKABE_FreePuppetIK( void *work );
void TAKABE_CmdPuppetIK( void *work, int command, int param );
void TAKABE_UtilPuppetIK_AutoConfigOfEnemey( void *work, int mode );
void TAKABE_UtilPuppetIK_CopyStatus( void *dst_work, void *src_work );
void TAKABE_UtilPuppetIK_Piku( void *work, int joint_num );
#endif



#endif
