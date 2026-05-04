/*
	ps2.h
	ＰＳ２関連設定マクロ移植ヘッダ(from eestruct.h) [[connection settings macro porting header]]
	$Id: ps2.h,v 1.1 2000/09/28 00:27:51 usr02774 Exp $
*/

#ifndef __PS2_H__
#define __PS2_H__

#ifdef __cplusplus
extern "C"{
#endif


/*-vif0--------------------------------------------------*/

#define SCE_VIF0_SET_CODE(immediate, num, cmd, irq) ((u_int)(immediate) | ((u_int)(num) << 16) | ((u_int)(cmd) << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_NOP(irq) ((u_int)(irq) << 31)

#define SCE_VIF0_SET_STCYCL(wl, cl, irq) ((u_int)(cl) | ((u_int)(wl) << 8) | ((u_int)(0) << 16) | ((u_int)0x01 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_ITOP(itop, irq) ((u_int)(itop) | ((u_int)0x04 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_STMOD(stmod, irq) ((u_int)(stmod) | ((u_int)0x05 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_MARK(mark, irq) ((u_int)(mark) | ((u_int)0x07 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_FLUSHE(irq) (((u_int)0x10 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_MSCAL(vuaddr, irq) ((u_int)(vuaddr) | ((u_int)0x14 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_MSCNT(irq) (((u_int)0x17 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_STMASK(irq) (((u_int)0x20 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_STROW(irq) (((u_int)0x30 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_STCOL(irq) (((u_int)0x31 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_MPG(vuaddr, num, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x4a) << 24) | ((u_int)(irq) << 31))

#define SCE_VIF0_SET_UNPACK(vuaddr, num, cmd, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x60 | (cmd)) << 24) | ((u_int)(irq) << 31))



/*-vif1--------------------------------------------------*/


#define SCE_VIF1_SET_CODE(immediate, num, cmd, irq) ((u_int)(immediate) | ((u_int)(num) << 16) | ((u_int)(cmd) << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_NOP(irq) ((u_int)(irq) << 31)

#define SCE_VIF1_SET_STCYCL(wl, cl, irq) ((u_int)(cl) | ((u_int)(wl) << 8) | ((u_int)(0) << 16) | ((u_int)0x01 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_OFFSET(offset, irq) ((u_int)(offset) | ((u_int)0x02 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_BASE(base, irq) ((u_int)(base) | ((u_int)0x03 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_ITOP(itop, irq) ((u_int)(itop) | ((u_int)0x04 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_STMOD(stmod, irq) ((u_int)(stmod) | ((u_int)0x05 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_MSKPATH3(msk, irq) ((u_int)(msk) | ((u_int)0x06 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_MARK(mark, irq) ((u_int)(mark) | ((u_int)0x07 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_FLUSHE(irq) (((u_int)0x10 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_FLUSH(irq) (((u_int)0x11 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_FLUSHA(irq) (((u_int)0x13 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_MSCAL(vuaddr, irq) ((u_int)(vuaddr) | ((u_int)0x14 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_MSCNT(irq) (((u_int)0x17 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_MSCALF(vuaddr, irq) ((u_int)(vuaddr) | ((u_int)0x15 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_STMASK(irq) (((u_int)0x20 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_STROW(irq) (((u_int)0x30 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_STCOL(irq) (((u_int)0x31 << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_MPG(vuaddr, num, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x4a) << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_DIRECT(count, irq) ((u_int)(count) | ((u_int)(0x50) << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_DIRECTHL(count, irq) ((u_int)(count) | ((u_int)(0x51) << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_UNPACK(vuaddr, num, cmd, irq) ((u_int)(vuaddr) | ((u_int)(num) << 16) | ((u_int)(0x60 | (cmd)) << 24) | ((u_int)(irq) << 31))

#define SCE_VIF1_SET_UNPACKR(vuaddr, num, cmd, irq) ((u_int)((vuaddr) | 0x8000) | ((u_int)(num) << 16) | ((u_int)(0x60 | (cmd)) << 24) | ((u_int)(irq) << 31))

/*-gif--------------------------------------------------*/

/* Utility for making giftag */

#ifdef __GNUC__
#define ALIGN_STRUCT_START(alignment)
#define ALIGN_STRUCT_END(alignment) __attribute__((aligned(alignment)))
#else
#define ALIGN_STRUCT_START(alignment) __declspec(align(alignment))
#define ALIGN_STRUCT_END(alignment)
#endif

ALIGN_STRUCT_START(16) struct _sceGifTag
{
	unsigned long64 NLOOP:15;
	unsigned long64 EOP:1;
	unsigned long64 pad16:16;
	unsigned long64 id:14;
	unsigned long64 PRE:1;
	unsigned long64 PRIM:11;
	unsigned long64 FLG:2;
	unsigned long64 NREG:4;
	unsigned long64 REGS0:4;
	unsigned long64 REGS1:4;
	unsigned long64 REGS2:4;
	unsigned long64 REGS3:4;
	unsigned long64 REGS4:4;
	unsigned long64 REGS5:4;
	unsigned long64 REGS6:4;
	unsigned long64 REGS7:4;
	unsigned long64 REGS8:4;
	unsigned long64 REGS9:4;
	unsigned long64 REGS10:4;
	unsigned long64 REGS11:4;
	unsigned long64 REGS12:4;
	unsigned long64 REGS13:4;
	unsigned long64 REGS14:4;
	unsigned long64 REGS15:4;
}
ALIGN_STRUCT_END(16);

typedef struct _sceGifTag sceGifTag;

#define SCE_GIF_SET_TAG(nloop, eop, pre, prim, flg, nreg) \
	((long64)(nloop) | ((long64)(eop)<<15) | ((long64)(pre) << 46) | \
	((long64)(prim)<<47) | ((long64)(flg)<<58) | ((long64)(nreg)<<60))

#if 0 /* no support 128bit variable */
#define SCE_GIF_CLEAR_TAG(tp) \
	(*(u_long128 *)(tp) = (u_long128)0)
#endif

#define SCE_GIF_PACKED		0
#define SCE_GIF_REGLIST		1
#define SCE_GIF_IMAGE		2
#define SCE_GIF_PACKED_AD 	0x0e

/* Bit patterns for GS general registers */

typedef struct {
	unsigned long64 A:2;
	unsigned long64 B:2;
	unsigned long64 C:2;
	unsigned long64 D:2;
	unsigned long64 pad8:24;
	unsigned long64 FIX:8;
	unsigned long64 pad40:24;
} sceGsAlpha;

typedef struct {
	unsigned long64 SBP:14;
	unsigned long64 pad14:2;
	unsigned long64 SBW:6;
	unsigned long64 pad22:2;
	unsigned long64 SPSM:6;
	unsigned long64 pad30:2;
	unsigned long64 DBP:14;
	unsigned long64 pad46:2;
	unsigned long64 DBW:6;
	unsigned long64 pad54:2;
	unsigned long64 DPSM:6;
	unsigned long64 pad62:2;
} sceGsBitbltbuf;

typedef struct {
	unsigned long64 WMS:2;
	unsigned long64 WMT:2;
	unsigned long64 MINU:10;
	unsigned long64 MAXU:10;
	unsigned long64 MINV:10;
	unsigned long64 MAXV:10;
	unsigned long64 pad44:20;
} sceGsClamp;

typedef struct {
	unsigned long64 CLAMP:1;
	unsigned long64 pad01:63;
} sceGsColclamp;

typedef struct {
	unsigned long64 DIMX00:3;
	unsigned long64 pad00:1;
	unsigned long64 DIMX01:3;
	unsigned long64 pad01:1;
	unsigned long64 DIMX02:3;
	unsigned long64 pad02:1;
	unsigned long64 DIMX03:3;
	unsigned long64 pad03:1;

	unsigned long64 DIMX10:3;
	unsigned long64 pad10:1;
	unsigned long64 DIMX11:3;
	unsigned long64 pad11:1;
	unsigned long64 DIMX12:3;
	unsigned long64 pad12:1;
	unsigned long64 DIMX13:3;
	unsigned long64 pad13:1;

	unsigned long64 DIMX20:3;
	unsigned long64 pad20:1;
	unsigned long64 DIMX21:3;
	unsigned long64 pad21:1;
	unsigned long64 DIMX22:3;
	unsigned long64 pad22:1;
	unsigned long64 DIMX23:3;
	unsigned long64 pad23:1;

	unsigned long64 DIMX30:3;
	unsigned long64 pad30:1;
	unsigned long64 DIMX31:3;
	unsigned long64 pad31:1;
	unsigned long64 DIMX32:3;
	unsigned long64 pad32:1;
	unsigned long64 DIMX33:3;
	unsigned long64 pad33:1;
} sceGsDimx;

typedef struct {
	unsigned long64 DTHE:1;
	unsigned long64 pad01:63;
} sceGsDthe;

typedef struct {
	unsigned long64 FBA:1;
	unsigned long64 pad01:63;
} sceGsFba;

typedef struct {
	unsigned long64 pad00;
} sceGsFinish;

typedef struct {
	unsigned long64 FCR:8;
	unsigned long64 FCG:8;
	unsigned long64 FCB:8;
	unsigned long64 pad24:40;
} sceGsFogcol;

typedef struct {
	unsigned long64 FBP:9;
	unsigned long64 pad09:7;
	unsigned long64 FBW:6;
	unsigned long64 pad22:2;
	unsigned long64 PSM:6;
	unsigned long64 pad30:2;
	unsigned long64 FBMSK:32;
} sceGsFrame;

typedef struct {
	unsigned long64 WDATA;
} sceGsHwreg;

typedef struct {
	u_int ID;
	u_int IDMSK;
} sceGsLabel;

typedef struct {
	unsigned long64 TBP1:14;
	unsigned long64 TBW1:6;
	unsigned long64 TBP2:14;
	unsigned long64 TBW2:6;
	unsigned long64 TBP3:14;
	unsigned long64 TBW3:6;
	unsigned long64 pad60:4;
} sceGsMiptbp1;

typedef struct {
	unsigned long64 TBP4:14;
	unsigned long64 TBW4:6;
	unsigned long64 TBP5:14;
	unsigned long64 TBW5:6;
	unsigned long64 TBP6:14;
	unsigned long64 TBW6:6;
	unsigned long64 pad60:4;
} sceGsMiptbp2;

typedef struct {
	unsigned long64 PABE:1;
	unsigned long64 pad01:63;
} sceGsPabe;

typedef struct {
	unsigned long64 PRIM:3;
	unsigned long64 IIP:1;
	unsigned long64 TME:1;
	unsigned long64 FGE:1;
	unsigned long64 ABE:1;
	unsigned long64 AA1:1;
	unsigned long64 FST:1;
	unsigned long64 CTXT:1;
	unsigned long64 FIX:1;
	unsigned long64 pad11:53;
} sceGsPrim;

typedef struct {
	unsigned long64 pad00:3;
	unsigned long64 IIP:1;
	unsigned long64 TME:1;
	unsigned long64 FGE:1;
	unsigned long64 ABE:1;
	unsigned long64 AA1:1;
	unsigned long64 FST:1;
	unsigned long64 CTXT:1;
	unsigned long64 FIX:1;
	unsigned long64 pad11:53;
} sceGsPrmode;

typedef struct {
	unsigned long64 AC:1;
	unsigned long64 pad01:63;
} sceGsPrmodecont;

typedef struct {
	u_int R:8;
	u_int G:8;
	u_int B:8;
	u_int A:8;
	float Q;
} sceGsRgbaq;

typedef struct {
	unsigned long64 MSK:2;
	unsigned long64 pad02:62;
} sceGsScanmsk;

typedef struct {
	unsigned long64 SCAX0:11;
	unsigned long64 pad11:5;
	unsigned long64 SCAX1:11;
	unsigned long64 pad27:5;
	unsigned long64 SCAY0:11;
	unsigned long64 pad43:5;
	unsigned long64 SCAY1:11;
	unsigned long64 pad59:5;
} sceGsScissor;

typedef struct {
	u_int ID;
	u_int IDMSK;
} sceGsSignal;

typedef struct {
	float S;
	float T;
} sceGsSt;

typedef struct {
	unsigned long64 ATE:1;
	unsigned long64 ATST:3;
	unsigned long64 AREF:8;
	unsigned long64 AFAIL:2;
	unsigned long64 DATE:1;
	unsigned long64 DATM:1;
	unsigned long64 ZTE:1;
	unsigned long64 ZTST:2;
	unsigned long64 pad19:45;
} sceGsTest;

typedef struct {
	unsigned long64 TBP0:14;
	unsigned long64 TBW:6;
	unsigned long64 PSM:6;
	unsigned long64 TW:4;
	unsigned long64 TH:4;
	unsigned long64 TCC:1;
	unsigned long64 TFX:2;
	unsigned long64 CBP:14;
	unsigned long64 CPSM:4;
	unsigned long64 CSM:1;
	unsigned long64 CSA:5;
	unsigned long64 CLD:3;
} sceGsTex0;

typedef struct {
	unsigned long64 LCM:1;
	unsigned long64 pad01:1;
	unsigned long64 MXL:3;
	unsigned long64 MMAG:1;
	unsigned long64 MMIN:3;
	unsigned long64 MTBA:1;
	unsigned long64 pad10:9;
	unsigned long64 L:2;
	unsigned long64 pad21:11;
	unsigned long64 K:12;
	unsigned long64 pad44:20;
} sceGsTex1;

typedef struct {
	unsigned long64 pad00:20;
	unsigned long64 PSM:6;
	unsigned long64 pad26:11;
	unsigned long64 CBP:14;
	unsigned long64 CPSM:4;
	unsigned long64 CSM:1;
	unsigned long64 CSA:5;
	unsigned long64 CLD:3;
} sceGsTex2;

typedef struct {
	unsigned long64 TA0:8;
	unsigned long64 pad08:7;
	unsigned long64 AEM:1;
	unsigned long64 pad16:16;
	unsigned long64 TA1:8;
	unsigned long64 pad40:24;
} sceGsTexa;

typedef struct {
	unsigned long64 CBW:6;
	unsigned long64 COU:6;
	unsigned long64 COV:10;
	unsigned long64 pad22:42;
} sceGsTexclut;

typedef struct {
	unsigned long64 pad00;
} sceGsTexflush;

typedef struct {
	unsigned long64 XDR:2;
	unsigned long64 pad02:62;
} sceGsTrxdir;

typedef struct {
	unsigned long64 SSAX:11;
	unsigned long64 pad11:5;
	unsigned long64 SSAY:11;
	unsigned long64 pad27:5;
	unsigned long64 DSAX:11;
	unsigned long64 pad43:5;
	unsigned long64 DSAY:11;
	unsigned long64 DIR:2;
	unsigned long64 pad61:3;
} sceGsTrxpos;

typedef struct {
	unsigned long64 RRW:12;
	unsigned long64 pad12:20;
	unsigned long64 RRH:12;
	unsigned long64 pad44:20;
} sceGsTrxreg;

typedef struct {
	unsigned long64 U:14;
	unsigned long64 pad14:2;
	unsigned long64 V:14;
	unsigned long64 pad30:34;
} sceGsUv;

typedef struct {
	unsigned long64 OFX:16;
	unsigned long64 pad16:16;
	unsigned long64 OFY:16;
	unsigned long64 pad48:16;
} sceGsXyoffset;

typedef struct {
	unsigned long64 X:16;
	unsigned long64 Y:16;
	unsigned long64 Z:32;
} sceGsXyz;

typedef struct {
	unsigned long64 X:16;
	unsigned long64 Y:16;
	unsigned long64 Z:24;
	unsigned long64 F:8;
} sceGsXyzf;

typedef struct {
	unsigned long64 ZBP:9;
	unsigned long64 pad09:15;
	unsigned long64 PSM:4;
	unsigned long64 pad28:4;
	unsigned long64 ZMSK:1;
	unsigned long64 pad33:31;
} sceGsZbuf;


/* bit access macro for GS general registers */

//	
// GS_PRIM
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// |                                                               |
// |                                                               |
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                         |F|C|F|A|A|F|T|I|     |
// |                                         |I|T|S|A|B|G|M|I|PRIM |
// |                                         |X|X|T|1|E|E|E|P|     |
// |                                         | |T| | | | | | |     |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// bit definition
//	
#define GS_PRIM_PRIM_M  		(0x07<< 0)
#define GS_PRIM_IIP_M  			(0x01<< 3)
#define GS_PRIM_TME_M  			(0x01<< 4)
#define GS_PRIM_FGE_M  			(0x01<< 5)
#define GS_PRIM_ABE_M  			(0x01<< 6)
#define GS_PRIM_AA1_M  			(0x01<< 7)
#define GS_PRIM_FST_M  			(0x01<< 8)
#define GS_PRIM_CTXT_M  		(0x01<< 9)
#define GS_PRIM_FIX_M  		 	(0x01<<10)

#define GS_PRIM_PRIM_O  		( 0)
#define GS_PRIM_IIP_O  			( 3)
#define GS_PRIM_TME_O  			( 4)
#define GS_PRIM_FGE_O  			( 5)
#define GS_PRIM_ABE_O  			( 6)
#define GS_PRIM_AA1_O  			( 7)
#define GS_PRIM_FST_O  			( 8)
#define GS_PRIM_CTXT_O  		( 9)
#define GS_PRIM_FIX_O  		 	(10)

//	
// GS_XYOFFSET
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                               |                                |
// |                               |              OFY               |
// |                               |                                |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                               |                                |
// |                               |              OFX               |
// |                               |                                |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// bit definition
//	
#define GS_XYOFFSET_OFX_M  		(0xffff<< 0)
#define GS_XYOFFSET_OFY_M  		(0xffffL<<32)

#define GS_XYOFFSET_OFX_O  		( 0)
#define GS_XYOFFSET_OFY_O  		(32)


// GS_TEX0:
//
// 63            56              48               40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     |             |C| C |                           |   |T|   |  
// | CLD |    CSA      |S| P |          CBP              |TFX|C|TH1|
// |     |             |K| S |                           |   |C|   |
// |     |             | | M |                           |   | |   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16              8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   |       |           |           |                           | 
// |TH0|  TW   |   PSM     |   TBW     |         TBP0              |
// |   |       |           |           |                           |
// |   |       |           |           |                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_TEX0_TBP0_M 	(0x3fff<< 0)
#define GS_TEX0_TBW_M 	(0x003f<<14)
#define GS_TEX0_PSM_M 	(0x003f<<20)
#define GS_TEX0_TW_M 	(0x000f<<26)
#define GS_TEX0_TH_M 	(0x000fL<<30)
#define GS_TEX0_TCC_M 	(0x0001L<<34)
#define GS_TEX0_TFX_M 	(0x0003L<<35)
#define GS_TEX0_CBP_M 	(0x3fffL<<37)
#define GS_TEX0_CPSM_M 	(0x000fL<<51)
#define GS_TEX0_CSM_M 	(0x0001L<<55)
#define GS_TEX0_CSA_M 	(0x001fL<<56)
#define GS_TEX0_CLD_M 	(0x0007L<<61)

#define GS_TEX0_TBP0_O 	( 0)
#define GS_TEX0_TBW_O 	(14)
#define GS_TEX0_PSM_O 	(20)
#define GS_TEX0_TW_O 	(26)
#define GS_TEX0_TH_O 	(30)
#define GS_TEX0_TCC_O 	(34)
#define GS_TEX0_TFX_O 	(35)
#define GS_TEX0_CBP_O 	(37)
#define GS_TEX0_CPSM_O 	(51)
#define GS_TEX0_CSM_O 	(55)
#define GS_TEX0_CSA_O 	(56)
#define GS_TEX0_CLD_O 	(61)


// GS_TEX1:
//
// 63            56              48               40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                               |               |
// |                                               |      K        |
// |                                               |               |
// |                                               |               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16              8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                     |   |                 |M|     |M|     | |L| 
// |                     | L |                 |T|MMIN |M| MXL | |C|
// |                     |   |                 |B|     |A|     | |M|
// |                     |   |                 |A|     |G|     | | |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_TEX1_LCM_M 	(0x01<< 0)
#define GS_TEX1_MXL_M 	(0x07<< 2)
#define GS_TEX1_MMAG_M 	(0x01<< 5)
#define GS_TEX1_MMIN_M 	(0x07<< 6)
#define GS_TEX1_MTBA_M 	(0x01<< 9)
#define GS_TEX1_L_M 	(0x03<<19)
#define GS_TEX1_K_M 	(0xffL<<32)

#define GS_TEX1_LCM_O 	( 0)
#define GS_TEX1_MXL_O 	( 2)
#define GS_TEX1_MMAG_O 	( 5)
#define GS_TEX1_MMIN_O 	( 6)
#define GS_TEX1_MTBA_O 	( 9)
#define GS_TEX1_L_O 	(19)
#define GS_TEX1_K_O 	(32)


// GS_TEX2:
//
// 63             56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     |         |C|   C   |                           |         |  
// | CLD |   CSA   |S|   P   |           CBP             |         |
// |     |         |M|   S   |                           |         |
// |     |         | |   M   |                           |         |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16              8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |           |           |                                       | 
// |           |   PSM     |                                       |
// |           |           |                                       |
// |           |           |                                       |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_TEX2_PSM_M 	(0x3f  <<20)
#define GS_TEX2_CBP_M 	(0x3fffL<<37)
#define GS_TEX2_CPSM_M 	(0x0fL  <<51)
#define GS_TEX2_CSM_M 	(0x01L  <<55)
#define GS_TEX2_CSA_M 	(0x1fL  <<56)
#define GS_TEX2_CLD_M 	(0x07L  <<61)

#define GS_TEX2_PSM_O 	(20)
#define GS_TEX2_CBP_O 	(37)
#define GS_TEX2_CPSM_O 	(51)
#define GS_TEX2_CSM_O 	(55)
#define GS_TEX2_CSA_O 	(56)
#define GS_TEX2_CLD_O 	(61)


// GS_TEXCLUT:
//
// 63             56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// |                                                               |
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                       |           |     |     |
// |                                       |    COV    | COU | CBW |
// |                                       |           |     |     |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_TEXCLUT_CBW_M 	(0x07<<0)
#define GS_TEXCLUT_COU_M 	(0x07<<3)
#define GS_TEXCLUT_COV_M 	(0x3f<<6)

#define GS_TEXCLUT_CBW_O 	(0)
#define GS_TEXCLUT_COU_O 	(3)
#define GS_TEXCLUT_COV_O 	(6)


// GS_MIPTBP1:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       |           |                           |           | T |
// |       |           |                           |           | B |
// |       |    TBW3   |           TBP3            |    TBW2   | P |
// |       |           |                           |           | 2 |
// |       |           |                           |           | H |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       |           |                           |
// |                       |           |                           |
// |        TBP2L          |    TBW1   |           TBP1            |
// |                       |           |                           |
// |                       |           |                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_MIPTBP1_TBP1_M 	(0x3fff<< 0)
#define GS_MIPTBP1_TBW1_M 	(0x3f  <<14)
#define GS_MIPTBP1_TBP2_M 	(0x3fffL<<20)
#define GS_MIPTBP1_TBW2_M 	(0x3fL  <<34)
#define GS_MIPTBP1_TBP3_M 	(0x3fffL<<40)
#define GS_MIPTBP1_TBW3_M 	(0x3fL  <<54)

#define GS_MIPTBP1_TBP1_O 	( 0)
#define GS_MIPTBP1_TBW1_O 	(14)
#define GS_MIPTBP1_TBP2_O 	(20)
#define GS_MIPTBP1_TBW2_O 	(34)
#define GS_MIPTBP1_TBP3_O 	(40)
#define GS_MIPTBP1_TBW3_O 	(54)


// GS_MIPTBP2:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       |           |                           |           | T |
// |       |           |                           |           | B |
// |       |    TBW6   |           TBP6            |    TBW5   | P |
// |       |           |                           |           | 5 |
// |       |           |                           |           | H |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                       |           |                           |
// |                       |           |                           |
// |        TBP5L          |    TBW4   |           TBP4            |
// |                       |           |                           |
// |                       |           |                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_MIPTBP2_TBP4_M 	(0x3fff<< 0)
#define GS_MIPTBP2_TBW4_M 	(0x3f  <<14)
#define GS_MIPTBP2_TBP5_M 	(0x3fffL<<20)
#define GS_MIPTBP2_TBW5_M 	(0x3fL  <<34)
#define GS_MIPTBP2_TBP6_M 	(0x3fffL<<40)
#define GS_MIPTBP2_TBW6_M 	(0x3fL  <<54)

#define GS_MIPTBP2_TBP4_O 	( 0)
#define GS_MIPTBP2_TBW4_O 	(14)
#define GS_MIPTBP2_TBP5_O 	(20)
#define GS_MIPTBP2_TBW5_O 	(34)
#define GS_MIPTBP2_TBP6_O 	(40)
#define GS_MIPTBP2_TBW6_O 	(54)


// GS_TEXA:
//
// 63             56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                               |               |
// |                                               |      TA1      |
// |                                               |               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                               |A|             |               |
// |                               |E|             |       TA0     |
// |                               |M|             |               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_TEXA_TA0_M 	(0xff<< 0)
#define GS_TEXA_AEM_M 	(0x01<<15)
#define GS_TEXA_TA1_M 	(0xffL<<32)

#define GS_TEXA_TA0_O 	( 0)
#define GS_TEXA_AEM_O 	(15)
#define GS_TEXA_TA1_O 	(32)

 

// GS_CLAMP:
//
// 63             56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                       |                   |MIN|
// |                                       |        MAXV       | V1|
// |                                       |                   |   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |               |                   |                   | W | W |
// |     MINV0     |        MAXU       |        MINU       | M | M |
// |               |                   |                   | T | S |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_CLAMP_WMS_M  	(0x03 << 0)
#define GS_CLAMP_WMT_M  	(0x03 << 2)
#define GS_CLAMP_MINU_M 	(0x3ff<< 4)
#define GS_CLAMP_MAXU_M 	(0x3ff<<14)
#define GS_CLAMP_MINV_M 	(0x3ffL<<24)
#define GS_CLAMP_MAXV_M 	(0x3ffL<<34)

#define GS_CLAMP_WMS_O  	( 0)
#define GS_CLAMP_WMT_O  	( 2)
#define GS_CLAMP_MINU_O 	( 4)
#define GS_CLAMP_MAXU_O 	(14)
#define GS_CLAMP_MINV_O 	(24)
#define GS_CLAMP_MAXV_O 	(34)
 

// GS_FOGCOL:
//
// 63             56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// |                                                               |
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |               |               |               |               |
// |               |      FCB      |       FCG     |      FCR      |
// |               |               |               |               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_FOGCOL_FCR_M  	(0xff<< 0)
#define GS_FOGCOL_FCG_M  	(0xff<< 8)
#define GS_FOGCOL_FCB_M  	(0xff<<16)

#define GS_FOGCOL_FCR_O  	( 0)
#define GS_FOGCOL_FCG_O  	( 8)
#define GS_FOGCOL_FCB_O 	(16)
 

// GS_SCISSOR:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         |                     |         |                     |
// |         |        SCAY1        |         |         SCAY0       |
// |         |                     |         |                     |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         |                     |         |                     |
// |         |        SCAX1        |         |        SCAX0        |
// |         |                     |         |                     |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_SCISSOR_SCAX0_M  	(0x7ff<< 0)
#define GS_SCISSOR_SCAX1_M  	(0x7ff<<16)
#define GS_SCISSOR_SCAY0_M  	(0x7ffL<<32)
#define GS_SCISSOR_SCAY1_M  	(0x7ffL<<48)

#define GS_SCISSOR_SCAX0_O  	( 0)
#define GS_SCISSOR_SCAX1_O  	(16)
#define GS_SCISSOR_SCAY0_O 	(32)
#define GS_SCISSOR_SCAY1_O 	(48) 


// GS_TEST:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// |                                                               |
// |                                                               |
// |                                                               |
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                         | Z |Z| |D| A |               |  A  |A|
// |                         | T |T| |A| F |               |  T  |T|
// |                         | S |E| |T| A |     AREF      |  S  |E|
// |                         | T | | |E| I |               |  T  | |
// |                         |   | | | | L |               |     | |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_TEST_ATE_M  		(0x01<< 0)
#define GS_TEST_ATST_M  	(0x07<< 1)
#define GS_TEST_AREF_M  	(0xff<< 4)
#define GS_TEST_AFAIL_M  	(0x03<<12)
#define GS_TEST_DATE_M  	(0x01<<14)
#define GS_TEST_DATM_M  	(0x01<<15)
#define GS_TEST_ZTE_M 	 	(0x01<<16)
#define GS_TEST_ZTST_M  	(0x03<<17)



#define GS_TEST_ATE_O  		( 0)
#define GS_TEST_ATST_O  	( 1)
#define GS_TEST_AREF_O  	( 4)
#define GS_TEST_AFAIL_O  	(12)
#define GS_TEST_DATE_O  	(14)
#define GS_TEST_DATM_O  	(15)
#define GS_TEST_ZTE_O 	 	(16)
#define GS_TEST_ZTST_O  	(17)


// GS_ALPHA:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                               |               |
// |                                               |      FIX      |
// |                                               |               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                               |   |   |   |   |
// |                                               | D | C | B | A |
// |                                               |   |   |   |   |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition 
//	
#define GS_ALPHA_A_M  		(0x03<< 0)
#define GS_ALPHA_B_M  		(0x03<< 2)
#define GS_ALPHA_C_M  		(0x03<< 4)
#define GS_ALPHA_D_M 	  	(0x03<< 6)
#define GS_ALPHA_FIX_M  	(0xffL<<32)

#define GS_ALPHA_A_O  		( 0)
#define GS_ALPHA_B_O  		( 2)
#define GS_ALPHA_C_O  		( 4)
#define GS_ALPHA_D_O 	 	( 6)
#define GS_ALPHA_FIX_O  	(32)


// GS_FRAME:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                               |
// |                             FBMSK                            |
// |                                                               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   |           |   |           |               |               |
// |   |    PSM    |   |    FBW    |               |      FBP      |
// |   |           |   |           |               |               |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition
//	
#define GS_FRAME_FBP_M  		(0xff      << 0)
#define GS_FRAME_FBM_M  		(0x3f      <<16)
#define GS_FRAME_PSM_M  		(0x3f      <<24)
#define GS_FRAME_FBMSK_M  		(0xffffffffL<<32)

#define GS_FRAME_FBP_O  		( 0)
#define GS_FRAME_FBM_O  		(16)
#define GS_FRAME_PSM_O  		(24)
#define GS_FRAME_FBMSK_O 	 	(32)


// GS_ZBUF:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                                             |Z|
// |                                                             |M|
// |                                                             |S|
// |                                                             |K|
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |       |       |                             |                 |
// |       |  PSM  |                             |       ZBP       |
// |       |       |                             |                 |
// |       |       |                             |                 |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition
//	
#define GS_ZBUF_ZBP_M  			(0x1ff<< 0)
#define GS_ZBUF_PSM_M  			(0x0f <<24)
#define GS_ZBUF_ZMSK_M  		(0x01L <<32)

#define GS_ZBUF_ZBP_O 	 		( 0)
#define GS_ZBUF_PSM_O  			(24)
#define GS_ZBUF_ZMSK_O  		(32)


// GS_BITBLTBUF:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   |           |   |           |   |                           |
// |   |   DPSM    |   |    DBW    |   |             DBP           |
// |   |           |   |           |   |                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |   |           |   |           |   |                           |
// |   |    SPSM   |   |    SBW    |   |          SBP              |
// |   |           |   |           |   |                           |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition
//	
#define GS_BITBLTBUF_SBP_M  			(0x3fff<< 0)
#define GS_BITBLTBUF_SBW_M  			(0x3f  <<16)
#define GS_BITBLTBUF_SPSM_M  			(0x3f  <<24)
#define GS_BITBLTBUF_DBP_M  			(0x3fffL<<32)
#define GS_BITBLTBUF_DBW_M  			(0x3fL  <<48)
#define GS_BITBLTBUF_DPSM_M  			(0x3fL  <<56)

#define GS_BITBLTBUF_SBP_O  			( 0)
#define GS_BITBLTBUF_SBW_O  			(16)
#define GS_BITBLTBUF_SPSM_O  			(24)
#define GS_BITBLTBUF_DBP_O  			(32)
#define GS_BITBLTBUF_DBW_O  			(48)
#define GS_BITBLTBUF_DPSM_O  			(56)



// GS_TRXPOS:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |     |   |                     |         |                     |
// |     |DIR|        DSAY         |         |         DSAX        |
// |     |   |                     |         |                     |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |         |                     |         |                     |
// |         |         SSAY        |         |         SSAX        |
// |         |                     |         |                     |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition
//	
#define GS_TRXPOS_SSAX_M  			(0x7ff<< 0)
#define GS_TRXPOS_SSAY_M  			(0x7ff<<16)
#define GS_TRXPOS_DSAX_M  			(0x7ffL<<32)
#define GS_TRXPOS_DSAY_M  			(0x7ffL<<48)
#define GS_TRXPOS_DIR_M  			(0x03L <<59)

#define GS_TRXPOS_SSAX_O  			( 0)
#define GS_TRXPOS_SSAY_O  			(16)
#define GS_TRXPOS_DSAX_O  			(32)
#define GS_TRXPOS_DSAY_O  			(48)
#define GS_TRXPOS_DIR_O  			(59)


// GS_TRXREG:
//
//  63            56              48              40              32
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                       |                       |
// |                                       |          RRH          |
// |                                       |                       |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	
//  31            24              16               8               0
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// |                                       |                       |
// |                                       |           RRW         |
// |                                       |                       |
// +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
// definition
//	
#define GS_TRXREG_RRW_M  			(0xfff<< 0)
#define GS_TRXREG_RRH_M  			(0xfffL<<32)

#define GS_TRXREG_RRW_O  			( 0)
#define GS_TRXREG_RRH_O  			(32)


/* Macros for making bit pattern of GS registers */

#define SCE_GS_SET_ALPHA_1	SCE_GS_SET_ALPHA
#define SCE_GS_SET_ALPHA_2	SCE_GS_SET_ALPHA
#define SCE_GS_SET_ALPHA(a, b, c, d, fix) \
	((u_long64)(a)       | ((u_long64)(b) << 2)     | ((u_long64)(c) << 4) | \
	((u_long64)(d) << 6) | ((u_long64)(fix) << 32))

#define SCE_GS_SET_BITBLTBUF(sbp, sbw, spsm, dbp, dbw, dpsm) \
	((u_long64)(sbp)         | ((u_long64)(sbw) << 16) | \
	((u_long64)(spsm) << 24) | ((u_long64)(dbp) << 32) | \
	((u_long64)(dbw) << 48)  | ((u_long64)(dpsm) << 56))

#define SCE_GS_SET_CLAMP_1	SCE_GS_SET_CLAMP
#define SCE_GS_SET_CLAMP_2	SCE_GS_SET_CLAMP
#define SCE_GS_SET_CLAMP(wms, wmt, minu, maxu, minv, maxv) \
	((u_long64)(wms)         | ((u_long64)(wmt) << 2) | \
	((u_long64)(minu) << 4)  | ((u_long64)(maxu) << 14) | \
	((u_long64)(minv) << 24) | ((u_long64)(maxv) << 34))

#define SCE_GS_SET_COLCLAMP(clamp) ((u_long64)(clamp))

#define SCE_GS_SET_DIMX(dm00, dm01, dm02, dm03, dm10, dm11, dm12, dm13, \
		dm20, dm21, dm22, dm23, dm30, dm31, dm32, dm33) \
	((u_long64)(dm00)        | ((u_long64)(dm01) << 4)  | \
	((u_long64)(dm02) << 8)  | ((u_long64)(dm03) << 12) | \
	((u_long64)(dm10) << 16) | ((u_long64)(dm11) << 20) | \
	((u_long64)(dm12) << 24) | ((u_long64)(dm13) << 28) | \
	((u_long64)(dm20) << 32) | ((u_long64)(dm21) << 36) | \
	((u_long64)(dm22) << 40) | ((u_long64)(dm23) << 44) | \
	((u_long64)(dm30) << 48) | ((u_long64)(dm31) << 52) | \
	((u_long64)(dm32) << 56) | ((u_long64)(dm33) << 60))

