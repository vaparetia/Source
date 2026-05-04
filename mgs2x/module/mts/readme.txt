MTS: EE用デバッグユーティリティ＆スレッド生成ライブラリ
	2000/05/19	K.Uehara
	$Id: readme.txt,v 1.10 2002/09/11 06:18:37 usr01475 Exp $

デバステ・T10000両方で使用できる例外ダンプと、スタックチェックを
機能として持つ、スレッド管理関数群<MTS>を用意した。
スレッド起動をこの関数で行なうことにより、デバステでも、
例外ダンプ・スタックチェックが可能になる。

file:
	mts.h
		MTS ヘッダ
	debugutil.h
		例外ダンプ、スタックチェックヘッダ
	mts.a
		ライブラリ
	mts_m.a
		マスター用ライブラリ。MTS_*系の関数から例外ダンプや
		スタックチェックを呼び出さない。printfも出力しない。
		<<必ずマスターはこちらを使用すること。>>

MTSを使用した場合は、DEBUG_*系の関数が暗黙に呼び出されるが、
MTSを使用しない場合でも直接呼び出すことで使用することができる。

例外ダンプに関しては、EE-kernelの非公開関数を使用して実現しているため、
EE-kernelの今後のバージョンアップの際に使用不能になる可能性がある。
＃作成時のEE-kernelのバージョンは、1.5.3。
（追記）ライブラリ1.6.0でSetDebugHandler関数がサポートされたが、
 これはeretでハンドラを呼び出すタイプのため、デバッガに戻れない。
 SetDebugHandler内部でも、mtsがやっていることとほぼ同じことをやっているので、
 mtsでは、とりあえず現行のやりかたのままでいく。

例外コールバックでは、syscallを使ったkernel命令の呼出しは一切できない。
（i系の関数もだめ／レジスタ値のカーネル内部ワークへの設定ができないため）

また、例外ダンプについては、ベクタを元のアドレスに戻せないため、
LoadExecで起動しなおす時には新しい方のプログラムで再設定する必要がある。
# MTSを使っているか、DEBUG_InitExceptionしないと駄目。
# 雑誌付録系のランチャーに戻る体験版のマスターもmts_m.aを使うのが無難。

なお、特にexception.c辺りでgccの拡張機能をかなり使っているため、
gcc以外の環境でのコンパイルはできない（と思う）

printfをオーバーライドする関係上、必ずlibcより前でリンクすること。

------ スタックトレースの読み方 --------

02/09/11より、例外ダンプ時に簡易スタックトレースが表示されるようになった。
（これにより、レジスタダンプを削った）

スタックトレースとはいっても、「スタックをサーチして関数内部を指しているらしい
アドレス値があったら表示する」だけ。従って、途中の関数で引数や変数に関数へのポインタを
使用した場合は、その値も表示してしまう。

どこが.textセクションかは各プログラムの実装によるため、
void MTS_SetIsTextFunc( int (*func)( void *addr ) );
で、そのアドレスがテキストエリアかどうかを返す関数を登録しないと
この機能は働かない。

また、「スタック上の戻りアドレス」なので、デバッガで表示されるアドレスとは
+8 ずれているので注意が必要。(戻りアドレスは+8になるので)

---------------------------------------

関数は以下の通り。

------------------ mts.h ---------------------
-- MTS系

カーネルのマルチスレッド機能と、下の例外・スタック系の関数を使って、
EEでのスレッドの起動を管理するためのコンビニエンス関数群。
スレッドの優先などの管理はEE-kernelのものをそのまま使う。
スレッドの総数はMTS_THREAD_MAXで規定されている。
この関数群を使用した時は、DEBUG_*を使用する必要はない。

基本的には、以下のようにmainを書けばいい
---
void Main( int id, void *arg )
{
	// 本来のmain
}

#define STACKSIZE (16*1024)	// てきとう
static u_long128 main_stack[ STACKSIZE/sizeof( u_long128 ) ];

void main( void )
{
	MTS_BootThread( "MAIN", Main, 24, main_stack, STACKSIZE, NULL );// 24はてきとう
}
---

**** 基本スレッド管理

void MTS_BootThread( char *name, void (*func)( int id, void *arg )
					, int pri, void *stack_top, int stack_size, void *arg );
	MTSを起動して、funcを指定したプライオリティのスレッドとして動かす。
	nameは一意の識別子でなくてはならない。(12文字まで)
	funcの引数はEE-kernelのスレッドID。
	main()関数はDMA系の初期化とこれだけ。
	main関数のスタックと同じものを指定しても動くはずだが、
	できれば別のスタック領域を使うことが望ましい。
	この関数中で起動スレッドはSleepする。
	例外発生時には、現在設定されているstack(app.cmd中のstack)を使用する。

