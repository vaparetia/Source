#ifndef _c_indemo_h_
#define _c_indemo_h_

typedef enum {
  CODEC_DEMO_IN,
  CODEC_DEMO_OUT
} CodecDemoMode;

#define MONO_R   80
#define MONO_G   200
#define MONO_B   200
#define MONO_BASE 100

/* 動作開始から動作完了までの時間(1/300単位) */
#define DEMO_TIME   60


#ifndef _c_indemo_c_
#define EXT extern
#else
#define EXT
#endif


#undef EXT

void * NewCodecIntoDemo(CodecDemoMode mode, int fade);
int    CodecBG_Recover(void * workp, int fade);
int    IsBGsetuped(void);
int    IsBGrecovered(void);


#endif /* _c_indemo_h_ */