#define SCE_GS_SET_DTHE(dthe) ((u_long64)(dthe))

#define SCE_GS_SET_FBA_1	SCE_GS_SET_FBA
#define SCE_GS_SET_FBA_2	SCE_GS_SET_FBA
#define SCE_GS_SET_FBA(fba) ((u_long64)(fba))

#define SCE_GS_SET_FOG(f) ((u_long64)(f) << 56)

#define SCE_GS_SET_FOGCOL(fcr, fcg, fcb) \
	((u_long64)(fcr) | ((u_long64)(fcg) << 8) | ((u_long64)(fcb) << 16))

#define SCE_GS_SET_FRAME_1	SCE_GS_SET_FRAME
#define SCE_GS_SET_FRAME_2	SCE_GS_SET_FRAME
#define SCE_GS_SET_FRAME(fbp, fbw, psm, fbmask) \
	((u_long64)(fbp)        | ((u_long64)(fbw) << 16) | \
	((u_long64)(psm) << 24) | ((u_long64)(fbmask) << 32))

#define SCE_GS_SET_LABEL(id, idmsk) \
	((u_long64)(id) | ((u_long64)(idmsk) << 32))

#define SCE_GS_SET_MIPTBP1_1	SCE_GS_SET_MIPTBP1
#define SCE_GS_SET_MIPTBP1_2	SCE_GS_SET_MIPTBP1
#define SCE_GS_SET_MIPTBP1(tbp1, tbw1, tbp2, tbw2, tbp3, tbw3) \
	((u_long64)(tbp1)        | ((u_long64)(tbw1) << 14) | \
	((u_long64)(tbp2) << 20) | ((u_long64)(tbw2) << 34) | \
	((u_long64)(tbp3) << 40) | ((u_long64)(tbw3) << 54))

