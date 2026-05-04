//----------------------------------------------------------------------------

#include "stdafx.h"
#include "AssetToolMPP.h"
#include "ShaderCompilerMPP.h"

//----------------------------------------------------------------------------

#include "Engine/System/CStopWatch.h"
#include "Engine/System/CGuid.h"

#include "Engine/Basics/CStringExtras.h"
#include "Engine/Mechanics/CCRC.h"

#include "Engine/Streams/CDiskInputStream.h"

#include "Tools/Helper/Helper.h"

#include "Boost/foreach.hpp"
#include "gcroot.h"

#include "Tools/AssetToolMPP/Shader/ShaderBinaryWriter.h"

//----------------------------------------------------------------------------

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Xml;
using namespace System::Xml::XPath;
using namespace System::Diagnostics;

//----------------------------------------------------------------------------

#include "Cg/cg.h"
#include "ExtLibraries/CG/include/Cg/cgGL.h"
#include "gl/glut.h"

#include "Renderer/Base/Material/CCompiledShaderCacheTypes.h"
#include "Tools/AssetToolMPP/Shader/CCGEffect.h"
#include "Engine/Mechanics/TTokenSet.h"
#include "Engine/Streams/CGrowableMemoryOutStream.h"

extern int cgbGenerateFromNVBufferEncode(const void *source, size_t size, size_t *outputSize, void **intermediateRepresentation);
extern int cgbGenerateFromNVBufferFinalize(void *intermediateRepresentation, void *outputBuffer, size_t outputSize);

namespace ShaderCooker
{

   //----------------------------------------------------------------------------

   void OnCGError(CGcontext ctx, CGerror err, void *data);

   //----------------------------------------------------------------------------

   static const TTokenSet<CCGEffectData::EType> skDataTypeTokens[] =
   {
      "bool",        CCGEffectData::kType_Bool1,
      "bool2",       CCGEffectData::kType_Bool2,
      "bool3",       CCGEffectData::kType_Bool3,
      "bool4",       CCGEffectData::kType_Bool4,

      "int",         CCGEffectData::kType_Int1,
      "int2",        CCGEffectData::kType_Int2,
      "int3",        CCGEffectData::kType_Int3,
      "int4",        CCGEffectData::kType_Int4,

      "float",       CCGEffectData::kType_Float1,
      "float2",      CCGEffectData::kType_Float2,
      "float3",      CCGEffectData::kType_Float3,
      "float4",      CCGEffectData::kType_Float4,

      "float3x3",    CCGEffectData::kType_Float3x3,
      "float3x4",    CCGEffectData::kType_Float3x4,
      "float4x4",    CCGEffectData::kType_Float4x4,

      "half",       CCGEffectData::kType_Float1,
      "half2",      CCGEffectData::kType_Float2,
      "half3",      CCGEffectData::kType_Float3,
      "half4",      CCGEffectData::kType_Float4,

      "half3x3",    CCGEffectData::kType_Float3x3,
      "half3x4",    CCGEffectData::kType_Float3x4,
      "half4x4",    CCGEffectData::kType_Float4x4,

      "string",      CCGEffectData::kType_String,

      "program",     CCGEffectData::kType_Program,
      "texture",     CCGEffectData::kType_Texture,
      "sampler2D",   CCGEffectData::kType_Sampler2D,

      NULL,          CCGEffectData::kType_Invalid
   };

   //----------------------------------------------------------------------------

   bool ProcessProgramParameters(CGprogram program, std::vector<CCGEffectNamedData> & programParameter);

