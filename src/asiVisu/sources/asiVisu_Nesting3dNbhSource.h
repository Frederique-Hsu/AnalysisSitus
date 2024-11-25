//-----------------------------------------------------------------------------
// Created on: 12 October 2024
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

#ifndef asiVisu_Nesting3dNbhSource_h
#define asiVisu_Nesting3dNbhSource_h

// asiVisu includes
#include <asiVisu.h>

// asiAlgo includes
#include <asiAlgo_NestingNbh3d.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkType.h>
#include <vtkUnstructuredGridAlgorithm.h>

//-----------------------------------------------------------------------------

//! Source of unstructured data for a positional neighborhood cursor of
//! 3D nesting (packing) algorithm.
class asiVisu_Nesting3dNbhSource : public vtkUnstructuredGridAlgorithm
{
// RTTI and construction:
public:

  vtkTypeMacro(asiVisu_Nesting3dNbhSource, vtkUnstructuredGridAlgorithm)

  asiVisu_EXPORT static asiVisu_Nesting3dNbhSource*
    New();

public:

  //! Sets the neighborhood to visualize.
  //! \param[in] nbh the neighborhood to set.
  asiVisu_EXPORT void
    SetInputNbh(const asiAlgo::nesting::Nbh3d& nbh);

public:

  //! Initializes source with diagnostic tools: progress notifier and
  //! imperative plotter.
  //! \param progress [in] progress notifier.
  //! \param plotter  [in] imperative plotter.
  void SetDiagnosticTools(ActAPI_ProgressEntry progress,
                          ActAPI_PlotterEntry  plotter)
  {
    m_progress = progress;
    m_plotter  = plotter;
  }

protected:

  //! This method (called by superclass) performs conversion of our native
  //! data structures to VTK unstructured grid.
  //!
  //! \param[in]  request      describes "what" algorithm should do. This is
  //!                          typically just one key such as REQUEST_INFORMATION.
  //! \param[in]  inputVector  inputs of the algorithm.
  //! \param[out] outputVector outputs of the algorithm.
  //! \return status.
  asiVisu_EXPORT virtual int
    RequestData(vtkInformation*        request,
                vtkInformationVector** inputVector,
                vtkInformationVector*  outputVector);

private:

  //! Populates unstructured grid with the points from the passed axis.
  void populateAxis(const Handle(asiAlgo::nesting::Container3dGrid)& grid,
                    const std::vector<asiAlgo::nesting::Pos3d>&      nodes,
                    vtkUnstructuredGrid*                             pData);

  //! Adds a line cell into the unstructured grid data set.
  //! \param[in]     ptStart the first point.
  //! \param[in]     ptEnd   the second point.
  //! \param[in,out] pData   the unstructured data set being populated.
  //! \return ID of the just added VTK cell.
  vtkIdType
    registerLine(const gp_Pnt&        ptStart,
                 const gp_Pnt&        ptEnd,
                 vtkUnstructuredGrid* pData);

  //! Adds the passed coordinates as another point to the VTK data set.
  //! \param[in]     coords coordinates of the point to add.
  //! \param[in,out] pData  unstructured grid being constructed.
  //! \return ID of the newly added point.
  vtkIdType
    addPoint(const gp_Pnt&        coords,
             vtkUnstructuredGrid* pData);

private:

  //! Default constructor.
  asiVisu_Nesting3dNbhSource();

  //! Destructor.
  ~asiVisu_Nesting3dNbhSource();

private:

  asiVisu_Nesting3dNbhSource(const asiVisu_Nesting3dNbhSource&) = delete;
  asiVisu_Nesting3dNbhSource& operator=(const asiVisu_Nesting3dNbhSource&) = delete;

private:

  //! Neighborhood to visualize.
  asiAlgo::nesting::Nbh3d m_nbh;

  //! Progress notifier.
  ActAPI_ProgressEntry m_progress;

  //! Imperative plotter.
  ActAPI_PlotterEntry m_plotter;

};

#endif
