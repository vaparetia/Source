#include <sys/types.h>
#include "fmt_km4.h"

/* Enumuration */
enum
{
    X=0, Y, Z, XY=Z, W, XYZ=W, XYZW
} ;


enum {
    FLG_NORMAL = 0x00000001,
    FLG_VERTEX = 0x00000002,
} ;


typedef struct { float m[4][4] ; } FMATRIX ;

typedef struct anim_t ANIM ;
typedef union  var_t  VAR  ;
typedef struct list_t LIST ;
typedef struct poly_t POLY ;

struct anim_t
{
    int      vert_id ;
    FVECTOR *org   ;
    short   *verts ;
    short   *norms ;
} ;

union var_t
{
    float f ;
    char *s ;
    int   i ;
    VAR  *v ;
} ;

struct poly_t
{
    int     v[4] ;
    FVECTOR n    ;
} ;

struct list_t
{
    LIST    *next    ;
    LIST    *prev    ;

    char     name[64];

    FMATRIX  matrix  ;
    FVECTOR *patch   ;
    int      n_patch ;
    FVECTOR *mesh    ;
    int      n_mesh  ;
    POLY    *poly    ;
    int      n_poly  ;
    ANIM    *anime   ;
    int      n_anime ;
    ANIM    *para    ;
} ;
