//-----------------------------------------------------------------------------
// Created on: 14 June 2024
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
#include <asiAlgo_SemanticCode.h>

// asiAlgo includes
#include <asiAlgo_JsonDict.h>
#include <asiAlgo_Utils.h>

#if defined USE_RAPIDJSON

// Rapidjson includes
#include <rapidjson/document.h>

typedef rapidjson::Document::Array     t_jsonArray;
typedef rapidjson::Document::ValueType t_jsonValue;
typedef rapidjson::Document::Object    t_jsonObject;

#endif

//-----------------------------------------------------------------------------

void asiAlgo_SemanticCode::FromJSON(void*                 pJsonGenericObj,
                                    asiAlgo_SemanticCode& code)
{
#if defined USE_RAPIDJSON
  t_jsonValue*
    pJsonObj = reinterpret_cast<t_jsonValue*>(pJsonGenericObj);

  for ( t_jsonValue::MemberIterator propIt = pJsonObj->MemberBegin();
        propIt != pJsonObj->MemberEnd(); propIt++ )
  {
    std::string propName( propIt->name.GetString() );

    if ( propName == asiPropName_Code )
    {
      code.code = propIt->value.GetInt();
    }
    //
    else if ( propName == asiPropName_Label )
    {
      code.label = propIt->value.GetString();
    }
    //
    else if ( propName == asiPropName_FaceIds )
    {
      t_jsonArray arr = propIt->value.GetArray();
      asiAlgo_Utils::Json::ReadFeature(&arr, code.faceIds);
    }
    //
    else if ( propName == asiPropName_EdgeIds )
    {
      t_jsonArray arr = propIt->value.GetArray();
      asiAlgo_Utils::Json::ReadFeature(&arr, code.edgeIds);
    }
    //
    else if ( propName == asiPropName_VertexIds )
    {
      t_jsonArray arr = propIt->value.GetArray();
      asiAlgo_Utils::Json::ReadFeature(&arr, code.vertexIds);
    }
  }
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_SemanticCode::ToJSON(const asiAlgo_SemanticCode& code,
                                  const int                   indent,
                                  std::ostream&               out)
{
  std::string ws(indent, ' ');
  std::string nl = "\n" + ws;

  /* Dump props */

  out << nl << "\"" << asiPropName_Code << "\": " << code.code;

  if ( !code.label.empty() )
    out << "," << nl << "\"" << asiPropName_Label << "\": \"" << code.label << "\"";

  if ( !code.faceIds.IsEmpty() )
    out << "," << nl << "\"" << asiPropName_FaceIds << "\": " << asiAlgo_Utils::Json::FromFeature(code.faceIds);

  if ( !code.edgeIds.IsEmpty() )
    out << "," << nl << "\"" << asiPropName_EdgeIds << "\": " << asiAlgo_Utils::Json::FromFeature(code.edgeIds);

  if ( !code.vertexIds.IsEmpty() )
    out << "," << nl << "\"" << asiPropName_VertexIds << "\": " << asiAlgo_Utils::Json::FromFeature(code.vertexIds);
}
