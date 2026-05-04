#ifndef _read_seq_h_
#define _read_seq_h_

#include "mtn_list.h"

typedef struct {
  int           time;
  unsigned char id;
  unsigned char power;
} seqStep;



#ifndef _read_seq_c_
#define EXT extern
#define INIT(n)
#else
#define EXT
#define INIT(n)  = n
#endif /* _read_seq_c_ */



#undef EXT
#undef INIT(n)


seqStep * rseqReadLipSequence(lipMotionList * list, char * fname);
seqStep * rseqReadLipSequenceByStream(lipMotionList * list, FILE * rfp);
void      rseqReleaseSequence(seqStep * seq);

#endif /* _read_seq_h_ */
