/*
	main.c
	パッド振動データ管理ファイラー

	2000/06/22 K.Takabe
	$Id: main.c,v 1.2 2000/06/23 12:27:53 usr02774 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <wchar.h>
#include <curses.h>
#include <term.h>
#include <dirent.h>
#include <fnmatch.h>

/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
static char	*MainMenuCommandUs[] = {
	"Save Data As ... ",
	"Open File ... ",
	"Save Data ",
	"Over Write ... ",
	NULL
};
static char	*MainMenuCommandJp[] = {
	"名前を付けて保存 ... ",
	"ファイルを開く ... ",
	"現在の名前でセーブ ... ",
	"指定ファイルへ上書き ... ",
	NULL
};
static char *OverWriteMesUs = "Over write to <%s>. Ok ?" ;
static char *OverWriteMesJp = "<%s> に上書きします。よろしいですか？" ;

static char	*DataFileExt = ".vib" ;
static char	*DataFileMask = "*.vib" ;

/* ---------------------------------------------------------------- */
char	**MainMenuCommand ;
char	*OverWriteMes ;

/* ---------------------------------------------------------------- */
void MainProg( void );
void DispFiles( void );
int DialogYesNo( WINDOW *parent_win, char *mes );
char* FileSelect( WINDOW *win, char *wildcard );
char* InputDialog( WINDOW *parent_win );
int CommandSelect( WINDOW *parent_win, char **command_list, int offset_x, int offset_y );
int SearchFiles( const char *path, const char *wildcard, char filename_list[][64], int max_list );

int CheckExt( char *filename );
void DelFileExt( char *filename );
void AddFileExt( char *filename, char *ext );

/* ---------------------------------------------------------------- */
void main( int argc, char **argv )
{
	int		flag = 0 ;

	/* curses initialize */
	initscr(); cbreak(); noecho();
	nonl();
	intrflush( stdscr, FALSE );
	keypad( stdscr, TRUE );

	/* コマンドライン解析 */
	for ( argc--, argv++ ; argc > 0 ; argc--, argv++ ){
		if ( argv[0][0] == '-' ){
			/* コマンド解析 */
			if ( argv[0][1] == 'u' ){
				flag |= 0x0001 ;
			}
		} else {
			/* その他 */
		}
	}

	if ( flag & 0x0001 ){
		MainMenuCommand = MainMenuCommandUs ;
		OverWriteMes = OverWriteMesUs ;
	} else {
		MainMenuCommand = MainMenuCommandJp ;
		OverWriteMes = OverWriteMesJp ;
	}

	/* main */
	MainProg();

	/* curses terminate */
	endwin();
}

/* ---------------------------------------------------------------- */
void MainProg( void )
{
	int		command, loop_flag = 1 ;
	char	exec_buffer[ 256 ];
	char	find_filebuffer[1][ 64 ];
	char	*input_file, current_filename[ 256 ] = "" ;
	WINDOW	*client_win ;

	{/* クライアント部ウィンドウ作成 */
		int		px, py, pw, ph ;
		getbegyx( stdscr, py, px );
		getmaxyx( stdscr, ph, pw );
		client_win = subwin( stdscr, ph-4, pw, 4, 0 );
		intrflush( client_win, FALSE );
		keypad( client_win, TRUE );
		wborder( client_win, 0, 0, 0, 0, 0, 0, 0, 0 );
	}

	while ( loop_flag ){

		wborder( stdscr, 0, 0, 0, 0, 0, 0, 0, 0 );
		wclear( stdscr );
		wborder( stdscr, 0, 0, 0, 0, 0, 0, 0, 0 );
		mvwprintw( stdscr, 1, 1, " MGS2 Vibration data filer  Ver. 0.01" );
		mvwprintw( stdscr, 2, 1, " current file: %s", current_filename );
		wrefresh( stdscr );

		wclear( client_win );
		wborder( client_win, 0, 0, 0, 0, 0, 0, 0, 0 );
		wrefresh( client_win );
		while ( ( command = CommandSelect( client_win, MainMenuCommand, 1, 1 ) ) == -1 );

		switch ( command ){
		case 0:/* 名前を付けて保存 */
			input_file = InputDialog( client_win );
			if ( input_file != NULL ){
				AddFileExt( input_file, DataFileExt );
				/* 同一名ファイルの存在チェック */
				if ( SearchFiles( ".", input_file, find_filebuffer, 1 ) != 0 ){
					sprintf( exec_buffer, OverWriteMes, input_file );
					if ( DialogYesNo( client_win, exec_buffer ) == 0 ) break ;
				}
				sprintf( exec_buffer, "cp default%s %s", DataFileExt, input_file );
				system( exec_buffer );
				strcpy( current_filename, input_file );
			}
			break ;
		case 1:/* ファイルを開く */
			input_file = FileSelect( client_win, DataFileMask );
			if ( input_file != NULL ){
				AddFileExt( input_file, DataFileExt );
				sprintf( exec_buffer, "cp %s default%s", input_file, DataFileExt );
				system( exec_buffer );
				strcpy( current_filename, input_file );
			}
			break ;
		case 2:/* 現在のファイル名で保存 */
			if ( input_file != NULL ){
				AddFileExt( input_file, DataFileExt );
				sprintf( exec_buffer, OverWriteMes, current_filename );
				if ( DialogYesNo( client_win, exec_buffer ) ){
					sprintf( exec_buffer, "cp default%s %s", DataFileExt, current_filename );
					system( exec_buffer );
				}
			}
			break ;
		case 3:/* 他のファイルに上書き保存 */
			input_file = FileSelect( client_win, DataFileMask );
			if ( input_file != NULL ){
				AddFileExt( input_file, DataFileExt );
				sprintf( exec_buffer, OverWriteMes, input_file );
				if ( DialogYesNo( client_win, exec_buffer ) ){
					sprintf( exec_buffer, "cp default%s %s", DataFileExt, input_file );
					system( exec_buffer );
					strcpy( current_filename, input_file );
				}
			}
			break ;
		case -1:
			break ;
		}
	}

	delwin( client_win );
}

