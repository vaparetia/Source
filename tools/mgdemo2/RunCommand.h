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


#ifndef __INC_RUNCOMMAND__
#define __INC_RUNCOMMAND__

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
 * static
 */

extern char* lppMakeCommand[];
extern char* lppMakeLinkCommand[];


/******************************************************************************
 * public
 */
int RunCommnad(int nMakeType, char* lpszTarget, char* lpszResident, char* lpszCDRomImgDirectory);


/*****************************************************************************/

#endif // __INC_RUNCOMMAND__
