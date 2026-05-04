#pragma once

//Start:CFERangeScale_float_declaration:215F0250:3BAB9EFA *** Machine generated code - do not edit ***
class CFERangeScale_float : public IFloatEvaluator
{
public:
   CFERangeScale_float(CEvaluatorAllocator &allocator);
   virtual ~CFERangeScale_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xE0B17143;

   enum EProperties
   {
      kP_Input                                         = 0x19E91DD3,
      kP_InputMin                                      = 0x3043E680,
      kP_InputMax                                      = 0x0C4ED9D9,
      kP_OutputMin                                     = 0xA38F6423,
      kP_OutputMax                                     = 0x9F825B7A,
      kP_ClampOutputRange                              = 0x7EC92266,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mInput;
   boost::shared_ptr<IFloatEvaluator>              mInputMin;
   boost::shared_ptr<IFloatEvaluator>              mInputMax;
   boost::shared_ptr<IFloatEvaluator>              mOutputMin;
   boost::shared_ptr<IFloatEvaluator>              mOutputMax;
   bool                                            mClampOutputRange;
//End:CFERangeScale_float_declaration:215F0250 *** Machine generated code - do not edit *** $End$:CFERangeScale_float_declaration:215F0250
};

