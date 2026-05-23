#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFETimeStep_float(gfloatEvaluatorFactoriesEngine, CFETimeStep_float::kType_0x885478A7, CFETimeStep_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

//----------------------------------------------------------------------------

CFETimeStep_float::CFETimeStep_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
, mLastTime(allocator.AllocFloat())
//Start:CFETimeStep_float_ctor:33BEC1F2:B875BBB4 *** Machine generated code - do not edit ***
{
   mScale = 1.0f;
   mOffset = 0.0f;
//End:CFETimeStep_float_ctor:33BEC1F2 *** Machine generated code - do not edit *** $End$:CFETimeStep_float_ctor:33BEC1F2
}

//----------------------------------------------------------------------------

CFETimeStep_float::~CFETimeStep_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFETimeStep_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CEvaluatorDataBlock & dataBlock = const_cast<CEvaluatorDataBlock&>(updateData.mDataBlock);
   real32 & lastTime = dataBlock.Value(mLastTime);
   real32 const deltaTime = updateData.mLocalTime - lastTime;
   lastTime = updateData.mLocalTime;
   return deltaTime * mScale + mOffset;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFETimeStep_float_body:EF57EA45:0EF01FAE *** Machine generated code - do not edit ***
uint32 const CFETimeStep_float::kType_0x885478A7 = 0x885478A7;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFETimeStep_float::GetType() const
{
   return kType_0x885478A7;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFETimeStep_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFETimeStep_float * pEval = new CFETimeStep_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFETimeStep_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Scale:   // 0x2D870A80
         {
            mScale = applyProperties.mStream.ReadReal32();
            break;
         }
         case kP_Offset:   // 0x5EA6CFE6
         {
            mOffset = applyProperties.mStream.ReadReal32();
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFETimeStep_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFETimeStep_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFETimeStep_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFETimeStep_float::Initialize(CEvaluatorUpdateData &updateData) const
{
//End:CFETimeStep_float_body:EF57EA45 *** Machine generated code - do not edit *** $End$:CFETimeStep_float_body:EF57EA45
   updateData.mDataBlock.SetValue(mLastTime, updateData.mLocalTime);
}
#endif //NO_EVALUATOR_INITIALIZER