#define SCE_GS_SET_MIPTBP2_1	SCE_GS_SET_MIPTBP2
#define SCE_GS_SET_MIPTBP2_2	SCE_GS_SET_MIPTBP2
#define SCE_GS_SET_MIPTBP2(tbp4, tbw4, tbp5, tbw5, tbp6, tbw6) \
	((u_long64)(tbp4)        | ((u_long64)(tbw4) << 14) | \
	((u_long64)(tbp5) << 20) | ((u_long64)(tbw5) << 34) | \
	((u_long64)(tbp6) << 40) | ((u_long64)(tbw6) << 54))

#define SCE_GS_SET_PABE(pabe) ((u_long64)(pabe))

#define SCE_GS_SET_PRIM(prim, iip, tme, fge, abe, aa1, fst, ctxt, fix) \
	((u_long64)(prim)      | ((u_long64)(iip) << 3)  | ((u_long64)(tme) << 4) | \
	((u_long64)(fge) << 5) | ((u_long64)(abe) << 6)  | ((u_long64)(aa1) << 7) | \
	((u_long64)(fst) << 8) | ((u_long64)(ctxt) << 9) | ((u_long64)(fix) << 10))

#define SCE_GS_SET_PRMODE(iip, tme, fge, abe, aa1, fst, ctxt, fix) \
	(((u_long64)(iip) << 3) | ((u_long64)(tme) << 4)  | \
	((u_long64)(fge) << 5) | ((u_long64)(abe) << 6)  | ((u_long64)(aa1) << 7) | \
	((u_long64)(fst) << 8) | ((u_long64)(ctxt) << 9) | ((u_long64)(fix) << 10))

