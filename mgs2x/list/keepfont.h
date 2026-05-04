#if 1
/* サブスタンスはフォントはＰＡＬも含めて１種類だけ */

binary		keepfontsub.row
rename keepfontsub.row keepfont.row

#else

#ifdef MGS2_VMODE_PAL
binary		keepfontec.row
rename keepfontec.row keepfont.row
#else
binary		keepfont.row
#endif

#endif

#ifdef MGS2_XBOX
binary sitanker.row siplant.row sivr.row sist.row
#endif
