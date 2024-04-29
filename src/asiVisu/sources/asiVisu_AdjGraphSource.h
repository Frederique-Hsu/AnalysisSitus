//-----------------------------------------------------------------------------
// Created on: 27 April 2024
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

#ifndef asiVisu_AdjGraphSource_h
#define asiVisu_AdjGraphSource_h

// asiVisu includes
#include <asiVisu_Utils.h>

// asiAlgo includes
#include <asiAlgo_AAG.h>

// VTK includes
#include <vtkPolyDataAlgorithm.h>
#include <vtkSmartPointer.h>
#include <vtkType.h>

//-----------------------------------------------------------------------------

//! This class provides a source of polygonal data for visualizing face
//! adjacency graphs embedded in a three-dimensional model. The aim behind
//! such visualization is to be able to observe AAG in the same scene as
//! the model, rather than in a separate graph view. We adapted this concept
//! from the "occwl" software suite by Autodesk AI Lab.
class asiVisu_AdjGraphSource : public vtkPolyDataAlgorithm
{
// RTTI and construction:
public:

  vtkTypeMacro(asiVisu_AdjGraphSource, vtkPolyDataAlgorithm);

  asiVisu_EXPORT static asiVisu_AdjGraphSource*
    New();

// Kernel methods:
public:

  //! Initializes the data source algorithm with the AAG to visualize.
  //! \param[in] aag the graph to visualize.
  //! \return true if the poly data has been produced.
  asiVisu_EXPORT bool
    SetInputGraph(const Handle(asiAlgo_AAG)& aag);

protected:

  //! This method (called by superclass) performs conversion of our native
  //! data structures to VTK polygonal representation.
  //!
  //! \param[in]  request      describes "what" algorithm should do. This is
  //!                          typically just one key such as REQUEST_INFORMATION.
  //! \param[in]  inputVector  inputs of the algorithm.
  //! \param[out] outputVector outputs of the algorithm.
  asiVisu_EXPORT virtual int
    RequestData(vtkInformation*        request,
                vtkInformationVector** inputVector,
                vtkInformationVector*  outputVector);

protected:

  //! Adds the given point to the passed polygonal data set.
  //! \param[in]     point    point to add.
  //! \param[in,out] polyData polygonal data set being populated.
  //! \return ID of the just added VTK point.
  asiVisu_EXPORT vtkIdType
    registerGridPoint(const gp_Pnt& point,
                      vtkPolyData*  polyData);

  //! Adds a line cell into the polygonal data set.
  //! \param[in]     ptStart  first point ID.
  //! \param[in]     ptEnd    second point ID.
  //! \param[in]     type     vexity of the corresponding dihedral angle.
  //! \param[in,out] polyData polygonal data set being populated.
  //! \return ID of the just added VTK cell.
  asiVisu_EXPORT vtkIdType
    registerLine(const vtkIdType      ptStart,
                 const vtkIdType      ptEnd,
                 const asiVisu_Vexity type,
                 vtkPolyData*         polyData);

protected:

  //! Ctor.
  asiVisu_EXPORT
    asiVisu_AdjGraphSource();

  //! Dtor.
  asiVisu_EXPORT
    ~asiVisu_AdjGraphSource();

private:

  asiVisu_AdjGraphSource(const asiVisu_AdjGraphSource&);
  asiVisu_AdjGraphSource& operator=(const asiVisu_AdjGraphSource&);

private:

  Handle(asiAlgo_AAG) m_aag; //!< AAG to visualize.

};

#endif
