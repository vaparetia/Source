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

enum {
	MAKE_TYPE_ALL = 0,
	MAKE_TYPE_ALL_ONE_STAGE,
	MAKE_TYPE_GO,
	MAKE_TYPE_SCN,
	MAKE_TYPE_STAGE,
	MAKE_TYPE_LINK,
	MAKE_TYPE_LINK_ONE_STAGE,
	
};

extern char* lppMakeCommand[];
extern char* lppMakeLinkCommand[];


/******************************************************************************
 * public
 */
int RunCommnad(int nMakeType, char* lpszTarget);


/*****************************************************************************/

#endif // __INC_RUNCOMMAND__
