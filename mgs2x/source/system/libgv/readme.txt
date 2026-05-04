libgv ゲーム環境ライブラリ
	1999/04/15 K.Uehara
	$Id: readme.txt,v 1.1.1.3 2002/11/19 11:42:45 Yoshizawa1 Exp $

■ファイル

makefile
readme.txt

libgv.h			ヘッダファイル
libgv.cnf		メモリコンフィグレーションなど定数宣言ファイル

gvd.c			gvデーモン
actor.c			アクター処理
memory.c		メモリ管理
pad.c			パッド管理
debug.c			デバッグ支援

■関数

- gvd.c -

void GV_StartDaemon( void );

GVライブラリの初期化と管理デーモンの起動。
プログラム中1回だけ呼ばれる。

- actor.c -

<ACTOR SYSTEM 制御>

void GV_InitActorSystem( void );

	ACTOR システムそのものの初期化。

void GV_DumpActorSystem( void );

	デバッグ用。すべてのACTORのリストをレベルごとに出力する。

void GV_ExecActorSystem( void );

	ACTOR システムの実行。

void GV_DestroyActorSystem( int kill );

	kill 以下のレベルのプロセスをすべて終了させる。

<個々のACTOR処理>

void GV_InitActor( int level, void *work );		// 互換

	work の先頭にあるACTOR構造体を levelのACTORとして登録する。

void *GV_NewActor( int level, int size );		// 互換

	sizeバイトのメモリを確保し、０クリアした後、先頭をACTOR構造体
	と見なして、levelのACTORとして登録する。

void *GV_CreateActor( int level, int class, int size )
	sizeバイトのメモリを確保し、０クリアした後、先頭をACTOR構造体
	と見なして、level, classのACTORとして登録する。

void GV_SetNamedActor( void *this, void *act, void *die, char *name );

	thisをACTORへのポインタと見なして、Act関数とDie関数を設定する。
	nameをデバッグ用に設定する。

void GV_SetActor( void *this, void *act, void *die );

	thisをACTORへのポインタと見なして、Act関数とDie関数を設定する。

void GV_DestroyActorQuick( void *this );

	thisのACTORを切り放し、Dieに登録された関数を実行する。
	また、thisに確保されたメモリを解放する。

void GV_DestroyActor( void *this );

	thisのACTORに終了要求を出す。
	ActにDestroyActorQuickをつなぐので、実際に終了するのは
	そのACTORが次に実行されるとき。

void GV_DestroyOtherActor( void *target );

	自分以外のACTORを殺すとき、リストを検索して存在するかどうか
	確認してからDestroyする。