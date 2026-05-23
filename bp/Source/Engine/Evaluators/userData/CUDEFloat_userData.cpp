#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories guserDataEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCUDEFloat_userData(guserDataEvaluatorFactoriesEngine, CUDEFloat_userData::kType_0xABC24CA4, CUDEFloat_userData::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CUDEFloat_userData::CUDEFloat_userData(CEvaluatorAllocator &allocator)
: IUserDataEvaluator()
//Start:CUDEFloat_userData_ctor:DA04C3E2:509C7441 *** Machine generated code - do not edit ***
{
   mIdentifier = std::string("_Float");
//End:CUDEFloat_userData_ctor:DA04C3E2 *** Machine generated code - do not edit *** $End$:CUDEFloat_userData_ctor:DA04C3E2
}

CUDEFloat_userData::~CUDEFloat_userData()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

char const * CUDEFloat_userData::GetIdentifier() const
{
   return mIdentifier.c_str();
}

IUserDataEvaluator::EDataType CUDEFloat_userData::GetUserDataType() const
{
   return kDT_Float;
}

#if !defined(NO_EVALUATOR_GET_VALUE)

void CUDEFloat_userData::GetUserData(CEvaluatorUpdateData const &updateData, void * pOutBuffer) const
{
   real32 *pFloatOut = static_cast<real32 *>(pOutBuffer);
   *pFloatOut = mValue->GetValue(updateData);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CUDEFloat_userData_body:06EDE855:E9A0FE2F *** Machine generated code - do not edit ***
uint32 const CUDEFloat_userData::kType_0xABC24CA4 = 0xABC24CA4;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CUDEFloat_userData::GetType() const
{
   return kType_0xABC24CA4;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CUDEFloat_userData::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CUDEFloat_userData * pEval = new CUDEFloat_userData(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CUDEFloat_userData::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Identifier:   // 0x387380BA
         {
            mIdentifier = applyProperties.mStream.ReadString();
            break;
         }
         case kP_Value:   // 0xDCB67730
         {
            if (!applyProperties.UpdateOnly())
            {
               mValue.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mValue.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CUDEFloat_userData.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CUDEFloat_userData::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mValue)
   {
      uint32 const evalPtr = mValue->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mValue) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CUDEFloat_userData::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mValue) mValue->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CUDEFloat_userData::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mValue.get()) mValue->Initialize(updateData);
//End:CUDEFloat_userData_body:06EDE855 *** Machine generated code - do not edit *** $End$:CUDEFloat_userData_body:06EDE855
}
#endif //NO_EVALUATOR_INITIALIZER


