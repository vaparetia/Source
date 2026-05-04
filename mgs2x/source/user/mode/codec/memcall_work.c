//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#include "memcall_work.h"

/*
 * 人名と周波数の対応
 */
CODEC_Memory CODEC_registed_list[8] = {
  {0, 0, -1},  {0, 0, -1},
  {0, 0, -1},  {0, 0, -1},
  {0, 0, -1},  {0, 0, -1},
  {0, 0, -1},  {0, 0, -1}
};

/* 最後に選択したメモリー呼び出し上の項目 */
int          CODEC_mem_last_select = -1;  /* まだ誰も選択されたことがない */

/*
 * 無線メモリー呼び出し全項目リセット
 */
int CODEC_MemCallReset(void)
{
  int i;

  CODEC_mem_last_select = -1;
  for(i = 0; i < 8; i++)
    {
      CODEC_registed_list[i].registed = 0;
      CODEC_registed_list[i].freq     = 0;
      CODEC_registed_list[i].name_res = -1;
    } 
  return 0;
}
