#pragma once

//Start:CFEDistance_float_declaration:B246B87B:FB1151B5 *** Machine generated code - do not edit ***
class CFEDistance_float : public IFloatEvaluator
{
public:
   CFEDistance_float(CEvaluatorAllocator &allocator);
   virtual ~CFEDistance_float();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual real32 GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xC2D64126;

   enum EProperties
   {
      kP_A                                             = 0xD3D99E8B,
      kP_B                                             = 0x4AD0CF31,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IVectorEvaluator>             mA;
   boost::shared_ptr<IVectorEvaluator>             mB;
//End:CFEDistance_float_declaration:B246B87B *** Machine generated code - do not edit *** $End$:CFEDistance_float_declaration:B246B87B
};

