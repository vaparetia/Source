/******************************************************************************
 * System	: DemoAct
 * Computer : DOS/V or UNIX
 * OS		: Microsoft Windows or UNIX or LIUNX
 * Compiler : Microsoft Visual C++ or cc or gcc
 * Module   : Kelnel
 */

/******************************************************************************
 * included
 */

#include <stdio.h>
#include <stdlib.h>

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

#include "RunCommand.h"

/******************************************************************************
 * functions
 */

/******************************************************************************
 * publics
 */
char* lppMakeCommand[] = {
	"cd scn; make;",
	"cd scn; make stage;",
	"cd source; make link;",
	"cd source; make go;",
	"" };

char* lppMakeLinkOneStage[] = {
	"cd source; make MGS2STAGE=",
	" link;",
	"" };

enum {
	RUN_SCN,
	RUN_STAGE,
	RUN_LINK,
	RUN_GO,
	};

/******************************************************************************
 * make などを行う
 */
int RunCommand(			/* 0 : 異常 */
	int   nMakeType,		/* make type */
	char* lpszTarget)
{
	int  i;
	char szCommand[_MAX_TEXT];

	switch(nMakeType)
	{
	case MAKE_TYPE_ALL:
		/* make scn */
		if(system(lppMakeCommand[RUN_SCN]) != 0)
			return 0;

		/* make stage */
		if(system(lppMakeCommand[RUN_STAGE]) != 0)
			return 0;

		/* make link */
		if(system(lppMakeCommand[RUN_LINK]) != 0)
			return 0;

		/* make go */
		if(system(lppMakeCommand[RUN_GO]) != 0)
			return 0;
		break;

	case MAKE_TYPE_ALL_ONE_STAGE:
		/* make scn */
		if(system(lppMakeCommand[RUN_SCN]) != 0)
			return 0;

		/* make stage */
		if(system(lppMakeCommand[RUN_STAGE]) != 0)
			return 0;

		/* make link */
		sprintf(szCommand, "%s\"init %s\"%s", lppMakeLinkOneStage[0], lpszTarget,
			lppMakeLinkOneStage[1]);
		if(system(szCommand) != 0)
			return 0;

		/* make go */
		if(system(lppMakeCommand[RUN_GO]) != 0)
			return 0;
		break;

	case MAKE_TYPE_SCN:
		/* make scn */
		printf("mgdemo - %s\n", lppMakeCommand[RUN_SCN]);
		if(system(lppMakeCommand[RUN_SCN]) != 0)
			return 0;
		break;

	case MAKE_TYPE_STAGE:
		/* make stage */
		printf("mgdemo - %s\n", lppMakeCommand[RUN_STAGE]);
		if(system(lppMakeCommand[RUN_STAGE]) != 0)
			return 0;
		break;

	case MAKE_TYPE_LINK:
		/* make link */
		printf("mgdemo - %s\n", lppMakeCommand[RUN_LINK]);
		if(system(lppMakeCommand[RUN_LINK]) != 0)
			return 0;
		break;

	case MAKE_TYPE_LINK_ONE_STAGE:
		/* make link */
		sprintf(szCommand, "%s\"init %s\"%s", lppMakeLinkOneStage[0], lpszTarget,
			lppMakeLinkOneStage[1]);
		if(system(szCommand) != 0)
			return 0;
		break;

	case MAKE_TYPE_GO:
		/* make go */
		if(system(lppMakeCommand[RUN_GO]) != 0)
			return 0;
		break;

	default:
		printf("No Command.\n");
		break;
	}

	return 1;
}
