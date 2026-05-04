#include <stdarg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parse.h"

/*
   バーポーズモード用
*/

int verbose_mode = 0;
int no_warn_flag = 0;

#ifdef __CYGWIN__

static void euc_to_sjis( char *buf )
{
	unsigned char *p;

	p = ( unsigned char * )buf;
	while( *p != '\0' ){
		if( *p & 0x80 ){
			int high, low;
			high = p[ 0 ] - 0x80;
			low = p[ 1 ] - 0x80;
			if( high & 1 ){
				low += 0x1f;
				if( low >= 0x7f ) low++;
			} else {
				low += 0x7e;
			}
			high = ( high - ' ' - 1 ) / 2 + 0x81;
			if( high >= 0xa0 ) high += '@';
			p[ 0 ] = high;
			p[ 1 ] = low;
			p ++;
		} else if( p[ 0 ] == '\\' && p[ 1 ] == 'n' ){
			/* 変換され損なった改行文字 */
			p[ 0 ] = ' ';
			p[ 1 ] = '\n';
			p++;
		}
		p++;
	}
}

static void my_vprintf( char *format, va_list ap )
{
	char buf[ 256 ];

	vsprintf( buf, format, ap );
	euc_to_sjis( buf );
	printf( buf );
}

#define vprintf my_vprintf

#endif

void PRINTF( char *format, ... )
{
	va_list ap;
	va_start( ap, format );
	if( verbose_mode ){
		vprintf( format, ap );
	}
	va_end( ap );
}

void DUMP( char *format, ... )
{
	va_list ap;
	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );
}

void ERROR( char *format, ... )
{
	va_list ap;

	print_file_current_pos( "Error" );
	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );
	print_current_line();

	exit( 1 );
}

void WARNING( char *format, ... )
{
	
	va_list ap;

	if( no_warn_flag ) return;

	print_file_current_pos( "Warning" );
	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );

	print_current_line();
}

void FATAL( char *format, ... )
{
	va_list ap;

	va_start( ap, format );
	vprintf( format, ap );
	va_end( ap );

	exit( 1 );
}

//BP - adding this helper function here because it's the most appropriate
//shared file b/t gclconv and gclk.

#define BP_MAX_ARGC 1024
#define BP_MAX_ARGV_BUF 4096

int bp_argc = 0;
char * bp_argv[BP_MAX_ARGC];

static char bp_argv_buf[BP_MAX_ARGV_BUF] = { 0 };
char * curr_argv_buf = bp_argv_buf;

static void bp_add_arg( const char * const arg )
{
   int len = strlen( arg );
   if( bp_argc == BP_MAX_ARGC )
   {
      FATAL("Error: bp argc overflow!\n");
   }
   if( arg + len + 1 >= bp_argv_buf + sizeof(bp_argv_buf) )
   {
      FATAL("Error: bp argvbuf overflow!\n");
   }
   strcpy( curr_argv_buf, arg );
   bp_argv[bp_argc] = curr_argv_buf;
   ++bp_argc;
   curr_argv_buf += len + 1;
}

void bp_expand_args( int argc, char *argv[] )
{
   int i;
   for( i=0; i < argc; ++i )
   {
      const char * const arg = argv[i];
      if( *arg == '@' )
      {
         char seps[]   = " \r\n";
         int fsize;
         char * token;
         char * argbuf;
         FILE * fp = fopen( arg+1, "rt" );
         if( !fp )
         {
            FATAL("Error: could not open %s for reading!\n", arg+1);
         }
         fseek( fp, 0, SEEK_END );
         fsize = ftell( fp );
         fseek( fp, 0, SEEK_SET );
         argbuf = malloc( fsize+1 );
         memset(argbuf,0,fsize+1);
         fread( argbuf, fsize, 1, fp );
         fclose( fp );

         token = strtok( argbuf, seps );
         while( token )
         {
            bp_add_arg( token );
            token = strtok( NULL, seps );
         }
         free( argbuf );
      }
      else
      {
         bp_add_arg( arg );  
      }
   }
}

