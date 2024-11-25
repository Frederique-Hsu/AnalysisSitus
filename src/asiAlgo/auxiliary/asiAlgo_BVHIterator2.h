//-----------------------------------------------------------------------------
// Created on: 11 September 2024
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

#pragma once

// asiAlgo includes
#include <asiAlgo.h>

// Includes from OCCT.
#include <BVH_BinaryTree.hxx>

//-----------------------------------------------------------------------------

//! \ingroup ASI_CORE
//!
//! BVH iterator for parallel traversal of two BVH trees.
class asiAlgo_BVHIterator2
{
  const static int STACK_DEPTH = 132;

  struct BVH_Item
  {
    BVH_Item()
    : m_id1(-1),
      m_id2(-1)
    {}

    BVH_Item(const int id1,
             const int id2)
    : m_id1(id1),
      m_id2(id2)
    {}

    int m_id1;
    int m_id2;
  };

public:

  //! Constructor accepting 2 BVH trees for initialization of the iterator.
  //! \param [in] bvh1 the first tree to traverse.
  //! \param [in] bvh2 the second tree to traverse.
  asiAlgo_EXPORT
    asiAlgo_BVHIterator2(const opencascade::handle<BVH_Tree<double, 3> >& bvh1,
                         const opencascade::handle<BVH_Tree<double, 3> >& bvh2);

public:

  //! \return true if something remains to iterate, false -- the iteration
  //!         process is over.
  asiAlgo_EXPORT bool
    More() const;

  //! Moves iterator to the next position. This method checks if one of four
  //! possible conditions is realized for the currently iterated BVH nodes:
  //!
  //! - leaf-leaf
  //! - leaf-box
  //! - box-leaf
  //! - box-box
  asiAlgo_EXPORT void
    Next();

public:

  //! Method to return current item from the first BVH tree.
  const BVH_Vec4i& Current1() const
  { return m_current1; }

  //! Method to return current item from the second BVH tree.
  const BVH_Vec4i& Current2() const
  { return m_current2; }

  //! Checks whether is current node of the first tree a leaf or not.
  bool IsLeaf1() const
  { return m_isLeaf1; }

  //! Checks whether is current node of the second tree a leaf or not.
  bool IsLeaf2() const
  { return m_isLeaf2; }

  //! Method to return current index for the first tree.
  int CurrentIndex1() const
  { return m_currentNode.m_id1; }

  //! Method to return current index for the second tree.
  int CurrentIndex2() const
  { return m_currentNode.m_id2; }

  //! Blocks left subtree for the first tree.
  void BlockLeftLeft()
  { m_blocked[0] = true; }

  //! Blocks right subtree for the first tree.
  void BlockLeftRight()
  { m_blocked[1] = true; }

  //! Blocks left subtree for the second tree.
  void BlockRightLeft()
  { m_blocked[2] = true; }

  //! Blocks right subtree for the second tree.
  void BlockRightRight()
  { m_blocked[3] = true; }

  //! Blocks left subtree for the first tree.
  void BlockLeafLeft()
  { m_blocked[0] = true; }

  //! Blocks right subtree for the first tree.
  void BlockLeafRight()
  { m_blocked[1] = true; }

  //! Blocks left subtree for the second tree.
  void BlockLeftLeaf()
  { m_blocked[2] = true; }

  //! Blocks right subtree for the second tree.
  void BlockRightLeaf()
  { m_blocked[3] = true; }

protected:

  opencascade::handle<BVH_Tree<double, 3>> m_bvh1;               //!< The 1-st BVH to iterate over.
  opencascade::handle<BVH_Tree<double, 3>> m_bvh2;               //!< The 2-nd BVH to iterate over.

  // Iteration variables.
  BVH_Item                                 m_stack[STACK_DEPTH]; //!< Non-traversed nodes to return to.
  int                                      m_stackHead;          //!< Pointer to the stack head.
  BVH_Item                                 m_currentNode;        //!< Current node.
  bool                                     m_blocked[4];         //!< Nodes to stop traversal (their children will be skipped).

  /*
   *  m_blocked[0] : BLOCKED left in the 1-st BVH
   *  m_blocked[1] : BLOCKED right in the 1-st BVH
   *  m_blocked[2] : BLOCKED left in the 2-nd BVH
   *  m_blocked[3] : BLOCKED right in the 2-nd BVH
   */

  // Internal variables to speedup traversal.
  BVH_Vec4i                                m_current1;
  BVH_Vec4i                                m_current2;
  bool                                     m_isLeaf1;
  bool                                     m_isLeaf2;

};
