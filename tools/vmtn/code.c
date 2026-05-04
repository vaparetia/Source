#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define _code_c_
#include "code.h"

/*
 * Shift_JIS の上位/下位から、EUC-JP コードへの変換を行う
 */
static int sjis_to_euc(int h, int l)
{
  int euc;

  if(h <= 0x9f)
    h = ((h << 1) - ((l < 0x9f) ? 0xe1 : 0xe0)) & 0xff;
  else
    h = ((h << 1) - ((l < 0x9f) ? 0x161 : 0x160)) & 0xff;

  if(l < 0x7f) l -= 0x1f;
  else if(l < 0x9f) l -= 0x20;
  else l -= 0x7e;

  euc = ((h << 8) | (l & 0xff)) | 0x8080;
  return euc;
}

void code_exchange(char * str)
{
  unsigned char * u_str;
  int l, h, euc;

  u_str = str;

  while(*u_str)
    {
      h = *u_str;
      /* 変換の必要が無い1バイトコードであればそのまま残す */
      /* Shift_JIS コードと JIS X 0201 kana に該当しない文字は、
	 全てそのまま残す。 */
      if(h <= 0x80)
	{
	  u_str++;
	  continue;
	}

      /* !!! 警告 !!!  JIS X 0201 kana には対応しない。 */
      l = *(u_str + 1);
      euc = sjis_to_euc(h, l);
      *u_str = (euc >> 8) & 0xff;
      *(u_str + 1) = euc & 0xff;
      u_str += 2;
    }
}
