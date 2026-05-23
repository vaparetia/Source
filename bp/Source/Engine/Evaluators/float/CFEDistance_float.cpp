#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories gfloatEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCFEDistance_float(gfloatEvaluatorFactoriesEngine, CFEDistance_float::kType_0xC2D64126, CFEDistance_float::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

//----------------------------------------------------------------------------

CFEDistance_float::CFEDistance_float(CEvaluatorAllocator &allocator)
: IFloatEvaluator()
//Start:CFEDistance_float_ctor:DA3C879A:15D54739 *** Machine generated code - do not edit ***
{
//End:CFEDistance_float_ctor:DA3C879A *** Machine generated code - do not edit *** $End$:CFEDistance_float_ctor:DA3C879A
}

//----------------------------------------------------------------------------

CFEDistance_float::~CFEDistance_float()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

real32 CFEDistance_float::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return CVector3::Distance( mA->GetValue( updateData ), mB->GetValue( updateData ) );
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CFEDistance_float_body:06D5AC2D:8B225611 *** Machine generated code - do not edit ***
uint32 const CFEDistance_float::kType_0xC2D64126 = 0xC2D64126;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CFEDistance_float::GetType() const
{
   return kType_0xC2D64126;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CFEDistance_float::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CFEDistance_float * pEval = new CFEDistance_float(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CFEDistance_float::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_A:   // 0xD3D99E8B
         {
            if (!applyProperties.UpdateOnly())
            {
               mA.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mA.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_B:   // 0x4AD0CF31
         {
            if (!applyProperties.UpdateOnly())
            {
               mB.reset(static_cast<IVectorEvaluator*>(applyProperties.mFactory.BuildVectorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mB.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CFEDistance_float.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CFEDistance_float::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mA)
   {
      uint32 const evalPtr = mA->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mA) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mB)
   {
      uint32 const evalPtr = mB->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mB) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CFEDistance_float::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mA) mA->PostLoadUpdate();
   if (mB) mB->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CFEDistance_float::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mA.get()) mA->Initialize(updateData);
   if (mB.get()) mB->Initialize(updateData);
//End:CFEDistance_float_body:06D5AC2D *** Machine generated code - do not edit *** $End$:CFEDistance_float_body:06D5AC2D
}
#endif //NO_EVALUATOR_INITIALIZER


