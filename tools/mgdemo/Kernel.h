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
#define COMMAND_DEMO      "demo"
#define COMMAND_DEMOLOOP  "loop"
#define COMMAND_HELP      "help"
#define COMMAND_GO        "go"
#define COMMAND_MODELPLUS "-m"

#define MGDEMO_DEMO                0x00000001
#define MGDEMO_LOOP                0x00000002
#define MGDEMO_HELP                0x00000004
#define MGDEMO_GO                  0x00000008
#define MGDEMO_MODELPLUS           0x00000010
#define MGDEMO_MAKE_ALL            0x00010000
#define MGDEMO_MAKE_ONE_STAGE      0x00020000
#define MGDEMO_MAKE_SCN            0x00040000
#define MGDEMO_MAKE_STAGE          0x00080000
#define MGDEMO_MAKE_LINK           0x00100000
#define MGDEMO_MAKE_LINK_ONE_STAGE 0x00200000

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
extern int CheckFile(char* lpszFilePath);

/* src ファイルエディット */
extern char* lpszCheckEUCCode;
extern int EditSrcFile(char* lpszListFilePath, char* lpszSrcFilePath, char* lpszSrcFileName, char* lpszListDirectory);

/*****************************************************************************/

#endif // __INC_KERNEL__
