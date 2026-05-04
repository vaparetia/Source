#include <stdio.h>

main( int argc, char *argv[] )
{
    int i ;
    FILE *fp ;

    if ( argc !=3 && argc !=2 )
	printf( "Usage : chng_kmsid <fname> [<id>]\nChange KMS-Format ID(13112177 as default)\n" ) ;
    if ( argc == 3 )
	i = atoi( argv[2] ) ;
    else
	i = 13112177 ;
    if ( (fp = fopen( argv[1], "r+" )) )
    {
	fwrite( &i, sizeof(int), 1, fp ) ;
	fclose( fp ) ;
    }
}