int  MTS_NewThread( char *name, void (*func)( int id, void *arg )
					, int pri, void stack_top, int stack_size, void *arg );
	funcを指定したプライオリティのスレッドとして動かす。
	CreateしてStartする。
	nameは一意の識別子でなくてはならない。(12文字まで)
	返り値、funcの引数はEE-kernelのスレッドID。

void MTS_ExitThread( void );
	タスクの強制終了。

int MTS_GetThreadId( char *name );
	タスクIDを名前で検索する。

void MTS_SetExceptionCallback( void (*func)( void ) );
	例外コールバックを登録する。
	タスク系の情報表示の後に実行される。

void MTS_PANIC( char *format, ... );
	内部でアドレスエラーをおこし、例外画面を表示する。
	マスター用のモジュールをリンクした場合は、なにもしないで戻ってくる。

**** スレッド管理ユーティリティ

<<EVENT FLAG>>

EEでIOPのイベントフラグとほぼ同等の機能を実現する。
ただし、以下の制限あり。
・MTS_EventFlag構造体はユーザープログラム中にstaticで確保する。
・1つのイベントフラグで待ちに入るスレッドは1つ。
・エラーチェックはいいかげん。
・31ビット。

typedef struct {
	int value;
	int mask;
	int wait_threadid;
	int mode;
} MTS_EVENTFLAG;

MTS_EVENTFLAG *MTS_CreateEventFlag( MTS_EVENTFLAG *eventflag );

イベントフラグを設定する。

int MTS_WaitEventFlag( MTS_EVENTFLAG *ev, int mask, int mode, int timeout );

mode
#define MTS_EW_MODE_AND		0x0001
#define MTS_EW_MODE_OR		0x0002
#define MTS_EW_MODE_CLEAR	0x0004

返り値
enum {
	MTS_EW_OK = 0,			// 正常終了
	MTS_EW_NO_WAIT  = -1,	// すでに条件が成立していた
	MTS_EW_TIMEOUT = -2,	// timeout(hsync)経過しても条件が成立しなかった
};

int MTS_SetEventFlag( MTS_EVENTFLAG *ev, int mask );
int MTS_iSetEventFlag( MTS_EVENTFLAG *ev, int mask );

int MTS_CancelEventFlag( MTS_EVENTFLAG *ev );

<<DELAY THREAD>>
一定HSYNCスリープする。

void MTS_DelayThread( int wait_time );
wait_time は HSYNC単位。

---------------- debugutil.h -----------------
-- 例外系

このモジュールがリンクされると、printfがオーバーライドされる。
例外発生時に画面にレジスタダンプとprintfのログを表示する。
標準の設定では、512*448 でVRAMの０番地から表示。
printfのログは最大過去１６行、５１２バイト。
printfは最大128文字までに制限される。
変更したい場合は、exception.cの先頭の#define列を変更する。

例外の画面ダンプのうち、レジスタリストはr1-r24を表示している。
r25,26はカーネル、r27-31はgp,sp,fp,ra,pc。

void DEBUG_InitException( void *stacktop );
	例外発生時に画面にダンプするモジュールのセットアップを行なう。
	必ず常駐領域で呼び出すこと。
	mainの頭で設定しても大丈夫。IOPをリブートしても保存されているようだ。
	stacktopは、例外処理モジュールで使用されるスタックアドレスの<底>。
	NULLを指定すると、0x001FFFF0に設定される。

void DEBUG_SetExceptionCallback( void (*func)( unsigned long *regs ) );
	例外発生時に呼ばれるコールバック関数の登録。
	1つしか登録できないので、注意。
	regsはセーブされたレジスタ。$0が入っていないため、1ずれていることに注意。

int cprintf( const char *format, ... );
	ログがとられないprintf。

void excep_printf( const char *format, ... );
	例外コールバックからのみ呼び出しが可能。
	画面に表示するprintf。

-- スタック系

typedef struct _debug_stackcheck_info {
	struct _debug_stackcheck_info *next;
	char *name;
	void *top;
	int size;
} DEBUG_STACKCHECK_INFO;

void DEBUG_SetStackCheck( DEBUG_STACKCHECK_INFO *stack, char *name, void *top, int size );
	スタックチェックを行なうために、スタック領域を変な値で埋めつくす。
	一応、現在の$spをチェックして、そこまでしか埋めないようにしているが、
	カレントのスレッドのスタックには使用しない方が無難。

DEBUG_STACKCHECK_INFO *DEBUG_ReleaseStack( void );
	スレッドの終了時に必ず呼び出す。
	スタックポインタの値からカレントのスタック情報を検索し、
	チェックした後にそのスタック情報を切り離す。

int DEBUG_GetStackUseSize( void );
	現在のスレッドのスタック最大使用量をかえす。
	スタックオーバーなどが起きていれば、ハングアップする。
	spを直接参照するので、コールバック中などからは呼び出せない。
	そのタスクがMTSで起動しているか、DEBUG_SetStackCheckを使用している必要がある。
	mts_m.aでは、０を返す。(本来はマスター状態で呼ばれるべきものではない）
