/******************************************************************************
 * System	: DemoAct
 * Computer : DOS/V
 * OS		: Microsoft Windows
 * Compiler : Microsoft Visual C++
 * Module	: Win32 Console Application
 */

#ifndef __INC_KERNEL__
#define __INC_KERNEL__

/******************************************************************************
 * included
 */
#ifdef _WINDOWS
#include <direct.h>
#endif
#ifdef _UNIX
#include <unistd.h>
#endif
#ifdef _LINUX
#include <unistd.h>
#endif

/******************************************************************************
 * defined
 */
#define DEMO_HD_STAGE "demohd"

#define COMMAND_DEMO              "demo"
#define COMMAND_DEMOLOOP          "loop"
#define COMMAND_HELP              "help"
#define COMMAND_MAKE_HD_MODULE    "insthd"
#define COMMAND_MAKE_HD_NO_FORMAT "insthd_no_form"
#define COMMAND_CUT_CHECK         "skip"

#define MGDEMO_DEMO           0x00000001
#define MGDEMO_LOOP           0x00000002
#define MGDEMO_HELP           0x00000004
#define MGDEMO_CUT_CHECK      0x00000010
#define MGDEMO_MAKE_HD_MODULE 0x00000020
#define MGDEMO_HD_FORMAT      0x00000040

#ifdef _WINDOWS
#define getcwd(_x, _y) _getcwd(_x, _y)
#define chdir(_x) _chdir(_x)
#define sleep(_x) Sleep(_x);
#endif

 /******************************************************************************
 * typedef and struct
 */



/******************************************************************************
 * globals
 */
/* ファイルの存在チェック */
extern int CheckFile(char* lpszFilePath, int nErrorPrintFlag);
extern int CheckDirectory(char* lpszDirectoryPath, int nErrorPrintFlag);

/* src ファイルエディット */
extern char* lpszCheckEUCCode;
extern int EditSrcFile(char* lpszListFilePath, char* lpszSrcFilePath, char* lpszSrcFileName, char* lpszListDirectory);

/*****************************************************************************/

#endif // __INC_KERNEL__
