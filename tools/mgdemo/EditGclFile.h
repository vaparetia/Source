/******************************************************************************
 * System	: DemoAct
 * Computer : DOS/V
 * OS		: Microsoft Windows
 * Compiler : Microsoft Visual C++
 * Module	: Win32 Console Application
 */

/*
#Íö(EUCÈ½ÄêÍÑÊ¸»ú)
*/


#ifndef __INC_DEMOFILE__
#define __INC_DEMOFILE__

/******************************************************************************
 * included
 */

/******************************************************************************
 * defined
 */

/******************************************************************************
 * typedef and struct
 */

/******************************************************************************
 * extern
 */

/* DemoActCommads.c wrote */
extern char* lpszCheckCodeDemo;
extern char* lpszCheckEUCCode;
extern char* lppszCommentDefine[];
extern char* lppszDefine[];
extern char* lppszCommentMap[];
extern char* lppszMap[];
extern char* lppszCommentDemo[];
extern char* lppszDemo[];
extern char* lpszDemoLoop;
extern char* lpszFindDemoAct;
extern char* lpszDemoStringMark;
extern char* lpszViewCountMark;
extern char* lpszViewStartModeMark;
extern char* lpszFrameBaseTimeMark;
extern char* lpszStartTimeMark;

/* EditGclFile.c */
extern int EditGclFile(char* lpszGclFileName, char* lpszGclFileString, char* lpszDemoFileName, int nCommand, char* lpszDrawString, char lpszViewCount[], char lpszViewStartMode[], char lpszFrameBaseTime[], char lpszStartTime[]);

/*****************************************************************************/

#endif // __INC_DEMOFILE__