#define SCE_GS_SET_PRMODECONT(ac) ((u_long64)(ac))

#define SCE_GS_SET_RGBAQ(r, g, b, a, q) \
	((u_long64)(r)        | ((u_long64)(g) << 8) | ((u_long64)(b) << 16) | \
	((u_long64)(a) << 24) | ((u_long64)(q) << 32))

#define SCE_GS_SET_SCANMSK(msk) ((u_long64)(msk))

#define SCE_GS_SET_SCISSOR_1	SCE_GS_SET_SCISSOR
#define SCE_GS_SET_SCISSOR_2	SCE_GS_SET_SCISSOR
#define SCE_GS_SET_SCISSOR(scax0, scax1, scay0, scay1) \
	((u_long64)(scax0)        | ((u_long64)(scax1) << 16) | \
	((u_long64)(scay0) << 32) | ((u_long64)(scay1) << 48))

#define SCE_GS_SET_SIGNAL(id, idmsk) \
	((u_long64)(id) | ((u_long64)(idmsk) << 32))

#define SCE_GS_SET_ST(s, t) ((u_long64)(s) |  ((u_long64)(t) << 32))

/* datm bit is unavailable on GS Rev.1 */
#define SCE_GS_SET_TEST_1 SCE_GS_SET_TEST
#define SCE_GS_SET_TEST_2 SCE_GS_SET_TEST
#define SCE_GS_SET_TEST(ate, atst, aref, afail, date, datm, zte, ztst) \
	((u_long64)(ate)         | ((u_long64)(atst) << 1) | \
	((u_long64)(aref) << 4)  | ((u_long64)(afail) << 12) | \
	((u_long64)(date) << 14) | ((u_long64)(datm) << 15) | \
	((u_long64)(zte) << 16)  | ((u_long64)(ztst) << 17))

