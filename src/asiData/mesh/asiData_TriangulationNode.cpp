//-----------------------------------------------------------------------------
// Created on: 10 July 2017
//-----------------------------------------------------------------------------
// Copyright (c) 2017-present, Sergey Slyadnev
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
#include <asiData_TriangulationNode.h>

// asiData includes
#include <asiData_BVHParameter.h>

// Active Data includes
#include <ActData_ParameterFactory.h>

// asiAlgo includes
#include <asiAlgo_Utils.h>

#if defined USE_MOBIUS
  using namespace mobius;
#endif

//-----------------------------------------------------------------------------

//! Default constructor. Registers all involved Parameters.
asiData_TriangulationNode::asiData_TriangulationNode() : ActData_BaseNode()
{
  REGISTER_PARAMETER(Name,      PID_Name);
  REGISTER_PARAMETER(RealArray, PID_Options);
  //
  REGISTER_PARAMETER(Group,     PID_GroupTrsf);
  REGISTER_PARAMETER(Real,      PID_TrsfTx);
  REGISTER_PARAMETER(Real,      PID_TrsfTy);
  REGISTER_PARAMETER(Real,      PID_TrsfTz);
  REGISTER_PARAMETER(Real,      PID_TrsfRx);
  REGISTER_PARAMETER(Real,      PID_TrsfRy);
  REGISTER_PARAMETER(Real,      PID_TrsfRz);
  REGISTER_PARAMETER(RealArray, PID_TrsfMx);
  //
  REGISTER_PARAMETER(Group,     PID_GroupPrs);
  REGISTER_PARAMETER(Int,       PID_DisplayMode);
  REGISTER_PARAMETER(Bool,      PID_UseScalars);
  REGISTER_PARAMETER(Int,       PID_Color);
  REGISTER_PARAMETER(Int,       PID_EdgesColor);
  REGISTER_PARAMETER(Bool,      PID_HasVertices);

  // Non-standard Parameters.
  this->registerParameter(PID_BVH,           asiData_BVHParameter::Instance(), false);
  this->registerParameter(PID_Triangulation, asiData_MeshParameter::Instance(), false);
}

//! Returns new DETACHED instance of Mesh Node ensuring its correct
//! allocation in a heap.
//! \return new instance of Triangulation Node.
Handle(ActAPI_INode) asiData_TriangulationNode::Instance()
{
  return new asiData_TriangulationNode();
}

//! Performs initial actions required to make the Node WELL-FORMED.
void asiData_TriangulationNode::Init()
{
  // Initialize name Parameter.
  this->InitParameter(PID_Name, "Name");

  // Set default values.
#if defined USE_MOBIUS
  this->SetTriangulation (nullptr);
#endif
  this->SetOptions       (nullptr);
  this->SetBVH           (nullptr);
  this->SetUseScalars    (true);
  this->SetColor         (120 << 16 | 120 << 8 | 120); // Initial color.
  this->SetEdgesColor    (0   << 16 | 0   << 8 | 0);   // Initial edge color.
  this->SetDisplayMode   (1);
  this->SetHasVertices   (false);

  // Set identity transformation.
  ActParamTool::AsRealArray( this->Parameter(PID_TrsfMx) )->SetArray( new HRealArray(0, 11, 0.) );
  //
  this->SetTransformation(0., 0., 0., 0., 0., 0.);

  // Initialize Parameter flags.
  this->InitParameter(PID_GroupTrsf,   "Transformation", "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_TrsfTx,      "Tx",             "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_TrsfTy,      "Ty",             "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_TrsfTz,      "Tz",             "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_TrsfRx,      "Rx",             "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_TrsfRy,      "Ry",             "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_TrsfRz,      "Rz",             "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_GroupPrs,    "Presentation",   "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_DisplayMode, "Display mode",   "PrsMeshDisplayMode", 0,                       true);
  this->InitParameter(PID_UseScalars,  "Use scalars",    "",                   ParameterFlag_IsVisible, true);
  this->InitParameter(PID_Color,       "Color",          "PrsCustomColor",     ParameterFlag_IsVisible, true);
  this->InitParameter(PID_EdgesColor,  "Color of edges", "PrsCustomColor",     ParameterFlag_IsVisible, true);
  this->InitParameter(PID_HasVertices, "Show vertices",  "",                   ParameterFlag_IsVisible, true);
}

