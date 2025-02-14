//-----------------------------------------------------------------------------
// Created on: 28 September 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Quaoar Studio LLC (www.quaoar-studio.com)
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
#include <asiAlgo_Shaft.h>

// asiAlgo includes
#include <asiAlgo_JsonDict.h>
#include <asiAlgo_Utils.h>

#if defined USE_RAPIDJSON

// Rapidjson includes
#include <rapidjson/document.h>

typedef rapidjson::Document::Array     t_jsonArray;
typedef rapidjson::Document::ValueType t_jsonValue;

#endif

//-----------------------------------------------------------------------------

asiAlgo_Shaft::asiAlgo_Shaft()
//
: Standard_Transient (),
  diameter           (0.),
  length             (0.)
{}

//-----------------------------------------------------------------------------

bool asiAlgo_Shaft::IsEqual(const Handle(asiAlgo_Shaft)& other,
                            const double                 linToler,
                            const double                 angTolDeg) const
{
  if ( !this->fids.IsEqual(other->fids) )
    return false;

  if ( Abs(this->diameter - other->diameter) > linToler )
    return false;

  if ( Abs(this->length - other->length) > linToler )
    return false;

  if ( !asiAlgo_Utils::Verify::AreEqualAxes(this->axis, other->axis, angTolDeg) )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_Shaft::FromJSON(void*                  pJsonGenericObj,
                             Handle(asiAlgo_Shaft)& shaft)
{
#if defined USE_RAPIDJSON
  t_jsonValue*
    pJsonObj = reinterpret_cast<t_jsonValue*>(pJsonGenericObj);

  // Iterate members of the shaft object.
  for ( t_jsonValue::MemberIterator mit = pJsonObj->MemberBegin();
        mit != pJsonObj->MemberEnd(); mit++ )
  {
    std::string prop( mit->name.GetString() );

    // Face IDs.
    if ( prop == asiPropName_FaceIds )
    {
      if ( shaft.IsNull() )
        shaft = new asiAlgo_Shaft;

      t_jsonArray arr = mit->value.GetArray();
      asiAlgo_Utils::Json::ReadFeature(&arr, shaft->fids);
    }

    // Diameter.
    else if ( prop == asiPropName_Diameter )
    {
      if ( shaft.IsNull() )
        shaft = new asiAlgo_Shaft;

      shaft->diameter = mit->value.GetDouble();
    }

    // Length.
    else if ( prop == asiPropName_Length )
    {
      if ( shaft.IsNull() )
        shaft = new asiAlgo_Shaft;

      shaft->length = mit->value.GetDouble();
    }

    // Axis.
    else if ( prop == asiPropName_Axis )
    {
      if ( !mit->value.IsNull() )
      {
        if ( shaft.IsNull() )
          shaft = new asiAlgo_Shaft;

        t_jsonArray arr = mit->value.GetArray();

        gp_XYZ coords;
        asiAlgo_Utils::Json::ReadCoords(&arr, coords);
        //
        if ( coords.Modulus() > RealEpsilon() )
          shaft->axis = gp_Ax1(gp::Origin(), coords);
      }
    }
  }
#endif
}

//-----------------------------------------------------------------------------

void asiAlgo_Shaft::ToJSON(const Handle(asiAlgo_Shaft)& shaft,
                           const int                    indent,
                           std::ostream&                out)
{
  std::string ws(indent, ' ');
  std::string nl = "\n" + ws;

  /* Dump props */

  out <<        nl << "\"" << asiPropName_FaceIds  << "\": " << asiAlgo_Utils::Json::FromFeature(shaft->fids);
  out << "," << nl << "\"" << asiPropName_Diameter << "\": " << shaft->diameter;
  out << "," << nl << "\"" << asiPropName_Length   << "\": " << shaft->length;
  out << "," << nl << "\"" << asiPropName_Axis     << "\": " << asiAlgo_Utils::Json::FromDirAsTuple( shaft->axis.Direction() );
}
