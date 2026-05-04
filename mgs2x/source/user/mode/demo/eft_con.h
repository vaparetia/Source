#ifndef EFT_CON_H
#define EFT_CON_H
/* ----------------------------------------------------------------- */
    /*
	    デモエフェクトコントロール構造体
	*/
typedef ALIGN16_DECL(struct) _EftControl {
	FVECTOR	mov;
	SVECTOR	rot;
	int		name;
	struct _EftControl *next;
} EFTCONTROL ;

/* eft_con.c */
extern void DM_EftControlClearList( void );
extern void DM_EftControlAddList( int name, EFTCONTROL *add );
extern void DM_EftControlDelList( EFTCONTROL *del );
extern EFTCONTROL *DM_GetEftControl( int name );

/* demo_eft.c */
extern void  DM_EftControlMatrix( EFTCONTROL *eft_con, FMATRIX *world );
#endif
