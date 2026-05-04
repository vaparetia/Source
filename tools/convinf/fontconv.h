/*
	fontconv.h
		1999/12/20	K.Uehara
		$Id: fontconv.h,v 1.2 2002/04/05 07:29:44 usr01475 Exp $
*/

void font_set_load_path( char *path );
void font_init_font_file( char *all_font_file );
void font_load_table( char *filename );
void font_conv_buffer( char *dest, char *src );
void font_output_font_data( FILE *fp );

void font_reset_font_table( void );
int font_get_data_size( void );
