#include <stdio.h>
#include <string.h>
#include <vector>

#include "Engine/Basics/BPEEnvironment.h"
#include "Engine/StlExtras/BPEStlExtras.h"

#include <systemgesture.h>
#include <sceerror.h>
#include "BP_BuildDefines.h"
#include "Engine/System/VTACOsContext.h"

#include "gesture_vta.h"

#if MGS_VERSION == 2
#include "libgv.h"
#else
#include "pad.h"
#define PAD_L2 _PAD_L2
#define PAD_R2 _PAD_R2
#endif

namespace
{
   enum 
   {
      kGesture_RecognizerPoolSize = 8,
   };

   struct SRecognizerEntry
   {
      SRecognizerEntry()
         : mVersion(0)
         , mIsInUse(false)
      {
         memset(&mRecognizer, 0, sizeof(mRecognizer));
      }

      SceSystemGestureTouchRecognizer mRecognizer;
      uint16 mVersion;
      bool mIsInUse;
   };

   struct STouchGesture
   {
      // Pool of touch recognizers used by the game for custom purposes.
      SRecognizerEntry mRecognizerPool[kGesture_RecognizerType_Count][kGesture_RecognizerPoolSize];

      SceSystemGestureTouchRecognizer mTapRecognizerBackTouch;

      // Back touch drag is used for two different gestures. Dragging from the center outward to the right and dragging
      // from the center outward to the left.
      SceSystemGestureTouchRecognizer mDragRecognizerBackTouch;
      SceSystemGestureVector2 mBackTouchDeltaDrag[2]; // One for left side of screen, one for right side of screen.
      SceInt32 mBackTouchDragEventId[2]; // One for left screen drag, one for right screen drag

      SceSystemGestureTouchRecognizer mDragRecognizerFrontTouch;
      SceInt16 mFrontTouchDragEventId;
      SceUInt64 mFrontTouchLastEventTime;
      SceSystemGestureVector2 mFrontTouchPos;
      SceSystemGestureVector2 mFrontTouchDeltaDrag;
      SceSystemGestureVector2 mFrontTouchDeltaDragAbs;

      SceSystemGestureTouchRecognizer mTapRecognizerFrontTouch;

      SceSystemGestureVector2 mBackPanelDims; // x is width, y is height
      SceSystemGestureVector2 mBackPanelCenter;

      SceSystemGestureVector2 mFrontPanelDims; // x is width, y is height

      unsigned int mButtonMask[kGesture_TouchScreen_NumScreens];

      bool mBackTouchPressed;
      bool mBackTouchReleased;
      bool mBackTouchTapped;

      bool mFrontTouchPressed;
      bool mFrontTouchReleased;
      bool mFrontTouchTapped;
      bool mFrontTouchDragging;
   };

   int const kButtonPressDragThreshold = 50;
   int gGestureInitialized = FALSE;
   int gGestureReinitialize = FALSE;
   int gSystemResumeCount = 0;

   // Back touch is 1919 units wide. To prevent the user from accidentally activating the rear touch pad controls, add an 
   // artificial dead-zone around the edges.
   SceSystemGestureVector2 const gkPanelBorderOffset[SCE_TOUCH_PORT_MAX_NUM] = 
   {
      { 0, 0 },
      { 100, 0 },
   };

   STouchGesture gTouchGesture;

   class CTouchPanelDimensions
   {
   public:
      CTouchPanelDimensions()
      {
         SceTouchPanelInfo panelInfo;

         for(size_t i = SCE_TOUCH_PORT_FRONT; i <= SCE_TOUCH_PORT_MAX_NUM; ++i)
         {
            sceTouchGetPanelInfo( i, &panelInfo );

            mDimensionData[i].mMinX = panelInfo.minAaX + gkPanelBorderOffset[i].x;
            mDimensionData[i].mMinY = panelInfo.minAaY + gkPanelBorderOffset[i].y;
            mDimensionData[i].mMaxX = panelInfo.maxAaX - gkPanelBorderOffset[i].x;
            mDimensionData[i].mMaxY = panelInfo.maxAaY - gkPanelBorderOffset[i].y;
         }
      }

      inline const STouchPanelDimensions& GetTouchPanelDimensions(int panel) const
      {
         return mDimensionData[panel];
      }

   private:
      STouchPanelDimensions mDimensionData[SCE_TOUCH_PORT_MAX_NUM];
   };
}

