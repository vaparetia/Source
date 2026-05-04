#ifndef _codeccrt_h_
#define _codeccrt_h_

#ifndef _codeccrt_c_
#define EXT extern
#else
#define EXT
#endif /* _codeccrt_c_ */



#undef EXT

void * NewCodecEffectCRT(int side, int x, int y, int w, int h);
void * NewCodecEffectCRT_SCN(int name, int where);

#endif /* _codeccrt_h_ */
