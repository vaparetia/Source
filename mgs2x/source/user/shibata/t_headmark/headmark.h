/*
     headmark.h
     ヘッドマーク define
     2000/06/15/ T.Shibata
     
	 $Id: headmark.h,v 1.1.1.3 2002/11/19 11:48:52 Yoshizawa1 Exp $
*/
#ifndef HEADMARK_H
#define HEADMARK_H

#define		HMK2_TYPE_CALL	(0x80000000)					//コール
#define		HMK2_TYPE_KILL	(0x40000000 | HMK2_TYPE_CALL)	//!,?,Zzz などを殺す

#define		HMK2_TYPE_TRGT	(0x20000000)	//ターゲットあり(or して下さい)

#define		HMK2_TYPE_MARK	(0x8000)		//チェック用
#define		HMK2_TYPE_CHECK	(0x7fff)		//チェック用

#define		HMK2_TYPE_RED_AT	(0x80008000)	//! 赤		(前回  0)
#define		HMK2_TYPE_RED_QE	(0x80008001)	//?			(前回  1)
#define		HMK2_TYPE_YLW_AT	(0x80008002)	//! 黄		(前回  2)
#define		HMK2_TYPE_YLW_QE	(0x80008003)	//?			(前回  3)
#define		HMK2_TYPE_WHT_AT	(0x80008004)	//! 白		(前回  4)
#define		HMK2_TYPE_WHT_QE	(0x80008005)	//?			(前回  5)
#define		HMK2_TYPE_PNK_AT	(0x80008006)	//! 桃		(前回  6)

#define		HMK2_TYPE_LONG_AT	(0x8000800c)	//! ロング

#define		HMK2_TYPE_BREAK_AT	(0x8000800a)	//! 壊れ
#define		HMK2_TYPE_BREAK_QT	(0x8000800b)	//? 
#define		HMK2_TYPE_BREAK_AUTO	(0x8000800c)	//!? 壊れ判別

#define		HMK2_TYPE_RED_AT_MIN	(0x80008010)	//! 赤 小	
#define		HMK2_TYPE_RED_QE_MIN	(0x80008011)	//?		
#define		HMK2_TYPE_YLW_AT_MIN	(0x80008012)	//! 黄	
#define		HMK2_TYPE_YLW_QE_MIN	(0x80008013)	//?		
#define		HMK2_TYPE_WHT_AT_MIN	(0x80008014)	//! 白	
#define		HMK2_TYPE_WHT_QE_MIN	(0x80008015)	//?		
#define		HMK2_TYPE_PNK_AT_MIN	(0x80008016)	//! 桃	

#define		HMK2_TYPE_RED_AT_BIG	(0x80008020)	//! 赤 大
#define		HMK2_TYPE_RED_QE_BIG	(0x80008021)	//?		
#define		HMK2_TYPE_YLW_AT_BIG	(0x80008022)	//! 黄	
#define		HMK2_TYPE_YLW_QE_BIG	(0x80008023)	//?		
#define		HMK2_TYPE_WHT_AT_BIG	(0x80008024)	//! 白	
#define		HMK2_TYPE_WHT_QE_BIG	(0x80008025)	//?		
#define		HMK2_TYPE_PNK_AT_BIG	(0x80008026)	//! 桃

#define		HMK2_TYPE_ZZZ		(0x80000000)	//Zzz		(前回  6)
#define		HMK2_TYPE_PIYO		(0x80000001)	//ひよこ	(前回  7)
#define		HMK2_TYPE_PIYO_A	(0x80000002)	//Zzz		(前回  8)
#define		HMK2_TYPE_POWA		(0x80000003)	//放心		(前回  9)
#define		HMK2_TYPE_KYUPI		(0x80000004)	//キュピーン

#define		HMK2_TYPE_VALUE		(0x80000800)	//Piyo数指定 
                                                //Ｘ個にする時(0x8000080x)0->4	
                                                //ひよこ数指定 
                                                //Ｘ個にする時(0x8000080x)0->3 増加不可	
#define		HMK2_TYPE_PIYO_VALUE	(0x80001800)	//Zzz数指定 
                                                //Ｘ個にする時(0x8000080x)0->4	
                                                //ひよこ数指定 
                                                //Ｘ個にする時(0x8000080x)0->3 増加不可	
extern void  *NewControl_Headmark2( FMATRIX *world, int *type, TARGET *trg, CONTROL *cntrl );

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
