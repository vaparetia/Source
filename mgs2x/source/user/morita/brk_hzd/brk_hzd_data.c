//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  brk_hzd_data.c
  共有箱型ハザード
  
  2000/03/30 T. Morita
  2000/10/16 1.9 T.Morita 
  $Id: brk_hzd_data.c,v 1.1.1.3 2002/11/19 11:45:31 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"

#include "brk_hazard.h"


#define UNIT_ORG(_x,_y,_z) \
{ { {1.0f,0.0f,0.0f,0.0f},\
    {0.0f,1.0f,0.0f,0.0f},\
    {0.0f,0.0f,1.0f,0.0f},\
    {(_x),(_y),(_z),1.0f} } }
#define ROTZ_P21_ORG(_x,_y,_z) \
{ { { 0.933580f, 0.358368f, 0.0f, 0.0f}, \
    {-0.358368f, 0.933580f, 0.0f, 0.0f}, \
    { 0.0f     , 0.0f     , 1.0f, 0.0f}, \
    { (_x)     , (_y)     , (_z), 1.0f} } }
#define ROTZ_N21_ORG(_x,_y,_z) \
{ { { 0.933580f,-0.358368f, 0.0f, 0.0f}, \
    { 0.358368f, 0.933580f, 0.0f, 0.0f}, \
    { 0.0f     , 0.0f     , 1.0f, 0.0f}, \
    { (_x)     , (_y)     , (_z), 1.0f} } }
#define ROTZ_P45_ORG(_x,_y,_z) \
{ { { 0.707107f, 0.707107f, 0.0f, 0.0f}, \
    {-0.707107f, 0.707107f, 0.0f, 0.0f}, \
    { 0.0f     , 0.0f     , 1.0f, 0.0f}, \
    { (_x)     , (_y)     , (_z), 1.0f} } }
#define ROTX_N10_ORG(_x,_y,_z) \
{ { { 1.0f, 0.0f     , 0.0f     , 0.0f}, \
    { 0.0f, 0.984823f,-0.173561f, 0.0f}, \
    { 0.0f, 0.173561f, 0.984823f, 0.0f}, \
    { (_x), (_y)     , (_z)     , 1.0f} } }
#define ROTX_N07_ORG(_x,_y,_z) \
{ { { 1.0f, 0.0f     , 0.0f     , 0.0f}, \
    { 0.0f, 0.994527f,-0.104475f, 0.0f}, \
    { 0.0f, 0.104475f, 0.994527f, 0.0f}, \
    { (_x), (_y)     , (_z)     , 1.0f} } }
#define ROTZ_N16_ORG(_x,_y,_z) \
{ { { 0.990268f,-0.139173f, 0.0f, 0.0f}, \
    { 0.139173f, 0.990268f, 0.0f, 0.0f}, \
    { 0.0f     , 0.0f     , 1.0f, 0.0f}, \
    { (_x)     , (_y)     , (_z), 1.0f} } }
#define ROTZ_P16_ORG(_x,_y,_z) \
{ { { 0.990268f, 0.139173f, 0.0f, 0.0f}, \
    {-0.139173f, 0.990268f, 0.0f, 0.0f}, \
    { 0.0f     , 0.0f     , 1.0f, 0.0f}, \
    { (_x)     , (_y)     , (_z), 1.0f} } }
#define ROTZ_N17_ORG(_x,_y,_z) \
{ { { 0.983147f,-0.182818f, 0.0f, 0.0f}, \
    { 0.182818f, 0.983147f, 0.0f, 0.0f}, \
    { 0.0f     , 0.0f     , 1.0f, 0.0f}, \
    { (_x)     , (_y)     , (_z), 1.0f} } }
#define ROTZ_P17_ORG(_x,_y,_z) \
{ { { 0.983147f, 0.182818f, 0.0f, 0.0f}, \
    {-0.182818f, 0.983147f, 0.0f, 0.0f}, \
    { 0.0f     , 0.0f     , 1.0f, 0.0f}, \
    { (_x)     , (_y)     , (_z), 1.0f} } }
