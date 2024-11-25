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

// Own include
#include <asiAlgo_BVHIterator2.h>

//-----------------------------------------------------------------------------

asiAlgo_BVHIterator2::asiAlgo_BVHIterator2(const opencascade::handle<BVH_Tree<double, 3> >& bvh1,
                                           const opencascade::handle<BVH_Tree<double, 3> >& bvh2)
: m_bvh1          ( bvh1 ),
  m_bvh2          ( bvh2 ),
  m_stackHead     ( -1 ),
  m_currentNode   ( BVH_Item(0, 0) ) // First index.
{
  for ( int i = 0; i < STACK_DEPTH; ++i )
    m_stack[i] = BVH_Item(0, 0);

  m_blocked[0] = false;
  m_blocked[1] = false;
  m_blocked[2] = false;
  m_blocked[3] = false;

  m_current1 = m_bvh1->NodeInfoBuffer()[m_currentNode.m_id1];
  m_current2 = m_bvh2->NodeInfoBuffer()[m_currentNode.m_id2];
  m_isLeaf1  = m_current1.x() != 0;
  m_isLeaf2  = m_current2.x() != 0;
}

//-----------------------------------------------------------------------------

bool asiAlgo_BVHIterator2::More() const
{
  if ( m_currentNode.m_id1 == -1 && m_currentNode.m_id2 == -1 )
    return false;

  return true;
}

//-----------------------------------------------------------------------------

void asiAlgo_BVHIterator2::Next()
{
  // The purpose of this method is to set the value of m_currentNode properly.
  BVH_Item items2Process[4] = { BVH_Item(-1, -1), BVH_Item(-1, -1), 
                                BVH_Item(-1, -1), BVH_Item(-1, -1) };
  int      itemsCount = 0;

  /* leaf-leaf */
  if ( m_isLeaf1 && m_isLeaf2 )
  {
    m_currentNode = (m_stackHead < 0) ? BVH_Item(-1, -1) : m_stack[m_stackHead--]; // Return to parent.
  }

  /* leaf-box */
  else if ( m_isLeaf1 && !m_isLeaf2 )
  {
    // The first tree is a leaf.
    // The second tree is not a leaf.
    const int leftChild  = m_current2.y();
    const int rightChild = m_current2.z();

    // Check if children are not blocked externally.
    if ( !m_blocked[2] )
      items2Process[itemsCount++] = BVH_Item(m_currentNode.m_id1, leftChild);
    //
    if ( !m_blocked[3] )
      items2Process[itemsCount++] = BVH_Item(m_currentNode.m_id1, rightChild);

    // Set current node to one of the children. It can be -1 if
    // there is nothing to iterate over.
    m_currentNode = items2Process[0]; // Left item is iterated first (because it is left ;).

    // Fill stack to be able to return to non-traversed items.
    if ( itemsCount == 2 )
      m_stack[++m_stackHead] = items2Process[1];

    // If there is no any allowed child, move back in the stack.
    if ( !itemsCount && m_stackHead >= 0 )
      m_currentNode = m_stack[m_stackHead--];
  }

  /* box-leaf */
  else if ( !m_isLeaf1 && m_isLeaf2 )
  {
    // The first tree is not a leaf.
    // The second tree is a leaf.
    const int leftChild  = m_current1.y();
    const int rightChild = m_current1.z();

    // Check if children are not blocked externally.
    if ( !m_blocked[0] )
      items2Process[itemsCount++] = BVH_Item(leftChild, m_currentNode.m_id2);
    //
    if ( !m_blocked[1] )
      items2Process[itemsCount++] = BVH_Item(rightChild, m_currentNode.m_id2);

    // Set current node to one of the children. It can be -1 if
    // there is nothing to iterate over.
    m_currentNode = items2Process[0]; // Left item is iterated first (because it is left ;).

    // Fill stack to be able to return to non-traversed items.
    if ( itemsCount == 2 )
      m_stack[++m_stackHead] = items2Process[1];

    // If there is no any allowed child, move back in the stack.
    if ( !itemsCount && m_stackHead >= 0 )
      m_currentNode = m_stack[m_stackHead--];
  }
  
  /* box-box */
  else if ( !m_isLeaf1 && !m_isLeaf2 )
  {
    // The first tree is not a leaf.
    // The second tree is not a leaf.
    const int leftChild1  = m_current1.y();
    const int rightChild1 = m_current1.z();
    const int leftChild2  = m_current2.y();
    const int rightChild2 = m_current2.z();

    // Check if children are not blocked externally.
    if ( !m_blocked[0] )
      items2Process[itemsCount++] = BVH_Item(leftChild1, leftChild2);

    if ( !m_blocked[1] )
      items2Process[itemsCount++] = BVH_Item(leftChild1, rightChild2);

    if ( !m_blocked[2] )
      items2Process[itemsCount++] = BVH_Item(rightChild1, leftChild2);

    if ( !m_blocked[3] )
      items2Process[itemsCount++] = BVH_Item(rightChild1, rightChild2);

    // Set current node to one of the children. It can be -1 if
    // there is nothing to iterate over.
    m_currentNode = items2Process[0]; // Left item is iterated first (because it is left ;).

    for ( int i = 1; i < itemsCount; ++i )
      m_stack[++m_stackHead] = items2Process[i];

    // If there is no any allowed child, move back in the stack.
    if ( !itemsCount && m_stackHead >= 0 )
      m_currentNode = m_stack[m_stackHead--];
  }

  m_blocked[0] = false;
  m_blocked[1] = false;
  m_blocked[2] = false;
  m_blocked[3] = false;

  if ( m_currentNode.m_id1 != -1 && m_currentNode.m_id2 != -1 )
  {
    m_current1 = BVH::Array<int, 4>::Value(m_bvh1->NodeInfoBuffer(), m_currentNode.m_id1);
    m_current2 = BVH::Array<int, 4>::Value(m_bvh2->NodeInfoBuffer(), m_currentNode.m_id2);
    m_isLeaf1  = m_current1.x() != 0;
    m_isLeaf2  = m_current2.x() != 0;
  }
}
