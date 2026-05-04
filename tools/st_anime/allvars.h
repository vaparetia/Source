#ifndef __VARS_H__
#define __VARS_H__


#define ANM_SCALE    100.0f

/* list.c */
extern void init_varlist( int i ) ;
extern void add_to_varlist( VAR *a ) ;
extern void free_varlist() ;
extern void reset_varlist() ;
extern int get_fvec_sub_vert( short *svec, FVECTOR *fvec ) ;
extern float get_nth_float_from_varlist( int n ) ;
extern void get_fvec_from_varlist( FVECTOR *a ) ;
extern void get_poly_from_varlist( POLY *a ) ;
extern float get_float_from_varlist() ;

extern LIST *newnode_to_list( char *name ) ;
extern LIST *search_all_list( char *name ) ;
extern void delete_all_list() ;

/* misc.c */
extern void apply_matrix( FVECTOR *o, FMATRIX *m, FVECTOR *i ) ;

/* create.c */
extern int make_datas() ;

/* output.c */
//extern int output_anm( char *name, char *cv2_name ) ;
extern int output_anm( char *xsi_name, CV2_OBJS *c ) ;

/* xsi.y -> y.tab.c */
extern int yyparse() ;

/* main.c */
extern int open_file_with_ext( FILE **fp, char *src, char *ext, char *mode ) ;


#ifdef __MAIN_FILE__

extern FILE *yyin  ;
extern int (*post_actions[])( char *name ) ;
extern int (*pre_actions[])( char *name ) ;

FVECTOR *patch = NULL ;
int      n_patch = 0  ;
FVECTOR *mesh  = NULL ;
int      n_mesh  = 0  ;
ANIM    *anime = NULL ;
int      n_anime = 0  ;

ANIM *param = NULL ;
char *param_mode ; /* SCENE must be ignored*/

LIST *list    = NULL ;
LIST *current = NULL ;
int   n_frame = 0    ;
int   n_verts = 0    ;
int   scaling = ANM_SCALE ;
int   skip_frame = 0 ;
int   flag         ;
int   lines   = 1    ;



#else

extern FILE *yyin ;
extern int (*post_actions[])( char *name ) ;
extern int (*pre_actions[])( char *name ) ;

extern FVECTOR *patch   ;
extern int      n_patch ;
extern FVECTOR *mesh    ;
extern int      n_mesh  ;
extern ANIM    *anime   ;
extern int      n_anime ;

extern ANIM *param   ;
extern char *param_mode ; /* SCENE must be ignored*/

extern LIST *list    ;
extern LIST *current ;
extern int   n_frame ;
extern int   n_verts ;
extern int   scaling ;
extern int   skip_frame ;

extern int   flag    ;
extern int   lines   ;

#endif  /*__MAIN_FILE__ */


#endif  /*__VARS_H__*/




