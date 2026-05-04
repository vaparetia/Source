#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gemitterPVEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCEPVEDisc_emitterPV(gemitterPVEvaluatorFactoriesEngine, CEPVEDisc_emitterPV::kType_0x6EDECC96, CEPVEDisc_emitterPV::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CEPVEDisc_emitterPV::CEPVEDisc_emitterPV(CEvaluatorAllocator &allocator)
: IEmitterPVEvaluator()
//Start:CEPVEDisc_emitterPV_ctor:E9CDCCF2:15D54739 *** Machine generated code - do not edit ***
{
//End:CEPVEDisc_emitterPV_ctor:E9CDCCF2 *** Machine generated code - do not edit *** $End$:CEPVEDisc_emitterPV_ctor:E9CDCCF2
}

CEPVEDisc_emitterPV::~CEPVEDisc_emitterPV()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

void CEPVEDisc_emitterPV::GetValues(CEvaluatorUpdateData const &updateData, CVector3 &position, CVector3 &velocity) const
{
   CVector3 const upAxis(mUpAxis->GetValue(updateData).Normalized());
   real32 const radius = mRadius->GetValue(updateData);
   real32 const emitterVelocity = mVelocity->GetValue(updateData);
   real32 const angle = mAngle->GetValue(updateData) * gkDegrees2Radians32;

   CVector3 xAxis(CVector3::kConstructUninitialized);
   if (upAxis.mX > 0.8f)
   {
		xAxis = CVector3::Cross( upAxis, CVector3(0,1,0) );
   }
	else
   {
      xAxis = CVector3::Cross( upAxis, CVector3(1,0,0) );
   }
   CVector3 const yAxis(CVector3::Cross( upAxis, xAxis ));

   // When upAxis is 0,1,0, at angle zero rotate clockwise from the z axis
   CVector3 discPosition((xAxis * -cosf(angle)) + (yAxis * sinf(angle)));
   if (mUpAxisComponent)
   {
      // Not really a disc anymore when we add this!

      // Add up component
      discPosition += upAxis * mUpAxisComponent->GetValue(updateData);
      // Need to renormalized to keep radius.
      discPosition = discPosition.Normalized();
   }
   velocity = discPosition * emitterVelocity;
   position = discPosition * radius;

   if (mCenterOffset)
   {
      // Add optional center offset
      position += mCenterOffset->GetValue(updateData);
   }
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CEPVEDisc_emitterPV_body:3524E745:C7B93B49 *** Machine generated code - do not edit ***
uint32 const CEPVEDisc_emitterPV::kType_0x6EDECC96 = 0x6EDECC96;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CEPVEDisc_emitterPV::GetType() const
{
   return kType_0x6EDECC96;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CEPVEDisc_emitterPV::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CEPVEDisc_emitterPV * pEval = new CEPVEDisc_emitterPV(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CEPVEDisc_emitterPV::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
         case kP_Angle:   // 0xC1406A0D
         {
            if (!applyProperties.UpdateOnly())
            {
               mAngle.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mAngle.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_UpAxisComponent:   // 0xD91EF443
         {
            if (!applyProperties.UpdateOnly())
            {
               mUpAxisComponent.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mUpAxisComponent.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_UpAxis:   // 0x102A2EF0
         {
            if (!applyProperties.UpdateOnly())
            {
               mUpAxis.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mUpAxis.get(), applyProperties, size, allocator);
            }
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CEPVEDisc_emitterPV.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CEPVEDisc_emitterPV::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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
   if (mAngle)
   {
      uint32 const evalPtr = mAngle->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mAngle) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mUpAxisComponent)
   {
      uint32 const evalPtr = mUpAxisComponent->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mUpAxisComponent) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mUpAxis)
   {
      uint32 const evalPtr = mUpAxis->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mUpAxis) - ((uint8*) this));
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

void CEPVEDisc_emitterPV::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mRadius) mRadius->PostLoadUpdate();
   if (mVelocity) mVelocity->PostLoadUpdate();
   if (mAngle) mAngle->PostLoadUpdate();
   if (mUpAxisComponent) mUpAxisComponent->PostLoadUpdate();
   if (mUpAxis) mUpAxis->PostLoadUpdate();
   if (mCenterOffset) mCenterOffset->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CEPVEDisc_emitterPV::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mRadius.get()) mRadius->Initialize(updateData);
   if (mVelocity.get()) mVelocity->Initialize(updateData);
   if (mAngle.get()) mAngle->Initialize(updateData);
   if (mUpAxisComponent.get()) mUpAxisComponent->Initialize(updateData);
   if (mUpAxis.get()) mUpAxis->Initialize(updateData);
   if (mCenterOffset.get()) mCenterOffset->Initialize(updateData);
//End:CEPVEDisc_emitterPV_body:3524E745 *** Machine generated code - do not edit *** $End$:CEPVEDisc_emitterPV_body:3524E745
}

#endif //NO_EVALUATOR_INITIALIZER

