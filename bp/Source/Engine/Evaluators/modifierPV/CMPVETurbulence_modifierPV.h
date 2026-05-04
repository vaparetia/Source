#pragma once

//Start:CMPVETurbulence_modifierPV_declaration:63A00899:B3AB91B5 *** Machine generated code - do not edit ***
class CMPVETurbulence_modifierPV : public IModifierPVEvaluator
{
public:
   CMPVETurbulence_modifierPV(CEvaluatorAllocator &allocator);
   virtual ~CMPVETurbulence_modifierPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual uint32 ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xF2204526;

   enum EProperties
   {
      kP_Strength                                      = 0x2FCF838E,
      kP_Frequency                                     = 0xA48E3AB0,
      kP_Time                                          = 0xCFA6377B,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mStrength;
   boost::shared_ptr<IFloatEvaluator>              mFrequency;
   boost::shared_ptr<IFloatEvaluator>              mTime;
//End:CMPVETurbulence_modifierPV_declaration:63A00899 *** Machine generated code - do not edit *** $End$:CMPVETurbulence_modifierPV_declaration:63A00899
};

