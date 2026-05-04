#ifndef _voice_class_h_
#define _voicd_class_h_



#ifdef _voice_class_c_
#define EXT
#define INIT(n)  = n
#else
#define EXT extern
#define INIT(n)
#endif /* _voice_class_c_ */






#undef EXT
#undef INIT(n)




#endif /* _voice_class_h_ */
