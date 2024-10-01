//-----------------------------------------------------------------------------
// Created on: 30 September 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Quaoar Studio LLC (www.quaoar-studio.com)
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

#ifndef asiAlgo_RecognitionRuleShaft_h
#define asiAlgo_RecognitionRuleShaft_h

// asiAlgo includes
#include <asiAlgo_RecognitionRule.h>

//-----------------------------------------------------------------------------

//! \ingroup ASI_AFR
//!
//! Feature recognition rule for cylindrical shafts.
class asiAlgo_RecognitionRuleShaft : public asiAlgo_RecognitionRule
{
  // OCCT RTTI
  DEFINE_STANDARD_RTTI_INLINE(asiAlgo_RecognitionRuleShaft, asiAlgo_RecognitionRule)

public:

  //! Constructs the rule initializing it with the given AAG iterator.
  //! \param[in] aag_it   the AAG iterator.
  //! \param[in] target_R the radius of interest (upper barrier).
  //! \param[in] progress the progress entry.
  //! \param[in] plotter  the plotter entry.
  asiAlgo_RecognitionRuleShaft(const Handle(asiAlgo_AAGIterator)& aag_it,
                               const double                       target_R,
                               ActAPI_ProgressEntry               progress,
                               ActAPI_PlotterEntry                plotter)
  //
  : asiAlgo_RecognitionRule (aag_it, progress, plotter),
    m_fRadius               (0.0),
    m_fTargetRadius         (target_R),
    m_fLinToler             (1.e-6),
    m_fAngToler             (1.0/180.0*M_PI), // 1 degree is a default precision for coaxiality check.
    m_fCanRecPrec           (1.e-3)
  {}

private:

  //! Recognizes feature starting from the current position of AAG iterator.
  //! \param[out] featureFaces   the detected feature faces.
  //! \param[out] featureIndices the indices of the detected feature faces.
  //! \return true/false.
  virtual bool
    recognize(TopTools_IndexedMapOfShape& featureFaces,
              TColStd_PackedMapOfInteger& featureIndices) override;

public:

  //! \return radius of the detected shaft.
  double GetRadius() const { return m_fRadius; }

  //! \return target radius.
  double GetTargetRadius() const { return m_fTargetRadius; }

  //! Sets linear tolerance to use.
  //! \param[in] tol the tolerance to use.
  void SetLinearTolerance(const double tol) { m_fLinToler = tol; }

  //! Sets the precision of canonical recognition to use.
  //! \param[in] prec the precision to set.
  void SetCanRecPrecision(const double prec) { m_fCanRecPrec = prec; }

protected:

  //! Checks whether the passed face is a cylinder.
  //! \param[in] fid ID of the face to check.
  //! \return true/false.
  bool isCylindrical(const int fid) const;

  //! Checks whether the passed face is a cylinder.
  //! \param[in]  fid          face to check.
  //! \param[in]  checkNoHints indicates whether to check for feature hints
  //!                          to avoid having them.
  //! \param[in]  checkOuter   checks if this face is not a bore, i.e., a small
  //!                          shift along its normal increases the radius.
  //! \param[out] radius       radius of the host cylinder.
  //! \param[out] angle_min    min angle of the cylindrical surface.
  //! \param[out] angle_max    max angle of the cylindrical surface.
  //! \param[out] ax           axis of the cylinder.
  //! \return true/false.
  bool isCylindrical(const int  fid,
                     const bool checkNoHints,
                     const bool checkOuter,
                     double&    radius,
                     double&    angle_min,
                     double&    angle_max,
                     gp_Ax1&    ax) const;

  //! Recursive function to iterate cylindrical faces that are
  //! neighbors to the given seed.
  //! \param[in]     sid       the 1-based ID of the starting face.
  //! \param[in]     fid       the 1-based ID of the seed face.
  //! \param[in]     refRadius the reference radius.
  //! \param[in]     refAxis   the reference axis.
  //! \param[in,out] sumAng    the collected total angle.
  //! \param[out]    collected the collected cylindrical faces.
  void visitNeighborCylinders(const int        sid,
                              const int        fid,
                              const double     refRadius,
                              const gp_Ax1&    refAxis,
                              double&          sumAng,
                              asiAlgo_Feature& collected);

protected:

  double m_fRadius;       //!< Radius of the detected hole.
  double m_fTargetRadius; //!< Target radius.
  double m_fLinToler;     //!< Linear tolerance.
  double m_fAngToler;     //!< Angular tolerance.
  double m_fCanRecPrec;   //!< Precision of canonical recognition.

};

#endif
