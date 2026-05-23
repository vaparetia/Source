#pragma once

//Start:CFEInitialGaussianRandom_float_declaration:D5A65E8E:CF1D379A *** Machine generated code - do not edit ***
class CFEInitialGaussianRandom_float : public IFloatEvaluator
{
public:
   CFEInitialGaussianRandom_float(CEvaluatorAllocator &allocator);
   virtual ~CFEInitialGaussianRandom_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x2FF0584E;

   enum EProperties
   {
      kP_Mean                                          = 0xD0CA408E,
      kP_Variance                                      = 0xBBB90070,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mMean;
   boost::shared_ptr<IFloatEvaluator>              mVariance;
//End:CFEInitialGaussianRandom_float_declaration:D5A65E8E *** Machine generated code - do not edit *** $End$:CFEInitialGaussianRandom_float_declaration:D5A65E8E
   CEvalFloatData                                  mInitialValue;
};