//-----------------------------------------------------------------------------
// Generic naming
//-----------------------------------------------------------------------------

//! Accessor for the Node's name.
//! \return name of the Node.
TCollection_ExtendedString asiData_TriangulationNode::GetName()
{
  return ActParamTool::AsName( this->Parameter(PID_Name) )->GetValue();
}

//! Sets name for the Node.
//! \param[in] name the name to set.
void asiData_TriangulationNode::SetName(const TCollection_ExtendedString& name)
{
  ActParamTool::AsName( this->Parameter(PID_Name) )->SetValue(name);
}

//-----------------------------------------------------------------------------
// Handy accessors
//-----------------------------------------------------------------------------

//! \return triangulation parameter.
Handle(asiData_MeshParameter)
  asiData_TriangulationNode::GetTriangulationParam() const
{
  Handle(asiData_MeshParameter)
    param = Handle(asiData_MeshParameter)::DownCast( this->Parameter(PID_Triangulation) );

  return param;
}

#if defined USE_MOBIUS

//! \return stored tessellation.
t_ptr<t_mesh>
  asiData_TriangulationNode::GetTriangulation(const bool applyTransform) const
{
  Handle(asiData_MeshParameter) param = this->GetTriangulationParam();
  //
  if ( param.IsNull() )
    return nullptr;

  t_ptr<t_mesh> mesh = static_cast<t_mesh*>( param->GetMesh() );

  if ( !applyTransform || mesh.IsNull())
    return mesh;

  // Apply transformation.
  double tx, ty, tz, rxDeg, ryDeg, rzDeg;
  this->GetTransformation(tx, ty, tz, rxDeg, ryDeg, rzDeg);
  //
  return mesh->Transformed(tx, ty, tz, rxDeg, ryDeg, rzDeg);
}

//! Sets tessellation to store.
//! \param[in] triangulation the tessellation to store.
void asiData_TriangulationNode::SetTriangulation(const t_ptr<t_mesh>& triangulation)
{
  Handle(asiData_MeshParameter) param = this->GetTriangulationParam();
  //
  if ( param.IsNull() )
    return;

  param->SetMesh(triangulation);
}

#endif

//! \return stored BVH.
Handle(asiAlgo_BVHFacets) asiData_TriangulationNode::GetBVH() const
{
  return Handle(asiData_BVHParameter)::DownCast( this->Parameter(PID_BVH) )->GetBVH();
}

//! Sets BVH to store.
//! \param bvh [in] BVH to store.
void asiData_TriangulationNode::SetBVH(const Handle(asiAlgo_BVHFacets)& bvh)
{
  Handle(asiData_BVHParameter)::DownCast( this->Parameter(PID_BVH) )->SetBVH(bvh);
}

//! Returns triangulation options.
//! \return domain-specific triangulation options.
Handle(TColStd_HArray1OfReal) asiData_TriangulationNode::GetOptions() const
{
  return ActParamTool::AsRealArray( this->Parameter(PID_Options) )->GetArray();
}

//! Sets domain-specific triangulation options.
//! \param options [in] triangulation options to set.
void asiData_TriangulationNode::SetOptions(const Handle(TColStd_HArray1OfReal)& options)
{
  ActParamTool::AsRealArray( this->Parameter(PID_Options) )->SetArray(options);
}

//! Sets the transformation components.
//! \param[in] tx    translation along the OX axis (in model units).
//! \param[in] ty    translation along the OY axis (in model units).
//! \param[in] tz    translation along the OZ axis (in model units).
//! \param[in] rxDeg rotation around OX axis (in degrees).
//! \param[in] ryDeg rotation around OY axis (in degrees).
//! \param[in] rzDeg rotation around OZ axis (in degrees).
void asiData_TriangulationNode::SetTransformation(const double tx,
                                                  const double ty,
                                                  const double tz,
                                                  const double rxDeg,
                                                  const double ryDeg,
                                                  const double rzDeg)
{
  ActParamTool::AsReal( this->Parameter(PID_TrsfTx) )->SetValue(tx);
  ActParamTool::AsReal( this->Parameter(PID_TrsfTy) )->SetValue(ty);
  ActParamTool::AsReal( this->Parameter(PID_TrsfTz) )->SetValue(tz);
  ActParamTool::AsReal( this->Parameter(PID_TrsfRx) )->SetValue(rxDeg);
  ActParamTool::AsReal( this->Parameter(PID_TrsfRy) )->SetValue(ryDeg);
  ActParamTool::AsReal( this->Parameter(PID_TrsfRz) )->SetValue(rzDeg);

  this->UpdateTransformationMx();
}

