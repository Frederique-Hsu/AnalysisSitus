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

// Own include
#include <asiEngine_Nesting3d.h>

// asiData includes
#include <asiData_Nesting3dNode.h>
#include <asiData_Nesting3dContainerNode.h>
#include <asiData_Nesting3dCursorNode.h>

//-----------------------------------------------------------------------------

Handle(asiData_Nesting3dNode) asiEngine_Nesting3d::Create_Nesting3d()
{
  // Create Node.
  Handle(asiData_Nesting3dNode)
    N = Handle(asiData_Nesting3dNode)::DownCast( asiData_Nesting3dNode::Instance() );
  //
  m_model->GetNesting3dPartition()->AddNode(N);

  // Initialize.
  N->Init();
  N->SetName("Nesting 3D");

  // Add as child to the Root Node.
  m_model->GetRootNode()->AddChildNode(N);

  return N;
}

//-----------------------------------------------------------------------------

Handle(asiData_Nesting3dNode)
  asiEngine_Nesting3d::Find_Nesting3d(const bool create)
{
  Handle(asiData_Nesting3dNode) N;

  // Lookup the partition.
  Handle(asiData_Partition<asiData_Nesting3dNode>) P = m_model->GetNesting3dPartition();
  //
  for ( ActData_BasePartition::Iterator pit(P); pit.More(); )
  {
    N = Handle(asiData_Nesting3dNode)::DownCast( pit.Value() );
    break;
  }
  //
  if ( !N.IsNull() )
    return N;

  // If here, no Nesting 3D Node is found.
  return create ? this->Create_Nesting3d() : nullptr;
}

//-----------------------------------------------------------------------------

Handle(asiData_Nesting3dContainerNode)
  asiEngine_Nesting3d::Create_Container(const t_extString& name)
{
  Handle(asiData_Nesting3dNode)
    NN = asiEngine_Nesting3d::Find_Nesting3d();

  // Create Node.
  Handle(asiData_Nesting3dContainerNode)
    N = Handle(asiData_Nesting3dContainerNode)::DownCast( asiData_Nesting3dContainerNode::Instance() );
  //
  m_model->GetNesting3dContainerPartition()->AddNode(N);

  // Initialize.
  N->Init();
  N->SetName(name);

  // Add as child to the Nesting 3D Node.
  NN->AddChildNode(N);

  return N;
}

//-----------------------------------------------------------------------------

Handle(asiData_Nesting3dContainerNode)
  asiEngine_Nesting3d::Find_Container(const t_extString& name,
                                      const bool         create)
{
  Handle(asiData_Nesting3dContainerNode) N;

  // Lookup the partition.
  Handle(asiData_Partition<asiData_Nesting3dContainerNode>) P = m_model->GetNesting3dContainerPartition();
  //
  for ( ActData_BasePartition::Iterator pit(P); pit.More(); )
  {
    Handle(asiData_Nesting3dContainerNode)
      nextNode = Handle(asiData_Nesting3dContainerNode)::DownCast( pit.Value() );
    //
    if ( nextNode->GetName() == name )
    {
      N = nextNode;
      break;
    }
  }
  //
  if ( !N.IsNull() )
    return N;

  // If here, no Nesting Container is found.
  return create ? this->Create_Container(name) : nullptr;
}

//-----------------------------------------------------------------------------

Handle(asiData_Nesting3dCursorNode) asiEngine_Nesting3d::Create_Cursor()
{
  Handle(asiData_Nesting3dNode)
    NN = asiEngine_Nesting3d::Find_Nesting3d();

  // Create Node.
  Handle(asiData_Nesting3dCursorNode)
    N = Handle(asiData_Nesting3dCursorNode)::DownCast( asiData_Nesting3dCursorNode::Instance() );
  //
  m_model->GetNesting3dCursorPartition()->AddNode(N);

  // Initialize.
  N->Init();
  N->SetName("Cursor");
  N->SetUserFlags(NodeFlag_IsPresentedInPartView | NodeFlag_IsPresentationVisible);

  // Add as child to the Nesting 3D Node.
  NN->AddChildNode(N);

  return N;
}

//-----------------------------------------------------------------------------

Handle(asiData_Nesting3dCursorNode)
  asiEngine_Nesting3d::Find_Cursor(const bool create)
{
  Handle(asiData_Nesting3dCursorNode) N;

  // Lookup the partition.
  Handle(asiData_Partition<asiData_Nesting3dCursorNode>) P = m_model->GetNesting3dCursorPartition();
  //
  for ( ActData_BasePartition::Iterator pit(P); pit.More(); )
  {
    N = Handle(asiData_Nesting3dCursorNode)::DownCast( pit.Value() );
    break;
  }
  //
  if ( !N.IsNull() )
    return N;

  // If here, no Nesting Cursor is found.
  return create ? this->Create_Cursor() : nullptr;
}
