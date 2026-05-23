#pragma once

//Start:CMPVEVelocityDamp_modifierPV_declaration:4924DEC1:93B43B50 *** Machine generated code - do not edit ***
class CMPVEVelocityDamp_modifierPV : public IModifierPVEvaluator
{
public:
   CMPVEVelocityDamp_modifierPV(CEvaluatorAllocator &allocator);
   virtual ~CMPVEVelocityDamp_modifierPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual uint32 ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x44FB1D59;

   enum EProperties
   {
      kP_LinearDampFactor                              = 0x30115BB3,
      kP_SquaredDampFactor                             = 0x2FA856C4,
      kP_ClampMinVelocity                              = 0x1682FA23,
      kP_ClampMaxVelocity                              = 0xDA8F812C,
      kP_MinVelocity                                   = 0xED84FB1E,
      kP_MaxVelocity                                   = 0x21898011,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mLinearDampFactor;
   boost::shared_ptr<IFloatEvaluator>              mSquaredDampFactor;
   bool                                            mClampMinVelocity;
   bool                                            mClampMaxVelocity;
   boost::shared_ptr<IFloatEvaluator>              mMinVelocity;
   boost::shared_ptr<IFloatEvaluator>              mMaxVelocity;
//End:CMPVEVelocityDamp_modifierPV_declaration:4924DEC1 *** Machine generated code - do not edit *** $End$:CMPVEVelocityDamp_modifierPV_declaration:4924DEC1
};

