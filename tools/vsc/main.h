#ifndef _main_h_
#define _main_h_

#define PERSONAL
#undef  PERSONAL

#ifdef PERSONAL
#define CONFIG_FILE ".voice"
#define DICTIONARY_DIR  "/usr/local/share/vsc/"
#else
#define CONFIG_DIR  "/u/develop/mj001b/tools/shared/vsc/"
#define CONFIG_FILE "voice.cnf"
#define DICTIONARY_DIR  "/u/develop/mj001b/tools/shared/vsc/"
#endif

#define WAV_SCALE_1  100.0
#define WAV_SCALE_2  750000

/* このフレーム数以上無音フレームが続いたら、そこまでを一つの処理単位とする */
#define SIL_FRAMES  30

typedef enum {
  LANG_JAPANESE,
  LANG_ENGLISH
} LANG;



#ifndef _main_c_
#define EXT extern
#define INIT(n)
#else
#define EXT
#define INIT(n)  = n
#endif

EXT   int  mVarbose   INIT(0);
EXT   int  mGroupDisp INIT(0);
EXT   int  mDicDisp   INIT(0);
EXT   int  mVoiceMode INIT(0);
EXT   char mDicDir[256];
EXT   LANG mLangMode INIT(LANG_JAPANESE);
#undef EXT
#undef INIT(n)

#endif /* _main_h_ */
