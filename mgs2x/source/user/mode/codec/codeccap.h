#ifndef _codeccap_h_
#define _codeccap_h_

struct GM_STREAM_CONTROL_s;

int    GM_GetCodecCapStatus(void);
void * NewStreamCodecCapDriver( struct GM_STREAM_CONTROL_s * ctrl, int type);

int    GM_StreamCodecCaptionDriverInit(void);

#endif /* _codeccap_h_ */
