#ifndef _label_h_
#define _label_h_


typedef struct _lblLabel {
  struct _lblLabel * prev;
  struct _lblLabel * next;
  char * label;
  int    id;
} lblLabel;


typedef struct _lblBlock {
  lblLabel * begin;
  lblLabel * end;

  int newer_num;
  
} lblBlock;


#define LBLERR_REGISTED    -1
#define LBLERR_NO_MEMORY   -2
#define LBLERR_UNDEFINED   -3

#ifndef _label_c_
#define EXT extern
#else
#define EXT
#endif /* _label_c_ */


#undef EXT


lblBlock * lblCreateLabelBlock(void);
int        lblReleaseLabelBlock(lblBlock * block);
int        lblDefineLabel(lblBlock * block, char * label);
int        lblUndefLabel(lblBlock * block, char * label);
int        lblRefLabel(lblBlock * block, char * label);

#ifdef _DEBUG_
void       lblDbgShow(FILE * output, lblBlock * block);
#endif /* _DEBUG_ */

#endif /* _label_h_ */
