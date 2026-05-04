//----------------------------------------------------------------------------
// ShaderSetup.h
// Bluepoint
// Copyright 2008
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

#ifdef SHADER_MEASURE_MODE

// returns size of command buffer required in bytes
int const MeasureShaderCommandBufferHole(uint8* vertexShaderDataLS, uint8* fragmentShaderDataLS);

#else

void SetShader(uint8* pShaderFragmentDataLS,
               uint8* pShaderVertexDataLS,
               uint16 const commandBufferHoleSize,
               uint8* materialConstantParametersLS, int const materialConstantParametersSize,
               uint8* materialRuntimeParametersSharedLS, int const materialRuntimeParametersSharedSize,
               uint8* materialRuntimeParametersUnsharedLS, int const materialRuntimeParametersUnsharedSize,
               uint8* scratchMemoryLS,
               uint32 const outFragmentProgramEA,
               uint32 const outFragmentProgramOffset,
               uint32 const outCommandBufferHoleEA,
               uint32 const jumpToNextOffset
               );
#endif
