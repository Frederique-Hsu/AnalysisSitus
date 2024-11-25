//-----------------------------------------------------------------------------
// Created on: 11 October 2024
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

#ifndef asiEngine_Nesting3d_h
#define asiEngine_Nesting3d_h

// asiEngine includes
#include <asiEngine_Base.h>

// asiData includes
#include <asiData_Nesting3dCursorNode.h>
#include <asiData_Nesting3dNode.h>

//! API for the data model operations related to 3D nesting.
class asiEngine_Nesting3d : public asiEngine_Base
{
public:

  //! Ctor.
  //! \param[in] model    the data model instance.
  //! \param[in] progress the progress notifier.
  //! \param[in] plotter  the imperative plotter.
  asiEngine_Nesting3d(const Handle(asiEngine_Model)& model,
                      ActAPI_ProgressEntry           progress = nullptr,
                      ActAPI_PlotterEntry            plotter  = nullptr)
  //
  : asiEngine_Base(model, progress, plotter) {}

public:

  //! Creates the root Node for all 3D nesting data objects.
  //! \return the newly created Nesting 3D Node.
  asiEngine_EXPORT Handle(asiData_Nesting3dNode)
    Create_Nesting3d();

  //! Finds or creates a root Node for all 3D nesting objects.
  //! \return the existing or newly created Nesting 3D Node.
  asiEngine_EXPORT Handle(asiData_Nesting3dNode)
    Find_Nesting3d(const bool create = false);

  //! Creates the 3D nesting container data object.
  //! \param[in] name the container's name.
  //! \return the newly created Nesting Container Node.
  asiEngine_EXPORT Handle(asiData_Nesting3dContainerNode)
    Create_Container(const t_extString& name);

  //! Finds or creates a 3D Nesting Container Node.
  //! \param[in] name the container's name to look for.
  //! \return the existing or newly created Container Node.
  asiEngine_EXPORT Handle(asiData_Nesting3dContainerNode)
    Find_Container(const t_extString& name,
                   const bool         create = false);

  //! Creates the 3D nesting cursor data object.
  //! \return the newly created Nesting Cursor Node.
  asiEngine_EXPORT Handle(asiData_Nesting3dCursorNode)
    Create_Cursor();

  //! Finds or creates a 3D Nesting Cursor Node.
  //! \return the existing or newly created Cursor Node.
  asiEngine_EXPORT Handle(asiData_Nesting3dCursorNode)
    Find_Cursor(const bool create = false);

};

#endif
