/*
  
  VRの名前のハッシュ値を得る(Web用)
  2002/08/06 M.Kobayashi
  $Id: web_vr_hash.c,v 1.2 2002/08/27 13:41:16 usr10656 Exp $
  
*/

#include	<stdio.h>
#include        <string.h>
#include	"clearcode.h"

char *program_name = "get_vr_hash";

void usage( void )
{
  printf("arguments error.\n");
	printf( "get_vr_hash name\n" );
	exit( 1 );
}

int main ( int argc, char* argv[] )
{	
  char * name;
	if( argc != 2 ) usage();

	name = strdup(argv[1]);
	{
	  char * p;
	  for(p = name; *p; p++) *p = toupper(*p);
	}
	printf("namehash : %d\n", ClearCodeCalcStringsCrc( name ) & 0xff);
	free(name);
	return 0;
}


