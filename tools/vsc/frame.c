#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define _frame_c_
#include "frame.h"
#include "pcmdata.h"

/*
 * フレームウィンドウを初期化
 *
 * Prototype:  int frmInit(PcmFILE *pcm);
 *
 * Args:       pcm      順次読み出す PCM ファイルの管理構造体ポインタ
 * Return:     int      初期化成功時 0 / 失敗時非 0 を返す。
 */
int frmInit(PcmFILE *pcm)
{
  /* 先頭から 25[ms] 分読み出す */
  frmWindowSize = pcmRead(frmWindowBuf, FRAME_WINDOW, pcm);
  return (!frmWindowSize) ? -1 : 0;
}

/*
 * フレームウィンドウの内容を、10[ms]分スライドさせる
 *
 * Prototype:   int frmSlideWindow(PcmFILE *pcm)
 *
 * Args:       pcm      順次読み出す PCM ファイルの管理構造体ポインタ
 * Return:     int      スライド成功時 0 / 失敗時非 0 を返す。
 */
int frmSlideWindow(PcmFILE *pcm)
{
  int i;

  if(frmWindowSize < FRAME_WINDOW) return -1;

  frmWindowSize -= FRAME_STEP;
  for(i = 0; i < frmWindowSize; i++)
    frmWindowBuf[i] = frmWindowBuf[i + FRAME_STEP];
  frmWindowSize += pcmRead(frmWindowBuf + frmWindowSize, FRAME_STEP, pcm);
  return (frmWindowSize == FRAME_WINDOW) ? 0 : -1;
}
