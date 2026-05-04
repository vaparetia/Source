#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define _main_c_
#include "main.h"
#include "vec.h"
#include "group.h"
#include "dic.h"

/*
 * 抽出ベクトルファイルと時系列ラベルファイルを照合し,
 * 音韻辞書ファイルにそのベクトルをラベル付きで登録する
 */
int main(int argc, char **argv)
{
  grGroup   ** g_list;
  vecVector ** v_list;
  dicREF    ** r_list;
  char * lbl_file;
  char * vec_file;
  char * dic_file;
  TOKEN *l_tkn;
  TOKEN *v_tkn;
  TOKEN *d_tkn;
  dicDicROOT *dic;
  FILE *wfp;

  if(argc != 4)
    {
      fprintf(stderr, "usage: %s labelfile vecfile dicfile\n", argv[0]);
      return EXIT_FAILURE;
    }

  lbl_file = argv[1];
  vec_file = argv[2];
  dic_file = argv[3];

  if(NULL == (l_tkn = tknOpen(lbl_file)))
    {
      fprintf(stderr, "could not open '%s'.\n", lbl_file);
      return EXIT_FAILURE;
    }

  if(NULL == (g_list = grReadGroup(l_tkn)))
    {
      tknClose(l_tkn);
      fprintf(stderr, "could not read '%s'.\n", lbl_file);
      return EXIT_FAILURE;
    }

  tknClose(l_tkn);

  if(NULL == (v_tkn = tknOpen(vec_file)))
    {
      fprintf(stderr, "could not open '%s'.\n", vec_file);
      grReleaseGroup(g_list);
      return EXIT_FAILURE;
    }

  if(NULL == (v_list = vecReadVector(v_tkn)))
    {
      tknClose(v_tkn);
      fprintf(stderr, "could not read '%s'.\n", vec_file);
       grReleaseGroup(g_list);
      return EXIT_FAILURE;
    }
  tknClose(v_tkn);


  /*
   * 辞書を読み込む。
   * 辞書ファイルが無い場合は新規作成する
   */
  if(NULL == (d_tkn = tknOpen(dic_file)))
    dic = dicNewDictionaly();         /* 新規作成 */
  else
    {
      dic = dicReadDictionaly(d_tkn); /* 読み込み */
      tknClose(d_tkn);
    }
  if(NULL == dic)
    {
      fprintf(stderr, "could not create dictionaly.\n");
      return EXIT_FAILURE;
    }

  /*
   * 時系列の照合をとる
   */
  r_list = dicMatchVectorAndGroup(v_list, g_list);
  dicAppendDictionaly(dic, r_list);
  free(r_list);

  /*
   * 追加した結果を辞書ファイルに書き込む
   */
  if(NULL == (wfp = fopen(dic_file, "w")))
    {
      fprintf(stderr, "could not write dictionaly file.\n");
      return EXIT_FAILURE;
    }

  dicWriteDictionaly(wfp, dic);

  fclose(wfp);

  return EXIT_SUCCESS;
  
}
