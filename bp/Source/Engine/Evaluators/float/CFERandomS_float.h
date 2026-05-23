#pragma once

//Start:CFERandomS_float_declaration:67534DE1:F9147877 *** Machine generated code - do not edit ***
class CFERandomS_float : public IFloatEvaluator
{
public:
   CFERandomS_float(CEvaluatorAllocator &allocator);
   virtual ~CFERandomS_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x27ADE674;

   enum EProperties
   {
      kP_Min                                           = 0x99365B30,
      kP_Max                                           = 0xA53B6469,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mMin;
   float                                           mMax;
//End:CFERandomS_float_declaration:67534DE1 *** Machine generated code - do not edit *** $End$:CFERandomS_float_declaration:67534DE1
};

