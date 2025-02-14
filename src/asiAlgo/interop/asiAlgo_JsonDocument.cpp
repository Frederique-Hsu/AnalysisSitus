//-----------------------------------------------------------------------------
// Created on: 06 November 2024
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

#define RAPIDJSON_HAS_STDSTRING 1

// Own include
#include <asiAlgo_JsonDocument.h>

// asiAlgo includes
#include <asiAlgo_JsonDict.h>

// OpenCascade includes
#include <OSD_OpenFile.hxx>
#include <Standard_ProgramError.hxx>
#include <TCollection_AsciiString.hxx>

#if defined USE_RAPIDJSON

// Rapidjson includes
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

#endif

// Standard includes
#include <vector>

#if defined USE_RAPIDJSON

//-----------------------------------------------------------------------------

namespace
{
  void findRecursivelyObj(t_jsonValue*       pJsonObj,
                          const std::string& name,
                          bool&              isFound,
                          rapidjson::Value&  found);

  void findRecursivelyArr(t_jsonArray*       pJsonArr,
                          const std::string& name,
                          bool&              isFound,
                          rapidjson::Value&  found)
  {
    if ( isFound )
      return;

    // Iterate elements.
    for ( t_jsonValue::ValueIterator eit = pJsonArr->Begin();
          eit != pJsonArr->End(); eit++ )
    {
      if ( eit->IsObject() )
      {
        t_jsonValue obj = eit->GetObject();

        // Continue recursively.
        findRecursivelyObj(&obj, name, isFound, found);
        //
        if ( isFound )
          break;
      }
    }
  }

  void findRecursivelyObj(t_jsonValue*       pJsonObj,
                          const std::string& name,
                          bool&              isFound,
                          rapidjson::Value&  found)
  {
    if ( isFound )
      return;

    if ( !pJsonObj->IsObject() )
      return;

    // Iterate members.
    for ( t_jsonValue::MemberIterator mit = pJsonObj->MemberBegin();
          mit != pJsonObj->MemberEnd(); mit++ )
    {
      std::string prop( mit->name.GetString() );

      if ( prop == name )
      {
        if ( mit->value.IsObject() && !mit->value.IsNull() )
        {
          isFound = true;
          found   = mit->value;
          break;
        }

        if ( mit->value.IsArray() && !mit->value.IsNull() )
        {
          t_jsonArray arr = mit->value.GetArray();

          if ( !arr.Empty() )
          {
            isFound = true;
            found   = arr;
            break;
          }
        }

        // Elementary property.
        if ( mit->value.IsNumber() || mit->value.IsBool() || mit->value.IsString() )
        {
          isFound = true;
          break;
        }
      }

      // Continue recursively.
      if ( mit->value.IsObject() )
      {
        t_jsonValue obj = mit->value.GetObject();
        findRecursivelyObj(&obj, name, isFound, found);
      }
      //
      if ( mit->value.IsArray() )
      {
        t_jsonArray arr = mit->value.GetArray();
        findRecursivelyArr(&arr, name, isFound, found);
      }
      //
      if ( isFound )
        break;
    }
  }
}

//-----------------------------------------------------------------------------

asiAlgo_JsonDocument::asiAlgo_JsonDocument()
//
: m_pValue     ( new rapidjson::Document(rapidjson::kObjectType) ),
  m_bToRelease ( true ),
  m_bIsDoc     ( true ),
  m_bIsPretty  ( false )
{}

//-----------------------------------------------------------------------------

asiAlgo_JsonDocument::asiAlgo_JsonDocument(t_jsonValue* pValue,
                                           const bool   copyValue)
//
: m_bToRelease (copyValue ? true : false),
  m_bIsDoc     (false),
  m_bIsPretty  (false)
{
  if ( copyValue && pValue != nullptr )
  {
    // Make deep copy.
    m_pValue = new rapidjson::Document( pValue->GetType() );
    rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
    m_pValue->CopyFrom( *pValue, pDoc->GetAllocator() );
  }
  else
  {
    m_pValue = pValue;
  }
}

//-----------------------------------------------------------------------------

