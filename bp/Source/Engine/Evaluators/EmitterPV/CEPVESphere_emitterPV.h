#pragma once

//Start:CEPVESphere_emitterPV_declaration:4E6A356B:783E8CA4 *** Machine generated code - do not edit ***
class CEPVESphere_emitterPV : public IEmitterPVEvaluator
{
public:
   CEPVESphere_emitterPV(CEvaluatorAllocator &allocator);
   virtual ~CEPVESphere_emitterPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual void GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0xBA288E1C;

   enum EProperties
   {
      kP_Radius                                        = 0x3CD06B6C,
      kP_Velocity                                      = 0x47AEB4FF,
      kP_CenterOffset                                  = 0x73523E97,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mRadius;
   boost::shared_ptr<IFloatEvaluator>              mVelocity;
   boost::shared_ptr<IVectorEvaluator>             mCenterOffset;
//End:CEPVESphere_emitterPV_declaration:4E6A356B *** Machine generated code - do not edit *** $End$:CEPVESphere_emitterPV_declaration:4E6A356B
};

