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

#pragma once

// asiAlgo includes
#include <asiAlgo.h>

// Active Data includes
#include <ActAPI_IProgressNotifier.h>

// OpenCascade includes
#include <Message_ProgressIndicator.hxx>

//! This class represents OCCT-compliant wrapper of custom progress notifier
//! for the sake of bridging notification mechanisms.
class asiAlgo_CascadeMessageIndicator : public Message_ProgressIndicator
{
public:

  DEFINE_STANDARD_RTTI_INLINE( asiAlgo_CascadeMessageIndicator, Message_ProgressIndicator )

public:

  //! Constructor.
  asiAlgo_EXPORT
    asiAlgo_CascadeMessageIndicator();

  //! Update progress state
  //! \param[in] theForce the flag for forcing update
  //! \return false if update interval has not elapsed and true otherwise.
  virtual Standard_Boolean Show( const Standard_Boolean force = Standard_True );

  //! Check for canceled state.
  //! \return True if the user has send to a break signal
  virtual Standard_Boolean UserBreak();

  //! Establish connection with progress notifier.
  //! \param[in] notifier the progress notifier.
  asiAlgo_EXPORT
    void ConnectNotifier( const Handle(ActAPI_IProgressNotifier)& notifier );

protected:
  //! Should update presentation of the progress indicator
  //! \param[in] theScope is the current scope being advanced
  //! \param[in] isForce is intended for forcing update in case if it is required 
  virtual void Show(const Message_ProgressScope& scope,
                    const Standard_Boolean       isForce) {
      (void)scope; Show(isForce);
    }

private:
  Standard_Real                    m_lastValue;
  Handle(ActAPI_IProgressNotifier) m_notifier;
};