   public ref class ProcessProgramWorkItemVTA : Tools::WorkQueue::WorkItem
   {
   public:
      ProcessProgramWorkItemVTA(System::String^ compileCommand,
#if 0
         System::String^ stripCommand,
#endif
         System::String^ temporaryFile,
         CCGEffectDataProgram* pOutput)
         :  mCompileCommand(compileCommand)
#if 0
         ,  mStripCommand(stripCommand)
#endif
         ,  mTemporaryFile(temporaryFile)
         ,  mpOutput(pOutput)
      {
      }

      static void OutputDataReceived( Object ^o, DataReceivedEventArgs ^e )
      {
         if ( e->Data != nullptr ) 
         {
            Console::WriteLine( e->Data->ToString() );
         }
      }

      virtual void Perform() override
      {
         // Compile shader
         {
            Console::WriteLine( "{0} {1}", "psp2cgc.exe", mCompileCommand );

            ProcessStartInfo^ info = gcnew ProcessStartInfo("psp2cgc.exe", mCompileCommand);
            info->UseShellExecute = false;
            info->RedirectStandardError = true;
            info->RedirectStandardOutput = true;

            Process^ compileProcess = Process::Start(info);
            compileProcess->OutputDataReceived += gcnew DataReceivedEventHandler( OutputDataReceived );
            compileProcess->ErrorDataReceived += gcnew DataReceivedEventHandler( OutputDataReceived );
            compileProcess->BeginErrorReadLine();
            compileProcess->BeginOutputReadLine();
            compileProcess->WaitForExit();

            if( compileProcess->ExitCode  != 0)
            {
               throw gcnew Exception(String::Format("Error during shader compilation: {0}", mCompileCommand));
            }

            delete compileProcess;
         }

#if 0
         // Strip shader
         {

            ProcessStartInfo^ info = gcnew ProcessStartInfo("sce-cgcstrip.exe", mStripCommand);
            info->UseShellExecute = false;

            Process^ stripProcess = Process::Start(info);

            stripProcess->WaitForExit();

            if( stripProcess->ExitCode  != 0)
               throw gcnew Exception(String::Format("Error during shader stripping: {0}", mStripCommand));
         }
#endif

         // Load compiled nvb file, munge it and store it (conversion to Level C CGB is done here).
         std::string tempOutputFilename =  Helper::StringHelper::ConvertString(mTemporaryFile);
         {
            CDiskInputStream inStream(tempOutputFilename.c_str() );

            int const nvbShaderSize = inStream.GetFileLength();
            void * pNvbShaderData = malloc(nvbShaderSize);

            inStream.Get(pNvbShaderData, nvbShaderSize);

            // 16 bytes into the file is a 4-byte checksum of sorts, appears to be some combination of the source and commandline
            // we zero it out to make like shaders compare alike, hopefully it doesn't completely hose the razor debugging
            char *p = (char*)pNvbShaderData;
            p[16] = p[17] = p[18] = p[19] = 0;
#if 0
            size_t cgbSize = 0;
            void* pCgbData = NULL;

            if( cgbGenerateFromNVBufferEncode(pNvbShaderData, nvbShaderSize, &cgbSize, &pCgbData) != 0 )
               throw gcnew Exception("Error while converting NVB Shader to CGB shader.");

            mpOutput->SetBinaryShader(cgbSize, (const char*)pCgbData);

            cgbGenerateFromNVBufferFinalize(pCgbData, NULL, 0);
#else
            mpOutput->SetBinaryShader( nvbShaderSize, (char const *)pNvbShaderData );
#endif
            free(pNvbShaderData);

         }

         // delete temporary file
         _unlink(tempOutputFilename.c_str());
      }

   private:
      System::String^         mCompileCommand;
#if 0
      System::String^         mStripCommand;
#endif
      System::String^         mTemporaryFile;
      CCGEffectDataProgram*   mpOutput;
   };

   bool ProcessProgramVTA(Tools::WorkQueue::WorkQueue^ workQueue, CGstateassignment stateAssignment, std::string const & inputPathStr, const char ** pCompileOptions, const char * stateName, CCGEffectStateAssignmentContainer & outStateAssignments, CCGEffectState const & effectState )
   {
      CGprogram program = cgGetProgramStateAssignmentValue(stateAssignment);

      std::vector<CCGEffectNamedData> programParameters;
      ProcessProgramParameters(program, programParameters);

      std::string const tempOutputFilename = CStringExtras::Stringize("%s.%s.tmp", inputPathStr.c_str(), CGuid::Generate().AsString().c_str());

      const char * pEntry = cgGetProgramString(program, CG_PROGRAM_ENTRY);

      // compile shader
      std::string compileCommand;
      {
//         compileCommand = "-q ";
         char const **pArgs = pCompileOptions;

         while (*pArgs != NULL)
         {
            compileCommand.append( CStringExtras::Stringize("\"%s\" ", *pArgs) );
            pArgs++;
         }

         compileCommand.append( CStringExtras::Stringize("-fx -nocolor -lang en -o \"%s\" ", tempOutputFilename.c_str()) );
         //compileCommand.append( "-D__VITACMDLINE -cache -cachedir c:\\vtacache " );
         compileCommand.append( "-D__VITACMDLINE " );
         compileCommand.append( CStringExtras::Stringize("-entry %s ", pEntry) );
         compileCommand.append( "-O4 " );
         compileCommand.append( "-fastprecision " );

         if (strcmp(stateName, "VertexShader") == 0)
         {
            compileCommand.append( "-profile sce_vp_psp2 -DVITA_VS " );
         }
         else
         {
            compileCommand.append( "-profile sce_fp_psp2 -DVITA_PS " );
         }

         // Filename
         compileCommand.append( inputPathStr );
      }

#if 0
      // strip shader
      std::string stripCommand;
      {
         stripCommand = CStringExtras::Stringize("-param \"%s\"", tempOutputFilename.c_str());
      }
#endif
      CCGEffectDataProgram* pData = new CCGEffectDataProgram(programParameters);
      outStateAssignments.mData.push_back(CCGEffectStateAssignment(effectState, pData));

      workQueue->Add(gcnew ProcessProgramWorkItemVTA(gcnew System::String(compileCommand.c_str()), 
#if 0
         gcnew System::String(stripCommand.c_str()), 
#endif
         gcnew System::String(tempOutputFilename.c_str()), pData));

      return true;
   }

