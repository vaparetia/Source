/******************************************************************************
 * System	: mgdemo
 * Computer : DOS/V
 * OS		: Microsoft Windows
 * Compiler : Microsoft Visual C++
 * Module	: Win32 Console Application
 */
/******************************************************************************
 * コマンド解析関数のヘッダー
 */


#ifndef __INC_ANALYZECOMMAND__
#define __INC_ANALYZECOMMAND__

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
 * globals
 */
int AnalyzeCommand(int argc, char* argv[], int* lpnCommand, char* lpszTarget, char* lpszDemoFile, char* szString, char* lpszViewCount, char* lpszViewStartMode, char* lpszFrameBaseTime, char* lpszStartTime, int* lpnPCMStream, char* lpszAddGclFile);

/*****************************************************************************/

#endif // __INC_ANALYZECOMMAND__
