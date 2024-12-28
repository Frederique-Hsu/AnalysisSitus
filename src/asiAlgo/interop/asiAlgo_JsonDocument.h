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

#ifndef asiAlgo_JsonDocument_HeaderFile
#define asiAlgo_JsonDocument_HeaderFile

// asiAlgo includes
#include <asiAlgo.h>

// OpenCascade includes
#include <Standard_Type.hxx>

#if defined USE_RAPIDJSON

// Rapidjson includes
#include <rapidjson/document.h>

typedef rapidjson::Document::Array     t_jsonArray;
typedef rapidjson::Document::ValueType t_jsonValue;
typedef rapidjson::Document::Object    t_jsonObject;

#endif

class TCollection_AsciiString;

//-----------------------------------------------------------------------------

//! \ingroup ASI_INTEROP
//!
//! Interface to JSON document.
class asiAlgo_JsonDocument : public Standard_Transient
{
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_JsonDocument, Standard_Transient)

#if defined USE_RAPIDJSON

public:

  //! Ctor.
  asiAlgo_EXPORT
    asiAlgo_JsonDocument();

  //! Creates a copy of the passed JSON document.
  asiAlgo_EXPORT
    asiAlgo_JsonDocument(t_jsonValue* pValue, const bool copyValue = false);

  //! Dtor.
  asiAlgo_EXPORT virtual
    ~asiAlgo_JsonDocument();

public:

  //! Loads document from file.
  //! \param[in] inFilename target filename.
  asiAlgo_EXPORT virtual bool
    LoadFromFile(const TCollection_AsciiString& inFilename);

  //! Creates document from string.
  //! \param[in] inString input string with JSON data.
  asiAlgo_EXPORT virtual bool
    LoadFromString(const TCollection_AsciiString& inString);

  //! Returns true if the JSON document contains a property
  //! with the given name.
  //! \param[in] name property name to check.
  //! \return true/false.
  asiAlgo_EXPORT virtual bool
    Contains(const std::string& name) const;

  //! Returns value of a property with the given name or
  //! the specified default value if the property is not found.
  //! \param[in] name     property name in question.
  //! \param[in] defValue default value.
  //! \return value of the property.
  asiAlgo_EXPORT virtual int32_t
    GetInt32(const std::string& name, const int32_t defValue) const;

  //! Returns value of a property with the given name or
  //! the specified default value if the property is not found.
  //! \param[in] name     property name in question.
  //! \param[in] defValue default value.
  //! \return value of the property.
  asiAlgo_EXPORT virtual double
    GetFloat64(const std::string& name, const double defValue) const;

  //! Returns string value of a property with the given name or
  //! the specified default string if the property is not found.
  //! \param[in] name     property name in question.
  //! \param[in] defValue default string.
  //! \return string value of the property.
  asiAlgo_EXPORT virtual const char*
    GetString(const std::string& name, const char* defValue) const;

  //! Returns subdocument as a property with the given name.
  //! \param[in] name    property name in question.
  //! \param[in] copyDoc indicates whether the returned subdocument
  //!                    should reuse the memory allocated for the
  //!                    current document.
  //! \return subdocument.
  asiAlgo_EXPORT virtual Handle(asiAlgo_JsonDocument)
    GetSubDocument(const std::string& name, const bool copyDoc = false) const;

  //! Returns array as a property with the given name.
  //! \param[in] name property name in question.
  //! \return array as another document.
  asiAlgo_EXPORT Handle(asiAlgo_JsonDocument)
    GetArray(const std::string& name) const;

  //! \return number of memebers in the object.
  asiAlgo_EXPORT int
    GetMemberCount() const;

  //! Checks recursively if this document contains data
  //! for the given property name.
  //! \param[in] name the property name to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    HasData(const std::string& name) const;

  //! Checks recursively if this document contains the passed
  //! diagnostic code.
  //! \param[in] code the diagnostic code to check.
  //! \return true/false.
  asiAlgo_EXPORT bool
    HasDiagnosticCode(const int code) const;

public:

  //! Accessor for the internal pointer.
  //! \return raw pointer to the underlying JSON value.
  t_jsonValue* GetValuePtr()
  {
    return m_pValue;
  }

  //! Creates uninitialized subdocument.
  //! \return new subdocument.
  Handle(asiAlgo_JsonDocument) MakeSubDocument() const
  {
    return new asiAlgo_JsonDocument();
  }

  //! Creates an array.
  //! \return created array as a subdocument.
  virtual Handle(asiAlgo_JsonDocument) MakeArray() const
  {
    t_jsonValue* pValue = new rapidjson::Document();
    pValue->SetArray();
    return new asiAlgo_JsonDocument(pValue);
  }

  //! Returns format output.
  bool IsPrettyFormat() const
  {
    return m_bIsPretty;
  }

  //! Sets format output.
  //! \param[in] isPretty mode to set.
  void SetPrettyFormat(const bool isPretty)
  {
    m_bIsPretty = isPretty;
  }

public:

  //! Dumps document to file.
  //! \param[in] outFilename target filename.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT virtual bool
    WriteIntoFile(const TCollection_AsciiString& outFilename) const;

  //! Dumps document to string.
  //! \return document as string.
  asiAlgo_EXPORT virtual TCollection_AsciiString
    WriteIntoString() const;

  //! Appends field with given name and value.
  asiAlgo_EXPORT virtual void
    SetInt32(const std::string& name,
             const int32_t      value);

  //! Appends field with given name and value.
  asiAlgo_EXPORT virtual void
    SetFloat64(const std::string& name,
               const double       value);

  //! Appends field with given name and value.
  asiAlgo_EXPORT virtual void
    SetString(const std::string& name,
              const char*        value);

  //! Appends field with given name and value.
  asiAlgo_EXPORT virtual void
    SetSubDocument(const std::string&    name,
                   asiAlgo_JsonDocument* pDoc);

  //! Appends field with given name and value.
  asiAlgo_EXPORT virtual void
    SetArray(const std::string&    name,
             asiAlgo_JsonDocument* pArray);

  //! Appends field with given name and value.
  asiAlgo_EXPORT virtual void
    PushBackInt32ToArray(const int32_t value);

  //! Appends field with given name and value.
  asiAlgo_EXPORT virtual void
    PushBackFloat64ToArray(const double value);

  //! Appends field with given name and value.
  asiAlgo_EXPORT virtual void
    PushBackSubDocToArray(asiAlgo_JsonDocument* pSubDoc); 

protected:

  t_jsonValue* m_pValue;     //!< Pointer to the document or its property.
  bool         m_bToRelease; //!< Whether to release instead of reuse memory.
  bool         m_bIsDoc;     //!< Indicates whether this object is a document or a property.
  bool         m_bIsPretty;  //!< Indicates whether a "pretty" output format is enabled.
#endif

};
#endif
#endif
