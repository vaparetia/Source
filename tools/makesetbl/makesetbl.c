/*
	makesetbl.c
	2000/03/08 Y.Korekado
	$Id: makesetbl.c,v 1.2 2000/03/15 04:19:19 usr01749 Exp $
	
	se_table 作成プログラム
*/

#include "seutil.c"
/* --------------------------------------------------- */
static void MakeSeTable( char *filename, SE_SEL *allse, int allse_num, SE_SEL *stse, int stse_num )
{
	char	save_file[] = "tbl" ;
	int i ;
	unsigned char a ;
	FILE	*fp ;

	for( i=0; i<allse_num; i++ ) {
		stage_table[i] = SearchSe( &allse[i].name[0], stse, stse_num ) ;
	}

	MDU_ChangeExtension( filename, &save_file[0] ) ;

	if( ( fp = fopen( filename, "wb" ) ) == NULL ){
		printf( "ファイル %sがオープンできません\n", filename );
	}
	printf( "writing %s ...\n", filename );

	for( i=0; i<allse_num; i++ ) {
		a = (unsigned char)stage_table[i] ;
		if ( a!= 0xff ) {
			fprintf( fp, "0x%x: %s ",i, &allse[i].name[0] );
			fprintf(fp, " [0x%x]\n", a ) ;
		}
	}

	fclose( fp );
	
}

static void SaveSeTable( char *filename )
{
	char	save_file[] = "ztb" ;
	FILE	*fp ;

	MDU_ChangeExtension( filename, &save_file[0] ) ;

	if( ( fp = fopen( filename, "wb" ) ) == NULL ){
		printf( "ファイル %sがオープンできません\n", filename );
	}
	printf( "writing %s ...\n", filename );
	fwrite( &stage_table[0], sizeof( char ) * MAX_STAGE, 1, fp );
	fclose( fp );

}

/* --------------------------------------------------- */
int	main( int argc, char **argv )
{
	FILE *fp;
	char	se_stage_file[] = "../se_stage.sel" ;
	char	save_file[] = "se_table.ztb" ;

	char	*filename, *satgename ;
	int		allse_num, stagese_num ;

	printf( "make se table\n" );

	InitBuff() ;

    MDU_ParseOption( argc, argv ) ;

	/* ステージＳＥＬデータセット */
    if ( MDU_GetOption( 'i' ) != NULL ) {
	    satgename = MDU_GetNextValue() ;
	} else {
		printf(" not find stage se file!!\n") ;
		return (-1) ;
	}
	if ( ( fp = fopen( satgename, "rt" ) ) == NULL ){
		printf(" not find const file!!\n") ;
		return (-1) ;
	}
	stagese_num = ReadSel( fp, &SetStageBuff[0] );
	fclose( fp );

	/* 全ての擬音ＳＥＬデータセット */
    filename = &se_stage_file[0] ;
    if ( MDU_GetOption( 's' ) != NULL ) {
	    filename = MDU_GetNextValue() ;
	}
	printf(" stage sel [%s]\n",filename);
	if ( ( fp = fopen( filename, "rt" ) ) == NULL ){
		printf(" not find stage file!!\n") ;
		return (-1) ;
	}
	allse_num = ReadSel( fp, &StageSeBuff[0] );
	fclose( fp );

	/* ＳＥテーブル作成 */
	MakeSeTable( satgename, &StageSeBuff[0], allse_num, &SetStageBuff[0], stagese_num ) ;

	/* 出力 */
	SaveSeTable( &save_file[0] ) ;
//	SaveSeTable( satgename ) ;

	return 0;
}