//! Gets the transformation components.
//! \param[out] tx    translation along the OX axis (in model units).
//! \param[out] ty    translation along the OY axis (in model units).
//! \param[out] tz    translation along the OZ axis (in model units).
//! \param[out] rxDeg rotation around OX axis (in degrees).
//! \param[out] ryDeg rotation around OY axis (in degrees).
//! \param[out] rzDeg rotation around OZ axis (in degrees).
void asiData_TriangulationNode::GetTransformation(double& tx,
                                                  double& ty,
                                                  double& tz,
                                                  double& rxDeg,
                                                  double& ryDeg,
                                                  double& rzDeg) const
{
  tx    = ActParamTool::AsReal( this->Parameter(PID_TrsfTx) )->GetValue();
  ty    = ActParamTool::AsReal( this->Parameter(PID_TrsfTy) )->GetValue();
  tz    = ActParamTool::AsReal( this->Parameter(PID_TrsfTz) )->GetValue();
  rxDeg = ActParamTool::AsReal( this->Parameter(PID_TrsfRx) )->GetValue();
  ryDeg = ActParamTool::AsReal( this->Parameter(PID_TrsfRy) )->GetValue();
  rzDeg = ActParamTool::AsReal( this->Parameter(PID_TrsfRz) )->GetValue();
}

//! Sets transformation matrix by its components.
//!
//! \verbatim
//!   V1   V2   V3     T       XYZ       XYZ
//! | a11  a12  a13   a14 |   | x |     | x'|
//! | a21  a22  a23   a24 |   | y |     | y'|
//! | a31  a32  a33   a34 |   | z |  =  | z'|
//! |  0    0    0     1  |   | 1 |     | 1 |
//! \endverbatim
void asiData_TriangulationNode::SetTransformationMx(const double a11,
                                                    const double a12,
                                                    const double a13,
                                                    const double a14,
                                                    const double a21,
                                                    const double a22,
                                                    const double a23,
                                                    const double a24,
                                                    const double a31,
                                                    const double a32,
                                                    const double a33,
                                                    const double a34)
{
  Handle(ActData_RealArrayParameter)
    param = ActParamTool::AsRealArray( this->Parameter(PID_TrsfMx) );

  param->SetElement(0,  a11);
  param->SetElement(1,  a12);
  param->SetElement(2,  a13);
  param->SetElement(3,  a14);
  param->SetElement(4,  a21);
  param->SetElement(5,  a22);
  param->SetElement(6,  a23);
  param->SetElement(7,  a24);
  param->SetElement(8,  a31);
  param->SetElement(9,  a32);
  param->SetElement(10, a33);
  param->SetElement(11, a34);

  param->SetModified();
}

//! Returns the stored transformation matrix by its components.
void asiData_TriangulationNode::GetTransformationMx(double& a11,
                                                    double& a12,
                                                    double& a13,
                                                    double& a14,
                                                    double& a21,
                                                    double& a22,
                                                    double& a23,
                                                    double& a24,
                                                    double& a31,
                                                    double& a32,
                                                    double& a33,
                                                    double& a34) const
{
  Handle(ActData_RealArrayParameter)
    param = ActParamTool::AsRealArray( this->Parameter(PID_TrsfMx) );

  Handle(HRealArray) arr = param->GetArray();
  //
  a11 = arr->Value(0);
  a12 = arr->Value(1);
  a13 = arr->Value(2);
  a14 = arr->Value(3);
  a21 = arr->Value(4);
  a22 = arr->Value(5);
  a23 = arr->Value(6);
  a24 = arr->Value(7);
  a31 = arr->Value(8);
  a32 = arr->Value(9);
  a33 = arr->Value(10);
  a34 = arr->Value(11);
}