#define ROTY_P24_ORG(_x,_y,_z) \
{ { { 0.9103665f, 0.0f, 0.4138029f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    {-0.4138029f, 0.0f, 0.9103665f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }
#define ROTY_N24_ORG(_x,_y,_z) \
{ { {-0.9103665f, 0.0f, 0.4138029f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    {-0.4138029f, 0.0f,-0.9103665f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }
#define ROTY_N27_ORG(_x,_y,_z) \
{ { {-0.8949344f, 0.0f, 0.4461978f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    {-0.4461978f, 0.0f,-0.8949344f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }
#define ROTY_P33_ORG(_x,_y,_z) \
{ { { 0.8320503f, 0.0f, 0.5547002f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    {-0.5547002f, 0.0f, 0.8320503f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }
#define ROTY_N40_ORG(_x,_y,_z) \
{ { { 0.7660445f, 0.0f,-0.6427876f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    { 0.6427876f, 0.0f, 0.7660445f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }
#define ROTY_P40_ORG(_x,_y,_z) \
{ { { 0.7660445f, 0.0f, 0.6427876f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    {-0.6427876f, 0.0f, 0.7660445f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }
#define ROTY_N44_ORG(_x,_y,_z) \
{ { { 0.7193398f, 0.0f,-0.6946584f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    { 0.6946584f, 0.0f, 0.7193398f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }
#define ROTY_P44_ORG(_x,_y,_z) \
{ { { 0.7193398f, 0.0f, 0.6946584f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    {-0.6946584f, 0.0f, 0.7193398f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }
#define ROTX_P21_ORG(_x,_y,_z) \
{ { { 1.0f, 0.0f     , 0.0f     , 0.0f}, \
    { 0.0f, 0.933580f, 0.358368f, 0.0f}, \
    { 0.0f,-0.358368f, 0.933580f, 0.0f}, \
    { (_x), (_y)     , (_z)     , 1.0f} } }
#define ROTX_N21_ORG(_x,_y,_z) \
{ { { 1.0f, 0.0f     , 0.0f     , 0.0f}, \
    { 0.0f, 0.933580f,-0.358368f, 0.0f}, \
    { 0.0f, 0.358368f, 0.933580f, 0.0f}, \
    { (_x), (_y)     , (_z)     , 1.0f} } }

#define ROTY_P81_ORG(_x,_y,_z) \
{ { { 0.9881475f, 0.0f, 0.1535073f, 0.0f}, \
    { 0.0f      , 1.0f, 0.0f      , 0.0f}, \
    {-0.1535073f, 0.0f, 0.9881475f, 0.0f}, \
    { (_x)      , (_y), (_z)      , 1.0f} } }

#define UNIT(_x,_y,_z)      UNIT_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTZ_P21(_x,_y,_z)  ROTZ_P21_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTZ_N21(_x,_y,_z)  ROTZ_N21_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTZ_P45(_x,_y,_z)  ROTZ_P45_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTX_N10(_x,_y,_z)  ROTX_N10_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTX_N07(_x,_y,_z)  ROTX_N07_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTZ_N16(_x,_y,_z)  ROTZ_N16_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTZ_P16(_x,_y,_z)  ROTZ_P16_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTZ_N17(_x,_y,_z)  ROTZ_N17_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTZ_P17(_x,_y,_z)  ROTZ_P17_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_P33(_x,_y,_z)  ROTY_P33_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_P24(_x,_y,_z)  ROTY_P24_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_N24(_x,_y,_z)  ROTY_N24_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_N27(_x,_y,_z)  ROTY_N27_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_P40(_x,_y,_z)  ROTY_P40_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_N40(_x,_y,_z)  ROTY_N40_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_P44(_x,_y,_z)  ROTY_P44_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_N44(_x,_y,_z)  ROTY_N44_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTX_P21(_x,_y,_z)  ROTX_P21_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTX_N21(_x,_y,_z)  ROTX_N21_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)
#define ROTY_P81(_x,_y,_z)  ROTY_P81_ORG(_x,_y,_z),UNIT_ORG(0,0,0),UNIT_ORG(0,0,0)


/**********************************************

  タンカー編  ステージハザード データ

  **********************************************/
static HZD_BOX w01f_stair[]=
{
    { BRK_HZD_INSIDE ,NULL,UNIT(    0, 1500, -12875 ),{11800,1500, 6875 } },
    { BRK_HZD_OUTSIDE,NULL,UNIT(    0,    0, -14250 ),{ 2000, 750,  250 } },
    { BRK_HZD_OUTSIDE,NULL,UNIT(-2500,    0, -14375 ),{  500, 750,  375 } },
    { BRK_HZD_OUTSIDE,NULL,UNIT( 2500,    0, -14375 ),{  500, 750,  375 } },
    { BRK_HZD_OUTSIDE,NULL,UNIT(    0,    0, -14250 ),{ 2900, 850,   50 } },
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;				

static HZD_BOX w01f_bar[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT( 8000, 1400, -18975 ),{ 1500,1400,  875 } },
    { BRK_HZD_OUTSIDE,NULL,UNIT( 6375,    0, -19500 ),{  375,2800,  100 } },/*出っぱり支柱 */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 6800,    0, -19830 ),{  200,2800,  250 } },/*棚 右側  */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 9450,    0, -19830 ),{  200,2800,  250 } },/*棚 左側  */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8125,    0, -19830 ),{ 1375,1000,  250 } },/*棚 最下段*/
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8125, 1340, -19850 ),{ 1400,  40,  270 } },/*棚 中段*/
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8125, 1720, -19850 ),{ 1400,  40,  270 } },/*棚 中上段*/
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8125, 2020, -19850 ),{ 1400,  40,  270 } },/*棚 最上段*/
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8135, 2225, -19850 ),{   25, 165,  270 } },/*棚 最上段 仕切り*/
    { BRK_HZD_OUTSIDE,NULL,UNIT( 7765, 1885, -19850 ),{   25, 165,  270 } },/*棚 最上段 右仕切り*/
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8555, 1885, -19850 ),{   25, 165,  270 } },/*棚 最上段 左仕切り*/

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w01f_bar2[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT( 8000, 1400, -18540 ),{ 1500,1400,1040 } },
    { BRK_HZD_OUTSIDE,NULL,UNIT( 6375,    0, -19500 ),{  375,2800, 100 } },/* 出っぱり支柱   */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8200,  900, -17650 ),{  150, 480, 150 } },/* サーバー       */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 7875,  700, -17675 ),{ 1625, 200, 425 } },/* バーカウンター */

//    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTX_N10( 7875,  700, -17675 ),{ 1625, 200, 425 } },/*テスト*/

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w01f_bar3[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT( 4750, 1400, -17040 ),{ 4750,1400,2540 } },
    { BRK_HZD_OUTSIDE,NULL,UNIT( 6375,    0, -19500 ),{  375,2800, 100 } },/* 出っぱり支柱   */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8200,  900, -17650 ),{  150, 480, 150 } },/* サーバー       */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 7875,  700, -17675 ),{ 1625, 200, 425 } },/* バーカウンター */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 6250,    0, -17700 ),{  250,1000, 700 } },/* カウンター短い */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 7700,    0, -17250 ),{ 1700,1000, 250 } },/* カウンター長い */

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w01f_tvspace1[] =
{
    { BRK_HZD_INSIDE ,		     NULL,UNIT(-8175,1400,-17050),{1425,1400,2550 } },
    { BRK_HZD_OUTSIDE,		     NULL,UNIT(-9250,   0,-19600),{ 500, 650, 850 } },/* 端の台 */
    { BRK_HZD_OUTSIDE,		     NULL,UNIT(-9600,   0,-17050),{ 350, 650,2550 } },/* 中央台 */
  //{ BRK_HZD_OUTSIDE,		     NULL,UNIT(-9250,   0,-14500),{ 500, 650, 850 } },/* 端の台 */
    { BRK_HZD_OUTSIDE,		     NULL,UNIT(-9600,   0,-18500),{ 200,2800, 250 } },/* 突き出し柱 */
  //{ BRK_HZD_OUTSIDE,		     NULL,UNIT(-9600,   0,-15600),{ 200,2800, 250 } },/* 突き出し柱 */
    { BRK_HZD_OUTSIDE,		     NULL,UNIT(-9600,   0,-19600),{ 200,2800, 200 } },/* 角の柱 */
  //{ BRK_HZD_OUTSIDE,		     NULL,UNIT(-9600,   0,-14500),{ 200,2800, 200 } },/* 角の柱 */

    { BRK_HZD_OUTSIDE,		     NULL,UNIT(-9050,   0,-19100),{ 200, 700, 200 } },/* 壷しき */
    { BRK_HZD_OUTSIDE,		     NULL,UNIT(-7750, 675,-17000),{ 500,  75, 850 } },/* テーブル卓 */
    { BRK_HZD_OUTSIDE,		     NULL,UNIT(-7750,   0,-17000),{ 100, 600, 150 } },/* テーブル足 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTX_N10(-7750, 380,-19550),{ 950, 350, 200 } },/* 赤ソファー背もたれ */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTX_N07(-7750, -70,-19480),{ 950, 400, 700 } },/* 赤ソファー座 */

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;				