#define SCE_GS_SET_TEX0_1	SCE_GS_SET_TEX0
#define SCE_GS_SET_TEX0_2	SCE_GS_SET_TEX0
#define SCE_GS_SET_TEX0(tbp, tbw, psm, tw, th, tcc, tfx, \
			cbp, cpsm, csm, csa, cld) \
	((u_long64)(tbp)         | ((u_long64)(tbw) << 14) | \
	((u_long64)(psm) << 20)  | ((u_long64)(tw) << 26) | \
	((u_long64)(th) << 30)   | ((u_long64)(tcc) << 34) | \
	((u_long64)(tfx) << 35)  | ((u_long64)(cbp) << 37) | \
	((u_long64)(cpsm) << 51) | ((u_long64)(csm) << 55) | \
	((u_long64)(csa) << 56)  | ((u_long64)(cld) << 61))

#define SCE_GS_SET_TEX1_1	SCE_GS_SET_TEX1
#define SCE_GS_SET_TEX1_2	SCE_GS_SET_TEX1
#define SCE_GS_SET_TEX1(lcm, mxl, mmag, mmin, mtba, l, k) \
	((u_long64)(lcm)        | ((u_long64)(mxl) << 2)  | \
	((u_long64)(mmag) << 5) | ((u_long64)(mmin) << 6) | \
	((u_long64)(mtba) << 9) | ((u_long64)(l) << 19) | \
	((u_long64)(k) << 32))

