/*
	makesedef.c
	2000/03/08 Y.Korekado
	$Id: makesedef.c,v 1.3 2000/03/15 04:19:19 usr01749 Exp $
	
	se_defin.h 作成プログラム
*/

#include "seutil.c"

//#define CONST_SAVE (1)
/* --------------------------------------------------- */
static void SaveSeDefine( char *filename, SE_SEL *cnse, int cnse_num, SE_SEL *stse, int stse_num )
{
	FILE	*fp ;
	int i, num ;
	char	eucbuff[MAX_LINE] ;
	
	if( ( fp = fopen( filename, "wt" ) ) == NULL ){
		printf( "ファイル %sがオープンできません\n", filename );
		return ;
	}
	printf( "save %s\n", filename );

	fprintf( fp, "enum{\n" ) ;

#ifdef CONST_SAVE
	fprintf( fp, "/* 常駐ＳＥ 0～%d*/\n",cnse_num );
	for( i=0; i<cnse_num; i++ ) {
		fprintf( fp, "%s,\t%s\n", cnse[i].name, cnse[i].comment );
//		printf(" [%d] %s ",i, &stse[i].name[0] ) ;
//		printf(" %s\n",allse[num].lst ) ;
	}
#endif

	fprintf( fp, "\n/* 非常駐ＳＥ 256～%d */\n",256+stse_num );
	ConvSJIS2EUC( stse[0].comment, &eucbuff ) ;
	fprintf( fp, "%s=256,\t%s\n", stse[0].name, eucbuff );
	for( i=1; i<stse_num; i++ ) {
		ConvSJIS2EUC( stse[i].comment, &eucbuff ) ;
		fprintf( fp, "%s,\t%s\n", stse[i].name, eucbuff );
//		fprintf( fp, "%s\n", stse[i].name );
//		printf(" [%d] %s ",i, &stse[i].name[0] ) ;
//		printf(" %s\n",allse[num].lst ) ;
	}
	fprintf( fp, "} ;\n" );
	fclose( fp );
}


/* --------------------------------------------------- */
int	main( int argc, char **argv )
{
	FILE *fp;
	char	se_const_file[] = "se_const.sel" ;
	char	se_stage_file[] = "se_stage.sel" ;
	char	save_file[] = "se_defin.h" ;
	char	*filename, *satgename ;
	int		constse_num, stagese_num ;

	printf( "make se defin\n" );

    MDU_ParseOption( argc, argv ) ;

#ifdef CONST_SAVE
	/* ステージＳＥＬデータセット */
    if ( MDU_GetOption( 'c' ) != NULL ) {
	    satgename = MDU_GetNextValue() ;
	} else {
		satgename = &se_const_file[0] ;
	}
	if ( ( fp = fopen( satgename, "rt" ) ) == NULL ){
		constse_num = 0 ;
	} else {
		constse_num = ReadSel( fp, &ConstSeBuff[0] );
		fclose( fp );
	}
	printf("const se = [%d]\n",constse_num) ;
#endif

	/* 全ての擬音ＳＥＬデータセット */
    if ( MDU_GetOption( 's' ) != NULL ) {
	    filename = MDU_GetNextValue() ;
	} else {
	    filename = &se_stage_file[0] ;
	}
	if ( ( fp = fopen( filename, "rt" ) ) == NULL ){
		printf(" not find se stage file!!\n") ;
		return (-1) ;
	}
	stagese_num = ReadSel( fp, &StageSeBuff[0] );
	fclose( fp );

    if ( MDU_GetOption( 'o' ) != NULL ) {
	    filename = MDU_GetNextValue() ;
	} else {
		filename = &save_file[0] ;
	}
	/* 出力 */
	SaveSeDefine( filename, &ConstSeBuff[0], constse_num, &StageSeBuff[0], stagese_num ) ;

	return 0;
}


