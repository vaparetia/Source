#pragma once

//Start:CCEBuildColorRGBA_declaration:23113CEE:6B2F0B95 *** Machine generated code - do not edit ***
class CCEBuildColorRGBA : public IColorEvaluator
{
public:
   CCEBuildColorRGBA(CEvaluatorAllocator &allocator);
   virtual ~CCEBuildColorRGBA();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual CColorf GetValue(CEvaluatorUpdateData const &updateData) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xEEB9D6E9;

   enum EProperties
   {
      kP_R                                             = 0x5767DF55,
      kP_G                                             = 0x3ABA3BBE,
      kP_B                                             = 0x4AD0CF31,
      kP_A                                             = 0xD3D99E8B,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mR;
   boost::shared_ptr<IFloatEvaluator>              mG;
   boost::shared_ptr<IFloatEvaluator>              mB;
   boost::shared_ptr<IFloatEvaluator>              mA;
//End:CCEBuildColorRGBA_declaration:23113CEE *** Machine generated code - do not edit *** $End$:CCEBuildColorRGBA_declaration:23113CEE
};

