#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gboolEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCBEConvertFloatToBool_bool(gboolEvaluatorFactoriesEngine, CBEConvertFloatToBool_bool::kType_0x63EBD56A, CBEConvertFloatToBool_bool::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CBEConvertFloatToBool_bool::CBEConvertFloatToBool_bool(CEvaluatorAllocator &allocator)
: IBoolEvaluator()
//Start:CBEConvertFloatToBool_bool_ctor:A49A1EEF:15D54739 *** Machine generated code - do not edit ***
{
//End:CBEConvertFloatToBool_bool_ctor:A49A1EEF *** Machine generated code - do not edit *** $End$:CBEConvertFloatToBool_bool_ctor:A49A1EEF

}

CBEConvertFloatToBool_bool::~CBEConvertFloatToBool_bool()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

bool CBEConvertFloatToBool_bool::GetValue(CEvaluatorUpdateData const &updateData) const
{
   return mInput->GetValue(updateData) >= 0.5f;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CBEConvertFloatToBool_bool_body:78733558:8294012C *** Machine generated code - do not edit ***
uint32 const CBEConvertFloatToBool_bool::kType_0x63EBD56A = 0x63EBD56A;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CBEConvertFloatToBool_bool::GetType() const
{
   return kType_0x63EBD56A;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CBEConvertFloatToBool_bool::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CBEConvertFloatToBool_bool * pEval = new CBEConvertFloatToBool_bool(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CBEConvertFloatToBool_bool::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
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
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CBEConvertFloatToBool_bool.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CBEConvertFloatToBool_bool::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mInput)
   {
      uint32 const evalPtr = mInput->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mInput) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CBEConvertFloatToBool_bool::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mInput) mInput->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CBEConvertFloatToBool_bool::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mInput.get()) mInput->Initialize(updateData);
//End:CBEConvertFloatToBool_bool_body:78733558 *** Machine generated code - do not edit *** $End$:CBEConvertFloatToBool_bool_body:78733558

}

#endif //NO_EVALUATOR_INITIALIZER
