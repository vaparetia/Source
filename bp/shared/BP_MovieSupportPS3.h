//====================================================================================
//                               FMV Player
//====================================================================================

#ifndef __CMOVIEPLAYER_H
#define __CMOVIEPLAYER_H

#include <cell/cell_fs.h>
#include <cell/codec.h>

class CTexture;

class CMoviePlayer
{
public:
   enum EMPEGState 
   {
      kMPEGState_Waiting,
      kMPEGState_Opening,
      kMPEGState_Open,
      kMPEGState_Buffering,
      kMPEGState_Buffered,
      kMPEGState_Ready,
      kMPEGState_Playing,
      kMPEGState_Stopped,
      kMPEGState_Closed,

      kMPEGState_Count
   };

   explicit CMoviePlayer(BP_Movie_Render_Callback readCallback, char* pMemoryStream);
   virtual ~CMoviePlayer();

   void StartMPEG();
   void CloseMPEG();

   void Update(int const shouldDraw);

   EMPEGState GetMPEGState() const { return mMPEGState; }

   enum { kFrameTextureCount = 3 };

private:
   bool ContinueReading();
   void ContinueDecoding();

   static unsigned int vdec_callback_static (CellVdecHandle, CellVdecMsgType msg_type, int err_code, void *);
   unsigned int VdecCallback(CellVdecHandle, CellVdecMsgType msg_type, int err_code);

public:
   struct AUFrame
   {
      unsigned char* mStart;
      unsigned int mLength;
      unsigned int mEnd;
   };

   enum { kNumFrames = 128 };

public:
   int            mFullscreen;

   EMPEGState     mMPEGState;
   CTexture *     mpTexture;
   CTexture *     mpQueuedTexture;

   char*          mpMemoryStream;

   BP_Movie_Render_Callback mReadCallback;

   CellVdecHandle mVdecHandle;
   unsigned int   mPicCnt;
   unsigned int   mAuDone;
   bool           mIsEOF;
   unsigned int   mBytesRead;
   unsigned int   mCurrStreamPos;
   unsigned int   mFrameStreamPos;
   unsigned int   mMostRecentBytesRead;
   unsigned int   mPicCurr;
   unsigned int   mPicFree;
   int            mNumFrame;
   unsigned int   mTotal;
   int            mCurrFrame;
   int            mPrevFrame;
   unsigned long long mStartTime64;
   float          mVideoTime;
   float          mFrameRate;
   bool           mbDone;
   int            mNumDrawnFrame;
   int            mError;
   bool           mbStart;

   AUFrame        mFrames[kNumFrames];
   DG_TEX         mDGTex;
};

//====================================================================================

#endif // __CMOVIEPLAYER_H
