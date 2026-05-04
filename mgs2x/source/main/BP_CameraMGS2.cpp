///----------------------------------------------------------------------------
// BP_CameraMGS2.cpp
//----------------------------------------------------------------------------

///----------------------------------------------------------------------------
// INCLUDES
//----------------------------------------------------------------------------

#include "Engine/Stdafx.h"
#include "camera.h"

//----------------------------------------------------------------------------
// MGS2 CAMERA SETTINGS
//----------------------------------------------------------------------------

const SBP_CameraDef gBP_CameraDefs[] = 
{
// Cinemas - from Konami \110916_camera_adj_data\20110915_naka_MGS2.txt

//(p028_01_p01.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w11c" *****
   SBP_CameraDef( kCamDefV4, "w11c", 0x004fd5b7, 0x7d6f4b3b, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11c", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11c", 0x0d504872,       1680, kCSM_Wide_Variable, 0.11f, 0.00f, -1.00f ),
   SBP_CameraDef( kCamDefV4, "w11c", 0x0d504872,       1930, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w11c" *****

//(p040_09_p05a.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d036p03" *****
   SBP_CameraDef( kCamDefV4, "d036p03", 0xa1843f8a,       5142, kCSM_Wide_Variable, 0.50f, -0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d036p03", 0xa1843f8a,       5178, kCSM_Wide_Variable, 0.00f, -0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d036p03" *****

// JIRA MGSTWO-229:  REMAPPED!!
//(p046_01_p01.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d046p01" *****
   SBP_CameraDef( kCamDefV4, "d046p01", 0x448444b1,       2510, kCSM_Wide_Variable, 0.40f, 0.20f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d046p01", 0x448444b1,       2794, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d046p01", 0x448444b1,       5820, kCSM_Wide_Variable, 0.75f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d046p01", 0x448444b1,       6022, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d046p01", 0x448444b1,       8458, kCSM_Wide_Variable, 0.92f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d046p01", 0x448444b1,       8768, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d046p01" *****

//(p062_08_p04.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d053p01" *****
   SBP_CameraDef( kCamDefV4, "d053p01", 0x439df3ae,       1406, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d053p01" *****

//JIRA-MGSTWO-490 "d070p01"
//(p070_07_p06.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d070p01" *****
   SBP_CameraDef( kCamDefV4, "d070p01", 0xd7492805,       3386, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d070p01", 0xd7492805,       3836, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d070p01", 0xd7492805,       6508, kCSM_Wide_Variable, 0.90f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d070p01", 0xd7492805,       6904, kCSM_Wide_Variable, 0.25f, 0.00f, 0.11f ),
   SBP_CameraDef( kCamDefV4, "d070p01", 0xd7492805,       7084, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d070p01", 0xd7492805,       7450, kCSM_Wide_Variable, 0.45f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d070p01", 0xd7492805,       7800, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d070p01" *****

// JIRA MGSTWO-233:
//(p070_16_p10.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d070px9" *****
   SBP_CameraDef( kCamDefV4, "d070px9", 0x41f74c8e,       1200, kCSM_Wide_Variable, 0.50f, -0.73f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d070px9", 0x41f74c8e,       1560, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d070px9", 0x6bec717d,       4580, kCSM_Wide_Variable, 0.20f, -0.20f, -0.20f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d070px9" *****

//(p077_02_p01.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w45a" *****
   SBP_CameraDef( kCamDefV4, "w45a", 0x5b6658d5,        670, kCSM_Wide_Variable, 0.70f, 0.20f, -0.10f ),
   SBP_CameraDef( kCamDefV4, "w45a", 0x5b6658d5,        700, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w45a" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w45a, a45a" ***** 
   SBP_CameraDef( kCamDefV4, "w45a, a45a", 0x002038b0, 0x9210d4b9, kCSM_Wide_Variable, 1.00f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w45a, a45a", 0x0058bd37, 0x1f81828b, kCSM_Wide_Variable, 1.00f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w45a, a45a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 1.00f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w45a, a45a" *****

//JIRA MGSTWO-235
//(p080_07_p04.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w51a" *****
   SBP_CameraDef( kCamDefV4, "w51a", 0x162f5ac6,       2486, kCSM_Wide_Variable, 0.92f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w51a", 0x162f5ac6,       3424, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w51a", 0x8af9ae50,       3676, kCSM_Wide_Variable, 0.30f, -0.10f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w51a", 0x8af9ae50,       4306, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w51a", 0x8af9ae50,       5876, kCSM_Wide_Variable, 0.50f, -0.40f, -0.10f ),
   SBP_CameraDef( kCamDefV4, "w51a", 0x8af9ae50,       6096, kCSM_Wide_Variable, 0.00f, -0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w51a" *****

// JIRA MGSTWO-239, 350
//(p080_11_p06.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d080p06" *****
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,       1320, kCSM_Wide_Variable, 0.30f, 0.70f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,       1708, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,       3308, kCSM_Wide_Variable, 1.00f, -0.54f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,       3406, kCSM_Wide_Variable, 0.00f, -0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,       6832, kCSM_Wide_Variable, 0.50f, 0.50f, -0.20f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,       7008, kCSM_Wide_Variable, 0.00f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,       8322, kCSM_Wide_Variable, 1.00f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,       8906, kCSM_Wide_Variable, 0.00f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,      10197, kCSM_Wide_Variable, 0.80f, 0.20f, -0.20f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x6ce1ef84,      10257, kCSM_Wide_Variable, 0.00f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x7f77eeb2,       1570, kCSM_Wide_Variable, 0.72f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d080p06", 0x7f77eeb2,       1740, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d080p06" *****

//(p082_02_p02.sdt)
//(p082_04_p03.sdt)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d082p01" *****
   SBP_CameraDef( kCamDefV4, "d082p01", 0xa3158020,       3540, kCSM_Wide_Variable, 0.30f, 1.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xa3158020,       3770, kCSM_Wide_Variable, 0.30f, 1.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xa3158020,       4184, kCSM_Wide_Variable, 1.00f, -0.16f, -0.10f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xa3158020,       4528, kCSM_Wide_Variable, 0.10f, 0.20f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xa3158020,       4764, kCSM_Wide_Variable, 0.80f, 0.22f, -0.20f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xa3158020,       5168, kCSM_Wide_Variable, 0.00f, -0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,         -1, kCSM_Wide_Variable, 0.82f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,        766, kCSM_Wide_Variable, 0.82f, 0.00f, -0.20f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       1580, kCSM_Wide_Variable, 0.00f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       1810, kCSM_Wide_Variable, 0.80f, 0.00f, -0.40f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       2236, kCSM_Wide_Variable, 0.80f, 0.00f, -0.40f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       3114, kCSM_Wide_Variable, 0.80f, 0.00f, -0.60f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       3610, kCSM_Wide_Variable, 0.82f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       4390, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       5150, kCSM_Wide_Variable, 0.82f, 0.00f, -0.40f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       7036, kCSM_Wide_Variable, 0.80f, 0.00f, -0.20f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       7368, kCSM_Wide_Variable, 0.40f, 0.00f, -0.20f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       7500, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       7806, kCSM_Wide_Variable, 0.50f, 0.00f, -0.30f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       8824, kCSM_Wide_Variable, 0.90f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,       9928, kCSM_Wide_Variable, 0.70f, 0.00f, -0.70f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,      10046, kCSM_Wide_Variable, 0.82f, 0.00f, -0.60f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,      11150, kCSM_Wide_Variable, 0.90f, 0.00f, -0.60f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,      11592, kCSM_Wide_Variable, 1.00f, 0.30f, -0.50f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,      12022, kCSM_Wide_Variable, 0.00f, -0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,      12872, kCSM_Wide_Variable, 0.60f, -0.00f, -1.00f ),
   SBP_CameraDef( kCamDefV4, "d082p01", 0xed6fd01a,      13180, kCSM_Wide_Variable, 0.00f, -0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d082p01" *****

// Cinemas - from Konami \110916_camera_adj_data\20110915_tanaka_MGS2.txt

//t11a1D_1.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d11t" *****
SBP_CameraDef( kCamDefV4, "d11t", 0x6332b2b4,       1930, kCSM_Wide_Variable, 0.82f, 0.00f, -0.26f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d11t" *****

//t11a1D_2.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d11t" *****
SBP_CameraDef( kCamDefV4, "d11t", 0xe5a6c01a,       1930, kCSM_Wide_Variable, 0.82f, 0.00f, -0.40f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d11t" *****

//t11a1D_5.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d11t" *****
SBP_CameraDef( kCamDefV4, "d11t", 0xf8a3f0a2,       1930, kCSM_Wide_Variable, 0.82f, 0.00f, -0.40f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d11t" *****

//t12a1D.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d12t" *****
   SBP_CameraDef( kCamDefV4, "d12t", 0xedbdb557,       8018, kCSM_Wide_Variable, 0.81f, 0.00f, -0.22f ),
   SBP_CameraDef( kCamDefV4, "d12t", 0xedbdb557,       8278, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d12t" *****

//From M:\FromKP\Drop_10_7_11\MG_Staff-Cinematics\2011-10-07-KJP-DEMO-CORRECTION\mgs2\t13a1D.txt
//t13a1D.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d13t" *****
   SBP_CameraDef( kCamDefV4, "d13t", 0x2117b5c9,          0, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d13t", 0x2117b5c9,        720, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d13t", 0x2117b5c9,       4765, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d13t", 0x2117b5c9,       5274, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d13t", 0x2117b5c9,       6384, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d13t" *****

//t14a1D.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d14t" *****
   SBP_CameraDef( kCamDefV4, "d14t", 0x2bd2bcd0,          0, kCSM_Wide_Variable, 1.00f, 0.00f, -0.90f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d14t" *****


//From M:\FromKP\Drop_10_7_11\MG_Staff-Cinematics\2011-10-07-KJP-DEMO-CORRECTION\mgs2\t13a1D.txt
// JIRA MGSTWO-337
//p012_01_p01.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d012p01" *****
   SBP_CameraDef( kCamDefV4, "d012p01", 0x5aadccdd,       2468, kCSM_Wide_Variable, 0.50f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d012p01", 0x5aadccdd,       2674, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d012p01", 0x5aadccdd,       5528, kCSM_Wide_Variable, 1.00f, 0.40f, 0.07f ),
   SBP_CameraDef( kCamDefV4, "d012p01", 0x5aadccdd,       5624, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d012p01", 0x5aadccdd,       6874, kCSM_Wide_Variable, 1.00f, -0.60f, -0.70f ),
   SBP_CameraDef( kCamDefV4, "d012p01", 0x5aadccdd,       7042, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d012p01", 0x5aadccdd,       11186, kCSM_Wide_Variable, 0.70f, 0.00f, -0.50f ),
   SBP_CameraDef( kCamDefV4, "d012p01", 0x5aadccdd,       11338, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d012p01" *****

//p014_11_p04.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d014p01" *****
   SBP_CameraDef( kCamDefV4, "d014p01", 0x92b28d73,        910, kCSM_Wide_Variable, 0.37f, 0.00f, -0.30f ),
   SBP_CameraDef( kCamDefV4, "d014p01", 0x92b28d73,       1032, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d014p01" *****

//p021_01_p01.sdt
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d021p01" *****
   SBP_CameraDef( kCamDefV4, "d021p01", 0x90f972e3,          0, kCSM_Wide_Variable, 0.90f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d021p01", 0x90f972e3,        780, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d021p01" *****

// BP tweaks


// JIRA MGSTWO-554
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d00t" *****
   SBP_CameraDef( kCamDefV4, "d00t", 0x3e77d4c7,          9, kCSM_Wide_Variable, 0.70f, 1.00f, -0.80f ),
   SBP_CameraDef( kCamDefV4, "d00t", 0x3e77d4c7,       1202, kCSM_Wide_Variable, 0.00f, -0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d00t" *****

// JIRA MGSTWO-226,227
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d01t" *****
   SBP_CameraDef( kCamDefV4, "d01t", 0x7449a6f7,       2280, kCSM_Wide_Variable, 1.00f, 1.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d01t", 0x7449a6f7,       3329, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d01t", 0xfac6a114,       2778, kCSM_Wide_Variable, 1.00f, -1.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d01t", 0xfac6a114,       3262, kCSM_Wide_Variable, 0.00f, -0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d01t", 0xfac6a114,       4150, kCSM_Wide_Variable, 1.00f, 1.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d01t", 0xfac6a114,       4648, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d01t", 0xfac6a114,       6828, kCSM_Wide_Variable, 1.00f, -1.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d01t", 0xfac6a114,       7086, kCSM_Wide_Variable, 0.00f, -0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d01t" *****

// JIRA MGSTWO-232: REMAPPED!! plant/story/emma_event_end(w20a)
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "d005p03" *****
   SBP_CameraDef( kCamDefV4, "d005p03", 0x3590ce64,       2040, kCSM_Wide_Variable, 1.00f, -0.20f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "d005p03", 0x3590ce64,       2570, kCSM_Wide_Variable, 0.00f, -0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "d005p03" *****

//JIRA-MGSTWO-393,2191
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w01a" ***** 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00609a71, 0xb6ec3469, kCSM_Wide_Variable, 0.90f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x0088397f, 0x48f19b77, kCSM_Wide_Variable, 0.77f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x0088397f, 0xa645b4bd, kCSM_Wide_Variable, 0.68f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f272, 0x3980faad, kCSM_Wide_Variable, 0.51f, 0.00f, 0.70f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0x1c65a4ef, kCSM_Wide_Variable, 0.90f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0x4206bdae, kCSM_Wide_Variable, 0.64f, 0.00f, 0.59f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0x487dbd15, kCSM_Wide_Variable, 0.90f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0x58219a4c, kCSM_Wide_Variable, 0.84f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0x77ce8985, kCSM_Wide_Variable, 0.84f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0xac5a270e, kCSM_Wide_Variable, 0.90f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0xb416e6ea, kCSM_Wide_Variable, 0.90f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0xbbc3d9b1, kCSM_Wide_Variable, 0.49f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0xcf607baf, kCSM_Wide_Variable, 0.84f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0xd0cee69e, kCSM_Wide_Variable, 0.49f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0xd431575b, kCSM_Wide_Variable, 0.49f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0xf3d655b2, kCSM_Wide_Variable, 0.84f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01a, a01a, ta01a", 0x00f4f273, 0xfbb9b1d7, kCSM_Wide_Variable, 0.90f, 0.00f, 0.00f ), 
//****** MGS2 END CAMERA SETTINGS FOR AREA "w01a" ***** 

//JIRA-MGSTWO-391,2193
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w01b, a01b, ta01b" ***** 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 7840611, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00609a71, 0x2b351108, kCSM_Wide_Variable, 0.88f, 0.00f, 0.28f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00609a71, 0xf98b5a63, kCSM_Wide_Variable, 0.88f, 0.00f, 0.28f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00e7cb66, 0xfd6a7f72, kCSM_Wide_Variable, 0.88f, 0.00f, 0.28f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 1568122, kCSM_Wide_Variable, 0.16f, -0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0x097a3bbb, kCSM_Wide_Variable, 0.37f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0x4d7e391d, kCSM_Wide_Variable, 0.47f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0x893b6a11, kCSM_Wide_Variable, 0.47f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xb0f963a2, kCSM_Wide_Variable, 0.37f, -1.00f, 0.68f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xc68cb7ce, kCSM_Wide_Variable, 0.37f, 1.00f, 0.68f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xd20f0960, kCSM_Wide_Variable, 0.88f, 0.00f, 0.28f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xd97db772, kCSM_Wide_Variable, 0.47f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xdbc4e280, kCSM_Wide_Variable, 0.47f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xe693d00b, kCSM_Wide_Variable, 0.88f, 0.00f, 0.28f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xe822da5b, kCSM_Wide_Variable, 0.37f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xf40d7a63, kCSM_Wide_Variable, 0.88f, 0.00f, 0.28f ), 
   SBP_CameraDef( kCamDefV4, "w01b, a01b, ta01b", 0x00f4f273, 0xf90acc9c, kCSM_Wide_Variable, 0.47f, 0.00f, -0.00f ), 
//****** MGS2 END CAMERA SETTINGS FOR AREA "w01b, a01b, ta01b" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w01c, a01c, ta01c" ***** 
   SBP_CameraDef( kCamDefV4, "w01c, a01c, ta01c", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.48f, 0.00f, 0.60f ), 
   SBP_CameraDef( kCamDefV4, "w01c, a01c, ta01c", 0x00e1e293, 0xc26485eb, kCSM_Wide_Variable, 0.48f, 0.00f, 0.60f ), 
   SBP_CameraDef( kCamDefV4, "w01c, a01c, ta01c", 0x00e1e292, 0x02624c7e, kCSM_Wide_Variable, 0.67f, 0.96f, -0.44f ), 
   SBP_CameraDef( kCamDefV4, "w01c, a01c, ta01c", 0x00e1e292, 0x175952fa, kCSM_Wide_Variable, 0.48f, 0.00f, 0.60f ), 
   SBP_CameraDef( kCamDefV4, "w01c, a01c, ta01c", 0x00e1e292, 0x272d6e45, kCSM_Wide_Variable, 0.67f, -0.96f, -0.44f ), 
   SBP_CameraDef( kCamDefV4, "w01c, a01c, ta01c", 0x00e1e292, 0x27ca14dc, kCSM_Wide_Variable, 0.48f, 0.00f, 0.60f ), 
   SBP_CameraDef( kCamDefV4, "w01c, a01c, ta01c", 0x00e1e294, 0xe97be0b5, kCSM_Wide_Variable, 0.67f, 0.96f, -0.44f ), 
//****** MGS2 END CAMERA SETTINGS FOR AREA "w01c, a01c, ta01c" *****

//JIRA-MGSTWO-390, 1819
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w01d" ***** 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0xa0ab28e0, kCSM_Wide_Variable, 1.00f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e293, 0xce4aff06, kCSM_Wide_Variable, 1.00f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00660aa7, 0x00000000, kCSM_Wide_Variable, 0.07f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x0088397f, 0x5af4b391, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00b5bfa1, 0x00000000, kCSM_Wide_Variable, 0.34f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0x1786ea1c, kCSM_Wide_Variable, 0.79f, 0.00f, -0.10f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0x1eb8ee79, kCSM_Wide_Variable, 0.93f, 0.00f, -0.84f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0x561e0615, kCSM_Wide_Variable, 0.41f, 1.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0x9a375ad0, kCSM_Wide_Variable, 0.90f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0xb93dfd1f, kCSM_Wide_Variable, 0.93f, 0.00f, -0.84f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0xc11b605f, kCSM_Wide_Variable, 0.60f, 0.00f, 0.85f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0xc866a5a0, kCSM_Wide_Variable, 0.93f, 0.00f, -0.84f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0xf11bba52, kCSM_Wide_Variable, 0.93f, 0.00f, -0.84f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0xf3fb5b0e, kCSM_Wide_Variable, 1.00f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e292, 0xfe4e466f, kCSM_Wide_Variable, 1.00f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e293, 0x3f396e4f, kCSM_Wide_Variable, 0.20f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e293, 0x4291a882, kCSM_Wide_Variable, 0.75f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e293, 0x4bdc42d2, kCSM_Wide_Variable, 0.79f, -0.00f, -0.10f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e293, 0x89549a97, kCSM_Wide_Variable, 0.63f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e293, 0xf7d733c5, kCSM_Wide_Variable, 0.06f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e294, 0xb7ce7c1a, kCSM_Wide_Variable, 0.27f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00e1e295, 0x62466750, kCSM_Wide_Variable, 0.34f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01d, a01d, ta01d", 0x00f5fb54, 0x32eadd5f, kCSM_Wide_Variable, 0.67f, 0.00f, 1.00f ), 
//****** MGS2 END CAMERA SETTINGS FOR AREA "w01d" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w01e, a01e, ta01e" *****
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e293, 12501230, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e294, 12501230, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e292, 0x3c8ed020, kCSM_Wide_Variable, 0.90f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e293,    2500246, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e293, 0x3eb773a3, kCSM_Wide_Variable, 0.46f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e293, 0x9a017358, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e293, 0xff731ff8, kCSM_Wide_Variable, 0.46f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e294,    2500246, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0x00e1e294, 0xff731ff8, kCSM_Wide_Variable, 0.46f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w01e, a01e, ta01e", 0xe5e29a83,        123, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w01e, a01e, ta01e" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w01f" ***** 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00e7cb66, 0x8e57ba06, kCSM_Wide_Variable, 0.42f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f272, 0x18171b4e, kCSM_Wide_Variable, 0.42f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f273, 0xa778bad4, kCSM_Wide_Variable, 0.42f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00609a71, 0x6ea3b8ac, kCSM_Wide_Variable, 0.77f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00609a71, 0xf6699494, kCSM_Wide_Variable, 0.00f, 0.00f, -0.01f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00e7cb66, 0x519ca7f5, kCSM_Wide_Variable, 0.77f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00e7cb66, 0xa4d19aa9, kCSM_Wide_Variable, 0.77f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f272, 0x1b5fdbe0, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f272, 0x596dbe91, kCSM_Wide_Variable, 0.12f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f272, 0x721e5521, kCSM_Wide_Variable, 0.48f, 0.02f, 0.23f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f272, 0x7e667446, kCSM_Wide_Variable, 0.42f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f272, 0xb7f111ef, kCSM_Wide_Variable, 0.42f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f272, 0xce5eff34, kCSM_Wide_Variable, 0.48f, 0.00f, 0.23f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f273, 0x4566ddbd, kCSM_Wide_Variable, 0.63f, -1.00f, -1.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f273, 0x767f96da, kCSM_Wide_Variable, 0.42f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f273, 0xafbabee9, kCSM_Wide_Variable, 0.77f, 0.00f, -0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f273, 0xe1117cfb, kCSM_Wide_Variable, 0.35f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f273, 0xed6b1f47, kCSM_Wide_Variable, 0.35f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w01f, a01f, ta01f", 0x00f4f273, 0xfea98d36, kCSM_Wide_Variable, 0.77f, 0.00f, -0.00f ), 
//****** MGS2 END CAMERA SETTINGS FOR AREA "w01f" *****

//JIRA: MGSTWO-392, KDEUS-61
///****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w02a, a02a, ta02a, a02b" ***** 
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x00660aa7, 0x40a19b35, kCSM_Wide_Variable, 0.06f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x00660aa7, 0x616619a8, kCSM_Wide_Variable, 0.06f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x00660aa7, 0xabac0d3b, kCSM_Wide_Variable, 0.06f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x00660aa7, 0xb4eebf26, kCSM_Wide_Variable, 0.06f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x002038b0, 0x853073cb, kCSM_Wide_Variable, 1.00f, 0.04f, 0.10f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x002038b0, 0xa2cd3c0a, kCSM_Wide_Variable, 0.55f, 0.00f, 0.40f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x002038b0, 0xdccbbc84, kCSM_Wide_Variable, 0.55f, 0.00f, 0.40f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x002db0af, 0x54640cef, kCSM_Wide_Variable, 0.06f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x002db0b0, 0xd12b31f5, kCSM_Wide_Variable, 0.06f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x00f4f273,      51352, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w02a, a02a, ta02a, a02b", 0x00f4f273, 0x162a5cb9, kCSM_Wide_Variable, 0.06f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w02a, a02a, ta02a, a02b" *****

//JIRA-MGSTWO-394
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w03a, a03a" ***** 
   SBP_CameraDef( kCamDefV4, "w03a, a03a", 0x00609a71, 0x4bda5558, kCSM_Wide_Variable, 0.77f, 0.00f, 0.56f ),
   SBP_CameraDef( kCamDefV4, "w03a, a03a", 0x00609a71, 0x5bffc77f, kCSM_Wide_Variable, 0.77f, 0.00f, 0.56f ),
   SBP_CameraDef( kCamDefV4, "w03a, a03a", 0x00e7cb66, 0x28030f96, kCSM_Wide_Variable, 0.77f, 0.00f, 0.56f ),
   SBP_CameraDef( kCamDefV4, "w03a, a03a", 0x00e93854, 0x4bcf9b41, kCSM_Wide_Variable, 0.77f, 0.00f, 0.56f ),
   SBP_CameraDef( kCamDefV4, "w03a, a03a", 0x00e93854, 0x748bde24, kCSM_Wide_Variable, 0.80f, 0.00f, -0.79f ),
   SBP_CameraDef( kCamDefV4, "w03a, a03a", 0x00e93854, 0x9937c3e5, kCSM_Wide_Variable, 0.80f, 0.00f, -0.79f ),
   SBP_CameraDef( kCamDefV4, "w03a, a03a", 0x00e93854, 0xce63e8ea, kCSM_Wide_Variable, 0.82f, 0.00f, -0.79f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w03a, a03a" *****

//JIRA-MGSTWO-336
//JIRA-MGSTWO-395
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w03b, a03b" ***** 
   SBP_CameraDef( kCamDefV4, "w03b, a03b", 0x00609a71, 0x0179a9e4, kCSM_Wide_Variable, 1.00f, -0.13f, 0.85f ),
   SBP_CameraDef( kCamDefV4, "w03b, a03b", 0x00609a71, 0x0c590b7b, kCSM_Wide_Variable, 1.00f, -0.13f, 0.85f ),
   SBP_CameraDef( kCamDefV4, "w03b, a03b", 0x00609a71, 0x76eeedc8, kCSM_Wide_Variable, 0.41f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w03b, a03b", 0x00e7cb66, 0xf5bb3fac, kCSM_Wide_Variable, 1.00f, -0.13f, 0.85f ),
   SBP_CameraDef( kCamDefV4, "w03b, a03b", 0x00e93854, 0xb9c1e8e8, kCSM_Wide_Variable, 0.41f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w03b, a03b", 0x00fb8776, 0xf44b9d6b, kCSM_Wide_Variable, 1.00f, -0.13f, 0.85f ),
   SBP_CameraDef( kCamDefV4, "w03b, a03b", 0xa7acbd42,       2640, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w03b, a03b", 0xa7acbd42,       3072, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w03b, a03b" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w04a, a04a" *****
   SBP_CameraDef( kCamDefV4, "w04a, a04a", 0x003d568c, 0x212bcac2, kCSM_Wide_Variable, 0.65f, 0.00f, 0.08f ),
   SBP_CameraDef( kCamDefV4, "w04a, a04a", 0x00e1e292, 0xb75ecd3c, kCSM_Wide_Variable, 0.65f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w04a, a04a", 0x00f5fb54, 0x82763fdd, kCSM_Wide_Variable, 0.36f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w04a, a04a", 0x00f5fb54, 0xd75c85ae, kCSM_Wide_Variable, 0.25f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w04a, a04a" *****

//JIRA-MGSTWO-479
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w04b, a04b" *****
   SBP_CameraDef( kCamDefV4, "w04b, a04b", 0x00e1e293, 0xe145799e, kCSM_Wide_Variable, 0.71f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w04b, a04b", 0x00f5fb54, 0x82763fdd, kCSM_Wide_Variable, 0.35f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w04b, a04b", 0x00f5fb54, 0xbe0cf956, kCSM_Wide_Variable, 0.36f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w04b, a04b" *****

//JIRA-MGSTWO-424 "w11a"
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w11a" *****
   SBP_CameraDef( kCamDefV4, "w11a", 0x004fd5b7, 0x4c39f47d, kCSM_Wide_Variable, 0.21f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x004fd5b7, 0x67e2be1b, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x004fd5b7, 0x6fadfe6a, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x004fd5b7, 0xa7093883, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x004fd5b7, 0xa7111927, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x004fd5b7, 0xbd0d546c, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x004fd5b7, 0xe218d205, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x004fd5b7, 0xf32c6a33, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x0d7e9275, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x3fddec71, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x68a297a9, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x72a1332f, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x751d198a, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x7a9c9530, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x7e02c5da, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x96f78e41, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x9c6d889d, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0x9dba9efb, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00607873, 0xbe2bec95, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0x026c8279, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0x413290e3, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0x6efc787a, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0x79d0ed35, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0x8b8683ee, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0x8ef3da5c, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0xb06d39f2, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0xb6009b66, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0xd62674cb, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00609a71, 0xfa4d825d, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11a", 0x00e1e293, 0x25206fdb, kCSM_Wide_Variable, 0.48f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w11a" *****

//JIRA-MGSTWO-423 "w11b"
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w11b" *****
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0x25206fdb, kCSM_Wide_Variable, 0.48f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0x4c39f47d, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0x67e2be1b, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0x6fadfe6a, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0xa7093883, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0xa7111927, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0xbd0d546c, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0xc0bb5c5a, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x004fd5b7, 0xe218d205, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0x0d7e9275, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0x68a297a9, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0x751d198a, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0x7a9c9530, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0x86ded96c, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0x96f78e41, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0x9c6d889d, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0x9dba9efb, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0xbe2bec95, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00607873, 0xf893e53f, kCSM_Wide_Variable, 0.64f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0x413290e3, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0x6efc787a, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0x79d0ed35, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0x8b8683ee, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0x8ef3da5c, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0xb06d39f2, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0xb6009b66, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0xd62674cb, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00609a71, 0xfa4d825d, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.91f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w11b", 0x00e1e293, 0x25206fdb, kCSM_Wide_Variable, 0.48f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w11b" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w12b, a12b" *****
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x0088397f, 0x8d7be24a, kCSM_Wide_Variable, 0.40f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00609a71, 0x04dc5f62, kCSM_Wide_Variable, 1.00f, 0.00f, -1.00f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00609a71, 0xa08d87b5, kCSM_Wide_Variable, 1.00f, 0.00f, -1.00f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00609a71, 0xb2526f37, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00609a71, 0xf2c7b69d, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00b5bfa1, 0, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e292, 0x416b9547, kCSM_Wide_Variable, 1.00f, 0.00f, -1.00f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e292, 0x5cf21090, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e292, 0xab5e6335, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e292, 0xb4e70d72, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e292, 0xd4cb1f54, kCSM_Wide_Variable, 1.00f, 0.00f, -1.00f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e292, 0xe693d00b, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0x065dabb5, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0x1ddcad34, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0x2176783f, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0x243701ff, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0x43a36025, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0x449157a8, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0x81d180b6, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0xc40194e9, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0xd3e634d9, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0xde4d40b6, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
   SBP_CameraDef( kCamDefV4, "w12b, a12b", 0x00e1e293, 0xebb8f836, kCSM_Wide_Variable, 0.40f, 0.00f, 0.50f ), 
//****** MGS2 END CAMERA SETTINGS FOR AREA "w12b, a12b" *****

//JIRA: MGSTWO-413, KDEUS-61
   //****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w13a, a13c" *****
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x0060ccf7, 0x7aa074db, kCSM_Wide_Variable, 0.40f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x002038b0, 0xb66f93a5, kCSM_Wide_Variable, 0.18f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x002038b0, 0xe86224ab, kCSM_Wide_Variable, 1.00f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x002038b0, 0xfdcc13fe, kCSM_Wide_Variable, 0.50f, 0.00f, -0.40f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x0058bd37, 0x87732f59, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x00930be1, 0x5662ebd3, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x00930be1, 0xb4e332f3, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x00930be1, 0xdaf74d21, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x00e7cb66, 0x8a41c1ad, kCSM_Wide_Variable, 1.00f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x00e7cb66, 0xccde38f2, kCSM_Wide_Variable, 0.18f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x00f5fb54, 0x4a39f02c, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w13a, a13c", 0x00f5fb54, 0x6b543db4, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w13a, a13c" *****

//JIRA-MGSTWO-410
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w13b, a13b" ***** 
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00660aa7, 0xad936cb5, kCSM_Wide_Variable, 0.18f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00660aa7, 0xf17da9ee, kCSM_Wide_Variable, 0.18f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x002038b0, 0xb66f93a5, kCSM_Wide_Variable, 0.18f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x002038b0, 0xe86224ab, kCSM_Wide_Variable, 1.00f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x002038b0, 0xfdcc13fe, kCSM_Wide_Variable, 0.50f, 0.00f, -0.40f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x0058bd37, 0x87732f59, kCSM_Wide_Variable, 0.61f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00930be1, 0x3b53f19b, kCSM_Wide_Variable, 0.61f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00930be1, 0x5662ebd3, kCSM_Wide_Variable, 0.61f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00930be1, 0xb4e332f3, kCSM_Wide_Variable, 0.61f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00930be1, 0xdaf74d21, kCSM_Wide_Variable, 0.61f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.61f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00e7cb66, 0x8a41c1ad, kCSM_Wide_Variable, 1.00f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00e7cb66, 0xccde38f2, kCSM_Wide_Variable, 0.18f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00f5fb54, 0x4a39f02c, kCSM_Wide_Variable, 0.61f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w13b, a13b", 0x00f5fb54, 0x6b543db4, kCSM_Wide_Variable, 0.61f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w13b, a13b" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w14a, a14a" *****
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00660aa7, 0x1f294039, kCSM_Wide_Variable, 0.90f, 0.00f, 0.18f ),
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00660aa7, 0x24d5b08c, kCSM_Wide_Variable, 0.90f, 0.00f, 0.18f ),
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00660aa7, 0x3b970291, kCSM_Wide_Variable, 0.90f, 0.00f, 0.18f ),
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00660aa7, 0x4b0bd689, kCSM_Wide_Variable, 0.90f, 0.00f, 0.18f ),
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.90f, 0.00f, 0.18f ),
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00e1e292, 0x6eb0bd50, kCSM_Wide_Variable, 0.90f, 0.00f, 0.18f ),
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00e1e292, 0x9924d6f7, kCSM_Wide_Variable, 0.90f, 0.00f, 0.18f ),
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00e1e293, 0x7fa1289f, kCSM_Wide_Variable, 0.25f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w14a, a14a", 0x00e1e294, 0x736dbd3e, kCSM_Wide_Variable, 0.13f, 0.00f, -0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w14a, a14a" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w15a" *****
   SBP_CameraDef( kCamDefV4, "w15a", 0x002038b0, 0x9395bf2d, kCSM_Wide_Variable, 0.71f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w15a", 0x002038b0, 0xa09eb93f, kCSM_Wide_Variable, 0.49f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w15a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.49f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w15a", 0x00e7cb66, 0xe2399a89, kCSM_Wide_Variable, 0.71f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w15a" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w15b, a15b" ***** 
   SBP_CameraDef( kCamDefV4, "w15b, a15b", 0x00660aa7, 0x30fe8b6b, kCSM_Wide_Variable, 0.46f, 0.00f, 0.53f ),
   SBP_CameraDef( kCamDefV4, "w15b, a15b", 0x00660aa7, 0xe048e3c3, kCSM_Wide_Variable, 0.46f, 0.00f, 0.53f ),
   SBP_CameraDef( kCamDefV4, "w15b, a15b", 0x002038b0, 0x9395bf2d, kCSM_Wide_Variable, 0.69f, 0.00f, 0.38f ),
   SBP_CameraDef( kCamDefV4, "w15b, a15b", 0x002038b0, 0xa09eb93f, kCSM_Wide_Variable, 0.46f, 0.00f, 0.53f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w15b, a15b" *****

//JIRA: MGSTWO-396, KDEUS-61, KDJP-123
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w16a, a16a" *****
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00609a71, 0x6d518dcd, kCSM_Wide_Variable, 0.92f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.84f, 0.00f, -0.80f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00e1e292, 0x13bbd491, kCSM_Wide_Variable, 0.92f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00e1e292, 0x2d83f170, kCSM_Wide_Variable, 0.92f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00e1e292, 0xb372a7fd, kCSM_Wide_Variable, 0.94f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00e1e292, 0xb4e70d72, kCSM_Wide_Variable, 0.77f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00e1e292, 0xd68bca43, kCSM_Wide_Variable, 0.77f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00e1e293, 0x91af473d, kCSM_Wide_Variable, 0.84f, 0.00f, -0.80f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00e1e293, 0xc451819d, kCSM_Wide_Variable, 0.84f, 0.00f, -0.80f ),
   SBP_CameraDef( kCamDefV4, "w16a, a16a", 0x00e1e293, 0xddc71edd, kCSM_Wide_Variable, 0.94f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w16a, a16a" *****

//JIRA-MGSTWO-398 / 2097
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w16b" ***** 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00609a71, 0x6d518dcd, kCSM_Wide_Variable, 0.92f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00609a71, 0x72f40e37, kCSM_Wide_Variable, 0.84f, 0.00f, 0.21f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.84f, 0.00f, -0.80f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e292, 0x13bbd491, kCSM_Wide_Variable, 0.92f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e292, 0x2d83f170, kCSM_Wide_Variable, 0.92f, 0.00f, 0.80f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e292, 0xb372a7fd, kCSM_Wide_Variable, 0.94f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e292, 0xb4e70d72, kCSM_Wide_Variable, 0.82f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e292, 0xd68bca43, kCSM_Wide_Variable, 0.84f, 0.00f, 0.21f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e293, 0x063f3a19, kCSM_Wide_Variable, 0.94f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e293, 0x91af473d, kCSM_Wide_Variable, 0.84f, 0.00f, -0.80f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e293, 0xc14b29b4, kCSM_Wide_Variable, 0.70f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e293, 0xc451819d, kCSM_Wide_Variable, 0.84f, 0.00f, -0.80f ), 
   SBP_CameraDef( kCamDefV4, "w16b", 0x00e1e293, 0xddc71edd, kCSM_Wide_Variable, 0.80f, 0.00f, 1.00f ), 
//****** MGS2 END CAMERA SETTINGS FOR AREA "w16b" *****

//JIRA-MGSTWO-409
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w17a, a17a" *****
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x002038b0, 0x6700b3fc, kCSM_Wide_Variable, 0.25f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x002038b0, 0xb1b09e1a, kCSM_Wide_Variable, 0.46f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x0058bd37, 0xf289e9e7, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x00930be1, 0x7dc572b7, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x00930be1, 0x926e29c9, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x00930be1, 0xa3888a66, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x00930be1, 0xd04b2eb4, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x00f5fb54, 0x9bc00008, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x00f5fb54, 0xb8f03077, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w17a, a17a", 0x00f5fb54, 0xf856fa00, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w17a, a17a" *****

//JIRA-MGSTWO-401
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w18a, a18a" *****
   SBP_CameraDef( kCamDefV4, "w18a, a18a", 0x003d568c,    1664148, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w18a, a18a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.67f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w18a, a18a", 0x00e43af7, 0x58926864, kCSM_Wide_Variable, 0.67f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w18a, a18a", 0x00f5fb54, 0x881dfeab, kCSM_Wide_Variable, 0.67f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w18a, a18a", 0x00f5fb54, 0xf29ebd9d, kCSM_Wide_Variable, 0.67f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w18a, a18a" *****

//JIRA-MGSTWO-402
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w19a, a19a" *****
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x002038b0, 0x41160b63, kCSM_Wide_Variable, 0.69f, 0.00f, 0.96f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x002038b0, 0x44b1462f, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x002038b0, 0x9ed74c53, kCSM_Wide_Variable, 0.55f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x002038b0, 0xdb47e6bf, kCSM_Wide_Variable, 0.13f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x0058bd37, 0x87732f59, kCSM_Wide_Variable, 0.40f, -0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0x0bcf16a2, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0x2efec563, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0x433d8c9d, kCSM_Wide_Variable, 0.40f, -0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0x5647ba28, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0x6c4de9e5, kCSM_Wide_Variable, 0.40f, -0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0x8fa801fd, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0xa24e0e33, kCSM_Wide_Variable, 0.40f, -0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0xb57bcb6f, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0xbf911623, kCSM_Wide_Variable, 0.40f, -0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00930be1, 0xcd8d0680, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00e7cb66, 0xc161b132, kCSM_Wide_Variable, 0.13f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00e7cb66, 0xed4f2546, kCSM_Wide_Variable, 0.56f, 0.00f, 0.96f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00f5fb54, 0x2e57d4c8, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00f5fb54, 0x3c7dea41, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00f5fb54, 0x5dba0a3f, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00f5fb54, 0x64c653ae, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00f5fb54, 0xa5549289, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00f5fb54, 0xde03e63a, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w19a, a19a", 0x00f5fb54, 0xe005bd53, kCSM_Wide_Variable, 0.40f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w19a, a19a" *****

//JIRA-MGSTWO-404
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w20a, a20a, a20e, ta20a" ***** 
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00660aa7, 0x2b669b48, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00660aa7, 0xbf3fd593, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x004fd5b7, 0x18982307, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x004fd5b7, 0x22d1dee9, kCSM_Wide_Variable, 0.36f, 1.00f, 0.40f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x004fd5b7, 0x4100f00b, kCSM_Wide_Variable, 0.83f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x004fd5b7, 0xb07f147a, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x004fd5b7, 0xc90daa32, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x004fd5b7, 0xf99c1deb, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x004fd5b7, 0xfca103d1, kCSM_Wide_Variable, 0.83f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x004fd5b7, 0xfde17975, kCSM_Wide_Variable, 0.83f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00609a71, 0x218ce344, kCSM_Wide_Variable, 0.83f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00609a71, 0x6987fa33, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00609a71, 0xdbd7515b, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.83f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00e1e292, 0x5667a789, kCSM_Wide_Variable, 0.11f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00e1e292, 0x577ff4dc, kCSM_Wide_Variable, 0.29f, 1.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20a, a20a, a20e, ta20a", 0x00e1e292, 0x82ee12f1, kCSM_Wide_Variable, 0.83f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w20a, a20a, a20e, ta20a" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w20b, a20b" ***** 
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00660aa7, 0x05690069, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00660aa7, 0x2e2e1dd7, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00609a71, 0x5eb69a15, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.59f, -0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00e1e292, 0xb612b366, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00e1e292, 0xcfba415f, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00e1e293, 0x02e8be6a, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00e1e293, 0x14f0a1b6, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00e1e293, 0xbd0dc577, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00e1e293, 0xdfba8bc2, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20b, a20b", 0x00e1e294, 0x0e30bb14, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w20b, a20b" *****

//JIRA-MGSTWO-422
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w20c" *****
   SBP_CameraDef( kCamDefV4, "w20c", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20c", 0x00e1e292, 0xb612b366, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20c", 0x00e1e292, 0xcfba415f, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20c", 0x00e1e293, 0x02e8be6a, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20c", 0x00e1e293, 0x14f0a1b6, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20c", 0x00e1e293, 0xbd0dc577, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20c", 0x00e1e293, 0xdfba8bc2, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w20c" *****

//JIRA-MGSTWO-414
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w20d" *****
   SBP_CameraDef( kCamDefV4, "w20d", 0x00609a71, 0x5eb69a15, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20d", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20d", 0x00e1e292, 0xb612b366, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20d", 0x00e1e292, 0xcfba415f, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20d", 0x00e1e293, 0x02e8be6a, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20d", 0x00e1e293, 0x14f0a1b6, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20d", 0x00e1e293, 0xbd0dc577, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w20d", 0x00e1e293, 0xdfba8bc2, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w20d" *****

//JIRA-MGSTWO-406
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w21a, a21a" *****
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x002038b0, 0x10f6cc6b, kCSM_Wide_Variable, 0.53f, 0.00f, 0.30f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x002038b0, 0x3f2dffbd, kCSM_Wide_Variable, 0.32f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x002038b0, 0xe85c8e24, kCSM_Wide_Variable, 0.49f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x0058bd37, 0x6d5eddd4, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00930be1, 0x1d02131c, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00930be1, 0x51a2f6d0, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00930be1, 0x8a6ac7c8, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00930be1, 0xb4f39596, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00930be1, 0xce90f13a, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00e7cb66, 0xc7e106e5, kCSM_Wide_Variable, 0.49f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00f5fb54, 0x3b0e391e, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00f5fb54, 0x4fd66312, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00f5fb54, 0xe92ebd6f, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21a, a21a", 0x00f5fb54, 0xf8fe239d, kCSM_Wide_Variable, 0.12f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w21a, a21a" *****

//JIRA-MGSTWO-419
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w21b" *****
   SBP_CameraDef( kCamDefV4, "w21b", 0x002038b0, 0x10f6cc6b, kCSM_Wide_Variable, 0.59f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x002038b0, 0x3f2dffbd, kCSM_Wide_Variable, 0.32f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x0058bd37, 0x6d5eddd4, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00930be1, 0x1d02131c, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00930be1, 0x51a2f6d0, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00930be1, 0x8a6ac7c8, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00930be1, 0xb4f39596, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00930be1, 0xce90f13a, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00e7cb66, 0xc7e106e5, kCSM_Wide_Variable, 0.44f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00f5fb54, 0x3b0e391e, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00f5fb54, 0x4fd66312, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00f5fb54, 0xe92ebd6f, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w21b", 0x00f5fb54, 0xf8fe239d, kCSM_Wide_Variable, 0.10f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w21b" *****

//JIRA-MGSTWO-408
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w22a, a22a, a22b, ta22a" ***** 
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00660aa7, 0x5385eea1, kCSM_Wide_Variable, 0.67f, 0.00f, -0.02f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00660aa7, 0x6230b921, kCSM_Wide_Variable, 0.67f, 0.00f, -0.02f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00660aa7, 0xc6b5e0ff, kCSM_Wide_Variable, 0.67f, 0.00f, -0.02f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00660aa7, 0xc814acb7, kCSM_Wide_Variable, 0.67f, 0.00f, -0.02f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x004fd5b7, 0x7697967a, kCSM_Wide_Variable, 0.67f, 0.00f, -0.02f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x004fd5b7, 0xc9289473, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x004fd5b7, 0xfb9f2fe0, kCSM_Wide_Variable, 0.70f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x223e2969, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x4dfee77e, kCSM_Wide_Variable, 0.81f, 0.00f, 0.25f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x4e202bb3, kCSM_Wide_Variable, 0.70f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x5c408ae5, kCSM_Wide_Variable, 0.81f, 0.00f, 0.77f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x5e849023, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x726a9da8, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x76cc3ba1, kCSM_Wide_Variable, 0.70f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x7d5bf89e, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x853e1d7f, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x88f5b694, kCSM_Wide_Variable, 0.81f, 0.00f, 0.25f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0x9762b46a, kCSM_Wide_Variable, 0.75f, -0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0xa148619a, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00609a71, 0xa2a8464e, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x0069da94, 0x57cccff9, kCSM_Wide_Variable, 0.67f, 0.00f, -0.02f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x0069da94, 0x98821a89, kCSM_Wide_Variable, 0.81f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x0069da94, 0xaf0ffe6a, kCSM_Wide_Variable, 0.81f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x0069da94, 0xe5a99e27, kCSM_Wide_Variable, 0.25f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.67f, 0.00f, -0.02f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0x280f00bc, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0x2eabf52c, kCSM_Wide_Variable, 0.67f, 0.00f, -0.02f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0x328d9281, kCSM_Wide_Variable, 0.67f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0x37f5298a, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0x4531d92a, kCSM_Wide_Variable, 0.81f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0xb8662f5b, kCSM_Wide_Variable, 0.70f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0xc5e694fa, kCSM_Wide_Variable, 0.95f, 0.00f, 0.31f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0xedb74f20, kCSM_Wide_Variable, 0.95f, 0.00f, 0.31f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e292, 0xefd55120, kCSM_Wide_Variable, 0.75f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x2ba11e42, kCSM_Wide_Variable, 0.81f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x43529223, kCSM_Wide_Variable, 0.81f, 0.00f, 0.25f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x43d91d8c, kCSM_Wide_Variable, 0.96f, 0.00f, 0.53f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x48be378d, kCSM_Wide_Variable, 0.25f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x5a266dcf, kCSM_Wide_Variable, 0.24f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x704f6821, kCSM_Wide_Variable, 0.96f, 0.00f, 0.53f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x89531ae4, kCSM_Wide_Variable, 0.96f, 0.00f, 0.53f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x911c2b37, kCSM_Wide_Variable, 0.75f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0x933ed049, kCSM_Wide_Variable, 0.96f, 0.00f, 0.53f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0xc24c0972, kCSM_Wide_Variable, 0.75f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0xc5344c18, kCSM_Wide_Variable, 0.96f, 0.00f, 0.53f ),
   SBP_CameraDef( kCamDefV4, "w22a, a22a, a22b, ta22a", 0x00e1e293, 0xf1574af1, kCSM_Wide_Variable, 0.96f, 0.00f, 0.53f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w22a, a22a, a22b, ta22a" *****

//JIRA-MGSTWO-412
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w23a" *****
   SBP_CameraDef( kCamDefV4, "w23a", 0x002038b0, 0x2259d8da, kCSM_Wide_Variable, 0.80f, 0.00f, 0.90f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x002038b0, 0x57221113, kCSM_Wide_Variable, 1.00f, 0.00f, 0.30f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x002038b0, 0x9804cb7c, kCSM_Wide_Variable, 0.46f, -0.56f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x002038b0, 0xda054e6d, kCSM_Wide_Variable, 0.07f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x0058bd37, 0xb803e5be, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00930be1, 0x44f3899c, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00930be1, 0x7dc572b7, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00930be1, 0x9e7fd521, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00e7cb66, 0x751b84cd, kCSM_Wide_Variable, 0.80f, 0.00f, 0.90f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00e7cb66, 0x7d099b28, kCSM_Wide_Variable, 0.07f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00e7cb66, 0xb3f27e75, kCSM_Wide_Variable, 1.00f, 0.00f, 0.30f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00f5fb54, 0x9bc00008, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00f5fb54, 0xb8f03077, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00f5fb54, 0xddbc4fd0, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23a", 0x00f5fb54, 0xf856fa00, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w23a" *****

//JIRA-MGSTWO-411
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w23b, a23b" ***** 
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00660aa7, 0x0ad8813e, kCSM_Wide_Variable, 0.80f, 0.00f, 0.90f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00660aa7, 0xebdf7e20, kCSM_Wide_Variable, 0.80f, 0.00f, 0.90f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x002038b0, 0x2259d8da, kCSM_Wide_Variable, 0.80f, 0.00f, 0.90f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x002038b0, 0x57221113, kCSM_Wide_Variable, 1.00f, 0.00f, 0.30f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x002038b0, 0x9804cb7c, kCSM_Wide_Variable, 0.46f, -0.56f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x002038b0, 0xda054e6d, kCSM_Wide_Variable, 0.07f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x0058bd37, 0xb803e5be, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00930be1, 0x44f3899c, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00930be1, 0x7dc572b7, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00930be1, 0x9e7fd521, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00e7cb66, 0x751b84cd, kCSM_Wide_Variable, 0.80f, 0.00f, 0.90f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00e7cb66, 0x7d099b28, kCSM_Wide_Variable, 0.07f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00e7cb66, 0xb3f27e75, kCSM_Wide_Variable, 1.00f, 0.00f, 0.30f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00f5fb54, 0x9bc00008, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00f5fb54, 0xb8f03077, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w23b, a23b", 0x00f5fb54, 0xf856fa00, kCSM_Wide_Variable, 0.07f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w23b, a23b" *****

//JIRA: MGSTWO-426, KDEUS-61
   //****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w24a, a24a, ta24a" ***** 
   SBP_CameraDef( kCamDefV4, "a24a, a24a, ta24a", 0x0088397f, 0xa15569b2, kCSM_Wide_Variable, 0.60f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "a24a, a24a, ta24a", 0x0088397f, 0xa269cf16, kCSM_Wide_Variable, 0.90f, 0.00f, 1.00f ), 
   SBP_CameraDef( kCamDefV4, "a24a, a24a, ta24a", 0x0088397f, 0xd109daa0, kCSM_Wide_Variable, 0.40f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "a24a, a24a, ta24a", 0x0088397f, 0xeae82e3e, kCSM_Wide_Variable, 1.00f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00660aa7, 0xad6861ce, kCSM_Wide_Variable, 0.39f, -0.30f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00660aa7, 0xc9f38551, kCSM_Wide_Variable, 0.39f, -1.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00609a71, 0x01d14ca5, kCSM_Wide_Variable, 0.63f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00609a71, 0x156b7e19, kCSM_Wide_Variable, 0.63f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00609a71, 0x314c0f03, kCSM_Wide_Variable, 0.63f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00609a71, 0x61c2ba21, kCSM_Wide_Variable, 0.63f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00609a71, 0xaff6ef33, kCSM_Wide_Variable, 0.63f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.40f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00e1e292,          0, kCSM_Wide_Variable, 0.91f, 0.00f, 0.48f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f272, 0x2e3ea266, kCSM_Wide_Variable, 0.76f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f272, 0x7524a0c9, kCSM_Wide_Variable, 0.90f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f272, 0x7a6083ed, kCSM_Wide_Variable, 0.63f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f272, 0x9a037fdc, kCSM_Wide_Variable, 0.25f, -0.45f, 0.48f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f272, 0xcfbb89cc, kCSM_Wide_Variable, 1.00f, 0.00f, 0.85f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f272, 0xf3e40913, kCSM_Wide_Variable, 0.90f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0x09b96c33, kCSM_Wide_Variable, 0.65f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0x14e3c42f, kCSM_Wide_Variable, 0.90f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0x163c59ca, kCSM_Wide_Variable, 0.63f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0x1a0b16fd, kCSM_Wide_Variable, 0.90f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0x32a47d26, kCSM_Wide_Variable, 0.65f, -0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0x4243ed30, kCSM_Wide_Variable, 0.90f, -0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0x5f05d2c7, kCSM_Wide_Variable, 0.90f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0x6e944adf, kCSM_Wide_Variable, 0.39f, 1.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0xa242fdf4, kCSM_Wide_Variable, 0.65f, -0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0xb4919aa1, kCSM_Wide_Variable, 0.90f, 0.00f, 0.20f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0x00f4f273, 0xf5d36705, kCSM_Wide_Variable, 0.39f, -1.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0xf22af758,        361, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24a, a24a, ta24a", 0xf22af758,        362, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w24a, a24a, ta24a" *****

   //****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w24b, a24b" *****
   SBP_CameraDef( kCamDefV4, "a24b, a24b", 0x0088397f, 0x99b1819d, kCSM_Wide_Variable, 0.20f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "a24b, a24b", 0x0088397f, 0xb2629f36, kCSM_Wide_Variable, 4.00f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00609a71, 0x7bceb837, kCSM_Wide_Variable, 0.91f, 0.00f, 0.48f ),
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.91f, 0.00f, 0.48f ),
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00e1e292, 0x360391af, kCSM_Wide_Variable, 0.91f, 0.00f, 0.48f ),
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00e1e292, 0x602077bf, kCSM_Wide_Variable, 0.25f, 0.00f, 0.50f ),
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00e1e292, 0x7befa99f, kCSM_Wide_Variable, 0.25f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00e1e292, 0x85dbf27e, kCSM_Wide_Variable, 0.91f, 0.00f, 0.48f ),
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00e1e292, 0x936da257, kCSM_Wide_Variable, 0.69f, 0.00f, 0.28f ),
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00e1e293, 0x3fa99011, kCSM_Wide_Variable, 0.69f, 0.00f, 0.28f ),
   SBP_CameraDef( kCamDefV4, "w24b, a24b", 0x00e1e293, 0xb51ccd52, kCSM_Wide_Variable, 0.91f, 0.00f, 0.48f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w24b, a24b" *****

//JIRA-MGSTWO-415, 594
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w24c, a24c, a24f" *****
   SBP_CameraDef( kCamDefV4, "w24c, a24c, a24f", 0x00609a71, 0x9725f410, kCSM_Wide_Variable, 0.31f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24c, a24c, a24f", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.31f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24c, a24c, a24f", 0x00e1e292, 0x995b063b, kCSM_Wide_Variable, 0.31f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24c, a24c, a24f", 0x00e1e293, 0x58a02de2, kCSM_Wide_Variable, 0.31f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24c, a24c, a24f", 0x00e1e293, 0x821e6fea, kCSM_Wide_Variable, 0.31f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24c, a24c, a24f", 0x00e1e293, 0x83b0064f, kCSM_Wide_Variable, 0.31f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24c, a24c, a24f", 0x00e1e293, 0xdc281372, kCSM_Wide_Variable, 0.31f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24c, a24c, a24f", 0x00e1e293, 0xfc2b2e56, kCSM_Wide_Variable, 0.31f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w24c, a24c, a24f" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w24d, a24d, a24g" ***** 
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00660aa7, 0x18b710ae, kCSM_Wide_Variable, 0.57f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00660aa7, 0x7b2add8d, kCSM_Wide_Variable, 0.57f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f272, 0xe5442624, kCSM_Wide_Variable, 0.57f, 0.00f, 0.80f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x02cabb7f, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x04996acf, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x06aa89d5, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x0f3b7bda, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x160529ab, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x1c10f23d, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x1c625cba, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x36d0fb47, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x41bb38cd, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x421c5be3, kCSM_Wide_Variable, 0.23f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x470e852b, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x567c2f49, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x59a5c280, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x64e8d4e0, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0x9fd80b44, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0xa06f2b45, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0xb2ee7d87, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0xcf1a1282, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0xe5e00556, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w24d, a24d, a24g", 0x00f4f273, 0xf25704e2, kCSM_Wide_Variable, 0.61f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w24d, a24d, a24g" *****

//JIRA-MGSTWO-416
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w25c" *****
   SBP_CameraDef( kCamDefV4, "w25c", 0x002038b0, 0x3a4804c6, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x002038b0, 0x563800e7, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x002038b0, 0x650eadae, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x002038b0, 0xee520ed4, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x00609a71, 0xa9b4c95a, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x00660aa7, 0xb351a003, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x00d61044, 0xc00fe67f, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x00d61044, 0xce735afa, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x00f5fb54, 0x38d22e72, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x00f5fb54, 0x7fbbded7, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w25c", 0x00f5fb54, 0xda0a7f80, kCSM_Wide_Variable, 0.63f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w25c" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w28a, a28a" *****
   SBP_CameraDef( kCamDefV4, "w28a, a28a", 0x002038b0, 0xb3d79c5c, kCSM_Wide_Variable, 0.32f, 1.00f, -0.50f ),
   SBP_CameraDef( kCamDefV4, "w28a, a28a", 0x0058bd37, 0x4a789e23, kCSM_Wide_Variable, 0.46f, -0.39f, -1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w28a, a28a" *****

//JIRA-MGSTWO-417
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w31a, a31a, ta31a" ***** 
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x0060ccf7, 0x8668662d, kCSM_Wide_Variable, 0.62f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x0060ccf7, 0x9a9b68b0, kCSM_Wide_Variable, 0.45f, 0.00f, 0.00f ), 
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00660aa7, 0x2bcb3ea9, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00660aa7, 0x2ca7b89f, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00660aa7, 0x441558ac, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00660aa7, 0x5b57eab1, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00660aa7, 0xb05a7cbf, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00660aa7, 0xf9bd5e00, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x004fd5b7, 0x03cd8a77, kCSM_Wide_Variable, 0.62f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x004fd5b7, 0x0fdbc32c, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x004fd5b7, 0x567fea1e, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x004fd5b7, 0x574474fe, kCSM_Wide_Variable, 0.60f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x004fd5b7, 0x76711f68, kCSM_Wide_Variable, 0.39f, 0.00f, 0.60f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x004fd5b7, 0x8654236e, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x004fd5b7, 0xb404befa, kCSM_Wide_Variable, 0.54f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x004fd5b7, 0xed20de89, kCSM_Wide_Variable, 0.54f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00609a71, 0x7c67e9d5, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00e1e292, 0x628cdaaa, kCSM_Wide_Variable, 0.65f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00e1e292, 0x8a6ea546, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00e1e292, 0x9103b3df, kCSM_Wide_Variable, 0.45f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00e1e292, 0xb8d75799, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00e1e293, 0x0d8fb649, kCSM_Wide_Variable, 0.97f, 0.00f, 0.47f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00e1e293, 0x173e8e5b, kCSM_Wide_Variable, 0.45f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00e1e293, 0x2707ad01, kCSM_Wide_Variable, 0.62f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31a, a31a, ta31a", 0x00e1e293, 0xa4fab1b5, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w31a, a31a, ta31a" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w31b" *****
   SBP_CameraDef( kCamDefV4, "w31b", 0x00609a71, 0x5ea6d49f, kCSM_Wide_Variable, 0.76f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31b", 0x00609a71, 0xd63fa6ba, kCSM_Wide_Variable, 0.76f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31b", 0x00930be1, 0x63830116, kCSM_Wide_Variable, 0.68f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31b", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.76f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31b", 0x00b7f056, 0x707a04d2, kCSM_Wide_Variable, 1.00f, 0.12f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31b", 0x00f4f273, 0x11f3c48e, kCSM_Wide_Variable, 0.76f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31b", 0x00f4f273, 0x66b607c6, kCSM_Wide_Variable, 0.76f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31b", 0x00f4f273, 0xa7837393, kCSM_Wide_Variable, 0.76f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w31b" *****

// JIRA-MGSTWO-425
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w31c" *****
   SBP_CameraDef( kCamDefV4, "w31c", 0x004fd5b7, 0xb1e7e39f, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31c", 0x00607873, 0x04ac6943, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31c", 0x00607873, 0xb8432fc8, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31c", 0x00609a71, 0x2317fa46, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31c", 0x00609a71, 0x58a9749d, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31c", 0x00609a71, 0xa97ef3d0, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31c", 0x00609a71, 0xf8c52f32, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31c", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w31c" *****

// JIRA-MGSTWO-766
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w31d" *****
   SBP_CameraDef( kCamDefV4, "w31d", 0x004fd5b7, 0x03cd8a77, kCSM_Wide_Variable, 0.62f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x004fd5b7, 0x0fdbc32c, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x004fd5b7, 0x567fea1e, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x004fd5b7, 0x574474fe, kCSM_Wide_Variable, 0.60f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x004fd5b7, 0x76711f68, kCSM_Wide_Variable, 0.37f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x004fd5b7, 0x8654236e, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x004fd5b7, 0xb404befa, kCSM_Wide_Variable, 0.88f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x004fd5b7, 0xed20de89, kCSM_Wide_Variable, 0.88f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x00609a71, 0x7c67e9d5, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x00e1e292, 0x628cdaaa, kCSM_Wide_Variable, 0.30f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x00e1e292, 0x8a6ea546, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x00e1e292, 0x9103b3df, kCSM_Wide_Variable, 0.45f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x00e1e292, 0xb8d75799, kCSM_Wide_Variable, 0.85f, 0.00f, 0.88f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x00e1e293, 0x0d8fb649, kCSM_Wide_Variable, 0.97f, 0.00f, 0.47f ),
   SBP_CameraDef( kCamDefV4, "w31d", 0x00e1e293, 0xa4fab1b5, kCSM_Wide_Variable, 0.98f, 0.00f, 0.45f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w31d" *****

// JIRA-MGSTWO-418
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w31f" *****
   SBP_CameraDef( kCamDefV4, "w31f", 0x004fd5b7, 0x71f6e5e5, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x004fd5b7, 0x9bf2f878, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x004fd5b7, 0xab014fff, kCSM_Wide_Variable, 0.36f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x004fd5b7, 0xb1e7e39f, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x004fd5b7, 0xceab50a5, kCSM_Wide_Variable, 0.37f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00607873, 0x04ac6943, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00607873, 0xb8432fc8, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0x1e2f0cff, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0x1eab6410, kCSM_Wide_Variable, 0.36f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0x2317fa46, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0x32bb883b, kCSM_Wide_Variable, 0.37f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0x35ecbd1e, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0x58a9749d, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0x89e13450, kCSM_Wide_Variable, 0.37f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0x978e86d7, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xa4f999fd, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xa97ef3d0, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xba7af2e0, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xcb3e2991, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xcce7a414, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xcf030fba, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xf8c52f32, kCSM_Wide_Variable, 0.44f, 0.00f, 1.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xfd113773, kCSM_Wide_Variable, 0.36f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00609a71, 0xfef14511, kCSM_Wide_Variable, 0.37f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x0060ccf7, 0x05de13b8, kCSM_Wide_Variable, 0.37f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w31f", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.59f, 0.00f, 0.44f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w31f" *****

// JIRA MGSTWO-231: REMAPPED!
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w32b" *****
   SBP_CameraDef( kCamDefV4, "w32b", 0x564c6e8b,       1528, kCSM_Wide_Variable, 0.74f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w32b", 0x564c6e8b,       1754, kCSM_Wide_Variable, 0.00f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w32b" *****

//JIRA-MGSTWO-420
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w41a, a41a, a41b" ***** 
   SBP_CameraDef( kCamDefV4, "w41a, a41a, a41b", 0x00660aa7, 0xe5268737, kCSM_Wide_Variable, 0.69f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w41a, a41a, a41b", 0x00660aa7, 0xfa64352a, kCSM_Wide_Variable, 0.69f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w41a, a41a, a41b", 0x002038b0, 0xea2e0b62, kCSM_Wide_Variable, 0.69f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w41a, a41a, a41b", 0x002038b0, 0xfa4abead, kCSM_Wide_Variable, 0.69f, 0.00f, -0.00f ),
   SBP_CameraDef( kCamDefV4, "w41a, a41a, a41b", 0x0058bd37, 0xa79bd742, kCSM_Wide_Variable, 0.69f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w41a, a41a, a41b", 0x00e7cb66, 0x66b9342b, kCSM_Wide_Variable, 0.69f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w41a, a41a, a41b" *****

//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w42a, a42a, ta42a" ***** 
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00660aa7, 0x1b05cbae, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00660aa7, 0x41b7b129, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00660aa7, 0x809489b8, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00660aa7, 0x9d7903d5, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00e1e293, 0x3f18f669, kCSM_Wide_Variable, 0.15f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00e1e293, 0x70d8863e, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00e1e293, 0xc49fe4c4, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w42a, a42a, ta42a", 0x00e1e294,    2267759, kCSM_Wide_Variable, 0.80f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w42a, a42a, ta42a" ***** 

//JIRA-MGSTWO-421
//****** MGS2 BEGIN CAMERA SETTINGS FOR AREA "w44a, a44a" *****
   SBP_CameraDef( kCamDefV4, "w44a, a44a", 0x00b5bfa1,          0, kCSM_Wide_Variable, 0.65f, 0.00f, 0.00f ),
   SBP_CameraDef( kCamDefV4, "w44a, a44a", 0x00e1e292, 0xb4e70d72, kCSM_Wide_Variable, 0.65f, 0.00f, 0.00f ),
//****** MGS2 END CAMERA SETTINGS FOR AREA "w44a, a44a" *****
};

const int gBP_CameraDefsCount = sizeof(gBP_CameraDefs) / sizeof(gBP_CameraDefs[0]);

//----------------------------------------------------------------------------
// C++ FUNCTIONS
//----------------------------------------------------------------------------

const SBP_CameraDef* BP_Camera_GetDefs()
{
   return gBP_CameraDefs;
}

//----------------------------------------------------------------------------

const int BP_Camera_GetDefsCount()
{
   return gBP_CameraDefsCount;
}

//----------------------------------------------------------------------------
// MGS2 CINEMA SETTINGS
//----------------------------------------------------------------------------

// These streaming cinemas have forced bars, forced wide screen always
const SBP_CinemaDef gBP_CinemaDefs[] =
{
   // (snake_tanker/stage/w02a)
   { "w02a", 0xc559c83a, "scndemo_w02a.sdt",    kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGSTWO-1999

   // (plant_raiden/story/very easy/after ninja demo(w20c), progress to w24a
   { "w24a", 0x849f3a0e, "scndemo_w24a.sdt",    kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGSTWO-2000

   // (plant_raiden/stage/very easy/w24b
   { "w24b", 0xc33f40de, "scndemo_w24b.sdt",    kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGSTWO-2001

   // (plant_raiden/stage/very easy/item full/w31a
   { "w31a", 0xadb597af, "p048_01_p01.sdt",     kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGSTWO-2002
   
   // (snake_tanker/stage/w01b)
   { "w01b", 0xda95debe, "scndemo_w01b_00.sdt", kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGSTWO-2029
   { "w01b", 0xe7f5f70e, "scndemo_w01b_01.sdt", kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGSTWO-2029
   { "w01b", 0xa0558dde, "scndemo_w01b_02.sdt", kCinema_Bars_On, kCinema_Mode_WideScreen },   // MGSTWO-2029
};

//----------------------------------------------------------------------------

// These in game cut scenes have bars based on options, but are always forced wide screen
const SBP_CutsceneDef gBP_CutsceneDefs[] =
{
   // Raiden/olga microphone cut scene
   // (plant_raiden/story/shell2_in)
   { "w31a", 0x00e1e293, 0xa4fab1b5, kCinema_Bars_Options, kCinema_Mode_WideScreen },   // MGSTWO-2034
   { "w31a", 0x00000000, 0x00000000, kCinema_Bars_Options, kCinema_Mode_WideScreen },   // MGSTWO-2034

   // The "last killing shot cutscene" of Vamp has Black bars that need removed.
   // (plant_raiden/very easy/vamp(w13c)
   { "w31c", 0x00000000, 0x00000000, kCinema_Bars_Options, kCinema_Mode_WideScreen },   // MGSTWO-2036

   // The cutscene of transitioning in to the Solidus fight has Black bars that are midway fading out.
   // (plant_raiden/demo test/raise the AG)
   { "w61a", 0x00000000, 0x00000000, kCinema_Bars_Options, kCinema_Mode_WideScreen },   // MGSTWO-2039

};

//----------------------------------------------------------------------------

const int gBP_CinemaDefsCount = sizeof(gBP_CinemaDefs) / sizeof(gBP_CinemaDefs[0]);

//----------------------------------------------------------------------------

const SBP_CinemaDef* BP_Cinema_GetDefs()
{
   return gBP_CinemaDefs;
}

//----------------------------------------------------------------------------

const int BP_Cinema_GetDefsCount()
{
   return gBP_CinemaDefsCount;
}

//----------------------------------------------------------------------------

const int gBP_CutsceneDefsCount = sizeof(gBP_CutsceneDefs) / sizeof(gBP_CutsceneDefs[0]);

//----------------------------------------------------------------------------

const SBP_CutsceneDef* BP_Cutscene_GetDefs()
{
   return gBP_CutsceneDefs;
}

//----------------------------------------------------------------------------

const int BP_Cutscene_GetDefsCount()
{
   return gBP_CutsceneDefsCount;
}

//----------------------------------------------------------------------------
// C BLENDING FUNCTIONS
//----------------------------------------------------------------------------

void GV_NearExp4BP( SBP_CameraSettings* set1, SBP_CameraSettings* set2 )
{
   // Blend auxiliary vars
   set1->offsetX = GV_NearExp4F( set1->offsetX, set2->offsetX );
   set1->offsetY = GV_NearExp4F( set1->offsetY, set2->offsetY );
   set1->aspectX = GV_NearExp4F( set1->aspectX, set2->aspectX );
   set1->aspectY = GV_NearExp4F( set1->aspectY, set2->aspectY );
}

//----------------------------------------------------------------------------

void GV_NearExp8BP( SBP_CameraSettings* set1, SBP_CameraSettings* set2 )
{
   // Blend auxiliary vars
   set1->offsetX = GV_NearExp8F( set1->offsetX, set2->offsetX );
   set1->offsetY = GV_NearExp8F( set1->offsetY, set2->offsetY );
   set1->aspectX = GV_NearExp8F( set1->aspectX, set2->aspectX );
   set1->aspectY = GV_NearExp8F( set1->aspectY, set2->aspectY );
}

//----------------------------------------------------------------------------

void GV_NearExp16BP( SBP_CameraSettings* set1, SBP_CameraSettings* set2 )
{
   // Blend auxiliary vars
   set1->offsetX = GV_NearExp16F( set1->offsetX, set2->offsetX );
   set1->offsetY = GV_NearExp16F( set1->offsetY, set2->offsetY );
   set1->aspectX = GV_NearExp16F( set1->aspectX, set2->aspectX );
   set1->aspectY = GV_NearExp16F( set1->aspectY, set2->aspectY );
}

//----------------------------------------------------------------------------

void GV_NearTimeBP( SBP_CameraSettings* set1, SBP_CameraSettings* set2, int time )
{
   // Blend auxiliary vars
   set1->offsetX = GV_NearTimeF( set1->offsetX, set2->offsetX, time );
   set1->offsetY = GV_NearTimeF( set1->offsetY, set2->offsetY, time );
   set1->aspectX = GV_NearTimeF( set1->aspectX, set2->aspectX, time );
   set1->aspectY = GV_NearTimeF( set1->aspectY, set2->aspectY, time );
}

//----------------------------------------------------------------------------
