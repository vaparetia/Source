#ifndef _cdc_mind_h_
#define _cdc_mind_h_

void   CDC_KillMindVoice(void);                      /* 心の声を殺す         */
void   CDC_SetupMindVoice(int side, int * vox_array);/* vox を設定           */
void   CDC_MindVoicePressure(int * press_levels);    /* ボタン押下強度の設定 */
void * NewCodecMindVoice(void);                      /* 心の声 Actor 起動    */

#endif /* _cdc_mind_h_ */
