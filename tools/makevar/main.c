/*
   main.c
   makevar : main

   2000/10/12	M.Sonoyama
   $Id: main.c,v 1.7 2002/01/28 05:35:35 usr02011 Exp $
*/

#include 	<stdio.h>
#include 	<stdlib.h>
#include 	<string.h>

#include	"MDU_util.h"

#define	DEFAULT_INPUT_DIR	"/u/develop/mj001data/fix/vib"
#define	MAGIC				(10101010)
#define	VERSION				(2)
//#define	VERSION				(1)

#define	NEW

static	int		DumpMode = 0 ;

typedef	struct	{
	int			magic ;
	u_short		version ;
	u_short		flag ;
	int			n_elems ;
	u_short		*elemNo ;
#ifdef NEW
	int			*name_id ;
#endif
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

	char			vibfilelist[ 512 ][ 256 ] ;
	int				n_vibfile ;

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
#ifdef NEW
	if ( def->name_id != NULL ) MDU_Free( def->name_id ) ;
#endif
}

/* 使用法 */
static	void	Usage( void )
{
	printf( "makevar : vib-file archiver\n" ) ;
	printf( "\t Usage : makevar list-file[.mls||.vls] [-options]\n" ) ;
	printf( "\t options :\n" ) ;
	printf( "\t -o <file> : Set output file name.\n" ) ;
	printf( "\t -l <dir> : Directory for input files.\n" ) ;
	printf( "\t -d       : Direct Vibfile List Mode.\n" ) ;
	printf( "\t -t       : Varfile Dump Mode\n" ) ;
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
	if ( MDU_GetOption( 't' ) != NULL ) {
		DumpMode = 1 ;
		return 0 ;
	}
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
#ifdef NEW
	if ( MDU_GetOption( 'd' ) != NULL ) {
		int			i ;
		char		*ext ;

		work->flag |= 0x20 ;	/* Direct Vibflie-list Mode */
		work->n_vibfile = 0 ;
		for ( i = 0; i < argc; i ++ ) {
			if ( work->n_vibfile >= 512 ) {
				printf( "too many vibfile-list!!\n" ) ;
				break ;
			}
			ext = MDU_GetExtension( argv[ i ] ) ;
			if ( strcmp( ext, "vib" ) ) continue ;
			strcpy( work->vibfilelist[ work->n_vibfile ], argv[ i ] ) ;
			++ work->n_vibfile ;
		}
	}
#endif
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

static	int		ComOpen = 0 ;

static	int		CommentCheck( char **elem, int *n_elems ) 
{
	int			i, j, len, end ;
	int			m, n ;
	char		**elem2 ;
	int			n_elems2 ;

	elem2 = ( char ** )MDU_AllocChar2( 16, 256 ) ;
	if ( elem2 == NULL ) exit( -1 ) ;
	n_elems2 = 0 ;

	m = n = 0 ;
	end = 0 ;

	for ( i = 0; i < *n_elems; i ++ ) {
		len = strlen( elem[ i ] ) ;
		n = 0 ;
		for ( j = 0; j < len; j ++ ) {
			if ( j != len - 1 ) {
				if ( ComOpen == 0 &&
					elem[ i ][ j ] == '/' &&
					elem[ i ][ j + 1 ] == '/' ) {
					/* 行コメント */
					if ( n != 0 ) elem2[ m ][ n ] = '\0' ;
					end = 1 ;
					break ;
				}
				if ( elem[ i ][ j ] == '/' &&
					elem[ i ][ j + 1 ] == '*' ) {
					/* コメント開始 */
					if ( ComOpen == 0 && n != 0 ) elem2[ m ][ n ] = '\0' ;
					ComOpen ++ ;
					j ++ ;
					continue ;
				} else if ( elem[ i ][ j ] == '*' &&
						   elem[ i ][ j + 1 ] == '/' ) {
					/* コメント終了 */
					ComOpen -- ;
					j ++ ;
					continue ;
				}
			}
			if ( ComOpen == 0 ) {
				elem2[ m ][ n ] = elem[ i ][ j ] ;
				n ++ ;
			}
		}
		if ( n != 0 ) {
			elem2[ m ][ n ] = '\0' ;
			m ++ ;
			n_elems2 = m ;
		}
		if ( end ) break ;
	}
	if ( ComOpen < 0 ) {
		printf( "comment broken\n" ) ;
		exit( -1 ) ;
	}
#if 0
	for ( i = 0; i < *n_elems; i ++ ) {
		printf( "%s ", elem[ i ] ) ;
	}
	if ( *n_elems != 0 ) printf( "\n" ) ;
#endif
	for ( i = 0; i < n_elems2; i ++ ) {
		len = strlen( elem2[ i ] ) ;
		for ( j = 0; j < len; j ++ ) {
			elem[ i ][ j ] = elem2[ i ][ j ] ;
		}
		elem[ i ][ j ] = '\0' ;
	}
#if 0
	printf( "%d elems : ", n_elems2 ) ;
	for ( i = 0; i < n_elems2; i ++ ) {
		printf( "%s ", elem[ i ] ) ;
	}
	printf( "\n" ) ;
#endif
	*n_elems = n_elems2 ;
	MDU_FreeChar2( elem2, 16 ) ;
	return ( n_elems2 == 0 ) ;
}

/* includeファイル読み込み */
static	int	ReadIncludeFile( Work *work, char *file, int *n, int *vn, FILE *vfp[],
							 int velemNo[], int velemsize[] )
{
	FILE			*fp ;
	char			**elem, vib[ 512 ], *ext ;
	int				n_elems ;
	int				vls, error ;
	VAR_HEADER		*def ;

	error = 0 ;
	elem = ( char ** )MDU_AllocChar2( 16, 256 ) ;
	if ( elem == NULL ) return -1 ;
	file ++ ;	/* 進める */
	ext = strrchr( file, '"' ) ;
	*ext = '\0' ;

	fp = fopen( file, "r" ) ;
	if ( fp == NULL ) {
		MDU_FreeChar2( elem, 16 ) ;
		return -1 ;
	}
printf( "include %s\n", file ) ;
	def = &work->def ;
	ext = MDU_GetExtension( file ) ;
	if ( !strcmp( ext, "vls" ) ) {
		vls = 1 ;
	} else {
		vls = 0 ;
	}
	while( MDU_GetSentenceElem( fp, elem, &n_elems ) >= 0 ) {
		if ( n_elems == 0 ) continue ;
		if ( CommentCheck( elem, &n_elems ) ) continue ;
		if ( n_elems == 0 ) continue ;

		if ( !strcmp( elem[ 0 ], "#include" ) ) {
			if ( ReadIncludeFile( work, elem[ 1 ], n, vn, vfp, velemNo, velemsize ) < 0 ) {
				error = -1 ;
				break ;
			}
			continue ;
		}
		if ( elem[ 0 ][ 0 ] == '/' &&
			 elem[ 0 ][ 1 ] == '/' ) {
			continue ;
		}
		ext = MDU_GetExtension( elem[ 0 ] ) ;	
		if ( vls ) {
			if ( strcmp( ext, "vib" ) ) continue ;
		} else {
			if ( strcmp( ext, "mt3" ) && strcmp( ext, "mtn" ) ) continue ;
		}
		MDU_ChangeExtension( elem[ 0 ], "vib" ) ;
		strcpy( vib, work->input_dir ) ;
		strcat( vib, "/" ) ;
		strcat( vib, elem[ 0 ] ) ;
		vfp[ *n ] = fopen( vib, "rb" ) ;
		velemNo[ *n ] = *vn ;
		*vn = *vn + 1 ;
		if ( *vn >= 512 ) {
			error = -1 ;
			break ;
		}
		if ( vfp[ *n ] == NULL ) {
			if ( vls ) {
				printf( "makevar warning : %s is not exist.\n", vib ) ;
			}
			continue ;
		}
		velemsize[ *n ] = MDU_GetFileSize( vib ) ;
		if ( velemsize[ *n ] <= 0 ) continue ;
		if ( work->flag & 0x10 ) {
			printf( "find %s[%d][%d]\n", vib, velemNo[ *n ], *n ) ;
		}
		*n = *n + 1 ;
		if ( *n >= 512 ) {
			error = -1 ;
			break ;
		}
	}
	MDU_FreeChar2( elem, 16 ) ;
	fclose( fp ) ;
	return error ;
}

/* リストファイル読み込み */
static	int	ReadListFile( Work *work )
{
	FILE			*fp, *vfp[ 512 ] ;
	char			**elem, vib[ 512 ], *ext ;
	int				n_elems, n, vn, velemNo[ 512 ], velemsize[ 512 ] ;
	int				i, vls, comment ;
	VAR_HEADER		*def ;

	elem = ( char ** )MDU_AllocChar2( 16, 256 ) ;
	if ( elem == NULL ) return -1 ;

	fp = fopen( work->list_file, "r" ) ;
	if ( fp == NULL && !( work->flag & 0x20 ) ) {
		MDU_FreeChar2( elem, 16 ) ;
		return -1 ;
	}

	def = &work->def ;
	ext = MDU_GetExtension( work->list_file ) ;
#ifdef NEW
	if ( work->flag & 0x20 ) {
		def->flag = GM_VAR_FLAG_VLS ;
		vls = 1 ;
	} else if ( !strcmp( ext, "vls" ) ) {
#else
	if ( !strcmp( ext, "vls" ) ) {	
#endif
		def->flag = GM_VAR_FLAG_VLS ;
		vls = 1 ;
	} else {
		def->flag = GM_VAR_FLAG_MLS ;
		vls = 0 ;
	}
	work->list_fp = fp ;
	n = vn = 0 ;
#ifdef NEW
	if ( work->flag & 0x20 ) {
		/* Direct Vibfile-list Mode */
		for ( i = 0; i < work->n_vibfile; i ++ ) {
			strcpy( vib, work->input_dir ) ;
			strcat( vib, "/" ) ;
			strcat( vib, work->vibfilelist[ i ] ) ;
			//printf( "%s %d\n", vib, MDU_GetStrCode( MDU_TailName( vib ) ) ) ;
			vfp[ i ] = fopen( vib, "rb" ) ;
			if ( vfp[ i ] == NULL ) {
				printf( "makevar error : %s is not exist.\n", vib ) ;
				return -1 ;
			}
			velemsize[ i ] = MDU_GetFileSize( vib ) ;
		}
		n = work->n_vibfile ;
		goto read_list_file_end ;
	}
#endif
	comment = 0 ;
	ComOpen = 0 ;
	while( MDU_GetSentenceElem( fp, elem, &n_elems ) >= 0 ) {
		if ( n_elems == 0 ) continue ;
		if ( CommentCheck( elem, &n_elems ) ) continue ;
		if ( n_elems == 0 ) continue ;

		if ( !strcmp( elem[ 0 ], "#include" ) ) {
			if ( ReadIncludeFile( work, elem[ 1 ], &n, &vn, vfp, velemNo, velemsize ) < 0 ) break ;
			continue ;
		}
		if ( elem[ 0 ][ 0 ] == '/' &&
			 elem[ 0 ][ 1 ] == '/' ) {
			continue ;
		}
		ext = MDU_GetExtension( elem[ 0 ] ) ;	
		if ( vls ) {
			if ( strcmp( ext, "vib" ) ) continue ;
		} else {
			if ( strcmp( ext, "mt3" ) && strcmp( ext, "mtn" ) ) continue ;
		}
		MDU_ChangeExtension( elem[ 0 ], "vib" ) ;
		strcpy( vib, work->input_dir ) ;
		strcat( vib, "/" ) ;
		strcat( vib, elem[ 0 ] ) ;
		vfp[ n ] = fopen( vib, "rb" ) ;
		velemNo[ n ] = vn ;
		//printf( "%d %s\n", velemNo[ n ], elem[ 0 ] ) ;
		if ( ++ vn >= 512 ) break ;
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
		if ( ++ n >= 512 ) break ;
	}

read_list_file_end :

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
#ifdef NEW
	if ( work->flag & 0x20 ) {
		/* Direct Mode */
		def->name_id = ( int * )MDU_AllocInt( n ) ;
		if ( def->name_id == NULL ) return -1 ;
	} else {
		def->name_id = NULL ;
	}
#endif
	for ( i = 0; i < n; i ++ ) {
		if ( !vls ) def->elemNo[ i ] = velemNo[ i ] ;
		work->elemsize[ i ] = velemsize[ i ] ;
		def->elems[ i ] = ( u_char * )MDU_AllocChar( work->elemsize[ i ] ) ;
		if ( def->elems[ i ] == NULL ) return -1 ;
		fread( def->elems[ i ], 1, work->elemsize[ i ], vfp[ i ] ) ;
		fclose( vfp[ i ] ) ;
#ifdef NEW
		if ( work->flag & 0x20 ) {
			def->name_id[ i ] = MDU_GetStrCode( MDU_TailName( work->vibfilelist[ i ] ) ) ;
		}
#endif
		if ( work->flag & 0x10 ) {
			printf( "< %d >", velemNo[ i ] ) ;
			DumpVib( def->elems[ i ] ) ;
		}
	}

	if ( fp != NULL ) fclose( fp ) ;
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
	int			*name_id ;

	def1 = &work->def ;
	vls = ( def1->flag & GM_VAR_FLAG_VLS ) ? 1 : 0 ;

	size = sizeof( VAR_HEADER ) ;
	size = Align4( size ) ;
	if ( !vls ) {
		size += sizeof( u_short ) * def1->n_elems ;
		size = Align4( size ) ;
	}
#ifdef NEW
	if ( work->flag & 0x20 ) {
		size += sizeof( int ) * def1->n_elems ;
		size = Align4( size ) ;
	}
#endif
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
#ifdef NEW
		def2->name_id = NULL ;
#endif
		elemNo = ( u_short * )( ( int )def2 + ( int )def2->elemNo ) ;
		name_id = NULL ;
	} else {
		def2->elemNo = NULL ;
#ifdef NEW
		if ( work->flag & 0x20 ) {
			def2->name_id = ( int * )( Align4( sizeof( VAR_HEADER ) ) ) ;
			def2->elems = ( u_char ** )( Align4( ( int )def2->name_id 
												+ sizeof( int ) * def1->n_elems ) ) ;
			name_id = ( int * )( ( int )def2 + ( int )def2->name_id ) ;
		} else {
			def2->elems = ( u_char ** )( Align4( sizeof( VAR_HEADER ) ) ) ;
			name_id = NULL ;
		}
#else
		def2->elems = ( u_char ** )( Align4( sizeof( VAR_HEADER ) ) ) ;
		name_id = NULL ;
#endif
		elemNo = NULL ;
	}
	elems = ( u_char ** )( ( int )def2 + ( int )def2->elems ) ;
	elem = ( u_char * )( ( int )elems + Align4( sizeof( u_char * ) * def1->n_elems ) ) ;

	for ( i = 0; i < def1->n_elems; i ++ ) {
		if ( !vls ) *( elemNo ++ ) = def1->elemNo[ i ] ;
		*( elems ++ ) = ( u_char * )( ( int )elem - ( int )def2 ) ;
#ifdef NEW
		if ( work->flag & 0x20 ) *( name_id ++ ) = def1->name_id[ i ] ;
#endif
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
/* Version 2以外は読めません */
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
#ifdef NEW
	if ( def->name_id != NULL ) {
		def->name_id = ( int * )( ( int )def + ( int )def->name_id ) ;
	}
#endif
	for ( i = 0; i < def->n_elems; i ++ ) {
		def->elems[ i ] = ( u_char * )( ( int )def + ( int )def->elems[ i ] ) ;
	}

	printf( "%x flag\n", def->flag ) ;
	printf( "%d elems\n", def->n_elems ) ;
	for ( i = 0; i < def->n_elems; i ++ ) {
		if ( !vls ) printf( "[elem %d/%d]\n", def->elemNo[ i ], i ) ;
		else  	    printf( "[elem %d]\n", i ) ;
#ifdef NEW
		if ( def->name_id != NULL ) printf( "[name_id %d]\n", def->name_id[ i ] ) ;
#endif
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

/* メイン */
int		main( int argc, char **argv )
{
	Work		*work = NULL ;

	DumpMode = 0 ;
	work = ( Work * )MDU_Calloc( 1, sizeof( Work ) ) ;
	if ( work == NULL ) goto error_end ;
	if ( Initialize( work ) < 0 ) goto error_end ;
	if ( Analyze( work, argc, argv ) < 0 ) goto error_end ;

	if ( DumpMode == 1 ) {
		DumpVar( argv[ 1 ] ) ;
		FreeWork( work ) ;
		return 0 ;
	}

	if ( ReadListFile( work ) < 0 ) goto error_end ;
	if ( work->def.n_elems == 0 ) {
		printf( "makevar : no vibfile\n" ) ;
		FreeWork( work ) ;
		return 0 ;
	}

	if ( WriteVarFile( work ) < 0 ) goto error_end ;
	FreeWork( work ) ;

	//DumpVar( work->out_file ) ;

	return 0 ;
error_end :
	Usage() ;
	FreeWork( work ) ;
	return -1 ;
}
