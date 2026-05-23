#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVECircle_vector(gvector3EvaluatorFactoriesEngine, CVECircle_vector::kType_0x7B3B15F6, CVECircle_vector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVECircle_vector::CVECircle_vector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
//Start:CVECircle_vector_ctor:955DCAA2:15D54739 *** Machine generated code - do not edit ***
{
//End:CVECircle_vector_ctor:955DCAA2 *** Machine generated code - do not edit *** $End$:CVECircle_vector_ctor:955DCAA2
}

CVECircle_vector::~CVECircle_vector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVECircle_vector::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CVector3 const upAxis(mUpAxis->GetValue(updateData).Normalized());
   real32 const radius = mRadius->GetValue(updateData);
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
   CVector3 outPosition((xAxis * -radius * cosf(angle)) + (yAxis * radius * sinf(angle)));
   if (mCenter)
   {
      outPosition += mCenter->GetValue(updateData);
   }
   return outPosition;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVECircle_vector_body:49B4E115:E3C4C0F1 *** Machine generated code - do not edit ***
uint32 const CVECircle_vector::kType_0x7B3B15F6 = 0x7B3B15F6;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVECircle_vector::GetType() const
{
   return kType_0x7B3B15F6;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVECircle_vector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVECircle_vector * pEval = new CVECircle_vector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVECircle_vector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
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
         case kP_Center:   // 0x475CEE12
         {
            if (!applyProperties.UpdateOnly())
            {
               mCenter.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mCenter.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVECircle_vector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVECircle_vector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mAngle)
   {
      uint32 const evalPtr = mAngle->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mAngle) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mRadius)
   {
      uint32 const evalPtr = mRadius->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mRadius) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mUpAxis)
   {
      uint32 const evalPtr = mUpAxis->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mUpAxis) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mCenter)
   {
      uint32 const evalPtr = mCenter->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mCenter) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CVECircle_vector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mAngle) mAngle->PostLoadUpdate();
   if (mRadius) mRadius->PostLoadUpdate();
   if (mUpAxis) mUpAxis->PostLoadUpdate();
   if (mCenter) mCenter->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVECircle_vector::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mAngle.get()) mAngle->Initialize(updateData);
   if (mRadius.get()) mRadius->Initialize(updateData);
   if (mUpAxis.get()) mUpAxis->Initialize(updateData);
   if (mCenter.get()) mCenter->Initialize(updateData);
//End:CVECircle_vector_body:49B4E115 *** Machine generated code - do not edit *** $End$:CVECircle_vector_body:49B4E115
}

#endif //NO_EVALUATOR_INITIALIZER

