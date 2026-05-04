//----------------------------------------------------------------------------
// ShaderBinaryWriter.h
// Copyright 2010
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------

class CCGEffectCombination;

//----------------------------------------------------------------------------

typedef std::map< std::string, int > TVTARegisterMap;

void WriteShaderBinaryPS3(std::vector<CCGEffectCombination> const & effectCombinations, std::string const & outputPath);
void WriteShaderBinaryVTA(std::vector<CCGEffectCombination> const & effectCombinations, std::string const & outputPath, TVTARegisterMap const &registerMap );
