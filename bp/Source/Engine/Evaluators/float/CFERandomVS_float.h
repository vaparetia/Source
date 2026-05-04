#pragma once

//Start:CFERandomVS_float_declaration:86F000ED:030CFC79 *** Machine generated code - do not edit ***
class CFERandomVS_float : public IFloatEvaluator
{
public:
   CFERandomVS_float(CEvaluatorAllocator &allocator);
   virtual ~CFERandomVS_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x1B3E0C4F;

   enum EProperties
   {
      kP_CenterValue                                   = 0x2AA88BCF,
      kP_RandomVariation                               = 0xDE09F15B,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   float                                           mCenterValue;
   float                                           mRandomVariation;
//End:CFERandomVS_float_declaration:86F000ED *** Machine generated code - do not edit *** $End$:CFERandomVS_float_declaration:86F000ED
};

