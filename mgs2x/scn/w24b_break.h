/*
  w24b_break.gcl
  シェル１中央棟Ｂ１

  2001/07/29 T.Morita
  $Id: w24b_break.h,v 1.5 2001/09/11 14:18:36 usr04098 Exp $

*/

#define BRK_BOTTLE   1
#define BRK_GLASSES  0
#define BRK_MAGAZINE 1

// 壊れ酒瓶用
#define BTL_COL1		0x00271313
#define BTL_COL2		0x00320303
#define BTL_COL3		0x00000f0a
#define BTL_COL4		0x000f0f1f
#define BTL_COL5		0x00060503
#define BTL_COL6		0x00031003
#define BTL_COL7		0x001a0413
#define BTL_COL8		0x00707070
#define BTL_NOWINE		0x01000000

proc 壊れ物設定
{
#if d:BRK_GLASSES
    @テーブルの上の壊れグラス
#endif
#if d:BRK_BOTTLE
    @壊れ瓶
#endif
#if d:BRK_MAGAZINE
    @壊れ雑誌
#endif
}

proc 壊れ瓶
{
	chara プット瓶オブジェ 酒瓶置場 \
		-s 4 \
		   d:BOTTLE_HZD_TUNNEL -6000,-3200,-68050, -6700,-2000,-69950, d:BOTTLE_TUNNEL_XAXIS  \
		   d:BOTTLE_HZD_BOX  -6800,-2000,-68000, -6700,-4500,-70000 \
		-l d:LT2_NAME \
		-n (9*2) \
		-m \
		   p_btl01,p_btl01_brk  p_btl01,p_btl01_brk \
		   p_btl02,p_btl02_brk  p_btl03,p_btl03_brk \
		   p_btl04,p_btl04_brk  p_btl05,p_btl05_brk \
		   p_btl06,p_btl06_brk  p_btl07,p_btl07_brk \
		   p_btl06,p_btl06_brk  \

		   p_btl07,p_btl07_brk  p_btl02,p_btl02_brk \
		   p_btl03,p_btl03_brk  p_btl04,p_btl04_brk \
		   p_btl05,p_btl05_brk  p_btl06,p_btl06_brk \
		   p_btl07,p_btl07_brk  p_btl06,p_btl06_brk \
		   p_btl07,p_btl07_brk  \

		-p \
		   0,1024,0 (-6200+74*2),-3200,(-69100-750+23), d:BTL_COL1 \
		   0,1024,0 (-6200+44*2),-3200,(-69100-600-10), d:BTL_COL1 \
		   0,1024,0 (-6200+36*2),-3200,(-69100-450+17), d:BTL_COL2 \
		   0,1024,0 (-6200+40*2),-3200,(-69100-300+26), d:BTL_COL3 \
		   0,1024,0 (-6200+22*2),-3200,(-69100-150-28), d:BTL_COL4 \
		   0,1024,0 (-6200+20*2),-3200,(-69100+  0+19), d:BTL_COL5 \
		   0,1024,0 (-6200-23*2),-3200,(-69100+150+9 ), d:BTL_COL6 \
		   0,1024,0 (-6200+30*2),-3200,(-69100+300-40), d:BTL_COL7 \
		   0,1024,0 (-6200-28*2),-3200,(-69100+450+10), d:BTL_COL8 \

		   0,1024,0 (-6450+29*2),-3200,(-69100-750-2 ), d:BTL_COL8 \
		   0,1024,0 (-6450-7 *2),-3200,(-69100-600+31), d:BTL_COL8 \
		   0,1024,0 (-6450+28*2),-3200,(-69100-450-14), d:BTL_COL8 \
		   0,1024,0 (-6450+13*2),-3200,(-69100-300+8 ), d:BTL_COL8 \
		   0,1024,0 (-6450+2 *2),-3200,(-69100-150+29), d:BTL_COL8 \
		   0,1024,0 (-6450-73*2),-3200,(-69100+  0-24), d:BTL_COL8 \
		   0,1024,0 (-6450+21*2),-3200,(-69100+150-37), d:BTL_COL8 \
		   0,1024,0 (-6450-2 *2),-3200,(-69100+300-6 ), d:BTL_COL8 \
		   0,1024,0 (-6450-64*2),-3200,(-69100+450-15), d:BTL_COL8 \
		-h (d:BRK_HZD_W24B_BOTTLE_KEEP)
//		-broken $f:半分壊れフラグ＿テーブル１ $f:壊れフラグ＿テーブル１ \
//		-proc グラスもう壊れているテーブル１ グラスもう半分壊れているテーブル１


	chara プット壊れオブジェ 逆さグラス中上段左 \
		-model gls_01 gls_01brk btl_piece \
		-param \
			(-6300-240),(-3200+135),(-68400+200) 0,0,0 \
			(-6300-240),(-3200+135),(-68400+ 80) 0,0,0 \
			(-6300-240),(-3200+135),(-68400- 40) 0,0,0 \
			(-6300-120),(-3200+135),(-68400+200) 0,0,0 \
	//		(-6300-120),(-3200+135),(-68400+ 80) 0,0,0 \
			(-6300-120),(-3200+135),(-68400- 40) 0,0,0 \
			(-6300+  0),(-3200+135),(-68400+200) 0,0,0 \
			(-6300+  0),(-3200+135),(-68400+ 80) 0,0,0 \
	//		(-6300+  0),(-3200+135),(-68400- 40) 0,0,0 \
			(-6300+120),(-3200+135),(-68400+200) 0,0,0 \
			(-6300+120),(-3200+135),(-68400+ 80) 0,0,0 \
			(-6300+120),(-3200+135),(-68400- 40) 0,0,0 \
//			(-6300+240),(-3200+135),(-68400+200) 0,0,0 \
			(-6300+240),(-3200+135),(-68400+ 80) 0,0,0 \
			(-6300+240),(-3200+135),(-68400- 40) 0,0,0 \
		-brk_se d:se_code:SD_A_BOTTLE02 \
		-drp_se d:se_code:SD_A_BOTTLE01 \
		-hazard d:BRK_HZD_W24B_BOTTLE_KEEP \
		-number (5*3-3) \
		-explode 10 10 \ // 最初は10 30
		-flag  (d:BRK_NO_GRAVITY | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG | d:BRK_STRICT_FLR)

}

