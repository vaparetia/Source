#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gemitterPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCEPVEAngleSphere_emitterPV(gemitterPVEvaluatorFactoriesEngine, CEPVEAngleSphere_emitterPV::kType_0x179B6E78, CEPVEAngleSphere_emitterPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CEPVEAngleSphere_emitterPV::CEPVEAngleSphere_emitterPV(CEvaluatorAllocator &allocator)
: IEmitterPVEvaluator()
//Start:CEPVEAngleSphere_emitterPV_ctor:9D86D2E8:2D23D921 *** Machine generated code - do not edit ***
{
   mRotateCenterOffsetByEulerRotation = true;
//End:CEPVEAngleSphere_emitterPV_ctor:9D86D2E8 *** Machine generated code - do not edit *** $End$:CEPVEAngleSphere_emitterPV_ctor:9D86D2E8
}

CEPVEAngleSphere_emitterPV::~CEPVEAngleSphere_emitterPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

void CEPVEAngleSphere_emitterPV::GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const
{
   real32 const pitchVar = mPitchVariation->GetValue(updateData) * gkDegrees2Radians32;
   real32 const yawVar = mYawVariation->GetValue(updateData) * gkDegrees2Radians32;
   CVector3 vector = updateData.mpRandom->RandomVector3(yawVar, pitchVar);
   CMatrix34 rotMatrix(CMatrix34::Identity());
   if (mEulerAngleRotation)
   {
      // We can cache this for speed later (we'll need to implement type virtual for evaluators first so we can check for Const_vector)
      CVector3 const rotation(mEulerAngleRotation->GetValue(updateData));
      // We should use a faster version of this.
      rotMatrix = CMatrix34::RotateZ(CAngle::FromDegrees(rotation.GetZ())) *
                  CMatrix34::RotateY(CAngle::FromDegrees(rotation.GetY())) *
                  CMatrix34::RotateX(CAngle::FromDegrees(rotation.GetX()));
      vector = rotMatrix * vector;
   }

   velocity = vector * mVelocity->GetValue(updateData);
   position = vector * mRadius->GetValue(updateData);

   if (mCenterOffset)
   {
      if (mRotateCenterOffsetByEulerRotation)
      {
         position += rotMatrix * mCenterOffset->GetValue(updateData);
      }
      else
      {
         position += mCenterOffset->GetValue(updateData);
      }
   }
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CEPVEAngleSphere_emitterPV_body:416FF95F:4FCF281D *** Machine generated code - do not edit ***
uint32 const CEPVEAngleSphere_emitterPV::kType_0x179B6E78 = 0x179B6E78;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CEPVEAngleSphere_emitterPV::GetType() const
{
   return kType_0x179B6E78;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CEPVEAngleSphere_emitterPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CEPVEAngleSphere_emitterPV * pEval = new CEPVEAngleSphere_emitterPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CEPVEAngleSphere_emitterPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Radius:   // 0x3CD06B6C
         {
            if (!applyProperties.UpdateOnly())
            {
               mRadius.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mRadius.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Velocity:   // 0x47AEB4FF
         {
            if (!applyProperties.UpdateOnly())
            {
               mVelocity.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mVelocity.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_PitchVariation:   // 0x55FFC93D
         {
            if (!applyProperties.UpdateOnly())
            {
               mPitchVariation.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mPitchVariation.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_YawVariation:   // 0x59ABD40C
         {
            if (!applyProperties.UpdateOnly())
            {
               mYawVariation.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mYawVariation.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_EulerAngleRotation:   // 0xFD3D5FA2
         {
            if (!applyProperties.UpdateOnly())
            {
               mEulerAngleRotation.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mEulerAngleRotation.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_RotateCenterOffsetByEulerRotation:   // 0x04E3BB04
         {
            mRotateCenterOffsetByEulerRotation = applyProperties.mStream.ReadBool();
            break;
         }
         case kP_CenterOffset:   // 0x73523E97
         {
            if (!applyProperties.UpdateOnly())
            {
               mCenterOffset.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mCenterOffset.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CEPVEAngleSphere_emitterPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CEPVEAngleSphere_emitterPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mRadius)
   {
      uint32 const evalPtr = mRadius->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mRadius) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mVelocity)
   {
      uint32 const evalPtr = mVelocity->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mVelocity) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mPitchVariation)
   {
      uint32 const evalPtr = mPitchVariation->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mPitchVariation) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mYawVariation)
   {
      uint32 const evalPtr = mYawVariation->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mYawVariation) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mEulerAngleRotation)
   {
      uint32 const evalPtr = mEulerAngleRotation->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mEulerAngleRotation) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mCenterOffset)
   {
      uint32 const evalPtr = mCenterOffset->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mCenterOffset) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CEPVEAngleSphere_emitterPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mRadius) mRadius->PostLoadUpdate();
   if (mVelocity) mVelocity->PostLoadUpdate();
   if (mPitchVariation) mPitchVariation->PostLoadUpdate();
   if (mYawVariation) mYawVariation->PostLoadUpdate();
   if (mEulerAngleRotation) mEulerAngleRotation->PostLoadUpdate();
   if (mCenterOffset) mCenterOffset->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CEPVEAngleSphere_emitterPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mRadius.get()) mRadius->Initialize(updateData);
   if (mVelocity.get()) mVelocity->Initialize(updateData);
   if (mPitchVariation.get()) mPitchVariation->Initialize(updateData);
   if (mYawVariation.get()) mYawVariation->Initialize(updateData);
   if (mEulerAngleRotation.get()) mEulerAngleRotation->Initialize(updateData);
   if (mCenterOffset.get()) mCenterOffset->Initialize(updateData);
//End:CEPVEAngleSphere_emitterPV_body:416FF95F *** Machine generated code - do not edit *** $End$:CEPVEAngleSphere_emitterPV_body:416FF95F
}

#endif //NO_EVALUATOR_INITIALIZER

