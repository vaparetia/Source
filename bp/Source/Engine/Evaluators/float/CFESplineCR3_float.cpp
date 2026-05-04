#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFESplineCR3_float(gfloatEvaluatorFactoriesEngine, CFESplineCR3_float::kType_0x2D4E1EC3, CFESplineCR3_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFESplineCR3_float::CFESplineCR3_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFESplineCR3_float_ctor:D11E34CB:15D54739 *** Machine generated code - do not edit ***
{
//End:CFESplineCR3_float_ctor:D11E34CB *** Machine generated code - do not edit *** $End$:CFESplineCR3_float_ctor:D11E34CB
}

CFESplineCR3_float::~CFESplineCR3_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFESplineCR3_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   // We'll just clamp values outside time ranges for now.
   // Maybe add a flag to change this later.

   real32 const input = mInput->GetValue(updateData);
   real32 const time1 = mTime1->GetValue(updateData);
   float const float1 = mFloat1->GetValue(updateData);
   if (input < time1)
   {
      return float1;
   }

   real32 const time2 = mTime2->GetValue(updateData);
   float const float2 = mFloat2->GetValue(updateData);
   float const float3 = mFloat3->GetValue(updateData);

   if (input < time2)
   {
      // Interpolated float
      float const float0((float1 * 2.0f) - float2);
      real32 const s = (input - time1) / (time2 - time1);
      float output = MathUtils::CatmullRomSpline1(float0, float1, float2, float3, s);
      return output;
   }
   real32 const time3 = mTime3->GetValue(updateData);
   if (input > time3)
   {
      return float3;
   }

   // Interpolated float
   float const float4((float3 * 2.0f) - float2);
   real32 const s = (input - time2) / (time3 - time2);
   float output = MathUtils::CatmullRomSpline1(float1, float2, float3, float4, s);
   return output;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFESplineCR3_float_body:0DF71F7C:8969E411 *** Machine generated code - do not edit ***
uint32 const CFESplineCR3_float::kType_0x2D4E1EC3 = 0x2D4E1EC3;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFESplineCR3_float::GetType() const
{
   return kType_0x2D4E1EC3;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFESplineCR3_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFESplineCR3_float * pEval = new CFESplineCR3_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFESplineCR3_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Time1:   // 0x44C4E134
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime1.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime1.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Float1:   // 0x04DC2814
         {
            if (!applyProperties.UpdateOnly())
            {
               mFloat1.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mFloat1.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Time2:   // 0xDDCDB08E
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime2.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime2.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Float2:   // 0x9DD579AE
         {
            if (!applyProperties.UpdateOnly())
            {
               mFloat2.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mFloat2.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Time3:   // 0xAACA8018
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime3.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime3.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Float3:   // 0xEAD24938
         {
            if (!applyProperties.UpdateOnly())
            {
               mFloat3.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mFloat3.get(), applyProperties, size, allocator);
            }
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFESplineCR3_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFESplineCR3_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mTime1)
   {
      uint32 const evalPtr = mTime1->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime1) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mFloat1)
   {
      uint32 const evalPtr = mFloat1->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mFloat1) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mTime2)
   {
      uint32 const evalPtr = mTime2->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime2) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mFloat2)
   {
      uint32 const evalPtr = mFloat2->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mFloat2) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mTime3)
   {
      uint32 const evalPtr = mTime3->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime3) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mFloat3)
   {
      uint32 const evalPtr = mFloat3->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mFloat3) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
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

void CFESplineCR3_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mTime1) mTime1->PostLoadUpdate();
   if (mFloat1) mFloat1->PostLoadUpdate();
   if (mTime2) mTime2->PostLoadUpdate();
   if (mFloat2) mFloat2->PostLoadUpdate();
   if (mTime3) mTime3->PostLoadUpdate();
   if (mFloat3) mFloat3->PostLoadUpdate();
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFESplineCR3_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mTime1.get()) mTime1->Initialize(updateData);
   if (mFloat1.get()) mFloat1->Initialize(updateData);
   if (mTime2.get()) mTime2->Initialize(updateData);
   if (mFloat2.get()) mFloat2->Initialize(updateData);
   if (mTime3.get()) mTime3->Initialize(updateData);
   if (mFloat3.get()) mFloat3->Initialize(updateData);
   if (mInput.get()) mInput->Initialize(updateData);
//End:CFESplineCR3_float_body:0DF71F7C *** Machine generated code - do not edit *** $End$:CFESplineCR3_float_body:0DF71F7C
}

#endif //NO_EVALUATOR_INITIALIZER

