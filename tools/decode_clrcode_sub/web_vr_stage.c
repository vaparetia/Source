#include <stdio.h>
#include <stdlib.h>

#include "vrstagename.h"

static int strtoi(int *ret, char * str)
{
  char * ptr;
  int par = 10;
  int c;
  int num;

  ptr = str;
  if(*ptr == '0')
    {
      ptr++;
      if(*ptr == 'x')
	{
	  par = 16;
	  ptr++;
	}
      else par = 8;
    }
  *ret = 0;
  while(c = *ptr)
    {
      *ret = *ret * par;
      c = toupper(c);
      if((c >= '0') || (c <= '9')) num = c - '0';
      else if((c >= 'A') || (c <= 'F')) num = c - 'A' + 10;
      else return -1;
      if(num >= par) return -1;
      *ret += num;
      ptr++;
    }
  return 0;
}

static void error(void)
{
  printf("error.\n");
  exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
  int id;
  int plat;
  int region;

  if(argc != 4) error();
  if(strtoi(&id, argv[1])) error();
  if(strtoi(&plat, argv[2])) error();
  if(strtoi(&region, argv[3])) error();

  {
    char player[ VR_STAGENAME_MAX ];
    char mission[ VR_STAGENAME_MAX ];
    char mode[ VR_STAGENAME_MAX ];
    char submode[ VR_STAGENAME_MAX ];
    char level[ VR_STAGENAME_MAX ];

    GetVrStageName( id, plat, region,
		    player, mission, mode, submode, level );

    // submode == "" の場合もある
    printf("player : %s\n", player);
    printf("mission : %s\n", mission);
    printf("stgmode : %s\n", mode);
    printf("stgsubmode : %s\n", submode);
    printf("level : %s\n", level);
  }
  return EXIT_SUCCESS;
}
