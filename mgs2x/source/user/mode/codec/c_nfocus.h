#ifndef _c_nfocus_h_
#define _c_nfocus_h_

void nfocusSwitch(void * workp, int sw);
void nfocusSetFocus(void * workp, int t, int type, int near, int far);
void * NewCodecNearFocusEffect(int name, int chanl,
			       int max_plane, int near, int far);

#endif /* _c_nfocus_h_ */
