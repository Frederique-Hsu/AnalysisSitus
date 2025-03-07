//-----------------------------------------------------------------------------
// Created on: April 2012
//-----------------------------------------------------------------------------
// Copyright (c) 2017, OPEN CASCADE SAS
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
//    * Neither the name of OPEN CASCADE SAS nor the
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
//
// Web: http://analysissitus.org
//-----------------------------------------------------------------------------

// Own include
#include <ActData_MeshAttr.h>

// OCCT includes
#include <Standard_GUID.hxx>
#include <Standard_ImmutableObject.hxx>
#include <Standard_ProgramError.hxx>
#include <TDF_Data.hxx>

// Mesh includes
#include <ActData_Mesh_ElementsIterator.h>
#include <ActData_Mesh_Node.h>
#include <ActData_Mesh_Quadrangle.h>
#include <ActData_Mesh_Triangle.h>

#undef COUT_DEBUG

//-----------------------------------------------------------------------------
// Construction & settling-down routines
//-----------------------------------------------------------------------------

//! Default constructor.
ActData_MeshAttr::ActData_MeshAttr() : TDF_Attribute()
{
}

//! Settles down new Mesh Attribute to the given CAF Label.
//! \param Label [in] TDF Label to settle down the new Mesh Attribute onto.
//! \return newly created Mesh Attribute settled down onto the target Label.
Handle(ActData_MeshAttr) ActData_MeshAttr::Set(const TDF_Label& Label)
{
  Handle(ActData_MeshAttr) aMeshAttr;
  if ( !Label.FindAttribute(GUID(), aMeshAttr) )
  {
    aMeshAttr = new ActData_MeshAttr();
    Label.AddAttribute(aMeshAttr);

    aMeshAttr->NewEmptyMesh(); // Initialize with empty Mesh DS
  }
  return aMeshAttr;
}

//-----------------------------------------------------------------------------
// Accessors for Attribute's GUID
//-----------------------------------------------------------------------------

//! Returns statically defined GUID for Mesh Attribute.
//! \return statically defined GUID.
const Standard_GUID& ActData_MeshAttr::GUID()
{
  static Standard_GUID AttrGUID("6554F48B-EA68-4DBC-A8EC-D379D2BD7A54");
  return AttrGUID;
}

//! Accessor for GUID associated with this kind of CAF Attribute.
//! \return GUID of the CAF Attribute.
const Standard_GUID& ActData_MeshAttr::ID() const
{
  return GUID();
}

//-----------------------------------------------------------------------------
// Attribute's kernel methods:
//-----------------------------------------------------------------------------

//! Creates new instance of Mesh Attribute which is not initially populated
//! with any mesh structures. This method is mainly used by OCAF Undo/Redo
//! kernel as a part of Backup functionality. The preparation of Backup copy
//! of any CAF Attribute starts with NewEmpty invocation.
//! \return new instance of Mesh Attribute.
Handle(TDF_Attribute) ActData_MeshAttr::NewEmpty() const
{
  return new ActData_MeshAttr();
}

//! Performs data transferring from the given CAF Attribute to this one.
//! This method is mainly used by OCAF Undo/Redo kernel as a part of
//! Backup functionality.
//! \param MainAttr [in] CAF Attribute to copy data from.
void ActData_MeshAttr::Restore(const Handle(TDF_Attribute)& ActData_NotUsed(MainAttr))
{
  // Nothing is here
}

//! Supporting method for Copy/Paste functionality. Performs full copying of
//! the underlying mesh.
void ActData_MeshAttr::Paste(const Handle(TDF_Attribute)& Into,
                             const Handle(TDF_RelocationTable)&) const
{
  Handle(ActData_MeshAttr) IntoMesh = Handle(ActData_MeshAttr)::DownCast(Into);
  Handle(ActData_Mesh) IntoMeshDS = new ActData_Mesh();

  /* ==================
   *  Paste mesh nodes
   * ================== */

  ActData_Mesh_ElementsIterator aMeshNodesIt(m_mesh, ActData_Mesh_ET_Node);
  for ( ; aMeshNodesIt.More(); aMeshNodesIt.Next() )
  {
    // Access next node
    Handle(ActData_Mesh_Node)
      aNode = Handle(ActData_Mesh_Node)::DownCast( aMeshNodesIt.GetValue() );
    
    // Paste node
    IntoMeshDS->AddNodeWithID( aNode->X(), aNode->Y(), aNode->Z(), aNode->GetID() );
  }

  /* =====================
   *  Paste mesh elements
   * ===================== */

  ActData_Mesh_ElementsIterator aMeshElemsIt(m_mesh, ActData_Mesh_ET_Face);
  for ( ; aMeshElemsIt.More(); aMeshElemsIt.Next() )
  {
    const Handle(ActData_Mesh_Element)& anElem = aMeshElemsIt.GetValue();

    // Proceed with TRIANGLE elements
    if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Triangle) ) )
    {
      Handle(ActData_Mesh_Triangle) aTriElem = Handle(ActData_Mesh_Triangle)::DownCast(anElem);
      IntoMeshDS->AddElementWithID( aTriElem, aTriElem->GetID() );
    }
    // Proceed with QUADRANGLE elements
    else if ( anElem->IsInstance( STANDARD_TYPE(ActData_Mesh_Quadrangle) ) )
    {
      Handle(ActData_Mesh_Quadrangle) aQuadElem = Handle(ActData_Mesh_Quadrangle)::DownCast(anElem);
      IntoMeshDS->AddElementWithID( aQuadElem, aQuadElem->GetID() );
    }
  }

  IntoMesh->SetMesh(IntoMeshDS);
}

