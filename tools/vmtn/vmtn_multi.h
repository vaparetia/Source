#ifndef _vmtn_multi_h_
#define _vmtn_multi_h_

#include "vmotion.h"


typedef struct {
  int         code;     /* 話者コード                 */
  char      * name;     /* 話者名へのポインタ(文字列) */
  vmtnVowel * motions;  /* その話者用のモーション素材 */
  vmtnSet   * set;
} vmtnSpeaker;


typedef struct {
  int            nums;      /* 登録されている話者数             */
  vmtnSpeaker ** mtn_block; /* 各話者のモーション素材管理構造体 */
} vmtnContainer;

int             vmmSelectMotionSet(vmtnContainer * container,
				   int speaker_code, char * set_label);
vmtnSpeaker   * vmmReadSpeakerMotion(char * fname);
void            vmmReleaseSpeakerMotion(vmtnSpeaker * speaker);
vmtnContainer * vmmCreateContainer(void);
vmtnContainer * vmmRegistSpeaker(vmtnContainer * cont, vmtnSpeaker * speaker);
void            vmmReleaseContainer(vmtnContainer * cont);

#endif /* _vmtm_multi_h_ */