static HZD_BOX w01f_tvspace2[] =
{
    { BRK_HZD_INSIDE 		    ,NULL,UNIT( -4500, 1400, -17000 ),{1500,1400,2600 } },
#if 1
    { BRK_HZD_OUTSIDE		    ,NULL,UNIT( -5250,    0, -17000 ),{ 250,1000,1500 } },/*キャビネット全部*/
#else
    { BRK_HZD_OUTSIDE		    ,NULL,UNIT( -5250,    0, -15825 ),{ 250,1000, 325 } },/*キャビネット端*/
    { BRK_HZD_OUTSIDE		    ,NULL,UNIT( -5250,    0, -18175 ),{ 250,1000, 325 } },/*キャビネット端*/
    { BRK_HZD_OUTSIDE		    ,NULL,UNIT( -5250,    0, -17000 ),{ 250, 150,1500 } },/*キャビネット棚 最下段*/
    { BRK_HZD_OUTSIDE		    ,NULL,UNIT( -5250,  550, -17000 ),{ 250,  25,1500 } },/*キャビネット棚 中段*/
    { BRK_HZD_OUTSIDE		    ,NULL,UNIT( -5250,  975, -17000 ),{ 250,  25,1500 } },/*キャビネット棚 最上段*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P21( -5270,  725, -17000 ),{ 100, 200,1500 } },/*キャビネット棚 上段奥壁斜め*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P21( -5270,  300, -17000 ),{ 100, 200,1500 } },/*キャビネット棚 下段奥壁斜め*/
#endif

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;				

static HZD_BOX w01d_store[] =		
{
    { BRK_HZD_INSIDE ,NULL,UNIT( 9525-50,10400,-14500),{2275, 1400, 5300} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 7750-50, 9000,-16700),{ 500, 1000, 1500} }, /*ドア側壁の中央４つ*/
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8250-50, 9000,-16700),{1000, 1000,  500} }, /*ドア側壁の中央４つ*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(11800-50, 9000,-19450),{1000, 1600,  500} }, /*ドアの向かい２つ */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 7750-50, 9000,-10200),{ 500, 1000, 1000} }, /*奥の壁３つ       */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 8250-50, 9000,- 9700),{1000, 1000,  500} }, /*奥の壁３つ       */
#if 1
    { BRK_HZD_OUTSIDE,NULL,UNIT(10000-50,10620,-12625),{ 750,   30, 1375} }, /*棚の上段 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(10000-50,10120,-12625),{ 750,   30, 1375} }, /*棚の中段 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(10000-50, 9620,-12625),{ 750,   30, 1375} }, /*棚の下段 */
