#ifndef _cdc_movie_h_
#define _cdc_movie_h_

/* ムービー展開バッファの大きさ */
#define CMOVIE_WIDTH    176
#define CMOVIE_HEIGHT   144

#define CMOVIE_U0  (int)(0.5F * 16.0F)
#define CMOVIE_V0  (int)(0.5F * 16.0F)
#define CMOVIE_U1  (int)(((float)CMOVIE_WIDTH  - 0.5F) * 16.0F)
#define CMOVIE_V1  (int)(((float)CMOVIE_HEIGHT - 0.5F) * 16.0F)


/* ムービー展開バッファサイズ */
#ifdef PSX2
#define CMOVIE_SIZE     (CMOVIE_WIDTH*CMOVIE_HEIGHT*sizeof(unsigned short))
#else
#define CMOVIE_SIZE     (CMOVIE_WIDTH*CMOVIE_HEIGHT*sizeof(unsigned int))
#endif

void   CDC_SetMovieDispStatus(int sw);
void   CDC_KillMovie(void);
void * NewCodecMovie(SPR_OBJ * guide);

#endif /* _cdc_movie_h_ */
