/*
   mt3size.c

   2001/01/19	M.Sonoyama
   $Id: mt3size.c,v 1.1 2001/01/19 08:12:33 usr02011 Exp $
*/

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>

#include	"MDU_util.h"

#if 0
#define	DEFAULT_INPUT_DIR	"/u/develop/mj001data/fix/mtn/rai"
#define	MAGIC				(10101010)
#define	VERSION				(1)

typedef	struct	{
	int			magic ;
	u_short		version ;
	u_short		flag ;
	int			n_elems ;
	u_short		*elemNo ;
	u_char		**elems ;
} VAR_HEADER ;

enum {
	GM_VAR_FLAG_MLS	=	0x0000,
	GM_VAR_FLAG_VLS =	0x0001,
} ;

typedef	struct	{
	FILE			*list_fp ;
	FILE			*vib_fp ;
	FILE			*out_fp ;
	char			input_dir[ 256 ] ;
	char			list_file[ 256 ] ;
	char			out_file[ 256 ] ;
	int				flag ;
	VAR_HEADER		def ;
	VAR_HEADER		*def2 ;
	int				*elemsize ;
} Work ;

static	int		Align4( int size )
{
	return ( ( size + 3 ) / 4 * 4 ) ;
}

static	void	FreeVar( VAR_HEADER *def )
{
	int			i ;

	if ( def->elemNo != NULL ) MDU_Free( def->elemNo ) ;
	if ( def->elems != NULL ) {
		for ( i = 0; i < def->n_elems; i ++ ) {
			if ( def->elems[ i ] != NULL ) MDU_Free( def->elems[ i ] ) ;
		}
		MDU_Free( def->elems ) ;
	}
}

/* 使用法 */
static	void	Usage( void )
{
	printf( "makevar : vib-file archiver\n" ) ;
	printf( "\t Usage : makevar list-file[.mls||.vls] [-options]\n" ) ;
	printf( "\t options :\n" ) ;
	printf( "\t -o <file> : Set output file name.\n" ) ;
	printf( "\t -l <dir> : Directory for input files.\n" ) ;
}

/* 初期化 */
static	int		Initialize( Work *work )
{
	strcpy( work->input_dir, DEFAULT_INPUT_DIR ) ;
	work->list_fp = NULL ;
	work->vib_fp = NULL ;
	work->out_fp = NULL ;
	return 0 ;
}

/* オプションの解釈 */
static	int		Analyze( Work *work, int argc, char **argv ) 
{
	if ( argc < 2 ) return -1 ;
	MDU_ParseOption( argc, argv ) ;
	strcpy( work->list_file, argv[ 1 ] ) ;
	strcpy( work->out_file, argv[ 1 ] ) ;	
	MDU_ChangeExtension( work->out_file, "var" ) ;
	if ( MDU_GetOption( 'l' ) != NULL ) {
		strcpy( work->input_dir, MDU_GetNextValue() ) ;
	}
	if ( MDU_GetOption( 'o' ) != NULL ) {
		strcpy( work->out_file, MDU_GetNextValue() ) ;
		MDU_ChangeExtension( work->out_file, "var" ) ;
	}	
	if ( MDU_GetOption( 'v' ) != NULL ) {
		work->flag = 0x10 ;	/* Verbose Mode */
	}
	if ( work->flag & 0x10 ) {
		printf( "input_dir : %s\n", work->input_dir ) ;
		printf( "input_list : %s\n", work->list_file ) ;
		printf( "output_file : %s\n", work->out_file ) ;
	}
	return 0 ;
}

static	void	DumpVib( u_char *elem )
{
	int		t, v, c ;

	v = t = c = 0 ;
	do {
		v = elem[ 0 ] ; t = elem[ 1 ] ;
		printf( "<%d %d>", v, t ) ;
		elem += 2 ;
		if ( v == 0 && t == 0 ) c ++ ;
	} while( c < 2 ) ;
	printf( "\n" ) ;
}

