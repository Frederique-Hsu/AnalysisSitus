//-----------------------------------------------------------------------------
// Created on: 12 October 2024
// Created by: Sergey SLYADNEV
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Quaoar Studio LLC
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

#ifndef asiData_Nesting3dNbhParameter_h
#define asiData_Nesting3dNbhParameter_h

// asiData includes
#include <asiData_Nesting3dNbhAttr.h>

// Active Data includes
#include <ActData_UserParameter.h>
#include <ActData_Common.h>
#include <ActData_ParameterDTO.h>

//-----------------------------------------------------------------------------
// Parameter DTO
//-----------------------------------------------------------------------------

//! Data Transfer Object (DTO) corresponding to data wrapped with
//! the Nesting Neighborhood Parameter without any OCAF connectivity.
class asiData_Nesting3dNbhDTO : public ActData_ParameterDTO
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_Nesting3dNbhDTO, ActData_ParameterDTO)

public:

  //! Constructor accepting GID.
  //! \param GID [in] GID.
  asiData_Nesting3dNbhDTO(const ActAPI_ParameterGID& GID)
  : ActData_ParameterDTO(GID, Parameter_UNDEFINED) {}

public:

  asiAlgo::nesting::Nbh3d Nbh; //!< Neighborhood.

};

//-----------------------------------------------------------------------------
// Parameter
//-----------------------------------------------------------------------------

//! Node Parameter representing positional neighborhood for 3D nesting.
class asiData_Nesting3dNbhParameter : public ActData_UserParameter
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiData_Nesting3dNbhParameter, ActData_UserParameter)

public:

  asiData_EXPORT static Handle(asiData_Nesting3dNbhParameter)
    Instance();

public:

  asiData_EXPORT void
    SetNeighborhood(const asiAlgo::nesting::Nbh3d& nbh,
                    const ActAPI_ModificationType  MType           = MT_Touched,
                    const bool                     doResetValidity = true,
                    const bool                     doResetPending  = true);

  asiData_EXPORT asiAlgo::nesting::Nbh3d
    GetNeighborhood();

protected:

  asiData_EXPORT
    asiData_Nesting3dNbhParameter();

private:

  virtual bool isWellFormed() const;
  virtual int parameterType() const;

private:

  virtual void
    setFromDTO(const Handle(ActData_ParameterDTO)& DTO,
               const ActAPI_ModificationType       MType = MT_Touched,
               const bool                          doResetValidity = true,
               const bool                          doResetPending = true);

  virtual Handle(ActData_ParameterDTO)
    createDTO(const ActAPI_ParameterGID& GID);

protected:

  //! Tags for the underlying CAF Labels.
  enum Datum
  {
    DS_Nbh = ActData_UserParameter::DS_DatumLast,
    DS_DatumLast = DS_Nbh + RESERVED_DATUM_RANGE
  };

};

#endif
