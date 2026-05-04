#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _voice_class_c_
#include "analyze.h"
#include "segmentation.h"
#include "voice_class.h"


/*
 * 指定された範囲を音素セグメントとし,音素群ラベリングを行う。
 *
 * 返り値は以下の値のいずれかをとる。
 *
 * 有声音クラス
 *
 *   V_W       母音的セグメント
 *             /a/,/i/,/u/,/e/,/o/ の他、/N/,/wa/,/ya/,/yu/,/yo/ など
 *
 *   V_D       有声子音セグメント
 *             /d/,/b/,/g/
 *
 *   V_C       有声子音的セグメント
 *             /y/,/w/,/g/,/m/,/n/,/z/ のほか、V_D に分類されるものも含む。
 *
 *   V_V       有声音的セグメント
 *             V_W, V_C に分類されるものを含む。
 *
 * 無声音クラス
 *
 *   V_F       無声摩擦音的セグメント
 *             /k/, /h/, /s/, /c/, /x/             
 *
 *   V_U       無声子音的セグメント
 *             /p/, /t/ の他, V_F に分類されるものも含む
 *
 *
 * 無音クラス
 *   V_S       無音セグメント
 *             /g/, sil(無音)
 *
 *
 * 最悪の場合でも、V_V, V_U, V_S のいずれかに分類されなければならない。
 */
vGroup vcClassLabel(anaParam **p_list, int begin, int end)
{

  
  
  
  
  
  
  
}
