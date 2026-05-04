#if !defined(NO_EVALUATOR_REGISTRATION)
extern CRegisteredEvaluatorFactories gcolorEvaluatorFactoriesEngine;

// Register factory
static CRegisterEvaluatorFactory sRegisterFactoryCCEBuildColorRGBA(gcolorEvaluatorFactoriesEngine, CCEBuildColorRGBA::kType_0xEEB9D6E9, CCEBuildColorRGBA::Factory);

#endif //NO_EVALUATOR_REGISTRATION

#if !defined(NO_EVALUATOR_CONSTRUCTOR)

CCEBuildColorRGBA::CCEBuildColorRGBA(CEvaluatorAllocator &allocator)
: IColorEvaluator()
//Start:CCEBuildColorRGBA_ctor:6677411B:15D54739 *** Machine generated code - do not edit ***
{
//End:CCEBuildColorRGBA_ctor:6677411B *** Machine generated code - do not edit *** $End$:CCEBuildColorRGBA_ctor:6677411B
}

CCEBuildColorRGBA::~CCEBuildColorRGBA()
{
}

#endif //NO_EVALUATOR_CONSTRUCTOR

#if !defined(NO_EVALUATOR_GET_VALUE)

CColorf CCEBuildColorRGBA::GetValue(CEvaluatorUpdateData const &updateData) const
{
   real32 const r = mR->GetValue(updateData);
   real32 const g = mG->GetValue(updateData);
   real32 const b = mB->GetValue(updateData);
   real32 const a = mA->GetValue(updateData);

   return CColorf(r, g, b, a);
}

#endif //NO_EVALUATOR_GET_VALUE

//Start:CCEBuildColorRGBA_body:BA9E6AAC:233D3B61 *** Machine generated code - do not edit ***
uint32 const CCEBuildColorRGBA::kType_0xEEB9D6E9 = 0xEEB9D6E9;

#if !defined(NO_EVALUATOR_TYPE)

//----------------------------------------------------------------------------

uint32 CCEBuildColorRGBA::GetType() const
{
   return kType_0xEEB9D6E9;
}

#endif //NO_EVALUATOR_TYPE

#if !defined(NO_EVALUATOR_LOADER)

//----------------------------------------------------------------------------

IEvaluator * CCEBuildColorRGBA::Factory(CInputStream &inStream, CEvaluatorAllocator &allocator, IEvaluatorFactory const &factory)
{
   CCEBuildColorRGBA * pEval = new CCEBuildColorRGBA(allocator);
   CGameObjectApplyPropertiesData applyProperties(inStream, factory, CGameObjectApplyPropertiesData::kNone);
   pEval->ApplyProperties(applyProperties, allocator);
   return pEval;
}

//----------------------------------------------------------------------------

void CCEBuildColorRGBA::ApplyProperties(CGameObjectApplyPropertiesData &applyProperties, CEvaluatorAllocator &allocator)
{
   int32 const numProperties = applyProperties.mStream.ReadUint16();
   for (int loop = 0; loop < numProperties; loop++)
   {
      uint32 const fourCC = applyProperties.mStream.ReadUint32();
      uint16 size = applyProperties.mStream.ReadUint16();
      switch (fourCC)
      {
         case kP_R:   // 0x5767DF55
         {
            if (!applyProperties.UpdateOnly())
            {
               mR.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mR.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_G:   // 0x3ABA3BBE
         {
            if (!applyProperties.UpdateOnly())
            {
               mG.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mG.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_B:   // 0x4AD0CF31
         {
            if (!applyProperties.UpdateOnly())
            {
               mB.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mB.get(), applyProperties, size, allocator);
            }
            break;
         }
         case kP_A:   // 0xD3D99E8B
         {
            if (!applyProperties.UpdateOnly())
            {
               mA.reset(static_cast<IFloatEvaluator*>(applyProperties.mFactory.BuildFloatEvaluator(applyProperties.mStream.ReadUint32(), size, applyProperties.mStream, allocator)));
            }
            else
            {
               IEvaluator::UpdateProperties(mA.get(), applyProperties, size, allocator);
            }
            break;
         }
         default:
            property_load_error_printf( "Unknown property 0x%08x in evaluator loader CCEBuildColorRGBA.\n", fourCC );
            applyProperties.mStream.Get( NULL, size );
            break;
      }
   }
}

//----------------------------------------------------------------------------

uint32 CCEBuildColorRGBA::CopyToLinearStream(CLinearCopyStream &stream, IEvaluatorTypeToVTable const &evalToVTable) const
{
   uint32 pVTable = evalToVTable.InsertFakeVTable(stream, this);
   int const classPtrBufferPos = stream.Put(this, sizeof(*this));
   stream.SetValue(pVTable, classPtrBufferPos);
   
   if (mR)
   {
      uint32 const evalPtr = mR->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mR) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mG)
   {
      uint32 const evalPtr = mG->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mG) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mB)
   {
      uint32 const evalPtr = mB->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mB) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   if (mA)
   {
      uint32 const evalPtr = mA->CopyToLinearStream(stream, evalToVTable);
      int const evalPtrOffset = (int) (((uint8*) &mA) - ((uint8*) this));
      stream.SetValue(evalPtr, classPtrBufferPos + evalPtrOffset); // Change pointer to evaluator in copy
   }
   return stream.BufferPosToBufferPointer(classPtrBufferPos);
}

//----------------------------------------------------------------------------

void CCEBuildColorRGBA::PostLoadUpdate()
{
   // Also lock resources if present and call PostLoadUpdate on child properties.
   if (mR) mR->PostLoadUpdate();
   if (mG) mG->PostLoadUpdate();
   if (mB) mB->PostLoadUpdate();
   if (mA) mA->PostLoadUpdate();
}

#endif //NO_EVALUATOR_LOADER

#if !defined(NO_EVALUATOR_INITIALIZER)

//----------------------------------------------------------------------------

void CCEBuildColorRGBA::Initialize(CEvaluatorUpdateData &updateData) const
{
   if (mR.get()) mR->Initialize(updateData);
   if (mG.get()) mG->Initialize(updateData);
   if (mB.get()) mB->Initialize(updateData);
   if (mA.get()) mA->Initialize(updateData);
//End:CCEBuildColorRGBA_body:BA9E6AAC *** Machine generated code - do not edit *** $End$:CCEBuildColorRGBA_body:BA9E6AAC
}

#endif //NO_EVALUATOR_INITIALIZER

