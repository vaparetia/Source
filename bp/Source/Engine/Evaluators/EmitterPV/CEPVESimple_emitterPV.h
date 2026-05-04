#pragma once

//Start:CEPVESimple_emitterPV_declaration:6AE38FF4:9368D04E *** Machine generated code - do not edit ***
class CEPVESimple_emitterPV : public IEmitterPVEvaluator
{
public:
   CEPVESimple_emitterPV(CEvaluatorAllocator &allocator);
   virtual ~CEPVESimple_emitterPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual void GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x409A437A;

   enum EProperties
   {
      kP_Position                                      = 0xBF5A86A3,
      kP_Velocity                                      = 0x47AEB4FF,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IVectorEvaluator>             mPosition;
   boost::shared_ptr<IVectorEvaluator>             mVelocity;
//End:CEPVESimple_emitterPV_declaration:6AE38FF4 *** Machine generated code - do not edit *** $End$:CEPVESimple_emitterPV_declaration:6AE38FF4
};

