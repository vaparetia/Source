#pragma once

//Start:CFEConst_float_declaration:2126F3B4:A4683640 *** Machine generated code - do not edit ***
class ENGINE_API CFEConst_float : public IFloatEvaluator
{
public:
   CFEConst_float(CEvaluatorAllocator &allocator);
   virtual ~CFEConst_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x58C7BA28;

   enum EProperties
   {
      kP_Value                                         = 0xDCB67730,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mValue;
//End:CFEConst_float_declaration:2126F3B4 *** Machine generated code - do not edit *** $End$:CFEConst_float_declaration:2126F3B4
};

