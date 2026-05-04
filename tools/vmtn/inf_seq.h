#ifndef _inf_seq_h_
#define _inf_seq_h_

typedef struct {
  int    begin;    /* その話者領域の開始時刻(-1 で終端) */
  int    speaker;  /* 話者 ID(-1で無音)                 */
  char * spname;   /* 話者名文字列(無音の場合は NULL)   */
} TalkArea;


TalkArea * infReadSpeaker(char * fname);
TalkArea * infReadSpeakerByStream(FILE * rfp);
void       infReleaseSpeaker(TalkArea * talk);

int        get_strcode(char *string);

#endif /* _inf_seq_h_ */
