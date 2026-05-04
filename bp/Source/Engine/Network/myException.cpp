#include "StdAfx.h"
#include "myException.h"
//#include "iostream"

using namespace std;

myException::myException(int errCode,const string& errMsg)
{
	initVars();
	errorCode = errCode;
	if ( errMsg[0] ) errorMsg.append(errMsg);
}

void myException::initVars()
{
	errorCode = 0;
	errorMsg = "";
}

void myException::response()
{
   printf("Error detect: \n");
	printf("		==> error code: %d\n", errorCode);
	printf("		==> error message: %s\n", errorMsg.c_str());
}