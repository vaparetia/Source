#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFESplineCR4_float(gfloatEvaluatorFactoriesEngine, CFESplineCR4_float::kType_0xE8E9204D, CFESplineCR4_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CFESplineCR4_float::CFESplineCR4_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFESplineCR4_float_ctor:A586DD44:15D54739 *** Machine generated code - do not edit ***
{
//End:CFESplineCR4_float_ctor:A586DD44 *** Machine generated code - do not edit *** $End$:CFESplineCR4_float_ctor:A586DD44
}

CFESplineCR4_float::~CFESplineCR4_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFESplineCR4_float::GetValue(CEvaluatorUpdateData const &updateData) const
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
   float const float4 = mFloat4->GetValue(updateData);

   if (input < time3)
   {
      // Interpolated float
      real32 const s = (input - time2) / (time3 - time2);
      float output = MathUtils::CatmullRomSpline1(float1, float2, float3, float4, s);
      return output;
   }
   real32 const time4 = mTime4->GetValue(updateData);
   if (input > time4)
   {
      return float4;
   }

   // Interpolated float
   float const float5((float4 * 2.0f) - float3);
   real32 const s = (input - time3) / (time4 - time3);
   float output = MathUtils::CatmullRomSpline1(float2, float3, float4, float5, s);
   return output;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFESplineCR4_float_body:796FF6F3:8CC258F8 *** Machine generated code - do not edit ***
uint32 const CFESplineCR4_float::kType_0xE8E9204D = 0xE8E9204D;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFESplineCR4_float::GetType() const
{
   return kType_0xE8E9204D;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFESplineCR4_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFESplineCR4_float * pEval = new CFESplineCR4_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFESplineCR4_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
         case kP_Time4:   // 0x34AE15BB
         {
            if (!applyProperties.UpdateOnly())
            {
               mTime4.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mTime4.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Float4:   // 0x74B6DC9B
         {
            if (!applyProperties.UpdateOnly())
            {
               mFloat4.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mFloat4.get(), applyProperties, size, allocator);
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFESplineCR4_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFESplineCR4_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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
   if (mTime4)
   {
      uint32 const evalPtr = mTime4->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mTime4) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mFloat4)
   {
      uint32 const evalPtr = mFloat4->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mFloat4) - ((uint8*) this));
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

void CFESplineCR4_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mTime1) mTime1->PostLoadUpdate();
   if (mFloat1) mFloat1->PostLoadUpdate();
   if (mTime2) mTime2->PostLoadUpdate();
   if (mFloat2) mFloat2->PostLoadUpdate();
   if (mTime3) mTime3->PostLoadUpdate();
   if (mFloat3) mFloat3->PostLoadUpdate();
   if (mTime4) mTime4->PostLoadUpdate();
   if (mFloat4) mFloat4->PostLoadUpdate();
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFESplineCR4_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mTime1.get()) mTime1->Initialize(updateData);
   if (mFloat1.get()) mFloat1->Initialize(updateData);
   if (mTime2.get()) mTime2->Initialize(updateData);
   if (mFloat2.get()) mFloat2->Initialize(updateData);
   if (mTime3.get()) mTime3->Initialize(updateData);
   if (mFloat3.get()) mFloat3->Initialize(updateData);
   if (mTime4.get()) mTime4->Initialize(updateData);
   if (mFloat4.get()) mFloat4->Initialize(updateData);
   if (mInput.get()) mInput->Initialize(updateData);
//End:CFESplineCR4_float_body:796FF6F3 *** Machine generated code - do not edit *** $End$:CFESplineCR4_float_body:796FF6F3
}

#endif //NO_EVALUATOR_INITIALIZER