//! \return transformation matrix.
gp_Trsf asiData_TriangulationNode::GetTransformationMx() const
{
  // Get components of the transformation matrix.
  double a11, a12, a13, a14, a21, a22, a23, a24, a31, a32, a33, a34;
  this->GetTransformationMx(a11, a12, a13, a14,
                            a21, a22, a23, a24,
                            a31, a32, a33, a34);

  gp_Trsf T;
  T.SetValues(a11, a12, a13, a14,
              a21, a22, a23, a24,
              a31, a32, a33, a34);
  //
  return T;
}

//! Updates transformation matrix from `Tx`, `Ty`, `Tz`, `Rx`, `Ry`, `Rz`
//! parameters.
void asiData_TriangulationNode::UpdateTransformationMx()
{
  double tx, ty, tz, rxDeg, ryDeg, rzDeg;
  this->GetTransformation(tx, ty, tz, rxDeg, ryDeg, rzDeg);

  const double rx = rxDeg*M_PI/180.;
  const double ry = ryDeg*M_PI/180.;
  const double rz = rzDeg*M_PI/180.;

  // Update transformation matrix.
  gp_Trsf T = asiAlgo_Utils::Transformation(tx, ty, tz, rx, ry, rz);
  //
  const double a11 = T.Value(1, 1);
  const double a12 = T.Value(1, 2);
  const double a13 = T.Value(1, 3);
  const double a14 = T.Value(1, 4);
  const double a21 = T.Value(2, 1);
  const double a22 = T.Value(2, 2);
  const double a23 = T.Value(2, 3);
  const double a24 = T.Value(2, 4);
  const double a31 = T.Value(3, 1);
  const double a32 = T.Value(3, 2);
  const double a33 = T.Value(3, 3);
  const double a34 = T.Value(3, 4);
  //
  this->SetTransformationMx(a11, a12, a13, a14,
                            a21, a22, a23, a24,
                            a31, a32, a33, a34);
}

//! Sets the Boolean value indicating whether the scalars are to be used for
//! coloring the Part's Presentation.
//! \param on [in] value to set.
void asiData_TriangulationNode::SetUseScalars(const bool on)
{
  ActParamTool::AsBool( this->Parameter(PID_UseScalars) )->SetValue(on);
}

//! Accessor for the value of the Boolean Parameter indicating whether the
//! scalars are active.
//! \return true/false.
bool asiData_TriangulationNode::GetUseScalars() const
{
  return ActParamTool::AsBool( this->Parameter(PID_UseScalars) )->GetValue();
}

//! Sets color.
//! \param theColor [in] color to set.
void asiData_TriangulationNode::SetColor(const int theColor) const
{
  ActParamTool::AsInt( this->Parameter(PID_Color) )->SetValue(theColor);
}

//! Accessor for the stored color value.
//! \return color value.
int asiData_TriangulationNode::GetColor() const
{
  return ActParamTool::AsInt( this->Parameter(PID_Color) )->GetValue();
}

//! Sets edges color.
//! \param color [in] color to set.
void asiData_TriangulationNode::SetEdgesColor(const int color) const
{
  ActParamTool::AsInt(this->Parameter(PID_EdgesColor))->SetValue(color);
}

//! Accessor for the stored edges color value.
//! \return color value.
int asiData_TriangulationNode::GetEdgesColor() const
{
  return ActParamTool::AsInt(this->Parameter(PID_EdgesColor))->GetValue();
}

//! Sets display mode.
//! \param theMode [in] display mode value to set.
void asiData_TriangulationNode::SetDisplayMode(const int theMode) const
{
  ActParamTool::AsInt( this->Parameter(PID_DisplayMode) )->SetValue(theMode);
}

//! Accessor for the stored display mode value.
//! \return display mode value.
int asiData_TriangulationNode::GetDisplayMode() const
{
  return ActParamTool::AsInt( this->Parameter(PID_DisplayMode) )->GetValue();
}

//! Sets the Boolean value indicating whether the vertices are active
//! \param hasVertices [in] value to set.
void asiData_TriangulationNode::SetHasVertices(const bool hasVertices)
{
  ActParamTool::AsBool( this->Parameter(PID_HasVertices) )->SetValue(hasVertices);
}

//! Accessor for the value of the Boolean Parameter indicating whether the
//! vertices are active.
//! \return true/false.
bool asiData_TriangulationNode::HasVertices() const
{
  return ActParamTool::AsBool( this->Parameter(PID_HasVertices) )->GetValue();
}
