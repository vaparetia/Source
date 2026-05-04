#ifndef _photo_puppet_h_
#define _photo_puppet_h_

#define PHOTO_PUPPET_DefaultAction    0x0008a3fb

/*
 * シグナルの類
 */
#define PHOTO_SIGNAL_KILL_PUPPET   0x0003
#define PHOTO_SIGNAL_EXIT_PUPPET   0x0004

void * NewPhotoTermPuppet(int l2d_name, int init_action);
int    PHOTO_SetDefaultAction(int def_action,
			      int tierd_action, int tierd_time);
int    PHOTO_PuppetAction(int act_name, int a_time);
int    PHOTO_PuppetAck(void);

#endif /* _photo_puppet_h_ */