#else
    /*はみ出し防止のため一時的に１０cm小さくした*/ 
    { BRK_HZD_OUTSIDE,NULL,UNIT(10000-50,10620,-12625),{ 650,   30, 1275} }, /*棚の上段 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(10000-50,10120,-12625),{ 650,   30, 1275} }, /*棚の中段 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(10000-50, 9620,-12625),{ 650,   30, 1275} }, /*棚の下段 */
#endif
    { BRK_HZD_OUTSIDE,NULL,UNIT(10700-50, 9050,-13950),{  50, 1600,   50} }, /*棚の柱 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(10700-50, 9050,-11300),{  50, 1600,   50} }, /*棚の柱 */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 9300-50, 9050,-13950),{  50, 1600,   50} }, /*棚の柱 */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 9300-50, 9050,-11300),{  50, 1600,   50} }, /*棚の柱 */

    { BRK_HZD_OUTSIDE,NULL,UNIT( 7250, 10000,-17674),{ 478, 667,  516} }, /*中央４つ上の箱 ドア側上 */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 7250, 10000,-17640),{ 663, 333,  529} }, /*中央４つ上の箱 ドア側下 */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 7250, 10000,-15611),{ 476, 303,  478} }, /*中央４つ上の箱 奥側 */

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w01d_kitchen[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(- 8150,10450,-18116),{3650, 1450, 1684} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 5875, 9000,-18125),{ 375, 1000, 1675} }, /*流し*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 5450,10140,-19800),{ 100,   60, 2944} }, /*流し上段*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-11800, 9000,-16850),{4050, 1000,  400} }, /*手前*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-11800,10140,-16650),{4050,   60,  200} }, /*手前 上段*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 8900,11550,-19800),{4000,  250,  625} }, /*皿の棚の上*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 8000,10800,-19800),{2000,   50,  625} }, /*皿の棚の下*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 5887,11325,-19800),{ 162,  475,  625} }, /*皿の棚の縦左*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 8025,11075,-19800),{  30,  225,  625} }, /*皿の棚の縦中*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 8811,10400,-19800),{1239,   25,  300} }, /*下段棚の下*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 7326,10410,-19800),{ 295,  390,  377} }, /*給湯ジャー*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-10862,11200,-19800),{ 862,  500,  900} }, /*換気扇*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-11800, 9000,-19800),{6300, 1000,  800} }, /*コンロ 付近*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-11800, 9000,-17250),{1050,  600, 1100} }, /*大鍋 台座*/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w01d_dining1[] =
{
    { BRK_HZD_INSIDE                ,NULL,    UNIT(- 9200,10050,-12766),{2400, 1050, 3666} }, /*floor */
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(- 9250, 9770,-12750),{ 750,   30, 2250} }, /*table*/
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(-10200, 9000,-11750),{ 250,  450,  850} }, /*Bench 左奥*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P16(-10494, 9763,-11750),{  20,  135,  850} }, /*Bench 左奥背もたれ*/
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(-10200, 9000,-13750),{ 250,  450,  850} }, /*Bench 左前*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P16(-10494, 9763,-13750),{  20,  135,  850} }, /*Bench 左前背もたれ*/
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(- 8300, 9000,-11750),{ 250,  450,  850} }, /*Bench 右奥*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N16(- 8005, 9763,-11750),{  20,  135,  850} }, /*Bench 右奥背もたれ*/
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(- 8300, 9000,-13750),{ 250,  450,  850} }, /*Bench 右前*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N16(- 8005, 9763,-13750),{  20,  135,  850} }, /*Bench 右前背もたれ*/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

