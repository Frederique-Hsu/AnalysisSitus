//-----------------------------------------------------------------------------
// Created on: 28 December 2024
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

#pragma once

// asiAlgo includes
#include "asiAlgo.h"

// Active Data includes
#include <ActData_Mesh.h>

// OpenCascade includes
#include <Standard_Type.hxx>

// Forward declarations.
class Poly_Triangulation;
class TColStd_PackedMapOfInteger;

//! \ingroup ASI_MODELING
//!
//! This class performs submesh extraction using element indexes.
class asiAlgo_MeshExtractPortion
{
public:

  //! Empty constructor. Does nothing.
  asiAlgo_MeshExtractPortion()
  {}

  //! Constructor which sets input mesh.
  //! \param mesh [in] mesh to inspect.
  asiAlgo_MeshExtractPortion(const Handle(Poly_Triangulation)& mesh)
  : m_tris(mesh)
  {}

  //! Constructor which sets input mesh.
  //! \param mesh [in] mesh to inspect.
  asiAlgo_MeshExtractPortion(const Handle(ActData_Mesh)& mesh)
  : m_mesh(mesh)
  {}

public:

  //! Sets triangulation to extract submesh from.
  //! \param mesh [in] mesh to extact sub-mesh.
  void SetTriangulation(const Handle(Poly_Triangulation)& mesh)
  { m_tris = mesh; }

  //! Sets mesh to extract submesh from.
  //! \param mesh [in] mesh to extact sub-mesh.
  void SetMesh(const Handle(ActData_Mesh)& mesh)
  { m_mesh = mesh; }

public:

  //! Extracts submesh.
  //! \param elemIds [in]  Indexes of the mesh elements to extract.
  //! \param subMesh [out] Resulting submesh.
  //! \return done state, true in case of success and false otherwise.
  asiAlgo_EXPORT virtual bool
    Perform(const TColStd_PackedMapOfInteger& elemIds,
            Handle(Poly_Triangulation)&       subMesh);

  //! Extracts submesh.
  //! \param elemIdx [in]  Index of the mesh element to extract.
  //! \param subMesh [out] Resulting submesh.
  //! \return done state, true in case of success and false otherwise.
  asiAlgo_EXPORT virtual bool
    Perform(const int                   elemIdx,
            Handle(Poly_Triangulation)& subMesh);

private:

  //! Triangulation data structure to extract submesh from.
  Handle(Poly_Triangulation) m_tris;

  //! Mesh data structure to extract submesh from.
  Handle(ActData_Mesh) m_mesh;

};