proc テーブルの上のグラス
{
//		-s 2 \
//		   d:BOTTLE_HZD_BOX  -9850,-3250,-72425, -9000,-3350,-71575 \
//		-h (d:BRK_HZD_W24B_REFRESH)

	chara プット壊れオブジェ 逆さグラス中上段左 \
		-model gls_01 gls_01brk btl_piece \
		-param  (-56251-300),1425,(-100962+80) 0,0,0 \
			(-56251-300),1425,(-100962-40) 0,0,0 \
			(-56251-200),1425,(-100962+80) 0,0,0 \
			(-56251-200),1425,(-100962-40) 0,0,0 \
			(-56251-100),1425,(-100962+80) 0,0,0 \
			(-56251-100),1425,(-100962-40) 0,0,0 \
			(-56251+  0),1425,(-100962+80) 0,0,0 \
			(-56251+  0),1425,(-100962-40) 0,0,0 \
			(-56251+100),1425,(-100962+80) 0,0,0 \
			(-56251+100),1425,(-100962-40) 0,0,0 \
			(-56251+200),1425,(-100962+80) 0,0,0 \
			(-56251+200),1425,(-100962-40) 0,0,0 \
			(-56251+300),1425,(-100962+80) 0,0,0 \
		 	(-56251+300),1425,(-100962-40) 0,0,0 \
		-brk_se d:se_code:SD_A_BOTTLE02 \
		-drp_se d:se_code:SD_A_BOTTLE01 \
		-hazard (d:BRK_HZD_W24B_REFRESH) \
		-number 14 \
		-explode 10 10 \ // 最初は10 30
		-flag  (d:BRK_NO_GRAVITY | d:BRK_NO_BOXSHOW | d:BRK_NO_BOXTRG | d:BRK_STRICT_FLR)
}

proc 壊れ雑誌
{
	chara プット雑誌オブジェ エロ雑誌 \
			-number 14 \
			-break  book_naka book_hahen \
			-hazard (d:BRK_HZD_W24B_REFRESH) \
			-proc セット壊れフラグ＿エロ雑誌 \
			-model  plant_book_moto plant_book \
				-type 0 \
				-rot 240,-1024,0 \
				-pos (-10070-600),(720-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 1 \		    
				-rot 240,-900,0 \	    
				-pos (-10070-400),(720-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 2 \		    
				-rot 240,-900,0 \	    
				-pos (-10070-200),(720-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 3 \		    
				-rot 240,-900,0 \	    
				-pos (-10070+0  ),(720-3983),(-73940) \
				-light d:LT2_NAME \ // 真中
			-model  plant_book_moto plant_book \
				-type 4 /*-height 1*/ \	    
				-rot 240,-900,0 \	    
				-pos (-10070+200),(720-3983),(-73940) \
				-light d:LT2_NAME \	//エロ
			-model  plant_book_moto plant_book \
				-type 5 \		    
				-rot 240,-1024,0 \	    
				-pos (-10070+500),(720-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 6 \		    
				-rot 240,-1024,0 \	    
				-pos (-10070+750),(720-3983),(-73940) \
				-light d:LT2_NAME \

			-model  plant_book_moto plant_book \
				-type 6 \
				-rot 240,-1024,0 \
				-pos (-10070-600),(280-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 7 \		    
				-rot 240,-900,0 \	    
				-pos (-10070-400),(280-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 5 \		    
				-rot 240,-900,0 \	    
				-pos (-10070-200),(280-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 8 \
				-rot 240,-1024,0 \
				-pos (-10070+0  ),(280-3983),(-73940) \
				-light d:LT2_NAME \  // 真中
			-model  plant_book_moto plant_book \
				-type 0 \
				-rot 240,-900,0 \
				-pos (-10070+270),(280-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 3 \		    
				-rot 240,-1024,0 \	    
				-pos (-10070+500),(280-3983),(-73940) \
				-light d:LT2_NAME \
			-model  plant_book_moto plant_book \
				-type 2 \		    
				-rot 240,-1024,0 \	    
				-pos (-10070+750),(280-3983),(-73940) \
				-light d:LT2_NAME

	if ( $f:w24b_壊れフラグ＿エロ雑誌 )
	{
		mesg プット雑誌オブジェ エロ雑誌 もう壊れている
	}

}

proc セット壊れフラグ＿エロ雑誌
{
	eval ( $f:w24b_壊れフラグ＿エロ雑誌=1 ) ;
}
