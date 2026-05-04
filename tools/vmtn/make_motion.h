#ifndef _make_motion_h_
#define _make_motion_h_

#include "mtnlib.h"
#include "mtn_list.h"
#include "vmotion.h"

#ifndef _make_motion_c_
#define EXT extern
#define INIT(n)
#else
#define EXT
#define INIT(n)   = n
#endif /* _make_motion_c_ */



#undef EXT
#undef INIT(n)


FACE_MOTION * mkMakeMotion(FACE_MOTION * ret,
			   lipMotionList * list,
			   vmtnVowel * vowel,
			   int pre_id, int id,
			   float frame, float total_frame);

int           mkLabel2ID(lipMotionList * list, char * label);

FACE_MOTION * mkMotionPower(FACE_MOTION * ret, vmtnVowel * vowel,
			    FACE_MOTION * mtn, float power);

#endif /* _make_motion_h_ */

