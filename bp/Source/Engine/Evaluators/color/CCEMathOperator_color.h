#pragma once

//Start:CCEMathOperator_color_declaration:9EB3F15D:1F5393E2 *** Machine generated code - do not edit ***
class CCEMathOperator_color : public IColorEvaluator
{
public:
   CCEMathOperator_color(CEvaluatorAllocator &allocator);
   virtual ~CCEMathOperator_color();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x5B02DE8C;

   enum EProperties
   {
      kP_Operator                                      = 0x2ED0C5D7,
      kP_A                                             = 0xD3D99E8B,
      kP_B                                             = 0x4AD0CF31,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   enum EOperatorType
   {
      kOP_Add,
      kOP_Subtract,
      kOP_Multiply,

      EOperatorType_Count
   };

   // Regular component properties
   EOperatorType                                   mOperator;
   boost::shared_ptr<IColorEvaluator>              mA;
   boost::shared_ptr<IColorEvaluator>              mB;
//End:CCEMathOperator_color_declaration:9EB3F15D *** Machine generated code - do not edit *** $End$:CCEMathOperator_color_declaration:9EB3F15D
};

