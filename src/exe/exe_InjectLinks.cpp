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

// Own include
#include <exe_InjectLinks.h>

// exe includes
#include <exe_InjectLinksDict.h>

// OpenCascade includes
#include <TCollection_AsciiString.hxx>

// Standard includes
#include <vector>

// Qt includes
#pragma warning(push, 0)
#include <QDir>
#include <QDirIterator>
#include <QRegularExpressionMatch>
#include <QString>
#pragma warning(pop)

#define VARMARK std::string("%%")
#define EXTMARK std::string("!!")

namespace
{
  TCollection_AsciiString QString2AsciiStr(const QString& qstr)
  {
    return ( !qstr.isEmpty() ) ?
      TCollection_AsciiString( qstr.toUtf8().data() ) : TCollection_AsciiString();
  }
}

//-----------------------------------------------------------------------------

exe_InjectLinks::exe_InjectLinks(const std::string&   inputDir,
                                 ActAPI_ProgressEntry progress)
: m_inputDir(inputDir),
  m_progress(progress)
{
}

//-----------------------------------------------------------------------------

bool exe_InjectLinks::Perform()
{
  if ( !this->readConf() )
  {
    m_progress.SendLogMessage(LogErr(Normal) << "Failed to read a configuration file.");
    return false;
  }

  // Gather all files to process.
  std::vector<TCollection_AsciiString> inputFiles;
  //
  QDirIterator it(m_inputDir.c_str(),
                  QStringList() << "*.html",
                  QDir::Files,
                  QDirIterator::Subdirectories);
  //
  while ( it.hasNext() )
  {
    QString qFilename = it.next();
    inputFiles.push_back( QString2AsciiStr(qFilename) );
  }

  const int numFiles = int( inputFiles.size() );
  //
  m_progress.Init(numFiles);
  m_progress.SetMessageKey("Processing files");
  //
  m_progress.SendLogMessage(LogInfo(Normal) << "There are %1 file(s) to process..."
                                            << numFiles);

  // Process each file.
  for ( int k = 0; k < numFiles; ++k, m_progress.StepProgress(1) )
  {
    m_progress.SendLogMessage(LogInfo(Normal) << "Next file: '%1'."
                                              << inputFiles[k]);

    std::ifstream instream( inputFiles[k].ToCString() );
    std::stringstream buffer;
    buffer << instream.rdbuf();

    std::string       readout;
    std::stringstream outstream;

    while ( std::getline(buffer, readout) )
    {
      /* Class names */
      {
        QRegularExpression pattern("\\%\\%(\\w+)");
        QRegularExpressionMatchIterator i = pattern.globalMatch( readout.c_str() );
        //
        while ( i.hasNext() )
        {
          QRegularExpressionMatch match = i.next();
          std::string var = match.captured(1).toLocal8Bit().constData();
          //
          std::cout << "Substituting " << var;
          //
          std::string linkname = exe_InjectLinksDict::ClassLink(var);
          std::string link     = "<a href='https://analysissitus.org/refdoc/class";
                      link    += linkname;
                      link    += ".html'>";
                      link    += var.substr( 0, var.size() );
                      link    += "</a>";
          //
          std::cout << " with " << link << "..." << std::endl;
          //
          asiAlgo_Utils::Str::ReplaceAll(readout, VARMARK + var, link);
        }
      }

      /* Configured links */
      {
        QRegularExpression pattern("\\!\\!(\\w+)");
        QRegularExpressionMatchIterator i = pattern.globalMatch( readout.c_str() );
        //
        while ( i.hasNext() )
        {
          QRegularExpressionMatch match = i.next();
          std::string var = match.captured(1).toLocal8Bit().constData();
          //
          std::cout << "Substituting preconfigured " << var;
          //
          const auto& tpl = m_varLinks.find(var);
          //
          if ( tpl == m_varLinks.end() )
          {
            std::cout << ": FAILED" << std::endl;
            return false;
          }
          //
          std::string linkname = tpl->second;
          std::string link     = "<a href='";
                      link    += linkname;
                      link    += "'>";
                      link    += var.substr( 0, var.size() );
                      link    += "</a>";
          //
          std::cout << " with " << link << "..." << std::endl;
          //
          asiAlgo_Utils::Str::ReplaceAll(readout, EXTMARK + var, link);
        }
      }

      outstream << readout;
      outstream << "\n";
    }

    // Write back.
    std::ofstream outFile;
    outFile.open( inputFiles[k].ToCString() );
    outFile << outstream.rdbuf();
  }

  

  return true;
}

//-----------------------------------------------------------------------------

bool exe_InjectLinks::readConf()
{
  std::string
    fn = asiAlgo_Utils::Str::Slashed(m_inputDir) + "links.conf";

  std::ifstream FILE(fn);

  if ( !FILE.is_open() )
    return false;

  while ( !FILE.eof() )
  {
    char str[256];
    FILE.getline(str, 256);

    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::copy( std::istream_iterator<std::string>(iss),
               std::istream_iterator<std::string>(),
               std::back_inserter< std::vector<std::string> >(tokens) );

    if ( tokens.empty() || tokens.size() != 2 )
      continue;

    std::string name  = tokens[0];
    std::string value = tokens[1];

    if ( m_varLinks.find(name) == m_varLinks.end() )
      m_varLinks.insert({name, value});
  }

  FILE.close();
  return true;
}