#if 1
static HZD_BOX w01d_dining2[] =
{
    { BRK_HZD_INSIDE                ,NULL,    UNIT(- 9200+4500,10050,-12766),{2400, 1050, 3666} }, /*floor */
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(- 9250+4500, 9770,-12750),{ 750,   30, 2250} }, /*table*/
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(-10200+4500, 9000,-11750),{ 250,  450,  850} }, /*Bench 左奥*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P16(-10494+4500, 9763,-11750),{  20,  135,  850} }, /*Bench 左奥背もたれ*/
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(-10200+4500, 9000,-13750),{ 250,  450,  850} }, /*Bench 左前*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P16(-10494+4500, 9763,-13750),{  20,  135,  850} }, /*Bench 左前背もたれ*/
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(- 8300+4500, 9000,-11750),{ 250,  450,  850} }, /*Bench 右奥*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N16(- 8005+4500, 9763,-11750),{  20,  135,  850} }, /*Bench 右奥背もたれ*/
    { BRK_HZD_OUTSIDE               ,NULL,    UNIT(- 8300+4500, 9000,-13750),{ 250,  450,  850} }, /*Bench 右前*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N16(- 8005+4500, 9763,-13750),{  20,  135,  850} }, /*Bench 右前背もたれ*/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;
#else
static HZD_BOX w01d_dining2[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(- 9200+4500,10050,-12766),{2400, 1050, 3666} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 9250+4500, 9740,-12750),{ 750,   60, 2250} }, /*table*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-10200+4500, 9000,-11750),{ 250,  450,  850} }, /*Bench 左奥*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-10494+4500, 9763,-11750),{  60,  135,  850} }, /*Bench 左奥背もたれ*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-10200+4500, 9000,-13750),{ 250,  450,  850} }, /*Bench 左前*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-10494+4500, 9763,-13750),{  60,  135,  850} }, /*Bench 左前背もたれ*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 8300+4500, 9000,-11750),{ 250,  450,  850} }, /*Bench 右奥*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 8005+4500, 9763,-11750),{  60,  135,  850} }, /*Bench 右奥背もたれ*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 8300+4500, 9000,-13750),{ 250,  450,  850} }, /*Bench 右前*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(- 8005+4500, 9763,-13750),{  60,  135,  850} }, /*Bench 右前背もたれ*/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;
#endif

/**********************************************

  プラント編  ステージハザード データ

  **********************************************/
static HZD_BOX w11b_fortunestage[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(    0, -42000,-1400),{6500, 3000, 7600} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-6500, -45000,-9000),{2500, 5000, 5000} }, /*フォーチュン側左 */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 6500, -45000,-9000),{2500, 5000, 5000} }, /*フォーチュン側右 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-6500, -45000,-2000),{ 500, 5000,  500} }, /*縦ハリ左 前 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-6500, -45000, 2000),{ 500, 5000,  500} }, /*縦ハリ左 後 */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 6500, -45000,-2000),{ 500, 5000,  500} }, /*縦ハリ右 前 */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 6500, -45000, 2000),{ 500, 5000,  500} }, /*縦ハリ右 後 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_P44(-7000,-45000,7000),{1500,5000,2000} }, /*手前斜め*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N44( 7000,-45000,7000),{1500,5000,2000} }, /*手前斜め*/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w16a_kitchen[] =
{
#if 1
    { BRK_HZD_INSIDE ,NULL,UNIT(-48500,1950,-100000),{3000, 1950, 3000} }, /*floor */
#else
    { BRK_HZD_INSIDE ,NULL,UNIT(-48000,1950,-100000),{4000, 1950, 3000} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-45500,   0, -97000),{ 500, 4000, 1000} }, /*ドア側 縦柱 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-48375,   0,-100000),{1125, 1000,  750} }, /*調理机 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-45500,   0,-100600),{ 500, 1000, 3000} }, /*コンロ */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-45500,   0,-101125),{ 500, 1350,  475} }, /*エスプレッソメーカ */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-45500,   0,-102050),{ 500, 1200,  350} }, /*引出し*/
