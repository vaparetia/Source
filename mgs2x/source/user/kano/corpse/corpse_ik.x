/*
	corpse_ik.x
		死体の姿勢制御を外部で使用する場合の、
		関数のプロトタイプ宣言

	1999/10/5 K.Kano
	$Id: corpse_ik.x,v 1.1.1.3 2002/11/19 11:43:09 Yoshizawa1 Exp $
*/


#ifndef _corpse_ik_x_
#define _corpse_ik_x_

void *StartCorpseIK(CONTROL *ctrl,OBJECT *obj);
void ExitCorpseIK(void *work);
void Corpse_ChangeAction(void *work);

void PullCorpseWithShoulder(void *work);
void PullCorpseWithLegs(void *work);
int FreeCorpse(void *work);
void HoldFreeCorpse(void *work);
void DummyFreeCorpse(void *work);
void JumpFreeCorpse(void *work);
void JumpFreeCorpse2(void *work,int objnum);
void BackFromFreeCorpse(void *work);
int AfterJumpCorpse(void *work);

#endif