   //----------------------------------------------------------------------------

   bool ProcessStateAssignmentsVTA(Tools::WorkQueue::WorkQueue^ workQueue,
      CGcontext context,      
      CGstateassignment stateAssignment,
      CCGEffectStateAssignmentContainer & outStateAssignments,
      const char ** pCompileOptions, 
      bool isSampler,
      std::string const &inputPathStr) 
   {
      while( stateAssignment )
      {
         CGstate state = NULL;
         if( isSampler )
         {
            state = cgGetSamplerStateAssignmentState(stateAssignment);
         }
         else
         {
            state = cgGetStateAssignmentState(stateAssignment);
         }

         const char * stateName = cgGetStateName(state);
         CCGEffectState effectState( stateName, "");

         CGtype stateType = cgGetStateType(state);
         const char * dataTypeStr = cgGetTypeString(stateType);

         CCGEffectData::EType type = skDataTypeTokens->GetTokenNoCase(dataTypeStr);
         switch(type)
         {
         case CCGEffectData::kType_Bool1:
         case CCGEffectData::kType_Bool2:
         case CCGEffectData::kType_Bool3:
         case CCGEffectData::kType_Bool4:
            {
               int numVals = 0;
               CGbool const * values = cgGetBoolStateAssignmentValues(stateAssignment, &numVals);

               CCGEffectDataNumeric* pData = new CCGEffectDataNumeric(type);

               for( int i = 0; i < numVals; ++i )
                  pData->mData8.b4[i] = (values[i] == CG_TRUE);

               outStateAssignments.mData.push_back(CCGEffectStateAssignment(effectState, pData));
            }
            break;

         case CCGEffectData::kType_Int1:
         case CCGEffectData::kType_Int2:
         case CCGEffectData::kType_Int3:
         case CCGEffectData::kType_Int4:
            {
               int numVals = 0;
               int const * values = cgGetIntStateAssignmentValues(stateAssignment, &numVals);

               CCGEffectDataNumeric* pData = new CCGEffectDataNumeric(type);
               memcpy(&pData->mData32.i1, values, numVals * sizeof(int));
               outStateAssignments.mData.push_back(CCGEffectStateAssignment(effectState, static_cast<CCGEffectData *>( pData )));
            }
            break;

         case CCGEffectData::kType_Float1:
         case CCGEffectData::kType_Float2:
         case CCGEffectData::kType_Float3:
         case CCGEffectData::kType_Float4:
         case CCGEffectData::kType_Float3x3:
         case CCGEffectData::kType_Float3x4:
         case CCGEffectData::kType_Float4x4:
            {
               int numVals = 0;
               real32 const * values = cgGetFloatStateAssignmentValues(stateAssignment, &numVals);

               CCGEffectDataNumeric* pData = new CCGEffectDataNumeric(type);
               memcpy(&pData->mData32.f1, values, numVals * sizeof(real32));
               outStateAssignments.mData.push_back(CCGEffectStateAssignment(effectState, static_cast<CCGEffectData *>( pData )));
            }
            break;

         case CCGEffectData::kType_String:
            break;

         case CCGEffectData::kType_Program:
            {
               if( !ProcessProgramVTA(workQueue, stateAssignment, inputPathStr, pCompileOptions, stateName, outStateAssignments, effectState) )
                  return false;
            }
            break;

         case CCGEffectData::kType_Texture:
            {
               CGparameter texParameter = cgGetTextureStateAssignmentValue(stateAssignment);

               const char * textureName = cgGetParameterName(texParameter);

               CCGEffectDataString* pData = new CCGEffectDataString(textureName);
               outStateAssignments.mData.push_back(CCGEffectStateAssignment(effectState, pData));
            }
            break;

         default:
            bpe_console_printf("Unsupported parameter type: %s\n", dataTypeStr);
            return false;
         }

         stateAssignment = cgGetNextStateAssignment(stateAssignment);
      }

      return true;
   }

