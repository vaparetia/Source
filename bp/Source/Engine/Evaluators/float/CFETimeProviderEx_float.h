#pragma once

//Start:CFETimeProviderEx_float_declaration:DEEE9B11:8A30C11A *** Machine generated code - do not edit ***
class CFETimeProviderEx_float : public IFloatEvaluator
{
public:
   CFETimeProviderEx_float(CEvaluatorAllocator &allocator);
   virtual ~CFETimeProviderEx_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xE4F12DCB;

   enum EProperties
   {
      kP_Type                                          = 0x2CECF817,
      kP_Scale                                         = 0x2D870A80,
      kP_Wrap                                          = 0x0C087FE3,
      kP_Offset                                        = 0x5EA6CFE6,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   ETimeProviderType                               mType;
   float                                           mScale;
   float                                           mWrap;
   float                                           mOffset;
//End:CFETimeProviderEx_float_declaration:DEEE9B11 *** Machine generated code - do not edit *** $End$:CFETimeProviderEx_float_declaration:DEEE9B11
};

