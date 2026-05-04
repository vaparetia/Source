/*
	prekey.h
		キーリピート対応

	1999/12/13 K.Kano
	$Id: prekey.h,v 1.1.1.3 2002/11/19 11:43:30 Yoshizawa1 Exp $
*/


#ifndef _prekey_h_
#define _prekey_h_

#define REPEAT_FIRST	20
#define REPEAT_NEXT	3
#define REPEAT_SECOND	(REPEAT_FIRST+30)


typedef struct {
    int status;
    int auto_status;
    int press;

    int repeat_count;
    int repeat_next_count;
} PreviewKey_Param;


#ifdef _WORK_DECLARED_
PreviewKey_Param PreviewKey;
#else
extern PreviewKey_Param PreviewKey;
#endif


void PreKeyInit(void);
void PreKeyAct(void);


#endif
