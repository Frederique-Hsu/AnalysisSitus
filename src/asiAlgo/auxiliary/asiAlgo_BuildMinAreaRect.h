//-----------------------------------------------------------------------------
// Created on: September 2023
//-----------------------------------------------------------------------------
// Copyright (c) 2023-present, Sergey Slyadnev
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

#ifndef asiAlgo_BuildMinAreaRect_h
#define asiAlgo_BuildMinAreaRect_h

// asiAlgo includes
#include "asiAlgo.h"

// Active Data includes
#include <ActAPI_IPlotter.h>
#include <ActAPI_IProgressNotifier.h>

// OCCT includes
#include <gp_Pnt.hxx>
#include <gp_XY.hxx>
#include <Precision.hxx>

// STD includes
#include <vector>

//-----------------------------------------------------------------------------

//! \ingroup ASI_MODELING
//!
//! Computes 2D OBB for a convex hull of point cloud. The idea is borrowed from
//! "rotating calipers" method, see
//!   https://www-cgrl.cs.mcgill.ca/~godfried/research/calipers.html
//! and
//!   https://www.geometrictools.com/Documentation/MinimumAreaRectangle.pdf
//! for references.
//!
//! The implementation is based on the code from here:
//!   https://github.com/davideberly/GeometricTools/blob/master/GTE/Mathematics/MinimumAreaBox2.h
class asiAlgo_BuildMinAreaRect
{
public:

  //! Segment.
  struct Segment
  {
    //! Start and end of a segment.
    gp_XY a, b;

    //! Constructor.
    //! \param[in] a start of segment.
    //! \param[in] b end of segment.
    Segment(const gp_XY& a,
            const gp_XY& b)
    {
      this->a = a;
      this->b = b;
    }
  };

  //! Rectangle.
  struct Rect
  {
    //! Bottom left point.
    gp_XY location;

    //! Dimensions X and Y.
    gp_XY size;

    //! Constructor.
    Rect()
    {}

    //! Constructor.
    //! \param[in] a location.
    //! \param[in] b dimensions.
    Rect(const gp_XY& a, const gp_XY& c)
    {
      location = a;
      size = c - a;
    }

    //! Checks that Rectangle is empty.
    //! \return true/false.
    bool IsEmpty()
    {
      return location.X() == 0 &&
             location.Y() == 0 &&
             size.X() == 0 &&
             size.Y() == 0;
    }

    //! Calculates area.
    //! \return area.
    double GetArea()
    {
      return size.X() * size.Y();
    }

    //! Gets corner points of a rectangle
    //! \return corner points of a rectangle.
    std::vector<gp_XY> getPoints() const
    {
      return {
        { location.X(),            location.Y() },
        { location.X() + size.X(), location.Y() },
        { location.X() + size.X(), location.Y() + size.Y() },
        { location.X(),            location.Y() + size.Y() }
      };
    }
  };

  //! Bounding box.
  struct BoundingBox
  {
    //! The aligned corner points of the rectangle relative to orientation of original points.
    std::vector<gp_XY> rotatedBoxPoints;

    //! The aligned corner points of the rectangle relative to the X and Y axes.
    std::vector<gp_XY> alignedBoxPoints;

    //! Center of bounding box.
    gp_XY center;

    //! Smaller box side.
    double width;

    //! Larger box side.
    double height;

    //! Angle between smaller box side and X axis in radians.
    //! Positive value means box orientation from bottom right to top left,
    //! negative value means opposite.
    double widthAngle;

    //! Angle between larger box side and X axis in radians.
    //! Positive value means box orientation from bottom left to top right,
    //! negative value means opposite.
    double heightAngle;

    //! Weather the box is aligned to axes (widthAngle is 0 and height angle is 90).
    bool isAligned;
  };

public:

  //! Calculates the minimum bounding box.
  //! \param[in] points             points
  //! \param[in] bbox               bounding box with minimal area.
  //! \param[in] alignmentTolerance tolerance (in degrees) used for `isAligned`
  //!                               property of BoundingBox.
  //! \return true in case of success, false -- otherwise.
  asiAlgo_EXPORT static bool
    Calculate(const std::vector<gp_Pnt>& points,
              BoundingBox&               bbox,
              const double               alignmentTolerance = Precision::Angular(),
              ActAPI_ProgressEntry       progress           = nullptr,
              ActAPI_PlotterEntry        plotter            = nullptr);

private:

  //! Calculates cross product.
  //! \param[in] o start of vectors.
  //! \param[in] a end of first vector.
  //! \param[in] b end of second vector.
  //! \return cross product.
  static double cross(const gp_XY& o,
                      const gp_XY& a,
                      const gp_XY& b);

  //! Checks that two numbers are equal.
  //! \param[in] v1 first number.
  //! \param[in] v2 second number.
  //! \return result of the comparison. True, if the numbers are equal.
  static bool isDoubleEqual(const double v1,
                            const double v2);

  //! Constructs 2d convex hull.
  //! \param[in]  points     points lying in a plan parallel to XOY.
  //! \param[out] hullPoints convex hull.
  //! \return true in case of success, false -- otherwise.
  static bool convexHull(const std::vector<gp_Pnt>& points,
                         std::vector<gp_XY>&        hullPoints);

  //! Calculates angle between X axis and segment.
  //! \param[in] s segment.
  //! \return angle.
  static double angleToXAxis(const Segment& s);

  //! Rotates a point by a specified angle.
  //! \param[in] p     point.
  //! \param[in] angle angle.
  //! \return rotated point.
  static gp_XY rotateToXAxis(const gp_XY& p,
                             const double angle);

};

#endif
