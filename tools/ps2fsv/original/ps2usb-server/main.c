#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef _DEBUG_
#define DEBUG(args...)  fprintf(stderr, args)
#else
#define DEBUG(args...)
#endif /* _DEBUG_ */

#define _main_c_
#include "config.h"
#include "txresource.h"
#include "util.h"
#include "linQio.h"

#define RES_SETUP(res, v, label, def) if(NULL == (v = txrRef((res), (label)))) v = (def)

static char * USB_device = NULL;

/*
 * 動作条件等の初期化
 */
static void setup(void)
{
  char *base;
  char *config;   /* 設定ファイルのフルパスを得る */
  FILE *rfp;
  txRes * res;
  char * tmp;

  base = CONFIG_BASE;
  if(NULL == (config = malloc(strlen(base) + strlen(CONFIG_FILE) + 2)))
    return;

  strcpy(config, base);
  if(*(config + strlen(base) - 1) != '/') strcat(config, "/");

  strcat(config, CONFIG_FILE);

  DEBUG("Config: %s\n", config);

  rfp = fopen(config, "r");
  if(NULL == rfp) 
    {
      free(config);
      return;
    }

  res = txrGetResource(rfp);  /* リソースを取得 */
  DEBUG("Config Setup\n");

  /* RootPath の設定 */
  RES_SETUP(res, tmp, "RootPath", DEFAULT_ROOT);
  svSetRootPath(tmp);

  DEBUG("RootPath: %s\n", tmp);

  /* USB 入出力デバイスの指定 */
  if(USB_device != NULL)
    {
      free(USB_device);
      USB_device = NULL;
    }
  RES_SETUP(res, tmp, "Device", DEFAULT_DEVICE);
  USB_device = dup_string(tmp);

  DEBUG("Device: %s\n", tmp);

  txrRelease(res);  /* 初期化が終ったら,リソース構造体を開放する */
  free(config);
  fclose(rfp);

  return;
}


int main(int argc, char **argv)
{
  int ret;

  /*
  if(fork())
    {
      fprintf(stderr, "server running...\n");
      exit(EXIT_SUCCESS);
    }
  */
  ret = EXIT_SUCCESS;

  setup();

  if(svMain(USB_device))
    ret = EXIT_FAILURE;

  svRelease();   /* サーバ処理で確保したメモリの開放 */

  if(NULL != USB_device)
    free(USB_device);

  fprintf(stderr, "server termination.\n");
  return ret;
}

