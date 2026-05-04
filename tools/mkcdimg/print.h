/*
	header for print.c
	$Id: print.h,v 1.2 2000/07/07 09:15:46 usr01475 Exp $
*/

extern int verbose_mode;

void PRINTF( char *format, ... );		// verbose_mode only
void DUMP( char *format, ... );
void WARNING( char *format, ... );
void ERROR( char *format, ... );
void FATAL( char *format, ... );
