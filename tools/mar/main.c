#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<math.h>
#include	<ctype.h>

typedef long	int32 ;

#ifdef __GNUC__
#define NULL_ARRAY 0
#else
#define NULL_ARRAY 
#endif

#define MAX_FILES	(1024)

/* ========= 変数型定義 ========= */
typedef struct {
	int32	name_id ;				/* ファイル名ＩＤ */
	int32	flag ;					/* 各種フラグ情報 */
	int32	motion_base_tick ;		/* モーションのフレームベースカウント（１／３００秒単位） */
	int32	motion_length ;			/* モーションフレーム数 */
	short	*archives_data ;		/* アーカイブデータへのポイント */
	int32	archives_size ;			/* アーカイブデータのサイズ（ハーフワード単位） */
	int32	rot_units ;				/* 可動関節ユニット数 */
	int32	root_offset ;			/* リザーブ */
	int32	move_offset ;			/* 移動量データのオフセット */
	int32	turn_offset ;			/* 方向転換情報のオフセット */
	int32	fix_offset ;			/* 関節固定情報のオフセット */
	int32	rots_offset[ NULL_ARRAY ] ;		/* 関節回転情報のオフセット */
} MT3_FILE_HEADER ;

typedef struct {
	int32	offset ;
	int32	size ;
} FILE_TABLE ;

typedef struct {
	char	typeinfo[4] ;		/* ファイル判別ＩＤ用 */
	int32	joints ;			/* モーションで定義されている関節数 */
	int32	n_motion ;			/* 書庫に入っているモーションの数 */
	int32	header_size ;		/* アーカイブデータまでのオフセット */
	FILE_TABLE	data_table[ NULL_ARRAY ] ;
} MAR_HEADER;

/* ========= プロトタイプ宣言 ========= */
/* メインプログラム */
int ProgMain( int argc, char **argv );
/* 概存ファイルから全データを読み込む */
int ReadArchiveFile( char *fname );
/* ファイルに全データを書き出す */
int WriteArchiveFile( char *fname );
/* nt2ファイルを読み込んで一覧に加える */
int AddNT2File( char *fname );
/* 同じデータ列があるかどうかを調べる */
int SearchSameData( unsigned short *data, int num );

/* ========= グローバル変数 ========= */
int			ModeFlag ;				/* オプション認識用（手抜き） */
MAR_HEADER	MarHeader ;				/* オープン中のＭＡＲファイルヘッダ */
FILE_TABLE	FileTable[MAX_FILES] ;		/* 各ファイルへのオフセット */
int			ArchiveSize ;
char		ArchiveData[ 1024*1024*4 ];


int IndexSize;
int MaxMotion;
unsigned short *DataIndexList;
int MaxMotionData;
unsigned short *MotionData;

#define DEBUG_STOP(s) {	printf("%s\n",s);getchar();}

/* ========= プログラム本体 ========= */
int
main( int argc, char **argv )
{
	int data_num[256],num_count = 0;
	int i,mode = -1;
	char *data[MAX_FILES];
	int		ret ;

	/* ヘルプの表示 */
	if ( argc < 2 ){
		fprintf( stderr, "New version motion archiver 2.00   by K.Takabe\n");
		fprintf( stderr, "Usage: mar <option> arc-file [mt3-file] \n");
		fprintf( stderr, "Options  -a ... add file\n");
		//fprintf( stderr, "         -e ... extract file\n");
		fprintf( stderr, "         -c ... create a new archive file\n");
		fprintf( stderr, "         -i ... make a new archive file of initialaize only\n");
		return (0);
	}
	/* オプションチェック */
	//if ( argc > MAX_FILES ) argc = MAX_FILES ;
	for (i=1;i<argc;i++){
		if (argv[i][0] == '-'){
			if ( argv[i][1] == 'a' ){
				mode = 0;
			} else if (argv[i][1] == 'e'){
				mode = 1;
			} else if (argv[i][1] == 'c'){
				mode = 2;
			} else if (argv[i][1] == 'i'){
				mode = 3;
			}
		} else {
			if ( num_count < MAX_FILES ){
				data[num_count++] = argv[i];
			}
		}
	}
	if ( (mode == -1 && num_count == 0) || ((mode != -1 && mode != 3) && num_count < 2) ){
		fprintf( stderr, "Error !!\n");
		return (-1);
	}
	ModeFlag = mode;
	ret = ProgMain( num_count, data );
	return (ret);
}

/* メインプログラム */
int ProgMain( int argc, char **argv )
{
	int i;

	DataIndexList = malloc( 32 * 1024 );
	MotionData = malloc( 256 * 1024 );
	if ( ReadArchiveFile( argv[0] ) ) return (-1);

	if ( ModeFlag == 0 || ModeFlag == 2 ){
		for (i=1;i<argc;i++){
			AddMT3File( argv[i] );
		}
	}

	if ( ModeFlag != -1 ) WriteArchiveFile( argv[0] );

	return (0);
}