#endif
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51500,   0,-101875),{ 500, 1000, 1125} }, /*台   */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-52500,   0, -97000),{3000, 1000, 1000} }, /*キャビネット（低） */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w16a_sink[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-56080, 1950,-100950),{4000, 1950, 3000} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51500,    0,-101875),{ 500, 1000, 1125} }, /*台   */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-55200, 0, -99300),{1160, 4000, 1350} }, /* 流し埋めの壁 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-56318, 0,-101500),{ 480, 4000,   60} }, /* 食器棚敷居   */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-56040+600, 0,-100920-300),{485,750, 850} }, /* 流し底     */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-55000- 25, 0,-101500+ 13),{25,1000, 650} }, /* 流し       */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-55000-400, 0,-101500+200),{400,1000, 40} }, /* 流し       */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-56040+80 , 0,-100920- 40),{560,1000, 850} }, /* 食器一番下 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-56040, 1500,-100920),{485,  25, 650} }, /* 食器棚上     */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-56040, 1250,-100920),{485,  25, 650} }, /* 食器棚下     */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-56040, 3000,-100920),{485,1000, 650} }, /* 食器棚天井   */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w16a_dashboard[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-52375, 1950,-101500),{2875, 1950, 3500} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51500,    0,-101875),{ 500, 1000, 1125} }, /*台   */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51500, 1350,-102000),{ 250,   25, 1000} }, /*棚下 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51500, 1750,-102000),{ 250,   25, 1000} }, /*棚上 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51500, 1000,-101300),{  75,  750,   50} }, /*棚柱 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51500, 1000,-102700),{  75,  750,   50} }, /*棚柱 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N27(-55200, 0,-99300),{1160, 4000, 4000} }, /*流し埋めの壁 */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;



static HZD_BOX w16a_cabinet[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-49875,1950, -98887),{2625,1950,1887} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-52500,   0, -97000),{3000,1000,1000} }, /*台   */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-48375,   0,-100000),{1125,1000, 750} }, /*台   */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w16a_locker[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-53125, 1950,-103875),{2125, 1950, 1875} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51000,    0,-105750),{1500, 2000,  750} }, /*台   */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-51000,    0,-100750),{1000, 4000, 2250} }, /*台   */

    { BRK_HZD_OUTSIDE,NULL,ROTY_P81(-53050,0,-105550),{ 580, 110,  500} }, /*スイカの箱   */
    { BRK_HZD_OUTSIDE,NULL,ROTY_P81(-53000,0,-105850),{ 580, 2000, 250} }, /*スイカの箱   */
    { BRK_HZD_OUTSIDE,NULL,ROTY_P81(-52200,0,-105350),{ 330, 2000, 330} }, /*スイカの箱   */
    { BRK_HZD_OUTSIDE,NULL,ROTY_P81(-53585,0,-105385),{  20, 250, 280} }, /*スイカの箱 薄皮  */
    { BRK_HZD_OUTSIDE,NULL,ROTY_P81(-53100,0,-105060),{ 580, 250,  20} }, /*スイカの箱 薄皮  */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w16a_largeglass[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-59750, 2000,-102875),{3000, 2000, 2125} }, /*floor */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-60000,  625,-102825),{1500,  625,   75} }, /*台   */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-61050,  425,-104800),{ 450,  425,  200} }, /*台   */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w16a_toilet_men[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-59000,1575,-85500),{4250,1575,2000} },
    { BRK_HZD_OUTSIDE,NULL,UNIT(-63250,   0,-87450),{4500,2750,2000} }, /* トイレ側壁 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-54750,   0,-87450),{1500,3000,2000} }, /* 女子側の壁 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-57500,   0,-87450),{1250, 750, 700} }, /* 洗面台     */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w16a_toilet_women[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(59000-49000-57500,1575,-85500),{4250,1575,2000} },
    { BRK_HZD_OUTSIDE,NULL,UNIT(63250-49000-57500,   0,-87450),{4500,3000,2000} }, /* トイレ側壁 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(54750-49000-57500,   0,-87450),{1500,2750,2000} }, /* 女子側の壁 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(57500-49000-57500,   0,-87450),{1250, 750, 700} }, /* 洗面台     */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;