#define SCE_GS_SET_TEX2_1	SCE_GS_SET_TEX2
#define SCE_GS_SET_TEX2_2	SCE_GS_SET_TEX2
#define SCE_GS_SET_TEX2(psm, cbp, cpsm, csm, csa, cld) \
	(((u_long64)(psm) << 20) | ((u_long64)(cbp) << 37) | \
	((u_long64)(cpsm) << 51) | ((u_long64)(csm) << 55) | \
	((u_long64)(csa) << 56)  | ((u_long64)(cld) << 61))

#define SCE_GS_SET_TEXA(ta0, aem, ta1) \
	((u_long64)(ta0) | ((u_long64)(aem) << 15) | ((u_long64)(ta1) << 32))

#define SCE_GS_SET_TEXCLUT(cbw, cou, cov) \
	((u_long64)(cbw) | ((u_long64)(cou) << 6) | ((u_long64)(cov) << 12))

#define SCE_GS_SET_TRXDIR(xdr) ((u_long64)(xdr))

#define SCE_GS_SET_TRXPOS(ssax, ssay, dsax, dsay, dir) \
	((u_long64)(ssax)        | ((u_long64)(ssay) << 16) | \
	((u_long64)(dsax) << 32) | ((u_long64)(dsay) << 48) | \
	((u_long64)(dir) << 59))

