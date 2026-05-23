#pragma once

//Start:CFESmoothStep_float_declaration:F1618B78:B69CCF6D *** Machine generated code - do not edit ***
class CFESmoothStep_float : public IFloatEvaluator
{
public:
   CFESmoothStep_float(CEvaluatorAllocator &allocator);
   virtual ~CFESmoothStep_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xF4DB3BA2;

   enum EProperties
   {
      kP_Input                                         = 0x19E91DD3,
      kP_Scale                                         = 0x2D870A80,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mInput;
   boost::shared_ptr<IFloatEvaluator>              mScale;
//End:CFESmoothStep_float_declaration:F1618B78 *** Machine generated code - do not edit *** $End$:CFESmoothStep_float_declaration:F1618B78
};

