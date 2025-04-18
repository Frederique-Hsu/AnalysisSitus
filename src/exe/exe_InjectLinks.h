//-----------------------------------------------------------------------------
// Created on: 19 April 2025
//-----------------------------------------------------------------------------
// Copyright (c) 2025-present, Sergey Slyadnev
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

#ifndef exe_InjectLinks_HeaderFile
#define exe_InjectLinks_HeaderFile

// Active Data includes
#include <ActAPI_IProgressNotifier.h>

// Standard includes
#include <map>
#include <string>

//! This class replaces placeholders for the SDK classes and structures
//! with the corresponding links to the reference documentation.
class exe_InjectLinks
{
public:

  //! Ctor.
  exe_InjectLinks(const std::string&   inputDir,
                  ActAPI_ProgressEntry progress);

public:

  //! Performs link replacement.
  bool Perform();

private:

  //! Reads a configuration file with links.
  bool readConf();

private:

  std::string                        m_inputDir; //!< Directory with user's manual.
  std::map<std::string, std::string> m_varLinks; //!< Variables with their links.
  ActAPI_ProgressEntry               m_progress; //!< Progress notifier.

};

#endif
