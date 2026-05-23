#pragma once

#include <revolution/gx.h>

class CRVLRenderState
{
public:
   class CStateHandle
   {
   public:
      typedef void const * TUniqueID;

      CStateHandle();

      TUniqueID GetUniqueId() const { return mpUniqueId; }
      void const *GetData() const { return mPtr; }

      static TUniqueID const EmptyID() { return NULL; }
   protected:
      void Construct( void const *ptr, int const size );
   private:
      void const *mPtr;
      int mSize;
      void const *mpUniqueId;
      CStateHandle *mpNext;

      static CStateHandle *mpHead;
   };

   template <class T>
   class TStateHandle : public CStateHandle
   {
      friend class CRVLRenderState;
      typedef CStateHandle inherited;
   public:
      TStateHandle( T const &object )
         : mObj( object)
      {
         Construct( &mObj, sizeof( T ) );
      }

      T const &GetObject() const { return mObj; }

   private:
      BPE_DISABLE_COPY_AND_ASSIGNMENT( TStateHandle );

      T const mObj;
   };

   struct SPixelState
   {
      enum EColorUpdate
      {
         kColorUpdate_Disabled,
         kColorUpdate_Enabled
      };

      enum EAlphaUpdate
      {
         kAlphaUpdate_Disabled,
         kAlphaUpdate_Enabled
      };

      enum EDstAlpha
      {
         kDstAlpha_FromTev,
         kDstAlpha_Constant
      };

      enum EDepthCompare
      {
         kDepthCompare_Disabled,
         kDepthCompare_Enabled
      };

      enum EDepthUpdate
      {
         kDepthUpdate_Disabled,
         kDepthUpdate_Enabled
      };

      SPixelState( 
         GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor, GXLogicOp logic_op,
         EDepthCompare depthCompare, GXCompare depthFunc, EDepthUpdate depthUpdate,
         EColorUpdate colorUpdate, EAlphaUpdate alphaUpdate, EDstAlpha dstAlpha,
         GXCompare alphaCompare1, GXAlphaOp op = GX_AOP_AND, GXCompare alphaCompare2 = GX_ALWAYS )
         : mType( type )
         , mSrcFactor( src_factor )
         , mDstFactor( dst_factor )
         , mLogicOp( logic_op )
         , mDepthCompare( depthCompare )
         , mDepthFunc( depthFunc )
         , mDepthUpdate( depthUpdate )
         , mColorUpdate( colorUpdate )
         , mAlphaUpdate( alphaUpdate )
         , mDstAlpha( dstAlpha )
         , mAlphaCompare1( alphaCompare1 )
         , mAlphaOp( op )
         , mAlphaCompare2( alphaCompare2 )
      {
      }

      GXBlendMode mType;
      GXBlendFactor mSrcFactor;
      GXBlendFactor mDstFactor;
      GXLogicOp mLogicOp;
      EDepthCompare mDepthCompare;
      GXCompare mDepthFunc;
      EDepthUpdate mDepthUpdate;
      EColorUpdate mColorUpdate;
      EAlphaUpdate mAlphaUpdate;
      EDstAlpha mDstAlpha;
      GXCompare mAlphaCompare1;
      GXAlphaOp mAlphaOp;
      GXCompare mAlphaCompare2;
   };

   struct SVariableUnitStates
   {
      SVariableUnitStates( u8 numTevStages, u8 numIndStages, u8 numTexGens, u8 numChans )
         : mNumTev( numTevStages )
         , mNumInd( numIndStages )
         , mNumTex( numTexGens )
         , mNumChan( numChans )
      {
      }

      u8 mNumTev;
      u8 mNumInd;
      u8 mNumTex;
      u8 mNumChan;
   };

   struct STevOp
   {
      STevOp( 
         GXTevOp         op,
         GXTevBias       bias,
         GXTevScale      scale,
         GXBool          clamp,
         GXTevRegID      out_reg )
         : mOp( op ), mBias( bias ), mScale( scale ), mClamp( clamp ), mOutReg( out_reg )
      {
      }

      GXTevOp         mOp;
      GXTevBias       mBias;
      GXTevScale      mScale;
      GXBool          mClamp;
      GXTevRegID      mOutReg;
   };

   struct STevStageState
   {
      STevStageState(
         GXTevColorArg ca, GXTevColorArg cb, GXTevColorArg cc, GXTevColorArg cd, GXTevKColorSel colorSel,
         GXTevAlphaArg aa, GXTevAlphaArg ab, GXTevAlphaArg ac, GXTevAlphaArg ad, GXTevKAlphaSel alphaSel,
         STevOp const &color_op,
         STevOp const &alpha_op,
         GXTexCoordID texCoord,
         GXTexMapID texMap,
         GXChannelID channel )
         : mCA( ca ), mCB( cb ), mCC( cc ), mCD( cd )
         , mKColorSel( colorSel )
         , mAA( aa ), mAB( ab ), mAC( ac ), mAD( ad )
         , mKAlphaSel( alphaSel )
         , mColorOp( color_op )
         , mAlphaOp( alpha_op )
         , mTexCoord( texCoord )
         , mTexMap( texMap )
         , mChannel( channel )
      {
      }

      GXTevColorArg mCA, mCB, mCC, mCD;
      GXTevKColorSel mKColorSel;
      GXTevAlphaArg mAA, mAB, mAC, mAD;
      GXTevKAlphaSel mKAlphaSel;
      STevOp mColorOp, mAlphaOp;

      GXTexCoordID mTexCoord;
      GXTexMapID mTexMap;
      GXChannelID mChannel;
   };

   struct STexCoordGen
   {
      STexCoordGen( GXTexGenType func, GXTexGenSrc src_param, u32 mtx, GXBool normalize = GX_FALSE, u32 postmtx = GX_PTIDENTITY )
         : mFunc( func )
         , mSrc( src_param )
         , mMtx( mtx )
         , mNormalize( normalize )
         , mPostMtx( postmtx )
      {
      }

      GXTexGenType mFunc;
      GXTexGenSrc mSrc;
      u32 mMtx;
      GXBool mNormalize;
      u32 mPostMtx;
   };

   struct SFogState
   {
      SFogState( GXFogType type, f32 startZ, f32 endZ, f32 nearZ, f32 farZ, GXColor color )
         : mType( type ), mStartZ( startZ ), mEndZ( endZ ), mNearZ( nearZ ), mFarZ( farZ ), mColor( color )
      {

      }

      GXFogType mType;
      f32 mStartZ;
      f32 mEndZ;
      f32 mNearZ;
      f32 mFarZ;
      GXColor mColor;
   };

   typedef TStateHandle< SPixelState > TPixelStateHandle;
   typedef TStateHandle< SVariableUnitStates > TVariableUnitStatesHandle;
   typedef TStateHandle< STevStageState > TTevStageStateHandle;
   typedef TStateHandle< STexCoordGen > TTexcoordGenHandle;

   static TTexcoordGenHandle const &GetDefaultTexcoordGenForTexture( int const textureParamIndex );
   static TVariableUnitStatesHandle const &GetUnitStatesForSameTevsAndTexGens( int const tevAndTgCount );
   static TVariableUnitStatesHandle const &GetUnitStatesForSameTevsAndTexGensNoChan( int const tevAndTgCount );

   static STevOp const &GetDefaultTevOp();
};