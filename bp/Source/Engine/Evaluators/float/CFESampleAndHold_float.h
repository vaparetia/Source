#pragma once

//Start:CFESampleAndHold_float_declaration:D5B11930:8ADA899A *** Machine generated code - do not edit ***
class CFESampleAndHold_float : public IFloatEvaluator
{
public:
   CFESampleAndHold_float(CEvaluatorAllocator &allocator);
   virtual ~CFESampleAndHold_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xDB257918;

   enum EProperties
   {
      kP_Sample                                        = 0xF6A773F5,
      kP_Hold                                          = 0xBFF8A239,
      kP_Input                                         = 0x19E91DD3,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mSample;
   boost::shared_ptr<IFloatEvaluator>              mHold;
   boost::shared_ptr<IFloatEvaluator>              mInput;
//End:CFESampleAndHold_float_declaration:D5B11930 *** Machine generated code - do not edit *** $End$:CFESampleAndHold_float_declaration:D5B11930
   CEvalFloatData                                  mLastSample;
   CEvalFloatData                                  mLastSampleTime;
};