/* リストファイル読み込み */
static	int	ReadListFile( Work *work )
{
	FILE			*fp, *vfp[ 256 ] ;
	char			**elem, vib[ 256 ], *ext ;
	int				n_elems, n, vn, velemNo[ 256 ], velemsize[ 256 ] ;
	int				i, vls ;
	VAR_HEADER		*def ;

	elem = ( char ** )MDU_AllocChar2( 16, 256 ) ;
	if ( elem == NULL ) return -1 ;

	fp = fopen( work->list_file, "r" ) ;
	if ( fp == NULL ) return -1 ;

	def = &work->def ;
	ext = MDU_GetExtension( work->list_file ) ;
	if ( !strcmp( ext, "vls" ) ) {
		def->flag = GM_VAR_FLAG_VLS ;
		vls = 1 ;
	} else {
		def->flag = GM_VAR_FLAG_MLS ;
		vls = 0 ;
	}
	work->list_fp = fp ;
	n = vn = 0 ;
	while( MDU_GetSentenceElem( fp, elem, &n_elems ) >= 0 ) {
		if ( n_elems == 0 ) continue ;
		ext = MDU_GetExtension( elem[ 0 ] ) ;	
		if ( vls ) {
			if ( strcmp( ext, "vib" ) ) continue ;
		} else {
			if ( strcmp( ext, "mt3" ) ) continue ;
		}
		MDU_ChangeExtension( elem[ 0 ], "vib" ) ;
		strcpy( vib, work->input_dir ) ;
		strcat( vib, "/" ) ;
		strcat( vib, elem[ 0 ] ) ;
		vfp[ n ] = fopen( vib, "rb" ) ;
		velemNo[ n ] = vn ;
		if ( ++ vn >= 256 ) break ;
		if ( vfp[ n ] == NULL ) {
			if ( vls ) {
				printf( "makevar warning : %s is not exist.\n", vib ) ;
			}
			continue ;
		}
		velemsize[ n ] = MDU_GetFileSize( vib ) ;
		if ( velemsize[ n ] <= 0 ) continue ;
		if ( work->flag & 0x10 ) {
			printf( "find %s[%d][%d]\n", vib, velemNo[ n ], n ) ;
		}
		if ( ++ n >= 256 ) break ;
	}
	MDU_FreeChar2( elem, 16 ) ;
	def->n_elems = n ;
	if ( vls ) def->elemNo = NULL ;
	else {
		def->elemNo = ( u_short *)MDU_AllocShort( n ) ;
		if ( def->elemNo == NULL ) return -1 ;
	}
	def->elems = ( u_char ** )MDU_Calloc( n, sizeof( u_char * ) ) ;
	work->elemsize = ( int * )MDU_AllocInt( n ) ;
	if ( def->elems == NULL ||
		 work->elemsize == NULL ) return -1 ;
	for ( i = 0; i < n; i ++ ) {
		if ( !vls ) def->elemNo[ i ] = velemNo[ i ] ;
		work->elemsize[ i ] = velemsize[ i ] ;
		def->elems[ i ] = ( u_char * )MDU_AllocChar( work->elemsize[ i ] ) ;
		if ( def->elems[ i ] == NULL ) return -1 ;
		fread( def->elems[ i ], 1, work->elemsize[ i ], vfp[ i ] ) ;
		fclose( vfp[ i ] ) ;
		if ( work->flag & 0x10 ) {
			printf( "< %d >", velemNo[ i ] ) ;
			DumpVib( def->elems[ i ] ) ;
		}
	}

	fclose( fp ) ;
	work->list_fp = NULL ;
	return 0 ;
}

/* ファイル書き出し */
static	int		WriteVarFile( Work *work )
{
	FILE		*fp ;
	VAR_HEADER	*def1, *def2 ;
	int			size, i, vls ;
	u_short		*elemNo ;
	u_char		**elems, *elem ;

	def1 = &work->def ;
	vls = ( def1->flag & GM_VAR_FLAG_VLS ) ? 1 : 0 ;

	size = sizeof( VAR_HEADER ) ;
	size = Align4( size ) ;
	if ( !vls ) {
		size += sizeof( u_short ) * def1->n_elems ;
		size = Align4( size ) ;
	}
	size += sizeof( u_char * ) * def1->n_elems ;
	size = Align4( size ) ;
	for ( i = 0; i < def1->n_elems; i ++ ) {
		size += sizeof( u_char ) * work->elemsize[ i ] ;
		size = Align4( size ) ;
	}
	
	def2 = ( VAR_HEADER * )MDU_Alloc( size ) ;
	if ( def2 == NULL ) return -1 ;
	work->def2 = def2 ;
	
	def2->magic = MAGIC ;
	def2->version = VERSION ;
	def2->flag = def1->flag ;
	def2->n_elems = def1->n_elems ;
	if ( !vls ) {
		def2->elemNo = ( u_short * )( Align4( sizeof( VAR_HEADER ) ) ) ;
		def2->elems = ( u_char ** )( Align4( ( int )def2->elemNo 
											+ sizeof( u_short ) * def1->n_elems ) ) ;
		elemNo = ( u_short * )( ( int )def2 + ( int )def2->elemNo ) ;
	} else {
		def2->elemNo = NULL ;
		def2->elems = ( u_char ** )( Align4( sizeof( VAR_HEADER ) ) ) ;
		elemNo = NULL ;
	}
	elems = ( u_char ** )( ( int )def2 + ( int )def2->elems ) ;
	elem = ( u_char * )( ( int )elems + Align4( sizeof( u_char * ) * def1->n_elems ) ) ;

	for ( i = 0; i < def1->n_elems; i ++ ) {
		if ( !vls ) *( elemNo ++ ) = def1->elemNo[ i ] ;
		*( elems ++ ) = ( u_char * )( ( int )elem - ( int )def2 ) ;
		memcpy( elem, def1->elems[ i ], work->elemsize[ i ] ) ;
		elem = ( u_char * )( ( int )elem + Align4( work->elemsize[ i ] ) ) ;
	}

	fp = fopen( work->out_file, "wb" ) ;
	if ( fp == NULL ) return -1 ;
	fwrite( def2, size, 1, fp ) ;
	MDU_Free( def2 ) ;
	work->def2 = NULL ;
	fclose( fp ) ;

	return 0 ;
}

