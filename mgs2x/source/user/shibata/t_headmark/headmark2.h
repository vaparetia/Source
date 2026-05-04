/*
     headmark2.h
     ヘッドマーク define
     2000/06/15/ T.Shibata
     
	 $Id: headmark2.h,v 1.1.1.3 2002/11/19 11:48:52 Yoshizawa1 Exp $
*/
#ifndef HEADMARK2_H
#define HEADMARK2_H

#define		HMK2_TYPE_KILL	(0x40000000)	//!,?,Zzz などを殺す

#define		HMK2_TYPE_TRGT	(0x20000000)	//ターゲットあり(or して下さい)

#define		HMK2_TYPE_MARK	(0x8000)		//チェック用
#define		HMK2_TYPE_CHECK	(0x7fff)		//チェック用

#define		HMK2_TYPE_RED_AT	(0x00008000)	//! 赤		(前回  0)
#define		HMK2_TYPE_RED_QE	(0x00008001)	//?			(前回  1)
#define		HMK2_TYPE_YLW_AT	(0x00008002)	//! 黄		(前回  2)
#define		HMK2_TYPE_YLW_QE	(0x00008003)	//?			(前回  3)
#define		HMK2_TYPE_WHT_AT	(0x00008004)	//! 白		(前回  4)
#define		HMK2_TYPE_WHT_QE	(0x00008005)	//?			(前回  5)
#define		HMK2_TYPE_PNK_AT	(0x00008006)	//! 桃		(前回  6)

#define		HMK2_TYPE_LONG_AT	(0x0000800c)	//! ロング

#define		HMK2_TYPE_BREAK_AT	(0x0000800a)	//! 壊れ
#define		HMK2_TYPE_BREAK_QT	(0x0000800b)	//? 
#define		HMK2_TYPE_BREAK_AUTO	(0x0000800c)	//!? 壊れ判別

#define		HMK2_TYPE_RED_AT_MIN	(0x00008010)	//! 赤 小	
#define		HMK2_TYPE_RED_QE_MIN	(0x00008011)	//?		
#define		HMK2_TYPE_YLW_AT_MIN	(0x00008012)	//! 黄	
#define		HMK2_TYPE_YLW_QE_MIN	(0x00008013)	//?		
#define		HMK2_TYPE_WHT_AT_MIN	(0x00008014)	//! 白	
#define		HMK2_TYPE_WHT_QE_MIN	(0x00008015)	//?		
#define		HMK2_TYPE_PNK_AT_MIN	(0x00008016)	//! 桃	

#define		HMK2_TYPE_RED_AT_BIG	(0x00008020)	//! 赤 大
#define		HMK2_TYPE_RED_QE_BIG	(0x00008021)	//?		
#define		HMK2_TYPE_YLW_AT_BIG	(0x00008022)	//! 黄	
#define		HMK2_TYPE_YLW_QE_BIG	(0x00008023)	//?		
#define		HMK2_TYPE_WHT_AT_BIG	(0x00008024)	//! 白	
#define		HMK2_TYPE_WHT_QE_BIG	(0x00008025)	//?		
#define		HMK2_TYPE_PNK_AT_BIG	(0x00008026)	//! 桃

#define		HMK2_TYPE_ZZZ		(0x00000000)	//Zzz		(前回  6)
#define		HMK2_TYPE_PIYO		(0x00000001)	//ひよこ	(前回  7)
#define		HMK2_TYPE_PIYO_A	(0x00000002)	//Zzz		(前回  8)
#define		HMK2_TYPE_POWA		(0x00000003)	//放心		(前回  9)
#define		HMK2_TYPE_KYUPI		(0x00000004)	//キュピーン

#define		HMK2_TYPE_VALUE		(0x00000800)	//Piyo数指定 
                                                //Ｘ個にする時(0x0000080x)0->4	
                                                //ひよこ数指定 
                                                //Ｘ個にする時(0x0000080x)0->3 増加不可	
#define		HMK2_TYPE_PIYO_VALUE	(0x00001800)	//Zzz数指定 
                                                //Ｘ個にする時(0x0000080x)0->4	
                                                //ひよこ数指定 
                                                //Ｘ個にする時(0x0000080x)0->3 増加不可	
extern void *NewControl_Headmark3( FMATRIX *world, TARGET *trg, CONTROL *cntrl );
extern void CallActHeadMarks( void *work, int type );
//一発君
/*
    HeadMarkRun(FMATRIX *world, int type );
	world	：頭のマトリクス
	type	：マークの種類
		0：中赤！ (+16 小) (+32 大)
		1：  赤？
		2：  黄！
		3：  黄？
		4：  白！
		5：  白？
		6：  桃！
	注意:小は今のところ監視カメラ用になちゃった。すんまそん。
*/
static inline void HeadMarkRun(FMATRIX *world, int type )
{
	extern void *AN_HeadMark( FMATRIX *world, int mark );

	AN_HeadMark( world, type );
	AN_HeadMark( world, type | 0x4000 );
}
#endif
