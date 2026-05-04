#ifndef _memcall_h_
#define _memcall_h_

#include "memcall_work.h"
extern CODEC_Memory CODEC_registed_list[];
extern int          CODEC_mem_last_select;

/*
 * 名前表示領域指定オブジェクトの名前 strcode 定義
 */
#define MEM_name_0    0x004820f2
#define MEM_name_1    0x004820f3
#define MEM_name_2    0x004820f4
#define MEM_name_3    0x004820f5
#define MEM_name_4    0x004820f6
#define MEM_name_5    0x004820f7
#define MEM_name_6    0x004820f8
#define MEM_name_7    0x004820f9

/*
 * 周波数表示領域指定オブジェクトの名前 strcode 定義
 */
#define MEM_freq_0    0x005450e3
#define MEM_freq_1    0x005450e4
#define MEM_freq_2    0x005450e5
#define MEM_freq_3    0x005450e6
#define MEM_freq_4    0x005450e7
#define MEM_freq_5    0x005450e8
#define MEM_freq_6    0x005450e9
#define MEM_freq_7    0x005450ea

#define MEM_f1_00     0x007a0236
#define MEM_f0_10     0x00798256
#define MEM_f0_01     0x00798237
#define MEM_f_base    0x00229397

#define MEM_num0      0x0038e1d0       /* 数字 0 */
#define MEM_num1      0x0038e1d1       /* 数字 1 */
#define MEM_num2      0x0038e1d2       /* 数字 2 */
#define MEM_num3      0x0038e1d3       /* 数字 3 */
#define MEM_num4      0x0038e1d4       /* 数字 4 */
#define MEM_num5      0x0038e1d5       /* 数字 5 */
#define MEM_num6      0x0038e1d6       /* 数字 6 */
#define MEM_num7      0x0038e1d7       /* 数字 7 */
#define MEM_num8      0x0038e1d8       /* 数字 8 */
#define MEM_num9      0x0038e1d9       /* 数字 9 */

int    MemCallCount(void);
char * MemCallGetRes(int area);
int    MemCallGetFreq(int area);
int    MemCallSetName(int area, int freq, int name_id);
int    MemCallSelect(int area);
void * NewMemCallList(int l2d_handle, int selected);

#endif /* _memcall_h_ */