/* ---------------------------------------------------------------- */
/* 確認ダイアログ */
int DialogYesNo( WINDOW *parent_win, char *mes )
{
	WINDOW	*win ;
	int		pw, ph, px, py, key, ret = 0, len ;

	len = strlen( mes ); if ( len < 16 ) len = 16 ;
	len += 2 ;
	getbegyx( parent_win, py, px );
	getmaxyx( parent_win, ph, pw );
	win = subwin( parent_win, 4, len, ( ph - 4 ) / 2 + py, ( pw - len ) / 2 + px );
	intrflush( win, FALSE );
	keypad( win, TRUE );
	wborder( win, 0, 0, 0, 0, 0, 0, 0, 0 );
	mvwprintw( win, 1, 1, "%s", mes );
	mvwprintw( win, 2, 1, "[Yes/No]" );

	ret = -1 ;
	while ( ret == -1 ){
		key = wgetch( win );
		switch ( key ){
		case 'y':
		case 'Y':
			ret = 1 ;
			break ;
		case 'n':
		case 'N':
		case 0x1b:
			ret = 0 ;
			break ;
		}
	}
	wclear( win );
	wrefresh( win );
	delwin( win );

	return ( ret );
}

/* ---------------------------------------------------------------- */
/* ファイル選択ダイアログ */
char* FileSelect( WINDOW *parent_win, char *wildcard )
{
	static char		select_filename[ 64 ];
	int				i, cur_x, cur_y, n, w, h, scroll, key, exit_flag ;
	char			select_file_buffer[ 256 ][ 64 ];
	WINDOW			*win ;

	{/* ウィンドウ作成 */
		int		pw, ph, px, py ;
		getmaxyx( parent_win, ph, pw );
		win = subwin( parent_win, ph - 8, pw - 10, 8 / 2, 10 / 2 );
		intrflush( win, FALSE );
		keypad( win, TRUE );
	}

	/* ファイル名の取得 */
	n = SearchFiles( ".", wildcard, select_file_buffer, 256 );

	/* 表示初期化 */
	scroll = 0 ;
	cur_x = 2 ;
	cur_y = 0 ;
	//getbegyx( win, h, w );
	getmaxyx( win, h, w );
	w -= 3 ;
	h -= 2 ;

	/* 操作＆表示 */
	exit_flag = 0 ;
	while ( exit_flag == 0 ){
		wclear( win );
		wborder( win, 0, 0, 0, 0, 0, 0, 0, 0 );
		mvwprintw( win, 0, 1, " file select <cancel:Ctrl+G> " );
		/*mvwprintw( win, 0, 20, "<%08x>", key );*/
		/* ファイル一覧の表示 */
		for ( i = 0 ; i < h ; i++ ){
			int		y ;
			y = i + scroll ;
			if ( y >= 0 && y < n ){
				mvwprintw( win, i+1, cur_x, "%s", select_file_buffer[ y ] );
			}
		}
		mvwprintw( win, cur_y - scroll + 1, cur_x - 1, ">" );
		wmove( win, 0, 0 );
		/*wrefresh( win );*/

		/* キー入力 */
		switch( key = wgetch( win ) ){
		case KEY_UP:
			if ( cur_y > 0 ) cur_y-- ;
			if ( ( cur_y - scroll ) < 0 ) scroll-- ;
			break ;
		case KEY_DOWN:
			if ( cur_y < ( n - 1 ) ) cur_y++ ;
			if ( ( cur_y - scroll ) >= h ) scroll++ ;
			break ;
		case KEY_LEFT:
			cur_y -= 8 ;
			if ( cur_y < 0 ) cur_y = 0 ;
			while ( ( cur_y - scroll ) < 0 ) scroll-- ;
			break ;
		case KEY_RIGHT:
			cur_y += 8 ;
			if ( cur_y > ( n - 1 ) ) cur_y = n - 1 ;
			while ( ( cur_y - scroll ) >= h ) scroll++ ;
			break ;
		case 0x07:/* Ctrl+G */
		case 0x1b:/* ESC */
			exit_flag = -1 ;
			break ;
		case 0x0d:/* Return */
		case 0x4d:/* Enter */
			strcpy( select_filename, select_file_buffer[ cur_y ] );
			exit_flag = 1 ;
			break ;
		}
	}

	wclear( win );
	wrefresh( win );
	delwin( win );

	if ( exit_flag == -1 ) return ( NULL );
	DelFileExt( select_filename );
	return ( select_filename );
}