   //----------------------------------------------------------------------------

   bool ProcessAnnotations(CGannotation annotation,
      std::vector<CCGEffectNamedData> & annotations);

   //----------------------------------------------------------------------------

   void registerShaderStates(CGcontext context);

   //----------------------------------------------------------------------------

   struct SCGEffectCompilationStats
   {
      real32   cgfxCompileTime;
      real32   processingTime;
      real32   finishUCodeCompilation;
   };

   static bool CreateCGEffectVTA(System::String^ inputPath, Tools::AssetSystem::Manager^ assetManager, List<Tools::AssetSystem::BuildAsset ^> ^buildAssets, std::vector<CCGEffectCombination> * pEffectCombinations, SCGEffectCompilationStats * pStats)
   {
      Tools::WorkQueue::WorkQueue^ workQueue = gcnew Tools::WorkQueue::WorkQueue();
      workQueue->WorkerPool = gcnew Tools::WorkQueue::WorkThreadPool( 1, Environment::ProcessorCount );
      workQueue->ConcurrentLimit = Environment::ProcessorCount * 2;
      boost::scoped_ptr<LuaWrap> luaWrap(new LuaWrap());

      static bool sGlutInitialized = false;

      if ( !sGlutInitialized )
      {
         // Initialize glut (needed for cgfx compiler)
         int argc = 1;
         char * argv[] =
         {
            "",
            NULL
         };

         glutInit(&argc, argv);

         sGlutInitialized = true;
      }

      int windowId = glutCreateWindow("Test");

      cgSetErrorHandler(OnCGError, NULL);

      CGcontext context = cgCreateContext();

      try
      {
         std::string const inputPathStr = Helper::StringHelper::ConvertString(inputPath);

         cgGLRegisterStates(context);
         registerShaderStates(context);

         std::vector<SDefine> compileSettings;
         std::vector<std::string> skipQueries;

         GetCompileSettings(assetManager, buildAssets, inputPathStr, compileSettings, skipQueries);

         uint32 const combinationCount = bpe::max_val( 1UL, GetCombinationCount(compileSettings) );

         pEffectCombinations->clear();
         pEffectCombinations->reserve(combinationCount);

         pStats->cgfxCompileTime = 0.0f;
         pStats->processingTime = 0.0f;

         for( uint32 combination = 0; combination < combinationCount; ++combination )
         {
            std::vector<std::string> options;
            options.push_back("-I" + Helper::StringHelper::ConvertString(System::IO::Path::GetDirectoryName(inputPath)));
            options.push_back("-D_CG");
            options.push_back("-D_VTA");
            options.push_back("-DBPE_PLATFORM=3");

            // build list of combination define pairs (name=value)
            std::string const combinationDefines = GetCombinationDefines(compileSettings, combination);

            // determine if this combination should be skipped
            bool const shouldSkipCombination = CheckIfSkipCombination(*luaWrap.get(), combinationDefines.c_str(), skipQueries);

            // print information about combination
            uint32 const definesCRC = CCRC::CalculateCRC32AsString(combinationDefines.c_str());
            System::Console::WriteLine("{0} {1} / {2} ({3}) [0x{4:x00000000}]", shouldSkipCombination ? "Skipping " : "Compiling", combination + 1, combinationCount, gcnew String(combinationDefines.c_str()), definesCRC);

            // skip if this combination isn't necessary.
            if( shouldSkipCombination )
               continue;

            // add compile options for combination specific defines
            {
               std::vector<std::string> splitDefines;
               CStringExtras::Tokenize(combinationDefines.c_str(), splitDefines, ";");
               for( int i = 0; i < splitDefines.size(); ++i )
               {
                  options.push_back("-D" + std::string( splitDefines[i].c_str() ) );
               }
            }

            std::vector<const char*> optionsPtrs;
            {
               BOOST_FOREACH(std::string const & value, options)
                  optionsPtrs.push_back(value.c_str());

               optionsPtrs.push_back("-D_CG_WIN");
               optionsPtrs.push_back(NULL);
            }

            CStopWatch watch;
            CGeffect cgEffect = cgCreateEffectFromFile(context, inputPathStr.c_str(), &optionsPtrs.front());
            pStats->cgfxCompileTime += watch.GetElapsedTime();

            watch.Reset();

            std::vector<CCGEffectTechnique> techniques;
            std::vector<CCGEffectSampler> samplers;
            std::vector<CCGEffectTexture> textures;

            for( CGtechnique technique = cgGetFirstTechnique(cgEffect); technique != NULL; technique = cgGetNextTechnique(technique) )
            {
               const char * techniqueName = cgGetTechniqueName(technique);

               std::vector<CCGEffectPass> passes;
               for( CGpass pass = cgGetFirstPass(technique); pass != NULL; pass = cgGetNextPass(pass) )
               {
                  std::string passName( cgGetPassName(pass) );

                  CCGEffectStateAssignmentContainer stateAssignments;
                  if(!ProcessStateAssignmentsVTA(workQueue,
                     context, 
                     cgGetFirstStateAssignment(pass), 
                     stateAssignments,
                     &optionsPtrs.front(),
                     false,
                     inputPathStr))
                  {
                     return false;
                  }

                  std::vector<CCGEffectNamedData> annotations;
                  if(!ProcessAnnotations(cgGetFirstPassAnnotation(pass), annotations) )
                  {
                     return false;
                  }

                  passes.push_back(CCGEffectPass(passName, "", stateAssignments, annotations));
               }

               techniques.push_back(CCGEffectTechnique(techniqueName, "", passes));
            }

            if ( techniques.empty() )
            {
               printf( "Error in shader, no techniques found!\n" );
               return false;
            }

            for( CGparameter parameter = cgGetFirstEffectParameter(cgEffect); parameter != NULL; parameter = cgGetNextParameter(parameter) )
            {
               if(cgGetParameterClass(parameter) == CG_PARAMETERCLASS_SAMPLER &&
                  (true || cgIsParameterUsed(parameter, cgEffect) || cgIsParameterReferenced(parameter)))
               {
                  const char * samplerName = cgGetParameterName(parameter);
                  const char * samplerSemantic = cgGetParameterSemantic(parameter);

                  CCGEffectStateAssignmentContainer stateAssignments;
                  if(!ProcessStateAssignmentsVTA(workQueue,
                     context, 
                     cgGetFirstSamplerStateAssignment(parameter), 
                     stateAssignments, 
                     &optionsPtrs.front(),
                     true,
                     inputPathStr))
                  {
                     return false;
                  }

                  samplers.push_back(CCGEffectSampler(samplerName, samplerSemantic, stateAssignments));
               }
               else if( cgGetParameterType(parameter) == CG_TEXTURE )
               {
                  const char * textureName = cgGetParameterName(parameter);
                  const char * textureSemantic = cgGetParameterSemantic(parameter);

                  std::vector<CCGEffectNamedData> annotations;
                  if(!ProcessAnnotations(cgGetFirstParameterAnnotation(parameter), annotations))
                  {
                     return false;
                  }

                  textures.push_back(CCGEffectTexture(textureName, textureSemantic, annotations));
               }
            }

            pEffectCombinations->push_back(CCGEffectCombination(definesCRC, techniques, samplers, textures));
            pStats->processingTime += watch.GetElapsedTime();

            cgDestroyEffect(cgEffect);
         }

         CStopWatch waitWatch;
         while( !workQueue->WaitAll(TimeSpan::FromSeconds(0.1)) && workQueue->FailingItems == 0)
         {
            printf("\rWaiting: %d shaders remaining                            ", workQueue->Count);
         }

         printf("\n");

         bool anyFailures = workQueue->FailingItems > 0;

         workQueue->ShutDownWorkerPool();
         workQueue = nullptr;

         pStats->finishUCodeCompilation = waitWatch.GetElapsedTime();

         return !anyFailures;
      }
      finally
      {
         cgDestroyContext(context);

         glutDestroyWindow(windowId);
      }
   }

