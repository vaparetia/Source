//
// ローズ髪の毛つき設定サンプル
//
// $Id: cdc_face_sample_ros.h,v 1.2 2001/08/13 04:21:09 usr01363 Exp $
//

proc rto_顔設定ローズ髪の毛つき {
	face       0 ros_radio_mh_mt ローズ
	hair	   0 -n ros_hair_f_mh_mt -d 16   // ローズ髪の毛設定

	lightvec   0 30  -35  -40
	lightcol   0 160  187   2
	ambientcol 0 39   57    110
	facecamera 0 \
	  1000000  \   // far
	  11100    \   // zoom
	  -239     \   // heading
	  23625    \   // pan
	  84       \   // pitch
	  0            // gain
	facelimit 0 40000 512 512 113 170
	facedelay 0 75 75
}
