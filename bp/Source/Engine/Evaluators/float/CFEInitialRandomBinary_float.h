#pragma once

//Start:CFEInitialRandomBinary_float_declaration:1761318B:AE48C08D *** Machine generated code - do not edit ***
class CFEInitialRandomBinary_float : public IFloatEvaluator
{
public:
   CFEInitialRandomBinary_float(CEvaluatorAllocator &allocator);
   virtual ~CFEInitialRandomBinary_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x639EBA9C;

   enum EProperties
   {
      kP_ValueA                                        = 0xF5DC1850,
      kP_ValueB                                        = 0x6CD549EA,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mValueA;
   boost::shared_ptr<IFloatEvaluator>              mValueB;
//End:CFEInitialRandomBinary_float_declaration:1761318B *** Machine generated code - do not edit *** $End$:CFEInitialRandomBinary_float_declaration:1761318B
   CEvalFloatData                                  mInitialValue;
};

