#pragma once

//Start:CFERandomBinary_float_declaration:0944F39B:07C34A4B *** Machine generated code - do not edit ***
class CFERandomBinary_float : public IFloatEvaluator
{
public:
   CFERandomBinary_float(CEvaluatorAllocator &allocator);
   virtual ~CFERandomBinary_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x5698FEE1;

   enum EProperties
   {
      kP_ValueA                                        = 0xF5DC1850,
      kP_ValueB                                        = 0x6CD549EA,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mValueA;
   boost::shared_ptr<IFloatEvaluator>              mValueB;
//End:CFERandomBinary_float_declaration:0944F39B *** Machine generated code - do not edit *** $End$:CFERandomBinary_float_declaration:0944F39B
};

