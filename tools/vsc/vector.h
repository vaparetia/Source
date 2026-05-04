#ifndef _vector_h_
#define _vector_h_


#ifdef _vector_c_
#define EXT
#define INIT(n) = n
#else
#define EXT extern
#define INIT(n)
#endif /* _vector_c_ */





#undef EXT
#undef INIT



#endif  /* _vector_h_ */