   static cli::array< String ^ > ^remove_empties( cli::array< String ^ > ^inArr )
   {
      List< String ^ > ^l = gcnew List< String ^ >( inArr );

      while ( l->Remove( "" ) )
      {
      }

      return l->ToArray();
   }

   void ProcessFileForRegisterMapping( System::String ^rootPath, System::String ^inputPath, std::map<std::string, int> *pRegisters )
   {
      String ^splitChars = ": ()";
      cli::array<wchar_t> ^lineSplitChars = splitChars->ToCharArray();
      
      for each ( System::String ^line in System::IO::File::ReadAllLines( inputPath ) )
      {
         cli::array<String ^> ^split = remove_empties( line->Split( lineSplitChars ) );

         if ( line->Trim()->StartsWith( "#include" ) )
         {
            // Walk recursively

            String ^newPath = rootPath->Substring( 0, rootPath->LastIndexOfAny( ( gcnew String( "/\\" ) )->ToCharArray() ) );

            int firstQuote = line->IndexOf( '\"' );
            int lastQuote = line->LastIndexOf( '\"' );

            if ( firstQuote != -1 && lastQuote != -1 )
            {
               newPath = String::Format( "{0}/{1}", newPath,
                  line->Substring( firstQuote + 1, lastQuote - firstQuote - 1 ) );

               ProcessFileForRegisterMapping( rootPath, newPath, pRegisters );
            }
         }

         if ( split->Length < 4 )
         {
            continue;
         }

         String ^type = split[ 0 ]->Trim();
         String ^var = split[1]->Trim();
         String ^reg = split[2]->Trim();
         String ^regNumber = split[3]->Trim();

         if ( reg != "register" || type->StartsWith( "sampler" ) )
         {
            // we only care about non-sampler registers
            continue;
         }

         if ( !type->EndsWith( "4" ) &&
              !type->EndsWith( "3" ) )
         {
            throw gcnew Exception( String::Format( "Unknown type! {0}", type ) );
         }

         // Parse name
         int nameOpenBracket = var->IndexOf( '[' );
         int nameCloseBracket = var->IndexOf( ']' );

         std::string outName;

         if ( nameOpenBracket == -1 )
         {
            outName = Helper::StringHelper::ConvertString( var );
         }
         else
         {
            outName = Helper::StringHelper::ConvertString( var->Substring( 0, nameOpenBracket ) );
         }

         int outRegister;

         if ( regNumber->StartsWith( "c" ) )
         {
            Int32 registerNumber = Int32::Parse( regNumber->Substring( 1 ) );

            outRegister = registerNumber;
         }
         else
         {
            throw gcnew Exception( String::Format( "Line: '{0}' is malformed", line ) );
         }
         
         std::map<std::string,int>::iterator found = pRegisters->find( outName );

         if ( found != pRegisters->end() )
         {
            if ( found->second != outRegister )
            {
               throw gcnew Exception( String::Format( "Conflicting register for name {0}, was: {1} now: {2}", gcnew String( outName.c_str() ), found->second, outRegister ) );
            }
         }
         else
         {
            pRegisters->insert( TVTARegisterMap::value_type(outName, outRegister ) );
         }
      }
   }

