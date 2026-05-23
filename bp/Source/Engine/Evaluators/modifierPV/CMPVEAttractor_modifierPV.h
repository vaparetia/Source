#pragma once

//Start:CMPVEAttractor_modifierPV_declaration:A8DA10C8:33F53639 *** Machine generated code - do not edit ***
class CMPVEAttractor_modifierPV : public IModifierPVEvaluator
{
public:
   CMPVEAttractor_modifierPV(CEvaluatorAllocator &allocator);
   virtual ~CMPVEAttractor_modifierPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual uint32 ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x45440AE0;

   enum EProperties
   {
      kP_Strength                                      = 0x2FCF838E,
      kP_CenterOffset                                  = 0x73523E97,
      kP_DistanceFalloff                               = 0x2B8063CE,
      kP_FalloffRadius                                 = 0xC0D49CCB,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   enum EDistanceFalloff
   {
      kDF_None,
      kDF_Linear,
      kDF_Square,
      kDF_InvLinear,
      kDF_InvSquare,

      EDistanceFalloff_Count
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mStrength;
   boost::shared_ptr<IVectorEvaluator>             mCenterOffset;
   EDistanceFalloff                                mDistanceFalloff;
   boost::shared_ptr<IFloatEvaluator>              mFalloffRadius;
//End:CMPVEAttractor_modifierPV_declaration:A8DA10C8 *** Machine generated code - do not edit *** $End$:CMPVEAttractor_modifierPV_declaration:A8DA10C8
};

