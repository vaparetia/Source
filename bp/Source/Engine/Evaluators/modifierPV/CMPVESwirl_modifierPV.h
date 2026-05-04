#pragma once

//Start:CMPVESwirl_modifierPV_declaration:E538C829:73D48157 *** Machine generated code - do not edit ***
class CMPVESwirl_modifierPV : public IModifierPVEvaluator
{
public:
   CMPVESwirl_modifierPV(CEvaluatorAllocator &allocator);
   virtual ~CMPVESwirl_modifierPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual uint32 ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x6AB7DE9B;

   enum EProperties
   {
      kP_UpAxis                                        = 0x102A2EF0,
      kP_Strength                                      = 0x2FCF838E,
      kP_Drag                                          = 0xCD770255,
      kP_DistanceFalloff                               = 0x2B8063CE,
      kP_AttenuateStrength                             = 0xE61B2A03,
      kP_AttenuateDrag                                 = 0x4194B252,
      kP_FalloffRadius                                 = 0xC0D49CCB,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   enum EDistanceFalloff
   {
      kDF_None,
      kDF_Linear,
      kDF_Square,

      EDistanceFalloff_Count
   };

   // Regular component properties
   boost::shared_ptr<IVectorEvaluator>             mUpAxis;
   boost::shared_ptr<IFloatEvaluator>              mStrength;
   boost::shared_ptr<IFloatEvaluator>              mDrag;
   EDistanceFalloff                                mDistanceFalloff;
   bool                                            mAttenuateStrength;
   bool                                            mAttenuateDrag;
   boost::shared_ptr<IFloatEvaluator>              mFalloffRadius;
//End:CMPVESwirl_modifierPV_declaration:E538C829 *** Machine generated code - do not edit *** $End$:CMPVESwirl_modifierPV_declaration:E538C829
};

