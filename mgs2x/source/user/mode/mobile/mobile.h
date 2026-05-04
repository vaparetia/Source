#ifndef _mobile_h_
#define _mobile_h_

#define MOBILE_CHANL      4

#define MOBILE_ACT_LEVEL  GV_ACTOR_MANAGER

#include "mobact.h"

/* 携帯関係の SE */
#define PHONE_MUTE    SD_S_PHONE000
#define PHONE_HANGUP  SD_S_PHONE002

#define PHONE_RING    SD_S_PHONE001
#define PHONE_MELODY  SD_S_PHONEMEL

/*
 * 英語版では通常の電話着信音,日本語版では着メロ
 */
#ifdef ENGLISH
#define PHONE_CALL  PHONE_RING
#else
#define PHONE_CALL  PHONE_MELODY
#endif /* ENGLISH */

#define CALL_SOUND_ON
// #undef CALL_SOUND_ON


#endif /* _mobile_h_ */
