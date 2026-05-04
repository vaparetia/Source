#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEInterpolateSpeed_color(gcolorEvaluatorFactoriesEngine, CCEInterpolateSpeed_color::kType_0xD07A7E67, CCEInterpolateSpeed_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEInterpolateSpeed_color::CCEInterpolateSpeed_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
, mCurrentColor(allocator.AllocCColorf())
, mLastTime(allocator.AllocFloat())
//Start:CCEInterpolateSpeed_color_ctor:DD72A0B0:15D54739 *** Machine generated code - do not edit ***
{
//End:CCEInterpolateSpeed_color_ctor:DD72A0B0 *** Machine generated code - do not edit *** $End$:CCEInterpolateSpeed_color_ctor:DD72A0B0
}

CCEInterpolateSpeed_color::~CCEInterpolateSpeed_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEInterpolateSpeed_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CEvaluatorDataBlock & dataBlock = const_cast<CEvaluatorDataBlock&>(updateData.mDataBlock);   
   real32 & lastTime = dataBlock.Value(mLastTime);

   real32 const deltaTime = updateData.mLocalTime - lastTime;
   CColorf &currentColor = dataBlock.Value(mCurrentColor);

   if( deltaTime > 0.0f )
   {
      // Update time
      lastTime = updateData.mLocalTime;

      real32 const speed = bpe::max_val(0.0f, mSpeed->GetValue(updateData));      
      real32 const scaledSpeed = speed * deltaTime;
      CColorf const endColor(mEnd->GetValue(updateData));
      CVector4 delta((endColor - currentColor).AsVector4());
      delta.mX = MathUtils::ClampMinMax(delta.mX, -scaledSpeed, scaledSpeed);
      delta.mY = MathUtils::ClampMinMax(delta.mY, -scaledSpeed, scaledSpeed);
      delta.mZ = MathUtils::ClampMinMax(delta.mZ, -scaledSpeed, scaledSpeed);
      delta.mW = MathUtils::ClampMinMax(delta.mW, -scaledSpeed, scaledSpeed);
      currentColor += CColorf(delta);
   }
   return currentColor;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEInterpolateSpeed_color_body:019B8B07:E479E4C2 *** Machine generated code - do not edit ***
uint32 const CCEInterpolateSpeed_color::kType_0xD07A7E67 = 0xD07A7E67;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEInterpolateSpeed_color::GetType() const
{
   return kType_0xD07A7E67;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEInterpolateSpeed_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEInterpolateSpeed_color * pEval = new CCEInterpolateSpeed_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEInterpolateSpeed_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Start:   // 0x5EB87A8B
         {
            if (!applyProperties.UpdateOnly())
            {
               mStart.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mStart.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_End:   // 0x38B17551
         {
            if (!applyProperties.UpdateOnly())
            {
               mEnd.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mEnd.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Speed:   // 0xCEE7D1F2
         {
            if (!applyProperties.UpdateOnly())
            {
               mSpeed.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mSpeed.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEInterpolateSpeed_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEInterpolateSpeed_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mStart)
   {
      uint32 const evalPtr = mStart->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mStart) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mEnd)
   {
      uint32 const evalPtr = mEnd->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mEnd) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mSpeed)
   {
      uint32 const evalPtr = mSpeed->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mSpeed) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CCEInterpolateSpeed_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mStart) mStart->PostLoadUpdate();
   if (mEnd) mEnd->PostLoadUpdate();
   if (mSpeed) mSpeed->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEInterpolateSpeed_color::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mStart.get()) mStart->Initialize(updateData);
   if (mEnd.get()) mEnd->Initialize(updateData);
   if (mSpeed.get()) mSpeed->Initialize(updateData);
//End:CCEInterpolateSpeed_color_body:019B8B07 *** Machine generated code - do not edit *** $End$:CCEInterpolateSpeed_color_body:019B8B07
   updateData.mDataBlock.SetValue(mCurrentColor, mStart->GetValue(updateData));
   updateData.mDataBlock.SetValue(mLastTime, updateData.mLocalTime);
}
#endif //NO_EVALUATOR_INITIALIZER


