#pragma once

#ifndef __GESTURE_VTA__H__
#define __GESTURE_VTA__H__

#ifdef __cplusplus
extern "C"
{
#endif

#define kGesture_InvalidHandle ((GestureRecognizerHandle)0)

   typedef int GestureRecognizerHandle;

   typedef struct
   {
      short mMinX;
      short mMinY;
      short mMaxX;
      short mMaxY;
   } STouchPanelDimensions;

   typedef enum
   {
      kGesture_TouchScreen_Back,
      kGesture_TouchScreen_Front,
      kGesture_TouchScreen_NumScreens,
   } ETouchScreen;

   typedef enum
   {
      kGesture_DeadZone_None,
      kGesture_DeadZone_Default,
   } EBackTouchDeadZone;

   typedef enum
   {
      kGesture_RecognizerType_Drag,
      kGesture_RecognizerType_Tap,
      kGesture_RecognizerType_Count,
   } ERecognizerType;

   typedef enum _EGesturePadRelease
   {
      kGPR_Direct,
      kGPR_Filtered,
      kGPR_AlwaysFail,
   } EGesturePadRelease;

   void GestureUpdate();
   void GestureShutdown();
   void GestureGetTouchPanelDimensions(STouchPanelDimensions* outputDimensions, int panel);

   /// The following set of functions are used for interacting with the touch screens in a simple nature such as polling
   /// for their current state.

   /// Gets a drag strength on the right side of either the front or back touch pad.
   float GestureGetRightDragAmount(ETouchScreen whichScreen, EGesturePadRelease pr, float const minVal, float const maxVal);
   float GestureGetRightDragAmountAbs(ETouchScreen whichScreen, EGesturePadRelease pr, float const minVal, float const maxVal);

   /// Gets a drag strength on the left side of either the front or back touch pad.
   float GestureGetLeftDragAmount(ETouchScreen whichScreen, EGesturePadRelease pr, float const minVal, float const maxVal);
   float GestureGetLeftDragAmountAbs(ETouchScreen whichScreen, EGesturePadRelease pr, float const minVal, float const maxVal);

   /// Returns either -1, 1 or 0 depending on which direction (down, up, no touch) the user is dragging.
   int GestureGetRightDragDir(ETouchScreen whichScreen, EGesturePadRelease pr );

   /// The trigger buttons on other platforms are simulated using left/right drag gestures on the touch pad.
   unsigned int GestureGetTriggerButtonMask(ETouchScreen whichScreen, EGesturePadRelease pr);

   /// Sets the border on the back touch screen.
   void GestureConfigureDeadZone(EBackTouchDeadZone const deadZone);

   /// Gets the current state of the front touch screen.
   int GestureGetFrontTouchPressed(EGesturePadRelease pr);
   int GestureGetFrontTouchReleased(EGesturePadRelease pr);
   void GestureGetFrontTouchPressedPos(float* pX, float* pY, float const rangeScaler);
   int GestureGetFrontTouchTapped(EGesturePadRelease pr);
   void GestureResetFrontTouchTappedStatus();
   int GestureGetFrontDragging(EGesturePadRelease pr);

   /// Gets the current state of the back touch screen.
   int GestureGetBackTouchPressed(EGesturePadRelease pr);
   int GestureGetBackTouchReleased(EGesturePadRelease pr);
   int GestureGetBackTouchTapped(EGesturePadRelease pr);
   void GestureResetBackTouchTappedStatus();

   /// In cases where custom handling of gesture data is required, these functions can be used to get a handle to a 
   /// particular touch recognizer for custom gesture handling on the front touch screen.
   GestureRecognizerHandle GestureAllocRecognizerHandle(ERecognizerType recognizerType);
   void GestureReleaseRecognizerHandle(int handle);
   struct SceSystemGestureTouchRecognizer* GestureGetRecognizerByHandle(GestureRecognizerHandle handle);
   
   // Returns 1 if you should abort any gesture operation based on the PAD status
   int GestureShouldAbort( EGesturePadRelease pr );

#ifdef __cplusplus
};
#endif

#endif // _GESTURE_VTA__H__