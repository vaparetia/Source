/*
	listinfメインルーチン
	$Id: listinf.c,v 1.3 2002/02/06 05:59:36 usr01475 Exp $
*/

#include <stdio.h>
#include <string.h>

#include "infdata.h"
#include "fontconv.h"
#include "mymalloc.h"

int verbose_mode = 1;

static char *program_name = "convinf";

static void usage( void )
{
	printf( "%s <inffile>\n", program_name );

	exit( 1 );
}

//static char strbuffer[ 1024 ];

int main( int argc, char *argv[] )
{
	FILE *fp;
	int i;
	int text_only = 0;

	if( argc < 2 ){
		usage();
		return 1;
	}

	for( i = 1; i < argc; i++ ){
		// inf fileのロード
		char *input_filename = argv[ i ];

		if( strcmp( argv[ i ], "-t" ) == 0 ){
			text_only = 1;
			verbose_mode = 0;
			continue;
		}

		init_Malloc();

		if( ( fp = fopen( input_filename, "rb" ) ) == NULL ){
			printf( "file %s not found\n", input_filename );
			exit( 1 );
		}
		load_inf_file( fp );
		fclose( fp );

		printf( "--------- %s ---------\n", input_filename );
		{
			TALK_DIALOG *p;
			TALK_ACTION *ap;

			ap = get_action_top();

			for( p = get_dialog_top(); p != NULL; p = get_next_dialog( p ) ){
				TALK_AREA *area;

				area = get_talk_area( p->start, p->end );
				if( area == NULL ){
					printf( "???????:\n" );
				} else {
//					printf( "%s:\n", area->string );
					printf( "%X:\n", area->name );
				}
				printf( "%s\n", p->string );

				if( text_only == 0 ){
					if( ap != NULL ){
						for( ;; ){
							TALK_ACTION *next;
							next = ap;
							if( next == NULL ){
								break;
							}
							if( next->start < p->end ){
								printf( "ACTION name %08X val %08X start %d\n"
										, next->name, next->value, next->start );
								ap = get_next_action( ap );
							} else {
								break;
							}
						}
					}
				}
			}
			printf( "\n" );
		}
		free_Malloc();
	}
	return 0;
}
