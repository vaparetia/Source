/******************************************************************************
 * System	: Merge Scene exe
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Kelnel
 */

/******************************************************************************
 * included
 */

#include <stdio.h>

#ifdef _WINDOWS
#include <windows.h>
#include <SFDWIN.h>
#endif
#ifdef _UNIX
#include <SFDUNIX.h>
#endif
#ifdef _LINUX
#include <SFDLINUX.h>
#endif

#include "PrintHelpMenu.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */

void PrintHelpMenu()
{
	printf("\nMGS2 Demo Command Reference.   Ver.1.0\n");
	printf("\n    mgdemo [target] [stage] ([option])\n");
	printf("\n");
	printf("        target  : Demo file name.\n");
	printf("        stage   : Play Demo stage name\n");
	printf("        option  : Nothing        : Creates environment, and executes Demo.\n");
	printf("                  \'loop\'         : Loop mode.\n");
	printf("                  \'-s [\"TEXT\"] \' : Text Output mode.\n");
	printf("                  \'-e [time] \'   : View Effect List.\n");
	printf("                  \'-v [status] \' : View mode start status.\n");
	printf("                  \'-f [frame] \'  : Base frame count.\n");
	printf("                  \'-t [second] \' : Start timing count.\n");
	printf("\n    example...\n");
	printf("        >mgdemo s0001a\n");
	printf("            >>> make scn, make link, make go.\n");
}
