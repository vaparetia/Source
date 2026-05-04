/*
  cdc_load.c
  無線用データロードモジュール
  2000/09/29   Y.Kira

  $Id: cdc_load.c,v 1.1.1.3 2002/11/19 11:45:05 Yoshizawa1 Exp $
*/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#include "gameheader.h"
#include "codec.h"

#include "font.h"
#include "cjimaku.h"
#include "libfs.h"
#include "libdg.h"
#include "libmt.h"
#include "libutl.h"

#define _cdc_load_c_
#include "cdc_load.h"
#include "codecmem.h"
#include "codec_config.h"

#ifdef _DEBUG_
#define DBG(args...)  printf(args)
#else
#define DBG(args...)
#endif /* _DEBUG_ */

#define DIR_SIZE(_num) ((((_num) * 8 + 4 + FS_SECTOR_SIZE-1) / FS_SECTOR_SIZE) * FS_SECTOR_SIZE)

static void * data_top   = NULL;
static void * buffer_top = NULL;

void CDC_InitLoadingModule(void)
{
  data_top = buffer_top = NULL;
}

/*
 * 物理ファイル上における、論理ディレクトリの位置およびサイズ情報を得る。
 */
int CDC_CalcFileParamByEntry(unsigned int entry,
			     unsigned int *offset, unsigned int * size)
{
  *offset = entry & 0xfffff;
  *size   = (entry >> 20) * FS_SECTOR_SIZE;
  
  printf("entry: %d    offset: %d    size: %d\n", entry, *offset, *size);

  return 0;
}


/*
 * 無線用データロードのリクエスト発行。
 * 論理ディレクトリを読み込み、読み込んだディレクトリバッファのポインタを返す。
 */
void * CDC_LoadLogicalDirectory(int entry)
{
  unsigned int offset, size;
  void * buffer;

  if(NULL != buffer_top) codecFree(buffer_top);

  CDC_CalcFileParamByEntry(entry, &offset, &size);
  if(NULL == (buffer = codecMalloc(size + 128))) return NULL;
  buffer_top = buffer;
  printf("CDC_LoadLogicalDirectory(): buffer_top = %p\n", buffer_top);
  data_top = (void *)(((unsigned int)buffer + 127) & 0xffffff80);

  FS_LoadFileRequest(FS_FILE_FACE, offset, size, data_top);
  return data_top;
}

int CDC_SyncCodecDataLoad(void)
{
  int status;

  status = FS_LoadFileSync();
  return status;
}

void CDC_InitLoadData(void)
{
  struct {
    int tagnum;
    struct {
      int id;
      int offset;
    } tags[0];
  } * header;
  int i, begin;
  
  header = data_top;
  begin = DIR_SIZE(header->tagnum);
  
  for(i = 0; i < header->tagnum; i++)
    {
      if((header->tags[i].id & 0xff000000) == 0x7f000000) continue;
      DBG("ID = %08x\n", header->tags[i].id);
      GV_LoadInit( (char *)data_top + header->tags[i].offset + begin,
		   header->tags[i].id, GV_INIT_NOCACHE );
    }
}

void CDC_ReleaseLogicalDirectory(void)
{ 
  struct {
    int tagnum;
    struct {
      int id;
      int offset;
    } tags[0];
  } * header;
  int i, begin;

  if(buffer_top == NULL) return;  /* メモリが確保されていなければ何もしない */


  header = data_top;
  begin = DIR_SIZE(header->tagnum);
  
  for(i = 0; i < header->tagnum; i++)
    {
      if((header->tags[i].id & 0xff000000) != 0x13000000) continue;
      DBG("Release Texture(ID = %08x)\n", header->tags[i].id);
      DG_FreeTextureCache(data_top + header->tags[i].offset + begin);
    }
  printf("CDC_ReleaseLogicalDirectory(): buffer_top = %p\n", buffer_top);
  codecFree(buffer_top);
  printf("=================== Free Face Buffer ===========\n");
  data_top = buffer_top = NULL;
  DBG("CDC_ReleaseLogicalDirectory() is success.\n");
}


void * CDC_GetFileEntry(int strcode, int suffix)
{
  int i, id, offset;
  struct {
    int tagnum;
    struct {
      int id;
      int offset;
    } tags[0];
  } * header;
  int begin;

  if(NULL == (header = data_top)) return NULL;
  
  /* 照合用の ID を作成する */
  id = ((suffix - 'a') << 24) | strcode;

  begin = DIR_SIZE(header->tagnum);
  offset = -1;
  for(i = 0; i < header->tagnum; i++)
    {
      DBG("ID: %08x   file: %08x\n", id, header->tags[i].id);
      if(id == header->tags[i].id)
	{
	  offset = header->tags[i].offset;
	  break;
	}
    }
  
  if(offset < 0) return NULL;
  
  DBG("offset = %08x\n", offset + begin);
  
  return (unsigned char *)data_top + (offset + begin);
}

/*
 * 指定された Evm オブジェクトを参照する
 */
void * CDC_LoadEvmDef(int name)
{
  void * file_entry;

  /*
   * Evm の登録
   */
  file_entry = CDC_GetFileEntry(name, 'e');
  if(NULL == file_entry) return NULL;
  DBG("CDC_LoadEvmDef(): evm entry = %p\n", file_entry);
  return file_entry;
}
