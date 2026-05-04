/*
  
  VRの名前のハッシュ値を得る
  2002/08/06 M.Kobayashi
  $Id: get_vr_hash.c,v 1.1 2002/08/19 11:03:40 usr03700 Exp $
  
*/

#include	<stdio.h>
#include	"clearcode.h"

char *program_name = "get_vr_hash";

void usage( void )
{
	printf( "get_vr_hash name\n" );
	exit( 1 );
}

int main ( int argc, char* argv[] )
{	

	if( argc != 2 ) usage();

	printf("%x\n", ClearCodeCalcStringsCrc( argv[1] ) & 0xff);

	return 0;
}


