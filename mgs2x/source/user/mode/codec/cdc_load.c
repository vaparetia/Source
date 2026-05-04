//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
  cdc_load.c
  無線用データロードモジュール
  2000/09/29   Y.Kira

  $Id: cdc_load.c,v 1.2 2002/12/20 06:01:28 takaki Exp $
*/
#endif

#ifdef PSX2
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
#endif
#ifdef KP_XBOX
#include <xtl.h>
#endif

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

#ifdef PSX2
#undef _DEBUG_
#ifdef _DEBUG_
#define DBG(...)  printf(__VA_ARGS__)
#else
#define DBG(...)
#endif /* _DEBUG_ */
#endif
#ifdef KP_XBOX
#define DBG
#endif

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
  
  printf("entry: 0x%08x    offset: 0x%08x    size: 0x%08x(%d)\n",
	 entry, *offset, *size, *size);

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

  if ( buffer_top != NULL )
  {
     printf( "WARNING!  Memory Leak when loading CDC Resources!\n" );
     CDC_ReleaseLogicalDirectory();
     BP_BREAK;
  }
//  if(NULL != buffer_top) codecFree(buffer_top);

  CDC_CalcFileParamByEntry(entry, &offset, &size);
  if(NULL == (buffer = codecMalloc(size + 128))) return NULL;
  buffer_top = buffer;
  printf("CDC_LoadLogicalDirectory(): buffer_top = %p\n", buffer_top);
  data_top = (void *)(((unsigned int)buffer + 127) & 0xffffff80);

  FS_SlotPackLoadSet( data_top, entry );

  return data_top;
}

int CDC_SyncCodecDataLoad(void)
{
  int status;

  status = FS_SlotPackLoadSync();
  return status;
}

void CDC_InitLoadData(void)
{
   FS_SlotPackInit( data_top, SLOTPACK_INIT_MGS2_FACE );
}

void CDC_ReleaseLogicalDirectory(void)
{ 
   if ( buffer_top == NULL )
   {
      return;
   }

   FS_SlotPackClose( data_top, SLOTPACK_INIT_MGS2_FACE );

#ifdef KP_WINDOWS
	DG_ExeBufferSync(FALSE) ;	// 危険なので、とりあえず描画終了待ち
#endif

  codecFree(buffer_top);
  printf("=================== Free Face Buffer ===========\n");
  data_top = buffer_top = NULL;
  DBG("CDC_ReleaseLogicalDirectory() is success.\n");
}


void * CDC_GetFileEntry(int strcode, int suffix)
{
  int id;

  if(NULL == data_top) return NULL;
  
  /* 照合用の ID を作成する */
  id = ((suffix - 'a') << 24) | strcode;

  return FS_SlotPackGetData( data_top, id );
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
