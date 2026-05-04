#ifndef _codecmtn_h_
#define _codecmtn_h_

#define MAR_MAGIC_ID  (0x6152414d)  /* "MARa" */

void       * CDC_InitSequence(int n_layer, int id, int flag);
void         CDC_FreeSequence(SAR_CONTROL * sar_ctrl);
MAR_HEADER * CDC_GetMotionArchives(int motion_id);
void       * CDC_InitMotion(DG_OBJS * objs, int n_layer, int motion, int flag);
void         CDC_FreeMotion(MOTION_CONTROL * m_ctrl);
void         CDC_SetMotionData(MOTION_CONTROL * m_ctrl,
			       int layer, int motion, int time, u_long64 mask);

#endif /* _codecmtn_h_ */
