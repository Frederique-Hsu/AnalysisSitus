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

#ifndef asiVisu_BVHSource_h
#define asiVisu_BVHSource_h

// asiVisu includes
#include <asiVisu.h>

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkType.h>
#include <vtkUnstructuredGridAlgorithm.h>

// OpenCascade includes
#include <BVH_Tree.hxx>

#define BVHSource_Scalar_Left  0.
#define BVHSource_Scalar_Right 1.
#define BVHSource_Scalar_Root  2.

//-----------------------------------------------------------------------------

//! Source of unstructured data for a BVH tree.
class asiVisu_BVHSource : public vtkUnstructuredGridAlgorithm
{
// RTTI and construction:
public:

  vtkTypeMacro(asiVisu_BVHSource, vtkUnstructuredGridAlgorithm)

  asiVisu_EXPORT static asiVisu_BVHSource*
    New();

public:

  //! Sets the BVH to visualize.
  //! \param[in] bvh the BVH to set.
  asiVisu_EXPORT void
    SetInputBVH(const opencascade::handle<BVH_Tree<double, 3>>& bvh);

  //! Sets the tree level to visualize.
  //! \param[in] level the level to set.
  asiVisu_EXPORT void
    SetInputLevel(const int level);

  //! Enables/disables wireframe visualization mode.
  //! \param[in] on true/false.
  asiVisu_EXPORT void
    SetWireframeMode(const bool on);

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

  //! Adds VTK voxel to the unstructured data set being constructed.
  //!
  //! \param[in]     node0  coordinates of the 1-st node.
  //! \param[in]     node1  coordinates of the 2-nd node.
  //! \param[in]     node2  coordinates of the 3-rd node.
  //! \param[in]     node3  coordinates of the 4-th node.
  //! \param[in]     node4  coordinates of the 5-th node.
  //! \param[in]     node5  coordinates of the 6-th node.
  //! \param[in]     node6  coordinates of the 7-th node.
  //! \param[in]     node7  coordinates of the 8-th node.
  //! \param[in]     isLeft true for the left BVH branch, false for the right.
  //! \param[in,out] pData  unstructured data set being populated.
  //! \return ID of the just added VTK cell.
  vtkIdType
    registerVoxel(const gp_Pnt&        node0,
                  const gp_Pnt&        node1,
                  const gp_Pnt&        node2,
                  const gp_Pnt&        node3,
                  const gp_Pnt&        node4,
                  const gp_Pnt&        node5,
                  const gp_Pnt&        node6,
                  const gp_Pnt&        node7,
                  const bool           isLeft,
                  vtkUnstructuredGrid* pData);

  //! Adds VTK voxel as a wireframe to the unstructured data set being
  //! constructed. The real cell is not a voxel but 12 VTK_LINE cells.
  //!
  //! \param[in]     node0  coordinates of the 1-st node.
  //! \param[in]     node1  coordinates of the 2-nd node.
  //! \param[in]     node2  coordinates of the 3-rd node.
  //! \param[in]     node3  coordinates of the 4-th node.
  //! \param[in]     node4  coordinates of the 5-th node.
  //! \param[in]     node5  coordinates of the 6-th node.
  //! \param[in]     node6  coordinates of the 7-th node.
  //! \param[in]     node7  coordinates of the 8-th node.
  //! \param[in]     scalar scalar value to associate with cell points.
  //! \param[in,out] pData  unstructured data set being populated.
  void
    registerVoxelWireframe(const gp_Pnt&        node0,
                           const gp_Pnt&        node1,
                           const gp_Pnt&        node2,
                           const gp_Pnt&        node3,
                           const gp_Pnt&        node4,
                           const gp_Pnt&        node5,
                           const gp_Pnt&        node6,
                           const gp_Pnt&        node7,
                           const double         scalar,
                           vtkUnstructuredGrid* pData);

  //! Adds a line cell into the unstructured grid data set.
  //! \param[in]     ptStart the first point.
  //! \param[in]     ptEnd   the second point.
  //! \param[in]     scalar  the scalar value to associate with cell points.
  //! \param[in,out] pData   the unstructured data set being populated.
  //! \return ID of the just added VTK cell.
  vtkIdType
    registerLine(const gp_Pnt&        ptStart,
                 const gp_Pnt&        ptEnd,
                 const double         scalar,
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
  asiVisu_BVHSource();

  //! Destructor.
  ~asiVisu_BVHSource();

private:

  asiVisu_BVHSource(const asiVisu_BVHSource&) = delete;
  asiVisu_BVHSource& operator=(const asiVisu_BVHSource&) = delete;

private:

  //! BVH to visualize.
  opencascade::handle<BVH_Tree<double, 3>> m_bvh;

  //! Tree level to select for visualization.
  double m_iLevel;

  //! Wireframe mode of visualization on/off.
  bool m_bWireframe;

  //! Progress notifier.
  ActAPI_ProgressEntry m_progress;

  //! Imperative plotter.
  ActAPI_PlotterEntry m_plotter;

};

#endif
