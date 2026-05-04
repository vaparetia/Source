#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include "vagdata.h"
#include "vxcodec.h"

int main(int argc, char **argv)
{
  FILE *rfp;
  VAG_header * head;
  WORD * raw;
  int fd;
  LONG siz;
  int i;
  if(0 > (fd = open("/dev/dsp", O_WRONLY)))
    {
      fprintf(stderr, "could not open /dev/dsp device.\n");
      return EXIT_FAILURE;
    }

  for(i = 1; i < argc; i++)
    {
      if(NULL == (rfp = fopen(argv[i], "rb")))
	{
	  fprintf(stderr, "could not open file '%s'.\n", argv[1]);
	  goto err;
	}
      
      head = vagReadHeader(NULL, rfp);
      raw = vagDecodeAll(&siz, head, rfp);
      
      vagSetDSPdevice(fd, head);
      write(fd, raw, siz);

      free(raw);
      free(head);
      fclose(rfp);
    }

  close(fd);
  return EXIT_SUCCESS;

 err:
  close(fd);
  return EXIT_FAILURE;
}
