//-----------------------------------------------------------------------------
// Created on: 13 April 2025
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
#include <asiAlgo_NamingInfo.h>

#if defined USE_RAPIDJSON

// Rapidjson includes
#include <rapidjson/document.h>

typedef rapidjson::Document::Array     t_jsonArray;
typedef rapidjson::Document::ValueType t_jsonValue;

#endif

//-----------------------------------------------------------------------------

void asiAlgo_NamingInfo::FromJSON(void*               pJsonGenericObj,
                                  asiAlgo_NamingInfo& info)
{
#if defined USE_RAPIDJSON
  t_jsonValue*
    pJsonObj = reinterpret_cast<t_jsonValue*>(pJsonGenericObj);

  // Iterate members.
  for ( t_jsonValue::MemberIterator mit = pJsonObj->MemberBegin();
        mit != pJsonObj->MemberEnd(); mit++ )
  {
    std::string prop( mit->name.GetString() );

    // Face ID.
    if ( prop == asiPropName_FaceId )
    {
      if ( !mit->value.IsNull() )
        info.fid = mit->value.GetInt();
    }

    // Name.
    else if ( prop == asiPropName_Name )
    {
      if ( !mit->value.IsNull() )
        info.name = mit->value.GetString();
    }
  }
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_NamingInfo::ToJSON(const asiAlgo_NamingInfo& info,
                                const int                 indent,
                                std::ostream&             out,
                                const bool                pureJSON)
{
  std::string ws = pureJSON ? "" : std::string(indent, ' ');
  std::string nl = pureJSON ? "" : "\n" + ws;
  std::string qt = "\"";

  {
    // Face ID.
    out << nl << qt << asiPropName_FaceId << qt << ": " << info.fid;

    // Name.
    out << "," << nl << qt << asiPropName_Type << qt << ": " << qt << info.name << qt;
  }
}
