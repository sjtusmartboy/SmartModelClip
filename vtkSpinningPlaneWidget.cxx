
#include "vtkSpinningPlaneWidget.h"

#include "vtkActor.h"
#include "vtkAssemblyNode.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCellPicker.h"
#include "vtkConeSource.h"
#include "vtkDoubleArray.h"
#include "vtkFloatArray.h"
#include "vtkLineSource.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPlane.h"
#include "vtkQuadPlaneSource.h"
#include "vtkPlanes.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"

vtkStandardNewMacro(vtkSpinningPlaneWidget);

vtkCxxSetObjectMacro(vtkSpinningPlaneWidget,PlaneProperty,vtkProperty);

vtkSpinningPlaneWidget::vtkSpinningPlaneWidget()
{
	this->State = vtkQuadPlaneWidget::Start;
	this->EventCallbackCommand->SetCallback(vtkSpinningPlaneWidget::ProcessEvents);

	this->SetHandleSize(1);
	this->SetRepresentationToSurface();
};

vtkSpinningPlaneWidget::~vtkSpinningPlaneWidget(){};

void vtkSpinningPlaneWidget::ProcessEvents(vtkObject* vtkNotUsed(object), 
                                   unsigned long event,
                                   void* clientdata, 
                                   void* vtkNotUsed(calldata))
{
	
  vtkSpinningPlaneWidget* self = reinterpret_cast<vtkSpinningPlaneWidget *>( clientdata );

  //okay, let's do the right thing
  switch(event)
    {
    case vtkCommand::LeftButtonPressEvent:
      self->OnLeftButtonDown();
      break;
    case vtkCommand::LeftButtonReleaseEvent:
      self->OnLeftButtonUp();
      break;
	case vtkCommand::RightButtonPressEvent:
      self->OnRightButtonDown();
      break;
    case vtkCommand::RightButtonReleaseEvent:
      self->OnRightButtonUp();
      break;
    case vtkCommand::MouseMoveEvent:
      self->OnMouseMove();
      break;

    }
}

