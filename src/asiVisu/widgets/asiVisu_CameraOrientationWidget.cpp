// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "asiVisu_CameraOrientationWidget.h"

#include <vtkAbstractWidget.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <asiVisu_CameraInterpolator.h>
#include <asiVisu_CameraOrientationRepresentation.h>
#include <vtkCommand.h>
#include <vtkEvent.h>
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkWidgetCallbackMapper.h>
#include <vtkWidgetEvent.h>

#include <asiVisu_Utils.h>

//----------------------------------------------------------------------------

vtkStandardNewMacro(asiVisu_CameraOrientationWidget);

//----------------------------------------------------------------------------
asiVisu_CameraOrientationWidget::asiVisu_CameraOrientationWidget()
{
  // Define widget events.
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonPressEvent, vtkEvent::NoModifier, 0,
    0, nullptr, vtkWidgetEvent::Select, this, asiVisu_CameraOrientationWidget::SelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent, vtkEvent::NoModifier,
    0, 0, nullptr, vtkWidgetEvent::EndSelect, this, asiVisu_CameraOrientationWidget::EndSelectAction);
  this->CallbackMapper->SetCallbackMethod(vtkCommand::MouseMoveEvent, vtkEvent::NoModifier, 0, 0,
    nullptr, vtkWidgetEvent::Rotate, this, asiVisu_CameraOrientationWidget::MoveAction);

  this->CameraInterpolator->SetInterpolationTypeToSpline();

  // Initialize a default renderer.
  vtkNew<vtkRenderer> renderer;
  renderer->SetViewport(0.8, 0.8, 1.0, 1.0);
  renderer->GetActiveCamera()->ParallelProjectionOff();
  renderer->GetActiveCamera()->Dolly(0.25);
  renderer->InteractiveOff();
  renderer->SetLayer(1);
  this->SetDefaultRenderer(renderer);
}

//----------------------------------------------------------------------------
asiVisu_CameraOrientationWidget::~asiVisu_CameraOrientationWidget() = default;

//------------------------------------------------------------------------------
vtkRenderer* asiVisu_CameraOrientationWidget::GetParentRenderer()
{
  return this->ParentRenderer;
}

//------------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::SetDefaultRenderer(vtkRenderer* renderer)
{
  if (renderer == this->DefaultRenderer)
  {
    return;
  }
  // remove reorientation observer
  if (this->DefaultRenderer != nullptr)
  {
    this->DefaultRenderer->RemoveObserver(this->ReorientObserverTag);
  }
  const bool reEnable = this->Enabled;
  if (this->Enabled)
  {
    // remove previous default renderer from render window.
    if (this->Interactor)
    {
      this->Interactor->GetRenderWindow()->RemoveRenderer(this->DefaultRenderer);
    }
    this->SetEnabled(false);
  }

  // install observer to sync camera widget orientation with that of parent renderer's camera
  this->ReorientObserverTag = renderer->AddObserver(
    vtkCommand::StartEvent, this, &asiVisu_CameraOrientationWidget::OrientWidgetRepresentation);
  this->Superclass::SetDefaultRenderer(renderer);

  if (reEnable)
  {
    this->SetEnabled(true);
    if (this->Interactor)
    {
      this->Interactor->GetRenderWindow()->AddRenderer(this->DefaultRenderer);
    }
  }
}

