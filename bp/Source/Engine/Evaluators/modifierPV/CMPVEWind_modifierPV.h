#pragma once

//Start:CMPVEWind_modifierPV_declaration:5FD150C1:96987393 *** Machine generated code - do not edit ***
class CMPVEWind_modifierPV : public IModifierPVEvaluator
{
public:
   CMPVEWind_modifierPV(CEvaluatorAllocator &allocator);
   virtual ~CMPVEWind_modifierPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual uint32 ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x103ABDFE;

   enum EProperties
   {
      kP_WindSpeed                                     = 0x7FB7DF9A,
      kP_WindDrag                                      = 0x96904E6E,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IVectorEvaluator>             mWindSpeed;
   boost::shared_ptr<IFloatEvaluator>              mWindDrag;
//End:CMPVEWind_modifierPV_declaration:5FD150C1 *** Machine generated code - do not edit *** $End$:CMPVEWind_modifierPV_declaration:5FD150C1
};

