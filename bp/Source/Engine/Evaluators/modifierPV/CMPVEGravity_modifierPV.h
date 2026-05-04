#pragma once

//Start:CMPVEGravity_modifierPV_declaration:1A196A45:BD383157 *** Machine generated code - do not edit ***
class CMPVEGravity_modifierPV : public IModifierPVEvaluator
{
public:
   CMPVEGravity_modifierPV(CEvaluatorAllocator &allocator);
   virtual ~CMPVEGravity_modifierPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual uint32 ApplyModifier(CEvaluatorUpdateData const &updateData, void * pTempData, real32 const deltaTime, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xF121806A;

   enum EProperties
   {
      kP_Gravity                                       = 0x95A01C3B,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IVectorEvaluator>             mGravity;
//End:CMPVEGravity_modifierPV_declaration:1A196A45 *** Machine generated code - do not edit *** $End$:CMPVEGravity_modifierPV_declaration:1A196A45
};