/* ダンプ */
static	void	DumpVar( char *file )
{
	FILE			*fp = NULL ;
	VAR_HEADER		*def = NULL ;
	int				size, i, vls ;
	u_char			*elem ;

	fp = fopen( file, "rb" ) ;
	if ( fp == NULL ) goto dump_error ;

	size = MDU_GetFileSize( file ) ;
	if ( size <= 0 ) goto dump_error ;

	def = ( VAR_HEADER * )MDU_Alloc( size ) ;
	if ( def == NULL ) goto dump_error ;

	fread( def, size, 1, fp ) ;

	vls = ( def->flag & GM_VAR_FLAG_VLS ) ? 1 : 0 ;
	if ( !vls ) def->elemNo = ( u_short * )( ( int )def + ( int )def->elemNo ) ;
	else def->elemNo = NULL ;
	def->elems = ( u_char ** )( ( int )def + ( int )def->elems ) ;
	for ( i = 0; i < def->n_elems; i ++ ) {
		def->elems[ i ] = ( u_char * )( ( int )def + ( int )def->elems[ i ] ) ;
	}

	printf( "%x flag\n", def->flag ) ;
	printf( "%d elems\n", def->n_elems ) ;
	for ( i = 0; i < def->n_elems; i ++ ) {
		if ( !vls ) printf( "[elem %d/%d]\n", def->elemNo[ i ], i ) ;
		else  	    printf( "[elem %d]\n", def->elemNo[ i ] ) ;
		elem = def->elems[ i ] ;	
		DumpVib( elem ) ;
	}
dump_error :
	if ( def != NULL ) fclose( fp ) ;
	if ( fp != NULL ) fclose( fp ) ;
}

/* メモリ掃除 */
static	void	FreeWork( Work *work )
{
	if ( work != NULL ) {
		if ( work->list_fp != NULL ) fclose( work->list_fp ) ;
		if ( work->vib_fp != NULL ) fclose( work->vib_fp ) ;
		if ( work->out_fp != NULL ) fclose( work->out_fp ) ;
		FreeVar( &work->def ) ;
		if ( work->def2 != NULL ) {
//			FreeVar( work->def2 ) ;
			MDU_Free( work->def2 ) ;
		}
		if ( work->elemsize != NULL ) MDU_Free( work->elemsize ) ;
		MDU_Free( work ) ;
	}
}
#endif

/* メイン */
int		main( int argc, char **argv )
{
	FILE		*fp ;
//	FILE		*outfp ;
#define outfp stdout
	char		buf[ 512 ] ;
	char		file[ 512 ], full[ 512 ] ;
	int			size, i ;

	fp = fopen( argv[ 1 ], "r" ) ;
//	outfp = fopen( "mt3size_res.txt", "w" ) ;
	if ( fp == NULL ) return 0 ;
	while( !feof( fp ) ) {
		if ( fgets( buf, 256, fp ) == NULL ) return 0 ;
		if ( buf[ 0 ] > 'z' || buf[ 0 ] < 'A' ) {
			fprintf( outfp, "\n" ) ;
			continue ;
		}
		strcpy( full, argv[ 2 ] ) ;
		for ( i = 0; i < strlen( buf ); i ++ ) {
			if ( buf[ i ] == '\n' || buf[ i ] == ' ' || 
				 buf[ i ] == '\0' || buf[ i ] == '\t' ) break ;
			file[ i ] = buf[ i ] ;
		}
		file[ i ] = '\0' ;
		strcat( file, ".mt3\0" ) ;
		strcat( full, file ) ;
//		printf( "search file %s\n", full ) ;
		if ( atoi( argv[ 3 ] ) == 0 ) {
			size = MDU_GetFileSize( full ) ;
		} else {
			FILE	*fp2 ;
			int		buf2[ 4 ] ;
			fp2 = fopen( full, "rb" ) ;
			if ( fp2 == NULL ) size = -1 ;
			else {
				fread( buf2, sizeof( int ), 4, fp2 ) ;
				size = buf2[ 3 ] ;
				fclose( fp2 ) ;
			}
		}

		if ( size < 0 ) {
			fprintf( outfp, "0\n" ) ;
			continue ;
		}
		fprintf( outfp, "%d\n", size ) ;
	}
//	fclose( outfp ) ;
	return 1 ;
}
