#ifndef _radar_macro_h_
#define _radar_macro_h_

#define SET_COLOR_2DPRIM(_prim,_col) (*(int*)&(_prim)->sprite.col = (_col))
#define SET_COLOR_2DPRIM2(_prim,_r,_g,_b,_a) \
        SET_COLOR_2DPRIM((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))


#define SET_COLOR_SPIN(_prim,_col) (*(int*)&(_prim)->spin.col = (_col))
#define SET_COLOR_SPIN2(_prim,_r,_g,_b,_a) \
        SET_COLOR_SPIN((_prim),(((_a)<<24) | ((_b)<<16)| ((_g)<<8) | (_r)))



#define SET_COLOR_SNK(_prim) SET_COLOR_2DPRIM2(_prim,SNK_R,SNK_G,SNK_B,0x80);
#define SET_COLOR_SRC(_prim) SET_COLOR_2DPRIM2(_prim,SRC_R,SRC_G,SRC_B,0x80);
#define SET_COLOR_ALT(_prim) SET_COLOR_2DPRIM2(_prim,ALT_R,ALT_G,ALT_B,0x80);
#define SET_COLOR_ESC(_prim) SET_COLOR_2DPRIM2(_prim,ESC_R,ESC_G,ESC_B,0x80);
#define SET_COLOR_JAM(_prim) SET_COLOR_2DPRIM2(_prim,JAM_R,JAM_G,JAM_B,0x80);
#define SET_COLOR_DEF(_prim) SET_COLOR_2DPRIM2(_prim,0x80,0x80,0x80,0x80);

#define SET_COLOR_SNK_ALP(_prim ,_alp) \
        SET_COLOR_2DPRIM2(_prim,SNK_R,SNK_G,SNK_B,_alp);

#define SET_COLOR_SRC_ALP(_prim,_alp) \
        SET_COLOR_2DPRIM2(_prim,SRC_R,SRC_G,SRC_B,_alp);

#define SET_COLOR_ALT_ALP(_prim,_alp) \
        SET_COLOR_2DPRIM2(_prim,ALT_LEVEL_R,ALT_LEVEL_G,ALT_LEVEL_B,_alp);

#define SET_COLOR_ESC_ALP(_prim,_alp) \
        SET_COLOR_2DPRIM2(_prim,ESC_LEVEL_R,ESC_LEVEL_G,ESC_LEVEL_B,_alp);

#define SET_COLOR_JAM_ALP(_prim,_alp) \
        SET_COLOR_2DPRIM2(_prim,JAM_LEVEL_R,JAM_LEVEL_G,JAM_LEVEL_B,_alp);

#define SET_COLOR_DEF_ALP(_prim,_alp) \
        SET_COLOR_2DPRIM2(_prim,0x80,0x80,0x80,_alp);

/* work->statusにセット */
#define _ABSf(x) ((x>=0.0f)?(x):(-(x)))

#define VUSETCOL( a )	( (a) * 4 )

#endif /* _radar_macro_h_ */
