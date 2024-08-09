//-----------------------------------------------------------------------------
// Created on: 01 August 2024
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

#ifndef asiVisu_ShapeBVHDataProvider_h
#define asiVisu_ShapeBVHDataProvider_h

// asiVisu includes
#include <asiVisu_BVHDataProvider.h>

// asiAlgo includes
#include <asiAlgo_BVHFacets.h>

//-----------------------------------------------------------------------------

class asiVisu_BVHFacets : public asiVisu_BVHPrimitiveSet
{
  // OpenCascade RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_BVHFacets, asiVisu_BVHPrimitiveSet)

public:

  //! Ctor.
  asiVisu_BVHFacets(const Handle(asiAlgo_BVHFacets)& facets)
  {
    m_facets = facets;
  }

public:

  //! AABB provider for the primitive range `[primFirstId, primLastId]`.
  asiVisu_EXPORT virtual void
    GetBbox(const int  primFirstId,
            const int  primLastId,
            BVH_Vec3d& minCorner,
            BVH_Vec3d& maxCorner) const;

protected:

  Handle(asiAlgo_BVHFacets) m_facets;

};

//-----------------------------------------------------------------------------

//! BVH data provider for B-rep shapes.
class asiVisu_ShapeBVHDataProvider : public asiVisu_BVHDataProvider
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiVisu_ShapeBVHDataProvider, asiVisu_BVHDataProvider)

public:

  //! Ctor.
  //! \param[in] nodeId    the ID of the Data Node which contains a BVH Parameter to visualize.
  //! \param[in] paramList the input Parameters.
  asiVisu_EXPORT
    asiVisu_ShapeBVHDataProvider(const ActAPI_DataObjectId&           nodeId,
                                 const Handle(ActAPI_HParameterList)& paramList);

public:

  //! Returns associated Node ID.
  //! \return Node ID.
  virtual ActAPI_DataObjectId GetNodeID() const
  {
    return m_nodeID;
  }

public:

  //! \return the BVH tree to visualize.
  asiVisu_EXPORT virtual opencascade::handle<BVH_Tree<double, 3>>
    GetBVH() const;

  //! \return true if the BVH rendering mode is activated.
  asiVisu_EXPORT virtual bool
    IsRenderBVH() const;

  //! \return the selected level of the BVH tree to render alone. Returns
  //!         -1 if there is no level restriction set.
  asiVisu_EXPORT virtual int
    GetLevel() const;

  //! \return the Boolean flag indicating whether BVH is to be rendered in
  //!         wireframe mode.
  asiVisu_EXPORT virtual bool
    IsWireframe() const;

  //! \return the Boolean flag indicating whether the leaves-only mode is enabled.
  asiVisu_EXPORT bool
    IsLeavesOnly() const;

  //! \return the primitive set to give interpretation to BVH leaves.
  asiVisu_EXPORT virtual Handle(asiVisu_BVHPrimitiveSet)
    GetPrimitiveSet();

protected:

  //! Enumerates all Active Data Parameters playing as sources for DOMAIN -> VTK
  //! translation process. If any Parameter listed by this method is changed
  //! (more precisely, if its MTime record is updated), the translation must
  //! be repeated.
  //! \return list of source Parameters.
  asiVisu_EXPORT virtual Handle(ActAPI_HParameterList)
    translationSources() const;

protected:

  //! Source Node ID.
  ActAPI_DataObjectId m_nodeID;

  //! Source Parameters.
  Handle(ActAPI_HParameterList) m_params;

  //! Primitive set provider.
  Handle(asiVisu_BVHFacets) m_primSet;

};

#endif
