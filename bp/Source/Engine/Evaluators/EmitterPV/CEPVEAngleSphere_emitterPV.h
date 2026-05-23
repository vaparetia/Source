#pragma once

//Start:CEPVEAngleSphere_emitterPV_declaration:C946900C:CB97FD0D *** Machine generated code - do not edit ***
class CEPVEAngleSphere_emitterPV : public IEmitterPVEvaluator
{
public:
   CEPVEAngleSphere_emitterPV(CEvaluatorAllocator &allocator);
   virtual ~CEPVEAngleSphere_emitterPV();

   virtual void         Initialize(CEvaluatorUpdateData &updateData) const;
   virtual void GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const;
   virtual uint32       GetType() const;
   static IEvaluator *  Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory);
   virtual void         ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator);
   virtual void         PostLoadUpdate();
   virtual uint32       CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const;

   static const uint32                             kType_0x179B6E78;

   enum EProperties
   {
      kP_Radius                                        = 0x3CD06B6C,
      kP_Velocity                                      = 0x47AEB4FF,
      kP_PitchVariation                                = 0x55FFC93D,
      kP_YawVariation                                  = 0x59ABD40C,
      kP_EulerAngleRotation                            = 0xFD3D5FA2,
      kP_RotateCenterOffsetByEulerRotation             = 0x04E3BB04,
      kP_CenterOffset                                  = 0x73523E97,

      kP_Terminator                                    = 0xFFFFFFFF
   };

   // Regular component properties
   boost::shared_ptr<IFloatEvaluator>              mRadius;
   boost::shared_ptr<IFloatEvaluator>              mVelocity;
   boost::shared_ptr<IFloatEvaluator>              mPitchVariation;
   boost::shared_ptr<IFloatEvaluator>              mYawVariation;
   boost::shared_ptr<IVectorEvaluator>             mEulerAngleRotation;
   bool                                            mRotateCenterOffsetByEulerRotation;
   boost::shared_ptr<IVectorEvaluator>             mCenterOffset;
//End:CEPVEAngleSphere_emitterPV_declaration:C946900C *** Machine generated code - do not edit *** $End$:CEPVEAngleSphere_emitterPV_declaration:C946900C
};