//-----------------------------------------------------------------------------
// Accessors for domain-specific data
//-----------------------------------------------------------------------------

//! Prepares new empty Mesh DS.
void ActData_MeshAttr::NewEmptyMesh()
{
  m_mesh = new ActData_Mesh();
}

//! Sets Mesh DS to store.
//! \param Mesh [in] mesh to store.
//! \param doDelta [in] indicates whether to apply Delta.
void ActData_MeshAttr::SetMesh(const Handle(ActData_Mesh)& Mesh)
{
  m_mesh.Nullify();
  m_mesh = Mesh;
}

//! Returns the stored Mesh DS.
//! \return stored mesh.
Handle(ActData_Mesh) ActData_MeshAttr::GetMesh()
{
  return m_mesh;
}

//-----------------------------------------------------------------------------
// Manipulations with mesh
//-----------------------------------------------------------------------------

//! Creates new mesh node with the passed co-ordinates.
//! \param X [in] nodal X co-ordinate.
//! \param Y [in] nodal Y co-ordinate.
//! \param Z [in] nodal Z co-ordinate.
//! \return node ID.
Standard_Integer ActData_MeshAttr::AddNode(const Standard_Real X,
                                           const Standard_Real Y,
                                           const Standard_Real Z)
{
  // Check pre-conditions
  this->assertModificationAllowed();

  // Add node to Mesh DS
  Standard_Integer aResID = m_mesh->AddNode(X, Y, Z);

  return aResID;
}

//! Adds the passed mesh node to the Mesh DS. This method is useful in cases
//! when you already have an ID for your node.
//! \param X [in] nodal X co-ordinate.
//! \param Y [in] nodal Y co-ordinate.
//! \param Z [in] nodal Z co-ordinate.
//! \param ID [in] node ID.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MeshAttr::AddNodeWithID(const Standard_Real X,
                                                 const Standard_Real Y,
                                                 const Standard_Real Z,
                                                 const Standard_Integer ID)
{
  // Check pre-conditions
  this->assertModificationAllowed();

  // Add node to Mesh DS
  Standard_Boolean aRes = m_mesh->AddNodeWithID(X, Y, Z, ID);

  return aRes;
}

//! Removes mesh node with the given ID.
//! \param ID [in] ID of the mesh node to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MeshAttr::RemoveNode(const Standard_Integer ID)
{
  // Attempt to remove the mesh node
  Standard_Boolean isOk = m_mesh->RemoveNode(ID);

  return isOk;
}

//! Creates new mesh element by the given mesh nodes.
//! \param Nodes [in] collection of nodal IDs comprising the mesh element.
//! \param NbNodes [in] number of Nodes.
//! \return element ID.
Standard_Integer ActData_MeshAttr::AddElement(Standard_Address Nodes,
                                              const Standard_Integer NbNodes)
{
  // Check pre-conditions
  this->assertModificationAllowed();

  // Add element to the underlying Mesh DS
  Standard_Integer aResID = m_mesh->AddFace(Nodes, NbNodes);

  return aResID;
}

//! Creates new mesh element by the given mesh nodes. This method is useful
//! when you already have an ID for your element.
//! \param Nodes [in] collection of nodal IDs comprising the mesh element.
//! \param NbNodes [in] number of Nodes.
//! \param ID [in] ID of the element.
//! \return true in case of success, false -- otherwise.
Standard_Boolean
  ActData_MeshAttr::AddElementWithID(Standard_Address Nodes,
                                     const Standard_Integer NbNodes,
                                     const Standard_Integer ID)
{
  // Check pre-conditions
  this->assertModificationAllowed();

  // Add element to the underlying Mesh DS
  Standard_Boolean aRes = m_mesh->AddFaceWithID(Nodes, NbNodes, ID);

  return aRes;
}

//! Removes mesh element with the given ID.
//! \param ID [in] ID of the mesh element to remove.
//! \return true in case of success, false -- otherwise.
Standard_Boolean ActData_MeshAttr::RemoveElement(const Standard_Integer ID)
{
  // Check if the requested element exists in Mesh DS
  Handle(ActData_Mesh_Element) anElem = m_mesh->FindElement(ID);
  if ( anElem.IsNull() )
    return Standard_False;

  // Remove element
  m_mesh->RemoveElement(anElem);

  return Standard_True;
}

//-----------------------------------------------------------------------------
// Internal kernel methods
//-----------------------------------------------------------------------------

//! Checks whether modification is allowed on the Mesh Attribute. If not,
//! throws an appropriate exception.
void ActData_MeshAttr::assertModificationAllowed()
{
  if ( !Label().Data()->IsModificationAllowed() )
    Standard_ImmutableObject::Raise("ActData_MeshAttr changed outside transaction");

  if ( m_mesh.IsNull() )
    Standard_ProgramError::Raise("Mesh DS is NULL");
}
