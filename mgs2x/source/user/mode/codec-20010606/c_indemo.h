#ifndef _c_indemo_h_
#define _c_indemo_h_

typedef enum {
  CODEC_DEMO_IN,
  CODEC_DEMO_OUT
} CodecDemoMode;


/* 動作開始から動作完了までの時間(1/300単位) */
#define DEMO_TIME   60


#ifndef _c_indemo_c_
#define EXT extern
#else
#define EXT
#endif


#undef EXT

void * NewCodecIntoDemo(CodecDemoMode mode, int fade);





#endif /* _c_indemo_h_ */
