#if !defined(NO_EVALUATOR_REGISTRATION)

extern CRegisteredEvaluatorFactories guserDataEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCUDEBranch2_userData(guserDataEvaluatorFactoriesEngine, CUDEBranch2_userData::kType_0xB4D0A522, CUDEBranch2_userData::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CUDEBranch2_userData::CUDEBranch2_userData(CEvaluatorAllocator &allocator)
: IUserDataEvaluator()
//Start:CUDEBranch2_userData_ctor:853E6F3B:1CCA9E25 *** Machine generated code - do not edit ***
{
   mIdentifier = std::string("_Branch2");
//End:CUDEBranch2_userData_ctor:853E6F3B *** Machine generated code - do not edit *** $End$:CUDEBranch2_userData_ctor:853E6F3B
}

CUDEBranch2_userData::~CUDEBranch2_userData()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

char const * CUDEBranch2_userData::GetIdentifier() const
{
   return mIdentifier.c_str();
}

IUserDataEvaluator::EDataType CUDEBranch2_userData::GetUserDataType() const
{
   return kDT_Branch;
}

void CUDEBranch2_userData::GetUserData(CEvaluatorUpdateData const &updateData, void * pBufferOut) const
{
   uint32 * pEntries = (uint32 *) pBufferOut;
   *pEntries = 2;
   pEntries++;
   IUserDataEvaluator const ** pEvaluator = (IUserDataEvaluator const **) pEntries;
   *pEvaluator = mValue_A.get();
   pEvaluator++;
   *pEvaluator = mValue_B.get();
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CUDEBranch2_userData_body:59D7448C:07656EC4 *** Machine generated code - do not edit ***
uint32 const CUDEBranch2_userData::kType_0xB4D0A522 = 0xB4D0A522;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CUDEBranch2_userData::GetType() const
{
   return kType_0xB4D0A522;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CUDEBranch2_userData::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CUDEBranch2_userData * pEval = new CUDEBranch2_userData(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CUDEBranch2_userData::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
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
         case kP_Value_A:   // 0x6C062CB8
         {
            if (!applyProperties.UpdateOnly())
            {
               mValue_A.reset(static_cast<IUserDataEvaluator*>(applyProperties.mFactory.BuildUserDataEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mValue_A.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_Value_B:   // 0xF50F7D02
         {
            if (!applyProperties.UpdateOnly())
            {
               mValue_B.reset(static_cast<IUserDataEvaluator*>(applyProperties.mFactory.BuildUserDataEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mValue_B.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CUDEBranch2_userData.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CUDEBranch2_userData::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mValue_A)
   {
      uint32 const evalPtr = mValue_A->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mValue_A) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mValue_B)
   {
      uint32 const evalPtr = mValue_B->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mValue_B) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CUDEBranch2_userData::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mValue_A) mValue_A->PostLoadUpdate();
   if (mValue_B) mValue_B->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CUDEBranch2_userData::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mValue_A.get()) mValue_A->Initialize(updateData);
   if (mValue_B.get()) mValue_B->Initialize(updateData);
//End:CUDEBranch2_userData_body:59D7448C *** Machine generated code - do not edit *** $End$:CUDEBranch2_userData_body:59D7448C
}
#endif //NO_EVALUATOR_INITIALIZER


