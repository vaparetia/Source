#ifndef _SENSER_H_
#define _SENSER_H_

extern int InitSenser( Work * , int , int );
extern void Init_RayUvrgbScrPad( SenserRay * ); // センサー初期化
extern void Init_RayPosScrPad( Work * );
extern void InitSensorRay_Bound(FVECTOR * , SenserRay  * );
extern int InitSensorRay( Work * );
extern void Move_Ray( Work * );
extern int Check_Touch_Laser( Work * , FVECTOR * );
extern int SENSORRAY_Act( Work * );

#endif