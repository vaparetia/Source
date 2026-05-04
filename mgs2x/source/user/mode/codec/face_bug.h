#ifndef _bug_face_h_
#define _bug_face_h_

#define SIGNAL_BUGFACE_KILL   0x0070
#define SIGNAL_BUGFACE_DEFMTN 0x0071

void * BugFaceGetFaceWork(void * workp);
void * NewBugFace(int side, int dummy_bone, int speaker,
		  int min, int bias, int * model_list);


#endif /* _bug_face_h_ */