asiAlgo_JsonDocument::~asiAlgo_JsonDocument()
{
  if ( m_bToRelease )
  {
    rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
    pDoc->GetAllocator().Clear();
    delete m_pValue;
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_JsonDocument::LoadFromFile(const TCollection_AsciiString& inFilename)
{
  if ( m_bToRelease )
  {
    delete m_pValue;
  }

  m_pValue     = new rapidjson::Document();
  m_bIsDoc     = true;
  m_bToRelease = true;

  // Open file.
  std::ifstream stream;
  OSD_OpenStream(stream, inFilename.ToCString(), std::ios::in);
  //
  if ( !stream.is_open() || stream.fail() )
    return false;

  // Populate document.
  rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
  rapidjson::IStreamWrapper streamWrapper(stream);
  rapidjson::ParseResult pRes = pDoc->ParseStream(streamWrapper);
  //
  return !pRes.IsError();
}

//-----------------------------------------------------------------------------

bool asiAlgo_JsonDocument::LoadFromString(const TCollection_AsciiString& inString)
{
  if ( m_bToRelease )
  {
    delete m_pValue;
  }

  m_pValue     = new rapidjson::Document();
  m_bIsDoc     = true;
  m_bToRelease = true;

  // Populate document.
  rapidjson::Document*   pDoc = (rapidjson::Document*) m_pValue;
  rapidjson::ParseResult pRes = pDoc->Parse<rapidjson::kParseStopWhenDoneFlag>( inString.ToCString() );
  //
  return !pRes.IsError();
}

//-----------------------------------------------------------------------------

bool asiAlgo_JsonDocument::Contains(const std::string& name) const
{
  return m_pValue->HasMember(name);
}

//-----------------------------------------------------------------------------

int32_t
  asiAlgo_JsonDocument::GetInt32(const std::string& name,
                                 const int32_t      defValue) const
{
  rapidjson::Document::ConstMemberIterator iter = m_pValue->FindMember(name);
  //
  return ( iter != m_pValue->MemberEnd() && iter->value.IsInt() )
       ? iter->value.GetInt()
       : defValue;
}

//-----------------------------------------------------------------------------

double asiAlgo_JsonDocument::GetFloat64(const std::string& name,
                                        const double       defValue) const
{
  rapidjson::Document::ConstMemberIterator iter = m_pValue->FindMember(name);
  //
  return ( iter != m_pValue->MemberEnd() && iter->value.IsNumber() )
       ? iter->value.GetDouble()
       : defValue;
}

//-----------------------------------------------------------------------------

const char* asiAlgo_JsonDocument::GetString(const std::string& name,
                                            const char*        defValue) const
{
  rapidjson::Document::ConstMemberIterator iter = m_pValue->FindMember(name);
  //
  return ( iter != m_pValue->MemberEnd() && iter->value.IsString() )
       ? iter->value.GetString()
       : defValue;
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_JsonDocument)
  asiAlgo_JsonDocument::GetSubDocument(const std::string& name,
                                       const bool         copyDoc) const
{
  // Create a read-only reference to the value without data copying.
  rapidjson::Document::ConstMemberIterator iter = m_pValue->FindMember(name);
  //
  return ( iter != m_pValue->MemberEnd() && iter->value.IsObject() )
       ? new asiAlgo_JsonDocument(const_cast<t_jsonValue*>(&iter->value), copyDoc)
       : Handle(asiAlgo_JsonDocument)(); // null handle.
}

//-----------------------------------------------------------------------------

Handle(asiAlgo_JsonDocument)
  asiAlgo_JsonDocument::GetArray(const std::string& name) const
{
  // Create a read-only reference to the value without data copying.
  rapidjson::Document::ConstMemberIterator iter = m_pValue->FindMember(name);
  //
  return ( iter != m_pValue->MemberEnd() && iter->value.IsArray() )
       ? new asiAlgo_JsonDocument( const_cast<t_jsonValue*>(&iter->value) )
       : Handle(asiAlgo_JsonDocument)(); // null handle
}

//-----------------------------------------------------------------------------

int asiAlgo_JsonDocument::GetMemberCount() const
{
  if ( !m_bIsDoc )
    return -1;

  rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
  return pDoc->MemberCount();
}

//-----------------------------------------------------------------------------

bool asiAlgo_JsonDocument::HasData(const std::string& name) const
{
  bool             isFound = false;
  rapidjson::Value found;
  //
  ::findRecursivelyObj(m_pValue, name, isFound, found);

  return isFound;
}

//-----------------------------------------------------------------------------

bool asiAlgo_JsonDocument::HasDiagnosticCode(const int code) const
{
  bool             isFound = false;
  rapidjson::Value found;
  //
  ::findRecursivelyObj(m_pValue, asiPropName_StatusCodes, isFound, found);

  if ( isFound && found.IsArray() )
  {
    t_jsonArray jsonArr = found.GetArray();

    // Iterate elements.
    for ( t_jsonValue::ValueIterator eit = jsonArr.Begin();
          eit != jsonArr.End(); eit++ )
    {
      if ( eit->IsInt() )
      {
        const int currCode = eit->GetInt();
        //
        if ( currCode == code )
          return true;
      }
    }
  }

  return false;
}

//-----------------------------------------------------------------------------

void asiAlgo_JsonDocument::SetInt32(const std::string& name,
                                    const int32_t      value)
{
  if ( !m_bIsDoc )
  {
    throw Standard_ProgramError("asiAlgo_JsonDocument::SetInt32() - not a document");
  }

  rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
  t_jsonValue key( name, pDoc->GetAllocator() );
  m_pValue->AddMember( key, value, pDoc->GetAllocator() );
}

//-----------------------------------------------------------------------------

void asiAlgo_JsonDocument::SetFloat64(const std::string& name,
                                      const double       value)
{
  if ( !m_bIsDoc )
  {
    throw Standard_ProgramError("asiAlgo_JsonDocument::SetFloat64() - not a document");
  }

  rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
  t_jsonValue key( name, pDoc->GetAllocator() );
  m_pValue->AddMember( key, value, pDoc->GetAllocator() );
}

//-----------------------------------------------------------------------------

void asiAlgo_JsonDocument::SetString(const std::string& name,
                                     const char*        pValue)
{
  if ( !m_bIsDoc )
  {
    throw Standard_ProgramError("asiAlgo_JsonDocument::SetString() - not a document");
  }

  rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
  t_jsonValue key   ( name,   pDoc->GetAllocator() );
  t_jsonValue value ( pValue, pDoc->GetAllocator() );
  m_pValue->AddMember( key, value, pDoc->GetAllocator() );
}

//-----------------------------------------------------------------------------

void asiAlgo_JsonDocument::SetSubDocument(const std::string&    name,
                                          asiAlgo_JsonDocument* pSubDoc)
{
  if ( !m_bIsDoc )
  {
    throw Standard_ProgramError("asiAlgo_JsonDocument::SetSubDocument() - not a document");
  }

  if ( pSubDoc != nullptr )
  {
    rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
    t_jsonValue key( name, pDoc->GetAllocator() );
    m_pValue->AddMember( key, *pSubDoc->m_pValue, pDoc->GetAllocator() );
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_JsonDocument::SetArray(const std::string&    name,
                                    asiAlgo_JsonDocument* pArray)
{
  if ( !m_bIsDoc )
  {
    throw Standard_ProgramError("asiAlgo_JsonDocument::SetArray() - not a document");
  }

  if ( pArray != nullptr )
  {
    if ( !pArray->m_pValue->IsArray() )
    {
      throw Standard_ProgramError("asiAlgo_JsonDocument::SetArray() - passed not an array");
    }

    rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
    t_jsonValue key( name, pDoc->GetAllocator() );
    m_pValue->AddMember( key, *pArray->m_pValue, pDoc->GetAllocator() );
  }
}

//-----------------------------------------------------------------------------

void asiAlgo_JsonDocument::PushBackInt32ToArray(const int32_t value)
{
  if ( !m_pValue->IsArray() )
  {
    throw Standard_ProgramError("asiAlgo_JsonDocument::PushBackInt32ToArray() - pushed into not an array");
  }

  rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
  m_pValue->PushBack( value, pDoc->GetAllocator() );
}

//-----------------------------------------------------------------------------

void asiAlgo_JsonDocument::PushBackFloat64ToArray(const double value)
{
  if ( !m_pValue->IsArray() )
  {
    throw Standard_ProgramError("asiAlgo_JsonDocument::PushBackFloat64ToArray() - pushed into not an array");
  }

  rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
  m_pValue->PushBack( value, pDoc->GetAllocator() );
}

//-----------------------------------------------------------------------------

void asiAlgo_JsonDocument::PushBackSubDocToArray(asiAlgo_JsonDocument* pSubDoc)
{
  if ( !m_pValue->IsArray() )
  {
    throw Standard_ProgramError("asiAlgo_JsonDocument::PushBackSubDocToArray() - pushed into not an array");
  }

  if ( pSubDoc != nullptr )
  {
    rapidjson::Document* pDoc = (rapidjson::Document*) m_pValue;
    m_pValue->PushBack( *pSubDoc->m_pValue, pDoc->GetAllocator() );
  }
}

//-----------------------------------------------------------------------------

bool asiAlgo_JsonDocument::WriteIntoFile(const TCollection_AsciiString& outFilename) const
{
  std::ofstream fileStream;
  OSD_OpenStream(fileStream, outFilename.ToCString(), std::ios::out);

  bool result = false;
  rapidjson::OStreamWrapper streamWrapper(fileStream);
  if ( !m_bIsPretty )
  {
    rapidjson::Writer<rapidjson::OStreamWrapper> writer(streamWrapper);
    result = m_pValue->Accept(writer);
  }
  else
  {
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(streamWrapper);
    result = m_pValue->Accept(writer);
  }

  fileStream.close();
  return result && fileStream.good();
}

//-----------------------------------------------------------------------------

TCollection_AsciiString asiAlgo_JsonDocument::WriteIntoString() const
{
  TCollection_AsciiString result;

  rapidjson::StringBuffer buff;
  buff.Clear();

  bool isOk = false;
  if ( !m_bIsPretty )
  {
    rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
    isOk = m_pValue->Accept(writer);
  }
  else
  {
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
    isOk = m_pValue->Accept(writer);
  }

  if ( isOk )
  {
    result = TCollection_AsciiString( buff.GetString(),
                                      static_cast<int>( buff.GetSize() ) );
  }

  return result;
}

#endif