static HZD_BOX w16a_dining_table1[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-62625,2000,-94350),{2625,2000,3850} },
    { BRK_HZD_OUTSIDE,NULL,UNIT(-62250,725,-94250),{ 750,75,2250} }, /* 台   */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-63225,350,-93175),{ 300,75,875} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-63225,350,-95325),{ 300,75,875} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-61250,350,-93175),{ 300,75,875} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-61250,350,-95325),{ 300,75,875} }, /* 椅子 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P21(-63575,650,-93175),{ 25,275,875 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P21(-63575,650,-95325),{ 25,275,875 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N21(-60925,650,-93175),{ 25,275,875 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N21(-60925,650,-95325),{ 25,275,875 } },/**/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w16a_dining_table2[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-62625+5000,2000,-94350),{3125,2000,3850} },
    { BRK_HZD_OUTSIDE,NULL,UNIT(-62250+4000,725,-94250),{750,75,2250} }, /* 台   */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-63225+4000,350,-93175),{300,75,875} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-63225+4000,350,-95325),{300,75,875} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-61250+4000,350,-93175),{300,75,875} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-61250+4000,350,-95325),{300,75,875} }, /* 椅子 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P21(-63575+4000,650,-93175),{ 25,275,875 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P21(-63575+4000,650,-95325),{ 25,275,875 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N21(-60925+4000,650,-93175),{ 25,275,875 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N21(-60925+4000,650,-95325),{ 25,275,875 } },/**/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

static HZD_BOX w16a_dining_table3[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-60200, 725,-99550),{3125,2000,3850} },
    { BRK_HZD_OUTSIDE,NULL,UNIT(-60250, 725,-99500),{2250,75,750} }, /* 台   */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-59150, 350,-100475),{875,75,300} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-59150, 350,-98525),{875,75,300} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-61250, 350,-100475),{875,75,300} }, /* 椅子 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-61250, 350,-98525),{875,75,300} }, /* 椅子 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTX_N21(-59150,650,-100820),{ 875,275,25 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTX_N21(-61350,650,-100820),{ 875,275,25 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTX_P21(-61350,650,-98175),{ 875,275,25 } },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTX_P21(-59150,650,-98175),{ 875,275,25 } },/**/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w24b_refresh[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT( -8050,-2500,-72125),{3800,2000,2625} },
				     
    { BRK_HZD_OUTSIDE,NULL,UNIT(-11850,-2500,-69500),{ 850,4000,1000} }, /* 縦柱  */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-11850,-2500,-74750),{ 850,4000,1250} }, /* 縦柱  */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-11850,-4500,-72000),{ 850, 750,2000} }, /* 花壇  */

    { BRK_HZD_OUTSIDE,NULL,UNIT(-8500,-4500,-74750),{ 500, 850, 750} }, /* ソファー はり  */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-4250,-4500,-74750),{4750, 500, 750} }, /* ソファー       */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-4250,-4500,-74750),{ 750, 500,3250} }, /* ソファー       */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-8000,-4000,-74750),{4000, 350, 160} }, /* ソファー 背もたれ */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-4250,-4500,-72025),{ 750, 850, 525} }, /* ソファー はり  */

    { BRK_HZD_OUTSIDE,NULL,UNIT(-9425,-3350,-72000),{ 425, 100, 425} }, /* 台卓 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-9425,-4500,-72000),{  75,1150,  75} }, /* 台棒 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-7175,-3350,-72000),{ 425, 100, 425} }, /* 台卓 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-7175,-4500,-72000),{  75,1150,  75} }, /* 台棒 */

    { BRK_HZD_OUTSIDE,NULL,UNIT(-11000,-4500,-74850),{2000,1500, 900} }, /* 棚 */

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;



