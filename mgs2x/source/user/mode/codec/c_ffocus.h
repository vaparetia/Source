#ifndef _c_ffocus_h_
#define _c_ffocus_h_

void ffocusSwitch(void * workp, int sw);
void ffocusSetFocus(void * workp, int t, int type, int near, int far);
void * NewCodecFarFocusEffect( int name, int chanl,
			       int max_plane, int near, int far);

#endif /* _c_ffocus_h_ */
