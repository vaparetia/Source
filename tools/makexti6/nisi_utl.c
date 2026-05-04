/*
	nisi_utl.c

	2002/04/29
	NISINO Motoaki
	
	$ld$
*/

#include <stdio.h>
#include <stdarg.h>


void my_assert_disp(char *file_name, int line, const char *format, ...)
{

	const int BUF_MAX = 0xff;
	char buf[BUF_MAX];
	int ret;
	

	printf("%s (%d)\n", file_name, line);

	if (format != NULL) {
		va_list	ap;
		va_start(ap, format);
		//ret = vsprintf(buf,  format, ap);
		ret = vsnprintf(buf, BUF_MAX, format, ap);
		va_end(ap);
	}
	printf("%s",buf);
	
}


void my_assert_disp2(const char *format, ...)
{

	const int BUF_MAX = 0xff;
	char buf[BUF_MAX];
	int ret;
	
	if (format != NULL) {
		va_list	ap;
		va_start(ap, format);
		ret = vsnprintf(buf, BUF_MAX, format, ap);
		va_end(ap);
	}
	printf("%s",buf);
	
}

