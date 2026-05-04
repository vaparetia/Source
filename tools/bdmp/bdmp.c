#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static int dump(FILE *rfp);

static unsigned long begin;
static int           begin_enable = 0;

static unsigned long end;
static int           end_enable = 0;

static unsigned char block[256];

static enum {
  MD_BYTE,
  MD_WORD,
  MD_LONG
} mode = MD_BYTE;

static enum {
  ED_LITTLE,
  ED_BIG
} endian_mode = ED_LITTLE;


static void usage(char *argv0)
{
  fprintf(stderr,
	  "\nusage: %s [options] filename\n\n"
	  "  -a begin[-term]      dump area(begin and term is HEX address).\n"
	  "  -w                   WORD(16bit) dump mode.\n"
	  "  -l                   LONG(32bit) dump mode.\n"
	  "  -little              Little Endian(WORD/LONG mode) [default]\n"
	  "  -big                 Big Endian(WORD/LONG mode)\n\n", argv0);

  exit(EXIT_FAILURE);
}

/*
 * 数列を数値に変換する。
 * 符号無し 16bit 表記にのみ対応。
 */
static int str2int(int * ret, char *str)
{
  char *p;
  int c, d;

  *ret = 0;
  p = str;

  while(c = *p++)
    {
      *ret = *ret * 16;
      if(c >= '0' && c <= '9') d = c - '0';
      else
	{
	  c = toupper(c);
	  if(c >= 'A' && c <= 'F') d = c - 'A' + 10;
	  else return -1;
	}

      *ret += d;
    }

  return 0;  
}


static int set_area(char * arg)
{
  int i, len;
  char *str;

  /* 開始アドレスを得る */
  for(i = 0; arg[i] != 0; i++) if(arg[i] == '-') break;

  if(i > 0)
    {
      if(NULL == (str = malloc(i + 1))) return -1;
      strncpy(str, arg, i);
      str[i] = 0;
      if(str2int(&begin, str))
	{
	  free(str);
	  return -1;
	}
      free(str);
      begin_enable = 1;
    }

  /* 終了アドレスを得る */
  /* 終端アドレスが指定されていない場合は、始端アドレスの指定のみ。 */
  if(!arg[i]) return 0;
  len = strlen(arg + i + 1);
  if(!len) return 0;

  if(NULL == (str = malloc(len + 1))) return -1;
  strcpy(str, arg + i + 1);

  if(str2int(&end, str))
    {
      free(str);
      return -1;
    }
  free(str);
  end_enable = 1;
  return 0;
}  

int main(int argc, char **argv)
{
  int i, ret;
  char *fname = NULL;
  FILE *rfp;

  for(i = 1; i < argc; i++)
    {
      if(!strcmp(argv[i], "-a"))
	{
	  /* ダンプ範囲アドレス指定オプション */
	  if(i + 1 >= argc)
	    {
	      fprintf(stderr, "bad parameter.\n");
	      return EXIT_FAILURE;
	    }
	  if(set_area(argv[i + 1]))
	    {
	      fprintf(stderr, "bad parameter.\n");
	      return EXIT_FAILURE;
	    }
	  i++;
	  continue;
	}
      if(!strcmp(argv[i], "-w"))
	{
	  mode = MD_WORD;
	  continue;
	}
      if(!strcmp(argv[i], "-l"))
	{
	  mode = MD_LONG;
	  continue;
	}

      if(!strcmp(argv[i], "-big"))
	{
	  endian_mode = ED_BIG;
	  continue;
	}

      if(!strcmp(argv[i], "-little"))
	{
	  endian_mode = ED_LITTLE;
	  continue;
	}

      if(*argv[i] == '-')
	{
	  fprintf(stderr, "unknown option.\n");
	  return EXIT_FAILURE;
	}
      if(NULL != fname)
	{
	  fprintf(stderr, "bad parameter.\n");
	  return EXIT_FAILURE;
	}
      fname = argv[i];
    }
  if(NULL == fname) usage(argv[0]);
  if(NULL == (rfp = fopen(fname, "rb")))
    {
      fprintf(stderr, "could not open file '%s'\n", fname);
      return EXIT_FAILURE;
    }

  ret = dump(rfp);
  fclose(rfp);
  return ret;
}

static int dump(FILE *rfp)
{
  unsigned long target, pos;
  int max, i;
  int start, term, step;
  int x, y, disp;

  /*
   * 表示モードにより、開始/終端アドレスを補正する
   */
  switch(mode)
    {
    case MD_BYTE:
      step = 1;
      break;
    case MD_WORD:
      step = 2;
      begin &= 0xfffffffe;
      end = (end + 1) & 0xfffffffe;
      break;
    case MD_LONG:
      step = 4;
      begin &= 0xfffffffc;
      end = (end + 3) & 0xfffffffc;
      break;
    }
      
  /* 始点アドレスが指定されている場合は、
     始点を含むブロック先頭までシークする。*/
  pos = 0;
  if(begin_enable)
    {
      target = 0xffffff00 & begin;
      fseek(rfp, target, SEEK_SET);
      pos = ftell(rfp);

      if(pos != target)
	{
	  fprintf(stderr, "begin address is larger than file size.\n");
	  return EXIT_FAILURE;
	}
    }

  do {
    /* バッファのクリンナップ */
    for(i = 0; i < 256; i++) block[i] = 0;
    max = fread(block, 1, 256, rfp);
    start = (begin_enable && (pos < begin)) ? (begin & 0xff) : 0;
    term  = (end_enable && (pos + 256 > end)) ? (end & 0xff) : 256;
    if(term > max) term = max;
    for(y = start / 16; y < (term + 15) / 16; y++)
      {
	printf("%08x  ", y * 16 + pos);
	for(x = 0; x < 16; x += step)
	  {
	    if((y * 16 + x < start) ||
	       (y * 16 + x >= term))
	      {
		for(i = 0; i < step; i++) printf("   ");
		continue;
	      }
	    switch(mode)
	      {
	      case MD_BYTE:
		if(x == 8)
		  printf("-%02x", block[y * 16 + x]);
		else
		  printf(" %02x", block[y * 16 + x]);
		break;
	      case MD_WORD:
		if(endian_mode == ED_BIG)
		  printf("  %02x%02x",
			 block[y * 16 + x], block[y * 16 + x + 1]);
		else
		  printf("  %02x%02x",
			 block[y * 16 + x + 1], block[y * 16 + x]);
		break;
	      case MD_LONG:
		if(endian_mode == ED_BIG)
		  printf("    %02x%02x%02x%02x",
			 block[y * 16 + x],
			 block[y * 16 + x + 1],
			 block[y * 16 + x + 2],
			 block[y * 16 + x + 3]);
		else
		  printf("    %02x%02x%02x%02x",
			 block[y * 16 + x + 3],
			 block[y * 16 + x + 2],
			 block[y * 16 + x + 1],
			 block[y * 16 + x]);

		break;
	      }
	  }

	/* メモリの ASCII イメージを出力する */
	printf(" / ");
	for(x = 0; x < 16; x++)
	  {
	    if((y * 16 + x < start) ||
	       (y * 16 + x >= term))
	      {
		printf(" ");
		continue;
	      }
	    if((block[y * 16 + x] >= 0x20) &&
	       (block[y * 16 + x] < 0x7f)) printf("%c", block[y * 16 + x]);
	    else
	      printf(".");
	  }

	printf("\n");
      }
    pos = ftell(rfp);
  } while((max == 256) && (!end_enable || (end_enable && (pos < end))));
}