   bool WriteShaderVTA(System::String ^ inputPath, System::String ^ outputPath, Tools::AssetSystem::Manager ^ assetManager, Tools::AssetSystem::PlatformType::EPlatform const platform)
   {
      CStopWatch totalWatch;

      if ( platform != Tools::AssetSystem::PlatformType::EPlatform::kVita )
      {
         Console::WriteLine( "WriteShaderVTA only knows how to cook VTA, not {0}", platform );
         return false;
      }

      TVTARegisterMap registers;
      ProcessFileForRegisterMapping( inputPath, inputPath, &registers );

      List<Tools::AssetSystem::BuildAsset ^> ^buildAssets = gcnew List<Tools::AssetSystem::BuildAsset ^>();
      parse_include_files_for_build_assets( inputPath, assetManager, buildAssets );

      std::vector<CCGEffectCombination> effectCombinations;

      SCGEffectCompilationStats stats;

      if( !CreateCGEffectVTA(inputPath, assetManager, buildAssets, &effectCombinations, &stats) )
         return false;

      Tools::AssetSystem::BuildAssets::WriteAssets( inputPath, buildAssets, assetManager, platform );

      std::string const outputPathStr = Helper::StringHelper::ConvertString(outputPath);

      WriteShaderBinaryVTA(effectCombinations, outputPathStr, registers );
      Tools::AssetSystem::BuildAssets::WriteAssets( inputPath, buildAssets, assetManager, platform );

      System::Console::WriteLine("Total: {0}s Parsing time: {1}s Processing time: {2}s Finish UCode: {3}s", totalWatch.GetElapsedTime(), stats.cgfxCompileTime, stats.processingTime, stats.finishUCodeCompilation);

      return true;
   }

}//namespace ShaderCooker

//----------------------------------------------------------------------------

