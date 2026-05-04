#pragma once

//Start:CFEInitialRandom_float_declaration:1E6938F2:8005E6ED *** Machine generated code - do not edit ***
class CFEInitialRandom_float : public IFloatEvaluator
{
public:
   CFEInitialRandom_float(CEvaluatorAllocator &allocator);
   virtual ~CFEInitialRandom_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x8138F4E0;

   enum EProperties
   {
      kP_Min                                           = 0x99365B30,
      kP_Max                                           = 0xA53B6469,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mMin;
   boost::shared_ptr<IFloatEvaluator>              mMax;
//End:CFEInitialRandom_float_declaration:1E6938F2 *** Machine generated code - do not edit *** $End$:CFEInitialRandom_float_declaration:1E6938F2
   CEvalFloatData                                  mInitialValue;
};

