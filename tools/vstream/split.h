#ifndef _split_h_
#define _split_h_

#include "seq.h"

VoiceSeq  * splLoadSequence(FILE *rfp);
void        splRelease(VoiceSeq ** seq);
/* 自動分割関数 */
VoiceSeq ** splAutoSplit(VoiceSeq * seq, int packet_size);

void        splOutput(FILE *wfp, VoiceSeq ** sp_seq, int packet_size);

#endif /* _split_h_ */