//------------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::SetParentRenderer(vtkRenderer* parentRen)
{
  if (this->ParentRenderer == parentRen)
  {
    return;
  }

  // detach from previous parent renderer
  if (this->ParentRenderer != nullptr)
  {
    auto renWin = this->ParentRenderer->GetRenderWindow();
    if (renWin != nullptr)
    {
      if (renWin->HasRenderer(this->DefaultRenderer))
      {
        renWin->RemoveRenderer(this->DefaultRenderer);
      }
      const int& numLayers = renWin->GetNumberOfLayers();
      renWin->SetNumberOfLayers(numLayers - 1);
      renWin->RemoveObserver(this->ResizeObserverTag);
    }
  }

  // attach to given parent.
  if (parentRen != nullptr)
  {
    auto renWin = parentRen->GetRenderWindow();
    if (renWin != nullptr)
    {
      if (!renWin->HasRenderer(this->DefaultRenderer))
      {
        renWin->AddRenderer(this->DefaultRenderer);
      }
      this->SetInteractor(renWin->GetInteractor());
      const int& numLayers = renWin->GetNumberOfLayers();
      renWin->SetNumberOfLayers(numLayers + 1);
      // In order to occupy sufficient space as per the padding and size of the representation,
      // the widget always invokes the SquareResize callback at the beginning of every frame.
      // We do it like that because the viewport (xmin,xmax, ymin, ymax) of the DefaultRenderer
      // may be different than the previously computed values. Otherwise, in a
      // serialization/deserialization setup, the viewport values could revert back since a resize
      // event is never triggered upon deserialization. This approach is acceptable since the
      // SquareResize method is quite efficient.
      this->ResizeObserverTag = renWin->AddObserver(
        vtkCommand::StartEvent, this, &asiVisu_CameraOrientationWidget::SquareResize);
    }
  }

  // assign
  this->ParentRenderer = parentRen;
  this->Modified();
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::SetRepresentation(asiVisu_CameraOrientationRepresentation* r)
{
  this->Superclass::SetWidgetRepresentation(r);
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::CreateDefaultRepresentation()
{
  if (!this->WidgetRep)
  {
    this->WidgetRep = asiVisu_CameraOrientationRepresentation::New();
  }
}

void asiVisu_CameraOrientationWidget::ComputeWidgetState(int X, int Y, int modify /* =0*/)
{
  auto rep = asiVisu_CameraOrientationRepresentation::SafeDownCast(this->WidgetRep);
  if (rep == nullptr)
  {
    return;
  }

  // Compute and get representation's interaction state.
  this->WidgetRep->ComputeInteractionState(X, Y, modify);
  const auto& interactionState = rep->GetInteractionStateAsEnum();

  // Synchronize widget state with representation.
  if (interactionState == asiVisu_CameraOrientationRepresentation::InteractionStateType::Outside)
  {
    this->WidgetState = WidgetStateType::Inactive;
  }
  else if (interactionState == asiVisu_CameraOrientationRepresentation::InteractionStateType::Hovering)
  {
    this->WidgetState = WidgetStateType::Hot;
  }

  // Refresh representation to match interaction state.
  rep->ApplyInteractionState(interactionState);
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::SelectAction(vtkAbstractWidget* w)
{
  // cast to ourself
  asiVisu_CameraOrientationWidget* const self = asiVisu_CameraOrientationWidget::SafeDownCast(w);
  if (self == nullptr)
  {
    return;
  }

  // can only be selected if already hot.
  if (self->WidgetState != WidgetStateType::Hot)
  {
    return;
  }

  // Get event position.
  const int& X = self->Interactor->GetEventPosition()[0];
  const int& Y = self->Interactor->GetEventPosition()[1];

  // Begin widget interaction.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);

  // we're now activated
  self->WidgetState = WidgetStateType::Active;

  // this captures the event position.
  self->WidgetRep->StartWidgetInteraction(e);
  self->GrabFocus(self->EventCallbackCommand);

  self->EventCallbackCommand->AbortFlagOn();
  self->StartInteraction();
  self->InvokeEvent(vtkCommand::StartInteractionEvent);
  self->Render();
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::EndSelectAction(vtkAbstractWidget* w)
{
  // cast to ourself
  asiVisu_CameraOrientationWidget* const self = asiVisu_CameraOrientationWidget::SafeDownCast(w);
  if (self == nullptr)
  {
    return;
  }
  auto rep = asiVisu_CameraOrientationRepresentation::SafeDownCast(self->WidgetRep);
  if (rep == nullptr)
  {
    return;
  }

  // can only be deselected if already selected
  if (self->WidgetState != WidgetStateType::Active)
  {
    return;
  }

  if (self->ParentRenderer == nullptr)
  {
    return;
  }

  // get event position.
  const int& X = self->Interactor->GetEventPosition()[0];
  const int& Y = self->Interactor->GetEventPosition()[1];

  // end widget interaction.
  double e[2];
  e[0] = static_cast<double>(X);
  e[1] = static_cast<double>(Y);
  self->WidgetRep->EndWidgetInteraction(e);

  // deactivate widget.
  self->WidgetState = WidgetStateType::Inactive;

  // synchronize orientations
  if (rep->IsAnyHandleSelected() &&
    (rep->GetInteractionStateAsEnum() ==
      asiVisu_CameraOrientationRepresentation::InteractionStateType::Hovering))
  {
    double back[3], up[3];
    rep->GetBack(back);
    rep->GetUp(up);

    self->OrientParentCamera(back, up);
    // this fires off animation if needed
    if (self->Animate && self->AnimationTimerObserverTag == -1)
    {
      // update gizmo and camera to new orientation step by step.
      self->StartAnimation();
      return;
    }
    else
    {
     // self->ParentRenderer->ResetCamera();
      self->Render();
    }
  }

  // one might move the mouse out of the widget's interactive area during animation
  // need to compute state.
  self->ComputeWidgetState(X, Y, 1);

  self->ReleaseFocus();
  self->EventCallbackCommand->AbortFlagOn();
  self->EndInteraction();
  self->InvokeEvent(vtkCommand::EndInteractionEvent);
  self->Render();
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::StartAnimation()
{
  this->AnimatorCurrentFrame = 1;
  this->AnimationTimerId = this->Interactor->CreateRepeatingTimer(1);
  this->AnimationTimerObserverTag = this->Interactor->AddObserver(
    vtkCommand::TimerEvent, this, &asiVisu_CameraOrientationWidget::PlayAnimationSingleFrame);
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::PlayAnimationSingleFrame(
  vtkObject*, unsigned long event, void* callData)
{
  if (event == vtkCommand::TimerEvent &&
    (*reinterpret_cast<int*>(callData)) == this->AnimationTimerId)
  {
    if (this->AnimatorCurrentFrame < this->AnimatorTotalFrames)
    {
      this->InterpolateCamera(this->AnimatorCurrentFrame);
      //this->ParentRenderer->ResetCamera();
      this->Render();
      this->AnimatorCurrentFrame++;
    }
    else
    {
      this->StopAnimation();
    }
  }
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::StopAnimation()
{
  if (this->Interactor->DestroyTimer(this->AnimationTimerId))
  {
    this->Interactor->RemoveObserver(this->AnimationTimerObserverTag);
    this->AnimationTimerObserverTag = -1;
    // get event position.
    const int& X = this->Interactor->GetEventPosition()[0];
    const int& Y = this->Interactor->GetEventPosition()[1];
    // one might have moved the mouse out of the widget's interactive area during animation
    // need to compute state.
    this->ComputeWidgetState(X, Y, 1);

    this->ReleaseFocus();
    this->EventCallbackCommand->AbortFlagOn();
    this->EndInteraction();
    this->InvokeEvent(vtkCommand::EndInteractionEvent);
    this->Render();
  }
  else
  {
    vtkErrorMacro(<< "Failed to stop animation timer " << this->AnimationTimerId);
  }
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::MoveAction(vtkAbstractWidget* w)
{
  // cast to ourself
  asiVisu_CameraOrientationWidget* const self = asiVisu_CameraOrientationWidget::SafeDownCast(w);
  if (self == nullptr)
  {
    return;
  }
  auto rep = asiVisu_CameraOrientationRepresentation::SafeDownCast(self->WidgetRep);
  if (rep == nullptr)
  {
    return;
  }

  // Get event position.
  const int& X = self->Interactor->GetEventPosition()[0];
  const int& Y = self->Interactor->GetEventPosition()[1];

  // can only rotate if previously selected, else simply compute widget state.
  if (self->WidgetState != WidgetStateType::Active)
  {
    self->ComputeWidgetState(X, Y, 1);
  }
  else // pick handle.
  {
    rep->ComputeInteractionState(X, Y, 0);
    if (self->ParentRenderer == nullptr)
    {
      return;
    }
    auto cam = self->ParentRenderer->GetActiveCamera();
    if (cam == nullptr)
    {
      return;
    }

    double e[2];
    e[0] = static_cast<double>(X);
    e[1] = static_cast<double>(Y);

    // compute representation's azimuth, elevation
    self->WidgetRep->WidgetInteraction(e);

    // copy widget's az, elev to parent cam.
    cam->Azimuth(rep->GetAzimuth());
    cam->Elevation(rep->GetElevation());
    cam->OrthogonalizeViewUp();
    self->ParentRenderer->ResetCameraClippingRange();
    if (self->Interactor->GetLightFollowCamera())
    {
      self->ParentRenderer->UpdateLightsGeometryToFollowCamera();
    }

    self->EventCallbackCommand->AbortFlagOn();
    self->InvokeEvent(vtkCommand::InteractionEvent);
  }
  if (self->WidgetState != WidgetStateType::Inactive)
  {
    self->Render();
  }
}

//-----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::OrientParentCamera(double back[3], double up[3])
{
  if (this->ParentRenderer == nullptr)
  {
    return;
  }

  vtkCamera* cam = this->ParentRenderer->GetActiveCamera();

  this->CameraInterpolator->Initialize();

  // get old camera vars
  double dstPos[3] = {}, srcPos[3] = {}, srcUp[3] = {}, focalP[3] = {}, distV[3] = {};
  cam->GetFocalPoint(focalP);
  cam->GetPosition(srcPos);
  cam->GetViewUp(srcUp);
  this->CameraInterpolator->AddCamera(0, cam);

  // move camera to look down 'back'
  vtkMath::Subtract(srcPos, focalP, distV);
  double dist = vtkMath::Norm(distV);
  for (int i = 0; i < 3; ++i)
  {
    dstPos[i] = focalP[i] - back[i] * dist;
  }

  // set new camera vars
  cam->SetFocalPoint(focalP);
  cam->SetPosition(dstPos);
  cam->SetViewUp(up);
  cam->ComputeViewPlaneNormal();
  this->CameraInterpolator->AddCamera(this->AnimatorTotalFrames - 1, cam);
}

//-----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::OrientWidgetRepresentation()
{
  if (this->ParentRenderer == nullptr)
  {
    return;
  }
  auto rep = asiVisu_CameraOrientationRepresentation::SafeDownCast(this->WidgetRep);
  if (rep == nullptr)
  {
    return;
  }
  vtkCamera* cam = this->ParentRenderer->GetActiveCamera();
  if (cam != nullptr)
  {
    const double* orient = cam->GetOrientationWXYZ();
    const double& angle = orient[0];
    const double* axis = orient + 1;

    rep->GetTransform()->Identity();
    rep->GetTransform()->RotateWXYZ(angle, axis);
  }
}

//-----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::InterpolateCamera(int t)
{
  if (this->ParentRenderer == nullptr)
  {
    return;
  }
  vtkCamera* cam = this->ParentRenderer->GetActiveCamera();
  if (cam == nullptr)
  {
    return;
  }
  this->CameraInterpolator->InterpolateCamera(t, cam);
  cam->OrthogonalizeViewUp(); // the interpolation results in invalid view up, sometimes ..
  cam->ComputeViewPlaneNormal();
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::SquareResize()
{
  if (this->DefaultRenderer == nullptr)
  {
    return;
  }

  auto renWin = this->DefaultRenderer->GetRenderWindow();
  if (renWin == nullptr)
  {
    return;
  }
  auto rep = asiVisu_CameraOrientationRepresentation::SafeDownCast(this->WidgetRep);
  if (rep == nullptr)
  {
    return;
  }

  const int* const size = rep->GetSize();
  const int maxSz = (size[0] > size[1]) ? size[0] : size[1];
  const int* const padding = rep->GetPadding();
  const auto& anchoredTo = rep->GetAnchorPosition();
  double xmin = 0., xmax = 0., ymin = 0., ymax = 0.;
  // vp: ViewPort | pad: Padding | w: width | h: height
  const double vpw = static_cast<double>(maxSz) / renWin->GetActualSize()[0];
  const double vph = static_cast<double>(maxSz) / renWin->GetActualSize()[1];
  const double vppadw = static_cast<double>(padding[0]) / renWin->GetActualSize()[0];
  const double vppadh = static_cast<double>(padding[1]) / renWin->GetActualSize()[1];

  switch (anchoredTo)
  {
    case asiVisu_CameraOrientationRepresentation::AnchorType::LowerLeft:
      xmin = 0. + vppadw;
      xmax = vpw + vppadw;
      ymin = 0. + vppadh;
      ymax = vph + vppadh;
      break;
    case asiVisu_CameraOrientationRepresentation::AnchorType::LowerRight:
      xmin = 1. - vpw - vppadw;
      xmax = 1. - vppadw;
      ymin = 0. + vppadh;
      ymax = vph + vppadh;
      break;
    case asiVisu_CameraOrientationRepresentation::AnchorType::UpperLeft:
      xmin = 0.0 + vppadw;
      xmax = vpw + vppadw;
      ymin = 1. - vph - vppadh;
      ymax = 1. - vppadh;
      break;
    case asiVisu_CameraOrientationRepresentation::AnchorType::UpperRight:
      xmin = 1. - vpw - vppadw;
      xmax = 1. - vppadw;
      ymin = 1. - vph - vppadh;
      ymax = 1. - vppadh;
      break;
    default:
      break;
  }
  this->DefaultRenderer->SetViewport(xmin, ymin, xmax, ymax);
}

//----------------------------------------------------------------------------
void asiVisu_CameraOrientationWidget::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
