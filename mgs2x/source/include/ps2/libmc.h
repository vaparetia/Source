//BP - Stub file
#ifndef __LIBMC_H__
#define __LIBMC_H__

typedef int	_iconVu0IVECTOR[4];
typedef float	_iconVu0FVECTOR[4];

typedef struct
{
   unsigned char Head[4];
   unsigned short Reserv1;
   unsigned short OffsLF;
   unsigned int Reserv2;
   unsigned int TransRate;
   _iconVu0IVECTOR BgColor[4];
   _iconVu0FVECTOR LightDir[3];
   _iconVu0FVECTOR LightColor[3];
   _iconVu0FVECTOR Ambient;
   unsigned char TitleName[68];
   unsigned char FnameView[64];
   unsigned char FnameCopy[64];
   unsigned char FnameDel[64];
   unsigned char Reserve3[512];
} sceMcIconSys;

#define sceMcFuncNoCardInfo	(1)
#define sceMcFuncNoOpen		(2)
#define sceMcFuncNoClose	(3)
#define sceMcFuncNoSeek		(4)
#define sceMcFuncNoRead		(5)
#define sceMcFuncNoWrite	(6)
#define sceMcFuncNoFlush	(10)
#define sceMcFuncNoMkdir	(11)
#define sceMcFuncNoChDir	(12)
#define sceMcFuncNoGetDir	(13)
#define sceMcFuncNoFileInfo	(14)
#define sceMcFuncNoDelete	(15)
#define sceMcFuncNoFormat	(16)
#define sceMcFuncNoUnformat	(17)
#define sceMcFuncNoEntSpace	(18)
#define sceMcFuncNoRename	(19)
#define sceMcFuncChgPrior	(20)
#define sceMcFuncSlotMax	(21)

#endif //__LIBMC_H__
