#pragma once

enum ETimeProviderType
{
   kTPT_LocalTime,
   kTPT_InstanceTime,
   kTPT_UniverseTime
};

//Start:CFETimeProvider_float_declaration:A0ACDECA:F39E60FA *** Machine generated code - do not edit ***
class CFETimeProvider_float : public IFloatEvaluator
{
public:
   CFETimeProvider_float(CEvaluatorAllocator &allocator);
   virtual ~CFETimeProvider_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x275039A8;

   enum EProperties
   {
      kP_Type                                          = 0x2CECF817,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   ETimeProviderType                               mType;
//End:CFETimeProvider_float_declaration:A0ACDECA *** Machine generated code - do not edit *** $End$:CFETimeProvider_float_declaration:A0ACDECA

};

