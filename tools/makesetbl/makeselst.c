/*
	makeselst.c
	2000/03/09 Y.Korekado
	$Id: makeselst.c,v 1.2 2000/03/15 04:19:19 usr01749 Exp $

	seパック用 lstファイル 作成プログラム
*/

#include "seutil.c"

/* --------------------------------------------------- */
static void SaveSeLst( char *filename, SE_SEL *allse, int allse_num, SE_SEL *stse, int stse_num )
{
	char	save_file[] = "lst" ;
	FILE	*fp ;
	int i, num ;
	
	MDU_ChangeExtension( filename, &save_file[0] ) ;
	if( ( fp = fopen( filename, "wt" ) ) == NULL ){
		printf( "ファイル %sがオープンできません\n", filename );
	}

	printf( "writing %s ...\n", filename );
	for( i=0; i<stse_num; i++ ) {
		if ( stse[i].lst[0] !='\0' ) {
			fprintf( fp, "%s\n", stse[i].lst );
		} else {
			num = SearchSe( &stse[i].name[0], allse, allse_num ) ;
			fprintf( fp, "%s\n", allse[num].lst );
		}
//		printf(" [%d] %s ",i, &stse[i].name[0] ) ;
//		printf(" %s\n",allse[num].lst ) ;
	}
	fclose( fp );
}

/* --------------------------------------------------- */
int	main( int argc, char **argv )
{
	FILE *fp;
//	char	se_const_file[] = "se_const.sel" ;
	char	se_stage_file[] = "../se_stage.sel" ;
	char	setstage_file[] = "se_se79.sel" ;
	char	*filename, *satgename ;
	int		allse_num, stagese_num ;

	printf( "make se lst\n" );

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
		printf(" not find stage file!!\n") ;
		return (-1) ;
	}
	stagese_num = ReadSel( fp, &SetStageBuff[0] );
	fclose( fp );
printf( "make se num [%d]\n", stagese_num );

	/* 全ての擬音ＳＥＬデータセット */
    if ( MDU_GetOption( 's' ) != NULL ) {
	    filename = MDU_GetNextValue() ;
	} else {
	    filename = &se_stage_file[0] ;
	}
	if ( ( fp = fopen( filename, "rt" ) ) == NULL ){
		printf(" not find se_stage sel file!!\n") ;
		return (-1) ;
	}
	allse_num = ReadSel( fp, &StageSeBuff[0] );
	fclose( fp );

	/* ＳＥlst作成 */
	SaveSeLst( satgename, &StageSeBuff[0], allse_num, &SetStageBuff[0], stagese_num ) ;

	return 0;
}
