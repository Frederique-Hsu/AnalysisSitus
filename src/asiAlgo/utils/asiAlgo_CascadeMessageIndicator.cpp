//-----------------------------------------------------------------------------
// Created on: 10 March 2025
//-----------------------------------------------------------------------------
// Copyright (c) 2025, Julia Slyadneva
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

// Own include.
#include <asiAlgo_CascadeMessageIndicator.h>

//-----------------------------------------------------------------------------

asiAlgo_CascadeMessageIndicator::asiAlgo_CascadeMessageIndicator()
  : m_lastValue( 0.0 ),
    m_notifier( NULL )
{
  // Empty
}

//-----------------------------------------------------------------------------

Standard_Boolean asiAlgo_CascadeMessageIndicator::Show( const Standard_Boolean force )
{
  Standard_Real pos = GetPosition();

  if ( (!force && m_lastValue == pos) || UserBreak() )
  {
    // Return false if update interval has not elapsed
    return Standard_False;
  }

  m_lastValue = pos;

  // Set progress value for the notifier.
  if ( !m_notifier.IsNull() )
    m_notifier->SetProgress( (int)std::round(m_lastValue * m_notifier->Capacity()) );

  return Standard_True;
}

//-----------------------------------------------------------------------------

Standard_Boolean asiAlgo_CascadeMessageIndicator::UserBreak()
{
  if ( !m_notifier.IsNull() )
    return m_notifier->IsCancelling();

  return false;
}

//-----------------------------------------------------------------------------

void asiAlgo_CascadeMessageIndicator::ConnectNotifier( const Handle(ActAPI_IProgressNotifier)& notifier )
{
  m_notifier = notifier;
}
