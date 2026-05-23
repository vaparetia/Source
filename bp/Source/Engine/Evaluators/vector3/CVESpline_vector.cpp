#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gvector3EvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCVESpline_vector(gvector3EvaluatorFactoriesEngine, CVESpline_vector::kType_0x9BD933C3, CVESpline_vector::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CVESpline_vector::CVESpline_vector(CEvaluatorAllocator &allocator)
: IVectorEvaluator()
//Start:CVESpline_vector_ctor:8DDDFA94:15D54739 *** Machine generated code - do not edit ***
{
//End:CVESpline_vector_ctor:8DDDFA94 *** Machine generated code - do not edit *** $End$:CVESpline_vector_ctor:8DDDFA94
}

CVESpline_vector::~CVESpline_vector()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CVector3 CVESpline_vector::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const time = mInput->GetValue(updateData);

   SSplineEvaluationCache xCache;
   real32 const x = mX.Evaluate(time, &xCache);
   SSplineEvaluationCache yCache;
   real32 const y = mY.Evaluate(time, &yCache);
   SSplineEvaluationCache zCache;
   real32 const z = mZ.Evaluate(time, &zCache);

   return CVector3(x, y, z);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CVESpline_vector_body:5134D123:06F27DA1 *** Machine generated code - do not edit ***
uint32 const CVESpline_vector::kType_0x9BD933C3 = 0x9BD933C3;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CVESpline_vector::GetType() const
{
   return kType_0x9BD933C3;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CVESpline_vector::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CVESpline_vector * pEval = new CVESpline_vector(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CVESpline_vector::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_X:   // 0xB7B2364B
         {
            mX.ConstructFromStream(applyProperties.mStream);
            break;
         }
         case kP_Y:   // 0xC0B506DD
         {
            mY.ConstructFromStream(applyProperties.mStream);
            break;
         }
         case kP_Z:   // 0x59BC5767
         {
            mZ.ConstructFromStream(applyProperties.mStream);
            break;
         }
         case kP_Input:   // 0x19E91DD3
         {
            if (!applyProperties.UpdateOnly())
            {
               mInput.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mInput.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CVESpline_vector.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CVESpline_vector::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   {
      int const memberPtrOffset = (int) (((uint8*) &mX) - ((uint8*) this));
      // Pointers fixed up in CopyToLinear_Inplace
      mX.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);
   }
   {
      int const memberPtrOffset = (int) (((uint8*) &mY) - ((uint8*) this));
      // Pointers fixed up in CopyToLinear_Inplace
      mY.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);
   }
   {
      int const memberPtrOffset = (int) (((uint8*) &mZ) - ((uint8*) this));
      // Pointers fixed up in CopyToLinear_Inplace
      mZ.CopyToLinearStream_Inplace(stream, classPtrBufferPos + memberPtrOffset);
   }
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CVESpline_vector::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CVESpline_vector::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
//End:CVESpline_vector_body:5134D123 *** Machine generated code - do not edit *** $End$:CVESpline_vector_body:5134D123
}

#endif //NO_EVALUATOR_INITIALIZER

#if !defined(NO_EVALUATOR_TYPE)


void CVESpline_vector::GetInfo(STypeInfo &info) const
{
   IVectorEvaluator::GetInfo(info);
   
   // Get min/max time ranges for all spines   
   info.mTimeRangeStart = gkReal32Max;
   info.mTimeRangeEnd = gkReal32Min;
   
   {
      CSpline const &spline = mX;      
      if (spline.mKeys.size() > 0)
      {
         info.mTimeRangeStart = bpe::min_val(spline.mKeys[0].mTime, info.mTimeRangeStart);
         info.mTimeRangeEnd = bpe::max_val(spline.mKeys[spline.mKeys.size() - 1].mTime, info.mTimeRangeEnd);
      }   
   }
   
   {
      CSpline const &spline = mY;      
      if (spline.mKeys.size() > 0)
      {
         info.mTimeRangeStart = bpe::min_val(spline.mKeys[0].mTime, info.mTimeRangeStart);
         info.mTimeRangeEnd = bpe::max_val(spline.mKeys[spline.mKeys.size() - 1].mTime, info.mTimeRangeEnd);
      }   
   }
   
   {
      CSpline const &spline = mZ;      
      if (spline.mKeys.size() > 0)
      {
         info.mTimeRangeStart = bpe::min_val(spline.mKeys[0].mTime, info.mTimeRangeStart);
         info.mTimeRangeEnd = bpe::max_val(spline.mKeys[spline.mKeys.size() - 1].mTime, info.mTimeRangeEnd);
      }   
   }
   
   if (info.mTimeRangeStart == gkReal32Max)
   {
      // No keys for x, y, or z
      info.mTimeRangeStart = 0.0f;
      info.mTimeRangeEnd = gkEpsilon32;      // Return small delta to prevent callers having to add logic for zero time ranges.
   }
   
   if (info.mTimeRangeStart == info.mTimeRangeEnd)
   {   
      // Only 1 key, return small delta to prevent callers having to add logic for zero time ranges.
      info.mTimeRangeEnd = info.mTimeRangeStart * (1.0f + (10.0f * gkEpsilon32));      
   }
}

#endif


