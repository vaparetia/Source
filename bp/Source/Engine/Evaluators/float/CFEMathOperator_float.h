#pragma once

//Start:CFEMathOperator_float_declaration:B9865687:BE186713 *** Machine generated code - do not edit ***
class CFEMathOperator_float : public IFloatEvaluator
{
public:
   CFEMathOperator_float(CEvaluatorAllocator &allocator);
   virtual ~CFEMathOperator_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xF4F1C8F0;

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
      kOP_Divide,
      kOP_Modulo,
      kOP_Max,
      kOP_Min,
      kOP_Sine,
      kOP_Cosine,
      kOP_Abs,
      kOP_Ceil,
      kOP_Floor,
      kOP_Round,

      EOperatorType_Count
   };

   // Regular component properties
   EOperatorType                                   mOperator;
   boost::shared_ptr<IFloatEvaluator>              mA;
   boost::shared_ptr<IFloatEvaluator>              mB;
//End:CFEMathOperator_float_declaration:B9865687 *** Machine generated code - do not edit *** $End$:CFEMathOperator_float_declaration:B9865687
};

