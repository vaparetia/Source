#pragma once

//Start:CFERandomV_float_declaration:59D9C05B:D6C54CC3 *** Machine generated code - do not edit ***
class CFERandomV_float : public IFloatEvaluator
{
public:
   CFERandomV_float(CEvaluatorAllocator &allocator);
   virtual ~CFERandomV_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x7595C9D3;

   enum EProperties
   {
      kP_CenterValue                                   = 0x2AA88BCF,
      kP_RandomVariation                               = 0xDE09F15B,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mCenterValue;
   boost::shared_ptr<IFloatEvaluator>              mRandomVariation;
//End:CFERandomV_float_declaration:59D9C05B *** Machine generated code - do not edit *** $End$:CFERandomV_float_declaration:59D9C05B
};

