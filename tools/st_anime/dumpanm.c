#include <stdio.h>
#include <ctype.h>

void main( int argc, char *argv[] )
{
    FILE *in = stdin ;
    int   i, block = 100 ;
    int   n_block=0 ;
    char  flg[] = { 0,0,0,0,0,0 } ;
    short s[3] ;

    if ( argc == 1 )
	flg[2] = 1 ;
    for ( ; --argc>0 ; )
    {
	if ( (*++argv)[0] == '-' )
	    switch( (*argv)[1] )
	    {
	    case 'h':
		printf( "'dump' is to dump the binary-file into various way.\n"
			"Usage: dump [-option] <binary file>\n"
			"  -h   show this.\n" ) ;
		exit( 1 ) ;
		break ;
	    case 'b':
		block = atoi( *(++argv) ) ;
		argc-- ;
		break ;
	    default:
		printf( "'dump' is to dump the binary-file into various way.\n" ) ;
	    }
	else if ( (in = fopen( *argv, "rb" ) ) == NULL )
	    printf( "Can't open %s.\n", *argv ), exit( 1 );
    }

    i = 1 ;
    while( !feof(in) )
    {
	fread( s, sizeof(short), 3, in ) ;
	printf( i%3 ? "[%4d %4d %4d]\t": "[%4d %4d %4d]\n", s[0],s[1],s[2]  ) ;
	if ( i >= block )
	    printf( "\n-----%dblocks\n", n_block++ ), i = 0 ;
	i++ ;
    }
}

