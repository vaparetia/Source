/*
   qar (quad word ar) メインルーチン
   $Id: qar.c,v 1.4 2000/07/07 09:16:14 usr01475 Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

extern long get_strcode( char *str );

/*
	PS2でquad word境界にそろえるためのフォーマット。
	すべての入力ファイルはquad word単位のサイズ。
	フォーマットは以下のとおり

start:
	file1
	file2
	file3
	:
ofs_table:
	dc.w filenum
	dc.w flag		// 0=no symbol, 1=include symbol
	dc.l file1 id
	dc.l file1 size
	dc.l file2 id
	dc.l file2 size
	dc.l file3 id
	dc.l file3 size
	:
-- symbol format
	db	file1name
	db	file2name
	db	file3name
	:
--
	dc.l ofs_table
*/

#define TRUE	1
#define FALSE	0

static int verbose_mode = FALSE;
static int split_symbol_mode = FALSE;

static void usage( void )
{
	printf( "qar {cth}[vs] <qarfile> [<inputfile>...]\n" );
	printf( "  options:\n" );
	printf( "    h : show this help.\n" );
	printf( "    c : create qar.\n" );
	printf( "    t : show files list.\n" );
	printf( "    v : verbose.\n" );
	printf( "    s : split symbol.\n" );
   printf( "    x : extract files.\n" );
	exit( 1 );
}

enum {
	NO_COMMAND = 0,
	COMMAND_CREATE = 1,
	COMMAND_LIST = 2,
   COMMAND_EXTRACT = 3
};

static int analyze_option( char *options )
{
	char *p;
	int mode = NO_COMMAND;

	for( p = options; *p != '\0'; p++ ){
		switch( *p ){
		  case 'c':
			if( mode != NO_COMMAND ){
				printf( "duplicate command\n" );
				exit( 1 );
			}
			mode = COMMAND_CREATE;
			break;
		  case 't':
			if( mode != NO_COMMAND ){
				printf( "duplicate command\n" );
				exit( 1 );
			}
			mode = COMMAND_LIST;
			break;
		  case 'v':
			verbose_mode = TRUE;
			break;
		  case 's':
			split_symbol_mode = TRUE;
			break;
		  case '-':
			/* 無視 */
			break;
        case 'x':
         mode = COMMAND_EXTRACT;
         break;
		  case 'h':
		  default:
			usage();
			break;
		}
	}
	if( mode == NO_COMMAND ){
		usage();
	}
	return mode;
}

#define BUFFER_SIZE	(1024*1024)

static int filecopy( FILE *out, char *infile )
{
	FILE *in;
	char *buffer;
	int size;

	buffer = malloc( BUFFER_SIZE );
	if( buffer == NULL ){
		printf( "NO MEMORY\n" );
		exit( 1 );
	}
	if( ( in = fopen( infile, "rb" ) ) == NULL ){
		printf( "NO FILE %s\n", infile );
		return -1;
	}
	size = 0;
	for( ;; ){
		int readsize;
		readsize = fread( buffer, sizeof( char ), BUFFER_SIZE, in );
		fwrite( buffer, sizeof( char ), readsize, out );
		size += readsize;
		if( readsize < BUFFER_SIZE ){
			break;
		}
	}

	free( buffer );
	fclose( in );

	return size;
}

typedef struct {
	int id;
	int size;
} FILEINFO;

static void fputw( int value, FILE *fp )
{
	short v;
	v = ( short )value;
	fwrite( &v, sizeof( short ), 1, fp );
}

static void trim_align( FILE *fp, int align )
{
	int pos;
	pos = ftell( fp );
	if( pos % align > 0 ){
		int i;
		for( i = align - pos % align; i > 0; i-- ){
			fputc( 0, fp );
		}
	}
}

static int get_fileid( char *filename )
{
	char *p;
	static char root[ 256 ];
	int ext = 0;

	p = strrchr( filename, '/' );
	if( p == NULL ){
		p = filename;
	} else {
		p ++;
	}
	strcpy( root, p );
	if( ( p = strrchr( root, '.' ) ) != NULL ){
		ext = *( p + 1 ) - 'a';
		*p = '\0';
	}
	return get_strcode( root ) | ( ext << 24 );
}

