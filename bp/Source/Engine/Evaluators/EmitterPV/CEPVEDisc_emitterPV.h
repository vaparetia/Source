#pragma once

//Start:CEPVEDisc_emitterPV_declaration:F84A6EF4:97AE47EB *** Machine generated code - do not edit ***
class CEPVEDisc_emitterPV : public IEmitterPVEvaluator
{
public:
   CEPVEDisc_emitterPV(CEvaluatorAllocator &allocator);
   virtual ~CEPVEDisc_emitterPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual void GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x6EDECC96;

   enum EProperties
   {
      kP_Radius                                        = 0x3CD06B6C,
      kP_Velocity                                      = 0x47AEB4FF,
      kP_Angle                                         = 0xC1406A0D,
      kP_UpAxisComponent                               = 0xD91EF443,
      kP_UpAxis                                        = 0x102A2EF0,
      kP_CenterOffset                                  = 0x73523E97,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mRadius;
   boost::shared_ptr<IFloatEvaluator>              mVelocity;
   boost::shared_ptr<IFloatEvaluator>              mAngle;
   boost::shared_ptr<IFloatEvaluator>              mUpAxisComponent;
   boost::shared_ptr<IVectorEvaluator>             mUpAxis;
   boost::shared_ptr<IVectorEvaluator>             mCenterOffset;
//End:CEPVEDisc_emitterPV_declaration:F84A6EF4 *** Machine generated code - do not edit *** $End$:CEPVEDisc_emitterPV_declaration:F84A6EF4
};

