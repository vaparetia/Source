#pragma once

//Start:CFESin_float_declaration:52970C74:1E203C49 *** Machine generated code - do not edit ***
class CFESin_float : public IFloatEvaluator
{
public:
   CFESin_float(CEvaluatorAllocator &allocator);
   virtual ~CFESin_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x4BA4F69F;

   enum EProperties
   {
      kP_OutputMagnitude                               = 0x58C3C196,
      kP_SinMagnitude                                  = 0x85F839E1,
      kP_Bias                                          = 0xE2B5C565,
      kP_Phase                                         = 0x707CF9CF,
      kP_InputScale                                    = 0x7C92C27C,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mOutputMagnitude;
   float                                           mSinMagnitude;
   float                                           mBias;
   float                                           mPhase;
   float                                           mInputScale;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CFESin_float_declaration:52970C74 *** Machine generated code - do not edit *** $End$:CFESin_float_declaration:52970C74
};

