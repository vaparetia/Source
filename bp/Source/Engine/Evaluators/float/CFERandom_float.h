#pragma once

//Start:CFERandom_float_declaration:F40CD410:B821CD6B *** Machine generated code - do not edit ***
class CFERandom_float : public IFloatEvaluator
{
public:
   CFERandom_float(CEvaluatorAllocator &allocator);
   virtual ~CFERandom_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xE59B23F7;

   enum EProperties
   {
      kP_Min                                           = 0x99365B30,
      kP_Max                                           = 0xA53B6469,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mMin;
   boost::shared_ptr<IFloatEvaluator>              mMax;
//End:CFERandom_float_declaration:F40CD410 *** Machine generated code - do not edit *** $End$:CFERandom_float_declaration:F40CD410
};