/* 概存ファイルから全データを読み込む */
int ReadArchiveFile( char *fname )
{
	FILE	*arc;
	int		i, size ;

	if (ModeFlag == 2 || ModeFlag == 3){/* ファイルの新規作成 */
#ifdef _XBOX
		fprintf( stdout, "Create a new file !!\n");
#else
		fprintf( stderr, "Create a new file !!\n");
#endif
		MarHeader.typeinfo[0] = 'M';
		MarHeader.typeinfo[1] = 'A';
		MarHeader.typeinfo[2] = 'R';
		MarHeader.typeinfo[3] = 'a';
		MarHeader.joints = 0;
		MarHeader.n_motion = 0;
		MarHeader.header_size = 0;
		ArchiveSize = 0 ;
		return ( 0 );
	} else {/* 概存ファイルのオープン */
		if ((arc = fopen(fname,"rb")) == NULL ) return (-1);
		fread( &MarHeader, sizeof(MAR_HEADER), 1, arc );
		if (!( MarHeader.typeinfo[0] == 'M' && MarHeader.typeinfo[1] == 'A'
			  && MarHeader.typeinfo[2] == 'R' && MarHeader.typeinfo[3] == 'a' )){
#ifdef _XBOX
			fprintf( stdout, "This file is no MAR-file !!!(%s)\n", fname );
#else
			fprintf( stdeff, "This file is no MAR-file !!!(%s)\n", fname );
#endif
			fclose( arc );
			return (-1);
		} else {
#ifdef _XBOX
			fprintf( stdout, "Now reading MAR-file .....(%s)\n", fname );
#else
			fprintf( stderr, "Now reading MAR-file .....(%s)\n", fname );
#endif
		}
	}

	/* 各ＭＴ３ファイルへのオフセット情報を読み込む */
	fread( FileTable, sizeof(FILE_TABLE), ( MarHeader.n_motion + 1 ) & 0xfffe, arc );

	/* モーションデータの読み込み */
	for ( size = 0 , i = 0 ; i < MarHeader.n_motion ; i++ ) size += FileTable[ i ].size ;
	fread( ArchiveData, sizeof(char), size, arc );
	ArchiveSize = size ;

	fclose( arc );
	return (0);
}
/* ファイルに全データを書き出す */
int WriteArchiveFile( char *fname )
{
	FILE *arc;

	if ((arc = fopen(fname,"wb")) == NULL ) return (-1);
#ifdef _XBOX
	fprintf( stdout, "Now writing MAR-file .....(%s)\n", fname );
#else
	fprintf( stderr, "Now writing MAR-file .....(%s)\n", fname );
#endif

	MarHeader.header_size = sizeof(MAR_HEADER) + sizeof(FILE_TABLE) * ( ( MarHeader.n_motion + 1 ) & 0xfffe ) ;
	fwrite( &MarHeader, sizeof(MAR_HEADER), 1, arc );
	fwrite( FileTable, sizeof(FILE_TABLE), ( MarHeader.n_motion + 1 ) & 0xfffe, arc );/* 16バイト境界にそろえる */
	fwrite( ArchiveData, sizeof(char), ArchiveSize, arc );

	fclose( arc );
	return (0);
}

/* mt3ファイルを読み込んで一覧に加える */
int AddMT3File( char *fname )
{
	FILE			*fp;
	MT3_FILE_HEADER	*mt3 ;
	int				size ;

	/* ファイルオープン */
	if ((fp = fopen(fname,"rb")) == NULL ){
		fprintf( stderr, "Can't open file !!!(%s)\n", fname );
		return (-1);
	}
#ifdef _XBOX
	fprintf( stdout, "Now adding MT3-file .....(%s)\n", fname );
#else
	fprintf( stderr, "Now adding MT3-file .....(%s)\n", fname );
#endif
	/* ファイルサイズ取得 */
	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	/* アーカイバへの追加 */
	fread( &ArchiveData[ ArchiveSize ], sizeof(char), size, fp );
	FileTable[ MarHeader.n_motion ].offset = ArchiveSize ;
	FileTable[ MarHeader.n_motion ].size = ( size + 15 ) & ( ~0xf ) ;	/* １６バイト境界にそろえる */
	mt3 = (MT3_FILE_HEADER*)&ArchiveData[ ArchiveSize ] ;
	MarHeader.n_motion++ ;
	ArchiveSize += ( size + 15 ) & ( ~0xf ) ;	/* １６バイト境界にそろえる */ ;

	if ( MarHeader.n_motion == 1 ){
		MarHeader.joints = mt3->rot_units ;
	}
	fclose( fp );
	return (0);
}