/* ---------------------------------------------------------------- */
/* ファイル名入力ダイアログ */
char* InputDialog( WINDOW *parent_win )
{
	static char		input_buffer[ 256 ];
	WINDOW			*win ;
	SCREEN			*screen ;
	int				exit_flag, cur_x, input_end, i, key ;
	char			tmp_buffer[ 256 ];

	{/* ウィンドウ作成 */
		int		pw, ph, px, py ;
		getbegyx( parent_win, py, px );
		getmaxyx( parent_win, ph, pw );
		win = subwin( parent_win, 3, 66, ( ph - 3 ) / 2 , ( pw - 66 ) / 2 );
		intrflush( win, FALSE );
		keypad( win, TRUE );
	}


	/* 入力行編集 */
	exit_flag = 0 ;
	cur_x = 0 ;
	input_end = 0 ;
	while ( exit_flag == 0 ){
		wclear( win );
		wborder( win, 0, 0, 0, 0, 0, 0, 0, 0 );
		mvwprintw( win, 0, 1, " Input filename " );
		wmove( win, 1, 1 );
		input_buffer[ input_end ] = '\0' ;
		mvwprintw( win, 1, 1, "%s", input_buffer );
		wmove( win, 1, 1 + cur_x );

		/* キー入力 */
		switch( key = wgetch( win ) ){
		case KEY_UP:
			break ;
		case KEY_DOWN:
			break ;
		case KEY_LEFT:
			if ( cur_x > 0 ) cur_x-- ;
			break ;
		case KEY_RIGHT:
			if ( cur_x < input_end ) cur_x++ ;
			break ;
		case 0x04:/* Ctrl+D */
		case 0x7f:/* del */
			if ( cur_x < input_end ){
				for ( i = cur_x ; i < input_end ; i++ ) input_buffer[ i ] = input_buffer[ i + 1 ];
				input_end-- ;
			}
			break ;
		case 0x107:/* back space(Ctrl+H) */
			if ( cur_x > 0 ){
				for ( i = cur_x ; i < input_end ; i++ ) input_buffer[ i - 1 ] = input_buffer[ i ];
				cur_x-- ;
				input_end-- ;
			}
			break ;
		case ' ':/* SPACE */
		case 0x09:/* TAB */
			{
				char	*select_file ;
				sprintf( tmp_buffer, "%s*%s", input_buffer, DataFileExt );
				select_file = FileSelect( stdscr, tmp_buffer );
				if ( select_file != NULL ){
					strcpy( input_buffer, select_file );
					cur_x = input_end = strlen( input_buffer );
				}
			}
			break ;
		case 0x1b:/* ESC */
			exit_flag = -1 ;
			break ;
		case 0x0d:/* Return */
		case 0x4d:/* Enter */
			exit_flag = 1 ;
			if ( input_end == 0 ) exit_flag = -1 ;
			break ;
		default:
			if ( key <= ' ' || key >= 128 ) break ;
			for ( i = input_end ; i >= cur_x ; i-- ) input_buffer[ i + 1 ] = input_buffer[ i ];
			input_buffer[ cur_x ] = key & 0xff ;
			cur_x++ ;
			input_end++ ;
			break ;
		}
	}

	wclear( win );
	wrefresh( win );
	delwin( win );

	if ( exit_flag == -1 ) return ( NULL );
	if ( input_buffer[0] == '\0' ) return ( NULL );
	return ( input_buffer );
}