#define SCE_GS_SET_TRXREG(rrw, rrh) \
	((u_long64)(rrw) | ((u_long64)(rrh) << 32))

#define SCE_GS_SET_UV(u, v) ((u_long64)(u) | ((u_long64)(v) << 16))

#define SCE_GS_SET_XYOFFSET_1	SCE_GS_SET_XYOFFSET
#define SCE_GS_SET_XYOFFSET_2	SCE_GS_SET_XYOFFSET
#define SCE_GS_SET_XYOFFSET(ofx, ofy) ((u_long64)(ofx) | ((u_long64)(ofy) << 32))

#define SCE_GS_SET_XYZ3 SCE_GS_SET_XYZ
#define SCE_GS_SET_XYZ2 SCE_GS_SET_XYZ
#define SCE_GS_SET_XYZ(x, y, z) \
	((u_long64)(x) | ((u_long64)(y) << 16) | ((u_long64)(z) << 32))

#define SCE_GS_SET_XYZF3 SCE_GS_SET_XYZF
#define SCE_GS_SET_XYZF2 SCE_GS_SET_XYZF
#define SCE_GS_SET_XYZF(x, y, z, f) \
	((u_long64)(x) | ((u_long64)(y) << 16) | ((u_long64)(z) << 32) | \
	((u_long64)(f) << 56))

