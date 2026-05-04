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
	printf("\nMGS2 Demo Command Reference.   Ver.4.2\n");
	printf("\n    mgdemo [target] [stage] ([option])\n");
	printf("\n");
	printf("        target  : Demo file name.\n");
	printf("        stage   : Play Demo stage name\n");
	printf("        option  : Nothing            : Creates environment, and executes Demo.\n");
	printf("                  \'loop\'             : Loop mode.\n");
	printf("                  \'insthd\'           : Make HD module. Set HD format.\n");
	printf("                  \'insthd_no_format\' : Make HD module..\n");
	printf("                  \'-s [\"TEXT\"] \'     : Text Output mode.\n");
	printf("                  \'-e [time] \'       : View Effect List.\n");
	printf("                  \'-v [status] \'     : View mode start status.\n");
	printf("                  \'-f [frame] \'      : Base frame count.\n");
	printf("                  \'-t [second] \'     : Start timing count.\n");
	printf("                  \'-p [0 or 1] \'     : PCM Stream. 0:off 1:on\n");
	printf("                  \'-a [filename] \'   : file name\n");
	printf("\n    example...\n");
	printf("        >mgdemo t00a1D d00a\n");
	printf("            >>> make stream, make scn make link, make go.\n");
}
