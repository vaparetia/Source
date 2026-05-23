#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEKeepInitial_color(gcolorEvaluatorFactoriesEngine, CCEKeepInitial_color::kType_0x2B968578, CCEKeepInitial_color::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEKeepInitial_color::CCEKeepInitial_color(CEvaluatorAllocator &allocator)
: IColorEvaluator()
, mInitialValue(allocator.AllocCColorf())
//Start:CCEKeepInitial_color_ctor:E9AE171D:15D54739 *** Machine generated code - do not edit ***
{
//End:CCEKeepInitial_color_ctor:E9AE171D *** Machine generated code - do not edit *** $End$:CCEKeepInitial_color_ctor:E9AE171D
}

CCEKeepInitial_color::~CCEKeepInitial_color()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEKeepInitial_color::GetValue(CEvaluatorUpdateData const &updateData) const
{
   CColorf const &color = updateData.mDataBlock.GetValue(mInitialValue);
   return color;
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEKeepInitial_color_body:35473CAA:8A789587 *** Machine generated code - do not edit ***
uint32 const CCEKeepInitial_color::kType_0x2B968578 = 0x2B968578;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEKeepInitial_color::GetType() const
{
   return kType_0x2B968578;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEKeepInitial_color::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEKeepInitial_color * pEval = new CCEKeepInitial_color(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEKeepInitial_color::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_Value:   // 0xDCB67730
         {
            if (!applyProperties.UpdateOnly())
            {
               mValue.reset(static_cast<IColorEvaluator*>(applyProperties.mFactory.BuildColorEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mValue.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEKeepInitial_color.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEKeepInitial_color::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
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

void CCEKeepInitial_color::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mValue) mValue->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEKeepInitial_color::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mValue.get()) mValue->Initialize(updateData);
//End:CCEKeepInitial_color_body:35473CAA *** Machine generated code - do not edit *** $End$:CCEKeepInitial_color_body:35473CAA
   updateData.mDataBlock.SetValue(mInitialValue, mValue->GetValue(updateData));
}

#endif //NO_EVALUATOR_INITIALIZER