extern "C"
{
   void gesture_initialize()
   {
      if (!gGestureInitialized)
      {
         gGestureInitialized = TRUE;
         gSystemResumeCount = OsContext()->GetSystemResumeCount();
         gGestureReinitialize = FALSE;

         sceSystemGestureInitializePrimitiveTouchRecognizer(NULL);

         sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
         sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_START);

         memset(&gTouchGesture, 0, sizeof(gTouchGesture));

         STouchPanelDimensions panelDims;
         GestureGetTouchPanelDimensions(&panelDims, SCE_TOUCH_PORT_BACK);
         gTouchGesture.mBackPanelDims.x = panelDims.mMaxX - panelDims.mMinX;
         gTouchGesture.mBackPanelDims.y = panelDims.mMaxY - panelDims.mMinY;
         gTouchGesture.mBackPanelCenter.x = panelDims.mMinX + (gTouchGesture.mBackPanelDims.x / 2);
         gTouchGesture.mBackPanelCenter.y = panelDims.mMinY + (gTouchGesture.mBackPanelDims.y / 2);

         // Create a back touch drag recognizer to always keep around. This drag recognizer is used throughout the game
         // to simulate analog trigger button presses.
         SceSystemGestureRectangle backTouchRect = 
         { 
            panelDims.mMinX, 
            panelDims.mMinY, 
            panelDims.mMaxX - panelDims.mMinX, 
            panelDims.mMaxY - panelDims.mMinY 
         };

         GestureGetTouchPanelDimensions(&panelDims, SCE_TOUCH_PORT_FRONT);
         gTouchGesture.mFrontPanelDims.x = panelDims.mMaxX - panelDims.mMinX;
         gTouchGesture.mFrontPanelDims.y = panelDims.mMaxY - panelDims.mMinY;

         gTouchGesture.mBackTouchDragEventId[0] = -1;
         gTouchGesture.mBackTouchDragEventId[1] = -1;
         gTouchGesture.mFrontTouchDragEventId = -1;
         gTouchGesture.mFrontTouchLastEventTime = 0;

         // Use the entire front touch pad.
         sceSystemGestureCreateTouchRecognizer(&gTouchGesture.mDragRecognizerFrontTouch, SCE_SYSTEM_GESTURE_TYPE_DRAG,
            SCE_TOUCH_PORT_FRONT, NULL, NULL);

         sceSystemGestureCreateTouchRecognizer(&gTouchGesture.mTapRecognizerFrontTouch, SCE_SYSTEM_GESTURE_TYPE_TAP,
            SCE_TOUCH_PORT_FRONT, NULL, NULL);

         // Put an artificial border in the rear touchpad to prevent users from accidentally dragging or touching the 
         // touch pad edges.
         sceSystemGestureCreateTouchRecognizer(&gTouchGesture.mDragRecognizerBackTouch, SCE_SYSTEM_GESTURE_TYPE_DRAG, 
            SCE_TOUCH_PORT_BACK, &backTouchRect, NULL);

         SceSystemGestureTouchRecognizerParameter tapAndHoldParam;
         tapAndHoldParam.tapAndHold.timeToInvokeEvent = 30;         
         sceSystemGestureCreateTouchRecognizer(&gTouchGesture.mTapRecognizerBackTouch, SCE_SYSTEM_GESTURE_TYPE_TAP_AND_HOLD,
            SCE_TOUCH_PORT_BACK, &backTouchRect, &tapAndHoldParam);

         for (int currRecognizerType = 0; currRecognizerType < kGesture_RecognizerType_Count; currRecognizerType++)
         {
            for (int currPoolEntry = 0; currPoolEntry < kGesture_RecognizerPoolSize; currPoolEntry++)
            {
               SRecognizerEntry* pEntry = &gTouchGesture.mRecognizerPool[currRecognizerType][currPoolEntry];

               switch (currRecognizerType)
               {
               case kGesture_RecognizerType_Drag:
                  sceSystemGestureCreateTouchRecognizer(&pEntry->mRecognizer, SCE_SYSTEM_GESTURE_TYPE_DRAG, SCE_TOUCH_PORT_FRONT, NULL, NULL);
                  break;
               case kGesture_RecognizerType_Tap:
                  sceSystemGestureCreateTouchRecognizer(&pEntry->mRecognizer, SCE_SYSTEM_GESTURE_TYPE_TAP, SCE_TOUCH_PORT_FRONT, NULL, NULL);
                  break;
               default:
                  BPE_ASSERT(false, "Invalid gesture type.");
                  break;
               }
            }
         }
      }

      // Armature fix:
      // when suspending and resuming, the touch recognizers can get into a broken state
      // reset them
      // fix for MGSTHREE-3126
      int systemResumeCount = OsContext()->GetSystemResumeCount();
      if (gGestureReinitialize || gSystemResumeCount != systemResumeCount)
      {
         sceSystemGestureResetTouchRecognizer(&gTouchGesture.mDragRecognizerFrontTouch);
         sceSystemGestureResetTouchRecognizer(&gTouchGesture.mTapRecognizerFrontTouch);
         sceSystemGestureResetTouchRecognizer(&gTouchGesture.mDragRecognizerBackTouch);
         sceSystemGestureResetTouchRecognizer(&gTouchGesture.mTapRecognizerBackTouch);
         for (int currRecognizerType = 0; currRecognizerType < kGesture_RecognizerType_Count; currRecognizerType++)
         {
            for (int currPoolEntry = 0; currPoolEntry < kGesture_RecognizerPoolSize; currPoolEntry++)
            {
               sceSystemGestureResetTouchRecognizer(&gTouchGesture.mRecognizerPool[currRecognizerType][currPoolEntry].mRecognizer);
            }
         }

         // Arm fix:
         // reset other variables
         // fix for MGSTWO-3275
         gTouchGesture.mFrontTouchDragEventId = -1;
         gTouchGesture.mFrontTouchLastEventTime = 0;

         // left and right
         gTouchGesture.mBackTouchDragEventId[0] = -1;
         gTouchGesture.mBackTouchDragEventId[1] = -1;
         gTouchGesture.mBackTouchDeltaDrag[0].x = 0;
         gTouchGesture.mBackTouchDeltaDrag[0].y = 0;
         gTouchGesture.mBackTouchDeltaDrag[1].x = 0;
         gTouchGesture.mBackTouchDeltaDrag[1].y = 0;

         // front and back
         gTouchGesture.mButtonMask[0] = 0;
         gTouchGesture.mButtonMask[1] = 0;

         gTouchGesture.mBackTouchPressed  = false;
         gTouchGesture.mBackTouchReleased = false;
         gTouchGesture.mBackTouchTapped   = false;

         gTouchGesture.mFrontTouchPressed  = false;
         gTouchGesture.mFrontTouchReleased = false;
         gTouchGesture.mFrontTouchTapped   = false;
         gTouchGesture.mFrontTouchDragging = false;

         gSystemResumeCount = systemResumeCount;
         gGestureReinitialize = FALSE;
      }
   }

   void update_backtouch_recognizer()
   {
      // Check dragging gestures...

      int result = sceSystemGestureUpdateTouchRecognizer(&gTouchGesture.mDragRecognizerBackTouch);
      if (result != SCE_OK)
      {
         gGestureReinitialize = TRUE;
         return;
      }

      SceSystemGestureTouchEvent touchEvent;
      int eventCount = sceSystemGestureGetTouchEventsCount(&gTouchGesture.mDragRecognizerBackTouch);
      int center = gTouchGesture.mBackPanelCenter.x;

      for (int i = 0; i < eventCount; i++)
      {
         result = sceSystemGestureGetTouchEventByIndex(&gTouchGesture.mDragRecognizerBackTouch, i, &touchEvent);
         if (result != SCE_OK)
         {
            gGestureReinitialize = TRUE;
            return;
         }
         switch (touchEvent.eventState)
         {
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_BEGIN:
            if (touchEvent.property.drag.pressedPosition.x < center && gTouchGesture.mBackTouchDragEventId[0] == -1)
               gTouchGesture.mBackTouchDragEventId[0] = touchEvent.eventID;

            if (touchEvent.property.drag.pressedPosition.x > center && gTouchGesture.mBackTouchDragEventId[1] == -1)
               gTouchGesture.mBackTouchDragEventId[1] = touchEvent.eventID;
            break;

         case SCE_SYSTEM_GESTURE_TOUCH_STATE_ACTIVE:
            if (gTouchGesture.mBackTouchDragEventId[0] == touchEvent.eventID)
            {
               gTouchGesture.mBackTouchDeltaDrag[0].x += touchEvent.property.drag.deltaVector.x;
               gTouchGesture.mBackTouchDeltaDrag[0].y += touchEvent.property.drag.deltaVector.y;
            }
            else if (gTouchGesture.mBackTouchDragEventId[1] == touchEvent.eventID)
            {
               gTouchGesture.mBackTouchDeltaDrag[1].x += touchEvent.property.drag.deltaVector.x;
               gTouchGesture.mBackTouchDeltaDrag[1].y += touchEvent.property.drag.deltaVector.y;
            }
            break;

         case SCE_SYSTEM_GESTURE_TOUCH_STATE_END:
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_CANCELLED:
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_INACTIVE:
            if (touchEvent.eventID == gTouchGesture.mBackTouchDragEventId[0])
            {
               gTouchGesture.mBackTouchDeltaDrag[0].x = 0;
               gTouchGesture.mBackTouchDeltaDrag[0].y = 0;
               gTouchGesture.mBackTouchDragEventId[0] = -1;
            }
            else if (touchEvent.eventID == gTouchGesture.mBackTouchDragEventId[1])
            {
               gTouchGesture.mBackTouchDeltaDrag[1].x = 0;
               gTouchGesture.mBackTouchDeltaDrag[1].y = 0;
               gTouchGesture.mBackTouchDragEventId[1] = -1;
            }
            break;
         }
      }

      // rear drag is used for simulating the trigger button presses.
      gTouchGesture.mButtonMask[kGesture_TouchScreen_Back] = 0;

      // If the user drags vertically on the left side of the rear touch pad the drag amount is based on the vertical 
      // delta and both trigger buttons are considered pressed.
      int absXDeltaL = std::abs(gTouchGesture.mBackTouchDeltaDrag[0].x);
      int absYDeltaL = std::abs(gTouchGesture.mBackTouchDeltaDrag[0].y);

      int absXDeltaR = std::abs(gTouchGesture.mBackTouchDeltaDrag[1].x);
      int absYDeltaR = std::abs(gTouchGesture.mBackTouchDeltaDrag[1].y);

      if (absYDeltaL > absXDeltaL && absYDeltaL > (kButtonPressDragThreshold / 2))
      {
         gTouchGesture.mButtonMask[kGesture_TouchScreen_Back] |= PAD_L2;
      }
      else if (gTouchGesture.mBackTouchDeltaDrag[0].x <= -kButtonPressDragThreshold)
      {
         gTouchGesture.mButtonMask[kGesture_TouchScreen_Back] |= PAD_L2;
      }

      if (absYDeltaR > absXDeltaR && absYDeltaR > (kButtonPressDragThreshold / 2))
      {
         gTouchGesture.mButtonMask[kGesture_TouchScreen_Back] |= PAD_R2;
      }
      else if (gTouchGesture.mBackTouchDeltaDrag[1].x > kButtonPressDragThreshold)
      {
         gTouchGesture.mButtonMask[kGesture_TouchScreen_Back] |= PAD_R2;
      }

      // Check for tap..

      result = sceSystemGestureUpdateTouchRecognizer(&gTouchGesture.mTapRecognizerBackTouch);
      if (result != SCE_OK)
      {
         // try to reset
         gGestureReinitialize = TRUE;
         return;
      }
      eventCount = sceSystemGestureGetTouchEventsCount(&gTouchGesture.mTapRecognizerBackTouch);
      for (int i = 0; i < eventCount; i++)
      {
         result = sceSystemGestureGetTouchEventByIndex(&gTouchGesture.mTapRecognizerBackTouch, i, &touchEvent);
         if (result != SCE_OK)
         {
            // try to reset
            gGestureReinitialize = TRUE;
            return;
         }
         switch (touchEvent.eventState)
         {
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_BEGIN:

            gTouchGesture.mBackTouchPressed = true;
            gTouchGesture.mBackTouchReleased = false;
            gTouchGesture.mBackTouchTapped = true;
            break;

         case SCE_SYSTEM_GESTURE_TOUCH_STATE_END:
            gTouchGesture.mBackTouchReleased = true;
            gTouchGesture.mBackTouchPressed = false;
            break;
         }
      }
   }

   void update_fronttouch_recognizer()
   {
      int const kDragThreshold = 0;
      SceSystemGestureTouchEvent touchEvent;

      // Update the front drag gesture...
      int result = sceSystemGestureUpdateTouchRecognizer(&gTouchGesture.mDragRecognizerFrontTouch);
      if (result != SCE_OK)
      {
         // try to reset
         gGestureReinitialize = TRUE;
         return;
      }

      int eventCount = sceSystemGestureGetTouchEventsCount(&gTouchGesture.mDragRecognizerFrontTouch);
      for (int i = 0; i < eventCount; i++)
      {
         result = sceSystemGestureGetTouchEventByIndex(&gTouchGesture.mDragRecognizerFrontTouch, i, &touchEvent);
         if (result != SCE_OK)
         {
            // try to reset
            gGestureReinitialize = TRUE;
            return;
         }
         switch (touchEvent.eventState)
         {
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_BEGIN:
            if (gTouchGesture.mFrontTouchDragEventId == -1)
            {
               gTouchGesture.mFrontTouchDragEventId = touchEvent.eventID;
               gTouchGesture.mFrontTouchLastEventTime = 0;
               gTouchGesture.mFrontTouchPos = touchEvent.property.drag.pressedPosition;
               gTouchGesture.mFrontTouchPressed = true;
               gTouchGesture.mFrontTouchReleased = false;
               gTouchGesture.mFrontTouchDragging = false;
            }
            break;

         case SCE_SYSTEM_GESTURE_TOUCH_STATE_ACTIVE:
            if (gTouchGesture.mFrontTouchDragEventId == touchEvent.eventID)
            {
               gTouchGesture.mFrontTouchPos = touchEvent.property.drag.currentPosition;

               if (bpe::abs(touchEvent.property.drag.deltaVector.x) > kDragThreshold || 
                  bpe::abs(touchEvent.property.drag.deltaVector.y) > kDragThreshold)
               {
                  gTouchGesture.mFrontTouchDragging = true;
               }

               gTouchGesture.mFrontTouchDeltaDrag.x += touchEvent.property.drag.deltaVector.x;
               gTouchGesture.mFrontTouchDeltaDrag.y += touchEvent.property.drag.deltaVector.y;

               if (gTouchGesture.mFrontTouchLastEventTime != touchEvent.updatedTime)
               {
                  gTouchGesture.mFrontTouchDeltaDragAbs.x = touchEvent.property.drag.deltaVector.x;
                  gTouchGesture.mFrontTouchDeltaDragAbs.y = touchEvent.property.drag.deltaVector.y;
               }

               gTouchGesture.mFrontTouchLastEventTime = touchEvent.updatedTime;
            }
            break;

         case SCE_SYSTEM_GESTURE_TOUCH_STATE_END:
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_CANCELLED:
         case SCE_SYSTEM_GESTURE_TOUCH_STATE_INACTIVE:
            if (touchEvent.eventID == gTouchGesture.mFrontTouchDragEventId)
            {
               gTouchGesture.mFrontTouchDragEventId = -1;
               gTouchGesture.mFrontTouchPressed = false;
               gTouchGesture.mFrontTouchReleased = true;
               gTouchGesture.mFrontTouchDragging = false;
               gTouchGesture.mFrontTouchDeltaDrag.x = 0;
               gTouchGesture.mFrontTouchDeltaDrag.y = 0;
               gTouchGesture.mFrontTouchDeltaDragAbs.x = 0;
               gTouchGesture.mFrontTouchDeltaDragAbs.y = 0;
            }
            break;
         }
      }

      // x drag is used for simulating the trigger button presses.
      gTouchGesture.mButtonMask[kGesture_TouchScreen_Front] = 0;
      if (gTouchGesture.mFrontTouchDeltaDrag.x <= -kButtonPressDragThreshold)
      {
         gTouchGesture.mButtonMask[kGesture_TouchScreen_Front] |= PAD_L2;
      }

      if (gTouchGesture.mFrontTouchDeltaDrag.x >= kButtonPressDragThreshold)
      {
         gTouchGesture.mButtonMask[kGesture_TouchScreen_Front] |= PAD_R2;
      }

      // Update the front tap...
      result = sceSystemGestureUpdateTouchRecognizer(&gTouchGesture.mTapRecognizerFrontTouch);
      if (result != SCE_OK)
      {
         // try to reset
         gGestureReinitialize = TRUE;
         return;
      }
      eventCount = sceSystemGestureGetTouchEventsCount(&gTouchGesture.mTapRecognizerFrontTouch);
      for (int i = 0; i < eventCount; i++)
      {
         result = sceSystemGestureGetTouchEventByIndex(&gTouchGesture.mTapRecognizerFrontTouch, i, &touchEvent);
         if (result != SCE_OK)
         {
            // try to reset
            gGestureReinitialize = TRUE;
            return;
         }
         if (touchEvent.eventState != SCE_SYSTEM_GESTURE_TOUCH_STATE_CANCELLED)
         {
            gTouchGesture.mFrontTouchPos = touchEvent.property.tap.position;
            gTouchGesture.mFrontTouchTapped = true;
         }
      }
   }

   void GestureUpdate()
   {
      gesture_initialize();

      int const kNumBuffers = 1;

      SceTouchData frontTouchData[kNumBuffers];
      SceTouchData backTouchData[kNumBuffers];

      // get the latest data
      int frontDataBufs = sceTouchPeek(SCE_TOUCH_PORT_FRONT, &frontTouchData[0], kNumBuffers);
      int backDataBufs = sceTouchPeek(SCE_TOUCH_PORT_BACK, &backTouchData[0], kNumBuffers);
      int numDataBufs = (frontDataBufs < backDataBufs) ? frontDataBufs : backDataBufs;

      int result;
      
      for (int i = 0; i < numDataBufs; ++i)
      {
         result = sceSystemGestureUpdatePrimitiveTouchRecognizer(&frontTouchData[i], &backTouchData[i]);
         if (result != SCE_OK)
         {
            // try to reset
            gGestureReinitialize = TRUE;
            return;
         }
      }

      for (int currType = 0; currType < kGesture_RecognizerType_Count; currType++)
      {
         for (int currEntry = 0; currEntry < kGesture_RecognizerPoolSize; currEntry++)
         {
            SRecognizerEntry* pEntry = &gTouchGesture.mRecognizerPool[currType][currEntry];

            if (pEntry->mIsInUse)
            {
               result = sceSystemGestureUpdateTouchRecognizer(&pEntry->mRecognizer);
               if (result != SCE_OK)
               {
                  // try to reset
                  gGestureReinitialize = TRUE;
                  return;
               }
            }
         }
      }

      update_backtouch_recognizer();
      update_fronttouch_recognizer();
   }

   void GestureShutdown()
   {
      sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_STOP);
      sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, SCE_TOUCH_SAMPLING_STATE_STOP);
   }

   void GestureGetTouchPanelDimensions( STouchPanelDimensions* outputDimensions, int panel )
   {
      static CTouchPanelDimensions panelDimensionsData;
      const STouchPanelDimensions& dimensions = panelDimensionsData.GetTouchPanelDimensions( panel );

      outputDimensions->mMinX = dimensions.mMinX;
      outputDimensions->mMinY = dimensions.mMinY;
      outputDimensions->mMaxX = dimensions.mMaxX;
      outputDimensions->mMaxY = dimensions.mMaxY;
   }

   void GestureConfigureDeadZone(EBackTouchDeadZone const deadZone)
   {
      STouchPanelDimensions panelDims;

      if (deadZone == kGesture_DeadZone_Default)
      {
         GestureGetTouchPanelDimensions(&panelDims, SCE_TOUCH_PORT_BACK);
      }
      else if (deadZone == kGesture_DeadZone_None)
      {
         SceTouchPanelInfo panelInfo;
         sceTouchGetPanelInfo(SCE_TOUCH_PORT_BACK, &panelInfo);

         panelDims.mMinX = panelInfo.minAaX;
         panelDims.mMinY = panelInfo.minAaY;
         panelDims.mMaxX = panelInfo.maxAaX;
         panelDims.mMaxY = panelInfo.maxAaY;
      }

      gTouchGesture.mBackPanelDims.x = panelDims.mMaxX - panelDims.mMinX;
      gTouchGesture.mBackPanelDims.y = panelDims.mMaxY - panelDims.mMinY;
      gTouchGesture.mBackPanelCenter.x = panelDims.mMinX + (gTouchGesture.mBackPanelDims.x / 2);
      gTouchGesture.mBackPanelCenter.y = panelDims.mMinY + (gTouchGesture.mBackPanelDims.y / 2);

      SceSystemGestureRectangle backTouchRect;
      backTouchRect.x = panelDims.mMinX;
      backTouchRect.y = panelDims.mMinY;
      backTouchRect.width = panelDims.mMaxX - panelDims.mMinX;
      backTouchRect.height = panelDims.mMaxY - panelDims.mMinY;

      sceSystemGestureUpdateTouchRecognizerRectangle(&gTouchGesture.mDragRecognizerBackTouch, &backTouchRect);
   }

   float GestureGetRightDragAmount(ETouchScreen whichScreen, EGesturePadRelease pr, float const minVal, float const maxVal)
   {
      float pressure = 0.0f;

      if ( GestureShouldAbort( pr ) )
      {
         return minVal;
      }

      if (whichScreen == kGesture_TouchScreen_Front)
      {
         // When using the front touch for simulating analog trigger pressure, the user can drag on the right portion
         // of the front touch in a vertical direction to increase and decrease the pressure.
         int kRightThreshold = gTouchGesture.mFrontPanelDims.x - 300;
         if (gTouchGesture.mFrontTouchPos.x > kRightThreshold)
         {
            // Scale the maximum drag amount instead of using the whole vertical touch range so the user doesn't have 
            // to drag over the whole screen.
            float scaler = gTouchGesture.mFrontPanelDims.y * 0.35f;

            // Flip so moving in an upward direction is increasing.
            pressure = -gTouchGesture.mFrontTouchDeltaDrag.y / scaler;
         }
      }
      else if (whichScreen == kGesture_TouchScreen_Back)
      {
         // Left drag amount also allows for vertical dragging. Calculate the "pressure" by using the delta with the 
         // biggest change.
         if (std::abs(gTouchGesture.mBackTouchDeltaDrag[1].x) > std::abs(gTouchGesture.mBackTouchDeltaDrag[1].y))
         {
            float halfPanelWidth = gTouchGesture.mBackPanelDims.x * 0.5f;
            pressure = gTouchGesture.mBackTouchDeltaDrag[1].x / halfPanelWidth;
         }
         else
         {
            float halfPanelHeight = gTouchGesture.mBackPanelDims.y * 0.5f;
            pressure = gTouchGesture.mBackTouchDeltaDrag[1].y / halfPanelHeight;
         }
      }

      // Clamp and scale to requested range.
      float result = minVal + (pressure * (maxVal-minVal));
      if (result > maxVal)
         result = maxVal;
      else if (result < minVal)
         result = minVal;

      return result;
   }

   float GestureGetLeftDragAmount(ETouchScreen whichScreen, EGesturePadRelease pr, float const minVal, float const maxVal)
   {
      float pressure = 0.0f;

      if ( GestureShouldAbort( pr ) )
      {
         return minVal;
      }

      if (whichScreen == kGesture_TouchScreen_Back)
      {
         // Left drag amount also allows for vertical dragging. Calculate the "pressure" by using the delta with the 
         // biggest change.
         if (std::abs(gTouchGesture.mBackTouchDeltaDrag[0].x) > std::abs(gTouchGesture.mBackTouchDeltaDrag[0].y))
         {
            float halfPanelWidth = gTouchGesture.mBackPanelDims.x * 0.5f;
            pressure = -gTouchGesture.mBackTouchDeltaDrag[0].x / halfPanelWidth;
         }
         else
         {
            float halfPanelHeight = gTouchGesture.mBackPanelDims.y * 0.5f;
            pressure = gTouchGesture.mBackTouchDeltaDrag[0].y / halfPanelHeight;
         }
      }
      else
      {
         BPE_ASSERT(false, "Left drag doesn't support front touch.");
      }

      // Clamp and scale to requested range.
      float result = minVal + (pressure * (maxVal-minVal));
      if (result > maxVal)
         result = maxVal;
      else if (result < minVal)
         result = minVal;

      return result;
   }

   float GestureGetRightDragAmountAbs(ETouchScreen whichScreen, EGesturePadRelease pr, float const minVal, float const maxVal)
   {
      float pressure = 0.0f;

      if ( GestureShouldAbort( pr ) )
      {
         return minVal;
      }

      if (whichScreen == kGesture_TouchScreen_Front)
      {
         // When using the front touch for simulating analog trigger pressure, the user can drag on the right portion
         // of the front touch in a vertical direction to increase and decrease the pressure.
         int kRightThreshold = gTouchGesture.mFrontPanelDims.x - 300;
         // Armature fix:
         // Keep the touch region away from the weapon selector box in the bottom right
         // fixes MGSTHREE-3087
         int verticalThreshold = 3 * gTouchGesture.mFrontPanelDims.y / 4;
         if (gTouchGesture.mFrontTouchPos.x > kRightThreshold
            && gTouchGesture.mFrontTouchPos.y < verticalThreshold)
         {
            // Scale the maximum drag amount instead of using the whole vertical touch range so the user doesn't have 
            // to drag over the whole screen.
            float scaler = gTouchGesture.mFrontPanelDims.y * 0.35f;

            // Flip so moving in an upward direction is increasing.
            pressure = fabsf(-gTouchGesture.mFrontTouchDeltaDrag.y / scaler);
         }
      }
      else if (whichScreen == kGesture_TouchScreen_Back)
      {
         // When using the back touch, the user can drag from the center out of the backtouch to the right side and back
         // to the center to simulate the trigger pressure.
         float halfPanelWidth = gTouchGesture.mBackPanelDims.x * 0.5f;
         pressure = gTouchGesture.mBackTouchDeltaDrag[1].x / halfPanelWidth;
      }

      if (pressure == 0.0f)
      {
         return 0.0f;
      }
      else
      {
         // Clamp and scale to requested range.
         float result = minVal + (pressure * (maxVal-minVal));
         if (result > maxVal)
            result = maxVal;
         else if (result < minVal)
            result = minVal;

         return result;
      }
   }

   float GestureGetLeftDragAmountAbs(ETouchScreen whichScreen, EGesturePadRelease pr, float const minVal, float const maxVal)
   {
      float pressure = 0.0f;

      if ( GestureShouldAbort( pr ) )
      {
         return minVal;
      }

      if (whichScreen == kGesture_TouchScreen_Back)
      {
         float halfPanelWidth = gTouchGesture.mBackPanelDims.x * 0.5f;
         pressure = fabsf(-gTouchGesture.mBackTouchDeltaDrag[0].x / halfPanelWidth);
      }
      else
      {
         BPE_ASSERT(false, "Left drag doesn't support front touch.");
      }

      // Clamp and scale to requested range.
      float result = minVal + (pressure * (maxVal-minVal));
      if (result > maxVal)
         result = maxVal;
      else if (result < minVal)
         result = minVal;

      return result;
   }

   int GestureGetRightDragDir(ETouchScreen whichScreen, EGesturePadRelease pr )
   {
      int dir = 0;
      if ( !GestureShouldAbort( pr ) )
      {
         if (whichScreen == kGesture_TouchScreen_Front)
         {
            // Armature fix:
            // keep the drag away from the bottom icons
            // fix for MGSTWO-2840
            if (gTouchGesture.mFrontTouchPos.y < 3 * gTouchGesture.mFrontPanelDims.y / 4)
            {
               if (gTouchGesture.mFrontTouchDeltaDragAbs.y > 0)
               {
                  dir = 1;
               }
               else if (gTouchGesture.mFrontTouchDeltaDragAbs.y < 0)
               {
                  dir = -1;
               }
            }
         }
         else if (whichScreen == kGesture_TouchScreen_Back)
         {
            if (gTouchGesture.mBackTouchDeltaDrag[1].y > 0)
            {
               dir = 1;
            }
            else if (gTouchGesture.mBackTouchDeltaDrag[1].y < 0)
            {
               dir = -1;
            }
         }
      }
      return dir;
   }

   unsigned int GestureGetTriggerButtonMask(ETouchScreen whichScreen, EGesturePadRelease pr)
   {
      if ( GestureShouldAbort( pr ) )
      {
         return 0;
      }

      return gTouchGesture.mButtonMask[whichScreen];
   }

   int GestureGetBackTouchPressed(EGesturePadRelease pr)
   {
      return gTouchGesture.mBackTouchPressed && !GestureShouldAbort( pr );
   }

   int GestureGetBackTouchReleased(EGesturePadRelease pr)
   {
      return gTouchGesture.mBackTouchReleased || GestureShouldAbort( pr );
   }

   int GestureGetFrontTouchPressed(EGesturePadRelease pr)
   {
      return gTouchGesture.mFrontTouchPressed  && !GestureShouldAbort( pr );
   }

   int GestureGetFrontTouchReleased(EGesturePadRelease pr)
   {
      return gTouchGesture.mFrontTouchReleased  || GestureShouldAbort( pr );
   }

   void GestureGetFrontTouchPressedPos(float* pX, float* pY, float const rangeScaler)
   {
      *pX = gTouchGesture.mFrontTouchPos.x / (float)gTouchGesture.mFrontPanelDims.x / rangeScaler;
      *pY = gTouchGesture.mFrontTouchPos.y / (float)gTouchGesture.mFrontPanelDims.y / rangeScaler;
   }

   int GestureGetFrontTouchTapped(EGesturePadRelease pr)
   {
      return gTouchGesture.mFrontTouchTapped && !GestureShouldAbort( pr );
   }

   void GestureResetFrontTouchTappedStatus()
   {
      gTouchGesture.mFrontTouchTapped = false;
   }

   int GestureGetFrontDragging(EGesturePadRelease pr)
   {
      return gTouchGesture.mFrontTouchDragging && !GestureShouldAbort( pr ) ;
   }

   int GestureGetBackTouchTapped(EGesturePadRelease pr)
   {
      return gTouchGesture.mBackTouchTapped && !GestureShouldAbort( pr );
   }

   void GestureResetBackTouchTappedStatus()
   {
      gTouchGesture.mBackTouchTapped = false;
   }

   static inline GestureRecognizerHandle GestureEncodeHandle(int version, int type, int index)
   {
      return (version << 16) | (type << 8) | (index + 1);
   }

   static inline void GestureDecodeHandle(int* pVersion, int* pType, int* pIndex, int handle)
   {
      if (pVersion)
      {
         *pVersion = handle >> 16;
      }

      if (pType)
      {
         *pType = (handle & 0x0000ff00) >> 8;
      }

      if (pIndex)
      {
         *pIndex = (handle & 0xff) - 1;
      }
   }

   GestureRecognizerHandle GestureAllocRecognizerHandle(ERecognizerType recognizerType)
   {
      SRecognizerEntry* pRecognizers = &gTouchGesture.mRecognizerPool[recognizerType][0];
      int lowestVersionIndex = 0;
      int currPoolEntry;

      for (currPoolEntry = 0; currPoolEntry < kGesture_RecognizerPoolSize; currPoolEntry++)
      {
         // Go ahead and cache the entry with the lowest version count in case we run out of handles and need to 
         // hijack one.
         if (pRecognizers[currPoolEntry].mVersion < pRecognizers[lowestVersionIndex].mVersion)
         {
            lowestVersionIndex = currPoolEntry;
         }

         if (!pRecognizers[currPoolEntry].mIsInUse)
         {
            pRecognizers[currPoolEntry].mIsInUse = true;
            break;
         }
      }

      if (currPoolEntry == kGesture_RecognizerPoolSize)
      {
         // Ran out of available recognizers so hijack the entry with the lowest version number.
         currPoolEntry = lowestVersionIndex;
      }

      pRecognizers[currPoolEntry].mVersion++;

      // Create a handle using:
      //  * 16 bits for the version.
      //  * 8 bits for the recognizer type
      //  * 8 bits for the pool index.
      return GestureEncodeHandle(pRecognizers[currPoolEntry].mVersion, recognizerType, currPoolEntry);
   }

   void GestureReleaseRecognizerHandle(GestureRecognizerHandle handle)
   {
      if (GestureGetRecognizerByHandle(handle))
      {
         int recognizerType;
         int index;
         int version;

         GestureDecodeHandle(&version, &recognizerType, &index, handle);
         gTouchGesture.mRecognizerPool[recognizerType][index].mIsInUse = false;
      }
   }

   SceSystemGestureTouchRecognizer* GestureGetRecognizerByHandle(GestureRecognizerHandle handle)
   {
      if (handle == kGesture_InvalidHandle)
         return NULL;

      int version;
      int recognizerType;
      int index;

      GestureDecodeHandle(&version, &recognizerType, &index, handle);
      BPE_ASSERT_NO_MSG(recognizerType < kGesture_RecognizerType_Count);
      BPE_ASSERT_NO_MSG(index < kGesture_RecognizerPoolSize);

      SRecognizerEntry* pEntry = &gTouchGesture.mRecognizerPool[recognizerType][index];
      if (pEntry->mVersion == version && pEntry->mIsInUse)
      {
         // Found a valid handle if the versions match.
         return &pEntry->mRecognizer;
      }
      else
      {
         // If the version doesn't match then we ran out of handles at some point and had to hijack an existing one.
         // Inform the user their handle is now invalid by returning NULL so the error condition can be handled.
         return NULL;
      }
   }

   int GestureShouldAbort( EGesturePadRelease pr )
   {
#if MGS_VERSION == 3
      return ( pr == kGPR_Filtered && ( GV_PadData(0)->flag & GV_PAD_RELEASE ) );
#else
      return ( pr == kGPR_AlwaysFail ) || ( pr == kGPR_Filtered && ( GV_PadData[ 0 ].flag & GV_PAD_RELEASE ) );
#endif
   }
}