static void make_qword_archive( char *outfile, char **infile, int in_file_num )
{
	FILE *out;
	FILEINFO *fileinfo;
	int i;
	int total;

	fileinfo = malloc( sizeof( FILEINFO ) * in_file_num );
	if( fileinfo == NULL ){
		printf( "NO MEMORY\n" );
		exit( 1 );
	}

	if( ( out = fopen( outfile, "wb" ) ) == NULL ){
		printf( "NOT OPEN FILE %s\n", outfile );
		exit( 1 );
	}

	for( i = 0; i < in_file_num; i++ ){
		int size;

		if( verbose_mode ){
			printf( "add %s\n", infile[ i ] );
		}

		size = filecopy( out, infile[ i ] );
		if( size <= 0 ){
			printf( "Error Occured\n" );
			fclose( out );
			remove( outfile );
			exit( 1 );
		}
		if( size % 128 != 0 ){
			printf( "Warning: %s is not 128 align\n", infile[ i ] );
			trim_align( out, 128 );
		}
		fileinfo[ i ].id = get_fileid( infile[ i ] );
		fileinfo[ i ].size = size;

		if( verbose_mode ){
			printf( "id %08X size %d\n", fileinfo[ i ].id, size );
		}
	}

	total = ftell( out );

	fputw( in_file_num, out );
	fputw( split_symbol_mode, out );

	fwrite( fileinfo, sizeof( FILEINFO ), in_file_num, out );

	if( split_symbol_mode == FALSE ){
		/* ファイル名出力 */
		for( i = 0; i < in_file_num; i++ ){
			char *p;

			p = strrchr( infile[ i ], '/' );
			if( p == NULL ){
				p = infile[ i ];
			} else {
				p ++;
			}
			fwrite( p, sizeof( char ), strlen( p ) + 1, out );
		}
	}

	trim_align( out, 4 );

	fwrite( &total, sizeof( int ), 1, out );

	fclose( out );
	free( fileinfo );
}

typedef void (*TIterFn)(FILEINFO *info, char *filename, FILE *fpArchive);

static void iterate_qword_archive( char *arfile, TIterFn iterFn, int numFilesToMatch, char **filesToMatch )
{
   FILE *fp;
   int end, table;
   char *buffer;
   FILEINFO *info;
   char *filename;
   int filenum;
   int symflag;
   long int offset;

   if( ( fp = fopen( arfile, "rb" ) ) == NULL ){
      printf( "NOT OPEN %s\n", arfile );
      exit( 1 );
   }

   fseek( fp, - sizeof( int ), SEEK_END );
   end = ftell( fp );
   fread( &table, sizeof( int ), 1, fp );

   buffer = malloc( end - table );
   if( buffer == NULL ){
      printf( "NO MEMORY\n" );
      exit( 1 );
   }

   fseek( fp, table, SEEK_SET );
   fread( buffer, sizeof( char ), end - table, fp );

   filenum = *( short * )( buffer + 0 );
   symflag = *( short * )( buffer + 2 );
   info = ( FILEINFO * )( buffer + 4 );
   filename = ( char * )( info + filenum );

   offset = 0;
   {
      int i;
      for( i = 0; i < filenum; i++ ){
         fseek(fp, offset, SEEK_SET);
         if (numFilesToMatch == 0)
         {
            iterFn(info, symflag ? NULL : filename, fp);
         }
         else
         {
            int j;
            for (j = 0; j < numFilesToMatch; ++j)
            {
               if (symflag)
               {
                  char caId[12];
                  sprintf(caId, "%08x", info->id);
                  if (!_strnicmp(caId, filesToMatch[j], strlen(caId)))
                  {
                     iterFn(info, NULL, fp);
                  }
               }
               else if (!_strnicmp(filesToMatch[j], filename, strlen(filename)))
               {
                  iterFn(info, filename, fp);
               }
            }
         }
         if (!symflag)
         {
            filename += strlen(filename) + 1;
         }
         offset += info->size;
         info ++;
      }
   }
   free( buffer );
   fclose( fp );
}

static void dump_qword_archive_iterator(FILEINFO *info, char *filename, FILE *fpArchive)
{
   printf( "ID %08X size %d", info->id, info->size );
   if( filename != NULL ){
      printf( "\t: %s\n", filename );
   } else {
      printf( "\n" );
   }
}

void extract_qword_archive_iterator(FILEINFO *info, char *filename, FILE *fpArchive)
{
   FILE *fpItem;
   char caFile[16];
   sprintf(caFile, "Q%08x.q", info->id);
   if (filename == NULL)
   {
      filename = caFile;
   }
   fpItem = fopen(filename, "wb");
   if (fpItem != NULL)
   {
      char *buf = (char *) malloc(info->size);
      if (buf != NULL)
      {
         fread(buf, 1, info->size, fpArchive);
         fwrite(buf, 1, info->size, fpItem);
         free(buf);
         printf("Extracting '%s'\n", filename);
      }
      fclose(fpItem);
   }
}

int main( int argc, char *argv[] )
{
	int mode;

	if( argc < 2 ){
		usage();
	}
	mode = analyze_option( argv[ 1 ] );
	if( mode == COMMAND_CREATE )
   {
		make_qword_archive( argv[ 2 ], &argv[ 3 ], argc - 3 );
	}
   else if( mode == COMMAND_LIST )
   {
      iterate_qword_archive(argv[2], dump_qword_archive_iterator, 0, NULL);
	}
   else if (mode == COMMAND_EXTRACT)
   {
      char *arName = argv[2];
      if (argc == 3)
      {
         iterate_qword_archive(arName, extract_qword_archive_iterator, 0, NULL);
      }
      else
      {
         char **fileNames = &argv[3];
         int numFiles = argc-3;
         iterate_qword_archive(arName, extract_qword_archive_iterator, numFiles, fileNames);
      }
   }
	
	return 0;
}