void vtkSpinningPlaneWidget::OnLeftButtonDown()
{		
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
    this->State = vtkSpinningPlaneWidget::Outside;
    return;
    }
  
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then try to pick the plane.
  vtkAssemblyPath *path;
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
  {
	  this->State = vtkSpinningPlaneWidget::Moving;
	  this->HighlightHandle(path->GetFirstNode()->GetViewProp());
  }
  else
  {
	  this->BoundaryPicker->Pick(X,Y,0.0,this->CurrentRenderer);
	  path = this->BoundaryPicker->GetPath();
	  if ( path != NULL )
	  {
		  vtkProp *prop = path->GetFirstNode()->GetViewProp();
		  if(prop == this->BoundaryActor[2])
		  {
			  this->State = vtkQuadPlaneWidget::BoundaryDragging;
			  this->HighlightBoundary(path->GetFirstNode()->GetViewProp());
		  }
	  }
	  else 
	  {
		  this->PlanePicker->Pick(X,Y,0.0,this->CurrentRenderer);
		  path = this->PlanePicker->GetPath();

		  if ( path != NULL )
		  {
			  vtkProp *prop = path->GetFirstNode()->GetViewProp();
			  if ( prop == this->ConeActor || prop == this->LineActor ||
				  prop == this->ConeActor2 || prop == this->LineActor2 )
			  {
				  this->State = vtkSpinningPlaneWidget::Spinning;
				  this->HighlightNormal(1);
			  }
		  }
		  else
		  {
			  //rotate the view
			  this->State = vtkSpinningPlaneWidget::Outside;
			  this->HighlightHandle(NULL);
			  return;
		  }
	  }
  }

  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkSpinningPlaneWidget::OnLeftButtonUp()
{
  if ( this->State == vtkSpinningPlaneWidget::Outside ||
       this->State == vtkSpinningPlaneWidget::Start )
    {
    return;
    }

  this->State = vtkSpinningPlaneWidget::Start;
    if(this->CurrentHandle == this->Handle[0] || this->CurrentHandle == this->Handle[1] ||
  this->CurrentHandle == this->Handle[2] || this->CurrentHandle == this->Handle[3] )
		this->HighlightHandle(NULL);

  if(this->CurrentHandle == this->BoundaryActor[0] || this->CurrentHandle == this->BoundaryActor[1] ||
  this->CurrentHandle == this->BoundaryActor[2] || this->CurrentHandle == this->BoundaryActor[3] )
		this->HighlightBoundary(NULL);
  this->HighlightPlane(0);
  this->HighlightNormal(0);
  //this->SizeHandles();

  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkSpinningPlaneWidget::OnRightButtonDown()
{
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Okay, make sure that the pick is in the current renderer
  if (!this->CurrentRenderer || !this->CurrentRenderer->IsInViewport(X, Y))
    {
    this->State = vtkSpinningPlaneWidget::Outside;
    return;
    }
  
  // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  vtkAssemblyPath *path;
  this->HandlePicker->Pick(X,Y,0.0,this->CurrentRenderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
    {
    this->State = vtkSpinningPlaneWidget::Scaling;
    this->HighlightPlane(1);
    this->HighlightHandle(path->GetFirstNode()->GetViewProp());
    }
  else //see if we picked the plane or a normal
    {
    this->PlanePicker->Pick(X,Y,0.0,this->CurrentRenderer);
    path = this->PlanePicker->GetPath();
    if ( path == NULL )
      {
      this->State = vtkSpinningPlaneWidget::Outside;
      return;
      }
    }
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->StartInteraction();
  this->InvokeEvent(vtkCommand::StartInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkSpinningPlaneWidget::OnRightButtonUp()
{
  if ( this->State == vtkSpinningPlaneWidget::Outside ||
       this->State == vtkSpinningPlaneWidget::Start )
    {
    return;
    }

  this->State = vtkSpinningPlaneWidget::Start;
  this->HighlightPlane(0);
  //this->SizeHandles();
  
  this->EventCallbackCommand->SetAbortFlag(1);
  this->EndInteraction();
  this->InvokeEvent(vtkCommand::EndInteractionEvent,NULL);
  this->Interactor->Render();
}

void vtkSpinningPlaneWidget::OnMouseMove()
{
  // See whether we're active
  if ( this->State == vtkSpinningPlaneWidget::Outside || 
       this->State == vtkSpinningPlaneWidget::Start )
    {
    return;
    }
  
  int X = this->Interactor->GetEventPosition()[0];
  int Y = this->Interactor->GetEventPosition()[1];

  // Do different things depending on state
  // Calculations everybody does
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z;

  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  if ( !camera )
  {
	  return;
  }

  // Compute the two points defining the motion vector
  this->ComputeWorldToDisplay(this->LastPickPosition[0], 
                              this->LastPickPosition[1],
                              this->LastPickPosition[2], focalPoint);
  z = focalPoint[2];
  this->ComputeDisplayToWorld(
    double(this->Interactor->GetLastEventPosition()[0]),
    double(this->Interactor->GetLastEventPosition()[1]),
    z, prevPickPoint);
  this->ComputeDisplayToWorld(double(X), double(Y), z, pickPoint);
  
  // Process the motion
  if ( this->State == vtkSpinningPlaneWidget::Moving )
  {
	  // Okay to process

	  if ( this->CurrentHandle == this->Handle[2] )
	  {
		  this->MovePoint2(prevPickPoint, pickPoint);
	  }
	  else if ( this->CurrentHandle == this->Handle[3] )
	  {
		  this->MovePoint3(prevPickPoint, pickPoint);
	  }

  }
  else if ( this->State == vtkSpinningPlaneWidget::Spinning )
  {
	  this->Spin(prevPickPoint, pickPoint);
  }
  else if (this->State == vtkQuadPlaneWidget::BoundaryDragging)
  {
	  this->BoundaryDrag(prevPickPoint, pickPoint);
  }

// Interact, if desired
this->EventCallbackCommand->SetAbortFlag(1);
this->InvokeEvent(vtkCommand::InteractionEvent,NULL);

this->Interactor->Render();
}


void vtkSpinningPlaneWidget::Spin(double *p1,double *p2)
{
  double *o = this->PlaneSource->GetOrigin();
  double *pt1 = this->PlaneSource->GetPoint1();
  double *pt2 = this->PlaneSource->GetPoint2();
  double *pt3 = this->PlaneSource->GetPoint3();

  double v[3]; //vector of motion
  double axis[3]; //axis of rotation
  double center[3];

  for (int i = 0; i < 3; i++) {
    v[i] = p2[i] - p1[i];   // mouse motion vector in world space
    axis[i] = pt1[i] - o[i];  // Create axis of rotation and angle of rotation
	center[i] = 0.5*(pt1[i]+o[i]);
  }
  
  vtkMath::Normalize(axis);

  // Radius vector (from center to cursor position)
  double rv[3] = {p2[0] - o[0],
                  p2[1] - o[1],
                  p2[2] - o[2]};

  // Distance between center and cursor location
  double rs = vtkMath::Normalize(rv);

  // Spin direction
  double ax_cross_rv[3];
  vtkMath::Cross(axis,rv,ax_cross_rv);

  // Spin angle
  double theta = vtkMath::DegreesFromRadians( vtkMath::Dot( v, ax_cross_rv ) / rs );

  this->Transform->Identity();
  this->Transform->Translate(center[0],center[1],center[2]);
  this->Transform->RotateWXYZ(theta,axis);
  this->Transform->Translate(-center[0],-center[1],-center[2]);

  //Set the corners
  double oNew[3], pt1New[3], pt2New[3],pt3New[3];
  this->Transform->TransformPoint(o,oNew);
  this->Transform->TransformPoint(pt1,pt1New);
  this->Transform->TransformPoint(pt2,pt2New);
  this->Transform->TransformPoint(pt3,pt3New);

  this->PlaneSource->SetOrigin(oNew);
  this->PlaneSource->SetPoint1(pt1New);
  this->PlaneSource->SetPoint2(pt2New);
  this->PlaneSource->SetPoint3(pt3New);
  this->PlaneSource->Update();

  this->PositionHandles();
}

int vtkSpinningPlaneWidget::HighlightHandle(vtkProp *prop)
{
  // first unhighlight anything picked
  if ( this->CurrentHandle )
    {
    this->CurrentHandle->SetProperty(this->HandleProperty);
    }

  this->CurrentHandle = static_cast<vtkActor *>(prop);

  if ( this->CurrentHandle )
    {
    this->ValidPick = 1;
    this->HandlePicker->GetPickPosition(this->LastPickPosition);
	if ( (this->CurrentHandle == this->Handle[2])||(this->CurrentHandle ==this->Handle[3] ))
		this->CurrentHandle->SetProperty(this->SelectedHandleProperty);
    for (int i=0; i<4; i++) //find handle
      {
      if ( this->CurrentHandle == this->Handle[i] )
        {
        return i;
        }
      }
    }
  
  return -1;
}


int vtkSpinningPlaneWidget::HighlightBoundary(vtkProp *prop)
{
  // first unhighlight anything picked
  if ( this->CurrentHandle )
    {
    this->CurrentHandle->SetProperty(this->BoundaryProperty);
    }

  this->CurrentHandle = static_cast<vtkActor *>(prop);

  if ( this->CurrentHandle )
    {
    this->ValidPick = 1;
    this->BoundaryPicker->GetPickPosition(this->LastPickPosition);
    this->CurrentHandle->SetProperty(this->SelectedBoundaryProperty);

      if ( this->CurrentHandle == this->BoundaryActor[2] )
        {
        return 2;
        }
    }
  
  return -1;
}
