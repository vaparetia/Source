#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _test_c_
#include "test.h"
#include "vmtn_multi.h"

int main(int argc, char **argv)
{
  vmtnContainer * cont;
  vmtnSpeaker   * speak;

  if(NULL == (cont = vmmCreateContainer()))
    {
      fprintf(stderr, "could not create container.\n");
      return EXIT_FAILURE;
    }
  if(NULL == (speak = vmmReadSpeakerMotion("snake", "motions.inf")))
    {
      vmmReleaseContainer(cont);
      fprintf(stderr, "could not read motions.");
      return EXIT_FAILURE;
    }
  
  if(NULL == vmmRegistSpeaker(cont, speak))
    {
      vmmReleaseContainer(cont);

      return EXIT_FAILURE;
    }
  


  
  vmmReleaseContainer(cont);
  printf("success.\n");
  return EXIT_SUCCESS;
}