static HZD_BOX w24b_bottlekeep[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-5475,-2500,-69000),{1225,2000,2000} },
				     
    { BRK_HZD_OUTSIDE,NULL,UNIT(-6700,-4500,-69000),{ 700,1300,1000} }, /* ゴミ箱 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-6700,-4500,-70000),{ 700,1700,  50} }, /* ゴミ箱 縦板 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-6700,-4500,-68000),{ 700,1700,  50} }, /* ゴミ箱 縦板 */

    { BRK_HZD_OUTSIDE,NULL,UNIT(-6700,-4500,-68000),{ 700,1700,  50} }, /* ゴミ箱 縦板 */
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w24c_party[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT( 2000,-5500,-61250),{2000,3000,1750} },

    { BRK_HZD_OUTSIDE,NULL,UNIT( 2000,-8500,-61400),{1500,1000, 400} }, /* テーブル */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 3200,-8500,-63000),{ 300, 800,2000} }, /* テーブル */
    { BRK_HZD_OUTSIDE,NULL,UNIT(  900,-8500,-63000),{ 400, 800,2000} }, /* テーブル */

    { BRK_HZD_OUTSIDE,NULL,UNIT( 3200,-7350,-62400),{ 300, 50, 600} }, /* 棚        */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 3200,-6950,-62400),{ 300, 50, 600} }, /* 棚        */
    { BRK_HZD_OUTSIDE,NULL,UNIT( 3200,-6550,-62400),{ 300, 50, 600} }, /* 棚        */

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w24c_speech[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(-8000,-5500,-54500),{3500,3000,4000} },

    { BRK_HZD_OUTSIDE,NULL,UNIT(-9825,-7500,-53025),{ 175,1050, 175} }, /* 演説台端 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-9825,-7500,-55975),{ 175,1050, 175} }, /* 演説台端 */
    { BRK_HZD_OUTSIDE,NULL,UNIT(-9250,-7500,-54500),{ 250,1050,1000} }, /* 演説台中央 */
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_N44(-9500,-7500,-55666),{ 250,1050,475} },/**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTY_P44(-9500,-7500,-53333),{ 250,1050,475} },/**/


    { BRK_HZD_OUTSIDE,NULL,UNIT(-10000,-8500,-54500),{2000,1000,4000} }, /* 舞台 */

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;




static HZD_BOX w24d_comp_room1[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(0,-14000,-53000),{3000,2000,4000} },

    { BRK_HZD_OUTSIDE,NULL,UNIT(0,-15175,-52525),{ 1000,75, 1975} },
    { BRK_HZD_OUTSIDE,NULL,UNIT(0,-14625,-52525),{ 150,625, 1975} },

    { BRK_HZD_OUTSIDE,NULL,UNIT(0,-16000,-50610),{ 1000,2000,60} },/*サイド*/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_P17( 180,-14625,-52525),{ 80, 625, 1975} }, /**/
    { BRK_HZD_OUTSIDE|BRK_HZD_ROTATE,NULL,ROTZ_N17(-180,-14625,-52525),{ 80, 625, 1975} }, /**/

    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;


static HZD_BOX w24d_comp_room2[] =
{
    { BRK_HZD_INSIDE ,NULL,UNIT(    0,-14000,-53875),{8150,2000,6625} },
    { BRK_HZD_OUTSIDE,NULL,UNIT(-5625,-16000,-54000),{ 375,4000, 500} },/*左柱*/
    { BRK_HZD_OUTSIDE,NULL,UNIT(-4550,-16000,-54000),{1550,1300, 625} },

    { BRK_HZD_OUTSIDE,NULL,UNIT( 5375,-16000,-56250),{ 375,3000,1750} },/*左側ロッカー*/
    { BRK_HZD_OUTSIDE,NULL,UNIT( 4550,-16000,-53812),{1150,1300, 313} },/*コンピュータ*/
    { BRK_HZD_NULL   ,NULL,UNIT( 0,0,0 ), { 0,0,0 } },
} ;

/**********************************************

  データリスト

  **********************************************/
HZD_BOX *BRK_HZDList[] = {
    NULL,

    /*タンカー編*/
    w01f_stair, w01f_bar, w01f_tvspace1, w01f_tvspace2, /*4*/
    w01d_store, w01d_kitchen, w01d_dining1, w01d_dining2, w01d_dining1,/*9*/
    w01f_bar2, w01f_bar3, /*11*/

    /*プラント編*/
    w11b_fortunestage,/*12*/

    w16a_kitchen, w16a_sink, w16a_dashboard, w16a_largeglass,/*16*/
    w16a_toilet_men, w16a_toilet_women,/*18*/
    w16a_dining_table1, w16a_dining_table2, w16a_dining_table3, /*21*/
    w16a_locker, w16a_cabinet,/*23*/

    w24b_refresh, w24b_bottlekeep,/*25*/
    w24c_party, w24c_speech,      /*27*/

    w24d_comp_room1, w24d_comp_room2, /*28*/
} ;

int BRK_HZD_NList = sizeof(BRK_HZDList)/sizeof(HZD_BOX *) ;
