//-----------------------------------------------------------------------------
// Created on: 18 October 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Sergey Slyadnev
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// Own include
#include <asiTest_SuppressFeatures.h>

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::runTestScript(const int   funcID,
                                                const char* filename)
{
  // Get filename of script to execute.
  TCollection_AsciiString fullFilename = GetFilename(filename);

  // Execute test script.
  outcome res = evaluate(fullFilename, DescriptionFn(), funcID);

  // Set description variables.
  SetVarDescr("filename", fullFilename,       ID(), funcID);
  SetVarDescr("time",     res.elapsedTimeSec, ID(), funcID);

  // Return status.
  return res;
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test001(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_001.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test002(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_002.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test003(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_003.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test004(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_004.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test005(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_005.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test006(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_006.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test007(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_007.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test008(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_008.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test009(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_009.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test010(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_010.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test011(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_011.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test012(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_012.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test013(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_013.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test014(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_014.tcl");
}

//-----------------------------------------------------------------------------

outcome asiTest_SuppressFeatures::test015(const int funcID, const bool)
{
  return runTestScript(funcID, "editing/suppress-features/suppress-features_015.tcl");
}
