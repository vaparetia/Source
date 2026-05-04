//BP - Stub file
#ifndef __LIBMC2_H__
#define __LIBMC2_H__

#define SCE_MC2_DMA_BUFFER_MAX	(16)
#define SCE_MC2_MAX_DEVICE_NAME	(16)
#define SCE_MC2_MAX_NAME_LEN	(32)
#define SCE_MC2_MAX_PATH_LEN	(68)
#define SCE_MC2_PORT_1		(2)
#define SCE_MC2_PORT_2		(3)
#define SCE_MC2_SPECIFIC_PORT		( 1 << 1 )
#define SCE_MC2_SPECIFIC_DRIVER_NUMBER	( 1 << 2 )
#define SCE_MC2_SPECIFIC_DEVICE_NAME	( 1 << 3 )

typedef struct {
   unsigned char reserved;
   unsigned char sec;
   unsigned char min;
   unsigned char hour;
   unsigned char day;
   unsigned char month;
   unsigned short year;
} SceMc2DateParam;

typedef struct {
   SceMc2DateParam creation;
   SceMc2DateParam modification;
   unsigned int size;
   unsigned short attribute;
   unsigned short resv1;
   char name[SCE_MC2_MAX_NAME_LEN];
} SceMc2DirParam;

typedef struct {
   int type;		/* メディアタイプ 1:PS1/PDA, 2:PS2 */
   int isFormat;		/* 1:フォーマット済み 0:未フォーマット */
   int freeClust;		/* 空きクラスタ数 */
} SceMc2InfoParam;

#endif //__LIBMC2_H__