#define SCE_GS_SET_ZBUF_1	SCE_GS_SET_ZBUF
#define SCE_GS_SET_ZBUF_2	SCE_GS_SET_ZBUF
#define SCE_GS_SET_ZBUF(zbp, psm, zmsk) \
	((u_long64)(zbp) | ((u_long64)(psm) << 24) | \
	((u_long64)(zmsk) << 32))

#define SCE_GS_SET_PMODE(en1, en2, crtmd, mmod, amod, \
		slbg, alp) \
	((u_long64)(en1)            | ((u_long64)(en2) << 1)      | \
	((u_long64)(crtmd) << 2)    | ((u_long64)(mmod) << 5)     | \
	((u_long64)(amod) << 6)     | ((u_long64)(slbg) << 7)     | \
	((u_long64)(alp) << 8))

#define SCE_GS_SET_DISPLAY1	SCE_GS_SET_DISPLAY
#define SCE_GS_SET_DISPLAY2	SCE_GS_SET_DISPLAY
#define SCE_GS_SET_DISPLAY(dx, dy, magh, magv, dw, dh) \
	((u_long64)(0x28c+dx)    | \
	((u_long64)((sceGsInterMode?50:25) + dy) << 12) | \
	((u_long64)(magh) << 23) | ((u_long64)(magv) << 27) | \
	((u_long64)(dw) << 32)   | ((u_long64)(dh) << 44))

#define SCE_GS_SET_DISPFB1	SCE_GS_SET_DISPFB
#define SCE_GS_SET_DISPFB2	SCE_GS_SET_DISPFB
#define SCE_GS_SET_DISPFB(fbp, fbw, psm, dbx, dby) \
	((u_long64)(fbp)        | ((u_long64)(fbw) << 9)  | \
	((u_long64)(psm) << 15) | ((u_long64)(dbx) << 32) | \
	((u_long64)(dby) << 43))

#define SCE_GS_SET_SMODE2(intr, ffmd, dpms) \
	((u_long64)(intr) | ((u_long64)(ffmd) << 1) | ((u_long64)(dpms) << 2))

#define SCE_GS_SET_EXTBUF(exbp, exbw, fbin, wffmd, emoda, emodc, wdx, wdy) \
	((u_long64)(exbp)         | ((u_long64)(exbw) << 14)  | \
	((u_long64)(fbin) << 20)  | ((u_long64)(wffmd) << 22) | \
	((u_long64)(emoda) << 23) | ((u_long64)(emodc) << 25) | \
	((u_long64)(wdx) << 32)   | ((u_long64)(wdy) << 43))

#define SCE_GS_SET_EXTDATA(sx, sy, smph, smpv, ww, wh) \
	((u_long64)(sx)          | ((u_long64)(sy) << 12)   | \
	((u_long64)(smph) << 23) | ((u_long64)(smpv) << 27) | \
	((u_long64)(ww) << 32)   | ((u_long64)(wh) << 44))

#define SCE_GS_SET_EXTWRITE(write) ((u_long64)(write))

#define SCE_GS_SET_BGCOLOR(r, g, b) \
	((u_long64)(r) | ((u_long64)(g) << 8) | ((u_long64)(b) << 16))

/* GS registers address */
	/*-- vertex info. reg--*/
#define SCE_GS_PRIM		0x00
#define SCE_GS_RGBAQ		0x01
#define SCE_GS_ST		0x02
#define SCE_GS_UV		0x03
#define SCE_GS_XYZF2		0x04
#define SCE_GS_XYZ2		0x05
//#define SCE_GS_XYZF		0x0a
#define SCE_GS_FOG		0x0a
#define SCE_GS_XYZF3		0x0c
#define SCE_GS_XYZ3		0x0d
#define SCE_GS_RGBAQ2		0x11
#define SCE_GS_ST2		0x12
#define SCE_GS_UV2		0x13
#define SCE_GS_XYOFFSET_1	0x18
#define SCE_GS_XYOFFSET_2	0x19
#define SCE_GS_PRMODECONT	0x1a

	/*-- drawing attribute reg. --*/
#define SCE_GS_PRMODE		0x1b
#define SCE_GS_TEX0_1		0x06
#define SCE_GS_TEX0_2		0x07
#define SCE_GS_TEX1_1		0x14
#define SCE_GS_TEX1_2		0x15
#define SCE_GS_TEX2_1		0x16
#define SCE_GS_TEX2_2		0x17
#define SCE_GS_TEXCLUT		0x1c
#define SCE_GS_SCANMSK		0x22
#define SCE_GS_MIPTBP1_1	0x34
#define SCE_GS_MIPTBP1_2	0x35
#define SCE_GS_MIPTBP2_1	0x36
#define SCE_GS_MIPTBP2_2	0x37
#define SCE_GS_CLAMP_1		0x08
#define SCE_GS_CLAMP_2		0x09
#define SCE_GS_TEXA		0x3b
#define SCE_GS_FOGCOL		0x3d
#define SCE_GS_TEXFLUSH		0x3f

	/*-- pixel operation reg. --*/
#define SCE_GS_SCISSOR_1	0x40
#define SCE_GS_SCISSOR_2	0x41
#define SCE_GS_ALPHA_1		0x42
#define SCE_GS_ALPHA_2		0x43
#define SCE_GS_DIMX		0x44
#define SCE_GS_DTHE		0x45
#define SCE_GS_COLCLAMP		0x46
#define SCE_GS_TEST_1		0x47
#define SCE_GS_TEST_2		0x48
#define SCE_GS_PABE		0x49
#define SCE_GS_FBA_1		0x4a
#define SCE_GS_FBA_2		0x4b

	/*-- buffer reg. --*/
#define SCE_GS_FRAME_1		0x4c
#define SCE_GS_FRAME_2		0x4d
#define SCE_GS_ZBUF_1		0x4e
#define SCE_GS_ZBUF_2		0x4f

	/*-- inter-buffer transfer reg. --*/
#define SCE_GS_BITBLTBUF	0x50
#define SCE_GS_TRXPOS		0x51
#define SCE_GS_TRXREG		0x52
#define SCE_GS_TRXDIR		0x53
#define SCE_GS_HWREG		0x54

	/*-- other reg. --*/
#define SCE_GS_SIGNAL		0x60
#define SCE_GS_FINISH		0x61
#define SCE_GS_LABEL		0x62
#define SCE_GS_NOP		0x7f






#ifdef __cplusplus
}
#endif


#endif