/* ---------------------------------------------------------------- */
/* コマンド選択ダイアログ */
int CommandSelect( WINDOW *parent_win, char **command_list, int offset_x, int offset_y )
{
	static char		select_filename[ 64 ];
	int				i, cur_x, cur_y, n, w, h, scroll, key, exit_flag ;
	char			select_file_buffer[ 256 ][ 64 ];
	WINDOW			*win ;

	{/* ウィンドウサイズ計算 */
		char	**list ;
		int		len, pw, ph, px, py ;

		w = 0 ;
		h = 0 ;
		n = 0 ;
		list = command_list ;
		while ( *list != NULL ){
			len = strlen( *list );
			if ( len > w ) w = len ;
			n++ ;
			list++ ;
		}
		getmaxyx( stdscr, ph, pw );
		if ( ( n + 2 ) > ( ph - offset_y ) )	h = ph ;
		else									h = n + 2 ;
		w += 3 ;
		if ( offset_x < 0 ) offset_x = 0 ;
		if ( offset_y < 0 ) offset_y = 0 ;
		if ( ( offset_x + w ) > pw ) offset_x = pw - w ;
	}

	{/* ウィンドウ作成 */
		int		pw, ph, px, py ;
		getbegyx( parent_win, py, px );
		getmaxyx( parent_win, ph, pw );
		win = subwin( parent_win, h, w, offset_y + py, offset_x + px );
		intrflush( win, FALSE );
		keypad( win, TRUE );
	}

	/* 表示初期化 */
	scroll = 0 ;
	cur_x = 2 ;
	cur_y = 0 ;
	getmaxyx( win, h, w );
	w -= 3 ;
	h -= 2 ;
	h = 4 ;

	/* 操作＆表示 */
	exit_flag = 0 ;
	while ( exit_flag == 0 ){
		wclear( win );
		wborder( win, 0, 0, 0, 0, 0, 0, 0, 0 );
		mvwprintw( win, 0, 1, " commnad select " );
		/*mvwprintw( win, 0, 20, "<%08x>", key );*/
		/* コマンド一覧の表示 */
		for ( i = 0 ; i < h ; i++ ){
			int		y ;
			y = i + scroll ;
			if ( y >= 0 && y < n ){
				mvwprintw( win, i+1, cur_x, "%s", command_list[ y ] );
			}
		}
		mvwprintw( win, cur_y - scroll + 1, cur_x - 1, ">" );
		wmove( win, 0, 0 );
		/*wrefresh( win );*/

		/* キー入力 */
		switch( key = wgetch( win ) ){
		case KEY_UP:
			if ( cur_y > 0 ) cur_y-- ;
			if ( ( cur_y - scroll ) < 0 ) scroll-- ;
			break ;
		case KEY_DOWN:
			if ( cur_y < ( n - 1 ) ) cur_y++ ;
			if ( ( cur_y - scroll ) >= h ) scroll++ ;
			break ;
		case 0x07:/* Ctrl+G */
		case 0x1b:/* ESC */
			cur_y = -1 ;
			exit_flag = -1 ;
			break ;
		case 0x0d:/* Return */
		case 0x4d:/* Enter */
			exit_flag = 1 ;
			break ;
		}
	}

	wclear( win );
	wrefresh( win );
	delwin( win );

	return ( cur_y );
}
/* ---------------------------------------------------------------- */
/* ファイルを検索 */
int SearchFiles( const char *path, const char *wildcard, char filename_list[][64], int max_list )
{
	DIR				*dir ;
	struct dirent	*ent ;
	int				n ;

	/* ファイル名の取得 */
	if ( ( dir = opendir( path ) ) == NULL ){
		return ( 0 );
	}
	n = 0 ;
	ent = readdir( dir );
	while ( ent != NULL ){
		if ( fnmatch( wildcard, ent->d_name, 0 ) == 0 ){
			sprintf( filename_list[ n ], "%s", ent->d_name ) ;
			if ( ++n >= max_list ) break ;
		}
		readdir_r( dir, ent, &ent );
	}
	closedir( dir );

	return ( n );
}

/* ---------------------------------------------------------------- */
/* 拡張子の有無をチェック */
int CheckExt( char *filename )
{
	int		i, len ;
	len = strlen( filename ) ;
	for ( i = len - 1 ; i >= 0 ; i-- ){
		if ( filename[i] == '.' ) return ( i ) ;
	}
	return ( -1 );
}

/* 拡張子を削除 */
void DelFileExt( char *filename )
{
	int	pos ;
	pos = CheckExt( filename );
	if ( pos != -1 ){
		filename[ pos ] = '\0' ;
	}
}

/* 拡張子があれば取り除き新しい拡張子を設定(".???"で指定すること) */
void AddFileExt( char *filename, char *ext )
{
	int	pos ;
	DelFileExt( filename );
	pos = strlen( filename );
	strcpy( &filename[pos], ext );
}
