/*
   MDU_mdl.h
   モデル関連ヘッダ for MGS2 on linux

   by M.Sonoyama 1999.Sep.～ 
   Modified by K.Kano , 11/13/1999

   $Id: MDU_mdl.h,v 1.14 2002/06/04 13:45:38 usr01363 Exp $
   
   KONAMI COMPUTER ENTERTAINMENT JAPAN WEST CS1
*/

#include	"fmt_kms.h"
#include	"fmt_km4.h"
#include	"P3D.h"

#include "Kmx.h"
#include "block.h"

/* MDU_mdlSave.c */
extern	KM3_DEF2	*MDU_LoadKm3( char * ) ;
extern	KM4_DEF2	*MDU_LoadKm4( char * ) ;
extern	CV2_OBJS	*MDU_LoadCv2( char * ) ;
extern	KMS_DEF		*MDU_LoadKms( char * ) ;

KMS2_DEF *MDU_LoadKms2(char *name);
EVM_DEF *MDU_LoadEvm(char *name);


/* MDU_mdlSave.c */
extern	void		MDU_SaveKm3( char *, KM3_DEF2 * ) ;
extern	void		MDU_SaveKm4( char *, KM4_DEF2 * ) ;
extern	void		MDU_SaveCv2( char *, CV2_OBJS * ) ;
extern	void		MDU_SaveKms( char *, KMS_DEF * ) ;

int MDU_SaveKms2(char *name,KMS2_DEF *sdef,int multi_uv_flag);
int MDU_SaveEvm(char *name,EVM_DEF *sdef,int multi_uv_flag);


/* MDU_mdl2km3.c */
extern	KM3_DEF2	*MDU_Mdl2Km3( LPCSTR, LPP3DXYZ, HP3DMODEL, int ) ;

/* MDU_mdl2km4.c */
extern	KM4_DEF2	*MDU_Mdl2Km4( LPCSTR, LPP3DXYZ, HP3DMODEL, int ) ;

/* MDU_mdlUtil.c */
extern	void		MDU_KmsChangeTexID( KMS_DEF *, char **, char **, int ) ;
extern  void		MDU_Kms2ChangeTexID( KMS2_DEF *sdef, char **olds, char **news, int n ) ;
extern	void		MDU_JointModel( KM3_DEF * ) ;
extern	void		MDU_ModelVertsInt( KM3_DEF * ) ;
extern	void		MDU_Km4JointModel( KM4_DEF * ) ;
extern	void		MDU_Km4ModelVertsInt( KM4_DEF * ) ;
extern	void		MDU_DumpKm3( KM3_DEF2 * ) ;
extern	void		MDU_DumpKms( KMS_DEF * ) ;
extern	void		MDU_CheckVertUsrdata( KM4_DEF * ) ;

/* MDU_mdlKm32Kms.c */
extern	void		MDU_mdlFreeKm3Def2( KM3_DEF2 * ) ;
extern	KMS_DEF		*MDU_Km32Kms( KM3_DEF2 * ) ;

/* MDU_mdlKm42Kms.c */
extern	void		MDU_mdlFreeKm4Def2( KM4_DEF2 * ) ;
extern	KMS_DEF		*MDU_Km42Kms( KM4_DEF2 *, CV2_OBJS **, int ) ;

/* MDU_mdlDivide.c */
extern	KM3_DEF2	*MDU_DivideKm3( KM3_DEF2 *, SVECTOR * ) ;
extern	KM3_DEF2	*MDU_SplitKm3( KM3_DEF2 * ) ;

/* MDU_mdlDivideKm4.c */
extern	KM4_DEF2	*MDU_DivideKm4( KM4_DEF2 *, SVECTOR * ) ;
extern	KM4_DEF2	*MDU_SplitKm4( KM4_DEF2 * ) ;

enum {
	MDU_FLAG_NO_LIMIT	= 0x00000001,	// 1ストリップ頂点上限なしフラグ
	MDU_FLAG_INDEX		= 0x00000002,	// インデックスデータ生成フラグ
	MDU_FLAG_YFORMAT	= 0x00000004,	// Kms2/X-Box拡張フォーマットの出力
};
