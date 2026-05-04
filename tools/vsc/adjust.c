/*
 * 発音による各種音響的変動の補正を行なう。
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _adjust_h_

#include "analyze.h"
#include "segmentation.h"
#include "label.h"
#include "adjust.h"

/*
 * 補正モジュール初期化
 */
int adjInit(void)
{
  
}

/*
 * 音素スケジュールの補正
 */
adjList * adjAdjust(labVList * v_list)
{
  int t;
  adjList * a_list;
  int nums;

  nums++;
  for(t = 0; v_list[t].t >= 0; t++) nums+;

  if(NULL == (a_list = malloc(sizeof(adjList) * (nums + 1)))) return NULL;

  for(t = 0; t < nums; t++)
    {
      
      
      
      
      
    }

  
  
  
  
  
  
  
}
