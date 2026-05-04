/*
	header for print.c
	$Id: print.h,v 1.1 2002/07/12 04:10:58 usr01475 Exp $
*/

extern int verbose_mode;

void PRINTF( char *format, ... );		// verbose_mode only
void DUMP( char *format, ... );
void WARNING( char *format, ... );
void ERROR( char *format, ... );
void FATAL( char *format, ... );
