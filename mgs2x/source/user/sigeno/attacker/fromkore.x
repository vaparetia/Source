/*
/user/korekado内関数 enemy.xに記述の無い物
K.Sigeno
$Id: fromkore.x,v 1.1.1.3 2002/11/19 11:49:03 Yoshizawa1 Exp $
*/
//attacker/atsneak.c
extern void ENE_Attacker_Think1_Normal( ENETHINK * ) ;
//noitice/notice.c
extern void ENE_NoticeCheck( ENETHINK * );
//attacker/enedamag.c
extern void ENE_EnemyStartModeDamage( ENETHINK  * );
extern void ENE_Enemy_Think1_Damage( ENETHINK  * );
//attacker/atsneak.c
extern void	ENE_AttackerStartModeSneak( ENETHINK * );
//enemy/enemy.c
extern void ENE_Gravitation( ENETHINK * );
//action/action.c
extern void AT_SetDurable( BODYPARAM *,int,int,int,int);
//enemy\enaction.c
extern void ENE_ActMedication( ACTION *, int ) ;

